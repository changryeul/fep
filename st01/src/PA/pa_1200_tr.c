#define     _GLOBAL

/*------------------------------------------------------------------------
 *  System  : Connect to KRX (한국거래소 접속)
 *  Author  : PSH
 *  Module  : 채권 KRX 주문응답/체결/장운영정보 수신 (TCP Receive)
 *  File    : pa_1200_tr.c
 *
 *  [개요]
 *  KRX(한국거래소)와 TCP로 연결하여 채권 주문응답, 체결결과,
 *  장운영정보를 수신하는 프로세스이다.
 *  KMAPv2.0 프로토콜을 사용하며, 82바이트 KRX_HEADER 기반 통신.
 *
 *  [빌드 옵션]  (make 시 -D 플래그로 선택)
 *  - A1201 : 채권 주문응답 수신 (회원처리호가 → pa_1201_mp로 전달)
 *  - A1202 : 채권 체결결과 수신 (체결 → pa_1401_mp로 전달)
 *  - A1601 : 장운영정보 #1 수신 (공개장운영 등)
 *  - A1602 : 장운영정보 #2 수신 (종목마감, 기준가 등)
 *
 *  [통신 절차]
 *  1단계) 로그온: TR_LOON → SCHLIQ00000 전송 → 응답 수신
 *  2단계) 업무개시: TR_LINK → SCHOPQ10000 전송 → 응답 수신
 *  3단계) 데이터 수신 루프: TCHODR00000(데이터), SCHHEQ(회선시험), SCHLOQ(로그아웃)
 *
 *  [fep_common 공유 함수 사용]
 *  - Device_Open_Logon() : TCP 접속 + 로그온 처리
 *  - Device_Read()       : KRX 메시지 수신 (길이 기반)
 *  - Device_Write()      : KRX 메시지 송신
 *  - Device_Close_Base() : 소켓 종료 + 상태 초기화
 *  - Line_Change()       : 회선 절체 (MAIN ↔ BACKUP)
 *  - Time_Out_Disconnect(): 타임아웃시 회선시험 응답 전송
 *  - Log_Out_Base()      : 로그아웃 응답 전송
 *
 *  [PA vs PB 차이]
 *  - PA: 암호화 없음 (Handshake는 항상 NOTOK 리턴)
 *  - PB: INISAFE 암호화 사용 (Handshake로 키교환 수행)
 *------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "krx_trcode.h"
#include    "ifaddrs.h"
#include    "fep_common.h"
/*
#include    "fep_fepj.h"
#include    "krx_struct.h"
*/

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
 *  - A1201/A1202 : 채권주문 응답/체결 (Header 82 + Data 최대 318)
 *  - A1601       : 장운영정보 #1 (Data 최대 100)
 *  - A1602       : 장운영정보 #2 (Data 최대 150)
 *------------------------------------------------------------------------*/
#if defined(A1201) || defined(A1202)
#define     DATA_SIZE       400           /* Header(82) + Data(318)_가변포함 */
#elif defined A1601
#define     DATA_SIZE       100           /* Data(100)_가변포함 */
#elif defined A1602
#define     DATA_SIZE       150           /* Data(148)_가변포함 */
#endif
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DEVICE_TIME     3  * 1000                       /*  3 sec — 접속 재시도 간격 */
#define     MAIN_TIME       15 * 1000       /* Heart Beat 간격 15 sec */
#define     FOREVER_TIME    30 * 1000                       /* 30 sec — 대기 타임아웃 */

#define     FIFO_EVENT      0           /* poll 이벤트: FIFO 입력 (데몬 제어) */
#define     SOCKET_EVENT    1           /* poll 이벤트: TCP 소켓 수신 (KRX 데이터) */

#define     MAX_CNT         1
#define     RESP_GAP        1000

#define     NO_TIME         "0830"      /* 채권정규시장은 09시 ~ 15시 까지, 시간외 없음 */

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
FILE_BUFF_FORMAT            W_Fmt;          /* FIFO 쓰기용 버퍼 (BUFF_RW_HEAD + Data + LF) */
KRX_NOTE_ALL_JUMUN_Q_FMT    J_Q_Fmt;        /* KRX 주문포맷 (max 6건) (82+300) */
KRX_NOTE_JUMUN_R_FMT        J_R_Fmt;        /* KRX 세션 응답 포맷 (82+4+11) */
KRX_NOTE_JUMUN_S_FMT        Re_W_Fmt;       /* 내부 주문응답 송신 포맷 */
KRX_HEADER                  Header_Fmt;     /* KRX Header (82바이트) */
KRX_R_SESSION_FMT           KR_Fmt;         /* KRX 세션 포맷 (82+116) */
FILE_DATA_HEAD              File_Data_Head; /* 파일 데이터 헤더 (20바이트) */
struct pollfd       Poll[2];                /* poll 배열: [0]=FIFO, [1]=TCP소켓 */

/*
 * fep_common 패턴: FmtPtr에 자기 모듈의 세션 포맷을 연결
 * - PA 모듈은 KR_Fmt (KRX_R_SESSION_FMT) 사용
 * - PB 모듈도 동일한 구조 사용
 */
void *FmtPtr = (void *)&KR_Fmt;

/*
 * Handshake 스텁 — PA는 암호화 미사용이므로 항상 NOTOK 리턴
 * (PB는 sub/fep_encrypt.c의 실제 Handshake 함수 사용)
 */
int     Handshake(void) { return (NOTOK); }

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_1200_TR(void);          /* 메인 루프 */
void    Init_Parameters(void);     /* 초기화 */
void    Socket_Event_Rtn(void);    /* TCP 수신 이벤트 처리 */
void    Device_Open(int);          /* KRX 접속 (로그온/업무개시) */
void    Time_Out_Rtn(void);        /* 타임아웃 처리 */
int     Analyze_Data(void);        /* 수신 데이터 분석 → TR 코드 판별 */
void    Write_Data(int);           /* 수신 데이터를 FIFO로 전달 */
int     Make_Send_Msg(int);        /* 송신 메시지 조립 (KMAPv2.0) */
void    Log_Out(void);             /* 로그아웃 처리 */

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    Init_Proc(argc, argv);         /* 프로세스 공통 초기화 */
    PA_1200_TR();                  /* 메인 처리 루프 */
    Exit_Process();                /* 프로세스 종료 */
}   /* End of main ()   */

/*------------------------------------------------------------------------
 *  PA_1200_TR — 메인 이벤트 루프
 *
 *  [처리 흐름]
 *  1) OpenFlag OFF → 로그온 시도 (TR_LOON)
 *  2) LogOnFlag ON, OpenFlag OFF → 업무개시 시도 (TR_LINK)
 *  3) OpenFlag ON → 데이터 수신 대기 (poll)
 *  4) poll 이벤트 발생 → FIFO/소켓 분기 처리
 *
 *  [접속 재시도]
 *  ConnectRetryCnt가 3회 이상이면 Line_Change()로 회선 절체 후 재시도
 *------------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
void    PA_1200_TR(void)
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
            else                                /* 로그온은 된 경우 → 업무개시 시도 */ {
                Device_Open(TR_LINK);

                if (OpenFlag == OFF)            /* 업무개시 실패 */ {
                    PollCnt = 1;
                    TimeOut = DEVICE_TIME;      /* 3초 */
                }
                else                            /* 업무개시 성공 → 데이터 수신 모드 */ {
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
}   /* End of PA_1200_TR () */

/*************************************************************************
 *  Init_Parameters — 프로세스 변수 초기화
 *
 *  - 상태 플래그를 OFF로 초기화
 *  - TCP2 설정에서 KRX IP/Port 정보 로딩
 *  - 회선 상태(MAIN/BACKUP) 설정
 *  - ApType 생성: 실행파일명에서 추출 (예: "PA1200TR")
 *  - Poll[0]에 FIFO(데몬 신호) fd 설정
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

    S_K = TCP2_LINE_GU;                 /* 현재 사용할 회선 (MAIN 또는 BACKUP) */

    /* KRX 접속 IP 주소를 문자열로 조립 */
    sprintf(IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
            TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    Log(TCP_OK, "IP[%s] port[%d]", IpAddr, PORT_NO);

    /* 프로세스 상태를 ON으로 (MAIN, BACKUP 모두) */
    TCP2_PROC_ST1(MAIN) = ON;
    TCP2_PROC_ST1(BACKUP) = ON;

    /* 회선 상태를 OFF로 초기화 */
    TCP2_LINE_ST1(MAIN) = OFF;
    TCP2_LINE_ST1(BACKUP) = OFF;

    /* ApType 생성: 실행파일명에서 모듈코드 추출 */
    sprintf(ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));     /* 소문자 → 대문자 변환 */

    /* Poll[0] = FIFO (데몬의 제어 신호를 받는 파이프) */
    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;

    return;
}   /* End of Init_Parameters ()    */

/*************************************************************************
 *  Socket_Event_Rtn — TCP 소켓 수신 이벤트 처리
 *
 *  [처리 흐름]
 *  1) Device_Read()로 KRX 메시지 수신
 *  2) 82바이트 KRX_HEADER를 Header_Fmt에 복사
 *  3) Analyze_Data()로 메시지 유형 판별 (RP_DATA/RP_POLL/RP_STOP)
 *  4) RP_DATA일 경우:
 *     a) 시퀀스 검증 (INT_SEQ+1 == FirstSeq)
 *     b) ME그룹 시퀀스 검증
 *     c) TR코드별 분기 → Write_Data()로 후속 프로세스에 전달
 *  5) RP_POLL: 회선시험 (무처리)
 *  6) RP_STOP: 로그아웃 (무처리)
 *************************************************************************/
/*----------------------------------------------------------------------*/
void    Socket_Event_Rtn(void)
/*----------------------------------------------------------------------*/
{
    int     rval, rt, cnt, r_meg_no, r_meg_seq;
    char    t_time[12];

    /* KRX 메시지 수신 (fep_common 공유 함수) */
    rval = Device_Read();

    /* 수신 실패 시 리턴 (Device_Read 내부에서 로그 처리) */
    if (rval < 0)
        return;

    DeviceSendFlag = OFF;
    ErrCd = 0;

    /* 수신 데이터의 KRX 헤더(82바이트) 추출 */
    memset(&Header_Fmt, 0, KRX_HEAD_LEN);
    memcpy(&Header_Fmt, DataBuff, KRX_HEAD_LEN);

    /* 메시지 유형 분석 → ReTrCode에 RP_DATA/RP_POLL/RP_STOP 저장 */
    ReTrCode = Analyze_Data();

    /* ************************************ */
    /* 여기서는 RP_DATA, RP_POLL 2개만 체크 */
    /* ************************************ */
    switch (ReTrCode) {
        case    RP_DATA:
            /* ================================================ */
            /* 시퀀스 검증 — 모든 데이터는 INT_SEQ+1로 수신      */
            /* 불일치시 로그 남기고 프로세스 종료 (운영자에게 알림) */
            /* ================================================ */
            if (INT_SEQ+1 != FirstSeq) {
                TCP2_LINE_ST = OpenFlag = OFF;

                Log(USR_ERROR, "PR_DATA recv:Header invalid KRX SEQ [%d] INT_SEQ[%d]",
                        FirstSeq, INT_SEQ);

                sleep(3);                          /* 3초 후 종료 */
                Exit_Process();                    /* 종료, 운영자에게 알리기 위해서 */
        }

        /* ================================================ */
        /* ME그룹 시퀀스 검증 및 저장                          */
        /* KRX는 ME그룹별로 별도 시퀀스를 관리함               */
        /* (대량 데이터도 우선 받아서 seq처리는 함)             */
        /* ================================================ */
        {
            KRX_MSG_COMMON *msg = (KRX_MSG_COMMON *)DataBuff;
            r_meg_no  = AtoIf(msg->Body.Megrp_no, KRX_MEGRPNO_LEN);        /* ME그룹번호 */
            r_meg_seq = AtoIf(msg->Body.DataSeq,   KRX_DATASEQ_LEN);       /* ME그룹 일련번호 */

            if (INT_MEG_SEQ(r_meg_no-1) + 1 == r_meg_seq)
                INT_MEG_SEQ(r_meg_no-1) = r_meg_seq;    /* 정상: 시퀀스 갱신 */
            else {
                TCP2_LINE_ST = OpenFlag = OFF;

                Log(USR_ERROR, "PR_DATA recv:Data invalid r_meg_no [%d], r_meg_seq[%d] !=  INT_MEG_SEQ[%d]",
                        r_meg_no, r_meg_seq, INT_MEG_SEQ(r_meg_no-1));

                sleep(3);                          /* 3초 후 종료 */
                Exit_Process();                    /* 종료, 운영자에게 알리기 위해서 */
            }

            /* ============================================================ */
            /* A1201/A1202: 채권 주문응답/체결 수신                          */
            /* ============================================================ */
#if defined(A1201) || defined(A1202)
            /* TR 코드별 분기 — IS_TR/IS_TR_PREFIX 매크로로 판별 */
            if (IS_TR(msg, TR_BOND_EXECUTION))                                  /* 채권 체결결과(일반,조성) 317바이트 */ {
                Write_Data(2);         /* pa_1401_mp로 전달 (체결 처리) */
            }
            else
                if (IS_TR_PREFIX(msg, TR_BOND_ORDER_RESP)   ||                  /* 채권 일반처리호가 291 (1신규,2정정,3취소) */
                        IS_TR_PREFIX(msg, TR_BOND_MM_RESP)      ||                  /* 채권 조성처리호가 295 (1신규,2정정,3취소) */
                        IS_TR_PREFIX(msg, TR_KILL_SWITCH)       )                   /* Kill Switch 응답 161 (1정상,2거부) */ {
                Write_Data(1);         /* pa_1201_mp로 전달 (응답 처리) */
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
            /* A1601/A1602: 장운영정보 수신                                  */
            /* ============================================================ */
#elif defined(A1601) || defined(A1602)

            /*
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
            */
            /* TR 코드별 분기 */
            if (IS_TR(msg, TR_MARKET_OPR_FULL))                                 /* 공개장운영, 91바이트 */ {
                Write_Data(1);         /* pa_1601_mp로 전달 */
            }
            else                                                                        /* 나머지는 2번으로, 처리안함 */
            if (IS_TR_PREFIX(msg, TR_MARKET_OPR_PREFIX))                        /* 장운영 관련 TR */ {
                Write_Data(2);         /* pa_1602_mp로 전달 */
            }
            else
                if (IS_TR(msg, TR_IF_END_MARKET))                               /* 장운영 인터페이스 종료 153 */ {
                Write_Data(2);         /* pa_1602_mp로 전달 */
                Log(USR_OK, "장운영 인터페이스 종료");
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
#endif
        }   /* end of KRX_MSG_COMMON *msg scope */

        break;
        case    RP_POLL:        /* 회선시험 요청 — 별도 처리 없음 */
        case    RP_STOP:        /* 로그아웃 요청 — 별도 처리 없음 */
            break;
        default:
            break;
    }

    return;
}   /* End of Socket_Event_Rtn ()   */

/*************************************************************************
 *  Device_Open — KRX 접속 처리
 *
 *  [TR_LOON: 로그온]
 *  - Device_Open_Logon()으로 TCP 접속 + 로그온 수행
 *    (0=PA모드: 암호화 없음)
 *
 *  [TR_LINK: 업무개시]
 *  - Make_Send_Msg()로 SCHOPQ10000 메시지 조립 후 전송
 *  - 응답 수신: SCHOPR00000이면 성공
 *  - 거부사유코드 "0000"이면 정상 → OpenFlag ON
 *  - 시퀀스 불일치시 Exit_Process() (운영자에게 알림)
 *************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Open(int tr_code)
/*----------------------------------------------------------------------*/
{
    int     rt, rval;

    if (tr_code == TR_LOON) {
        /* 0=PA모드 (암호화 없음), 0=tcp2 인덱스 */
        Device_Open_Logon(0, 0);
    }
    else if (tr_code == TR_LINK) {
        while (START_S != END) {
            /* 업무개시 요청 메시지 조립 및 전송 */
            rt = Make_Send_Msg(TR_LINK);
            memset(DataBuff, 0, sizeof (DataBuff));
            memcpy(DataBuff, &KR_Fmt, SendLen);
            Device_Write();
            Log(USR_OK, "send LINK request");

            /* 업무개시 응답 수신 */
            rval = Device_Read();
            if (rval < 0 ||
                    memcmp(((KRX_HEADER *)DataBuff)->MsgType, "SCHOPR00000", 11) != 0) {
                Log(USR_ERROR, "LINK response recv error");
                close(Sockfd);
                break;
            }

            memset(&KR_Fmt, 0, sizeof (KRX_R_SESSION_FMT));
            memcpy(&KR_Fmt, DataBuff, RecvLen);

            /* 응답의 시퀀스 번호 추출 */
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

                /* 업무개시 성공 */
                TCP2_NET_STA(S_K) = ON;
                TCP2_LINE_ST = OpenFlag = ON;
                ConnectRetryCnt = 0;

                Log(TCP_OK, "LINK OK");

                break;
            }
            else {
                /* 거부사유코드가 0000이 아닌 경우 → 에러 */
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

/*----------------------------------------------------------------------*/
void    Device_Close(void)
/*----------------------------------------------------------------------*/
{
    Device_Close_Base();       /* fep_common 공유 함수: 소켓 종료 + 상태 초기화 */
}   /* End of Device_Close ()   */

/*************************************************************************
 *  Time_Out_Rtn — 타임아웃 처리
 *
 *  PollCnt=1 : 접속 대기 중 → 로그만 출력
 *  PollCnt=2,3 : 데이터 수신 대기 중 → Time_Out_Disconnect()로 회선시험 응답
 *************************************************************************/
/*----------------------------------------------------------------------*/
void    Time_Out_Rtn(void)
/*----------------------------------------------------------------------*/
{
    memset(DataBuff, 0, sizeof (DataBuff));

    switch (PollCnt) {
        case    1:      /* 접속 대기 중 — 회선 미연결 상태 */
            if (TimeOut == FOREVER_TIME)
                Log(USR_OK, "poll timeout <%d>:NSTAT[%d]",
                        INT_SEQ, TCP2_NET_STA(S_K));
            break;
        case    2:      /* 데이터 수신 대기 중 → 회선시험 응답 전송 */
        case    3:
            Time_Out_Disconnect("FOT");
            break;
        default:
            break;
    }
}   /* End of Time_Out_Rtn ()   */

/*************************************************************************
 *  Analyze_Data — 수신 데이터의 메시지 유형 판별
 *
 *  KRX_HEADER의 MsgType(11바이트)를 비교하여 분류:
 *  - SCHHEQ00000 → HeartBeat(회선시험요청) → RP_POLL
 *  - SCHLOQ00000 → LogOut(로그아웃요청)   → RP_STOP
 *  - TCHODR00000 → Data(업무데이터)        → RP_DATA
 *
 *  참고: 거부사유코드는 DataBuff[KRX_HEAD_LEN] 위치 (4바이트)
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
    else if (memcmp(Header_Fmt.MsgType, "TCHODR00000", 11) == 0)        /* Data(업무데이터) */ {
        Log(USR_OK, "DATA수신(TCHODR00000): <%d:%d> 거부사유코드[%4.4s]",
                FirstSeq, INT_SEQ, &DataBuff[KRX_HEAD_LEN]);
        return (RP_DATA);
    }

    Log(USR_ERROR, "Analyze_Data: unknown MsgType[%.11s]", Header_Fmt.MsgType);
    return (RP_STOP);
}   /* End of Analyze_Data ()   */

/*************************************************************************
 *  Write_Data — 수신 데이터를 FIFO(named pipe)로 전달
 *
 *  [p_flag에 따른 전달 대상]
 *  - A1201: p_flag=1 → pa_1201_mp (주문응답), p_flag=2 → pa_1401_mp (체결)
 *  - A1601: p_flag=1 → pa_1601_mp, p_flag=2 → pa_1602_mp
 *
 *  [데이터 구성]
 *  FILE_BUFF_FORMAT = BUFF_RW_HEAD + FILE_DATA_HEAD(20) + Data + LF
 *  - BUFF_RW_HEAD : If_Seq, ApType, ResponseCode, RecvTime 등
 *  - FILE_DATA_HEAD : Length, DataSeq, ResponseCode, LineFlag
 *  - Data : KRX 헤더 이후 본문 (DataBuff[82]부터)
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
    Get_MicroTime(m_time);             /* 마이크로초 단위 현재 시각 */

    /* BUFF_RW_HEAD 영역 설정 */
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

    /* 본문 데이터 복사 — KRX 헤더(82바이트) 이후 데이터만 추출 */
    /* 1201 : 4+11 + 회원처리호가, 거래소에서 4+11+회원처리호가 수신 */
    /* 1401 : 체결정보 체결전문,   거래소에서 체결만 수신 */
    /* 1601/1602 : 장운영정보수신전문, 거래소에서 장운영정보만 수신 */
    memcpy(W_Fmt.Data,         &DataBuff[KRX_HEAD_LEN],    RecvLen - KRX_HEAD_LEN);
    W_Fmt.LineFeed[0] = '\n';

    /* FIFO에 1건 쓰기 (p_flag * 10 = 출력 파일 인덱스) */
    rt = F_W(p_flag * 10, (void *)&W_Fmt, 1);

    if (rt != 1) {
        Log(SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,p_flag-1));
        Exit_Process();
    }

    Log(USR_OK, "file write[%s:%d:%d]",
            OFN(D_K,P_K,p_flag-1), OFW(D_K,P_K,p_flag-1,0), rt);

    /* 시퀀스 증가 */
    INT_SEQ += rt;

    Set_TR_Time();

    return;
}   /* End of Write_Data () */

/*************************************************************************
 *  Make_Send_Msg — KMAPv2.0 송신 메시지 조립
 *
 *  [공통 헤더 구성] (82바이트)
 *  - BeginString  : "KMAPv2.0" (8바이트)
 *  - BodyLength   : 본문 길이
 *  - MsgSeqNum    : 시퀀스 번호
 *  - SenderCompID : 회원사 코드 (TCP_COMPANY)
 *  - SendingTime  : 전송 시각
 *  - Encrypt      : "N" (PA는 암호화 미사용)
 *
 *  [TR코드별 메시지]
 *  - TR_LOON : 로그온 요청 (SCHLIQ00000) — ID(10)+PW(30)+복구요청("Y")
 *  - TR_LINK : 업무개시 요청 (SCHOPQ10000) — 거부사유(4)+ME그룹시퀀스(2+110)
 *  - RP_POLL : 회선시험 응답 (SCHHER00000) — 거부사유(4)
 *  - RP_LOOU : 로그아웃 응답 (SCHLOR00000) — 거부사유(4)
 *************************************************************************/
/*----------------------------------------------------------------------*/
int     Make_Send_Msg(int tr_code)
/*----------------------------------------------------------------------*/
{
    int     rt;
    char    d_time[18];

    rt = 0;
    memset(&KR_Fmt, 0, sizeof (KRX_R_SESSION_FMT));

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
        case    TR_LOON:    /* 로그온 요청 */
            memcpy(KR_Fmt.Header.MsgType,  "SCHLIQ00000",      11);
            ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum,    sizeof (KR_Fmt.Header.MsgSeqNum));
            memset(KR_Fmt.Data,    0x20,   41);                        /* Logon Size = 41바이트 */
            memcpy(KR_Fmt.Data,            LOGON_ID(D_K,P_K),  10);    /* 접속 ID (10바이트) */
            memcpy(&KR_Fmt.Data[10],       LOGON_PW(D_K,P_K),  30);    /* 비밀번호 (30바이트) */
            memcpy(&KR_Fmt.Data[40],       "Y",                 1);    /* 복구 요청 플래그 */
            MsgLen  = strlen(KR_Fmt.Data);
            ItoAf(MsgLen, KR_Fmt.Header.BodyLength,    sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_LINK:    /* 업무개시 요청 */
            memcpy(KR_Fmt.Header.MsgType,  "SCHOPQ10000",      11);
            ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum,    sizeof (KR_Fmt.Header.MsgSeqNum));

            /* DATA Set — 거부사유코드(4) + ME그룹시퀀스(2+110) */
            memcpy(KR_Fmt.Data,    "0000", 4);     /* 거부사유코드 */

            if (INT_SEQ == 0)   /* 최초접속시 모두 0으로 초기화 */ {
                memset(&KR_Fmt.Data[4], '0', 2+110);
        }
        else                /* 재접속시 이전 ME그룹별 시퀀스 전송 */ {
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
    End of Program (pa_1200_tr.c)
*************************************************************************/
