typedef struct _aaaa_
{
    int       n_cnt;
    int       n_indx[500];
} CSTINDX_ST;

typedef struct _bbbbb_
{
    char      s_tnr_id                     [  3+1];  /* 테너코드 TOD, TOM, SPT, W01, M01, M02, M03, M06, */       
    char      s_fx_pdcd                    [  3+1];  /* FX상품코드 SPT, FWD                              */       
    char      s_tnr_ptrn_dcd               [  1+1];  /* 테너구분 S: 표준                                 */       
    char      s_sldy                       [  8+1];  /* 결제일                                           */       
                                                     /*                                                  */       
    double    d_ndd                               ;  /* 일수                                             */       
    double    d_dldv                              ;  /* 일할                                             */       
    double    d_ask_spt_cvmg                      ;  /* ASK 현물환 Cover딜러 마진                        */       
    double    d_ask_spt_cpmg                      ;  /* ASK 현물환 Corp딜러 마진                         */       
    double    d_ask_fwd_cvmg                      ;  /* ASK 선물환 Cover딜러 마진                        */       
    double    d_ask_fwd_cpmg                      ;  /* ASK 선물환 Corp딜러 마진                         */       
    double    d_ask_hdom                          ;  /* ASK 본점마진 합계                                */       
    double    d_ask_swap_pnt                      ;  /*                                                  */       
                                                     /*                                                  */       
    double    d_bid_spt_cvmg                      ;  /* BID 현물환 Cover딜러 마진                        */       
    double    d_bid_spt_cpmg                      ;  /* BID 현물환 Corp딜러 마진                         */       
    double    d_bid_fwd_cvmg                      ;  /* BID 선물환 Cover딜러 마진                        */       
    double    d_bid_fwd_cpmg                      ;  /* BID 선물환 Corp딜러 마진                         */       
    double    d_bid_hdom                          ;  /* BID 본점마진 합계                                */       
    double    d_bid_swap_pnt                      ;  /*                                                  */       

} TNRHDOM_ST;

typedef struct _aa_
{
    char        s_pair_id            [7+1];  /*                                       */
    char        s_clc_dsnc                 [  1+1];  /* 통화계산구분 1:XXX/USD, 2: DIV 3: MUL */
    int         n_digit                           ;  /* DIGIT                                 */
    double      d_digit_val                       ;  /* DIGIT 값 0.01, 0.001, 0.0001          */
    double      d_clc_unit                        ;  /* 계산단위 JPY/KRW 100, 나머지 1        */
    char        s_spt_bomg_dcd             [  1+1];  /* FWD 마진유형구분코드 1:금액, 2:율     */
    double      d_spt_bymg                        ;  /*                                       */
    double      d_spt_slmg                        ;  /*                                       */
    char        s_fwd_bomg_dcd             [  1+1];  /* FWD 마진유형구분코드 1:금액, 2:율     */
    double      d_fwd_bymg                        ;  /*                                       */
    double      d_fwd_slmg                        ;  /*                                       */
} PAIRMRGN_ST;

typedef struct _bb_
{
    char        s_pair_id          [  7+1];
    char        s_clc_dsnc                 [  1+1];  /* 통화계산구분 1:XXX/USD, 2: DIV 3: MUL */
    int         n_digit                           ;  /* DIGIT                                 */
    double      d_digit_val                       ;  /* DIGIT 값 0.01, 0.001, 0.0001          */
    double      d_clc_unit                        ;  /* 계산단위 JPY/KRW 100, 나머지 1        */
    char        s_spt_bomg_dcd             [  1+1];  /* FWD 마진유형구분코드 1:금액, 2:율     */
    double      d_spt_bymg                        ;  /*                                       */
    double      d_spt_slmg                        ;  /*                                       */
    char        s_fwd_bomg_dcd             [  1+1];  /* FWD 마진유형구분코드 1:금액, 2:율     */
    double      d_fwd_bymg                        ;  /*                                       */
    double      d_fwd_slmg                        ;  /*                                       */

} FNLPAIR_ST;      



typedef struct _cc_                                               
{                                                                 
    char         s_csac_idnt_no      [30+1];  /* 전행고객실명대체번호 */
    char         s_cust_grp_id       [6+1];  /* 본점마진구룹ID       */
    char         s_emp_grp_yn              [  1+1];  /* 직원그룹여부 Y,N     */
    int          n_fnl_cnt                        ;  /* 재정Pair 건수        */
    FNLPAIR_ST   fnlmgst            [MAX_PAIR_CNT];  /* 재정Pair 정보        */
    PAIRMRGN_ST  usdmgst                          ;  /*                      */
    int          n_std_cnt                        ;  /*                      */
    PAIRMRGN_ST  stdmgst            [MAX_PAIR_CNT];  /*                      */
} CUSTMRGN_ST;                                                    

