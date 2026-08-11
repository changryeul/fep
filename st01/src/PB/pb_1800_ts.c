#define     _GLOBAL

/*------------------------------------------------------------------------
#   System  : Connect to KRX
#   Author  : PSH
#   Module  : 주문송신
#   File    : pb_1800_ts.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "krx_trcode.h"
#include    "ifaddrs.h"
#include    "fep_common.h"
#include    "fep_encrypt.h"

/* 주문 송신 Size 직결 */
/* 채권일반_254, 채권지수_255 */
#define     DATA_SIZE       500           /* 직결Header(55) + Data(244 or 245)_업무헤더 */
#include    "buf_struct.h"

/* 55바이트 모듈명� 중간가 */

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     TCP_TIME_OUT    35

#define     DEVICE_TIME     3 * 1000                        /*  3 sec   */
#define     MAIN_TIME       5 * 1000        /* Heart Beat 직결  5 sec   */
#define     DATA_TIME       15 * 1000       /* Heart Beat 3회  15 sec    */
#define     FOREVER_TIME    60 * 1000                       /* 60 sec   */

#define     FIFO_EVENT      0
#define     SOCKET_EVENT    1
#define     DATA_EVENT      2

#define     MAX_CNT         1
#define     RESP_GAP        1000

#define     NO_TIME         "0830"      // 채우기�Խ직결� 09가 ~ 15가 직결, 시간외 직결

#ifdef  SAM_USE
#define     WR_CNT          W_CNT(0,0)
#define     RD_CNT          R_CNT(0,0)
#define     IN_NAME         IFN(D_K,P_K,0)
#else
#define     WR_CNT          IDW_CNT(0,0)
#define     RD_CNT          IDR_CNT(0,0)
#define     IN_NAME         IDN(D_K,P_K,0)
#endif
#define     PORT_NO         TCP2_PORT_NO

/*
KRX 주문 직결 직결�帧가
0101 : ȣ시장순서데로,           직결Continue,   직결retry
0004 : 모듈명 일련번호 직결,   직결Close,      처리요구 다시
0090 : 시스템에가,                직결Close,      처리요구 다시
0013 : ȣ시장순서데로,           직결Close,      처리요구 다시

기타
0020 : TPS가건수 초과,        1초과 주문sleep
0102 : 매매거래시간 직결 가 (체결회선)
COD모듈명

기타2
주문메인서버가 체권메인서버가 ü�ܶ가� 가 가 LogOut

*/

/* 암복호화 추가 */
#ifndef NO_INISAFE
char        KRX_INITECH_CONF_PATH[256];

// 직결 직결 필요
net_ctx *EnCtx = NULL;

unsigned char   *sinitout = NULL;
unsigned char   *supdateout = NULL;

unsigned char   *cinitout = NULL;
unsigned char   *cupdateout = NULL;
unsigned char   *cfinalout = NULL;

int             cinitoutl = 0;
int             cupdateoutl = 0;
int             cfinaloutl = 0;
#endif

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int     ConnectRetryCnt;
int     Sockfd, ErrCd, SendLen, MsgLen, RecvLen, ReTrCode;
int     PollCnt, FirstSeq, TimeOut;
int     back_int_seq, back_rd_cnt;
double  RTime = 0, STime = 0;
double  SendMsec, RecvMsec, RespMsec;
char    DeviceSendFlag, LogOnFlag, OpenFlag, DataBuff[KRX_DATA_BUFF_SIZE], IpAddr[20];
char    NoTime[] = NO_TIME;
char    S_Data[KRX_DATA_BUFF_SIZE];

FILE_BUFF_FORMAT            R_Fmt[MAX_CNT], W_Fmt[MAX_CNT];
KRX_HEADER                  Header_Fmt;  // KRX Header (82)
KRX_JUMUN_R_FMT             Reply;  // 82+ 4+11 직결
KRX_SESSION_FMT             S_Fmt;  // KRX 직결 직결 (82+41)

struct pollfd       Poll[3];
void    *FmtPtr = (void *)&S_Fmt;

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PB_1100_TS(void);
void    Init_Parameters(void);
void    Socket_Event_Rtn(void);
void    Data_Event_Rtn(void);
void    Device_Open(int);
void    Device_Close(void);
void    Device_Write(void);
int     Device_Read(void);
void    Time_Out_Rtn(void);
int     Analyze_Data(void);
void    Write_Response_Data(int);
int     Make_Send_Msg(int);
int     Make_Data_Block(void);
void    Log_Out(void);
#ifndef NO_INISAFE
unsigned char* EncryptAndMakeSendPacket(const void*, size_t, int*);
#endif

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    Init_Proc(argc, argv);
    PB_1100_TS();
    Exit_Process();
}   /* End of main ()   */

/*----------------------------------------------------------------------*/
void    PB_1100_TS(void)
/*----------------------------------------------------------------------*/
{
    int     rt, i;

    Init_Parameters();

    while (START_S != END) {
        Stat_Save();

        if (TCP2_NET_STA(S_K) == END || TCP2_NET_STA(S_K) == JOB_STOP) {                                               /* 직결/직결    */
                if (LogOnFlag == ON && TCP2_NET_STA(S_K) == END) {
                Device_Close();
            }

            continue;
        }
        else                                            /* 서버주문시각 */ {
            /* -------------------------------------------------------------------- */
            /* MAIN, BACKUP, S_K 가뿹                                            */
            /* MAIN, BACKUP가 Memory가 1가 이상함 모듈명 협의시(Socket, Netstat가)*/
            /* 메모리 업무헤더 가�ǵ� 직결 모듈명. 이외 Process가 업무헤더 Socket */
            /* 처리를 하는 Logic직결 가�.(Tcp2.ini 직결)                          */
            /* S_K가 MAIN, BACKUP가 가�ǵ� 직결 직결 가되는 Socket가 직결� 직결  */
            /* - 주목적:                                                         */
            /*  업무헤더 Socket직결 직결 가� Socket가 기록하고 활용하기 직결 가  */
            /* - 모듈명�:                                                           */
            /*  MAIN, BACKUP: Memory가 Status 직결� 가� (Netstat, Tcp2_stat가)  */
            /*  S_K: 비교하여 가븸 가                                             */
            /* -------------------------------------------------------------------- */
            /* LogOnFlag: TCP Connect & LOGON check Flag                            */
            /*            Init_Parameter => OFF.                                    */
            /*            Device_Open    => ON.                                     */
            /*            Device_Close   => OFF.                                    */
            /* -------------------------------------------------------------------- */
            /* OpenFlag: 업무헤더 Flag, DATA 전송업무 직결 직결                     */
            /*             Init_Parameter => OFF.                                   */
            /*             Socket_Event_rtn => ON.                                  */
            /*             Device_Close => OFF.                                     */
            /* -------------------------------------------------------------------- */
            /* DeviceSendFlag: DATA 직결 check Flag                                 */
            /*                 Init_Parameter => OFF.                               */
            /*                 Socket_Event_Rtn => OFF.                             */
            /*                 Time_Out_Rtn => Device_Write()가 ON(TR_POLL)         */
            /*                 Data_Event_Rtn => ON.                                */
            /*                 Make_Send_Msg (TR_DATA) => Device_Write()가 ON.      */
            /* 2025 : Data_Event_Rtn & Make_Send_Msg 싱크시에만 가ȿ, �매크로 업무헤더 직결 */
            /* -------------------------------------------------------------------- */

            /* 개시를 해야만 주문만 가 가 직결, 모듈명 모듈명 모듈명 기본마스터 업무헤더 미해당. 개시를 retry */
            if (OpenFlag == OFF) {  // 장시간 �ȵ활용
                if (LogOnFlag == OFF)  // 로그온이 �ȵ활용 로그온 시도
                    Device_Open(TR_LOON);

                if (LogOnFlag == OFF) {  // 직결 로그온이 �ȵ활용
                    PollCnt = 1;
                    TimeOut = DEVICE_TIME;  // 3가
                }
                else {  // 로그온이 �활용
                    Device_Open(TR_LINK);

                    if (OpenFlag == OFF) {  // 직결 로그온이 �ȵ활용
                        PollCnt = 1;
                        TimeOut = DEVICE_TIME;  // 3가
                    }
                    else {
                        PollCnt = 3;
                        TimeOut = MAIN_TIME;  // 5가
                    }
                }
            }
            else {  // 장시간 가 가�
                if (DeviceSendFlag == ON) {  // DATA가 송신�Ѱ가, LogOut , 직결ް� 전략처리를가. DeviceSendFlag가 LogOut가 ON
                    PollCnt = 2;
                    TimeOut = DATA_TIME;  // 15가
                }
                else {  // DATA가 송신�Ѱ가, 주문/HeartBeat/KillSwitch , Async가 송수신 기록하고
                    Log(USR_OK, "OK22 W[%d] R[%d]", WR_CNT, RD_CNT);
                    PollCnt = 3;
                    TimeOut = MAIN_TIME;  // 5가

                    if (WR_CNT > RD_CNT && DeviceSendFlag == OFF) {
                        Data_Event_Rtn();
                        continue;
                    }
                }
            }
        }

        rt = poll(Poll, PollCnt, TimeOut);
        if (rt < 0) {
            if (SYS_NO == EINTR)
                Log(SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
            else
                Log(SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);

            continue;
        }
        else if (rt == 0) {
            Time_Out_Rtn();
            continue;
        }

        /* poll 이벤트 단일 순회 — POLLHUP 체크 후 POLLIN 처리 */
        for (i = 0; i < PollCnt; i ++) {
            if (Poll[i].revents & POLLHUP) {
                if (i == SOCKET_EVENT) {
                    Log(TCP_ERROR, "socket disconnected[%#06x]",
                            Poll[i].revents);
                    return;
                }

                Log(SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
            }

            if (Poll[i].revents & POLLIN) {
                Poll[i].revents = 0;

                switch (i) {
                    case    FIFO_EVENT:
                        Fifo_Event_Rtn();
                        break;
                    case    SOCKET_EVENT:
                        Socket_Event_Rtn();
                        break;
                    case    DATA_EVENT:
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

    Device_Close();

    return;
}   /* End of PB_1100_TS () */

/*************************************************************************
    Function        : . Init_Parameters
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . All Program Parameters Init
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Init_Parameters(void)
/*----------------------------------------------------------------------*/
{
    DeviceSendFlag = OFF;
    LogOnFlag = OFF;
    OpenFlag = OFF;
    ErrCd = 0;
    ConnectRetryCnt = 0;

    if (TIME_OUT == 0)
        TIME_OUT = TCP_TIME_OUT;
    Log(USR_OK, "TIME_OUT[%d]", TIME_OUT);

    S_K = TCP2_LINE_GU;

    sprintf(IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
            TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    Log(TCP_OK, "IP[%s] port[%d]", IpAddr, PORT_NO);

    TCP2_PROC_ST = ON;
    TCP2_PROC_ST1(MAIN) = ON;
    TCP2_PROC_ST1(BACKUP) = ON;

    TCP2_LINE_ST = OFF;
    TCP2_LINE_ST1(MAIN) = OFF;
    TCP2_LINE_ST1(BACKUP) = OFF;

    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;
    Poll[2].fd = INPUT_FD;
    Poll[2].events = POLLIN;

    if (DELAY_TIME == 0)
        DELAY_TIME = RESP_GAP;

    /* 202509 복호화 초기화 함수 호가 */
    //  INL_Initialize(CLIENT_CTX, KRX_INITECH_CONF_PATH, NULL);
    //  INL_New_Ctx(CLIENT_CTX, &EnCtx);

#ifndef NO_INISAFE
    {
        char *env_inisafe = getenv("INISAFENET_HOME");
        if (env_inisafe == NULL) {
            Log(SYS_FATAL, "getenv(INISAFENET_HOME) is NULL");
            return;
        }
        memset(KRX_INITECH_CONF_PATH, 0, sizeof(KRX_INITECH_CONF_PATH));
        sprintf(KRX_INITECH_CONF_PATH, "%s/conf/INISAFENet.cnf", env_inisafe);
        Log(USR_OK, "KRX_INITECH_CONF_PATH[%s]", KRX_INITECH_CONF_PATH);
    }
#endif

    return;
}   /* End of Init_Parameters ()    */

/*************************************************************************
    Function        : . Socket_Event_Rtn
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . TCP Data Recv & Response Action
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Socket_Event_Rtn(void)
/*----------------------------------------------------------------------*/
{
    int     rval, rt, cnt;
    char    t_time[12];

    rval = Device_Read();

    if (rval < 0)
        return;

    /* ********************************************************** */
    /*
        1.   LogOn      SCHLIR00000 : 87  ( 5) = 82 +4+1
        2.   LIOK(주문) SCHOPR00000 : 108 (26) = 82 +4+11+11
        3.   LIOK(체권) SCHOPR10000 : 198 (27) = 82 +4+2+11*10
        4.   HeartBeat  SCHHER00000 : 86  ( 4) = 82 +4 (송수신 직결)
        5.   LogOut     SCHLOQ00000 : 86  ( 4) = 82 +4
        6.   주문거부   TCHODR00000 : 97  (15) = 82 +4+11
        6-1. 회원처리호가/체권

        KRX_SESSION_FMT             S_Fmt    : 82 + Data(41)
        KRX_NOTE_ALL_JUMUN_Q_FMT    J_Q_Fmt  : 82 + 300
        KRX_NOTE_JUMUN_R_FMT        J_R_Fmt  : 82 + 4+11

        - 주문번문
        1. LogOn직결,           모듈명
        2. LIOK(LINK직결),      모듈명
        4. HeartBeat직결,       가�⼭
        6. 주문번문(거부),        가�⼭

        - 체결후�
        1.   LogOn직결
        3.   LIOK(LINK직결)
        5.   LogOut신청
        6-1. 회원처리호가/체권
    */
    /* ********************************************************** */
    DeviceSendFlag = OFF;

    memset(&S_Fmt, 0, sizeof (KRX_SESSION_FMT));
    /* sfmt-overflow-fix: 구조체 크기로 클램프 (BSS 오버런 방지) */
    memcpy(&S_Fmt, DataBuff,
            RecvLen > (int)sizeof (KRX_SESSION_FMT) ?
            (int)sizeof (KRX_SESSION_FMT) : RecvLen);

    ReTrCode = Analyze_Data();

    /* ************************************ */
    /* 가�⼭가 RP_DATA, RP_POLL 2직결 체크 */
    /* ************************************ */
    switch (ReTrCode) {
        case    RP_DATA:    /* 주문전문의 "0020"(TPS가하지않고)직결, 가� 거부 */
            /* Seq 처리 가 직결 */
            if (FirstSeq > INT_SEQ) {  // 수신버퍼 Seq가 직결 INT_SEQ직결 클수없다. 모듈명 Process직결.
                TCP2_LINE_ST = OpenFlag = OFF;

                Log(USR_ERROR, "RP_DATA recv:invalid KRX SEQ <%d:%d>",
                        FirstSeq, INT_SEQ);

                sleep(3);  // 3직결 직결
                Exit_Process();  // 직결, �뒤에가 알리기 통해서
        }
        //          else if (FirstSeq == INT_SEQ)           // 직결
        else if (FirstSeq < INT_SEQ) {  // 업무헤더
            /* Data 모듈명 모듈명 직결�迡 모듈명 �״가 만돌려준다.
               모듈명� 1건씩 넣고 받는데 1건별로가 로직이며 기본마스터
                                                    모듈명 모듈명 모듈명
               모듈명� 모듈명� 한다.(업무헤더� Sync가�)         */
            Log(USR_WARN, "RP_DATA recv:Seq Change INT_SEQ[%d] => FirstSeq[%d]",
                    INT_SEQ, FirstSeq);

            /* 직결�ڵ尡 모듈명 ReadCount가 �ΰ� INT_SEQ가 -1한다. */
            if (memcmp(S_Fmt.Data, RESP_SUCCESS, KRX_ERRCODE_LEN) != 0)
                RD_CNT = RD_CNT -1;
        }

        /* 직결� 모듈명� 사용한다. 직결�̵� 모듈명 */
        Write_Response_Data(1);

        break;
        case    RP_POLL:
            //          Make_Send_Msg (TR_POLL);
            //          memset (DataBuff, 0, sizeof (DataBuff));
            //          memcpy (DataBuff, &S_Fmt, SendLen);
            //          Device_Write ();
            break;
        default:
            break;
    }

    return;
}   /* End of Socket_Event_Rtn ()   */

/*************************************************************************
    Function        : . Data_Event_Rtn
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . SHM Data Processing
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Data_Event_Rtn(void)
/*----------------------------------------------------------------------*/
{
    int     rt;
    char    tmp[128];

    ErrCd = 0;
    memset(DataBuff, 0, sizeof (DataBuff));

    rt = Make_Send_Msg(TR_DATA);
    if (rt == 0) {  // 직결
        // 2025 암복호화 모듈명 가�⼭ 하지않고 Make_Send_Msg직결 DataBuff가 고려해서 처리해둔다.
        Device_Write();

        Add_Count(PS_R_1, 1);
        INT_SEQ++;

        if (LogOnFlag == ON)
            Set_TR_Time();

        DeviceSendFlag = ON;
    }

    rt = read(INPUT_FD, tmp, sizeof(tmp));

    return;
}   /* End of Data_Event_Rtn () */

/* Free_All, Handshake: moved to sub/fep_encrypt.c */

/*************************************************************************
    Function        : .  Device_Open
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . Svm Line Status Set & TCPIP Poll fd set & LOGON
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Open(int tr_code)
/*----------------------------------------------------------------------*/
{
    int     rt, rval;

    if (tr_code == TR_LOON) {
        Sockfd = Socket();

        if (Sockfd < 0) {
            Log(TCP_ERROR, "socket fail:Sockfd[%d] {%d:%s}",
                    Sockfd, SYS_NO, SYS_STR);
            return;
        }

        Log(USR_OK, "socket created:Sockfd[%d]", Sockfd);
        Log(USR_OK, "connecting to %s:%d", IpAddr, PORT_NO);

        rt = Connect(Sockfd, IpAddr, PORT_NO);

        if (rt < 0) {
            Log(TCP_ERROR, "connect fail {%d:%s}", SYS_NO, SYS_STR);
            close(Sockfd);
            return;
        }

        if (Handshake() < 0) {
            close(Sockfd);
            return;
        }

        rt = Make_Send_Msg(TR_LOON);
        memset(DataBuff, 0, sizeof (DataBuff));
        memcpy(DataBuff, &S_Fmt, SendLen);
        Device_Write();
        Log(USR_OK, "send LOGON request");

        rval = Device_Read();
        if (rval < 0 ||
                memcmp(((KRX_HEADER *)DataBuff)->MsgType, "SCHLIR00000", 11) != 0) {
            Log(USR_ERROR, "LOGON response recv error TrCode[%11.11s]", ((KRX_HEADER *)DataBuff)->MsgType);
            close(Sockfd);
            return;
        }

        if (IS_RESP_OK(DataBuff)) {
            Log(USR_OK, "LOGON success");
        }
        else {
            ErrCd = AtoIf(&DataBuff[KRX_HEAD_LEN], KRX_ERRCODE_LEN);
            Err_Msg();

            close(Sockfd);
            return;
        }

        LogOnFlag = ON;

        Poll[1].fd = Sockfd;
        Poll[1].events = POLLIN;
        Log(TCP_OK, "TCP Connect & LOGON OK");
    }
    else if (tr_code == TR_LINK) {
        while (START_S != END) {
            rt = Make_Send_Msg(TR_LINK);
            memset(DataBuff, 0, sizeof (DataBuff));
            memcpy(DataBuff, &S_Fmt, SendLen);
            Device_Write();
            Log(USR_OK, "send LINK request");

            rval = Device_Read();
            if (rval < 0 ||
                    memcmp(((KRX_HEADER *)DataBuff)->MsgType, "SCHOPR00000", 11) != 0) {
                Log(USR_ERROR, "LINK response recv error");
                close(Sockfd);
                break;
            }

            memset(&S_Fmt, 0, sizeof (KRX_SESSION_FMT));
            /* sfmt-overflow-fix: 구조체 크기로 클램프 */
            memcpy(&S_Fmt, DataBuff,
                    RecvLen > (int)sizeof (KRX_SESSION_FMT) ?
                    (int)sizeof (KRX_SESSION_FMT) : RecvLen);

            FirstSeq = AtoIf(S_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));

            Log(USR_OK, "시장순서데로(SCHOPR00000): [%4.4s] <%d:%d>",
                    S_Fmt.Data, FirstSeq, INT_SEQ);

            if (memcmp(S_Fmt.Data, RESP_SUCCESS, KRX_ERRCODE_LEN) == 0) {
                /* Seq 처리 */
                if (FirstSeq > INT_SEQ) {
                    TCP2_LINE_ST = OpenFlag = OFF;

                    Log(USR_ERROR, "TR_LINK recv:invalid KRX SEQ <%d:%d>",
                            FirstSeq, INT_SEQ);

                    sleep(3);  // 3직결 직결
                    Exit_Process();  // 직결, �뒤에가 알리기 통해서
                }
                else if (FirstSeq < INT_SEQ) {
                    Log(USR_WARN, "TR_LINK recv:Seq Change [%d] => [%d] KRX SEQ <%d>",
                            INT_SEQ, FirstSeq, FirstSeq);

                    RD_CNT = INT_SEQ = FirstSeq;  // 모듈명 Seq가 직결
                    PROC(D_K,P_K).counter_seq = FirstSeq;  // 모듈명 Seq가 직결
                }

                TCP2_NET_STA(S_K) = ON;
                TCP2_LINE_ST = OpenFlag = ON;
                ConnectRetryCnt = 0;

                Log(TCP_OK, "LINK OK");

                break;
            }
            else {
                if (memcmp(S_Fmt.Data, "0101", 4) == 0) {  // 호가구간 모듈명
                    Log(USR_OK, "0100 호가가격이 개시요청 3직결 Retry!!!");
                    sleep(3);  // 3가 쉬었다가 �簳가 시도

                    break;
                }

                /* 0101가 다른 모듈명 Process가 죽여서 �뒤에가 �˷직결� */
                Log(USR_ERROR, "TR_LINK recv:invalid ResponseCode[%4.4s] KRX SEQ <%d:%d>",
                        S_Fmt.Data, FirstSeq, INT_SEQ);

                sleep(3);  // 3직결 직결
                Exit_Process();  // 직결, �뒤에가 알리기 통해서
            }
        }  // End Of While
    }

    return;
}   /* End of Device_Open ()    */

/*************************************************************************
    Function        : .  Device_Close
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . Svm Line Status Set
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Close(void)
/*----------------------------------------------------------------------*/
{
    close(Sockfd);
    Log(TCP_OK, "TCP device close");

    LogOnFlag = OFF;
    TCP2_NET_STA(S_K) = OFF;
    TCP2_LINE_ST = OpenFlag = OFF;

    /* 2025 복호화 초기화 작업 */
#ifndef NO_INISAFE
    Free_All((void *)(long)1);
    INL_Cleanup(CLIENT_CTX);
#endif

    return;
}   /* End of Device_Close ()   */

/*************************************************************************
    Function        : .  Device_Write
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . Tcpip Data Send
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Write(void)
/*----------------------------------------------------------------------*/
{
    int     rt;

    //    rt = Select_Send (Sockfd, DataBuff, strlen (DataBuff));
    rt = Select_Send(Sockfd, DataBuff, SendLen);

    /* 모듈명�޿� */
    memset(S_Data, 0, sizeof(S_Data));
    memcpy(S_Data, DataBuff,   SendLen);

    if (rt != OK) {
        Log(TCP_ERROR, "TCP data send fail");
        Device_Close();
    }

    //    Log (TCP_OK, "TCP SD [%s](%d)<%d>", DataBuff, strlen (DataBuff), INT_SEQ);
    Log(TCP_OK, "TCP SD [%s](%d)<%d>", DataBuff, SendLen, INT_SEQ);

    /* 모듈명 다른부분 */

    return;
}   /* End of Device_Write ()   */

/*************************************************************************
    Function        : .  Device_Read
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int (0:success, -1:failure)
    Comment         : . Tcpip Data Recv
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Device_Read(void)
/*----------------------------------------------------------------------*/
{
    int     rt;
    char    m_time[24];

    memset(DataBuff, 0, sizeof (DataBuff));
    RecvLen = 0;

    rt = Select_Receive_Krx(Sockfd, DataBuff, KRX_HEAD_LEN);

    if (rt <= 0) {
        Device_Close();
        return (NOTOK);
    }

    RecvLen = rt;
    Log(TCP_OK, "TCP RD [%s](%d)<%d>", DataBuff, RecvLen, INT_SEQ);

    return (OK);
}   /* End of Device_Read ()    */

/*************************************************************************
    Function        : .  Time_Out_Rtn
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . Timeout Control
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Time_Out_Rtn(void)
/*----------------------------------------------------------------------*/
{
    int     rt;

    memset(DataBuff, 0, sizeof (DataBuff));

    switch (PollCnt) {
        case    2:
            if (TCP2_NET_STA(S_K) == OFF || TCP2_NET_STA(S_K) == ON) {
                TCP2_LINE_ST = END;
                Log(TCP_ERROR, "no data from KRX. check status <%d>", INT_SEQ);
                Device_Close();
                ConnectRetryCnt ++;

                if (ConnectRetryCnt == 3) {
                    Line_Change();
                    ConnectRetryCnt = 0;
            }
        }

        break;
        case    3:
            rt = Make_Send_Msg(TR_POLL);
            memcpy(DataBuff, &S_Fmt, SendLen);
            Device_Write();
            DeviceSendFlag = ON;
            break;
        default:
            break;
    }

    return;
}   /* End of Time_Out_Rtn ()   */

/*************************************************************************
    Function        : . Analyze_Data
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int TR_TYPE
    Comment         : . TCP Recv Data Analysis And TR_CODE Return
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Analyze_Data(void)
/*----------------------------------------------------------------------*/
{

    FirstSeq = AtoIf(S_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));

    if (memcmp(S_Fmt.Header.MsgType, "SCHHER00000", 11) == 0) {  // HeartBeat
        Log(USR_OK, "회원사는�û(SCHHER00000): [%4.4s] <%d:%d>",
                S_Fmt.Data, FirstSeq, INT_SEQ);
        return (RP_POLL);
    }
    else if (memcmp(S_Fmt.Header.MsgType, "SCHLOR00000", 11) == 0) {  // LogOut
        Log(USR_OK, "LogOut신청(SCHLOR00000): [%4.4s] <%d:%d>",
                S_Fmt.Data, FirstSeq, INT_SEQ);
        return (RP_STOP);
    }
    else if (memcmp(S_Fmt.Header.MsgType, "TRDERP00101", 11) == 0) {  // 직결
        Log(USR_OK, "장치 직결(TRDERP00101): <%d:%d> �źλ�레코드[%4.4s]",
                FirstSeq, INT_SEQ, S_Fmt.Data);
        return (RP_DATA);
    }

    Log(USR_ERROR, "Analyze_Data: unknown MsgType[%.11s]", S_Fmt.Header.MsgType);
    return (RP_STOP);
}   /* End of Analyze_Data ()   */

/*************************************************************************
    Function        : . Write_Respose_Data
    Parameters IN   : . d_cnt   : write count
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . 주문거부에 직결 가 대용주권, file write
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Write_Response_Data(int d_cnt)
/*----------------------------------------------------------------------*/
{
    int     i, rt, cnt;
    char    resp_time[12];

    if (d_cnt <= 0)
        return;

    /* 직결 수신버퍼 직결 사용한다. */
    /* KRX Header 82가 직결(15=4+11가) */
    /* sync가 1직결 처리한다 */
    for (i = 0; i < d_cnt; i ++) {
        /* Space가 초기화 */
        memset(W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);

        /* 1. Seq (8) */
        ItoAf(OFW_CNT(0,0)+1,          W_Fmt[i].Seq,               sizeof (W_Fmt[0].Seq));
        /* 2. If_Seq (8) */
        memcpy(W_Fmt[i].If_Seq,        S_Fmt.Header.MsgSeqNum,     sizeof (W_Fmt[0].If_Seq));  // 직결� Seq
        /* 3. ApType (8) */
        memcpy(W_Fmt[i].ApType,        R_Fmt[i].ApType,            sizeof (W_Fmt[0].ApType));
        /* 4. ResponseCode (4), 모듈명 업무헤더 한다.(0000), 단축코드� 직결 */
        memcpy(W_Fmt[i].ResponseCode,  RES_NORMAL,                 strlen(RES_NORMAL));
        /* 5. set KRX response time   (10)
        sprintf (resp_time, "%010.06f", RespMsec);
        memcpy (W_Fmt[i].RecvTime1, resp_time, sizeof (W_Fmt[i].RecvTime1));
        */
        /* 6. RecvTime2 (12) */
        memcpy(W_Fmt[i].RecvTime2,     &S_Fmt.Header.SendingTime[8],   9);  // 12자리임 9자리임
        /* 7. DataHeader(20), Async가.. 알수없다 */
        memcpy(W_Fmt[i].DataHeader,    R_Fmt[i].DataHeader,        HEAD_SIZE);
        /* 8. DATA 직결(4+11 +300) */

        /* Write Format : 4+11 + 55(SPACE) + 주문번문 */
        /* KRX Reply (4+11) */
        memcpy(W_Fmt[i].Data,          S_Fmt.Data,                 sizeof(KRX_NOTE_JUMUN_R_DATA));  // 4+11
        /* Read Order Data (420 include tmp) */
        memcpy(&W_Fmt[i].Data[sizeof(KRX_NOTE_JUMUN_R_DATA)],        R_Fmt[i].Data, 420);
        W_Fmt[i].LineFeed[0] = '\n';

    }

    rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);
    if (rt != 1) {
        Log(SAM_FATAL, "file write fail[%s:%d]", OFN(D_K,P_K,0), rt);
        return;
    }

    Log(USR_OK, "Write!! Error Response Data[%s:%d:%d]", OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), i);

    return;
}   /* End of Write_Response_Data ()    */

/*************************************************************************
    Function        : . Make_Send_Msg
    Parameters IN   : . TR_TYPE
    Parameters OUT  : .
    Return Code     : . int (-1:no data, 0:success)
    Comment         : . Send data header making
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Make_Send_Msg(int tr_code)
/*----------------------------------------------------------------------*/
{
    int     rt, datacnt = 1;
    char    d_time[18];

    rt = 0;
    memset(&S_Fmt, 0, sizeof (KRX_SESSION_FMT));
    memset(&S_Fmt, 0x20, sizeof (KRX_HEADER));

    /* Make The Header */
    // 1. 업무헤더
    memcpy(S_Fmt.Header.BeginString, "KMAPv2.0",   8);
    // 2. 메시지길이(직결 Set)
    ItoAf(0, S_Fmt.Header.BodyLength, sizeof (S_Fmt.Header.BodyLength));
    // 3. 메세지Type(직결 Set)
    ItoAf(0, S_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));
    // 4. 일련번호(직결 Set)
    // 5. 회원번호
    memcpy(S_Fmt.Header.SenderCompID, TCP_COMPANY, strlen(TCP_COMPANY));
    Get_DateMilliTime(d_time);
    // 6. �재시도가 회차별 부호(SPACE)
    // 7. 회신시송신 회차별 부호(SPACE)

    // 8. 비지니스
    memcpy(S_Fmt.Header.SendingTime, d_time, sizeof (S_Fmt.Header.SendingTime));
    // 9. 데이터건수(직결 Set)
    ItoAf(0, S_Fmt.Header.DataCnt, sizeof (S_Fmt.Header.DataCnt));
    // 10. 복호화 직결
    memcpy(S_Fmt.Header.Encrypt,       "N",        1);  // 초기� N, 주문만 Y
    SendLen = KRX_HEAD_LEN;

    switch (tr_code) {
#ifndef NO_INISAFE
        case    TR_HSI:     /* handshake init */
            memcpy(S_Fmt.Header.MsgType,  "SCHLIQ00101",      11);
            memcpy(S_Fmt.Data, "0000", 4);  // INL_Handshake_Init() return value
            memcpy(&S_Fmt.Data[4], (char *)cinitout, cinitoutl);
            MsgLen = cinitoutl + 4;
            ItoAf(MsgLen, S_Fmt.Header.BodyLength,    sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_HSU:     /* handshake update */
            memcpy(S_Fmt.Header.MsgType,  "SCHLIQ00103",      11);
            memcpy(S_Fmt.Data, "0000", 4);  // INL_Handshake_Update() return value
            memcpy(&S_Fmt.Data[4], (char *)cupdateout, cupdateoutl);
            MsgLen = cupdateoutl + 4;
            ItoAf(MsgLen, S_Fmt.Header.BodyLength,    sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_HSF:     /* handshake final */
            memcpy(S_Fmt.Header.MsgType,  "SCHLIQ00105",      11);
            memcpy(S_Fmt.Data, "0000", 4);  // INL_Handshake_Final() return value
            memcpy(&S_Fmt.Data[4], (char *)cfinalout, cfinaloutl);
            MsgLen = cfinaloutl + 4;
            ItoAf(MsgLen, S_Fmt.Header.BodyLength,    sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
#endif
        case    TR_LOON:    /* 로그온 신청 */
            memcpy(S_Fmt.Header.MsgType,   "SCHLIQ00000",  11);
            ItoAf(INT_SEQ, S_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));
            memset(S_Fmt.Data, 0x20, 41);  // Logon size
            memcpy(S_Fmt.Data,         LOGON_ID(D_K,P_K), 10);
            //          memcpy (&S_Fmt.Data[10],    LOGON_PW(D_K,P_K), 30);
            memcpy(&S_Fmt.Data[10],    LOGON_PW(D_K,P_K), strlen(LOGON_PW(D_K,P_K)));
            memcpy(&S_Fmt.Data[40],    "Y",                1);
            MsgLen  = strlen(S_Fmt.Data);
            ItoAf(MsgLen, S_Fmt.Header.BodyLength, sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_LINK:    /* 업무헤더 신청 */
            memcpy(S_Fmt.Header.MsgType,   "SCHOPQ00000",  11);
            ItoAf(INT_SEQ, S_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));
            memcpy(S_Fmt.Data, "0000", 4);
            memcpy(&S_Fmt.Data[4], "           ", 11);
            ItoAf(INT_SEQ, &S_Fmt.Data[15],    11);
            MsgLen  = strlen(S_Fmt.Data);
            ItoAf(MsgLen, S_Fmt.Header.BodyLength, sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_POLL:    /* 회원사는 신청 */
            memcpy(S_Fmt.Header.MsgType,       "SCHHEQ00000",  11);
            ItoAf(INT_SEQ, S_Fmt.Header.MsgSeqNum,     sizeof (S_Fmt.Header.MsgSeqNum));
            memcpy(S_Fmt.Data, "0000", 4);
            MsgLen  = strlen(S_Fmt.Data);
            ItoAf(MsgLen,  S_Fmt.Header.BodyLength,    sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_LOOU:    /* 로그아웃 신청 */
            memcpy(S_Fmt.Header.MsgType,       "SCHLOQ00000",  11);
            ItoAf(INT_SEQ, S_Fmt.Header.MsgSeqNum,     sizeof (S_Fmt.Header.MsgSeqNum));
            MsgLen  = 0;
            ItoAf(MsgLen,  S_Fmt.Header.BodyLength,    sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_DATA:    /* 장치송신 */
            memset(DataBuff,       0,              sizeof (DataBuff));
            memcpy(S_Fmt.Header.Encrypt,   "Y",            1);  // 초기� N, 주문만 Y
            memcpy(S_Fmt.Header.MsgType,   "TRDERP00101",  11);
            ItoAf(INT_SEQ+1, S_Fmt.Header.MsgSeqNum,   sizeof (S_Fmt.Header.MsgSeqNum));

            ItoAf(datacnt,         S_Fmt.Header.DataCnt,   3);

            Log(USR_OK, "TT01");
            MsgLen = Make_Data_Block();
            Log(USR_OK, "TT02");
            if (MsgLen <= 0) {
                rt = -1;
                break;
        }

        SendLen += MsgLen;
        break;
        default:
            break;
    }

    return (rt);
}   /* End of Make_Send_Msg ()  */

/*************************************************************************
    Function        : . Make_Data_Block
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int
                        직결: KRX send record count (1 ~ MAX_CNT)
                        0: no data
    Comment         : . 1) If Memory Data Exist -> Memory Data Send
                        2) Data File Read & Time Out Check
                            Time Out Check -> PS_EW  Write -> PS_R_1 Seq Add
                            KRX Send Data  -> Seq Set & Memory Load
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Make_Data_Block(void)
/*----------------------------------------------------------------------*/
{
    int     r_cnt, t_cnt, i, j, rt, Size_Len, ow_seq;
    int     enc_len = 0;
    unsigned char *ret_data = NULL;
    char    err_cd[10], tmp[128], w_buf[FILE_BUF_LEN];

    i = t_cnt = Size_Len = 0;
    memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);
    memset(W_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);

    /* 송신 파일에서 KRX Header + 주문나간상태 받는다 */
    r_cnt = F_R(PS_R_1, (void *)R_Fmt, MAX_CNT);
    if (r_cnt < 0 || r_cnt > MAX_CNT) {
        Log(SAM_FATAL, "F_R(PS_R_1)[%s]", IFN(D_K,P_K,0));
        Exit_Process();
    }
    else if (r_cnt == 0) {
        return (0);
    }

    /* SKIP CHECK
        if ( memcmp (&R_Fmt[0].DataHeader[10],   "REJC", 4) == 0 ) {
            return (-2);        // SKIP
        }
    */

    /* KRX Header 82가 직결 */
    for (i = 0; i < r_cnt; i ++) {
        /* 수신버퍼 모듈명 모듈명 "55Byte + 주문번문" */
        if (memcmp(&R_Fmt[i].Data[11], "TRDERP00101", 11) == 0)    /* 회선시험요구(420) */ {
            Size_Len    = 420;
        }
        else {
            Log(USR_ERROR, "Undefined DataType Was Read [%s][%11.11s]", R_Fmt[i].Data, R_Fmt[i].Data);
            sleep(3);
            Exit_Process();
        }

        ItoAf(INT_SEQ+1, &R_Fmt[i].Data[0],                11);  // 일련번호 Update가 처리넘김

        // 가 신청 모듈명
        Log(USR_OK, "&R_Fmt[i].Data [%s]", &R_Fmt[i].Data);
        /* 202509 거래량 암복호화 처리 추가, Start */
        // 우선체결 넘김 직결 pointer + ũ�⸦ 직결 넘김� 가 모듈명
#ifndef NO_INISAFE
        ret_data = EncryptAndMakeSendPacket(&R_Fmt[i].Data,
                Size_Len,
                &enc_len);
        if (ret_data) {
            // 복호화된 데이터부를 J_Q_Fmt.JumunData[0]가 직결
            // 2025 암보화된 자료� 직결� 가 커진다. OverFlow모듈명 발생한.
            memcpy(DataBuff,       &S_Fmt.Header,  KRX_HEAD_LEN);
            ItoAf(enc_len,        &DataBuff[8],       6);
            memcpy(&DataBuff[78],  "001",      3);  // 단건만 처리
            memcpy(&DataBuff[sizeof(KRX_HEADER)],  ret_data,   enc_len);
            // 복호화 직결 직결
            INL_Free_Buf(ret_data);
            ret_data = NULL;
        }
        else {
            Log(USR_ERROR, "복호화 직결 error, 확대 필요");

            sleep(3);
            Exit_Process();
        }
#else
        /* NO_INISAFE: 평문 직접 복사 */
        memcpy(DataBuff,       &S_Fmt.Header,  KRX_HEAD_LEN);
        ItoAf(Size_Len,       &DataBuff[8],       6);
        memcpy(&DataBuff[78],  "001",      3);
        memcpy(&DataBuff[sizeof(KRX_HEADER)],  &R_Fmt[i].Data,   Size_Len);
        enc_len = Size_Len;
#endif

        /* 202509 거래량 암복호화 처리 추가, End */

    }  // 직결�δ� 1직결, n가 불가

    return (enc_len);  // 복호화된 Size가 넘김.
}   /* End of Make_Data_Block ()    */

/*************************************************************************
    Function        : . get time to the unit of msec (millisecond)
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . double
*************************************************************************/
#ifndef NO_INISAFE
/*----------------------------------------------------------------------*/
unsigned char* EncryptAndMakeSendPacket(const void* plain_data, size_t plain_len, int* enc_len)
/*----------------------------------------------------------------------*/
{
    int result = 0;
    unsigned char* enc_data = NULL;

    // 1. 복호화 직결 (EnCtx가 �ڵ어싱크는 이미 초기화시 가�¶가 직결)
    result = INL_Encrypt(EnCtx,
            (unsigned char*)plain_data,
            (int)plain_len,
            &enc_data,
            enc_len);
    if (result != 0) {
        Log(USR_ERROR, "ERROR: INL_Encrypt failed with code %d\n", result);
        if (enc_data) {
            INL_Free_Buf(enc_data);
        }
        return NULL;
    }

    // enc_data가 INL 내부에서 malloc/alloc 가 직결
    // 호출자가 �ݵ가 INL_Free_Buf() 되는 free()가 반응해야 가
    return enc_data;
}
#endif

/* Get_Msec: moved to sub/fep_common.c */

/*************************************************************************
    Function        : . Log_Out
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Log_Out(void)
/*----------------------------------------------------------------------*/
{
    int     rt, rval;

    rt = Make_Send_Msg(TR_LOOU);
    memset(DataBuff, 0, sizeof (DataBuff));
    memcpy(DataBuff, &S_Fmt, SendLen);
    Device_Write();
    Log(USR_OK, "send LOGOUT request");

    rval = Device_Read();
    if (rval < 0) {
        Log(USR_ERROR, "LOGOUT response recv error");
        close(Sockfd);
        return;
    }

    if (IS_RESP_OK(DataBuff)) {
        Log(USR_OK, "LOGOUT success");

        Device_Close();
        TCP2_NET_STA(S_K) = END;
    }
    else {
        ErrCd = AtoIf(&DataBuff[KRX_HEAD_LEN], KRX_ERRCODE_LEN);
        Err_Msg();
    }

    return;
}   /* End of Log_Out ()    */

/* Err_Msg, Line_Change: moved to sub/fep_common.c */

/*************************************************************************
    End of Program (pb_1100_ts.c)
*************************************************************************/
