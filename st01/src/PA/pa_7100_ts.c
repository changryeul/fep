#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : 시세분배 TCP 송신
#   File    : pa_7100_ts.c
#
#   설명  : FIFO에서 시세 데이터를 읽어 TCP 소켓으로 Client에
#             송신하는 프로세스. 채권/파생 시세를 TR코드별로 분류하여
#             길이 헤더 없이(0xFF 종단) 전송한다.
#             A7612=채권시세(700바이트), A7613=파생시세(100바이트).
#
#   동작 흐름:
#             1) TCP Connect → Client 접속
#             2) poll() 대기 (소켓/FIFO 이벤트)
#             3) FIFO 이벤트 → 시세 데이터 읽기 → TCP 송신
#             4) 타임아웃 → 링크 체크(LK) 패킷 송신
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "cli_interface.h"

#define     MAX_CNT         1

/*
 * DATA_SIZE: 빌드 옵션에 따라 시세 데이터 크기 결정
 *   A7612: 채권시세 → 700바이트 (A301K, G701K, B601K, A601K)
 *   A7613: 파생시세 → 100바이트 (A701K, M401K)
 */
#if defined A7612
#define     DATA_SIZE       700
#elif defined A7613
#define     DATA_SIZE       100
#endif
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     FOREVER_TIME    60 * 1000       /* Poll 대기 시간 (60초, 밀리초 단위) */

#define     SOCKET_EVENT    0               /* Poll 이벤트: TCP 소켓 */
#define     DATA_EVENT      1               /* Poll 이벤트: FIFO 데이터 */

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int         Sockfd, Newfd;                  /* TCP 소켓 FD, 접속 FD */
int         SendLen, MsgLen, ReTrCode;      /* 송신 길이, 메시지 길이, TR코드 */
int         PollCnt, FirstSeq, TimeOut, Pk, PortNo, SendFlag;
char        ApType[10], IpAddr[20];         /* 어플리케이션 타입, 접속 IP */
char        RecvPkt[CLI_BUFF_MAX_LEN], SendPkt[CLI_BUFF_MAX_LEN];   /* 수신/송신 패킷 버퍼 */
char        LogOnFlag, OpenFlag;            /* 접속 상태 플래그 */

FILE_BUFF_FORMAT    R_Fmt[MAX_CNT], W_Fmt[MAX_CNT]; /* FIFO 읽기/쓰기 버퍼 */

struct pollfd       Poll[2];                /* poll 이벤트 배열: [0]=소켓, [1]=FIFO */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_7100_TS(void);
void    Init_Parameters(void);
void    Fifo_Event_Rtn(void);
void    Socket_Event_Rtn(void);
void    Data_Event_Rtn(void);
void    Device_Write(void);
void    Device_Open(void);
void    Device_Close(void);
void    Time_Out_Rtn(void);
void    Make_Send_Msg(int);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 시세 TCP 송신 루프 실행.          */
/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);                 /* 프로세스 초기화 */
    PA_7100_TS();                          /* 메인 처리 루프 */
    Exit_Process();                        /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PA_7100_TS: 메인 처리 루프                                            */
/*    1) 주말이면 대기 (HOLIDAY_APPLY 옵션)                             */
/*    2) TCP 연결 후 poll() 기반 이벤트 처리                          */
/*    3) SOCKET_EVENT → 소켓 이벤트 처리                               */
/*    4) DATA_EVENT → FIFO에서 시세 읽어 TCP 송신                       */
/*    5) 타임아웃 → 링크 체크 패킷 송신                                 */
/*----------------------------------------------------------------------*/
void    PA_7100_TS(void) {
    int     rt, i;

    Init_Parameters();

#if defined HOLIDAY_APPLY
    /*
     * 주말 체크: 토/일요일이면 60초 간격으로 대기
     * 평일이 되면 루프를 빠져나와 정상 처리 시작
     */
    char    t_time[12], dt[20];
    time_t  t = time(NULL);
    struct  tm  tm, *tp;

    while (1) {
        Get_Time(t_time);

        memset(dt, 0, sizeof (dt));
        sprintf(dt, "%.4s-%.2s-%.2s %.2s:%.2s:%.2s", DAEMON(D_K).date,
                DAEMON(D_K).date+4, DAEMON(D_K).date+6, t_time, t_time+2, t_time+4);
        strptime(dt, "%Y-%m-%d %H:%M:%S", &tm);
        tp = localtime(&t);

        if (tp->tm_wday == 0 || tp->tm_wday == 6)       /* 일요일(0) 또는 토요일(6) */ {
            Log(USR_OK, "it's weekend. sleeping...[%d]", tp->tm_wday);
            sleep(60);
        }
        else {
            Log(USR_OK, "it's not weekend. not sleeping...[%d]", tp->tm_wday);
            break;
        }
    }
#endif

    /* === 메인 루프: 프로세스 종료 신호(END)까지 반복 === */
    while (START_S != END) {
        Stat_Save();

        /*
         * TCP 연결 상태 관리:
         *   NET_STA == END/JOB_STOP → 종료/중지 상태
         *   그 외 → 정상 주문 시간 → TCP 연결 유지
         */
        if (TCP2_NET_STA(0) == END || TCP2_NET_STA(0) == JOB_STOP) {
            /* 종료 상태: 접속 중이면 연결 해제 */
            if (LogOnFlag == ON && TCP2_NET_STA(0) == END) {
                Device_Close();
            }
        }
        else {
            /* 정상 상태: 미접속이면 TCP 연결 시도 */
            if (OpenFlag == OFF) {
                Device_Open();
                TCP2_NET_STA(0) = ON;

                if (OpenFlag == OFF)
                    continue;               /* 접속 실패 → 재시도 */
            }

            PollCnt = 2;                    /* poll 감시 대상: 소켓 + FIFO */
            TimeOut = FOREVER_TIME;         /* 60초 타임아웃 */
        }

        /* poll() 이벤트 대기 */
        rt = poll(Poll, PollCnt, TimeOut);
        if (rt < 0) {
            if (SYS_NO == EINTR)
                Log(SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
            else
                Log(SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);

            continue;
        }
        else if (rt == 0) {
            Time_Out_Rtn();                /* 타임아웃 → 링크 체크 */
            continue;
        }

        /* 이벤트 감지: detect_poll_event로 어떤 FD에서 이벤트가 발생했는지 확인 */
        i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);
        if (i == -1) return;                /* 심각한 오류 → 종료 */
            if (i == -2) continue;              /* 무시할 이벤트 → 다음 루프 */

            switch (i) {
            case SOCKET_EVENT:              /* TCP 소켓 이벤트 (접속/해제) */
                Socket_Event_Rtn();
                break;
            case DATA_EVENT:                /* FIFO 데이터 도착 → 시세 송신 */
                Data_Event_Rtn();
                break;
            default:
                Log(USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                Exit_Process();
                break;
        }
    }

    return;
}   /* End of PA_7100_TS () */

/*************************************************************************
    Function        : Init_Parameters
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 전역 변수 초기화.
                      TCP 접속 IP/Port 설정, poll FD 등록.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Init_Parameters(void) {
    LogOnFlag = OFF;                        /* 로그온 상태 OFF */
    OpenFlag = OFF;                         /* TCP 연결 상태 OFF */

    L_K = 0;                                /* 라인 키 초기화 */

    /* TCP 접속 대상 IP 주소 조합 (cfg의 tcp2.ini에서 설정) */
    sprintf(IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
            TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));

    PortNo = TCP2_PORT_NO;
    Log(TCP_OK, "Server Side port[%d]", PortNo);

    TCP2_PROC_ST = ON;                      /* 프로세스 상태: 가동중 */
    TCP2_LINE_ST = OFF;                     /* 라인 상태: 미접속 */

    /* FIFO 이벤트용 poll FD 등록 */
    Poll[1].fd = INPUT_FD;
    Poll[1].events = POLLIN;

    return;
}   /* End of Init_Parameters () */

/*************************************************************************
    Function        : Socket_Event_Rtn
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : TCP 소켓 이벤트 처리 (현재 로그만 출력).
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Socket_Event_Rtn(void) {
    Log(USR_OK, "Socket_Event_Rtn Ok");

    return;
}   /* End of Socket_Event_Rtn () */

/*************************************************************************
    Function        : Data_Event_Rtn
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : FIFO 데이터 이벤트 처리.
                      1) Make_Send_Msg()로 시세 데이터 → 송신 패킷 조립
                      2) Device_Write()로 TCP 송신
                      3) FIFO 잔여 알림 데이터 소진 (read loop)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Data_Event_Rtn(void) {
    int     rt, r_cnt, i, j;
    char    tmp[128];

    Make_Send_Msg(TR_DATA);                /* 시세 데이터로 송신 패킷 조립 */

    /* TCP 송신 */
    Device_Write();
    Set_TR_Time();

    /*
     * FIFO 알림 데이터 소진:
     * poll()이 FIFO 이벤트를 감지하면 알림 바이트가 남아 있으므로
     * 다음 poll()이 즉시 리턴되지 않도록 모두 읽어서 소진함
     */
    while (1) {
        rt = read(INPUT_FD, tmp, sizeof(tmp));

        if (rt == 0)
            break;                          /* 더 이상 데이터 없음 */
        else if (rt == -1) {
            if (SYS_NO == EINTR)
                continue;                   /* 시그널 인터럽트 → 재시도 */
            if (SYS_NO != 11)               /* EAGAIN(11) 외 오류만 로그 */
                Log(FIF_ERROR, "Poll:cannot read FIFO {%d:%s}", SYS_NO, SYS_STR);
            break;
        }
    }

    return;
}   /* End of Data_Event_Rtn () */

/*************************************************************************
    Function        : Device_Open
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : TCP 소켓 생성 및 서버 접속.
                      접속 성공 시 OpenFlag/LogOnFlag = ON,
                      poll FD에 소켓 등록.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Open(void) {
    int     rt, rval;

    Sockfd = Socket();

    if (Sockfd < 0) {
        SLog(TCP_ERROR, "socket fail:Sockfd[%d] {%d:%s}",
                Sockfd, SYS_NO, SYS_STR);
        return;
    }

    SLog(USR_OK, "socket created:Sockfd[%d]", Sockfd);
    SLog(USR_OK, "connecting to %s:%d", IpAddr, TCP2_PORT_NO);

    rt = Connect(Sockfd, IpAddr, TCP2_PORT_NO);

    if (rt < 0) {
        SLog(TCP_ERROR, "connect fail {%d:%s}", SYS_NO, SYS_STR);
        close(Sockfd);
        return;
    }

    OpenFlag = LogOnFlag = ON;              /* 접속 성공 */

    Poll[1].fd = Sockfd;                    /* poll에 소켓 FD 등록 */
    Poll[1].events = POLLIN;
    SLog(TCP_OK, "TCP Connect & LOGON OK");

    return;
}   /* End of Device_Open () */

/*************************************************************************
    Function        : Device_Close
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : TCP 소켓 해제 및 상태 플래그 OFF 설정.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Close(void) {
    close(Sockfd);
    close(Newfd);
    Log(TCP_OK, "TCP device close");

    LogOnFlag = OFF;
    TCP2_LINE_ST = OpenFlag = OFF;

    return;
}   /* End of Device_Close () */

/*************************************************************************
    Function        : Device_Write
    Parameters IN   : (없음, 전역변수 SendPkt 사용)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : SendPkt의 내용을 TCP로 송신.
                      송신 실패 시 연결 해제.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Write(void) {
    int     rt;

    rt = Select_Send(Newfd, SendPkt, strlen(SendPkt));

    if (rt != OK) {
        Log(TCP_ERROR, "TCP data send fail");
        Device_Close();
    }

    Log(TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen(SendPkt), INT_SEQ);

    return;
}   /* End of Device_Write () */

/*************************************************************************
    Function        : Time_Out_Rtn
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 타임아웃(60초) 발생 시 링크 체크(LK) 패킷 송신.
                      접속 상태 확인용 하트비트 역할.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Time_Out_Rtn(void) {
    int     rt;
    char    in_num[10];

    Make_Send_Msg(TR_POLL);                /* 링크 체크 패킷 조립 */
    Device_Write();                        /* TCP 송신 */

    return;
}   /* End of Time_Out_Rtn () */

/*************************************************************************
    Function        : Make_Send_Msg
    Parameters IN   : tr_code - 패킷 유형 (TR_POLL=링크체크, TR_DATA=시세)
    Parameters OUT  : (없음, 전역변수 SendPkt에 결과 저장)
    Return Code     : void
    Comment         : 송신 패킷을 조립한다.
                      TR_POLL: "LK000000000" + 0xFF (링크 체크)
                      TR_DATA: FIFO에서 시세 읽어 TR코드별 구조체 크기만큼
                               복사 후 0xFF 종단 마커 추가

    패킷 포맷:
      [시세데이터(가변길이)] [0xFF 종단]
      ※ 길이 헤더 없음 — 0xFF로 패킷 경계 구분

    TR코드별 시세 구조체 (A7612 채권):
      A301K → CO_A301K (채권호가)
      G701K → CO_G701K (채권시세)
      B601K → CO_B601K (채권거래량)
      A601K → CO_A601K (채권종목정보)
    TR코드별 시세 구조체 (A7613 파생):
      A701K → CO_A701A (파생시세)
      M401K → CO_M401K (파생거래량)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Make_Send_Msg(int tr_code) {
    int     r_cnt, data_length;
    int     arry_cnt, arry_len, arry_dat;
    char    d_time[16];

    memset(SendPkt, 0, sizeof (SendPkt));

    if (tr_code == TR_POLL) {
        /* 링크 체크 패킷: "LK000000000" + 0xFF */
        memcpy(SendPkt, "LK000000000", 11);
        SendPkt[11] = (char)0xFF;
    }
    else                                                /* TR_DATA: 시세 데이터 송신 */ {
        /* FIFO에서 시세 데이터 읽기 */
        memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);
        r_cnt = F_R(PS_R_1, (void *)R_Fmt, MAX_CNT);
        if (r_cnt < 0 || r_cnt > MAX_CNT) {
            Log(SAM_FATAL, "F_R(PS_R_1) R cnt Err [%s] r_cnt[%d]", IDN(D_K,P_K,0), r_cnt);
            Exit_Process();
        }
        else if (r_cnt == 0) {
            return;                         /* 읽을 데이터 없음 */
        }
        else {
            /*
             * TR코드(앞 5바이트)로 시세 종류를 판별하여
             * 해당 구조체 크기만큼 SendPkt에 복사 후 0xFF 종단 추가
             *
             * 주의: 아래 memcmp은 원본 코드의 memcpy 오기(誤記)로 보임.
             *       memcmp은 비교만 하고 복사하지 않으므로 SendPkt에
             *       실제 데이터가 들어가지 않는 잠재적 이슈가 있음.
             */
#if defined A7612
            if (memcmp(&R_Fmt[0].Data, "A301K", 5) == 0) {
                memcmp(SendPkt, &R_Fmt[0].Data, sizeof(CO_A301K));
                SendPkt[sizeof(CO_A301K)] = (char)0xFF;
            }
            else if (memcmp(&R_Fmt[0].Data, "G701K", 5) == 0) {
                memcmp(SendPkt, &R_Fmt[0].Data, sizeof(CO_G701K));
                SendPkt[sizeof(CO_G701K)] = (char)0xFF;
            }
            else if (memcmp(&R_Fmt[0].Data, "B601K", 5) == 0) {
                memcmp(SendPkt, &R_Fmt[0].Data, sizeof(CO_B601K));
                SendPkt[sizeof(CO_B601K)] = (char)0xFF;
            }
            else if (memcmp(&R_Fmt[0].Data, "A601K", 5) == 0) {
                memcmp(SendPkt, &R_Fmt[0].Data, sizeof(CO_A601K));
                SendPkt[sizeof(CO_A601K)] = (char)0xFF;
            }
#elif defined A7613
            if (memcmp(&R_Fmt[0].Data, "A701K", 5) == 0) {
                memcmp(SendPkt, &R_Fmt[0].Data, sizeof(CO_A701A));
                SendPkt[sizeof(CO_A701A)] = (char)0xFF;
            }
            else if (memcmp(&R_Fmt[0].Data, "M401K", 5) == 0) {
                memcmp(SendPkt, &R_Fmt[0].Data, sizeof(CO_M401K));
                SendPkt[sizeof(CO_M401K)] = (char)0xFF;
            }
#endif
        }
    }

    return;
}   /* End of Make_Send_Msg () */

/*************************************************************************
    End of Program (pa_7100_ts.c)
*************************************************************************/
