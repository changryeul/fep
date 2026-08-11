#define     _GLOBAL
/*------------------------------------------------------------------------
 *  Module  : 현물시세수신 DD (Data Distribution)
 *  File    : pa_7100_dd.c
 *
 *  FIFO에서 수신한 시세 데이터를 공유메모리(SHM)에 기록하는 프로세스.
 *  빌드 옵션(-D)에 따라 처리하는 시세 종류가 달라진다:
 *
 *    A7201 : 금융파생 종목정보 (CO_A001F → Shm_FinFut, 1400B)
 *    A7102 : 채권시세 수신 (A301K/G701K/B601K → Shm_Note, 700B)
 *    A7103 : 채권시세 수신 (동일, 100B)
 *    A7202 : 파생시세 수신 (A306F/G706F/B606F → Shm_FinFut, 450B)
 *    A7203 : 파생시세 수신 (동일, 100B)
 *    A7181 : RDS01 채권종목정보 배치 (→ Shm_Rds01, 1200B)
 *    A7182 : RDS02 KTS종목정보 배치 (→ Shm_Note, 350B)
 *
 *  처리 흐름:
 *    1) FIFO에서 시세 데이터 읽기 (F_R)
 *    2) Set_Sise()에서 TR코드별 분류 후 SHM에 기록
 *    3) 종목정보 마지막(SEQ=999999) 수신 시 qsort로 Key 정렬
 *    4) 한도체크용 STANDARD_SISE_FORMAT 설정 (상하한가, 승수 등)
 *------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
 *  Header Files
 *------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

/*------------------------------------------------------------------------
 *  빌드 옵션별 데이터 크기 설정
 *  - 각 빌드 옵션마다 수신하는 시세 전문의 크기가 다름
 *------------------------------------------------------------------------*/
#if defined A7102
#define     DATA_SIZE       700     /* 채권시세(체결+호가) */
#elif defined(A7103) || defined(A7203)
#define     DATA_SIZE       100     /* 채권/파생시세(장운영) */
#elif defined A7201
#define     DATA_SIZE       1400    /* 금융파생 종목정보 */
#elif defined A7202
#define     DATA_SIZE       450     /* 파생시세(체결+호가) */
#elif defined A7181
#define     DATA_SIZE       1200    /* RDS01 채권종목정보 */
#elif defined A7182
#define     DATA_SIZE       350     /* RDS02 KTS종목정보 */
#endif

#include    "buf_struct.h"
/*------------------------------------------------------------------------
 *  Constants and Structures
 *------------------------------------------------------------------------*/
#define     DATA_TIME       60 * 1000   /* Poll 타임아웃: 60초(밀리초) */
/* 시세 Seq 위치 Size */
#define     S_H_SIZE        5           /* TR코드(5바이트) 뒤가 SEQ 시작 */
/* 기본마스터 Seq 위치 Size */
#define     M_H_SIZE        27  /* TR(5) + SEQ(8) + JCNT(6) + DATE(8) */

/*------------------------------------------------------------------------
 *  종목코드 검색용 Key 구조체
 *  - A7181/A7182/A7102/A7103/A7203: 채권용 Key (KS_NOTE_EXPCODE)
 *  - A7201/A7202: 파생용 Key (KS_EXPCODE)
 *------------------------------------------------------------------------*/
#if defined(A7181) || defined(A7182) || defined(A7102) || defined(A7103) || defined(A7203)
KS_NOTE_EXPCODE     Key;
#elif defined(A7201) || defined(A7202)
KS_EXPCODE          Key;
#endif

int         Che_Gbn;                /* 체결구분 플래그 (1:정상, -1:미분류) */
char        ApType[10];             /* 프로세스 유형 문자열 (예: "PA7100DD") */
FILE_BUFF_FORMAT        W_Fmt;      /* FIFO 쓰기 버퍼 */
BUFF_RW_HEAD            f_head;     /* 버퍼 읽기/쓰기 헤더 */

/*------------------------------------------------------------------------
 *  Global Variables
 *------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
 *  Function Prototypes
 *------------------------------------------------------------------------*/
void    PA_7100_DD(void);
void    Set_Sise(char *);

/*------------------------------------------------------------------------
 *  main: 프로세스 초기화 → 시세수신 루프 → 종료
 *------------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);     /* 프로세스 초기화 (SHM 연결, FIFO 오픈 등) */
    PA_7100_DD();              /* 메인 시세수신 루프 */
    Exit_Process();            /* 프로세스 정상 종료 */
}   /* End of main ()   */

/*------------------------------------------------------------------------
 *  PA_7100_DD: 메인 이벤트 루프
 *    - FIFO에서 시세 데이터를 반복 읽기
 *    - 읽은 데이터를 Set_Sise()로 SHM에 기록
 *    - 데이터 없으면 Poll_File()로 대기 (60초 타임아웃)
 *------------------------------------------------------------------------*/
void    PA_7100_DD(void) {
    int             i, rt, len, R_Cnt;
    char            m_time[24];
    char            fifo_name[100], bumun[4];
    char            W2_Fmt[2048];
    FILE_BUFF_FORMAT    R_Fmt[1];   /* FIFO 읽기 버퍼 (1건씩 처리) */

    /*--------------------------------------------------------------------
     *  기초정보 재기동시 초기화 작업
     *  - 빌드 옵션에 따라 SHM 영역을 초기화
     *--------------------------------------------------------------------*/
#if defined A7201
    /* A7201: 금융파생 종목정보 SHM 초기화 */
    Shm_FinFut[0].total_item_cnt = 0;
    Shm_Risk[0].Max_Seq[1] = 0; /* 조회시 수신받은 seq가 Max를 넘는지 체크 */

    /* A0(종목정보) 영역 초기화 */
    for (i = 0; i < SHM_MAX_DEV_FIF; i++) {
        memset(Shm_FinFut[i].A0.tr_gbn, 0, sizeof (CO_A001F));
    }

    /* 종목코드 검색 Key 초기화 */
    memset(Shm_Item[0].D_Key, 0, sizeof (KS_EXPCODE) * SHM_MAX_DEV_FIF);

#elif defined A7181
    /* A7181: RDS01 채권종목정보 SHM 초기화 */
    Shm_Note[2].total_item_cnt = 0; /* RDS01의 총 종목수량은 KTS의 [2]에서 관리 */

    /* A0(종목정보) 영역 초기화 */
    for (i = 0; i < SHM_MAX_RDS01; i++) {
        memset(Shm_Rds01[i].seq_no, 0, sizeof (CO_A001_RDS01));
    }

    /* Key 초기화 , 미사용
    memset (Shm_Item[0].D_Key, 0, sizeof (KS_EXPCODE) * SHM_MAX_DEV_FIF); */

#elif defined A7182
    /* A7182: RDS02 KTS종목정보 - 반드시 채권종목(A7181) 배치 이후 실행 */
    if (Shm_Note[2].total_item_cnt <= 0) {
        Log(USR_OK, "채권종목정보 배치처리 전입니다. 60초 sleep ");
        sleep(60);
        return;
    }

    Shm_Note[0].total_item_cnt = 0;
    Shm_Risk[0].Max_Seq[0] = 0; /* 조회시 수신받은 seq가 Max를 넘는지 체크, 미사용 */

    /* A0(종목정보) 영역 초기화 */
    for (i = 0; i < SHM_MAX_NOTE; i++) {
        memset(Shm_Note[i].A0.seq_no, 0, sizeof (CO_A001_RDS02));
    }

    /* 종목코드 검색 Key 초기화 */
    memset(Shm_Item[0].N_Key, 0, sizeof (KS_NOTE_EXPCODE) * SHM_MAX_NOTE);

#else
    /* 기타: ApType 문자열 생성 (예: "PA7100DD") */
    sprintf(ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));
#endif

    /*--------------------------------------------------------------------
     *  메인 이벤트 루프
     *  - 외부 루프: Poll_File()로 FIFO 이벤트 대기
     *  - 내부 루프: FIFO에서 데이터 읽어 Set_Sise() 호출
     *--------------------------------------------------------------------*/
    while (START_S != JOB_END) {
        Stat_Save();   /* 프로세스 상태 저장 (살아있음 표시) */

        while (START_S != JOB_END) {
            Stat_Save();
            memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT));

            /* FIFO에서 시세 데이터 1건 읽기 */
            R_Cnt = F_R(PS_R_1, (void *)R_Fmt, 1);
            if (R_Cnt < 0) {
                Log(SAM_FATAL, "cannot read file[%s,%d:%s]",
                        IFN(D_K,P_K,0), SYS_NO, SYS_STR);
                sleep(1);
                Exit_Process();
            }
            else if (R_Cnt == 0)
                break;  /* 읽을 데이터 없음 → Poll 대기로 */

            /* 수신 데이터를 SHM에 기록 */
            Set_Sise(R_Fmt[0].Data);

            Set_TR_Time();     /* TR 시각 갱신 */
            INT_SEQ ++;         /* 내부 시퀀스 번호 증가 */

            Add_Count(PS_R_1, 1);   /* 처리 건수 카운트 */
        }

        /* FIFO 이벤트 대기 (60초 타임아웃) */
        rt = Poll_File(DATA_TIME);

        if (rt == 1)
            Log(USR_OK, "poll timeout <%d>", INT_SEQ);
        else if (rt == -1)
            continue;   /* 에러 시 재시도 */
    }

    return;
}   /* End of PA_7100_DD () */

/*************************************************************************
 *  Function        : Set_Sise
 *  Parameters IN   : p_buf - 수신된 시세 데이터 (TR코드 5바이트 + 본문)
 *  Parameters OUT  : 없음 (SHM에 직접 기록)
 *  Return Code     : void
 *  Comment         : 시세 데이터를 TR코드별로 분류하여 SHM에 기록
 *
 *  빌드 옵션별 처리:
 *    A7201: A006F(금융파생종목) → Shm_FinFut[].A0, qsort Key, 한도Setting
 *    A7102/A7103: A301K(채권체결)/G701K(호가)/B601K(우선호가) → Shm_Note
 *    A7202/A7203: A306F(파생체결)/G706F(호가)/B606F(우선호가) → Shm_FinFut
 *    A7181: RDS01 채권종목정보 → Shm_Rds01 (중복검사 포함)
 *    A7182: RDS02 KTS종목정보 → Shm_Note (중복검사, qsort, 한도Setting)
 *************************************************************************/
void    Set_Sise(char *p_buf) {
    int     ii, s_k, idx, rt, sign;
    char    m_time[24], TrCode[10];
    int     Idx;

    memset(m_time, 0, sizeof (m_time));
    Get_MicroTime(m_time);

    /*====================================================================
     *  A7201: 금융파생 종목정보 (A006F)
     *  - SHM_FinFut에 종목정보 기록
     *  - 마지막 데이터(999999999999) 수신 시:
     *    → 한도체크용 STANDARD_SISE_FORMAT 설정
     *    → 종목코드 qsort 정렬 후 Key 보관
     *====================================================================*/
#if defined A7201
    if (memcmp(p_buf, "A006F", 5) == 0) {
        /* SEQ 번호를 인덱스로 사용하여 SHM에 기록 */
        idx = AtoIf(&p_buf[S_H_SIZE], sizeof (Shm_FinFut[0].A0.seq_no));
        memcpy(Shm_FinFut[idx].A0.tr_gbn, p_buf, sizeof (CO_A001F));

        /* 최대 종목 수 갱신 */
        if (Shm_FinFut[0].total_item_cnt < idx) Shm_FinFut[0].total_item_cnt = idx;

        /*------------------------------------------------------------
         *  시세마지막(종목코드 999999999999) 수신 시
         *  → 한도체크용 STANDARD_SISE_FORMAT 일괄 설정
         *  → 종목코드 qsort 처리하여 Key 보관
         *------------------------------------------------------------*/
        if (memcmp(&p_buf[M_H_SIZE], "999999999999", sizeof (Shm_FinFut[0].A0.item_code)) == 0) {
            /* 조회시 수신받은 seq가 Max를 넘는지 체크용 */
            Shm_Risk[0].Max_Seq[1] = Shm_FinFut[0].total_item_cnt;

            for (ii = 1; ii < Shm_FinFut[0].total_item_cnt; ii++) {
                /* 현재가(기준가격): 11자리, 첫째자리는 부호(+/-) */
                if (memcmp(Shm_FinFut[ii].A0.base_prc, "-", 1) == 0)
                    sign = -1;
                else
                    sign = 1;
                Shm_Risk[0].S_Sise[1][ii].crprc = sign * (double)AtoDf(&Shm_FinFut[ii].A0.base_prc[1], sizeof (Shm_FinFut[0].A0.base_prc)-1);

                /* *************************************** */
                /* 한도체크용 STANDARD_SISE_FORMAT Setting  */
                /* *************************************** */
                /* 기준가격: 소수점으로 수신, 1번은 부호 */
                Shm_Risk[0].S_Sise[1][ii].m_stdard_price = Shm_Risk[0].S_Sise[1][ii].crprc;
                /* 종목코드 */
                memcpy(Shm_Risk[0].S_Sise[1][ii].m_item_cd,
                        Shm_FinFut[ii].A0.item_code, sizeof (Shm_FinFut[0].A0.item_code));
                /* 종목거래가능 여부 */
                memcpy(Shm_Risk[0].S_Sise[1][ii].m_item_stat,
                        Shm_FinFut[ii].A0.trade_susp_yn, sizeof (Shm_FinFut[0].A0.trade_susp_yn));
                /* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */
                /* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만(현물용)
                memcpy (Shm_Risk[0].S_Sise[1][ii].nb_shares_stock_1per,
                    &Shm_FinFut[ii].A0.listed_stock[3], 10);    */
                /* 지정가호가조건코드, 지정가호가취소조건코드 */
                Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_2 = AtoIf(&Shm_FinFut[ii].A0.lim_ord_cancel_cond_cd[4], 1);
                /* 조건부지정가 호가조건코드, 조건부지정가호가취소조건코드 */
                Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_I = AtoIf(&Shm_FinFut[ii].A0.cond_lim_ord_cancel_cond_cd[4], 1);
                /* 시장가 호가조건코드(1대신 파생용) */
                Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_T = AtoIf(&Shm_FinFut[ii].A0.mkt_ord_cancel_cond_cd[4], 1);
                /* 최유리지정가 호가조건코드(X대신 파생용) */
                Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_W = AtoIf(&Shm_FinFut[ii].A0.best_lim_ord_cancel_cond_cd[4], 1);
                /* 시장가호가조건코드 (현물용)
                Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_1 = AtoIf (&Shm_FinFut[ii].A0.mkt_ord_cancel_cond_cd[4], 1);    */
                /* 최유리지정가 호가조건코드(현물용)
                Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_X = AtoIf (&Shm_FinFut[ii].A0.best_lim_ord_cancel_cond_cd[4], 1);   */

                /* 상한수량: 16자리인데 종목수량은 10이라 10자리만 처리 */
                Shm_Risk[0].S_Sise[1][ii].m_max_qty = (double)AtoDf(&Shm_FinFut[ii].A0.up_qty[6], 10);
                /* 정리매매 여부(현물용)
                memcpy (Shm_Risk[0].S_Sise[1][ii].m_clearance_gbn, Shm_FinFut[ii].A0.clear_gubun, 1);       */
                /* 시가기준가종목(현물용)
                memcpy (Shm_Risk[0].S_Sise[1][ii].m_start_price_gbn, Shm_FinFut[ii].A0.open_st_price, 1);       */
                /* 최고호가가격(현물용)
                Shm_Risk[0].S_Sise[1][ii].m_top_price = (double)AtoDf(Shm_FinFut[ii].A0.high_bid, 11);  */
                /* 최저호가가격(현물용)
                Shm_Risk[0].S_Sise[1][ii].m_lowest_price = (double)AtoDf(Shm_FinFut[ii].A0.low_bid, 11);    */

                /*----------------------------------------------------
                 *  가격제한 최종단계별 상하한가 설정
                 *    001: 1단계, 002: 2단계, 003: 3단계
                 *    - 각 단계별 상한가/하한가를 SHM에 설정
                 *    - 첫 바이트가 '-'이면 음수 처리
                 *----------------------------------------------------*/
                if (memcmp(Shm_FinFut[ii].A0.prc_lmt_finl_stg, "001", 3) == 0) {
                    /* 상한가 (1단계) */
                    Shm_Risk[0].S_Sise[1][ii].m_h_lmt = (double)AtoDf(&Shm_FinFut[ii].A0.prc_lmt_stg1_up[1], sizeof (Shm_FinFut[0].A0.prc_lmt_stg1_up)-1);
                    if (memcmp(Shm_FinFut[ii].A0.prc_lmt_stg1_up, "-", 1) == 0)
                        Shm_Risk[0].S_Sise[1][ii].m_h_lmt = Shm_Risk[0].S_Sise[1][ii].m_h_lmt * -1;
                    /* 하한가 (1단계) */
                    Shm_Risk[0].S_Sise[1][ii].m_l_lmt = (double)AtoDf(&Shm_FinFut[ii].A0.prc_lmt_stg1_lo[1], sizeof (Shm_FinFut[0].A0.prc_lmt_stg1_up)-1);
                    if (memcmp(Shm_FinFut[ii].A0.prc_lmt_stg1_lo, "-", 1) == 0)
                        Shm_Risk[0].S_Sise[1][ii].m_l_lmt = Shm_Risk[0].S_Sise[1][ii].m_l_lmt * -1;
                }
                else if (memcmp(Shm_FinFut[ii].A0.prc_lmt_finl_stg, "002", 3) == 0) {
                    /* 상한가 (2단계) */
                    Shm_Risk[0].S_Sise[1][ii].m_h_lmt = (double)AtoDf(&Shm_FinFut[ii].A0.prc_lmt_stg2_up[1], sizeof (Shm_FinFut[0].A0.prc_lmt_stg2_up)-1);
                    if (memcmp(Shm_FinFut[ii].A0.prc_lmt_stg2_up, "-", 1) == 0)
                        Shm_Risk[0].S_Sise[1][ii].m_h_lmt = Shm_Risk[0].S_Sise[1][ii].m_h_lmt * -1;
                    /* 하한가 (2단계) */
                    Shm_Risk[0].S_Sise[1][ii].m_l_lmt = (double)AtoDf(&Shm_FinFut[ii].A0.prc_lmt_stg2_lo[1], sizeof (Shm_FinFut[0].A0.prc_lmt_stg2_up)-1);
                    if (memcmp(Shm_FinFut[ii].A0.prc_lmt_stg2_lo, "-", 1) == 0)
                        Shm_Risk[0].S_Sise[1][ii].m_l_lmt = Shm_Risk[0].S_Sise[1][ii].m_l_lmt * -1;
                }
                else if (memcmp(Shm_FinFut[ii].A0.prc_lmt_finl_stg, "003", 3) == 0) {
                    /* 상한가 (3단계) */
                    Shm_Risk[0].S_Sise[1][ii].m_h_lmt = (double)AtoDf(&Shm_FinFut[ii].A0.prc_lmt_stg3_up[1], sizeof (Shm_FinFut[0].A0.prc_lmt_stg3_up)-1);
                    if (memcmp(Shm_FinFut[ii].A0.prc_lmt_stg3_up, "-", 1) == 0)
                        Shm_Risk[0].S_Sise[1][ii].m_h_lmt = Shm_Risk[0].S_Sise[1][ii].m_h_lmt * -1;
                    /* 하한가 (3단계) */
                    Shm_Risk[0].S_Sise[1][ii].m_l_lmt = (double)AtoDf(&Shm_FinFut[ii].A0.prc_lmt_stg3_lo[1], sizeof (Shm_FinFut[0].A0.prc_lmt_stg3_up)-1);
                    if (memcmp(Shm_FinFut[ii].A0.prc_lmt_stg3_lo, "-", 1) == 0)
                        Shm_Risk[0].S_Sise[1][ii].m_l_lmt = Shm_Risk[0].S_Sise[1][ii].m_l_lmt * -1;
                }
                /* 승수 (거래승수: 계약당 금액 산정에 사용) */
                Shm_Risk[0].S_Sise[1][ii].m_multiplier = AtoIf(&Shm_FinFut[ii].A0.trade_mult[1], sizeof (Shm_FinFut[0].A0.trade_mult)-1);
                /* 행사가격: 18자리중 12자리(소수점2자리)까지 처리, 2025 확인필요 */
                Shm_Risk[0].S_Sise[1][ii].m_striking_price = AtoDf(&Shm_FinFut[ii].A0.strike_prc[6], sizeof (Shm_FinFut[0].A0.strike_prc)-6);
                if (memcmp(Shm_FinFut[ii].A0.strike_prc, "-", 1) == 0)
                    Shm_Risk[0].S_Sise[1][ii].m_striking_price = Shm_Risk[0].S_Sise[1][ii].m_striking_price * -1;
                /* 거래종료일 (파생용, 8자리 YYYYMMDD) */
                memcpy(Shm_Risk[0].S_Sise[1][ii].m_closeday, Shm_FinFut[ii].A0.delist_date, 8);
                /* 기초자산전일종가.. (파생용) 이건 위탁만 조사.. 뺀다.
                memcpy (Shm_Risk[0].S_Sise[1][ii].basic_asset_price, Shm_FinFut[ii].A0., 8);      */
                /* 증권 그룹ID (현물용)
                memcpy (Shm_Risk[0].S_Sise[1][ii].m_group_id, Shm_FinFut[ii].A0.group_id, 2); */
                /* *************************************** */

                /* qsort용 Key 저장: idx + 종목코드 */
                Shm_Item[0].D_Key[ii].idx = ii;
                memcpy(Shm_Item[0].D_Key[ii].expcode, Shm_FinFut[ii].A0.item_code,
                        sizeof (Shm_FinFut[0].A0.item_code));
            }
            /* 종목코드 오름차순 정렬 → 이진검색 가능하게 함 */
            qsort(Shm_Item[0].D_Key, Shm_FinFut[0].total_item_cnt, sizeof (KS_EXPCODE), CmpExpcode);
        }
    }

    /*====================================================================
     *  A7102/A7103: 채권시세 수신
     *  - A301K(체결): crprc(현재가) → Shm_Risk
     *  - G701K(호가): crprc + sell_1_price + buy_1_price → Shm_Risk
     *  - B601K(우선호가): sell_1_price + buy_1_price → Shm_Risk
     *  - M401K(장운영스케줄): Shm_Note[0].M4에 기록
     *  - A701K(장운영): Shm_Note[0].A7에 기록
     *====================================================================*/
#elif defined(A7102) || defined(A7103)
    /* 수신시세 => Memory 시세에 Set */
    if (memcmp(p_buf, "A301K", 5) == 0) {
        /* A301K: 채권체결 (223바이트) */
        Che_Gbn = 1;
        /* SEQ 번호를 인덱스로 사용 */
        Idx = AtoIf(&p_buf[S_H_SIZE], sizeof(Shm_Note[0].A3.seq_no));
        memcpy(Shm_Note[Idx].A3.tr_gbn, p_buf, sizeof (CO_A301K));

        /* 공용현재가_체결가: 7.2(소수점 아래 2자리) */
        Shm_Risk[0].S_Sise[0][Idx].crprc =
        (double)AtoDf(Shm_Note[Idx].A3.crprc, sizeof (Shm_Note[0].A3.crprc));
    }
    else if (memcmp(p_buf, "G701K", 5) == 0) {
        /* G701K: 일반채권/국고채권 체결+우선호가 (643바이트) */
        Che_Gbn = 1;
        Idx = AtoIf(&p_buf[S_H_SIZE], sizeof(Shm_Note[0].G7.seq_no));
        Shm_Note[Idx].HogaLastGbn = 0;      /* 호가 최종 구분: 0=일반 */
        memcpy(Shm_Note[Idx].G7.tr_gbn, p_buf, sizeof (CO_G701K));

        /* 공용 현재가 */
        Shm_Risk[0].S_Sise[0][Idx].crprc =
        (double)AtoDf(Shm_Note[Idx].G7.crprc, sizeof (Shm_Note[0].G7.crprc));
        /* 공용 매도1호가가격 */
        Shm_Risk[0].S_Sise[0][Idx].sell_1_price =
        (double)AtoDf(Shm_Note[Idx].G7.ask1_price, sizeof (Shm_Note[0].G7.ask1_price));
        /* 공용 매수1호가가격 */
        Shm_Risk[0].S_Sise[0][Idx].buy_1_price =
        (double)AtoDf(Shm_Note[Idx].G7.bid1_price, sizeof (Shm_Note[0].G7.bid1_price));
    }
    else if (memcmp(p_buf, "B601K", 5) == 0) {
        /* B601K: 일반채권/국고채권 우선호가 (462바이트) */
        Che_Gbn = 1;
        Idx = AtoIf(&p_buf[S_H_SIZE], sizeof(Shm_Note[0].B6.seq_no));
        Shm_Note[Idx].HogaLastGbn = 1;      /* 호가 최종 구분: 1=우선호가 */
        memcpy(Shm_Note[Idx].B6.tr_gbn, p_buf, sizeof (CO_B601K));

        /* 공용 매도1호가가격 */
        Shm_Risk[0].S_Sise[0][Idx].sell_1_price =
        (double)AtoDf(Shm_Note[Idx].B6.ask1_price, sizeof (Shm_Note[0].B6.ask1_price));
        /* 공용 매수1호가가격 */
        Shm_Risk[0].S_Sise[1][Idx].buy_1_price =
        (double)AtoDf(Shm_Note[Idx].B6.bid1_price, sizeof (Shm_Note[0].B6.bid1_price));
    }
    else if (memcmp(p_buf, "M401K", 5) == 0) {
        /* M401K: 장운영스케줄 (83바이트) */
        memcpy(Shm_Note[0].M4.tr_gbn, p_buf, sizeof (CO_M401K));
    }
    else if (memcmp(p_buf, "A701K", 5) == 0) {
        /* A701K: 장운영 (68바이트) */
        memcpy(Shm_Note[0].A7.tr_gbn, p_buf, sizeof (CO_A701A));
    }
    else {
        /* 미분류 TR코드 → 로그만 기록 */
        Log(USR_OK, "Other TrCode[%s][%d]", p_buf, sizeof(p_buf));
        Che_Gbn = -1;
    }

    /*====================================================================
     *  A7202/A7203: 파생시세 수신
     *  - A306F(체결): crprc + 실시간상하한가 → Shm_Risk
     *  - G706F(호가): crprc + 실시간상하한가 + 매도수1호가 → Shm_Risk
     *  - B606F(우선호가): 매도수1호가 → Shm_Risk
     *  - M406F(장운영스케줄): Shm_FinFut[0].M4에 기록
     *  - 파생은 부호 처리가 필요함 (첫 바이트 '-' 체크)
     *====================================================================*/
#elif defined(A7202) || defined(A7203)
    /* 수신시세 => Memory 시세에 Set */
    if (memcmp(p_buf, "A306F", 5) == 0) {
        /* A306F: 파생체결 */
        Che_Gbn = 1;
        Idx = AtoIf(&p_buf[S_H_SIZE], sizeof(Shm_FinFut[0].A3.seq_no));
        memcpy(Shm_FinFut[Idx].A3.tr_gbn, p_buf, sizeof (CO_A301F));

        /* 공용 현재가 (부호 처리: 첫 바이트 '-' → 음수) */
        if (memcmp(Shm_FinFut[Idx].A3.crprc, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].crprc =
        sign *
        (double)AtoDf(&Shm_FinFut[Idx].A3.crprc[1], sizeof (Shm_FinFut[0].A3.crprc) - 1);
        /* 공용 실시간상한가가격 */
        if (memcmp(Shm_FinFut[Idx].A3.dyn_upper_limit, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].realtime_hprc =
        (double)AtoDf(&Shm_FinFut[Idx].A3.dyn_upper_limit[1],
                sizeof (Shm_FinFut[0].A3.dyn_upper_limit) - 1);
        /* 공용 실시간하한가가격 */
        if (memcmp(Shm_FinFut[Idx].A3.dyn_lower_limit, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].realtime_lprc =
        (double)AtoDf(&Shm_FinFut[Idx].A3.dyn_lower_limit[1],
                sizeof (Shm_FinFut[0].A3.dyn_lower_limit) - 1);
    }
    else if (memcmp(p_buf, "G706F", 5) == 0) {
        /* G706F: 파생호가 */
        Che_Gbn = 1;
        Idx = AtoIf(&p_buf[S_H_SIZE], sizeof(Shm_FinFut[0].A3.seq_no));
        Shm_FinFut[Idx].HogaLastGbn = 0;    /* 호가 최종 구분: 0=일반 */
        memcpy(Shm_FinFut[Idx].G7.tr_gbn, p_buf, sizeof (CO_G701F));

        /* 공용 현재가 (부호 처리) */
        if (memcmp(Shm_FinFut[Idx].G7.crprc, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].crprc =
        sign *
        (double)AtoDf(&Shm_FinFut[Idx].G7.crprc[1], sizeof (Shm_FinFut[0].G7.crprc) - 1);
        /* 공용 실시간상한가가격 */
        if (memcmp(Shm_FinFut[Idx].G7.dyn_upper_limit, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].realtime_hprc =
        (double)AtoDf(&Shm_FinFut[Idx].G7.dyn_upper_limit[1],
                sizeof(Shm_FinFut[0].G7.dyn_upper_limit) - 1);
        /* 공용 실시간하한가가격 */
        if (memcmp(Shm_FinFut[Idx].G7.dyn_lower_limit, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].realtime_lprc =
        (double)AtoDf(&Shm_FinFut[Idx].G7.dyn_lower_limit[1],
                sizeof(Shm_FinFut[0].G7.dyn_lower_limit) - 1);
        /* 공용 매도1호가가격 (부호 처리) */
        if (memcmp(Shm_FinFut[Idx].G7.ask1_price, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].sell_1_price =
        (double)AtoDf(&Shm_FinFut[Idx].G7.ask1_price[1],
                sizeof(Shm_FinFut[0].G7.ask1_price) - 1);
        /* 공용 매수1호가가격 (부호 처리) */
        if (memcmp(Shm_FinFut[Idx].G7.bid1_price, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].buy_1_price =
        (double)AtoDf(&Shm_FinFut[Idx].G7.bid1_price[1],
                sizeof(Shm_FinFut[0].G7.bid1_price) - 1);
    }
    else if (memcmp(p_buf, "B606F", 5) == 0) {
        /* B606F: 파생 우선호가 */
        Che_Gbn = 1;
        Idx = AtoIf(&p_buf[S_H_SIZE], sizeof(Shm_FinFut[0].A3.seq_no));
        Shm_FinFut[Idx].HogaLastGbn = 1;    /* 호가 최종 구분: 1=우선호가 */
        memcpy(Shm_FinFut[Idx].B6.tr_gbn, p_buf, sizeof (CO_B601F));

        /* 공용 매도1호가가격 (부호 처리) */
        if (memcmp(Shm_FinFut[Idx].B6.ask1_price, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].sell_1_price =
        (double)AtoDf(&Shm_FinFut[Idx].B6.ask1_price[1],
                sizeof(Shm_FinFut[0].B6.ask1_price) - 1);
        /* 공용 매수1호가가격 (부호 처리) */
        if (memcmp(Shm_FinFut[Idx].B6.bid1_price, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].buy_1_price =
        (double)AtoDf(&Shm_FinFut[Idx].B6.bid1_price[1],
                sizeof(Shm_FinFut[0].B6.bid1_price) - 1);
    }
    else if (memcmp(p_buf, "M406F", 5) == 0) {
        /* M406F: 파생 장운영스케줄 */
        memcpy(Shm_FinFut[0].M4.tr_gbn, p_buf, sizeof (CO_A701A));
    }
    else {
        /* 미분류 TR코드 */
        Log(USR_OK, "Other TrCode[%s][%d]", p_buf, sizeof(p_buf));
        Che_Gbn = -1;
    }

    /*====================================================================
     *  A7181: RDS01 채권종목정보 배치 수신
     *  - Shm_Rds01에 종목정보를 순차적으로 저장
     *  - 빈 슬롯(000000000000) 찾아서 기록
     *  - 중복 종목코드는 덮어쓰기
     *====================================================================*/
#elif defined A7181
    CO_A001_RDS01 *dat = (CO_A001_RDS01 *)p_buf;

    for (ii = 1; ii < SHM_MAX_RDS01; ii++) {
        /* 빈 슬롯 발견 → 신규 종목 저장 */
        if (memcmp(Shm_Rds01[ii].item_code, "000000000000", 12) <= 0) {
            Shm_Note[2].total_item_cnt++;
            memcpy(Shm_Rds01[ii].seq_no, p_buf, sizeof (CO_A001_RDS01));
            break;
        }

        /* 중복 종목코드 발견 → 최신 데이터로 갱신 */
        if (memcmp(Shm_Rds01[ii].item_code, dat->item_code, 12) == 0) {
            memcpy(Shm_Rds01[ii].seq_no, p_buf, sizeof (CO_A001_RDS01));
            return;
        }

        /* SHM 용량 초과 체크 */
        if (ii >= SHM_MAX_RDS01 - 1) {
            Log(USR_ERROR, "NOTE SHM COUNT ERROR!!! CHECK PLZ!!! ii[%d], SHM_MAX_NOTE_RDS01[%d]", ii, SHM_MAX_RDS01);
            return;
        }
    }

    /*====================================================================
     *  A7182: RDS02 KTS종목정보 배치 수신
     *  - Shm_Note에 종목정보를 순차적으로 저장
     *  - 중복 종목코드는 덮어쓰기
     *  - 마지막(999999999999) 수신 시:
     *    → 한도체크용 STANDARD_SISE_FORMAT 설정
     *    → 종목코드 qsort 정렬 후 Key 보관
     *====================================================================*/
#elif defined A7182
    /* 기접수 처리여부, 있으면 업어친다(나중거로) */

    CO_A001_RDS02 *dat = (CO_A001_RDS02 *)p_buf;

    for (ii = 1; ii < SHM_MAX_NOTE; ii++) {
        /* 빈 슬롯 발견 → 신규 종목 저장 */
        if (memcmp(Shm_Note[ii].A0.item_code, "000000000000", 12) <= 0) {
            Shm_Note[0].total_item_cnt++;
            Shm_Risk[0].Max_Seq[1] = Shm_Note[0].total_item_cnt;    /* 조회시 seq Max 체크, 999..99 안줄지도 모르니 */
            memcpy(Shm_Note[ii].A0.seq_no, p_buf, sizeof (CO_A001_RDS02));
            break;
        }

        /* 중복 종목코드 발견 → 최신 데이터로 갱신 */
        if (memcmp(Shm_Note[ii].A0.item_code, dat->item_code, 12) == 0) {
            memcpy(Shm_Note[ii].A0.seq_no, p_buf, sizeof (CO_A001_RDS02));
            return;
        }

        /* SHM 용량 초과 체크 */
        if (ii >= SHM_MAX_NOTE - 1) {
            Log(USR_ERROR, "NOTE SHM COUNT ERROR!!! CHECK PLZ!!! ii[%d], SHM_MAX_NOTE[%d]", ii, SHM_MAX_NOTE);
            return;
        }
    }

    /*----------------------------------------------------------------
     *  마지막 종목(999999999999) 수신 시 → 한도Setting + qsort
     *----------------------------------------------------------------*/
    if (memcmp(Shm_Note[ii].A0.item_code, "999999999999", 12) == 0) {
        Shm_Risk[0].Max_Seq[1] = Shm_Note[0].total_item_cnt;    /* 조회시 seq Max 체크 */

        for (ii = 1; ii < Shm_Note[0].total_item_cnt; ii++) {
            /* 현재가(기준가격): 부호(1바이트) + 가격 */
            Shm_Risk[0].S_Sise[0][ii].crprc = (double)AtoDf(&Shm_Note[ii].A0.base_prc[1], sizeof (Shm_Note[0].A0.base_prc) - 1);
            /* *************************************** */
            /* 한도체크용 STANDARD_SISE_FORMAT Setting  */
            /* 종목코드 */
            memcpy(Shm_Risk[0].S_Sise[0][ii].m_item_cd, Shm_Note[ii].A0.item_code, sizeof (Shm_Note[0].A0.item_code));
            /* 종목거래가능 여부 */
            memcpy(Shm_Risk[0].S_Sise[0][ii].m_item_stat, Shm_Note[ii].A0.trade_susp_yn, sizeof (Shm_Note[0].A0.trade_susp_yn));
            /* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */
            /* 기준가격: 양의수만사용(1)+ 소수점 미만 둘째자리 0 고정(소수점 1자리만 사용) */
            Shm_Risk[0].S_Sise[0][ii].m_stdard_price = Shm_Risk[0].S_Sise[0][ii].crprc;
            /* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만(현물용)
            memcpy (Shm_Risk[0].S_Sise[0][ii].nb_shares_stock_1per,
                &Shm_Note[ii].A0.listed_stock[3], 10);  */
            /* 지정가만 사용 가능, 조건코드 없음 */
            /* 지정가호가조건코드, 지정가호가취소조건코드
            Shm_Risk[0].S_Sise[0][ii].m_cds_gbn_2 = AtoIf (&Shm_Note[ii].A0.lim_ord_cancel_cond_cd[4], 1);  */
            /* 조건부지정가 호가조건코드, 조건부지정가호가취소조건코드
            Shm_Risk[0].S_Sise[0][ii].m_cds_gbn_I = AtoIf (&Shm_Note[ii].A0.cond_lim_ord_cancel_cond_cd[4], 1); */
            /* 시장가 호가조건코드(1대신 파생용)
            Shm_Risk[0].S_Sise[0][ii].m_cds_gbn_T = AtoIf (&Shm_Note[ii].A0.mkt_ord_cancel_cond_cd[4], 1);  */
            /* 최유리지정가 호가조건코드(X대신 파생용)
            Shm_Risk[0].S_Sise[0][ii].m_cds_gbn_W = AtoIf (&Shm_Note[ii].A0.best_lim_ord_cancel_cond_cd[4], 1); */
            /* 시장가호가조건코드 (현물용)
            Shm_Risk[0].S_Sise[0][ii].m_cds_gbn_1 = AtoIf (&Shm_Note[ii].A0.mkt_ord_cancel_cond_cd[4], 1);  */
            /* 최유리지정가 호가조건코드(현물용)
            Shm_Risk[0].S_Sise[0][ii].m_cds_gbn_X = AtoIf (&Shm_Note[ii].A0.best_lim_ord_cancel_cond_cd[4], 1); */

            /* 상한수량: 16자리인데 종목수량은 10이라 10자리만 처리, 채권은 없음
            Shm_Risk[0].S_Sise[0][ii].m_max_qty = (double)AtoDf(&Shm_Note[ii].A0.up_qty[6], 10);    */
            /* 정리매매 여부(현물용)
            memcpy (Shm_Risk[0].S_Sise[0][ii].m_clearance_gbn, Shm_Note[ii].A0.clear_gubun, 1);     */
            /* 시가기준가종목(현물용)
            memcpy (Shm_Risk[0].S_Sise[0][ii].m_start_price_gbn, Shm_Note[ii].A0.open_st_price, 1);     */
            /* 최고호가가격(현물용)
            Shm_Risk[0].S_Sise[0][ii].m_top_price = (double)AtoDf(Shm_Note[ii].A0.high_bid, 9); */
            /* 최저호가가격(현물용)
            Shm_Risk[0].S_Sise[0][ii].m_lowest_price = (double)AtoDf(Shm_Note[ii].A0.low_bid, 9);   */

            /* 가격제한 최종단계: 채권은 없음, 상하한가가 있음 */
            /* 2025 7181 읽어서 상하한가 처리하는 방식 고려 */

            /* 승수: 채권은 확인필요 2025, 우선 1,000원으로 */
            Shm_Risk[0].S_Sise[0][ii].m_multiplier = 1000;
            /* 행사가격: 18자리중 12자리(소수점2자리)까지 처리, 2025 채권확인필요
            Shm_Risk[0].S_Sise[0][ii].m_striking_price = AtoDf (Shm_Note[ii].A0.strike_prc, 12)/100;    */
            /* 거래종료일 (파생용)
            memcpy (Shm_Risk[0].S_Sise[0][ii].m_closeday, Shm_Note[ii].A0.delist_date, 8); */
            /* 기초자산전일종가.. (파생용) 이건 위탁만 조사.. 뺀다.
            memcpy (Shm_Risk[0].S_Sise[0][ii].basic_asset_price, Shm_Note[ii].A0., 8);      */
            /* 증권 그룹ID (현물용)
            memcpy (Shm_Risk[0].S_Sise[0][ii].m_group_id, Shm_Note[ii].A0.group_id, 2); */
            /* *************************************** */

            /* qsort용 Key 저장: idx + board_id + item_code */
            Shm_Item[0].N_Key[ii].idx = ii;
            memcpy(Shm_Item[0].N_Key[ii].expcode, Shm_Note[ii].A0.board_id,
                    sizeof (Shm_Note[0].A0.board_id) + sizeof (Shm_Note[0].A0.item_code));
        }
        /* 종목코드 오름차순 정렬 → 이진검색 가능하게 함 */
        qsort(Shm_Item[0].N_Key, Shm_Note[0].total_item_cnt, sizeof (KS_NOTE_EXPCODE), CmpExpcode);
    }
#endif

    return;
}   /* End of Set_Sise ()   */

/*************************************************************************
 *  End of program (pa_7100_dd.c)
 *************************************************************************/
