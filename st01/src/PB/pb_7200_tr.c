#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : KRX시세수신 TCP용 (KRX, IMECO), 시세FEP전용
#   File    : pb_7200_tr.c
#
#   설명  : 외부 시세 서버에 TCP로 접속하여 시세 데이터를 수신하고,
#             수신한 데이터를 FIFO에 기록하는 프로세스.
#             TCP 연결 관리, 데이터 수신, FIFO 기록을 담당한다.
#             pb_7100_dd와 쌍으로 동작 (수신 → FIFO → 배포).
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "cli_interface.h"

#define     MAX_CNT         1

#define     DATA_SIZE       400
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DEVICE_TIME     3 * 1000            /* 장치 대기 시간 (3초, 밀리초 단위) */
#define     DATA_TIME       (30+5) * 1000       /* 하트비트 대기 (30+5=35초) */
#define     FOREVER_TIME    60 * 1000           /* 최대 대기 시간 (60초) */

#define     SOCKET_EVENT    0                   /* Poll 이벤트: 소켓 데이터 도착 */

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int     Sockfd, Newfd;                          /* TCP 소켓 FD, Accept된 FD */
int     SendLen, MsgLen, ReTrCode;              /* 송신길이, 메시지길이, 재전송코드 */
int     PollCnt, FirstSeq, TimeOut;             /* Poll 카운트, 첫 시퀀스, 타임아웃 */
int     Pk, PortNo, SendFlag;                   /* 프로세스키, 포트번호, 송신플래그 */
char    ApType[10], IpAddr[20];                 /* 어플리케이션 타입, 서버 IP */
char    RecvPkt[CLI_BUFF_MAX_LEN];              /* TCP 수신 버퍼 */
char    SendPkt[CLI_BUFF_MAX_LEN];              /* TCP 송신 버퍼 */
char    LogOnFlag, OpenFlag;                    /* 로그온 상태, 연결 상태 */

FILE_BUFF_FORMAT    W_Fmt;                      /* FIFO 쓰기용 버퍼 */

struct pollfd       Poll[1];                    /* poll() 이벤트 감시 배열 */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PB_7200_TR(void);
void    Init_Parameters(void);
void    Fifo_Event_Rtn(void);
void    Socket_Event_Rtn(void);
void    Device_Write(void);
void    Device_Open(void);
void    Device_Close(void);
int     Device_Read(void);
void    Time_Out_Rtn(void);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 시세 수신 루프 실행.              */
/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);                     /* 프로세스 초기화 */
    PB_7200_TR();                              /* 메인 처리 루프 */
    Exit_Process();                            /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PB_7200_TR: 메인 처리 루프                                            */
/*    1) TCP 소켓 연결 (Device_Open)                                    */
/*    2) poll()로 소켓 이벤트 대기                                      */
/*    3) 이벤트 발생 시 Socket_Event_Rtn()으로 데이터 수신/FIFO 기록   */
/*    4) 장 종료 시 Device_Close()로 연결 해제                           */
/*----------------------------------------------------------------------*/
void    PB_7200_TR(void) {
    int     rt, i;

    Init_Parameters();                         /* 파라미터 초기화 (IP, 포트 등) */

#if defined HOLIDAY_APPLY
    /*
     * 주말 체크: 토/일요일이면 60초마다 대기
     * 평일이 되면 루프 탈출하여 시세 수신 시작
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

        if (tp->tm_wday == 0 || tp->tm_wday == 6) {
            Log(USR_OK, "it's weekend. sleeping...[%d]", tp->tm_wday);
            sleep(60);                         /* 주말 대기 */
        }
        else {
            Log(USR_OK, "it's not weekend. not sleeping...[%d]", tp->tm_wday);
            break;                              /* 평일 → 시세 수신 시작 */
        }
    }
#endif

    /* === 메인 루프: END 신호까지 반복 === */
    while (START_S != END) {
        Stat_Save();                           /* 프로세스 상태 저장 */

        /* 장 종료 또는 중지 상태 처리 */
        if (TCP2_NET_STA(0) == END || TCP2_NET_STA(0) == JOB_STOP) {
            if (LogOnFlag == ON && TCP2_NET_STA(0) == END) {
                Device_Close();                /* 로그온 상태에서 종료 → 연결 해제 */
            }
        }
        else {
            /* 정상 주문시간: TCP 연결이 없으면 연결 시도 */
            if (OpenFlag == OFF) {
                Device_Open();                 /* TCP 연결 시도 */
                TCP2_NET_STA(0) = ON;

                if (OpenFlag == OFF) {
                    sleep(5);                  /* 연결 실패 시 5초 대기 후 재시도 */
                    continue;
                }
            }

            /* poll() 설정: 소켓 1개, 무제한 대기(60초) */
            PollCnt = 1;
            TimeOut = FOREVER_TIME;

            rt = poll(Poll, PollCnt, TimeOut);
            if (rt < 0) {
                /* poll 오류 처리 */
                if (SYS_NO == EINTR)
                    Log(SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
                else
                    Log(SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);

                continue;
            }
            else if (rt == 0) {
                Time_Out_Rtn();                /* 타임아웃 처리 */
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
    }

    return;
}   /* End of PB_7200_TR () */

/*************************************************************************
    Function        : Init_Parameters
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 프로세스 파라미터 초기화.
                      설정에서 서버 IP/포트를 읽고, 상태 플래그 초기화.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Init_Parameters(void) {
    LogOnFlag = OFF;
    OpenFlag = OFF;

    L_K = 0;

    /* 설정에서 TCP 서버 IP 주소 조합 */
    sprintf(IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
            TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));

    PortNo = TCP2_PORT_NO;
    Log(TCP_OK, "Server Side port[%d]", PortNo);

    TCP2_PROC_ST = ON;                          /* 프로세스 상태: 기동 */
    TCP2_LINE_ST = OFF;                         /* 회선 상태: 미연결 */

    return;
}   /* End of Init_Parameters () */

/*************************************************************************
    Function        : Socket_Event_Rtn
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : TCP 소켓에서 시세 데이터를 수신하여
                      FIFO 버퍼(W_Fmt)에 헤더와 함께 기록한다.
                      수신 데이터의 앞 4바이트(길이)를 제외한 본문만 저장.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Socket_Event_Rtn(void) {
    int     rval, rt, w_gbn = 1;
    char    m_time[24], Chg_Data[KRX_DATA_BUFF_SIZE];

    rval = Device_Read();                      /* TCP에서 시세 데이터 수신 */

    INT_SEQ ++;                                 /* 내부 시퀀스 번호 증가 */

    if (rval < 0)
        return;                                 /* 수신 실패 시 리턴 */

    Get_MicroTime(m_time);

    /* FIFO 쓰기 버퍼 초기화 및 헤더 설정 */
    memset(&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));

    ItoAf(O_W_CNT1 + 1, W_Fmt.If_Seq, sizeof (W_Fmt.If_Seq));
    memcpy(W_Fmt.ApType, ApType, sizeof (W_Fmt.ApType));
    memcpy(W_Fmt.ResponseCode, RES_NORMAL, strlen(RES_NORMAL));
    memcpy(W_Fmt.RecvTime1, m_time, sizeof (W_Fmt.RecvTime1));
    memcpy(W_Fmt.RecvTime2, &m_time[sizeof(W_Fmt.RecvTime1)],
            sizeof (W_Fmt.RecvTime2));

    /* 수신 데이터를 FIFO 데이터 영역에 복사 (앞 4바이트 길이 제외) */
    memset(W_Fmt.DataHeader, 0x20, 20+DATA_SIZE);
    memcpy(W_Fmt.Data, RecvPkt+4, strlen(RecvPkt)-4 - 1);
    W_Fmt.Data[OFS(D_K,P_K,w_gbn-1)] = '\n';
    W_Fmt.LineFeed[0] = '\n';

    /* F5 order-pipeline-dshm: proc.ini 설정(ODN/OFN)으로 경로 자동 선택
       - ODN_1(out_d) 설정 시 DSHM 링 기록 (write-behind는 SyncManager 담당)
       - 기본(OFN_1)은 기존 파일 큐 그대로 */
    if (PROC(D_K,P_K).out_d[0] != 0) {
        rt = DSHM_W(TS_W1_1, (void *)&W_Fmt, 1);
        if (rt != 1) {
            Log(SAM_FATAL, "DSHM write fail[%s] rt[%d]", ODN(D_K,P_K,0), rt);
            close(Sockfd);
            return;
        }
    }
    else {
        /* FIFO에 1건 기록 */
        rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);
        if (rt != 1) {
            Log(SAM_FATAL, "file write fail[%s] rt[%d]", OFN(D_K,P_K,w_gbn-1), rt);
            close(Sockfd);
            return;
        }
    }

    return;
}   /* End of Socket_Event_Rtn () */

/*************************************************************************
    Function        : Device_Open
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : TCP 소켓 생성 후 시세 서버에 접속.
                      성공 시 OpenFlag/LogOnFlag=ON, poll FD 등록.
                      실패 시 3초 대기 후 리턴 (메인 루프에서 재시도).
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Open(void) {
    int     rt, rval;

    /* TCP 소켓 생성 */
    Sockfd = Socket();

    if (Sockfd < 0) {
        Log(TCP_ERROR, "socket fail:Sockfd[%d] {%d:%s}",
                Sockfd, SYS_NO, SYS_STR);
        return;
    }

    Log(USR_OK, "socket created:Sockfd[%d]", Sockfd);
    Log(USR_OK, "connecting to %s:%d", IpAddr, TCP2_PORT_NO);

    /* 시세 서버에 TCP 접속 */
    rt = Connect(Sockfd, IpAddr, TCP2_PORT_NO);

    if (rt < 0) {
        Log(TCP_ERROR, "connect fail {%d:%s}", SYS_NO, SYS_STR);
        close(Sockfd);
        sleep(3);                              /* 접속 실패 → 3초 대기 */
        return;
    }

    /* 접속 성공: 상태 플래그 설정 및 poll FD 등록 */
    OpenFlag = LogOnFlag = ON;

    Poll[0].fd = Sockfd;
    Poll[0].events = POLLIN;                    /* 데이터 수신 이벤트 감시 */
    Log(TCP_OK, "TCP Connect & LOGON OK");

    return;
}   /* End of Device_Open () */

/*************************************************************************
    Function        : Device_Close
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : TCP 연결 해제. 소켓 닫고 상태 플래그 초기화.
                      3초 대기 후 리턴.
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
    Function        : Device_Read
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : OK(0)=성공, NOTOK(-1)=실패
    Comment         : TCP 소켓에서 시세 패킷 수신.
                      Select_Receive_Cli()로 길이 헤더(4바이트) + 본문 수신.
                      불완전 패킷이면 한번 더 시도.
                      실패 시 Device_Close()로 연결 해제.
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Device_Read(void) {
    int     rt;
    char    m_time[24];

    memset(RecvPkt, 0, sizeof (RecvPkt));

    /* 시세 패킷 수신 (길이헤더 4바이트 + 본문) */
    rt = Select_Receive_Cli(Sockfd, RecvPkt);

    if (rt == 0) {
        /* 불완전 패킷: 한번 더 수신 시도 */
        rt = Select_Receive_Cli(Sockfd, RecvPkt);
    }

    if (rt <= 0 || rt > KRX_DATA_BUFF_SIZE) {
        Log(TCP_ERROR, "TCP RD Worng Length[%s] (%d/%d)<%d>", RecvPkt, strlen(RecvPkt), rt, INT_SEQ);
        Device_Close();
        return (NOTOK);
    }

    Log(TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen(RecvPkt), INT_SEQ);

    return (OK);
}   /* End of Device_Read () */

/*************************************************************************
    Function        : Device_Write
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : TCP 소켓으로 데이터 송신.
                      Select_Send()로 SendPkt 내용을 전송.
                      실패 시 Device_Close()로 연결 해제.
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
    Comment         : poll() 타임아웃 발생 시 호출. 로그 출력만 수행.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Time_Out_Rtn(void) {
    Log(USR_OK, "TimeOut");

    return;
}   /* End of Time_Out_Rtn () */

/*************************************************************************
    End of Program (pb_7200_tr.c)
*************************************************************************/
