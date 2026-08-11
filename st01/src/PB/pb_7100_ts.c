#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : 채권 시세분배 TCP 송신 (PB 모듈)
#   File    : pb_7100_ts.c
#
#   설명  : FIFO에서 시세/주문 데이터를 읽어 TCP 소켓으로 Client에
#             송신하는 프로세스. PA 모듈의 pa_7100_ts.c와 동일한
#             구조이며, PB 모듈 전용으로 빌드 옵션이 더 다양하다.
#
#   빌드 옵션별 DATA_SIZE:
#             B1401 → 400바이트 (주문접수: pa_1201_mp, pa_5201_qs)
#             B1402 → 400바이트 (주문접수+체결: pa_1201/1401/5201/5401)
#             B1403 → 200바이트 (장운영: pa_1601_mp)
#             B1411 → 1300바이트 (시세: pa_7801_dd)
#             B7612 → 700바이트 (채권시세: A301K/G701K/B601K/A601K)
#             B7613 → 100바이트 (파생시세: A701K/M401K)
#
#   동작 흐름:
#             1) TCP Connect → Client 접속
#             2) poll() 대기 (소켓/FIFO 이벤트)
#             3) 미처리 데이터 있으면 즉시 처리 (WR_CNT > RD_CNT)
#             4) FIFO 이벤트 → 데이터 읽기 → TCP 송신
#             5) 타임아웃 → 링크 체크(LK) 패킷 송신
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "cli_interface.h"

#define     MAX_CNT         1

/*
 * DATA_SIZE: 빌드 옵션에 따라 수신 데이터 크기 결정
 * 하나의 소스로 여러 바이너리를 빌드함
 */
#if defined B1401
#define     DATA_SIZE       400             /* 주문접수 */
#elif defined B1402
#define     DATA_SIZE       400             /* 주문접수+체결 */
#elif defined B1403
#define     DATA_SIZE       200             /* 장운영 */
#elif defined B1411
#define     DATA_SIZE       1300            /* 시세 */
#elif defined B7612
#define     DATA_SIZE       700             /* 채권시세 */
#elif defined B7613
#define     DATA_SIZE       100             /* 파생시세 */
#endif
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     FOREVER_TIME    30 * 1000       /* Poll 대기 시간 (30초, 밀리초 단위) */

#define     SOCKET_EVENT    0               /* Poll 이벤트: TCP 소켓 */
#define     DATA_EVENT      1               /* Poll 이벤트: FIFO 데이터 */

/* FIFO 쓰기/읽기 카운터 매크로 (미처리 데이터 확인용) */
#define WR_CNT W_CNT(0,0)                   /* 쓰기 카운터 */
#define RD_CNT R_CNT(0,0)                   /* 읽기 카운터 */

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
void    PB_7100_TS(void);
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
    PB_7100_TS();                          /* 메인 처리 루프 */
    Exit_Process();                        /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PB_7100_TS: 메인 처리 루프                                            */
/*    1) 주말이면 대기 (HOLIDAY_APPLY 옵션)                             */
/*    2) TCP 연결 후 poll() 기반 이벤트 처리                          */
/*    3) 미처리 데이터(WR_CNT > RD_CNT) 있으면 즉시 처리             */
/*    4) POLLHUP 감지 시 소켓 재접속                                    */
/*    5) SOCKET_EVENT/DATA_EVENT별 처리 함수 호출                      */
/*----------------------------------------------------------------------*/
void    PB_7100_TS(void) {
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
         *   그 외 → 정상 시간 → TCP 연결 유지
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
            TimeOut = FOREVER_TIME;         /* 30초 타임아웃 */

            /*
             * 미처리 데이터 즉시 처리:
             * FIFO의 쓰기 카운터(WR_CNT)가 읽기 카운터(RD_CNT)보다
             * 크면 아직 처리하지 않은 데이터가 있으므로
             * poll 대기 없이 즉시 Data_Event_Rtn 호출
             */
            if (WR_CNT > RD_CNT) {
                Data_Event_Rtn();
                continue;
            }

            Log(USR_OK, "HONG PollCnt [%d]", PollCnt);
            rt = poll(Poll, PollCnt, TimeOut);
            if (rt < 0) {
                if (SYS_NO == EINTR)
                    Log(SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
                else
                    Log(SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);

                continue;
            }
            else if (rt == 0) {
                Time_Out_Rtn();            /* 타임아웃 → 링크 체크 */
                continue;
            }

            /* poll 이벤트 단일 순회 — POLLHUP 체크 후 POLLIN 처리 */
            for (i = 0; i < PollCnt; i++) {
                if (Poll[i].revents & POLLHUP) {
                    if (i == SOCKET_EVENT) {
                        Log(TCP_ERROR, "socket disconnected[%#06x]", Poll[i].revents);
                        return;
                    }

                    Log(SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
                }

                if (Poll[i].revents & POLLIN) {
                    Poll[i].revents = 0;

                    switch (i) {
                        case SOCKET_EVENT:
                            Socket_Event_Rtn();
                            break;
                        case DATA_EVENT:
                            Data_Event_Rtn();
                            break;
                        default:
                            Log(USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                            Exit_Process();
                            break;
                    }
                }
            }
        }
    }

    return;
}   /* End of PB_7100_TS () */

/*************************************************************************
    Function        : Init_Parameters
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 전역 변수 초기화.
                      TCP 접속 IP/Port 설정, poll FD 등록.
                      B7612/B7613 빌드: 시세 스킵용 R=W 설정.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Init_Parameters(void) {
    LogOnFlag = OFF;                        /* 로그온 상태 OFF */
    OpenFlag = OFF;                         /* TCP 연결 상태 OFF */

    L_K = 0;                                /* 라인 키 초기화 */

    /* TCP 접속 대상 IP 주소 조합 */
    sprintf(IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
            TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));

    PortNo = TCP2_PORT_NO;
    Log(TCP_OK, "Server Side port[%d]", PortNo);

    TCP2_PROC_ST = ON;                      /* 프로세스 상태: 가동중 */
    TCP2_LINE_ST = OFF;                     /* 라인 상태: 미접속 */

    /* FIFO 이벤트용 poll FD 등록 */
    Poll[1].fd = INPUT_FD;
    Poll[1].events = POLLIN;

#if defined(B7612) || defined(B7613)
    /*
     * 시세 빌드: R=W 설정으로 기동 시 밀린 데이터를 스킵
     * 시세는 최신 데이터만 필요하므로 과거 데이터는 건너뜀
     * (주문/체결 데이터는 이 처리를 하지 않음)
     */
    R_CNT(0,1) = W_CNT(0,0);
#endif

    return;
}   /* End of Init_Parameters () */

/*************************************************************************
    Function        : Socket_Event_Rtn
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : TCP 소켓 이벤트 처리.
                      Client가 연결을 끊었을 때 호출됨.
                      소켓을 닫고 5초 후 재접속을 시도.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Socket_Event_Rtn(void) {
    Log(USR_OK, "Socket_Event_Rtn Ok");
    close(Sockfd);
    sleep(5);

    return;
}   /* End of Socket_Event_Rtn () */

/*************************************************************************
    Function        : Data_Event_Rtn
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : FIFO 데이터 이벤트 처리.
                      1) Make_Send_Msg()로 데이터 → 송신 패킷 조립
                      2) Device_Write()로 TCP 송신
                      3) FIFO 잔여 알림 데이터 소진
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Data_Event_Rtn(void) {
    int     rt, r_cnt, i, j;
    char    tmp[1024];

    Make_Send_Msg(TR_DATA);                /* 데이터로 송신 패킷 조립 */

    /* TCP 송신 */
    Device_Write();
    Set_TR_Time();

    /* FIFO 알림 데이터 소진 (다음 poll 대기를 위해) */
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
                      Poll[0]에 소켓 FD 등록.
                      접속 실패 시 3초 대기 후 리턴.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Open(void) {
    int     rt, rval;

    Sockfd = Socket();

    if (Sockfd < 0) {
        Log(TCP_ERROR, "socket fail:Sockfd[%d] {%d:%s}",
                Sockfd, SYS_NO, SYS_STR);
        return;
    }

    Log(USR_OK, "socket created:Sockfd[%d]", Sockfd);
    Log(USR_OK, "connecting to %s:%d", IpAddr, TCP2_PORT_NO);

    rt = Connect(Sockfd, IpAddr, TCP2_PORT_NO);

    if (rt < 0) {
        Log(TCP_ERROR, "connect fail {%d:%s}", SYS_NO, SYS_STR);
        close(Sockfd);
        sleep(3);
        return;
    }

    OpenFlag = LogOnFlag = ON;              /* 접속 성공 */

    Poll[0].fd = Sockfd;                    /* poll[0]에 소켓 FD 등록 */
    Poll[0].events = POLLIN;
    Log(TCP_OK, "TCP Connect & LOGON OK");

    return;
}   /* End of Device_Open () */

/*************************************************************************
    Function        : Device_Close
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : TCP 소켓 해제 및 상태 플래그 OFF 설정.
                      3초 대기 후 재접속을 위한 준비.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Close(void) {
    close(Sockfd);
    sleep(3);
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

    rt = Select_Send(Sockfd, SendPkt, strlen(SendPkt));

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
    Comment         : 타임아웃(30초) 발생 시 링크 체크(LK) 패킷 송신.
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
                      TR_POLL: "0011" + "LK000000000" (4바이트 길이 + 링크체크)
                      TR_DATA: "NNNN" + 시세데이터 (4바이트 길이 + 데이터)

    패킷 포맷 (PA 버전과 다름):
      [4바이트 길이 헤더] [시세 데이터]
      ※ PA 버전은 0xFF 종단 방식, PB 버전은 길이 헤더 방식

    B7612 채권시세 TR: A301K, G701K, B601K, A601K
    B7613 파생시세 TR: A701K, M401K
    기타 빌드: DATA_SIZE 고정 길이로 송신
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Make_Send_Msg(int tr_code) {
    int     r_cnt, data_length;
    int     arry_cnt, arry_len, arry_dat;
    char    d_time[16];

    memset(SendPkt, 0, sizeof (SendPkt));

    if (tr_code == TR_POLL) {
        /* 링크 체크 패킷: "0011" + "LK000000000" */
        memcpy(SendPkt, "0011", 4);
        memcpy(&SendPkt[4], "LK000000000", 11);
    }
    else                                                /* TR_DATA: 시세 데이터 송신 */ {
        /* FIFO에서 데이터 읽기 */
        memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);
        r_cnt = F_R(PS_R_1, (void *)R_Fmt, MAX_CNT);
        if (r_cnt < 0 || r_cnt > MAX_CNT) {
            Log(SAM_FATAL, "F_R(PS_R_1) R cnt Err [%s] r_cnt[%d]", IFN(D_K,P_K,0), r_cnt);
            Exit_Process();
        }
        else if (r_cnt == 0) {
            return;                         /* 읽을 데이터 없음 */
        }
        else {
            /*
             * TR코드(앞 5바이트)로 시세 종류를 판별하여
             * 해당 구조체 크기를 4바이트 길이 헤더에 기록 후
             * 데이터를 복사
             */
#if defined B7612
            if (memcmp(&R_Fmt[0].Data, "A301K", 5) == 0) {
                sprintf(SendPkt, "%04d", (int)sizeof(CO_A301K));
                memcpy(&SendPkt[4], &R_Fmt[0].Data, sizeof (CO_A301K));
            }
            else if (memcmp(&R_Fmt[0].Data, "G701K", 5) == 0) {
                sprintf(SendPkt, "%04d", (int)sizeof(CO_G701K));
                memcpy(&SendPkt[4], &R_Fmt[0].Data, sizeof(CO_G701K));
            }
            else if (memcmp(&R_Fmt[0].Data, "B601K", 5) == 0) {
                sprintf(SendPkt, "%04d", (int)sizeof (CO_B601K));
                memcpy(&SendPkt[4], &R_Fmt[0].Data, sizeof(CO_B601K));
            }
            else if (memcmp(&R_Fmt[0].Data, "A601K", 5) == 0) {
                sprintf(SendPkt, "%04d", (int)sizeof (CO_A601K));
                memcpy(&SendPkt[4], &R_Fmt[0].Data, sizeof (CO_A601K));
            }
#elif defined B7613
            if (memcmp(&R_Fmt[0].Data, "A701K", 5) == 0) {
                sprintf(SendPkt, "%04d", (int)sizeof (CO_A701K));
                memcpy(&SendPkt[4], &R_Fmt[0].Data, sizeof (CO_A701K));
            }
            else if (memcmp(&R_Fmt[0].Data, "M401K", 5) == 0) {
                sprintf(SendPkt, "%04d", (int)sizeof (CO_M401K));
                memcpy(&SendPkt[4], &R_Fmt[0].Data, sizeof (CO_M401K));
            }
#else
            /* 기타 빌드: DATA_SIZE 고정 길이로 송신 */
            sprintf(SendPkt, "%04d", DATA_SIZE);
            memcpy(&SendPkt[4], &R_Fmt[0].Data, DATA_SIZE);
#endif

            Add_Count(PS_R_2, 1);          /* 송신 건수 카운터 증가 */
        }
    }

    return;
}   /* End of Make_Send_Msg () */

/*************************************************************************
    End of Program (pb_7100_ts.c)
*************************************************************************/
