#include "WLM001.h"
#include "WLM003.h"

int CSTINDX_ST_Print( CSTINDX_ST* ptr)
{
    printf( "%s", "----[ CSTINDX_ST ]----------------------------------------------------------------------\n");
    printf( "                              n_cnt                  4    0 = [%d]\n", 	ptr->n_cnt);
    printf( "                              n_indx                 4    4 = [%d]\n", 	ptr->n_indx);
    printf( "%s", "----------------------------------------------------------------------[ CSTINDX_ST ]----\n");

    return sizeof( CSTINDX_ST);
}
/*
int TNRHDOM_ST_Print( TNRHDOM_ST* ptr)
{
    printf( "%s", "----[ TNRHDOM_ST ]----------------------------------------------------------------------\n");
    printf( "테너코드 TOD, TOM, SPT, W01,  s_tnr_id               4    0 = [%.4s]\n", 	ptr->s_tnr_id);
    printf( "FX상품코드 SPT, FWD           s_fx_pdcd              4    4 = [%.4s]\n", 	ptr->s_fx_pdcd);
//    printf( "테너구분 S: 표준              s_tnr_ptrn_dcd         2    8 = [%.2s]\n", 	ptr->s_tnr_ptrn_dcd);
//    printf( "결제일                        s_sldy                 9   10 = [%.9s]\n", 	ptr->s_sldy);
//    printf( "일수                          d_ndd                  8   19 = [%f]\n", 	ptr->d_ndd);
//    printf( "일할                          d_dldv                 8   27 = [%f]\n", 	ptr->d_dldv);
    printf( "ASK 현물환 Cover딜러 마진     d_ask_spt_cvmg         8   35 = [%f]\n", 	ptr->d_ask_spt_cvmg);
    printf( "ASK 현물환 Corp딜러 마진      d_ask_spt_cpmg         8   43 = [%f]\n", 	ptr->d_ask_spt_cpmg);
    printf( "ASK 선물환 Cover딜러 마진     d_ask_fwd_cvmg         8   51 = [%f]\n", 	ptr->d_ask_fwd_cvmg);
    printf( "ASK 선물환 Corp딜러 마진      d_ask_fwd_cpmg         8   59 = [%f]\n", 	ptr->d_ask_fwd_cpmg);
    printf( "ASK 본점마진 합계             d_ask_hdom             8   67 = [%f]\n", 	ptr->d_ask_hdom);
    printf( "                              d_ask_swap_pnt         8   75 = [%f]\n", 	ptr->d_ask_swap_pnt);
    printf( "BID 현물환 Cover딜러 마진     d_bid_spt_cvmg         8   83 = [%f]\n", 	ptr->d_bid_spt_cvmg);
    printf( "BID 현물환 Corp딜러 마진      d_bid_spt_cpmg         8   91 = [%f]\n", 	ptr->d_bid_spt_cpmg);
    printf( "BID 선물환 Cover딜러 마진     d_bid_fwd_cvmg         8   99 = [%f]\n", 	ptr->d_bid_fwd_cvmg);
    printf( "BID 선물환 Corp딜러 마진      d_bid_fwd_cpmg         8  107 = [%f]\n", 	ptr->d_bid_fwd_cpmg);
    printf( "BID 본점마진 합계             d_bid_hdom             8  115 = [%f]\n", 	ptr->d_bid_hdom);
    printf( "                              d_bid_swap_pnt         8  123 = [%f]\n", 	ptr->d_bid_swap_pnt);
    printf( "%s", "----------------------------------------------------------------------[ TNRHDOM_ST ]----\n");

    return sizeof( TNRHDOM_ST);
}
*/
int PAIRMRGN_ST_Print( PAIRMRGN_ST* ptr)
{
    printf( "    %s", "----[ PAIRMRGN_ST ]---------------------------------------------------------------------\n");
    printf( "                                  s_pair_id              8    0 = [%.8s]\n", 	ptr->s_pair_id);
//    printf( "    통화계산구분 1:XXX/USD, 2: D  s_clc_dsnc             2    8 = [%.2s]\n", 	ptr->s_clc_dsnc);
//    printf( "    DIGIT                         n_digit                4   10 = [%d]\n", 	ptr->n_digit);
//    printf( "    DIGIT 값 0.01, 0.001, 0.0001  d_digit_val            8   14 = [%f]\n", 	ptr->d_digit_val);
//    printf( "    계산단위 JPY/KRW 100, 나머지  d_clc_unit             8   22 = [%f]\n", 	ptr->d_clc_unit);
    printf( "    FWD 마진유형구분코드 1:금액,  s_spt_bomg_dcd         2   30 = [%.2s]\n", 	ptr->s_spt_bomg_dcd);
    printf( "                                  d_spt_bymg             8   32 = [%f]\n", 	ptr->d_spt_bymg);
    printf( "                                  d_spt_slmg             8   40 = [%f]\n", 	ptr->d_spt_slmg);
    printf( "    FWD 마진유형구분코드 1:금액,  s_fwd_bomg_dcd         2   48 = [%.2s]\n", 	ptr->s_fwd_bomg_dcd);
    printf( "                                  d_fwd_bymg             8   50 = [%f]\n", 	ptr->d_fwd_bymg);
    printf( "                                  d_fwd_slmg             8   58 = [%f]\n", 	ptr->d_fwd_slmg);
    printf( "    %s", "---------------------------------------------------------------------[ PAIRMRGN_ST ]----\n");

    return sizeof( PAIRMRGN_ST);
}

int FNLPAIR_ST_Print( PAIRMRGN_ST* ptr)
{
    printf( "    %s", "----[ FNLPAIR_ST ]----------------------------------------------------------------------\n");
    printf( "                                  s_pair_id              8    0 = [%.8s]\n", 	ptr->s_pair_id);
//    printf( "    통화계산구분 1:XXX/USD, 2: D  s_clc_dsnc             2    8 = [%.2s]\n", 	ptr->s_clc_dsnc);
//    printf( "    DIGIT                         n_digit                4   10 = [%d]\n", 	ptr->n_digit);
//    printf( "    DIGIT 값 0.01, 0.001, 0.0001  d_digit_val            8   14 = [%f]\n", 	ptr->d_digit_val);
//    printf( "    계산단위 JPY/KRW 100, 나머지  d_clc_unit             8   22 = [%f]\n", 	ptr->d_clc_unit);
    printf( "    FWD 마진유형구분코드 1:금액,  s_spt_bomg_dcd         2   30 = [%.2s]\n", 	ptr->s_spt_bomg_dcd);
    printf( "                                  d_spt_bymg             8   32 = [%f]\n", 	ptr->d_spt_bymg);
    printf( "                                  d_spt_slmg             8   40 = [%f]\n", 	ptr->d_spt_slmg);
    printf( "    FWD 마진유형구분코드 1:금액,  s_fwd_bomg_dcd         2   48 = [%.2s]\n", 	ptr->s_fwd_bomg_dcd);
    printf( "                                  d_fwd_bymg             8   50 = [%f]\n", 	ptr->d_fwd_bymg);
    printf( "                                  d_fwd_slmg             8   58 = [%f]\n", 	ptr->d_fwd_slmg);
    printf( "    %s", "----------------------------------------------------------------------[ FNLPAIR_ST ]----\n");

    return sizeof( PAIRMRGN_ST);
}

int CUSTMRGN_ST_Print( CUSTMRGN_ST* ptr)
{
	int			i;

    printf( "%s", "----[ CUSTMRGN_ST ]---------------------------------------------------------------------\n");
    printf( "전행고객실명대체번호          s_csac_idnt_no        31    0 = [%.31s]\n", 	ptr->s_csac_idnt_no);
    printf( "본점마진구룹ID                s_cust_grp_id          7   31 = [%.7s]\n", 	ptr->s_cust_grp_id);
    printf( "직원그룹여부 Y,N              s_emp_grp_yn           2   38 = [%.2s]\n", 	ptr->s_emp_grp_yn);
    printf( "재정Pair 건수                 n_fnl_cnt              4   40 = [%d]\n", 	ptr->n_fnl_cnt);

	for( i = 0; i < ptr->n_fnl_cnt; i++)
    	FNLPAIR_ST_Print( &ptr->fnlmgst[ i]);
    PAIRMRGN_ST_Print( &ptr->usdmgst);
    printf( "                              n_std_cnt              4   44 = [%d]\n", 	ptr->n_std_cnt);
	for( i = 0; i < ptr->n_std_cnt; i++)
    	PAIRMRGN_ST_Print( &ptr->stdmgst[ i]);
    printf( "%s", "---------------------------------------------------------------------[ CUSTMRGN_ST ]----\n");

    return sizeof( CUSTMRGN_ST);
}

