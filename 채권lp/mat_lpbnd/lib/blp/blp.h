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

#include "mem.h"
#include "sem.h"
#include "mcp.h"
#define	DATA_SIZE 2048
#include "buf_struct.h"
#include "shm_memory.h"

#ifndef BLP_H
#define	BLP_H		1

#define		BLP_KEY					0xbb001001
#define		BLP_MAX_TBL				40
#define		BLP_SISE_HOGA			5				/* 시세 호가 범위 - BLP_MAX_HOGA에 포함되므로 MAX_HOGA를 크게 잡아야 */
#define		BLP_MAX_HOGA			( 25 + ( BLP_SISE_HOGA * 2 ))		/* 매수 매도 포함 - 중간 가격을 구하기 위해 홀수로 설정 */
#define		BLP_MAX_LP				3				/* 시장 조성 호가 갯수 */
#define		BLP_MAX_HEDGE			10				/* 햇지 주문 저장 갯수 */
#define		BLP_MAX_ORDER			1000			/* 주문 저장 갯수 */
#define		BLP_MAX_MK_STAT			3				/* 시장 STAT 갯수 오전/오후/마감 */

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
typedef struct _blp_time_
{
	time_t	start;                                          /* LP 조성 시작 시간 */
	time_t	end;                                            /* LP 조성 종료 시간 */
	int		lp_time;										/* LP 최소조성시간 (초) */
	int		exe_delay;										/* 체결이후 조성 대기 시간 */
	int		rev_wait;										/* 반대매매 유지시간 */
	int		submit_limit;									/* 시장조성 제출 한도 시간 - 제출 못하면 종료 */
	int		end_stat;										/* 매매 정리 여부 1:매매정리 0:매매정리 하지 않음 */
    double	sped_prc				[  BLP_MAX_LP];			/* 10 스프래드가격 1,2,3      */
    double	ord_qanty				[  BLP_MAX_LP];			/* 13 주문수량 1,2,3          */
}	BLP_TIME;

typedef struct _blp_arg_
{
    char	exe_ymd					[  8+1] ;			/* 1  실행일자                */
    int		exe_no							;			/* 2  실행번호                */
    char	proc_stus_dstcd			[  1+1] ;			/* 3  처리상태구분코드        */
    char	item_cd					[ 12+1] ;			/* 8  종목코드                */
    char	mm_item_dstcd			[  2+1] ;			/* 9  유동성공급종목구분코드  */
    int		ord_qanty_unit					;			/* 16 주문수량단위            */
    char	dspratio_taget_dstcd	[  2+1] ;			/* 17 괴리율대상구분코드      */
    char	dspratio_dstcd			[  1+1] ;			/* 18 KRX괴리율구분코드       */  
    double	dspratio						;			/* 19 괴리율                  */
    double	tick_unit						;			/* 20 틱단위                  */
    char	trdr_uno				[  5+1];			/* 21 거래원번호              */
    char	trdr_no					[  4+1];			/* 22 CMBS트레이더번호        */
    double	prv_yild						;			/* 23 민간평가수익률          */
    double	prv_prc							;			/* 24 민간평가가격            */
    double	clsng_prc						;			/* 25 종가                    */
    double	clsng_yild						;			/* 26 종가수익률              */
	char	account_no              [ 11+1] ;			/* 27 계좌번호                */
	int		mk_stat							;			/* 시장 조성 순번 0:대기 1:오전 2:오후 3:마감 */
	BLP_TIME	mk_time			[ BLP_MAX_MK_STAT];	/* 시장조성 순번별 시간 */
}	BLP_ARG;

/* 종목일련번호 추가? */
typedef struct _blp_arg_if_
{
    char	exe_ymd						[  8];			/* 실행일자                */
    char	exe_no						[  5];			/* 실행번호                */
    char	proc_stus_dstcd				[  1];			/* 처리상태구분코드        */
    char	item_cd						[ 12];			/* 종목코드                */
    char	mm_item_dstcd				[  2];			/* 유동성공급종목구분코드  */
    char	ord_qanty_unit				[  5];			/* 주문수량단위            */
    char	dspratio_taget_dstcd		[  2];			/* 괴리율대상구분코드      */
    char	dspratio_dstcd				[  1];			/* KRX괴리율구분코드       */
    char	dspratio					[ 11];			/* 괴리율                  */
    char	tick_unit					[ 11];			/* 틱단위                  */
    char	trdr_uno					[  5];			/* 거래원번호              */
    char	trdr_no						[  4];			/* CMBS트레이더번호        */
    char	prv_yild					[ 11];			/* 민간평가수익률          */
    char	prv_prc						[ 11];			/* 민간평가가격            */
    char	clsng_prc					[ 11];			/* 종가                    */
    char	clsng_yild					[ 11];			/* 종가수익률              */
    char	account_no					[ 12];			/* 계좌번호                */
    char	mk_stat						[  1];			/* 시장 조성 순번 항상 3 1:오전 2:오후 3:마감 */

    char	start_1						[  6];			/* LP 조성 시작 시간 HHMMSS */
    char	end_1						[  6];			/* LP 조성 종료 시간 HHMMSS */
	char	lp_time_1					[  5];			/* LP 최소조성시간 (분) */
    char	exe_delay_1					[  5];			/* 체결이후 조성 대기 시간 (초) */
    char	rev_wait_1					[  5];			/* 반대매매 유지시간 (초) */
    char	submit_limit_1				[  5];			/* 시장조성 제출 한도 시간 - 제출 못하면 종료 (초) */
    char	sped_prc_11					[ 11];			/* 스프래드가격 1,2,3      */
    char	sped_prc_12					[ 11];			/* 스프래드가격 1,2,3      */
    char	sped_prc_13					[ 11];			/* 스프래드가격 1,2,3      */
    char	ord_qanty_11				[ 10];			/* 주문수량 1,2,3          */
    char	ord_qanty_12				[ 10];			/* 주문수량 1,2,3          */
    char	ord_qanty_13				[ 10];			/* 주문수량 1,2,3          */

    char	start_2						[  6];			/* LP 조성 시작 시간 HHMMSS */
    char	end_2						[  6];			/* LP 조성 종료 시간 HHMMSS */
	char	lp_time_2					[  5];			/* LP 최소조성시간 (분) */
    char	exe_delay_2					[  5];			/* 체결이후 조성 대기 시간 (초) */
    char	rev_wait_2					[  5];			/* 반대매매 유지시간 (초) */
    char	submit_limit_2				[  5];			/* 시장조성 제출 한도 시간 - 제출 못하면 종료 (초) */
    char	sped_prc_21					[ 11];			/* 스프래드가격 1,2,3      */
    char	sped_prc_22					[ 11];			/* 스프래드가격 1,2,3      */
    char	sped_prc_23					[ 11];			/* 스프래드가격 1,2,3      */
    char	ord_qanty_21				[ 10];			/* 주문수량 1,2,3          */
    char	ord_qanty_22				[ 10];			/* 주문수량 1,2,3          */
    char	ord_qanty_23				[ 10];			/* 주문수량 1,2,3          */

    char	start_3						[  6];			/* LP 조성 시작 시간 HHMMSS */
    char	end_3						[  6];			/* LP 조성 종료 시간 HHMMSS */
	char	lp_time_3					[  5];			/* LP 최소조성시간 (분) */
    char	exe_delay_3					[  5];			/* 체결이후 조성 대기 시간 (초) */
    char	rev_wait_3					[  5];			/* 반대매매 유지시간 (초) */
    char	submit_limit_3				[  5];			/* 시장조성 제출 한도 시간 - 제출 못하면 종료 (초) */
    char	sped_prc_31					[ 11];			/* 스프래드가격 1,2,3      */
    char	sped_prc_32					[ 11];			/* 스프래드가격 1,2,3      */
    char	sped_prc_33					[ 11];			/* 스프래드가격 1,2,3      */
    char	ord_qanty_31				[ 10];			/* 주문수량 1,2,3          */
    char	ord_qanty_32				[ 10];			/* 주문수량 1,2,3          */
    char	ord_qanty_33				[ 10];			/* 주문수량 1,2,3          */
}   BLP_ARG_IF;

typedef struct _member_area_
{
	char	reserved					[ 30];			/* 원장 사용 영역 */
	char	auto_yn						[  1];			/* 자동주문 여부 A=자동주문 C=Client 주문 */
	char	strategy					[  2];			/* 전략 여부 전략=ST                      */
	char	stg_no						[  2];			/* 전략번호 */
	char	market						[  2];			/* 시장구분 01=채권일반 02=채권LP */
	char	index						[  5];			/* A0 순번 */
	char	acc_seq						[  2];			/* 계좌번호 Seq */
	char	stock						[  1];			/* 유가증권 주식선물 구분 채권=0 */
	char	spread						[  1];			/* 스프레드여부 0:normal, 1:스프레드 */
	char	dummy						[  1];			/* 모르는 field */
	char	proc_nm						[  4];			/* Process Nick Name pa_50203mp => 0203 */
}	BLP_MEMBER_AREA;

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
	int		order_no;
}	BLP_STAT;

#if 0
/** ***************************************************************************
**  @st         BLP_STAT
**  @brief
**  채권시장조성 호가 RECORD - 주문 
***************************************************************************** */
typedef struct _blp_ord_tbl_
{
	int		mode;
	int		stat;
	time_t	ord_time;
	char	order[ 2048];
}	BLP_ORD_TBL;
#endif

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
	int		lp_no;			/* LP 주문 순번 */
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
	int			stat;			/* 0:not_order 1:주문 2:주문확인 3:정정 4:정정확인 5:취소 6:취소확인 9:주문오류 */
								/* 홀수:주문 짝수:확인 */
	int			ho_no;			/* 시세 호가 테이블 위치 rec->no */
	int			ord_stat;		/* 주문상태 0:없음 1:주문 2:정정 3:취소 */
	int			res_stat;		/* 응답상태 0:없음 1:응답 2:체결 3:거부 */
	char		ord_no[ 32];	/* 주문 번호 */
	char		org_no[ 32];	/* 원 주문번호 */
	double		ask_prc;		/* 매도 주문 가격 */
	double		bid_prc;		/* 매수 주문 가격 */
	double		ask_vol;		/* 매도 주문 수량 */
	double		bid_vol;		/* 매수 주문 수량 */
	double		ask_exe_vol;	/* 매도 체결 수량 */
	double		bid_exe_vol;	/* 매수 체결 수량 */
}	BLP_ORD;

/** ***************************************************************************
**  @st         BLP_STAT
**  @brief
**  채권시장조성 호가 RECORD - 주문 
***************************************************************************** */
typedef struct _blp_hedge_
{
	int			done;			/* 완료 여부 1:체결 완료 2:취소완료 */
	int			ord_stat;		/* 주문상태 0:없음 1:주문 2:취소 */
	int			res_stat;		/* 응답상태 0:없음 1:응답 2:거부 3:체결 4:완료 5:오류 */
	char		ord_no[ 32];	/* 주문 번호 */
	char		org_no[ 32];	/* 원 주문 번호 */
	int			side;			/* 매도:1 매수:2 */
	double		prc;			/* 주문 가격 */
	double		vol;			/* 주문 수량 */
	double		exe_vol;		/* 체결 수량 - 부분체결 누적 */
	time_t		ord_time;		/* 주문 시간 */
	int			gap_time;		/* 주문 수행 시간 */
}	BLP_HEDGE;


/** ***************************************************************************
**  @st         BLP_STAT
**  @brief
**  채권시장조성 상태 호가 TABLE
***************************************************************************** */
typedef struct _blp_hoga_tbl
{
	int				id;						/* 종목별 id - 0번째 tbl은 전체 갯수 등록 */
	int				item_pos;				/* 종목 table 번호 */
	char			item_code[ 16];			/* 종목코드 */
	int				item_no;				/* 종목순번 */
	int				proc_cnt;				/* 처리 횟수 */
	int				ord_cnt;				/* 주문 처리 횟수 */
	struct timeval	proc_time;				/* 처리 시간 */
	int				seq_no;					/* 시세 순번 */
	int				mode;					/* 0:대기모드 1:전략수행 2:취소모드 3:햇지모드 4:종료 */
	int				mode_next;				/* 0:없음 1:전략수행 2:취소모드 3:햇지모드 4:종료 */
	time_t			mode_time;				/* 모드 변경 시간 */
	int				stat;					/* 0:대기 1:주문 2:확인 */
	char			board_id[ 2];
	char			stg_id[ 16];			/* 전략 번호 */
#if 0
	time_t			start_time;				/* 전략 시작 시간 */
	time_t			end_time;				/* 전략 종료 시간 */
	time_t			stop_time;				/* 전략 멈춤 시간 */
	time_t			cont_time;				/* 전략 재게 시간 */
#endif
	int				ask_base;				/* 매도 1 호가 번호 */
	int				bid_base;				/* 매수 1 호가 번호 */
	int				ask_gap;				/* 직전 호가와 차이 */
	int				bid_gap;				/* 직전 호가와 차이 */
	int				ask_vol_gap;			/* 수량 차이 */
	int				bid_vol_gap;			/* 수량 차이 */
	int				hedge_cnt;				/* 햇지 주문 건수 */
	BLP_ARG			arg;					/* 전략 parameter */
	BLP_ORD			order[ BLP_MAX_LP];		/* 주문 1,2,3 호가 */
	BLP_ORD			old_order[ BLP_MAX_LP];	/* 이전 주문 1,2,3 호가 */
	BLP_HEDGE		hedge[ BLP_MAX_HEDGE];	/* 햇지 주문 */
	BLP_SISE		sise;					/* 거래소 시세 */
	BLP_HOGA_REC	rec[ BLP_MAX_HOGA];		/* 호가 table 0:매도 끝:매수 */
	BLP_HOGA_REC	old[ BLP_MAX_HOGA];		/* 이전 호가 table */
#if 0
	BLP_ORD_TBL		ord_tbl[ BLP_MAX_ORDER]	/* 주문 테이블 */
#endif
}	BLP_TBL;

/** ***************************************************************************
**  @st         BLP_STAT
**  @brief
**  채권시장조성 상태 공유메모리 MAP
***************************************************************************** */
typedef struct _shm_map_
{
	BLP_STAT		stat;
	BLP_TBL	tbl[ BLP_MAX_TBL];
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
	int			tbl_pos;			/* 종목 테이블 위치 */
	MCP			*mcp;				/* sise socket for test */
}	BLP;

#endif	/* BLP_H */


/***** Module : blp.c *****/
BLP*        Blp_CreateForce( key_t blp_key);                                /* 채권 시장조성에 필요한 ipc를 생성 ########## */
BLP*        Blp_Create( key_t blp_key);                                     /* 채권시장조성에 필요한 ipc를 생성 */
int         Blp_Remove( BLP *blp);                                          /* 채권시장조성에서 생성한 ipc를 삭제 */
BLP*        Blp_Open( key_t blp_key, int service);                          /* 채권시장조성 Open */
int         Blp_Close( BLP *blp);                                           /* 채권시장조성 Close */
int         Blp_ShmInit( BLP *blp);                                         /* 공유메모리 값을 초기화 - create 시만  */
int         Blp_Init( BLP *blp);                                            /* BLP struct 값 initial - open 시 */
int         Blp_Process( BLP *blp, void *data);                             /* 전략 실행 */
int         Blp_StopProcess( BLP *blp, void *data);                         /* 전략 실행 */
int         Blp_WaitProcess( BLP *blp);                                     /* 전략 실행 */
int         Blp_TimeCheck( BLP *blp, BLP_TBL *tbl);                         /* 시장조성 시간 check 대기/오전/오후/마감 */
int         Blp_ClearTbl( BLP *blp, BLP_TBL *tbl);                          /* 종목코드로 찾기 */
int         Blp_GetItem( BLP *blp, char *item_code);                        /* 종목코드로 찾기 */
int         Blp_GetPrice( BLP *blp, BLP_TBL *tbl, double price);            /* 가격으로 찾기 */
int         Blp_AddItem( BLP *blp, char *item_code);                        /* 종목코드 등록 */
int         Blp_UpdateHoga( BLP *blp, BLP_TBL *tbl, int lp_no);             /* 호가 테이블 LP 주문 update */
int         Blp_List( BLP *blp);                                            /* 채권시장조성 호가 테이블 리스트 */
int         Blp_Lock( BLP *blp);                                            /* semaphore lock */
int         Blp_Unlock( BLP *blp);                                          /* semaphore unlock */
int         Blp_Stat( BLP *blp, char *item_code);                           /* 채권시장조성 공유메모리 상태 */
int         Blp_SetArg( BLP *blp, char *rec, char *ap_type);                /* sise receive from file - for test */
char*       Blp_TvToS( struct timeval *tv);                                 /* util - struct timeval to string */
int         Blp_SiseOpen( BLP *blp);                                        /* for sise receive - test */
int         Blp_SiseRecv( BLP *blp, char *buf, int sz);                     /* for sise receive - test */
int         Od_Write_DataBond( int option, char *buf, int sz);              /* for sise receive - test */
int         GetOrderNo( char *buf, int sz);                                 /* for sise receive - test */

/***** Module : blp_sise.c *****/
int         Blp_ProcessSiseExe( BLP *blp, BLP_TBL *tbl, BLP_SISE *sise, CO_G701K *g701k);/* 거래소 체결 시세 처리 */
int         Blp_ProcessSiseHoga( BLP *blp, BLP_TBL *tbl, BLP_SISE *sise, CO_G701K *g701k);/* 거래소 호가 시세 처리 */
int         Blp_ProcessSise( BLP *blp, char *krx_sise);                     /* 거래소 시세 처리 - 전체 종목 처리 (현재 안씀) */
int         Blp_GetSiseFromFile( BLP *blp, CO_B601K *b601k, char *f_name);  /* sise receive from file - for test */
int         Blp_MakeHogaTbl( BLP *blp, BLP_TBL *tbl, BLP_SISE *sise);       /* 호가 테이블 구성 */

/***** Module : blp_order.c *****/
int         Blp_ProcessOrder( BLP *blp, BLP_TBL *tbl);                      /* 주문가격 계산 및 호가 제출 */
int         Blp_ProcessExecute( BLP *blp, FILE_BUFF_FORMAT *file_buffer);   /* 거래소 주문응답/체결 처리 */
int         Blp_ProcOrdChk( BLP *blp, BLP_TBL *tbl, KRX_NOTE_SETTLE_RESP_DATA *ttrodp41301);/* 주문응답처리 - TTRMOP41301 */
int         Blp_ProcLpOrdChk( BLP *blp, BLP_TBL *tbl, KRX_LP_NOTE_SETTLE_RESP_DATA *ttrodp41301);/* LP 주문응답처리 */
int         Blp_ProcExeChk( BLP *blp, BLP_TBL *tbl, KRX_NOTE_SETTLE_DATA *settle);/* 체결 처리 */
int         Blp_ProcessCancel( BLP *blp, BLP_TBL *tbl);                     /* 모든 LP 주문을 취소 */
int         Blp_ProcHdgOrd( BLP *blp, BLP_TBL *tbl);                        /* 모든 LP 주문을 취소 */
int         Blp_ProcessHedge( BLP *blp, BLP_TBL *tbl);                      /* 모든 LP 주문을 취소 */
int         Blp_CheckOrder( BLP *blp, BLP_TBL *tbl, BLP_SISE *sise);        /* 기준 가격에 대한 유효 호가 설정  */
int         Blp_Order( BLP *blp, BLP_TBL *tbl, int lp_no, double ask_prc, double bid_prc);/* LP 주문  */
int         Blp_MakeOrder( BLP *blp, BLP_TBL *tbl, int no, int option);     /* 채권시장조성 호가 테이블 리스트 */
int         Blp_MakeLpOrder( BLP *blp, BLP_TBL *tbl, int no, int option);   /* 채권시장조성 호가 테이블 리스트 */
int         Blp_GetOrderNoOMS( BLP *blp, char *rec, int sz);                /* 주문 번호 채번 */
int         Blp_GetOrderNoOMS( BLP *blp, char *rec, int sz);

/***** Module : blp_convert.c *****/
int         Blp_ConvertExe( BLP *blp, BLP_SISE *sise, CO_G701K *g701k);     /* 거래소 체결 시세 처리 */
int         Blp_ConvertHoga( BLP *blp, BLP_SISE *sise, CO_B601K *b601k);    /* 거래소 호가 시세 처리 */

/***** Module : blp_print.c *****/
int         Blp_ConvertArg( BLP_ARG *arg, char *rec);
int         BLP_ARG_IF_Print( BLP_ARG_IF* ptr);
int         SHM_NOTE_Print( SHM_NOTE* ptr);
int         CO_B601K_Print( CO_B601K* ptr);
int         CO_B601K_PrintFile( CO_B601K* ptr, FILE *fp);
int         CO_G701K_Print( CO_G701K* ptr);
int         BLP_ARG_Print( BLP_ARG* ptr);
int         BLP_SISE_Print( BLP_SISE* ptr);
int         KRX_NOTE_JUMUN_DATA_Print( KRX_NOTE_JUMUN_DATA* ptr);
int         BLP_MEMBER_AREA_Print( BLP_MEMBER_AREA* ptr);
int         KRX_LP_NOTE_JUMUN_DATA_Print( KRX_LP_NOTE_JUMUN_DATA* ptr);
int         FILE_BUFF_FORMAT_Print( FILE_BUFF_FORMAT* ptr);
int         KRX_LP_NOTE_SETTLE_RESP_DATA_Print( KRX_LP_NOTE_SETTLE_RESP_DATA* ptr);
int         KRX_NOTE_SETTLE_DATA_Print( KRX_NOTE_SETTLE_DATA* ptr);
int         KRX_NOTE_SETTLE_RESP_DATA_Print( KRX_NOTE_SETTLE_RESP_DATA* ptr);

