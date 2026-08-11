#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : 전략기동/종료 관리
#   File    : pa_9000_mp.c
#
#   설명  : 자동매매 전략 프로세스의 기동/종료를 관리하는 프로세스.
#             FIFO에서 전략 기동 요청(TR코드: 500100)을 수신하면
#             사용 가능한 자동매매 프로세스 슬롯을 찾아 바이너리를
#             복사(cp)하고 기동시킨다.
#             프로세스 시작 시 SHM의 시세/미체결/리스크/주문번호/
#             계좌한도/잔고 정보를 초기화한다.
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

#define     DATA_SIZE   2048
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DATA_TIME       60 * 1000       /* Poll 대기 시간 (60초, 밀리초 단위) */
#define     READ_MAX        1               /* FIFO에서 한 번에 읽을 최대 건수 */
#define     KRX_PROCNT      4               /* KRX 관련 프로세스 수 */
#define     PLAY_TIME       "0739"          /* 기동 시작 시간 (07:39) */
#define     AUTO_PLAY_TIME  "081000"        /* 자동 기동 시간 (08:10:00) */

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int                 R_Cnt, RR;              /* FIFO 읽기 건수, 라운드로빈 인덱스 */
char                ApType[10];             /* 어플리케이션 타입 */
FILE_BUFF_FORMAT    W_Fmt, R_Fmt[READ_MAX]; /* 쓰기/읽기 버퍼 */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_9000_MP(int);
void    Init_Parameters(int);
void    Analyze_Data(void);
void    Start_Client(char *);
void    Write_Auto_Data(int, char *, int);
void    Stop_Client(char *, int);
void    Auto_Stop_Client(char *);
void    Write_Data(int);
void    Write_Auto_Fifo(int);
void    Stop_All_Client(char *, int);
void    Acc_Search(void);
void    Acc_Update(char *);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 전략 관리 루프 실행.              */
/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);                 /* 프로세스 초기화 */
    PA_9000_MP(argc);                      /* 메인 처리 루프 (argc로 초기화 여부 판단) */
    Exit_Process();                        /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PA_9000_MP: 메인 처리 루프                                            */
/*    1) 09시 이전이면 SHM 초기화 수행                                    */
/*    2) FIFO에서 전략 기동/종료 요청 수신                              */
/*    3) Analyze_Data()로 요청 분석 및 처리                             */
/*    4) 데이터가 없으면 Poll_File()로 60초 대기                       */
/*----------------------------------------------------------------------*/
void    PA_9000_MP(int argc) {
    int     rt, DelayFlag;
    char    m_time[24];

    DelayFlag = OFF;                        /* 초기화 수행 여부 플래그 */

    /* ApType 생성: 실행파일명에서 모듈+기능번호+타입 추출 */
    sprintf(ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));

    /* === 외부 루프: 프로세스 종료 신호(JOB_END)까지 반복 === */
    while (START_S != JOB_END) {
        Stat_Save();

        /* === 내부 루프: FIFO에 데이터가 있는 동안 계속 읽음 === */
        while (START_S != JOB_END) {
            Stat_Save();

            /*
             * 최초 1회 SHM 초기화 수행
             * 09시 이전에만 초기화 가능 (장중 초기화 방지)
             */
            if (DelayFlag == OFF) {
                memset(m_time, 0, sizeof(m_time));
                Get_Time(m_time);

                if (memcmp(m_time, "0900", 4) < 0) /* 9시 이전에만 초기화 허용 */
                    Init_Parameters(argc);
                Log(USR_OK, "Init_P OK");
                DelayFlag = ON;             /* 초기화 완료 표시 */
            }

            memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * READ_MAX);

            R_Cnt = F_R(PS_R_1, (void *)R_Fmt, READ_MAX);

            if (R_Cnt < 0) {
                /* 읽기 실패: 오류 로그 후 프로세스 종료 */
                Log(SAM_FATAL, "cannot read File[%s,%d:%s]",
                        IFN(D_K,P_K,0), SYS_NO, SYS_STR);
                sleep(1);
                Exit_Process();
            }
            else if (R_Cnt == 0)
                break;                      /* 읽을 데이터 없음 → Poll 대기 */

            Log(USR_OK, "RD [%s:%d][%d]",
                    IFN(D_K,P_K,0), R_Cnt, IFR(D_K,P_K,0,0));
            Analyze_Data();                /* 전략 기동/종료 요청 분석 */
        }

        /* FIFO에 데이터가 없으면 최대 60초간 대기 */
        rt = Poll_File(DATA_TIME);

        if (rt == 1)
            Log(USR_OK, "poll timeout <%d>", INT_SEQ);
        else if (rt == -1)
            continue;                       /* 오류 → 다음 루프 */
    }
}   /* End of PA_9000_MP () */

/*************************************************************************
    Function        : Init_Parameters
    Parameters IN   : argc - 명령행 인자 수 (1이면 초기화 강제 수행)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : SHM의 기초 자료를 초기화한다:
                      1) 시세 관련: auto_use, dont_trade 초기화
                      2) 미체결 내역: 시장별/계좌별 미체결 건수 0으로 리셋
                      3) 리스크(손익): 전체 PROFIT 구조체 memset
                      4) 주문번호 범위: 시장별/전략별 START/USED/END 번호 할당
                      5) 계좌 한도: 1회 한도, 누적 한도 설정값/사용값 초기화
                      6) 잔고 정보: 종목별/계좌별 보유/미체결 수량 초기화
                      7) 배치 처리 건수: 수신/처리 카운터 0으로 리셋
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Init_Parameters(int argc) {
    int     i, j, k;

    RR = 0;                                 /* 라운드로빈 인덱스 초기화 */

    /* ===== [Step 1] 시세 관련 초기화 ===== */
    /* 시세 초기화는 기본 DD 프로세스에서 수행하므로
     * 여기서는 주문 관련 필드(auto_use, dont_trade)만 초기화 */

    /* 채권KTS 종목별 자동매매/거래불가 플래그 초기화 */
    for (i = 0; i < SHM_MAX_NOTE; i++) {
        Shm_Risk[0].S_Sise[0][i].auto_use = 0;
        Shm_Risk[0].S_Sise[0][i].dont_trade = 0;
    }
    /* 금융파생 종목별 자동매매/거래불가 플래그 초기화 */
    for (i = 0; i < SHM_MAX_DEV_FIF; i++) {
        Shm_Risk[0].S_Sise[1][i].auto_use = 0;
        Shm_Risk[0].S_Sise[1][i].dont_trade = 0;
    }

    /* ===== [Step 2] 미체결 내역 초기화 ===== */
    for (i = 0; i < RISK_MK_CNT; i++)           /* 시장별 */ {
        for (j = 0; j < ACC_NO_CNT; j++)        /* 계좌별 */ {
            Shm_Mk_PreMatch[0].MeChe_Cnt[i][j] = 0;

            for (k = 0; k < MAX_MICHE; k++) /* 미체결 슬롯별 */
                Shm_Mk_PreMatch[0].F_MiChe[i][j][k].Jan_Cnt = 0;
        }
    }

    /* ===== [Step 3] 리스크(손익) 초기화 ===== */
    memset(&Shm_Risk[0].ProFit[0][0][0].item_getcnt, 0,
            sizeof (PROFIT) * RISK_MK_CNT * SHM_MAX_NOTE * ACC_NO_CNT);

    /* ===== [Step 4] 주문번호 범위 할당 ===== */
    /*
     * 메인 서버(REAL1)와 백업 서버(REAL2) 구분:
     *   REAL1 또는 TEST: 오프셋 0 (기본 주문번호 사용)
     *   REAL2(백업): 오프셋 100,000,000 (주문번호 충돌 방지)
     */
    long    dual_atv_no[2];                 /* 백업 서버 주문번호 오프셋 */

    if ((memcmp(_FEP_DIV, "REAL1", 5) == 0) ||
            (memcmp(_FEP_DIV, "TEST", 4) == 0)) {
        dual_atv_no[0] = 0;             /* 메인 서버: 오프셋 없음 */
        dual_atv_no[1] = 0;
    }
    else {
        dual_atv_no[0] = MK01_ORDER_NO_BACK_GAP;    /* 백업: 100,000,000 오프셋 */
        dual_atv_no[1] = MK02_ORDER_NO_BACK_GAP;
    }

    /*
     * 주문번호 초기화 조건:
     *   argc == 1 (인자 없이 실행) 이거나
     *   SHM의 주문번호가 아직 초기화되지 않은 경우
     *
     * 전략별 주문번호 범위:
     *   각 전략(MAX_AUTO_PROC=40개)에 고유 범위를 할당
     *   START = BASE + (GAP × i) + dual_offset
     *   END   = START + GAP - 1
     *   예) 전략0: 200,000,000 ~ 200,999,999
     *       전략1: 201,000,000 ~ 201,999,999
     *       ...
     *   10만번 미만은 Client 채번 영역으로 예약
     */
    if (argc == 1 ||
            Shm_Risk[0].Order_No[1][0].START_JMNO != MK02_ORDER_NO_BASE + dual_atv_no[0]) {
        for (i = 0; i < MAX_AUTO_PROC; i++) {
            /* 채권 시장 주문번호 범위 */
            Shm_Risk[0].Order_No[0][i].START_JMNO = MK01_ORDER_NO_BASE + (MK01_ORDER_NO_GAP*i) + dual_atv_no[0];
            Shm_Risk[0].Order_No[0][i].USED_JMNO  = MK01_ORDER_NO_BASE + (MK01_ORDER_NO_GAP*i) + dual_atv_no[0];
            Shm_Risk[0].Order_No[0][i].END_JMNO   = MK01_ORDER_NO_BASE + (MK01_ORDER_NO_GAP*i) + MK01_ORDER_NO_GAP + dual_atv_no[0] - 1;

            /* 금융파생 시장 주문번호 범위 */
            Shm_Risk[0].Order_No[1][i].START_JMNO = MK02_ORDER_NO_BASE + (MK02_ORDER_NO_GAP*i) + dual_atv_no[1];
            Shm_Risk[0].Order_No[1][i].USED_JMNO  = MK02_ORDER_NO_BASE + (MK02_ORDER_NO_GAP*i) + dual_atv_no[1];
            Shm_Risk[0].Order_No[1][i].END_JMNO   = MK02_ORDER_NO_BASE + (MK02_ORDER_NO_GAP*i) + MK02_ORDER_NO_GAP + dual_atv_no[1] - 1;
        }
    }

    /* ===== [Step 5] 계좌 한도 초기화 ===== */
    for (i = 0; i < RISK_MK_CNT; i++)           /* 시장별 */ {
        for (j = 0; j < ACC_NO_CNT; j++)        /* 계좌별 */ {
            /* 1회 한도 설정값 (주문 수량/금액/틱 제한) */
            memcpy(Shm_Risk[0].O_M_Fund[i][j].qty_gbn,   " ", 1);
            memcpy(Shm_Risk[0].O_M_Fund[i][j].money_gbn, " ", 1);
            memcpy(Shm_Risk[0].O_M_Fund[i][j].tick_gbn,  " ", 1);
            Shm_Risk[0].O_M_Fund[i][j].qty   = 0;
            Shm_Risk[0].O_M_Fund[i][j].money = 0;
            Shm_Risk[0].O_M_Fund[i][j].tick  = 0;

            /* 누적 한도 설정값 (매도/매수 건수/금액 제한) */
            memcpy(Shm_Risk[0].T_M_Fund[i][j].do_cnt_gbn,   " ", 1);
            memcpy(Shm_Risk[0].T_M_Fund[i][j].do_money_gbn, " ", 1);
            memcpy(Shm_Risk[0].T_M_Fund[i][j].su_cnt_gbn,   " ", 1);
            memcpy(Shm_Risk[0].T_M_Fund[i][j].su_money_gbn, " ", 1);
            Shm_Risk[0].T_M_Fund[i][j].do_cnt   = 0;
            Shm_Risk[0].T_M_Fund[i][j].do_money = 0;
            Shm_Risk[0].T_M_Fund[i][j].su_cnt   = 0;
            Shm_Risk[0].T_M_Fund[i][j].su_money = 0;

            /* 누적 한도 사용값 (실제 사용된 매도/매수 건수/금액) */
            memcpy(Shm_Risk[0].T_S_Fund[i][j].do_cnt_gbn,   " ", 1);
            memcpy(Shm_Risk[0].T_S_Fund[i][j].do_money_gbn, " ", 1);
            memcpy(Shm_Risk[0].T_S_Fund[i][j].su_cnt_gbn,   " ", 1);
            memcpy(Shm_Risk[0].T_S_Fund[i][j].su_money_gbn, " ", 1);
            Shm_Risk[0].T_S_Fund[i][j].do_cnt   = 0;
            Shm_Risk[0].T_S_Fund[i][j].do_money = 0;
            Shm_Risk[0].T_S_Fund[i][j].su_cnt   = 0;
            Shm_Risk[0].T_S_Fund[i][j].su_money = 0;
        }
    }

    /* ===== [Step 6] 잔고 정보 초기화 ===== */
    for (i = 0; i < RISK_MK_CNT; i++)           /* 시장별 */ {
        for (j = 0; j < SHM_MAX_NOTE; j++)      /* 종목별 */ {
            for (k = 0; k < ACC_NO_CNT; k++)    /* 계좌별 */ {
                Shm_Risk[0].ProFit[i][j][k].item_getcnt        = 0; /* 보유수량 */
                Shm_Risk[0].ProFit[i][j][k].item_get_avg_price  = 0;    /* 평균매입가 */

                Shm_Risk[0].ProFit[i][j][k].item_su_michecnt    = 0;    /* 매수미체결 수량 */
                Shm_Risk[0].ProFit[i][j][k].item_su_miche_gum   = 0;    /* 매수미체결 금액 */
                Shm_Risk[0].ProFit[i][j][k].item_do_michecnt    = 0;    /* 매도미체결 수량 */
                Shm_Risk[0].ProFit[i][j][k].item_do_miche_gum   = 0;    /* 매도미체결 금액 */
            }
        }
    }

    /* ===== [Step 7] 배치 처리 건수 초기화 ===== */
    for (i = 0; i < 7; i++) {
        Shm_Risk[0].Batch_Cnt[i].W_cnt = 0; /* 배치 쓰기 건수 */
        Shm_Risk[0].Batch_Cnt[i].R_cnt = 0; /* 배치 읽기 건수 */
    }

    return;
}   /* End of Init_Parameters () */

/*************************************************************************
    Function        : Analyze_Data
    Parameters IN   : (없음, 전역변수 R_Fmt/R_Cnt 사용)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : FIFO에서 읽은 전략 기동/종료 요청을 분석한다.
                      TR코드별 처리:
                        500100 → 전략 기동 요청 → Start_Client()
                        기타   → 오류 응답(500120) → Write_Data()
                      ApType_Cd 범위: 5010~5999만 유효
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Analyze_Data(void) {
    int             i, rt;
    char            m_time[24];
    SEARCH_HEADER   *i_hd;                  /* 요청 데이터 헤더 포인터 */

    memset(m_time, 0, sizeof (m_time));
    memset(&W_Fmt, ' ', sizeof (FILE_BUFF_FORMAT));

    for (i = 0; i < R_Cnt; i++) {
        i_hd = (SEARCH_HEADER *)R_Fmt[i].Data;

        if (memcmp(i_hd->TrCode, "500100", 6) == 0) {
            /* 전략 기동 요청: ApType_Cd 범위 검증 (5010~5999) */
            if ((memcmp(i_hd->ApType_Cd, "5010", 4) >= 0) &&
                    (memcmp(i_hd->ApType_Cd, "5999", 4) <= 0)) {
                Start_Client(R_Fmt[i].Data);
            }
            else {
                /* ApType_Cd 범위 초과 → 오류 응답 */
                Log(USR_ERROR, "ApType_Cd error Input[%50.50s]", i_hd->ApType_Cd);
                memset(&W_Fmt, ' ', sizeof (FILE_BUFF_FORMAT));
                memcpy(W_Fmt.Data, &R_Fmt[i].Data, sizeof (SEARCH_HEADER));
                memcpy(W_Fmt.Data, "500120", 6);   /* 오류 응답 TR코드 */
                Write_Data(-1);
            }
        }
        else {
            /* TR코드 오류 → 오류 응답 */
            Log(USR_ERROR, "TR code error Input[%50.50s]", i_hd->TrCode);
            memset(&W_Fmt, ' ', sizeof (FILE_BUFF_FORMAT));
            memcpy(W_Fmt.Data, &R_Fmt[i].Data, sizeof (SEARCH_HEADER));
            memcpy(W_Fmt.Data, "500120", 6);       /* 오류 응답 TR코드 */
            Write_Data(-1);
        }

        Add_Count(PS_R_1, 1);              /* 처리 건수 카운터 증가 */
        INT_SEQ ++;                         /* 내부 시퀀스 번호 증가 */
        Set_TR_Time();                     /* TR 시간 갱신 */
    }

    return;
}   /* End of Analyze_Data () */

/*************************************************************************
    Function        : Start_Client
    Parameters IN   : p_buf - 전략 기동 요청 데이터 포인터
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 자동매매 전략 프로세스를 기동한다.
                      1) 사용 가능한 프로세스 슬롯 검색 (라운드로빈)
                      2) 전략 바이너리를 슬롯 바이너리로 복사 (cp)
                      3) Daemon에 기동 신호 전달 (write DTART_FD)
                      4) 성공 응답(500110) 전달

    프로세스 슬롯 검색 규칙:
      - 최대 MAX_AUTO_PROC(40)개 슬롯
      - 이전 기동 위치(RR)부터 순환 검색
      - process_status != 1인 슬롯만 사용 가능
      - 프로세스명: pa_50{N}{M}mp (N=슬롯/10+1, M=슬롯%10)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Start_Client(char *p_buf) {
    int                     pk, w_flag, i, rt, flag, bin_type, m;
    int                     auto_f_tcnt, auto_f_scnt, auto_m_tcnt, auto_m_scnt;
    char                    pname[12], cmd[256], m_time[20], chk_proc[20];
    char                    st_name[20], tg_name[10];
    int                     search_seq, get_proc;
    SEARCH_HEADER           *i_hd;

    memset(m_time, 0, sizeof(m_time));
    Get_Time(m_time);

    memset(&W_Fmt, ' ', sizeof (FILE_BUFF_FORMAT));
    memcpy(W_Fmt.Data, p_buf, DATA_SIZE);

    i_hd = (SEARCH_HEADER *)p_buf;

    /* 전략 바이너리명 생성: pa_{ApType_Cd}_mp */
    memset(st_name, 0, sizeof (st_name));
    sprintf(st_name, "pa_%4.4s_mp", i_hd->ApType_Cd);

    /*
     * 사용 가능한 프로세스 슬롯 검색 (라운드로빈):
     * RR부터 시작하여 MAX_AUTO_PROC 슬롯을 순회하며
     * process_status != 1(가동중이 아닌) 슬롯을 찾는다
     */
    get_proc = 0;
    search_seq = RR;
    for (i = 0; i < MAX_AUTO_PROC; i++) {
        if (search_seq >= MAX_AUTO_PROC)
            search_seq = 1;
        else
            search_seq++;

        /* 슬롯 번호로 프로세스명 생성 */
        memset(pname, 0, sizeof (pname));
        if (search_seq % 10 == 0)
            sprintf(pname, "%2.2s_50%01d%02dmp", _SubSystem_Name, search_seq/10, 10);
        else
            sprintf(pname, "%2.2s_50%01d%02dmp", _SubSystem_Name, search_seq/10+1, search_seq%10);

        /* Daemon 프로세스 목록에서 해당 프로세스 검색 */
        for (pk = 0; pk < DAEMON(D_K).p_count; pk++) {
            if (memcmp(PROC(D_K,pk).process_id, pname, 10) == 0) {
                if (PROC(D_K,pk).process_status == 1)
                    continue;               /* 이미 가동중 → 다음 검색 */
                else {
                    get_proc = 1;           /* 사용 가능한 슬롯 발견 */
                    IDR(D_K,pk,0,0) = IDW(D_K,pk,0,0);
                    break;
                }
            }

            if (pk == DAEMON(D_K).p_count - 1) {
                /* 프로세스명이 Daemon에 미등록 */
                Log(USR_ERROR, "Not Search process Name[%s] p_count[%d] pk[%d]",
                        pname, DAEMON(D_K).p_count, pk);
                memcpy(W_Fmt.Data, "500120", 6);   /* 오류 응답 */
                Write_Data(-1);
                return;
            }
        }

        if (get_proc == 0 && i >= MAX_AUTO_PROC - 1) {
            /* 모든 슬롯이 사용중 → 기동 불가 */
            Log(USR_ERROR, "Fail Auto Run, process Full Run input[%50.50s]", i_hd->TrCode);
            memcpy(W_Fmt.Data, "500120", 6);       /* 오류 응답 */
            Write_Data(-1);
            return;
        }

        if (get_proc > 0) {
            RR = search_seq;                /* 다음 검색 시작점 갱신 */
            break;
        }
    }

    /*
     * 전략 바이너리 복사: cp st_name → pname
     * 예) cp bin/pa_5010_mp bin/pa_50101mp
     */
    Log(USR_OK, "cp [%s/%s %s/%s]", _FEP_BIN, st_name, _FEP_BIN, pname);

    sprintf(cmd, "cp %s/%s %s/%s", _FEP_BIN, st_name, _FEP_BIN, pname);

    rt = system(cmd);

    if (rt < 0 && SYS_NO != 10) {
        Log(SYS_FATAL, "system call fail[%s] {%d:%s}",
                cmd, SYS_NO, SYS_STR);
        Exit_Process();
    }

    /* Daemon에 프로세스 기동 신호 전달 */
    PROC(D_K,pk).start_status = JOB_INIT;
    PROC(D_K,pk).process_status = 1;        /* 가동중 표시 */
    write(DTART_FD, "1", 1);               /* Daemon에 기동 신호 */

    /* 성공 응답(500110) 전달 */
    memcpy(W_Fmt.Data, "500110", 6);

    memset(tg_name, 0, sizeof (tg_name));
    sprintf(tg_name, "%5.5s", &pname[3]);
    memcpy(&W_Fmt.Data[14], tg_name, 5);   /* 기동된 프로세스 번호 기록 */
    Write_Data(1);

    return;
}   /* End of Start_Client () */

/*************************************************************************
    Function        : Write_Data
    Parameters IN   : p_flag - 쓰기 플래그 (1=성공, -1=오류)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : W_Fmt 버퍼의 내용을 Client FIFO(TS_W1_1)에 기록.
                      줄바꿈(LineFeed) 추가 후 FIFO에 1건 쓰기.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Write_Data(int p_flag) {
    int     rt;

    W_Fmt.LineFeed[0] = '\n';               /* 레코드 종단 마커 */

    rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);

    if (rt != 1) {
        Log(SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,0));
        Exit_Process();
    }

    Log(USR_OK, "file write[%s:%d:%d]",
            OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), rt);

    return;
}   /* End of Write_Data () */

/*************************************************************************
    End of Program (pa_9000_mp.c)
*************************************************************************/
