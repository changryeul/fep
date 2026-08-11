#ifndef     __KRX_MK_H
#define     __KRX_MK_H
/*------------------------------------------------------------------------
#   Module  : common structures of file format
#   File    : krx_mk.h
------------------------------------------------------------------------*/
/*
	// 현물 
	A0011/A0012 : 유가증권/Kosdaq Master
	A3011/A3012/A3021 : 유가증권/Kosdaq/ELW 체결
	B6011/B6012 : 유가증권/Kosdaq 호가
	// ELW, A0011+A1011(ELW종목배치)+I7011(LP정보)
	A1011 : ELW종목배치
	I7011 : LP정보
	B7011 : 호가LP포함 

	// 파생(선물)
	A0014/A0015 : 지수선물/주식선물 Master
	A3014/A3015 : 지수선물/주식선물 A3체결
	G7014/G7015 : 지수선물/주식선물 G7체결
	B6014/B6015 : 지수선물/주식선물 호가

	// 	파생(옵션)
	A0034/A0025 : 지수옵션/주식옵션 Master
	A3034/A3025 : 지수옵션/주식옵션 A3체결
	G7034/G7025 : 지수옵션/주식옵션 G7체결
	B6034/B6025 : 지수옵션/주식옵션 호가

	// ReDefine Info
	A0011/A0012
	A0014/A0034/A0015/A0025

	// TR Info
	SIF_A0
	SIF_A3
	SIF_G7
	SIF_B6

	SIO_A3
	SIO_G7
	SIO_B6

	STOCK_A0
	STOCK_A3
	STOCK_B6

	STOCK_A1
	STOCK_I7
	STOCK_B7
	STOCK_N8
	STOCK_M8
	STOCK_A1
	STOCK_S1
	STOCK_S2

	SSF_A3
	SSF_G7
	SSF_B6

	SSO_A3
	SSO_G7
	SSO_B6
*/

/* 2025 New Start */
/* ******************************************************************* */
/*	파생시장공통					00F
--------------------------------------------
	kospi200선물					01F
--------------------------------------------
	코스닥150선물					02F
--------------------------------------------
	KOSPI200옵션					03F
--------------------------------------------
	주식선물						04F
--------------------------------------------
	주식옵션						05F
--------------------------------------------
	금융상품선물(국채,금리,통화)	06F			파생에서는 이것만 사용
--------------------------------------------
	상품옵션(휴면)					07F
--------------------------------------------
	변동성지수선물					08F
	"섹터지수선물, 
--------------------------------------------
	코스닥글로벌지수선물"			09F
--------------------------------------------
	일반상품선물(금,돈육(휴면))		10F
--------------------------------------------
	미니kospi200선물				11F
--------------------------------------------
	미니kospi200옵션				12F
--------------------------------------------
	KRX300선물						13F
--------------------------------------------
	EURO STOXX 50선물				14F
--------------------------------------------
	코스닥150옵션					15F
--------------------------------------------
	코스피 200 위클리 옵션			16F
--------------------------------------------
*/
/* ******************************************************************** */

/* TR정의 */
/*
	- 파생종목정보 RDS(TRDESP01001)에서 수신하는 Struct를 구성한다. 종목별 "가격단위"를 알수있는게 RDS뿐이다.

	- 선물(금융상품선물(국채,금리,통화) 16F 기준 사용TR
	A001F(10301/11301) : 1318, 종목정보
	A301F : 173, 파생체결
	G701F : 431, 파생체결G7 (우선호가  5단계) 04F & 05F(제외)
	B601F : 324, 파생우선호가 (우선호가  5단계) 04F & 05F(제외)
    A701A : 68,  장운영정보(모든 A7공통.. All)
	M401A : 83,  장운영스캐쥴
	R101F : 359, 파생 장운영TS + 우선호가 (우선호가  5단계) 단일가 세션, 04F, 05F 제외

	G704F : 파생체결G7 (우선호가 10단계) 04F & 05F
	B604F : 파생우선호가 (우선호가 10단계) 04F & 05F
	R104F : 파생 장운영TS + 우선호가 (우선호가 10단계) 단일가 세션, 04F, 05F 포함

	A601F : 파생종목마감
	Q201F : 파생동적상하한가 적용및해제(05없음)
	V101F : 가격제한폭확대발동
	I2000 : Polling Data

	- 제외 TR
	O606F : 배분정보
	IF06F : 그룹호가접수중지/해제 공개
	B201F : 파생시세 SnapShot  5호가(04,05제외)
	B204F : 파생시세 SnapShot 10호가(04,05전용)
	C401F : 파생협의거래결과(장종료후)
	M701F : Eurex연계 장개시전 협의거래(08:45 1회)
	H101F : 파생 상품별 투자자별 통계 (08:45~15:45, [30초 주기]), 서비스상품별 상이
	H101F : 파생 상품별 투자자별 통계 (전일확정치:07:30, 당일확정치:16:30), 서비스상품별 상이
	H201F : 파생종목 미결제약정수량 데이터 (전일확정치:장개시전, 장중:10초주기,당일확정치:장종료후)
	H301F : 선물종목 정산가격 데이터 (장종료후)

	- 선물 Port (모든 Port에는 I2000이 있다)
	A001F 100M : 10301 / 11301 , 233.38.231.91
           12M : 10301 / 11301 , 233.38.231.91

	M401F
		  100M : 10315 / 11315 , 233.38.231.93
		   12M : 10315 / 11315 , 233.38.231.93

	A7/O6/B6/A3/G7/R1/A6/Q2/V1/I2
	A701F 100M :               , 233.38.231.92 
				* 파생시장 시세 송신 port 분리 및 종목별 보드별 일련번호 제공
				- 10302(코스피200선물)
				- 10303(코스닥150선물)
				- 10304(개별주식선물)
				- 10305(금융상품선물(국채,금리,통화)),			금리선물 단위 100,000,000 (1억)
				- 10306(일반상품선물(금, 돈육(휴면)))
				- 10307(미니코스피200선물)
				- 10308(코스피200변동성지수선물)
				- 10309(코스피200섹터지수선물
				- 10310(KRX300선물)
		   12M :               , 233.38.231.112
				* 파생시장 시세 송신 port 분리
				- 10342(코스피200선물)
				- 10343(코스닥150선물)
				- 10344(개별주식선물)
				- 10345(금융상품선물(국채,금리,통화))
				- 10346(일반상품선물(금, 돈육(휴면)))
				- 10347(미니코스피200선물)
				- 10348(코스피200변동성지수선물)
				- 10349(코스피200섹터지수선물
				- 10350(KRX300선물)

	G701F : 파생체결G7 (우선호가  5단계) 04F & 05F(제외)
	B601F : 파생우선호가 (우선호가  5단계) 04F & 05F(제외)
    A701A : 장운영정보(모든 A7공통.. All)
	M401A : 장운영스캐쥴
	R101F : 파생 장운영TS + 우선호가 (우선호가  5단계) 단일가 세션, 04F, 05F 제외

	G704F : 파생체결G7 (우선호가 10단계) 04F & 05F
	B604F : 파생우선호가 (우선호가 10단계) 04F & 05F
	R104F : 파생 장운영TS + 우선호가 (우선호가 10단계) 단일가 세션, 04F, 05F 포함

	A601F : 파생종목마감
	Q201F : 파생동적상하한가 적용및해제(05없음)
	V101F : 가격제한폭확대발동

*/

/* 가격단위규칙, START	RSD파일명 TRDESP01901 (수신여부는 실제 Data수신후) */
/*
	A0.001		-999999999		999999999		0.001
	A0.005		-999999999		999999999		0.005
	A0.01		-999999999		999999999		0.01
	A0.02		-999999999		999999999		0.02
	A0.05		-999999999		999999999		0.05
	A0.1		-999999999		999999999		0.1
	A0.2		-999999999		999999999		0.2
	A0.5		-999999999		999999999		0.5
	A1			-999999999		999999999		1
	A10			-999999999		999999999		10
	A100		-999999999		999999999		100
	AK			-999999999		999999999		1000
	A20			-999999999		999999999		20
	A25			-999999999		999999999		25
	A250		-999999999		999999999		250
	A5			-999999999		999999999		5
	A50			-999999999		999999999		50
	A500		-999999999		999999999		500
	A5_2		         0		999999999		5
	RNG_A5				 0		     2000		1
	RNG_A5			  2000		999999999		5
	RNG_FKQ			     0           2000		1
	RNG_FKQ		      2000           5000		5
	RNG_FKQ			  5000          20000		10
	RNG_FKQ			 20000          50000		50
	RNG_FKQ			 50000         200000		100
	RNG_FKQ			200000         500000		500
	RNG_FKQ			500000      999999999		1000
	RNG_KSQ			     0           2000		1
	RNG_KSQ		      2000           5000		5
	RNG_KSQ			  5000          20000		10
	RNG_KSQ			 20000          50000		50
	RNG_KSQ			 50000         200000		100
	RNG_KSQ			200000         500000		500
	RNG_KSQ			500000      999999999		1000
	RNG_STK			     0           2000		1
	RNG_STK		      2000           5000		5
	RNG_STK			  5000          20000		10
	RNG_STK			 20000          50000		50
	RNG_STK			 50000         200000		100
	RNG_STK			200000         500000		500
	RNG_STK			500000      999999999		1000
	RNG_OKI				 0		       10		0.01
	RNG_OKI			    10		999999999		0.05
	RNG_OKI_M			 0		        3		0.01
	RNG_OKI_M			 3		       10		0.02
	RNG_OKI_M		    10		999999999		0.05
	RNG_OQI				 0		       50		0.1
	RNG_OQI			    50		999999999		0.5
	RNG_OST			     0           1000		10
	RNG_OST		      1000           2000		20
	RNG_OST			  2000           5000		50
	RNG_OST			  5000          10000		100
	RNG_OST			 10000      999999999		200
*/
/* 가격단위규칙, END	*/

/* 금융파생의 가격단위 Infor */
/*
- 파생상품 가격단위								
	1. 국채선물 (KTB 선물)			고정값				
		종목명	기초자산	1틱(최소 가격단위)		1틱 금액	승수		
		3년 국채선물	3년 만기 국고채		0.01	10,000원	승수 1천만원	1틱에 10만원	0.01포인트 × 10,000,000원 = 10,000원
		5년 국채선물	5년 만기 국고채		0.01	10,000원			
		10년 국채선물	10년 만기 국고채	0.01	10,000원			
								
	2. 통화선물 (Currency Futures)							
		종목명	기초자산	1틱	1틱 금액			
		USD/KRW 선물	미국 달러-원 환율	0.01	5,000원		승수 500,000	1틱에 5천원		0.01 × 500,000 = 5,000원
		JPY/KRW 선물	100엔-원 환율		0.01	5,000원			
		EUR/KRW 선물	유로-원 환율		0.01	5,000원			
								
	3. 금리선물 (IRS 등)							
		KRX에서는 전통적인 금리선물은 거래되지 않고 있음, 국채선물이 금리상품 대용으로 사용되고 있음.						
*/

/* ************************************************** */
/* 장운영정보
	(파생A) DRV : A001F, A002F, A003F, A004F, A005F, A006F, A007F, A008F, A009F, A010F, A011F, A012F, A013F, A015F, A016F
*/
/* ******************************************************************** */
/* 파생종목정보 (A001F ~ A016F)											*/
/* ******************************************************************** */
typedef struct
{
	char tr_gbn                     [5];   /* TR CODE	*/
	char seq_no                		[8];   /* 정보분배일련번호 */
	char info_type_tot_items        [6];   /* 정보구분총종목수 */
	char biz_date                   [8];   /* 영업일자 */
	char item_code                    [12];  /* 종목코드 */
	char info_stock_idx             [6];   /* 정보분배종목인덱스 */
	char fut_opt_type_cd            [1];   /* 선물옵션구분코드 */
	char product_id                 [11];  /* 상품ID */
	char item_short_cd              [9];   /* 종목단축코드 */
	char item_kor_nm                [80];  /* 종목명 */
	char item_short_nm              [40];  /* 종목약명 */
	char item_eng_nm                [80];  /* 종목영문명 */
	char item_eng_short_nm          [40];  /* 종목영문약명 */
	char mkt_prod_grp_id            [3];   /* 장운영상품그룹ID */
	char list_date                  [8];   /* 상장일자 */
	char delist_date                [8];   /* 상장폐지일자 */
	char spr_base_item_type_cd      [1];   /* 스프레드기준종목구분코드 */
	char finl_sett_meth_cd          [1];   /* 최종결제방법코드 */
	char prc_lmt_exp_dir_cd         [1];   /* 가격제한확대적용방향코드 */
	char prc_lmt_finl_stg           [3];   /* 가격제한최종단계 */
	char prc_lmt_stg1_up            [11];  /* 가격제한1단계상한가 */
	char prc_lmt_stg2_up            [11];  /* 가격제한2단계상한가 */
	char prc_lmt_stg3_up            [11];  /* 가격제한3단계상한가 */
	char prc_lmt_stg1_lo            [11];  /* 가격제한1단계하한가 */
	char prc_lmt_stg2_lo            [11];  /* 가격제한2단계하한가 */
	char prc_lmt_stg3_lo            [11];  /* 가격제한3단계하한가 */
	char base_prc                   [11];  /* 기준가격 */
	char undly_asset_id             [3];   /* 기초자산ID */
	char right_ex_type_cd           [1];   /* 권리행사유형코드 */
	char spr_comp_cd                [2];   /* 스프레드구성코드 */
	char spr_comp_item_cd1          [12];  /* 스프레드구성종목코드1 */
	char spr_comp_item_cd2          [12];  /* 스프레드구성종목코드2 */
	char last_trade_date            [8];   /* 최종거래일자 */
	char finl_sett_date             [8];   /* 최종결제일자 */
	char sett_mnth_ser              [3];   /* 결제월일련번호 */
	char exp_date                   [8];   /* 만기일자 */
	char strike_prc                 [18];  /* 행사가격, 확인필요 9(10)V9(8) */
	char adj_type_cd                [1];   /* 조정구분코드 */
	char trade_unit                 [22];  /* 거래단위 */
	char trade_mult                 [22];  /* 거래승수 */
	char mkt_make_type_cd           [1];   /* 시장조성구분코드 */
	char list_type_cd               [1];   /* 상장유형코드 */
	char eqv_prc                    [11];  /* 등가격 */
	char adj_rsn_cd                 [2];   /* 조정사유코드 */
	char undly_asset_item_cd        [12];  /* 기초자산종목코드 */
	char undly_asset_close_prc      [11];  /* 기초자산종가 */
	char remain_days                [8];   /* 잔존일수 */
	char adj_base_prc               [18];  /* 조정기준가격, 우선 기준가로 했음 */
	char base_prc_type_cd           [2];   /* 기준가격구분코드 */
	char trade_base_prc_type_cd     [1];   /* 매매용기준가격구분코드 */
	char prev_day_adj_close         [18];  /* 전일조정종가 */
	char neg_blk_trade_target_yn    [1];   /* 협의대량매매대상여부 */
	char prev_day_mgn_base_prc      [23];  /* 전일증거금기준가격 */
	char mgn_base_prc_type_cd       [2];   /* 증거금기준가격구분코드 */
	char sett_theo_prc              [16];  /* 정산이론가격 */
	char base_theo_prc              [16];  /* 기준이론가격 */
	char prev_day_sett_prc          [18];  /* 전일정산가격 */
	char trade_susp_yn              [1];   /* 거래정지여부 */
	char fut_circuit_brk_up         [11];  /* 선물CIRCUIT_BREAKERS상한가 */
	char fut_circuit_brk_lo         [11];  /* 선물CIRCUIT_BREAKERS하한가 */
	char view_strike_prc            [18];  /* 조회용행사가격, 확인필요 9(10)V9(8) */
	char atm_type_cd                [1];   /* ATM구분코드 */
	char last_trade_date_yn         [1];   /* 최종거래일여부 */
	char ex_div_div_val             [16];  /* 배당락후배당가치 */
	char prev_day_close             [11];  /* 전일종가 */
	char prev_day_close_type_cd     [1];   /* 전일종가구분코드 */
	char prev_day_open              [11];  /* 이전일자시가 */
	char prev_day_high              [11];  /* 이전일자고가 */
	char prev_day_low               [11];  /* 이전일자저가 */
	char first_trade_date           [8];   /* 최초거래일자 */
	char last_exec_time             [9];   /* 최종체결시각 */
	char sett_prc_type_cd           [2];   /* 정산가격구분코드 */
	char sett_prc_theo_prc_div_rate [13];  /* 정산가격이론가격괴리율 */
	char prev_day_open_int          [12];  /* 전일미결제약정수량 */
	char prev_day_sell_pri_qt_prc   [11];  /* 전일매도우선호가가격 */
	char prev_day_buy_pri_qt_prc    [11];  /* 전일매수우선호가가격 */
	char impl_vol                   [11];  /* 내재변동성 */
	char list_high_prc              [11];  /* 상장중최고가 */
	char list_low_prc               [11];  /* 상장중최저가 */
	char yr_high_prc                [11];  /* 연중최고가 */
	char yr_low_prc                 [11];  /* 연중최저가 */
	char list_high_prc_date         [8];   /* 상장중최고가일자 */
	char list_low_prc_date          [8];   /* 상장중최저가일자 */
	char yr_high_prc_date           [8];   /* 연중최고가일자 */
	char yr_low_prc_date            [8];   /* 연중최저가일자 */
	char yr_base_days               [8];   /* 연간기준일수 */
	char mnth_trade_days            [8];   /* 월간거래일수 */
	char yr_trade_days              [8];   /* 연간거래일수 */
	char prev_day_exec_ord_qty      [15];  /* 전일체결건수 */
	char prev_day_acc_vol           [12];  /* 전일누적거래량 */
	char prev_day_acc_amt           [22];  /* 전일누적거래대금 */
	char prev_day_tot_acc_vol       [15];  /* 전일총누적거래량 */
	char prev_day_tot_acc_amt       [22];  /* 전일총누적거래대금 */
	char int_rate                   [11];  /* 금리 */
	char stk_fut_open_int_limit     [15];  /* 주식선물미결제한도수량 */
	char undly_asset_prod_grp_id    [4];   /* 기초자산상품군ID */
	char mgn_offset_ratio           [11];  /* 증거금OFFSET비율 */

	/* Bitwise 1:FAS, 2:FOK, 4:FAK, 8:GTS (Good for The Session), 15:GTC (Good Till Cancel), 32:GTD (Good Till Date)	*/
	char lim_ord_cancel_cond_cd     [5];   /* 지정가호가취소조건코드 */
	char mkt_ord_cancel_cond_cd     [5];   /* 시장가호가취소조건코드 */
	char cond_lim_ord_cancel_cond_cd[5];   /* 조건부지정가호가취소조건코드 */
	char best_lim_ord_cancel_cond_cd[5];   /* 최유리지정가호가취소조건코드 */

	char efp_trade_target_yn        [1];   /* EFP거래대상여부 */
	char flex_trade_target_yn       [1];   /* FLEX거래대상여부 */
	char efp_exec_vol               [12];  /* EFP체결수량 */
	char efp_trade_amt              [22];  /* EFP거래대금 */
	char mkt_closed_yn              [1];   /* 휴장여부 */
	char dyn_prc_lmt_yn             [1];   /* 동적가격제한여부 */
	char dyn_up_prc_intv            [11];  /* 동적상한가간격 */
	char dyn_lo_prc_intv            [11];  /* 동적하한가간격 */
	char undly_asset_mkt_id         [3];   /* 기초자산시장ID */
	char up_qty                     [23];  /* 상한수량 */
	char lo_qty                     [23];  /* 하한수량 */
	char neg_blk_trade_up_qty       [23];  /* 협의대량매매상한수량 */
	char neg_blk_trade_lo_qty       [23];  /* 협의대량매매하한수량 */
	char base_prod_id               [11];  /* 기준상품ID */
	char sub_prod_id                [11];  /* 부상품ID */
	char base_prod_item_ord_qty     [6];   /* 기준상품종목수 */
	char sub_prod_item_ord_qty      [6];   /* 부상품종목수 */
	char sett_week                  [2];   /* 결제주 */
	char dormant_yn                 [1];   /* 휴면여부 */
	char dormant_desig_date         [8];   /* 휴면지정일자 */
	char msg_end_key                [1];   /* 정보분배메세지종료키워드 */
}	CO_A001F;	/* TR: A001F ~ A016F */

/* ************************************************** */
/* 파생체결
	(파생A) DRV : A301F, A302F, A303F, A304F, A305F, A306F, A307F, A308F, A309F, A310F, A311F, A312F, A313F, A315F, A316F
*/
/* ************************************************** */
typedef struct
{
	char tr_gbn			    [5];  /* TR CODE						*/
	char seq_no            [8];   /* 정보분배일련번호 */
	char board_id               [2];   /* 보드ID */
	char session_id             [2];   /* 세션ID */
	char item_code                [12];  /* 종목코드 */
	char info_stock_idx         [6];   /* 정보분배종목인덱스 */
	char trade_time             [12];  /* 매매처리시각 */
	char crprc             [9];   /* 체결가격 */
	char volume                 [9];   /* 거래량 */
	char near_month_crprc  [9];   /* 근월물체결가격 */
	char far_month_price        [9];   /* 원월물체결가격 */
	char open_price             [9];   /* 시가 */
	char high_price             [9];   /* 고가 */
	char low_price              [9];   /* 저가 */
	char prev_price             [9];   /* 직전가격 */
	char accum_volume           [12];  /* 누적거래량 */
	char accum_trade_amt        [22];  /* 누적거래대금 */
	char final_buy_sell_code    [1];   /* 최종매도매수구분코드 */
	char dyn_upper_limit        [9];   /* 동적상한가 */
	char dyn_lower_limit        [9];   /* 동적하한가 */
	char msg_end_key            [1];   /* 정보분배메세지종료키워드 */
}	CO_A301F;	/* TR: A301F ~ A316F */

/* ************************************************** */
/* 파생 체결 + 우선호가 (우선호가 5단계)
	(파생A) DRV : G701F, G702F, G703F, G706F, G707F, G708F, G709F, G710F, G711F, G712F, G713F, G715F, G716F
*/
/* ************************************************** */
typedef struct
{
	char tr_gbn			    	[5];  /* TR CODE						*/
	char seq_no            [8];   /* 정보분배일련번호 */
	char board_id               [2];   /* 보드ID */
	char session_id             [2];   /* 세션ID */
	char item_code                [12];  /* 종목코드 */
	char info_stock_idx         [6];   /* 정보분배종목인덱스 */
	char trade_time             [12];  /* 매매처리시각 */
	char crprc             [9];   /* 체결가격 */
	char volume                 [9];   /* 거래량 */
	char near_month_crprc  [9];   /* 근월물체결가격 */
	char far_month_price        [9];   /* 원월물체결가격 */
	char open_price             [9];   /* 시가 */
	char high_price             [9];   /* 고가 */
	char low_price              [9];   /* 저가 */
	char prev_price             [9];   /* 직전가격 */
	char accum_volume           [12];  /* 누적거래량 */
	char accum_trade_amt        [22];  /* 누적거래대금 */
	char final_buy_sell_code    [1];   /* 최종매도매수구분코드 */
	char dyn_upper_limit        [9];   /* 동적상한가 */
	char dyn_lower_limit        [9];   /* 동적하한가 */
	char ask1_price             [9];   /* 매도1단계우선호가가격 */
	char bid1_price             [9];   /* 매수1단계우선호가가격 */
	char ask1_qty               [9];   /* 매도1단계우선호가잔량 */
	char bid1_qty               [9];   /* 매수1단계우선호가잔량 */
	char ask1_ord_qty           [5];   /* 매도1단계우선호가주문건수 */
	char bid1_ord_qty           [5];   /* 매수1단계우선호가주문건수 */
	char ask2_price             [9];   /* 매도2단계우선호가가격 */
	char bid2_price             [9];   /* 매수2단계우선호가가격 */
	char ask2_qty               [9];   /* 매도2단계우선호가잔량 */
	char bid2_qty               [9];   /* 매수2단계우선호가잔량 */
	char ask2_ord_qty           [5];   /* 매도2단계우선호가주문건수 */
	char bid2_ord_qty           [5];   /* 매수2단계우선호가주문건수 */
	char ask3_price             [9];   /* 매도3단계우선호가가격 */
	char bid3_price             [9];   /* 매수3단계우선호가가격 */
	char ask3_qty               [9];   /* 매도3단계우선호가잔량 */
	char bid3_qty               [9];   /* 매수3단계우선호가잔량 */
	char ask3_ord_qty           [5];   /* 매도3단계우선호가주문건수 */
	char bid3_ord_qty           [5];   /* 매수3단계우선호가주문건수 */
	char ask4_price             [9];   /* 매도4단계우선호가가격 */
	char bid4_price             [9];   /* 매수4단계우선호가가격 */
	char ask4_qty               [9];   /* 매도4단계우선호가잔량 */
	char bid4_qty               [9];   /* 매수4단계우선호가잔량 */
	char ask4_ord_qty           [5];   /* 매도4단계우선호가주문건수 */
	char bid4_ord_qty           [5];   /* 매수4단계우선호가주문건수 */
	char ask5_price             [9];   /* 매도5단계우선호가가격 */
	char bid5_price             [9];   /* 매수5단계우선호가가격 */
	char ask5_qty               [9];   /* 매도5단계우선호가잔량 */
	char bid5_qty               [9];   /* 매수5단계우선호가잔량 */
	char ask5_ord_qty           [5];   /* 매도5단계우선호가주문건수 */
	char bid5_ord_qty           [5];   /* 매수5단계우선호가주문건수 */
	char total_ask_qty          [9];   /* 매도호가총잔량 */
	char total_bid_qty          [9];   /* 매수호가총잔량 */
	char valid_ask_ord_qty      [5];   /* 매도호가유효건수 */
	char valid_bid_ord_qty      [5];   /* 매수호가유효건수 */
	char msg_end_key            [1];   /* 정보분배메세지종료키워드 */
}	CO_G701F;	/* TR: G701F ~ G716F, 04F/05F 제외 */

/* ************************************************** */
/* 파생 체결 + 우선호가 (우선호가 10단계)
	(파생A) DRV : G704F, G705F
*/
/* ************************************************** */
typedef struct
{
	char tr_gbn			    [5];  /* TR CODE						*/
	char seq_no            [8];   /* 정보분배일련번호 */
	char board_id               [2];   /* 보드ID */
	char session_id             [2];   /* 세션ID */
	char item_code                [12];  /* 종목코드 */
	char info_stock_idx         [6];   /* 정보분배종목인덱스 */
	char trade_time             [12];  /* 매매처리시각 */
	char crprc             [9];   /* 체결가격 */
	char volume                 [9];   /* 거래량 */
	char near_month_crprc  [9];   /* 근월물체결가격 */
	char far_month_price        [9];   /* 원월물체결가격 */
	char open_price             [9];   /* 시가 */
	char high_price             [9];   /* 고가 */
	char low_price              [9];   /* 저가 */
	char prev_price             [9];   /* 직전가격 */
	char accum_volume           [12];  /* 누적거래량 */
	char accum_trade_amt        [22];  /* 누적거래대금 */
	char final_buy_sell_code    [1];   /* 최종매도매수구분코드 */
	char dyn_upper_limit        [9];   /* 동적상한가 */
	char dyn_lower_limit        [9];   /* 동적하한가 */
	char ask1_price             [9];   /* 매도1단계우선호가가격 */
	char bid1_price             [9];   /* 매수1단계우선호가가격 */
	char ask1_qty               [9];   /* 매도1단계우선호가잔량 */
	char bid1_qty               [9];   /* 매수1단계우선호가잔량 */
	char ask1_ord_qty           [5];   /* 매도1단계우선호가주문건수 */
	char bid1_ord_qty           [5];   /* 매수1단계우선호가주문건수 */
	char ask2_price             [9];   /* 매도2단계우선호가가격 */
	char bid2_price             [9];   /* 매수2단계우선호가가격 */
	char ask2_qty               [9];   /* 매도2단계우선호가잔량 */
	char bid2_qty               [9];   /* 매수2단계우선호가잔량 */
	char ask2_ord_qty           [5];   /* 매도2단계우선호가주문건수 */
	char bid2_ord_qty           [5];   /* 매수2단계우선호가주문건수 */
	char ask3_price             [9];   /* 매도3단계우선호가가격 */
	char bid3_price             [9];   /* 매수3단계우선호가가격 */
	char ask3_qty               [9];   /* 매도3단계우선호가잔량 */
	char bid3_qty               [9];   /* 매수3단계우선호가잔량 */
	char ask3_ord_qty           [5];   /* 매도3단계우선호가주문건수 */
	char bid3_ord_qty           [5];   /* 매수3단계우선호가주문건수 */
	char ask4_price             [9];   /* 매도4단계우선호가가격 */
	char bid4_price             [9];   /* 매수4단계우선호가가격 */
	char ask4_qty               [9];   /* 매도4단계우선호가잔량 */
	char bid4_qty               [9];   /* 매수4단계우선호가잔량 */
	char ask4_ord_qty           [5];   /* 매도4단계우선호가주문건수 */
	char bid4_ord_qty           [5];   /* 매수4단계우선호가주문건수 */
	char ask5_price             [9];   /* 매도5단계우선호가가격 */
	char bid5_price             [9];   /* 매수5단계우선호가가격 */
	char ask5_qty               [9];   /* 매도5단계우선호가잔량 */
	char bid5_qty               [9];   /* 매수5단계우선호가잔량 */
	char ask5_ord_qty           [5];   /* 매도5단계우선호가주문건수 */
	char bid5_ord_qty           [5];   /* 매수5단계우선호가주문건수 */
	char ask6_price             [9];   /* 매도6단계우선호가가격 */
	char bid6_price             [9];   /* 매수6단계우선호가가격 */
	char ask6_qty               [9];   /* 매도6단계우선호가잔량 */
	char bid6_qty               [9];   /* 매수6단계우선호가잔량 */
	char ask6_ord_qty           [5];   /* 매도6단계우선호가주문건수 */
	char bid6_ord_qty           [5];   /* 매수6단계우선호가주문건수 */
	char ask7_price             [9];   /* 매도7단계우선호가가격 */
	char bid7_price             [9];   /* 매수7단계우선호가가격 */
	char ask7_qty               [9];   /* 매도7단계우선호가잔량 */
	char bid7_qty               [9];   /* 매수7단계우선호가잔량 */
	char ask7_ord_qty           [5];   /* 매도7단계우선호가주문건수 */
	char bid7_ord_qty           [5];   /* 매수7단계우선호가주문건수 */
	char ask8_price             [9];   /* 매도8단계우선호가가격 */
	char bid8_price             [9];   /* 매수8단계우선호가가격 */
	char ask8_qty               [9];   /* 매도8단계우선호가잔량 */
	char bid8_qty               [9];   /* 매수8단계우선호가잔량 */
	char ask8_ord_qty           [5];   /* 매도8단계우선호가주문건수 */
	char bid8_ord_qty           [5];   /* 매수8단계우선호가주문건수 */
	char ask9_price             [9];   /* 매도9단계우선호가가격 */
	char bid9_price             [9];   /* 매수9단계우선호가가격 */
	char ask9_qty               [9];   /* 매도9단계우선호가잔량 */
	char bid9_qty               [9];   /* 매수9단계우선호가잔량 */
	char ask9_ord_qty           [5];   /* 매도9단계우선호가주문건수 */
	char bid9_ord_qty           [5];   /* 매수9단계우선호가주문건수 */
	char ask10_price            [9];   /* 매도10단계우선호가가격 */
	char bid10_price            [9];   /* 매수10단계우선호가가격 */
	char ask10_qty              [9];   /* 매도10단계우선호가잔량 */
	char bid10_qty              [9];   /* 매수10단계우선호가잔량 */
	char ask10_ord_qty          [5];   /* 매도10단계우선호가주문건수 */
	char bid10_ord_qty          [5];   /* 매수10단계우선호가주문건수 */
	char total_ask_qty          [9];   /* 매도호가총잔량 */
	char total_bid_qty          [9];   /* 매수호가총잔량 */
	char valid_ask_ord_qty      [5];   /* 매도호가유효건수 */
	char valid_bid_ord_qty      [5];   /* 매수호가유효건수 */
	char msg_end_key            [1];   /* 정보분배메세지종료키워드 */
}	CO_G704F;	/* TR: G704, G705F */

/* ************************************************** */
/* 파생우선호가
	(파생A) DRV : B601F, B602F, B603F, B606F, B607F, B608F, B609F, B610F, B611F, B612F, B613F, B615F, B616F
*/
/* ************************************************** */
typedef struct
{
	char tr_gbn			    [5];  /* TR CODE						*/
	char seq_no          [8];  /* 정보분배일련번호 */
	char board_id             [2];  /* 보드ID */
	char session_id           [2];  /* 세션ID */
	char item_code              [12]; /* 종목코드 */
	char info_stock_idx       [6];  /* 정보분배종목인덱스 */
	char trade_time           [12]; /* 매매처리시각 */
	char ask1_price           [9];  /* 매도1단계우선호가가격 */
	char bid1_price           [9];  /* 매수1단계우선호가가격 */
	char ask1_qty             [9];  /* 매도1단계우선호가잔량 */
	char bid1_qty             [9];  /* 매수1단계우선호가잔량 */
	char ask1_ord_qty         [5];  /* 매도1단계우선호가주문건수 */
	char bid1_ord_qty         [5];  /* 매수1단계우선호가주문건수 */
	char ask2_price           [9];  /* 매도2단계우선호가가격 */
	char bid2_price           [9];  /* 매수2단계우선호가가격 */
	char ask2_qty             [9];  /* 매도2단계우선호가잔량 */
	char bid2_qty             [9];  /* 매수2단계우선호가잔량 */
	char ask2_ord_qty         [5];  /* 매도2단계우선호가주문건수 */
	char bid2_ord_qty         [5];  /* 매수2단계우선호가주문건수 */
	char ask3_price           [9];  /* 매도3단계우선호가가격 */
	char bid3_price           [9];  /* 매수3단계우선호가가격 */
	char ask3_qty             [9];  /* 매도3단계우선호가잔량 */
	char bid3_qty             [9];  /* 매수3단계우선호가잔량 */
	char ask3_ord_qty         [5];  /* 매도3단계우선호가주문건수 */
	char bid3_ord_qty         [5];  /* 매수3단계우선호가주문건수 */
	char ask4_price           [9];  /* 매도4단계우선호가가격 */
	char bid4_price           [9];  /* 매수4단계우선호가가격 */
	char ask4_qty             [9];  /* 매도4단계우선호가잔량 */
	char bid4_qty             [9];  /* 매수4단계우선호가잔량 */
	char ask4_ord_qty         [5];  /* 매도4단계우선호가주문건수 */
	char bid4_ord_qty         [5];  /* 매수4단계우선호가주문건수 */
	char ask5_price           [9];  /* 매도5단계우선호가가격 */
	char bid5_price           [9];  /* 매수5단계우선호가가격 */
	char ask5_qty             [9];  /* 매도5단계우선호가잔량 */
	char bid5_qty             [9];  /* 매수5단계우선호가잔량 */
	char ask5_ord_qty         [5];  /* 매도5단계우선호가주문건수 */
	char bid5_ord_qty         [5];  /* 매수5단계우선호가주문건수 */
	char total_ask_qty        [9];  /* 매도호가총잔량 */
	char total_bid_qty        [9];  /* 매수호가총잔량 */
	char valid_ask_ord_qty    [5];  /* 매도호가유효건수 */
	char valid_bid_ord_qty    [5];  /* 매수호가유효건수 */
	char expected_price       [9];  /* 예상체결가 */
	char expected_qty         [9];  /* 예상체결수량 */
	char msg_end_key          [1];  /* 정보분배메세지종료키워드 */	
}	CO_B601F;	/* TR: B601F ~ B616F */

/* ************************************************** */
/* 파생우선호가
	(파생A) DRV : B604F, B605F	
*/
/* ************************************************** */
typedef struct
{
	char tr_gbn			    [5];  /* TR CODE						*/
	char seq_no          [8];  /* 정보분배일련번호 */
	char board_id             [2];  /* 보드ID */
	char session_id           [2];  /* 세션ID */
	char item_code              [12]; /* 종목코드 */
	char info_stock_idx       [6];  /* 정보분배종목인덱스 */
	char trade_time           [12]; /* 매매처리시각 */
	char ask1_price           [9];  /* 매도1단계우선호가가격 */
	char bid1_price           [9];  /* 매수1단계우선호가가격 */
	char ask1_qty             [9];  /* 매도1단계우선호가잔량 */
	char bid1_qty             [9];  /* 매수1단계우선호가잔량 */
	char ask1_ord_qty         [5];  /* 매도1단계우선호가주문건수 */
	char bid1_ord_qty         [5];  /* 매수1단계우선호가주문건수 */
	char ask2_price           [9];  /* 매도2단계우선호가가격 */
	char bid2_price           [9];  /* 매수2단계우선호가가격 */
	char ask2_qty             [9];  /* 매도2단계우선호가잔량 */
	char bid2_qty             [9];  /* 매수2단계우선호가잔량 */
	char ask2_ord_qty         [5];  /* 매도2단계우선호가주문건수 */
	char bid2_ord_qty         [5];  /* 매수2단계우선호가주문건수 */
	char ask3_price           [9];  /* 매도3단계우선호가가격 */
	char bid3_price           [9];  /* 매수3단계우선호가가격 */
	char ask3_qty             [9];  /* 매도3단계우선호가잔량 */
	char bid3_qty             [9];  /* 매수3단계우선호가잔량 */
	char ask3_ord_qty         [5];  /* 매도3단계우선호가주문건수 */
	char bid3_ord_qty         [5];  /* 매수3단계우선호가주문건수 */
	char ask4_price           [9];  /* 매도4단계우선호가가격 */
	char bid4_price           [9];  /* 매수4단계우선호가가격 */
	char ask4_qty             [9];  /* 매도4단계우선호가잔량 */
	char bid4_qty             [9];  /* 매수4단계우선호가잔량 */
	char ask4_ord_qty         [5];  /* 매도4단계우선호가주문건수 */
	char bid4_ord_qty         [5];  /* 매수4단계우선호가주문건수 */
	char ask5_price           [9];  /* 매도5단계우선호가가격 */
	char bid5_price           [9];  /* 매수5단계우선호가가격 */
	char ask5_qty             [9];  /* 매도5단계우선호가잔량 */
	char bid5_qty             [9];  /* 매수5단계우선호가잔량 */
	char ask5_ord_qty         [5];  /* 매도5단계우선호가주문건수 */
	char bid5_ord_qty         [5];  /* 매수5단계우선호가주문건수 */
	char ask6_price           [9];  /* 매도6단계우선호가가격 */
	char bid6_price           [9];  /* 매수6단계우선호가가격 */
	char ask6_qty             [9];  /* 매도6단계우선호가잔량 */
	char bid6_qty             [9];  /* 매수6단계우선호가잔량 */
	char ask6_ord_qty         [5];  /* 매도6단계우선호가주문건수 */
	char bid6_ord_qty         [5];  /* 매수6단계우선호가주문건수 */
	char ask7_price           [9];  /* 매도7단계우선호가가격 */
	char bid7_price           [9];  /* 매수7단계우선호가가격 */
	char ask7_qty             [9];  /* 매도7단계우선호가잔량 */
	char bid7_qty             [9];  /* 매수7단계우선호가잔량 */
	char ask7_ord_qty         [5];  /* 매도7단계우선호가주문건수 */
	char bid7_ord_qty         [5];  /* 매수7단계우선호가주문건수 */
	char ask8_price           [9];  /* 매도8단계우선호가가격 */
	char bid8_price           [9];  /* 매수8단계우선호가가격 */
	char ask8_qty             [9];  /* 매도8단계우선호가잔량 */
	char bid8_qty             [9];  /* 매수8단계우선호가잔량 */
	char ask8_ord_qty         [5];  /* 매도8단계우선호가주문건수 */
	char bid8_ord_qty         [5];  /* 매수8단계우선호가주문건수 */
	char ask9_price           [9];  /* 매도9단계우선호가가격 */
	char bid9_price           [9];  /* 매수9단계우선호가가격 */
	char ask9_qty             [9];  /* 매도9단계우선호가잔량 */
	char bid9_qty             [9];  /* 매수9단계우선호가잔량 */
	char ask9_ord_qty         [5];  /* 매도9단계우선호가주문건수 */
	char bid9_ord_qty         [5];  /* 매수9단계우선호가주문건수 */
	char ask10_price          [9];  /* 매도10단계우선호가가격 */
	char bid10_price          [9];  /* 매수10단계우선호가가격 */
	char ask10_qty            [9];  /* 매도10단계우선호가잔량 */
	char bid10_qty            [9];  /* 매수10단계우선호가잔량 */
	char ask10_ord_qty        [5];  /* 매도10단계우선호가주문건수 */
	char bid10_ord_qty        [5];  /* 매수10단계우선호가주문건수 */
	char total_ask_qty        [9];  /* 매도호가총잔량 */
	char total_bid_qty        [9];  /* 매수호가총잔량 */
	char valid_ask_ord_qty    [5];  /* 매도호가유효건수 */
	char valid_bid_ord_qty    [5];  /* 매수호가유효건수 */
	char expected_price       [9];  /* 예상체결가 */
	char expected_qty         [9];  /* 예상체결수량 */
	char msg_end_key          [1];  /* 정보분배메세지종료키워드 */
}	CO_B604F;	/* TR: B604F, B605F (2개 Only) */

/* ************************************************** */
/* 파생 장운영TS + 우선호가 (우선호가 5단계)
	(파생A) DRV : R101F, R102F, R103F, R106F, R107F, R108F, R109F, R110F, R111F, R112F, R113F, R115F, R116F
*/
/* ************************************************** */
/* 장운영정보 (A701A)
	(증권A) STK : A701S
	(증권C) STK : A702S, A703S, A704S
	(증권B) KSQ : A701Q
	(증권B) KNX : A701X
	(채권A) BND : A701B
	(채권A) SMB : A701M
	(채권A) KTS : A701K
	(채권A) RPO : A701R
	(파생A) DRV : A701F, A702F, A703F, A704F, A705F, A706F, A707F, A708F, A709F, A710F, A711F, A712F, A713F, A715F, A716F
	(일반A) CMD : A701G
	(일반A) ETS : A701E
*/
/* ************************************************** */
typedef struct
{
	char tr_gbn			   		[5];	/* TR CODE					*/
	char seq_no          	[8];	/* 정보분배일련번호 		*/
	char board_id             	[2];	/* 보드ID 					*/
	char session_id           	[2];	/* 세션ID 					*/
	char item_code              	[12];	/* 종목코드 				*/
	char info_stock_idx       	[6];	/* 정보분배종목인덱스 		*/
	char trade_time           	[12];	/* 매매처리시각 			*/
	char event_id             	[3];	/* 보드이벤트ID 			*/
	char event_start_time     	[9];	/* 보드이벤트시작시각 		*/
	char event_group_code     	[5];	/* 보드이벤트적용군코드 	*/
	char suspend_reason       	[3];	/* 거래정지사유코드 		*/
	char msg_end_key          	[1];	/* 정보분배메세지종료키워드 */
}	CO_A701A;	/* TR: A701A는 All을 의미 */
/* A701F ~ A716F, A701S ~ A704S, A701B/A701M/A701K/A701R, G/E/X/Q.. 모든 A7 공통 */

/* ************************************************** */
/* 장운영스캐줄공개
	(증권A) STK : M401S
	(증권C) STK : M402S, M403S, M404S
	(증권B) KSQ : M401Q
	(증권B) KNX : M401X
	(채권A) BND : M401B
	(채권A) SMB : M401M
	(채권A) KTS : M401K
	(채권A) RPO : M401R
	(파생A) DRV : M401F, M402F, M403F, M404F, M405F, M406F, M407F, M408F, M409F, M410F, M411F, M412F, M413F, M415F, M416F
	(일반A) CMD : M401G
	(일반A) ETS : M401E
*/
/* ************************************************** */
typedef struct
{
	char tr_gbn			    [5];  /* TR CODE						*/
	char mkt_prod_grp_id       [3];  /* 장운영상품그룹ID */
	char board_id              [2];  /* 보드ID */
	char event_id              [3];  /* 보드이벤트ID */
	char event_start_time      [9];  /* 보드이벤트시작시각 */
	char event_group_code      [5];  /* 보드이벤트적용군코드 */
	char sess_open_close       [2];  /* 세션개시종료코드 */
	char session_id            [2];  /* 세션ID */
	char item_code               [12]; /* 종목코드 */
	char listed_stock          [12]; /* 상장사종목코드 */
	char product_id            [11]; /* 상품ID */
	char suspend_reason        [3];  /* 거래정지사유코드 */
	char susp_type             [1];  /* 거래정지발생유형코드 */
	char apply_stage           [2];  /* 적용단계 */
	char prc_limit_ext         [1];  /* 기준종목가격제한확대발생코드 */
	char prc_limit_tm          [9];  /* 가격제한확대예정시각 */
	char msg_end_key           [1];  /* 정보분배메세지종료키워드 */
}	CO_M401F;	/* TR: M401F ~ M416F */

/* ************************************************** */
/* 파생 장운영TS + 우선호가 (우선호가  5단계) 단일가 세션, 04F, 05F 제외
	(파생A) DRV : R101F, R102F, R103F, R106F, R107F, R108F, R109F, R110F, R111F, R112F, R113F, R115F, R116F
*/
/* ************************************************** */
typedef struct
{
	char tr_gbn			    [5];  /* TR CODE						*/
	char seq_no          [8];  /* 정보분배일련번호 */
	char board_id             [2];  /* 보드ID */
	char session_id           [2];  /* 세션ID */
	char item_code              [12]; /* 종목코드 */
	char info_stock_idx       [6];  /* 정보분배종목인덱스 */
	char trade_time           [12]; /* 매매처리시각 */
	char event_id             [3];  /* 보드이벤트ID */
	char event_start_time     [9];  /* 보드이벤트시작시각 */
	char event_group_code     [5];  /* 보드이벤트적용군코드 */
	char dyn_upper_limit      [9];  /* 동적상한가 */
	char dyn_lower_limit      [9];  /* 동적하한가 */
	char ask1_price           [9];  /* 매도1단계우선호가가격 */
	char bid1_price           [9];  /* 매수1단계우선호가가격 */
	char ask1_qty             [9];  /* 매도1단계우선호가잔량 */
	char bid1_qty             [9];  /* 매수1단계우선호가잔량 */
	char ask1_ord_qty         [5];  /* 매도1단계우선호가주문건수 */
	char bid1_ord_qty         [5];  /* 매수1단계우선호가주문건수 */
	char ask2_price           [9];  /* 매도2단계우선호가가격 */
	char bid2_price           [9];  /* 매수2단계우선호가가격 */
	char ask2_qty             [9];  /* 매도2단계우선호가잔량 */
	char bid2_qty             [9];  /* 매수2단계우선호가잔량 */
	char ask2_ord_qty         [5];  /* 매도2단계우선호가주문건수 */
	char bid2_ord_qty         [5];  /* 매수2단계우선호가주문건수 */
	char ask3_price           [9];  /* 매도3단계우선호가가격 */
	char bid3_price           [9];  /* 매수3단계우선호가가격 */
	char ask3_qty             [9];  /* 매도3단계우선호가잔량 */
	char bid3_qty             [9];  /* 매수3단계우선호가잔량 */
	char ask3_ord_qty         [5];  /* 매도3단계우선호가주문건수 */
	char bid3_ord_qty         [5];  /* 매수3단계우선호가주문건수 */
	char ask4_price           [9];  /* 매도4단계우선호가가격 */
	char bid4_price           [9];  /* 매수4단계우선호가가격 */
	char ask4_qty             [9];  /* 매도4단계우선호가잔량 */
	char bid4_qty             [9];  /* 매수4단계우선호가잔량 */
	char ask4_ord_qty         [5];  /* 매도4단계우선호가주문건수 */
	char bid4_ord_qty         [5];  /* 매수4단계우선호가주문건수 */
	char ask5_price           [9];  /* 매도5단계우선호가가격 */
	char bid5_price           [9];  /* 매수5단계우선호가가격 */
	char ask5_qty             [9];  /* 매도5단계우선호가잔량 */
	char bid5_qty             [9];  /* 매수5단계우선호가잔량 */
	char ask5_ord_qty         [5];  /* 매도5단계우선호가주문건수 */
	char bid5_ord_qty         [5];  /* 매수5단계우선호가주문건수 */
	char total_ask_qty        [9];  /* 매도호가총잔량 */
	char total_bid_qty        [9];  /* 매수호가총잔량 */
	char valid_ask_ord_qty    [5];  /* 매도호가유효건수 */
	char valid_bid_ord_qty    [5];  /* 매수호가유효건수 */
	char expected_price       [9];  /* 예상체결가 */
	char expected_qty         [9];  /* 예상체결수량 */
	char msg_end_key          [1];  /* 정보분배메세지종료키워드 */
}	CO_R101F;	/* TR: R101F ~ R116F, 04F/05F제외 */

/* ************************************************** */
/* 파생 장운영TS + 우선호가 (우선호가 10단계) 단일가 세션, 04F, 05F 전용
	(파생A) DRV : G704F, G705F
*/
/* ************************************************** */
typedef struct
{
	char tr_gbn			    [5];  /* TR CODE						*/
	char seq_no            [8];   /* 정보분배일련번호 */
	char board_id               [2];   /* 보드ID */
	char session_id             [2];   /* 세션ID */
	char item_code                [12];  /* 종목코드 */
	char info_stock_idx         [6];   /* 정보분배종목인덱스 */
	char trade_time             [12];  /* 매매처리시각 */
	char event_id               [3];   /* 보드이벤트ID */
	char event_start_time       [9];   /* 보드이벤트시작시각 */
	char event_group_code       [5];   /* 보드이벤트적용군코드 */
	char dyn_upper_limit        [9];   /* 동적상한가 */
	char dyn_lower_limit        [9];   /* 동적하한가 */
	char ask1_price             [9];   /* 매도1단계우선호가가격 */
	char bid1_price             [9];   /* 매수1단계우선호가가격 */
	char ask1_qty               [9];   /* 매도1단계우선호가잔량 */
	char bid1_qty               [9];   /* 매수1단계우선호가잔량 */
	char ask1_ord_qty           [5];   /* 매도1단계우선호가주문건수 */
	char bid1_ord_qty           [5];   /* 매수1단계우선호가주문건수 */
	char ask2_price             [9];   /* 매도2단계우선호가가격 */
	char bid2_price             [9];   /* 매수2단계우선호가가격 */
	char ask2_qty               [9];   /* 매도2단계우선호가잔량 */
	char bid2_qty               [9];   /* 매수2단계우선호가잔량 */
	char ask2_ord_qty           [5];   /* 매도2단계우선호가주문건수 */
	char bid2_ord_qty           [5];   /* 매수2단계우선호가주문건수 */
	char ask3_price             [9];   /* 매도3단계우선호가가격 */
	char bid3_price             [9];   /* 매수3단계우선호가가격 */
	char ask3_qty               [9];   /* 매도3단계우선호가잔량 */
	char bid3_qty               [9];   /* 매수3단계우선호가잔량 */
	char ask3_ord_qty           [5];   /* 매도3단계우선호가주문건수 */
	char bid3_ord_qty           [5];   /* 매수3단계우선호가주문건수 */
	char ask4_price             [9];   /* 매도4단계우선호가가격 */
	char bid4_price             [9];   /* 매수4단계우선호가가격 */
	char ask4_qty               [9];   /* 매도4단계우선호가잔량 */
	char bid4_qty               [9];   /* 매수4단계우선호가잔량 */
	char ask4_ord_qty           [5];   /* 매도4단계우선호가주문건수 */
	char bid4_ord_qty           [5];   /* 매수4단계우선호가주문건수 */
	char ask5_price             [9];   /* 매도5단계우선호가가격 */
	char bid5_price             [9];   /* 매수5단계우선호가가격 */
	char ask5_qty               [9];   /* 매도5단계우선호가잔량 */
	char bid5_qty               [9];   /* 매수5단계우선호가잔량 */
	char ask5_ord_qty           [5];   /* 매도5단계우선호가주문건수 */
	char bid5_ord_qty           [5];   /* 매수5단계우선호가주문건수 */
	char ask6_price             [9];   /* 매도6단계우선호가가격 */
	char bid6_price             [9];   /* 매수6단계우선호가가격 */
	char ask6_qty               [9];   /* 매도6단계우선호가잔량 */
	char bid6_qty               [9];   /* 매수6단계우선호가잔량 */
	char ask6_ord_qty           [5];   /* 매도6단계우선호가주문건수 */
	char bid6_ord_qty           [5];   /* 매수6단계우선호가주문건수 */
	char ask7_price             [9];   /* 매도7단계우선호가가격 */
	char bid7_price             [9];   /* 매수7단계우선호가가격 */
	char ask7_qty               [9];   /* 매도7단계우선호가잔량 */
	char bid7_qty               [9];   /* 매수7단계우선호가잔량 */
	char ask7_ord_qty           [5];   /* 매도7단계우선호가주문건수 */
	char bid7_ord_qty           [5];   /* 매수7단계우선호가주문건수 */
	char ask8_price             [9];   /* 매도8단계우선호가가격 */
	char bid8_price             [9];   /* 매수8단계우선호가가격 */
	char ask8_qty               [9];   /* 매도8단계우선호가잔량 */
	char bid8_qty               [9];   /* 매수8단계우선호가잔량 */
	char ask8_ord_qty           [5];   /* 매도8단계우선호가주문건수 */
	char bid8_ord_qty           [5];   /* 매수8단계우선호가주문건수 */
	char ask9_price             [9];   /* 매도9단계우선호가가격 */
	char bid9_price             [9];   /* 매수9단계우선호가가격 */
	char ask9_qty               [9];   /* 매도9단계우선호가잔량 */
	char bid9_qty               [9];   /* 매수9단계우선호가잔량 */
	char ask9_ord_qty           [5];   /* 매도9단계우선호가주문건수 */
	char bid9_ord_qty           [5];   /* 매수9단계우선호가주문건수 */
	char ask10_price            [9];   /* 매도10단계우선호가가격 */
	char bid10_price            [9];   /* 매수10단계우선호가가격 */
	char ask10_qty              [9];   /* 매도10단계우선호가잔량 */
	char bid10_qty              [9];   /* 매수10단계우선호가잔량 */
	char ask10_ord_qty          [5];   /* 매도10단계우선호가주문건수 */
	char bid10_ord_qty          [5];   /* 매수10단계우선호가주문건수 */
	char total_ask_qty          [9];   /* 매도호가총잔량 */
	char total_bid_qty          [9];   /* 매수호가총잔량 */
	char valid_ask_ord_qty      [5];   /* 매도호가유효건수 */
	char valid_bid_ord_qty      [5];   /* 매수호가유효건수 */
	char expected_price         [9];   /* 예상체결가 */
	char expected_qty           [9];   /* 예상체결수량 */
	char msg_end_key            [1];   /* 정보분배메세지종료키워드 */
}	CO_R104F;	/* TR: R104F, R105F */

/* ************************************************** */
/* 파생종목마감
	(파생A) DRV : A601F, A602F, A603F, A604F, A605F, A606F, A607F, A608F, A609F, A610F, A611F, A612F, A613F, A615F, A616F
*/
/* ************************************************** */
typedef struct
{
	char tr_gbn			    [5];  /* TR CODE						*/
	char seq_no            [8];       /* 정보분배일련번호 */
	char board_id               [2];       /* 보드ID */
	char item_code                [12];      /* 종목코드 */
	char info_stock_idx         [6];       /* 정보분배종목인덱스 */
	char undly_asset_close_prc  [9];       /* 종목마감종가 */
	char close_type_cd          [1];       /* 종가구분코드 */
	char accum_volume           [12];      /* 누적거래량 */
	char accum_trade_amt        [22];      /* 누적거래대금 */
	char msg_end_key            [1];       /* 정보분배메세지종료키워드 */
}	CO_A601F;	/* TR: A601F ~ A616F */

/* ************************************************** */
/* 파생 동적상하한가 적용 및 해제
	(파생A) DRV : Q201F, Q202F, Q203F, Q204F, Q206F, Q208F, Q209F, Q210F, Q211F, Q212F, Q216F
*/
/* ************************************************** */
typedef struct
{
	char tr_gbn			    [5];  /* TR CODE						*/
	char seq_no          [8];  /* 정보분배일련번호 */
	char board_id             [2];  /* 보드ID */
	char item_code              [12]; /* 종목코드 */
	char info_stock_idx       [6];  /* 정보분배종목인덱스 */
	char trade_time           [12]; /* 매매처리시각 */
	char dyn_prc_lmt_yn       [1];  /* 동적가격제한설정코드 */
	char dyn_upper_limit      [9];  /* 동적상한가 */
	char dyn_lower_limit      [9];  /* 동적하한가 */
	char msg_end_key          [1];  /* 정보분배메세지종료키워드 */
}	CO_Q201F;	/* TR: Q201F ~ Q216F */

/* ************************************************** */
/* 가격제한폭확대발동
	(파생A) DRV : V101F, V102F, V103F, V104F, V105F, V108F, V109F, V111F, V112F, V113F, V115F, V116F
*/
/* ************************************************** */
typedef struct
{
	char tr_gbn			    [5];  /* TR CODE						*/
	char seq_no          [8];  /* 정보분배일련번호 */
	char board_id             [2];  /* 보드ID */
	char item_code              [12]; /* 종목코드 */
	char info_stock_idx       [6];  /* 정보분배종목인덱스 */
	char prc_limit_tm         [9];  /* 가격확대시각 */
	char prc_lmt_exp_stg_up   [2];  /* 가격제한확대상한단계 */
	char prc_lmt_exp_stg_lo   [2];  /* 가격제한확대하한단계 */
	char upper_limit          [9];  /* 상한가 */
	char lower_limit          [9];  /* 하한가 */
	char msg_end_key          [1];  /* 정보분배메세지종료키워드 */
}	CO_V101F;	/* TR: V101F ~ V116F */

/* ************************************************** */
/* Polling
*/
/* ************************************************** */
typedef struct
{
	char tr_gbn			    [5];  /* TR CODE						*/
	char minute_time        [4];  /* 1분단위시각 */
	char msg_end_key        [1];  /* 정보분배메세지종료키워드 */
}	CO_I2000;	/* TR: I2000, 모든 Port에 있음 */


/* ******************************************************************** */

/* TR정의_채권 */
/*
	TRDESP50101 채권종목정보(1173)
	채권은 종목정보를 시세에서 처리하지않고 RDS에서 수신받아서  처리
	CO_A001_RDS0X => 채권으로 명 한다. 

	CO_A001_RDS01 = TRDESP50101_채권종목정보,
	CO_A001_RDS02 = TRDESP50102_KTS종목정보

	A001K = CO_A001_RDS01 + CO_A001_RDS02

	-- CO_A001_RDS01는 기본으로 모든 정보가 내려오고 KTS도 포함되어 있다. 이중 KTS만 발라내야 한다.
	-- 발라진 CO_A001_RDS01에서 CO_A001_RDS02정보가 더해진것이 종목정보이다.

	<시장구분>
	K : 장내채권
	M : 장내소액채권
	R : 레포
	B : 일반채권
	7 : 채권

	<정보+시장>
	국채시장공통		00K
	장내국채			01K
	소액채권시장공통	00M
	장내소액채권		01M
	레포시장공통		00R
	장내RP(REPO)		01R
	일반채권시장공통	00B
	장내일반채권		01B
	장외채권(호가집중)	077

	- 채권 사용 TR 정의
	A001B : 채권 종목정보
	A001R : REPO 종목정보
	B601K : 일반채권, 국고채권 우선호가
	A301K : 채권 체결
	G701K : 일반채권, 국고채권 체결 + 우선호가

	C401B, C401K : 협의거래 결과
	C401R	REPO : 협의거래 결과
	I601B, I601M, I601K, I601R : 종목이벤트 정보
	G300B : 소매채권분류코드 정보
	S001R : REPO 거래일물가능 정보
	F901B, F901M : 주식관련사채/소액채권 발행정보
	BN01B : 일반채권 분할상환일 정보
	CB01R : REPO 분류정보
	P401B, P401M, P401K : 채권분류별 투자자별 통계
	G001M : 소액채권 신고시장 수익률
	PA01K :	국고채권 WIT 확정정보
	PB01K : 국고채권 단기수익률
	PC01K : 국고채권 평균수익률
	J9077 : 채권표준코드 발행정보
	JA077 : 채권표준코드 신용평가정보
	JB077 : 채권표준코드 발행정보 TEXT
	A701K : 장운영TS
	A601K : 채권 종목마감
	I2000 : Polling
	M401B, M401M, M401K, M401R : 장운영스케줄공개
	R301B, R301M, R301K, R301R : 회원제재해제공개
	R401B : 일반채권 결제적용 기준환율 공개
		
	G1000 : 채권지수_시장지수 그룹코드
		    채권지수_시장지수 만기코드
	G2000 : 채권지수_시장지수 지수
	JA000 : 채권지수_프라임지수 그룹코드
		    채권지수_프라임지수 만기코드
	J4000 : 채권지수_프라임지수 지수
	IG000 : 채권지수_KRX 채권지수(종가형지수)
	K1000 : 채권지수_KTB지수 지수
	K8000 : 채권지수_KTB지수 TermStructure

										시세	RDS	인터페이스ID	길이	
	종목정보	A001K	KTS, 국채				O					350
				A001B	장내일반채권	O		?	IFMSBTD0016		308	
				A001M	장내소액채권			?			
				A001R	장내RP_REPO		O		?	IFMSBTD0017		100	
								
	일반채권, 국고채권 우선호가	
				B601K	KTS, 국채		O			IFMSRPD0023		462		CO_B601K
				B601B	장내일반채권	O			상동			상동	상동		
				B601M	장내소액채권	O			IFMSRPD0024		882		B601M
				B601R	장내RP_REPO		O			IFMSRPD0025		1387	B601R
								
	체결		A301K	KTS, 국채		O			IFMSRPD0027		223		CO_A301K
				A301B	장내일반채권	O				
				A301M	장내소액채권	O				
				A301R	장내RP_REPO					
								
	체결+우선호가	
				G701K	KTS, 국채		O			IFMSRPD0029		643		CO_G701K
				G701B	장내일반채권	O			상동			상동	상동	
				G701M	장내소액채권	O			IFMSRPD0030		1063	G701M
				G701R	장내RP_REPO		O			IFMSRPD0031		1586	G701R
								
	장운영TS	A701K	KTS, 국채		O			IFMSRPD0008		68		CO_A701K
				A701B	장내일반채권	O			상동			상동	상동
				A701M	장내소액채권	O			상동            상동    상동
				A701R	장내RP_REPO		O			상동            상동    상동
								
	채권종목마감, 보드별 종목마감시 제공	
				A601K	KTS, 국채		O			IFMSRPD0032		57		CO_A601K
				A601B	장내일반채권	O			상동            상동    상동
				A601M	장내소액채권	O			상동            상동    상동
				A601R	장내RP_REPO		O			상동            상동    상동
								
	장운영스케줄공개	
				M401K	KTS, 국채		O			IFMSRPD0019		83		CO_M401K
				M401B	장내일반채권	O			상동            상동    상동
				M401M	장내소액채권	O			상동            상동    상동
				M401R	장내RP_REPO		O			상동            상동    상동


	- 채권 Port 01
	A701K : 68  장운영TS
	B601K : 462 일반채권, 국고채권 우선호가
	A301K : 223 채권 체결
	G701K : 643 일반채권, 국고채권 체결 + 우선호가
	A601K : 57  채권 종목마감
	I2000 : 10  Polling, Log만 File은 처리안함

                               운영/테스트/부산테스트
	K =>   8M, 233.38.231.152, 10402/11402/12402, TR동일
	B =>   8M, 233.38.231.152, 10403/11403/12403, TR동일
	M =>   8M, 233.38.231.152, 10404/11404/12404, TR동일
	R =>   8M, 233.38.231.152, 10405/11405/12405, TR다름 (A7/B6/G7/A6/I2)

	K => 512K, 233.38.231.162, 10412/11412/12412, TR동일
	B => 512K, 233.38.231.162, 10413/11413/12413, TR추가 (OA)
	M => 512K, 233.38.231.162, 10414/11414/12414, TR추가 (OA)
	R => 512K, 233.38.231.162, 10415/11415/12415, TR다름 (A7/B6/G7/A6/I2)

	- 채권 Port 02
	A001B                         채권 종목정보, RDS정보의 종목정보를 사용
	A001R                         REPO 종목정보, RDS정보의 종목정보를 사용
	C401B, C401K                  협의거래 결과                 
	C401R                         REPO 협의거래 결과            
	I601B, I601M, I601K, I601R    종목이벤트 정보               
	G300B                         소매채권분류코드 정보         
	S001R                         REPO 거래일물가능 정보        
	F901B, F901M                  주식관련사채/소액채권 발행정보
	BN01B                         일반채권 분할상환일 정보      
	CB01R                         REPO 분류정보                 
	P401B, P401M, P401K           채권분류별 투자자별 통계      
	G001M                         소액채권 신고시장 수익률      
	PA01K                         국고채권 WIT 확정정보         
	PB01K                         국고채권 단기수익률           
	PC01K                         국고채권 평균수익률           
	J9077                         채권표준코드 발행정보         
	JA077                         채권표준코드 신용평가정보     
	JB077                         채권표준코드 발행정보 TEXT    
	I2000                         Polling                       

	IP/PORT 동일 (512K & 8M)
	512K => 233.38.231.151, 10401/11401/12401
	8M   => 233.38.231.151, 10401/11401/12401

	- 채권 Port 03
	M401B, M401M, M401K, M401R  장운영스케줄공개               
	R301B, R301M, R301K, R301R  회원제재해제공개               
	R401B                       일반채권 결제적용 기준환율 공개
	I2000                       Polling  

	IP/PORT 동일 (512K & 8M)
	512K => 233.38.231.153, 10406/11406/12406
	8M   => 233.38.231.153, 10406/11406/12406

	- 채권 Port 04
	G1000       채권지수_시장지수 그룹코드       
				채권지수_시장지수 만기코드       
	G2000       채권지수_시장지수 지수           
	JA000       채권지수_프라임지수 그룹코드     
				채권지수_프라임지수 만기코드     
	J4000       채권지수_프라임지수 지수         
	IG000       채권지수_KRX 채권지수(종가형지수)
	I2000       Polling                          

	IP/PORT 동일 (512K & 8M)
	512K => 233.38.231.154, 10407/11407/12407
	8M   => 233.38.231.154, 10407/11407/12407

	- 채권 Port 05
	K1000   채권지수_KTB지수 지수         
	K8000   채권지수_KTB지수 TermStructure
	I2000   Polling                       

	IP/PORT 동일 (512K & 8M)
	512K => 233.38.231.155, 10408/11408/12408
	8M   => 233.38.231.155, 10408/11408/12408



	- 선물(금융상품선물(국채,금리,통화) 16F 기준 사용TR
	A001F(10301/11301) : 종목정보
	A301F : 파생체결
	G701F : 파생체결G7 (우선호가  5단계) 04F & 05F(제외)
	B601F : 파생우선호가 (우선호가  5단계) 04F & 05F(제외)
    A701A : 장운영정보(모든 A7공통.. All)
	M401A : 장운영스캐쥴
	R101F : 파생 장운영TS + 우선호가 (우선호가  5단계) 단일가 세션, 04F, 05F 제외

	G704F : 파생체결G7 (우선호가 10단계) 04F & 05F
	B604F : 파생우선호가 (우선호가 10단계) 04F & 05F
	R104F : 파생 장운영TS + 우선호가 (우선호가 10단계) 단일가 세션, 04F, 05F 포함

	A601F : 파생종목마감
	Q201F : 파생동적상하한가 적용및해제(05없음)
	V101F : 가격제한폭확대발동
	I2000 : Polling Data

	- 제외 TR
	O606F : 배분정보
	IF06F : 그룹호가접수중지/해제 공개
	B201F : 파생시세 SnapShot  5호가(04,05제외)
	B204F : 파생시세 SnapShot 10호가(04,05전용)
	C401F : 파생협의거래결과(장종료후)
	M701F : Eurex연계 장개시전 협의거래(08:45 1회)
	H101F : 파생 상품별 투자자별 통계 (08:45~15:45, [30초 주기]), 서비스상품별 상이
	H101F : 파생 상품별 투자자별 통계 (전일확정치:07:30, 당일확정치:16:30), 서비스상품별 상이
	H201F : 파생종목 미결제약정수량 데이터 (전일확정치:장개시전, 장중:10초주기,당일확정치:장종료후)
	H301F : 선물종목 정산가격 데이터 (장종료후)

	- 선물 Port (모든 Port에는 I2000이 있다)
	A001F 100M : 10301 / 11301 , 233.38.231.91
           12M : 10301 / 11301 , 233.38.231.91

	M401F
		  100M : 10315 / 11315 , 233.38.231.93
		   12M : 10315 / 11315 , 233.38.231.93

	A7/O6/B6/A3/G7/R1/A6/Q2/V1/I2
	A701F 100M :               , 233.38.231.92 
				* 파생시장 시세 송신 port 분리 및 종목별 보드별 일련번호 제공
				- 10302(코스피200선물)
				- 10303(코스닥150선물)
				- 10304(개별주식선물)
				- 10305(금융상품선물(국채,금리,통화))
				- 10306(일반상품선물(금, 돈육(휴면)))
*/


/* ******************************************************** */
/* 	CO_A001_RDS01;		// 채권종목정보_RDS, TRDESP50101	*/
/* ******************************************************** */
typedef struct
{
	char seq_no [11];                  /* 메세지일련번호 */
	char tr_code [11];                     /* 트랜잭션코드 */
	char send_date [8];                    /* 전송일자 */
	char biz_date [8];                     /* 영업일자 */
	char mkt_id [3];                       /* 시장ID */
	char item_code [12];                     /* 종목코드 */
	char item_kor_nm [80];                 /* 종목명 */
	char item_short_nm [40];               /* 종목약명 */
	char item_eng_nm [80];                 /* 종목영문명 */
	char item_eng_short_nm [40];           /* 종목영문약명 */
	char me_grp_no [2];                    /* ME그룹번호 */
	char issue_org_cd [5];                 /* 발행기관코드 */
	char bond_Ipo_gbn_cd [1];              /* 채권상장구분코드 */
	char bond_class_cd [6];                /* 채권분류코드 */
	char bond_type_cd [2];                 /* 채권유형코드 */
	char local_bond_gbn_cd [1];            /* 지방채구분코드 */
	char bond_guar_gbn_cd [1];             /* 채권보증구분코드 */
	char pay_guar_rate [13];               /* 지급보증율 */
	char special_bond_type_cd [1];         /* 특이채권유형코드 */
	char opt_bond_cd [1];                  /* 옵션부사채코드 */
	char int_pay_meth_cd [2];              /* 이자지급방법코드 */
	char risk_bond_redemp_type_cd [1];     /* 리스크채권상환유형코드 */
	char bond_issue_meth_cd [3];           /* 채권발행방법코드 */
	char asset_liqui_gbn_cd [2];           /* 자산유동화구분코드 */
	char debt_repay_pri_gbn_cd [1];        /* 채무변제순위구분코드 */
	char list_date [8];                    /* 상장일자 */
	char issue_date [8];                   /* 발행일자 */
	char redemp_date [8];                  /* 상환일자 */
	char sales_date [8];                   /* 매출일 */
	char first_int_pay_date [8];           /* 최초이자지급일자 */
	char bond_issue_rate [13];             /* 채권발행율 */
	char face_int_rate [14];               /* 표면이자율 */
	char int_pay_calc_months [4];          /* 이자지급계산월수 */
	char coupon_pay_meth_cd [1];           /* 이표지급방법코드 */
	char int_day_pay_gbn_cd [1];           /* 이자일지급기준구분코드 */
	char int_month_end_gbn_cd [1];         /* 이자월말구분코드 */
	char int_unit_less_proc_cd [1];        /* 이자원단위미만처리코드 */
	char int_pay_unit_months [4];          /* 이자지급단위월수 */
	char bond_sales_type_cd [1];           /* 채권매출형태코드 */
	char bond_pre_sales_int_pay_meth_cd [1]; /* 채권선매출이자지급방법코드 */
	char issue_amt [22];                   /* 발행금액 */
	char list_amt [22];                    /* 상장금액 */
	char bulk_amt_confirm_yn [1];          /* 일괄금액확정여부 */
	char curr_gbn_cd [1];                  /* 통화구분코드 */
	char mat_redemp_rate [13];             /* 만기상환비율 */
	char guar_yield [13];                  /* 보장수익률 */
	char guar_yield_apply_date [8];        /* 보장수익률적용일자 */
	char add_yield [13];                   /* 추가수익률 */
	char add_yield_apply_date [8];         /* 추가수익률적용일자 */
	char facility_fund [22];               /* 시설자금 */
	char operation_fund [22];              /* 운영자금 */
	char refinance_fund [22];              /* 차환자금 */
	char etc_fund [22];                    /* 기타자금 */
	char name_yn [1];                      /* 기명여부 */
	char tax_yn [1];                       /* 과세여부 */
	char manage_org_cd [4];                /* 주관기관코드 */
	char pay_guar_org_cd [4];              /* 지급보증기관코드 */
	char trustee_org_cd [4];               /* 수탁기관코드 */
	char register_org_cd [4];              /* 등록기관코드 */
	char prin_int_pay_agent_org_cd [4];    /* 원리금지급대행기관코드 */
	char bond_short_cd [9];                /* 채권단축코드 */
	char bond_delist_reason_cd [2];        /* 채권상장폐지사유코드 */
	char bond_delist_date [8];             /* 채권상장폐지일자 */
	char stk_rel_bond_right_gbn_cd [2];    /* 주식관련사채권리구분코드 */
	char target_item_cd [12];              /* 대상종목코드 */
	char target_item_nm [60];              /* 대상종목명 */
	char stk_rel_bond_right_exe_prc [11];  /* 주식관련사채권리행사가격 */
	char exe_rate [7];                     /* 행사비율 */
	char exe_start_date [8];               /* 행사개시일자 */
	char exe_end_date [8];                 /* 행사종료일자 */
	char cash_pay_rate [4];                /* 현금납입비율 */
	char substitute_pay_rate [4];          /* 대용납입비율 */
	char claim_org_cd [4];                 /* 청구기관코드 */
	char div_base_date_gbn_cd [1];         /* 배당기산일구분코드 */
	char profit_part_accum_yn [1];         /* 이익참가누적여부 */
	char new_shr_sub_right_after_item_cd [12]; /* 신주인수권행사이후종목코드 */
	char split_redemp_type_gbn_cd [1];     /* 분할상환유형구분코드 */
	char equal_redemp_amt [22];            /* 균등상환액 */
	char grace_months [4];                 /* 거치개월수 */
	char redemp_period_int_gbn_cd [1];     /* 상환기간이자구분코드 */
	char split_redemp_cnt [5];             /* 분할상환횟수 */
	char int_rate_dec_etc_base_rate_nm [20]; /* 이자율결정기타기준금리명 */
	char add_int_rate [10];                /* 가산금리 */
	char upper_face_int_rate [14];         /* 상한표면이자율 */
	char lower_face_int_rate [14];         /* 하한표면이자율 */
	char trade_susp_yn [1];                /* 거래정지여부 */
	char suspend_reason [3];                  /* 거래정지사유코드 */
	char small_trade_type_cd [3];          /* 소액매매종류코드 */
	char prev_int_pay_date [8];            /* 전기이자지급일자 */
	char next_int_pay_date [8];            /* 차기이자지급일자 */
	char substitute_prc [11];              /* 대용가격 */
	char fx_apply_substitute_prc [11];     /* 환율적용대용가격 */
	char bond_close_prc [11];              /* 채권종가 */
	char close_yield [13];                 /* 종가수익률 */
	char close_form_date [8];              /* 종가형성일자 */
	char if_trade_vol [16];                /* I/F거래량 */
	char base_prc [11];                    /* 기준가격 */
	char special_issue_cond_content [60];  /* 특이발행조건내용 */
	char bank_holiday_int_pay_dec_cd [1];  /* 은행휴무일이자지급결정코드 */
	char new_cap_sec_yn [1];               /* 신종자본증권여부 */
	char cond_cap_sec_type_cd [1];         /* 조건부자본증권유형코드 */
	char face_int_rate_confirm_yn [1];     /* 표면이자율확정여부 */
	char bond_strip_gbn_cd [1];            /* 채권스트립구분코드 */
	char target_org_bond_cd [12];          /* 대상원본채권코드 */
	char strip_unsep_bal [22];             /* 스트립미분리잔액 */
	char price_link_yn [1];                /* 물가연동여부 */
	char issue_date_ref_idx [11];          /* 발행일참조지수 */
	char face_int_rate_dec_base_rate_cd [2]; /* 표면이자율결정기준금리코드 */
	char int_rate_dec_base_date [8];       /* 이자율결정기준일자 */
	char bond_unit_day_int_base_gbn_cd [1]; /* 채권단수일이자기준구분코드 */
	char bank_holiday_int_base_rate_cd [1]; /* 은행휴무일이자기준금리코드 */
	char bank_holiday_int_comp_pass_int_rate [14]; /* 은행휴무일이자대비경과이자율 */
	char bank_holiday_prin_pay_date_dec_cd [1]; /* 은행휴무원금지급일결정코드 */
	char bank_holiday_prin_base_rate_cd [1]; /* 은행휴무일원금기준금리코드 */
	char bank_holiday_prin_pass_int_rate [14]; /* 은행휴무일원금경과이자율 */
	char arrange_trade_yn [1];             /* 정리매매여부 */
	char retail_bond_class_cd [2];         /* 소매채권분류코드 */
	char short_sale_limit_yn [1];          /* 공매도제한대상여부 */
	char upper_limit [11];                 /* 상한가 */
	char lower_limit [11];                 /* 하한가 */
	char prc_unit_rule_id [10];            /* 가격단위규칙ID */
	char invest_caution_bond_gbn_cd [1];  /* 투자유의채권구분코드 */
	char split_redemp_date [8];            /* 분할상환일자 */
}	CO_A001_RDS01;		/* TR: 채권종목정보_RDS, TRDESP50101 */

/* ******************************************************** */
/* 	CO_A001_RDS02;		// KTS종목정보_RDS, TRDESP50101		*/
/* DB처리시 Key는 영업일자+종목코드+보드ID 					*/
/* ******************************************************** */
typedef struct
{
	char seq_no					 [11];   /* 메세지일련번호	*/
	char tr_code 					 [11];   /* 트랜잭션코드	*/
	char send_date                    [8];   /* 전송일자		*/
	char biz_date                     [8];   /* 영업일자		*/
	char undly_asset_mkt_id           [3];   /* 시장ID, "KTS"	*/
											 /* BND:채권
												KTS:KTS
												SMB:소액채권
												RPO:REPO		*/
	char board_id                     [2];   /* 보드ID			*/
											 /* 채권 보드ID
												1) 일반호가 : 정규장, G1 (일반/소액/KTS/REPO)
												2) 신고매매 : 정규장, S1 (일반/REPO/KTS)
												 			  	    , SA (REPO/국채금융REPO 신고매매)
												 			  	    , SB (REPO/    정부REPO 신고매매)
												3) 발 행 전 : 정규장, WS (KTS/WIT신고매매)	
												4) RFQ		: 정규장, R1 (KTS/REPO), 협의매매	*/
	char item_code                      [12];  /* 종목코드		*/
	char prev_day_close               [11];  /* 종가			*/
											 /* 소수점 미만 둘째 자리의 값은 0으로 고정(소수점 1자리까지만 사용) */
											 /* 01234567.89		1_7_1_2(11)
												부호(1), 양수만 사용, 숫자0이외 표시불가
												정수(7), 
												소수점(1)
												소수(2)	*/
	char close_yield                  [13];  /* 종가수익률		*/
											 /* 당일 형성된 최종 채권수익률을 의미함 */
											 /* +-12345.678901	1_5_1_6(13)
												부호(1), 양수는 0, 음수는 -
												정수(5), 정수5자리
												소수점(1), 
												소수(2), 소수점 3자리까지만 유효 나머지는 0	*/
	char base_prc                     [11];  /* 기준가격,기준가액*/
											 /* 당일 매매의 기준이 되는 가격
												상한가, 하한가 산출의 기준이 되는 가격
												소수점 미만 둘째 자리의 값은 0으로 고정(소수점 1자리까지만 사용) */
											 /* 01234567.89
												부호(1), 양수만 사용, 숫자0이외 표시불가
												정수(7), 
												소수점(1)
												소수(2)	*/
	char bond_list_gbn_cd             [1];   /* 국채종목구분코드*/
											 /* W:발행일전거래(WIT), 2024년 10월 이후 종목정보에 없음
												0:선매출('15.03.16)
												1:지표
												2:경과 */
	char bond_class_cd                [3];   /* 국채종류코드	*/
											 /* 025:양곡채, (삭제됨)
												027:외평채
												035:국고채
												101:통안채
												526:예보채
												835:국고변동
												935:국고물가
												C35:스트립국고채('15.03.16) */
	char trade_susp_yn                [1];   /* 거래정지여부	*/
											 /* Y:거래정지, N:거래재개 */
	char suspend_reason               [3];   /* 거래정지사유코드*/
	char item_kor_nm                  [80];  /* 종목한글명		*/
	char item_short_nm                [40];  /* 종목한글약명	*/
	char item_eng_nm                  [80];  /* 종목영문명		*/
	char item_eng_short_nm            [40];  /* 종목영문약명	*/
	char price_link_yn                [1];   /* 물가연동여부	*/
											 /* Y-Yes, N-No		*/
	char bond_strip_gbn_cd            [1];   /* 스트립시장조성여부*/
											 /* Y: 스트립 시장조성여부 해당 N: 미해당 */
	char prc_unit_rule_id             [10];  /* 가격단위규칙ID	*/
											 /* 호가를 제출할 수 있는 가격의 최소단위를 제한하는 규칙을 식별하기 위한 ID */
											 /* 2025.03.04 제도변경건 추가
* '채권일반호가입력', '채권조성호가입력' 전문 호가 제출 시 채권종목정보 전문의 '가격단위규칙ID'에 따른 거부처리 로직이 있어야 함
   - A1   (  1원/POINT) :   1원/POINT 미만 단위의 가격으로 호가 제출 시 거부처리  예) 00009999.50
   - A0.5 (0.5원/POINT) : 0.5원/POINT 미만 단위의 가격으로 호가 제출 시 거부처리  예) 00009999.40
   - A0.1 (0.1원/POINT) : 0.1원/POINT 미만 단위의 가격으로 호가 제출 시 거부처리  예) 00009999.05				*/

}	CO_A001_RDS02;		/* TR: KTS종목정보_RDS,  TRDESP50102 */

/* 
	□ 채권일반호가 정정/취소 전문의 유효성 검증을 강화하여 프로토콜 상의 정의된 값과 다르게 입력시 호가 거부 안내  

	 - 시행시기 : 차세대시스템 가동('23.1월) 
	 - 대상 : 채권일반호가입력( [정정]TCHODR40002 [취소]TCHODR40003 )
	 - 정의사항 

	ORDER_TYPE_CODE( 호가유형코드 )
	 - [취소]호가는 SPACE로 입력( 기존 오입력사례 : [2] )

	ORDER_CONDITION_CODE( 호가조건코드 )
	 - [취소]호가는 SPACE로 입력( 기존 오입력사례 : [0] )

	ASK_TYPE_CODE( 매도유형코드 )
	 - [정정][취소]호가의 경우 SPACE로 입력( 기존 오입력사례 : [00], [01] )

	TRUST_PRINCIPAL_TYPE_CODE( 위탁자기구분코드 )
	 - [취소]호가는 SPACE로 입력( 기존 오입력사례 : [10],[30] )

	TRUST_COMPANY_NUMBER( 위탁사번호 )
	 - [정정][취소]호가의 경우 SPACE로 입력

	ACCOUNT_TYPE_CODE( 계좌구분코드 )
	 - [정정][취소]호가의 경우 SPACE로 입력( 기존 오입력사례 : [00], [01] )

	COUNTRY_CODE( 국가코드 )
	 - [정정][취소]호가의 경우 SPACE로 입력( 기존 오입력사례 : [410], [000], [001] )

	INVESTOR_TYPE_CODE( 투자자구분코드 )
	 - [정정][취소]호가의 경우 SPACE로 입력( 기존 오입력사례 : [0000], [1000], [3000], [8000] )

	FOREIGN_INVESTOR_TYPE_CODE( 외국인투자자구분코드 )
	 - [정정][취소]호가의 경우 SPACE로 입력( 기존 오입력사례 : [00], [01] )

	MM_ORD_TP_CD( 시장조성자호가구분코드 )
	 - [취소]호가는 [0]으로 입력( 기존 오입력사례 : [3] )
*/

/* ******************************************************** */
/* 파생종목정보 RDS, 종목별 가격조건을 체크하기 위해서 필요 */
/* ******************************************************** */
typedef struct
{
	char seq_no               [11]; /* 메세지일련번호 */
	char tr_code                  [11]; /* 트랜잭션코드 */
	char mkt_id                   [3];  /* 시장ID */
	char send_date                [8];  /* 전송일자 */
	char biz_date                 [8];  /* 영업일자 */
	char sec_grp_id               [2];  /* 증권그룹ID */
	char undly_asset_id           [3];  /* 기초자산ID */
	char undly_asset_cd           [2];  /* 기초자산코드 */
	char product_id               [11]; /* 상품ID */
	char item_code                  [12]; /* 종목코드 */
	char mkt_kor_nm               [80]; /* 시장한글명 */
	char mkt_eng_nm               [80]; /* 시장영문명 */
	char sec_grp_kor_nm           [80]; /* 증권그룹한글명 */
	char sec_grp_eng_nm           [80]; /* 증권그룹영문명 */
	char undly_asset_kor_nm       [80]; /* 기초자산한글명 */
	char undly_asset_eng_nm       [80]; /* 기초자산영문명 */
	char item_short_cd            [9];  /* 종목단축코드 */
	char item_kor_nm              [80]; /* 종목한글명 */
	char item_eng_nm              [80]; /* 종목영문명 */
	char item_eng_short_nm        [40]; /* 종목영문약명 */
	char list_date                [8];  /* 상장일자 */
	char list_type_cd             [1];  /* 상장유형코드 */
	char me_grp_no                [2];  /* ME그룹번호 */
	char settl_mm                 [6];  /* 결제월 */
	char last_trade_date          [8];  /* 최종거래일자 */
	char finl_sett_date           [8];  /* 최종결제일자 */
	char exp_date                 [8];  /* 만기일자 */
	char trade_unit               [22]; /* 거래단위 */
	char trade_mult               [22]; /* 거래승수 */
	char strike_prc               [18]; /* 행사가격 */
	char atm_type_cd              [1];  /* ATM구분코드 */
	char view_strike_prc          [18]; /* 조회용행사가격 */
	char arrange_trade_yn         [1];  /* 정리매매여부 */
	char trade_susp_yn            [1];  /* 거래정지여부 */
	char suspend_reason           [3];  /* 거래정지사유코드 */
	char susp_start_date          [8];  /* 거래정지개시일자 */
	char susp_end_date            [8];  /* 거래정지종료일자 */
	char finl_sett_meth_cd        [1];  /* 최종결제방법코드 */
	char right_type_cd            [1];  /* 권리유형코드 */
	char right_ex_type_cd         [1];  /* 권리행사유형코드 */
	char adj_type_cd              [1];  /* 조정구분코드 */
	char mmth_type_cd             [1];  /* 월물구분코드 */
	char spr_base_item_type_cd    [1];  /* 스프레드기준종목구분코드 */
	char spr_comp_item_cd1        [12]; /* 스프레드구성종목코드1 */
	char spr_comp_item_cd2        [12]; /* 스프레드구성종목코드2 */
	char undly_asset_item_cd      [12]; /* 기초자산종목코드 */
	char mkt_prod_grp_id          [3];  /* 장운영상품그룹ID */
	char remain_days              [8];  /* 잔존일수 */
	char mkt_make_type_cd         [1];  /* 시장조성구분코드 */
	char last_exec_time           [9];  /* 최종체결시각 */
	char first_match_date         [8];  /* 최초체결일자 */
	char lim_ord_cancel_cond_cd   [5];  /* 지정가호가취소조건코드 */
	char mkt_ord_cancel_cond_cd   [5];  /* 시장가호가취소조건코드 */
	char cond_lim_ord_cancel_cd   [5];  /* 조건부지정가취소호가조건코드 */
	char best_lim_ord_cancel_cd   [5];  /* 최유리지정가취소호가조건코드 */
	char pri_lim_ord_cancel_cd    [5];  /* 최우선지정가취소호가조건코드 */
	char upper_limit              [11]; /* 상한가 */
	char lower_limit              [11]; /* 하한가 */
	char base_prc                 [11]; /* 기준가격 */
	char base_prc_type_cd         [2];  /* 기준가격구분코드 */
	char base_prc_max_theo_prc    [16]; /* 기준가격적용최대이론가격 */
	char settl_prc                [18]; /* 정산가격 */
	char sett_prc_type_cd         [2];  /* 정산가격구분코드 */
	char mgn_base_prc             [18]; /* 증거금기준가격 */
	char mgn_base_prc_type_cd     [2];  /* 증거금기준가격구분코드 */
	char close_prc                [11]; /* 종가 */
	char close_type_cd            [1];  /* 종가구분코드 */
	char fut_circuit_brk_up       [11]; /* 선물CIRCUIT_BREAKERS상한가 */
	char fut_circuit_brk_lo       [11]; /* 선물CIRCUIT_BREAKERS하한가 */
	char if_prc_sign_use_yn       [1];  /* I/F가격부호사용여부 */
	char if_prc_int_valid_digit   [3];  /* I/F가격정수유효자리수 */
	char if_prc_dec_valid_digit   [3];  /* I/F가격소수유효자리수 */
	char liquidity_mgmt_yn        [1];  /* 유동성관리여부 */
	char mkt_closed_yn            [1];  /* 휴장여부 */
	char mkt_closed_reason_cd     [2];  /* 휴장사유코드 */
	char real_time_prc_lmt_yn     [1];  /* 실시간가격제한여부 */
	char if_real_time_up_intv     [11]; /* I/F실시간상한가간격 */
	char if_real_time_lo_intv     [11]; /* I/F실시간하한가간격 */
	char neg_blk_trd_base_item    [12]; /* 협의대량매매기준종목코드 */
	char prc_lmt_exp_dir_cd       [1];  /* 가격제한확대적용방향코드 */
	char prc_lmt_finl_stg         [3];  /* 가격제한최종단계 */
	char if_up_qty                [16]; /* I/F상한수량 */
	char if_lo_qty                [16]; /* I/F하한수량 */
	char if_neg_blk_trd_up_qty    [16]; /* I/F협의대량매매상한수량 */
	char if_neg_blk_trd_lo_qty    [16]; /* I/F협의대량매매하한수량 */
	char sett_week                [2];  /* 결제주 */
	char base_prod_id             [11]; /* 기준상품ID */
	char sub_prod_id              [11]; /* 부대상품ID */
	char base_prod_item_cnt       [6];  /* 기준상품 종목수 */
	char sub_prod_item_cnt        [6];  /* 부대상품 종목수 */
	char dormant_yn               [1];  /* 휴면여부 */
	char dormant_desig_date       [8];  /* 휴면지정일자 */
	char prc_unit_rule_id         [10]; /* 가격단위규칙ID */
}	A006F_RDS;		/* TR: 파생 종목정보 TRDESP01001 */


/* **************************************************** */
/* 채권 우선호가(B601K, KTS) 462 byte						*/
/* **************************************************** */
typedef struct
{
	char tr_gbn			    	[5];	/* TR CODE			*/
	char seq_no           	[8];	/* 정보분배일련번호 */
	char board_id              	[2];	/* 보드ID */
	char session_id            	[2];	/* 세션ID */
	char item_code               	[12];	/* 종목코드 */
	char trade_time            	[12];	/* 매매처리시각 */
	char ask1_price            	[11];	/* 매도1단계우선호가가격 */
	char bid1_price            	[11];	/* 매수1단계우선호가가격 */
	char bond_ask1_remain_vol  	[15]; /* 채권매도1단계우선호가잔량 */
	char bond_bid1_remain_vol  	[15]; /* 채권매수1단계우선호가잔량 */
	char ask1_yield            	[13]; /* 매도1단계우선호가수익률 */
	char bid1_yield            	[13]; /* 매수1단계우선호가수익률 */
	char ask2_price            	[11]; /* 매도2단계우선호가가격 */
	char bid2_price            	[11]; /* 매수2단계우선호가가격 */
	char bond_ask2_remain_vol  	[15]; /* 채권매도2단계우선호가잔량 */
	char bond_bid2_remain_vol  	[15]; /* 채권매수2단계우선호가잔량 */
	char ask2_yield            	[13]; /* 매도2단계우선호가수익률 */
	char bid2_yield            	[13]; /* 매수2단계우선호가수익률 */
	char ask3_price            	[11]; /* 매도3단계우선호가가격 */
	char bid3_price            	[11]; /* 매수3단계우선호가가격 */
	char bond_ask3_remain_vol  	[15]; /* 채권매도3단계우선호가잔량 */
	char bond_bid3_remain_vol  	[15]; /* 채권매수3단계우선호가잔량 */
	char ask3_yield            	[13]; /* 매도3단계우선호가수익률 */
	char bid3_yield            	[13]; /* 매수3단계우선호가수익률 */
	char ask4_price            	[11]; /* 매도4단계우선호가가격 */
	char bid4_price            	[11]; /* 매수4단계우선호가가격 */
	char bond_ask4_remain_vol  	[15]; /* 채권매도4단계우선호가잔량 */
	char bond_bid4_remain_vol  	[15]; /* 채권매수4단계우선호가잔량 */
	char ask4_yield            	[13]; /* 매도4단계우선호가수익률 */
	char bid4_yield            	[13]; /* 매수4단계우선호가수익률 */
	char ask5_price            	[11]; /* 매도5단계우선호가가격 */
	char bid5_price            	[11]; /* 매수5단계우선호가가격 */
	char bond_ask5_remain_vol  	[15]; /* 채권매도5단계우선호가잔량 */
	char bond_bid5_remain_vol  	[15]; /* 채권매수5단계우선호가잔량 */
	char ask5_yield            	[13]; /* 매도5단계우선호가수익률 */
	char bid5_yield            	[13]; /* 매수5단계우선호가수익률 */
	char bond_total_ask_vol    	[15]; /* 채권매도호가총잔량 */
	char bond_total_bid_vol    	[15]; /* 채권매수호가총잔량 */
	char msg_end_key           	[1];  /* 정보분배메세지종료키워드 */
}	CO_B601K;		/* TR: 채권채결(KTS) B601K */

/* **************************************************** */
/* 채권 체결(A301K, KTS)								*/
/* **************************************************** */
typedef struct
{
	char tr_gbn			    	[5];	/* TR CODE			*/
	char seq_no            [8];	/* 정보분배일련번호 */
	char board_id              	[2];	/* 보드ID */
	char session_id            	[2];	/* 세션ID */
	char item_code               	[12];	/* 종목코드 */
	char trade_time            	[12];	/* 매매처리시각 */
	char crprc            	[11];	/* 체결가격 */
	char volume                	[10];	/* 거래량 */
	char biz_date              	[8];	/* 거래일자 */
	char trade_amt		       	[22];	/* 거래대금 */
	char exec_yield            	[13];	/* 체결수익률 */
	char open_price            	[11];	/* 시가 */
	char high_price            	[11];	/* 고가 */
	char low_price             	[11];	/* 저가 */
	char open_yield            	[13];	/* 시가수익률 */
	char high_yield            	[13];	/* 고가수익률 */
	char low_yield             	[13];	/* 저가수익률 */
	char bond_accum_exec_vol   	[15];	/* 채권누적체결수량 */
	char accum_trade_amt       	[22];	/* 누적거래대금 */
	char settl_date            	[8];	/* 결제일자 */
	char msg_end_key           	[1];	/* 정보분배메세지종료키워드 */
}	CO_A301K;		/* TR: 채권채결(KTS) A301K */

/* **************************************************** */
/* 채권 체결(G701K, KTS)								*/
/* **************************************************** */
typedef struct
{
	char tr_gbn			    	[5];	/* TR CODE			*/
	char seq_no            [8];	/* 정보분배일련번호 */
	char board_id            	[2];	/* 보드ID */
	char session_id          	[2];	/* 세션ID */
	char item_code             	[12];	/* 종목코드 */
	char trade_time          	[12];	/* 매매처리시각 */
	char crprc          	[11];	/* 체결가격 */
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
}	CO_G701K;		/* TR: 채권채결(KTS) G701K */

/* **************************************************** */
/* 채권 장운영TS(A701K, KTS)							*/
/* **************************************************** */
#if 0
CO_A701A 사용
typedef struct
{
	char tr_gbn			    	[5];	/* TR CODE			*/
	char seq_no            [8];	/* 정보분배일련번호 */
	char board_id            	[2];	/* 보드ID */
	char session_id          	[2];	/* 세션ID */
	char item_code             	[12];	/* 종목코드 */
	char info_stock_idx      	[6];	/* 정보분배종목인덱스 */
	char trade_time          	[12];	/* 매매처리시각 */
	char event_id            	[3];	/* 보드이벤트ID */
	char event_start_time    	[9];	/* 보드이벤트시작시각 */
	char event_group_code    	[5];	/* 보드이벤트적용군코드 */
	char suspend_reason      	[3];	/* 거래정지사유코드 */
	char msg_end_key         	[1];	/* 정보분배메세지종료키워드 */
}	CO_A701K;		/* TR: 채권 장운영TS(KTS) A701K */
#endif
typedef CO_A701A	CO_A701K;

/* **************************************************** */
/* 채권 종목마감(A601K, KTS)								*/
/* **************************************************** */
typedef struct
{
	char tr_gbn			    	[5];	/* TR CODE			*/
	char seq_no            [8];	/* 정보분배일련번호 */
	char board_id         		[2];	/* 보드ID */
	char item_code          		[12];	/* 종목코드 */
	char repo_period      		[4];	/* REPO기간 */
	char close_prc        		[11];	/* 종목마감종가 */
	char close_yield      		[13];	/* 종목마감종가수익률 */
	char close_type_cd    		[1];	/* 종가구분코드 */
	char msg_end_key      		[1];	/* 정보분배메세지종료키워드 */
}	CO_A601K;		/* TR: 채권종목마감(KTS) A601K */

/* **************************************************** */
/* 채권  장운영스케줄공개(M401K, KTS)					*/
/* **************************************************** */
typedef struct
{
	char tr_gbn			    	[5];	/* TR CODE			*/
	char seq_no            [8];	/* 정보분배일련번호 */
	char mkt_prod_grp_id      	[3];	/* 장운영상품그룹ID */
	char board_id             	[2];	/* 보드ID */
	char event_id             	[3];	/* 보드이벤트ID */
	char event_start_time     	[9];	/* 보드이벤트시작시각 */
	char event_group_code     	[5];	/* 보드이벤트적용군코드 */
	char sess_open_close      	[2];	/* 세션개시종료코드 */
	char session_id           	[2];	/* 세션ID */
	char item_code              	[12];	/* 종목코드 */
	char listed_stock         	[12];	/* 상장사종목코드 */
	char product_id           	[11];	/* 상품ID */
	char suspend_reason       	[3];	/* 거래정지사유코드 */
	char susp_type            	[1];	/* 거래정지발생유형코드 */
	char apply_stage          	[2];	/* 적용단계 */
	char prc_limit_ext        	[1];	/* 기준종목가격제한확대발생코드 */
	char prc_limit_tm         	[9];	/* 가격제한확대예정시각 */
	char msg_end_key          	[1];	/* 정보분배메세지종료키워드 */
}	CO_M401K;		/* TR: 채권 장운영스케줄공개(KTS) M401K */

/* **************************************************** */
/* 가격단위규칙(TRDESP01901) 20240809 신규, 81			*/
/* 호가정합성체크시 주문가격의 정합성 체크용			*/
/* 채권, 파생의 RDS 종목정보에 종목별 가격단위규칙 있음 */
/* **************************************************** */
typedef struct
{
	char seq_no             [11];	/* 메세지일련번호 */
	char tr_code               	[11];	/* 트랜잭션코드 */
	char send_date           	[8];	/* 전송일자 */
	char biz_date           	[8];	/* 영업일자 */
	char prc_unit_rule_id   	[10];	/* 가격단위규칙ID */
	char min_prc            	[11];	/* 구간최소가격 */
	char max_prc            	[11];	/* 구간최대가격 */
	char prc_unit           	[11];	/* 가격단위 */
}	CO_TRDESP01901;		/* TR: RDS가격단위규칙 */

/*************************************************************************
    End of Program (krx_mk.h)
*************************************************************************/
#endif
