#ifndef __ARB_H__
#define __ARB_H__

// -------------------------------------------------------------
// 공통 인클루드
// -------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>   // bool 타입 사용
#include "sem.h"
#define DATA_SIZE 2048
#include "buf_struct.h"

// -------------------------------------------------------------
// 매크로: 방향/정책/유틸
// -------------------------------------------------------------
#define ARB_CFG_PATH "/fsfxwin/fep/yarb/conf/config.ini"

#define DIR_NONE     0     // 방향 없음
#define DIR_BUY      1     // 매수차익(현물매수/선물매도)
#define DIR_SELL    -1     // 매도차익(현물매도/선물매수) 

#define EX_SIMUL       0   // 동시 전송(논리적 동시)
#define EX_FUT_FIRST   1   // 선물 우선
#define EX_SPOT_FIRST  2   // 현물 우선

#define OT_FAK         0    // Fill And Kill (=IOC Immediate-Or-Cancle) 
#define OT_FOK         1    // Fill Or Kill
#define OT_FAS         2    // Fill And Store 

#define SO_LEG_SPOT   0     // 현물 레그
#define SO_LEG_FUT    1     // 선물 레그

#define ARB_BOTH      0    // 양방향 허용
#define ARB_BUY       1    // 매수차익만 
#define ARB_SELL      2    // 매도차익만 

#define ORD_KIND_ENTRY 0    // 진입주문
#define ORD_KIND_CLOSE 1    // 청산주문

#define SIDE_BUY       +1     // 매수
#define SIDE_SELL      -1     // 매도

#define ORD_TYPE_M        0     //시장가주문
#define ORD_TYPE_L        1     //지정가주문 

#define MAX_STRATS    200
#define FUT_SYM_LEN   12 
#define SPOT_SYM_LEN  6 
#define NAME_LEN      64

// Message queue type 
#define MSG_TYPE_SPOT_ORD    1
#define MSG_TYPE_SPOT_ORDLOG 2
#define MSG_TYPE_FUT_ORDLOG  3

// 세트 상태 
#define SET_VALID            1 //유효(세트 전체 체결X)    
#define SET_NOT_VALID        0 //완료(세트 전체 체결O)

// 거래단계
#define TRADE_PHASE_ENTRY    0
#define TRADE_PHASE_EXIT_1   1
#define TRADE_PHASE_EXIT_2   2
#define TRADE_PHASE_HARD_CUT 3

// 전략중단코드
#define ARB_STOP_CD          99

// -------------------------------------------------------------
// 구조체: 시세 스냅샷
// -------------------------------------------------------------
typedef struct {
    char   symb_fut [12];
    char   symb_spot[7];

	double px_spot_bid;         // 현물 매수호가
	double px_spot_ask;         // 현물 매도호가
	
	double px_fut_bid;          // 선물 매수호가(1호가)
	double px_fut_ask;          // 선물 매도호가(1호가)
	
	int    qty_fut_bid;         // 선물 매수잔량(1호가)
	int    qty_fut_ask;         // 선물 매도잔량(1호가)
	
	int    ts_mono_ms;          // (옵션) 모노토닉 타임(ms)
	char   quote_id_bid[30+1];   // quote_id_bid -> 신한 사용 
	char   quote_id_ask[30+1];   // quote_id_ask -> 신한 사용 
} MarketSnap;


// -------------------------------------------------------------
// 구조체: 전략 설정(데몬 기동 시 Shared에 적재)
//   - 수량 단위: 현물 M, 선물 계약(CT)
// -------------------------------------------------------------
typedef struct {
    int    today;
	char   aptype[10];
	int    seq_no;                       // 전략인덱스
	int    strategy_id;
	char   strategy_name[NAME_LEN+1];
	int    strategy_stat;                // 전략상태 : 1.구동중 2.일시정지 3.정지

	char   spot_sym[SPOT_SYM_LEN+1];
	char   fut_sym[FUT_SYM_LEN+1];

	int    base_lot_spot;               // 기본 현물 M 
	int    base_lot_fut;                // 기본 선물 CT
	int    max_lot_abs_fut;             // 한쪽 방향 누적 최대 절대 선물 수량  
	int    max_lot_abs_spot;            // 한쪽 방향 누적 최대 절대 현물 수량
	int    min_lot_fut;                 // 최소선물잔량
	double tick_unit_fut;               // 선물틱단위(double) 
	double tick_offset;                 // 선물틱오프셋 - 호가별 가중치

	double entry_buy_spread_bp;         // 진입 기준 매수 스프레드 (0~)
	double entry_sell_spread_bp;        // 진입 기준 매도 스프레드 (0~)
	double exit_spread_bp;              // 진입 후 병렬 청산 스프레드(bp - 부호있음)

	int    entry_start_time;            // 진입 시작 HHMMSS
	int    close_t1;                    // 1차 청산 시작 HHMMSS
	int    close_t2;                    // 2차 청산 시작 HHMMSS
	int    hard_cut_time;               // 하드컷 HHMMSS

	int    close_spread_enabled_t1;     // 1차 스프레드 임계 사용여부
	double close_spread_bp_t1;          // 1차 임계 bp(부호있음)
	int    close_spread_enabled_t2;     // 2차 스프레드 임계 사용여부
	double close_spread_bp_t2;          // 2차 임계 bp(부호있음))
	int    hard_cut_enabled;            // 하드컷(최종청산) 사용 여부
	double hard_cut_bp;                 // (옵션) 하드컷 시 bp --  하드컷시 시장가에 주문하므로 필요x

	int    exec_mode;                   // EX_SIMUL / EX_FUT_FIRST / EX_SPOT_FIRST
	int    tif_type_fut;                // OT_FAK / OT_FOK / OT_FAS
	int    tif_type_spot;               // OT_IOC / OT_FOK
	int    fill_pref_entry;             // PREF_FILL_GUARANTEE / PREF_FILL_BETTERPX
	int    ord_type_fut;                // 0 시장가 1 지정가(FOK) 2 지정가(FAS-1호가) 3 지정가(FAS-2호가) 4 지정가(FAS-3호가)
	int    ord_type_spot;			    // 0 시장가 1 지정가
	
	int    auto_mode;                  // 1=자동, 0=수동
	double auto_mode_deadband;         // |sprd| < deadband → 진입 금지
	int    arb_dir_mode;               // ARB_BOTH / ARB_LONG_ONLY / ARB_SHORT_ONLY

	int    order_interval_ms;          // 주문 간격(ms) : 한 세트(현선) 종료 후 대기

	int    pos_spot;                   // 전략별 현물 잔고(-:매도 +:매수)
    int    pos_fut;                    // 전략별 선물 잔고(-:매도 +:매수)

    int    basis_cd;                   // 베이시스계산구분코드 - 1.Swap, 2.이론스프레드(딜러수기입력)
	double basis_spread;               // 베이시스계산스프레드

	char   fcm_id_fut[3+1];            // 선물FCMID 
	char   fcm_acno_fut[20+1];         // 선물계좌번호 
	char   exch_spot[1+1];             // 현물거래소코드 
	char   fcm_acno_spot[20+1];        // 현물계좌번호
	char   ref_no[15+1];               // 참조번호(1 구분자 + 8 주문일자 + 6 일런번호)
    char   bsns_dt_fut[8+1];           // 선물영업일자(야간용)
    char   bsns_dt_spot[8+1];          // 현물영업일자
    char   bk_no[11+1];                // 북번호
    char   lgen_no[10+1];              // 거래참여자번호
    char   trdr_no[4+1];               // 트레이더번호
	int    retry_cnt;                  // 현물주문재시도횟수 
	int    retry_interval_ms;          // 현물주문재시도간격

    int    fut_idx;                    // 종목인덱스(시퀀스)
    int    spot_idx;                   // 종목인덱스(시퀀스)

	char   scid[50+1];                 // SendCompID
    char   tgid[50+1];                 // TargetCompID
    
	char   day_ngt_cd[1+1];            // 주야간구분코드 
	char   expire_dt[8+1];             // NDF만기일자

	time_t  end_tm_t;                  // 전략자동종료시간
	time_t  ent_tm_t;                  // 진입시간 
	time_t  close_t1_t;                // 1차청산시간
	time_t  close_t2_t;                // 2차청산시간
	time_t  hard_cut_time_t;           // 최종청산시간

} StrategyMem;

// -------------------------------------------------------------
// 주문 로그(세트 단위에서 개별 주문 기록)
// -------------------------------------------------------------
#define MAX_SET_ORD_LOG  100     

// 주문 로그 ( 체결시 체결수량 누적 )
typedef struct {
	int    leg;                  // 0:SO_LEG_SPOT / 1:SO_LEG_FUT
	long   ord_id;               // 주문ID(브로커 OID/CLID 또는 내부생성)
	int    ordtype;              // 0:지정가 / 1:시장가 
	int    tiftype;              // 0:FAS /1:FOK/ 2:FAS
	int    side;                 // +1 매수 / -1 매도
	int    qty;                  // 수량(M 또는 CT)
	double px;                   // 전송 가격
	int    ts_hhmmss;            // 전송 시각(HHMMSS)
	int    filled_qty;           // 누적 체결 수량
	double filled_px;            // 평균 체결가
	int    valid;                // 1: 유효 0: 완료
} SetOrdLogEnt;

// 세트 주문 로그
typedef struct {
	SetOrdLogEnt ent[MAX_SET_ORD_LOG];
	int    n;              // 유효 엔트리 수
} SetOrdLog;
// -------------------------------------------------------------
// 구조체: 세트 상태(런타임)
// -------------------------------------------------------------
typedef struct {
    int    valid ;        // 세트 유효 여부 1:유효  0: 완료
	int    set_id;        // 세트 LOID
    int    step  ;        // 진입 ->체결 ->청산 ->완료
    // ==  포지션 관리  ==
	int    pos_spot;            // 현물 포지션(M, +매수/-매도)
	int    pos_fut;             // 선물 포지션(CT, +매수/-매도)
      
	int    real_retry_cnt;      // 실제 시도한 현물주문 재시도 횟수 

	int    fut_reject_stat;     // -2: Auto Cancel -1: 거부응답 0:미응답 1:체결응답
	int    fut_fill_stat;       // 0:미체결, 1:체결 
	int    spot_reject_stat;    // -1: 거부응답 0:미응답 1:체결응답
	int    spot_fill_stat;      // 0:미체결, 1:체결 

	// == 주문 이력  === 
	SetOrdLog        ordlog;

} SetState;

// 주문 번호
typedef struct
{
    int     set_id;                 // set 번호 
    long    fut_ordno_base;         // 선물주문번호 베이스
    long    fut_ordno;              // 선물주문번호
	long    fut_ordno_end;          // 선물주문번호 전략당 마지막 대역
	long    fut_ordno_last;         // 선물주문번호 최종 대역
    long    spot_ordno_base;        // 현물주문번호 베이스 (30억부터 ~)
    long    spot_ordno;             // 현물주문번호
	long    spot_ordno_end;         // 현물주문번호 전략당 마지막 대역
	long    spot_ordno_last;        // 현물주문번호 최종 대역
} OrdNo;

// New 주문번호 -- 확장성을 위해 현물 / 선물 Shm 분리하여 관리
typedef struct
{
	SEM     *sem;               /* semaphore struct */
	int     ord_dt;             // 주문일자
    long    fut_ordno;          // 선물주문번호
} FutOrdNo;

typedef struct
{
	SEM     *sem;               /* semaphore struct */
	int     ord_dt;             // 주문일자
    long    spot_ordno;         // 현물주문번호
} SpotOrdNo;

// ----------------------------------------------------------------
// 전략 슬롯 (전략 + 세트들)
// 차익거래를 당일 max 몇번까지 허용할 것인가? 일단 2000번만 허용하자. 
// ----------------------------------------------------------------
#define MAX_SETS_PER_STRAT  2000
typedef struct {
    StrategyMem strategy;
    OrdNo       ordno;    
    SetState    sets[MAX_SETS_PER_STRAT];
    int         n; // 세트개수? 
} StrategySlot;

// -------------------------------------------------------------
// 구조체: 공유 루트
// -------------------------------------------------------------
typedef struct {
	StrategySlot slots[MAX_STRATS];
	MarketSnap   snaps[MAX_STRATS]; 
} SharedRoot;

// ----------------------------------------------------------------
// 전역 인덱스 (분리된 전략/세트 인덱스)
// ----------------------------------------------------------------
extern SharedRoot *g_shared;
extern int g_strat_idx;
extern int g_set_idx;
extern int g_fill_fd;

extern FutOrdNo   *g_shm_oid_fut;
extern SpotOrdNo  *g_shm_oid_spot;
//extern  SHM_FIN_FUT *g_md_shm_fut;
//extern  SHM_FX      *g_md_shm_spot;

#define G_STRAT (&g_shared->slots[g_strat_idx].strategy)
#define G_ORDNO (&g_shared->slots[g_strat_idx].ordno)
#define G_SET   (&g_shared->slots[g_strat_idx].sets[g_set_idx])
#define G_SNAP  (&g_shared->snaps[g_strat_idx])      
#define G_SET_CNT    g_shared->slots[g_strat_idx].n
#define G_OID_FUT  (g_shm_oid_fut)
#define G_OID_SPOT (g_shm_oid_spot)

#define POS_SPOT    (G_SET->pos_spot)
#define POS_FUT     (G_SET->pos_fut)
#define ORDNO_FUT   (G_ORDNO->fut_ordno)
#define ORDNO_SPOT  (G_ORDNO->spot_ordno)

extern int g_test_mode;  // 테스트 모드 여부

// -------------------------------------------------------------
// 유틸: 현재 HHMMSS
// -------------------------------------------------------------
static inline int get_hhmmss_now(void)
{
	time_t t = time(NULL);
	struct tm *tm_info = localtime(&t);
	return tm_info->tm_hour*10000 + tm_info->tm_min*100 + tm_info->tm_sec;
}

static inline int get_today(void)
{
    time_t t = time(NULL);

    struct tm *tm_info=localtime(&t);

    int yyyymmdd = (tm_info->tm_year+1900) * 10000  +
                   (tm_info->tm_mon+1) * 100 + tm_info->tm_mday;
    return yyyymmdd;
}

static inline void get_todate(char *buf, size_t size)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    snprintf(buf, size, "%04d%02d%02d",
            tm_info->tm_year + 1900,
            tm_info->tm_mon  + 1,
            tm_info->tm_mday);
}

// 인라인 함수: 문자열의 일부를 double로 변환
static inline double str2double(const char *src,  int len) 
{
	char buf[64];
	strncpy(buf, src + 0, len);
	buf[len] = '\0';
	return strtod(buf, NULL);
}
// 인라인 함수: 문자열의 일부를 int로 변환 (소수점 아래는 무시)
static inline int str2int(const char *src, int len) {
	char buf[64];
	strncpy(buf, src , len);
	buf[len] = '\0';
	return atoi(buf);
}

// -------------------------------------------------------------
// 체결 이벤트 구조체
// ------------------------------------------------------------- 
typedef struct {
	char user_area[20];  // 회원처리 항목
    long ord_id;         // 주문 ID 
	int  side;           // 매매구분 : + 1 매수, - 매도
    double fill_px;      // 체결 가격
    int fill_qty;        // 체결 수량
} FillEvent;

// -------------------------------------------------------------
// 전략설정
// -------------------------------------------------------------
typedef struct
{
	char trcode[6];
	char aptype[5];
	char reserved[39];
} SEARCH_HD;

typedef struct
{
    char    seq_no                        [   6];   // 전략실행일련번호
    char    sf_mrgn_stus_dstcd            [   1];   // 현선물차익거래상태구분코드
	char    proc_id                       [   5];   // 프로세스ID
    char    sf_mrgn_rule_id               [   5];   // 현선물차익거래룰ID
    char    fx_exch_dstcd                 [  10];   // FX거래소구분코드- S:SMB
    char    fx_fcm_acno                   [  20];   // FXFCM계좌번호
    char    fx_prdct_cd                   [   6];   // FX상품코드
    char    fx_ofpr_dstcd                 [   1];   // FX호가구분코드 - 2.지정가(만 사용)
    char    fx_base_ordr_unit_qty         [  10];   // FX기본주문단위수량
    char    fx_max_ordr_unit_qty          [  10];   // FX최대주문단위수량
    char    ftrs_fcm_id                   [   3];   // 선물FCMID
    char    ftrs_fcm_acno                 [  20];   // 선물FCM계좌번호
    char    ftrs_items_cd                 [  12];   // 선물종목코드
    char    ftrs_ofpr_dstcd               [   1];   // 호가구분코드 - T.시장가, 2.지정가
    char    ftrs_base_ordr_unit_qty       [  10];   // 선물기본주문단위수량
    char    ftrs_max_ordr_unit_qty        [  10];   // 선물최대주문단위수량
    char    ftrs_min_rmandr               [  10];   // 선물최소수량
    char    ftrs_tick_unit                [  10];   // 선물틱단위
    char    mrgn_drct_dstcd               [   1];   // 차익방향구분코드 - 1.매수, 2.매도, 3.양방향
    char    ordr_prity_dstcd              [   1];   // 주문우선순위구분코드 - 1.동시, 2.선물
    char    basis_calc_dstcd              [   1];   // 베이시스계산구분코드 - 1.Swap, 2.이론스프레드(딜러수기입력)
    char    basis_calc_sprd               [  10];   // 베이시스계산스프레드
    char    ordr_intval_ms                [  10];   // 주문간격밀리초값
    char    ent_buy_sprd_dstcd            [   1];   // 진입매수스프레드구분코드
    char    ent_sell_sprd_dstcd           [   1];   // 진입매도스프레드구분코드
    char    lqdt_start_sprd_dstcd         [   1];   // 청산개시스프레드구분코드(1차청산, 청산은 옵션) - 1.절대 , 9.미사용
    char    lqdt_cmpl_sprd_dstcd          [   1];   // 강제청산스프레드구분코드(2차청산, 청산은 옵션) - 1.절대 , 9.미사용
    char    ent_buy_sprd                  [  15];   // 진입매수스프레드값
    char    ent_sell_sprd                 [  15];   // 진입매도스프레드값
    char    lqdt_sprd                     [  15];   // 청산스프레드값
    char    lqdt_start_sprd               [  15];   // 청산개시스프레드값(1차청산)
    char    lqdt_cmpl_sprd                [  15];   // 강제청산스프레드값(2차청산)
    char    ent_sched_hms                 [   6];   // 진입예정시각
    char    lqdt_start_sched_hms          [   6];   // 청산개시예정시각(1차청산)
    char    lqdt_cmpl_sched_hms           [   6];   // 강제청산예정시각(2차청산)
    char    last_lqdt_end_sched_hms       [   6];   // 최종종료예정시간(최종청산)
    char    ref_no                        [  15];   // 참조번호(1 구분자 + 8 주문일자 + 6 일런번호)
    char    ftrs_bsns_dt                  [   8];   // 선물영업일자(야간용)
    char    spot_bsns_dt                  [   8];   // 현물영업일자
	char    bk_no                         [  11];   // 북번호
    char    lgen_no                       [  10];   // 거래참여자번호
    char    trdr_no                       [   4];   // 트레이더번호
    char    retry_cnt                     [   5];   // 현물주문재시도횟수
    char    retry_intval_ms               [  10];   // 현물주문재시도간격
	char    scid                          [  50];   // SendCompID
    char    tgid                          [  50];   // TargetCompID
    char    day_ngt_dstcd                 [   1];   // 주야간구분코드 
    char    expire_dt                     [   8];   // NDF 만기일자
} STRATEGY_BODY;

// -------------------------------------------------------------
// 세트 주문 백오피스로 실시간 전송 
// ------------------------------------------------------------- 

int Arb_Init   (char *aptype, char *msg);
int Arb_Process(char *msg);
int Arb_Exec   (int gb , char *data);    // gb = 1: 현물 gb== 2 :선물
int Arb_Spot_Exec  (int gb, int side,  long ord_id, long fill_qty, double fill_px);
int Arb_Fut_Exec   (int gb, int side,  long ord_id, long fill_qty, double fill_px);
int Arb_Feed       (void);
int Arb_Read_Msg(FILE_BUFF_FORMAT *data);

int handle_fut_fill_event (FillEvent *fill);
int handle_spot_fill_event(FillEvent *fill);

#endif // __COMMON_H__
