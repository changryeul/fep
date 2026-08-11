/*******************************************************************************
 * (C) COPYRIGHT Winway Systems Co., Ltd. 2014
 * All Rights Reserved
 * Licensed Materials - Property of WINWAY Co., Ltd.
 *
 * This program contains proprietary information of WINWAY Co., Ltd
 * All embodying confidential information, ideas and expressions can't be
 * reproceduced, or transmitted in any form or by any means, electronic, 
 * mechanical, or otherwise without the written permission of WINWAY.
 *
 *  Components   : mds.h - Marketdata processing System
 *  Release Ver  : 1.0.0
 ******************************************************************************/
#ifndef	_MDS_H_
#define	_MDS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <limits.h>
#include <math.h>
//#include <ctype.h>
//#include "isam.h"
#include "mdcommon.h"
//#include "mdfold.h"
#undef	TICK

#define	TZ_KST	"TZ=ROK"

#define	MAX_WHOIS	5000

#define	LEDGER_RKEY		"glb"

#define	MAX_PORT	80


#define	LM_SUFFIX	".1"
//#define	LM_SUFFIX	"00"
#define	FILTER_MKEY_IDX	MAX_PORT+1

#define	PUSH_QUOT	0x01		// event bits (REAL-TR : ?A) ? : On exchange configuration 
#define	PUSH_BOOK	0x02		// event bits (REAL-TR : ?B) ? : On exchange configuration
#define	PUSH_MARKET	0x04		// event bits (REAL-TR : ?C) ? : On exchange configuration
#define	PUSH_PRED	0x08		// event bits (REAL-TR : ?D) ? : On exchange configuration
#define	PUSH_TUJA	0x10		// event bits (REAL-TR : ?E) ? : On exchange configuration

// 종목상태 (stat)
#define	OPENED		0x01
#define	CLOSED		0x02
#define	STOPED		0x04		// 매매거래정지
#define	MKTERR		0x08		// 장애발생

// 종목체크정보(jchk)
#define	JCHK_LM		0x01		// lead month symbol
#define	JCHK_LM_	0x02		// lead month symbol of next trading day

#define TNRTOM  "01"
#define TNRSPOT "02"
#define TNR1W  "03"
#define TNR1M  "10"
#define TNR2M  "11"
#define TNR3M  "12"
#define TNR6M  "15"
#define TNR9M  "18"
#define TNR12M "21"
 

//#define MAX_CODETYPE	25

static char subCodeSpot[1][SYMB_SUBLEN] = {"300"};
static char subFowardode[MAX_TENNER][SYMB_SUBLEN] = {"403","410","411","412","415","418","421","401","402"};
static char subSwapcode[MAX_TENNER][SYMB_SUBLEN] =  {"503","510","511","512","515","518","521","501","502"};



//
// Exchange definition
//
typedef	struct {
	int		exid;			// exchange id
	char	exnm[8];		// short name
	int		type;			// exchange type
	char	desc[40];		// exchange description(english name)
	int		ipck;			// IPC key for shared memory & ....
	int		schm;			// Schema
	int		xxxx;			// reverved
	int		book_level;		// reverved
	char	dirp[128];		// data file pathname or database connection string
	char	TZ[60];			// time zone
	struct	{
		int	from;			// from time (hhmm)
		int	to;				// to time (hhmm)
		int	wday;			// trading day of week
		int	h24t;			// no weekend & 24 our rading
	} trading;				// trading hour
	struct	{
		int	open;			// open batch time
		int	close;			// close batch
	} batch;
	int		delay_t[2];		// time to delay(seconds)
	int		room;			// maximum no of symbol for shared memory
	int		keep[2];		// TICK, INTR saving days
	int		push;			// realtime push
	int		push_id;		// realtime push id
	int		bfil;			// realtime filterling 1:filter
	int		qfil;			// realtime filterling 1:filter
	int		nofr;			// no of receive threads
	int		nofp;			// no of port to receive 
	int		moff;			// valid months offset for symbols
	struct	{
		int		seqn;		// if 0, undefined port
		char	name[16];	// port id (=product name)
		char 	ipad[20];	// ip address
		int  	port;		// port number
	} from[MAX_PORT];
	struct	{
		int		cast;		// if TRUE, multicasting
		char	neta[20];	// local address
		char	ipad[20];	// multicasting IP-address
		int		port;		// 
	} notify;
	int	watch[24][60];		// watch interval
	struct	{
		int		interval;
		int		many;
		char	phone[8][16];
	} alert;
	int		llog;			// log level
	char	logf[128];		// log path
} XCHG;

// xchg.type
#define	XT_STOCKS	0x01
#define	XT_FUTURE	0x02
#define	XT_OPTION	0x04
#define	XT_FUTOPT	0x08

typedef	struct {
	int	type;				// FOREX, FUTURE, OPTION
	int	flag;				// open flags
	int	whoami;				// who am i ?
	char	procname[32];			// process name
	int	delayed;			// delayed open ?
	int	exid;				// exchange ID
	char	exnm[16];			// exhange name
	int	dbis;				// access data ?
	char	dirp[128];			// data directory path
	char	TZ[60];				// time zone
	time_t	g2et;				// GMT to exchange time
	time_t	e2lt;				// diference with local time
	int	isdst;				// daylight saving time ?
	struct	{
		int	from;			// from time (hhmm)
		int	to;			// to time (hhmm)
	} trading;
	struct	{
		int	open;			// open batch time
		int	close;			// close batch
		int	batch[4];		// batch data sending time
		int	eod[4];			// eod sending time
	} batch;
	uint32_t xymd;				// current date
	uint32_t xhms;				// current time
	uint32_t kymd;				// current date
	uint32_t khms;				// current time
	uint32_t lymd;				// last trading day
	uint32_t tymd;				// current trading day for CME evlauation corrent trading day
	uint32_t nymd;				// next trading day for CME evlauation correct trading day
	int	delay;				// delayed processsing ?
	int	endian;				// LITTLE or BIG ENDIAN ?
	char	logf[128];			// log file name
	int	llog;				// log level
	XCHG	*xchg;				// exchange information
	void	*arch;				// archive shared memory pointer
	void	*indx;				// pointer of INDEX
	void	*fold;				// symbol folder shared memory pointer
	void	*fptr;				// current position of folder to access folder sequencially
	void	*info;				// market information
	void	*ctx;				// context
	void	*prod;				// product information
	void	*ttbl;				// tick table
	void	*usr;				// user's application information 
} MARKET;

// MDS.whoami
#define	I_AM_COOKER	1			// market data cooker(open mode : O_RDWR|O_CREAT)
#define	I_AM_WRITER	2			//  WRONLY or RDWR
#define	I_AM_READER	3			// RDONLY

#define	B_ENDIAN	0			// Big endian
#define	L_ENDIAN	1			// Little endian

typedef	struct	{
	int  (*open)(MARKET *);			// market open procedure
	int  (*recv)(MARKET *, void *);		// market data receiver
	int  (*clos)(MARKET *);			// market closing procedure
	void (*xday)(MARKET *);			// change to new day
} MDSPROC;

#define	YMD(y,m,d)	((y) * 10000 + (m) * 100 + (d))
#define	HMS(h,m,s)	((h) * 10000 + (m) * 100 + (s))
#define	YEAR(x)		((x)/10000)
#define	MONTH(x)	((x)%10000) / 100
#define	MDAY(x)		((x) % 100)
#define	HOUR(x)		((x)/10000)
#define	MINUTE(x)	((x)%10000) / 100
#define	SECOND(x)	((x)%100)
#define	ENDMK_HMS	888888			// end of market
#define	ENDAH_HMS	999999			// end of after hour trading
#define	YYMMDD(ymd)	(ymd % 1000000)
#define	CCYYMMDD(ymd)	YEAR(ymd) <= 99 ? ymd + 20000000 : ymd

// NetChgSign
#define	_UL_	1
#define	_UP_	2
#define	_NC_	3
#define	_DL_	4
#define	_DN_	5

//
// instrument(security) type
//
typedef	enum {
	ST_STOCK = 0,
	ST_PREFERRED_STOCK,
	ST_ETF,
	ST_ELW,
	ST_WARRANT,
	ST_BC,					// beneficiary certificate
	ST_ETN,
	ST_INDEX = 10,
	ST_FUTURE,
	ST_OPTION,
	ST_FOREX,
	ST_PRODUCT,				// 현물
	ST_SPREAD				// 스프레드

} INSTRUMENT_TYPE;

// underlying product type
typedef	enum {
	UP_UNKNOWN = 0,		
	UP_INDEXES = 1,	
	UP_AGRICULTURE,			
	UP_CURRENCY = 4,
	UP_EQUITY,
	UP_OTHERS = 12,
	UP_INTEREST_RATE = 14,
	UP_FX_CASH,
	UP_ENERGY,
	UP_METAL
} UNDERLYING_PRODUCT;

#define		MAX_SEEK_FOLD		1000
typedef	struct {
	int	n_folder;			// number of folder
	void   *l_folder;			// folder pointer of lead month symbol
	void   *p_folder[MAX_SEEK_FOLD];			// folder ponter
} WHERE;



#define	LOG_MUST	0
#define	LOG_ERROR	1
#define	LOG_WARNING	2
#define	LOG_PROGRESS	3
#define	LOG_DEBUG	4


//  file type :
#define	MSTR_TYPE	1
#define	TICK_TYPE	2
#define	HEOD_TYPE	3
#define	DAY1_TYPE	4

//  file id number :
#define	MSTR		0			// Master
#define	QUOT		1			// Quotes
#define	BOOK		2			// market depth
#define	TICK		3			// Intraday tick
#define	INTR		4			// Intraay 1 minute candle
#define	TUJA		5			// statistics by investor
#define	PRED		9			// predicted price

#define	HEOD		10			// Historical price data
#define	TEOD		11			// Historical investor data
#define	VEOD		12			// option implied volatility

typedef	struct {
	int	dbid;				// file ID number
	int	dodo;				// action flags
	char	name[20];			// file name
	int	type;				// naming type
	int	doff;				// offset of date
	int	size;				// size of record
	struct	keydesc *keydesc;		// CISAM key description
} SCHEMA;

#define	_OP_	0x01				// automatic open 
#define	_CR_	0x02				// automatic creation by market startup
#define	_DF_	0x04				// do deferred synchronization

typedef	struct {
	int	isfd;				// isam file descriptor
	int	iserrno;			// error number
	MARKET	*market;			// market for isam
	SCHEMA	*schema;			// schema definition
	int	busy;				// busy flag
} ISAMF;

typedef	struct {
	char	exnm[8];			// exchange name
	int	(*init)(MARKET *);		// initializer`
} BELONG;

int		mds_init(MARKET *market);
MARKET *mds_open(const char *exnm, int mode);
int		mds_cooker(MARKET *market, MDSPROC *procedure, int noreturn);
void	mds_close(MARKET *market);

int 	mds_exchange(const char *exnm, XCHG *xchg);
XCHG   *mds_exchanges();
XCHG   *mds_exchange_by_exid(int exid);


// market open/close
void	mds_market_open(MARKET *market);
void	mds_market_close(MARKET *market);

// shm folder access
void  *	mds_getfolder(MARKET *market, const char *symb);
void	mds_setfolder(MARKET *market, const char *symb);
int		mds_rewfolder(MARKET *market);
void  * mds_popfolder(MARKET *market);
void  * mds_newfolder(MARKET *market, const char *symb);
void    mds_delfolder(MARKET *market, const char *symb);
void  	mds_clrfolder(MARKET *market, void *folder);
int     mds_syncfolder(MARKET *market, void *folder, int dbid);
void    mds_pushfolder(MARKET *market, void *folder, int event);
int     mds_seekfolder(MARKET *market, const char *prefix, WHERE *where);
int		mds_lmsfolder(MARKET *market, WHERE *where, const char *symbols);
void	mds_pushtuja(MARKET *market, void *tuja);

// date & time
uint32_t mds_nextday(uint32_t ymd);
uint32_t mds_prevday(uint32_t ymd);
uint32_t mds_pastday(MARKET *market, uint32_t base, int doff);
void 	 mds_timezone(MARKET *market);
void 	 mds_calendar(MARKET *market);
int 	 mds_holiday(MARKET *market, uint32_t locdate, int caller);
int 	 mds_chckday(MARKET *market, uint32_t xymd);
void 	 mds_time(MARKET *market, time_t clock, uint32_t *xymd, uint32_t *xhms, uint32_t *kymd, uint32_t *khms);
void 	 mds_ktime(MARKET *market, time_t clock, uint32_t *kymd, uint32_t *khms);
void 	 mds_kortime(MARKET *market, uint32_t xymd, uint32_t xhms, uint32_t *kymd, uint32_t *khms);
int 	 mds_korhour(MARKET *market, time_t clock);
void 	 mds_loctime(MARKET *market, uint32_t kymd, uint32_t khms, uint32_t *xymd, uint32_t *xhms);
int 	 mds_fixtime(MARKET *market, const char *string, uint32_t *xymd, uint32_t *xhms, uint32_t *kymd, uint32_t *khms);
time_t	 mds_mktime(MARKET *market, uint32_t xymd, uint32_t xhms);
uint32_t mds_utc2loc(MARKET *market, const char *tstring);
uint32_t mds_date2julian(uint32_t date);
uint32_t mds_julian2date(uint32_t days);
int 	 mds_day4week (uint32_t date);
int 	 mds_day4year(uint32_t date);
void mds_kortm(MARKET *market, struct tm *xtm, struct tm *ktm);

// data tabel i/o functions
int		mds_upsert(MARKET *market, void *folder, int dbid, const void *row);
int		mds_fetch(MARKET *market, int dbid, void *row, int mode, int limit, int cflag);

// cisam function for market
int		mds_isinit(MARKET *market);
int		mds_isopen(MARKET *market);
int		mds_isbuild(MARKET *market, int dbid, uint32_t xymd);
int		mds_isrenewal(MARKET *market);
int		mds_isupsert(MARKET *market, int dbid, void *record);
int		mds_isdelete(MARKET *market, int dbid, void *record);
int		mds_iscleanup(MARKET *market, int dbid);
int		mds_isdrop(MARKET *market, int dbid, const char *file_name);
int		mds_iskeep(MARKET *market, int dbid, int days);
int		mds_isfd(MARKET *market, int dbid, void *record, int *flag);
int		mds_fetch_isfd(MARKET *market, int dbid, int xymd, int *flag);
void	mds_isclose(MARKET *market);
void	mds_islock(MARKET *market);
void	mds_isunlock(MARKET *market);
void	mds_key2isam(MARKET *market, void *row, struct keydesc *keydesc);
void	mds_key2host(MARKET *market, void *row, struct keydesc *keydesc);

int		mds_fetch(MARKET *market, int dbid, void *record, int mode, int limit, int cflag);
int		mds_isfetch(MARKET *market, int dbid, void *record, int mode, int limit, int cflag);

// cisam wrapper function
int		is_list(MARKET *market, int dbid, int *xymd);
ISAMF *	is_open(MARKET *market, int dbid, int xymd, int mode);
ISAMF *	is_build(MARKET *market, int dbid, int xymd);
int		is_read(ISAMF *isamf, char *record, int mode);
int		is_write(ISAMF *isamf, char *record);
int		is_rewrite(ISAMF *isamf, char *record);
int		is_upsert(ISAMF *isamf,  char *record);
void	is_close(ISAMF *isamf);

// misc functions
void	mds_alert(MARKET *market, const char *msg);
void	mds_postmsg(int exid, const char *msg);
void	mds_symb4push(MARKET *market, char *pushsymb, const char *symb, int delayed);
int 	mds_exec(MARKET *market, const char *command, int wait);
void 	mds_lock(MARKET *market);
void 	mds_unlock(MARKET *market);
void 	mds_sleep(int microseconds);
void 	mds_procname(char *procname);
void 	mds_log(MARKET *market, int level, const char *format, ...);
void 	mds_printf(const char *file, unsigned int line, const char *func, const char *format, ...);
uint32_t 	mds_tickbegin(MARKET *market, char *symb, uint32_t xymd);
int 	mds_tickend(MARKET *market, char *symb, uint32_t xymd);
uint32_t	mds_gettymd(MARKET *market);
int 	mds_tmplog(MARKET *market, char *pname, const char *format, ...);
int		csvform(const char *, char csvb[32][128]);

// leadmonth functions
void	mds_leadmonth_symbol(MARKET *market);
void	mds_leadmonth_update(MARKET *market, char *symb);
void    mds_t1_leadmonth_update(MARKET *market, char *symb);


#define     PI          3.141592653589793238462643 
#define     DaysInYear  365     // 1년은 365일

double sgn( double x);  // sign function
double f(double x, double y, double aprime, double bprime, double rho);






void	qpricef(char *buf, const char *form, double price, double base);
void	changef(char *buf, const char *form, int sign, double diff);
void	udratef(char *buf, double rate);

void 	str2s(char *ts, int tl, char *fs, int fl);
int 	str2i(char *s, int l);
int64_t str2l(char *s, int l);
float 	str2f(char *s, int l);
double 	str2d(char *s, int l);
double 	str2p(char *s, int l, int denominator, int prefix);

#define	STR2S(x, y)	str2s(x, sizeof(x), y, sizeof(y))
#define	STR2I(x)	str2i(x, sizeof(x))
#define	STR2L(x)	str2l(x, sizeof(x))
#define	STR2F(x)	str2f(x, sizeof(x))
#define	STR2D(x)	str2d(x, sizeof(x))
#define	STR2P(x,y,z)	str2p(x, sizeof(x), y, z)
#define	STR2N(x)	memset(x, 0, sizeof(x))



#if (defined (DEBUG))
#   define LOG(v,  l, f, ...) 		mds_log(v, l, __FILE__, __LINE__, __func__, f, __VA_ARGS__)
#   define PRINTF(f, ...)		mds_printf(__FILE__, __LINE__, __func__, f, __VA_ARGS__)
#else
#   define LOG(v,  l, f, ...) 		mds_log(v, l, NULL, 0, NULL, f, __VA_ARGS__)
#   define PRINTF(f, ...)
#endif

#define SET_STR(buf, fmt, ...) do {						\
	    int  _x_len_;                                   \
	    char _x_buf_[1024*2] = {0,};                    \
	    _x_len_ = sprintf(_x_buf_, fmt, ##__VA_ARGS__); \
	    _x_buf_[_x_len_] = ' ' ;                        \
	    memcpy(buf, _x_buf_, sizeof(buf));              \
} while(0) 

#define SET_PRC(buf, fmt, ...) do{						\
	    int  _x_len_;                                   \
	    char _x_buf_[1024*2] = {0,};                    \
	    _x_len_ = sprintf(_x_buf_, fmt, ##__VA_ARGS__); \
	    _x_buf_[_x_len_] = ' ' ;                        \
	    memcpy(buf, _x_buf_, sizeof(buf));              \
		if (atof(buf) == 0.)							\
			memset(buf, ' ', sizeof(buf));				\
} while(0)

#ifdef __cplusplus
}
#endif



#endif
