#define     _GLOBAL

/*------------------------------------------------------------------------
#   System  : Connect to KRX (파생 · 선물/옵션)
#   Module  : 주문송신 (KRX-direct, 현/파 공용 전문 TCHODR1xxxx)
#   File    : pc_1100_ts.c
#
#   시장 재편 A안: letter=시장(c=파생). 빌드모델 개선(1소스=1바이너리):
#     - -DPCxxxx 다중바이너리 없음. 프로세스ID는 argv[0](=pc_1101_ts)에서 Init_Proc가 파싱.
#     - 채권(pb_1100_ts.c) 템플릿 기반, KRX-direct 통일이라 세션/이벤트루프 공용.
#       차별점은 주문 전문 struct·크기(파생/현물 공용 TCHODR10001=294B)뿐.
#   전문: 주문 KRX_JUMUN_DATA(294) / 응답 TTRODP11301(318) / 체결 KRX_SETTLE_DATA(233).
#   IMECO 폐기 — 파생도 거래소(KRX) 직접 형태. (docs/01-plan/.../pc-derivatives-pilot.plan.md)
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "ifaddrs.h"
#include    "fep_common.h"
#include    "fep_encrypt.h"
#include    "lat_trace.h"
/* 주문 송신 Size 정의 : 파생/현물 호가입력 TCHODR10001 = 294 (내부헤더 55 + 294 < 400) */
#define     DATA_SIZE       400
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     TCP_TIME_OUT    35

#define     DEVICE_TIME     3 * 1000                        /*  3 sec   */
#define     MAIN_TIME       5 * 1000        /* Heart Beat 간격  5 sec */
#define     DATA_TIME       15 * 1000       /* Heart Beat 3회  15 sec    */
#define     FOREVER_TIME    60 * 1000                       /* 60 sec   */

#define     FIFO_EVENT      0
#define     SOCKET_EVENT    1
#define     DATA_EVENT      2

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

#define     CHK_PROC        "pc_1201_tr"                    /* 체결Process가 인터페이스종료 수신후 종료되면 주문송신도 종료 */

/*
KRX 주문 서비스 업무흐름도 (현/파 공용 세션)
0101 : 호가접수개시전,           세션Continue, 개시retry
0004 : 데이터 일련번호 오류,     세션Close,    처음부터 다시
0090 : 시스템오류,               세션Close,    처음부터 다시
0013 : 호가접수정지중,           세션Close,    처음부터 다시
0020 : TPS허용건수 초과,         1초간 주문sleep
*/

#ifndef NO_INISAFE
/* 암복호화 추가 */
char        KRX_INITECH_CONF_PATH[256];

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
int     PollCnt, FirstSeq, TimeOut, Pk;
int     back_rd_cnt;
double  RTime = 0, STime = 0;
double  SendMsec, RecvMsec, RespMsec;
char    DeviceSendFlag, LogOnFlag, OpenFlag, DataBuff[KRX_DATA_BUFF_SIZE], IpAddr[20];
char    NoTime[] = NO_TIME;

FILE_BUFF_FORMAT    R_Fmt[MAX_CNT], W_Fmt[MAX_CNT];
KRX_JUMUN_Q_FMT     J_Q_Fmt;  // KRX 파생/현물 주문포맷 (82 + 294*6)
KRX_JUMUN_R_FMT     J_R_Fmt;  // KRX 세션 응답 포맷 (82 + 4+11)
KRX_SESSION_FMT     S_Fmt;    // KRX 세션 포맷 (82 + 41)
struct pollfd       Poll[3];
void *FmtPtr = (void *)&S_Fmt;

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PC_1100_TS(void);
void    Init_Parameters(void);
void    Socket_Event_Rtn(void);
void    Data_Event_Rtn(void);
void    Device_Open(int);
void    Time_Out_Rtn(void);
int     Analyze_Data(void);
void    Write_Response_Data(int);
int     Make_Send_Msg(int);
int     Make_Data_Block(void);
void    Log_Out(void);
int     Chk_Risk_All(char *);
#ifndef NO_INISAFE
unsigned char* EncryptAndMakeSendPacket(const void*, size_t, int*);
#endif

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    Init_Proc(argc, argv);
#ifdef LAT_TRACE
    LAT_INIT(argv[0]);
#endif
    PC_1100_TS();
    Exit_Process();
}   /* End of main ()   */

/*----------------------------------------------------------------------*/
void    PC_1100_TS(void)
/*----------------------------------------------------------------------*/
{
    int     rt, i;

    Init_Parameters();

    while (START_S != END) {
        Stat_Save();

        if (TCP2_NET_STA(S_K) != END && TCP2_NSTAT(D_K,Pk,S_K) == END) {         /* 장운영 정보 Check */
                Log_Out();
        }

        if (TCP2_NET_STA(S_K) == END || TCP2_NET_STA(S_K) == JOB_STOP) {         /* 종료/중지 */
                if (LogOnFlag == ON && TCP2_NET_STA(S_K) == END) {
                Device_Close();
            }

            rt = poll(Poll, 1, 1000);
            if (rt > 0 && (Poll[FIFO_EVENT].revents & POLLIN))
                Fifo_Event_Rtn();
            continue;
        }
        else                                            /* 정상주문시간 */ {
            /* LogOnFlag: TCP Connect & LOGON check / OpenFlag: 업무개시 / DeviceSendFlag: DATA 전송 check */

            /* 개시를 해야만 주문을 낼 수 있음. 정규장 전에는 개시응답을 정상으로 안줌 → 개시만 retry */
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
                        PollCnt = 3;
                        TimeOut = MAIN_TIME;  // 5초
                    }
                }
            }
            else {  // 개시가 된 경우
                if (DeviceSendFlag == ON) {  // DATA 송신 후 LogOut 응답 대기
                    PollCnt = 2;
                    TimeOut = DATA_TIME;  // 15초
                }
                else {  // 주문/HeartBeat/KillSwitch, Async 송수신
                    Log_Hot(USR_OK, "OK22 W[%d] R[%d]", WR_CNT, RD_CNT);
                    PollCnt = 3;
                    TimeOut = MAIN_TIME;  // 5초

                    if (WR_CNT > RD_CNT) {  // 주문이 있으면 먼저 처리
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

        i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);
        if (i == -1) return;
        if (i == -2) continue;

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

    Device_Close();

    return;
}   /* End of PC_1100_TS () */

/*************************************************************************
    Function        : . Init_Parameters
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

    /* 체결Process(pc_1201_tr) process key 구함.
       Phase 1(주문송신 단독 기동, 체결은 Phase 2 산출물)에서는 짝 프로세스가
       아직 미등록일 수 있다 → FATAL 종료 대신 self-monitor로 폴백(오종료 방지).
       점진적 부문 bring-up(시장 재편) 요구를 반영한 것으로, 미등록은 WARN 로깅. */
    for (Pk = 0; Pk < DAEMON(D_K).p_count; Pk ++) {
        if (memcmp(PROC(D_K,Pk).process_id, CHK_PROC, 10) == 0)
            break;
    }
    if (Pk >= DAEMON(D_K).p_count) {
        Log(USR_WARN, "paired process[%s] not registered - standalone(self-monitor) 모드", CHK_PROC);
        Pk = P_K;   /* 자기 라인 상태로 장운영 체크 */
    }

    if (DELAY_TIME == 0)
        DELAY_TIME = RESP_GAP;

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
    Function        : . Socket_Event_Rtn  (TCP Data Recv & Response Action)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Socket_Event_Rtn(void)
/*----------------------------------------------------------------------*/
{
    int     rval, mun;

    rval = Device_Read();

    if (rval < 0)
        return;

    /* 세션 응답 공용:
        1. LogOn      SCHLIR00000
        2. LIOK(주문) SCHOPR00000
        4. HeartBeat  SCHHER00000
        5. LogOut     SCHLOR00000
        6. 주문거부   TCHODR00000 (82+4+11)
    */
    DeviceSendFlag = OFF;

    memset(&S_Fmt, 0, sizeof (KRX_SESSION_FMT));
    /* sfmt-overflow-fix: RecvLen이 구조체 크기를 넘으면 BSS 오버런 → 구조체 크기로 클램프 */
    memcpy(&S_Fmt, DataBuff,
            RecvLen > (int)sizeof (KRX_SESSION_FMT) ?
            (int)sizeof (KRX_SESSION_FMT) : RecvLen);

    ReTrCode = Analyze_Data();

    switch (ReTrCode) {
        case    RP_DATA:    /* 주문응답은 "0020"(TPS초과) 빼고 모두 거부 */
            if (memcmp(S_Fmt.Data, "0020", 4) == 0) {
                Log(USR_OK, "TPS초과 오류 수신 1초 sleep!!!");
                sleep(1);
                break;
            }

            if (ErrCd != 0) Err_Msg();

            /* Seq 처리 후 종료 */
            if (FirstSeq > INT_SEQ) {  // 수신 Seq가 INT_SEQ보다 클 수 없다 → 오류후 종료
                TCP2_LINE_ST = OpenFlag = OFF;

                Log(USR_ERROR, "RP_DATA recv:invalid KRX SEQ Recv:Seq[%d] > Int_Seq[%d]",
                        FirstSeq, INT_SEQ);

                sleep(3);
                Exit_Process();
            }
            else {  // 주문에서 DATA 수신 = 오류 발생
                Log(USR_WARN, "RP_DATA recv:Seq Change INT_SEQ[%d] => FirstSeq[%d]",
                        INT_SEQ, FirstSeq);

                Log(USR_OK, "01.Change INT_SEQ[%d] RD_CNT[%d]", INT_SEQ, RD_CNT);
                mun = (INT_SEQ - FirstSeq) - 1;

                /* rdcnt-underflow-guard: 되감기가 읽기커서를 음수화하면 재전송 폭주 위험 */
                if (mun > RD_CNT) {
                    Log(USR_ERROR, "RP_DATA recv:Seq rewind underflow mun[%d]>RD_CNT[%d] "
                            "FirstSeq[%d] INT_SEQ[%d] - 중복재전송 위험, 중단",
                            mun, RD_CNT, FirstSeq, INT_SEQ);
                    sleep(3);
                    Exit_Process();
                }

                INT_SEQ -= mun;
                RD_CNT  -= mun;
                Log(USR_OK, "02.Change INT_SEQ[%d] RD_CNT[%d]", INT_SEQ, RD_CNT);

                /* 나머지는 오류 처리 */
                Write_Response_Data(1);
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
    Function        : . Data_Event_Rtn  (SHM/FIFO Data Processing)
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
    if (rt == 0) {  // 정상
        Device_Write();
#ifdef LAT_TRACE
        LAT_POINT("OUT", &R_Fmt[0].Data[36], 10);
#endif

        INT_SEQ++;
        /* F5 order-pipeline: 입력 종류에 맞는 커서 전진 (DSHM vs 파일큐) */
        if (PROC(D_K,P_K).in_d[0] != 0)
            Dshm_Add_Count(PS_R_1, 1);
        else
            Add_Count(PS_R_1, 1);

        if (LogOnFlag == ON)
            Set_TR_Time();
    }

    rt = read(INPUT_FD, tmp, sizeof(tmp));

    return;
}   /* End of Data_Event_Rtn () */

/* Free_All, Handshake: sub/fep_encrypt.c 공용 (NO_INISAFE에선 stub) */

/*************************************************************************
    Function        : .  Device_Open  (Line Status Set & Poll fd set & LOGON)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Open(int tr_code)
/*----------------------------------------------------------------------*/
{
    int     rt, rval, mun;

    if (tr_code == TR_LOON) {
        Device_Open_Logon(1, 0);
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

            Log(USR_OK, "업무개시응답(SCHOPR00000): [%4.4s] <%d:%d:%d>",
                    S_Fmt.Data, FirstSeq, INT_SEQ, LOAD_CNT);

            if (memcmp(S_Fmt.Data, "0000", 4) == 0) {
                /* Seq 처리 */
                if (FirstSeq > INT_SEQ) {
                    TCP2_LINE_ST = OpenFlag = OFF;

                    Log(USR_ERROR, "TR_LINK recv:invalid KRX SEQ <%d:%d>",
                            FirstSeq, INT_SEQ);

                    sleep(3);
                    Exit_Process();
                }
                else if (FirstSeq < INT_SEQ) {
                    Log(USR_WARN, "TR_LINK recv:Seq Change [%d] => [%d] KRX SEQ <%d>",
                            INT_SEQ, FirstSeq, FirstSeq);

                    mun = INT_SEQ - FirstSeq;

                    /* rdcnt-underflow-guard: 되감기 재전송 폭주 위험 시 fail-safe 중단 */
                    if (mun > RD_CNT) {
                        TCP2_LINE_ST = OpenFlag = OFF;
                        Log(USR_ERROR, "TR_LINK recv:Seq rewind underflow mun[%d]>RD_CNT[%d] "
                                "FirstSeq[%d] INT_SEQ[%d] - 중복재전송 위험, 중단",
                                mun, RD_CNT, FirstSeq, INT_SEQ);
                        sleep(3);
                        Exit_Process();
                    }

                    RD_CNT  -= mun;
                    INT_SEQ -= mun;  // 수신측 Seq로 변경
                }

                LOAD_CNT = 0;
                TCP2_NET_STA(S_K) = ON;
                TCP2_LINE_ST = OpenFlag = ON;
                ConnectRetryCnt = 0;

                Log(TCP_OK, "LINK OK");

                break;
            }
            else {
                if (memcmp(S_Fmt.Data, "0101", 4) == 0) {  // 호가접수 개시전
                    Log(USR_OK, "0100 호가접수전 개시요청 3초후 Retry!!!");
                    sleep(3);

                    break;
                }

                /* 0101외 오류는 Process 종료로 운영자에게 알림 */
                Log(USR_ERROR, "TR_LINK recv:invalid ResponseCode[%4.4s] KRX SEQ <%d:%d>",
                        S_Fmt.Data, FirstSeq, INT_SEQ);

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
    Function        : .  Time_Out_Rtn  (Timeout Control)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Time_Out_Rtn(void)
/*----------------------------------------------------------------------*/
{
    memset(DataBuff, 0, sizeof (DataBuff));

    switch (PollCnt) {
        case    2:
            Time_Out_Disconnect("KRX");
            break;
        case    3:
            Make_Send_Msg(TR_POLL);
            memcpy(DataBuff, &S_Fmt, SendLen);
            Device_Write();
            break;
        default:
            break;
    }
}   /* End of Time_Out_Rtn ()   */

/*************************************************************************
    Function        : . Analyze_Data  (TCP Recv Data 분석 → TR_TYPE)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Analyze_Data(void)
/*----------------------------------------------------------------------*/
{
    FirstSeq = AtoIf(S_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));

    if (memcmp(S_Fmt.Header.MsgType, "SCHHER00000", 11) == 0) {  // HeartBeat
        Log(USR_OK, "회선시험요청(SCHHER00000): [%4.4s] <%d:%d:%d>",
                S_Fmt.Data, FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_POLL);
    }
    else if (memcmp(S_Fmt.Header.MsgType, "SCHLOR00000", 11) == 0) {  // LogOut
        Log(USR_OK, "LogOut요청(SCHLOR00000): [%4.4s] <%d:%d:%d>",
                S_Fmt.Data, FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_STOP);
    }
    else if (memcmp(S_Fmt.Header.MsgType, "TCHODR00000", 11) == 0) {  // 주문Error
        Log(USR_OK, "주문오류(TCHODR00000): <%d:%d:%d> 거부사유코드[%4.4s]",
                FirstSeq, INT_SEQ, LOAD_CNT, S_Fmt.Data);
        return (RP_DATA);
    }

    Log(USR_ERROR, "Analyze_Data: unknown MsgType[%.11s]", S_Fmt.Header.MsgType);
    return (RP_STOP);
}   /* End of Analyze_Data ()   */

/*************************************************************************
    Function        : . Write_Response_Data  (주문거부 값 돌려주기, file write)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Write_Response_Data(int d_cnt)
/*----------------------------------------------------------------------*/
{
    int     i, rt, cnt;

    if (d_cnt <= 0)
        return;

    memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT));

    Log(USR_OK, "Before DSHM_R RD_CNT[%d]", RD_CNT);

    cnt = DSHM_R(PS_R_1, (void *)R_Fmt, 1);
    if (cnt < 0 || cnt > 1) {
        Log(SAM_ERROR, "DSHM_R(PS_R_1)[%s] 읽기실패 거부처리 못했음_1, cnt[%d] RD_CNT[%d]", IDN(D_K,P_K,0), cnt, RD_CNT);
        return;
    }
    else if (cnt == 0) {
        Log(SAM_ERROR, "DSHM_R(PS_R_1)[%s] 읽기실패 거부처리 못했음_2, cnt[%d] RD_CNT[%d]", IDN(D_K,P_K,0), cnt, RD_CNT);
        return;
    }

    /* 주문거부: 수신전문 "4+11 + 55(찾아서) + 주문전문" 전달 (KRX Header 82 제외), Async 1건 */
    Log(USR_OK, "Make Packet 01 RD_CNT[%d]", RD_CNT);
    for (i = 0; i < d_cnt; i ++) {
        memset(W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);

        /* 1. Seq (8) */
        memcpy(W_Fmt[i].Seq,           &S_Fmt.Data[4],             sizeof (W_Fmt[0].Seq));
        /* 2. If_Seq (8) */
        memcpy(W_Fmt[i].If_Seq,        S_Fmt.Header.MsgSeqNum,     sizeof (W_Fmt[0].If_Seq));
        /* 3. ApType (8) */
        memcpy(W_Fmt[i].ApType,        R_Fmt[i].ApType,            sizeof (W_Fmt[0].ApType));
        /* 4. ResponseCode (4) - 전달은 정상(0000), 오류코드는 위에서 처리 */
        memcpy(W_Fmt[i].ResponseCode,  RES_NORMAL,                 strlen(RES_NORMAL));
        /* 6. RecvTime2 (12자리중 9자리) */
        memcpy(W_Fmt[i].RecvTime2,     &S_Fmt.Header.SendingTime[8],   9);
        /* 7. DataHeader(20) */
        memcpy(W_Fmt[i].DataHeader,    R_Fmt[i].DataHeader,        HEAD_SIZE);
        /* 8. DATA 조합: KRX Reply(4+11) + 주문전문 */
        memcpy(W_Fmt[i].Data,          S_Fmt.Data,                 sizeof(KRX_JUMUN_R_DATA));  // 4+11
        memcpy(&W_Fmt[i].Data[sizeof(KRX_JUMUN_R_DATA)],           R_Fmt[i].Data,  310);
        W_Fmt[i].LineFeed[0] = '\n';
    }
    Log(USR_OK, "Maked Packet 02");

    rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);  // OFN1 = 응답분배
    if (rt != 1) {
        Log(SAM_FATAL, "file write fail[%s:%d]", OFN(D_K,P_K,0), rt);
        return;
    }

    Add_Count(PS_R_1, 1);

    Log(USR_OK, "Write OK!! Error Response Data[%s:%d:%d] RD_CNT[%d]", OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), i, RD_CNT);

    return;
}   /* End of Write_Response_Data ()    */

/*************************************************************************
    Function        : . Make_Send_Msg  (Send data header making)
    Return Code     : . int (-1:no data, 0:success)
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

    /* Make The Header (현/파 공용) */
    memcpy(S_Fmt.Header.BeginString, "KMAPv2.0",   8);
    ItoAf(0, S_Fmt.Header.BodyLength, sizeof (S_Fmt.Header.BodyLength));
    ItoAf(0, S_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));
    memcpy(S_Fmt.Header.SenderCompID, TCP_COMPANY, strlen(TCP_COMPANY));
    Get_DateMilliTime(d_time);
    memcpy(S_Fmt.Header.SendingTime, d_time, sizeof (S_Fmt.Header.SendingTime));
    ItoAf(0, S_Fmt.Header.DataCnt, sizeof (S_Fmt.Header.DataCnt));
    memcpy(S_Fmt.Header.Encrypt,       "N",        1);  // 초기 N, 주문만 Y
    SendLen = KRX_HEAD_LEN;

    switch (tr_code) {
#ifndef NO_INISAFE
        case    TR_HSI:     /* handshake init */
            memcpy(S_Fmt.Header.MsgType,  "SCHLIQ00101",      11);
            memcpy(S_Fmt.Data, "0000", 4);
            memcpy(&S_Fmt.Data[4], (char *)cinitout, cinitoutl);
            MsgLen = cinitoutl + 4;
            ItoAf(MsgLen, S_Fmt.Header.BodyLength,    sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_HSU:     /* handshake update */
            memcpy(S_Fmt.Header.MsgType,  "SCHLIQ00103",      11);
            memcpy(S_Fmt.Data, "0000", 4);
            memcpy(&S_Fmt.Data[4], (char *)cupdateout, cupdateoutl);
            MsgLen = cupdateoutl + 4;
            ItoAf(MsgLen, S_Fmt.Header.BodyLength,    sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_HSF:     /* handshake final */
            memcpy(S_Fmt.Header.MsgType,  "SCHLIQ00105",      11);
            memcpy(S_Fmt.Data, "0000", 4);
            memcpy(&S_Fmt.Data[4], (char *)cfinalout, cfinaloutl);
            MsgLen = cfinaloutl + 4;
            ItoAf(MsgLen, S_Fmt.Header.BodyLength,    sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
#endif
        case    TR_LOON:    /* 로그온 요청 */
            memcpy(S_Fmt.Header.MsgType,   "SCHLIQ00000",  11);
            ItoAf(INT_SEQ, S_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));
            memset(S_Fmt.Data, 0x20, 41);
            memcpy(S_Fmt.Data,         LOGON_ID(D_K,P_K), 10);
            memcpy(&S_Fmt.Data[10],    LOGON_PW(D_K,P_K), strlen(LOGON_PW(D_K,P_K)));
            memcpy(&S_Fmt.Data[40],    "Y",                1);
            MsgLen  = strlen(S_Fmt.Data);
            ItoAf(MsgLen, S_Fmt.Header.BodyLength, sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_LINK:    /* 업무개시 요청 */
            memcpy(S_Fmt.Header.MsgType,   "SCHOPQ00000",  11);
            ItoAf(INT_SEQ, S_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));
            memcpy(S_Fmt.Data, "0000", 4);
            memcpy(&S_Fmt.Data[4], "           ", 11);
            ItoAf(INT_SEQ, &S_Fmt.Data[15],    11);
            MsgLen  = strlen(S_Fmt.Data);
            ItoAf(MsgLen, S_Fmt.Header.BodyLength, sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_POLL:    /* 회선시험 요청 */
            memcpy(S_Fmt.Header.MsgType,       "SCHHEQ00000",  11);
            ItoAf(INT_SEQ, S_Fmt.Header.MsgSeqNum,     sizeof (S_Fmt.Header.MsgSeqNum));
            memcpy(S_Fmt.Data, "0000", 4);
            MsgLen  = strlen(S_Fmt.Data);
            ItoAf(MsgLen,  S_Fmt.Header.BodyLength,    sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_LOOU:    /* 로그아웃 요청 */
            memcpy(S_Fmt.Header.MsgType,       "SCHLOQ00000",  11);
            ItoAf(INT_SEQ, S_Fmt.Header.MsgSeqNum,     sizeof (S_Fmt.Header.MsgSeqNum));
            MsgLen  = 0;
            ItoAf(MsgLen,  S_Fmt.Header.BodyLength,    sizeof(S_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_DATA:    /* 주문요청 + Kill Switch */
            memcpy(S_Fmt.Header.Encrypt,       "Y",            1);  // 주문만 Y
            memset(&J_Q_Fmt,           0,              sizeof (KRX_JUMUN_Q_FMT));
            memcpy(&J_Q_Fmt.Header,    &S_Fmt.Header,  KRX_HEAD_LEN);

            memcpy(J_Q_Fmt.Header.MsgType,     "TCHODR00000",  11);
            ItoAf(INT_SEQ+1, J_Q_Fmt.Header.MsgSeqNum, sizeof (J_Q_Fmt.Header.MsgSeqNum));

            MsgLen = Make_Data_Block();
            if (MsgLen <= 0) {
                rt = -1;
                break;
            }

            ItoAf(datacnt, J_Q_Fmt.Header.DataCnt,     sizeof (J_Q_Fmt.Header.DataCnt));
            memcpy(&DataBuff[78],  "001",      3);  // 단건 처리
            SendLen += MsgLen;
            break;
        default:
            break;
    }

    return (rt);
}   /* End of Make_Send_Msg ()  */

/*************************************************************************
    Function        : . Make_Data_Block
    Return Code     : . int (정상: KRX send size, 0: no data)
    Comment         : . 입력큐(파일/DSHM) 주문 1건 → DataBuff = KRX_HEADER(82) + 주문전문
                        파생/현물 호가입력 TCHODR10001/2/3 = 294B (KRX_JUMUN_DATA)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Make_Data_Block(void)
/*----------------------------------------------------------------------*/
{
    int     r_cnt, i, Size_Len;
    int     enc_len = 0;
#ifndef NO_INISAFE
    unsigned char *ret_data = NULL;
#endif

    i = Size_Len = 0;
    memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);
    memset(W_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);

    /* 송신 파일/큐: 내부헤더 + 주문전문 (Data[0]=DataSeq(11), Data[11]=Transaction_Code(11)) */
    /* F5 order-pipeline: proc.ini 설정(IDN/IFN)으로 DSHM/파일큐 자동 선택 */
    if (PROC(D_K,P_K).in_d[0] != 0) {
        r_cnt = DSHM_R(PS_R_1, (void *)R_Fmt, MAX_CNT);
        if (r_cnt < 0 || r_cnt > MAX_CNT) {
            Log(SAM_FATAL, "DSHM_R(PS_R_1)[%s]", IDN(D_K,P_K,0));
            Exit_Process();
        }
    }
    else {
        r_cnt = F_R(PS_R_1, (void *)R_Fmt, MAX_CNT);
        if (r_cnt < 0 || r_cnt > MAX_CNT) {
            Log(SAM_FATAL, "F_R(PS_R_1)[%s]", IFN(D_K,P_K,0));
            Exit_Process();
        }
    }

    if (r_cnt == 0) {
        return (0);
    }

    for (i = 0; i < r_cnt; i ++) {
        /* 파생/현물 호가입력 TCHODR10001(신규)/10002(정정)/10003(취소) = 294B */
        if ( (memcmp(&R_Fmt[i].Data[11], "TCHODR10001", 11) == 0)  ||
                (memcmp(&R_Fmt[i].Data[11], "TCHODR10002", 11) == 0)  ||
                (memcmp(&R_Fmt[i].Data[11], "TCHODR10003", 11) == 0)  ) {
            Size_Len = sizeof (KRX_JUMUN_DATA);  // 294
        }
        else {
            Log(USR_ERROR, "Undefined DataType Was Read [%11.11s]", &R_Fmt[i].Data[11]);
            sleep(3);
            Exit_Process();
        }

        LOAD_CNT ++;
#ifdef LAT_TRACE
        /* 주문ID offset: DataSeq(11)+TrCode(11)+Megrp(2)+Board(2)+Member(5)+Branch(5) = 36 (파생은 MktId 없음) */
        LAT_POINT("IN", &R_Fmt[i].Data[36], 10);
#endif
        /* 일련번호 Update 후 처리 넘김 */
        ItoAf(INT_SEQ+1, &R_Fmt[i].Data[0], 11);

        Log_Hot(USR_OK, "&R_Fmt[i].Data [%s]", &R_Fmt[i].Data[0]);

#ifndef NO_INISAFE
        /* 202509 거래소 암복호화 처리 */
        ret_data = EncryptAndMakeSendPacket(&R_Fmt[i].Data, Size_Len, &enc_len);
        if (ret_data) {
            memcpy(DataBuff,   &J_Q_Fmt.Header,    sizeof(KRX_HEADER));
            ItoAf(enc_len,    &DataBuff[8],       6);
            memcpy(&DataBuff[sizeof(KRX_HEADER)],  ret_data,   enc_len);
            INL_Free_Buf(ret_data);
            ret_data = NULL;
        }
        else {
            LOAD_CNT --;
            Log(USR_ERROR, "암호화 오류 error, 확인 필요");
            sleep(3);
            Exit_Process();
        }
#else
        /* NO_INISAFE: 평문 직접 조립 (KRX_HEADER + 주문전문 294) */
        memcpy(DataBuff,   &J_Q_Fmt.Header,    sizeof(KRX_HEADER));
        ItoAf(Size_Len,   &DataBuff[8],       6);
        memcpy(&DataBuff[sizeof(KRX_HEADER)],  &R_Fmt[i].Data,   Size_Len);
        enc_len = Size_Len;
#endif
    }  // 실제로는 1건 (Async)

    return (enc_len);
}   /* End of Make_Data_Block ()    */

/*************************************************************************
    Function        : . Chk_Risk_All  (reserved)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Chk_Risk_All(char *p_buf)
/*----------------------------------------------------------------------*/
{
    return 0;
}

#ifndef NO_INISAFE
/*----------------------------------------------------------------------*/
unsigned char* EncryptAndMakeSendPacket(const void* plain_data, size_t plain_len, int* enc_len)
/*----------------------------------------------------------------------*/
{
    int result = 0;
    unsigned char* enc_data = NULL;

    result = INL_Encrypt(EnCtx, (unsigned char*)plain_data, (int)plain_len, &enc_data, enc_len);
    if (result != 0) {
        Log(USR_ERROR, "ERROR: INL_Encrypt failed with code %d\n", result);
        if (enc_data) {
            INL_Free_Buf(enc_data);
        }
        return NULL;
    }

    return enc_data;
}
#endif

/*************************************************************************
    Function        : . Log_Out
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Log_Out(void)
/*----------------------------------------------------------------------*/
{
    if (Log_Out_Base() == OK) {
        PROC(D_K,P_K).start_status = JOB_END;
        PROC(D_K,P_K).process_status = 2;
        write(DTART_FD, "1", 1);
    }
}   /* End of Log_Out ()    */

/*************************************************************************
    End of Program (pc_1100_ts.c)
*************************************************************************/
