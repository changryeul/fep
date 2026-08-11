#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : IMECO 시세수신 TCP
#   File    : pa_2700_tr.c
#
#   설명  : IMECO 서버에 TCP로 접속하여 금융파생 시세 데이터를
#             수신하는 프로세스. 로그인 → 데이터 요청 → 시세 수신
#             순서로 동작하며, 수신된 시세를 TR코드별로 분류하여
#             서로 다른 FIFO에 기록한다.
#
#   프로토콜 흐름:
#             1) TCP 접속 → LOGIN 전송
#             2) LOGOK 수신 → REQRL(데이터 요청) 전송
#             3) 시세 데이터 수신 → FIFO 기록
#             4) RESHB(하트비트) 수신 → REQHB 응답 전송
#
#   수신 TR코드 분류:
#             A0       → W_Flag=1 → TS_W1_1 (호가)
#             A3/G7/B6 → W_Flag=2 → TS_W2_1 (체결/시세)
#             H3/A6/A7/R1 → 미사용 (무시)
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "fep_common.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     TCP_TIME_OUT    35              /* TCP 수신 타임아웃 (35초) */

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int     Sockfd, PortNo, DataCnt, PktType, DataSize, MaxCnt, RetryCnt, W_Flag;
/* Sockfd=소켓FD, PortNo=접속포트, PktType=현재 패킷유형,
 * DataSize=데이터크기, W_Flag=FIFO 쓰기 분기값 */
char    ApType[10], IpAddr[20], ErrCd[8];   /* 어플리케이션 타입, 접속 IP, 오류코드 */
char    RecvPkt[TCP_BUFF_MAX_LEN], SendPkt[TCP_BUFF_MAX_LEN];   /* 수신/송신 패킷 버퍼 */

FILE_DATA_HEAD      File_Data_Head;         /* FIFO 기록용 데이터 헤더 (20바이트) */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_2200_TR(void);
void    Init_Parameters(void);
void    Connection(void);
int     Receive_Packet(void);
void    Send_Packet(void);
int     Write_Data(void);
void    Register_Signal(void);
void    Catch_Signal(int);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 IMECO 시세 수신 루프 실행.            */
/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);                 /* 프로세스 초기화 */

    PA_2200_TR();                          /* 메인 처리 루프 */

    TCP2_CON_STA = OFF;                     /* 접속 상태 OFF */
    close(Sockfd);
    Exit_Process();                        /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PA_2200_TR: 메인 처리 루프                                            */
/*    1) 시그널 핸들러 등록 (SIGPIPE, SIGTERM)                          */
/*    2) 주말 체크 (HOLIDAY_CHECK 옵션)                                   */
/*    3) IMECO 서버 접속 → LOGIN 전송                                 */
/*    4) LOGOK 수신 대기 → 성공 시 데이터 수신 루프 진입                */
/*    5) 수신 루프: 시세/하트비트 계속 수신                               */
/*----------------------------------------------------------------------*/
void    PA_2200_TR(void) {
    int     rt;

    Register_Signal();                     /* SIGPIPE, SIGTERM 핸들러 등록 */
    Init_Parameters();                     /* 전역 변수 초기화 */

#ifdef  HOLIDAY_CHECK
    /*
     * 주말 체크: 토/일요일이면 60초 간격으로 대기
     * 평일이 되면 루프를 빠져나와 정상 처리 시작
     */
    while (1) {
        char    t_time[12], dt[20];
        time_t  t = time(NULL);
        struct  tm tm, *tp;

        Get_Time(t_time);

        memset(dt, 0, sizeof (dt));
        sprintf(dt, "%.4s-%.2s-%.2s %.2s:%.2s:%.2s", DAEMON(D_K).date,
                DAEMON(D_K).date+4, DAEMON(D_K).date+6, t_time, t_time+2, t_time+4);
        strptime(dt, "%Y-%m-%d %H:%M:%S", &tm);
        tp = localtime(&t);

        if (tp->tm_wday == 0 || tp->tm_wday == 6)   /* 일요일(0) 또는 토요일(6) */ {
            Log(USR_OK, "it's weekend. sleeping...[%d]", tp->tm_wday);
            sleep(60);
            continue;
        }
        else
            break;
    }
#endif

    /* [Step 1] IMECO 서버 접속 및 LOGIN */
    PktType = T_LINK;                       /* LOGIN 패킷 */
    Connection();                          /* TCP 접속 */
    Send_Packet();                         /* LOGIN 전송 */

    /* [Step 2] LOGOK 응답 대기 (최초 1회) */
    while (1) {
        rt = Receive_Packet();

        if (rt == FAIL)
            continue;                       /* 시그널 인터럽트 → 재시도 */
        else if (rt == NOTOK)
            return;                         /* 오류 → 종료 */
        else
            break;                          /* 정상 수신 → 데이터 루프 진입 */
    }

    /* [Step 3] 시세 데이터 수신 루프 */
    while (START_S < JOB_END) {
        Stat_Save();

        rt = Receive_Packet();             /* 시세/하트비트 수신 */
    }

    return;
}   /* End of PA_2200_TR () */

/*************************************************************************
    Function        : Init_Parameters
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 전역 변수 초기화.
                      TCP 접속 IP/Port 설정, 데이터 크기 결정.
                      ApType 생성 (실행파일명에서 추출).
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Init_Parameters(void) {
    SYS_NO = 0;
    S_K = 0;
    RetryCnt = 0;
    DataCnt = 0;

    /* TCP 타임아웃이 미설정이면 기본값(35초) 사용 */
    if (TIME_OUT == 0)
        TIME_OUT = TCP_TIME_OUT;

    Log(USR_OK, "TIME_OUT[%d]", TIME_OUT);

    /* ApType 생성: 실행파일명에서 모듈+기능번호+타입 추출 */
    sprintf(ApType, "%-2.2s%-4.4s%c%c",
            _Exe_Name, _Exe_Name+3, _Exe_Name[8], _Exe_Name[9] == 's' ? 'R' : 'S');
    LtoU(ApType, strlen(ApType));

    /* 데이터 크기: SAM_USE 여부에 따라 OFS 또는 ODS 사용 */
#ifdef  SAM_USE
    DataSize = OFS(D_K,P_K,0);
#else
    DataSize = ODS(D_K,P_K,0);
#endif

    MaxCnt = 1;

    /* TCP 접속 대상 IP/Port 설정 */
    sprintf(IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
            TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    PortNo = TCP2_PORT_NO;

    return;
}   /* End of Init_Parameters () */

/*************************************************************************
    Function        : Connection
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : IMECO 서버에 TCP 접속.
                      소켓 생성 → Connect → SO_LINGER 설정.
                      접속 실패 시 5초 대기 후 프로세스 종료.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Connection(void) {
    int     rt;

    while (START_S != END) {
        Sockfd = Socket();

        if (Sockfd < 0) {
            TCP2_LINE_ST = OFF;
            Log(TCP_ERROR, "socket fail:Sockfd[%d] {%d:%s}",
                    Sockfd, SYS_NO, SYS_STR);
            sleep(5);
            continue;
        }

        TCP2_LINE_ST = ON;
        Log(USR_OK, "socket created:Sockfd[%d]", Sockfd);
        Log(USR_OK, "connecting to %s:%d", IpAddr, PortNo);

        rt = Connect(Sockfd, IpAddr, PortNo);

        if (rt < 0) {
            TCP2_CON_STA = OFF;
            TCP2_LINE_ST = OFF;
            TCP2_NET_STA(S_K) = OFF;
            Log(TCP_ERROR, "connect fail {%d:%s}", SYS_NO, SYS_STR);
            close(Sockfd);
            sleep(5);

            Exit_Process();                /* 접속 실패 → 프로세스 종료 */
        }

        TCP2_CON_STA = ON;
        TCP2_NET_STA(S_K) = ON;
        Log(USR_OK, "connected to %s:%d", IpAddr, PortNo);
        break;
    }

    Set_Socket_Linger(Sockfd);             /* SO_LINGER 설정 (정상 종료 보장) */

    return;
}   /* End of Connection () */

/*************************************************************************
    Function        : Receive_Packet
    Parameters IN   : (없음)
    Parameters OUT  : (없음, RecvPkt에 결과 저장)
    Return Code     : OK(0)=정상, FAIL(-1)=재시도, NOTOK(1)=종료
    Comment         : IMECO 서버로부터 패킷 수신 및 처리.
                      수신 TR코드별 처리:
                        LOGOK → REQRL(데이터 요청) 전송
                        LOGER → 로그인 오류 → 종료
                        RESHB → REQHB(하트비트 응답) 전송
                        A0    → 호가 시세 → W_Flag=1 → FIFO 기록
                        A3/G7/B6 → 체결/시세 → W_Flag=2 → FIFO 기록
                        H3/A6/A7/R1 → 미사용 → 무시
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Receive_Packet(void) {
    int     rt;
    char    error_flag;

    error_flag = OFF;
    memset(RecvPkt, 0, sizeof (RecvPkt));

    rt = Select_Receive_Imeco_Sise(Sockfd, RecvPkt);

    if (rt == OK)
        return (NOTOK);                     /* 연결 종료 */
    else if (rt == NOTOK) {
        if (SYS_NO == EINTR)
            return (FAIL);                  /* 시그널 인터럽트 → 재시도 */

        return (NOTOK);                     /* 오류 → 종료 */
    }

    Log(TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen(RecvPkt), INT_SEQ);

    /*
     * 수신 패킷 분류 (RecvPkt[10]부터 TR코드 시작):
     *   수신 가능: LOGOK, LOGER, RESHB, 시세TR(A0/A3/G7/B6...)
     *   송신 가능: LOGIN, REQHB, REQRL
     */
    W_Flag = 0;
    if (memcmp(&RecvPkt[10], "LOGOK", 5) == 0)         /* 로그인 성공 */ {
        PktType = T_DATA;                   /* REQRL(데이터 요청) 전송 */
        Send_Packet();
    }
    else if (memcmp(&RecvPkt[10], "LOGER", 5) == 0)        /* 로그인 오류 */ {
        Log(USR_ERROR, "Check Plz!!  Recv LOGIN ERROR RecvPkt[%s][%d]",
                RecvPkt, strlen(RecvPkt));
        return (NOTOK);
    }
    else if (memcmp(&RecvPkt[10], "RESHB", 5) == 0)        /* 하트비트 */ {
        PktType = T_POOK;                   /* REQHB(하트비트 응답) 전송 */
        Send_Packet();
    }
    else if ((memcmp(&RecvPkt[10], "A0", 2) == 0))     /* 호가 시세 */ {
        W_Flag = 1;                         /* → TS_W1_1 FIFO에 기록 */
    }
    else if ((memcmp(&RecvPkt[10], "A3", 2) == 0) ||       /* 체결 */
            (memcmp(&RecvPkt[10], "G7", 2) == 0) ||       /* 시세 */
            (memcmp(&RecvPkt[10], "B6", 2) == 0))     /* 거래량 */ {
        W_Flag = 2;                         /* → TS_W2_1 FIFO에 기록 */
    }
    else if ((memcmp(&RecvPkt[10], "H3", 2) == 0) ||       /* 정산가 (미사용) */
            (memcmp(&RecvPkt[10], "A6", 2) == 0) ||       /* 종목마감 (미사용) */
            (memcmp(&RecvPkt[10], "A7", 2) == 0) ||       /* 장운영 (미사용) */
            (memcmp(&RecvPkt[10], "R1", 2) == 0))     /* 장운영+호가 (미사용) */ {
        W_Flag = 0;                         /* 무시 */
    }
    else {
        Log(USR_ERROR, "Else Case Receive RecvPkt[%5.5s]", &RecvPkt[10]);
        return (NOTOK);
    }

    /* W_Flag > 0이면 FIFO에 시세 데이터 기록 */
    if (W_Flag > 0) {
        rt = Write_Data();
        if (rt != OK) {
            Log(USR_ERROR, "Write_Data Error");
            sleep(3);
            return (NOTOK);
        }
    }

    return (OK);
}   /* Receive_Packet () */

/*************************************************************************
    Function        : Send_Packet
    Parameters IN   : (없음, 전역변수 PktType 사용)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : IMECO 서버로 패킷 전송.
                      PktType별 패킷 포맷:

                      IMECO 패킷 구조 (50바이트 헤더):
                        [0-9]   BodyLength (10자리, 제로패딩)
                        [10-14] TR Code (LOGIN/REQHB/REQRL)
                        [15-24] SeqNo (항상 0)
                        [25-28] ErrorCode (항상 0)
                        [29-48] SendTime (yyyymmddhhmmssmmm + 공백3)

                      Data부 (LOGIN만 해당):
                        [50-59] ID: "KBFG2     "
                        [60-69] PW: "KBFG2     "
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Send_Packet(void) {
    int     rt;
    char    d_time[18];

    memset(SendPkt, 0, sizeof (SendPkt));

    memset(d_time, 0, sizeof (d_time));
    Get_DateMilliTime(d_time);             /* yyyymmddhhmmssmmm */

    memset(SendPkt, 0, sizeof(SendPkt));

    /*
     * 패킷 헤더 조립:
     * 1. BodyLength: 헤더를 제외한 Body 크기
     * 2. TR Code: 패킷 유형별 코드
     */
    if (PktType == T_LINK)                              /* LOGIN (로그인) */ {
        memcpy(&SendPkt[0],  "0000000080", 10);        /* Body 80바이트 */
        memcpy(&SendPkt[10], "LOGIN",       5);
    }
    else if (PktType == T_POOK)                         /* REQHB (하트비트 응답) */ {
        memcpy(&SendPkt[0],  "0000000040", 10);        /* Body 없음 (헤더만) */
        memcpy(&SendPkt[10], "REQHB",       5);
    }
    else if (PktType == T_DATA)                         /* REQRL (데이터 요청) */ {
        memcpy(&SendPkt[0],  "0000000040", 10);        /* Body 없음 (헤더만) */
        memcpy(&SendPkt[10], "REQRL",       5);
    }

    /* 3. SeqNo: 항상 0 */
    memcpy(&SendPkt[15], "0000000000", 10);
    /* 4. ErrorCode: 항상 0 */
    memcpy(&SendPkt[25], "0000",        4);
    /* 5. 송신 시간 (밀리초 포함 17자리 + 공백 3자리) */
    memcpy(&SendPkt[29], d_time,       17);
    memcpy(&SendPkt[29+17], "   ",      3);

    /* Data부 (LOGIN만 해당): ID/PW */
    if (PktType == T_LINK) {
        memcpy(&SendPkt[50], "KBFG2     ", 10);        /* ID */
        memcpy(&SendPkt[60], "KBFG2     ", 10);        /* PW */
    }

    /* TCP 송신 */
    rt = Select_Send(Sockfd, SendPkt, strlen(SendPkt));
    if (rt != OK) {
        TCP2_CON_STA = OFF;
        close(Sockfd);
        Log(TCP_ERROR, "TCP SD ERROR[%s](%d)<%d> {%d:%s}",
                SendPkt, strlen(SendPkt), INT_SEQ, SYS_NO, SYS_STR);
        sleep(5);
        Exit_Process();
    }

    Log(TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen(SendPkt), INT_SEQ);

    return;
}   /* Send_Packet () */

/*************************************************************************
    Function        : Write_Data
    Parameters IN   : (없음, 전역변수 RecvPkt/W_Flag 사용)
    Parameters OUT  : (없음)
    Return Code     : OK(0)=성공, NOTOK=실패
    Comment         : 수신된 시세 데이터를 FIFO에 기록한다.
                      데이터 포맷: BUFF_RW_HEAD(50) + FILE_DATA_HEAD(20) + 시세데이터

                      W_Flag에 따라 기록 대상 FIFO가 다름:
                        1 → TS_W1_1 (호가 데이터)
                        2 → TS_W2_1 (체결/시세 데이터)

                      데이터 영역은 RecvPkt[50]부터 시작하며
                      길이는 RecvPkt[5]에서 5자리로 읽어온다.
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Write_Data(void) {
    int         i, next, seq, rt;
    char        m_time[24];
    char        media_gbn[2];
    char        w_data[2048];               /* FIFO 쓰기용 버퍼 */
    BUFF_RW_HEAD f_head;

    memset(m_time, 0, sizeof (m_time));
    Get_MicroTime(m_time);
    memset(w_data, 0x20, sizeof (w_data));
    memset(&File_Data_Head, ' ', HEAD_SIZE);

    /* BUFF_RW_HEAD (50바이트) 조립 */
    memset(&f_head, 0x20, sizeof(f_head));
    ItoAf(INT_SEQ + 1, f_head.Seq,           sizeof(f_head.Seq));
    ItoAf(INT_SEQ + 1, f_head.If_Seq,        sizeof(f_head.If_Seq));
    memcpy(f_head.ApType,       ApType,       sizeof(f_head.ApType));
    memcpy(f_head.ResponseCode, RES_NORMAL,   strlen(RES_NORMAL));
    memcpy(f_head.RecvTime1,    m_time,       sizeof(f_head.RecvTime1));
    memcpy(f_head.RecvTime2,    &m_time[10],  sizeof(f_head.RecvTime2));

    /* FILE_DATA_HEAD (20바이트) 조립 */
    ItoAf(OFS(D_K,P_K,W_Flag-1), File_Data_Head.Length,       sizeof (File_Data_Head.Length));
    ItoAf(INT_SEQ + 1,           File_Data_Head.DataSeq,      sizeof (File_Data_Head.DataSeq));
    memcpy(File_Data_Head.ResponseCode, RES_NORMAL, strlen(RES_NORMAL));
    memcpy(File_Data_Head.LineFlag,     _Exe_Name+4, 3);

    /* 헤더 + 데이터 조립 */
    memcpy(f_head.DataHeader, &File_Data_Head, sizeof (FILE_DATA_HEAD));
    memcpy(w_data,            &f_head,         sizeof(BUFF_RW_HEAD));

    /* 시세 데이터: RecvPkt[50]부터 길이(RecvPkt[5], 5자리)만큼 복사 */
    memcpy(&w_data[sizeof(BUFF_RW_HEAD)], &RecvPkt[50], AtoIf(&RecvPkt[5], 5));
    w_data[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,W_Flag-1)] = '\n';

    SYS_NO = 0;

    /* W_Flag에 따라 다른 FIFO에 기록 */
    if (W_Flag == 1)
        rt = F_W(TS_W1_1, (void *)w_data, 1);      /* 호가 → FIFO 1 */
    else if (W_Flag == 2)
        rt = F_W(TS_W2_1, (void *)w_data, 1);      /* 체결/시세 → FIFO 2 */

    if (rt != 1) {
        Log(SAM_FATAL, "file write[%s]", OFN(D_K,P_K,W_Flag-1));
        return (NOTOK);
    }

    INT_SEQ += 1;
    Log(USR_OK, "data write[%s:%d:%d]", OFN(D_K,P_K,W_Flag-1), OFW_CNT(0,W_Flag-1), 1);
    Log(USR_OK, "Write ok[%s][%d]", w_data, strlen(w_data));
    Set_TR_Time();

    return (OK);
}   /* End of Write_Data () */

/*************************************************************************
    Function        : Register_Signal
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 시그널 핸들러 등록.
                      SIGPIPE: 상대방 소켓 종료 시 발생 → Catch_Signal
                      SIGTERM: 프로세스 종료 요청 → Catch_Signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Register_Signal(void) {
    struct sigaction act;

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = Catch_Signal;

    if (sigaction(SIGPIPE, &act, NULL) < 0) {
        Log(SYS_ERROR, "sigaction(SIGPIPE) {%d:%s}", SYS_NO, SYS_STR);
        return;
    }

    if (sigaction(SIGTERM, &act, NULL) < 0) {
        Log(SYS_ERROR, "sigaction(SIGTERM) {%d:%s}", SYS_NO, SYS_STR);
        return;
    }

    return;
}   /* End of Register_Signal () */

/*************************************************************************
    Function        : Catch_Signal
    Parameters IN   : signo - 수신된 시그널 번호
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 시그널 핸들러. SIGPIPE/SIGTERM 수신 시
                      TCP 연결을 해제하고 프로세스를 종료한다.
                      _in_signal_handler로 재진입 방지.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Catch_Signal(int signo) {
    _in_signal_handler = 1;                 /* 시그널 핸들러 진입 표시 (재진입 방지) */
    SIG_WRITE_MSG("[SIGNAL] pa_2700_tr caught signal\n");

    TCP2_CON_STA = OFF;                     /* 접속 상태 OFF */
    close(Sockfd);
    Exit_Process();
}   /* End of Catch_Signal () */

/*************************************************************************
    End of Program (pa_2700_tr.c)
*************************************************************************/
