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

/* ******************************************************************* */
/* 지수선물 */
/* ******************************************************************* */
typedef struct
{
	char		tr_gbn							[5 ];		 /* TR CODE	*/
	char	    cnt                             [5 ];        /* 종목수	*/                        
    char        date                            [8 ];        /* 영업일자	*/                      
    char        item_code                       [12];        /* 종목코드            */
    char        seq_no                          [6 ];        /* 종목SEQ	*/                      
    char        derivation_id                   [11];        /* 파생품목ID	*/                    
    char        compress_code                   [9 ];        /* 선물종목단축코드	*/              
    char        kor_nm                          [80];        /* 종목한글명	*/                    
    char        kor_nm_abbr                     [40];        /* 종목한글약명	*/                  
    char        elan_nm                         [80];        /* 종목영문명	*/                    
    char        elan_nm_abbr                    [40];        /* 종목영문약명	*/                  
    char        list_date                       [8 ];        /* 상장일자	*/                      
    char        delist_date                     [8 ];        /* 상장폐지일자	*/                  
    char        spread_basis                    [1 ];        /* 스프레드기준종목구분코드	*/      
    char        last_setl_code                  [1 ];        /* 최종결제방법코드	*/              
    char        limit_price_way_cd              [1 ];        /* 가격제한확대적용방향코드	*/      
    char        limit_price_last_stat           [3 ];        /* 가격제한최종단계	*/              
    char        hlprc01_sign                    [1 ];        /* 가격제한1단계SIGN부호	*/        
    char        hlprc01                         [12];        /* 가격제한1단계상한가 (10V2)	*/    
    char        llprc01_sign                    [1 ];        /* 가격제한1단계SIGN부호	*/        
    char        llprc01                         [12];        /* 가격제한1단계하한가 (10V2)	*/    
    char        hlprc02_sign                    [1 ];        /* 가격제한2단계SIGN부호	*/        
    char        hlprc02                         [12];        /* 가격제한2단계상한가 (10V2)	*/    
    char        llprc02_sign                    [1 ];        /* 가격제한2단계SIGN부호	*/        
    char        llprc02                         [12];        /* 가격제한2단계하한가 (10V2)	*/    
    char        hlprc03_sign                    [1 ];        /* 가격제한3단계SIGN부호	*/        
    char        hlprc03                         [12];        /* 가격제한3단계상한가 (10V2)	*/    
    char        llprc03_sign                    [1 ];        /* 가격제한3단계SIGN부호	*/        
    char        llprc03                         [12];        /* 가격제한3단계하한가 (10V2)	*/    
    char        stprc                           [12];        /* 기준가 (10V2)	*/                
    char        underlying_id                   [3 ];        /* 기초자산ID	*/                    
    char        kyun_event_kind_gbn             [1 ];        /* 권리행사유형코드	*/              
    char        spread_kind_gbn                 [2 ];        /* 스프레드유형코드	*/              
    char        int_std_cd1                     [12];        /* 스프레드근월물표준코드	*/        
    char        int_std_cd2                     [12];        /* 스프레드원월물표준코드	*/        
    char        last_trade_date                 [8 ];        /* 최종거래일자	*/                  
    char        last_settle_date                [8 ];        /* 최종결제일자	*/                  
    char        gubun_code                      [3 ];        /* 월물구분코드	*/                  
    char        maturity_date                   [8 ];        /* 만기일자	*/                      
    char        striking_price                  [17];        /* 행사가격 9(9)V9(8)	*/            
    char        control_gbn                     [1 ];        /* 조정구분	*/                      
    char        trade_unit                      [17];        /* 거래단위 9(9)V9(8)	*/            
    char        trade_multi                     [21];        /* 거래승수 9(13)V9(8)	*/          
    char        marketcontrol_code              [1 ];        /* 시장조성구분코드	*/              
    char        normal_gubun                    [1 ];        /* 상장유형코드	*/                  
    char        equal_price                     [12];        /* 등가격 9(10)V9(2)	*/            
    char        adjustment_reason               [2 ];        /* 조정사유코드	*/                  
    char        underlying_code                 [12];        /* 기초자산종목코드	*/              
    char        underlying_close_price          [12];        /* 기초자산종가	*/                  
    char        remain_day                      [7 ];        /* 잔존일수	*/                      
    char        adjustment_basis_price          [17];        /* 조정기준가격 9(9)V9(8)	*/        
    char        basis_price_gubun               [2 ];        /* 기준가격구분코드	*/              
    char        trade_basis_price_gubun         [1 ];        /* 매매용기준가격구분코드	*/        
    char        sign_3                          [1 ];        /* Sign부호3	*/                    
    char        pdy_adjust_close_price          [17];        /* 전일조정종가격 9(9)V9(8)	*/      
    char        discuss_large_trade             [1 ];        /* 협의대량매매대상여부	*/          
    char        pdy_deposit_stand_prc           [17];        /* 전일증거금기준가격 9(9)V9(8)	*/  
    char        pdy_deposit_stand_prc_cd        [2 ];        /* 전일증거금기준가격코드	*/        
    char        settle_theory_price             [15];        /* 정산이론가격 9(9)V9(6)	*/        
    char        standrd_theory_price            [15];        /* 기준이론가격 9(9)V9(6)	*/        
    char        pdy_settle_price                [17];        /* 전일정산가격 9(9)V9(8)	*/        
    char        trade_stop                      [1 ];        /* 거래정지여부	*/                  
    char        cb_appl_hprc                    [12];        /* C.B.적용상한가 (10V2)	*/        
    char        cb_appl_lprc                    [12];        /* C.B.적용하한가 (10V2)	*/        
    char        inquiry_striking_price          [17];        /* 조회용행사가격 9(9)V9(8)	*/      
    char        atm_gubun                       [1 ];        /* atm구분코드	*/                  
    char        last_trade_day_gubun            [1 ];        /* 최종거래일여부	*/                
    char        allotment_value                 [15];        /* 배당가치 9(9)V9(6)	*/            
    char        sign_4                          [1 ];        /* Sign부호4	*/                    
    char        pdy_cprc                        [12];        /* 전일종가 (10V2)	*/              
    char        pdy_cprc_gubun                  [1 ];        /* 전일종가구분코드	*/              
    char        sign_5                          [1 ];        /* Sign부호5	*/                    
    char        pdy_oprc                        [12];        /* 전일시가 (10V2)	*/              
    char        sign_6                          [1 ];        /* Sign부호6	*/                    
    char        pdy_hprc                        [12];        /* 전일고가 (10V2)	*/              
    char        sign_7                          [1 ];        /* Sign부호7	*/                    
    char        pdy_lprc                        [12];        /* 전일저가 (10V2)	*/              
    char        first_contract_date             [8 ];        /* 최초체결일자	*/                  
    char        pdy_last_contract_time          [8 ];        /* 전일최종체결시각	*/              
    char        pdy_slmt_gbn                    [2 ];        /* 전일정산가격구분	*/              
    char        sign_8                          [1 ];        /* Sign부호8	*/                    
    char        disparate_ratio                 [12];        /* 정산가격이론가격괴리율(6V6)	*/  
    char        pdy_usetl_engg_qty              [10];        /* 전일미결제약정수량	*/            
    char        sign_9                          [1 ];        /* Sign부호9	*/                    
    char        pdy_sell_price                  [12];        /* 전일매도우선호가 (10V2)	*/      
    char        sign_10                         [1 ];        /* Sign부호10	*/                    
    char        pdy_buy_price                  [12];        /* 전일매수우선호가 (10V2)	*/      
    char        inhrc_chdps                     [10];        /* 내재변동성 (6V4)	*/              
    char        sign_11                         [1 ];        /* Sign부호11	*/                    
    char        tmp_high_pr                     [12];        /* 상장중최고가 (10V2)	*/          
    char        sign_12                         [1 ];        /* Sign부호12	*/                    
    char        tmp_low_pr                      [12];        /* 상장중최저가 (10V2)	*/          
    char        sign_13                         [1 ];        /* Sign부호13	*/                    
    char        year_high_pr                    [12];        /* 연중최고가 (10V2)	*/            
    char        sign_14                         [1 ];        /* Sign부호14	*/                    
    char        year_low_pr                     [12];        /* 연중최저가 (10V2)	*/            
    char        tmp_high_dt                     [8 ];        /* 상장중최고가일자	*/              
    char        tmp_low_dt                      [8 ];        /* 상장중최저가일자	*/              
    char        year_high_dt                    [8 ];        /* 연중최고가일자	*/                
    char        year_low_dt                     [8 ];        /* 연중최저가일자	*/                
    char        year_standard_day               [8 ];        /* 연간기준일수	*/                  
    char        month_trade_day                 [8 ];        /* 월간거래일수	*/                  
    char        year_trade_day                  [8 ];        /* 연간거래일수	*/                  
    char        pdy_tr_cnt                      [16];        /* 전일체결건수	*/                  
    char        pdy_tr_qty                      [12];        /* 전일체결수량	*/                  
    char        pdy_tr_mny                      [22];        /* 전일거래대금	*/                  
    char        pdy_large_qty                   [12];        /* 전일협의대량매매체결수량	*/      
    char        pdy_large_mny                   [22];        /* 전일협의대량매매거래대금	*/      
    char        cd_price_rate                   [6 ];        /* CD금리 (3V3)	*/                  
    char        unsettle_hando                  [15];        /* 미결제한도계약수	*/              
    char        position_goods                  [4 ];        /* 소속상품군	*/                    
    char        goods_opset                     [9 ];        /* 상품군옵셋율 9(7)V9(2)	*/        
    char        limits_order                    [5 ];        /* 지정가호가조건구분코드	*/        
    char        market_order                    [5 ];        /* 시장가호가조건구분코드	*/        
    char        condition_order                 [5 ];        /* 조건부지정가호가조건구분코드	*/  
    char        advantage_order                 [5 ];        /* 최유리지정가호가조건구분코드	*/  
    char        efp_trade_yn                    [1 ];        /* EFP거래대상여부	*/              
    char        flex_trade_yn                   [1 ];        /* FLEX거래대상여부	*/              
    char        bef_efp_tradeamt                [12];        /* 전일EFP체결수량	*/              
    char        bef_efp_moneyamt                [22];        /* 전일EFP거래대금	*/              
    char        business_yn                     [1 ];        /* 휴장여부	*/                      
    char        realtime_limit_yn               [1 ];        /* 실시간가격제한여부	*/            
    char        realtime_hi_pgap_sign           [1 ];        /* SIGN부호	*/                      
    char        realtime_hi_pgap                [12];        /* 실시간상한가간격	*/              
    char        realtime_low_pgap_sign          [1 ];        /* SIGN부호	*/                      
    char        realtime_low_pgap               [12];        /* 실시간하한가간격	*/              
    char        underlying_mk_id				[3 ];        /* 기초자산시장ID	*/                
    char        hi_limit_cnt					[16];        /* 상한수량	*/                      
    char        low_limit_cnt					[16];        /* 하한수량	*/                      
    char        combig_hi_qty                   [16];        /* 협의대량매매상한수량	*/          
    char        combig_low_qty                  [16];        /* 협의대량매매하한수량	*/          
    char        filler1                         [36];        /* filler1	*/                      
    char        off_yn                          [1 ];        /* 휴면여부	*/                      
    char        off_day_yn                      [8 ];        /* 휴면지정일자	*/                  
    char        filler                          [15];        /* Filler	*/                        
}	SIF_A0014;		// A0014

typedef struct
{
	char	tr_gbn				[5 ];		/* TR CODE						*/
    char    item_code           [12];       /* 종목코드                     */  
    char    seq_no              [2 ];       /* 종목일련번호                 */  
    char    board_id            [2 ];       /* 보드ID                       */
    char    crprc_sign          [1 ];       /* 현재가격부호                 */  
    char    crprc               [5 ];       /* 현재가 (3V2)                 */  
    char    chekyul_qty         [6 ];       /* 체결수량                     */  
    char    session_id          [2 ];       /* 세션ID                 		*/        
    char    chekyul_tm          [8 ];       /* 체결시각                     */  
    char    fiction_crprc1      [5 ];       /* 최근월물의제약정가격 (3V2)   */  
    char    fiction_crprc2      [5 ];       /* 원월물의제약정가격 (3V2)     */  
    char    oprc_sign           [1 ];       /* 시가부호                     */  
    char    oprc                [5 ];       /* 시가 (3V2)                   */  
    char    hprc_sign           [1 ];       /* 고가부호                     */  
    char    hprc                [5 ];       /* 고가 (3V2)                   */  
    char    lprc_sign           [1 ];       /* 저가부호                     */  
    char    lprc                [5 ];       /* 저가 (3V2)                   */  
    char    before_crprc_sign   [1 ];       /* 직전가격부호                 */  
    char    before_crprc        [5 ];       /* 직전가격 (3V2)               */  
    char    tot_con_qty         [7 ];       /* 누적체결수량                 */  
    char    tot_con_amt         [12];       /* 누적거래대금 (단위:천원)     */  
    char    combig_che_amt      [7 ];       /* 협의대량누적체결수량			*/          
    char    last_bidask_cd      [1 ];       /* 최종매도매수구분코드			*/          
    char    realtime_hprc_sign  [1 ];       /* 실시간상한가부호				*/              
    char    realtime_hprc       [5 ];       /* 실시간상한가					*/                  
    char    realtime_lprc_sign  [1 ];       /* 실시간하한가부호				*/              
    char    realtime_lprc       [5 ];       /* 실시간하한가					*/                  
}	SIF_A3014;			// A3014


typedef struct
{
	char	  tr_gbn				  [5 ];	 	 /* TR CODE						 */
	char      item_code               [12];      /* 종목코드                     */   
	char      seq_no                  [2 ];      /* 종목일련번호                 */   
	char      board_id                [2 ];      /* 보드ID                       */
	char      crprc_sign              [1 ];      /* 현재가격부호                 */   
	char      crprc                   [5 ];      /* 현재가 (3V2)                 */   
	char      chekyul_qty             [6 ];      /* 체결수량                     */   
	char      session_id              [2 ];      /* 세션ID                       */   
	char      chekyul_tm              [8 ];      /* 체결시각                     */   
	char      fiction_crprc1          [5 ];      /* 최근월물의제약정가격 (3V2)   */   
	char      fiction_crprc2          [5 ];      /* 원월물의제약정가격 (3V2)     */   
	char      oprc_sign               [1 ];      /* 시가부호                     */   
	char      oprc                    [5 ];      /* 시가 (3V2)                   */   
	char      hprc_sign               [1 ];      /* 고가부호                     */   
	char      hprc                    [5 ];      /* 고가 (3V2)                   */   
	char      lprc_sign               [1 ];      /* 저가부호                     */   
	char      lprc                    [5 ];      /* 저가 (3V2)                   */   
	char      before_crprc_sign       [1 ];      /* 직전가격부호                 */   
	char      before_crprc            [5 ];      /* 직전가격 (3V2)               */   
	char      tot_con_qty             [7 ];      /* 누적체결수량                 */   
	char      tot_con_amt             [12];      /* 누적거래대금 (단위:천원)     */   
	char      combig_che_amt          [7 ];      /* 협의대량누적체결수량         */   
	char      last_bidask_cd          [1 ];      /* 최종매도매수구분코드         */   

	char      buy_tot_best_qty        [6 ];      /* 매수총호가잔량               */   
	char      buy_1_sign              [1 ];      /* 매수1단계부호                */   
	char      buy_1_price            [5 ];      /* 매수1단계우선호가격 (3V2)    */   
	char      buy_1_price_qty        [6 ];      /* 매수1단계우선호가잔량        */   
	char      buy_2_sign              [1 ];      /* 매수2단계부호                */   
	char      buy_2_price            [5 ];      /* 매수2단계우선호가격 (3V2)    */   
	char      buy_2_price_qty        [6 ];      /* 매수2단계우선호가잔량        */   
	char      buy_3_sign              [1 ];      /* 매수3단계부호                */   
	char      buy_3_price            [5 ];      /* 매수3단계우선호가격 (3V2)    */   
	char      buy_3_price_qty        [6 ];      /* 매수3단계우선호가잔량        */   
	char      buy_4_sign              [1 ];      /* 매수4단계부호                */   
	char      buy_4_price            [5 ];      /* 매수4단계우선호가격 (3V2)    */   
	char      buy_4_price_qty        [6 ];      /* 매수4단계우선호가잔량        */   
	char      buy_5_sign              [1 ];      /* 매수5단계부호                */   
	char      buy_5_price            [5 ];      /* 매수5단계우선호가격 (3V2)    */   
	char      buy_5_price_qty        [6 ];      /* 매수5단계우선호가잔량        */   
	char      sell_tot_best_qty        [6 ];      /* 매도총호가잔량               */   
	char      sell_1_sign              [1 ];      /* 매도1단계부호                */   
	char      sell_1_price            [5 ];      /* 매도1단계우선호가격 (3V2)    */   
	char      sell_1_price_qty        [6 ];      /* 매도1단계우선호가잔량        */   
	char      sell_2_sign              [1 ];      /* 매도2단계부호                */   
	char      sell_2_price            [5 ];      /* 매도2단계우선호가격 (3V2)    */   
	char      sell_2_price_qty        [6 ];      /* 매도2단계우선호가잔량        */   
	char      sell_3_sign              [1 ];      /* 매도3단계부호                */   
	char      sell_3_price            [5 ];      /* 매도3단계우선호가격 (3V2)    */   
	char      sell_3_price_qty        [6 ];      /* 매도3단계우선호가잔량        */   
	char      sell_4_sign              [1 ];      /* 매도4단계부호                */   
	char      sell_4_price            [5 ];      /* 매도4단계우선호가격 (3V2)    */   
	char      sell_4_price_qty        [6 ];      /* 매도4단계우선호가잔량        */   
	char      sell_5_sign              [1 ];      /* 매도5단계부호                */   
	char      sell_5_price            [5 ];      /* 매도5단계우선호가격 (3V2)    */   
	char      sell_5_price_qty        [6 ];      /* 매도5단계우선호가잔량        */   
	char      buy_tot_best_cnt        [5 ];      /* 매수유효호가건수             */   
	char      buy_1_best_cnt          [4 ];      /* 매수1단계우선호가건수        */   
	char      buy_2_best_cnt          [4 ];      /* 매수2단계우선호가건수        */   
	char      buy_3_best_cnt          [4 ];      /* 매수3단계우선호가건수        */   
	char      buy_4_best_cnt          [4 ];      /* 매수4단계우선호가건수        */   
	char      buy_5_best_cnt          [4 ];      /* 매수5단계우선호가건수        */   
	char      sell_tot_best_cnt        [5 ];      /* 매도유효호가건수             */   
	char      sell_1_best_cnt          [4 ];      /* 매도1단계우선호가건수        */   
	char      sell_2_best_cnt          [4 ];      /* 매도2단계우선호가건수        */   
	char      sell_3_best_cnt          [4 ];      /* 매도3단계우선호가건수        */   
	char      sell_4_best_cnt          [4 ];      /* 매도4단계우선호가건수        */   
	char      sell_5_best_cnt          [4 ];      /* 매도5단계우선호가건수        */   
	char      realtime_hprc_sign      [1 ];      /* 실시간상한가부호             */   
	char      realtime_hprc           [5 ];      /* 실시간상한가                 */   
	char      realtime_lprc_sign      [1 ];      /* 실시간하한가부호             */   
	char      realtime_lprc           [5 ];      /* 실시간하한가                 */   
}	SIF_G7014;				// G7014


typedef struct
{
	char	tr_gbn				  [5 ];	 	 	/* TR CODE						*/
	char    item_code             [12];         /* 종목코드                     */            
	char    seq_no                [2 ];         /* 종목일련번호                 */
	char    board_id              [2 ];    		/* 보드 ID  					*/ 
	char    session_id            [2 ];    		/* 세션 ID  					*/ 

	char    buy_tot_best_qty      [6 ];         /* 매수총호가잔량               */          
	char    buy_1_sign            [1 ];         /* 매수1단계부호                */    
	char    buy_1_price          [5 ];         /* 매수1단계우선호가격 (3V2)    */      
	char    buy_1_price_qty      [6 ];         /* 매수1단계우선호가잔량        */          
	char    buy_2_sign            [1 ];         /* 매수2단계부호                */    
	char    buy_2_price          [5 ];         /* 매수2단계우선호가격 (3V2)    */      
	char    buy_2_price_qty      [6 ];         /* 매수2단계우선호가잔량        */          
	char    buy_3_sign            [1 ];         /* 매수3단계부호                */    
	char    buy_3_price          [5 ];         /* 매수3단계우선호가격 (3V2)    */      
	char    buy_3_price_qty      [6 ];         /* 매수3단계우선호가잔량        */          
	char    buy_4_sign            [1 ];         /* 매수4단계부호                */    
	char    buy_4_price          [5 ];         /* 매수4단계우선호가격 (3V2)    */      
	char    buy_4_price_qty      [6 ];         /* 매수4단계우선호가잔량        */          
	char    buy_5_sign            [1 ];         /* 매수5단계부호                */    
	char    buy_5_price          [5 ];         /* 매수5단계우선호가격 (3V2)    */      
	char    buy_5_price_qty      [6 ];         /* 매수5단계우선호가잔량        */          

	char    sell_tot_best_qty      [6 ];         /* 매도총호가잔량               */          
	char    sell_1_sign            [1 ];         /* 매도1단계부호                */    
	char    sell_1_price          [5 ];         /* 매도1단계우선호가격 (3V2)    */      
	char    sell_1_price_qty      [6 ];         /* 매도1단계우선호가잔량        */          
	char    sell_2_sign            [1 ];         /* 매도2단계부호                */    
	char    sell_2_price          [5 ];         /* 매도2단계우선호가격 (3V2)    */      
	char    sell_2_price_qty      [6 ];         /* 매도2단계우선호가잔량        */          
	char    sell_3_sign            [1 ];         /* 매도3단계부호                */    
	char    sell_3_price          [5 ];         /* 매도3단계우선호가격 (3V2)    */      
	char    sell_3_price_qty      [6 ];         /* 매도3단계우선호가잔량        */          
	char    sell_4_sign            [1 ];         /* 매도4단계부호                */    
	char    sell_4_price          [5 ];         /* 매도4단계우선호가격 (3V2)    */      
	char    sell_4_price_qty      [6 ];         /* 매도4단계우선호가잔량        */          
	char    sell_5_sign            [1 ];         /* 매도5단계부호                */    
	char    sell_5_price          [5 ];         /* 매도5단계우선호가격 (3V2)    */      
	char    sell_5_price_qty      [6 ];         /* 매도5단계우선호가잔량        */          

	char    buy_tot_best_cnt      [5 ];         /* 매수유효호가건수             */          
	char    buy_1_best_cnt        [4 ];         /* 매수1단계우선호가건수        */        
	char    buy_2_best_cnt        [4 ];         /* 매수2단계우선호가건수        */        
	char    buy_3_best_cnt        [4 ];         /* 매수3단계우선호가건수        */        
	char    buy_4_best_cnt        [4 ];         /* 매수4단계우선호가건수        */        
	char    buy_5_best_cnt        [4 ];         /* 매수5단계우선호가건수        */        

	char    sell_tot_best_cnt      [5 ];         /* 매도유효호가건수             */          
	char    sell_1_best_cnt        [4 ];         /* 매도1단계우선호가건수        */        
	char    sell_2_best_cnt        [4 ];         /* 매도2단계우선호가건수        */        
	char    sell_3_best_cnt        [4 ];         /* 매도3단계우선호가건수        */        
	char    sell_4_best_cnt        [4 ];         /* 매도4단계우선호가건수        */        
	char    sell_5_best_cnt        [4 ];         /* 매도5단계우선호가건수        */        

	char    best_amt_accept_tm    [8 ];    		/* 호가 접수시간 				*/ 
	char    expect_crprc_sign     [1 ];    		/* 예상체결가격부호 			*/ 
	char    expect_crprc          [5 ];    		/* 예상체결가격 				*/ 
}	SIF_B6014;					// B6014

/* ******************************************************************* */
/* 지수옵션 */
/* ******************************************************************* */

typedef struct
{
	char	tr_gbn				[5 ];		/* TR CODE						*/
    char    item_code           [12];       /* 종목코드                     */  
    char    seq_no              [4 ];       /* 종목일련번호                 */  
    char    board_id            [2 ];       /* 보드ID                       */
    char    crprc               [5 ];       /* 현재가 (3V2)                 */  
    char    chekyul_qty         [7 ];       /* 체결수량                     */  
    char    session_id          [2 ];       /* 세션ID                 		*/        
    char    chekyul_tm          [8 ];       /* 체결시각                     */  
    char    oprc                [5 ];       /* 시가 (3V2)                   */  
    char    hprc                [5 ];       /* 고가 (3V2)                   */  
    char    lprc                [5 ];       /* 저가 (3V2)                   */  
    char    before_crprc        [5 ];       /* 직전가격 (3V2)               */  
    char    tot_con_qty         [8 ];       /* 누적체결수량                 */  
    char    tot_con_amt         [11];       /* 누적거래대금 (단위:천원)     */  
    char    combig_che_amt      [8 ];       /* 협의대량누적체결수량			*/          
    char    last_bidask_cd      [1 ];       /* 최종매도매수구분코드			*/          
    char    realtime_hprc       [5 ];       /* 실시간상한가					*/                  
    char    realtime_lprc       [5 ];       /* 실시간하한가					*/                  
}	SIO_A3034;					// A3034

typedef struct
{
	char	  tr_gbn				  [5 ];	 	 /* TR CODE						 */
	char      item_code               [12];      /* 종목코드                     */   
	char      seq_no                  [4 ];      /* 종목일련번호                 */   
	char      board_id                [2 ];      /* 보드ID                       */
	char      crprc                   [5 ];      /* 현재가 (3V2)                 */   
	char      chekyul_qty             [7 ];      /* 체결수량                     */   
	char      session_id              [2 ];      /* 세션ID                       */   
	char      chekyul_tm              [8 ];      /* 체결시각                     */   
	char      oprc                    [5 ];      /* 시가 (3V2)                   */   
	char      hprc                    [5 ];      /* 고가 (3V2)                   */   
	char      lprc                    [5 ];      /* 저가 (3V2)                   */   
	char      before_crprc            [5 ];      /* 직전가격 (3V2)               */   
	char      tot_con_qty             [8 ];      /* 누적체결수량                 */   
	char      tot_con_amt             [11];      /* 누적거래대금 (단위:천원)     */   
	char      combig_che_amt          [8 ];      /* 협의대량누적체결수량         */   
	char      last_bidask_cd          [1 ];      /* 최종매도매수구분코드         */   

	char      buy_tot_best_qty        [7 ];      /* 매수총호가잔량               */   
	char      buy_1_price            [5 ];      /* 매수1단계우선호가격 (3V2)    */   
	char      buy_1_price_qty        [7 ];      /* 매수1단계우선호가잔량        */   
	char      buy_2_price            [5 ];      /* 매수2단계우선호가격 (3V2)    */   
	char      buy_2_price_qty        [7 ];      /* 매수2단계우선호가잔량        */   
	char      buy_3_price            [5 ];      /* 매수3단계우선호가격 (3V2)    */   
	char      buy_3_price_qty        [7 ];      /* 매수3단계우선호가잔량        */   
	char      buy_4_price            [5 ];      /* 매수4단계우선호가격 (3V2)    */   
	char      buy_4_price_qty        [7 ];      /* 매수4단계우선호가잔량        */   
	char      buy_5_price            [5 ];      /* 매수5단계우선호가격 (3V2)    */   
	char      buy_5_price_qty        [7 ];      /* 매수5단계우선호가잔량        */   

	char      sell_tot_best_qty        [7 ];      /* 매도총호가잔량               */   
	char      sell_1_price            [5 ];      /* 매도1단계우선호가격 (3V2)    */   
	char      sell_1_price_qty        [7 ];      /* 매도1단계우선호가잔량        */   
	char      sell_2_price            [5 ];      /* 매도2단계우선호가격 (3V2)    */   
	char      sell_2_price_qty        [7 ];      /* 매도2단계우선호가잔량        */   
	char      sell_3_price            [5 ];      /* 매도3단계우선호가격 (3V2)    */   
	char      sell_3_price_qty        [7 ];      /* 매도3단계우선호가잔량        */   
	char      sell_4_price            [5 ];      /* 매도4단계우선호가격 (3V2)    */   
	char      sell_4_price_qty        [7 ];      /* 매도4단계우선호가잔량        */   
	char      sell_5_price            [5 ];      /* 매도5단계우선호가격 (3V2)    */   
	char      sell_5_price_qty        [7 ];      /* 매도5단계우선호가잔량        */   

	char      buy_tot_best_cnt        [5 ];      /* 매수유효호가건수             */   
	char      buy_1_best_cnt          [4 ];      /* 매수1단계우선호가건수        */   
	char      buy_2_best_cnt          [4 ];      /* 매수2단계우선호가건수        */   
	char      buy_3_best_cnt          [4 ];      /* 매수3단계우선호가건수        */   
	char      buy_4_best_cnt          [4 ];      /* 매수4단계우선호가건수        */   
	char      buy_5_best_cnt          [4 ];      /* 매수5단계우선호가건수        */   

	char      sell_tot_best_cnt        [5 ];      /* 매도유효호가건수             */   
	char      sell_1_best_cnt          [4 ];      /* 매도1단계우선호가건수        */   
	char      sell_2_best_cnt          [4 ];      /* 매도2단계우선호가건수        */   
	char      sell_3_best_cnt          [4 ];      /* 매도3단계우선호가건수        */   
	char      sell_4_best_cnt          [4 ];      /* 매도4단계우선호가건수        */   
	char      sell_5_best_cnt          [4 ];      /* 매도5단계우선호가건수        */   

	char      realtime_hprc           [5 ];      /* 실시간상한가                 */   
	char      realtime_lprc           [5 ];      /* 실시간하한가                 */   
}	SIO_G7034;				// G7034

typedef struct
{
	char	tr_gbn				  [5 ];			/* TR CODE					   	*/
	char	item_code			  [12];			/* 종목코드						*/
	char	seq_no				  [4 ];			/* 종목일련번호					*/
	char    board_id              [2 ];    		/* 보드 ID  					*/ 
	char    session_id            [2 ];    		/* 세션 ID  					*/ 

	char    buy_tot_best_qty      [7 ];         /* 매수총호가잔량               */          
	char    buy_1_price          [5 ];         /* 매수1단계우선호가격 (3V2)    */      
	char    buy_1_price_qty      [7 ];         /* 매수1단계우선호가잔량        */          
	char    buy_2_price          [5 ];         /* 매수2단계우선호가격 (3V2)    */      
	char    buy_2_price_qty      [7 ];         /* 매수2단계우선호가잔량        */          
	char    buy_3_price          [5 ];         /* 매수3단계우선호가격 (3V2)    */      
	char    buy_3_price_qty      [7 ];         /* 매수3단계우선호가잔량        */          
	char    buy_4_price          [5 ];         /* 매수4단계우선호가격 (3V2)    */      
	char    buy_4_price_qty      [7 ];         /* 매수4단계우선호가잔량        */          
	char    buy_5_price          [5 ];         /* 매수5단계우선호가격 (3V2)    */      
	char    buy_5_price_qty      [7 ];         /* 매수5단계우선호가잔량        */          

	char    sell_tot_best_qty      [7 ];         /* 매도총호가잔량               */          
	char    sell_1_price          [5 ];         /* 매도1단계우선호가격 (3V2)    */      
	char    sell_1_price_qty      [7 ];         /* 매도1단계우선호가잔량        */          
	char    sell_2_price          [5 ];         /* 매도2단계우선호가격 (3V2)    */      
	char    sell_2_price_qty      [7 ];         /* 매도2단계우선호가잔량        */          
	char    sell_3_price          [5 ];         /* 매도3단계우선호가격 (3V2)    */      
	char    sell_3_price_qty      [7 ];         /* 매도3단계우선호가잔량        */          
	char    sell_4_price          [5 ];         /* 매도4단계우선호가격 (3V2)    */      
	char    sell_4_price_qty      [7 ];         /* 매도4단계우선호가잔량        */          
	char    sell_5_price          [5 ];         /* 매도5단계우선호가격 (3V2)    */      
	char    sell_5_price_qty      [7 ];         /* 매도5단계우선호가잔량        */          

	char    buy_tot_best_cnt      [5 ];         /* 매수유효호가건수             */          
	char    buy_1_best_cnt        [4 ];         /* 매수1단계우선호가건수        */        
	char    buy_2_best_cnt        [4 ];         /* 매수2단계우선호가건수        */        
	char    buy_3_best_cnt        [4 ];         /* 매수3단계우선호가건수        */        
	char    buy_4_best_cnt        [4 ];         /* 매수4단계우선호가건수        */        
	char    buy_5_best_cnt        [4 ];         /* 매수5단계우선호가건수        */        

	char    sell_tot_best_cnt      [5 ];         /* 매도유효호가건수             */          
	char    sell_1_best_cnt        [4 ];         /* 매도1단계우선호가건수        */        
	char    sell_2_best_cnt        [4 ];         /* 매도2단계우선호가건수        */        
	char    sell_3_best_cnt        [4 ];         /* 매도3단계우선호가건수        */        
	char    sell_4_best_cnt        [4 ];         /* 매도4단계우선호가건수        */        
	char    sell_5_best_cnt        [4 ];         /* 매도5단계우선호가건수        */        

	char    best_amt_accept_tm    [8 ];    		/* 호가 접수시간 				*/ 
	char    expect_crprc          [5 ];    		/* 예상체결가격 				*/ 
}	SIO_B6034;				// B6034


/* ******************************************************************* */
/* 현물 */
/* ******************************************************************* */

typedef struct
{
	char	tr_gbn				      [5 ];	 	 /* TR CODE						 */
	char    item_code                 [12];      /* 종목코드 (국제표준코드)      */
	char    seq_no                    [8 ];      /* 일련번호                     */
	char    compress_code             [9 ];      /* 단축코드:'XXXXXX   '         */
	char    name                      [40];      /* 종목한글약명                 */
	char    english_name              [40];      /* 종목영문약명                 */
	char    sales_date                [8 ];      /* 영업일자                     */
	char    group_no                  [5 ];      /* 정보분배그룹번호             */
	char    group_id                  [2 ];      /* 증권그룹ID                   */
	char    trading_unit              [1 ];      /* 단위매매체결여부             */
	char    dividend_stock            [2 ];      /* 락구분                       */
	char    list_price_gubun          [2 ];      /* 액면가변경구분               */
	char    open_st_price             [1 ];      /* 시가기준가격종목여부         */
	char    revaluation_gubun         [2 ];      /* 재평가종목사유코드           */
	char    st_price_gubun            [1 ];      /* 기준가격변경여부             */
	char    option_cloase_gubun       [1 ];      /* 임의종료가능여부             */
	char    alram_notice_gubun        [1 ];      /* 시장경보위험예고여부         */
	char    alram_gubun               [2 ];      /* 시장경보구분코드             */
	char    governance_gubun          [1 ];      /* 지배구조우량여부             */
	char    admin_gubun               [1 ];      /* 관리종목여부                 */
	char    unfaithful_notice         [1 ];      /* 불성실공시지정여부           */
	char    backdoor_listing          [1 ];      /* 우회상장여부                 */
	char    trade_stop                [1 ];      /* 거래정지여부                 */
	char    index_upjong_1            [3 ];      /* 지수업종대분류               */
	char    index_upjong_2            [3 ];      /* 지수업종중분류               */
	char    index_upjong_3            [3 ];      /* 지수업종소분류               */
	char    upjong_id                 [10];      /* 업종ID                 		 */
	char    kospi200_upjong           [1 ];      /* KOSPI200세부업종             */
	char    market_capital            [1 ];      /* 시가총액규모코드             */
	char    gubun_1                   [1 ];      /* (유가)제조업여부(코스닥)중소기업여부            */
	char    krx100_stock              [1 ];      /* KRX100종목여부               */
	char    filler                    [1 ];      /* FILLER       				 */
	char    gubun_2                   [1 ];      /* (유가)지배구조지수종목여부(코스닥)소속부구분코드 */
	char    investment_agency         [2 ];      /* 투자기구구분코드             */
	char    gubun_3                   [1 ];      /* (유가)KOSPI지수종목여부 (코스닥)KOSDAQ지수종목여부 */
	char    kospi100_gubun            [1 ];      /* KOSPI100여부                 */
	char    kospi50_gubun             [1 ];      /* KOSPI50여부                  */
	char    filler_01                 [1 ];      /* filler 						 */
	char    filler_02                 [1 ];      /* filler 						 */
	char    filler_03                 [1 ];      /* filler 						 */
	char    filler_04                 [1 ];      /* filler 						 */
	char    filler_05                 [1 ];      /* filler 						 */
	char    filler_06                 [1 ];      /* filler 						 */
	char    filler_07                 [1 ];      /* filler 						 */
	char    filler_08                 [1 ];      /* filler 						 */
	char    filler_09                 [1 ];      /* filler 						 */
	char    filler_10                 [1 ];      /* filler 						 */
	char    filler_11                 [1 ];      /* filler 						 */
	char    filler_12                 [1 ];      /* filler 						 */
	char    filler_13                 [1 ];      /* filler 						 */
	char    stprc			          [9 ];      /* 기준가                       */
	char    prev_cprc_gbn             [1 ];      /* 전일종가구분코드             */
	char    prev_cprc                 [9 ];      /* 전일종가                     */
	char    prev_trading_shares       [12];      /* 전일거래량                   */
	char    prev_trading_amt          [18];      /* 전일거래대금                 */
	char    high_limit_price          [9 ];      /* 상한가                       */
	char    low_limit_price           [9 ];      /* 하한가                       */
	char    replace_price             [9 ];      /* 대용가격                     */
	char    list_price                [12];      /* 액면가 9(9)V9(3)             */
	char    issue_price               [9 ];      /* 발행가격                     */
	char    list_date                 [8 ];      /* 상장일자                     */
	char    listed_stock              [15];      /* 상장주식수                   */
	char    clear_gubun               [1 ];      /* 정리매매여부                 */
	char    eps_sign                  [1 ];      /* EPS부호                      */
	char    eps                       [9 ];      /* EPS                          */
	char    per_sign                  [1 ];      /* PER부호                      */
	char    per                       [6 ];      /* PER 9(4)V9(2)                */
	char    eps_except                [1 ];      /* EPS산출제외여부              */
	char    bps_sign                  [1 ];      /* BPS부호                      */
	char    bps                       [9 ];      /* BPS                          */
	char    pbr_sign                  [1 ];      /* PBR부호                      */
	char    pbr                       [6 ];      /* PBR 9(4)V9(2)                */
	char    bps_except                [1 ];      /* BPS산출제외여부              */
	char    loss_gubun                [1 ];      /* 결손여부                     */
	char    dividend                  [8 ];      /* 주당배당금                   */
	char    dividend_except           [1 ];      /* 주당배당금산출제외여부       */
	char    dividend_rate             [7 ];      /* 배당수익율                   */
	char    exist_start_date          [8 ];      /* 존립개시일자                 */
	char    exist_end_date            [8 ];      /* 존립종료일자                 */
	char    use_start_date            [8 ];      /* 행사기간개시일자             */
	char    use_end_date              [8 ];      /* 행사기간종료일자             */
	char    striking_price            [12];      /* ELW신주인수권증권행사가격    */
	char    capital                   [21];      /* 자본금 9(18)V9(3)            */
	char    credit_order              [1 ];      /* 신용주문가능여부             */
	char    limits_order              [5 ];      /* 지정가호가조건구분코드       */
	char    market_order              [5 ];      /* 시장가호가조건구분코드       */
	char    condition_order           [5 ];      /* 조건부지정가호가조건구분코드 */
	char    advantage_order           [5 ];      /* 최유리지정가호가조건구분코드 */
	char    first_order               [5 ];      /* 최우선지정가호가조건구분코드 */
	char    increase_gubun            [2 ];      /* 증자구분코드                 */
	char    preference_shares         [1 ];      /* 종류주권구분코드                   */
	char    national_stock            [1 ];      /* 국민주여부                   */
	char    evlt_bid                  [9 ];      /* 평가가격                     */
	char    low_bid                   [9 ];      /* 최저호가가격                 */
	char    high_bid                  [9 ];      /* 최고호가가격                 */
	char    term_qty_unit             [5 ];      /* 정규장매매수량단위           */
	char    otm_bns_qty_unit          [5 ];      /* 시간외매매수량단위           */
	char    reits_gubun               [1 ];      /* 리츠종류코드                 */
	char    obj_stock                 [12];      /* 목적주권종목코드             */
	char    filler_21                 [1 ];      /* filler */
	char    filler_22                 [3 ];      /* filler */
	char    filler_23                 [1 ];      /* filler */
	char    filler_24                 [4 ];      /* filler */
	char    currency_iso              [3 ];      /* 통화ISO코드                    */
	char    nation                    [3 ];      /* 국가코드                       */
	char    lp_canflag                [1 ];      /* 시장조성가능여부               */
	char    otm_trade                 [1 ];      /* 시간외매매가능여부             */
	char    before_otm_close_price    [1 ];      /* 장개시전시간외종가가능여부     */
	char    before_otm_block_trade    [1 ];      /* 장개시전시간외대량매매가능 여부*/
	char    before_otm_basket_trade   [1 ];      /* 장개시전시간외바스켓가능 여부  */
	char    expect_crprc_open         [1 ];      /* 예상체결가공개여부             */
	char    short_stock_selling       [1 ];      /* 공매도가능여부                 */
	char    filler_30                 [3 ];      /* FILLER                         */
	char    etf_trace_earn_rate_sign  [1 ];      /* 추적수익율배수부호             */
	char    etf_trace_earn_rate       [11];      /* 추적수익율배수                 */
	char    filler_41                 [1 ];      /* FILLER                         */
	char    filler_42                 [1 ];      /* FILLER                         */
	char    filler_43                 [1 ];      /* FILLER                         */
	char    filler_44                 [3 ];      /* FILLER                         */
	char    filler_45                 [1 ];      /* FILLER                         */
	char    filler_46                 [1 ];      /* FILLER                         */
	char    regulation_s_flag         [1 ];      /* Regulation S 적용종목여부      */
	char    spac_flag                 [1 ];      /* 기업인수목적회사여부           */
	char    asm_type                  [1 ];      /* 과세유형코드                   */
	char    replace_price_rate        [11];      /* 대용가격사정비율               */
	char    filler_51                 [1 ];      /* FILLER                         */
	char    filler_52                 [1 ];      /* FILLER                         */
	char    filler_53                 [1 ];      /* FILLER                         */
	char    filler_54                 [1 ];      /* FILLER                         */
	char    cuaution_flag             [1 ];      /* (코스닥)투자주의환기종목여부   */
	char    delist_date               [8 ];      /* 상장폐지일자                   */
	char    filler_60                 [1 ];      /* FILLER                         */
	char    short_hot_flag            [1 ];      /* 단기과열종목구분코드           */
	char    etf_copy_flag             [1 ];      /* ETF복제방법구분코드            */
	char    filler_70                 [1 ];      /* FILLER                         */
	char    k200_hi_dividend_flag     [1 ];      /* KOSPI200고배당지수여부         */
	char    k200_low_dividend_flag    [1 ];      /* KOSPI200저변동성지수여부       */
	char    filler_80                 [3 ];      /* FILLER                         */
	char    maturity_date             [8 ];      /* 만기일자                       */
	char    filler_90                 [3 ];      /* FILLER                         */
	char    distribution_cd           [2 ];      /* 분배금형태코드                 */
	char    maturity_repay_sday       [8 ];      /* 만기상환가격결정시작일자       */
	char    maturity_repay_eday       [8 ];      /* 만기상환가격결정종료일자       */
	char    etf_comm_gb_cd            [1 ];      /* ETP상품구분코드                */
	char    idex_make_cd              [2 ];      /* 지수산출기관코드               */
	char    idex_market_id            [6 ];      /* 지수시장분류ID                 */
	char    idex_seq                  [3 ];      /* 지수일련번호                   */
	char    chidex_rinvers_cd         [2 ];      /* 추적지수레버리지인버스구분코드 */
	char    atidex_rinvers_cd         [2 ];      /* 참고지수레버리지인버스구분코드 */
	char    idex_gbnid1               [6 ];      /* 지수자산분류ID1                */
	char    idex_gbnid2               [6 ];      /* 지수자산분류ID2                */
	char    lp_order_canflag          [1 ];      /* LP주문가능여부                 */
	char    k150idex_flag             [1 ];      /* KOSDAQ150지수종목여부          */
	char    low_vol_flag              [1 ];      /* 저유동성여부                   */
	char    crash_flag                [1 ];      /* 이상급등여부                   */
	char    krx300idx_flag            [1 ];      /* KRX300지수여부                 */
	char    hi_limit_cnt              [16];      /* 상한수량                       */
	char    k200_cmunst_dupflag       [1 ];      /* KOSPI200커뮤니케이션서비스섹터중복여부 */
	char    inv_inducing_flag         [1 ];      /* 투자유의종목여부               */
	char    stock_notenf_flag         [1 ];      /* 상장주식수부족종목여부         */
	char    krx_bbig_ndeal_flag       [1 ];      /* KRX BBIG K-뉴딜지수여부        */
	char    krx_second_ndeal_flag     [1 ];      /* KRX 2차전지 K-뉴딜지수여부     */
	char    krx_bio_neal_flag         [1 ];      /* KRX 바이오 K-뉴딜지수여부      */
	char    filler_99				  [125];     /* FILLER                         */
}	STOCK_A0011;				// A0011


typedef struct
{
	char	  tr_gbn			      [5 ];	   /* TR CODE					   */
	char      item_code               [12];    /* 종목코드 (국제표준코드)      */
	char      seq_no                  [5 ];    /* 종목일련번호 				   */
	char      board_id                [2 ];    /* 보드ID 					   */
	char      up_down_flag            [1 ];    /* 전일대비구분                 */
	char      raising_differ          [9 ];    /* 전일대비                     */
	char      crprc			          [9 ];    /* 체결가격                     */
	char      crprc_qty               [10];    /* 체결수량                     */
	char      session_id              [2 ];    /* 세션ID                  	   */
	char      opening_price           [9 ];    /* 시가                         */
	char      highest_price           [9 ];    /* 고가                         */
	char      lowest_price            [9 ];    /* 저가                         */
    char      tot_con_qty             [12];		/* 누적체결수량                 */  
    char      tot_con_amt             [18];		/* 누적거래대금 (단위:천원)     */  
	char      last_bidask_cd          [1 ];    /* 최종매도매수구분코드         */
	char      contract_crprc_agree    [1 ];    /* 체결가와1호가일치여부        */
	char      contract_time           [6 ];    /* 체결시각                     */
	char      lp_holding_cnt          [15];    /* LP보유수량                   */
	char      sell_1_price             [9 ];    /* 매도1호가                    */
	char      buy_1_price             [9 ];    /* 매수1호가                    */
	char      filler                  [6 ];    /* FILLER                       */
}	STOCK_A3011;				// A3011/A3012, A3021(ELW)


typedef struct
{
	char	tr_gbn					[5 ];		/* TR CODE					   	*/
	char	item_code				[12];		/* 종목코드						*/
	char	seq_no					[5 ];		/* 종목일련번호					*/
    char    tot_con_qty             [12];       /* 누적체결수량                 */  

	struct	STOCK_ARRY
	{
		char    sell_price           [9 ];       /* 매도 호가                    */
		char    buy_price           [9 ];       /* 매수 호가                    */
		char    sell_qty             [12];       /* 매도 잔량                    */
		char    buy_qty             [12];       /* 매수 잔량                    */
	}	hoga[10];								/* 10호가						*/

	char    sell10_tot_qty			[12];		/* 10단계 총매도호가 잔량		*/          
	char    buy10_tot_qty			[12];		/* 10단계 총매수호가 잔량		*/          
	char	filler1					[12];		/* filler						*/
	char	filler2					[12];		/* filler						*/
	char	mkclose_sell_tot_qty		[12];		/* 장종료후 시간외 매도총호가잔량	*/
	char	mkclose_buy_tot_qty		[12];		/* 장종료후 시간외 매수총호가잔량	*/
	char	session_id				[2 ];		/* 세션 ID						*/
	char	board_id				[2 ];		/* 보드 ID						*/
	char    expect_crprc			[9 ];    	/* 예상체결가격 				*/ 
	char    expect_crprc_qty		[12];    	/* 예상체결수량 				*/ 
	char    rvl_way_flag			[1 ];    	/* 경쟁대량 방향구분			*/ 
	char	filler3					[7 ];		/* filler						*/
}	STOCK_B6011;			// B6011


/* ******************************************************************* */
/* ELW관련 */
/* ******************************************************************* */
typedef struct
{
	char    tr_gbn					[ 5];		/* A1011(ELW)                   */
    char    item_code				[12];		/* 종목코드 (국제표준코드)      */
    char    seq_no					[ 8];		/* 일련번호                     */
    char    elw_entry_name			[80];		/* ELW발행시장참가자한글명      */
    char    elw_entry_eng_name		[80];		/* ELW발행시장참가자영문명      */
    char    elw_entry_no			[ 5];		/* ELW발행시장참가자번호        */
    struct ARRAY1{
        char   elw_asset_id			[ 3];		/* ELW구성종목시장ID			*/
    } m_id[5];
    struct ARRAY2{
        char   elw_under_asset		[12];		/* ELW기초자산종목코드          */
    } under[5];
    struct ARRAY4{
        char   elw_under_asset_rate	[12];		/* ELW기초자산구성비            */
    } rate[5];
    char    elw_market_gubun		[ 1];		/* ELW기초자산시장구분코드      */
    char    elw_idx					[ 3];		/* ETF지수업종코드              */
    char    elw_right				[ 1];		/* ELW권리유형코드              */
    char    elw_right_use_code		[ 1];		/* ELW권리행사유형코드          */
    char    elw_last_payment		[  1];		/* ELW최종결제방법코드          */
    char    elw_last_trade_date		[ 8];		/* ELW최종거래일자              */
    char    elw_payment_date		[ 8];		/* ELW지급일자                  */
    char    elw_under_asset_price	[12];		/* ELW기초자산기초가격          */
    char    elw_right_use			[200];		/* ELW권리행사내용              */
    char    elw_conversion_rate		[12];		/* ELW전환비율 9(6)V9(6)        */
    char    elw_price_up_rate		[ 8];		/* ELW가격상승참가율 9(6)V9(2)  */
    char    elw_compensation_rate	[ 8];		/* ELW보상율율 9(6)V9(2)        */
    char    elw_allowance			[21];		/* ELW확정지급액 9(18)V9(3)     */
    char    elw_payment_name		[80];		/* ELW지급대리인명              */
    char    elw_expiration_value	[200];		/* ELW만기평가가격방식          */
    char    elw_different_option	[ 1];		/* ELW이색옵션구분코드          */
    char    elw_lp_holding_cnt		[12];		/* ELW LP보유수량               */
    char    filler					[ 7];		/* FILLER                       */
}	ELW_A1011;				// A1011


typedef struct
{
	char    tr_gbn					[ 5];		/* A1011(ELW)                   */
	char    item_code				[12];		/* 종목코드 (국제표준코드)      */
    char    seq_no					[ 8];		/* 일련번호 (건수체크용)        */
    char    entry_no				[ 5];		/* 시장참가자 번호(운용사)      */
    char    lp_start_date			[ 8];		/* LP 시작일자                  */
    char    lp_end_date				[ 8];		/* LP 종료일자                  */
    char    min_multiple			[11];		/* 최소호가수량배수             */
    char    max_multiple			[11];		/* 최대호가수량배수             */
    char    sprd_cd					[ 1];		/* 호가스프레드단위코드
													(R:가격비율, Y:수익율비율(채권), 
													 T:호가가격단위 배수, 
													 A:절대값(채권) */
    char    hoga_spread				[21];		/* 호가스프레드값(9(13)V9(8)	*/
    char    huhoga_spread			[11];		/* 휴장호가스프레드배수         */
    char    duty_timeterm			[ 6];		/* 의무호가제출시간간격(sec)	*/
    char    medo_min_gum			[21];		/* 매도최소호가금액 (9(18)V9(3))*/
    char    mesu_min_gum			[21];		/* 매수최소호가금액 (9(18)V9(3))*/
    char    min_gum					[21];		/* 최소호가금액 (9(18)V9(3))	*/
    char    max_gum					[21];		/* 최대호가금액 (9(18)V9(3))	*/
    char    filler					[58];		/* FILLER                       */
}	ELW_I7011;				// I7011

/* ELW LP 포함 호가 */
typedef struct
{
    char    tr_gbn					[ 5];		/* B7011(유가증권), B7012(ELW)  */
    char    item_code				[12];		/* 종목코드 (국제표준코드)      */
	char	seq_no					[ 5];		/* 종목일련번호					*/
    char    tot_con_qty				[12];		/* 누적체결수량 (단위:주)       */
    struct  ELW_ARRY
    {
        char    sell_price			[ 9];		/* 매도호가 (단위:원)           */
        char    buy_price			[ 9];		/* 매수호가 (단위:원)           */
        char    sell_qty				[12];		/* 매도호가잔량 (단위:주)       */
        char    buy_qty				[12];		/* 매수호가잔량 (단위:주)       */
        char    LP_sell_qty			[12];		/* LP 매도호가잔량 (단위:주)    */
        char    LP_buy_qty			[12];		/* LP 매수호가잔량 (단위:주)    */
    }   hoga[10];
    char    tot_sell_qty				[12];		/* 총매도호가잔량 (단위:주)     */
    char    tot_buy_qty				[12];		/* 총매수호가잔량 (단위:주)     */
    char    filler1					[12];		/* FILLER                       */
    char    filler2					[12];		/* FILLER                       */
    char    after_otm_offer_qty		[12];		/* 장종료후시간외매도총호가잔량 */
    char    after_otm_bid_qty		[12];		/* 장종료후시간외매수총호가잔량 */
	char	session_id				[ 2];		/* 세션ID						*/
	char	board_id				[ 2];		/* 보드ID						*/
    char    expect_crprc			[ 9];		/* 예상체결가격                 */
    char    expect_crprc_qty		[12];		/* 예상체결수량                 */
	char	compete_away_gbn		[ 1];		/* 경쟁대량 방향구분 0:해당없음, 1:매도, 2:매수	*/
	char	tmp						[ 7];		/* filler						*/
}	STOCK_B7011;				// B7012
typedef	STOCK_B7011	STOCK_B7021;


typedef struct
{
    char    tr_gbn					[ 5];		/* B8011						*/
    char    item_code				[12];		/**< 종목코드					*/
    char    seq_no					[ 5];		/**< 일련번호 					*/
    char    tot_con_qty        		[12];   	/**< 누적체결수량(계약)                     */
    char    tot_sell_qty				[12];		/* 총매도호가잔량 (단위:주)     */
    char    tot_buy_qty				[12];		/* 총매수호가잔량 (단위:주)     */
}	STOCK_B8011;				// B8012

/* ***************************************************************************** */
/* 사용여부 확인필요 */
/* ETF 배치 (사무수탁배치) */
typedef struct
{
    char    tr_gbn					[ 5];		/* N8011(ETF)												*/
    char    item_code				[12];		/**< 종목코드												*/
    char    seq_no					[ 8];		/**< 일련번호 1~99999999    건수체크용						*/
    char    etf_listing				[10];		/**< ETF유통주식수											*/
    char    etf_usunamtw			[15];		/**< ETF유통순자산총액										*/
    char    etftotcap				[15];		/**< ETF순자산총액      단위:원								*/
    char    prenav					[ 9];		/**< ETF최종순자산가치 9(7)V9(2)
                                        			 송신일자의 최종NAV 자료								*/
    char    etf_usunamtf			[15];		/**< ETF외화유통순자산총액									*/
    char    etf_sunamt				[15];		/**< ETF외화순자산총액										*/
    char    etf_nav					[ 9];		/**< ETF외화최종순자산가치  9(7)V9(2)						*/
    char    etfcu					[ 8];		/**< ETF CU 수량            단위:증권						*/
    char    prebas_recprice			[ 9];		/**< 전일과표기준가격       9(7)v9(2) 2010.06.28			*/
    char    prediv_recprice			[ 9];		/**< 전일배당전과표기준가격 9(7)v9(2) 2010.06.28			*/
    char    precash_divamt			[12];		/**< 전일현금배당금액       9(10)v9(2) 2010.06.28			*/
    char    pre2bas_recprice		[ 9];		/**< 전전일과표기준가격     9(7)v9(2) 2010.06.28			*/
    char    fstprenti_recprice		[ 9];		/**< 해외주식전일비과세과표기준가 9(7)v9(2) 2016.02.29		*/
    char    fstpreb2_recprice		[ 9];		/**< 해외주식전일비과세배당전과표기준가 9(7)v9(2) 2016.02.29 */
    char    fst2prenti_recprice		[ 9];		/**< 해외주식비과세전전일과표기준가 9(7)v9(2) 2016.02.29	*/
    char    filler					[62];		/**< Filler													*/
}	STOCK_N8011;			/* N8011 */

/* M8011 ETF 운영사정보 */
typedef struct {
    char    tr_gbn					[ 5];		/**< 구분코드                                   */
    char    item_code				[12];		/**< ETF 코드                                   */
    char    seq_no					[ 8];		/**< 일련번호   1~99999999  건수체크용      	*/
    char    trusteeno				[ 3];		/**< 사무수탁 회사번호      한국예탁원:908=>903, 미래에셋펀드서 비스:049 */
    char    opercd					[ 6];		/**< 운용사 코드                                */
    char    hname					[50];		/**< 운용사 한글약명                            */
    char    ename					[40];		/**< 운용사 영문약명                            */
    char    filler					[ 5];		/**< FILLER                                 	*/
} STOCK_M8011;

/* A1041 ETN 종목배치 */
typedef struct {
    char    tr_gbn					[ 5];		/**< 구분코드                                   */
    char    item_code				[12];		/**< 종목코드                                   */
    char    seq_no					[ 8];		/**< 일련번호 1~99999999    건수체크용          */
    char    issuernmk				[80];		/**< ETN발행시장참가자한글명                    */
    char    issuernme				[80];		/**< ETN발행시장참가자영문명                    */
    char    issuerno				[ 5];		/**< ETN발행시장참가자번호                      */
    char    settletype				[ 1];		/**< ETN최종결제방법코드 						*/
    char    lastdate				[ 8];		/**< ETN최종거래일자        YYYYMMDD            */
    char    payday					[ 8];		/**< ETN지급일자            YYYYMMDD            */
    char    lp_holdvol				[12];		/**< ETN LP보유수량         					*/
    char    losslimitprofit			[ 2];		/* 손실제한ETN수익구조코드 						*/
    char    maxsang					[12];		/* ETN최대상환가격 								*/
    char    minsang					[12];		/* ETN최소상환가격 								*/
    char    rfunyn					[ 1];		/* ETN조기상환가능여부 							*/
    char    rfuncd					[ 2];		/* ETN조기상환주기코드 							*/
    char    instcode1				[ 2];		/* 평가가격산출기관코드1 						*/
    char    instcode2				[ 2];		/* 평가가격산출기관코드2 						*/
    char    filler					[47];		/**< FILLER                                 	*/
} STOCK_A1041;			// A1041

/* S1011 ETN사무수탁정보 */
typedef struct {
    char    tr_gbn					[ 5];		/**< 구분코드                                   */
    char    item_code				[12];		/**< 종목코드                                   */
    char    seq_no					[ 8];		/**< 일련번호 1~99999999    건수체크용          */
    char    last_iiv				[ 9];		/* 최종지표가치 								*/
    char    iiv_amt					[15];		/* 지표가치금액 								*/
    char    prebaseprice			[ 9];		/* 전일과표기준가격 							*/
    char    predividendprice		[ 9];		/* 전일배당전과표기준가격 						*/
    char    predividendamt			[12];		/* 전일현금배당금액 							*/
    char    pprebaseprice			[ 9];		/* 전전일가격과표기준가격 						*/
    char    varfutgubun				[ 2];		/* 변동성선물구분 0:변동성선물ETN 1:기타 		*/
    char    midrfunqty				[10];		/* 중도상환청구최소수량 						*/
    char    midrfunfee				[10];		/* 중도상환수수료율 							*/
    char    tax						[ 9];		/* 제비용 										*/
    char    filler					[10];		/**< FILLER                                 	*/
} STOCK_S1011;

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* ETN종목코드			*/
	char	time					[ 6];		/* 시간					*/
	char	preiiv					[ 9];		/* 전일IV				*/
	char	iiv						[ 9];		/* 장중/최종IV			*/
	char	filler					[28];		/* space				*/
} STOCK_S3011;		/* ETN사무수탁정보 */

/* S2011 ETN 기초지수 구성종목 */
typedef struct {
    char    tr_gbn          [ 5];   /**< 구분코드                                   */
    char    item_code         [12];   /**< 종목코드                                   */
    char    seq_no           [ 8];   /**< 일련번호 1~99999999    건수체크용              */
    char    bdate           [ 8];   /* 처리일자 */
    char    trustno         [ 3];   /* 사무수탁 회사번호 */
    char    jong_cnt        [ 4];   /* 구성종목수 */
    char    jgcode          [12];   /* 표준 종목코드    */
    char    basename        [80];   /* 종목명           */
    char    comprate        [ 7];   /* 구성비 */
    char    filler          [60];   /**< FILLER                                 */
} STOCK_S2011;


/* ******************************************************************* */
/* 주식선물 */
/* ******************************************************************* */
/* A3015 */
typedef struct
{
	char    tr_gbn          		[ 5];   /**< 구분코드                                   */
    char    item_code       		[12];   /**< 종목코드                                   */
    char    seq_no          		[ 4];   /**< 종목일련번호                               */
    char    board_id        		[ 2];   /**< 보드ID (EXTURE+)                           
                                   		     	-일반호가 G1:정규장                         
                                       		 	-일반호가 G2:장개시전시간외종가             
                                        		-일반호가 G3:장종료후시간외종가             
                                        		-일반호가 G4:장종료후시간외단일가           
                                        		-일반호가 G7:일반Buy-In                     
                                        		-일반호가 G8:당일Buy-In                     
                                        		-경쟁대량 I1:정규장                         
                                        		-경쟁대량 I2:장개시전시간외                 
                                        		-대량       B1:정규장                           
                                        		-대량       B2:장개시전시간외                   
                                        		-대량       B3:장종료후시간외                   
                                        		-바스켓   K1:정규장                         
                                        		-바스켓   K2:장개시전시간외                 
                                        		-바스켓   K3:장종료후시간외                 */
    char    crprc_sign      		[ 1];   /**< 현재가부호	                                */
    char    crprc           		[ 7];   /**< 현재가                                 	*/
    char    cvolume         		[ 6];   /**< 체결수량                                   */
    char    sid             		[ 2];   /**< 세션ID                                     */
    char    time            		[ 8];   /**< 처리시간                                   */
    char    gmjyakprice     		[ 7];   /**< 최근월물 의제약정가격                  	*/
    char    wmjyakprice     		[ 7];   /**< 원월물 의제약정가격                        */
    char    oprc_sign       		[ 1];   /**< 시가                                       */
    char    oprc            		[ 7];   /**< 시가                                       */
    char    hprc_sign       		[ 1];   /**< 고가                                       */
    char    hprc            		[ 7];   /**< 고가                                       */
    char    lprc_sign       		[ 1];   /**< 저가                                       */
    char    lprc            		[ 7];   /**< 저가                                       */
    char    before_crprc_sign		[ 1];   /**< 직전가                                 */
    char    before_crprc    		[ 7];   /**< 직전가                                 */
    char    tot_con_qty        		[ 7];   /**< 누적체결수량(계약)                     */
    char    value           		[15];   /**< 누적거래대금(천원)                     */
    char    last_bidask_cd    		[ 1];   /**< 최종매도매수구분코드 (1:매도 2:매수)   */
    char    realtime_hprc_sign		[ 1];   /**< 실시간상한가                               */
    char    realtime_hprc   		[ 7];   /**< 실시간상한가                               */
    char    realtime_lprc_sign		[ 1];   /**< 실시간하한가                               */
    char    realtime_lprc   		[ 7];   /**< 실시간하한가                               */
}	SSF_A3015;		// A3015


/* G7015 */
typedef struct {
    char    tr_gbn          		[ 5];   /**< 구분코드                                   */
    char    item_code         		[12];   /**< 종목코드                                   */
    char    seq_no           		[ 4];   /**< 순번                                       */
    char    board_id         		[ 2];   /**< 보드ID (EXTURE+)                           \n
                                        		-일반호가 G1:정규장                         \n
                                        		-일반호가 G2:장개시전시간외종가             \n
                                        		-일반호가 G3:장종료후시간외종가             \n
                                        		-일반호가 G4:장종료후시간외단일가           \n
                                        		-일반호가 G7:일반Buy-In                     \n
                                        		-일반호가 G8:당일Buy-In                     \n
                                        		-경쟁대량 I1:정규장                         \n
                                        		-경쟁대량 I2:장개시전시간외                 \n
                                        		-대량       B1:정규장                           \n
                                        		-대량       B2:장개시전시간외                   \n
                                        		-대량       B3:장종료후시간외                   \n
                                        		-바스켓   K1:정규장                         \n
                                        		-바스켓   K2:장개시전시간외                 \n*/
    char    crprc_sign           	[ 1];   /**< 현재가                                     */
    char    crprc           		[ 7];   /**< 현재가                                     */
    char    chekyul_qty     		[ 6];   /**< 체결수량                                   */
    char    session_id             	[ 2];   /**< 세션ID
                                       			*호가가 처리된 장상태                           \n
                                   					00 : 초기(장개시전) 01 : 시가단일가             \n
                                   					11 : 시가단일가연장 20 : 장중단일가             \n
                                   					21 : 장중단일가연장 30 : 종가단일가             \n
                                   					40 : 접속           99 : 장종료                 */
    char    chekyul_tm            	[ 8];   /**< 체결시각                                   */
    char    fiction_crprc1     		[ 7];   /**< 근월물 의제약정가격                        */
    char    fiction_crprc2     		[ 7];   /**< 원월물 의제약정가격                        */
    char    oprc_sign            	[ 1];   /**< 시가부호                                   */
    char    oprc            		[ 7];   /**< 시가                                       */
    char    hprc_sign            	[ 1];   /**< 고가부호                                   */
    char    hprc            		[ 7];   /**< 고가                                       */
    char    lprc_sign         		[ 1];   /**< 저가부호                                   */
    char    lprc         			[ 7];   /**< 저가                                       */
    char    before_crprc_sign      	[ 1];   /**< 직전가부호                                 */
    char    before_crprc        	[ 7];   /**< 직전가        	                            */
    char    tot_con_qty      		[ 7];   /**< 누적체결수량(계약)                     */
    char    tot_con_amt				[15];   /**< 누적거래대금(천원)                     */
    char    last_bidask_cd			[ 1];   /**< 최종매도매수구분코드 (1:매도 2:매수)   */

    char    tot_bid_jan				[ 8];   /**< 총매수 호가 잔량      */
	struct  BID_HOGA_ARRY
	{
		char    bid_sign			[ 1];	/* 매수호가부호				*/
		char    bid					[ 7];	/* 매수호가					*/
		char    bid_jan				[ 7];	/* 매수잔량					*/
	}	bid_hoga[10];

    char    tot_ask_jan				[ 8];   /**< 총매도 호가 잔량       */
	struct  ASK_HOGA_ARRY
	{
		char    ask_sign			[ 1];	/* 매도호가부호				*/
		char    ask					[ 7];	/* 매도호가					*/
		char    ask_jan				[ 7];	/* 매도잔량					*/
	}	ask_hoga[10];

    char    tot_bid_best_qty		[ 5];   /**< 매수유효호가건수		*/
	char    bid01_qty				[ 4];	/* 매수잔량					*/
	char    bid02_qty				[ 4];	/* 매수잔량					*/
	char    bid03_qty				[ 4];	/* 매수잔량					*/
	char    bid04_qty				[ 4];	/* 매수잔량					*/
	char    bid05_qty				[ 4];	/* 매수잔량					*/
	char    bid06_qty				[ 4];	/* 매수잔량					*/
	char    bid07_qty				[ 4];	/* 매수잔량					*/
	char    bid08_qty				[ 4];	/* 매수잔량					*/
	char    bid09_qty				[ 4];	/* 매수잔량					*/
	char    bid10_qty				[ 4];	/* 매수잔량					*/

    char    tot_ask_best_qty		[ 5];   /**< 매도유효호가건수		*/
	char    ask01_qty				[ 4];	/* 매도잔량					*/
	char    ask02_qty				[ 4];	/* 매도잔량					*/
	char    ask03_qty				[ 4];	/* 매도잔량					*/
	char    ask04_qty				[ 4];	/* 매도잔량					*/
	char    ask05_qty				[ 4];	/* 매도잔량					*/
	char    ask06_qty				[ 4];	/* 매도잔량					*/
	char    ask07_qty				[ 4];	/* 매도잔량					*/
	char    ask08_qty				[ 4];	/* 매도잔량					*/
	char    ask09_qty				[ 4];	/* 매도잔량					*/
	char    ask10_qty				[ 4];	/* 매도잔량					*/

    char    realtime_hprc_sign		[ 1];   /**< 실시간상한가           */
    char    realtime_hprc   		[ 7];   /**< 실시간상한가           */
    char    realtime_lprc_sign		[ 1];   /**< 실시간하한가           */
    char    realtime_lprc   		[ 7];   /**< 실시간하한가           */
}	SSF_G7015;		// G7015

/* B6015 */
typedef struct {
    char    tr_gbn          		[ 5];   /**< 구분코드                                   */
    char    item_code         		[12];   /**< 종목코드                                   */
    char    seq_no           		[ 4];   /**< 순번                                       */
    char    board_id         		[ 2];   /**< 보드ID (EXTURE+)                           \n
                                        		-일반호가 G1:정규장                         \n
                                        		-일반호가 G2:장개시전시간외종가             \n
                                        		-일반호가 G3:장종료후시간외종가             \n
                                        		-일반호가 G4:장종료후시간외단일가           \n
                                        		-일반호가 G7:일반Buy-In                     \n
                                        		-일반호가 G8:당일Buy-In                     \n
                                        		-경쟁대량 I1:정규장                         \n
                                        		-경쟁대량 I2:장개시전시간외                 \n
                                        		-대량       B1:정규장                           \n
                                        		-대량       B2:장개시전시간외                   \n
                                        		-대량       B3:장종료후시간외                   \n
                                        		-바스켓   K1:정규장                         \n
                                        		-바스켓   K2:장개시전시간외                 \n*/
    char    session_id             	[ 2];   /**< 세션ID
                                       			*호가가 처리된 장상태                           \n
                                   					00 : 초기(장개시전) 01 : 시가단일가             \n
                                   					11 : 시가단일가연장 20 : 장중단일가             \n
                                   					21 : 장중단일가연장 30 : 종가단일가             \n
                                   					40 : 접속           99 : 장종료                 */

    char    tot_bid_jan				[ 8];   /**< 총매수 호가 잔량      */
	struct
	{
		char    bid_sign			[ 1];	/* 매수호가부호				*/
		char    bid					[ 7];	/* 매수호가					*/
		char    bid_jan				[ 7];	/* 매수잔량					*/
	}	bid_hoga[10];

    char    tot_ask_jan				[ 8];   /**< 총매도 호가 잔량       */
	struct
	{
		char    ask_sign			[ 1];	/* 매도호가부호				*/
		char    ask					[ 7];	/* 매도호가					*/
		char    ask_jan				[ 7];	/* 매도잔량					*/
	}	ask_hoga[10];

    char    tot_bid_best_qty		[ 5];   /**< 매수유효호가건수		*/
	char    bid01_qty				[ 4];	/* 매수잔량					*/
	char    bid02_qty				[ 4];	/* 매수잔량					*/
	char    bid03_qty				[ 4];	/* 매수잔량					*/
	char    bid04_qty				[ 4];	/* 매수잔량					*/
	char    bid05_qty				[ 4];	/* 매수잔량					*/
	char    bid06_qty				[ 4];	/* 매수잔량					*/
	char    bid07_qty				[ 4];	/* 매수잔량					*/
	char    bid08_qty				[ 4];	/* 매수잔량					*/
	char    bid09_qty				[ 4];	/* 매수잔량					*/
	char    bid10_qty				[ 4];	/* 매수잔량					*/

    char    tot_ask_best_qty		[ 5];   /**< 매도유효호가건수		*/
	char    ask01_qty				[ 4];	/* 매도잔량					*/
	char    ask02_qty				[ 4];	/* 매도잔량					*/
	char    ask03_qty				[ 4];	/* 매도잔량					*/
	char    ask04_qty				[ 4];	/* 매도잔량					*/
	char    ask05_qty				[ 4];	/* 매도잔량					*/
	char    ask06_qty				[ 4];	/* 매도잔량					*/
	char    ask07_qty				[ 4];	/* 매도잔량					*/
	char    ask08_qty				[ 4];	/* 매도잔량					*/
	char    ask09_qty				[ 4];	/* 매도잔량					*/
	char    ask10_qty				[ 4];	/* 매도잔량					*/

	char    best_amt_accept_tm		[ 8];      /* 호가접수시각                 */

    char    expect_crprc_sign		[ 1];   /**< 예상체결가격부호		*/
    char    expect_crprc	   		[ 7];   /**< 예상체결가격			*/
}	SSF_B6015;		// B6015


/* A3025 */
typedef struct
{
	char    tr_gbn          		[ 5];   /**< 구분코드                                   */
    char    item_code       		[12];   /**< 종목코드                                   */
    char    seq_no          		[ 5];   /**< 종목일련번호                               */
    char    board_id        		[ 2];   /**< 보드ID (EXTURE+)                           
                                   		     	-일반호가 G1:정규장                         
                                       		 	-일반호가 G2:장개시전시간외종가             
                                        		-일반호가 G3:장종료후시간외종가             
                                        		-일반호가 G4:장종료후시간외단일가           
                                        		-일반호가 G7:일반Buy-In                     
                                        		-일반호가 G8:당일Buy-In                     
                                        		-경쟁대량 I1:정규장                         
                                        		-경쟁대량 I2:장개시전시간외                 
                                        		-대량       B1:정규장                           
                                        		-대량       B2:장개시전시간외                   
                                        		-대량       B3:장종료후시간외                   
                                        		-바스켓   K1:정규장                         
                                        		-바스켓   K2:장개시전시간외                 
                                        		-바스켓   K3:장종료후시간외                 */
    char    crprc           		[ 7];   /**< 현재가                                 	*/
    char    cvolume         		[ 7];   /**< 체결수량                                   */
    char    sid             		[ 2];   /**< 세션ID                                     */
    char    time            		[ 8];   /**< 처리시간                                   */
    char    oprc            		[ 7];   /**< 시가                                       */
    char    hprc            		[ 7];   /**< 고가                                       */
    char    lprc            		[ 7];   /**< 저가                                       */
    char    before_crprc    		[ 7];   /**< 직전가                                 */
    char    tot_con_qty        		[ 7];   /**< 누적체결수량(계약)                     */
    char    value           		[15];   /**< 누적거래대금(천원)                     */
    char    last_bidask_cd    		[ 1];   /**< 최종매도매수구분코드 (1:매도 2:매수)   */
    char    realtime_hprc   		[ 7];   /**< 실시간상한가                               */
    char    realtime_lprc   		[ 7];   /**< 실시간하한가                               */
}	SSO_A3025;		// A3025

/* G7025 */
typedef struct {
    char    tr_gbn          		[ 5];   /**< 구분코드                                   */
    char    item_code         		[12];   /**< 종목코드                                   */
    char    seq_no           		[ 5];   /**< 순번                                       */
    char    board_id         		[ 2];   /**< 보드ID (EXTURE+)                           \n
                                        		-일반호가 G1:정규장                         \n
                                        		-일반호가 G2:장개시전시간외종가             \n
                                        		-일반호가 G3:장종료후시간외종가             \n
                                        		-일반호가 G4:장종료후시간외단일가           \n
                                        		-일반호가 G7:일반Buy-In                     \n
                                        		-일반호가 G8:당일Buy-In                     \n
                                        		-경쟁대량 I1:정규장                         \n
                                        		-경쟁대량 I2:장개시전시간외                 \n
                                        		-대량       B1:정규장                           \n
                                        		-대량       B2:장개시전시간외                   \n
                                        		-대량       B3:장종료후시간외                   \n
                                        		-바스켓   K1:정규장                         \n
                                        		-바스켓   K2:장개시전시간외                 \n*/
    char    crprc           		[ 7];   /**< 현재가                                     */
    char    chekyul_qty     		[ 7];   /**< 체결수량                                   */
    char    session_id             	[ 2];   /**< 세션ID
                                       			*호가가 처리된 장상태                           \n
                                   					00 : 초기(장개시전) 01 : 시가단일가             \n
                                   					11 : 시가단일가연장 20 : 장중단일가             \n
                                   					21 : 장중단일가연장 30 : 종가단일가             \n
                                   					40 : 접속           99 : 장종료                 */
    char    chekyul_tm            	[ 8];   /**< 체결시각                                   */
    char    oprc            		[ 7];   /**< 시가                                       */
    char    hprc            		[ 7];   /**< 고가                                       */
    char    lprc         			[ 7];   /**< 저가                                       */
    char    before_crprc        	[ 7];   /**< 직전가        	                            */
    char    tot_con_qty      		[ 7];   /**< 누적체결수량(계약)                     */
    char    tot_con_amt				[15];   /**< 누적거래대금(천원)                     */
    char    last_bidask_cd			[ 1];   /**< 최종매도매수구분코드 (1:매도 2:매수)   */

    char    tot_bid_jan				[ 7];   /**< 총매수 호가 잔량      */
	struct
	{
		char    bid					[ 7];	/* 매수호가					*/
		char    bid_jan				[ 7];	/* 매수잔량					*/
	}	bid_hoga[10];

    char    tot_ask_jan				[ 7];   /**< 총매도 호가 잔량       */
	struct
	{
		char    ask					[ 7];	/* 매도호가					*/
		char    ask_jan				[ 7];	/* 매도잔량					*/
	}	ask_hoga[10];

    char    tot_bid_best_qty		[ 6];   /**< 매수유효호가건수		*/
	char    bid01_qty				[ 5];	/* 매수잔량					*/
	char    bid02_qty				[ 5];	/* 매수잔량					*/
	char    bid03_qty				[ 5];	/* 매수잔량					*/
	char    bid04_qty				[ 5];	/* 매수잔량					*/
	char    bid05_qty				[ 5];	/* 매수잔량					*/
	char    bid06_qty				[ 5];	/* 매수잔량					*/
	char    bid07_qty				[ 5];	/* 매수잔량					*/
	char    bid08_qty				[ 5];	/* 매수잔량					*/
	char    bid09_qty				[ 5];	/* 매수잔량					*/
	char    bid10_qty				[ 5];	/* 매수잔량					*/

    char    tot_ask_best_qty		[ 6];   /**< 매도유효호가건수		*/
	char    ask01_qty				[ 5];	/* 매도잔량					*/
	char    ask02_qty				[ 5];	/* 매도잔량					*/
	char    ask03_qty				[ 5];	/* 매도잔량					*/
	char    ask04_qty				[ 5];	/* 매도잔량					*/
	char    ask05_qty				[ 5];	/* 매도잔량					*/
	char    ask06_qty				[ 5];	/* 매도잔량					*/
	char    ask07_qty				[ 5];	/* 매도잔량					*/
	char    ask08_qty				[ 5];	/* 매도잔량					*/
	char    ask09_qty				[ 5];	/* 매도잔량					*/
	char    ask10_qty				[ 5];	/* 매도잔량					*/

    char    realtime_hprc   		[ 7];   /**< 실시간상한가           */
    char    realtime_lprc   		[ 7];   /**< 실시간하한가           */
}	SSO_G7025;		// G7025


/* B6025 */
typedef struct {
    char    tr_gbn          		[ 5];   /**< 구분코드                                   */
    char    item_code         		[12];   /**< 종목코드                                   */
    char    seq_no           		[ 5];   /**< 순번                                       */
    char    board_id         		[ 2];   /**< 보드ID (EXTURE+)                           \n
                                        		-일반호가 G1:정규장                         \n
                                        		-일반호가 G2:장개시전시간외종가             \n
                                        		-일반호가 G3:장종료후시간외종가             \n
                                        		-일반호가 G4:장종료후시간외단일가           \n
                                        		-일반호가 G7:일반Buy-In                     \n
                                        		-일반호가 G8:당일Buy-In                     \n
                                        		-경쟁대량 I1:정규장                         \n
                                        		-경쟁대량 I2:장개시전시간외                 \n
                                        		-대량       B1:정규장                           \n
                                        		-대량       B2:장개시전시간외                   \n
                                        		-대량       B3:장종료후시간외                   \n
                                        		-바스켓   K1:정규장                         \n
                                        		-바스켓   K2:장개시전시간외                 \n*/
    char    session_id             	[ 2];   /**< 세션ID
                                       			*호가가 처리된 장상태                           \n
                                   					00 : 초기(장개시전) 01 : 시가단일가             \n
                                   					11 : 시가단일가연장 20 : 장중단일가             \n
                                   					21 : 장중단일가연장 30 : 종가단일가             \n
                                   					40 : 접속           99 : 장종료                 */

    char    tot_bid_jan				[ 7];   /**< 총매수 호가 잔량      */
	struct
	{
		char    bid					[ 7];	/* 매수호가					*/
		char    bid_jan				[ 7];	/* 매수잔량					*/
	}	bid_hoga[10];

    char    tot_ask_jan				[ 7];   /**< 총매도 호가 잔량       */
	struct
	{
		char    ask					[ 7];	/* 매도호가					*/
		char    ask_jan				[ 7];	/* 매도잔량					*/
	}	ask_hoga[10];

    char    tot_bid_best_qty		[ 6];   /**< 매수유효호가건수		*/
	char    bid01_qty				[ 5];	/* 매수잔량					*/
	char    bid02_qty				[ 5];	/* 매수잔량					*/
	char    bid03_qty				[ 5];	/* 매수잔량					*/
	char    bid04_qty				[ 5];	/* 매수잔량					*/
	char    bid05_qty				[ 5];	/* 매수잔량					*/
	char    bid06_qty				[ 5];	/* 매수잔량					*/
	char    bid07_qty				[ 5];	/* 매수잔량					*/
	char    bid08_qty				[ 5];	/* 매수잔량					*/
	char    bid09_qty				[ 5];	/* 매수잔량					*/
	char    bid10_qty				[ 5];	/* 매수잔량					*/

    char    tot_ask_best_qty		[ 6];   /**< 매도유효호가건수		*/
	char    ask01_qty				[ 5];	/* 매도잔량					*/
	char    ask02_qty				[ 5];	/* 매도잔량					*/
	char    ask03_qty				[ 5];	/* 매도잔량					*/
	char    ask04_qty				[ 5];	/* 매도잔량					*/
	char    ask05_qty				[ 5];	/* 매도잔량					*/
	char    ask06_qty				[ 5];	/* 매도잔량					*/
	char    ask07_qty				[ 5];	/* 매도잔량					*/
	char    ask08_qty				[ 5];	/* 매도잔량					*/
	char    ask09_qty				[ 5];	/* 매도잔량					*/
	char    ask10_qty				[ 5];	/* 매도잔량					*/

	char    best_amt_accept_tm		[ 8];      /* 호가접수시각                 */

    char    expect_crprc	   		[ 7];   /**< 예상체결가격			*/
}	SSO_B6025;		// B6025

/* 지수 */
typedef struct
{
    char    tr_gbn					[ 5];		/*                              */
    char    up_gbn					[ 3];		/* 업종코드(01~06등)            */
    char    time					[ 6];		/* 시각 (PREJJJ:장전예상지수종료,
                                         			HHMMSS:장중, JUNJJJ:장종료/
                                           			장종료예상지수종료,
                                            		EXTJJJ:시간외종료)          */
    char    idx						[ 8];		/* 지수 (6V2)                   */
    char    sign					[ 1];		/* 부호 ('+':상승, "-":하락,
                                            		" ":보합)                   */
    char    dif						[ 8];		/* 대비 (6V2)                   */
    char    qty						[ 8];		/* 거래량, 체결량 (단위:천주)   */
    char    amt						[ 8];		/* 거래대금 (단위:백만원)       */
	char	tmp						[ 2];		/* filler						*/
}   STOCK_JISU; /* 거래소,코스닥 지수 D0011/D2011... */

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 5];		/* 종목일련번호			*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	board_event_id			[ 3];		/* 보드이벤트ID			*/
	char	board_event_st			[ 6];		/* 보드이벤트ID 시작시간*/
	char	board_event_cd			[ 5];		/* 보드이벤트ID 적용군코드*/
	char	session_startend_cd		[ 2];		/* 세션개시종료코드		*/
												/* BS:보드개시
													BE:보드종료
													SS:세션개시
													SE:세션종료
													SH:세션정지
													SR:세션재개			*/
	char	session_id				[ 2];		/* 세션ID				*/
	char	s_item_code				[12];		/* 상장사 종목코드		*/
    char	trade_stop_rcd			[ 3];		/* 거래정지사유코드		*/
	char	trade_stop_tcd			[ 1];		/* 거래정지발생유형코드	*/
	char	apply_depth				[ 2];		/* 적용단계				*/
}	M4011;	// M4011/012 공통
typedef	M4011 M4012;

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 3];		/* 종목일련번호			*/
	char	prod_id					[11];		/* 상품ID				*/
	char	mkprod_gid				[ 3];		/* 장운영상품그룹ID		*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	board_event_id			[ 3];		/* 보드이벤트ID			*/
	char	board_event_st			[ 8];		/* 보드이벤트ID 시작시간*/
	char	board_event_cd			[ 5];		/* 보드이벤트ID 적용군코드*/
	char	session_startend_cd		[ 2];		/* 세션개시종료코드		*/
												/* BS:보드개시
													BE:보드종료
													SS:세션개시
													SE:세션종료
													SH:세션정지
													SR:세션재개			*/
	char	session_id				[ 2];		/* 세션ID				*/
	char	apply_depth				[ 2];		/* 적용단계				*/
	char	basic_itm_way			[ 1];		/* 기준종목가격확대발생코드 */
	char	price_ext_tm			[ 8];		/* 가격확대예정시각		*/
}	M4014;		// M4124/015/016 공통

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 4];		/* 종목일련번호			*/
	char	prod_id					[11];		/* 상품ID				*/
	char	mkprod_gid				[ 3];		/* 장운영상품그룹ID		*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	board_event_id			[ 3];		/* 보드이벤트ID			*/
	char	board_event_st			[ 8];		/* 보드이벤트ID 시작시간*/
	char	board_event_cd			[ 5];		/* 보드이벤트ID 적용군코드*/
	char	session_startend_cd		[ 2];		/* 세션개시종료코드		*/
												/* BS:보드개시
													BE:보드종료
													SS:세션개시
													SE:세션종료
													SH:세션정지
													SR:세션재개			*/
	char	session_id				[ 2];		/* 세션ID				*/
	char	apply_depth				[ 2];		/* 적용단계				*/
	char	basic_itm_way			[ 1];		/* 기준종목가격확대발생코드 */
	char	price_ext_tm			[ 8];		/* 가격확대예정시각		*/
}	M4034;	

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 5];		/* 종목일련번호			*/
	char	prod_id					[11];		/* 상품ID				*/
	char	mkprod_gid				[ 3];		/* 장운영상품그룹ID		*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	board_event_id			[ 3];		/* 보드이벤트ID			*/
	char	board_event_st			[ 8];		/* 보드이벤트ID 시작시간*/
	char	board_event_cd			[ 5];		/* 보드이벤트ID 적용군코드*/
	char	session_startend_cd		[ 2];		/* 세션개시종료코드		*/
												/* BS:보드개시
													BE:보드종료
													SS:세션개시
													SE:세션종료
													SH:세션정지
													SR:세션재개			*/
	char	session_id				[ 2];		/* 세션ID				*/
	char	apply_depth				[ 2];		/* 적용단계				*/
	char	basic_itm_way			[ 1];		/* 기준종목가격확대발생코드 */
	char	price_ext_tm			[ 8];		/* 가격확대예정시각		*/
}	M4025;		// M4015/025 공통


typedef struct
{
	char	tr_gbn					[ 5];		/* TR코드				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 5];		/* 종목일련번호			*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	basic_price				[ 9];		/* 기준가격				*/
	char	basic_hprc				[ 9];		/* 기준가격상한가		*/
	char	basic_lprc				[ 9];		/* 기준가격하한가		*/
	
}	STOCK_A4011;	// 기준가결정, A4011, A4012 함께 사용

/* 시장별 A0, A3, G7, B6, M4, V1, .... A4는 현물만 있다 */
/* ********************** */
/* KRX 300 */
typedef struct {
	char	tr_gbn					[ 5];		/* TR코드 				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 2];		/* 종목일련번호			*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	crprc_sign				[ 1];		/* 현재가부호			*/
	char	crprc					[ 6];		/* 현재가				*/
	char	cvolume					[ 6];		/* 건별체결수량			*/
	char	sid						[ 2];		/* 세션ID				*/
	char	time					[ 8];		/* 체결시각				*/
	char	gmjyakprice				[ 6];		/* 근월물 의제약정가격	*/
	char	wmjyakprice				[ 6];		/* 원월물 의제약정가격	*/
	char	oprc_sign				[ 1];		/* 시가부호				*/
	char	oprc					[ 6];		/* 시가					*/
	char	hprc_sign				[ 1];		/* 고가부호				*/
	char	hprc					[ 6];		/* 고가					*/
	char	lprc_sign				[ 1];		/* 저가부호				*/
	char	lprc					[ 6];		/* 저가					*/
	char	before_crprc_sign		[ 1];		/* 직전가부호			*/
	char	before_crprc			[ 6];		/* 직전가				*/
	char	tot_con_qty				[ 7];		/* 누적체결수량(계약)	*/
	char	value					[11];		/* 누적거래대금(천원)	*/
	char	blocktdvolall			[ 7];		/* 협의대량누적체결수량	*/
	char	lastflag				[ 1];		/* 최종매도매수구분코드(1매도)	*/
	char	realtime_hprc_sign		[ 1];		/* 실시간상한가			*/
	char	realtime_hprc			[ 6];		/* 실시간상한가부호		*/
	char	realtime_lprc_sign		[ 1];		/* 실시간하한가			*/
	char	realtime_lprc			[ 6];		/* 실시간하한가부호		*/
}	A3164;
/* KRX 300 */
/* ******* */

/* ****************** */
/* Kosdaq 150 Futures */
typedef struct {
	char	tr_gbn					[ 5];		/* TR코드 				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 2];		/* 종목일련번호			*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	crprc_sign				[ 1];		/* 현재가부호			*/
	char	crprc					[ 6];		/* 현재가				*/
	char	cvolume					[ 6];		/* 건별체결수량			*/
	char	sid						[ 2];		/* 세션ID				*/
	char	time					[ 8];		/* 체결시각				*/
	char	gmjyakprice				[ 6];		/* 근월물 의제약정가격	*/
	char	wmjyakprice				[ 6];		/* 원월물 의제약정가격	*/
	char	oprc_sign				[ 1];		/* 시가부호				*/
	char	oprc					[ 6];		/* 시가					*/
	char	hprc_sign				[ 1];		/* 고가부호				*/
	char	hprc					[ 6];		/* 고가					*/
	char	lprc_sign				[ 1];		/* 저가부호				*/
	char	lprc					[ 6];		/* 저가					*/
	char	before_crprc_sign		[ 1];		/* 직전가부호			*/
	char	before_crprc			[ 6];		/* 직전가				*/
	char	tot_con_qty				[ 7];		/* 누적체결수량(계약)	*/
	char	value					[11];		/* 누적거래대금(천원)	*/
	char	filler					[ 7];		/* filler				*/
	char	lastflag				[ 1];		/* 최종매도매수구분코드(1매도)	*/
	char	realtime_hprc_sign		[ 1];		/* 실시간상한가			*/
	char	realtime_hprc			[ 6];		/* 실시간상한가부호		*/
	char	realtime_lprc_sign		[ 1];		/* 실시간하한가			*/
	char	realtime_lprc			[ 6];		/* 실시간하한가부호		*/
}	A3024;

typedef struct {
	char	tr_gbn					[ 5];		/* TR코드 				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 2];		/* 종목일련번호			*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	crprc_sign				[ 1];		/* 현재가부호			*/
	char	crprc					[ 6];		/* 현재가				*/
	char	cvolume					[ 6];		/* 건별체결수량			*/
	char	sid						[ 2];		/* 세션ID				*/
	char	time					[ 8];		/* 체결시각				*/
	char	gmjyakprice				[ 6];		/* 근월물 의제약정가격	*/
	char	wmjyakprice				[ 6];		/* 원월물 의제약정가격	*/
	char	oprc_sign				[ 1];		/* 시가부호				*/
	char	oprc					[ 6];		/* 시가					*/
	char	hprc_sign				[ 1];		/* 고가부호				*/
	char	hprc					[ 6];		/* 고가					*/
	char	lprc_sign				[ 1];		/* 저가부호				*/
	char	lprc					[ 6];		/* 저가					*/
	char	before_crprc_sign		[ 1];		/* 직전가부호			*/
	char	before_crprc			[ 6];		/* 직전가				*/
	char	tot_con_qty				[ 7];		/* 누적체결수량(계약)	*/
	char	value					[11];		/* 누적거래대금(천원)	*/
	char	filler					[ 7];		/* filler				*/
	char	lastflag				[ 1];		/* 최종매도매수구분코드(1매도)	*/

	char    buy_tot_best_qty      	[6 ];         /* 매수총호가잔량               */          
	char    buy_1_sign            	[1 ];         /* 매수1단계부호                */    
	char    buy_1_price          	[6 ];         /* 매수1단계우선호가격 (4V2)    */      
	char    buy_1_price_qty      	[6 ];         /* 매수1단계우선호가잔량        */          
	char    buy_2_sign            	[1 ];         /* 매수2단계부호                */    
	char    buy_2_price          	[6 ];         /* 매수2단계우선호가격 (4V2)    */      
	char    buy_2_price_qty      	[6 ];         /* 매수2단계우선호가잔량        */          
	char    buy_3_sign            	[1 ];         /* 매수3단계부호                */    
	char    buy_3_price          	[6 ];         /* 매수3단계우선호가격 (4V2)    */      
	char    buy_3_price_qty      	[6 ];         /* 매수3단계우선호가잔량        */          
	char    buy_4_sign            	[1 ];         /* 매수4단계부호                */    
	char    buy_4_price          	[6 ];         /* 매수4단계우선호가격 (4V2)    */      
	char    buy_4_price_qty      	[6 ];         /* 매수4단계우선호가잔량        */          
	char    buy_5_sign            	[1 ];         /* 매수5단계부호                */    
	char    buy_5_price          	[6 ];         /* 매수5단계우선호가격 (4V2)    */      
	char    buy_5_price_qty      	[6 ];         /* 매수5단계우선호가잔량        */          

	char    sell_tot_best_qty      	[6 ];         /* 매도총호가잔량               */          
	char    sell_1_sign            	[1 ];         /* 매도1단계부호                */    
	char    sell_1_price          	[6 ];         /* 매도1단계우선호가격 (4V2)    */      
	char    sell_1_price_qty      	[6 ];         /* 매도1단계우선호가잔량        */          
	char    sell_2_sign            	[1 ];         /* 매도2단계부호                */    
	char    sell_2_price          	[6 ];         /* 매도2단계우선호가격 (4V2)    */      
	char    sell_2_price_qty      	[6 ];         /* 매도2단계우선호가잔량        */          
	char    sell_3_sign            	[1 ];         /* 매도3단계부호                */    
	char    sell_3_price          	[6 ];         /* 매도3단계우선호가격 (4V2)    */      
	char    sell_3_price_qty      	[6 ];         /* 매도3단계우선호가잔량        */          
	char    sell_4_sign            	[1 ];         /* 매도4단계부호                */    
	char    sell_4_price          	[6 ];         /* 매도4단계우선호가격 (4V2)    */      
	char    sell_4_price_qty      	[6 ];         /* 매도4단계우선호가잔량        */          
	char    sell_5_sign            	[1 ];         /* 매도5단계부호                */    
	char    sell_5_price          	[6 ];         /* 매도5단계우선호가격 (4V2)    */      
	char    sell_5_price_qty      	[6 ];         /* 매도5단계우선호가잔량        */          

	char    buy_tot_best_cnt      	[5 ];         /* 매수유효호가건수             */          
	char    buy_1_best_cnt        	[4 ];         /* 매수1단계우선호가건수        */        
	char    buy_2_best_cnt        	[4 ];         /* 매수2단계우선호가건수        */        
	char    buy_3_best_cnt        	[4 ];         /* 매수3단계우선호가건수        */        
	char    buy_4_best_cnt        	[4 ];         /* 매수4단계우선호가건수        */        
	char    buy_5_best_cnt        	[4 ];         /* 매수5단계우선호가건수        */        
	char    sell_tot_best_cnt      	[5 ];         /* 매도유효호가건수             */          
	char    sell_1_best_cnt        	[4 ];         /* 매도1단계우선호가건수        */        
	char    sell_2_best_cnt        	[4 ];         /* 매도2단계우선호가건수        */        
	char    sell_3_best_cnt        	[4 ];         /* 매도3단계우선호가건수        */        
	char    sell_4_best_cnt        	[4 ];         /* 매도4단계우선호가건수        */        
	char    sell_5_best_cnt        	[4 ];         /* 매도5단계우선호가건수        */        
	char	realtime_hprc_sign		[ 1];		  /* 실시간상한가			*/
	char	realtime_hprc			[ 6];		  /* 실시간상한가부호		*/
	char	realtime_lprc_sign		[ 1];		  /* 실시간하한가			*/
	char	realtime_lprc			[ 6];		  /* 실시간하한가부호		*/
}	G7024;

typedef struct {
	char	tr_gbn					[ 5];		/* TR코드 				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 2];		/* 종목일련번호			*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	sid						[ 2];		/* 세션ID				*/

	char    buy_tot_best_qty      	[6 ];         /* 매수총호가잔량               */          
	char    buy_1_sign            	[1 ];         /* 매수1단계부호                */    
	char    buy_1_price          	[6 ];         /* 매수1단계우선호가격 (4V2)    */      
	char    buy_1_price_qty      	[6 ];         /* 매수1단계우선호가잔량        */          
	char    buy_2_sign            	[1 ];         /* 매수2단계부호                */    
	char    buy_2_price          	[6 ];         /* 매수2단계우선호가격 (4V2)    */      
	char    buy_2_price_qty      	[6 ];         /* 매수2단계우선호가잔량        */          
	char    buy_3_sign            	[1 ];         /* 매수3단계부호                */    
	char    buy_3_price          	[6 ];         /* 매수3단계우선호가격 (4V2)    */      
	char    buy_3_price_qty      	[6 ];         /* 매수3단계우선호가잔량        */          
	char    buy_4_sign            	[1 ];         /* 매수4단계부호                */    
	char    buy_4_price          	[6 ];         /* 매수4단계우선호가격 (4V2)    */      
	char    buy_4_price_qty      	[6 ];         /* 매수4단계우선호가잔량        */          
	char    buy_5_sign            	[1 ];         /* 매수5단계부호                */    
	char    buy_5_price          	[6 ];         /* 매수5단계우선호가격 (4V2)    */      
	char    buy_5_price_qty      	[6 ];         /* 매수5단계우선호가잔량        */          

	char    sell_tot_best_qty      	[6 ];         /* 매도총호가잔량               */          
	char    sell_1_sign            	[1 ];         /* 매도1단계부호                */    
	char    sell_1_price          	[6 ];         /* 매도1단계우선호가격 (4V2)    */      
	char    sell_1_price_qty      	[6 ];         /* 매도1단계우선호가잔량        */          
	char    sell_2_sign            	[1 ];         /* 매도2단계부호                */    
	char    sell_2_price          	[6 ];         /* 매도2단계우선호가격 (4V2)    */      
	char    sell_2_price_qty      	[6 ];         /* 매도2단계우선호가잔량        */          
	char    sell_3_sign            	[1 ];         /* 매도3단계부호                */    
	char    sell_3_price          	[6 ];         /* 매도3단계우선호가격 (4V2)    */      
	char    sell_3_price_qty      	[6 ];         /* 매도3단계우선호가잔량        */          
	char    sell_4_sign            	[1 ];         /* 매도4단계부호                */    
	char    sell_4_price          	[6 ];         /* 매도4단계우선호가격 (4V2)    */      
	char    sell_4_price_qty      	[6 ];         /* 매도4단계우선호가잔량        */          
	char    sell_5_sign            	[1 ];         /* 매도5단계부호                */    
	char    sell_5_price          	[6 ];         /* 매도5단계우선호가격 (4V2)    */      
	char    sell_5_price_qty      	[6 ];         /* 매도5단계우선호가잔량        */          

	char    buy_tot_best_cnt      	[5 ];         /* 매수유효호가건수             */          
	char    buy_1_best_cnt        	[4 ];         /* 매수1단계우선호가건수        */        
	char    buy_2_best_cnt        	[4 ];         /* 매수2단계우선호가건수        */        
	char    buy_3_best_cnt        	[4 ];         /* 매수3단계우선호가건수        */        
	char    buy_4_best_cnt        	[4 ];         /* 매수4단계우선호가건수        */        
	char    buy_5_best_cnt        	[4 ];         /* 매수5단계우선호가건수        */        
	char    sell_tot_best_cnt      	[5 ];         /* 매도유효호가건수             */          
	char    sell_1_best_cnt        	[4 ];         /* 매도1단계우선호가건수        */        
	char    sell_2_best_cnt        	[4 ];         /* 매도2단계우선호가건수        */        
	char    sell_3_best_cnt        	[4 ];         /* 매도3단계우선호가건수        */        
	char    sell_4_best_cnt        	[4 ];         /* 매도4단계우선호가건수        */        
	char    sell_5_best_cnt        	[4 ];         /* 매도5단계우선호가건수        */        

	char	time					[ 8];		  /* 호가접수시간				  */
	char	yprice_sign				[ 1];		  /* 예상체결가격부호			  */
	char	yprice					[ 6];		  /* 예상체결가격				  */
}	B6024;

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 3];		/* 종목일련번호			*/
	char	prod_id					[11];		/* 상품ID				*/
	char	mkprod_gid				[ 3];		/* 장운영상품그룹ID		*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	board_event_id			[ 3];		/* 보드이벤트ID			*/
	char	board_event_st			[ 8];		/* 보드이벤트ID 시작시간*/
	char	board_event_cd			[ 5];		/* 보드이벤트ID 적용군코드*/
	char	session_startend_cd		[ 2];		/* 세션개시종료코드		*/
												/* BS:보드개시
													BE:보드종료
													SS:세션개시
													SE:세션종료
													SH:세션정지
													SR:세션재개			*/
	char	session_id				[ 2];		/* 세션ID				*/
	char	apply_depth				[ 2];		/* 적용단계				*/
	char	basic_itm_way			[ 1];		/* 기준종목가격확대발생코드 */
	char	price_ext_tm			[ 8];		/* 가격확대예정시각		*/
}	M4024;

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 3];		/* 종목일련번호			*/
	char	prod_id					[11];		/* 상품ID				*/
	char	mkprod_gid				[ 3];		/* 장운영상품그룹ID		*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	cexttime				[ 8];		/* 가격확대시간			*/
	char	uplmtpricestep			[ 2];		/* 가격제한확대상한단계 */
	char	dnlmtpricestep			[ 2];		/* 가격제한확대상한단계 */
	char	uplmtprice_sign			[ 1];		/* 상한가부호 			*/
	char	uplmtprice				[ 6];		/* 상한가				*/
	char	dnlmtprice_sign			[ 1];		/* 하한가부호 			*/
	char	dnlmtprice				[ 6];		/* 하한가				*/
}	V1024;
/* Kosdaq 150 Futures */
/* ****************** */

/* ****************** */
/* Kosdaq 150 Options */
typedef struct {
	char	tr_gbn					[ 5];		/* TR코드 				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 4];		/* 종목일련번호			*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	crprc					[ 6];		/* 현재가				*/
	char	cvolume					[ 6];		/* 건별체결수량			*/
	char	sid						[ 2];		/* 세션ID				*/
	char	time					[ 8];		/* 체결시각				*/
	char	oprc					[ 6];		/* 시가					*/
	char	hprc					[ 6];		/* 고가					*/
	char	lprc					[ 6];		/* 저가					*/
	char	before_crprc			[ 6];		/* 직전가				*/
	char	tot_con_qty				[ 7];		/* 누적체결수량(계약)	*/
	char	value					[11];		/* 누적거래대금(천원)	*/
	char	filler					[ 7];		/* filler				*/
	char	lastflag				[ 1];		/* 최종매도매수구분코드(1매도)	*/
	char	realtime_hprc			[ 6];		/* 실시간상한가부호		*/
	char	realtime_lprc			[ 6];		/* 실시간하한가부호		*/
}	A3174;	/* Kosdaq150 Options */

typedef struct {
	char	tr_gbn					[ 5];		/* TR코드 				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 4];		/* 종목일련번호			*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	crprc					[ 6];		/* 현재가				*/
	char	cvolume					[ 6];		/* 건별체결수량			*/
	char	sid						[ 2];		/* 세션ID				*/
	char	time					[ 8];		/* 체결시각				*/
	char	oprc					[ 6];		/* 시가					*/
	char	hprc					[ 6];		/* 고가					*/
	char	lprc					[ 6];		/* 저가					*/
	char	before_crprc			[ 6];		/* 직전가				*/
	char	tot_con_qty				[ 7];		/* 누적체결수량(계약)	*/
	char	value					[11];		/* 누적거래대금(천원)	*/
	char	filler					[ 7];		/* filler				*/
	char	lastflag				[ 1];		/* 최종매도매수구분코드(1매도)	*/

	char    buy_tot_best_qty      	[6 ];         /* 매수총호가잔량               */          
	char    buy_1_price          	[6 ];         /* 매수1단계우선호가격 (4V2)    */      
	char    buy_1_price_qty      	[6 ];         /* 매수1단계우선호가잔량        */          
	char    buy_2_price          	[6 ];         /* 매수2단계우선호가격 (4V2)    */      
	char    buy_2_price_qty      	[6 ];         /* 매수2단계우선호가잔량        */          
	char    buy_3_price          	[6 ];         /* 매수3단계우선호가격 (4V2)    */      
	char    buy_3_price_qty      	[6 ];         /* 매수3단계우선호가잔량        */          
	char    buy_4_price          	[6 ];         /* 매수4단계우선호가격 (4V2)    */      
	char    buy_4_price_qty      	[6 ];         /* 매수4단계우선호가잔량        */          
	char    buy_5_price          	[6 ];         /* 매수5단계우선호가격 (4V2)    */      
	char    buy_5_price_qty      	[6 ];         /* 매수5단계우선호가잔량        */          

	char    sell_tot_best_qty      	[6 ];         /* 매도총호가잔량               */          
	char    sell_1_price          	[6 ];         /* 매도1단계우선호가격 (4V2)    */      
	char    sell_1_price_qty      	[6 ];         /* 매도1단계우선호가잔량        */          
	char    sell_2_price          	[6 ];         /* 매도2단계우선호가격 (4V2)    */      
	char    sell_2_price_qty      	[6 ];         /* 매도2단계우선호가잔량        */          
	char    sell_3_price          	[6 ];         /* 매도3단계우선호가격 (4V2)    */      
	char    sell_3_price_qty      	[6 ];         /* 매도3단계우선호가잔량        */          
	char    sell_4_price          	[6 ];         /* 매도4단계우선호가격 (4V2)    */      
	char    sell_4_price_qty      	[6 ];         /* 매도4단계우선호가잔량        */          
	char    sell_5_price          	[6 ];         /* 매도5단계우선호가격 (4V2)    */      
	char    sell_5_price_qty      	[6 ];         /* 매도5단계우선호가잔량        */          

	char    buy_tot_best_cnt      	[5 ];         /* 매수유효호가건수             */          
	char    buy_1_best_cnt        	[4 ];         /* 매수1단계우선호가건수        */        
	char    buy_2_best_cnt        	[4 ];         /* 매수2단계우선호가건수        */        
	char    buy_3_best_cnt        	[4 ];         /* 매수3단계우선호가건수        */        
	char    buy_4_best_cnt        	[4 ];         /* 매수4단계우선호가건수        */        
	char    buy_5_best_cnt        	[4 ];         /* 매수5단계우선호가건수        */        
	char    sell_tot_best_cnt      	[5 ];         /* 매도유효호가건수             */          
	char    sell_1_best_cnt        	[4 ];         /* 매도1단계우선호가건수        */        
	char    sell_2_best_cnt        	[4 ];         /* 매도2단계우선호가건수        */        
	char    sell_3_best_cnt        	[4 ];         /* 매도3단계우선호가건수        */        
	char    sell_4_best_cnt        	[4 ];         /* 매도4단계우선호가건수        */        
	char    sell_5_best_cnt        	[4 ];         /* 매도5단계우선호가건수        */        
	char	realtime_hprc			[ 6];		  /* 실시간상한가부호		*/
	char	realtime_lprc			[ 6];		  /* 실시간하한가부호		*/
}	G7174;

typedef struct {
	char	tr_gbn					[ 5];		/* TR코드 				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 4];		/* 종목일련번호			*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	sid						[ 2];		/* 세션ID				*/

	char    buy_tot_best_qty      	[6 ];         /* 매수총호가잔량               */          
	char    buy_1_price          	[6 ];         /* 매수1단계우선호가격 (4V2)    */      
	char    buy_1_price_qty      	[6 ];         /* 매수1단계우선호가잔량        */          
	char    buy_2_price          	[6 ];         /* 매수2단계우선호가격 (4V2)    */      
	char    buy_2_price_qty      	[6 ];         /* 매수2단계우선호가잔량        */          
	char    buy_3_price          	[6 ];         /* 매수3단계우선호가격 (4V2)    */      
	char    buy_3_price_qty      	[6 ];         /* 매수3단계우선호가잔량        */          
	char    buy_4_price          	[6 ];         /* 매수4단계우선호가격 (4V2)    */      
	char    buy_4_price_qty      	[6 ];         /* 매수4단계우선호가잔량        */          
	char    buy_5_price          	[6 ];         /* 매수5단계우선호가격 (4V2)    */      
	char    buy_5_price_qty      	[6 ];         /* 매수5단계우선호가잔량        */          

	char    sell_tot_best_qty      	[6 ];         /* 매도총호가잔량               */          
	char    sell_1_price          	[6 ];         /* 매도1단계우선호가격 (4V2)    */      
	char    sell_1_price_qty      	[6 ];         /* 매도1단계우선호가잔량        */          
	char    sell_2_price          	[6 ];         /* 매도2단계우선호가격 (4V2)    */      
	char    sell_2_price_qty      	[6 ];         /* 매도2단계우선호가잔량        */          
	char    sell_3_price          	[6 ];         /* 매도3단계우선호가격 (4V2)    */      
	char    sell_3_price_qty      	[6 ];         /* 매도3단계우선호가잔량        */          
	char    sell_4_price          	[6 ];         /* 매도4단계우선호가격 (4V2)    */      
	char    sell_4_price_qty      	[6 ];         /* 매도4단계우선호가잔량        */          
	char    sell_5_price          	[6 ];         /* 매도5단계우선호가격 (4V2)    */      
	char    sell_5_price_qty      	[6 ];         /* 매도5단계우선호가잔량        */          

	char    buy_tot_best_cnt      	[5 ];         /* 매수유효호가건수             */          
	char    buy_1_best_cnt        	[4 ];         /* 매수1단계우선호가건수        */        
	char    buy_2_best_cnt        	[4 ];         /* 매수2단계우선호가건수        */        
	char    buy_3_best_cnt        	[4 ];         /* 매수3단계우선호가건수        */        
	char    buy_4_best_cnt        	[4 ];         /* 매수4단계우선호가건수        */        
	char    buy_5_best_cnt        	[4 ];         /* 매수5단계우선호가건수        */        
	char    sell_tot_best_cnt      	[5 ];         /* 매도유효호가건수             */          
	char    sell_1_best_cnt        	[4 ];         /* 매도1단계우선호가건수        */        
	char    sell_2_best_cnt        	[4 ];         /* 매도2단계우선호가건수        */        
	char    sell_3_best_cnt        	[4 ];         /* 매도3단계우선호가건수        */        
	char    sell_4_best_cnt        	[4 ];         /* 매도4단계우선호가건수        */        
	char    sell_5_best_cnt        	[4 ];         /* 매도5단계우선호가건수        */        

	char	time					[ 8];		  /* 호가접수시각				  */
	char	yprice					[ 6];		  /* 예상체결가격				  */
}	B6174;

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 4];		/* 종목일련번호			*/
	char	prod_id					[11];		/* 상품ID				*/
	char	mkprod_gid				[ 3];		/* 장운영상품그룹ID		*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	board_event_id			[ 3];		/* 보드이벤트ID			*/
	char	board_event_st			[ 8];		/* 보드이벤트ID 시작시간*/
	char	board_event_cd			[ 5];		/* 보드이벤트ID 적용군코드*/
	char	session_startend_cd		[ 2];		/* 세션개시종료코드		*/
												/* BS:보드개시
													BE:보드종료
													SS:세션개시
													SE:세션종료
													SH:세션정지
													SR:세션재개			*/
	char	session_id				[ 2];		/* 세션ID				*/
	char	apply_depth				[ 2];		/* 적용단계				*/
	char	basic_itm_way			[ 1];		/* 기준종목가격확대발생코드 */
	char	price_ext_tm			[ 8];		/* 가격확대예정시각		*/
}	M4174;

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 4];		/* 종목일련번호			*/
	char	prod_id					[11];		/* 상품ID				*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	cexttime				[ 8];		/* 가격확대시간			*/
	char	uplmtpricestep			[ 2];		/* 가격제한확대상한단계 */
	char	dnlmtpricestep			[ 2];		/* 가격제한확대상한단계 */
	char	uplmtprice				[ 6];		/* 상한가				*/
	char	dnlmtprice				[ 6];		/* 하한가				*/
}	V1174;

/* Kosdaq 150 Options */
/* ****************** */

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 4];		/* 종목일련번호			*/
	char	prod_id					[11];		/* 상품ID				*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	cexttime				[ 8];		/* 가격확대시간			*/
	char	uplmtpricestep			[ 2];		/* 가격제한확대상한단계 */
	char	dnlmtpricestep			[ 2];		/* 가격제한확대상한단계 */
	char	uplmtprice				[ 5];		/* 상한가				*/
	char	dnlmtprice				[ 5];		/* 하한가				*/
}	V1034;

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 4];		/* 종목일련번호			*/
	char	prod_id					[11];		/* 상품ID				*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	cexttime				[ 8];		/* 가격확대시간			*/
	char	uplmtpricestep			[ 2];		/* 가격제한확대상한단계 */
	char	dnlmtpricestep			[ 2];		/* 가격제한확대상한단계 */
	char	uplmtprice_sign			[ 1];		/* 상한가				*/
	char	uplmtprice				[ 7];		/* 상한가				*/
	char	dnlmtprice_sign			[ 1];		/* 하한가				*/
	char	dnlmtprice				[ 7];		/* 하한가				*/
}	V1015;

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 2];		/* 종목일련번호			*/
	char	prod_id					[11];		/* 상품ID				*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	cexttime				[ 8];		/* 가격확대시간			*/
	char	uplmtpricestep			[ 2];		/* 가격제한확대상한단계 */
	char	dnlmtpricestep			[ 2];		/* 가격제한확대상한단계 */
	char	uplmtprice_sign			[ 1];		/* 상한가				*/
	char	uplmtprice				[ 5];		/* 상한가				*/
	char	dnlmtprice_sign			[ 1];		/* 하한가				*/
	char	dnlmtprice				[ 5];		/* 하한가				*/
}	V1014;

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 2];		/* 종목일련번호			*/
	char	prod_id					[11];		/* 상품ID				*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	cexttime				[ 8];		/* 가격확대시간			*/
	char	uplmtpricestep			[ 2];		/* 가격제한확대상한단계 */
	char	dnlmtpricestep			[ 2];		/* 가격제한확대상한단계 */
	char	uplmtprice_sign			[ 1];		/* 상한가				*/
	char	uplmtprice				[ 6];		/* 상한가				*/
	char	dnlmtprice_sign			[ 1];		/* 하한가				*/
	char	dnlmtprice				[ 6];		/* 하한가				*/
}	V1164;

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 3];		/* 종목일련번호			*/
	char	prod_id					[11];		/* 상품ID				*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	cexttime				[ 8];		/* 가격확대시간			*/
	char	uplmtpricestep			[ 2];		/* 가격제한확대상한단계 */
	char	dnlmtpricestep			[ 2];		/* 가격제한확대상한단계 */
	char	uplmtprice				[ 7];		/* 상한가				*/
	char	dnlmtprice				[ 7];		/* 하한가				*/
}	V1025;

typedef	G7024	G7164;
typedef	B6024	B6164;
typedef	M4024	M4164;

/* ****************** */
/* 기타등등 요구TR	  */
typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* ETF종목코드			*/
	char	time					[ 6];		/* 시간					*/
	char	befor_nav				[ 9];		/* 전일nav, 9(7)V(2)	*/
	char	nav						[ 9];		/* nav, 9(7)V(2)		*/
	char	filler					[ 8];		/* space				*/
}	BW011;	/* ETF예상 nav(주기단축) */
typedef	BW011	I3011;	/* ETF예산 nav */

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* ETF종목코드			*/
	char	time					[ 6];		/* 시간					*/
	char	befor_nav				[ 9];		/* 전일nav, 9(7)V(2)	*/
	char	nav						[ 9];		/* nav, 9(7)V(2)		*/
	char	filler					[28];		/* space				*/
}	BV011;	/* ETF nav(주기단축) */
typedef	BV011	F7011;	/* ETF nav */
typedef	BV011	L5011;	/* 해외지수ETF nav */

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* ETF종목코드			*/
	char	seq_no					[ 5];		/* 종목배치(A0)			*/
	char	tot_con_qty				[12];		/* 체결수량				*/
	char	tot_sell_jan			[12];		/* 총매도수량			*/
	char	tot_buy_jan				[12];		/* 총매수수량			*/
	char	filler					[ 1];		/* space				*/
}	B8011;	/* 장개시전 호가잔량 */

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* ETF종목코드			*/
	char	seq_no					[ 5];		/* 종목배치(A0)			*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	tsigntime				[ 9];		/* 매매체결처리시각		*/
	char	vicleartime				[ 9];		/* VI해제시각			*/
	char	vigubun					[ 1];		/* VI종류코드			*/
	char	svibprice				[ 9];		/* 정적VI발동기준가격	*/
	char	dvibprice				[ 9];		/* 동적VI발동기준가격	*/
	char	viprice					[ 9];		/* VI발동가격			*/
	char	sgrate					[13];		/* 정적 괴리율			*/
	char	dgrate					[13];		/* 동적 괴리율			*/
	char	filler					[ 2];		/* space				*/
}	R8011;		/* 코스피 종목상태정보(VI)	*/

/* 20211213 */
// Q2는 선물/옵션/미니선물/미니옵션/주식선물 5개만 있음
typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 2];		/* 종목배치(A0)			*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	time					[ 8];		/* 처리시간				*/
	char	real_set_gbn			[ 1];		/* 실시간가격제한설정코드*/
												/* 0:해제,1:설정,2:재설정*/
	char	realtime_hprc_sign		[ 1];		/* 실시간상한가부호		*/
	char	realtime_hprc			[ 5];		/* 실시간상한가			*/
	char	realtime_lprc_sign		[ 1];		/* 실시간하한가부호		*/
	char	realtime_lprc			[ 5];		/* 실시간하한가			*/
}	Q2014;
//typedef Q2014	Q2124;	// 미니선물

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 4];		/* 종목배치(A0)			*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	time					[ 8];		/* 처리시간				*/
	char	real_set_gbn			[ 1];		/* 실시간가격제한설정코드*/
												/* 0:해제,1:설정,2:재설정*/
	char	realtime_hprc			[ 5];		/* 실시간상한가			*/
	char	realtime_lprc			[ 5];		/* 실시간하한가			*/
}	Q2034;
//typedef Q2034	Q2134;	// 미니옵션

typedef struct
{
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	item_code				[12];		/* 종목코드				*/
	char	seq_no					[ 4];		/* 종목배치(A0)			*/
	char	board_id				[ 2];		/* 보드ID				*/
	char	time					[ 8];		/* 처리시간				*/
	char	real_set_gbn			[ 1];		/* 실시간가격제한설정코드*/
												/* 0:해제,1:설정,2:재설정*/
	char	realtime_hprc_sign		[ 1];		/* 실시간상한가부호		*/
	char	realtime_hprc			[ 7];		/* 실시간상한가			*/
	char	realtime_lprc_sign		[ 1];		/* 실시간하한가부호		*/
	char	realtime_lprc			[ 7];		/* 실시간하한가			*/
}	Q2015;		// 주식선물
/* 20211213 */
/* 기타등등 요구TR	  */
/* ****************** */

/* **************** */
/* 지수				*/
/* 
 * 18018 : AA011 	KRX300지수
           AB011	KRX300예상지수
		   D2011	KOSPI200지수
		   D3011	KOSPI200예상지수
*/
typedef struct {
	char	tr_gbn					[ 5];		/* TR Code				*/
    char	up_code					[ 3];		/* 업종코드				*/
	char	time					[ 6];		/* 시간					*/
	char	jisu					[ 8];		/* 지수 9(6)V9(2)		*/
	char	sign					[ 1];		/* 부호					*/
	char	change					[ 8];		/* 대비					*/
	char	volume					[ 8];		/* 체결수량(단위:천주)	*/
	char	value					[ 8];		/* 거래대금(단위:백만원)*/
	char	filler					[ 2];		/* filler				*/
}	AA011;
typedef	AA011	AB011;	
typedef	AA011	D0011;	
typedef	AA011	D2011;	
typedef	AA011	D3011;	

/* **************** */
/* 해외시세			*/
/* **************** */
typedef struct {
	char		type         [  2];	    /*  데이터타입      	'M':마스터		*/
	char		vcod         [  1];	    /* 	시세벤더코드    	'0': Main
																'1': Backup1
																'2': Backup2	*/
	char		vnam         [  8];	    /* 	시세벤더이름    	*/
	char		seqn         [  9];	    /* 	일련번호    		*/
	char		exch         [  8];	    /* 	거래소코드    		*/
	char		symb         [ 20];	    /* 	종목코드    		FCME			*/
	char		root         [ 12];	    /* 	품목코드    		*/
	char		name         [128];	    /* 	종목명    			*/
	char		styp         [  1];	    /* 	상품타입    		*/
	char		lymd         [  8];	    /* 	등록일자    		*/
	char		zymd         [  8];	    /* 	만기일자    		*/
	char		exym         [  6];	    /* 	만기년월    		*/
	char		zdiv         [  4];	    /* 	소수점자리수    	*/
	char		jjis         [  9];	    /* 	잔존일수    		*/
	char		unps         [ 20];	    /* 	기초자산코드    	*/
	char		corp         [  1];	    /* 	콜풋구분    		*/
	char		strk         [ 12];	    /* 	행사가    			*/
	char		unp_root     [ 12];	    /* 	기초자산품목코드	*/
	char		unp_mcod     [  1];	    /* 	기초자산월물코드	*/
	char		exch_symb    [ 20];	    /* 	거래소종목코드		*/
	char		leg1_symb    [ 20];	    /* 	원월물종목코드		*/
	char		leg1_exym    [  6];	    /* 	원월물만기년월		*/
	char		leg2_symb    [ 20];	    /* 	근월물종목코드		*/
	char		leg2_exym    [  6];	    /* 	근월물만기년월		*/
	char		dttm         [ 21];	    /* 	송신일시    		*/
}	OC_MASTER;	/* 363	*/

typedef struct {
	char		type    [ 2];   /*   데이터타입   'Q':현재가            */
	char		vcod    [ 1];   /*   시세벤더코드 "'0': Main            
                                            '1': Backup1          
                                            '2': Backup2"         		*/
	char		vnam    [ 8];   /*   시세벤더이름 GEMIZIP               */
	char		seqn    [ 9];   /*   일련번호                           */
	char		exch    [ 8];   /*   거래소코드   FCME                  */
	char		symb    [20];   /*   종목코드     6AH18                 */
	char		tymd    [ 8];   /*   거래일자     "개미집 CME만 해당 
                                            기타거래소는 정보없음"		*/
	char		kymd    [ 8];   /*   한국체결일자 20180618              */
	char		khms    [ 6];   /*   한국체결시간 102034                */
	char		open    [20];   /*   시가                      2818.25  */
	char		high    [20];   /*   고가                      2818.25  */
	char		low     [20];   /*   저가                      2818.25  */
	char		last    [20];   /*   현재가                    2818.25  */
	char		diff    [20];   /*   전일대비                    42.35  */
	char		rate    [ 6];   /*   등락률         1.52                */
	char		evol    [ 9];   /*   체결수량     000000001             */
	char		tvol    [12];   /*   누적체결수량 000000045033          */
	char		dttm    [21];   /*   송신일시     20180618102034-123456 */
}	OC_Q;	/* 218	*/

typedef struct {
	char		type	[ 2];	/*   데이터타입    'B':호가             */
	char		vcod	[ 1];	/*   시세벤더코드  "'0': Main            
								  	'1': Backup1          
								  	'2': Backup2"         				*/
	char		vnam	[ 8];	/*   시세벤더이름  GEMIZIP              */
	char		seqn	[ 9];	/*   일련번호                           */
	char		exch	[ 8];	/*   거래소코드    FCME                 */
	char		symb	[20];	/*  종목코드      6AH18                 */
	char		bymd	[ 8];	/*   영업일자      "개미집 CME만 해당    
								  기타거래소는 정보없음"*/
	char		kymd	[ 8];	/*   한국거래일자  20180618             */
	char		khms	[ 6];	/*   한국거래시간  102034               */
	char		dttm	[21];	/*  송신일시      20180618102034-123456 */
	char		dept	[ 2];	/*   호가레벨      05                   */
	char		pask	[20];	/*  매도호가      '             2818.25'*/
	char		vask	[ 8];	/*   매도호가수량  00000096             */
	char		nask	[ 6];	/*   매도호가건수  0000043              */
	char		pbid	[20];	/*  매수호가                   2818.25  */
	char		vbid	[ 8];	/*   매수호가수량  00000123             */
	char		nbid	[ 6];	/*   매수호가건수  000048               */
}	OC_B;	/* 433	*/

typedef struct {
	char		type  [ 2];   /*     데이터타입    'S':정산가           */ 
	char		vcod  [ 1];   /*     시세벤더코드  "'0': Main            
									'1': Backup1          
									'2': Backup2"        				*/ 
	char		vnam  [ 8];   /*     시세벤더이름                       */ 
	char		seqn  [ 9];   /*     일련번호                           */
	char		exch  [ 8];   /*     거래소코드    FCME                 */ 
	char		symb  [20];   /*    종목코드      6AH18                 */ 
	char		symd  [ 8];   /*     정산일자      "개미집 CME만 해당    
									기타거래소는 정보없음"				*/
	char		setp  [20];   /*    정산가                              */
	char		dttm  [21];   /*    송신일시      20180618102034-123456 */
}	OC_S;	/* 97	*/

/*************************************************************************
    End of Program (krx_mk.h)
*************************************************************************/
#endif
