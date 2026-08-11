#ifndef	_MDS_H_
#define	_MDS_H_	

#include <pthread.h>
#include <ctype.h>
#include <time.h>

#define	SQL_CNT		sqlca.sqlerrd[2]

int l_dbconnect(void);
int l_dbdisconnect(void);
int l_dbcommit_ex(char *conn);
int l_dbrollback_ex(char *conn);

#ifdef __cplusplus
extern "C" {
#endif

#define		SYMB_LEN			8

#define 	TENOR_STP			0
#define 	TENOR_TOD			1
#define 	TENOR_TOM			2
#define 	TENOR_W01			3
#define 	TENOR_M01			4
#define 	TENOR_M02			5
#define 	TENOR_M03			6
#define 	TENOR_M06			7
#define 	TENOR_Y01			8
#define 	TENOR_MAX			9

typedef struct {
	char	exnm			[ 8];
	char	symb			[ 8];	
	int		kymd				;
	int		khms				;
	int		tymd                ;
	int		seqn				;
	char	tenor   		[ 8];
	struct {
		double open				;
		double open_tm          ;
		double high				;
		double high_tm          ;
		double lowp				;
		double lowp_tm          ;
		double clos				;
		double clos_tm          ;
		double base				;
		double last_valid		;
	} bid, ask, mid;
	char	tickkey		    [32];	// exnm(4) + symb(6) + 날짜(8) + 시간(9)
	int 	tenor_idx	        ;
} MDCANDLE;

typedef struct {
	int 	quot_flag			; 	// 호가 여부
	char	excode			[ 1];	// 원천 : 'S'=SMB, 'K'=KMB, 'E'=EBS, 'C'=CMB 'R':REUTER
	char	symb			[12];	// 통화코드
	char	tenor           [ 4];	// 테너ID
	char	date			[ 8];	// 수신일자 YYYYMMDD (서버시간)
	char	time			[ 9];	// 수신시간 HHMMSSSSS
	char	reqid			[20];	// MDReqID : Unique identifier of the client’s MarketDataRequest
	struct {
		double	prc				;	// Price of the MarketData Entry
		double	vol				;	// Quantity of the MarketData Entry. Always “0” For USD/KRW, CNH/KRW
		double	bestprc			;	// NotUse : Only USD/KRW, CNH/KRW. Not tradeable price, reference only
		double	bestvol			;	// NotUse : Only USD/KRW, CNH/KRW. Not tradeable price, reference only
		char	date		[ 8];	// NotUse : Specific date of trade settlement in YYYYMMDD format.
		char	quoteid		[12];	// NotUse : Unique identifier of this MarketData Snapshot
		char	excode      [ 1];
	} bid, ask;
	char	md_symbol       [16];

	int 	 fill_flag		;		// 체결여부
	char     fillid[32]     ;		// 체결ID
	int		 fillseq        ;		// 체결일련번호	
	double   fillprc		;		// 체결가격
	double	 fillqty		; 		// 체결수량
} MDSSISE;

// ascii 전문이 일반적이나, 시스템 내부시세이므로 binary 전문사용 (필요시 ascii로 변경해도 무방)
typedef struct {
	char 	type        [ 2];	// 'FA' SWAP rate 구분 위해
	char	excode		[ 1];	// 'S'MB/'K'MB/E'BS/'C'MB/'B'EST/'Z'CUST
	char	bidex		[ 1];	// BID원천 : 'S':SMB, 'K':KMB, 'E':EBS, 'C':CMB
	char	askex		[ 1];	// ASK원천 : 'S':SMB, 'K':KMB, 'E':EBS, 'C':CMB
	char	symb		[ 7];	// root symbol
	char	tickkey		[32];	// exnm(4) + symb(6) + 날짜(8) + 시간(9)
	char	date		[ 8];	// 수신일자 YYYYMMDD (서버시간)
	char	time		[ 9];	// 수신시간 HHMMSSSSS
	double	usdbid			;	// Current USDKRW BID
	double	usdask			;	// Current USDKRW OFFER
	double	bidprc			;	// Price of the MarketData Entry
	double	askprc			;	// Price of the MarketData Entry
	double	bidqty			;	// Quantity of the MarketData Entry. Always “0” For USD/KRW, CNH/KRW
	double	askqty			;	// Quantity of the MarketData Entry. Always “0” For USD/KRW, CNH/KRW
	double  midprc			;	// 중간가
	double  fillprc			;   // 체결가
} APSISE;

typedef struct {
	char	type		[ 2];	// must be 'FA'
	char	rdcode		[20];	// realtime symbol : 원천(1:S/K/E/C/'B'est/'Z'cust) + symb(6)
	char	excode		[ 1];	// 'S'MB/'K'MB/E'BS/'C'MB/'B'EST/'Z'CUST
	char	bidex		[ 1];	// BID원천 : 'S':SMB, 'K':KMB, 'E':EBS, 'C':CMB
	char	askex		[ 1];	// ASK원천 : 'S':SMB, 'K':KMB, 'E':EBS, 'C':CMB
	char	symb		[10];	// root symbol
	char	kymd		[ 8];	// korean trading time
	char	khms		[ 6];	// korean trading time

	char	bid_copen	[ 1];	// 시가 색상 ('+', '-', ' ')
	char	bid_open	[12];	// open price
	char	bid_chigh	[ 1];	// 고가 색상 ('+', '-', ' ')
	char	bid_high	[12];	// high price
	char	bid_clow	[ 1];	// 저가 색상 ('+', '-', ' ')
	char	bid_low		[12];	// low price
	char	bid_clast	[ 1];	// 현재가 색상 ('+', '-', ' ')
	char	bid_last	[12];	// last price
	char	bid_sign	[ 1];	// change sign
	char	bid_cdiff	[ 1];	// 대비 색상 ('+', '-', ' ')
	char	bid_diff	[12];	// net change
	char	bid_crate	[ 1];	// 등락율 색상 ('+', '-', ' ')
	char	bid_rate	[ 6];	// change rate

	char	ask_copen	[ 1];	// 시가 색상 ('+', '-', ' ')
	char	ask_open	[12];	// open price
	char	ask_chigh	[ 1];	// 고가 색상 ('+', '-', ' ')
	char	ask_high	[12];	// high price
	char	ask_clow	[ 1];	// 저가 색상 ('+', '-', ' ')
	char	ask_low		[12];	// low price
	char	ask_clast	[ 1];	// 현재가 색상 ('+', '-', ' ')
	char	ask_last	[12];	// last price
	char	ask_sign	[ 1];	// change sign
	char	ask_cdiff	[ 1];	// 대비 색상 ('+', '-', ' ')
	char	ask_diff	[12];	// net change
	char	ask_crate	[ 1];	// 등락율 색상 ('+', '-', ' ')
	char	ask_rate	[ 6];	// change rate

	char	mid_copen	[ 1];	// 시가 색상 ('+', '-', ' ')
	char	mid_open	[12];	// open price
	char	mid_chigh	[ 1];	// 고가 색상 ('+', '-', ' ')
	char	mid_high	[12];	// high price
	char	mid_clow	[ 1];	// 저가 색상 ('+', '-', ' ')
	char	mid_low		[12];	// low price
	char	mid_clast	[ 1];	// 현재가 색상 ('+', '-', ' ')
	char	mid_last	[12];	// last price
	char	mid_sign	[ 1];	// change sign
	char	mid_cdiff	[ 1];	// 대비 색상 ('+', '-', ' ')
	char	mid_diff	[12];	// net change
	char	mid_crate	[ 1];	// 등락율 색상 ('+', '-', ' ')
	char	mid_rate	[ 6];	// change rate

	char	cpask		[ 1];	// 매도호가 색상('+', '-', ' ')
	char	pask		[12];	// ask
	char	cpbid		[ 1];	// 매수호가 색상('+', '-', ' ')
	char	pbid		[12];	// bid
	char	spread		[ 6];	// Spread
	char	bestcpask	[ 1];	// best매도호가 색상('+', '-', ' ')
	char	bestpask	[12];	// best ask
	char	bestcpbid	[ 1];	// best매수호가 색상('+', '-', ' ')
	char	bestpbid	[12];	// best ask
	char	bestspread	[ 6];	// bestspread
	char	vask		[12];	// ask  size
	char	vbid		[12];	// bid size
	char	bestvask	[12];	// Best ask size
	char	bestvbid	[12];	// Best size
	char	tickkey		[32];	// exnm(4) + symb(6) + 날짜(8) + 시간(9)

	char	fill_price  [12];	// 체결가
	char 	usdkrw_bid  [12];	// USD/KRW 매수
	char	usdkrw_ask  [12];	// USD/KRW 매도
	char 	usdkrw_bid_best[12];	// USD/KRW 매수 베스트
	char	usdkrw_ask_best[12];	// USD/KRW 매도 베스트
	char 	client_req1 [ 8];	// 더미1(클라이언트 요구)
	char 	client_req2 [ 8];	// 더미2(클라이언트 요구)
} CUSTSISE;
#define		SZ_CUSTSISE		sizeof(CUSTSISE)


//#####################################
typedef	struct {
	int		exid			;	// exchange id
	char	exnm		[ 8];	// short name
	char	excode		[ 1];	// 'S'MBS/'K'MBS/E'MBS/'C'MBS/'B'est/'Z'CUST
	char	TZ			[60];	// time zone
	int		maxcnt			;	// maximum no of symbol for shared memory
	int		arbitrage		;	// 재정환율계산여부
	int		sendclient_flag	;	// 클라이어언트 시세 전용 여부
	int		db_pnum			;	// number of db update threads
	struct	{
		char	name	[16];	// port id (=product name)
		char 	ipad	[20];	// ip address
		int  	port		;	// port number
	} recv;
	struct	{
		int		cust		;	// CUST 전송여부
		int		cast		;	// AP 전송여부
		int		best		; 	// BEST 전송여부
		char	neta	[20];	// local address
		char	ipad	[20];	// multicasting IP-address
		int		port		;
	} apsnd;
	char	quenm	   [128];	// 시세 수신 큐이름 (exchange.cfg)
	char	dirp	   [128];	// data file path
	int		llog			;	// log level
	char	logf	   [128];	// log path
} XCHG;

//#####################################
//##### mds/src/inc/mds/context.h
typedef struct {
	char userid     [16];		// 관리자ID
	char usdkrw_exnm[8];		// USD/KRW 브로커
	int all_flag;  				// 전체 O:OFF 1:ON
	int run_flag;				// 개별 0:OFF 1:ON
	double value;				// USD/KRW:Available bid/offer Spread, Other: 시초가대비 등락율
} kswitch_t;

typedef	struct {
	XCHG	xchg			;	// exchange information
	int		sizefold        ;
	time_t	rtim			;	// receive time stamp
	int		rsum			;	// daily total
	int		tymd			;	// 영업일
	int		mrec			;	// max record
	int		nrec			;	// current record numbers of whereis
} MDARCH;						// shared memory pointer

//#####################################
typedef struct mdfold MDFOLD;
typedef	struct {
	int		whoami			;	// READER/WRITER/COOKER
	char	procname	[32];	// process name
	int		exid			;	// shm key 조합용
	char	exnm		[16];	// exhange name
	char	excode		[ 1];	// 'S'MBS/'K'MBS/E'MBS/'C'MBS/'B'est/'Z'CUST
	int		open_khms		;	// 시작시간
	int		clos_khms		;	// 종료시간
	int		openbat_khms	;	// 오픈배치 시간
	MDARCH	*arch			;	// (MDARCH *)
	MDFOLD	*fold			;	// (MDFOLD *)
} MARKET;

//#####################################
//##### mds/src/inc/mds/mdfold.h
typedef struct {
	char	 excode		[ 1];			// BID 원천 : 'S'MBS/'K'MBS/E'MBS/'C'MBS
	double	 open			;
	double	 open_tm        ;
	double	 high			;
	double   high_tm        ;
	double	 lowp			;
	double   lowp_tm        ;
	double	 last			;
	double   last_tm        ;
	double	 lastvol		;
	double   last_valid	  	;			// last 중'0'이 아닌값 
	double	 base			;			// 전일종가
	double	 best			;
	double	 bestvol		;
	int		 sign			;
	double	 diff			;
	double	 rate			;
	int		 dirf			;
} mdside_t; 

typedef struct {
	double  swap_bid		;
	double 	swap_ask		;
	double  swap_bid_man	;			// 수기입력
	double  swap_ask_man	;			// 수기입력
	mdside_t bid			;			// 
	mdside_t ask			;			// 
	mdside_t mid			;			// 
	int 	 tick_seqn		;
	MDCANDLE mdintr			;
	char	 tickkey[32]	;
} mdquot_t;

/*
struct mdfold {
	char	 symb		[SYMB_LEN];
	int		 seqn			;			// MDFOLD OFFSET
	unsigned tymd			;			// current trading day
	unsigned kymd			;			// last update date
	unsigned khms			;			// last update time
	int		 zdiv			;			// 직원 소수점자리수
	int		 custzdiv		;			// 고객 소수점자리수
	int		 swapzdiv		;			// 스왑 소수점자리수
	int		 mrktzdiv		;			// 마켓 소수점자리수
	int		 feed			;			// data feeder id number
	int		 trdf			;			// tradable flag
	int	 	 open_hms		;			// 장시작시간(hhmmss);

	mdquot_t mdquot[10]		;			// SPT, TOD, TOM, W01, M01, M02, M03, M06, Y01

	int		 crossrate		;			// 크로스레이트 여부
	char     fillid[32]     ;
	int		 fillseq        ;			// 체결일련번호	
	double   fillprc		;			// 체결가격
	double	 fillqty		; 			// 체결수량
	kswitch_t ks            ;           // 킬스위치
#if 1
	int		 close_hms		;			// 장종료시간
#endif
	char	 filler[256-4]    ;
};
*/

struct mdfold {
	char	 symb		[SYMB_LEN];
	int		 seqn			;			// MDFOLD OFFSET
	unsigned tymd			;			// current trading day
	unsigned kymd			;			// last update date
	unsigned khms			;			// last update time
	int		 zdiv			;			// 직원 소수점자리수
	int		 custzdiv		;			// 고객 소수점자리수
	int		 swapzdiv		;			// 스왑 소수점자리수
	int		 mrktzdiv		;			// 마켓 소수점자리수
	int		 feed			;			// data feeder id number
	int		 trdf			;			// tradable flag
	int	 	 open_hms		;			// 장시작시간(hhmmss);

	mdquot_t mdquot[10]		;			// SPT, TOD, TOM, W01, M01, M02, M03, M06, Y01

	int		 crossrate		;			// 크로스레이트 여부
	char     fillid[32]     ;
	int		 fillseq        ;			// 체결일련번호	
	double   fillprc		;			// 체결가격
	double	 fillqty		; 			// 체결수량
	kswitch_t ks            ;           // 킬스위치
#if 1
	int		 close_hms		;			// 장종료시간
#endif
	time_t 	 kswitch_time_usd;			// USD/KRW 킬스위치 작동 시간
	time_t	 kswitch_time_etc;			// 기타통화 킬스위치 작동시간
	int 	 best_flag;
	char	 filler[256-32]    ;
};



typedef struct {
    int     tymd      ; // 영업일
    char    exnm  [ 8]; // 거래소명
    char    symb  [ 8]; // 통화페어
    int     seqn      ; // 일련번호
    int     kymd      ; // 날짜
    int     khms      ; // 시간
    char    fillid[32]; // 체결번호
    double  fillprc   ; // 체결가격
    double  fillqty   ; // 체결수량
} mdfill_t;


//################################################################################################
// MDS.C
//################################################################################################

// MDS INITIALIZE
int    xchg_count();
XCHG*  xchg_get(const char *exnm);
XCHG*  xchg_getn(int idx);
int    market_init();
void   market_destroy();
MARKET* market_get(const char *exnm);
MARKET* market_getn(int idx);
MDFOLD* market_getfold(const char *exnm, const char *symb);
MDFOLD* market_find(MARKET *market, char *symb);
int     market_is_workinghour(MARKET *market, int khms);
int get_weekymd(int ymd);
void merge_candle(MDCANDLE *dest, MDCANDLE *src);


int select_fill(MARKET *market, char *exnm, char *symb, char *id, double *prc, double *qty);
double get_timestamp();
char*  get_timestampstr(double ts);
time_t get_timestampmktime(char *ts);
int get_openbatchtime();
int get_closebatchtime();
int get_marketopentime();
int get_marketclosetime();
int get_market_decimal_place(char *code);

/**
 * @brief 재정환율 여부
 */
int  mdfold_is_arbitrage(char *symb);

/**
 * @brief 크로스래이트 여부
 */
int  mdfold_is_crossrate(char *symb);

/**
 * @brief DB 저장용 tick 데이터 만든다
 */
MDCANDLE* mdfold_new_tick(MDFOLD *pfold, int tenor_idx, char *exnm);

/**
 * @brief DB 저장용 분 데이터 만든다
 */
MDCANDLE* mdfold_new_intr(MDFOLD *pfold, int tenor_idx, char *exnm);

/**
 * @brief DB 저장용 일 데이터 만든다
 */
MDCANDLE* mdfold_new_heod(MDFOLD *pfold, int tenor_idx, char *exnm);


/**
 * @brief DB 저장용 체결 데이터 만든다.
 */
mdfill_t* mdfold_new_fill(MDFOLD *pfold, char *exnm);

/**
 * @brief 공유메모리 시세 업데이트 (bid, ask)
 */
int  mdquot_update_bidask(mdquot_t *mdquot, MDSSISE *mdssise);

/**
 * @brief 공유에모리 스왑레이트 업데이트
 */
int mdquot_update_swaprate(mdquot_t *mdquot, double swap_bid, double swap_ask);

/**
 * @brief 테너 이름을 인덱스로 변환
 */
int tenor2index(char *in_tenor);

char *tenor_name(int idx);
void mdfold_print(MDFOLD *fold);
double round_double(double val, int zdiv);
int getxmlcfg_opentime(const char *exnm, const char *code);
int getxmlcfg_closetime(const char *exnm, const char *code);
void mdfold_update_openclose_time(const char *exnm, MDFOLD *pfold);
int  mdfold_is_workinghour(MDFOLD *fold, int hms);

#endif
