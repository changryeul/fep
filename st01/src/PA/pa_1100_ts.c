#define     _GLOBAL

/*------------------------------------------------------------------------
#   System  : Connect to KRX
#   Author  : PSH
#   Module  : 주문송신
#   File    : pa_1100_ts.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "krx_trcode.h"
#include    "ifaddrs.h"

/* 주문 송신 Size 정의 */
/* 채권일반_254, 채권조성_255 */
#define     DATA_SIZE       400           /* 내부Header(55) + Data(244 or 245)_가변포함 */
#include    "buf_struct.h"
#include    "fep_common.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DEVICE_TIME     3 * 1000                        /*  3 sec   */
#define     MAIN_TIME       5 * 1000        /* Heart Beat 간격  5 sec */
#define     DATA_TIME       15 * 1000       /* Heart Beat 3회  15 sec    */
#define     FOREVER_TIME    60 * 1000                       /* 60 sec   */

#define     FIFO_EVENT      0
#define     SOCKET_EVENT    1
#define     DATA_EVENT      2

#define     MAX_CNT         1
#define     RESP_GAP        1000

#define     NO_TIME         "0830"      // 채권정규시장은 09시 ~ 15시 까지, 시간외 없음

#ifdef  SAM_USE
#define     WR_CNT          W_CNT(0,0)
#define     RD_CNT          R_CNT(0,0)
#define     IN_NAME         IFN(D_K,P_K,0)
#else
#define     WR_CNT          IDW_CNT(0,0)
#define     RD_CNT          IDR_CNT(0,0)
#define  IN_NAME   IDN(D_K,P_K,0)
#endif
#define     PORT_NO         TCP2_PORT_NO

#define     CHK_PROC        "pa_1401_mp"                    /* 체결Process가 인터페이스종료(TCHEDP99000) 수신후 종료되면 주문송신도 종료 */

/*
KRX 주문 서비스 업무흐름도
0101 : 호가접수개시전,             세션Continue, 개시retry
0004 : 데이터 일련번호 오류,     세션Close,        처음부터 다시
0090 : 시스템오류,               세션Close,        처음부터 다시
0013 : 호가접수정지중,             세션Close,        처음부터 다시

기타
0020 : TPS허용건수 초과,      1초간 주문sleep
0102 : 매매거래시간 종류 후 (체결회선)
COD사용안함

기타2
주문프로세스는 체결프로세스가 체겨라감을 한 후 LogOut

*/
/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int     ConnectRetryCnt;
int     Sockfd, ErrCd, SendLen, MsgLen, RecvLen, ReTrCode;
int     PollCnt, FirstSeq, TimeOut, Pk;
int     back_int_seq, back_rd_cnt;
double  RTime = 0, STime = 0;
double  SendMsec, RecvMsec, RespMsec;
char    DeviceSendFlag, LogOnFlag, OpenFlag, DataBuff[KRX_DATA_BUFF_SIZE], IpAddr[20];
char    NoTime[] = NO_TIME;

DATA_FORMAT                 *SHM_DATA;
FILE_BUFF_FORMAT            R_Fmt[MAX_CNT], W_Fmt[MAX_CNT];
KRX_NOTE_ALL_JUMUN_Q_FMT    J_Q_Fmt;            // KRX 주문포맷(max 6) (82+300)
KRX_NOTE_JUMUN_R_FMT        J_R_Fmt;            // KRX 세션 응답 포맷 (82+4+11)
KRX_SESSION_FMT             S_Fmt;              // KRX 세션 포맷 (82+41)
struct pollfd       Poll[3];
void *FmtPtr = (void *)&S_Fmt;
int     Handshake (void) { return (NOTOK); }    /* PA: no encryption */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_1100_TS (void);
void    Init_Parameters (void);
void    Socket_Event_Rtn (void);
void    Data_Event_Rtn (void);
void    Device_Open (int);
void    Time_Out_Rtn (void);
int     Analyze_Data (void);
void    Write_Response_Data (int);
int     Make_Send_Msg (int);
int     Make_Data_Block (void);
void    Log_Out (void);
int     Chk_Risk_All (char *);

/*----------------------------------------------------------------------*/
int     main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    Init_Proc (argc, argv);
    PA_1100_TS ();
    Exit_Process ();
}   /* End of main ()   */

/*----------------------------------------------------------------------*/
void    PA_1100_TS (void)
/*----------------------------------------------------------------------*/
{
    int     rt, i;

    Init_Parameters ();

    while (START_S != END) {
        Stat_Save ();

        if (TCP2_NET_STA(S_K) != END && TCP2_NSTAT(D_K,Pk,S_K) == END) {                                          /* 장운영 정보 Check */
            Log_Out ();
        }

        if (TCP2_NET_STA(S_K) == END || TCP2_NET_STA(S_K) == JOB_STOP) {                                               /* 종료/중지    */
            if (LogOnFlag == ON && TCP2_NET_STA(S_K) == END) {
                Device_Close ();
            }

            continue;
        }
        else                                            /* 정상주문시간 */
        {
/* -------------------------------------------------------------------- */
/* MAIN, BACKUP, S_K 사용예                                            */
/* MAIN, BACKUP은 Memory상에 1개 이상의 복수건 정의시(Socket, Netstat등)*/
/* 메모리의 번지수에 정의된 값이 상이함. 이에 Process가 복수건의 Socket */
/* 처리를 하는 Logic에서 사용.(Tcp2.ini 참조)                          */
/* S_K는 MAIN, BACKUP의 정의된 값중 현재 사용되는 Socket의 대상을 지정  */
/* - 주목적:                                                           */
/*  복수건의 Socket등을 사용시 사용 Socket을 편리하게 활용하기 위한 값  */
/* - 사용차이:                                                          */
/*  MAIN, BACKUP: Memory에 Status 변경시 사용 (Netstat, Tcp2_stat등)   */
/*  S_K: 참조하여 사용만 함                                             */
/* -------------------------------------------------------------------- */
/* LogOnFlag: TCP Connect & LOGON check Flag                            */
/*            Init_Parameter => OFF.                                    */
/*            Device_Open    => ON.                                     */
/*            Device_Close   => OFF.                                    */
/* -------------------------------------------------------------------- */
/* OpenFlag: 업무개시 Flag, DATA 전송업무 가능 상태                     */
/*             Init_Parameter => OFF.                                   */
/*             Socket_Event_rtn => ON.                                  */
/*             Device_Close => OFF.                                     */
/* -------------------------------------------------------------------- */
/* DeviceSendFlag: DATA 전송 check Flag                   */
/*             Init_Parameter => OFF.                               */
/*                 Socket_Event_Rtn => OFF.                             */
/*                 Time_Out_Rtn => Device_Write()후 ON(TR_POLL)          */
/*                 Data_Event_Rtn => ON.                                */
/*                 Make_Send_Msg (TR_DATA) => Device_Write()후 ON.      */
/* 2025 : Data_Event_Rtn & Make_Send_Msg 싱크시에만 유효, 어싱크는 유요하지 않음 */
/* -------------------------------------------------------------------- */

    /* 개시를 해야만 주문을 낼 수 있음, 정규장 열리기 전에는 개시응답을 정상으로 안준다. 개시만 retry */
    if (OpenFlag == OFF)            // 개시가 안된경우
    {
         if (LogOnFlag == OFF)       // 로그온이 안된경우 로그온 시도
            Device_Open (TR_LOON);

        if (LogOnFlag == OFF)       // 아직 로그온이 안된경우
        {
            ConnectRetryCnt ++;
            if (ConnectRetryCnt >= 3) {
                Line_Change ();
                ConnectRetryCnt = 0;
            }

            PollCnt = 1;
            TimeOut = DEVICE_TIME;          // 3초
        }
        else                                // 로그온은 된경우
        {
            Device_Open (TR_LINK);

            if (OpenFlag == OFF)            // 아직 로그온이 안된경우
            {
                PollCnt = 1;
                TimeOut = DEVICE_TIME;      // 3초
            }
            else {
                PollCnt = 3;
                TimeOut = MAIN_TIME;        // 5초
            }
        }
    }
    else                        //개시가 된 경우
    {
            if (DeviceSendFlag == ON)       // DATA를 송신한경우, LogOut , 응답받고 종료처리하자. DeviceSendFlag은 LogOut만 ON
            {
            	PollCnt = 2;
                TimeOut = DATA_TIME;        // 15초
            }
            else                    // DATA를 송신한경우, 주문/HeartBeat/KillSwitch , Async라 송수신 가능하게
            {
                PollCnt = 3;
                TimeOut = MAIN_TIME;        // 5초

                if (WR_CNT > RD_CNT)        // HeartBeat/KillSwitch를 보냈더라도 주문이 있으면 먼저 처리하자.
                {
                    Data_Event_Rtn ();
                    continue;
                }
            }
        }
    }

        rt = poll (Poll, PollCnt, TimeOut);
        if (rt < 0) {
            if (SYS_NO == EINTR)
                Log (SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
            else
                Log (SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);

            continue;
        }
        else if (rt == 0) {
            Time_Out_Rtn ();
            continue;
        }

        i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);
        if (i == -1) return;
        if (i == -2) continue;

        switch (i) {
            case    FIFO_EVENT:
                Fifo_Event_Rtn ();
                break;
            case    SOCKET_EVENT:
                Socket_Event_Rtn ();
                break;
            case    DATA_EVENT:
                Data_Event_Rtn ();
                break;
            default:
                Log (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                Exit_Process ();
                break;
        }
    } /* while end */

    Device_Close ();

    return;
}   /* End of PA_1100_TS () */

/*************************************************************************
    Function        : . Init_Parameters
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . All Program Parameters Init
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Init_Parameters (void)
/*----------------------------------------------------------------------*/
{
    DeviceSendFlag = OFF;
    LogOnFlag = OFF;
    OpenFlag = OFF;
    ErrCd = 0;

    ConnectRetryCnt = 0;

    S_K = TCP2_LINE_GU;

    sprintf (IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
        TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    Log (TCP_OK, "IP[%s] port[%d]", IpAddr, PORT_NO);

    TCP2_PROC_ST1(MAIN) = ON;
    TCP2_PROC_ST1(BACKUP) = ON;

    TCP2_LINE_ST1(MAIN) = OFF;
    TCP2_LINE_ST1(BACKUP) = OFF;

    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;
    Poll[2].fd = INPUT_FD;
    Poll[2].events = POLLIN;

    if (PROC(D_K,P_K).data)
        SHM_DATA = (DATA_FORMAT *)PROC(D_K,P_K).data;

    /* 체결Process01 process key 구함 */
    for (Pk = 0; Pk < DAEMON(D_K).p_count; Pk ++) {
        if (memcmp (PROC(D_K,Pk).process_id, CHK_PROC, 10) == 0)
            break;

        if (Pk == DAEMON(D_K).p_count - 1) {
            Log (USR_FATAL, "unregistered process[%s]", CHK_PROC);
            Exit_Process ();
        }
    }

    if (DELAY_TIME == 0)
        DELAY_TIME = RESP_GAP;

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
void    Socket_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
    int     rval, rt, cnt;
    char    t_time[12];

    rval = Device_Read ();

    if (rval < 0)
        return;

/* ********************************************************** */
/*
    1.   LogOn      SCHLIR00000 : 87  ( 5) = 82 +4+1
    2.   LIOK(주문) SCHOPR00000 : 108 (26) = 82 +4+11+11
    3.   LIOK(체결) SCHOPR10000 : 198 (27) = 82 +4+2+11*10
    4.   HeartBeat  SCHHER00000 : 86  ( 4) = 82 +4 (송수신 동일)
    5.   LogOut     SCHLOQ00000 : 86  ( 4) = 82 +4
    6.   주문거부   TCHODR00000 : 97  (15) = 82 +4+11
    6-1. 회원처리호가/체결

    KRX_SESSION_FMT             S_Fmt    : 82 + Data(41)
    KRX_NOTE_ALL_JUMUN_Q_FMT    J_Q_Fmt  : 82 + 300
    KRX_NOTE_JUMUN_R_FMT        J_R_Fmt  : 82 + 4+11

    - 주문수신
    1. LogOn응답,         위에서
    2. LIOK(LINK응답),        위에서
    4. HeartBeat응답,     여기서
    6. 주문응답(거부),        여기서

    - 체결수신
    1.   LogOn응답
    3.   LIOK(LINK응답)
    5.   LogOut요청
    6-1. 회원처리호가/체결
*/
/* ********************************************************** */
    DeviceSendFlag = OFF;

    memset (&S_Fmt, 0, sizeof (KRX_SESSION_FMT));
    memcpy (&S_Fmt, DataBuff, RecvLen);

    ReTrCode = Analyze_Data ();

    /* ************************************ */
    /* 여기서는 RP_DATA, RP_POLL 2개만 체크 */
    /* ************************************ */
    switch (ReTrCode) {
        case    RP_DATA:    /* 주문응답은 "0020"(TPS허용수량초과)빼고, 모두 거부 */
            if (memcmp(S_Fmt.Data, "0020", 4) == 0) {       // 0020 (TPS허용수량초과)
                Log (USR_OK, "TCS초과 오류 수신 1초 sleep!!!");
                sleep (1);
                break;
            }

            if (ErrCd != 0) Err_Msg();

            /* Seq 처리 후 종료 */
            if (FirstSeq > INT_SEQ) {
                TCP2_LINE_ST = OpenFlag = OFF;

                Log (USR_ERROR, "RP_DATA recv:invalid KRX SEQ <%d:%d>",
                    FirstSeq, INT_SEQ);

                sleep (3);                          // 3초후 종료
                Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
            }
            else {
                Log (USR_WARN, "RP_DATA recv:Seq Change [%d] => [%d] KRX SEQ <%d>",
                    INT_SEQ, FirstSeq, FirstSeq);

                /* DATA오류를 받으면 해당 "Seq -1"까지 처리된 것이다 */
                /* Seq BackUp
                back_rd_cnt  = RD_CNT;
                back_int_seq = INT_SEQ;
                */

                RD_CNT = INT_SEQ = FirstSeq -1;                 // 오류 Seq -1로 변경, 그래야 Next로 오류건을 처리할 수 있음
                PROC(D_K,P_K).counter_seq = FirstSeq -1;        // 오류 Seq -1로 변경, 그래야 Next로 오류건을 처리할 수 있음,DR

                /* 나머지는 오류 처리 */
                Write_Response_Data (1);

                /* Seq 원복
                RD_CNT  = back_rd_cnt;
                INT_SEQ = back_int_seq;
                */

                Device_Close ();                            // 소켓 종료
            }

            break;
        case    RP_POLL:
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
void    Data_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
    int     rt;
    char    tmp[128];

    ErrCd = 0;
    memset (DataBuff, 0, sizeof (DataBuff));

    rt = Make_Send_Msg (TR_DATA);
    if (rt == 0)            // 정상
    {
        memcpy (DataBuff, &J_Q_Fmt, SendLen);
        Device_Write ();

        Dshm_Add_Count (PS_R_1, 1);

        if (LogOnFlag == ON)
            Set_TR_Time ();
    }

    while (1) {
        rt = read (INPUT_FD, tmp, sizeof(tmp));

        if (rt == 0)
            break;
        else if (rt == -1) {
            if (SYS_NO == EINTR)
                continue;
            Log (FIF_ERROR, "Poll:cannot read FIFO {%d:%s}",
                SYS_NO, SYS_STR);
            break;
        }
    }

    return;
}   /* End of Data_Event_Rtn () */

/*************************************************************************
    Function        : .  Device_Open
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . Svm Line Status Set & TCPIP Poll fd set & LOGON
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Open (int tr_code)
/*----------------------------------------------------------------------*/
{
    int     rt, rval;

    if (tr_code == TR_LOON) {
        Device_Open_Logon (0, 0);
    }
    else if (tr_code == TR_LINK) {
        while (START_S != END) {
            rt = Make_Send_Msg (TR_LINK);
            memset (DataBuff, 0, sizeof (DataBuff));
            memcpy (DataBuff, &S_Fmt, SendLen);
            Device_Write ();
            Log (USR_OK, "send LINK request");

            rval = Device_Read ();
            if (rval < 0 ||
                memcmp(((KRX_HEADER *)DataBuff)->MsgType, "SCHOPR00000", 11) != 0) {
                Log (USR_ERROR, "LINK response recv error");
                close (Sockfd);
                break;
            }

            memset (&S_Fmt, 0, sizeof (KRX_SESSION_FMT));
            memcpy (&S_Fmt, DataBuff, RecvLen);

            FirstSeq = AtoIf (S_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));

            if (memcmp(S_Fmt.Data, RESP_SUCCESS, KRX_ERRCODE_LEN) == 0) {
                /* Seq 처리 */
                if (FirstSeq > INT_SEQ) {
                    TCP2_LINE_ST = OpenFlag = OFF;

                    Log (USR_ERROR, "TR_LINK recv:invalid KRX SEQ <%d:%d>",
                        FirstSeq, INT_SEQ);

                    sleep (3);                          // 3초후 종료
                    Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
                }
                else if (FirstSeq < INT_SEQ) {
                    Log (USR_WARN, "TR_LINK recv:Seq Change [%d] => [%d] KRX SEQ <%d>",
                        INT_SEQ, FirstSeq, FirstSeq);

                    RD_CNT = INT_SEQ = FirstSeq;                // 수신측 Seq로 변경
                    PROC(D_K,P_K).counter_seq = FirstSeq;       // 수신측 Seq로 변경
                }

                LOAD_CNT = 0;
                TCP2_NET_STA(S_K) = ON;
                TCP2_LINE_ST = OpenFlag = ON;
                ConnectRetryCnt = 0;

                Log (TCP_OK, "LINK OK");

                break;
            }
            else {
                if (memcmp(S_Fmt.Data, "0101", 4) == 0)                     // 호가접수 개시전
                {
                    Log (USR_OK, "0100 호가접수전 개시요청 3초후 Retry!!!");
                    sleep (3);                      // 3초 쉬었다가 재개시 시도

                    break;
                }

                /* 0101외 다른 오류는 Process를 죽여서 운영자에게 알려야함 */
                Log (USR_ERROR, "TR_LINK recv:invalid ResponseCode[%4.4s] KRX SEQ <%d:%d>",
                    S_Fmt.Data, FirstSeq, INT_SEQ);

                sleep (3);                          // 3초후 종료
                Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
            }
        }   // End Of While
    }

    return;
}   /* End of Device_Open ()    */

/*----------------------------------------------------------------------*/
void    Device_Close (void)
/*----------------------------------------------------------------------*/
{
    Device_Close_Base ();
}   /* End of Device_Close ()   */

/*************************************************************************
    Function        : .  Time_Out_Rtn
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . Timeout Control
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Time_Out_Rtn (void)
/*----------------------------------------------------------------------*/
{
    memset (DataBuff, 0, sizeof (DataBuff));

    switch (PollCnt) {
        case    2:
            Time_Out_Disconnect ("KRX");
            break;
        case    3:
            Make_Send_Msg (TR_POLL);
            memcpy (DataBuff, &S_Fmt, SendLen);
            Device_Write ();
            break;
        default:
            break;
    }
}   /* End of Time_Out_Rtn ()   */

/*************************************************************************
    Function        : . Analyze_Data
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int TR_TYPE
    Comment         : . TCP Recv Data Analysis And TR_CODE Return
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Analyze_Data (void)
/*----------------------------------------------------------------------*/
{
/*
    1. LogOn      SCHLIR00000 : 87  ( 5) = 82 +4+1
    2. LIOK(주문) SCHOPR00000 : 108 (26) = 82 +4+11+11
    3. LIOK(체결) SCHOPR10000 : 198 (27) = 82 +4+2+11*10
    4. HeartBeat  SCHHER00000 : 86  ( 4) = 82 +4 (송수신 동일)
    5. LogOut     SCHLOQ00000 : 86  ( 4) = 82 +4
    6. 주문거부   TCHODR00000 : 97  (15) = 82 +4+11

    KRX_SESSION_FMT             S_Fmt    : 82 + Data(41)
    KRX_NOTE_ALL_JUMUN_Q_FMT    J_Q_Fmt  : 82 + 300
    KRX_NOTE_JUMUN_R_FMT        J_R_Fmt  : 82 + 4+11
*/

    FirstSeq = AtoIf (S_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));

/*
    if (memcmp(S_Fmt.Header.MsgType, "SCHLIR00000", 11) == 0)           // LogOn
    {
        Log (USR_OK, "LogOn응답(SCHLIR00000): [%4.4s] <%d:%d:%d>",
            S_Fmt.Data, FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_LOON);
    }
    else if (memcmp(S_Fmt.Header.MsgType, "SCHOPR00000", 11) == 0)      // LIOK
    {
        Log (USR_OK, "업무개시응답(SCHOPR00000): [%4.4s] <%d:%d:%d>",
            S_Fmt.Data, FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_LINK);
    }
    else if (memcmp(S_Fmt.Header.MsgType, "SCHHER00000", 11) == 0)      // HeartBeat
*/
    if (memcmp(S_Fmt.Header.MsgType, "SCHHER00000", 11) == 0)       // HeartBeat
    {
        Log (USR_OK, "회선시험요청(SCHHER00000): [%4.4s] <%d:%d:%d>",
            S_Fmt.Data, FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_POLL);
    }
    else if (memcmp(S_Fmt.Header.MsgType, "SCHLOR00000", 11) == 0)      // LogOut
    {
        Log (USR_OK, "LogOut요청(SCHLOR00000): [%4.4s] <%d:%d:%d>",
            S_Fmt.Data, FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_STOP);
    }
    else if (memcmp(S_Fmt.Header.MsgType, TR_SESSION_DATA, KRX_TRCODE_LEN) == 0)        // 주문Error
    {
        Log (USR_OK, "주문오류(TCHODR00000): <%d:%d:%d> 거부사유코드[%4.4s]",
            FirstSeq, INT_SEQ, LOAD_CNT, S_Fmt.Data);
        return (RP_DATA);
    }

    Log (USR_ERROR, "Analyze_Data: unknown MsgType[%.11s]", S_Fmt.Header.MsgType);
    return (RP_STOP);
}   /* End of Analyze_Data ()   */

/*************************************************************************
    Function        : . Write_Respose_Data
    Parameters IN   : . d_cnt   : write count
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . 주문거부에 대한 값 돌려주기, file write
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Write_Response_Data (int d_cnt)
/*----------------------------------------------------------------------*/
{
    int     i, rt, cnt;
    char    resp_time[12];

    if (d_cnt <= 0)
        return;

    memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT));

    /* 리턴받은 Error Data를 찾는다. 위에서 Seq는 처리해다.*/
    /* 오류코드만 받기때문에 주문전문을 알수없다. 그래서 seq로 찾아야 한다. */
    cnt = DSHM_R (PS_R_1, (void *)R_Fmt, 1);
    if (cnt < 0 || cnt > 1) {

        Log (SAM_ERROR, "DSHM_R(PS_R_1)[%s] 거부처리 못했음", IDN(D_K,P_K,0));
        return;     // 돌아가서 정리하고 죽는다.
    }
    else if (cnt == 0)
        return;     // 돌아가서 정리하고 죽는다.

    /* 주문거부는 수신받은 전문 "4+11+ 55(찾아서) + 주문전문"을 전달한다. */
    /* KRX Header 82는 제외 */
    /* Async로 1개만 처리한다 */
    for (i = 0; i < d_cnt; i ++) {
        /* Space로 초기화 */
        memset (W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);

        /* 1. Seq (8) */
        memcpy (W_Fmt[i].Seq,           &S_Fmt.Data[4],             sizeof (W_Fmt[0].Seq));         // Data의 Seq
        /* 2. If_Seq (8) */
        memcpy (W_Fmt[i].If_Seq,        S_Fmt.Header.MsgSeqNum,     sizeof (W_Fmt[0].If_Seq));      // 헤더의 Seq
        /* 3. ApType (8) */
        memcpy (W_Fmt[i].ApType,        R_Fmt[i].ApType,            sizeof (W_Fmt[0].ApType));
        /* 4. ResponseCode (4), 전달은 정상으로 한다.(0000), 오류코드는 위에 */
        memcpy (W_Fmt[i].ResponseCode,  RES_NORMAL,                 strlen (RES_NORMAL));
        /* 5. set KRX response time   (10)
        sprintf (resp_time, "%010.06f", RespMsec);
        memcpy (W_Fmt[i].RecvTime1, resp_time, sizeof (W_Fmt[i].RecvTime1));
        */
        /* 6. RecvTime2 (12) */
        memcpy (W_Fmt[i].RecvTime2,     &S_Fmt.Header.SendingTime[8],   9);                         // 12자리중 9자리만
        /* 7. DataHeader(20), Async라.. 알수없다 */
        memcpy (W_Fmt[i].DataHeader,    R_Fmt[i].DataHeader,        HEAD_SIZE);
        /* 8. DATA 조합(4+11 +300) */

        /* Write Format : 4+11 + 55(SPACE) + 주문전문 */
        /* KRX Reply (4+11) */
        memcpy (W_Fmt[i].Data,          S_Fmt.Data,                 sizeof(KRX_NOTE_JUMUN_R_DATA)); // 4+11
        /* Read Order Data (400 include tmp) */
        memcpy (&W_Fmt[i].Data[sizeof(KRX_NOTE_JUMUN_R_DATA)],      R_Fmt[i].Data,  310);           // 400중 310(0+주문번문)만 Copy
        W_Fmt[i].LineFeed[0] = '\n';

    }

    rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);      // OFN1 = pa_1201_mp
    if (rt != 1) {

        Log (SAM_FATAL, "file write fail[%s:%d]", OFN(D_K,P_K,0), rt);
        return;
    }

    Log (USR_OK, "Write!! Error Response Data[%s:%d:%d]", OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), i);

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
int     Make_Send_Msg (int tr_code)
/*----------------------------------------------------------------------*/
{
    int     rt, datacnt = 1;
    char    d_time[18];

    rt = 0;
    memset (&S_Fmt, 0, sizeof (KRX_SESSION_FMT));

    /* Make The Header */
    // 1. 전문유형
    memcpy (S_Fmt.Header.BeginString, "KMAPv2.0",   8);
    // 2. 메시지길이(추후 Set)
    ItoAf (0, S_Fmt.Header.BodyLength, sizeof (S_Fmt.Header.BodyLength));
    // 3. 메세지Type(추후 Set)
    ItoAf (0, S_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));
    // 4. 일련번호(추후 Set)
    // 5. 회원번호
    memcpy (S_Fmt.Header.SenderCompID, TCP_COMPANY, strlen (TCP_COMPANY));
    Get_DateMilliTime (d_time);
    // 6. 연계시도착 회원사 번호(SPACE)
    // 7. 회신시송신 회원사 번호(SPACE)

    // 8. 전송일시
    memcpy (S_Fmt.Header.SendingTime, d_time, sizeof (S_Fmt.Header.SendingTime));
    // 9. 데이터건수(추후 Set)
    ItoAf (0, S_Fmt.Header.DataCnt, sizeof (S_Fmt.Header.DataCnt));
    // 10. 암호화 유무
    memcpy (S_Fmt.Header.Encrypt,       "N",        1);     // 초기는 N, 주문만 Y
    SendLen = KRX_HEAD_LEN;

    switch (tr_code) {
        case    TR_LOON:    /* 로그온 요청 */
            memcpy (S_Fmt.Header.MsgType,   "SCHLIQ00000",  11);
            ItoAf (INT_SEQ, S_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));
            memset (S_Fmt.Data, 0x20, 41);              // Logon size
            memcpy (S_Fmt.Data,         LOGON_ID(D_K,P_K), 10);
            memcpy (&S_Fmt.Data[10],    LOGON_PW(D_K,P_K), 30);
            memcpy (&S_Fmt.Data[40],    "Y",                1);
            MsgLen  = strlen(S_Fmt.Data);
            ItoAf (MsgLen, S_Fmt.Header.BodyLength, sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_LINK:    /* 업무개시 요청 */
            memcpy (S_Fmt.Header.MsgType,   "SCHOPQ00000",  11);
            ItoAf (INT_SEQ, S_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));
            memcpy (S_Fmt.Data, "0000", 4);
            memcpy (&S_Fmt.Data[4], "          ", 11);
            ItoAf (INT_SEQ, &S_Fmt.Data[15],    11);
            MsgLen  = strlen(S_Fmt.Data);
            ItoAf (MsgLen, S_Fmt.Header.BodyLength, sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_POLL:    /* 회선시험 요청 */
            memcpy (S_Fmt.Header.MsgType,       "SCHHEQ00000",  11);
            ItoAf (INT_SEQ, S_Fmt.Header.MsgSeqNum,     sizeof (S_Fmt.Header.MsgSeqNum));
            memcpy (S_Fmt.Data, "0000", 4);
            MsgLen  = strlen(S_Fmt.Data);
            ItoAf (MsgLen,  S_Fmt.Header.BodyLength,    sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_LOOU:    /* 로그아웃 요청 */
            memcpy (S_Fmt.Header.MsgType,       "SCHLOQ00000",  11);
            ItoAf (INT_SEQ, S_Fmt.Header.MsgSeqNum,     sizeof (S_Fmt.Header.MsgSeqNum));
            MsgLen  = 0;
            ItoAf (MsgLen,  S_Fmt.Header.BodyLength,    sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_DATA:    /* 주문요청 + Kill Swith */
            memcpy (S_Fmt.Header.Encrypt,       "Y",            1);     // 초기는 N, 주문만 Y
            memset (&J_Q_Fmt,           0,              sizeof (KRX_JUMUN_Q_FMT));
            memcpy (&J_Q_Fmt.Header,    &S_Fmt.Header,  KRX_HEAD_LEN);

            memcpy (J_Q_Fmt.Header.MsgType,     TR_SESSION_DATA,    KRX_TRCODE_LEN);
            ItoAf (INT_SEQ+1, J_Q_Fmt.Header.MsgSeqNum, sizeof (J_Q_Fmt.Header.MsgSeqNum));

            MsgLen = Make_Data_Block ();
            if (MsgLen <= 0) {
                rt = -1;
                break;
            }

            // body size ( 주문전문길이, 헤더(82)배제 )
            ItoAf (MsgLen,  J_Q_Fmt.Header.BodyLength,  sizeof(J_Q_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            ItoAf (datacnt, J_Q_Fmt.Header.DataCnt,     sizeof (J_Q_Fmt.Header.DataCnt));
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
                        정상: KRX send record count (1 ~ MAX_CNT)
                        0: no data
    Comment         : . 1) If Memory Data Exist -> Memory Data Send
                        2) Data File Read & Time Out Check
                            Time Out Check -> PS_EW  Write -> PS_R_1 Seq Add
                            KRX Send Data  -> Seq Set & Memory Load
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Make_Data_Block (void)
/*----------------------------------------------------------------------*/
{
    int     r_cnt, t_cnt, i, j, rt, Size_Len, ow_seq;
    char    err_cd[10], tmp[128], w_buf[FILE_BUF_LEN];

    i = t_cnt = Size_Len = 0;
    memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);
    memset (W_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);

    /* 송신 파일에는 KRX Header + 주문전문으로 받는다 */
    r_cnt = DSHM_R (PS_R_1, (void *)R_Fmt, MAX_CNT);
    if (r_cnt < 0 || r_cnt > MAX_CNT) {
        Log (SAM_FATAL, "DSHM_R(PS_R_1)[%s]", IDN(D_K,P_K,0));
        Exit_Process ();
    }
    else if (r_cnt == 0) {
        return (0);
    }

    /* 호가정합성, 착오매매, 회원사내부룰에 의한 주문정합성 체크로직(협의필요) */
    rt = Chk_Risk_All (R_Fmt[i].Data);
    if (rt < 0) {
        /* Write Format : 4+11 + 55(SPACE) + 주문전문 */
        memset (w_buf, 0x20, sizeof (w_buf));
        memcpy (w_buf, R_Fmt[0].Seq, sizeof(BUFF_RW_HEAD));

        memset (err_cd, 0, sizeof(err_cd));
        if (abs(rt) > 9999)
            sprintf (err_cd, "%04d", abs(rt)/10);
        else
            sprintf (err_cd, "%04d", abs(rt));

        Log (USR_WARN, "정합성오류 err_cd[%4.4s]", err_cd);
// BUFF_RW_HEAD      : file buffer header (50 + 20 = 70 bytes)
// SEARCH_HEADER_LEN : sizeof (SEARCH_HEADER)) = 50 bytes

        memcpy (&w_buf[sizeof(BUFF_RW_HEAD)],       "REJE00000000000",      15);
        memcpy (&w_buf[sizeof(BUFF_RW_HEAD)+15], R_Fmt[0].Data, DATA_SIZE-15);      // 주문값 돌려주기
        w_buf[sizeof (BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

        /* 오류전달 */
        rt = F_W (TS_W1_1, w_buf, 1);
        if (rt != 1) {
            Log (SAM_FATAL, "file write[%s] rt[%d]", w_buf, rt);
            return (-1);
        }
        Log (USR_OK, "Err F_W Done. File write OK[%s][%d]", w_buf, strlen(w_buf));

        // 아래 for문을 처리하지 못하게 한다.
        r_cnt = 0;
        Dshm_Add_Count (PS_R_1, 1);     // 정상송신시에는 송신후 카운드증가
    }

    /* 주문거부는 수신받은 전문 "4+11+ 55(찾아서) + 주문전문"을 전달한다. */
    /* KRX Header 82는 제외 */
    for (i = 0; i < r_cnt; i ++) {
        /* 수신받은 데이터 포맷은 "55Byte + 주문전문" */
        if ( (memcmp (&R_Fmt[i].Data[11], "TCHODR40001", 11) == 0)  ||  /* 채권일반호가(254) */
             (memcmp (&R_Fmt[i].Data[11], "TCHODR40002", 11) == 0)  ||
             (memcmp (&R_Fmt[i].Data[11], "TCHODR40003", 11) == 0)  ) {
            Size_Len    = sizeof (KRX_NOTE_JUMUN_DATA);                 // 254
        }
        else
        if ( (memcmp (&R_Fmt[i].Data[11], "TCHMOR40001", 11) == 0)  ||  /* 채권LP일반호가(255) */
             (memcmp (&R_Fmt[i].Data[11], "TCHMOR40002", 11) == 0)  ||
             (memcmp (&R_Fmt[i].Data[11], "TCHMOR40003", 11) == 0)  ) {
            Size_Len    = sizeof (KRX_LP_NOTE_JUMUN_DATA);              // 255
        }
        else
        if (memcmp (&R_Fmt[i].Data[11], "TCHKOR10001", 11) == 0)        /* Kill Switch , KTS만 가능 (147) */
        {
            Size_Len    = 147;
        }
        else {
            Log (USR_ERROR, "Undefined DataType Was Read [%s][%11.11s]", R_Fmt[i].Data, R_Fmt[i].Data);
            sleep (3);
            Exit_Process ();
        }

        LOAD_CNT ++;
        SHM_DATA[i].seq = INT_SEQ + LOAD_CNT;
        memcpy (SHM_DATA[i].buf.Seq, R_Fmt[i].Seq,              sizeof (FILE_BUFF_FORMAT));
        ItoAf (SHM_DATA[i].seq, J_Q_Fmt.JumunData[i].DataSeq,   sizeof (J_Q_Fmt.JumunData[0].DataSeq));

        memcpy (J_Q_Fmt.JumunData[i].Data,  &R_Fmt[i].Data[sizeof(J_Q_Fmt.JumunData[0].DataSeq)],   // 0+11
                                                                Size_Len - 11);
    }

    return (Size_Len);
}   /* End of Make_Data_Block ()    */

/*************************************************************************
    Function        : . Chk_Risk_All
    Parameters IN   : . p_buf   : received data
    Parameters OUT  : .
    Return Code     : . int
    Comment         : . set sise data SHM (±aº≫/E￡°¡/A¼°a)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Chk_Risk_All (char *p_buf)
/*----------------------------------------------------------------------*/
{
    return 0;
}

/*************************************************************************
    Function        : . Log_Out
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Log_Out (void)
/*----------------------------------------------------------------------*/
{
    Log_Out_Base ();
}   /* End of Log_Out ()    */

/*************************************************************************
    End of Program (pa_1100_ts.c)
*************************************************************************/
