#define     _GLOBAL
/*========================================================================
 *  Module  : 장운영정보 수신 (TCP client, 비동기)
 *  File    : pa_1600_tr.c
 *  -----------------------------------------------------------------
 *  KRX(거래소) 장운영정보 서버에 TCP 클라이언트로 접속하여
 *  장운영관련 데이터(시간정보, 장상태 등)를 수신하는 프로세스.
 *
 *  빌드 옵션:
 *    A1601 : 현물(주식) 장운영정보 수신  (업무코드 J681)
 *    A2601 : 파생(선물/옵션) 장운영정보 수신 (업무코드 J682)
 *
 *  접속 절차:
 *    1단계: 기본 포트로 접속 → LINK(91) 전문 송신 → 응답에서 새 포트 수신
 *    2단계: 새 포트로 재접속 → STRT(92) 전문 송신 → 데이터 수신 루프
 *
 *  수신 프로토콜:
 *    91 = LINK (접속)     92 = RSOK (개시응답)
 *    93 = RSND (복구요청)  98 = POLL (상태확인)
 *    01~08 = DATA (실데이터)
 *========================================================================*/

/*------------------------------------------------------------------------
 *  헤더 파일
 *------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "fep_common.h"

/*------------------------------------------------------------------------
 *  상수 및 매크로 정의
 *------------------------------------------------------------------------*/
#define     TCP_TIME_OUT    35      /* TCP 수신 타임아웃 기본값 (초) */

/*
 * SAM_USE 여부에 따라 출력 대상이 달라짐
 *   SAM_USE 정의시: 파일(SAM) 기반 출력 (OFW_CNT, OFN)
 *   미정의시:       DSHM(공유메모리) 기반 출력 (ODW_CNT, ODN)
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

/*------------------------------------------------------------------------
 *  전역 변수
 *------------------------------------------------------------------------*/
int     Sockfd;                 /* TCP 소켓 파일디스크립터 */
int     PortNo;                 /* 접속 대상 포트 번호 */
int     DataCnt;                /* 수신 데이터 건수 누적 */
int     PktType;                /* 현재 전송할 패킷 타입 (T_LINK, T_STRT 등) */
int     DataSize;               /* 1건당 데이터 크기 (바이트) */
int     MaxCnt;                 /* TCP 1패킷에 담을 수 있는 최대 건수 */
int     RetryCnt;               /* 연속 접속 실패 횟수 (5회 초과시 서버 전환) */
char    ApType[10];             /* 업무식별코드 (예: "PA1600RS") */
char    IpAddr[20];             /* 접속 대상 IP 주소 문자열 */
char    ErrCd[8];               /* 오류코드 (Check_Header에서 설정) */
char    RecvPkt[TCP_BUFF_MAX_LEN];  /* TCP 수신 버퍼 */
char    SendPkt[TCP_BUFF_MAX_LEN];  /* TCP 송신 버퍼 */

TCP_MESSAGE     *R_Pkt  = (TCP_MESSAGE *)RecvPkt;   /* 수신 패킷 구조체 포인터 */
TCP_HEAD        *S_Pkt  = (TCP_HEAD *)SendPkt;      /* 송신 헤더 구조체 포인터 */

/*------------------------------------------------------------------------
 *  함수 프로토타입
 *------------------------------------------------------------------------*/
void    PA_1600_TR(void);
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
 *  Init_Proc으로 데몬/SHM 초기화 후 메인 루프 실행
 *======================================================================*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);

    PA_1600_TR();

    /* 종료 처리 */
    TCP2_CON_STA = OFF;
    close(Sockfd);
    Exit_Process();
}   /* End of main () */

/*======================================================================
 *  PA_1600_TR: 메인 처리 루틴
 *  ------------------------------------------------------------------
 *  1) 휴일 체크 (HOLIDAY_CHECK 정의시)
 *  2) 1단계 접속: LINK(91) → 응답에서 새 포트 획득
 *  3) 2단계 접속: STRT(92) → 데이터 수신 루프
 *  4) DATA(01~08) 수신 → Write_Data로 FIFO/DSHM 기록
 *======================================================================*/
void    PA_1600_TR(void) {
    int     rt;

    Register_Signal();
    Init_Parameters();

#ifdef  HOLIDAY_CHECK
    /*--------------------------------------------------------------
     *  휴일 체크: 토요일(6)/일요일(4?) 이면 60초 sleep 반복
     *  주의: 원본 코드에서 tm_wday==4(목) || ==6(토) 로 되어있음
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

        if  (tp->tm_wday == 4 || tp->tm_wday == 6) {
            Log(USR_OK, "it's weekend. sleeping...[%d]", tp->tm_wday);
            sleep(60);
            continue;
        }
        else
            break;
        /*
                    if (memcmp (Shm_Risk[0].business_day, DAEMON(D_K).date, 8) == 0)
                        break;
                    else {
                        Log (USR_OK, "system Day와 business Day가 다르다...[%8.8s][%8.8s]",
                            DAEMON(D_K).date, Shm_Risk[0].business_day);
                        sleep (600);
                        continue;
                    }
        */
    }
#endif

    /*--------------------------------------------------------------
     *  1단계: LINK(91) 접속 — 기본 포트로 연결하여 새 포트 번호 수신
     *--------------------------------------------------------------*/
    PktType = T_LINK;
    Connection();
    Send_Packet();

    while (1) {
        rt  = Receive_Packet();

        if  (rt == FAIL)
            continue;           /* 인터럽트 등 일시적 실패 → 재시도 */
        else    if (rt == NOTOK)
            return;             /* 심각한 오류 → 종료 */
        else
            break;              /* 정상 수신 → 다음 단계 */
    }

    /*--------------------------------------------------------------
     *  2단계: STRT(92) 접속 — 응답에서 받은 새 포트로 재연결
     *  R_Pkt->Head.SeqNo 필드에 새 포트 번호가 들어옴
     *--------------------------------------------------------------*/
    close(Sockfd);
    PortNo = AtoIf(R_Pkt->Head.SeqNo, sizeof (R_Pkt->Head.SeqNo));
    PktType = T_STRT;
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

    /*--------------------------------------------------------------
     *  데이터 수신 루프: JOB_END까지 DATA(01~08) 수신
     *--------------------------------------------------------------*/
    while (START_S < JOB_END) {
        Stat_Save();

        rt  = Receive_Packet();

        if  (rt == NOTOK)
            break;
        else    if (rt == FAIL)
            continue;
    }

    return;
}   /* End of PA_1600_TR () */

/*======================================================================
 *  Init_Parameters: 전역 변수 초기화
 *  ------------------------------------------------------------------
 *  - 서버 인덱스(S_K), 재시도 카운트, 데이터 카운트 0으로 초기화
 *  - 타임아웃 기본값 설정 (cfg에 없으면 35초)
 *  - 업무식별코드(ApType) 생성: 실행파일명에서 추출
 *  - DataSize: 1건당 레코드 크기 (SAM/DSHM에 따라 다름)
 *  - MaxCnt: TCP 1패킷에 담을 수 있는 최대 건수
 *  - 접속 IP/포트: cfg/tcp2.ini에서 읽어옴
 *======================================================================*/
void    Init_Parameters(void) {
    SYS_NO = 0;
    S_K = 0;                /* 서버 인덱스 0=주서버, 1=백업서버 */
    RetryCnt = 0;
    DataCnt = 0;

    /* 타임아웃: cfg 설정값이 0이면 기본 35초 사용 */
    if (TIME_OUT == 0)
        TIME_OUT    = TCP_TIME_OUT;

    Log(USR_OK, "TIME_OUT[%d]", TIME_OUT);

    /* 업무식별코드: 실행파일명에서 모듈명+프로세스번호+방향 조합 */
    sprintf(ApType, "%-2.2s%-4.4s%c%c",
            _Exe_Name,  _Exe_Name+3, _Exe_Name[8], _Exe_Name[9] == 's' ? 'R' : 'S');
    LtoU(ApType, strlen(ApType));     /* 소문자→대문자 변환 */

#ifdef  SAM_USE
    DataSize = OFS(D_K,P_K,0);          /* 파일 출력 레코드 크기 */
#else
    DataSize = ODS(D_K,P_K,0);          /* DSHM 출력 레코드 크기 */
#endif

    /* TCP 1패킷에 담을 수 있는 최대 건수 */
    MaxCnt = TCP_DATA_LEN / (HEAD_SIZE + DataSize);

    /* 접속 대상 IP/포트: tcp2.ini에서 D_K(데몬키), P_K(프로세스키), S_K(서버키)로 조회 */
    sprintf(IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
            TCP2_IP2(D_K,P_K,S_K),  TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    PortNo = TCP2_PORT_NO;

    return;
}   /* End of Init_Parameters () */

/*======================================================================
 *  Connection: TCP 서버에 접속
 *  ------------------------------------------------------------------
 *  - Socket 생성 → Connect 시도
 *  - 실패시: 5회 초과하면 백업 서버(S_K=1)로 전환
 *  - STRT 단계에서 실패하면 프로세스 종료
 *  - 성공시: Set_Socket_Linger로 소켓 옵션 설정
 *======================================================================*/
void    Connection(void) {
    int     rt;

    while (START_S != END) {
        /* 1. 소켓 생성 */
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

        /* 2. 서버 접속 시도 */
        rt  = Connect(Sockfd, IpAddr, PortNo);

        if  (rt < 0) {
            TCP2_CON_STA = OFF;
            TCP2_LINE_ST = OFF;
            TCP2_NET_STA(S_K) = OFF;
            Log(TCP_ERROR, "connect fail {%d:%s}", SYS_NO, SYS_STR);
            close(Sockfd);

            /* STRT 단계 실패: 복구 불가 → 종료 */
            if (PktType == T_STRT)
                Exit_Process();
            else {
                /*
                 * LINK 단계 실패: 5회 초과시 백업 서버로 전환
                 * S_K를 0↔1 토글하여 주/백업 서버 교대
                 */
                if (RetryCnt > 5) {
                    S_K = (S_K + 1) % 2;
                    sprintf(IpAddr, "%d.%d.%d.%d",
                            TCP2_IP1(D_K,P_K,S_K), TCP2_IP2(D_K,P_K,S_K),
                            TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
                    PortNo = TCP2_PORT_NO;
                    RetryCnt = 0;

                    Log(TCP_OK, "port changed to %s(%s:%d)",
                            S_K == 0 ? "main" : "backup", IpAddr, PortNo);
                }

                RetryCnt ++;
                sleep(3);
                continue;
            }
        }

        /* 접속 성공 */
        if  (PktType == T_STRT)
            TCP2_CON_STA = ON;

        TCP2_NET_STA(S_K)   = ON;
        Log(USR_OK, "connected to %s:%d", IpAddr, PortNo);
        break;
    }

    /* SO_LINGER 설정: 소켓 종료시 잔여 데이터 전송 보장 */
    Set_Socket_Linger(Sockfd);

    return;
}   /* End of Connection () */

/*======================================================================
 *  Receive_Packet: TCP 패킷 수신
 *  ------------------------------------------------------------------
 *  반환값:
 *    OK(0)    = 정상 수신 및 처리 완료
 *    FAIL(-1) = 일시적 오류 (인터럽트 등) → 재시도 가능
 *    NOTOK(1) = 심각한 오류 → 상위에서 종료 판단
 *
 *  수신 데이터 종류별 처리:
 *    91(LIOK) = 접속 응답 → 즉시 반환
 *    92(RSOK) = 개시 응답 → 복구요청(93) 송신
 *    98(POLL) = 상태확인 → POOK 응답 송신
 *    01~08    = DATA    → Write_Data로 파일/DSHM 기록
 *======================================================================*/
int     Receive_Packet(void) {
    int     rt, recv_len;
    char    error_flag;

    error_flag = OFF;
    memset(RecvPkt, 0, sizeof (RecvPkt));

    /* TCP 수신 (Select 기반 타임아웃 포함) */
    rt = Select_Receive(Sockfd, RecvPkt);

    if (rt == OK)
        return  (NOTOK);        /* 타임아웃 = 접속 끊김 */
    else if (rt == NOTOK) {
        if  (SYS_NO == EINTR)
            return (FAIL);      /* 시그널 인터럽트 → 재시도 */

        return  (NOTOK);
    }

    Log(TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen(RecvPkt), INT_SEQ);

    /* 패킷 길이 유효성 검증 */
    recv_len = strlen(RecvPkt);

    if (recv_len < TCP_HEAD_LEN || recv_len != rt ||
            recv_len    > TCP_MESSAGE_MAX_LEN) {
        Log(USR_ERROR, "invalid length[%d,%d]", recv_len, rt);
        PktType = T_LNTH;
        DataCnt = 0;
        sleep(3);
        return  (NOTOK);
    }

    /* 헤더 유효성 검증 (STX, Length, ResponseCode 등) */
    rt = Check_Header();

    if (rt != OK)
        return  (NOTOK);

    /* DataCnt 필드에 따라 전문 종류 분기 처리 */
    if (memcmp(R_Pkt->Head.DataCnt, "91", 2) == 0)     /* T_LIOK: 접속 응답 */ {
        return  (OK);
    }
    if (memcmp(R_Pkt->Head.DataCnt, "92", 2) == 0)     /* T_RSOK: 개시 응답 */ {
        PktType = T_RSND;       /* 복구요청(93) 송신 */
        Send_Packet();
    }
    else if (memcmp(R_Pkt->Head.DataCnt, "98", 2) == 0)    /* POLL 요청 */ {
        PktType = T_POOK;       /* POOK 응답 송신 */
        Send_Packet();
    }
    else
        if ((memcmp(R_Pkt->Head.DataCnt, "01", 2) >= 0) && /* T_DATA: 실데이터 (01~08) */
                (memcmp(R_Pkt->Head.DataCnt, "08", 2) <= 0)) {
        rt  = Write_Data();
        if  (rt != OK) {
            Log(USR_ERROR, "Write_Data Error");
            sleep(3);
            return (NOTOK);
        }
    }
    else {
        Log(USR_ERROR, "Else Case Data R_Pkt->Head.DataCnt[%2.2s]", R_Pkt->Head.DataCnt);
        return  (NOTOK);
    }

    return (OK);
}   /* Receive_Packet () */

/*======================================================================
 *  Send_Packet: TCP 패킷 송신
 *  ------------------------------------------------------------------
 *  PktType에 따라 헤더 필드를 설정한 후 TCP 전송
 *
 *  TCP_HEAD 구조:
 *    1.STX  2.Length  3.ApType(업무코드)  4.SR_gbn(송수신)
 *    5.Date  6.Time  7.ResponseCode  8.Q_gbn
 *    9.SeqNo  10.DataCnt  11.Last_SeqNo  12.Server_gbn
 *    13.Branch  14.Sv_No  15.Hts_Id  16.User_Id  17.Filler
 *
 *  빌드별 업무코드:
 *    A1601 → "J681" (현물 장운영)
 *    A2601 → "J682" (파생 장운영)
 *======================================================================*/
void    Send_Packet(void) {
    int     rt;
    char    t_time[12];

    memset(SendPkt, 0, sizeof (SendPkt));
    memset(SendPkt, 0x20, TCP_HEAD_LEN);   /* 공백(space)으로 초기화 */

    /* 1. STX (전문 시작 마커) */
    S_Pkt->Stx[0] = STX;

    /* 3. 업무식별코드: 빌드 옵션별로 다름 */
#if defined A1601
    memcpy(S_Pkt->ApType, "J681", sizeof (S_Pkt->ApType));
#elif   defined A2601
    memcpy(S_Pkt->ApType, "J682", sizeof (S_Pkt->ApType));
#endif

    /* 4. 송수신 구분: R=수신(Receive) */
    memcpy(S_Pkt->SR_gbn, "R",    sizeof (S_Pkt->SR_gbn));
    /* 5. 처리일자 */
    memcpy(S_Pkt->Date, DATE_CURR,sizeof (S_Pkt->Date));
    /* 6. 처리시각 (HHMMSS) */
    memset(t_time, 0, sizeof (t_time));
    Get_Time(t_time);
    memcpy(S_Pkt->Time, t_time,   sizeof (S_Pkt->Time));
    /* 7. 응답코드: '000' = 정상 */
    ItoAf(0, S_Pkt->ResponseCode, sizeof (S_Pkt->ResponseCode));
    /* 8. Queue 입력구분: '000' */
    ItoAf(0, S_Pkt->Q_gbn,        sizeof (S_Pkt->Q_gbn));
    /* 12. 서버구분: '1' = 정상주문 */
    memcpy(S_Pkt->Server_gbn, "1",sizeof (S_Pkt->Server_gbn));
    /* 13. 지점번호: SPACE (FEP 담당자 지시) */
    memcpy(S_Pkt->Branch, "   ",  sizeof (S_Pkt->Branch));
    /* 14. 물리적 지점번호: 8706 */
    memcpy(S_Pkt->Sv_No,  "8706", sizeof (S_Pkt->Sv_No));
    /* 15. HTS 단말ID */
    memcpy(S_Pkt->Hts_Id, "        ", sizeof (S_Pkt->Hts_Id));
    /* 16. 사번 or 로그인 ID */
    memcpy(S_Pkt->User_Id, "        ",sizeof (S_Pkt->User_Id));
    /* 17. Filler */
    memcpy(S_Pkt->Filler, "        ", sizeof (S_Pkt->Filler));

    /*
     * PktType별 가변 필드 설정
     *   T_LINK(91): 접속 요청  — SeqNo=0
     *   T_STRT(92): 개시 요청  — SeqNo=현재시퀀스
     *   T_RSND(93): 복구 요청  — SeqNo=현재시퀀스 (마지막 수신건부터 재전송 요청)
     *   T_POOK(98): POLL 응답  — SeqNo=현재시퀀스
     */
    switch (PktType) {
        case    T_LINK:
            ItoAf(TCP_HEAD_LEN-5, S_Pkt->Length, sizeof (S_Pkt->Length));
            ItoAf(0, S_Pkt->SeqNo,  sizeof (S_Pkt->SeqNo));
            memcpy(S_Pkt->DataCnt, "91",   sizeof (S_Pkt->DataCnt));
            ItoAf(0, S_Pkt->Last_SeqNo, sizeof (S_Pkt->Last_SeqNo));
            break;
        case    T_STRT:
            ItoAf(TCP_HEAD_LEN-5, S_Pkt->Length, sizeof (S_Pkt->Length));
            ItoAf(INT_SEQ, S_Pkt->SeqNo,   sizeof (S_Pkt->SeqNo));
            memcpy(S_Pkt->DataCnt, "92",   sizeof (S_Pkt->DataCnt));
            ItoAf(INT_SEQ, S_Pkt->Last_SeqNo,  sizeof (S_Pkt->Last_SeqNo));
            break;
        case    T_RSND:
            ItoAf(TCP_HEAD_LEN-5, S_Pkt->Length, sizeof (S_Pkt->Length));
            ItoAf(INT_SEQ, S_Pkt->SeqNo,   sizeof (S_Pkt->SeqNo));
            memcpy(S_Pkt->DataCnt, "93",   sizeof (S_Pkt->DataCnt));
            ItoAf(INT_SEQ, S_Pkt->Last_SeqNo,  sizeof (S_Pkt->Last_SeqNo));
            break;
        case    T_POOK:
            ItoAf(TCP_HEAD_LEN-5, S_Pkt->Length, sizeof (S_Pkt->Length));
            ItoAf(INT_SEQ, S_Pkt->SeqNo,   sizeof (S_Pkt->SeqNo));
            memcpy(S_Pkt->DataCnt, "98",   sizeof (S_Pkt->DataCnt));
            ItoAf(INT_SEQ, S_Pkt->Last_SeqNo,  sizeof (S_Pkt->Last_SeqNo));
            break;
        default:
            break;
    }

    /* TCP 송신 (Select 기반) */
    rt = Select_Send(Sockfd, SendPkt, TCP_HEAD_LEN);

    if (rt != OK) {
        TCP2_CON_STA    = OFF;
        close(Sockfd);
        Exit_Process();
    }

    Log(TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen(SendPkt), INT_SEQ);

    return;
}   /* Send_Packet () */

/*======================================================================
 *  Check_Header: 수신 패킷 헤더 유효성 검증
 *  ------------------------------------------------------------------
 *  검증 항목:
 *    1) STX (0x02) 마커 확인
 *    2) Length 필드 vs 실제 수신 길이 일치
 *    3) ResponseCode = 0 (정상)
 *    4) 접속/재접속 시 DataCnt가 기대값과 일치하는지
 *======================================================================*/
int     Check_Header(void) {
    int     val, i, len, data_cnt, data_len;
    char    *data = (char *)R_Pkt->Data;

    memset(ErrCd, '0', sizeof (ErrCd));

    /* STX(0x02) 검증 */
    if (R_Pkt->Head.Stx[0] != STX) {
        Log(USR_ERROR, "TH.Stx [%#04x:%#04x]", R_Pkt->Head.Stx[0], STX);
        memcpy(ErrCd, ERR_HEAD_T1, strlen(ERR_HEAD_T1));
        return  (NOTOK);
    }

    /* Length 검증: 전문길이 = 전체길이 - 5(STX+Length 4바이트) */
    val = AtoIf(R_Pkt->Head.Length, sizeof (R_Pkt->Head.Length));

    if (val != strlen(RecvPkt)-5 ||
            strlen(RecvPkt) < TCP_HEAD_LEN) {
        Log(USR_ERROR, "TH.Length [%d:%d]", val, strlen(RecvPkt));
        memcpy(ErrCd, ERR_HEAD_T2, strlen(ERR_HEAD_T2));
        return  (NOTOK);
    }

    data_len = val - TCP_HEAD_LEN - 5;

    /* ResponseCode 검증: 0이 아니면 오류 */
    if (AtoIf(R_Pkt->Head.ResponseCode,
            sizeof  (R_Pkt->Head.ResponseCode)) != 0) {
        Log(USR_ERROR, "TH.ResponseCode [%.4s:%04d]",
                R_Pkt->Head.ResponseCode, 0);
        memcpy(ErrCd, ERR_HEAD_T6, strlen(ERR_HEAD_T6));
        return  (NOTOK);
    }

    /* 접속/재접속 전문 종류 확인 (경고 로그만 출력) */
    if ((PktType == T_LINK && memcmp(R_Pkt->Head.DataCnt, "91", 2) != 0) ||
            (PktType    == T_STRT && memcmp(R_Pkt->Head.DataCnt, "92", 2) != 0) ) {
        Log(USR_ERROR, "TH.Rcv Data PktType[%d] [%s]", PktType, R_Pkt);
    }

    return (OK);
}   /* End of Check_Header () */

/*======================================================================
 *  Write_Data: 수신 데이터를 FIFO(SAM) 또는 DSHM에 기록
 *  ------------------------------------------------------------------
 *  수신 데이터 앞에 BUFF_RW_HEAD(70바이트)를 붙여서 기록
 *
 *  BUFF_RW_HEAD 구조:
 *    If_Seq(시퀀스) + ApType(업무코드) + ResponseCode + RecvTime1/2 + DataHeader
 *
 *  기록 대상:
 *    SAM_USE 정의시 → F_W (파일 FIFO 쓰기)
 *    미정의시        → DSHM_W (공유메모리 쓰기)
 *======================================================================*/
int     Write_Data(void) {
    int       i, next, seq, d_cnt, rt, len, f_size, rec_size;
    int       f_gbn;
    char         m_time[24], w_buf[FILE_BUF_LEN];
    char         *data = (char *)R_Pkt->Data;
    BUFF_RW_HEAD f_head;

    next = 0;

    /* 수신 데이터 건수 */
    d_cnt = AtoIf(R_Pkt->Head.DataCnt, sizeof (R_Pkt->Head.DataCnt));
    f_size = sizeof (BUFF_RW_HEAD);     /* 헤더 크기 (70바이트) */
    rec_size = f_size + DataSize + 1;   /* 1레코드 = 헤더 + 데이터 + 개행 */
    len = strlen(data);

    /* 마이크로초 단위 시각 획득 */
    memset(m_time, 0, sizeof (m_time));
    Get_MicroTime(m_time);

    memset(w_buf, ' ', sizeof (w_buf));

    /* BUFF_RW_HEAD 70바이트 설정 */
    ItoAf(INT_SEQ + i + 1, f_head.If_Seq, sizeof (f_head.If_Seq));
    memcpy(f_head.ApType, ApType, sizeof (f_head.ApType));
    ItoAf(0, f_head.ResponseCode, sizeof (f_head.ResponseCode));
    memcpy(f_head.RecvTime1, m_time, sizeof (f_head.RecvTime1));
    memcpy(f_head.RecvTime2, &m_time[sizeof(f_head.RecvTime1)],
            sizeof (f_head.RecvTime2));
    ItoAf(DataSize, f_head.DataHeader, 20);

    /* 버퍼 조립: [BUFF_RW_HEAD][수신데이터][패딩][\n] */
    memcpy(&w_buf[0], &f_head, f_size);
    memcpy(&w_buf[f_size], data, len);
    if (DataSize > len)
        memset(&w_buf[f_size+(len)], 0x20, DataSize - (len));
    w_buf[f_size+DataSize] = '\n';

    SYS_NO = 0;

    /* FIFO 또는 DSHM에 기록 */
#ifdef  SAM_USE
    rt  = F_W(TS_W1_1, w_buf, d_cnt);
#else
    rt  = DSHM_W(TS_W1_1, w_buf, d_cnt);
#endif

    if (rt != d_cnt) {
#ifdef  SAM_USE
        Log(SAM_FATAL, "file write[%s]", OUT_NAME);
#else
        Log(DSH_FATAL, "DSHM write[%s]", OUT_NAME);
#endif
        memcpy(S_Pkt->ResponseCode, ERR_FILE_WRITE, strlen(ERR_FILE_WRITE));
        return  (NOTOK);
    }

    INT_SEQ += d_cnt;
    Log(USR_OK, "data write[%s:%d:%d]", OUT_NAME, WR_CNT, d_cnt);
    Set_TR_Time();

    return (OK);
}   /* End of Write_Data () */

/*======================================================================
 *  Register_Signal: 시그널 핸들러 등록
 *  ------------------------------------------------------------------
 *  SIGPIPE: TCP 끊김시 프로세스 비정상 종료 방지
 *  SIGTERM: kill 명령 수신시 정상 종료 처리
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
 *  ------------------------------------------------------------------
 *  async-signal-safe 함수만 사용 (SIG_WRITE_MSG)
 *  TCP 접속 해제 후 프로세스 종료
 *======================================================================*/
void    Catch_Signal(int signo) {
    _in_signal_handler = 1;
    SIG_WRITE_MSG("[SIGNAL] pa_1600_tr caught signal\n");

    TCP2_CON_STA = OFF;
    close(Sockfd);
    Exit_Process();
}   /* End of Catch_Signal () */

/*========================================================================
 *  End of Program (pa_1600_tr.c)
 *========================================================================*/
