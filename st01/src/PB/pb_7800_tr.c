#define     _GLOBAL
/*------------------------------------------------------------------------
#   System  : Connect to KRX
#   Author  : PSH
#   Module  : 일괄송신
#   File    : pb_7800_tr.c
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

/* ********************************************** */
/*
    일괄송신(KRX직결) 가 2직결 기록하고 모듈명 직결.
    TRDESP50101 : 채기본마스터 (1,173 byte), 7801
    TRDESP50102 : KTS업무헤더  (  350 byte),    7802
*/
/* ********************************************** */

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     TCP_TIME_OUT    35

#define     DEVICE_TIME     3  * 1000                       /*  3 sec   */
#define     MAIN_TIME       35 * 1000       /* Heart Beat 직결 30 sec   */
#define     FOREVER_TIME    60 * 1000                       /* 60 sec   */

#define     FIFO_EVENT      0
#define     SOCKET_EVENT    1

#define     MAX_CNT         7               // RDS(일괄송신)가 전략처리를.

#define     NO_TIME         "0600"          // RDS(일괄송신)가 중간가ġ미국 모듈명 �ǹ̾가�

#define     PORT_NO         TCP2_PORT_NO

/* 암복호화 추가 */
#ifndef NO_INISAFE
char        KRX_INITECH_CONF_PATH[256];

// 직결 직결 필요
net_ctx         *EnCtx = NULL;

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
int     ConnectRetryCnt, D_End;
int     Sockfd, ErrCd, SendLen, MsgLen, RecvLen, ReTrCode;
int     PollCnt, FirstSeq, TimeOut, Pk;
int     back_int_seq, back_rd_cnt;
double  RTime = 0, STime = 0;
double  SendMsec, RecvMsec, RespMsec;
char    DeviceSendFlag, LogOnFlag, OpenFlag;
char    DataBuff[KRX_DATA_BUFF_SIZE], IpAddr[20], ApType[10];
char    NoTime[] = NO_TIME;

KRX_HEADER                  Header_Fmt;  // KRX Header (82)
KRX_JUMUN_R_FMT             Reply;  // 82+ 4+11 직결
FILE_DATA_HEAD              File_Data_Head;  // 20
KRX_R_SESSION_FMT           KR_Fmt;  // KRX 직결 직결 (82+116)
struct pollfd       Poll[2];
void    *FmtPtr = (void *)&KR_Fmt;

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PB_7800_TR(void);
void    Init_Parameters(void);
void    Socket_Event_Rtn(void);
void    Device_Open(int);
void    Time_Out_Rtn(void);
int     Analyze_Data(void);
void    Write_Data(int, int);
int     Make_Send_Msg(int);
void    Log_Out(void);
void    Check999(int);

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    Init_Proc(argc, argv);
    PB_7800_TR();
    Exit_Process();
}   /* End of main ()   */

/*----------------------------------------------------------------------*/
void    PB_7800_TR(void)
/*----------------------------------------------------------------------*/
{
    int     rt, i;

    Init_Parameters();

    while (START_S != END) {
        Stat_Save();

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
                    PollCnt = 2;
                    TimeOut = MAIN_TIME;  // 35가
                }
            }
        }
        else {  // 장시간 가 가�
            PollCnt = 2;
            TimeOut = MAIN_TIME;  // 35가
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
}   /* End of PB_7800_TR () */

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
    D_End = ErrCd = 0;
    ConnectRetryCnt = 0;

    if (TIME_OUT == 0)
        TIME_OUT = TCP_TIME_OUT;
    Log(USR_OK, "TIME_OUT[%d]", TIME_OUT);

    S_K = TCP2_LINE_GU;

    sprintf(IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
            TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    Log(TCP_OK, "IP[%s] port[%d]", IpAddr, PORT_NO);

    TCP2_PROC_ST1(MAIN) = ON;
    TCP2_PROC_ST1(BACKUP) = ON;

    TCP2_LINE_ST1(MAIN) = OFF;
    TCP2_LINE_ST1(BACKUP) = OFF;

    sprintf(ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));

    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;

    /* Log */
    Log(USR_OK, "SHM t_seq [%d]",         PROC(D_K,P_K).tr_seq);
    Log(USR_OK, "SHM curr_tr[%11.11s]",   PROC(D_K,P_K).curr_tr);

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
    int     rval, rt, cnt, r_meg_no, r_meg_seq;
    char    t_time[12];

    rval = Device_Read();

    if (rval < 0)
        return;

    DeviceSendFlag = OFF;
    ErrCd = 0;

    memset(&Header_Fmt, 0, KRX_HEAD_LEN);
    memcpy(&Header_Fmt, DataBuff, KRX_HEAD_LEN);

    ReTrCode = Analyze_Data();

    rt = 0;
    /* ************************************ */
    /* 가�⼭가 RP_DATA, RP_POLL 2직결 체크 */
    /* ************************************ */
    switch (ReTrCode) {
        case    TR_DATA:
            Log(USR_OK, "TR_DATA OK");

            /* 99999999 */
            if (memcmp(Header_Fmt.MsgType, TR_RDS01_BOND_ITEM, KRX_TRCODE_LEN) == 0) {  // Data, TRDESP50101 (RDS01)
                Log(USR_OK, "TR_DATA TRDESP50101 99999999999 OK");
#ifndef NO_INISAFE
            if (IS_ENCRYPTED((KRX_HEADER *)DataBuff))
                Write_Data(1, 1);  // pb_7801_dd(1400)
            else
#endif
                Write_Data(1, 0);  // pb_7801_dd(1400)
            Log(USR_OK, "DATA Reply OK 01!! DataBuff[%110.110s]", DataBuff);
    }
    else if (memcmp(Header_Fmt.MsgType, TR_RDS02_KTS_ITEM, KRX_TRCODE_LEN) == 0) {  // Data, TRDESP50102 (RDS02)
        Log(USR_OK, "TR_DATA TRDESP50102 99999999999 OK");
#ifndef NO_INISAFE
    if (IS_ENCRYPTED((KRX_HEADER *)DataBuff))
        Write_Data(2, 1);  // pb_7802_dd(350)
    else
#endif
        Write_Data(2, 0);  // pb_7802_dd(350)
    Log(USR_OK, "DATA Reply OK 02!! DataBuff[%110.110s]", DataBuff);
}
else
    Log(USR_OK, "DATA Reply OK 03 DataBuff[%110.110s]", DataBuff);

rt = Make_Send_Msg(RP_DATA);  // TR_DATA,RP_DATA,RP_POLL,RP_STOP
memset(DataBuff, 0, sizeof (DataBuff));
memcpy(DataBuff, &KR_Fmt, SendLen);
Device_Write();

break;

case    RP_DATA:
    /* Seq Check */
    /* 가� Data가 +1가 수신받는다, 아니면 로그인� 직결 */
    if (FirstSeq >= 9999999) {
        Log(USR_OK, "FirstSeq >= 9999999");
        break;
}

if (INT_SEQ+1 != FirstSeq) {
    TCP2_LINE_ST = OpenFlag = OFF;

    Log(USR_ERROR, "PR_DATA recv:Header invalid KRX SEQ [%d] INT_SEQ[%d]",
            FirstSeq, INT_SEQ);

    sleep(3);  // 3직결 직결
    Device_Close();
    Exit_Process();  // 직결, �뒤에가 알리기 통해서
}

{           /* TR 직결 */
    KRX_MSG_COMMON *msg = (KRX_MSG_COMMON *)DataBuff;
#ifndef NO_INISAFE
    if (IS_TR(msg, TR_RDS01_BOND_ITEM)) {  // RDS01 (채기본마스터)
        if (IS_ENCRYPTED((KRX_HEADER *)DataBuff))
        Write_Data(1, 1);  // pb_7801_dd(1400)
    else
        Write_Data(1, 0);  // pb_7801_dd(1400)
}
else
    if (IS_TR(msg, TR_RDS02_KTS_ITEM)) {  // RDS02 (KTS업무헤더)
    if (IS_ENCRYPTED((KRX_HEADER *)DataBuff))
    Write_Data(2, 1);  // pb_7802_dd(350)
else
    Write_Data(2, 0);  // pb_7802_dd(350)
}
else {
    if (IS_ENCRYPTED((KRX_HEADER *)DataBuff))
        Check999(1);
    else
        Check999(0);
    // Skip Data가 SEQ가 기동시켜서 Skip가Ų가.
    memcpy(PROC(D_K,P_K).curr_tr,  Header_Fmt.MsgType,     11);
    Log(USR_OK, "Data Skip tr_seq[%d], TR[%11.11s]", PROC(D_K,P_K).tr_seq, PROC(D_K,P_K).curr_tr);
}
#else
    if (IS_TR(msg, TR_RDS01_BOND_ITEM))       // RDS01
        Write_Data(1, 0);
    else if (IS_TR(msg, TR_RDS02_KTS_ITEM))   // RDS02
        Write_Data(2, 0);
    else {
        Check999(0);
        memcpy(PROC(D_K,P_K).curr_tr,  Header_Fmt.MsgType,     11);
        Log(USR_OK, "Data Skip tr_seq[%d], TR[%11.11s]", PROC(D_K,P_K).tr_seq, PROC(D_K,P_K).curr_tr);
    }
#endif

rt = Make_Send_Msg(RP_DATA);  // TR_DATA,RP_DATA,RP_POLL,RP_STOP
memset(DataBuff, 0, sizeof (DataBuff));
memcpy(DataBuff, &KR_Fmt, SendLen);
Device_Write();
Log(USR_OK, "DATA Reply OK!!");
}   /* end of KRX_MSG_COMMON *msg scope */

break;

case    RP_POLL:
    rt = Make_Send_Msg(TR_POLL);
    memset(DataBuff, 0, sizeof (DataBuff));
    memcpy(DataBuff, &KR_Fmt, SendLen);
    Device_Write();

    break;

case    RP_LOON:
    if (!IS_RESP_OK(DataBuff)) {
        Log(USR_ERROR, "LOGON Error [%4.4s]", &DataBuff[KRX_HEAD_LEN]);
        sleep(3);  // 3직결 직결
        Device_Close();
        Exit_Process();  // 직결, �뒤에가 알리기 통해서
}

break;

case    RP_STOP:
    Log(USR_OK, "LOGOUT OK Done");

    sleep(3);  // 3직결 직결
    Device_Close();

    PROC(D_K,P_K).start_status = JOB_END;
    Exit_Process();  // 직결, �뒤에가 알리기 통해서
    break;

default:
    break;
}

if (D_End == 1) {
    Log(USR_OK, "TEST OK01");
    //      INT_SEQ = 0;            // 받치기 TR직결 처리�ϴµ� TR가 직결(999..9)가 Seq가 초기화한다.
    D_End = 0;
    Log(USR_OK, "TEST OK02");
}

return;
}   /* End of Socket_Event_Rtn ()   */

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
        Device_Open_Logon(1, 1);
    }
    else if (tr_code == TR_LINK) {
        while (START_S != END) {
            rt = Make_Send_Msg(TR_LINK);
            memset(DataBuff, 0, sizeof (DataBuff));
            memcpy(DataBuff, &KR_Fmt, SendLen);
            Device_Write();
            Log(USR_OK, "send LINK request");

            rval = Device_Read();
            if (rval < 0 ||
                    memcmp(((KRX_HEADER *)DataBuff)->MsgType, "SCHOPR00000", 11) != 0) {
                Log(USR_ERROR, "LINK response recv error");
                close(Sockfd);
                break;
            }

            memset(&KR_Fmt, 0, sizeof (KRX_R_SESSION_FMT));
            memcpy(&KR_Fmt, DataBuff, RecvLen);

            FirstSeq = AtoIf(KR_Fmt.Header.MsgSeqNum, sizeof (KR_Fmt.Header.MsgSeqNum));

            if (memcmp(KR_Fmt.Data, RESP_SUCCESS, KRX_ERRCODE_LEN) == 0) {
                /* Seq 체크  */
                if (INT_SEQ != FirstSeq) {
                    TCP2_LINE_ST = OpenFlag = OFF;

                    Log(USR_ERROR, "PR_LINK recv:invalid KRX SEQ [%d] INT_SEQ[%d]",
                            FirstSeq, INT_SEQ);

                    sleep(3);  // 3직결 직결
                    Exit_Process();  // 직결, �뒤에가 알리기 통해서
                }

                TCP2_NET_STA(S_K) = ON;
                TCP2_LINE_ST = OpenFlag = ON;
                ConnectRetryCnt = 0;

                Log(TCP_OK, "LINK OK");

                break;
            }
            else {
                Log(USR_ERROR, "PR_LINK recv:invalid ResponseCode[%4.4s] KRX SEQ[%d] INT_SEQ[%d>",
                        KR_Fmt.Data, FirstSeq, INT_SEQ);
                Log(USR_ERROR, "PR_LINK recv:invalid ME_G[%d:%d:%d:%d:%d:%d:%d:%d:%d:%d]",
                        INT_MEG_SEQ(0), INT_MEG_SEQ(1), INT_MEG_SEQ(2),INT_MEG_SEQ(3),INT_MEG_SEQ(4),
                        INT_MEG_SEQ(5), INT_MEG_SEQ(6), INT_MEG_SEQ(7),INT_MEG_SEQ(8),INT_MEG_SEQ(9));

                sleep(3);  // 3직결 직결
                Exit_Process();  // 직결, �뒤에가 알리기 통해서
            }
        }  // End Of While
    }

    return;
}   /* End of Device_Open ()    */

/*----------------------------------------------------------------------*/
void    Device_Close(void)
/*----------------------------------------------------------------------*/
{
    Device_Close_Base();
#ifndef NO_INISAFE
    Free_All((void *)(long)1);
    INL_Cleanup(CLIENT_CTX);
#endif
}   /* End of Device_Close ()   */

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
    memset(DataBuff, 0, sizeof (DataBuff));

    switch (PollCnt) {
        case    1:
            if (TimeOut == FOREVER_TIME)
                Log(USR_OK, "poll timeout <%d>:NSTAT[%d]",
                        INT_SEQ, TCP2_NET_STA(S_K));
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
int     Analyze_Data(void)
/*----------------------------------------------------------------------*/
{
    FirstSeq = AtoIf(&Header_Fmt.MsgSeqNum[4],     7);

    /* 2025 직결 직결 */
    if (memcmp(PROC(D_K,P_K).curr_tr,    Header_Fmt.MsgType,     11) != 0) {
        INT_SEQ = 0;
        PROC(D_K,P_K).tr_seq = INT_SEQ;
        memcpy(PROC(D_K,P_K).curr_tr,    Header_Fmt.MsgType,     11);
    }

    if (memcmp(Header_Fmt.MsgType, "SCHLIR00000", 11) == 0) {  // LogOn직결
        Log(USR_OK, "LogOn직결(SCHLIR00000): [%4.4s] <%d:%d:%d>",
                &DataBuff[KRX_HEAD_LEN], FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_LOON);
    }
    else if (memcmp(Header_Fmt.MsgType, "SCHLOR00000", 11) == 0) {  // LogOut직결
        Log(USR_OK, "LogOut직결(SCHLOR00000): [%4.4s] <%d:%d:%d>",
                &DataBuff[KRX_HEAD_LEN], FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_STOP);
    }
    else if (memcmp(Header_Fmt.MsgType, "SCHHEQ00000", 11) == 0) {  // HeartBeat직결
        Log(USR_OK, "회선시험요구(SCHHER00000): [%4.4s] <%d:%d:%d>",
                &DataBuff[KRX_HEAD_LEN], FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_POLL);
    }
    else if (FirstSeq == 9999999) {  // if 직결 모듈명 가 가�Ѿ� 전략처리를 업무헤더.(직결 가�Ѿ가�)
        Log(USR_OK, "Data 99999999999 직결 INT_SEQ[%d]", INT_SEQ);
    return (TR_DATA);
}
else if ((memcmp(Header_Fmt.MsgType, TR_RDS01_BOND_ITEM, KRX_TRCODE_LEN) == 0)  ||  // Data, TRDESP50101 (RDS01)
        (memcmp(Header_Fmt.MsgType, TR_RDS02_KTS_ITEM, KRX_TRCODE_LEN) == 0)   ) {  //       TRDESP50102 (RDS02)
Log(USR_OK, "DATA직결(TRDESP5010): <%d:%d>", FirstSeq, INT_SEQ);
return (RP_DATA);
}
else {
    Log(USR_OK, "Else Case 직결(Skip Data): <%d:%d> Tr[%11.11s]", FirstSeq, INT_SEQ, Header_Fmt.MsgType);
    return (RP_DATA);
}

}   /* End of Analyze_Data ()   */

/*************************************************************************
    Function        : . Check999
    Parameters IN   : . p_flag  : write file
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . file write
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Check999(int YN)
/*----------------------------------------------------------------------*/
{
    int     rt, seq, i, for_i, d_size, d_seq, body_len;
    char    m_time[24];
    char    w_data[2048];

    memset(m_time, 0, sizeof (m_time));
    Get_MicroTime(m_time);

    for_i   =   AtoIf(Header_Fmt.DataCnt,      sizeof (Header_Fmt.DataCnt));
    body_len = AtoIf(Header_Fmt.BodyLength,    sizeof (Header_Fmt.BodyLength));

#ifndef NO_INISAFE
    if (YN) {  // 복호화("Y")
        /* ******************************************************************** */
        /* 20251013, START
         * 암복호화하는 모듈명 함수용 모듈명� 초과 직결 가�(n가이라) */
        int             result = 0;
    unsigned char   *dec_data = NULL;
    int             dec_len = 0;

    // -------------------------------------------------------
    // 1. 복호화 호가
    //    - 가�(82바이트)가 직결
    //    - body_len가 복호화된 데이터보다 직결
    // -------------------------------------------------------
    result = INL_Decrypt(EnCtx,
            (unsigned char*)&DataBuff[KRX_HEAD_LEN],  // 복호화된 직결 직결 장치
            body_len,  // 부호가 직결
            &dec_data,  // 복호화 가� 직결 (가�̺귯모듈명 할당)
            &dec_len);  // 복호화 가� 직결

    if (result != 0) {
        Log(USR_ERROR, "ERROR: INL_Decrypt failed with code %d\n", result);
        if (dec_data != NULL) INL_Free_Buf(dec_data);
        sleep(10);
        return;
    }

    // -------------------------------------------------------
    // 2. skip
    // 3. 복호화된 모듈명 가�
    // -------------------------------------------------------
    Log(USR_OK, "복호화 직결! 복호화된 직결 = [%d]", dec_len);
    Log(USR_OK, "복호화된 모듈명: [%s]", dec_data);

    /* 복호화된 DATA가 모듈명 직결 */
    if (for_i <= 0) {
        Log(USR_ERROR, "Check999: invalid DataCnt[%d]", for_i);
        INL_Free_Buf(dec_data);
        return;
    }
    d_size   = dec_len / for_i;
    Log(USR_OK, "dec_len[%d] d_size[%d] for_i[%d]", dec_len, d_size, for_i);

    /* n가 수신시 복호화된 업무헤더 직결� 가� 기본마스터 직결 구한다. 하는 테스트 준비가 인지하도록가 */
    for (i = 0; i < for_i; i++) {
        /* 20251013, END */
        /* ******************************************************************** */

        /* INT_SEQ 초기화 */
        if (for_i == 1) {

            d_seq = AtoIf(&Header_Fmt.MsgSeqNum[4], 7);  // 11자리임 가 7자리임
            Log(USR_OK, "0.Header_Fmt[%20.20s] H[%11.11s] d_seq[%d]", &Header_Fmt.MsgSeqNum[4], Header_Fmt.MsgSeqNum, d_seq);
        }
        else {
            d_seq = AtoIf(&dec_data[4+i*d_size], 7);  // 11자리임 가 7자리임
            Log(USR_OK, "1.dec_data[%20.20s] d_seq[%d]", &dec_data[4+i*d_size], d_seq);
        }
        if (d_seq == 9999999)
            D_End = 1;

        if (INT_SEQ+1 != d_seq  && D_End == 0) {  // 모듈명 �ƴϸ鼭 seq가 다른
            Log(USR_ERROR, "i[%d] 1.PR_DATA Data Seq ERROR DataSeq[%d] INT_SEQ[%d] dec_data[%20.20s] d_size[%d]", i, d_seq, INT_SEQ, &dec_data[4+i*d_size], d_size);
            // -------------------------------------------------------
            // 4. 달리 직결
            // -------------------------------------------------------
            INL_Free_Buf(dec_data);
            sleep(10);
            return;
        }

        if (D_End == 0) {
            INT_SEQ += 1;  // 9999..99가 SEQ직결 �Ƚ�Ŵ
            PROC(D_K,P_K).tr_seq = INT_SEQ;
            memcpy(PROC(D_K,P_K).curr_tr,  Header_Fmt.MsgType,     11);
            //              Log (USR_OK, "SEQ Add");
        }

        Set_TR_Time();
    }

    // -------------------------------------------------------
    // 4. 달리 직결
    // -------------------------------------------------------
    INL_Free_Buf(dec_data);
}
else
#endif
    {  // plaintext path ('N' or Not 'Y')
    if (for_i <= 0) {
    Log(USR_ERROR, "Check999: invalid DataCnt[%d]", for_i);
    return;
}
d_size = body_len / for_i;

Log(USR_OK, "가 d_size[%d] body_len[%d] for_i[%d]", d_size, body_len, for_i);

for (i = 0; i < for_i; i++) {
    /* 20251013, END */
    /* ******************************************************************** */

    /* INT_SEQ 초기화 */
    if (for_i == 1) {
        d_seq = AtoIf(&Header_Fmt.MsgSeqNum[4], 7);  // 11자리임 가 7자리임
        Log(USR_OK, "3.H[%11.11s] d_seq[%d]", Header_Fmt.MsgSeqNum, d_seq);
    }
    else {
        d_seq = AtoIf(&DataBuff[KRX_HEAD_LEN+4 + i*d_size], 7);
        Log(USR_OK, "4.DataBuff[%20.20s] d_seq[%d]", &DataBuff[KRX_HEAD_LEN+4 + i*d_size], d_seq);
    }

    if (d_seq == 9999999)
        D_End = 1;

    if (INT_SEQ+1 != d_seq  && D_End == 0) {  // 모듈명 �ƴϸ鼭 seq가 다른
        Log(USR_ERROR, "i[%d] 2.PR_DATA Data Seq ERROR DataSeq[%d] INT_SEQ[%d]", i, d_seq, INT_SEQ);
        sleep(10);
        return;
    }

    if (D_End == 0) {
        INT_SEQ += 1;  // 9999..99가 SEQ직결 �Ƚ�Ŵ
        PROC(D_K,P_K).tr_seq = INT_SEQ;
        memcpy(PROC(D_K,P_K).curr_tr,  Header_Fmt.MsgType,     11);
        //              Log (USR_OK, "SEQ Add");
    }

    Set_TR_Time();
}
}

}  // End of Check999

/*************************************************************************
    Function        : . Write_Data
    Parameters IN   : . p_flag  : write file
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . file write
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Write_Data(int p_flag, int YN)
/*----------------------------------------------------------------------*/
{
    int     rt, seq, i, for_i, d_size, d_seq, body_len;
    char    m_time[24];
    char    w_data[2048];
    BUFF_RW_HEAD    f_head;

    memset(m_time, 0, sizeof (m_time));
    Get_MicroTime(m_time);

    for_i   =   AtoIf(Header_Fmt.DataCnt,      sizeof (Header_Fmt.DataCnt));

    /* 복호화된 DATA가 모듈명 직결 */
    body_len = AtoIf(Header_Fmt.BodyLength,    sizeof (Header_Fmt.BodyLength));
    if (for_i <= 0) {
        Log(USR_ERROR, "Write_Data: invalid DataCnt[%d]", for_i);
        return;
    }
    d_size   = body_len / for_i;

#ifndef NO_INISAFE
    if (YN) {

        /* ******************************************************************** */
        /* 20251013, START
         * 암복호화하는 모듈명 함수용 모듈명� 초과 직결 가�(n가이라) */
        int             result = 0;
        unsigned char   *dec_data = NULL;
        int             dec_len = 0;

        // -------------------------------------------------------
        // 1. 복호화 호가
        //    - 가�(82바이트)가 직결
        //    - body_len가 복호화된 데이터보다 직결
        // -------------------------------------------------------
        result = INL_Decrypt(EnCtx,
                (unsigned char*)&DataBuff[KRX_HEAD_LEN],  // 복호화된 직결 직결 장치
                body_len,  // 부호가 직결
                &dec_data,  // 복호화 가� 직결 (가�̺귯모듈명 할당)
                &dec_len);  // 복호화 가� 직결

        if (result != 0) {
            Log(USR_ERROR, "ERROR: INL_Decrypt failed with code %d\n", result);
            if (dec_data != NULL) INL_Free_Buf(dec_data);
            return;
        }

        // -------------------------------------------------------
        // 2. skip
        // 3. 복호화된 모듈명 가�
        // -------------------------------------------------------
        Log(USR_OK, "복호화 직결! 복호화된 직결 = %d\n", dec_len);
        Log(USR_OK, "복호화된 모듈명: %.*s\n", dec_len, dec_data);

        /* 20251013, END */
        /* ******************************************************************** */

        d_size  =   dec_len / for_i;
        Log(USR_OK, "999.dec_len[%d] d_size[%d] for_i[%d]", dec_len, d_size, for_i);

        /* n가 수신시 복호화된 업무헤더 직결� 가� 기본마스터 직결 구한다. 하는 테스트 준비가 인지하도록가 */
        for (i = 0; i < for_i; i++) {
            /* INT_SEQ 초기화 */
            if (for_i == 1) {
                d_seq = AtoIf(&Header_Fmt.MsgSeqNum[4], 7);  // 11자리임 가 7자리임
                Log(USR_OK, "5.H[%11.11s] d_seq[%d]", Header_Fmt.MsgSeqNum, d_seq);
            }
            else {
                d_seq = AtoIf(&dec_data[4 + i*d_size], 7);  // 11자리임 가 7자리임
                Log(USR_OK, "6.dec_data[%20.20s] d_seq[%d]", &dec_data[4 + i*d_size], d_seq);
            }

            if ((i == for_i-1)  &&  // 모듈명 DATA 인데 "99999999999"가 INT_SEQ가 초기화한다.(& 직결�⸶후처리)
                    (d_seq == 9999999))
            D_End = 1;

            if (INT_SEQ+1 != d_seq  && D_End == 0) {  // 모듈명 �ƴϸ鼭 seq가 다른
                Log(USR_ERROR, "1.PR_DATA Data Seq ERROR DataSeq[%d] INT_SEQ[%d]", d_seq, INT_SEQ);
                // -------------------------------------------------------
                // 4. 달리 직결
                // -------------------------------------------------------
                INL_Free_Buf(dec_data);

                return;
            }

            if (D_End == 0) {
                INT_SEQ += 1;  // 9999..99가 SEQ직결 �Ƚ�Ŵ
                PROC(D_K,P_K).tr_seq = INT_SEQ;
                memcpy(PROC(D_K,P_K).curr_tr,  Header_Fmt.MsgType,     11);
                //              Log (USR_OK, "SEQ Add");
            }

            if (p_flag) {  // 0가 처리요구
                memset(w_data, 0x20,           sizeof (w_data));
                memset(&File_Data_Head, ' ',   HEAD_SIZE);
                memset(&f_head, 0x20, sizeof(f_head));

                /* Header 50 byte — BUFF_RW_HEAD struct */
                ItoAf(d_seq,           f_head.Seq,           sizeof(f_head.Seq));
                ItoAf(d_seq,           f_head.If_Seq,        sizeof(f_head.If_Seq));
                memcpy(f_head.ApType,         ApType,         sizeof(f_head.ApType));
                memcpy(f_head.ResponseCode,   RES_NORMAL,     strlen(RES_NORMAL));
                memcpy(f_head.RecvTime1,      m_time,         sizeof(f_head.RecvTime1));
                memcpy(f_head.RecvTime2,      &m_time[10],    sizeof(f_head.RecvTime2));

                /* Header 20 byte — FILE_DATA_HEAD */
                ItoAf(d_size,      File_Data_Head.Length,  sizeof (File_Data_Head.Length));  // 4
                ItoAf(d_seq,       File_Data_Head.DataSeq, sizeof (File_Data_Head.DataSeq));  // 8
                memcpy(File_Data_Head.ResponseCode,    RES_NORMAL,     strlen(RES_NORMAL));  // 4
                memcpy(File_Data_Head.LineFlag,        _Exe_Name+4,    3);  // 3

                memcpy(f_head.DataHeader,  &File_Data_Head,    sizeof(FILE_DATA_HEAD));
                memcpy(w_data,             &f_head,            sizeof(BUFF_RW_HEAD));

                /* Data, 82�불필요, 직결�¥가 data날짜가 데이터만 가해야한다. */
                memcpy(&w_data[sizeof(BUFF_RW_HEAD)],      &DataBuff[0],               KRX_HEAD_LEN);
                memcpy(&w_data[sizeof(BUFF_RW_HEAD)+KRX_HEAD_LEN], &dec_data[i*d_size],    d_size);
                w_data[sizeof(BUFF_RW_HEAD) + OFS(D_K,P_K,p_flag-1)] = '\n';

                rt = F_W(TS_W1_1, (void *)w_data, 1);
                if (rt != 1) {
                    Log(SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,p_flag-1));
                    // -------------------------------------------------------
                    // 4. 달리 직결
                    // -------------------------------------------------------
                    INL_Free_Buf(dec_data);

                    return;
                }

                Log(USR_OK, "1. file write[%s:%d:%d] F_Size[%d]",
                        OFN(D_K,P_K,p_flag-1), OFW(D_K,P_K,p_flag-1,0), rt, OFS(D_K,P_K,p_flag-1));
            }

            Set_TR_Time();
        }

        // -------------------------------------------------------
        // 4. 달리 직결
        // -------------------------------------------------------
        INL_Free_Buf(dec_data);

    }
    else
#endif
    {
        /* n가 수신시 복호화된 업무헤더 직결� 가� 기본마스터 직결 구한다. 하는 테스트 준비가 인지하도록가 */
        for (i = 0; i < for_i; i++) {
            /* INT_SEQ 초기화 */
            if (for_i == 1) {
                d_seq = AtoIf(&Header_Fmt.MsgSeqNum[4], 7);  // 11자리임 가 7자리임
                Log(USR_OK, "7.H[%11.11s] d_seq[%d] p_flag[%d]", Header_Fmt.MsgSeqNum, d_seq, p_flag);
            }
            else {
                d_seq = AtoIf(&DataBuff[KRX_HEAD_LEN+4 + i*d_size], 7);  // 11자리임 가 6자리임
                Log(USR_OK, "8.DataBuff[%20.20s] d_seq[%d]", &DataBuff[KRX_HEAD_LEN+4 + i*d_size], d_seq);
            }

            if ((i == for_i-1)  &&  // 모듈명 DATA 인데 "99999999999"가 INT_SEQ가 초기화한다.(& 직결�⸶후처리)
                    (d_seq == 9999999))
            D_End = 1;

            if (INT_SEQ+1 != d_seq  && D_End == 0) {  // 모듈명 �ƴϸ鼭 seq가 다른
                Log(USR_ERROR, "2.PR_DATA Data Seq ERROR DataSeq[%d] INT_SEQ[%d] DataBuff[%20.20s]", d_seq, INT_SEQ, &DataBuff[KRX_HEAD_LEN+ i*d_size]);
                return;
            }

            if (D_End == 0) {
                INT_SEQ += 1;  // 9999..99가 SEQ직결 �Ƚ�Ŵ
                PROC(D_K,P_K).tr_seq = INT_SEQ;
                memcpy(PROC(D_K,P_K).curr_tr,  Header_Fmt.MsgType,     11);
                //              Log (USR_OK, "SEQ Add");
            }

            if (p_flag) {  // 0가 처리요구
                memset(w_data, 0x20,           sizeof (w_data));
                memset(&File_Data_Head, ' ',   HEAD_SIZE);
                memset(&f_head, 0x20, sizeof(f_head));

                /* Header 50 byte — BUFF_RW_HEAD struct */
                ItoAf(d_seq,           f_head.Seq,           sizeof(f_head.Seq));
                ItoAf(d_seq,           f_head.If_Seq,        sizeof(f_head.If_Seq));
                memcpy(f_head.ApType,         ApType,         sizeof(f_head.ApType));
                memcpy(f_head.ResponseCode,   RES_NORMAL,     strlen(RES_NORMAL));
                memcpy(f_head.RecvTime1,      m_time,         sizeof(f_head.RecvTime1));
                memcpy(f_head.RecvTime2,      &m_time[10],    sizeof(f_head.RecvTime2));

                /* Header 20 byte — FILE_DATA_HEAD */
                ItoAf(d_size,      File_Data_Head.Length,  sizeof (File_Data_Head.Length));  // 4
                ItoAf(d_seq,       File_Data_Head.DataSeq, sizeof (File_Data_Head.DataSeq));  // 8
                memcpy(File_Data_Head.ResponseCode,    RES_NORMAL,     strlen(RES_NORMAL));  // 4
                memcpy(File_Data_Head.LineFlag,        _Exe_Name+4,    3);  // 3

                memcpy(f_head.DataHeader,  &File_Data_Head,    sizeof(FILE_DATA_HEAD));
                memcpy(w_data,             &f_head,            sizeof(BUFF_RW_HEAD));

                /* Data, 82�불필요, 직결�¥가 data날짜가 데이터만 가해야한다. */
                memcpy(&w_data[sizeof(BUFF_RW_HEAD)],                  &DataBuff[0],           KRX_HEAD_LEN);
                memcpy(&w_data[sizeof(BUFF_RW_HEAD)+KRX_HEAD_LEN], &DataBuff[KRX_HEAD_LEN+i*d_size],   d_size);
                w_data[sizeof(BUFF_RW_HEAD) + OFS(D_K,P_K,p_flag-1)] = '\n';

                rt = F_W(TS_W1_1, (void *)w_data, 1);
                if (rt != 1) {
                    Log(SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,p_flag-1));
                    return;
                }

                Log(USR_OK, "i[%d] W_DATA[%30.30s]", i, &DataBuff[KRX_HEAD_LEN+i*d_size]);
                //              Log (USR_OK, "2. file write[%s:%d:%d] recv_seq[%d] proc_cnt[%d/%d] size[%d] F_Size[%d]",
                //                  OFN(D_K,P_K,p_flag-1), OFW(D_K,P_K,p_flag-1,0), rt, d_seq, i, for_i, d_size, OFS(D_K,P_K,p_flag-1));
            }

            Set_TR_Time();
        }
    }

    return;
}   /* End of Write_Data () */

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
    int     i, rt, datacnt;
    char    d_time[18];

    rt = 0;
    memset(&KR_Fmt,        0,  sizeof (KRX_R_SESSION_FMT));
    memset(&KR_Fmt, 0x20, sizeof (KRX_HEADER));

    memcpy(KR_Fmt.Header.BeginString,  "KMAPv2.0", 8);
    ItoAf(0, KR_Fmt.Header.BodyLength, sizeof (KR_Fmt.Header.BodyLength));
    ItoAf(0, KR_Fmt.Header.MsgSeqNum,  sizeof (KR_Fmt.Header.MsgSeqNum));  // �Ʒ직결� Update
    memcpy(KR_Fmt.Header.SenderCompID, TCP_COMPANY, strlen(TCP_COMPANY));
    Get_DateMilliTime(d_time);
    memcpy(KR_Fmt.Header.SendingTime,  d_time, sizeof (KR_Fmt.Header.SendingTime));
    ItoAf(0, KR_Fmt.Header.DataCnt,    sizeof (KR_Fmt.Header.DataCnt));
    memcpy(KR_Fmt.Header.Encrypt,      "N",        1);  // 초기� N, 주문만 Y
    SendLen = KRX_HEAD_LEN;

    switch (tr_code) {
        /* 2025 Add */
#ifndef NO_INISAFE
        case    TR_HSI:     /* handshake init */
            memcpy(KR_Fmt.Header.MsgType,  "SCHLIQ00101",      11);
            memcpy(KR_Fmt.Data, "0000", 4);  // INL_Handshake_Init() return value
            memcpy(&KR_Fmt.Data[4], (char *)cinitout, cinitoutl);
            MsgLen = cinitoutl + 4;
            ItoAf(MsgLen, KR_Fmt.Header.BodyLength,    sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_HSU:     /* handshake update */
            memcpy(KR_Fmt.Header.MsgType,  "SCHLIQ00103",      11);
            memcpy(KR_Fmt.Data, "0000", 4);  // INL_Handshake_Update() return value
            memcpy(&KR_Fmt.Data[4], (char *)cupdateout, cupdateoutl);
            MsgLen = cupdateoutl + 4;
            ItoAf(MsgLen, KR_Fmt.Header.BodyLength,    sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_HSF:     /* handshake final */
            memcpy(KR_Fmt.Header.MsgType,  "SCHLIQ00105",      11);
            memcpy(KR_Fmt.Data, "0000", 4);  // INL_Handshake_Final() return value
            memcpy(&KR_Fmt.Data[4], (char *)cfinalout, cfinaloutl);
            MsgLen = cfinaloutl + 4;
            ItoAf(MsgLen, KR_Fmt.Header.BodyLength,    sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
#endif
            /* 2025 Add */
        case    TR_LOON:    /* 로그온 신청 */
            memcpy(KR_Fmt.Header.MsgType,  "SCHLIQ00000",      11);
            ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum,    sizeof (KR_Fmt.Header.MsgSeqNum));
            memset(KR_Fmt.Data,    0x20,   41);  // Logon Size
            memcpy(KR_Fmt.Data,            LOGON_ID(D_K,P_K),  10);
            memcpy(&KR_Fmt.Data[10],       LOGON_PW(D_K,P_K),  strlen(LOGON_PW(D_K,P_K)));
            memcpy(&KR_Fmt.Data[40],       "Y",                 1);
            MsgLen  = strlen(KR_Fmt.Data);
            ItoAf(MsgLen, KR_Fmt.Header.BodyLength,    sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_LINK:    /* 업무헤더 신청 */
            Log(USR_OK, "업무헤더 신청");
            memcpy(KR_Fmt.Header.MsgType,  "SCHOPQ00000",      11);
            ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum,    sizeof (KR_Fmt.Header.MsgSeqNum));
            memcpy(KR_Fmt.Data, "0000", 4);
            if (memcmp(PROC(D_K,P_K).curr_tr, "           ", 11) > 0) {
                memcpy(&KR_Fmt.Data[4],    PROC(D_K,P_K).curr_tr,  11);
                ItoAf(PROC(D_K,P_K).tr_seq,   &KR_Fmt.Data[15],   11);
        }
        else {
            memcpy(&KR_Fmt.Data[4], "           ", 11);
            ItoAf(INT_SEQ, &KR_Fmt.Data[15],       11);
        }
        MsgLen  = strlen(KR_Fmt.Data);
        ItoAf(MsgLen, KR_Fmt.Header.BodyLength, sizeof(KR_Fmt.Header.BodyLength));
        SendLen += MsgLen;

        break;
        case    TR_POLL:    /* 회원사는 신청 */
            memcpy(KR_Fmt.Header.MsgType,  "SCHHER00000",  11);
            ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum,    sizeof (KR_Fmt.Header.MsgSeqNum));
            memcpy(KR_Fmt.Data,            "0000",          4);
            MsgLen  = strlen(KR_Fmt.Data);
            ItoAf(MsgLen,  KR_Fmt.Header.BodyLength,   sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_LOOU:    /* 로그아웃 신청 */
            memcpy(KR_Fmt.Header.MsgType,  "SCHLOR00000",  11);
            ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum,    sizeof (KR_Fmt.Header.MsgSeqNum));
            MsgLen  = strlen(KR_Fmt.Data);
            ItoAf(MsgLen,  KR_Fmt.Header.BodyLength,   sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_DATA:    /* 후처리 모듈명, DATA가�丸 모듈명. */
        case    RP_DATA:    /* 로그아웃 신청 */
            memcpy(KR_Fmt.Header.MsgType,  Header_Fmt.MsgType,     11);
            ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum,    sizeof (KR_Fmt.Header.MsgSeqNum));
            memcpy(KR_Fmt.Data,        "0000",         4);
            ItoAf(INT_SEQ, &KR_Fmt.Data[4],            11);
            MsgLen  = strlen(KR_Fmt.Data);
            ItoAf(MsgLen,  KR_Fmt.Header.BodyLength,   sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        default:
            break;
    }

    return (rt);
}   /* End of Make_Send_Msg ()  */

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
    Log_Out_Base();
}   /* End of Log_Out ()    */

/*************************************************************************
    End of Program (pb_7800_tr.c)
*************************************************************************/
