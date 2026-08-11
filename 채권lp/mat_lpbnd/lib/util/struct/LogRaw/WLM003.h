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
 *  Components  : WLM003.h
 *  Rev. History: 고객본점마진그룹, 영업점수수료  Shared Memory
 *      Ver     Date    Information
 *      ------- ------- -----------------------------------------------
 *      1.00    2023-10 Winway initial version.
 ******************************************************************************/
 
 
#ifndef _WLM003_H
#define _WLM003_H

#include "comcom.h"
#include "WLM001.h"
#include "WLM002.h"

static SHM_ST cstst;

// --------------------------------------------------------------
// Shared Memory 구조체 
//---------------------------------------------------------------
//

typedef struct _aaaa_
{
	int       n_cnt;
	int       n_indx[500];
} CSTINDX_ST; 

typedef struct _bbbbb_
{
	char      s_tnr_id                     [  3+1];  // 테너코드 TOD, TOM, SPT, W01, M01, M02, M03, M06, Y01
	char      s_fx_pdcd                    [  3+1];  // FX상품코드 SPT, FWD
	char      s_tnr_ptrn_dcd               [  1+1];  // 테너구분 S: 표준
	char      s_sldy                       [  8+1];  // 결제일

	double    d_ndd                               ;  // 일수
	double    d_dldv                              ;  // 일할
	double    d_ask_spt_cvmg                      ;  // ASK 현물환 Cover딜러 마진
	double    d_ask_spt_cpmg                      ;  // ASK 현물환 Corp딜러 마진
	double    d_ask_fwd_cvmg                      ;  // ASK 선물환 Cover딜러 마진
	double    d_ask_fwd_cpmg                      ;  // ASK 선물환 Corp딜러 마진
	double    d_ask_hdom                          ;  // ASK 본점마진 합계
	double    d_ask_swap_pnt                      ;

	double    d_bid_spt_cvmg                      ;  // BID 현물환 Cover딜러 마진
	double    d_bid_spt_cpmg                      ;  // BID 현물환 Corp딜러 마진
	double    d_bid_fwd_cvmg                      ;  // BID 선물환 Cover딜러 마진
	double    d_bid_fwd_cpmg                      ;  // BID 선물환 Corp딜러 마진
	double    d_bid_hdom                          ;  // BID 본점마진 합계
	double    d_bid_swap_pnt                      ;

} TNRHDOM_ST;

typedef struct _aa_
{
	char        s_pair_id            [PAIRID_SZ+1];
	char        s_clc_dsnc                 [  1+1];  // 통화계산구분 1:XXX/USD, 2: DIV 3: MUL
	int         n_digit                           ;  // DIGIT
	double      d_digit_val                       ;  // DIGIT 값 0.01, 0.001, 0.0001
	double      d_clc_unit                        ;  // 계산단위 JPY/KRW 100, 나머지 1    
	char        s_spt_bomg_dcd             [  1+1];  // FWD 마진유형구분코드 1:금액, 2:율
	double      d_spt_bymg                        ;
	double      d_spt_slmg                        ;
	char        s_fwd_bomg_dcd             [  1+1];  // FWD 마진유형구분코드 1:금액, 2:율
	double      d_fwd_bymg                        ;
	double      d_fwd_slmg                        ;
} PAIRMRGN_ST;

typedef struct _bb_
{
	char        s_pair_id          [  PAIRID_SZ+1];
	char        s_clc_dsnc                 [  1+1];  // 통화계산구분 1:XXX/USD, 2: DIV 3: MUL
	int         n_digit                           ;  // DIGIT
	double      d_digit_val                       ;  // DIGIT 값 0.01, 0.001, 0.0001
	double      d_clc_unit                        ;  // 계산단위 JPY/KRW 100, 나머지 1    
	char        s_spt_bomg_dcd             [  1+1];  // FWD 마진유형구분코드 1:금액, 2:율
	double      d_spt_bymg                        ;
	double      d_spt_slmg                        ;
	char        s_fwd_bomg_dcd             [  1+1];  // FWD 마진유형구분코드 1:금액, 2:율
	double      d_fwd_bymg                        ;
	double      d_fwd_slmg                        ;

} FNLPAIR_ST;


typedef struct _cc_
{
	char         s_csac_idnt_no      [CUSTNO_SZ+1];  // 전행고객실명대체번호
	char         s_cust_grp_id       [CSTGRP_SZ+1];  // 본점마진구룹ID
	char         s_emp_grp_yn              [  1+1];	 // 직원그룹여부 Y,N
	int          n_fnl_cnt                        ;  // 재정Pair 건수
	FNLPAIR_ST   fnlmgst            [MAX_PAIR_CNT];  // 재정Pair 정보
	PAIRMRGN_ST  usdmgst                          ;
	int          n_std_cnt                        ;
	PAIRMRGN_ST  stdmgst            [MAX_PAIR_CNT];
} CUSTMRGN_ST;	


typedef struct {
	int             n_cst_cnt                     ;  // 건수
	CUSTMRGN_ST     cinfo           [MAX_CUST_CNT];  // 고객마진정보

	CSTINDX_ST      cstindx   [10][10][10][10][10];

} CUSTMRGN_SHM_ST;


// --------------------------------------------------------------
// Shared Memory 구조체 END
//---------------------------------------------------------------



/**********************************************************************************/
/* 수수료 계산 I/F                                                                */
/**********************************************************************************/
typedef struct _splt_in_st_
{
    char    s_csac_idnt_no      [ 30+1];        // 고객번호                       
	char    s_orgn_dsnc         [  1+1];        // 원천구분 1:고객거래, 2:내부거래, 3:대행거래
    char    s_pair_id           [  7+1];        // 통화페어                      
    char    s_sett_type         [  3+1];        // 상품구분: 'SPT(현물환)', 'FWD'
	char    s_tnr_id            [  3+1];        // 'TOD', 'TOM', 'SPT', 'S/N', 'W01', 'M01', 'M02', 'M03', 'M06', 'M09', 'Y01') 
	char    s_tnr_ptrn_dcd      [  1+1];        // 테너유형구분코드('S'-표준, 'U'-비표준, 'F'-만기선택선물환)

    char    s_bysel_dcd         [  1+1];        // 매입매도구분코드        
                                                // (1-BUY(Sell&Buy), 2-SELL(Buy&Sell))
    char    s_expi_fnsh_ymd     [  8+1];        // 만기종료년월일
    char    s_expi_sttg_ymd     [  8+1];        // 만기시작년월일             
    char    s_ordn_prc_cncd     [  1+1];        // 주문가격조건코드 
                                                // (1-시장가,2-지정가,3-예약주문) 
    double  d_fx_ordn_prc;                      // 주문가격                      
    double  d_bid_usd_prc;                      // BID USDKRW 가격               
    double  d_ask_usd_prc;                      // ASK USDKRW 가격               
    double  d_bid_std_prc;                      // BID 비재정 가격               
    double  d_ask_std_prc;                      // ASK 비재정 가격               
    double  d_bid_fnl_prc;                      // BID 재정 가격                 
    double  d_ask_fnl_prc;                      // ASK 재정 가격                 
}   SPLIT_IN_ST;

typedef struct _prd_lst_
{
    int     n_trhs_srn;                         // 거래내역일련번호               */
    char    s_crnc_pair_id      [  7+1];        // 통화페어ID                     */
	char    s_sett_type         [  3+1];
    char    s_tnr_ptrn_dcd      [  1+1];        // 테너유형구분코드(S:표준,U:비표준,F:만기선택선물환) */
    char    s_tnr_id            [  3+1];        // 테너ID                         */
    char    s_expi_fnsh_ymd     [  8+1];        // 만기종료년월일                 */
    char    s_bysel_dcd         [  1+1];        // 매입매도구분코드               */
    double  d_mrkt_spt_prc;                     // 시장SPOT가격                   */
    double  d_mrkt_swap_prc;                    // 시장SWAP가격         (SWAP포인 트) */
    double  d_fx_mrkt_prc;                      // FX시장가격                     */
    double  d_cvr_spr;                          // cover dealer 스프레드(cover 마 진) */
    double  d_fx_cvr_prc;                       // cover dealer 가격    (cover 마 진가격) */
    double  d_sls_spr;                          // corp dealer 스프레드 (corp 마진) */
    double  d_orcy_spr;                         // 당사스프레드         (본점마진)*/
    double  d_fx_orcy_prc;                      // FX당사가격           (본점가격)*/
    double  d_cus_spr;                          // 고객스프레드         (영업점마 진) */
    double  d_fx_cus_prc;                       // FX고객가격                     */
}   SPLIT_PRD_LIST;

typedef struct _split_put_st_
{
    char    s_pair_id           [  7+1];        // 통화페어                       */
    int     n_rec_cnt;                          /* 레코드 건수 1, 3               */
    SPLIT_PRD_LIST  rec         [    3];
} SPLIT_OUT_ST;

// 본점마진 계산을 위한 구조체
typedef struct {
	char      s_pair_id                    [  8+1];
	char      s_fx_pdcd                    [  3+1];  // FX상품코드 SPT, FWD
	char      s_tnr_id                     [  3+1];  // 테너코드 TOD, TOM, SPT, W01, M01, M02, M03, M06, Y01
	char      s_tnr_ptrn_dcd               [  1+1];  // 테너구분 S: 표준, 'U': 비표준, 'F'  만기선택선물환
	char      s_sldy                       [  8+1];  // 결제일
    char      s_bysel_dcd                  [  1+1];        /* 매입매도구분코드               */
	char      s_bomg_dcd                   [  1+1];  // 마진유형구분코드 1:금액, 2:율
	double    d_bomg                              ;  // 마진(율/금액)
	double    d_spt_prc;
	double    d_spt_cvmg                      ;  // ASK 현물환 Cover딜러 마진
	double    d_spt_cpmg                      ;  // ASK 현물환 Corp딜러 마진
	double    d_fwd_cvmg                      ;  // ASK 선물환 Cover딜러 마진
	double    d_fwd_cpmg                      ;  // ASK 선물환 Corp딜러 마진
	double    d_cvmg;
	double    d_cpmg;
	double    d_cvmg_prc                      ;  // ASK Cover마진 가격
	double    d_hdom                          ;  // ASK 본점마진 합계
	double    d_hdom_prc                      ;  // ASK 본점마진 가격
	double    d_swap_pnt                      ;  // ASK SWAP 포인트
	double    d_mrkt_prc                      ;  // ASK 시장 가격
	double    d_hdof_prc                      ;  // ASK 본점 가격 
	
	double    d_cst_mrgn                     ;
	double    d_cst_prc                       ;

} HDOMPRC_ST ;

#endif
// ----------------------------------------------------------------------------------------------
// END OF LINE
// ----------------------------------------------------------------------------------------------
