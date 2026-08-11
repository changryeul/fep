#define     _GLOBAL
/*------------------------------------------------------------------------
 *  Module  : 체결처리 (체결원장) — OMS 코어 공유 부문(PO)
 *  File    : po_1490_mp.c
 *
 *  KRX 회원체결결과를 수신하여 손익/잔고/미체결을 관리하는 프로세스.
 *  체결 데이터를 읽어 미체결(MK_PREMATCH) SHM에서 주문번호를 찾아 잔량을 감소시킨다.
 *
 *  시장 재편 Phase 3(po_ 코어 추출): pa_1490_mp.c(체결원장)를 po_로 이관.
 *  STATE-driven seam: 전역 미체결 SHM(MK_PREMATCH, MK_PM_SHM_KEY)을 그대로 attach.
 *
 *  [이번 슬라이스 = 채권(A1491, MK_GBN=0, 1st채널) 경로 무변경 재배치]
 *  correctness-critical 체결/미체결감소 로직 무변경 이관. `-DA1491` 대신 인소스
 *  `#define A1491`로 단일 바이너리화. 파생 경로(A2491, IMECO)는 제외 — 파생 체결은
 *  KRX TTRTDP21301로 별도 신구축(pc_1200_tr 체결 STATE-driven 소비, 별도 신중).
 *------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
 *  Header Files
 *------------------------------------------------------------------------*/
#define     A1491           /* 채권 체결처리 1st채널(MK_GBN=0). 파생 KRX 경로는 후속 */

#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "miche_idx.h"
#include    "seam_queue.h"      /* 부문간 SEAM 스테이징 큐(pc_/pb_ 수신 → 체결원장 소비) */
#include    "krx_ttrtdp21301.h" /* 현·파 회원체결결과 struct(파생 체결원장) */

/*------------------------------------------------------------------------
 *  빌드 옵션별 시장구분 및 데이터 크기
 *------------------------------------------------------------------------*/
#if defined(A1491) || defined(A1492)
#define     MK_GBN      0           /* 0: 채권 시장 */
#define     DATA_SIZE   400         /* 채권 체결전문 크기 */
#elif defined(A2491) || defined(A2492)
#define     MK_GBN      1           /* 1: 파생 시장 */
#define     DATA_SIZE   200         /* 파생 체결전문 크기 (IMECO) */
#endif

#include    "buf_struct.h"

/*------------------------------------------------------------------------
 *  Constants and Structures
 *------------------------------------------------------------------------*/
#define     DATA_TIME   60 * 1000   /* Poll 타임아웃: 60초(밀리초) */
#define     SEAM_POLL   200         /* SEAM 재스캔 주기(ms). 프로덕션 저지연은 doorbell FIFO 후속 */
#define     READ_MAX    1           /* FIFO에서 한 번에 읽는 건수 */

/*------------------------------------------------------------------------
 *  Global Variables
 *------------------------------------------------------------------------*/
int         R_Cnt, R_Max;           /* FIFO 읽기 건수 */
char        ApType[10];             /* 프로세스 유형 문자열 */
FILE_BUFF_FORMAT    W_Fmt, R_Fmt[READ_MAX]; /* FIFO 쓰기/읽기 버퍼 */
char        Seam_Buf[READ_MAX * SEAM_Q_RECSZ]; /* SEAM 슬롯(480B) 수신 버퍼 → R_Fmt로 복사 */

/* F6 miche-index: 계좌별 주문번호 조회 캐시 (프로세스-로컬, 권위는 SHM 스캔) */
static MICHE_IDX    Midx[ACC_NO_CNT];

/*------------------------------------------------------------------------
 *  Function Prototypes
 *------------------------------------------------------------------------*/
int     Init_Parameters(void);
void    Write_Data(int);
void    PO_1490_MP(void);
void    Analyze_Data(void);
int     Che_Rtn(char *, double, int, int, int);
void    Analyze_Che_Deriv(void);            /* 현·파 TTRTDP21301 체결 처리(MK_GBN 런타임 분기) */
int     Che_Rtn_Deriv(char *, double, int, int, int);  /* 파생 손익/잔고 갱신 */

/*------------------------------------------------------------------------
 *  main: 프로세스 초기화 → 체결처리 루프 → 종료
 *------------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    int     mi;

    Init_Proc(argc, argv);
    SEAM_Init();        /* 부문간 SEAM 스테이징 큐 확보(체결 → 체결원장) */

    for (mi = 0; mi < ACC_NO_CNT; mi ++)
        Miche_Idx_Reset(&Midx[mi]);

    PO_1490_MP();
    Exit_Process();
}   /* End of main ()   */

/*------------------------------------------------------------------------
 *  PO_1490_MP: 메인 이벤트 루프
 *    - FIFO에서 체결 데이터 읽기
 *    - Analyze_Data()로 체결 분석 및 SHM 갱신
 *    - 데이터 없으면 Poll_File()로 대기
 *------------------------------------------------------------------------*/
void    PO_1490_MP(void) {
    int     rt, read_flag;

    rt = Init_Parameters( );

    while (START_S != JOB_END) {
        Stat_Save();   /* 프로세스 상태 저장 */

        while (START_S != JOB_END) {
            int     si;

            Stat_Save();
            memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * READ_MAX);

            /* 부문간 전달: 파일큐 대신 전역 SEAM 체결 큐에서 소비(리더=체결원장).
               SEAM 슬롯(480B)에 FILE_BUFF_FORMAT 전체 저장 → R_Fmt로 복사 후
               Analyze_Data 파싱 바이트 동일. r_seq SHM 상주 → replay멱등. */
            R_Cnt = SEAM_R(SEAM_Q_EXEC, SEAM_R_LEDGER, Seam_Buf, READ_MAX);

            if (R_Cnt < 0) {
                Log(SAM_FATAL, "SEAM_R fail q=%d r=%d", SEAM_Q_EXEC, SEAM_R_LEDGER);
                sleep(1);
                Exit_Process();
            }
            else if (R_Cnt == 0)
                break;  /* 읽을 데이터 없음 → Poll 대기로 */

            for (si = 0; si < R_Cnt; si ++)
                memcpy(&R_Fmt[si], &Seam_Buf[si * SEAM_Q_RECSZ], sizeof (FILE_BUFF_FORMAT));

            Log(USR_OK, "SEAM RD [q=%d r=%d][%d]", SEAM_Q_EXEC, SEAM_R_LEDGER, R_Cnt);
            Analyze_Data();            /* 체결 분석 및 SHM 갱신 */
            /* SEAM 소비는 SEAM_R 시점에 r_seq 전진(파일큐 Add_Count 불필요) */
        }

        /* SEAM 재스캔 대기 */
        rt = Poll_File(SEAM_POLL);

        if (rt == 1)
            Log(USR_OK, "poll timeout <%d>", OFW(D_K,P_K,0,0));
        else if (rt == -1)
            continue;
    }
}   /* End of PO_1490_MP () */

/*************************************************************************
 *  Init_Parameters: 파라미터 초기화
 *  - ApType 문자열 생성 (예: "PA1490MP")
 *************************************************************************/
int     Init_Parameters() {
    sprintf(ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));

    return  (OK);
}

/*************************************************************************
 *  Analyze_Data: 체결 데이터 분석 및 미체결 SHM 검색
 *
 *  처리 흐름:
 *    1) 체결전문에서 TR코드 추출 (채권: 11바이트 offset, 파생: 1바이트)
 *    2) MembershipItem에서 시장구분/종목SEQ/계좌SEQ 추출
 *    3) 미체결(MiChe) SHM에서 주문번호 검색
 *    4) 찾으면: 주문잔량 감소, Che_Rtn()로 손익계산
 *    5) 못찾으면: 3초x3회 재시도 → 재처리 FIFO Write
 *
 *  주문응답/체결 도치 대응:
 *    - 주문응답보다 체결이 먼저 올 수 있음
 *    - A1491/A2491(1번째): 3초 sleep 후 재시도 (3회)
 *    - A1492/A2492(2번째): 60초 sleep 후 재시도 (재처리용)
 *************************************************************************/
void    Analyze_Data(void) {
    int     i, rt, che_cnt, retry_cnt, lp_gbn;
    int     mk_gbn, item_seq, acc_seq, imeco_gbn;
    double  od_price;
    char    tr_code[12];

    /* 체결전문에서 TR코드 추출 */
    memset(tr_code, 0, sizeof (tr_code));

    /* ******************************************************** */
    /* 주문응답/체결의 도치현상처리방안                          */
    /* -------------------------------------------------------- */
    /* 체결Data에 해당하는 주문번호가 없으면 3초 Sleep후 재처리  */
    /* 3회(3초*3회)후 ERROR LOG && SKIP                         */
    /* ******************************************************** */

    retry_cnt = 0;

    /*------------------------------------------------------------
     *  회원체결결과 처리
     *  - 채권(A1491/A1492): TTRTDP42301 (317바이트)
     *  - 파생(A2491/A2492): 'T'로 시작 (IMECO 체결)
     *------------------------------------------------------------*/
#if defined(A1491) || defined(A1492)
    /* 채권: KRX 헤더(4바이트) + 응답코드(11바이트) 뒤에 TR코드 */
    memcpy(tr_code, &R_Fmt[0].Data[11], 11);

    /* MK_GBN 런타임 분기(IMECO 폐기 후 KRX form): 현·파 체결(TTRTDP21301) 는
       파생 핸들러로 위임(채권 TTRTDP42301 경로는 무변경). */
    if (memcmp(tr_code, "TTRTDP21301", 11) == 0) {
        Analyze_Che_Deriv();
        return;
    }

    /* 채권은 체결전문이 동일하다. LP인지 일반인지 구분안됨. 계좌로 구분 */
    if (memcmp(tr_code, "TTRTDP42301", 11) == 0)       /* 채권 체결 (317B) */ {
        KRX_NOTE_SETTLE_DATA *dat =
        (KRX_NOTE_SETTLE_DATA *)R_Fmt[0].Data;

        imeco_gbn = 0;      /* 채권: MembershipItem offset 없음 */
#elif defined(A2491) || defined(A2492)
        /* 파생: 첫 바이트가 TR코드 */
        memcpy(tr_code, R_Fmt[0].Data, 1);

        if (memcmp(tr_code, "T", 1) == 0)          /* 파생 체결결과 */ {
            IMECO_SETTLE_DATA *dat =
            (IMECO_SETTLE_DATA *)R_Fmt[0].Data;

            imeco_gbn = -30;    /* IMECO: MembershipItem이 30바이트 짧음 */
#endif

            /*------------------------------------------------------------
             *  MembershipItem에서 주요 정보 추출
             *  +35(1): 시장구분 (01:채권일반, 02:채권LP, 11~13:금융파생)
             *  +37(5): A0 종목 SEQ 번호
             *  +42(2): 계좌 SEQ 번호
             *  +45(1): 스프레드 여부 (1:스프레드, 0:일반)
             *------------------------------------------------------------*/
            mk_gbn      = AtoIf(dat->MembershipItem+imeco_gbn+35, 1);  /* 시장구분 */
            item_seq    = AtoIf(dat->MembershipItem+imeco_gbn+37, 5);  /* 종목 Seq */
            acc_seq     = AtoIf(dat->MembershipItem+imeco_gbn+42, 2);  /* 계좌 seq */

            /* LP 여부 판단: 02=채권LP */
            if (memcmp(dat->MembershipItem+imeco_gbn+35, "02", 2) == 0)
                lp_gbn = 1;
            else
                lp_gbn = 0;

            /* ******************************************************** */
            /* MembershipItem 필드 설명 (30바이트부터 사용가능)          */
            /* 파생(IMECO)는 20바이트만 제공 → 스프레드 미사용           */
            /* -------------------------------------------------------- */
            /* +4~8  : 스프레드 근월물 종목SEQ                           */
            /* +16~20: 스프레드 원월물 종목SEQ                           */
            /* +30   : 매체구분 (A:서버자동, C:메리츠, T:윈웨이)         */
            /* +31~34: 전략에서 사용                                    */
            /* +35~36: 시장구분                                         */
            /* +37~41: A0 seq번호 (예: "00236")                         */
            /* +42~43: 계좌번호 seq (0~9:일반, 10~19:LP계좌)            */
            /* +44   : ETF/ETN/ELW 여부                                 */
            /* +45   : 스프레드종목 여부 (1:Y, 0:N)                     */
            /* +46   : 주식선물/옵션 종목구분                            */
            /* +47~50: ApType 4자리                                     */
            /* ******************************************************** */

            /*------------------------------------------------------------
             *  미체결(MiChe) SHM에서 체결된 주문번호 검색
             *  - 주문번호 10자리로 비교
             *  - 찾으면: 주문잔량 감소
             *  - 못찾으면: retry (도치 대응)
             *  F6: 캐시 선조회(검증 포함) - 히트 시 그 슬롯부터 루프
             *      진입해 기존 본문 그대로 실행, 미스 시 스캔 폴백+보정
             *------------------------------------------------------------*/
            while (1) {
                i = Miche_Idx_Get(&Midx[acc_seq], dat->OrderNo);
                if (i >= 0 && memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo,
                        dat->OrderNo, 10) != 0)
                    i = -1;     /* 낡은 캐시 - 폴백 */

                if (i < 0) {
                    for (i = 0; i < MAX_MICHE; i++) {
                        if (memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo,
                                dat->OrderNo, 10) == 0)
                            break;
                    }
                    if (i < MAX_MICHE)
                        Miche_Idx_Put(&Midx[acc_seq], dat->OrderNo, i);
                }

                if (i < MAX_MICHE)
                    break;      /* 찾음 - 아래 본문 처리 */

                /*------------------------------------------------------------
                 *  주문번호를 못찾은 경우: 도치 대응
                 *  (miche-deadcode-fix B-1: 루프 내부 데드코드였던 재시도 복원)
                 *  - 1번째(A1491/A2491): 3초 sleep × 3회 재시도
                 *  - 2번째(A1492/A2492): 60초 sleep × 3회 재시도
                 *  - 3회 초과: 재처리 FIFO에 Write 후 skip
                 *------------------------------------------------------------*/
                if (retry_cnt > 3) {
                    retry_cnt = 0;
#if defined(A1491) || defined(A1492)
                    Log(USR_ERROR, "체결주문번호 찾지못했음 재처리로 Write, 주문응답 미처리 JN[%10.10s]",
                            dat->OrderNo);

                    /* 재처리 FIFO에 Write */
                    memset(&W_Fmt, 0, sizeof(FILE_BUFF_FORMAT));
                    memcpy(&W_Fmt, &R_Fmt[0], sizeof(FILE_BUFF_FORMAT));
                    W_Fmt.LineFeed[0] = '\n';

                    /* 1491/2492 Write 처리 */
                    rt = F_W(TS_W2_1, (void *)&W_Fmt, 1);
                    if (rt != 1) {
                        Log(SAM_FATAL, "file write fail [%s]", OFN(D_K,P_K,1));
                    }
#endif
                    return;
                }

                retry_cnt ++;
                Log(USR_OK, "체결주문번호 찾지못했음 retry[%d]회 주문응답 미처리 JN[%10.10s]",
                        retry_cnt, dat->OrderNo);
#if defined(A1491) || defined(A2491)        /* 1번째: 3초 대기 */
                sleep(3);
#else                           /* 2번째: 60초 대기 (재처리용) */
                sleep(60);
#endif
            }   /* End of while (검색+도치 재시도) */

            for ( ; i < MAX_MICHE; i++) {
                /* 주문번호 매칭 → 주문잔량 감소 */
                if (memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo, dat->OrderNo, 10) == 0) {
                    Log(USR_OK, "OK Che Search [%10.10s] i[%d] acc[%d]", dat->OrderNo, i, acc_seq);

                    /* 주문잔량 = 기존잔량 - 체결수량 */
                    che_cnt = AtoIf(&dat->Trading_Volumn[1], sizeof (dat->Trading_Volumn)-1);
                    if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt - che_cnt < 0) {
                        Log(USR_ERROR, "체결 주문수량 부적합 주문번호 [%10.10s] 주문잔량[%d] 체결수량[%d]",
                                dat->OrderNo, Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt, che_cnt);
                        return;
                    }
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt =
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt - che_cnt;

                    /* 잔량이 0 이하면 미체결 건수 감소 */
                    if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt <= 0)
                        Shm_Mk_PreMatch[0].MeChe_Cnt[mk_gbn][acc_seq]--;

                    /* 주문가격 결정 (한도계산용) */
                    od_price = 0.0;
#if defined(A1491) || defined(A1492)
                    if (lp_gbn)     /* 채권LP: 매수/매도 가격이 다름 */ {
                        if (memcmp(dat->TradeFlag, "2", 1) == 0)       /* 채권매수 */
                            od_price = AtoDf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_Lp02, 9);
                        else                                            /* 채권매도 */
                        od_price = AtoDf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_Lp01, 9);
                    }
                    else            /* 채권일반 */ {
                        od_price = Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_dv;
                    }
#else
                    od_price = Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_dv;
#endif
                    break;
                }
            }   /* End of for (미체결 검색 - 도치 재시도는 위 while로 이동, B-1) */

            /*------------------------------------------------------------
             *  한도처리: Che_Rtn()으로 손익/잔고/미체결 갱신
             *  - 스프레드(+45='1'): 원월물 종목SEQ(+16) 사용
             *  - 일반: 종목SEQ(+37) 사용
             *------------------------------------------------------------*/
            rt = 0;
            if (memcmp(dat->MembershipItem+imeco_gbn+45, "1", 1) == 0) /* 스프레드 */ {
                rt = Che_Rtn(R_Fmt[0].Data, od_price,
                        AtoIf(dat->MembershipItem+imeco_gbn+35, 1),
                        AtoIf(dat->MembershipItem+imeco_gbn+16, 5),        /* 원월물 종목SEQ */
                        AtoIf(dat->MembershipItem+imeco_gbn+42, 2));
            }
            else                                                        /* 일반 */ {
                rt = Che_Rtn(R_Fmt[0].Data, od_price,
                        AtoIf(dat->MembershipItem+imeco_gbn+35, 1),
                        AtoIf(dat->MembershipItem+imeco_gbn+37, 5),        /* 종목SEQ */
                        AtoIf(dat->MembershipItem+imeco_gbn+42, 2));
            }

        }   /* End of 회원체결결과 */

        return;
    }   /* End of Analyze_Data ()   */

    /*************************************************************************
     *  Che_Rtn: 체결에 따른 잔고/손익/미체결 SHM 갱신
     *
     *  파라미터:
     *    p_buf    : 체결 데이터 원본
     *    od_price : 원주문가격 (미체결 내역에서 가져온 값)
     *    mk_gbn   : 시장구분 (0:채권, 1:파생)
     *    item_seq : 종목 SEQ
     *    acc_seq  : 계좌 SEQ
     *
     *  처리 내용:
     *    1) 잔고 변경: 방향 판단 후 수량 가감
     *       - 같은 방향: 잔고 합산, 평균단가 가중평균
     *       - 반대 방향: 잔고 차감, 방향전환 시 체결가로 평균단가 교체
     *    2) 미체결 감소: 체결수량/금액만큼 미체결에서 차감
     *    3) 스프레드: 근월물 + 원월물 각각 별도 처리
     *************************************************************************/
    int     Che_Rtn(char *p_buf, double od_price, int mk_gbn, int item_seq, int acc_seq) {
        int     i, rt, item_seqn, imeco_gbn;
        int     trad_flag, b_getcnt, n_getcnt;
        double  b_avg_d, che_price_d;
        /* 20220204: 스프레드 원월물 처리용 변수 */
        int     trad_flagn, b_getcntn;          /* 원월물 부호/잔고 */
        double  avg_tmp_d;                      /* 체결종목의 평균단가계산용 */
        double  avg_tmp_dn, b_avg_dn, che_price_dn; /* 스프레드 원월물 계산용 */
        /* 20220204 */
        char    item_code[13], r_time[20];

        /* 체결 데이터 구조체 캐스팅
           BUGFIX(2026-08-07): `&p_buf`(char**=파라미터 주소)를 struct로 읽던 버그 →
           `p_buf`(실제 버퍼)로 정정. 기존엔 손익/잔고(ProFit) 계산이 스택 garbage를
           읽어(프로덕션은 item_code 불일치로 early-return) 사실상 dead였음.
           ※ 이 수정으로 채권 체결 잔고/손익 accounting이 활성화되므로 운영 롤아웃 전 검토 권장. */
#if defined(A1491) || defined(A1492)
        KRX_NOTE_SETTLE_DATA    *dat = (KRX_NOTE_SETTLE_DATA *)p_buf;
        imeco_gbn = 0;          /* 채권: offset 없음 */
#elif defined(A2491) || defined(A2492)
        IMECO_SETTLE_DATA       *dat = (IMECO_SETTLE_DATA *)p_buf;
        imeco_gbn = -30;        /* IMECO: 30바이트 offset */
#endif

        memset(item_code, 0, sizeof (item_code));

        /* *************************** */
        /* 전시장 공통용 계산변수 추출  */
        /* *************************** */
        /* 체결수량 (부호(2바이트) 제외) */
        n_getcnt    = AtoIf(&dat->Trading_Volumn[2], sizeof (dat->Trading_Volumn) - 2);

        /* 20220204: 스프레드 여부에 따른 분기 */
        /* 체결단가: 채권/파생 모두 소수점2자리 */
        if (memcmp(dat->MembershipItem+imeco_gbn+45, "1", 1) != 0) /* 일반(비스프레드) */ {
            /* 체결부호: 1=매도(-), 2=매수(+) */
            if (memcmp(dat->TradeFlag, "1", 1) == 0)
                trad_flag  = -1;
            else
                trad_flag  =  1;

            /* 체결가격 (부호 2바이트 제외) */
            che_price_d = (double)AtoDf(&dat->Trading_price[2], sizeof (dat->Trading_price) - 2);
        }
#if defined(A2491) || defined(A2492)
        else    /* 스프레드 */ {
            /* 스프레드: 근월물/원월물 부호가 반대 */
            if (memcmp(dat->TradeFlag, "1", 1) == 0) {
                trad_flag   =  1;       /* 근월물 부호(매매부호와 반대) */
                trad_flagn  = -1;       /* 원월물 부호(매매부호와 동일) */
            }
            else {
                trad_flag   = -1;       /* 근월물 */
                trad_flagn  =  1;       /* 원월물 */
            }

            /* 근월물/원월물 체결가격 */
            che_price_d  = (double)AtoDf(&dat->Nearby_Trading_Price[2], sizeof (dat->Nearby_Trading_Price) - 2);
            che_price_dn = (double)AtoDf(&dat->Future_Trading_Price[2], sizeof (dat->Future_Trading_Price) - 2);
            /* 원월물 종목seq */
            item_seqn = AtoIf(dat->MembershipItem+imeco_gbn+16, 5);
            /* 원월물 기존잔고 */
            b_getcntn   = Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_getcnt;
            /* 원월물 기존평균단가 */
            b_avg_dn    = Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_get_avg_price;
        }
#endif

        /* 기존잔고 */
        b_getcnt    = Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt;
        /* 기존평균단가 */
        b_avg_d     = Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_get_avg_price;
        /* 20220204 */
        /* *************************** */

        /*------------------------------------------------------------
         *  종목코드 검증: SHM에서 해당 종목 코드 가져오기
         *------------------------------------------------------------*/
        if (mk_gbn == 0)            /* 0: 채권 */ {
            memcpy(item_code, Shm_Note[item_seq].A0.item_code, sizeof (Shm_Note[0].A0.item_code));
        }
        else if (mk_gbn == 1)       /* 1: 금융파생 */ {
            memcpy(item_code, Shm_FinFut[item_seq].A0.item_code, sizeof (Shm_FinFut[0].A0.item_code));
        }
        else {
            Log(USR_ERROR, "시장구분 오류 mk_gbn[%d]", mk_gbn);
            return (NOTOK);
        }

        /* 종목코드 매칭 검증 (스프레드는 종목코드 비교 불가) */
        if (memcmp(dat->MembershipItem+imeco_gbn+45, "1", 1) != 0) /* 비스프레드 */ {
            if (memcmp(dat->ItemCode, item_code, strlen(item_code)) != 0) {
                Log(USR_ERROR, "Data 종목 미매칭 수신종목[%d][%12.12s] A0[%12.12s] 시장구분[%d]",
                        item_seq, dat->ItemCode, item_code, mk_gbn);
                return (NOTOK);
            }
        }

        /* ******************************************************************** */
        /* 잔고 변경: 체결수량만큼 종목잔고 가감                                */
        /* -------------------------------------------------------------------- */
        /* 같은 방향(기존잔고와 체결이 같은 편): 잔고 합산, 가중평균 단가       */
        /* 반대 방향: 잔고 차감                                                 */
        /*   - 체결 > 기존: 방향전환, 평균단가=체결가                           */
        /*   - 체결 <= 기존: 잔고 감소, 평균단가 유지 (정산 시 0)               */
        /* ******************************************************************** */
        if (b_getcnt == 0 || (trad_flag * n_getcnt) * b_getcnt > 0) /* 같은 방향 */ {
            /* 잔고 = 기존 + 체결 */
            Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt =
            (trad_flag * n_getcnt) + b_getcnt;

            /* 평균단가 = 가중평균 */
            avg_tmp_d = ((b_avg_d * abs(b_getcnt)) + ((double)n_getcnt * che_price_d)) /
            (abs(b_getcnt) + n_getcnt);
        }
        else    /* 반대 방향 */ {
            if (n_getcnt > abs(b_getcnt))       /* 체결 > 기존 → 방향 전환 */ {
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt =
                trad_flag * (n_getcnt - abs(b_getcnt));

                /* 방향전환 시 평균단가 = 체결가 */
                avg_tmp_d = che_price_d;
            }
            else                                /* 체결 <= 기존 → 잔고 감소 */ {
                if (b_getcnt > 0)       /* 매수잔고 && 매도체결 */ {
                    Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt =
                    b_getcnt - n_getcnt;
                }
                else                    /* 매도잔고 && 매수체결 */ {
                    Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt =
                    b_getcnt + n_getcnt;
                }
                /* 정산 완료(잔고=0)이면 평균단가 초기화 */
                if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt == 0)
                    avg_tmp_d = 0;
                else
                    avg_tmp_d = b_avg_d;
            }
        }

        /* 20220204 */
        /*------------------------------------------------------------
         *  미체결 감소: 체결수량/금액만큼 미체결에서 차감
         *  - 스프레드는 미체결을 잡지 않으므로 금액 차감 안 함
         *  - 미체결 금액 = 체결수량 * 원주문가격 * 승수
         *------------------------------------------------------------*/
        if (trad_flag == 1)     /* 매수체결 */ {
            Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt -= n_getcnt;
            /* 금액: "원주문금액(미체결내역) * 체결수량 * 승수" 만큼 차감 */
            if (memcmp(dat->MembershipItem+imeco_gbn+45, "1", 1) != 0) /* 비스프레드 */
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum -= (n_getcnt * od_price) * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier;

            /* miche-deadcode-fix 결함A: '= 0' 대입 오타 수정('<= 0' 비교로),
               도달 불가였던 음수 이상 경고 복원 */
            if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt <= 0) {
                if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt < 0)
                    Log(USR_ERROR, "한도계산 이상함, 체크바람 매수미체결수량 음수나옴");
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum = 0;
            }
        }
        else                    /* 매도체결 */ {
            Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt -= n_getcnt;
            /* 금액 차감 */
            if (memcmp(dat->MembershipItem+imeco_gbn+45, "1", 1) != 0) /* 비스프레드 */
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum -= (n_getcnt * od_price) * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier;

            if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt <= 0) {
                if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt < 0)
                    Log(USR_ERROR, "한도계산 이상함, 체크바람 매도미체결수량 음수나옴");
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum = 0;
            }
        }

        /* 평균단가 SHM 반영 */
        Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_get_avg_price = avg_tmp_d;

        /*------------------------------------------------------------
         *  원월물 체결처리 (스프레드인 경우만)
         *  - 근월물과 동일한 로직을 원월물에 대해 수행
         *  - 미체결 금액 차감은 하지 않음 (스프레드는 미체결 미관리)
         *------------------------------------------------------------*/
        if (memcmp(dat->MembershipItem+imeco_gbn+45, "1", 1) == 0) /* 스프레드 */ {
            /* ************************************************************ */
            /* 스프레드시 원월물 처리를 추가로 해줘야한다. 20220204          */
            /* ************************************************************ */

            /* 원월물 잔고 계산 (근월물과 동일 로직) */
            if (b_getcntn == 0 || (trad_flagn * n_getcnt) * b_getcntn > 0)  /* 같은 방향 */ {
                Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_getcnt =
                (trad_flagn * n_getcnt) + b_getcntn;

                avg_tmp_dn = ((b_avg_dn * abs(b_getcntn)) + ((double)n_getcnt * che_price_dn)) /
                (abs(b_getcntn) + n_getcnt);
            }
            else    /* 반대 방향 */ {
                if (n_getcnt > abs(b_getcntn))      /* 방향 전환 */ {
                    Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_getcnt =
                    trad_flagn * (n_getcnt - abs(b_getcntn));
                    avg_tmp_dn = che_price_dn;
                }
                else                                /* 잔고 감소 */ {
                    if (b_getcntn > 0)      /* 매수잔고 && 매도체결 */ {
                        Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_getcnt =
                        b_getcntn - n_getcnt;
                    }
                    else                    /* 매도잔고 && 매수체결 */ {
                        Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_getcnt =
                        b_getcntn + n_getcnt;
                    }
                    if (Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_getcnt == 0)
                        avg_tmp_dn = 0;
                    else
                        avg_tmp_dn = b_avg_dn;
                }
            }

            /* 원월물 미체결 감소 (스프레드는 미체결을 잡지 않으므로 금액 차감 없음) */
            if (trad_flag == 1)     /* 매수 */ {
                Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_su_michecnt -= n_getcnt;

                /* miche-deadcode-fix 결함A: 근월물 분기와 동일 수정 */
                if (Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_su_michecnt <= 0) {
                    if (Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_su_michecnt < 0)
                        Log(USR_ERROR, "한도계산 이상함, 체크바람 매수미체결수량 음수나옴");
                    Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_su_miche_gum = 0;
                }
            }
            else                    /* 매도 */ {
                Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_do_michecnt -= n_getcnt;

                if (Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_do_michecnt <= 0) {
                    if (Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_do_michecnt < 0)
                        Log(USR_ERROR, "한도계산 이상함, 체크바람 매도미체결수량 음수나옴");
                    Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_do_miche_gum = 0;
                }
            }

            /* 원월물 평균단가 SHM 반영 */
            Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_get_avg_price = avg_tmp_dn;
        }

        return (OK);
    }   /* End of Che_Rtn */

    /*************************************************************************
     *  Analyze_Che_Deriv: 현·파 체결(TTRTDP21301) 처리 (MK_GBN 런타임 분기)
     *    채권 Analyze_Data 체결부를 TTRTDP21301_DATA 필드로 이식.
     *    미체결 OrderNo 검색 → 잔량 감소 → Che_Rtn_Deriv(손익/잔고).
     *    비스프레드 경로만(스프레드는 후속). 거부/자동취소 없음(체결전문).
     *************************************************************************/
    void    Analyze_Che_Deriv(void) {
        int     i, che_cnt, retry_cnt;
        int     mk_gbn, item_seq, acc_seq;
        double  od_price;
        TTRTDP21301_DATA    *dat = (TTRTDP21301_DATA *)R_Fmt[0].Data;

        /* 회원사용영역(60B, 내부규약): [35]시장 [37]종목seq [42]계좌seq */
        mk_gbn   = AtoIf(dat->Member_Use_Area+35, 1);
        item_seq = AtoIf(dat->Member_Use_Area+37, 5);
        acc_seq  = AtoIf(dat->Member_Use_Area+42, 2);

        retry_cnt = 0;

        /* 미체결 SHM에서 체결된 주문번호 검색(도치 대응: 3초×3회 재시도) */
        while (1) {
            i = Miche_Idx_Get(&Midx[acc_seq], dat->Order_Identification);
            if (i >= 0 && memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo,
                    dat->Order_Identification, 10) != 0)
                i = -1;     /* 낡은 캐시 - 폴백 */

            if (i < 0) {
                for (i = 0; i < MAX_MICHE; i++) {
                    if (memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo,
                            dat->Order_Identification, 10) == 0)
                        break;
                }
                if (i < MAX_MICHE)
                    Miche_Idx_Put(&Midx[acc_seq], dat->Order_Identification, i);
            }

            if (i < MAX_MICHE)
                break;      /* 찾음 */

            if (retry_cnt > 3) {
                Log(USR_ERROR, "체결주문번호 찾지못함(파생) skip JN[%10.10s]", dat->Order_Identification);
                return;
            }
            retry_cnt ++;
            Log(USR_OK, "체결주문번호 찾지못함(파생) retry[%d] JN[%10.10s]",
                    retry_cnt, dat->Order_Identification);
            sleep(3);
        }

        /* 주문잔량 감소 */
        for ( ; i < MAX_MICHE; i++) {
            if (memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo,
                    dat->Order_Identification, 10) == 0) {
                Log(USR_OK, "OK Che(파생) Search [%10.10s] i[%d] acc[%d]",
                        dat->Order_Identification, i, acc_seq);

                che_cnt = AtoIf(&dat->Trading_Volumn[1], sizeof (dat->Trading_Volumn)-1);
                if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt - che_cnt < 0) {
                    Log(USR_ERROR, "체결 주문수량 부적합(파생) JN[%10.10s] 잔량[%d] 체결[%d]",
                            dat->Order_Identification,
                            Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt, che_cnt);
                    return;
                }
                Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt -= che_cnt;

                if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt <= 0)
                    Shm_Mk_PreMatch[0].MeChe_Cnt[mk_gbn][acc_seq]--;

                od_price = Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_dv;
                break;
            }
        }

        /* 한도/손익: 비스프레드만(스프레드 후속) */
        if (memcmp(dat->Member_Use_Area+45, "1", 1) != 0)
            Che_Rtn_Deriv(R_Fmt[0].Data, od_price, mk_gbn, item_seq, acc_seq);

        return;
    }   /* End of Analyze_Che_Deriv */

    /*************************************************************************
     *  Che_Rtn_Deriv: 파생 체결 손익/잔고/미체결 갱신 (TTRTDP21301, 비스프레드)
     *    채권 Che_Rtn 비스프레드 경로를 TTRTDP21301_DATA 필드로 이식.
     *************************************************************************/
    int     Che_Rtn_Deriv(char *p_buf, double od_price, int mk_gbn, int item_seq, int acc_seq) {
        int     b_getcnt, n_getcnt, trad_flag;
        double  b_avg_d, che_price_d, avg_tmp_d;
        char    item_code[13];
        TTRTDP21301_DATA    *dat = (TTRTDP21301_DATA *)p_buf;  /* 채권 Che_Rtn의 &p_buf(버그) 답습 안 함 */

        memset(item_code, 0, sizeof (item_code));

        /* 체결수량(부호 2바이트 제외) */
        n_getcnt = AtoIf(&dat->Trading_Volumn[2], sizeof (dat->Trading_Volumn) - 2);

        /* 체결부호: 1=매도(-), 2=매수(+) */
        if (memcmp(dat->Ask_Bid_Type_Code, "1", 1) == 0)
            trad_flag = -1;
        else
            trad_flag = 1;

        /* 체결가격(부호 2바이트 제외) */
        che_price_d = (double)AtoDf(&dat->Trading_Price[2], sizeof (dat->Trading_Price) - 2);

        b_getcnt = Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt;
        b_avg_d  = Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_get_avg_price;

        /* 종목코드 검증(파생=Shm_FinFut) */
        if (mk_gbn == 1)
            memcpy(item_code, Shm_FinFut[item_seq].A0.item_code, sizeof (Shm_FinFut[0].A0.item_code));
        else {
            Log(USR_ERROR, "Che_Rtn_Deriv 시장구분 오류 mk_gbn[%d]", mk_gbn);
            return (NOTOK);
        }
        if (memcmp(dat->Issue_Code, item_code, strlen(item_code)) != 0) {
            Log(USR_ERROR, "Data 종목 미매칭(파생) 수신[%12.12s] A0[%12.12s]",
                    dat->Issue_Code, item_code);
            return (NOTOK);
        }

        /* 잔고 변경 (채권 로직 동일) */
        if (b_getcnt == 0 || (trad_flag * n_getcnt) * b_getcnt > 0) {    /* 같은 방향 */
            Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt = (trad_flag * n_getcnt) + b_getcnt;
            avg_tmp_d = ((b_avg_d * abs(b_getcnt)) + ((double)n_getcnt * che_price_d)) /
                        (abs(b_getcnt) + n_getcnt);
        }
        else {                                                          /* 반대 방향 */
            if (n_getcnt > abs(b_getcnt)) {                             /* 방향 전환 */
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt = trad_flag * (n_getcnt - abs(b_getcnt));
                avg_tmp_d = che_price_d;
            }
            else {                                                      /* 잔고 감소 */
                if (b_getcnt > 0)
                    Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt = b_getcnt - n_getcnt;
                else
                    Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt = b_getcnt + n_getcnt;
                if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt == 0)
                    avg_tmp_d = 0;
                else
                    avg_tmp_d = b_avg_d;
            }
        }

        /* 미체결 감소(체결수량*원주문가격*승수) */
        if (trad_flag == 1) {                                           /* 매수체결 */
            Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt -= n_getcnt;
            Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum -=
                (n_getcnt * od_price) * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier;
            if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt <= 0) {
                if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt < 0)
                    Log(USR_ERROR, "한도계산 이상(파생) 매수미체결수량 음수");
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum = 0;
            }
        }
        else {                                                          /* 매도체결 */
            Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt -= n_getcnt;
            Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum -=
                (n_getcnt * od_price) * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier;
            if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt <= 0) {
                if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt < 0)
                    Log(USR_ERROR, "한도계산 이상(파생) 매도미체결수량 음수");
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum = 0;
            }
        }

        Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_get_avg_price = avg_tmp_d;
        return (OK);
    }   /* End of Che_Rtn_Deriv */

    /**************************************************************************
     *  Write_Data: 종료시 후처리 Write
     *  - 현재 미사용 (pa_9001_mp에서 Auto 종료처리)
     **************************************************************************/
    void    Write_Data(int acc_seq) {
    }

    /*************************************************************************
     *  End of Program (po_1490_mp.c)
     *************************************************************************/
