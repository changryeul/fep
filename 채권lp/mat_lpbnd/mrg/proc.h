/*******************************************************************************
 * (C) COPYRIGHT Winway Co., Ltd. 2020
 * All Rights Reserved
 * Licensed Materials - Property of Winway
 *
 * This program contains proprietary information of Winway System.
 * All embodying confidential information, ideas and expressions can't be
 * reproceduced, or transmitted in any form or by any means, electronic,
 * mechanical, or otherwise without the written permission of Winway System.
 *
 *  Components  : sise_mrgn.h
 *  Rev. History: 본점마진그룹, 직거래고객 현물환, 선물환 시세 Shared Memory
 *      Ver     Date    Information
 *      ------- ------- -----------------------------------------------
 *      1.00    2023-10 Winway initial version.
 ******************************************************************************/
 
 
#ifndef _MRGN_SISE_H
#define _MRGN_SISE_H

#include "tkt.h"

#define  N_TOD      1
#define  N_TOM      2
#define  N_SPT      3
#define  N_W01      4
#define  N_M01      5
#define  N_M02      6
#define  N_M03      7
#define  N_M06      8
#define  N_Y01      9

// --------------------------------------------------------------
// Shared Memory 구조체 
//---------------------------------------------------------------

typedef struct {
	char      s_fx_pdcd                    [  3+1];  // FX상품코드 SPT, FWD
	char      s_tnr_tcd                    [  3+1];  // 테너코드  TOD, TOM, SPT, W01, M01, ...
	char      s_crnc_pair_id               [  8+1];  // 통화페어ID
	char      s_dhom_dcd                   [  1+1];  // 영업점마진구분코드 1: 마진금액, 2: 마진율
	double    d_bymg                              ;  // 매입마진율(금액)
	double    d_slmg                              ;  // 매도마진율(금액)
} MRGN_ST;

#define MRGN_ST_SZ            sizeof(RMSG_ST)



// 테너 마진 및 시제
typedef struct {

	char      s_tnr_tcd                    [  7+1];  // 테너코드 TOD, TOM, SPT, W01, M01, M02, M03, M06, Y01
	char      s_tnr_dsnc                   [  1+1];  // 테너구분 S: 표준, 'U': 비표준
	char      s_qout_orgn                  [ 30+1];  // 시세원천
	char      s_sldy                       [  8+1];  // 결제일

	double    d_ndd                               ;  // 일수
	double    d_dldv                              ;  // 일할
	double    d_ask_spt_cvmg                      ;  // ASK 현물환 Cover딜러 마진
	double    d_ask_spt_cpmg                      ;  // ASK 현물환 Corp딜러 마진
	double    d_ask_fwd_cvmg                      ;  // ASK 선물환 Cover딜러 마진
	double    d_ask_fwd_cpmg                      ;  // ASK 선물환 Corp딜러 마진
	double    d_ask_cvmg_prc                      ;  // ASK Cover마진 가격
	double    d_ask_hdom                          ;  // ASK 본점마진 합계
	double    d_ask_hdom_prc                      ;  // ASK 본점마진 가격
	double    d_ask_swap_pnt                      ;  // ASK SWAP 포인트
	double    d_ask_mrkt_prc                      ;  // ASK 시장 가격
	double    d_ask_hdof_prc                      ;  // ASK 본점 가격 

	double    d_bid_spt_cvmg                      ;  // BID 현물환 Cover딜러 마진
	double    d_bid_spt_cpmg                      ;  // BID 현물환 Corp딜러 마진
	double    d_bid_fwd_cvmg                      ;  // BID 선물환 Cover딜러 마진
	double    d_bid_fwd_cpmg                      ;  // BID 선물환 Corp딜러 마진
	double    d_bid_cvmg_prc                      ;  // BID Cover마진 가격
	double    d_bid_hdom                          ;  // BID 본점마진 합계
	double    d_bid_hdom_prc                      ;  // BID 본점마진 가격
	double    d_bid_swap_pnt                      ;  // BID SWAP 포인트
	double    d_bid_mrkt_prc                      ;  // BID 시장 가격
	double    d_bid_hdof_prc                      ;  // BID 본점 가격 
		
	double    d_ask_pdcp                          ;  // ASK 본점 전일종가 
	double    d_ask_ctpd_indc                     ;  // ASK 전일대비 증감
	double    d_ask_sttg_prc                      ;  // ASK 시작 가격
	double    d_ask_hgpr_prc                      ;  // ASK 고가 가격 
	double    d_ask_lw_prc                        ;  // ASK 저가 가격 

	double    d_bid_pdcp                          ;  // BID 본점 전일종가 
	double    d_bid_ctpd_indc                     ;  // BID 전일대비 증감
	double    d_bid_sttg_prc                      ;  // BID 시작 가격
	double    d_bid_hgpr_prc                      ;  // BID 고가 가격 
	double    d_bid_lw_prc                        ;  // BID 저가 가격
} FXTNR_ST;

#define      FXTNR_ST_SZ                  sizeof(FXTNR_ST)

#define MAX_TNR_CNT        9                             // 테너 최대 건수, 표준테너만 관리 

// 통화Pair 정보 
typedef struct {	
	char      s_pair_id                    [  7+1];	 // 통화페어
	char      s_clc_dsnc                   [  1+1];	 // 통화계산구분 1:XXX/USD, 2: DIV 3: MUL

	char      s_ask_orgn                   [  1+1];  // ASK원천 : 'S':SMB, 'K':KMB, 'E':EBS, 'C':CMB
	char      s_bid_orgn                   [  1+1];  // BID원천 : 'S':SMB, 'K':KMB, 'E':EBS, 'C':CMB
	char      s_rcv_ymd                    [  8+1];  // 수신일자	
	char      s_rcv_hms                    [  9+1];  // 수신시각

	int       n_digit                             ;  // DIGIT
	double    d_digit_val                         ;  // DIGIT 값 0.01, 0.001, 0.0001
	double    d_clc_unit                          ;	 // 계산단위 JPY/KRW 100, 나머지 1
	double    d_ask_spt_prc                       ;  // ASK SPOT 가격
	double    d_bid_spt_prc                       ;  // BID SPOT 가격
	double    d_ask_usd_spt_prc                   ;  // ASK USDKRW SPOT 가격
	double    d_bid_usd_spt_prc                   ;  // BID USDKRW SPOT 가격

	int       n_tnr_cnt                           ;
	FXTNR_ST  tnr [MAX_TNR_CNT]    ;  // 테너 본점마진 정보
} FXPAIR_ST ;

#define      FXPAIR_ST_SZ               sizeof(FXPAIR_ST)


/*
#define MAX_CUST_CNT      2000                       // 적재 고객 최대 건수
#define MAX_MRGN_CNT      100                        // 적재 상품 최대 건수
*/

#define MAX_CUST_CNT      2                          // 적재 고객 최대 건수
#define MAX_MRGN_CNT      10                         // 적재 상품 최대 건수

typedef struct _cus_info_
{
	char      s_csac_idnt_no       [  9+1];  // 고객번호
	char      s_hdom_group_id      [  3+1];  // 본점마진그룹ID
	char      s_hoga_hdom_dsnc     [  1+1];  // 호가본점마진구분 1: 그룹, 2: 직거래
	int       n_prd_cnt                   ;  // 상품건수
	MRGN_ST   mrgn          [MAX_MRGN_CNT];
}	CUST_INFO;


// 고객정보 및 수수료
typedef struct 
{
	int			n_cst_cnt                           ;  // 건수
    CUST_INFO	cus_info[MAX_CUST_CNT];		
} CUSTMRGN_SHM_ST;

#define CUSTMRGN_SHM_ST_SZ		sizeof(CUSTMRGN_SHM_ST)


#define MAX_GRP_CNT        10                             // 본점마진그룹 최대 건수
#define MAX_PAIR_CNT       100                            // 통화Pair 최대 건수

typedef struct 
{
	char      s_hdom_grp_id        [  3+1];  // 본점마진그룹ID
	int       n_pair_cnt                  ;  // 통화Pair 적재 건수
	FXPAIR_ST pair [MAX_PAIR_CNT]         ;
} FXGRP_ST;

#define FXGRP_ST_SZ     sizeof(FXGRP_ST)

// 본점마진그룹 본점마진, 시세정보 SHM
typedef struct {
	char      test[10];
	int       n_grp_cnt                           ;  // 그룹 적재 건수
	FXGRP_ST  grp [MAX_GRP_CNT]                   ;
} HDOMGRP_SHM_ST ;

#define HDOMGRP_SHM_ST_SZ		sizeof(HDOMGRP_SHM_ST)



// 직거래고객 본점마진, 시세 정보
typedef struct _cust_
{
	char        s_csac_idnt_no     [  9+1];  // 고객계좌식별번호
	FXPAIR_ST   fxpair                    ; 
}	CUST;

typedef struct 
{
	int			n_cst_cnt                           ;  // 고객 적재 건수
	CUST		 cust [MAX_CUST_CNT];	
} DRTRHDOM_SHM_ST ;

#define DRTRHDOM_SHM_ST_SZ		        sizeof(DRTRHDOM_SHM_ST)

// --------------------------------------------------------------
// Shared Memory 구조체 END
//---------------------------------------------------------------


// 통화Pair 정보 
typedef struct {	
	char      s_pair_id                    [  7+1];	 // 통화페어
	char      s_clc_dsnc                   [  1+1];	 // 통화계산구분 1:XXX/USD, 2: DIV 3: MUL

	char      s_ask_orgn                   [  1+1];  // ASK원천 : 'S':SMB, 'K':KMB, 'E':EBS, 'C':CMB
	char      s_bid_orgn                   [  1+1];  // BID원천 : 'S':SMB, 'K':KMB, 'E':EBS, 'C':CMB
	char      s_rcv_ymd                    [  8+1];  // 수신일자	
	char      s_rcv_hms                    [  9+1];  // 수신시각

	int       n_digit                             ;  // DIGIT
	double    d_digit_val                         ;  // DIGIT 값 0.01, 0.001, 0.0001
	double    d_clc_unit                          ;	 // 계산단위 JPY/KRW 100, 나머지 1
	double    d_ask_spt_prc                       ;  // ASK SPOT 가격
	double    d_bid_spt_prc                       ;  // BID SPOT 가격
	double    d_ask_usd_spt_prc                   ;  // ASK USDKRW SPOT 가격
	double    d_bid_sud_spt_prc                   ;  // BID USDKRW SPOT 가격
} QOUTPAIR_ST ;

#define      QOUTPAIR_ST_SZ               sizeof(QOUTFXPAIR_ST)


// 본점마진그룹 본점마진, 시세정보 
typedef struct {
	QOUTPAIR_ST   pair                            ;
	FXTNR_ST      tnr                             ;
} HDQOUT_ST ;

#define HDOUT_ST_SZ                     sizeof(DHQOUT_ST)




// 직거래고객 본점마진, 시세정보
typedef struct {
	char          s_csac_idnt_n            [  9+1];  // 고객계좌식별번호
	FXPAIR_ST     pair                            ;  // 통화Pair 정보
	FXTNR_ST      tnr                             ;  // 테너 마진 시세 정보
} DRTRQOUT_ST ;

#define DRTRQOUT_ST_SZ                  sizeof(DRTRQOUT_ST_ST)


int l_calc_sise_shm(APSISE_ST *p_sise, HDOMGRP_SHM_ST *p_hdomgrp, char *s_errcd);
double  l_round_x(double *d_val, int n_digit);
double  l_round_d(double d_val, int n_digit);
#endif
// ----------------------------------------------------------------------------------------------
// END OF LINE
// ----------------------------------------------------------------------------------------------
