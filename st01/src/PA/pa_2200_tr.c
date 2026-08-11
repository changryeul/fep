#define     _GLOBAL
/*========================================================================
 *  Module  : IMECO 파생 주문응답/체결 수신 (TCP client)
 *  File    : pa_2200_tr.c
 *  -----------------------------------------------------------------
 *  KRX IMECO(금융파생상품) 서버에 TCP 클라이언트로 접속하여
 *  회원처리호가(주문응답) 및 체결 데이터를 수신하는 프로세스.
 *
 *  IMECO 프로토콜 (KRX 표준과 다른 별도 프로토콜):
 *    헤더: IMECO_TCP_MESSAGE (20바이트 헤더 + 최대 4280바이트 데이터)
 *    MsgType: L=LIOK(로그인), H=POLL(하트비트), D=DATA(데이터)
 *    송신:   L=LINK(로그인요청), H=POOK(하트비트응답)
 *
 *  수신 데이터 종류 (DATA 전문의 첫 바이트):
 *    C = 회원처리호가 확인 (Confirm)
 *    R = 회원처리호가 거부 (Reject)
 *    A = 회원처리호가 자동취소 (Auto Cancel)
 *    I = 실시간가격제한 즉시변경
 *    P = 실시간가격제한 이후변경
 *    F = 체결/부분체결 (Fill/Partial Fill)
 *    T = 체결정보 (Trade)
 *
 *  오류코드: 0001~0102(KRX 시스템), E001~E006(세션), E101~E999(주문검증)
 *========================================================================*/

/*------------------------------------------------------------------------
 *  헤더 파일
 *------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "fep_common.h"

/*------------------------------------------------------------------------
 *  상수 및 매크로 정의
 *------------------------------------------------------------------------*/
#define     TCP_TIME_OUT    35      /* TCP 수신 타임아웃 기본값 (초) */

/*
 * SAM_USE 여부에 따라 출력 대상이 달라짐
 */
#ifdef  SAM_USE
#define     WR_CNT      OFW_CNT(0,0)
#define     WR_CNT2     OFW_CNT(1,0)
#define     OUT_NAME    OFN(D_K,P_K,0)
#define     OUT_NAME2   OFN(D_K,P_K,1)
#else
#define     WR_CNT      ODW_CNT(0,0)
#define     WR_CNT2     ODW_CNT(1,0)
#define     OUT_NAME    ODN(D_K,P_K,0)
#define     OUT_NAME2   ODN(D_K,P_K,1)
#endif
#define     DATA_SIZE       200
#include    "buf_struct.h"

/*------------------------------------------------------------------------
 *  전역 변수
 *------------------------------------------------------------------------*/
int     Sockfd;                 /* TCP 소켓 파일디스크립터 */
int     PortNo;                 /* 접속 대상 포트 번호 */
int     DataCnt;                /* 수신 데이터 건수 누적 */
int     PktType;                /* 현재 전송할 패킷 타입 */
int     DataSize;               /* 1건당 데이터 크기 */
int     MaxCnt;                 /* 1패킷 최대 건수 */
int     RetryCnt;               /* 접속 재시도 횟수 */
char    ApType[10];             /* 업무식별코드 */
char    IpAddr[20];             /* 접속 대상 IP 주소 */
char    ErrCd[8];               /* 오류코드 */
char    RecvPkt[TCP_BUFF_MAX_LEN];  /* TCP 수신 버퍼 */
char    SendPkt[TCP_BUFF_MAX_LEN];  /* TCP 송신 버퍼 */

FILE_BUFF_FORMAT    W_Fmt;          /* FIFO 쓰기 버퍼 */
IMECO_TCP_MESSAGE   *R_Pkt = (IMECO_TCP_MESSAGE *)RecvPkt;  /* 수신 패킷 (20+4280) */
IMECO_TCP_MESSAGE   *S_Pkt = (IMECO_TCP_MESSAGE *)SendPkt;  /* 송신 패킷 (20+4280) */
FILE_DATA_HEAD      File_Data_Head;                         /* 파일 데이터 헤더 (20바이트) */

/*------------------------------------------------------------------------
 *  함수 프로토타입
 *------------------------------------------------------------------------*/
void    PA_2200_TR(void);
void    Init_Parameters(void);
void    Connection(void);
int     Receive_Packet(void);
void    Send_Packet(void);
int     Check_Header(void);
int     Write_Data(void);
void    Register_Signal(void);
void    Catch_Signal(int);

/*======================================================================
 *  main: 프로세스 진입점
 *======================================================================*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);

    PA_2200_TR();

    TCP2_CON_STA = OFF;
    close(Sockfd);
    Exit_Process();
}   /* End of main () */

/*======================================================================
 *  PA_2200_TR: 메인 처리 루틴
 *  ------------------------------------------------------------------
 *  1) 휴일 체크 (HOLIDAY_CHECK)
 *  2) LINK(로그인) → LIOK 응답 대기
 *  3) 데이터 수신 루프: DATA/POLL 처리
 *======================================================================*/
void    PA_2200_TR(void) {
    int     rt;

    Register_Signal();
    Init_Parameters();

#ifdef  HOLIDAY_CHECK
    /*--------------------------------------------------------------
     *  휴일 체크: 토(6)/일(0)요일이면 60초 sleep
     *--------------------------------------------------------------*/
    while (1) {
        char    t_time[12], dt[20];
        time_t  t = time(NULL);
        struct  tm tm, *tp;

        Get_Time(t_time);

        memset(dt, 0, sizeof (dt));
        sprintf(dt, "%.4s-%.2s-%.2s %.2s:%.2s:%.2s", DAEMON(D_K).date,
                DAEMON(D_K).date+4, DAEMON(D_K).date+6, t_time, t_time+2, t_time+4);
        strptime(dt, "%Y-%m-%d %H:%M:%S", &tm);
        tp  = localtime(&t);

        if  (tp->tm_wday == 0 || tp->tm_wday == 6) {
            Log(USR_OK, "it's weekend. sleeping...[%d]", tp->tm_wday);
            sleep(60);
            continue;
        }
        else
            break;
    }
#endif

    /* LINK(로그인) 요청 → LIOK 응답 대기 */
    PktType = T_LINK;
    Connection();
    Send_Packet();

    while (1) {
        rt  = Receive_Packet();

        if  (rt == FAIL)
            continue;
        else    if (rt == NOTOK)
            return;
        else
            break;
    }

    /* 데이터 수신 루프 */
    while (START_S < JOB_END) {
        Stat_Save();

        rt  = Receive_Packet();

        if  (rt == NOTOK)
            break;
        else    if (rt == FAIL)
            continue;
    }

    return;
}   /* End of PA_2200_TR () */

/*======================================================================
 *  Init_Parameters: 전역 변수 초기화
 *======================================================================*/
void    Init_Parameters(void) {
    SYS_NO = 0;
    S_K = 0;
    RetryCnt = 0;
    DataCnt = 0;

    if (TIME_OUT == 0)
        TIME_OUT    = TCP_TIME_OUT;

    Log(USR_OK, "TIME_OUT[%d]", TIME_OUT);

    sprintf(ApType, "%-2.2s%-4.4s%c%c",
            _Exe_Name,  _Exe_Name+3, _Exe_Name[8], _Exe_Name[9] == 's' ? 'R' : 'S');
    LtoU(ApType, strlen(ApType));

#ifdef  SAM_USE
    DataSize = OFS(D_K,P_K,0);
#else
    DataSize = ODS(D_K,P_K,0);
#endif

    MaxCnt = 1;     /* IMECO: 1패킷 1건 고정 */

    sprintf(IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
            TCP2_IP2(D_K,P_K,S_K),  TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    PortNo = TCP2_PORT_NO;

    return;
}   /* End of Init_Parameters () */

/*======================================================================
 *  Connection: IMECO 서버에 TCP 접속
 *  ------------------------------------------------------------------
 *  pa_1600_tr.c와 달리 접속 실패시 즉시 종료 (재시도 없음)
 *======================================================================*/
void    Connection(void) {
    int     rt;

    while (START_S != END) {
        Sockfd  = Socket();

        if  (Sockfd < 0) {
            TCP2_LINE_ST = OFF;
            Log(TCP_ERROR, "socket fail:Sockfd[%d] {%d:%s}",
                    Sockfd, SYS_NO, SYS_STR);
            sleep(5);
            continue;
        }

        TCP2_LINE_ST    = ON;
        Log(USR_OK, "socket created:Sockfd[%d]", Sockfd);
        Log(USR_OK, "connecting to %s:%d", IpAddr, PortNo);

        rt  = Connect(Sockfd, IpAddr, PortNo);

        if  (rt < 0) {
            TCP2_CON_STA = OFF;
            TCP2_LINE_ST = OFF;
            TCP2_NET_STA(S_K) = OFF;
            Log(TCP_ERROR, "connect fail {%d:%s}", SYS_NO, SYS_STR);
            close(Sockfd);
            sleep(5);

            Exit_Process();    /* 접속 실패시 즉시 종료 */
        }

        TCP2_CON_STA        = ON;
        TCP2_NET_STA(S_K)   = ON;
        Log(USR_OK, "connected to %s:%d", IpAddr, PortNo);
        break;
    }

    Set_Socket_Linger(Sockfd);

    return;
}   /* End of Connection () */

/*======================================================================
 *  Receive_Packet: IMECO 패킷 수신
 *  ------------------------------------------------------------------
 *  Select_Receive_Imeco: IMECO 전용 수신 함수 (헤더 20바이트 기반)
 *
 *  수신 전문 종류별 처리:
 *    L(LIOK) = 로그인 응답 → 즉시 반환
 *    H(POLL) = 하트비트    → POOK 응답 송신
 *    D(DATA) = 실데이터    → Write_Data로 FIFO/DSHM 기록
 *======================================================================*/
int     Receive_Packet(void) {
    int     rt;
    char    error_flag;

    error_flag = OFF;
    memset(RecvPkt, 0, sizeof (RecvPkt));

    rt = Select_Receive_Imeco(Sockfd, RecvPkt);

    if (rt == OK)
        return  (NOTOK);
    else if (rt == NOTOK) {
        if  (SYS_NO == EINTR)
            return (FAIL);

        return  (NOTOK);
    }

    Log(TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen(RecvPkt), INT_SEQ);

    /* 헤더 유효성 검증 (MsgType, ResponseCode 등) */
    rt = Check_Header();

    if (rt != OK)
        return  (NOTOK);

    /*
     * MsgType별 분기:
     *   L = LIOK (로그인 응답)
     *   H = POLL (하트비트 → POOK 응답)
     *   D = DATA (주문응답/체결 → Write_Data)
     */
    if (memcmp(R_Pkt->Head.MsgType, "L", 1) == 0) {
        return (OK);
    }
    else if (memcmp(R_Pkt->Head.MsgType, "H", 1) == 0) {
        PktType = T_POOK;
        Send_Packet();
    }
    else if (memcmp(R_Pkt->Head.MsgType, "D", 1) == 0) {
        rt  = Write_Data();
        if  (rt != OK) {
            Log(USR_ERROR, "Write_Data Error");
            sleep(3);
            return (NOTOK);
        }
    }
    else {
        Log(USR_ERROR, "Else Case Receive R_Pkt->MsgType[%1.1s]", R_Pkt->Head.MsgType);
        return  (NOTOK);
    }

    return (OK);
}   /* Receive_Packet () */

/*======================================================================
 *  Check_Header: IMECO 수신 패킷 헤더 유효성 검증
 *  ------------------------------------------------------------------
 *  검증 항목:
 *    1) MsgType: L/H/D 중 하나인지
 *    2) DATA 전문: 첫 바이트가 C/R/A/I/P/F/T 중 하나인지
 *    3) Length: IMECO_HEAD_LEN(20) 이상이고 수신길이와 일치하는지
 *    4) ResponseCode: "0000" 아니면 오류 메시지 출력
 *
 *  IMECO 오류코드 (상세):
 *    0001~0020: KRX 시스템 오류 (인증/시퀀스/길이/암호화 등)
 *    0090:      KRX 시스템 장애
 *    0101~0102: 장시간 전후 거부
 *    E001~E006: 세션 오류 (인증/MAC/메시지타입 등)
 *    E101~E127: 주문 검증 오류 (주문번호/수량/가격/종목 등)
 *    E999:      알수없는 오류
 *======================================================================*/
int     Check_Header(void) {
    int     i, next, rt, val;
    char    U_ErrMsg[1024];

    memset(U_ErrMsg, 0, sizeof(U_ErrMsg));

    /* 1. MsgType 검증: L(로그인)/H(하트비트)/D(데이터) */
    if ( (memcmp(R_Pkt->Head.MsgType, "L", 1) != 0)   &&
            (memcmp(R_Pkt->Head.MsgType, "H", 1) != 0)   &&
            (memcmp(R_Pkt->Head.MsgType, "D", 1) != 0)       ) {
        SLog(USR_ERROR, "MsgType The value is invalid [%1.1s:%s]", R_Pkt->Head.MsgType, R_Pkt);
        return (NOTOK);
    }

    /* 2. DATA 전문: 트랜잭션 코드 유효성 (C/R/A/I/P/F/T) */
    if ( memcmp(R_Pkt->Head.MsgType, "D", 1) == 0 ) {
        if (memcmp(R_Pkt->Data, "C", 1) != 0   &&      /* C=확인, R=거부, A=자동취소 */
                memcmp(R_Pkt->Data, "R", 1) != 0   &&
                memcmp(R_Pkt->Data, "A", 1) != 0   &&
                memcmp(R_Pkt->Data, "I", 1) != 0   &&      /* I=실시간가격제한(즉시) */
                memcmp(R_Pkt->Data, "P", 1) != 0   &&      /* P=실시간가격제한(이후) */
                memcmp(R_Pkt->Data, "F", 1) != 0   &&      /* F=체결/부분체결 */
                memcmp(R_Pkt->Data, "T", 1) != 0   )       /* T=체결정보 */ {
            SLog(USR_ERROR, "Transaction The value is invalid [%1.1s:%s]", R_Pkt->Data, R_Pkt);
            return (NOTOK);
        }
    }

    /* 3. Length 검증 */
    if (strlen(RecvPkt) < IMECO_HEAD_LEN || strlen(RecvPkt) != rt) {
        SLog(USR_ERROR, "invalid length[%d:%d,%d]", strlen(RecvPkt), TCP_HEAD_LEN, rt);
        return (NOTOK);
    }

    /* 4. ResponseCode 검증: "0000" 이외는 오류 */
    if (memcmp(R_Pkt->Head.ResponseCode, "0000", sizeof(R_Pkt->Head.ResponseCode)) != 0) {
        /*------------------------------------------------------
         *  KRX 시스템 오류 (0001~0102)
         *------------------------------------------------------*/
        if (memcmp(R_Pkt->Head.ResponseCode, "0001", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "User validation(ID, Password) error");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0002", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "Session message sequence error");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0003", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "Member ID(header) error");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0004", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "Data sequence number error");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0005", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "Data count error");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0008", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "Session message type error. If undefined type is set. e.g. Login(SCHLIQ00000)");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0010", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "Message length error");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0011", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "Encryption or decryption error");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0012", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "Null data in message body");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0013", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "Order is not allowed. new(existing 0103)");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0014", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "Incorrect message length. Available from 2013.06.27");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0018", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "Incorrect transaction code on body");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0019", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "After final deadline");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0020", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "Over TPS counts");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0090", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "KRS System Fail");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0101", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "Before trading hours. Can send orders after receiving a response to business-opening");
        else if (memcmp(R_Pkt->Head.ResponseCode, "0102", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "After trading hours. Denial message is sent via trade session");

        /*------------------------------------------------------
         *  세션 오류 (E001~E006)
         *------------------------------------------------------*/
        else if (memcmp(R_Pkt->Head.ResponseCode, "E001", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s", "Invalid Interface Sequence");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E002", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Invalid ID or Password");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E003", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Unregistered MAC Address");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E004", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Message Size Error");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E005", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Send 'H' or 'D' Message Type without 'L' Message Type");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E006", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Message Type Error");

        /*------------------------------------------------------
         *  주문 검증 오류 (E101~E127, E999)
         *------------------------------------------------------*/
        else if (memcmp(R_Pkt->Head.ResponseCode, "E101", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Invalid Order ID Range");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E102", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Duplicate Order ID");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E103", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Invalid Original Order ID Range");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E104", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Not Found Quantity of Original Order ID");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E105", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Not Amend Same Order Price");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E106", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Invalid Contract(ISIN) Code");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E107", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Ask/Bid Type Code Error");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E108", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Order Kind Error(New/Amend/Cancel Order)");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E109", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Account Information Error");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E110", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Invalid Quantity Unit");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E111", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Order Quantity Limit Exceeded");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E112", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Invalid Price Unit");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E113", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Order Type Error");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E114", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "IOC/FOK Order Condition Error");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E115", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Upper/Lower Limit Price Error");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E116", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Conditional Limit Order Error(same to the upper/lower limit price)");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E117", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Institutional Trading Reject(Circuit Breaker, Market Stop, etc.)");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E118", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Order Rejection due to expiry");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E119", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "KRX Connection Lost due to Hardware/Network Error");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E120", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Additional Margin Needed");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E121", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "End of the Market");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E122", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Delta Position Limit Exceeded");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E123", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "Prevention of Order Errors Limit Exceeded");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E125", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "호가접수중지");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E126", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "거래자ID오류");
        else if (memcmp(R_Pkt->Head.ResponseCode, "E127", sizeof(R_Pkt->Head.ResponseCode)) == 0)
            sprintf(U_ErrMsg, "%s",  "MOC전문항목 오류");
        else
            sprintf(U_ErrMsg, "%s",  "Unknown Error");

        SLog(USR_ERROR, "LogOn Error ResponseCode[%.4s][%s]", R_Pkt->Head.ResponseCode, U_ErrMsg);
        return (NOTOK);
    }

    return (OK);
}   /* End of Check_Header () */

/*======================================================================
 *  Send_Packet: IMECO 패킷 송신
 *  ------------------------------------------------------------------
 *  PktType별:
 *    T_LINK: 로그인 요청 (20+60바이트)
 *      - UserID(20) + Password(20) + ServerIP(20)
 *    T_POOK: 하트비트 응답 (20바이트, Body 없음)
 *      - SeqNo = "0000000000" 고정 (POOL/POOK은 Seq 0)
 *======================================================================*/
void    Send_Packet(void) {
    int     rt;
    char    t_time[12];

    memset(SendPkt, 0, sizeof (SendPkt));

    if ( PktType == T_LINK ) {
        /* 로그인 요청: 헤더(20) + 데이터(60) */
        memset(SendPkt, 0x20, 20+60);

        /* 헤더 설정 */
        memcpy(S_Pkt->Head.Length, "0060",         sizeof (S_Pkt->Head.Length));
        memcpy(S_Pkt->Head.MsgType, "L",           sizeof (S_Pkt->Head.MsgType));
        memcpy(S_Pkt->Head.ResponseCode, "0000",   sizeof (S_Pkt->Head.ResponseCode));
        ItoAf(INT_SEQ, S_Pkt->Head.SeqNo,          sizeof (S_Pkt->Head.SeqNo));
        memcpy(S_Pkt->Head.MsgCount, "0",          sizeof (S_Pkt->Head.MsgCount));

        /* 데이터: UserID(20) + Password(20) + ServerIP(20) */
        memcpy(S_Pkt->Data,           LOGON_ID(D_K,P_K),  20);
        memcpy(&S_Pkt->Data[20],      LOGON_PW(D_K,P_K),  20);
        memcpy(&S_Pkt->Data[40],      "123.123.123.123",   20);
    }
    else if ( PktType == T_POOK ) {
        /* 하트비트 응답: 헤더만 (20바이트, Body=0) */
        memset(SendPkt, 0x20, 20);

        memcpy(S_Pkt->Head.Length, "0000",         sizeof (S_Pkt->Head.Length));
        memcpy(S_Pkt->Head.MsgType, "H",           sizeof (S_Pkt->Head.MsgType));
        memcpy(S_Pkt->Head.ResponseCode, "0000",   sizeof (S_Pkt->Head.ResponseCode));
        /* POLL/POOK의 Seq는 0 고정 (문서 참고) */
        memcpy(S_Pkt->Head.SeqNo,  "0000000000",   sizeof (S_Pkt->Head.SeqNo));
        memcpy(S_Pkt->Head.MsgCount, "0",          sizeof (S_Pkt->Head.MsgCount));
    }

    rt = Select_Send(Sockfd, SendPkt, strlen(SendPkt));
    if (rt != OK) {
        TCP2_CON_STA    = OFF;
        close(Sockfd);
        Log(TCP_ERROR, "TCP SD ERROR[%s](%d)<%d> {%d:%s}", SendPkt, strlen(SendPkt), INT_SEQ, SYS_NO, SYS_STR);
        sleep(5);
        Exit_Process();
    }

    Log(TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen(SendPkt), INT_SEQ);

    return;
}   /* Send_Packet () */

/*======================================================================
 *  Write_Data: 수신 데이터를 FIFO(SAM) 또는 DSHM에 기록
 *  ------------------------------------------------------------------
 *  데이터 종류에 따라 출력 파일 분기:
 *    f_gbn=1: 회원처리호가 (C/R/A/I/P/F)
 *      → IMECO헤더(20) + 전문(121) = 141바이트 포함하여 기록
 *    f_gbn=9: 체결정보 (T)
 *      → 전문(121)만 기록 (IMECO헤더 제외)
 *======================================================================*/
int     Write_Data(void) {
    int         i, next, seq, d_cnt, rt;
    int         f_gbn;
    char        m_time[24];
    char        media_gbn[2];
    BUFF_RW_HEAD f_head;

    /* 데이터 종류 판별 */
    if (memcmp(R_Pkt->Data, "C", 1) == 0   ||      /* C=확인 */
            memcmp(R_Pkt->Data, "R", 1) == 0   ||      /* R=거부 */
            memcmp(R_Pkt->Data, "A", 1) == 0   )       /* A=자동취소 */ {
        f_gbn = 1;      /* 회원처리호가 → TS_W1_1 */
    }
    else
        if (memcmp(R_Pkt->Data, "I", 1) == 0   ||      /* I=실시간가격제한(즉시) */
                memcmp(R_Pkt->Data, "P", 1) == 0   ||      /* P=실시간가격제한(이후) */
                memcmp(R_Pkt->Data, "F", 1) == 0   )       /* F=체결/부분체결 */ {
        f_gbn = 1;      /* 회원처리호가에 포함 → TS_W1_1 */
    }
    else
        if (memcmp(R_Pkt->Data, "T", 1) == 0   )       /* T=체결정보 */ {
        f_gbn = 9;      /* 체결 → 별도 처리 */
    }

    /*
     * FILE_BUFF_FORMAT 조립:
     *   If_Seq + ApType + ResponseCode + RecvTime + DataHeader + Data + LineFeed
     */
    memset(&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));
    memset(&File_Data_Head, ' ', HEAD_SIZE);
    memset(m_time, 0, sizeof (m_time));
    Get_MicroTime(m_time);

    ItoAf(INT_SEQ + 1, W_Fmt.If_Seq,           sizeof (W_Fmt.If_Seq));

    memcpy(W_Fmt.ApType,       ApType,         sizeof (W_Fmt.ApType));
    memcpy(W_Fmt.ResponseCode, RES_NORMAL,      strlen(RES_NORMAL));
    memcpy(W_Fmt.RecvTime1,    m_time,          sizeof (W_Fmt.RecvTime1));
    memcpy(W_Fmt.RecvTime2,    &m_time[sizeof(W_Fmt.RecvTime1)],
            sizeof (W_Fmt.RecvTime2));

    ItoAf(DATA_SIZE, File_Data_Head.Length,     sizeof (File_Data_Head.Length));
    ItoAf(INT_SEQ + 1, File_Data_Head.DataSeq, sizeof (File_Data_Head.DataSeq));
    memcpy(File_Data_Head.ResponseCode,  RES_NORMAL,    strlen(RES_NORMAL));
    memcpy(File_Data_Head.LineFlag,      _Exe_Name+4,   3);

    memcpy(W_Fmt.DataHeader, &File_Data_Head, HEAD_SIZE);

    /*
     * 데이터 복사:
     *   f_gbn=1: IMECO헤더(20) + 전문(121) = 141바이트
     *   f_gbn=9: 전문(121)만 (IMECO헤더 제외)
     */
    if (f_gbn == 1)
        memcpy(W_Fmt.Data, R_Pkt->Data, IMECO_HEAD_LEN + sizeof (IMECO_SETTLE_RESP_DATA));
    else
        memcpy(W_Fmt.Data, R_Pkt->Data, sizeof (IMECO_SETTLE_RESP_DATA));
    W_Fmt.LineFeed[0] = '\n';

    SYS_NO = 0;

    /* FIFO 또는 DSHM에 기록 */
#ifdef  SAM_USE
    if (f_gbn == 1)
        rt  = F_W(TS_W1_1, (void *)&W_Fmt, 1);
    else if (f_gbn == 2)
        rt  = F_W(TS_W2_1, (void *)&W_Fmt, 1);
#else
    if (f_gbn == 1)
        rt  = DSHM_W(TS_W1_1, (void *)&W_Fmt, 1);
    else if (f_gbn == 2)
        rt  = DSHM_W(TS_W2_1, (void *)&W_Fmt, 1);
#endif

    if (rt != d_cnt) {
#ifdef  SAM_USE
        if (f_gbn == 1)
            Log(SAM_FATAL, "file write[%s]", OUT_NAME);
        else if (f_gbn == 2)
            Log(SAM_FATAL, "file write[%s]", OUT_NAME2);
#else
        if (f_gbn == 1)
            Log(DSH_FATAL, "DSHM write[%s]", OUT_NAME);
        else if (f_gbn == 2)
            Log(DSH_FATAL, "DSHM write[%s]", OUT_NAME2);
#endif
        memcpy(S_Pkt->Head.ResponseCode, ERR_FILE_WRITE, strlen(ERR_FILE_WRITE));
        return  (NOTOK);
    }

    INT_SEQ += d_cnt;
    if (f_gbn == 1)
        Log(USR_OK, "data write[%s:%d:%d]", OUT_NAME, WR_CNT, d_cnt);
    else if (f_gbn == 2)
        Log(USR_OK, "data write[%s:%d:%d]", OUT_NAME2, WR_CNT2, d_cnt);
    Log(USR_OK, "Write ok[%s][%d]", W_Fmt.Data, strlen(W_Fmt.Data));
    Set_TR_Time();

    return (OK);
}   /* End of Write_Data () */

/*======================================================================
 *  Register_Signal: 시그널 핸들러 등록 (SIGPIPE, SIGTERM)
 *======================================================================*/
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

/*======================================================================
 *  Catch_Signal: 시그널 수신시 호출되는 핸들러
 *======================================================================*/
void    Catch_Signal(int signo) {
    _in_signal_handler = 1;
    SIG_WRITE_MSG("[SIGNAL] pa_2200_tr caught signal\n");

    TCP2_CON_STA = OFF;
    close(Sockfd);
    Exit_Process();
}   /* End of Catch_Signal () */

/*========================================================================
 *  End of Program (pa_2200_tr.c)
 *========================================================================*/
