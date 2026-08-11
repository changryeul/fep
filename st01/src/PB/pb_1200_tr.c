#define     _GLOBAL

/*------------------------------------------------------------------------
 *  System  : Connect to KRX (한국거래소 접속)
 *  Author  : PSH
 *  Module  : 채권 KRX 주문응답/체결/장운영정보 수신 (TCP Receive) — PB 모듈
 *  File    : pb_1200_tr.c
 *
 *  [개요]
 *  pa_1200_tr.c의 PB(채권직접접속) 버전이다.
 *  KRX와 TCP로 연결하여 채권 주문응답, 체결결과, 장운영정보를 수신한다.
 *  KMAPv2.0 프로토콜 사용, INISAFE 암호화 적용.
 *
 *  [PA vs PB 주요 차이점]
 *  1) INISAFE 암호화 사용:
 *     - fep_encrypt.h 포함, EnCtx/cinitout/cupdateout/cfinalout 변수
 *     - Handshake()는 sub/fep_encrypt.c의 실제 구현 사용
 *     - Make_Send_Msg에 TR_HSI/TR_HSU/TR_HSF (핸드셰이크) 추가
 *     - DecryptBody()로 수신 데이터 복호화
 *     - Device_Close에서 Free_All(), INL_Cleanup() 호출
 *  2) Device_Open_Logon(1, 0) — 1=PB모드(암호화 사용)
 *  3) RP_POLL 수신시 회선시험 응답을 즉시 전송 (PA는 무처리)
 *  4) 빌드옵션 B1211: DR(재해복구) 카피 전용
 *     - ME그룹 시퀀스 검증 없이 무조건 저장
 *     - 모든 데이터를 pb_8111_ts로 전달
 *  5) B1201: Write_Data에서 응답(p_flag=1)시 15바이트 패딩 추가
 *  6) Make_Send_Msg: Header를 0x20으로 초기화 (PA는 0x00)
 *     TR_LINK시 INT_SEQ > 0 조건 사용 (PA는 INT_SEQ == 0)
 *
 *  [빌드 옵션]
 *  - B1201 : 채권 주문응답/체결 수신 (회원처리호가+체결)
 *  - B1211 : 채권 DR(재해복구) 카피 수신
 *  - B1601 : 장운영정보 수신
 *
 *  [통신 절차]
 *  1단계) 로그온: Device_Open_Logon(1,0) → INISAFE 핸드셰이크 → 로그온
 *  2단계) 업무개시: TR_LINK → SCHOPQ10000 전송 → 응답 수신
 *  3단계) 데이터 수신 루프 (DecryptBody로 복호화 후 처리)
 *------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "krx_trcode.h"
#include    "ifaddrs.h"
#include    "fep_common.h"
#include    "fep_encrypt.h"         /* PB 전용: INISAFE 암호화 함수 */
#include    "lat_trace.h"

/* ********************************************** */
/*
    TTRMIP31301 : 공개장운영,                 91,    O
    TTRMIP32301 : 주식종목정보 공개,        146
    TTRMIP32303 : 회원 제재/해제 공개,       63
    TTRMIP32304 : 결제적용기준환율 공개,   53
    TTRMIP31302 : 기준가정보,                 71
    TTRMIP31303 : 임의종료,                 131
    TTRMIP31304 : 종목마감,                 105
    TTRMIP31306 : 배분정보,                  49
    TTRMIP31307 : VI(변동성완화장치),      117
    TTRMIP31308 : 실시간가격제한,           70
    TTRMIP31309 : 가격제한폭확대발동,         84
*/
/* ********************************************** */

/*------------------------------------------------------------------------
 *  빌드옵션별 데이터 버퍼 크기 정의
 *  - B1201/B1211 : 채권 응답/체결/DR카피 (Header 82 + Data 최대 318)
 *  - B1601       : 장운영정보 (Data 최대 200)
 *------------------------------------------------------------------------*/
#if defined(B1201) || defined(B1211)
#define     DATA_SIZE       400           /* Header(82) + Data(318)_가변포함 */
#elif defined B1601
#define     DATA_SIZE       200           /* Data(200)_가변포함 */
#endif
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     TCP_TIME_OUT    35          /* cfg에 미설정시 기본 타임아웃(초) */

#define     DEVICE_TIME     3  * 1000                       /*  3 sec — 접속 재시도 간격 */
#define     MAIN_TIME       15 * 1000       /* Heart Beat 간격 15 sec */
#define     FOREVER_TIME    30 * 1000                       /* 30 sec — 대기 타임아웃 */

#define     FIFO_EVENT      0           /* poll 이벤트: FIFO (데몬 제어) */
#define     SOCKET_EVENT    1           /* poll 이벤트: TCP 소켓 수신 */

#define     MAX_CNT         1
#define     RESP_GAP        1000

#define     NO_TIME         "0830"      /* 채권정규시장 09시~15시, 시간외 없음 */

/* FIFO 읽기/쓰기 카운터 매크로 — SAM_USE 여부에 따라 분기 */
#ifdef  SAM_USE
#define     WR_CNT          W_CNT(0,0)
#define     RD_CNT          R_CNT(0,0)
#define     IN_NAME         IFN(D_K,P_K,0)
#else
#define     WR_CNT          IDW_CNT(0,0)
#define     RD_CNT          IDR_CNT(0,0)
#define     IN_NAME         IDN(D_K,P_K,0)
#endif

#define     PORT_NO         TCP2_PORT_NO    /* KRX 접속 포트번호 */

/*------------------------------------------------------------------------
 *  INISAFE 암호화 관련 전역 변수 (PB 전용)
 *------------------------------------------------------------------------*/
#ifndef NO_INISAFE
char        KRX_INITECH_CONF_PATH[256];     /* INISAFE 설정 파일 경로 */

net_ctx         *EnCtx = NULL;              /* INISAFE 네트워크 컨텍스트 */

/* 서버측 핸드셰이크 출력 버퍼 */
unsigned char   *sinitout = NULL;
unsigned char   *supdateout = NULL;

/* 클라이언트측 핸드셰이크 입출력 버퍼 */
unsigned char   *cinitout = NULL;
unsigned char   *cupdateout = NULL;
unsigned char   *cfinalout = NULL;

int             cinitoutl = 0;              /* 핸드셰이크 init 출력 길이 */
int             cupdateoutl = 0;            /* 핸드셰이크 update 출력 길이 */
int             cfinaloutl = 0;             /* 핸드셰이크 final 출력 길이 */
#endif

/*------------------------------------------------------------------------
 *  전역 변수 (Global Variables)
 *------------------------------------------------------------------------*/
int     ConnectRetryCnt;                    /* 접속 재시도 횟수 (3회 초과시 회선 절체) */
int     Sockfd, ErrCd, SendLen, MsgLen, RecvLen, ReTrCode;
int     PollCnt, FirstSeq, TimeOut, Pk;
int     back_int_seq, back_rd_cnt;          /* 이전 시퀀스/카운트 백업 */
double  RTime = 0, STime = 0;               /* 수신/송신 시각 */
double  SendMsec, RecvMsec, RespMsec;       /* 밀리초 단위 시간 측정 */
char    DeviceSendFlag, LogOnFlag, OpenFlag;    /* 상태 플래그: OFF=미완, ON=완료 */
char    DataBuff[KRX_DATA_BUFF_SIZE], IpAddr[20], ApType[10];
char    NoTime[] = NO_TIME;

/* 주요 구조체 변수 */
FILE_BUFF_FORMAT            W_Fmt;          /* FIFO 쓰기용 버퍼 */
KRX_NOTE_ALL_JUMUN_Q_FMT    J_Q_Fmt;        /* KRX 주문포맷 (max 6건) (82+300) */
KRX_NOTE_JUMUN_R_FMT        J_R_Fmt;        /* KRX 세션 응답 포맷 (82+4+11) */
KRX_NOTE_JUMUN_S_FMT        Re_W_Fmt;       /* 내부 주문응답 송신 포맷 */
KRX_HEADER                  Header_Fmt;     /* KRX Header (82바이트) */
KRX_R_SESSION_FMT           KR_Fmt;         /* KRX 세션 포맷 (82+116) */
FILE_DATA_HEAD              File_Data_Head; /* 파일 데이터 헤더 (20바이트) */
struct pollfd       Poll[2];                /* poll 배열: [0]=FIFO, [1]=TCP소켓 */

/* fep_common 패턴: FmtPtr에 세션 포맷 연결 */
void    *FmtPtr = (void *)&KR_Fmt;

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PB_1200_TR(void);          /* 메인 루프 */
void    Init_Parameters(void);     /* 초기화 */
void    Socket_Event_Rtn(void);    /* TCP 수신 이벤트 처리 */
void    Device_Open(int);          /* KRX 접속 (로그온/업무개시) */
void    Time_Out_Rtn(void);        /* 타임아웃 처리 */
int     Analyze_Data(void);        /* 수신 데이터 분석 → TR 코드 판별 */
void    Write_Data(int);           /* 수신 데이터를 FIFO로 전달 */
int     Make_Send_Msg(int);        /* 송신 메시지 조립 (KMAPv2.0) */
void    Log_Out(void);             /* 로그아웃 처리 */
#ifndef NO_INISAFE
int     DecryptBody(char *, int);   /* PB 전용: 수신 데이터 복호화 */
#endif

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    Init_Proc(argc, argv);         /* 프로세스 공통 초기화 */
#ifdef LAT_TRACE
    LAT_INIT(argv[0]);
#endif
    PB_1200_TR();                  /* 메인 처리 루프 */
    Exit_Process();                /* 프로세스 종료 */
}   /* End of main ()   */

/*------------------------------------------------------------------------
 *  PB_1200_TR — 메인 이벤트 루프
 *
 *  PA_1200_TR()와 동일한 구조:
 *  1) OpenFlag OFF → 로그온 시도 (Device_Open_Logon은 PB모드로 INISAFE 핸드셰이크 수행)
 *  2) LogOnFlag ON → 업무개시 시도 (TR_LINK)
 *  3) OpenFlag ON → 데이터 수신 대기 (poll)
 *------------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
void    PB_1200_TR(void)
/*----------------------------------------------------------------------*/
{
    int     rt, i;

    Init_Parameters();

    while (START_S != END) {
        Stat_Save();

        /* ============================================================ */
        /* 접속 상태에 따른 분기 처리                                   */
        /* ============================================================ */
        if (OpenFlag == OFF)                /* 업무개시가 안된 경우 */ {
            if (LogOnFlag == OFF)               /* 로그온이 안된경우 → 로그온 시도 */
                Device_Open(TR_LOON);

            if (LogOnFlag == OFF)               /* 아직 로그온이 안된경우 */ {
                ConnectRetryCnt ++;
                if (ConnectRetryCnt >= 3)       /* 3회 실패시 회선 절체 */ {
                    Line_Change();
                    ConnectRetryCnt = 0;
                }

                PollCnt = 1;                    /* FIFO만 감시 */
                TimeOut = DEVICE_TIME;          /* 3초 */
            }
            else                                /* 로그온 성공 → 업무개시 시도 */ {
                Device_Open(TR_LINK);

                if (OpenFlag == OFF)            /* 업무개시 실패 */ {
                    PollCnt = 1;
                    TimeOut = DEVICE_TIME;      /* 3초 */
                }
                else                            /* 업무개시 성공 */ {
                    PollCnt = 2;
                    TimeOut = MAIN_TIME;        /* 15초 */
                }
            }
        }
        else                                    /* 업무개시 완료 상태 */ {
            PollCnt = 2;
            TimeOut = MAIN_TIME;                /* 15초 (HeartBeat 간격) */
        }

        /* ============================================================ */
        /* poll 대기 — FIFO/소켓 이벤트 감시                            */
        /* ============================================================ */
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
}   /* End of PB_1200_TR () */

/*************************************************************************
 *  Init_Parameters — 프로세스 변수 초기화
 *
 *  PA와의 차이:
 *  - TIME_OUT 기본값을 TCP_TIME_OUT(35초)으로 설정 (PA는 없음)
 *  - 나머지는 PA와 동일
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

    /* cfg 미설정시 기본 타임아웃 적용 */
    if (TIME_OUT == 0)
        TIME_OUT = TCP_TIME_OUT;
    Log(USR_OK, "TIME_OUT[%d]", TIME_OUT);

    S_K = TCP2_LINE_GU;                 /* 현재 사용할 회선 */

    /* KRX 접속 IP 주소 조립 */
    sprintf(IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
            TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    Log(TCP_OK, "IP[%s] port[%d]", IpAddr, PORT_NO);

    /* 프로세스/회선 상태 초기화 */
    TCP2_PROC_ST1(MAIN) = ON;
    TCP2_PROC_ST1(BACKUP) = ON;

    TCP2_LINE_ST1(MAIN) = OFF;
    TCP2_LINE_ST1(BACKUP) = OFF;

    /* ApType 생성 */
    sprintf(ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));

    /* Poll[0] = FIFO (데몬 제어 신호) */
    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;

    return;
}   /* End of Init_Parameters ()    */

/*************************************************************************
 *  Socket_Event_Rtn — TCP 소켓 수신 이벤트 처리
 *
 *  PA와의 차이:
 *  1) 수신 후 DecryptBody()로 INISAFE 복호화 수행
 *  2) B1211(DR카피): ME그룹 시퀀스 검증 없이 무조건 저장
 *  3) B1201: 응답(p_flag=1)시 15바이트 패딩 추가
 *  4) RP_POLL 수신시 회선시험 응답을 즉시 전송
 *  5) B1601: 장운영정보 — 모든 TR을 pb_1601_mp로 전달 (PA는 #1/#2 분리)
 *************************************************************************/
/*----------------------------------------------------------------------*/
void    Socket_Event_Rtn(void)
/*----------------------------------------------------------------------*/
{
    int     rval, rt, cnt, r_meg_no, r_meg_seq;
    int     body_len;
    char    t_time[12];

    /* KRX 메시지 수신 */
    rval = Device_Read();

    /* 수신 실패 시 리턴 */
    if (rval < 0)
        return;

    DeviceSendFlag = OFF;
    ErrCd = 0;

    /* KRX 헤더(82바이트) 추출 */
    memset(&Header_Fmt, 0, KRX_HEAD_LEN);
    memcpy(&Header_Fmt, DataBuff, KRX_HEAD_LEN);

#ifdef LAT_TRACE
    /* Body 선두 DataSeq(11)를 구간 키로 사용 */
    LAT_POINT("IN", &DataBuff[KRX_HEAD_LEN], 11);
#endif

    Log_Hot(USR_OK, "OK001");
    /* 메시지 유형 분석 */
    ReTrCode = Analyze_Data();
    Log_Hot(USR_OK, "OK002");

    /* 암호화된 DATA의 본문 길이 추출 */
    body_len = AtoIf(Header_Fmt.BodyLength, sizeof (Header_Fmt.BodyLength));

    /* ************************************ */
    /* RP_DATA, RP_POLL 체크                */
    /* ************************************ */
    switch (ReTrCode) {
        case    RP_DATA:
            /* ================================================ */
            /* 시퀀스 검증                                       */
            /* ================================================ */
            if (INT_SEQ+1 != FirstSeq) {
                TCP2_LINE_ST = OpenFlag = OFF;

                Log(USR_ERROR, "PR_DATA recv:Header invalid KRX SEQ [%d] INT_SEQ[%d]",
                        FirstSeq, INT_SEQ);

                sleep(3);                          /* 3초 후 종료 */
                Exit_Process();                    /* 종료, 운영자에게 알리기 위해서 */
        }

        /* ================================================ */
        /* PB 전용: INISAFE 복호화 처리                       */
        /* ================================================ */
#ifndef NO_INISAFE
        DecryptBody(&DataBuff[KRX_HEAD_LEN], body_len);
#endif

        /* ================================================ */
        /* ME그룹 시퀀스 검증 및 저장                          */
        /* ================================================ */
        {
            KRX_MSG_COMMON *msg = (KRX_MSG_COMMON *)DataBuff;
            r_meg_no  = AtoIf(msg->Body.Megrp_no, KRX_MEGRPNO_LEN);        /* ME그룹번호 */
            r_meg_seq = AtoIf(msg->Body.DataSeq,   KRX_DATASEQ_LEN);       /* ME그룹 일련번호 */

            /* ============================================================ */
            /* B1201/B1601: ME그룹 시퀀스 정합성 검증                        */
            /* ============================================================ */
#if defined(B1201) || defined(B1601)

            Log(USR_OK, "meg_no [%d], r_meg_seq[%d] INT[%d]", r_meg_no, r_meg_seq ,INT_MEG_SEQ(r_meg_no-1));
            if (INT_MEG_SEQ(r_meg_no-1) + 1 == r_meg_seq) {
                INT_MEG_SEQ(r_meg_no-1) = r_meg_seq;    /* 정상: 시퀀스 갱신 */
            }
            else {
                TCP2_LINE_ST = OpenFlag = OFF;

                Log(USR_ERROR, "PR_DATA recv:Data invalid r_meg_no [%d], r_meg_seq[%d] !=  INT_MEG_SEQ[%d]",
                        r_meg_no, r_meg_seq, INT_MEG_SEQ(r_meg_no-1));

                sleep(3);                          /* 3초 후 종료 */
                Exit_Process();                    /* 종료, 운영자에게 알리기 위해서 */
            }

            /* ============================================================ */
            /* B1211(DR카피): 시퀀스 검증 없이 무조건 저장                    */
            /* ============================================================ */
#elif defined B1211
            INT_MEG_SEQ(r_meg_no-1) = r_meg_seq;    /* DR: 무조건 시퀀스 저장 */
#endif

            /* ============================================================ */
            /* B1201: 채권 주문응답/체결 수신                                */
            /* ============================================================ */
#if defined B1201
            /* TR 코드별 분기 */
            if (IS_TR(msg, TR_BOND_EXECUTION))                                  /* 채권 체결결과(일반,조성) 317바이트 */ {
                Write_Data(2);         /* pb_1401_mp로 전달 (체결 처리) */
            }
            else
                if (IS_TR_PREFIX(msg, TR_BOND_ORDER_RESP)   ||                  /* 채권 일반처리호가 291 */
                        IS_TR_PREFIX(msg, TR_BOND_MM_RESP)      ||                  /* 채권 조성처리호가 295 */
                        IS_TR_PREFIX(msg, TR_KILL_SWITCH)       )                   /* Kill Switch 응답 161 */ {
                Write_Data(1);         /* pb_1201_mp로 전달 (응답 처리) */
            }
            else
                if (IS_TR(msg, TR_IF_END_SETTLE))                               /* 체결 인터페이스 종료 153 */ {
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
                Log(USR_WARN, "PR_DATA recv Not Define DATA Skip [%11.11s], [%s][%d]",
                        msg->Body.Transaction_Code, msg->Body.DataSeq, RecvLen);
                break;
            }

            /* ============================================================ */
            /* B1211(DR카피): 모든 데이터를 pb_8111_ts로 전달                */
            /* ============================================================ */
#elif defined B1211
            Log(USR_OK, "OK01");
            Write_Data(1);         /* pb_8111_ts로 전달 */
            Log(USR_OK, "OK02");

            /* ============================================================ */
            /* B1601: 장운영정보 수신                                        */
            /* (PA와 차이: PA는 #1/#2로 분리, PB는 모두 pb_1601_mp로 전달)    */
            /* ============================================================ */
#elif defined B1601

            /* ************************************************************************
                장운영데이터는 장운영 프로세스 번호#1, #2로 분리하여 송신
                장운영#1 : 공개장운영(공개장운영, 공개정보, 인터페이스 종료)
                장운영#2 : 종목마감  (종목마감, 기준가결정, 임의종료, 인터페이스 종료)

                TTRMIP31301 : 공개장운영,                91,    O
                TTRMIP32301 : 주식종목정보 공개,        146
                TTRMIP32303 : 회원 제재/해제 공개,       63
                TTRMIP32304 : 결제적용기준환율 공개,     53
                TTRMIP31302 : 기준가정보,                71
                TTRMIP31303 : 임의종료,                 131
                TTRMIP31304 : 종목마감,                 105
                TTRMIP31306 : 배분정보,                  49
                TTRMIP31307 : VI(변동성완화장치),       117
                TTRMIP31308 : 실시간가격제한,            70
                TTRMIP31309 : 가격제한폭확대발동,        84

             ************************************************************************ */

            /* TR 코드별 분기 */
            /* 수신받은 모든 장운영정보는 통합으로 전달. 202510 요청에 의함 */
            if (IS_TR_PREFIX(msg, TR_MARKET_OPR_PREFIX))                /* 공개장운영 등 */ {
                Write_Data(1);         /* pb_1601_mp로 전달 */
            }
            else
                if (IS_TR(msg, TR_IF_END_MARKET))           /* 장운영 인터페이스 종료 153 */ {
                Write_Data(1);         /* pb_1601_mp로 전달 */
                Log(USR_OK, "장운영 인터페이스 종료");

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
                Log(USR_WARN, "PR_DATA recv Not Define DATA Skip [%11.11s], [%s][%d]",
                        msg->Body.Transaction_Code, msg->Body.DataSeq, RecvLen);
                break;
            }
#endif
        }   /* end of KRX_MSG_COMMON *msg scope */

        break;

        case    RP_POLL:
            /* PB 전용: 회선시험 수신시 즉시 응답 전송 (PA는 무처리) */
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

/*************************************************************************
 *  DecryptBody — 수신 데이터 INISAFE 복호화 (PB 전용)
 *
 *  [처리 흐름]
 *  1) INL_Decrypt()로 암호화된 본문 복호화
 *  2) 복호화된 데이터를 DataBuff[KRX_HEAD_LEN]에 덮어쓰기
 *  3) RecvLen 갱신 (헤더 + 복호화된 길이)
 *  4) INL_Free_Buf()로 메모리 해제
 *
 *  [참고] PA에는 이 함수가 없음 (PA는 비암호화 통신)
 *************************************************************************/
#ifndef NO_INISAFE
/*----------------------------------------------------------------------*/
int DecryptBody(char *databuff, int body_len)
/*----------------------------------------------------------------------*/
{
    int result = 0;
    unsigned char *dec_data = NULL;
    int dec_len = 0;

    /* 1. INISAFE 복호화 호출
     *    - 헤더(82바이트)는 제외
     *    - body_len만큼만 복호화 */
    result = INL_Decrypt(EnCtx,
            (unsigned char*)databuff,      /* 암호화된 본문 시작 위치 */
            body_len,                      /* 암호화 길이 */
            &dec_data,                     /* 복호화 결과 (라이브러리가 할당) */
            &dec_len);                     /* 복호화 결과 길이 */

    if (result != 0) {
        Log(USR_ERROR, "ERROR: INL_Decrypt failed with code %d\n", result);
        if (dec_data != NULL) INL_Free_Buf(dec_data);
        return -1;
    }

    /* 2. 복호화된 데이터를 DataBuff에 복사 */
    memset(&DataBuff[KRX_HEAD_LEN],    0,          sizeof(DataBuff) - KRX_HEAD_LEN);
    memcpy(&DataBuff[KRX_HEAD_LEN],    dec_data,   dec_len);
    RecvLen = KRX_HEAD_LEN + dec_len;                           /* 수신길이 갱신 */

    /* 3. 복호화 결과 로깅 */
    Log(USR_OK, "복호화 성공! 복호화된 길이 = %d\n", dec_len);
    Log(USR_OK, "복호화된 데이터: %.*s\n", dec_len, dec_data);

    /* 4. 메모리 해제 */
    INL_Free_Buf(dec_data);

    return 0;
}
#endif

/* Free_All, Handshake: moved to sub/fep_encrypt.c */

/*************************************************************************
 *  Device_Open — KRX 접속 처리
 *
 *  PA와의 차이:
 *  - Device_Open_Logon(1, 0) — 1=PB모드 (INISAFE 핸드셰이크 수행)
 *  - LINK 응답 MsgType: "SCHOPR10000" (PA는 "SCHOPR00000")
 *************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Open(int tr_code)
/*----------------------------------------------------------------------*/
{
    int     rt, rval;

    if (tr_code == TR_LOON) {
        /* 1=PB모드 (INISAFE 핸드셰이크 수행), 0=tcp2 인덱스 */
        Device_Open_Logon(1, 0);
    }
    else if (tr_code == TR_LINK) {
        while (START_S != END) {
            /* 업무개시 요청 메시지 조립 및 전송 */
            rt = Make_Send_Msg(TR_LINK);
            memset(DataBuff, 0, sizeof (DataBuff));
            memcpy(DataBuff, &KR_Fmt, SendLen);
            Device_Write();
            break;
            Log(USR_OK, "send LINK request");

            /* 업무개시 응답 수신 — PB는 "SCHOPR10000" (PA는 "SCHOPR00000") */
            rval = Device_Read();
            if (rval < 0 ||
                    memcmp(((KRX_HEADER *)DataBuff)->MsgType, "SCHOPR10000", 11) != 0) {
                Log(USR_ERROR, "LINK response recv error");
                close(Sockfd);
                break;
            }

            memset(&KR_Fmt, 0, sizeof (KRX_R_SESSION_FMT));
            memcpy(&KR_Fmt, DataBuff, RecvLen);

            FirstSeq = AtoIf(KR_Fmt.Header.MsgSeqNum, sizeof (KR_Fmt.Header.MsgSeqNum));

            if (memcmp(KR_Fmt.Data, "0000", 4) == 0)        /* 거부사유코드 0000 = 정상 */ {
                /* 시퀀스 검증 */
                if (INT_SEQ != FirstSeq) {
                    TCP2_LINE_ST = OpenFlag = OFF;

                    Log(USR_ERROR, "PR_LINK recv:invalid KRX SEQ [%d] INT_SEQ[%d]",
                            FirstSeq, INT_SEQ);

                    sleep(3);                          /* 3초 후 종료 */
                    Exit_Process();                    /* 종료, 운영자에게 알리기 위해서 */
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

                sleep(3);                          /* 3초 후 종료 */
                Exit_Process();                    /* 종료, 운영자에게 알리기 위해서 */
            }
        }   /* End Of While */
    }

    return;
}   /* End of Device_Open ()    */

/*----------------------------------------------------------------------
 *  Device_Close — 소켓 종료 + 암호화 자원 해제
 *
 *  PA와의 차이:
 *  - Free_All(): INISAFE 핸드셰이크 버퍼 해제
 *  - INL_Cleanup(): INISAFE 컨텍스트 정리
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
void    Device_Close(void)
/*----------------------------------------------------------------------*/
{
    Device_Close_Base();               /* fep_common: 소켓 종료 + 상태 초기화 */
#ifndef NO_INISAFE
    Free_All((void *)(long)1);         /* INISAFE 핸드셰이크 버퍼 해제 */
    INL_Cleanup(CLIENT_CTX);           /* INISAFE 컨텍스트 정리 */
#endif
}   /* End of Device_Close ()   */

/*************************************************************************
 *  Time_Out_Rtn — 타임아웃 처리 (PA와 동일)
 *************************************************************************/
/*----------------------------------------------------------------------*/
void    Time_Out_Rtn(void)
/*----------------------------------------------------------------------*/
{
    memset(DataBuff, 0, sizeof (DataBuff));

    switch (PollCnt) {
        case    1:      /* 접속 대기 중 */
            if (TimeOut == FOREVER_TIME)
                Log(USR_OK, "poll timeout <%d>:NSTAT[%d]",
                        INT_SEQ, TCP2_NET_STA(S_K));
            break;
        case    2:      /* 데이터 수신 대기 중 → 회선시험 응답 */
        case    3:
            Time_Out_Disconnect("FOT");
            break;
        default:
            break;
    }
}   /* End of Time_Out_Rtn ()   */

/*************************************************************************
 *  Analyze_Data — 수신 메시지 유형 판별
 *
 *  PA와의 차이:
 *  - 빌드옵션별 DATA MsgType이 다름:
 *    B1201: TCHTDP00000 (채권 주문응답/체결)
 *    B1211: TCHDRP00000 (DR카피)
 *    B1601: TCHMIP00000 (장운영정보)
 *  - PA는 모두 TCHODR00000 사용
 *************************************************************************/
/*----------------------------------------------------------------------*/
int     Analyze_Data(void)
/*----------------------------------------------------------------------*/
{
    FirstSeq = AtoIf(Header_Fmt.MsgSeqNum, sizeof (Header_Fmt.MsgSeqNum));

    if (memcmp(Header_Fmt.MsgType, "SCHHEQ00000", 11) == 0)         /* HeartBeat(회선시험) */ {
        Log(USR_OK, "회선시험요청(SCHHEQ00000): [%4.4s] <%d:%d:%d>",
                &DataBuff[KRX_HEAD_LEN], FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_POLL);
    }
    else if (memcmp(Header_Fmt.MsgType, "SCHLOQ00000", 11) == 0)        /* LogOut(로그아웃) */ {
        Log(USR_OK, "LogOut요청(SCHLOQ00000): [%4.4s] <%d:%d:%d>",
                &DataBuff[KRX_HEAD_LEN], FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_STOP);
    }
#if defined B1201
    else if (memcmp(Header_Fmt.MsgType, "TCHTDP00000", 11) == 0)        /* B1201: 채권 주문응답/체결 */ {
        Log(USR_OK, "DATA수신(TCHTDP00000): <%d:%d>", FirstSeq, INT_SEQ);
#elif defined B1211
        else if (memcmp(Header_Fmt.MsgType, "TCHDRP00000", 11) == 0)        /* B1211: DR카피 */ {
            Log(USR_OK, "DATA수신(TCHDRP00000): <%d:%d>", FirstSeq, INT_SEQ);
#elif defined B1601
            else if (memcmp(Header_Fmt.MsgType, "TCHMIP00000", 11) == 0)        /* B1601: 장운영정보 */ {
                Log(USR_OK, "DATA수신(TCHMIP00000): <%d:%d>", FirstSeq, INT_SEQ);
#endif
                return (RP_DATA);
            }

            Log(USR_ERROR, "Analyze_Data: unknown MsgType[%.11s]", Header_Fmt.MsgType);
            return (RP_STOP);
        }   /* End of Analyze_Data ()   */

        /*************************************************************************
         *  Write_Data — 수신 데이터를 FIFO로 전달
         *
         *  PA와의 차이:
         *  - B1201 + p_flag=1(회원처리호가): 15바이트 "000000000000000" 패딩 추가
         *    (15+DATA 형식으로 처리)
         *  - B1211: DR카피 포맷 별도 처리 (1211 전용 주석 참조)
         *  - 로그 출력 형식이 약간 다름
         *************************************************************************/
        /*----------------------------------------------------------------------*/
        void    Write_Data(int p_flag)
        /*----------------------------------------------------------------------*/
        {
            int     rt, seq;
            char    m_time[24];

            /* ************************************ */
            /* 1201 : 82 + 4+11 + 회원처리호가전문  */
            /* 1401 : 82 + 체결전문                 */
            /* ************************************ */

            /* FIFO 쓰기 버퍼 초기화 */
            memset(&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));
            memset(&File_Data_Head, ' ', HEAD_SIZE);
            memset(m_time, 0, sizeof (m_time));
            Get_MicroTime(m_time);

            /* BUFF_RW_HEAD 설정 */
            ItoAf(INT_SEQ + 1, W_Fmt.If_Seq,   sizeof (W_Fmt.If_Seq));

            memcpy(W_Fmt.ApType, ApType,       sizeof (W_Fmt.ApType));
            memcpy(W_Fmt.ResponseCode, RES_NORMAL, strlen(RES_NORMAL));
            memcpy(W_Fmt.RecvTime1, m_time,    sizeof (W_Fmt.RecvTime1));
            memcpy(W_Fmt.RecvTime2,    &m_time[sizeof(W_Fmt.RecvTime1)],
                    sizeof (W_Fmt.RecvTime2));

            /* FILE_DATA_HEAD(20바이트) 설정 */
            ItoAf(HEAD_SIZE + DATA_SIZE, File_Data_Head.Length,
                    sizeof (File_Data_Head.Length));
            ItoAf(INT_SEQ + 1, File_Data_Head.DataSeq, sizeof (File_Data_Head.DataSeq));
            memcpy(File_Data_Head.ResponseCode, RES_NORMAL, strlen(RES_NORMAL));
            memcpy(File_Data_Head.LineFlag, _Exe_Name+4, 3);

            memcpy(W_Fmt.DataHeader, &File_Data_Head,  HEAD_SIZE);

            /* 본문 데이터 복사 */
            /* 1201 : 4+11 + 회원처리호가, 거래소에서 4+11+회원처리호가 수신 */
            /* 1401 : 체결정보 체결전문,   거래소에서 체결만 수신 */
            /* 1601/1602 : 장운영정보수신전문, 거래소에서 장운영정보만 수신 */
            /* 1211 : DR copy */
#if  defined B1201
            if (p_flag == 1)        /* B1201 + 회원처리호가: 15바이트 패딩 + DATA */ {
                memcpy(W_Fmt.Data,         "000000000000000",          15);
                memcpy(&W_Fmt.Data[15],    &DataBuff[KRX_HEAD_LEN],    RecvLen - KRX_HEAD_LEN);
            }
            else                    /* B1201 + 체결: DATA만 */
            memcpy(W_Fmt.Data,         &DataBuff[KRX_HEAD_LEN],    RecvLen - KRX_HEAD_LEN);
#else
            memcpy(W_Fmt.Data,         &DataBuff[KRX_HEAD_LEN],    RecvLen - KRX_HEAD_LEN);
#endif
            W_Fmt.LineFeed[0] = '\n';

            /* FIFO에 1건 쓰기 */
            rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);

            if (rt != 1) {
                Log(SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,p_flag-1));
                Exit_Process();
            }

#ifdef LAT_TRACE
            LAT_POINT("OUT", &DataBuff[KRX_HEAD_LEN], 11);
#endif

            Log_Hot(USR_OK, "file write[%s:%d:%d]", OFN(D_K,P_K,p_flag-1), OFW(D_K,P_K,p_flag-1,0), rt);

            Log_Hot(USR_OK, "file write[%s:%d:%d]", OFN(D_K,P_K,p_flag-1+2), OFW(D_K,P_K,p_flag-1+2,0), rt);

            INT_SEQ += rt;

            Set_TR_Time();

            return;
        }   /* End of Write_Data () */

        /*************************************************************************
         *  Make_Send_Msg — KMAPv2.0 송신 메시지 조립
         *
         *  PA와의 차이:
         *  1) Header를 0x20(공백)으로 초기화 (PA는 0x00)
         *  2) TR_HSI/TR_HSU/TR_HSF: INISAFE 핸드셰이크 메시지 추가
         *     - HSI: SCHLIQ00101 (handshake init)
         *     - HSU: SCHLIQ00103 (handshake update)
         *     - HSF: SCHLIQ00105 (handshake final)
         *  3) TR_LOON: 비밀번호 길이를 strlen()으로 처리 (PA는 고정 30바이트)
         *  4) TR_LINK: INT_SEQ > 0 조건 (PA는 INT_SEQ == 0)
         *     — PB는 최초시에도 memset 0으로 초기화하고, 재접속시만 ME그룹 시퀀스 전송
         *************************************************************************/
        /*----------------------------------------------------------------------*/
        int     Make_Send_Msg(int tr_code)
        /*----------------------------------------------------------------------*/
        {
            int     i, rt, datacnt;
            char    d_time[18];

            rt = 0;
            memset(&KR_Fmt, 0, sizeof (KRX_R_SESSION_FMT));
            memset(&KR_Fmt, 0x20, sizeof (KRX_HEADER));        /* PB: Header를 0x20으로 초기화 */

            /* ======================================== */
            /* KMAPv2.0 공통 헤더 설정                   */
            /* ======================================== */
            memcpy(KR_Fmt.Header.BeginString,  "KMAPv2.0", 8);
            ItoAf(0, KR_Fmt.Header.BodyLength, sizeof (KR_Fmt.Header.BodyLength));
            ItoAf(0, KR_Fmt.Header.MsgSeqNum,  sizeof (KR_Fmt.Header.MsgSeqNum));
            memcpy(KR_Fmt.Header.SenderCompID, TCP_COMPANY, strlen(TCP_COMPANY));
            Get_DateMilliTime(d_time);
            memcpy(KR_Fmt.Header.SendingTime,  d_time, sizeof (KR_Fmt.Header.SendingTime));
            ItoAf(0, KR_Fmt.Header.DataCnt,    sizeof (KR_Fmt.Header.DataCnt));
            memcpy(KR_Fmt.Header.Encrypt,      "N",        1);     /* 초기는 N, 주문만 Y */
            SendLen = KRX_HEAD_LEN;

            /* ======================================== */
            /* TR코드별 본문(Body) 조립                   */
            /* ======================================== */
            switch (tr_code) {
                /* ================================================ */
                /* PB 전용: INISAFE 핸드셰이크 메시지 (3단계)        */
                /* ================================================ */
#ifndef NO_INISAFE
                case    TR_HSI:     /* handshake init — 키교환 시작 */
                    memcpy(KR_Fmt.Header.MsgType,  "SCHLIQ00101",      11);
                    memcpy(KR_Fmt.Data, "0000", 4);                /* INL_Handshake_Init() 결과 */
                    memcpy(&KR_Fmt.Data[4], (char *)cinitout, cinitoutl);
                    MsgLen = cinitoutl + 4;
                    ItoAf(MsgLen, KR_Fmt.Header.BodyLength,    sizeof(KR_Fmt.Header.BodyLength));
                    SendLen += MsgLen;
                    break;
                case    TR_HSU:     /* handshake update — 키교환 진행 */
                    memcpy(KR_Fmt.Header.MsgType,  "SCHLIQ00103",      11);
                    memcpy(KR_Fmt.Data, "0000", 4);                /* INL_Handshake_Update() 결과 */
                    memcpy(&KR_Fmt.Data[4], (char *)cupdateout, cupdateoutl);
                    MsgLen = cupdateoutl + 4;
                    ItoAf(MsgLen, KR_Fmt.Header.BodyLength,    sizeof(KR_Fmt.Header.BodyLength));
                    SendLen += MsgLen;
                    break;
                case    TR_HSF:     /* handshake final — 키교환 완료 */
                    memcpy(KR_Fmt.Header.MsgType,  "SCHLIQ00105",      11);
                    memcpy(KR_Fmt.Data, "0000", 4);                /* INL_Handshake_Final() 결과 */
                    memcpy(&KR_Fmt.Data[4], (char *)cfinalout, cfinaloutl);
                    MsgLen = cfinaloutl + 4;
                    ItoAf(MsgLen, KR_Fmt.Header.BodyLength,    sizeof(KR_Fmt.Header.BodyLength));
                    SendLen += MsgLen;
                    break;
#endif
                case    TR_LOON:    /* 로그온 요청 */
                    memcpy(KR_Fmt.Header.MsgType,  "SCHLIQ00000",      11);
                    ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum,    sizeof (KR_Fmt.Header.MsgSeqNum));
                    memset(KR_Fmt.Data,    0x20,   41);                        /* Logon Size */
                    memcpy(KR_Fmt.Data,            LOGON_ID(D_K,P_K),  10);    /* 접속 ID */
                    /* PB: 비밀번호를 strlen으로 복사 (PA는 고정 30바이트) */
                    memcpy(&KR_Fmt.Data[10],       LOGON_PW(D_K,P_K),  strlen(LOGON_PW(D_K,P_K)));
                    memcpy(&KR_Fmt.Data[40],       "Y",                 1);    /* 복구 요청 */
                    MsgLen  = strlen(KR_Fmt.Data);
                    ItoAf(MsgLen, KR_Fmt.Header.BodyLength,    sizeof(KR_Fmt.Header.BodyLength));
                    SendLen += MsgLen;
                    break;
                case    TR_LINK:    /* 업무개시 요청 */
                    Log(USR_OK, "업무개시 요청");
                    memcpy(KR_Fmt.Header.MsgType,  "SCHOPQ10000",      11);
                    ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum,    sizeof (KR_Fmt.Header.MsgSeqNum));

                    /* 거부사유코드(4) + ME그룹시퀀스(2+110) */
                    memcpy(KR_Fmt.Data,    "0000", 4);

                    memset(&KR_Fmt.Data[4], '0', 2+110);    /* 초기값 0 */

                    if (INT_SEQ > 0)    /* 재접속시: 이전 ME그룹별 시퀀스 전송 */ {
                        /* meg-seq-log-fix: if_meg_seq[10]은 인덱스 0~9만 유효.
                           기존 로그는 (10) 범위초과 읽기 + 인덱스1 누락 + %d 11개/인자 10개
                           varargs 불일치였음 → 0~9 정확히 10개로 정정 */
                        Log(USR_OK, "INT_MEG_SEQ 0[%d] 1[%d] 2[%d] 3[%d] 4[%d] 5[%d] 6[%d] 7[%d] 8[%d] 9[%d]",
                                INT_MEG_SEQ(0), INT_MEG_SEQ(1), INT_MEG_SEQ(2), INT_MEG_SEQ(3), INT_MEG_SEQ(4),
                                INT_MEG_SEQ(5), INT_MEG_SEQ(6), INT_MEG_SEQ(7), INT_MEG_SEQ(8), INT_MEG_SEQ(9));

                        ItoAf(INT_MEG_SEQ(0), &KR_Fmt.Data[4+2], 11);
                        ItoAf(INT_MEG_SEQ(1), &KR_Fmt.Data[4+2+(1*11)], 11);
                        ItoAf(INT_MEG_SEQ(2), &KR_Fmt.Data[4+2+(2*11)], 11);
                        ItoAf(INT_MEG_SEQ(3), &KR_Fmt.Data[4+2+(3*11)], 11);
                        ItoAf(INT_MEG_SEQ(4), &KR_Fmt.Data[4+2+(4*11)], 11);
                        ItoAf(INT_MEG_SEQ(5), &KR_Fmt.Data[4+2+(5*11)], 11);
                        ItoAf(INT_MEG_SEQ(6), &KR_Fmt.Data[4+2+(6*11)], 11);
                        ItoAf(INT_MEG_SEQ(7), &KR_Fmt.Data[4+2+(7*11)], 11);
                        ItoAf(INT_MEG_SEQ(8), &KR_Fmt.Data[4+2+(8*11)], 11);
                        ItoAf(INT_MEG_SEQ(9), &KR_Fmt.Data[4+2+(9*11)], 11);
                }

                MsgLen  = strlen(KR_Fmt.Data);
                ItoAf(MsgLen, KR_Fmt.Header.BodyLength,    sizeof(KR_Fmt.Header.BodyLength));
                SendLen += MsgLen;
                break;
                case    RP_POLL:    /* 회선시험 응답 */
                    memcpy(KR_Fmt.Header.MsgType,      "SCHHER00000",  11);
                    ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum,    sizeof (KR_Fmt.Header.MsgSeqNum));
                    memcpy(KR_Fmt.Data,    "0000", 4);
                    MsgLen  = strlen(KR_Fmt.Data);
                    ItoAf(MsgLen,  KR_Fmt.Header.BodyLength,   sizeof(KR_Fmt.Header.BodyLength));
                    SendLen += MsgLen;
                    break;
                case    RP_LOOU:    /* 로그아웃 응답 */
                    memcpy(KR_Fmt.Header.MsgType,      "SCHLOR00000",  11);
                    ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum,    sizeof (KR_Fmt.Header.MsgSeqNum));
                    memcpy(KR_Fmt.Data,    "0000", 4);
                    MsgLen  = 0;
                    ItoAf(MsgLen,  KR_Fmt.Header.BodyLength,   sizeof(KR_Fmt.Header.BodyLength));
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
            Log_Out_Base();        /* fep_common 공유 함수: 로그아웃 응답 전송 */
        }   /* End of Log_Out ()    */

        /*************************************************************************
            End of Program (pb_1200_tr.c)
        *************************************************************************/
