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
 *  Components  : .h
 *  Rev. History: 체결처리시 정보
 *      Ver     Date    Information
 *      ------- ------- -----------------------------------------------
 *      1.00    2023-10 Winway initial version.
 ******************************************************************************/

typedef struct {
        char    excode          [ 1];   // 'S'MB/'K'MB/E'BS/'C'MB/'B'EST/'Z'CUST
        char    bidex           [ 1];   // BID원천 : 'S':SMB, 'K':KMB, 'E':EBS, 'C':CMB
        char    offerex         [ 1];   // ASK원천 : 'S':SMB, 'K':KMB, 'E':EBS, 'C':CMB
        char    symb            [ 7];   // root symbol
        char    date            [ 8];   // 수신일자 YYYYMMDD (서버시간)
        char    time            [ 9];   // 수신시간 HHMMSSSSS
        double  usdbid                  ;       // Current USDKRW BID
        double  usdoffer                ;       // Current USDKRW OFFER
        double  bidprc                  ;       // Price of the MarketData Entry
        double  offerprc                ;       // Price of the MarketData Entry
        double  bidqty                  ;       // Quantity of the MarketData Entry. Always “0” For USD/KRW, CNH/KRW
        double  offerqty                ;       // Quantity of the MarketData Entry. Always “0” For USD/KRW, CNH/KRW
} APSISE_ST;



 typedef struct {
	char      s_cntt_no                    [ 20+1];  // 체결번호
	char      s_hoga_key                   [ 30+1];  // 호가KEY
	char      s_csac_idnt_no               [  9+1];  // 고객번호
	char      s_pair_id                    [  7+1];  // 통화페어
	char      s_tnr_dsnc                   [  1+1];  // 테너구분 S:정규, U:비정규
	char      s_fx_pdcd                    [  3+1];  // 상품구분코드 FWD
	char      s_val_st_dt                  [  8+1];  // 시작일자
	char      s_val_ed_dt                  [  8+1];  // 종료일자

 } SPLIT_IN_ST;

 
 typedef struct {
	char      s_cntr                       [ 20+1];  // 체결번호
	char      s_hoga_key                   [ 30+1];  // 호가KEY
	struct prd_lst {
		int       n_seq;
		char      s_fx_pdcd                [  3+1];  // FX상품코드 SPT, FWD
		char      s_crnc_pair_id           [  8+1];  // 통화페어ID
		char      s_tnr_tcd                [  3+1];  // 테너코드  TOD, TOM, SPT, W0`, M01, ...
		char      s_sldy                   [  8+1];  // 결제일
		char      s_bysel_dcd              [  1+1];  // 매입매도구분코드
		double    d_spt_prc                      ;  // SPOT가격
		double    d_swap_pnt                      ;  // SWAP 포인트
		double    d_mrkt_prc                      ;  // 시장 가격
		double    d_covr_sprd                     ;  // Cover Dealer마진
		double    d_corp_sprd                     ;  // Corp Dealer마진
		double    d_hdof_sprd                     ;  // 본점스프레드
		double    d_hdof_prc                      ;  // 본점가격
	} rec[3];

 } SPLIT_OUT_ST;
