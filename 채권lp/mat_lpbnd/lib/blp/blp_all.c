/** ***************************************************************************
**  @file       blp.c
**  @date       2025/09/01
**  @author     cdc
**  @version    V0.0.20250901
**  @brif
**  채권 시장조성 라이브러리
**	blp.c			- 
***************************************************************************** */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#include "shm_memory.h"

#define	LogErr( format, args...)		Log( SYS_ERROR, format, ##args)
#define	LogLib( format, args...)		Log( USR_ERROR, format, ##args)
#define	LogCri( format, args...)		Log( USR_ERROR, format, ##args)
#define	LogMsg( format, args...)		Log( USR_OK, format, ##args)
#define	LogDbg( format, args...)		Log( USR_OK, format, ##args)
#define	LogWar( format, args...)		Log( USR_OK, format, ##args)
#define	LogRaw( format, args...)		Log( USR_OK, format, ##args)
#define	LogDel( format, args...)		

/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/
/***<mem.h>******************************************************************************/
#ifndef MEM_H
#define	MEM_H	1
typedef struct _mem_
{
	key_t		key;
	int			id;
	size_t		sz;
	void		*ptr;
}   MEM;
#endif /* MEM_H */

/***<sem.h>******************************************************************************/
#ifndef SEM_H
#define	SEM_H	1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union SemUnion
{
	int 				val;
	struct semid_ds		*buf;
	unsigned short int	*array;
	struct seminfo		*info;
};

typedef struct _sem_
{
	key_t		key;
	int			id;
	int			nsems;
	int			lock_cnt;
	struct sembuf		*lock;
	struct sembuf		*unlock;
}   SEM;

#endif /* SEM_H */

/***<etc.h>******************************************************************************/
#ifndef TRIM
#define	TRIM( x)		( TrimNR( x, sizeof( x)))
#endif

/***<blp.h>******************************************************************************/
/** ***************************************************************************
**  @file       blp.c
**  @date       2023/08/23
**  @author     cdc
**  @version    V2.0.20230823
**  @brif
**  매칭엔진 라이브러리 
**
**  blp.c		- 매칭엔진 기본 라이브러리
**  blp_group.c	- 그룹주문 처리
**  blp_stat.c	- 공유메모리 상태등를 출력
**  blp_jang.c	- 주문/매칭 장운영 check
**  convert.c	- 수신주문/처리주문/송신주문 간의 convert
**  print.c		- 출력을 위한 forblpting ... struct 명령을 기본
**  allog.c		- 수수료 엔진에서 쓰는 APLog를 위한 모듈
**  smq.c		- 주문수신 체결송신용 queue를 위한 모듈
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "shm_memory.h"

#ifndef BLP_H
#define	BLP_H		1

#define		BLP_MAX_TBL				40
#define		BLP_SISE_HOGA			5				/* 시세 호가 범위 - BLP_MAX_HOGA에 포함되므로 MAX_HOGA를 크게 잡아야 */
#define		BLP_MAX_HOGA			( 25 + ( BLP_SISE_HOGA * 2 ))		/* 매수 매도 포함 - 중간 가격을 구하기 위해 홀수로 설정 */
#define		BLP_MAX_LP				3				/* 시장 조성 호가 갯수 */

#define		BLP_MAX_RECORD			100000			/* 최대 주문 저장 건수 */
#define		BLP_MAX_CURRENT			32				/* max 통화 종류 - krw,usd,eur,jpy ... */
#define		BLP_MAX_CURR			64				/* max 거래 종류 - usd:krw, jpy:krw, eur:krw ... */ 
#define		BLP_MAX_JANG			1000			/* 장운영 check - 999 이상은 검토필요 */
#define		BLP_MAX_STATIS			10				/* max 통계 record */
#define		BLP_MAX_MSG				1000			/* error message blp record */
#define		BLP_MAX_GROUP			10				/* 동시에 처리 가능한 그룹 갯수 */
#define		BLP_IPC_KEY				0xfa001001		/* IPC 접근 key */
#define		BLP_ORD_PIPE			"blp_ord.fifo"	/* named pipe path */
#define		BLP_EXE_PIPE			"blp_exe.fifo"	/* named pipe path */
#define		BLP_BLP_PIPE			"blp_blp.fifo"	/* named pipe path */
#define		BLP_TIMEOUT				-9999
#define		BLP_MAX_CONFORM			100				/* 체결 역전 방지용 주문확인 전송 안된 주문 list */
#define		BLP_CONFORM_TIME		1				/* 주문확인이 안들어 왔을때 대기하는 시간 */
#define		BLP_CONFORM_TIMEOUT		30				/* 주문확인 없이 체결 전송 TIMEOUT  */

#ifndef MAX
#define		MAX(x,y)				((x>y)?x:y)
#define		MIN(x,y)				((x<y)?x:y)
#endif

/** ***************************************************************************
**  @st         BLP_STAT
**  @brief
**  채권시장조성 상태
***************************************************************************** */
typedef struct _blp_arg_
{
    char     sb31_exe_ymd                 [  8+1];  /* 1  실행일자                */
    int      sb31_exe_no                         ;  /* 2  실행번호                */
    char     sb31_proc_stus_dstcd         [  1+1];  /* 3  처리상태구분코드        */
    char     sb31_start_yms               [ 14+1];  /* 4  시작일시                */
    char     sb31_end_yms                 [ 14+1];  /* 5  종료일시                */
    char     sb31_lp_start_yms            [ 14+1];  /* 6  유동성공급시작일시      */
    char     sb31_lp_end_yms              [ 14+1];  /* 7  유동성공급종료일시      */
    char     sb31_item_cd                 [ 12+1];  /* 8  종목코드                */
    char     sb31_mm_item_dstcd           [  2+1];  /* 9  유동성공급종목구분코드  */
    double   sb31_sped_prc                [  BLP_MAX_LP];  /* 10 스프래드가격 1,2,3      */
    double   sb31_ord_qanty               [  BLP_MAX_LP];  /* 13 주문수량 1,2,3          */
    int      sb31_ord_qanty_unit                 ;  /* 16 주문수량단위            */
    char     sb31_dspratio_taget_dstcd    [  2+1];  /* 17 괴리율대상구분코드      */
    char     sb31_dspratio_dstcd          [  1+1];  /* 18 KRX괴리율구분코드       */  
    double   sb31_dspratio                       ;  /* 19 괴리율                  */
    double   sb31_tick_unit                      ;  /* 20 틱단위                  */
    char     sb31_trdr_uno                [  5+1];  /* 21 거래원번호              */
    char     sb31_trdr_no                 [  4+1];  /* 22 CMBS트레이더번호        */
    double   sb31_prv_yild                       ;  /* 23 민간평가수익률          */
    double   sb31_prv_prc                        ;  /* 24 민간평가가격            */
    double   sb31_clsng_prc                      ;  /* 25 종가                    */
    double   sb31_clsng_yild                     ;  /* 26 종가수익률              */
	char	sb31_account_no               [ 11+1];  /* 27 계좌번호                */
}	BLP_ARG;

/* **************************************************** */
/* 채권 우선호가(B601K, KTS) 462 byte                   */
/* **************************************************** */
typedef struct
{
    char tr_gbn                 [5];  /* TR CODE          */
    char seq_no                 [8];  /* 정보분배일련번호 */
    char board_id               [2];  /* 보드ID */
    char session_id             [2];  /* 세션ID */
    char item_code              [12]; /* 종목코드 */
    char trade_time             [12]; /* 매매처리시각 */
    char ask1_price             [11]; /* 매도1단계우선호가가격 */
    char bid1_price             [11]; /* 매수1단계우선호가가격 */
    char bond_ask1_remain_vol   [15]; /* 채권매도1단계우선호가잔량 */
    char bond_bid1_remain_vol   [15]; /* 채권매수1단계우선호가잔량 */
    char ask1_yield             [13]; /* 매도1단계우선호가수익률 */
    char bid1_yield             [13]; /* 매수1단계우선호가수익률 */
    char ask2_price             [11]; /* 매도2단계우선호가가격 */
    char bid2_price             [11]; /* 매수2단계우선호가가격 */
    char bond_ask2_remain_vol   [15]; /* 채권매도2단계우선호가잔량 */
    char bond_bid2_remain_vol   [15]; /* 채권매수2단계우선호가잔량 */
    char ask2_yield             [13]; /* 매도2단계우선호가수익률 */
    char bid2_yield             [13]; /* 매수2단계우선호가수익률 */
    char ask3_price             [11]; /* 매도3단계우선호가가격 */
    char bid3_price             [11]; /* 매수3단계우선호가가격 */
    char bond_ask3_remain_vol   [15]; /* 채권매도3단계우선호가잔량 */
    char bond_bid3_remain_vol   [15]; /* 채권매수3단계우선호가잔량 */
    char ask3_yield             [13]; /* 매도3단계우선호가수익률 */
    char bid3_yield             [13]; /* 매수3단계우선호가수익률 */
    char ask4_price             [11]; /* 매도4단계우선호가가격 */
    char bid4_price             [11]; /* 매수4단계우선호가가격 */
    char bond_ask4_remain_vol   [15]; /* 채권매도4단계우선호가잔량 */
    char bond_bid4_remain_vol   [15]; /* 채권매수4단계우선호가잔량 */
    char ask4_yield             [13]; /* 매도4단계우선호가수익률 */
    char bid4_yield             [13]; /* 매수4단계우선호가수익률 */
    char ask5_price             [11]; /* 매도5단계우선호가가격 */
    char bid5_price             [11]; /* 매수5단계우선호가가격 */
    char bond_ask5_remain_vol   [15]; /* 채권매도5단계우선호가잔량 */
    char bond_bid5_remain_vol   [15]; /* 채권매수5단계우선호가잔량 */
    char ask5_yield             [13]; /* 매도5단계우선호가수익률 */
    char bid5_yield             [13]; /* 매수5단계우선호가수익률 */
    char bond_total_ask_vol     [15]; /* 채권매도호가총잔량 */
    char bond_total_bid_vol     [15]; /* 채권매수호가총잔량 */
    char msg_end_key            [1];  /* 정보분배메세지종료키워드 */
}   _CO_B601K;                         // TR: 채권채결(KTS) B601K


/* **************************************************** */
/* 채권 체결(G701K, KTS)								*/
/* **************************************************** */
typedef struct
{
	char tr_gbn			    	[5];	/* TR CODE			*/
	char seq_no                 [8];	/* 정보분배일련번호 */
	char board_id            	[2];	/* 보드ID */
	char session_id          	[2];	/* 세션ID */
	char item_code             	[12];	/* 종목코드 */
	char trade_time          	[12];	/* 매매처리시각 */
	char crprc          	    [11];	/* 체결가격 */
	char volume              	[10];	/* 거래량 */
	char trade_date          	[8];	/* 거래일자 */
	char trade_amt		     	[22];	/* 거래대금 */
	char exec_yield          	[13];	/* 체결수익률 */
	char open_price          	[11];	/* 시가 */
	char high_price          	[11];	/* 고가 */
	char low_price           	[11];	/* 저가 */
	char open_yield          	[13];	/* 시가수익률 */
	char high_yield          	[13];	/* 고가수익률 */
	char low_yield           	[13];	/* 저가수익률 */
	char bond_accum_exec_vol 	[15];	/* 채권누적체결수량 */
	char accum_trade_amt     	[22];	/* 누적거래대금 */
	char settl_date          	[8];	/* 결제일자 */
	char ask1_price          	[11];	/* 매도1단계우선호가가격 */
	char bid1_price          	[11];	/* 매수1단계우선호가가격 */
	char bond_ask1_remain_vol	[15];	/* 채권매도1단계우선호가잔량 */
	char bond_bid1_remain_vol	[15];	/* 채권매수1단계우선호가잔량 */
	char ask1_yield          	[13];	/* 매도1단계우선호가수익률 */
	char bid1_yield          	[13];	/* 매수1단계우선호가수익률 */
	char ask2_price          	[11];	/* 매도2단계우선호가가격 */
	char bid2_price          	[11];	/* 매수2단계우선호가가격 */
	char bond_ask2_remain_vol	[15];	/* 채권매도2단계우선호가잔량 */
	char bond_bid2_remain_vol	[15];	/* 채권매수2단계우선호가잔량 */
	char ask2_yield          	[13];	/* 매도2단계우선호가수익률 */
	char bid2_yield          	[13];	/* 매수2단계우선호가수익률 */
	char ask3_price          	[11];	/* 매도3단계우선호가가격 */
	char bid3_price          	[11];	/* 매수3단계우선호가가격 */
	char bond_ask3_remain_vol	[15];	/* 채권매도3단계우선호가잔량 */
	char bond_bid3_remain_vol	[15];	/* 채권매수3단계우선호가잔량 */
	char ask3_yield          	[13];	/* 매도3단계우선호가수익률 */
	char bid3_yield          	[13];	/* 매수3단계우선호가수익률 */
	char ask4_price          	[11];	/* 매도4단계우선호가가격 */
	char bid4_price          	[11];	/* 매수4단계우선호가가격 */
	char bond_ask4_remain_vol	[15];	/* 채권매도4단계우선호가잔량 */
	char bond_bid4_remain_vol	[15];	/* 채권매수4단계우선호가잔량 */
	char ask4_yield          	[13];	/* 매도4단계우선호가수익률 */
	char bid4_yield          	[13];	/* 매수4단계우선호가수익률 */
	char ask5_price          	[11];	/* 매도5단계우선호가가격 */
	char bid5_price          	[11];	/* 매수5단계우선호가가격 */
	char bond_ask5_remain_vol	[15];	/* 채권매도5단계우선호가잔량 */
	char bond_bid5_remain_vol	[15];	/* 채권매수5단계우선호가잔량 */
	char ask5_yield          	[13];	/* 매도5단계우선호가수익률 */
	char bid5_yield          	[13];	/* 매수5단계우선호가수익률 */
	char bond_total_ask_vol  	[15];	/* 채권매도호가총잔량 */
	char bond_total_bid_vol  	[15];	/* 채권매수호가총잔량 */
	char msg_end_key         	[1];	/* 정보분배메세지종료키워드 */
}	_CO_G701K;		// TR: 채권채결(KTS) G701K


/** ***************************************************************************
**  @st         BLP_STAT
**  @brief
**  채권시장조성 상태
***************************************************************************** */
typedef struct _blp_stat_
{
	key_t	key;
	key_t	oms_key;
	int		service;								/* 1=service on, 0=not service */
	int		cnt;									/* 호가 테이블 갯수 */
}	BLP_STAT;

/** ***************************************************************************
**  @st         BLP_STAT
**  @brief
**  시세 5호가 convert 0:price 1:volume 
***************************************************************************** */
typedef struct _blp_sise_
{
	struct timeval	tv;
	struct timeval	tv_old;
	int				ho_cnt;							/* 호가 처리 건수 */
	int				exe_cnt;						/* 체결 처리 건수 */
	double			price;							/* 체결 가격 */
	double			volume;							/* 체결 수량 */
	double			ask[ 3][ BLP_SISE_HOGA +1];		/* 매도 [ 가격/수량][ 호가1~5] */
	double			bid[ 3][ BLP_SISE_HOGA +1];		/* 매수 [ 가격/수량][ 호가1~5] */
}	BLP_SISE;

/** ***************************************************************************
**  @st         BLP_STAT
**  @brief
**  채권시장조성 호가 RECORD - 호가 테이블 구성 record
***************************************************************************** */
typedef struct _blp_hoga_rec_
{
	int		ab;				/* ask=1 bid=2 */
	int		no;				/* record number */
	int		ho;				/* 호가 순번 */
	int		s_ho;			/* 시세 호가 순번 */
	double	price;			/* 가격 */
	double	volume;			/* 남은 수량 */
	int		ord_no;			/* LP 주문 순번 */
	int		gap;			/* 호가 단위 갭 */
	double	gap_vol;		/* 수량 차 */
}	BLP_HOGA_REC;

/** ***************************************************************************
**  @st         BLP_STAT
**  @brief
**  채권시장조성 호가 RECORD - 주문 
***************************************************************************** */
typedef struct _blp_ord_
{
	double		ask_prc;		/* 매도 주문 가격 */
	double		bid_prc;		/* 매수 주문 가격 */
	double		ask_vol;		/* 매도 주문 수량 */
	double		bid_vol;		/* 매수 주문 수량 */
	int			ho_no;			/* 시세 호가 테이블 위치 rec->no */
	char		ord_no[ 32];	/* 주문 번호 */
}	BLP_ORD;


/** ***************************************************************************
**  @st         BLP_STAT
**  @brief
**  채권시장조성 상태 호가 TABLE
***************************************************************************** */
typedef struct _blp_hoga_tbl
{
	int				id;						/* 종목별 id - 0번째 tbl은 전체 갯수 등록 */
	int				proc_cnt;				/* 처리 횟수 */
	int				seq_no;					/* 시세 순번 */
	char			board_id[ 2];
	char			item_code[ 12];			/* 종목코드 */
	double			tick;					/* 호가 단위 */
	int				ask_base;				/* 매도 1 호가 번호 */
	int				bid_base;				/* 매수 1 호가 번호 */
	int				ask_gap;				/* 직전 호가와 차이 */
	int				bid_gap;				/* 직전 호가와 차이 */
	int				ask_vol_gap;			/* 수량 차이 */
	int				bid_vol_gap;			/* 수량 차이 */
	BLP_ARG			arg;					/* 전략 parameter */
	BLP_ORD			order[ BLP_MAX_LP];		/* 주문 1,2,3 호가 */
	BLP_SISE		sise;					/* 거래소 시세 */
	BLP_HOGA_REC	rec[ BLP_MAX_HOGA];		/* 호가 table 0:매도 끝:매수 */
	BLP_HOGA_REC	old[ BLP_MAX_HOGA];		/* 이전 호가 table */
}	BLP_HOGA_TBL;

/** ***************************************************************************
**  @st         BLP_STAT
**  @brief
**  채권시장조성 상태 공유메모리 MAP
***************************************************************************** */
typedef struct _shm_map_
{
	BLP_STAT		stat;
	BLP_HOGA_TBL	tbl[ BLP_MAX_TBL];
	char		filler[ 1024];
}	BLP_MAP;

/** ***************************************************************************
**  @st         BLP_STAT
**  @brief
**  채권시장조성 상태 TAG Pointer
***************************************************************************** */
typedef struct _blp_
{
	key_t		key;				/* ipc 접근 key */
	MEM			*mem;				/* shared memory struct */
	SEM			*sem;				/* semaphore struct */
	BLP_MAP		*map;				/* shared memory base pointer */
}	BLP;

#endif	/* BLP_H */

double exp10(double x);
/***** Module : blp.c *****/
BLP*        Blp_CreateForce( key_t blp_key);                                /* 채권 시장조성에 필요한 ipc를 생성 */
BLP*        Blp_Create( key_t blp_key);                                     /* 채권시장조성에 필요한 ipc를 생성 */
int         Blp_Remove( BLP *blp);                                          /* 채권시장조성에서 생성한 ipc를 삭제 */
BLP*        Blp_Open( key_t blp_key, int service);                          /* 채권시장조성 Open */
int         Blp_Close( BLP *blp);                                           /* 채권시장조성 Close */
int         Blp_ShmInit( BLP *blp);                                         /* 채권시장조성 Close */
int         Blp_Lock( BLP *blp);                                            /* semaphore lock */
int         Blp_Unlock( BLP *blp);                                          /* semaphore lock */
int         Blp_GetItem( BLP *blp, char *item_code);                        /* 종목코드로 찾기 */
int         Blp_GetPrice( BLP *blp, BLP_HOGA_TBL *tbl, double price);       /* 가격으로 찾기 */
int         Blp_AddItem( BLP *blp, char *item_code);                        /* 종목코드 등록 */
int         Blp_Process( BLP *blp);                                         /* 채권시장조성 Close */
int         Blp_ProcessSise( BLP *blp, char *krx_sise);                     /* 거래소 시세 처리 */
int         Blp_ConvertExe( BLP *blp, BLP_SISE *sise, CO_G701K *g701k);     /* 거래소 체결 시세 처리 */
int         Blp_ConvertHoga( BLP *blp, BLP_SISE *sise, CO_B601K *b601k);    /* 거래소 호가 시세 처리 */
int         Blp_MakeHogaTbl( BLP *blp, BLP_HOGA_TBL *tbl, BLP_SISE *sise);  /* 호가 테이블 구성 */
int         Blp_CheckOrder( BLP *blp, BLP_HOGA_TBL *tbl, BLP_SISE *sise);   /* 기준 가격에 대한 유효 호가 설정  */
int         Blp_List( BLP *blp);                                            /* 채권시장조성 호가 테이블 리스트 */
int         Blp_Stat( BLP *blp, char *item_code);                           /* 채권시장조성 공유메모리 상태 */
int         Blp_GetSiseFromFile( BLP *blp, CO_B601K *b601k, char *f_name);  /* sise receive from file - for test */
int         Blp_SetArg( BLP *blp, char *rec);                               /* sise receive from file - for test */
char*       Blp_TvToS( struct timeval *tv);                                 /* util - struct timeval to string */
/***** Module : etc.c *****/
int         ToUpper( char *dst);                                            /* convert */
char*       AtoA( char *dst, char *src, int sz);                            /* convert */
char*       AtoAT( char *dst, int dst_sz, char *src, int src_sz);           /* convert */
char*       ItoA( char *rec, int value, int sz);                            /* convert */
char*       ItoA2( char *rec, int sz, int value);                           /* convert */
double      Dfloor( double dPrice, int ndecimal);                           /* 내림 ... 2진수 보정: Client 변환 함수 가져옴 */
double      NotDfloor( double value, int point);                            /* 내림 ... 2진수 보정 */
char*       DtoA( char *rec, double value, int sz);                         /* double to string */
char*       DtoAN( char *rec, double value, int sz, int n);                 /* double to string - 소숫점 자리, 반올림 (%sz.nf) */
char*       DtoAND( char *rec, double value, int sz, int n);                /* double to string - 소숫점 자리, 버림 (%sz.nf) */
char*       DtoANU( char *rec, double value, int sz, int n);                /* double to string - 소숫점 자리, 올림 */
char*       DtoALN( char *rec, double value, int sz, int n);                /* double to string - 소숫점 자리, 반올림 (%sz.nf), Left 정렬 */
char*       DtoALND( char *rec, double value, int sz, int n);               /* double to string - 소숫점 자리, 버림 (%sz.nf), Left 정렬 */
char*       DtoALNU( char *rec, double value, int sz, int n);               /* double to string - 소숫점 자리, 올림, Left 정렬 */
char*       DtoA0ND( char *rec, double value, int sz, int n);               /* double to string - 소숫점자리,버림(%sz.nf),0 채움 */
char*       DtoA0NU( char *rec, double value, int sz, int n);               /* double to string - 소숫점 자리,올림,0 채움 */
char*       DtoA0N( char *rec, double value, int sz);                       /* double to string - 0 채움 */
int         AtoI( char *rec, int sz);                                       /* convert */
long        AtoL( char *rec, int sz);                                       /* convert */
double      AtoF( char *rec, int sz);                                       /* convert */
double      AtoD( char *rec, int sz);                                       /* convert */
char*       LLtoA( char *rec, long long value, int sz);                     /* convert */
long long   AtoLL( char *rec, int sz);                                      /* convert */
char*       TtoS( time_t atime);                                            /* convert */
int         TtoA( char *str, time_t atime);                                 /* convert */
int         TtoAF( char *str, time_t atime, char *format);                  /* convert */
time_t      StoT( char *tstr);                                              /* convert */
int         TtoD( time_t cur_time);                                         /* convert time_t to YYMMDD */
int         TtoI( time_t cur_time);                                         /* convert time_t to HHMMSS */
char*       GetTimeStr();                                                   /* convert */
unsigned    StoI( char *str);                                               /* convert */
int         EtcMax( int a, int b);                                             /* convert */
int         EtcMin( int a, int b);                                             /* convert */
int         TrimR( char *rec);                                              /* convert */
int         TrimNR( char *rec, int len);                                    /* convert */
int         TrimN( char *rec, int len);                                     /* convert */
int         StrSetN( char *rec, int org, int dst, int len);                 /* convert */
int         IsHanGul( unsigned char *data, int pos);                        /* convert */
char*       GetPrevWeekDay( char *date);                                    /* 전일 날짜를 (YYYYMMDD) 형식으로 산출 */
/***** Module : blp_all.c *****/
MEM*        Mem_Create( key_t key, size_t sz);
MEM*        Mem_Open( key_t key);
int         Mem_Remove( MEM *mem);
int         Mem_RemoveByKey( key_t key);
int         Mem_Close( MEM *mem);
void*       Mem_GetPtr( MEM *mem);
SEM*        Sem_Create( key_t key);                                         /* semaphore create */
SEM*        Sem_Open( key_t key);                                           /* semaphore create */
int         Sem_Remove( SEM *sem);                                          /* semaphore create */
int         Sem_RemoveByKey( int key);                                      /* semaphore create */
int         Sem_Close( SEM *sem);                                           /* semaphore create */
int         Sem_GetId( SEM *sem);                                           /* semaphore create */
int         Sem_Lock( SEM *sem);                                            /* semaphore create */
int         Sem_LockT( SEM *sem, int timeout);                              /* semaphore lock - timeout(micro second) */
int         Sem_Unlock( SEM *sem);                                          /* semaphore create */
int         ToUpper( char *dst);                                            /* convert */
char*       AtoA( char *dst, char *src, int sz);                            /* convert */
char*       AtoAT( char *dst, int dst_sz, char *src, int src_sz);           /* convert */
char*       ItoA( char *rec, int value, int sz);                            /* convert */
char*       ItoA2( char *rec, int sz, int value);                           /* convert */
double      Dfloor( double dPrice, int ndecimal);                           /* 내림 ... 2진수 보정: Client 변환 함수 가져옴 */
double      NotDfloor( double value, int point);                            /* 내림 ... 2진수 보정 */
char*       DtoA( char *rec, double value, int sz);                         /* double to string */
char*       DtoAN( char *rec, double value, int sz, int n);                 /* double to string - 소숫점 자리, 반올림 (%sz.nf) */
char*       DtoAND( char *rec, double value, int sz, int n);                /* double to string - 소숫점 자리, 버림 (%sz.nf) */
char*       DtoANU( char *rec, double value, int sz, int n);                /* double to string - 소숫점 자리, 올림 */
char*       DtoALN( char *rec, double value, int sz, int n);                /* double to string - 소숫점 자리, 반올림 (%sz.nf), Left 정렬 */
char*       DtoALND( char *rec, double value, int sz, int n);               /* double to string - 소숫점 자리, 버림 (%sz.nf), Left 정렬 */
char*       DtoALNU( char *rec, double value, int sz, int n);               /* double to string - 소숫점 자리, 올림, Left 정렬 */
char*       DtoA0ND( char *rec, double value, int sz, int n);               /* double to string - 소숫점자리,버림(%sz.nf),0 채움 */
char*       DtoA0NU( char *rec, double value, int sz, int n);               /* double to string - 소숫점 자리,올림,0 채움 */
char*       DtoA0N( char *rec, double value, int sz);                       /* double to string - 0 채움 */
int         AtoI( char *rec, int sz);                                       /* convert */
long        AtoL( char *rec, int sz);                                       /* convert */
double      AtoF( char *rec, int sz);                                       /* convert */
double      AtoD( char *rec, int sz);                                       /* convert */
char*       LLtoA( char *rec, long long value, int sz);                     /* convert */
long long   AtoLL( char *rec, int sz);                                      /* convert */
char*       TtoS( time_t atime);                                            /* convert */
int         TtoA( char *str, time_t atime);                                 /* convert */
int         TtoAF( char *str, time_t atime, char *format);                  /* convert */
time_t      StoT( char *tstr);                                              /* convert */
int         TtoD( time_t cur_time);                                         /* convert time_t to YYMMDD */
int         TtoI( time_t cur_time);                                         /* convert time_t to HHMMSS */
char*       GetTimeStr();                                                   /* convert */
unsigned    StoI( char *str);                                               /* convert */
int         _Max( int a, int b);                                            /* convert */
int         _Min( int a, int b);                                            /* convert */
int         TrimR( char *rec);                                              /* convert */
int         TrimNR( char *rec, int len);                                    /* convert */
int         TrimN( char *rec, int len);                                     /* convert */
int         StrSetN( char *rec, int org, int dst, int len);                 /* convert */
int         IsHanGul( unsigned char *data, int pos);                        /* convert */
char*       GetPrevWeekDay( char *date);                                    /* 전일 날짜를 (YYYYMMDD) 형식으로 산출 */
BLP*        Blp_CreateForce( key_t blp_key);                                /* 채권시장조성 상태 */
BLP*        Blp_Create( key_t blp_key);                                     /* 채권시장조성에 필요한 ipc를 생성 */
int         Blp_Remove( BLP *blp);                                          /* 채권시장조성에서 생성한 ipc를 삭제 */
BLP*        Blp_Open( key_t blp_key, int service);                          /* 채권시장조성 Open */
int         Blp_Close( BLP *blp);                                           /* 채권시장조성 Close */
int         Blp_ShmInit( BLP *blp);                                         /* 채권시장조성 Close */
int         Blp_Lock( BLP *blp);                                            /* semaphore lock */
int         Blp_Unlock( BLP *blp);                                          /* semaphore lock */
int         Blp_GetItem( BLP *blp, char *item_code);                        /* 종목코드로 찾기 */
int         Blp_GetPrice( BLP *blp, BLP_HOGA_TBL *tbl, double price);       /* 가격으로 찾기 */
int         Blp_AddItem( BLP *blp, char *item_code);                        /* 종목코드 등록 */
int         Blp_Process( BLP *blp);                                         /* 채권시장조성 Close */
int         Blp_ProcessSise( BLP *blp, char *krx_sise);                     /* 거래소 시세 처리 */
int         Blp_ConvertExe( BLP *blp, BLP_SISE *sise, CO_G701K *g701k);     /* 거래소 체결 시세 처리 */
int         Blp_ConvertHoga( BLP *blp, BLP_SISE *sise, CO_B601K *b601k);    /* 거래소 호가 시세 처리 */
int         Blp_MakeHogaTbl( BLP *blp, BLP_HOGA_TBL *tbl, BLP_SISE *sise);  /* 호가 테이블 구성 */
int         Blp_CheckOrder( BLP *blp, BLP_HOGA_TBL *tbl, BLP_SISE *sise);   /* 기준 가격에 대한 유효 호가 설정  */
int         Blp_List( BLP *blp);                                            /* 채권시장조성 호가 테이블 리스트 */
int         Blp_Stat( BLP *blp, char *item_code);                           /* 채권시장조성 공유메모리 상태 */
int         Blp_GetSiseFromFile( BLP *blp, CO_B601K *b601k, char *f_name);  /* sise receive from file - for test */
int         Blp_SetArg( BLP *blp, char *rec);                               /* sise receive from file - for test */
char*       Blp_TvToS( struct timeval *tv);                                 /* util - struct timeval to string */
int         SHM_NOTE_Print( SHM_NOTE* ptr);
int         CO_B601K_Print( CO_B601K* ptr);
int         CO_B601K_PrintFile( CO_B601K* ptr, FILE *fp);
int         BLP_ARG_Print( BLP_ARG* ptr);


/***<mem.c>******************************************************************************/
/***** Module : memlog.c *****/
MEM*        MemLog_Create( key_t key, size_t sz);
MEM*        MemLog_Open( key_t key);
int         MemLog_Remove( MEM *mem);
int         MemLog_RemoveByKey( key_t key);
int         MemLog_Close( MEM *mem);
void*       MemLog_GetPtr( MEM *mem);

MEM *Mem_Create( key_t key, size_t sz)
{
	MEM		*mem;

	mem = ( MEM *)malloc( sizeof( MEM));
	if( mem == NULL)
	{
		LogErr( "malloc error. mem size=[%d]", sizeof( MEM));
		goto error;
	}
	memset( mem, 0x00, sizeof( MEM));

	mem->key = key;
	mem->sz = sz;

	mem->id = shmget( mem->key, mem->sz, IPC_CREAT | IPC_EXCL | 0666);
	if( mem->id < 0)
	{
		LogErr( "shmget error. key=[0x%08x], sz=[%d]", mem->key, mem->sz);
		goto error_1;
	}

	mem->ptr = shmat( mem->id, NULL, IPC_CREAT | 0666);
	if( mem->ptr == (void *)-1)
	{
		LogErr( "shmat error. id=[%d]", mem->id);
		goto error_1;
	}

	LogDel( "shard memory created. key=[0x%08x] id=[%d] sz=[%d]", key, mem->id, sz);
	return mem;

	error_1:
		free( mem);
	error:
		return NULL;
}

MEM *Mem_Open( key_t key)
{
	MEM		*mem;

	mem = ( MEM *)malloc( sizeof( MEM));
	if( mem == NULL)
	{
		LogErr( "malloc error. mem size=[%d]", sizeof( MEM));
		goto error;
	}
	memset( mem, 0x00, sizeof( MEM));

	mem->key = key;
	mem->sz = 0;

	mem->id = shmget( key, mem->sz, 0666);
	if( mem->id < 0)
	{
		LogErr( "shmget error. key=[0x%08x], sz=[%d]", mem->key, mem->sz);
		goto error_1;
	}

	mem->ptr = shmat( mem->id, NULL, 0666);
	if( mem->ptr == (void *)-1)
	{
		LogErr( "shmat error. id=[%d]", mem->id);
		goto error_1;
	}

	return mem;

	error_1:
		free( mem);
	error:
		return NULL;
}

int Mem_Remove( MEM *mem)
{
	int				rtn;
	struct shmid_ds	buf;

	rtn  = shmdt( mem->ptr);
	if( rtn < 0)
	{
		LogErr( "shmdt error. ptr=[%p]", mem->ptr);
		return rtn;
	}

	rtn = shmctl( mem->id, IPC_RMID, &buf);
	if( rtn < 0)
	{
		LogErr( "shmem remove error. id=[%d]", mem->id);
		return -1;
	}

	return rtn;
}

int Mem_RemoveByKey( key_t key)
{
	int				rtn;
	int				id;
	struct shmid_ds	buf;

	id = shmget( key, 0, 0666);
	if( id < 0)
	{
		LogErr( "shmget error. key=[0x%08x]", key);
		return -1;
	}

	rtn = shmctl( id, IPC_RMID, &buf);
	if( rtn < 0)
	{
		LogErr( "shmem remove error. id=[%d]", id);
		return -1;
	}

	return rtn;
}

int Mem_Close( MEM *mem)
{
	int		rtn;

	rtn = shmdt( mem->ptr);
	if( rtn < 0)
	{
		LogErr( "shmdt error. ptr=[%p]", mem->ptr);
		return -1;
	}

	free( mem);
	mem = NULL;
	
	return 1;
}

void *Mem_GetPtr( MEM *mem)
{
	return mem->ptr;
}

/***<sem.c>******************************************************************************/
/** ***************************************************************************
**  @file       sem.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  ipc semaphore lock 구현
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
SEM *Sem_Create( key_t key)
{
	int				rtn;
	SEM				*sem;
	union SemUnion	sem_union;

	sem = ( SEM *)malloc( sizeof( SEM));
	if( sem == NULL)
	{
		LogErr( "malloc error. sem size=[%d]", sizeof( SEM));
		goto error_3;
	}
	memset( sem, 0x00, sizeof( SEM));

	sem->lock = malloc( sizeof( struct sembuf));
	if( sem->lock == NULL)
	{
		LogErr( "malloc error. sem->lock size=[%d]", sizeof( struct sembuf));
		goto error_2;
	}
	sem->lock->sem_num = 0;
	sem->lock->sem_op = -1;
	sem->lock->sem_flg = SEM_UNDO;

	sem->unlock = malloc( sizeof( struct sembuf));
	if( sem->lock == NULL)
	{
		LogErr( "malloc error. sem->lock size=[%d]", sizeof( struct sembuf));
		goto error_1;
	}
	sem->unlock->sem_num = 0;
	sem->unlock->sem_op = 1;
	sem->unlock->sem_flg = SEM_UNDO;

	sem->key = key;
	sem->nsems = 1;
	sem->lock_cnt = 0;

	sem->id = semget( sem->key, sem->nsems, IPC_CREAT | IPC_EXCL | 0666);
	if( sem->id < 0)
	{
		LogErr( "semget error. key=[0x%08x]", sem->key);
		goto error;
	}

	sem_union.val = 1;
	rtn = semctl( sem->id, 0, SETVAL, sem_union);
	if( rtn < 0)
	{
		LogErr( "semctl error.");
		goto error;
	}

	LogDel( "semaphore created. key=[0x%08x] id=[%d] nsems=[%d]", key, sem->id, sem->nsems);
	return sem;

	error:
		free( sem->unlock);
	error_1:
		free( sem->lock);
	error_2:
		free( sem);
	error_3:
		return NULL;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
SEM *Sem_Open( key_t key)
{
	SEM		*sem;

	sem = ( SEM *)malloc( sizeof( SEM));
	if( sem == NULL)
	{
		LogErr( "malloc error. sem size=[%d]", sizeof( SEM));
		goto error_3;
	}
	memset( sem, 0x00, sizeof( SEM));

	sem->lock = malloc( sizeof( struct sembuf));
	if( sem->lock == NULL)
	{
		LogErr( "malloc error. sem->lock size=[%d]", sizeof( struct sembuf));
		goto error_2;
	}
	sem->lock->sem_num = 0;
	sem->lock->sem_op = -1;
	sem->lock->sem_flg = SEM_UNDO;

	sem->unlock = malloc( sizeof( struct sembuf));
	if( sem->unlock == NULL)
	{
		LogErr( "malloc error. sem->lock size=[%d]", sizeof( struct sembuf));
		goto error_1;
	}
	sem->unlock->sem_num = 0;
	sem->unlock->sem_op = 1;
	sem->unlock->sem_flg = SEM_UNDO;

	sem->key = key;
	sem->nsems = 0;
	sem->lock_cnt = 0;

	sem->id = semget( key, sem->nsems, 0666);
	if( sem->id < 0)
	{
		LogErr( "semget error. key=[0x%08x], nsems=[%d]", sem->key, sem->nsems);
		goto error;
	}

	LogDel( "semaphore opened. key=[0x%08x] id=[%d] nsems=[%d]", key, sem->id, sem->nsems);
	return sem;

	error:
		free( sem->lock);
	error_1:
		free( sem->unlock);
	error_2:
		free( sem);
	error_3:
		return NULL;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
int Sem_Remove( SEM *sem)
{
	int				rtn;
	union SemUnion	arg;

	arg.val = 1;
	rtn = semctl( sem->id, 0, IPC_RMID, arg);
	if( rtn < 0)
	{
		LogErr( "semem remove error. id=[%d]", sem->id);
		return -1;
	}

	free( sem->lock);
	free( sem->unlock);
	free( sem);

	return rtn;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
int Sem_RemoveByKey( int key)
{
	int				rtn;
	int				id;
	union SemUnion	arg;

	id = semget( key, 0, 0666);
	if( id < 0)
	{
		LogErr( "semget error. key=[0x%08x], nsems=[%d]", key, 0);
		return -1;
	}

	arg.val = 1;
	rtn = semctl( id, 0, IPC_RMID, arg);
	if( rtn < 0)
	{
		LogErr( "semem remove error. id=[%d]", id);
		return -1;
	}

	return rtn;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
int Sem_Close( SEM *sem)
{
	free( sem->lock);
	free( sem->unlock);
	free( sem);
	
	return 1;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
int Sem_GetId( SEM *sem)
{
	return sem->id;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
int Sem_Lock( SEM *sem)
{
	int		rtn;

	LogDel( "id=[%d] sem->loc_cnt=[%d]", sem->id, sem->lock_cnt);
	if( sem->lock_cnt > 0) 
	{
		/* LogWar( "already locked. sem->lock_cnt=[%d]", sem->lock_cnt); */
		return 0;
	}

	rtn = semop( sem->id, sem->lock, 1);
	sem->lock_cnt++;

	return rtn;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore lock - timeout(micro second)
***************************************************************************** */
int Sem_LockT( SEM *sem, int timeout)
{
	int				rtn;
	struct timespec	ts;

	LogDel( "id=[%d] sem->loc_cnt=[%d]", sem->id, sem->lock_cnt);

	if( sem->lock_cnt > 0) 
	{
		LogWar( "already locked. sem->id=[%d] sem->lock_cnt=[%d]", sem->id, sem->lock_cnt);
		usleep( timeout);
		return -1;
	}

	/* clock_gettime( CLOCK_REALTIME, &ts); */
	ts.tv_sec  = timeout / 1000000;
	ts.tv_nsec = ( timeout % 1000000) * 1000;

	rtn = semtimedop( sem->id, sem->lock, 1, &ts);
	if( rtn < 0)
	{
		LogErr( "semtimedop error. id=[%d] timeout=[%d.%d]", sem->id, ts.tv_sec, ts.tv_nsec);
		return rtn;
	}
	sem->lock_cnt++;

	return 1;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
int Sem_Unlock( SEM *sem)
{
	int		rtn;

	LogDel( "id=[%d] sem->loc_cnt=[%d]", sem->id, sem->lock_cnt);
	if( sem->lock_cnt == 0)
	{
		LogCri( "no locked action. sem->lock_cnt=[%d]", sem->lock_cnt);
		return -1;
	}

	rtn = semop( sem->id, sem->unlock, 1);
	if( rtn < 0)
	{
		LogErr( "semop error. id=[%d]", sem->id);
		return rtn;
	}

	sem->lock_cnt--;
	return 1;
}






/***<etc.c>******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <ctype.h>

#include <math.h>
/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int ToUpper( char *dst)
{
	int		i;
	int		sz = 0;

	sz = strlen( dst);

	for( i = 0; i < sz; i++)	dst[ i] = toupper( ( int)dst[ i]);
	
	return sz;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
char *AtoA( char *dst, char *src, int sz)
{
	char buf[ 512];

	sprintf( buf, "%-*s", sz, src);
	memcpy( dst, buf, sz);
	return dst;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
char *AtoAT( char *dst, int dst_sz, char *src, int src_sz)
{
	char buf[ 512];

	sprintf( buf, "%-*.*s", dst_sz, src_sz, src);
	memcpy( dst, buf, dst_sz);
	TrimNR( dst, dst_sz);
	return dst;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
char *ItoA( char *rec, int value, int sz)
{
	char buf[ 512];

	sprintf( buf, "%0*d", sz, value);
	memcpy( rec, buf, sz);
	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
char *ItoA2( char *rec, int sz, int value)
{
	char buf[ 512];

	sprintf( buf, "%0*d", sz, value);
	memcpy( rec, buf, sz);
	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  내림 ... 2진수 보정: Client 변환 함수 가져옴
***************************************************************************** */
double Dfloor( double dPrice, int ndecimal)
{
	double dValue = 0.0;
	double epsilon = 1e-8;
	char   strValue[ 512], *p;

	double dfactor = pow(10.0, ndecimal);
	if (dPrice > 0)
	{	
		dValue = dPrice * dfactor + epsilon;		
	}
	else if (dPrice < 0)
	{
		sprintf( strValue, "%.*f", ndecimal, dPrice * dfactor - epsilon);
		dValue = strtod( strValue, &p);
		/* if( p != NULL) LogMsg( "convert remain. p=[%p:%s]", p, p); */
	}
	else
	{
		return 0.0;
	}

	int nValue = (int)dValue;

	if (dPrice < 0 && (nValue - dValue) > 0.0)
		nValue -= 1;


	return nValue / dfactor;
}


/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  내림 ... 2진수 보정
***************************************************************************** */
double NotDfloor( double value, int point)
{
	double	e, d, d1;
	char 	buf[ 512];
	char	*p;

	e = exp10( ( double)point);
	d = value * e;
	sprintf( buf, "%15.*f", point, d);

	p = strchr( buf, '.');
	*p = 0;
	d1 = strtod( buf, &p);
	LogDbg( "d2=[%f]", d1);
	d = floor( d);
	LogDbg( "d3=[%f]", d);

	return d / e;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string
***************************************************************************** */
char *DtoA( char *rec, double value, int sz)
{
	char buf[ 512];

	sprintf( buf, "%*.4f", sz, value);
	memcpy( rec, buf, sz);
	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점 자리, 반올림 (%sz.nf)
***************************************************************************** */
char *DtoAN( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%*.*f", sz, n, round( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점 자리, 버림 (%sz.nf)
***************************************************************************** */
char *DtoAND( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%*.*f", sz, n, floor( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점 자리, 올림
***************************************************************************** */
char *DtoANU( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%*.*f", sz, n, ceil( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점 자리, 반올림 (%sz.nf), Left 정렬
***************************************************************************** */
char *DtoALN( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%-*.*f", sz, n, round( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점 자리, 버림 (%sz.nf), Left 정렬
***************************************************************************** */
char *DtoALND( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%-*.*f", sz, n, floor( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점 자리, 올림, Left 정렬
***************************************************************************** */
char *DtoALNU( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%-*.*f", sz, n, ceil( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점자리,버림(%sz.nf),0 채움
***************************************************************************** */
char *DtoA0ND( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%0*.*f", sz, n, floor( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점 자리,올림,0 채움
***************************************************************************** */
char *DtoA0NU( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%0*.*f", sz, n, ceil( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 0 채움
***************************************************************************** */
char *DtoA0N( char *rec, double value, int sz)
{
	char	buf[ 1024];

	sprintf( buf, "%0*f", sz, value);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int AtoI( char *rec, int sz)
{
	int		i;
	char	buf[ 512];

	memcpy( buf, rec, sz);
	buf[ sz] = 0;
	i = atoi( buf);
	return i;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
long AtoL( char *rec, int sz)
{
	long	l;
	char	buf[ 512];

	memcpy( buf, rec, sz);
	buf[ sz] = 0;
	l = atol( buf);
	return l;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
double AtoF( char *rec, int sz)
{
	double	d;
	char	buf[ 512];

	memcpy( buf, rec, sz);
	buf[ sz] = 0;
	d = atof( buf);
	return d;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
double AtoD( char *rec, int sz)
{
	double	d;
	char	buf[ 512];
	char	*ptr;

	memcpy( buf, rec, sz);
	buf[ sz] = 0;
	LogDel( "buf=[%s]", buf);
	d = strtod( buf, &ptr);
	LogDel( "d=[%f]", d);
	return d;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
char *LLtoA( char *rec, long long value, int sz)
{
	char buf[ 512];

	sprintf( buf, "%0*lld", sz, value);
	memcpy( rec, buf, sz);
	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
long long AtoLL( char *rec, int sz)
{
	long long		l;
	char			buf[ 512];

	memcpy( buf, rec, sz);
	buf[ sz] = 0;
	l = atoll( buf);
	return l;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
char *TtoS( time_t atime)
{
	static char tstr[ 512];
	struct tm	_tm, *tp = &_tm;
	struct tm	*tmp;

	if( atime == 0)
	{
		sprintf( tstr, "0000/00/00-00:00:00");
		return tstr;
	}
	tmp = localtime_r( &atime, tp);
	if( tmp == NULL)
	{
		LogErr( "localtime error. atime=[%ld]", atime);
		sprintf( tstr, "0000/00/00-00:00:00");
		return tstr;
	}

	sprintf( tstr, "%04d/%02d/%02d-%02d:%02d:%02d", 
		tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday,
			tp->tm_hour, tp->tm_min, tp->tm_sec);

	return tstr;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int TtoA( char *str, time_t atime)
{
	struct tm	*tp;
	char		*ptr;

	tp = localtime( &atime);
	if( tp == NULL)
	{
		LogErr( "localtime error. atime=[%ld]", atime);
		return 0;
	}

	ptr = strstr( str, "ss");	if( ptr) ItoA( ptr, tp->tm_sec, 2);
	ptr = strstr( str, "mm");	if( ptr) ItoA( ptr, tp->tm_min, 2);
	ptr = strstr( str, "hh");	if( ptr) ItoA( ptr, tp->tm_hour, 2);
	ptr = strstr( str, "DD");	if( ptr) ItoA( ptr, tp->tm_mday, 2);
	ptr = strstr( str, "MM");	if( ptr) ItoA( ptr, tp->tm_mon +1, 2);
	ptr = strstr( str, "YYYY");	
	if( ptr) 
	{
		ItoA( ptr, tp->tm_year + 1900, 4);
	}
	else 
	{
		ptr = strstr( str, "YY"); 
		if( ptr)	ItoA( ptr, tp->tm_year % 100, 2);
	}

	return 1;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int TtoAF( char *str, time_t atime, char *format)
{
	struct tm	*tp;
	char		*ptr;

	memcpy( str, format, strlen( format));

	tp = localtime( &atime);
	if( tp == NULL)
	{
		LogErr( "localtime error. atime=[%ld]", atime);
		return 0;
	}

	ptr = strstr( str, "ss");	if( ptr) ItoA( ptr, tp->tm_sec, 2);
	ptr = strstr( str, "mm");	if( ptr) ItoA( ptr, tp->tm_min, 2);
	ptr = strstr( str, "hh");	if( ptr) ItoA( ptr, tp->tm_hour, 2);
	ptr = strstr( str, "DD");	if( ptr) ItoA( ptr, tp->tm_mday, 2);
	ptr = strstr( str, "MM");	if( ptr) ItoA( ptr, tp->tm_mon +1, 2);
	ptr = strstr( str, "YYYY");	
	if( ptr) 
	{
		ItoA( ptr, tp->tm_year + 1900, 4);
	}
	else 
	{
		ptr = strstr( str, "YY"); 
		if( ptr)	ItoA( ptr, tp->tm_year % 100, 2);
	}

	return 1;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
/* convert time string to time_t - YYYY/MM/DD-hh:mm:ss */
time_t StoT( char *tstr)
{
	int			len = 19, stat = 0, i;
	char 		*token = "/:- ";
	char		str[ 512];
	char		*ptr;
	struct tm	tm_buf;
	time_t		rtn_time;

	memcpy( str, tstr, len);
	str[ len] = 0;

	memset( &tm_buf, 0x00, sizeof( struct tm));
	ptr = strtok( str, token);
	while( ptr != NULL)
	{
		i = atoi( ptr);
		switch( stat)
		{
			case 0:
				if( i < 1900) return 0;
				tm_buf.tm_year = i -1900;
				break;
			case 1:
				if( i <= 0) i = 1;
				tm_buf.tm_mon = i -1;
				break;
			case 2:
				if( i <= 0) i = 1;
				tm_buf.tm_mday = i;
				break;
			case 3:
				tm_buf.tm_hour = atoi( ptr);
				break;
			case 4:
				tm_buf.tm_min = atoi( ptr);
				break;
			case 5:
				tm_buf.tm_sec = atoi( ptr);
				break;
				
		}
		ptr = strtok( NULL, token);
		stat++;
	}

	rtn_time = mktime( &tm_buf);

	return rtn_time;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert time_t to YYMMDD
***************************************************************************** */
int	TtoD( time_t cur_time)
{
	int			int_day = 0;  /* YYYYMMDD */
	struct tm	*tp;

	tp = localtime( &cur_time);

	int_day  = ( tp->tm_year + 1900) * 10000;
	int_day += ( tp->tm_mon + 1) * 100;
	int_day += tp->tm_mday;

	return int_day;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert time_t to HHMMSS
***************************************************************************** */
int	TtoI( time_t cur_time)
{
	int			int_time = 0;  /* HHMMSS */
	struct tm	*tp;

	tp = localtime( &cur_time);

	int_time  = tp->tm_hour * 10000;
	int_time += tp->tm_min * 100;
	int_time += tp->tm_sec;

	return int_time;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
char *GetTimeStr()
{
	static char	time_str[ 32];
	time_t		cur_time;
	struct tm	*tp;

	time( &cur_time);
	tp = localtime( &cur_time);

	sprintf( time_str, "%04d/%02d/%02d-%02d:%02d:%02d", 
			tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);

	return time_str;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
unsigned int StoI( char *str)
{
	long long		rtn;

	if( !memcmp( str, "0x", 2)) 			rtn = strtoll( str, NULL, 16);
	else if( !memcmp( str, "0X", 2)) 		rtn = strtoll( str, NULL, 16);
	else if( !memcmp( str, "0",  1)) 		rtn = strtoll( str, NULL, 8);
	else									rtn = strtoll( str, NULL, 0);
	return ( unsigned int)rtn;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int _Max( int a, int b)
{
	return ((a)>(b)) ? (a):(b);
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int _Min( int a, int b)
{
	return ((a)<(b)) ? (a):(b);
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int TrimR( char *rec)
{
	int		pos;

	pos = strlen( rec) -1;
	if( pos < 0) return pos;

	while( rec[ pos] != 0)
	{
		switch( rec[ pos])
		{
			case ' ':		/* space */
			case '\t':		/* tab */
			case '\n':		/* line feed */
			case '\r':		/* carriage return */
			case '\v':		/* vertical tab */
			case '\f':		/* feed */
				rec[ pos] = 0;
				break;
			default:
				return pos +1;
		}
		pos--;
		if( pos < 0) return 0;
	}
	return 0;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int TrimNR( char *rec, int len)
{
	int		pos = 0;

	pos = len -1;
	while( pos >= 0)
	{
		switch( rec[ pos])
		{
			case 0x00:
			case ' ' :		/* space */
			case '\t':		/* tab */
			case '\n':		/* line feed */
			case '\r':		/* carriage return */
			case '\v':		/* vertical tab */
			case '\f':		/* feed */
				rec[ pos] = 0;
				break;
			default:
				return pos +1;
		}
		pos--;
	}

	return 0;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int TrimN( char *rec, int len)
{
	int		pos = 0;

	while( pos < len)
	{
		switch( rec[ pos])
		{
			case ' ':		/* space */
			case '\t':		/* tab */
			case '\n':		/* line feed */
			case '\r':		/* carriage return */
			case '\v':		/* vertical tab */
			case '\f':		/* feed */
				rec[ pos] = 0;
				break;
			default:
				break;
		}
		pos++;
	}

	return len;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int StrSetN( char *rec, int org, int dst, int len)
{
	int		pos = 0;

	while( pos < len)
	{
		if( rec[ pos] == org) rec[ pos] = dst;
		pos++;
	}

	return len;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int IsHanGul( unsigned char *data, int pos)
{
	int				cnt = 0;
	unsigned char 	c;

	while( 1)
	{
		c = data[ pos];

		if( c < 0x80) 
		{
			if( cnt == 0)	return 0;
			if( cnt % 2)	return 1;
			else			return 2;
		}
		cnt++;
		pos--;
	}
}

/** ***************************************************************************
**  @fn         int GetPrevWeekDay( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패 종료
**  @retval     양수 성공 종료
**  @exception
**  @remark
**  @brief
**  전일 날짜를 (YYYYMMDD) 형식으로 산출
**  전일 - 토요일,일요일 제외 공휴일 포함
***************************************************************************** */
char *GetPrevWeekDay( char *date)
{
	time_t		cur_time;
	struct tm	*tp;

	time( &cur_time);
	tp = localtime( &cur_time);

	switch( tp->tm_wday)
	{
		/* 어제 */
		default:
			cur_time -= ( 3600 * 24 );
			break;
		/* 그제 */
		case 0:
			cur_time -= ( 3600 * 24 * 2);
			break;
		/* 3일 전 */
		case 1:
			cur_time -= ( 3600 * 24 * 3);
			break;
	}
	tp = localtime( &cur_time);

	sprintf( date, "%04d%02d%02d", tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday);

	return date;
}

/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/


/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/
int				Continue = 1;
extern SHM_NOTE	*Shm_Note;

/** ***************************************************************************
**  @fu         int Blp_Create()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - blp pointer
**  @retval     실패    - NULL
**  @brief
**  채권 시장조성에 필요한 ipc를 생성
**	shared memory, semaphore 생성
***************************************************************************** */
BLP* Blp_CreateForce( key_t blp_key)
{
	BLP		*blp;

	blp = malloc( sizeof( BLP));
	if( blp == NULL)
	{
		LogErr( "malloc error.");
	}
	memset( blp, 0x00, sizeof( BLP));
	blp->key = blp_key;

	blp->mem = Mem_Create( blp->key, sizeof( BLP_MAP));
	if( blp->mem == NULL)
	{
		LogLib( "Mem_Create error.");
	}
	else
	{
		blp->map = Mem_GetPtr( blp->mem);
	}

	blp->sem = Sem_Create( blp->key);
	if( blp->sem == NULL)
	{
		LogLib( "Sem_Create error.");
		goto error;
	}

	Blp_ShmInit( blp);

	return blp;

	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int Blp_Create()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - blp pointer
**  @retval     실패    - NULL
**  @brief
**  채권시장조성에 필요한 ipc를 생성
**	shared memory, semaphore 생성
***************************************************************************** */
BLP* Blp_Create( key_t blp_key)
{
	BLP		*blp;

	blp = malloc( sizeof( BLP));
	if( blp == NULL)
	{
		LogErr( "malloc error.");
		goto error;
	}
	memset( blp, 0x00, sizeof( BLP));
	blp->key = blp_key;

	blp->mem = Mem_Create( blp->key, sizeof( BLP_MAP));
	if( blp->mem == NULL)
	{
		LogLib( "Mem_Create error.");
		goto error_1;
	}
	blp->map = Mem_GetPtr( blp->mem);
	blp->map->stat.key = blp->key;

	blp->sem = Sem_Create( blp->key);
	if( blp->sem == NULL)
	{
		LogLib( "Sem_Create error.");
		goto error_2;
	}

	Blp_ShmInit( blp);

	return blp;

	error_2:
		if( blp->sem != NULL) Sem_Remove( blp->sem);
		Mem_Remove( blp->mem);
	error_1:
		free( blp);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int Blp_Remove( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  채권시장조성에서 생성한 ipc를 삭제
**	shared memory, semaphore 삭제
***************************************************************************** */
int Blp_Remove( BLP *blp)
{
	int		rtn;

	rtn = Sem_Remove( blp->sem);
	if( rtn < 0)
	{
		LogLib( "Sem_Remove error. sem=[%p] key=[0x%08x]", blp->sem, blp->key);
		return -1;
	}

	rtn = Mem_Remove( blp->mem);
	if( rtn < 0)
	{
		LogLib( "Mem_Remove error. mem=[%p] key=[0x%08x]", blp->mem, blp->key);
		return -1;
	}

	free( blp);

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Open()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - blp pointer
**  @retval     실패    - NULL
**  @brief
**  채권시장조성 Open
***************************************************************************** */
BLP* Blp_Open( key_t blp_key, int service)
{
	BLP		*blp;

	blp = malloc( sizeof( BLP));
	if( blp == NULL)
	{
		LogErr( "malloc error.");
		goto error;
	}
	memset( blp, 0x00, sizeof( BLP));
	blp->key = blp_key;

	blp->mem = Mem_Open( blp->key);
	if( blp->mem == NULL)
	{
		LogLib( "Mem_Open error.");
		goto error_1;
	}
	blp->map = Mem_GetPtr( blp->mem);
	LogDel( "attach shared memory ... ptr=[%p]", blp->map);

	while( Continue && service == 0)
	{
		if( blp->map->stat.service) break;

		LogWar( "채권 시장조성 서비스 준비중 ... blp->map->stat.service=[%d]", blp->map->stat.service);
		sleep( 1);
	}

	blp->sem = Sem_Open( blp->key);
	if( blp->sem == NULL)
	{
		LogLib( "Sem_Open error.");
		goto error_2;
	}

	return blp;

	error_2:
		if( blp->sem != NULL) Sem_Close( blp->sem);
		Mem_Close( blp->mem);
	error_1:
		free( blp);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  채권시장조성 Close
***************************************************************************** */
int Blp_Close( BLP *blp)
{
	int		rtn;

	rtn = Sem_Close( blp->sem);
	if( rtn < 0)
	{
		LogLib( "Sem_Close error. sem=[%p] key=[0x%08x]", blp->sem, blp->key);
		return -1;
	}

	rtn = Mem_Close( blp->mem);
	if( rtn < 0)
	{
		LogLib( "Mem_Close error. mem=[%p] key=[0x%08x]", blp->mem, blp->key);
		return -1;
	}

	free( blp);

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  채권시장조성 Close
***************************************************************************** */
int Blp_ShmInit( BLP *blp)
{
	BLP_MAP		*map = blp->map;

	map->stat.key = blp->key;

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  semaphore lock
***************************************************************************** */
int Blp_Lock( BLP *blp)
{
	Sem_Lock( blp->sem);
	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  semaphore lock
***************************************************************************** */
int Blp_Unlock( BLP *blp)
{
	Sem_Unlock( blp->sem);

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 테이블 번호
**  @retval     실패    - -1
**  @brief
**  종목코드로 찾기
***************************************************************************** */
int Blp_GetItem( BLP *blp, char *item_code)
{
	int				i;
	int				cnt;
	BLP_HOGA_TBL	*tbl = &blp->map->tbl[ 0];

	cnt = tbl->id;	/* 등록된 종목 갯수 */
	LogDbg( "cnt=[%d]", cnt);

	for( i = 1; i <= cnt; i++)
	{
		tbl = &blp->map->tbl[ i];
		LogDbg( "mem[%.12s] arg[%.12s]", tbl->item_code, item_code);
		if( !memcmp( tbl->item_code, item_code, 12)) return i;
	}

	return -1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 호가 record 번호
**  @retval     실패    - -1
**  @brief
**  가격으로 찾기
***************************************************************************** */
int Blp_GetPrice( BLP *blp, BLP_HOGA_TBL *tbl, double price)
{
	int				i;
	int				cnt;
	BLP_HOGA_REC	*rec;

	for( i = 0; i < BLP_MAX_HOGA; i++)
	{
		rec = &tbl->rec[ i];
		if( price == rec->price) return i;
	}

	return -1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  종목코드 등록
***************************************************************************** */
int Blp_AddItem( BLP *blp, char *item_code)
{
	int				pos;
	BLP_HOGA_TBL	*tbl = &blp->map->tbl[ 0];

	pos = Blp_GetItem( blp, item_code);
	if( pos > 0)
	{
		LogMsg( "이미 등록된 종목 코드 입니다. item_code=[%.12s]", item_code);
		return 0;
	}
	LogDbg( "pos=[%d]", pos);

	Blp_Lock( blp);
	tbl->id++;
	pos = tbl->id;
	if( pos >= BLP_MAX_TBL)
	{
		LogCri( "더이상 등록할 TABLE 영역이 없습니다.");
		goto error;
	}
	Blp_Unlock( blp);

	tbl = &blp->map->tbl[ pos];
	memcpy( tbl->item_code, item_code, sizeof( tbl->item_code));
	tbl->id = pos;
	// tbl->tick = 1.00;

	return pos;

	error:
		Blp_Unlock( blp);
		return -1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  채권시장조성 Close
***************************************************************************** */
int Blp_Process( BLP *blp)
{
	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  거래소 시세 처리
***************************************************************************** */
int Blp_ProcessSise( BLP *blp, char *krx_sise)
{
	int				rtn;
	int				pos;
	int				task = 0;	/* 0:호가 1:체결 */
	CO_G701K		*g701k = ( CO_G701K *)krx_sise;
	BLP_HOGA_TBL	*tbl;
	BLP_SISE		*sise;

	/***********************************/
	/* LP 종목인지 check               */
	/***********************************/
	pos = Blp_GetItem( blp, g701k->item_code);
	if( pos <= 0)
	{
		LogMsg( "종목이 없습니다. item_code=[%.12s]", g701k->item_code);
		return 0;
	}
	LogDbg( "pos=[%d]", pos);
	tbl  = &blp->map->tbl[ pos];
	sise = &tbl->sise;

	/***********************************/
	/* 시세 convert                    */
	/***********************************/
	if( !memcmp( krx_sise, "G701K", 5)) 
	{ 
		LogDbg( "시세 체결 처리");
		rtn = Blp_ConvertExe( blp, sise, krx_sise);		
		task = 1; 
	}
	else
	if( !memcmp( krx_sise, "B601K", 5)) 
	{ 
		LogDbg( "시세 호가 처리");
		rtn = Blp_ConvertHoga( blp, sise, krx_sise);
		task = 0; 
	}
	else
	{
		LogCri( "시세 데이타 오류. sise=[%.5s]", krx_sise);
		rtn = -1;
	}

	/***********************************/
	/* 시장 조성 호가 조건 검사        */
	/***********************************/
	if( ( sise->ask[ 0][ 0] - sise->bid[ 0][ 0]) > tbl->arg.sb31_sped_prc[ 0])
	{
		LogMsg( "시장조성 조건이 맞지 않습니다.");
		LogMsg( "    ask1[%10.2f] - bid1[%10.2f] > sped1_prc[%10.2f]",
				sise->ask[ 0][ 0], sise->bid[ 0][ 0], tbl->arg.sb31_sped_prc[ 0]);
		/* 참고 - LP가 들어가 있는경우는 시장조건은 항상 참 이기 때문에 주문 check 불필요 (cancel process) */
		return 0;
	}

	/***********************************/
	/* pre set                         */
	/***********************************/
	tbl->proc_cnt++;
	tbl->seq_no++;		/* 시세 순번 = space  단순 1 증가 */
	sise->exe_cnt++;
	memcpy( tbl->board_id, g701k->board_id, sizeof( tbl->board_id));

	tbl->ask_base = ( BLP_MAX_HOGA -1) / 2 -1;		/* 매도 중간 position을 구함 - 매도/매수 중간값 자리 비워 놓음 */
	tbl->bid_base = ( BLP_MAX_HOGA +1) / 2;			/* 매수 중간 position을 구함 */

	/***********************************/
	/* HOGA TABLE 구성                 */
	/***********************************/
	memcpy( &tbl->old, &tbl->rec, sizeof( BLP_HOGA_REC) * BLP_MAX_HOGA);
	memset( &tbl->rec[ 0], 0x00, sizeof( BLP_HOGA_REC) * BLP_MAX_HOGA);
	Blp_MakeHogaTbl( blp, tbl, sise);

	return rtn;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  거래소 체결 시세 처리
***************************************************************************** */
int Blp_ConvertExe( BLP *blp, BLP_SISE *sise, CO_G701K *g701k)
{
	int				int_time, ms_time;
	int				wait;

	/***********************************/
	/* 시간 covert                     */
	/***********************************/
	memcpy( &sise->tv_old, &sise->tv, sizeof( struct timeval));
	int_time = AtoI( g701k->trade_time, 6);
	ms_time  = AtoI( &g701k->trade_time[ 6], 6);
	gettimeofday( &sise->tv, NULL);
	sise->tv.tv_sec = sise->tv.tv_sec - sise->tv.tv_sec % 86400 - 32400;	/* 하루 시작 */
	sise->tv.tv_sec = sise->tv.tv_sec + ( int_time / 10000) * 3600 + (( int_time % 10000) / 100) * 60 + int_time % 100;
	sise->tv.tv_usec = ms_time;
	wait = ( sise->tv.tv_sec - sise->tv_old.tv_sec) * 1000000 + sise->tv.tv_usec - sise->tv_old.tv_usec;
	LogDbg( "usleep[%d]", wait);
	if( wait > 0 && wait < 10000000) usleep( wait / 10);


	/***********************************/
	/* 체결 convert                    */
	/***********************************/
	sise->price = AtoD( g701k->crprc, sizeof( g701k->crprc));
	sise->price = AtoD( g701k->volume, sizeof( g701k->volume));

	/***********************************/
	/* 호가 convert                    */
	/***********************************/
	sise->ask[ 0][ 0] = AtoD( g701k->ask1_price, sizeof( g701k->ask1_price));
	sise->ask[ 0][ 1] = AtoD( g701k->ask2_price, sizeof( g701k->ask2_price));
	sise->ask[ 0][ 2] = AtoD( g701k->ask3_price, sizeof( g701k->ask3_price));
	sise->ask[ 0][ 3] = AtoD( g701k->ask4_price, sizeof( g701k->ask4_price));
	sise->ask[ 0][ 4] = AtoD( g701k->ask5_price, sizeof( g701k->ask5_price));
	sise->bid[ 0][ 0] = AtoD( g701k->bid1_price, sizeof( g701k->bid1_price));
	sise->bid[ 0][ 1] = AtoD( g701k->bid2_price, sizeof( g701k->bid2_price));
	sise->bid[ 0][ 2] = AtoD( g701k->bid3_price, sizeof( g701k->bid3_price));
	sise->bid[ 0][ 3] = AtoD( g701k->bid4_price, sizeof( g701k->bid4_price));
	sise->bid[ 0][ 4] = AtoD( g701k->bid5_price, sizeof( g701k->bid5_price));

	sise->ask[ 1][ 0] = AtoD( g701k->bond_ask1_remain_vol, sizeof( g701k->bond_ask1_remain_vol));
	sise->ask[ 1][ 1] = AtoD( g701k->bond_ask2_remain_vol, sizeof( g701k->bond_ask2_remain_vol));
	sise->ask[ 1][ 2] = AtoD( g701k->bond_ask3_remain_vol, sizeof( g701k->bond_ask3_remain_vol));
	sise->ask[ 1][ 3] = AtoD( g701k->bond_ask4_remain_vol, sizeof( g701k->bond_ask4_remain_vol));
	sise->ask[ 1][ 4] = AtoD( g701k->bond_ask5_remain_vol, sizeof( g701k->bond_ask5_remain_vol));
	sise->bid[ 1][ 0] = AtoD( g701k->bond_bid1_remain_vol, sizeof( g701k->bond_bid1_remain_vol));
	sise->bid[ 1][ 1] = AtoD( g701k->bond_bid2_remain_vol, sizeof( g701k->bond_bid2_remain_vol));
	sise->bid[ 1][ 2] = AtoD( g701k->bond_bid3_remain_vol, sizeof( g701k->bond_bid3_remain_vol));
	sise->bid[ 1][ 3] = AtoD( g701k->bond_bid4_remain_vol, sizeof( g701k->bond_bid4_remain_vol));
	sise->bid[ 1][ 4] = AtoD( g701k->bond_bid5_remain_vol, sizeof( g701k->bond_bid5_remain_vol));

	return sizeof( CO_G701K);
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  거래소 호가 시세 처리
***************************************************************************** */
int Blp_ConvertHoga( BLP *blp, BLP_SISE *sise, CO_B601K *b601k)
{
	int				int_time, ms_time;
	int				wait;

	/***********************************/
	/* 시간 covert                     */
	/***********************************/
	memcpy( &sise->tv_old, &sise->tv, sizeof( struct timeval));
	int_time = AtoI( b601k->trade_time, 6);
	ms_time  = AtoI( &b601k->trade_time[ 6], 6);
	gettimeofday( &sise->tv, NULL);
	sise->tv.tv_sec = sise->tv.tv_sec - sise->tv.tv_sec % 86400 - 32400;	/* 하루 시작 */
	sise->tv.tv_sec = sise->tv.tv_sec + ( int_time / 10000) * 3600 + (( int_time % 10000) / 100) * 60 + int_time % 100;
	sise->tv.tv_usec = ms_time;
	wait = ( sise->tv.tv_sec - sise->tv_old.tv_sec) * 1000000 + sise->tv.tv_usec - sise->tv_old.tv_usec;
	LogDbg( "usleep[%d]", wait);
	if( wait > 0 && wait < 10000000) usleep( wait / 10);

	/***********************************/
	/* hoga convert                    */
	/***********************************/
	sise->ask[ 0][ 0] = AtoD( b601k->ask1_price, sizeof( b601k->ask1_price));
	sise->ask[ 0][ 1] = AtoD( b601k->ask2_price, sizeof( b601k->ask2_price));
	sise->ask[ 0][ 2] = AtoD( b601k->ask3_price, sizeof( b601k->ask3_price));
	sise->ask[ 0][ 3] = AtoD( b601k->ask4_price, sizeof( b601k->ask4_price));
	sise->ask[ 0][ 4] = AtoD( b601k->ask5_price, sizeof( b601k->ask5_price));
	sise->bid[ 0][ 0] = AtoD( b601k->bid1_price, sizeof( b601k->bid1_price));
	sise->bid[ 0][ 1] = AtoD( b601k->bid2_price, sizeof( b601k->bid2_price));
	sise->bid[ 0][ 2] = AtoD( b601k->bid3_price, sizeof( b601k->bid3_price));
	sise->bid[ 0][ 3] = AtoD( b601k->bid4_price, sizeof( b601k->bid4_price));
	sise->bid[ 0][ 4] = AtoD( b601k->bid5_price, sizeof( b601k->bid5_price));

	sise->ask[ 1][ 0] = AtoD( b601k->bond_ask1_remain_vol, sizeof( b601k->bond_ask1_remain_vol));
	sise->ask[ 1][ 1] = AtoD( b601k->bond_ask2_remain_vol, sizeof( b601k->bond_ask2_remain_vol));
	sise->ask[ 1][ 2] = AtoD( b601k->bond_ask3_remain_vol, sizeof( b601k->bond_ask3_remain_vol));
	sise->ask[ 1][ 3] = AtoD( b601k->bond_ask4_remain_vol, sizeof( b601k->bond_ask4_remain_vol));
	sise->ask[ 1][ 4] = AtoD( b601k->bond_ask5_remain_vol, sizeof( b601k->bond_ask5_remain_vol));
	sise->bid[ 1][ 0] = AtoD( b601k->bond_bid1_remain_vol, sizeof( b601k->bond_bid1_remain_vol));
	sise->bid[ 1][ 1] = AtoD( b601k->bond_bid2_remain_vol, sizeof( b601k->bond_bid2_remain_vol));
	sise->bid[ 1][ 2] = AtoD( b601k->bond_bid3_remain_vol, sizeof( b601k->bond_bid3_remain_vol));
	sise->bid[ 1][ 3] = AtoD( b601k->bond_bid4_remain_vol, sizeof( b601k->bond_bid4_remain_vol));
	sise->bid[ 1][ 4] = AtoD( b601k->bond_bid5_remain_vol, sizeof( b601k->bond_bid5_remain_vol));

	return sizeof( CO_B601K);
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  호가 테이블 구성
***************************************************************************** */
int Blp_MakeHogaTbl( BLP *blp, BLP_HOGA_TBL *tbl, BLP_SISE *sise)
{
	int				pos;		/* rec 호가 위치 */
	int				no;			/* sise 호가 위치 */
	int				ho;			/* 호가순번 0:매도1호가 1:매도2호가 -1:매수1호가 */
	double			price;
	BLP_HOGA_REC	*rec, *old;;

	LogDbg( "start ....... ");

	/* 매도/매수 중간값 계산 */
	rec = &tbl->rec[ tbl->ask_base +1];
	rec->price = ( sise->ask[ 0][ 0] + sise->bid[ 0][ 0]) / 2;


	ho = 1;
	no = 0;
	price = sise->ask[ 0][ 0];	/* 매도 1호가 */
	for( pos = tbl->ask_base; pos >= BLP_SISE_HOGA; pos--)					/* 매도 set */
	{
		rec = &tbl->rec[ pos];
		old = &tbl->old[ pos];
		rec->ab = 1;
		rec->no = pos;
		rec->ho = ho++;
		rec->price = price;
		if( price == sise->ask[ 0][ no] && no < BLP_SISE_HOGA)				/* 시세 호가에 수량 set  */
		{
			rec->s_ho = no +1;
			rec->volume = sise->ask[ 1][ no];
			no++;
		}

		if( rec->price != old->price)				 						/* 호가갭 계산 */
		{
			rec->gap = -( int)(( old->price - rec->price) / tbl->arg.sb31_tick_unit);
			LogDbg( "ask old=[%10.2f] new=[%10.2f] rec->gap=[%d] vol=[%10.0f]", old->price, rec->price, rec->gap, rec->gap_vol);
		}

		if( rec->volume != old->volume)										/* 잔량 계산 */
		{
			rec->gap_vol = rec->volume - old->volume;
		}

		price += tbl->arg.sb31_tick_unit;
	}
	if( no < BLP_SISE_HOGA)	/* 호가 TABLE 범위안에 시세호가가 없을시 */
	{
		for( pos = BLP_SISE_HOGA -1; pos >= 0; pos--)
		{
			rec = &tbl->rec[ pos];
			rec->ab = 1;
			rec->ho = 0;
			rec->s_ho = no +1;
			rec->no = 0;
			rec->price = sise->ask[ 0][ no];
			rec->volume = sise->ask[ 1][ no];
			no++;
			if( no >= BLP_SISE_HOGA) break;
		}
	}

	ho = -1;
	no = 0;
	price = sise->ask[ 0][ 0];	/* 매도 1호가 - 매수/매도 사이에 갭이 있을 수 있으므로 매도1호가 기준 */
	for( pos = tbl->bid_base; pos < BLP_MAX_HOGA - BLP_SISE_HOGA; pos++)		/* 매수 set */
	{
		price -= tbl->arg.sb31_tick_unit;

		rec = &tbl->rec[ pos];
		old = &tbl->old[ pos];
		rec->ab = 2;
		rec->no = pos;
		rec->ho = ho--;
		rec->price = price;
		if( price == sise->bid[ 0][ no] && no < BLP_SISE_HOGA)
		{
			rec->s_ho = no +1;
			rec->volume = sise->bid[ 1][ no];
			no++;
		}

		if( rec->price != old->price)				 						/* 호가갭 계산 */
		{
			rec->gap = -( int)(( old->price - rec->price) / tbl->arg.sb31_tick_unit);
			LogDbg( "bid old=[%10.2f] new=[%10.2f] rec->gap=[%d]", old->price, rec->price, rec->gap);
		}

		if( rec->volume != old->volume)										/* 잔량 계산 */
		{
			rec->gap_vol = rec->volume - old->volume;
		}

	}
	LogDbg( "bid no=[%d]", no);
	if( no < BLP_SISE_HOGA)	/* 호가 TABLE 범위안에 시세호가가 없을시 */
	{
		for( pos = BLP_MAX_HOGA - BLP_SISE_HOGA; pos < BLP_MAX_HOGA; pos++)
		{
			rec = &tbl->rec[ pos];
			rec->ab = 2;
			rec->ho = 0;
			rec->s_ho = no +1;
			rec->no = 0;
			rec->price = sise->bid[ 0][ no];
			rec->volume = sise->bid[ 1][ no];
			no++;
			if( no >= BLP_SISE_HOGA) break;
		}
	}

	/* check order price */
	Blp_CheckOrder( blp, tbl, sise);

	return pos;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  기준 가격에 대한 유효 호가 설정 
***************************************************************************** */
int Blp_CheckOrder( BLP *blp, BLP_HOGA_TBL *tbl, BLP_SISE *sise)
{
	double	std_prc;
	double	ask_prc;
	double	bid_prc;
	double	diff_prc;
	int		i;
	int		ask_pos, bid_pos;
	int		ask_old, bid_old;
	BLP_ORD	*order;
	BLP_ARG	*arg;

	arg = &tbl->arg;

	for( i = 0; i < BLP_MAX_LP; i++)
	{
		order = &tbl->order[ i];
		std_prc = ( sise->ask[ 0][ 0] + sise->bid[ 0][ 0]) / 2.0;
		ask_prc = std_prc + ( tbl->arg.sb31_sped_prc[ i] / 2.0);
		bid_prc = std_prc - ( tbl->arg.sb31_sped_prc[ i] / 2.0);

		if( tbl->arg.sb31_tick_unit == 0.5)
		{
			diff_prc = ask_prc - floor( ask_prc);
			if( diff_prc >= 0.75)	ask_prc = round( ask_prc);
			else
			if( diff_prc >= 0.5 )	ask_prc = round( ask_prc) - 0.5;
			else
			if( diff_prc >= 0.25 )	ask_prc = round( ask_prc) + 0.5;
			else
									ask_prc = round( ask_prc);

			diff_prc = bid_prc - floor( bid_prc);
			if( diff_prc >= 0.75)	bid_prc = round( bid_prc);
			else
			if( diff_prc >= 0.5 )	bid_prc = round( bid_prc) - 0.5;
			else
			if( diff_prc >= 0.25 )	bid_prc = round( bid_prc) + 0.5;
			else
									bid_prc = round( bid_prc);
		}
		else
		if( tbl->arg.sb31_tick_unit == 1.0)
		{
			ask_prc = round( ask_prc);
			bid_prc = round( bid_prc);
		}
		else
		{
			ask_prc = round( ask_prc * 10.0) / 10.0;
			bid_prc = round( bid_prc * 10.0) / 10.0;
		}

		LogDbg( "std_prc[ %d]=[%f]", i, std_prc);
		LogDbg( "ask_prc[ %d]=[%f]", i, ask_prc);
		LogDbg( "bid_prc[ %d]=[%f]", i, bid_prc);

		ask_pos = Blp_GetPrice( blp, tbl, ask_prc);
		bid_pos = Blp_GetPrice( blp, tbl, bid_prc);
		LogDbg( "ask_pos[ %d]=[%d]", i, ask_pos);
		LogDbg( "bid_pos[ %d]=[%d]", i, bid_pos);

		if( order->ask_prc == 0.0)			/* 신규 */
		{
			order->ask_prc = ask_prc;
			order->bid_prc = bid_prc;
			tbl->rec[ ask_pos].ord_no = i +1;
			tbl->rec[ bid_pos].ord_no = i +1;
			order->ask_vol = arg->sb31_ord_qanty[ i] * arg->sb31_ord_qanty_unit;
			order->bid_vol = arg->sb31_ord_qanty[ i] * arg->sb31_ord_qanty_unit;
			LogDbg( "신규주문 [%d]", i);
			LogDbg( "    가격 [%10.2f %10.2f]", order->ask_prc, order->bid_prc);
			LogDbg( "    수량 [%10.0f %10.0f]", order->ask_vol, order->bid_vol);
		}
		else
		if( order->ask_prc != ask_prc)		/* 정정 */
		{
			LogDbg( "정정주문 [%d]", i);
			LogDbg( "    가격 [%10.2f %10.2f] -> [%10.2f %10.2f]", order->ask_prc, order->bid_prc, ask_prc, bid_prc);
			LogDbg( "    수량 [%10.0f/%10.0f]", order->ask_vol, order->bid_vol);
			ask_old = Blp_GetPrice( blp, tbl, order->ask_prc);
			bid_old = Blp_GetPrice( blp, tbl, order->bid_prc);
			tbl->rec[ ask_old].ord_no = 0;
			tbl->rec[ bid_old].ord_no = 0;
			tbl->rec[ ask_pos].ord_no = i +1;
			tbl->rec[ bid_pos].ord_no = i +1;
			order->ask_prc = ask_prc;
			order->bid_prc = bid_prc;
			order->ask_vol = arg->sb31_ord_qanty[ i] * arg->sb31_ord_qanty_unit;
			order->bid_vol = arg->sb31_ord_qanty[ i] * arg->sb31_ord_qanty_unit;
		}
		else								/* 유지 */
		{
			LogDbg( "주문유지 [%d]", i);
			tbl->rec[ ask_pos].ord_no = i +1;
			tbl->rec[ bid_pos].ord_no = i +1;
			LogDbg( "    가격 [%10.2f %10.2f]", order->ask_prc, order->bid_prc);
			LogDbg( "    수량 [%10.0f %10.0f]", order->ask_vol, order->bid_vol);
		}

		LogDbg( "-------------------------------------------------");
	}


	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  채권시장조성 호가 테이블 리스트
***************************************************************************** */
int Blp_List( BLP *blp)
{
	int				i, col = 120;
	BLP_MAP			*map = blp->map;
	BLP_HOGA_TBL	*tbl = &blp->map->tbl[ 0];

	LogRaw( "key           = [0x%08x]\n", map->stat.key);
	LogRaw( "oms_key       = [0x%08x]\n", map->stat.oms_key);
	LogRaw( "count         = [%d]\n", tbl->id);

	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");
	LogRaw( "%-2.2s ", "id");
	LogRaw( "%-6.6s ", "   cnt");
	LogRaw( "%-6.6s ", "   seq");
	LogRaw( "%-2.2s ", "bi");
	LogRaw( "%-12.12s ", "item_code");
	LogRaw( "%-2.2s ", "ab ");
	LogRaw( "%-2.2s ", "bb ");
	LogRaw( "\n");
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");

	for( i = 1; i < BLP_MAX_TBL; i++)
	{
		tbl = &map->tbl[ i];
		LogRaw( "%2d ", tbl->id);
		LogRaw( "%6d ", tbl->proc_cnt);
		LogRaw( "%6d ", tbl->seq_no);
		LogRaw( "%2.2s ", tbl->board_id);
		LogRaw( "%12.12s ", tbl->item_code);
		LogRaw( "%2d ", tbl->ask_base);
		LogRaw( "%2d ", tbl->bid_base);
		LogRaw( "\n");
	}
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");




	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  채권시장조성 공유메모리 상태
***************************************************************************** */
int Blp_Stat( BLP *blp, char *item_code)
{
	int				i, col = 120;
	int				pos;
	BLP_HOGA_TBL	*tbl;
	BLP_HOGA_REC	*rec;

	if( item_code == NULL)	return -1;

	pos = Blp_GetItem( blp, item_code);
	if( pos <= 0)
	{
		LogMsg( "종목이 없습니다. item_code=[%.12s]", item_code);
		return 0;
	}
	LogDbg( "pos=[%d]", pos);

	tbl = &blp->map->tbl[ pos];

	BLP_ARG_Print( &tbl->arg);

	LogMsg( "id         = [%d]", tbl->id);
	LogMsg( "proc_cnt   = [%d]", tbl->proc_cnt);
	LogMsg( "seq_no     = [%d]", tbl->seq_no);
	LogMsg( "board_id   = [%.2s]", tbl->board_id);
	LogMsg( "item_code  = [%.12s]", tbl->item_code);
	LogMsg( "ask_base   = [%2d]", tbl->ask_base);
	LogMsg( "bid_base   = [%2d]", tbl->bid_base);

	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");
	LogRaw( "%-2.2s ", "ab");
	LogRaw( "%-2.2s ", "no");
	LogRaw( "%-3.3s ", "seq");
	LogRaw( "%-2.2s ", " s");
	LogRaw( "%-10.10s ", "  price");
	LogRaw( "%-10.10s ", "  volume");
	LogRaw( "                    ");
	LogRaw( "%-2.2s ", "ab");
	LogRaw( "%-2.2s ", "no");
	LogRaw( "%-3.3s ", "seq");
	LogRaw( "%-2.2s ", " s");
	LogRaw( "%-10.10s ", "  price");
	LogRaw( "%-10.10s ", "  volume");
	LogRaw( "\n");
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");

	for( i = 0; i < BLP_MAX_HOGA; i++)
	{
		rec = &tbl->rec[ i];
		LogRaw( "%2d ", rec->ab);
		LogRaw( "%2d ", rec->no);
		LogRaw( "%3d ", rec->ho);
		if( rec->s_ho == 0)		LogRaw( "   ",  rec->s_ho);
		else					LogRaw( "%2d ", rec->s_ho);
		LogRaw( "%10.2f ", rec->price);
		LogRaw( "%10.0f ", rec->volume);
		LogRaw( "                    ");

		rec = &tbl->old[ i];
		LogRaw( "%2d ", rec->ab);
		LogRaw( "%2d ", rec->no);
		LogRaw( "%3d ", rec->ho);
		if( rec->s_ho == 0)		LogRaw( "   ",  rec->s_ho);
		else					LogRaw( "%2d ", rec->s_ho);
		LogRaw( "%10.2f ", rec->price);
		LogRaw( "%10.0f ", rec->volume);


		LogRaw( "\n");
	}
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  sise receive from file - for test
***************************************************************************** */
int Blp_GetSiseFromFile( BLP *blp, CO_B601K *b601k, char *f_name)
{
	static FILE	*fp = NULL;
	char		*ptr;
	char		rec[ 8192];

	if( fp == NULL)
	{
		fp = fopen( f_name, "r");
		if( fp == NULL)
		{
			LogErr( "fopen error. name=[%s]", f_name);
			return -1;
		}
	}

	ptr = fgets( rec, 8192, fp);
	if( ptr == NULL)
	{
		fclose( fp);
		fp = NULL;
		return 0;
	}
	LogDbg( "rec=[%s]", rec);
	memcpy( b601k, rec, sizeof( CO_B601K));

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  sise receive from file - for test
***************************************************************************** */
int Blp_SetArg( BLP *blp, char *rec)
{
	int 			stat = 0;
	int				le = 1;		/* loop end */
	int				pos;
	BLP_HOGA_TBL	*tbl;
	BLP_ARG			_arg, *arg = &_arg;

	while( Continue && le)
	{
		switch( stat)
		{
            case 0  : memset( arg, 0x00, sizeof( BLP_ARG));                    break;
            case 1  : memcpy( arg->sb31_exe_ymd             , &rec[ 50 ], 9 ); break;     /* 1  실행일자                */ 
            case 3  : memcpy( arg->sb31_proc_stus_dstcd     , &rec[ 63 ], 1 ); break;     /* 3  처리상태구분코드        */ 
            case 4  : memcpy( arg->sb31_start_yms           , &rec[ 64 ], 14); break;     /* 4  시작일시                */ 
            case 5  : memcpy( arg->sb31_end_yms             , &rec[ 78 ], 14); break;     /* 5  종료일시                */ 
            case 6  : memcpy( arg->sb31_lp_start_yms        , &rec[ 92 ], 14); break;     /* 6  유동성공급시작일시      */ 
            case 7  : memcpy( arg->sb31_lp_end_yms          , &rec[ 106], 14); break;     /* 7  유동성공급종료일시      */ 
            case 8  : memcpy( arg->sb31_item_cd             , &rec[ 120], 12); break;     /* 8  종목코드                */ 
            case 9  : memcpy( arg->sb31_mm_item_dstcd       , &rec[ 132], 2 ); break;     /* 9  유동성공급종목구분코드  */ 
            case 17 : memcpy( arg->sb31_dspratio_taget_dstcd, &rec[ 202], 2 ); break;     /* 17 괴리율대상구분코드      */ 
            case 18 : memcpy( arg->sb31_dspratio_dstcd      , &rec[ 204], 1 ); break;     /* 18 KRX괴리율구분코드       */ 
            case 21 : memcpy( arg->sb31_trdr_uno            , &rec[ 227], 5 ); break;     /* 21 거래원번호              */ 
            case 22 : memcpy( arg->sb31_trdr_no             , &rec[ 232], 4 ); break;     /* 22 CMBS트레이더번호        */ 
            case 27 : memcpy( arg->sb31_account_no          , &rec[ 280], 11); break;     /* 27 계좌번호                */ 
            case 2  : arg->sb31_exe_no                = AtoI( &rec[ 58 ], 5 ); break;     /* 2  실행번호                */ 
            case 16 : arg->sb31_ord_qanty_unit        = AtoI( &rec[ 197], 5 ); break;     /* 16 주문수량단위            */ 
            case 10 : arg->sb31_sped_prc[ 0]          = AtoD( &rec[ 134], 11); break;     /* 10 스프래드1가격           */ 
            case 11 : arg->sb31_sped_prc[ 1]          = AtoD( &rec[ 145], 11); break;     /* 11 스프래드2가격           */ 
            case 12 : arg->sb31_sped_prc[ 2]          = AtoD( &rec[ 156], 11); break;     /* 12 스프래드3가격           */ 
            case 13 : arg->sb31_ord_qanty[ 0]         = AtoD( &rec[ 167], 10); break;     /* 13 주문1수량               */ 
            case 14 : arg->sb31_ord_qanty[ 1]         = AtoD( &rec[ 177], 10); break;     /* 14 주문2수량               */ 
            case 15 : arg->sb31_ord_qanty[ 2]         = AtoD( &rec[ 187], 10); break;     /* 15 주문3수량               */ 
            case 19 : arg->sb31_dspratio              = AtoD( &rec[ 205], 11); break;     /* 19 괴리율                  */ 
            case 20 : arg->sb31_tick_unit             = AtoD( &rec[ 216], 11); break;     /* 20 틱단위                  */ 
            case 23 : arg->sb31_prv_yild              = AtoD( &rec[ 236], 11); break;     /* 23 민간평가수익률          */ 
            case 24 : arg->sb31_prv_prc               = AtoD( &rec[ 247], 11); break;     /* 24 민간평가가격            */ 
            case 25 : arg->sb31_clsng_prc             = AtoD( &rec[ 258], 11); break;     /* 25 종가                    */ 
            case 26 : arg->sb31_clsng_yild            = AtoD( &rec[ 269], 11); break;     /* 26 종가수익률              */
			default:
				le = 0;
				break;
		}
		stat++;
	}

	BLP_ARG_Print( arg);

	pos = Blp_GetItem( blp, arg->sb31_item_cd);
	if( pos <= 0)
	{
		LogMsg( "종목이 없습니다. item_code=[%.12s]", arg->sb31_item_cd);
		pos = Blp_AddItem( blp, arg->sb31_item_cd);
		if( pos <= 0)
		{
			LogMsg( "종목을 등록 할 수없습니다. item_code=[%.12s]", arg->sb31_item_cd);
			return 0;
		}
		LogMsg( "종목을 등록 하였습니다. item_code=[%.12s]", arg->sb31_item_cd);
	}
	LogDbg( "pos=[%d]", pos);

	tbl = &blp->map->tbl[ pos];
	memcpy( &tbl->arg, arg, sizeof( BLP_ARG));


	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  util - struct timeval to string
***************************************************************************** */
char* Blp_TvToS( struct timeval *tv)
{
	struct tm	*tp;
	static char str[ 64];

	if( tv == NULL)
	{
		sprintf( str, "%s", "0000/00/00-00:00:00.000000");
		return str;
	}

	tp = localtime( &tv->tv_sec);
	if( tp == NULL)
	{
		sprintf( str, "%s", "0000/00/00-00:00:00.000000");
		return str;
	}

	sprintf( str, "%04d/%02d/%02d-%02d:%02d:%02d.%06d",
			tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday,
			tp->tm_hour, tp->tm_min, tp->tm_sec,
			tv->tv_usec);

	return str;
}


int SHM_NOTE_Print( SHM_NOTE* ptr)
{
    LogRaw( "%s", "----[ SHM_NOTE ]------------------------------------------------------------------------\n");
    LogRaw( "종목수, seq 0에만 사용        total_item_cnt         4    0 = [%d]\n",     ptr->total_item_cnt);
    LogRaw( "보유여부 확인(평가금액용)     check_cnt              4    4 = [%d]\n",     ptr->check_cnt);
    LogRaw( "자동전략기동시 해당종목 +- ? auto_use               4    8 = [%d]\n",     ptr->auto_use);
    LogRaw( "최근 시세내역 30개의 Key      CURR_Arry_Key          4   12 = [%d]\n",     ptr->CURR_Arry_Key);
    LogRaw( "CURR_Arry_Key 직전값기억      Befor_CURR_Arry_Key    4   16 = [%d]\n",     ptr->Befor_CURR_Arry_Key);
    LogRaw( "0:체결(A3/G7), 1:호가(B6)     HogaLastGbn            4   20 = [%d]\n",     ptr->HogaLastGbn);
#if 0
    CO_M401K_Print( ptr->M4);
    CO_A701K_Print( ptr->A7);
    CO_A001_RDS02_Print( ptr->A0);
    CO_A301K_Print( ptr->A3);
    CO_G701K_Print( ptr->G7);
    CO_B601K_Print( ptr->B6);
    CO_G701K_Print( ptr->CURR_Arry);
#endif
    LogRaw( "%s", "------------------------------------------------------------------------[ SHM_NOTE ]----\n");

    return sizeof( SHM_NOTE);
}


int CO_B601K_Print( CO_B601K* ptr)
{
    LogRaw( "%s", "----[ _CO_B601K ]-----------------------------------------------------------------------\n");
    LogRaw( "TR CODE                       tr_gbn                 5    0 = [%.5s]\n",   ptr->tr_gbn);
    LogRaw( "정보분배일련번호              seq_no                 8    5 = [%.8s]\n",   ptr->seq_no);
    LogRaw( "보드ID                        board_id               2   13 = [%.2s]\n",   ptr->board_id);
    LogRaw( "세션ID                        session_id             2   15 = [%.2s]\n",   ptr->session_id);
    LogRaw( "종목코드                      item_code             12   17 = [%.12s]\n",  ptr->item_code);
    LogRaw( "매매처리시각                  trade_time            12   29 = [%.12s]\n",  ptr->trade_time);
    LogRaw( "매도1단계우선호가가격         ask1_price            11   41 = [%.11s]\n",  ptr->ask1_price);
    LogRaw( "매수1단계우선호가가격         bid1_price            11   52 = [%.11s]\n",  ptr->bid1_price);
    LogRaw( "채권매도1단계우선호가잔량     bond_ask1_remain_vol  15   63 = [%.15s]\n",  ptr->bond_ask1_remain_vol);
    LogRaw( "채권매수1단계우선호가잔량     bond_bid1_remain_vol  15   78 = [%.15s]\n",  ptr->bond_bid1_remain_vol);
    LogRaw( "매도1단계우선호가수익률       ask1_yield            13   93 = [%.13s]\n",  ptr->ask1_yield);
    LogRaw( "매수1단계우선호가수익률       bid1_yield            13  106 = [%.13s]\n",  ptr->bid1_yield);
    LogRaw( "매도2단계우선호가가격         ask2_price            11  119 = [%.11s]\n",  ptr->ask2_price);
    LogRaw( "매수2단계우선호가가격         bid2_price            11  130 = [%.11s]\n",  ptr->bid2_price);
    LogRaw( "채권매도2단계우선호가잔량     bond_ask2_remain_vol  15  141 = [%.15s]\n",  ptr->bond_ask2_remain_vol);
    LogRaw( "채권매수2단계우선호가잔량     bond_bid2_remain_vol  15  156 = [%.15s]\n",  ptr->bond_bid2_remain_vol);
    LogRaw( "매도2단계우선호가수익률       ask2_yield            13  171 = [%.13s]\n",  ptr->ask2_yield);
    LogRaw( "매수2단계우선호가수익률       bid2_yield            13  184 = [%.13s]\n",  ptr->bid2_yield);
    LogRaw( "매도3단계우선호가가격         ask3_price            11  197 = [%.11s]\n",  ptr->ask3_price);
    LogRaw( "매수3단계우선호가가격         bid3_price            11  208 = [%.11s]\n",  ptr->bid3_price);
    LogRaw( "채권매도3단계우선호가잔량     bond_ask3_remain_vol  15  219 = [%.15s]\n",  ptr->bond_ask3_remain_vol);
    LogRaw( "채권매수3단계우선호가잔량     bond_bid3_remain_vol  15  234 = [%.15s]\n",  ptr->bond_bid3_remain_vol);
    LogRaw( "매도3단계우선호가수익률       ask3_yield            13  249 = [%.13s]\n",  ptr->ask3_yield);
    LogRaw( "매수3단계우선호가수익률       bid3_yield            13  262 = [%.13s]\n",  ptr->bid3_yield);
    LogRaw( "매도4단계우선호가가격         ask4_price            11  275 = [%.11s]\n",  ptr->ask4_price);
    LogRaw( "매수4단계우선호가가격         bid4_price            11  286 = [%.11s]\n",  ptr->bid4_price);
    LogRaw( "채권매도4단계우선호가잔량     bond_ask4_remain_vol  15  297 = [%.15s]\n",  ptr->bond_ask4_remain_vol);
    LogRaw( "채권매수4단계우선호가잔량     bond_bid4_remain_vol  15  312 = [%.15s]\n",  ptr->bond_bid4_remain_vol);
    LogRaw( "매도4단계우선호가수익률       ask4_yield            13  327 = [%.13s]\n",  ptr->ask4_yield);
    LogRaw( "매수4단계우선호가수익률       bid4_yield            13  340 = [%.13s]\n",  ptr->bid4_yield);
    LogRaw( "매도5단계우선호가가격         ask5_price            11  353 = [%.11s]\n",  ptr->ask5_price);
    LogRaw( "매수5단계우선호가가격         bid5_price            11  364 = [%.11s]\n",  ptr->bid5_price);
    LogRaw( "채권매도5단계우선호가잔량     bond_ask5_remain_vol  15  375 = [%.15s]\n",  ptr->bond_ask5_remain_vol);
    LogRaw( "채권매수5단계우선호가잔량     bond_bid5_remain_vol  15  390 = [%.15s]\n",  ptr->bond_bid5_remain_vol);
    LogRaw( "매도5단계우선호가수익률       ask5_yield            13  405 = [%.13s]\n",  ptr->ask5_yield);
    LogRaw( "매수5단계우선호가수익률       bid5_yield            13  418 = [%.13s]\n",  ptr->bid5_yield);
    LogRaw( "채권매도호가총잔량            bond_total_ask_vol    15  431 = [%.15s]\n",  ptr->bond_total_ask_vol);
    LogRaw( "채권매수호가총잔량            bond_total_bid_vol    15  446 = [%.15s]\n",  ptr->bond_total_bid_vol);
    LogRaw( "정보분배메세지종료키워드      msg_end_key            1  461 = [%.1s]\n",   ptr->msg_end_key);
    LogRaw( "%s", "-----------------------------------------------------------------------[ _CO_B601K ]----\n");

    return sizeof( CO_B601K);
}

int CO_B601K_PrintFile( CO_B601K* ptr, FILE *fp)
{
    fprintf( fp, "%s", "----[ _CO_B601K ]-----------------------------------------------------------------------\n");
    fprintf( fp, "TR CODE                       tr_gbn                 5    0 = [%.5s]\n",   ptr->tr_gbn);
    fprintf( fp, "정보분배일련번호              seq_no                 8    5 = [%.8s]\n",   ptr->seq_no);
    fprintf( fp, "보드ID                        board_id               2   13 = [%.2s]\n",   ptr->board_id);
    fprintf( fp, "세션ID                        session_id             2   15 = [%.2s]\n",   ptr->session_id);
    fprintf( fp, "종목코드                      item_code             12   17 = [%.12s]\n",  ptr->item_code);
    fprintf( fp, "매매처리시각                  trade_time            12   29 = [%.12s]\n",  ptr->trade_time);
    fprintf( fp, "매도1단계우선호가가격         ask1_price            11   41 = [%.11s]\n",  ptr->ask1_price);
    fprintf( fp, "매수1단계우선호가가격         bid1_price            11   52 = [%.11s]\n",  ptr->bid1_price);
    fprintf( fp, "채권매도1단계우선호가잔량     bond_ask1_remain_vol  15   63 = [%.15s]\n",  ptr->bond_ask1_remain_vol);
    fprintf( fp, "채권매수1단계우선호가잔량     bond_bid1_remain_vol  15   78 = [%.15s]\n",  ptr->bond_bid1_remain_vol);
    fprintf( fp, "매도1단계우선호가수익률       ask1_yield            13   93 = [%.13s]\n",  ptr->ask1_yield);
    fprintf( fp, "매수1단계우선호가수익률       bid1_yield            13  106 = [%.13s]\n",  ptr->bid1_yield);
    fprintf( fp, "매도2단계우선호가가격         ask2_price            11  119 = [%.11s]\n",  ptr->ask2_price);
    fprintf( fp, "매수2단계우선호가가격         bid2_price            11  130 = [%.11s]\n",  ptr->bid2_price);
    fprintf( fp, "채권매도2단계우선호가잔량     bond_ask2_remain_vol  15  141 = [%.15s]\n",  ptr->bond_ask2_remain_vol);
    fprintf( fp, "채권매수2단계우선호가잔량     bond_bid2_remain_vol  15  156 = [%.15s]\n",  ptr->bond_bid2_remain_vol);
    fprintf( fp, "매도2단계우선호가수익률       ask2_yield            13  171 = [%.13s]\n",  ptr->ask2_yield);
    fprintf( fp, "매수2단계우선호가수익률       bid2_yield            13  184 = [%.13s]\n",  ptr->bid2_yield);
    fprintf( fp, "매도3단계우선호가가격         ask3_price            11  197 = [%.11s]\n",  ptr->ask3_price);
    fprintf( fp, "매수3단계우선호가가격         bid3_price            11  208 = [%.11s]\n",  ptr->bid3_price);
    fprintf( fp, "채권매도3단계우선호가잔량     bond_ask3_remain_vol  15  219 = [%.15s]\n",  ptr->bond_ask3_remain_vol);
    fprintf( fp, "채권매수3단계우선호가잔량     bond_bid3_remain_vol  15  234 = [%.15s]\n",  ptr->bond_bid3_remain_vol);
    fprintf( fp, "매도3단계우선호가수익률       ask3_yield            13  249 = [%.13s]\n",  ptr->ask3_yield);
    fprintf( fp, "매수3단계우선호가수익률       bid3_yield            13  262 = [%.13s]\n",  ptr->bid3_yield);
    fprintf( fp, "매도4단계우선호가가격         ask4_price            11  275 = [%.11s]\n",  ptr->ask4_price);
    fprintf( fp, "매수4단계우선호가가격         bid4_price            11  286 = [%.11s]\n",  ptr->bid4_price);
    fprintf( fp, "채권매도4단계우선호가잔량     bond_ask4_remain_vol  15  297 = [%.15s]\n",  ptr->bond_ask4_remain_vol);
    fprintf( fp, "채권매수4단계우선호가잔량     bond_bid4_remain_vol  15  312 = [%.15s]\n",  ptr->bond_bid4_remain_vol);
    fprintf( fp, "매도4단계우선호가수익률       ask4_yield            13  327 = [%.13s]\n",  ptr->ask4_yield);
    fprintf( fp, "매수4단계우선호가수익률       bid4_yield            13  340 = [%.13s]\n",  ptr->bid4_yield);
    fprintf( fp, "매도5단계우선호가가격         ask5_price            11  353 = [%.11s]\n",  ptr->ask5_price);
    fprintf( fp, "매수5단계우선호가가격         bid5_price            11  364 = [%.11s]\n",  ptr->bid5_price);
    fprintf( fp, "채권매도5단계우선호가잔량     bond_ask5_remain_vol  15  375 = [%.15s]\n",  ptr->bond_ask5_remain_vol);
    fprintf( fp, "채권매수5단계우선호가잔량     bond_bid5_remain_vol  15  390 = [%.15s]\n",  ptr->bond_bid5_remain_vol);
    fprintf( fp, "매도5단계우선호가수익률       ask5_yield            13  405 = [%.13s]\n",  ptr->ask5_yield);
    fprintf( fp, "매수5단계우선호가수익률       bid5_yield            13  418 = [%.13s]\n",  ptr->bid5_yield);
    fprintf( fp, "채권매도호가총잔량            bond_total_ask_vol    15  431 = [%.15s]\n",  ptr->bond_total_ask_vol);
    fprintf( fp, "채권매수호가총잔량            bond_total_bid_vol    15  446 = [%.15s]\n",  ptr->bond_total_bid_vol);
    fprintf( fp, "정보분배메세지종료키워드      msg_end_key            1  461 = [%.1s]\n",   ptr->msg_end_key);
    fprintf( fp, "%s", "-----------------------------------------------------------------------[ _CO_B601K ]----\n");

    return sizeof( CO_B601K);
}

int BLP_ARG_Print( BLP_ARG* ptr)
{
    LogRaw( "%s", "----[ BLP_ARG ]-------------------------------------------------------------------------\n");
    LogRaw( "1  실행일자                   sb31_exe_ymd           9    0 = [%.9s]\n",   ptr->sb31_exe_ymd);
    LogRaw( "2  실행번호                   sb31_exe_no            4    9 = [%d]\n",     ptr->sb31_exe_no);
    LogRaw( "3  처리상태구분코드           sb31_proc_stus_dstcd   2   13 = [%.2s]\n",   ptr->sb31_proc_stus_dstcd);
    LogRaw( "4  시작일시                   sb31_start_yms        15   15 = [%.15s]\n",  ptr->sb31_start_yms);
    LogRaw( "5  종료일시                   sb31_end_yms          15   30 = [%.15s]\n",  ptr->sb31_end_yms);
    LogRaw( "6  유동성공급시작일시         sb31_lp_start_yms     15   45 = [%.15s]\n",  ptr->sb31_lp_start_yms);
    LogRaw( "7  유동성공급종료일시         sb31_lp_end_yms       15   60 = [%.15s]\n",  ptr->sb31_lp_end_yms);
    LogRaw( "8  종목코드                   sb31_item_cd          13   75 = [%.13s]\n",  ptr->sb31_item_cd);
    LogRaw( "9  유동성공급종목구분코드     sb31_mm_item_dstcd     3   88 = [%.3s]\n",   ptr->sb31_mm_item_dstcd);
    LogRaw( "10 스프래드1가격              sb31_sped_prc[0]       8   91 = [%f]\n",     ptr->sb31_sped_prc[ 0]);
    LogRaw( "11 스프래드2가격              sb31_sped_prc[1]       8   99 = [%f]\n",     ptr->sb31_sped_prc[ 1]);
    LogRaw( "12 스프래드3가격              sb31_sped_prc[2]       8  107 = [%f]\n",     ptr->sb31_sped_prc[ 2]);
    LogRaw( "13 주문1수량                  sb31_ord_qanty[0]      8  115 = [%f]\n",     ptr->sb31_ord_qanty[ 0]);
    LogRaw( "14 주문2수량                  sb31_ord_qanty[1]      8  123 = [%f]\n",     ptr->sb31_ord_qanty[ 1]);
    LogRaw( "15 주문3수량                  sb31_ord_qanty[2]      8  131 = [%f]\n",     ptr->sb31_ord_qanty[ 2]);
    LogRaw( "16 주문수량단위               sb31_ord_qanty_unit    4  139 = [%d]\n",     ptr->sb31_ord_qanty_unit);
    LogRaw( "17 괴리율대상구분코드         sb31_dspratio_taget_dstcd   3  143 = [%.3s]\n",      ptr->sb31_dspratio_taget_dstcd);
    LogRaw( "18 KRX괴리율구분코드          sb31_dspratio_dstcd    2  146 = [%.2s]\n",   ptr->sb31_dspratio_dstcd);
    LogRaw( "19 괴리율                     sb31_dspratio          8  148 = [%f]\n",     ptr->sb31_dspratio);
    LogRaw( "20 틱단위                     sb31_tick_unit         8  156 = [%f]\n",     ptr->sb31_tick_unit);
    LogRaw( "21 거래원번호                 sb31_trdr_uno          6  164 = [%.6s]\n",   ptr->sb31_trdr_uno);
    LogRaw( "22 CMBS트레이더번호           sb31_trdr_no           5  170 = [%.5s]\n",   ptr->sb31_trdr_no);
    LogRaw( "23 민간평가수익률             sb31_prv_yild          8  175 = [%f]\n",     ptr->sb31_prv_yild);
    LogRaw( "24 민간평가가격               sb31_prv_prc           8  183 = [%f]\n",     ptr->sb31_prv_prc);
    LogRaw( "25 종가                       sb31_clsng_prc         8  191 = [%f]\n",     ptr->sb31_clsng_prc);
    LogRaw( "26 종가수익률                 sb31_clsng_yild        8  199 = [%f]\n",     ptr->sb31_clsng_yild);
    LogRaw( "27 계좌번호                   sb31_account_no       12  207 = [%.12s]\n",  ptr->sb31_account_no);
    LogRaw( "%s", "-------------------------------------------------------------------------[ BLP_ARG ]----\n");

    return sizeof( BLP_ARG);
}


