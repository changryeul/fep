#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : 시세 TCP 수신 (서버 모드)
#   File    : pa_7000_tr.c
#
#   설명  : 시세 데이터를 TCP 서버 모드로 수신하는 프로세스.
#             Listen/Accept로 Client(시세 송신 측) 접속을 기다린 후
#             시세 데이터를 수신하여 TR코드별로 분류,
#             FIFO에 기록하여 DD 프로세스에 전달한다.
#
#   빌드 옵션별 처리:
#             A7701/A7702: 금융파생 시세통수신 TCP
#               A006F(종목정보) → w_gbn=1
#               A306F/G706F/B606F/A706F(체결/시세) → w_gbn=2
#               M406F(장운영스케줄) → w_gbn=3
#             A7622: 채권 KTS 시세 수신 (체결/호가)
#               A301K/G701K/B601K/A601K → w_gbn=1
#             A7623: 채권 KTS 시세 수신 (장운영)
#               A701K/M401K → w_gbn=1
#
#   동작 흐름:
#             1) Socket → Bind → Listen (서버 준비)
#             2) Accept (Client 접속 대기)
#             3) poll() 대기 → 데이터 수신
#             4) TR코드별 분류 → FIFO 기록
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "cli_interface.h"
#include    "fep_common.h"

#define     MAX_CNT         1

#define     DATA_SIZE       2048
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     FOREVER_TIME    60 * 1000       /* Poll 대기 시간 (60초, 밀리초 단위) */

#define     SOCKET_EVENT    0               /* Poll 이벤트: TCP 소켓 */

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int         Sockfd, Newfd;                  /* 리스닝 소켓, 접속 소켓 */
int         PollCnt, FirstSeq, TimeOut, PortNo;
char        ApType[10];                     /* 어플리케이션 타입 */
char        RecvPkt[TCP_BUFF_MAX_LEN], SendPkt[TCP_BUFF_MAX_LEN];   /* 수신/송신 패킷 버퍼 */
char        OpenFlag;                       /* TCP 접속 완료 플래그 */

FILE_BUFF_FORMAT    W_Fmt;                  /* FIFO 쓰기용 버퍼 */

struct pollfd       Poll[1];                /* poll 이벤트 배열: [0]=접속 소켓 */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_7000_TR(void);
void    Init_Parameters(void);
void    Fifo_Event_Rtn(void);
void    Socket_Event_Rtn(void);
void    Device_Write(void);
void    Device_Open(void);
void    Device_Close(void);
int     Device_Read(void);
void    Time_Out_Rtn(void);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 시세 TCP 수신 루프 실행.          */
/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);                 /* 프로세스 초기화 */
    PA_7000_TR();                          /* 메인 처리 루프 */
    Exit_Process();                        /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PA_7000_TR: 메인 처리 루프                                            */
/*    1) 주말이면 대기 (HOLIDAY_APPLY 옵션)                             */
/*    2) TCP 서버 소켓 준비 (Socket/Bind/Listen)                      */
/*    3) Client Accept 대기 → 접속 완료                               */
/*    4) poll() → 데이터 수신 → TR코드별 FIFO 기록                        */
/*    5) POLLHUP → 연결 끊김 감지                                     */
/*----------------------------------------------------------------------*/
void    PA_7000_TR(void) {
    int     rt, i;
    int     size = 1000000;

    Init_Parameters();

#if defined HOLIDAY_APPLY
    /*
     * 주말 체크: 토/일요일이면 60초 간격으로 대기
     * 주말에는 프로세스 상태를 JOB_END/9로 설정하여 종료 상태 표시
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
            PROC(D_K,P_K).start_status = JOB_END;
            PROC(D_K,P_K).process_status = 9;
            sleep(60);
        }
        else {
            break;
        }
    }
#endif

    /* === 메인 루프: 프로세스 종료 신호(END)까지 반복 === */
    while (START_S != END) {
        Stat_Save();

        /*
         * TCP 연결 상태 관리:
         *   NET_STA == END/JOB_STOP → 종료/중지 → 연결 해제
         *   그 외 → 정상 시간 → Accept 대기 또는 데이터 수신
         */
        if (TCP2_NET_STA(0) == END || TCP2_NET_STA(0) == JOB_STOP) {
            Device_Close();
        }
        else {
            /*
             * 미접속 상태: 서버 소켓 준비 후 Accept 대기
             * Device_Open()으로 Socket/Bind/Listen 수행
             * accept()으로 Client 접속 대기 (블로킹)
             */
            if (OpenFlag == OFF) {
                Device_Open();

                TCP2_NET_STA(0) = ON;

                Log(USR_OK, "accepting ...");
                Newfd = accept(Sockfd, (struct sockaddr*)NULL, NULL);
                if (Newfd < 0) {
                    if (SYS_NO == ECONNABORTED)     /* 접속 중단 → 재시도 */ {
                        usleep(100000);
                        continue;
                    }

                    Log(TCP_ERROR, "accept[%d] {%d:%s}", Newfd, SYS_NO, SYS_STR);
                    return;
                }
                Log(USR_OK, "accepted");

                Set_Socket_Linger(Sockfd);     /* SO_LINGER 설정 */

                TCP2_LINE_ST = OpenFlag = ON;   /* 접속 완료 */

                /* 접속된 소켓을 poll에 등록 */
                Poll[0].fd = Newfd;
                Poll[0].events = POLLIN;

                PollCnt = 1;
                TimeOut = FOREVER_TIME;         /* 60초 타임아웃 */
            }
            else {
                /* 이미 접속중: poll 파라미터만 설정 */
                PollCnt = 1;
                TimeOut = FOREVER_TIME;
            }
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
            Time_Out_Rtn();                /* 타임아웃 → 로그 출력 */
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
                    default:
                        Log(USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                        Exit_Process();
                        break;
                }
            }
        }
    }

    return;
}   /* End of PA_7000_TR () */

/*************************************************************************
    Function        : Init_Parameters
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 전역 변수 초기화.
                      TCP 리스닝 포트 설정, ApType 생성.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Init_Parameters(void) {
    OpenFlag = OFF;                         /* TCP 접속 상태 OFF */

    L_K = 0;                                /* 라인 키 초기화 */

    PortNo = TCP2_PORT_NO;
    Log(TCP_OK, "Server Side port[%d]", PortNo);

    TCP2_PROC_ST = ON;                      /* 프로세스 상태: 가동중 */
    TCP2_LINE_ST = OFF;                     /* 라인 상태: 미접속 */

    /* ApType 생성: 실행파일명에서 모듈+기능번호+타입 추출 */
    sprintf(ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));

    return;
}   /* End of Init_Parameters () */

/*************************************************************************
    Function        : Socket_Event_Rtn
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : TCP 데이터 수신 이벤트 처리.
                      1) Device_Read()로 시세 데이터 수신
                      2) TR코드(앞 5바이트)로 시세 종류 분류
                      3) w_gbn에 따라 FIFO(TS_W1_1)에 기록

    w_gbn 분류:
      A7701/A7702 금융파생:
        1=A006F(종목정보), 2=A306F/G706F/B606F/A706F(시세),
        3=M406F(장운영), 0=기타(LK 등 무시)
      A7622 채권:
        1=A301K/G701K/B601K/A601K(시세), 0=기타
      A7623 채권:
        1=A701K/M401K(장운영), 0=기타
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Socket_Event_Rtn(void) {
    int     rval, rt, w_gbn = 0;
    char    m_time[24], Chg_Data[KRX_DATA_BUFF_SIZE];

    rval = Device_Read();

    if (rval < 0)
        return;

    /* === TR코드별 w_gbn 분류 === */
#if defined(A7701) || defined(A7702)
    /*
     * 금융파생 시세통수신 TCP:
     *   A001F(10301/11301): 1318바이트, 종목정보
     *   A301F: 173바이트, 파생체결
     *   G701F: 431바이트, 파생체결G7 (우선호가 5단계)
     *   B601F: 324바이트, 파생우선호가 (5단계)
     *   A701A: 68바이트, 장운영정보
     *   M401A: 83바이트, 장운영스케줄
     *   R101F: 359바이트, 파생 장운영TS + 우선호가
     */
    if (memcmp(RecvPkt, "A006F", 5) == 0)
        w_gbn = 1;                              /* 종목정보 → FIFO 1 */
    else if ((memcmp(RecvPkt, "A306F", 5) == 0) ||
            (memcmp(RecvPkt, "G706F", 5) == 0) ||
            (memcmp(RecvPkt, "B606F", 5) == 0) ||
            (memcmp(RecvPkt, "A706F", 5) == 0))
    w_gbn = 2;                              /* 시세/체결 → FIFO 2 */
    else if (memcmp(RecvPkt, "M406F", 5) == 0)
        w_gbn = 3;                              /* 장운영스케줄 → FIFO 3 */
    else {
        w_gbn = 0;                              /* 기타 (LK 등) → 무시 */
        Log(USR_OK, "Else(LK) Recv [%s][%d]", RecvPkt, strlen(RecvPkt));
    }
#elif defined A7622
    /*
     * 채권 KTS 시세 수신 (체결/호가):
     *   A301K: 223바이트, 채권 체결
     *   G701K: 643바이트, 채권 체결 + 우선호가
     *   B601K: 462바이트, 채권 우선호가
     *   A601K: 57바이트, 채권 종목마감
     */
    if ((memcmp(RecvPkt, "A301K", 5) == 0) ||
            (memcmp(RecvPkt, "G701K", 5) == 0) ||
            (memcmp(RecvPkt, "B601K", 5) == 0) ||
            (memcmp(RecvPkt, "A601K", 5) == 0))
    w_gbn = 1;                              /* 시세 → FIFO 1 */
    else {
        w_gbn = 0;
        Log(USR_OK, "Else(LK) Recv [%s][%d]", RecvPkt, strlen(RecvPkt));
    }
#elif defined A7623
    /*
     * 채권 KTS 시세 수신 (장운영):
     *   A701K: 68바이트, 장운영TS
     *   M401K: 83바이트, 장운영스케줄
     */
    if ((memcmp(RecvPkt, "A701K", 5) == 0) ||
            (memcmp(RecvPkt, "M401K", 5) == 0))
    w_gbn = 1;                              /* 장운영 → FIFO 1 */
    else {
        w_gbn = 0;
        Log(USR_OK, "Else(LK) Recv [%s][%d]", RecvPkt, strlen(RecvPkt));
    }
#endif

    /* === w_gbn > 0이면 FIFO에 시세 데이터 기록 === */
    if (w_gbn) {
        Get_MicroTime(m_time);

        /* FIFO 쓰기 버퍼(W_Fmt) 조립: 헤더 + 데이터 */
        memset(&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));

        /* 시퀀스 번호: w_gbn별 카운터 사용 */
        if (w_gbn == 1)
            ItoAf(O_W_CNT1 + 1, W_Fmt.If_Seq, sizeof (W_Fmt.If_Seq));
        else if (w_gbn == 2)
            ItoAf(O_W_CNT2 + 1, W_Fmt.If_Seq, sizeof (W_Fmt.If_Seq));
        else if (w_gbn == 3)
            ItoAf(O_W_CNT3 + 1, W_Fmt.If_Seq, sizeof (W_Fmt.If_Seq));
        memcpy(W_Fmt.ApType, ApType, sizeof (W_Fmt.ApType));
        memcpy(W_Fmt.ResponseCode, RES_NORMAL, strlen(RES_NORMAL));
        memcpy(W_Fmt.RecvTime1, m_time, sizeof (W_Fmt.RecvTime1));
        memcpy(W_Fmt.RecvTime2, &m_time[sizeof(W_Fmt.RecvTime1)],
                sizeof (W_Fmt.RecvTime2));

        /* 데이터 영역에 수신 패킷 복사 */
        memset(W_Fmt.DataHeader, 0x20, 20 + DATA_SIZE);
        memcpy(W_Fmt.Data, RecvPkt, strlen(RecvPkt));
        W_Fmt.Data[OFS(D_K,P_K,w_gbn-1)] = '\n';
        W_Fmt.LineFeed[0] = '\n';

        rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);
        if (rt != 1) {
            SLog(SAM_FATAL, "file write fail[%s] rt[%d]", OFN(D_K,P_K,w_gbn-1), rt);
            close(Sockfd);
            return;
        }
    }

    INT_SEQ ++;

    return;
}   /* End of Socket_Event_Rtn () */

/*************************************************************************
    Function        : Device_Open
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : TCP 서버 소켓 준비.
                      Socket 생성 → SO_REUSEADDR 설정 →
                      Bind(포트) → Listen → poll FD 등록.
                      5초 대기 후 시작 (이전 소켓 TIME_WAIT 해소 대기).
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Open(void) {
    int     rt;
    const   int on = 1;

    sleep(5);                              /* TIME_WAIT 해소 대기 */

    /* 소켓 생성 */
    Sockfd = Socket();
    if (Sockfd < 0) {
        Log(TCP_ERROR, "socket[%d] {%d:%s}", Sockfd, SYS_NO, SYS_STR);
        TCP2_NET_STA(0) = OFF;
        Exit_Process();
    }
    Log(USR_OK, "socket created:Sockfd[%d]", Sockfd);

    /* SO_REUSEADDR 설정 (포트 재사용 허용) */
    rt = setsockopt(Sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof (on));
    if (rt < 0) {
        Log(TCP_ERROR, "setsockopt[%d] {%d:%s}", rt, SYS_NO, SYS_STR);
        close(Sockfd);
        TCP2_NET_STA(0) = OFF;
        Exit_Process();
    }
    Log(USR_OK, "setsockopt[%d]", Sockfd);

    /* Bind: 포트에 바인딩 */
    rt = Bind(Sockfd, PortNo);
    if (rt < 0) {
        Log(TCP_ERROR, "bind[%d][%d] {%d:%s}", rt, PortNo, SYS_NO, SYS_STR);
        close(Sockfd);
        TCP2_NET_STA(0) = OFF;
        Exit_Process();
    }
    Log(USR_OK, "bind[%d]", Sockfd);

    /* Listen: 접속 대기 시작 */
    Listen(Sockfd);
    Log(USR_OK, "listen[%d]", Sockfd);

    /* poll에 리스닝 소켓 등록 */
    Poll[0].fd = Sockfd;
    Poll[0].events = POLLIN;

    return;
}   /* End of Device_Open () */

/*************************************************************************
    Function        : Device_Close
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : TCP 소켓(리스닝+접속) 해제 및 상태 플래그 OFF.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Close(void) {
    close(Sockfd);
    close(Newfd);
    Log(TCP_OK, "TCP device close");

    OpenFlag = OFF;

    return;
}   /* End of Device_Close () */

/*************************************************************************
    Function        : Device_Read
    Parameters IN   : (없음)
    Parameters OUT  : (없음, RecvPkt에 결과 저장)
    Return Code     : OK(0)=정상, NOTOK=오류
    Comment         : TCP 소켓에서 시세 데이터 수신.
                      Sise_Select_Receive()로 시세 전문 크기만큼 수신.
                      첫 수신에서 0(불완전)이면 한 번 더 시도.
                      수신 크기가 0 이하이거나 버퍼 초과이면 오류.
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Device_Read(void) {
    int     rt;
    char    m_time[24];

    memset(RecvPkt, 0, sizeof (RecvPkt));

    rt = Sise_Select_Receive(Newfd, RecvPkt);

    if (rt == 0) {
        /* 불완전 수신: 한 번 더 읽기 시도 */
        rt = Sise_Select_Receive(Newfd, RecvPkt);
    }

    if (rt <= 0 || rt > KRX_DATA_BUFF_SIZE) {
        Log(TCP_ERROR, "TCP RD Worng Length[%s] (%d)<%d>", RecvPkt, rt, INT_SEQ);
        Device_Close();
        return (NOTOK);
    }

    Log(TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen(RecvPkt), INT_SEQ);

    return (OK);
}   /* End of Device_Read () */

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
    Comment         : 타임아웃(60초) 발생 시 로그 출력.
                      이 프로세스는 서버 모드이므로 능동적
                      하트비트 전송은 하지 않는다.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Time_Out_Rtn(void) {
    Log(USR_OK, "TimeOut");

    return;
}   /* End of Time_Out_Rtn () */

/*************************************************************************
    End of Program (pa_7000_tr.c)
*************************************************************************/
