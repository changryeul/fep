#define     _GLOBAL

/*------------------------------------------------------------------------
 *  System  : Connect to KRX (파생 · 선물/옵션)
 *  Module  : 파생 KRX 주문응답/체결 수신 (TCP Receive) — PC 모듈
 *  File    : pc_1200_tr.c
 *
 *  [개요]
 *  pc_1100_ts(주문송신)의 짝. KRX와 TCP로 연결하여 파생 주문응답(회원처리호가)과
 *  체결결과를 수신, 내부 FIFO로 전달한다. KMAPv2.0 프로토콜.
 *
 *  시장 재편 A안: letter=시장(c=파생). IMECO 폐기 → KRX-direct 통일이라
 *  채권 수신기(pb_1200_tr.c)를 템플릿으로 신구축(현/파 공용 전문만 차별화).
 *    - 응답(회원처리호가): TTRODP11301 정상 / 11321 거부 / 11303 자동취소 (318B)
 *    - 체결결과         : TTRTDP21301 (현/파 233B)
 *    - 세션 수신 래퍼   : Header MsgType=TCHTDP00000, Body Transaction_Code=구체 전문
 *  개선 빌드모델: -DB120x 다중바이너리 없음. 1소스=1바이너리, argv[0]=pc_1201_tr.
 *  (채권 pb_1200_tr의 B1211 DR카피 / B1601 장운영 분기는 제외 — 파일럿은 응답/체결)
 *------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "krx_trcode.h"
#include    "ifaddrs.h"
#include    "fep_common.h"
#include    "fep_encrypt.h"
#include    "lat_trace.h"
#include    "seam_queue.h"      /* 부문간 SEAM 스테이징 큐(pc_ 수신 → po_ 소비) */

/* 응답 최대 318B(TTRODP11301) → Header(82)+Data(318) < 400 */
#define     DATA_SIZE       400
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     TCP_TIME_OUT    35

#define     DEVICE_TIME     3  * 1000                       /*  3 sec */
#define     MAIN_TIME       15 * 1000                       /* 15 sec HeartBeat */
#define     FOREVER_TIME    30 * 1000                       /* 30 sec */

#define     FIFO_EVENT      0
#define     SOCKET_EVENT    1

#define     MAX_CNT         1
#define     RESP_GAP        1000

#define     NO_TIME         "0830"

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

#ifndef NO_INISAFE
char        KRX_INITECH_CONF_PATH[256];
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
int     ConnectRetryCnt;
int     Sockfd, ErrCd, SendLen, MsgLen, RecvLen, ReTrCode;
int     PollCnt, FirstSeq, TimeOut, Pk;
int     back_int_seq, back_rd_cnt;
double  RTime = 0, STime = 0;
double  SendMsec, RecvMsec, RespMsec;
char    DeviceSendFlag, LogOnFlag, OpenFlag;
char    DataBuff[KRX_DATA_BUFF_SIZE], IpAddr[20], ApType[10];
char    NoTime[] = NO_TIME;

FILE_BUFF_FORMAT            W_Fmt;          /* FIFO 쓰기용 */
KRX_HEADER                  Header_Fmt;     /* KRX Header (82) */
KRX_R_SESSION_FMT           KR_Fmt;         /* KRX 세션 포맷 (82+116) */
FILE_DATA_HEAD              File_Data_Head; /* 파일 데이터 헤더 (20) */
struct pollfd       Poll[2];

void    *FmtPtr = (void *)&KR_Fmt;

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PC_1200_TR(void);
void    Init_Parameters(void);
void    Socket_Event_Rtn(void);
void    Device_Open(int);
void    Time_Out_Rtn(void);
int     Analyze_Data(void);
void    Write_Data(int);
int     Make_Send_Msg(int);
void    Log_Out(void);
#ifndef NO_INISAFE
int     DecryptBody(char *, int);
#endif

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    Init_Proc(argc, argv);
#ifdef LAT_TRACE
    LAT_INIT(argv[0]);
#endif
    SEAM_Init();        /* 부문간 SEAM 스테이징 큐 확보(응답/체결 → po_) */
    PC_1200_TR();
    Exit_Process();
}   /* End of main ()   */

/*----------------------------------------------------------------------*/
void    PC_1200_TR(void)
/*----------------------------------------------------------------------*/
{
    int     rt, i;

    Init_Parameters();

    while (START_S != END) {
        Stat_Save();

        if (OpenFlag == OFF) {
            if (LogOnFlag == OFF)
                Device_Open(TR_LOON);

            if (LogOnFlag == OFF) {
                ConnectRetryCnt ++;
                if (ConnectRetryCnt >= 3) {
                    Line_Change();
                    ConnectRetryCnt = 0;
                }

                PollCnt = 1;
                TimeOut = DEVICE_TIME;  // 3초
            }
            else {
                Device_Open(TR_LINK);

                if (OpenFlag == OFF) {
                    PollCnt = 1;
                    TimeOut = DEVICE_TIME;  // 3초
                }
                else {
                    PollCnt = 2;
                    TimeOut = MAIN_TIME;  // 15초
                }
            }
        }
        else {
            PollCnt = 2;
            TimeOut = MAIN_TIME;  // 15초
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

        for (i = 0; i < PollCnt; i ++) {
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
}   /* End of PC_1200_TR () */

/*************************************************************************
 *  Init_Parameters
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

    TCP2_PROC_ST1(MAIN) = ON;
    TCP2_PROC_ST1(BACKUP) = ON;

    TCP2_LINE_ST1(MAIN) = OFF;
    TCP2_LINE_ST1(BACKUP) = OFF;

    sprintf(ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));

    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;

    return;
}   /* End of Init_Parameters ()    */

/*************************************************************************
 *  Socket_Event_Rtn — TCP 수신 이벤트 처리
 *    수신 → (INISAFE 복호화) → ME그룹 시퀀스 검증 → TR코드별 FIFO 전달
 *      체결 TTRTDP21301        → Write_Data(2)
 *      회원처리호가 TTRODP113* → Write_Data(1) (4+11 패딩 + DATA)
 *      체결 IF종료 TCHEDP99000 → 종료
 *************************************************************************/
/*----------------------------------------------------------------------*/
void    Socket_Event_Rtn(void)
/*----------------------------------------------------------------------*/
{
    int     rval, r_meg_no, r_meg_seq;
    int     body_len;

    rval = Device_Read();
    if (rval < 0)
        return;

    DeviceSendFlag = OFF;
    ErrCd = 0;

    memset(&Header_Fmt, 0, KRX_HEAD_LEN);
    memcpy(&Header_Fmt, DataBuff, KRX_HEAD_LEN);

#ifdef LAT_TRACE
    LAT_POINT("IN", &DataBuff[KRX_HEAD_LEN], 11);
#endif

    ReTrCode = Analyze_Data();

    body_len = AtoIf(Header_Fmt.BodyLength, sizeof (Header_Fmt.BodyLength));

    switch (ReTrCode) {
        case    RP_DATA:
            /* Header 시퀀스 검증 */
            if (INT_SEQ+1 != FirstSeq) {
                TCP2_LINE_ST = OpenFlag = OFF;
                Log(USR_ERROR, "PR_DATA recv:Header invalid KRX SEQ [%d] INT_SEQ[%d]",
                        FirstSeq, INT_SEQ);
                sleep(3);
                Exit_Process();
            }

#ifndef NO_INISAFE
            DecryptBody(&DataBuff[KRX_HEAD_LEN], body_len);
#endif

            {
                KRX_MSG_COMMON *msg = (KRX_MSG_COMMON *)DataBuff;
                r_meg_no  = AtoIf(msg->Body.Megrp_no, KRX_MEGRPNO_LEN);
                r_meg_seq = AtoIf(msg->Body.DataSeq,  KRX_DATASEQ_LEN);

                /* ME그룹 시퀀스 정합성 검증 (현/파도 ME그룹 사용) */
                Log(USR_OK, "meg_no[%d] r_meg_seq[%d] INT[%d]",
                        r_meg_no, r_meg_seq, INT_MEG_SEQ(r_meg_no-1));
                if (INT_MEG_SEQ(r_meg_no-1) + 1 == r_meg_seq) {
                    INT_MEG_SEQ(r_meg_no-1) = r_meg_seq;
                }
                else {
                    TCP2_LINE_ST = OpenFlag = OFF;
                    Log(USR_ERROR, "PR_DATA recv:invalid r_meg_no[%d] r_meg_seq[%d] != INT_MEG_SEQ[%d]",
                            r_meg_no, r_meg_seq, INT_MEG_SEQ(r_meg_no-1));
                    sleep(3);
                    Exit_Process();
                }

                /* TR코드별 분기 (현/파) */
                if (IS_TR(msg, TR_ORDER_EXECUTION))                 /* 체결결과 TTRTDP21301 233B */ {
                    Write_Data(2);         /* 체결 처리 (→ pc_1400_mp 상당) */
                }
                else if (IS_TR_PREFIX(msg, TR_ORDER_RESP_PREFIX))   /* 회원처리호가 TTRODP113* 318B */ {
                    Write_Data(1);         /* 응답 처리 (→ pc_1200_mp 상당) */
                }
                else if (IS_TR(msg, TR_IF_END_SETTLE))              /* 체결 인터페이스 종료 */ {
                    Log(USR_OK, "체결 인터페이스 종료");
                    INT_SEQ++;

                    TCP2_NET_STA(MAIN) = END;
                    TCP2_NET_STA(BACKUP) = END;

                    if (TCP2_LINE_ST == ON || OpenFlag == ON)
                        Device_Close();

                    Log(USR_OK, "poll timeout <%d>", INT_SEQ);
                    sleep(60);
                    break;
                }
                else {
                    INT_SEQ++;
                    Log(USR_WARN, "PR_DATA recv Not Define DATA Skip [%11.11s] [%s][%d]",
                            msg->Body.Transaction_Code, msg->Body.DataSeq, RecvLen);
                    break;
                }
            }
            break;

        case    RP_POLL:
            /* 회선시험 수신 → 즉시 응답 */
            Make_Send_Msg(RP_POLL);
            memset(DataBuff, 0, sizeof (DataBuff));
            memcpy(DataBuff, &KR_Fmt, SendLen);
            Device_Write();
            break;
        case    RP_STOP:
            break;
        default:
            break;
    }

    return;
}   /* End of Socket_Event_Rtn ()   */

#ifndef NO_INISAFE
/*----------------------------------------------------------------------*/
int DecryptBody(char *databuff, int body_len)
/*----------------------------------------------------------------------*/
{
    int result = 0;
    unsigned char *dec_data = NULL;
    int dec_len = 0;

    result = INL_Decrypt(EnCtx, (unsigned char*)databuff, body_len, &dec_data, &dec_len);
    if (result != 0) {
        Log(USR_ERROR, "ERROR: INL_Decrypt failed with code %d\n", result);
        if (dec_data != NULL) INL_Free_Buf(dec_data);
        return -1;
    }

    memset(&DataBuff[KRX_HEAD_LEN], 0, sizeof(DataBuff) - KRX_HEAD_LEN);
    memcpy(&DataBuff[KRX_HEAD_LEN], dec_data, dec_len);
    RecvLen = KRX_HEAD_LEN + dec_len;

    INL_Free_Buf(dec_data);
    return 0;
}
#endif

/* Free_All, Handshake: sub/fep_encrypt.c 공용 (NO_INISAFE에선 stub) */

/*************************************************************************
 *  Device_Open — KRX 접속 (로그온/업무개시). LINK 응답 SCHOPR10000.
 *************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Open(int tr_code)
/*----------------------------------------------------------------------*/
{
    int     rt, rval;

    if (tr_code == TR_LOON) {
        Device_Open_Logon(1, 0);
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
                    memcmp(((KRX_HEADER *)DataBuff)->MsgType, "SCHOPR10000", 11) != 0) {
                Log(USR_ERROR, "LINK response recv error");
                close(Sockfd);
                break;
            }

            memset(&KR_Fmt, 0, sizeof (KRX_R_SESSION_FMT));
            memcpy(&KR_Fmt, DataBuff,
                    RecvLen > (int)sizeof (KRX_R_SESSION_FMT) ?
                    (int)sizeof (KRX_R_SESSION_FMT) : RecvLen);

            FirstSeq = AtoIf(KR_Fmt.Header.MsgSeqNum, sizeof (KR_Fmt.Header.MsgSeqNum));

            if (memcmp(KR_Fmt.Data, "0000", 4) == 0) {
                if (INT_SEQ != FirstSeq) {
                    TCP2_LINE_ST = OpenFlag = OFF;
                    Log(USR_ERROR, "PR_LINK recv:invalid KRX SEQ [%d] INT_SEQ[%d]",
                            FirstSeq, INT_SEQ);
                    sleep(3);
                    Exit_Process();
                }

                TCP2_NET_STA(S_K) = ON;
                TCP2_LINE_ST = OpenFlag = ON;
                ConnectRetryCnt = 0;

                Log(TCP_OK, "LINK OK");

                break;
            }
            else {
                Log(USR_ERROR, "PR_LINK recv:invalid ResponseCode[%4.4s] KRX SEQ[%d] INT_SEQ[%d]",
                        KR_Fmt.Data, FirstSeq, INT_SEQ);
                sleep(3);
                Exit_Process();
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
 *  Time_Out_Rtn
 *************************************************************************/
/*----------------------------------------------------------------------*/
void    Time_Out_Rtn(void)
/*----------------------------------------------------------------------*/
{
    memset(DataBuff, 0, sizeof (DataBuff));

    switch (PollCnt) {
        case    1:
            if (TimeOut == FOREVER_TIME)
                Log(USR_OK, "poll timeout <%d>:NSTAT[%d]", INT_SEQ, TCP2_NET_STA(S_K));
            break;
        case    2:
        case    3:
            Time_Out_Disconnect("FOT");
            break;
        default:
            break;
    }
}   /* End of Time_Out_Rtn ()   */

/*************************************************************************
 *  Analyze_Data — 수신 메시지 유형 판별
 *    수신측 세션 Data 래퍼 = TCHTDP00000 (Body Transaction_Code로 세분)
 *************************************************************************/
/*----------------------------------------------------------------------*/
int     Analyze_Data(void)
/*----------------------------------------------------------------------*/
{
    FirstSeq = AtoIf(Header_Fmt.MsgSeqNum, sizeof (Header_Fmt.MsgSeqNum));

    if (memcmp(Header_Fmt.MsgType, "SCHHEQ00000", 11) == 0) {          /* HeartBeat(회선시험) */
        Log(USR_OK, "회선시험요청(SCHHEQ00000): [%4.4s] <%d:%d:%d>",
                &DataBuff[KRX_HEAD_LEN], FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_POLL);
    }
    else if (memcmp(Header_Fmt.MsgType, "SCHLOQ00000", 11) == 0) {     /* LogOut */
        Log(USR_OK, "LogOut요청(SCHLOQ00000): [%4.4s] <%d:%d:%d>",
                &DataBuff[KRX_HEAD_LEN], FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_STOP);
    }
    else if (memcmp(Header_Fmt.MsgType, TR_SESSION_TRADE, 11) == 0) {  /* 응답/체결 (TCHTDP00000) */
        Log(USR_OK, "DATA수신(%.11s): <%d:%d>", TR_SESSION_TRADE, FirstSeq, INT_SEQ);
        return (RP_DATA);
    }

    Log(USR_ERROR, "Analyze_Data: unknown MsgType[%.11s]", Header_Fmt.MsgType);
    return (RP_STOP);
}   /* End of Analyze_Data ()   */

/*************************************************************************
 *  Write_Data — 수신 데이터를 FIFO로 전달
 *    p_flag=1 (회원처리호가): 15바이트 "000000000000000" 패딩 + DATA
 *    p_flag=2 (체결)        : DATA만
 *************************************************************************/
/*----------------------------------------------------------------------*/
void    Write_Data(int p_flag)
/*----------------------------------------------------------------------*/
{
    int     rt;
    char    m_time[24];

    memset(&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));
    memset(&File_Data_Head, ' ', HEAD_SIZE);
    memset(m_time, 0, sizeof (m_time));
    Get_MicroTime(m_time);

    ItoAf(INT_SEQ + 1, W_Fmt.If_Seq, sizeof (W_Fmt.If_Seq));

    memcpy(W_Fmt.ApType, ApType, sizeof (W_Fmt.ApType));
    memcpy(W_Fmt.ResponseCode, RES_NORMAL, strlen(RES_NORMAL));
    memcpy(W_Fmt.RecvTime1, m_time, sizeof (W_Fmt.RecvTime1));
    memcpy(W_Fmt.RecvTime2, &m_time[sizeof(W_Fmt.RecvTime1)], sizeof (W_Fmt.RecvTime2));

    ItoAf(HEAD_SIZE + DATA_SIZE, File_Data_Head.Length, sizeof (File_Data_Head.Length));
    ItoAf(INT_SEQ + 1, File_Data_Head.DataSeq, sizeof (File_Data_Head.DataSeq));
    memcpy(File_Data_Head.ResponseCode, RES_NORMAL, strlen(RES_NORMAL));
    memcpy(File_Data_Head.LineFlag, _Exe_Name+4, 3);

    memcpy(W_Fmt.DataHeader, &File_Data_Head, HEAD_SIZE);

    /* 본문: 응답(1)은 4+11 패딩(15) + 회원처리호가, 체결(2)은 DATA만 */
    if (p_flag == 1) {
        memcpy(W_Fmt.Data,      "000000000000000",       15);
        memcpy(&W_Fmt.Data[15], &DataBuff[KRX_HEAD_LEN], RecvLen - KRX_HEAD_LEN);
    }
    else {
        memcpy(W_Fmt.Data,      &DataBuff[KRX_HEAD_LEN], RecvLen - KRX_HEAD_LEN);
    }
    W_Fmt.LineFeed[0] = '\n';

    /* 부문간 전달: 파일큐(부문별키) 대신 전역 SEAM 스테이징 큐로 기록.
       응답(p_flag=1)→SEAM_Q_RESP, 체결(p_flag=2)→SEAM_Q_EXEC.
       레코드 = FILE_BUFF_FORMAT 전체 → po_가 슬롯→R_Fmt 복사로 필드 동일 소비. */
    rt = SEAM_W((p_flag == 2) ? SEAM_Q_EXEC : SEAM_Q_RESP,
                (void *)&W_Fmt, (int)sizeof(FILE_BUFF_FORMAT));
    if (rt != 1) {
        Log(SAM_FATAL, "SEAM_W fail q=%d rt=%d", (p_flag == 2) ? SEAM_Q_EXEC : SEAM_Q_RESP, rt);
        Exit_Process();
    }

#ifdef LAT_TRACE
    LAT_POINT("OUT", &DataBuff[KRX_HEAD_LEN], 11);
#endif

    Log_Hot(USR_OK, "SEAM write[q=%d:%d]", (p_flag == 2) ? SEAM_Q_EXEC : SEAM_Q_RESP, rt);

    INT_SEQ += rt;

    Set_TR_Time();

    return;
}   /* End of Write_Data () */

/*************************************************************************
 *  Make_Send_Msg — KMAPv2.0 송신 메시지 조립 (로그온/개시/회선/로그아웃)
 *************************************************************************/
/*----------------------------------------------------------------------*/
int     Make_Send_Msg(int tr_code)
/*----------------------------------------------------------------------*/
{
    int     rt;
    char    d_time[18];

    rt = 0;
    memset(&KR_Fmt, 0, sizeof (KRX_R_SESSION_FMT));
    memset(&KR_Fmt, 0x20, sizeof (KRX_HEADER));

    memcpy(KR_Fmt.Header.BeginString,  "KMAPv2.0", 8);
    ItoAf(0, KR_Fmt.Header.BodyLength, sizeof (KR_Fmt.Header.BodyLength));
    ItoAf(0, KR_Fmt.Header.MsgSeqNum,  sizeof (KR_Fmt.Header.MsgSeqNum));
    memcpy(KR_Fmt.Header.SenderCompID, TCP_COMPANY, strlen(TCP_COMPANY));
    Get_DateMilliTime(d_time);
    memcpy(KR_Fmt.Header.SendingTime,  d_time, sizeof (KR_Fmt.Header.SendingTime));
    ItoAf(0, KR_Fmt.Header.DataCnt,    sizeof (KR_Fmt.Header.DataCnt));
    memcpy(KR_Fmt.Header.Encrypt,      "N", 1);
    SendLen = KRX_HEAD_LEN;

    switch (tr_code) {
#ifndef NO_INISAFE
        case    TR_HSI:
            memcpy(KR_Fmt.Header.MsgType,  "SCHLIQ00101", 11);
            memcpy(KR_Fmt.Data, "0000", 4);
            memcpy(&KR_Fmt.Data[4], (char *)cinitout, cinitoutl);
            MsgLen = cinitoutl + 4;
            ItoAf(MsgLen, KR_Fmt.Header.BodyLength, sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_HSU:
            memcpy(KR_Fmt.Header.MsgType,  "SCHLIQ00103", 11);
            memcpy(KR_Fmt.Data, "0000", 4);
            memcpy(&KR_Fmt.Data[4], (char *)cupdateout, cupdateoutl);
            MsgLen = cupdateoutl + 4;
            ItoAf(MsgLen, KR_Fmt.Header.BodyLength, sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_HSF:
            memcpy(KR_Fmt.Header.MsgType,  "SCHLIQ00105", 11);
            memcpy(KR_Fmt.Data, "0000", 4);
            memcpy(&KR_Fmt.Data[4], (char *)cfinalout, cfinaloutl);
            MsgLen = cfinaloutl + 4;
            ItoAf(MsgLen, KR_Fmt.Header.BodyLength, sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
#endif
        case    TR_LOON:    /* 로그온 요청 */
            memcpy(KR_Fmt.Header.MsgType,  "SCHLIQ00000", 11);
            ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum, sizeof (KR_Fmt.Header.MsgSeqNum));
            memset(KR_Fmt.Data, 0x20, 41);
            memcpy(KR_Fmt.Data,      LOGON_ID(D_K,P_K), 10);
            memcpy(&KR_Fmt.Data[10], LOGON_PW(D_K,P_K), strlen(LOGON_PW(D_K,P_K)));
            memcpy(&KR_Fmt.Data[40], "Y", 1);
            MsgLen  = strlen(KR_Fmt.Data);
            ItoAf(MsgLen, KR_Fmt.Header.BodyLength, sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_LINK:    /* 업무개시 요청 (SCHOPQ10000, 체결/장운영/DropCopy 전용) */
            Log(USR_OK, "업무개시 요청");
            memcpy(KR_Fmt.Header.MsgType,  "SCHOPQ10000", 11);
            ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum, sizeof (KR_Fmt.Header.MsgSeqNum));

            memcpy(KR_Fmt.Data, "0000", 4);
            memset(&KR_Fmt.Data[4], '0', 2+110);    /* ME그룹시퀀스 초기값 0 */

            if (INT_SEQ > 0) {  /* 재접속시: 이전 ME그룹별 시퀀스 전송 */
                Log(USR_OK, "INT_MEG_SEQ 0[%d] 1[%d] 2[%d] 3[%d] 4[%d] 5[%d] 6[%d] 7[%d] 8[%d] 9[%d]",
                        INT_MEG_SEQ(0), INT_MEG_SEQ(1), INT_MEG_SEQ(2), INT_MEG_SEQ(3), INT_MEG_SEQ(4),
                        INT_MEG_SEQ(5), INT_MEG_SEQ(6), INT_MEG_SEQ(7), INT_MEG_SEQ(8), INT_MEG_SEQ(9));

                ItoAf(INT_MEG_SEQ(0), &KR_Fmt.Data[4+2],          11);
                ItoAf(INT_MEG_SEQ(1), &KR_Fmt.Data[4+2+(1*11)],   11);
                ItoAf(INT_MEG_SEQ(2), &KR_Fmt.Data[4+2+(2*11)],   11);
                ItoAf(INT_MEG_SEQ(3), &KR_Fmt.Data[4+2+(3*11)],   11);
                ItoAf(INT_MEG_SEQ(4), &KR_Fmt.Data[4+2+(4*11)],   11);
                ItoAf(INT_MEG_SEQ(5), &KR_Fmt.Data[4+2+(5*11)],   11);
                ItoAf(INT_MEG_SEQ(6), &KR_Fmt.Data[4+2+(6*11)],   11);
                ItoAf(INT_MEG_SEQ(7), &KR_Fmt.Data[4+2+(7*11)],   11);
                ItoAf(INT_MEG_SEQ(8), &KR_Fmt.Data[4+2+(8*11)],   11);
                ItoAf(INT_MEG_SEQ(9), &KR_Fmt.Data[4+2+(9*11)],   11);
            }

            MsgLen  = strlen(KR_Fmt.Data);
            ItoAf(MsgLen, KR_Fmt.Header.BodyLength, sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    RP_POLL:    /* 회선시험 응답 */
            memcpy(KR_Fmt.Header.MsgType,  "SCHHER00000", 11);
            ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum, sizeof (KR_Fmt.Header.MsgSeqNum));
            memcpy(KR_Fmt.Data, "0000", 4);
            MsgLen  = strlen(KR_Fmt.Data);
            ItoAf(MsgLen,  KR_Fmt.Header.BodyLength, sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    RP_LOOU:    /* 로그아웃 응답 */
            memcpy(KR_Fmt.Header.MsgType,  "SCHLOR00000", 11);
            ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum, sizeof (KR_Fmt.Header.MsgSeqNum));
            memcpy(KR_Fmt.Data, "0000", 4);
            MsgLen  = 0;
            ItoAf(MsgLen,  KR_Fmt.Header.BodyLength, sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        default:
            break;
    }

    return (rt);
}   /* End of Make_Send_Msg ()  */

/*----------------------------------------------------------------------*/
void    Log_Out(void)
/*----------------------------------------------------------------------*/
{
    Log_Out_Base();
}   /* End of Log_Out ()    */

/*************************************************************************
    End of Program (pc_1200_tr.c)
*************************************************************************/
