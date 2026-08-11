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
 *  Components  : mrgnsise.c
 *  Rev. History: 
 *      Ver     Date    Information
 *      ------- ------- -----------------------------------------------
 *      1.00    2023-10 Winway initial version.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mrgnsise.h"

// 일수 계산
int     l_getdaysbtwn(char *dt_from, char *dt_to, char *s_errcd);
double  l_calc_dldv(char *s_from, char *s_to, char *s_val, char *s_errcd);
int     l_calc_unstd_fwd_quot(HDQOUT_ST *fwd_qt1, HDQOUT_ST *fwd_qt2, HDQOUT_ST *fwd_qt, char *s_errcd);
int     l_calc_usd_qout(APSISE_ST *sise, HDQOUT_ST *qout);
int     l_calc_fnnl_qout(HDQOUT_ST *usd_qt, HDQOUT_ST *std_qt, HDQOUT_ST *fnnl_qt, char *s_errcd);
int     l_calc_fnnl_unstd(HDQOUT_ST *fnnl_qt1, HDQOUT_ST *fnnl_qt2, HDQOUT_ST *fnnl_qt, char *s_errcd);
int     l_calc_fnnl_expi_sclt(char *s_bysel_dcd, HDQOUT_ST *usd_qt1, HDQOUT_ST *usd_qt2, HDQOUT_ST *std_qt1, HDQOUT_ST *std_qt2, HDQOUT_ST *fnnl_qt, SPLIT_OUT_ST *splt_qt, char *s_errcd);
int     l_calc_usd_expi_sclt(char *s_bysel_dcd, HDQOUT_ST *fwd_qt1, HDQOUT_ST *fwd_qt2, SPLIT_OUT_ST *splt_qt, char *s_errcd);
int     l_calc_sise_fxpair(APSISE_ST *p_sise, FXPAIR_ST *p_fxpair, char *s_errcd);





double l_calc_dldv(s_from, s_to, s_val, s_errcd)
char    *s_from;
char    *s_to;
char    *s_val;
char    *s_errcd;
{
	double  d_tot;
	double  d_days; 
	
	d_tot   = (double) l_getdaysbtwn(s_from, s_to, s_errcd);
	d_days  = (double) l_getdaysbtwn(s_from, s_val, s_errcd);
	
	return (d_days / d_tot);
}

// 시작일과 종료일의 일수 리턴
int l_getdaysbtwn( s_dt_from, s_dt_to, s_errcd )
char *s_dt_from;
char *s_dt_to;
char *s_errcd;
{
	char temp[10];
	
	snprintf(temp, 4+1, "%s", s_dt_from);
	int year_from = atoi(temp);
	snprintf(temp, 2+1, "%s", s_dt_from + 4);
	int month_from = atoi(temp);
	snprintf(temp, 2+1, "%s", s_dt_from + 6);
	int day_from = atoi(temp);
	
	snprintf(temp, 4+1, "%s", s_dt_to);
	int year_to = atoi(temp);
	snprintf(temp, 2+1, "%s", s_dt_to + 4);
	int month_to = atoi(temp);
	snprintf(temp, 2+1, "%s", s_dt_to + 6);
	int day_to = atoi(temp);
	
	struct tm time_from, time_to;

	time_from.tm_year  = year_from - 1900;
	time_from.tm_mon   = month_from - 1;
	time_from.tm_mday  = day_from;
	time_from.tm_hour  = 0;
	time_from.tm_min   = 0;
	time_from.tm_sec   = 1;
	time_from.tm_isdst = -1;

	time_to.tm_year  = year_to - 1900;
	time_to.tm_mon   = month_to -1;
	time_to.tm_mday  = day_to;
	time_to.tm_hour  = 0;
	time_to.tm_min   = 0;
	time_to.tm_sec   = 1;
	time_to.tm_isdst = -1;
	
	time_t from = mktime( &time_from );
	// 일자변환 오류
	if ( from == -1 )
		return -1;
		
	time_t to   = mktime( &time_to   );
	// 일자변환 오류
	if ( to == -1 )
		return -1;
	
	return (int) ( (difftime( to, from)) / 86400L );
}



// round 처리 시 input 변수 값을 변경
double l_round_x(d_val, n_digit)
double *d_val;
int     n_digit;
{
	char        s_temp[32];
	char        s_pram[20];
    double  d_ret = *d_val < 0.0 ? *d_val - 0.0000000001 : *d_val + 0.0000000001;

	memset(s_temp, 0x00, sizeof(s_temp));
	memset(s_pram, 0x00, sizeof(s_pram));
	sprintf(s_pram, "%c.%dlf", '%', n_digit);
	
    snprintf(s_temp, sizeof(s_temp), s_pram, d_ret);
    *d_val = atof(s_temp);

    return *d_val;
}

// round 처리 시 input 변수 값을 변경 안함
double l_round_d(d_val, n_digit)
double d_val;
int    n_digit;
{
	char        s_temp[32];
	char        s_pram[20];
    double  d_ret = d_val < 0.0 ? d_val - 0.0000000001 : d_val + 0.0000000001;

	memset(s_temp, 0x00, sizeof(s_temp));
	memset(s_pram, 0x00, sizeof(s_pram));
	sprintf(s_pram, "%c.%dlf", '%', n_digit);
	
    snprintf(s_temp, sizeof(s_temp), s_pram, d_ret);
    d_ret = atof(s_temp);

    return d_ret;
}

//
int l_calc_sise_shm(p_sise, p_hdomgrp, s_errcd)
APSISE_ST      *p_sise;
HDOMGRP_SHM_ST *p_hdomgrp;
char           *s_errcd;
{   
	int n_gcnt  = 0;
	int n_pcnt  = 0;
	int n_tcnt  = 0;
	int rtn     = 0;
    	
   
	char s_symb    [6+1];
	char s_pair_id [6+1];
	char s_fnnl_id [6+1];
    
	FXPAIR_ST   *p_usd_fxpair = NULL;
	FXPAIR_ST   *p_std_fxpair = NULL; 
	FXPAIR_ST   *p_fnl_fxpair = NULL;

	memset(s_symb, 0x00, sizeof(s_symb));
	memcpy(s_symb, p_sise->symb, sizeof(p_sise->symb)-1);
		
	if (strncmp(s_symb, "USD", 3) == 0)
		sprintf(s_fnnl_id, "%.3s%.3s", s_symb+3, "KRW");
	else
		sprintf(s_fnnl_id, "%.3s%.3s", s_symb, "KRW");

    
	while(n_gcnt < p_hdomgrp->n_grp_cnt)
	{
		while(n_pcnt < p_hdomgrp->grp[n_gcnt].n_pair_cnt)
		{
			memset(s_pair_id, 0x00, sizeof(s_pair_id));
			memcpy(s_pair_id, p_hdomgrp->grp[n_gcnt].pair[n_pcnt].s_pair_id, sizeof(s_pair_id)-1);

			if (strncmp(s_symb, s_pair_id, sizeof(s_pair_id)-1) == 0 ||
				strncmp("USDKRW", s_pair_id, sizeof(s_pair_id)-1) == 0)
			{
				if (strncmp(s_pair_id, "USDKRW", 6) == 0)
					p_usd_fxpair  = (FXPAIR_ST *)&p_hdomgrp->grp[n_gcnt].pair[0];
				else if (strncmp(s_symb, s_pair_id, sizeof(s_symb)-1) == 0)
					p_std_fxpair  = (FXPAIR_ST *)&p_hdomgrp->grp[n_gcnt].pair[0];
				else if (strncmp(s_fnnl_id, s_pair_id, sizeof(s_fnnl_id)-1) == 0)
					p_fnl_fxpair  = (FXPAIR_ST *)&p_hdomgrp->grp[n_gcnt].pair[0];

				if (p_usd_fxpair != NULL && p_std_fxpair != NULL && p_fnl_fxpair != NULL)
				{   

					rtn = l_calc_sise_fxpair(p_sise, p_usd_fxpair, s_errcd);
					rtn = l_calc_sise_fxpair(p_sise, p_std_fxpair, s_errcd);
					rtn = l_calc_sise_fxpair(p_sise, p_fnl_fxpair, s_errcd);

					p_usd_fxpair  = NULL;
					p_std_fxpair  = NULL;
					p_fnl_fxpair  = NULL;

					break;

				}
			}
			n_pcnt++;
			if (n_pcnt >= p_hdomgrp->grp[n_gcnt].n_pair_cnt)
				printf("통화Pair 검색 오류[%s][%s][%s]\n", p_usd_fxpair == NULL ? "" : p_usd_fxpair->s_pair_id, p_std_fxpair == NULL ? "" : p_std_fxpair->s_pair_id, p_fnl_fxpair == NULL ? "" : p_fnl_fxpair->s_pair_id);


		}
		n_gcnt++;
	}
	return 0;
}



// USD/KRW, 비재정통화 FXPAIR 시세 계산
// 시세 수신시 계산
int l_calc_sise_fxpair(p_sise, p_fxpair, s_errcd)
APSISE_ST *p_sise;
FXPAIR_ST *p_fxpair;
char      *s_errcd;
{
	int      N_DGT ;
	int      n_cnt = 0;

	double   D_DGT_VAL;

	N_DGT     = p_fxpair->n_digit;
	D_DGT_VAL = p_fxpair->d_digit_val;

	p_fxpair->s_ask_orgn[0]     = p_sise->bidex[0];
	p_fxpair->s_bid_orgn[0]     = p_sise->offerex[0];

	p_fxpair->d_ask_spt_prc     = p_sise->offerprc;
	p_fxpair->d_bid_spt_prc     = p_sise->bidprc;

	p_fxpair->d_ask_usd_spt_prc = p_sise->usdoffer;
	p_fxpair->d_bid_usd_spt_prc = p_sise->usdbid;

	memcpy(p_fxpair->s_rcv_ymd, p_sise->date, sizeof(p_sise->date));
	memcpy(p_fxpair->s_rcv_hms, p_sise->time, sizeof(p_sise->time));

	while(n_cnt < p_fxpair->n_tnr_cnt)
	{

		// ASK(매입) 계산

		// 현물환
		if (strncmp(p_fxpair->tnr[n_cnt].s_tnr_tcd, "TOD", 3) == 0 ||
			strncmp(p_fxpair->tnr[n_cnt].s_tnr_tcd, "TOM", 3) == 0 ||
			strncmp(p_fxpair->tnr[n_cnt].s_tnr_tcd, "SPT", 3) == 0 )
		{
			// SPOT가격 + 현물환 Cover마진
			p_fxpair->tnr[n_cnt].d_ask_cvmg_prc = p_fxpair->d_ask_spt_prc + p_fxpair->tnr[n_cnt].d_ask_spt_cvmg;
		}
		// 선물환
		else
		{
			// SPOT가격 + 현물환 Cover마진 + 선물환 Cover마진
			p_fxpair->tnr[n_cnt].d_ask_cvmg_prc = p_fxpair->d_ask_spt_prc + p_fxpair->tnr[n_cnt].d_ask_spt_cvmg + p_fxpair->tnr[n_cnt].d_ask_fwd_cvmg;
		}

		// 본점마진가격
		p_fxpair->tnr[n_cnt].d_ask_hdom_prc = p_fxpair->d_ask_spt_prc + p_fxpair->tnr[n_cnt].d_ask_hdom;
		// 시장가격
		// SPOT가격 + SWAP포인트 * D_DGT_VAL[0.01, 0.001, 0.0001]
		p_fxpair->tnr[n_cnt].d_ask_mrkt_prc = l_round_d(p_fxpair->d_ask_spt_prc + p_fxpair->tnr[n_cnt].d_ask_swap_pnt * D_DGT_VAL, N_DGT);

		// 본점가격
		// 시장가격 + 본점마진
		p_fxpair->tnr[n_cnt].d_ask_hdof_prc = p_fxpair->tnr[n_cnt].d_ask_mrkt_prc + p_fxpair->tnr[n_cnt].d_ask_hdom;

		l_round_x(&p_fxpair->tnr[n_cnt].d_ask_hdof_prc, N_DGT);

		// 전일대비증감
		p_fxpair->tnr[n_cnt].d_ask_ctpd_indc = p_fxpair->tnr[n_cnt].d_ask_pdcp - p_fxpair->tnr[n_cnt].d_ask_hdof_prc;

		// 시작가
		if (p_fxpair->tnr[n_cnt].d_ask_sttg_prc == 0.0)
			p_fxpair->tnr[n_cnt].d_ask_sttg_prc = p_fxpair->tnr[n_cnt].d_ask_hdof_prc;
		// 고가
		if (p_fxpair->tnr[n_cnt].d_ask_hgpr_prc < p_fxpair->tnr[n_cnt].d_ask_hdof_prc)
			p_fxpair->tnr[n_cnt].d_ask_hgpr_prc = p_fxpair->tnr[n_cnt].d_ask_hdof_prc;
		// 저가
		if (p_fxpair->tnr[n_cnt].d_ask_lw_prc > p_fxpair->tnr[n_cnt].d_ask_hdof_prc)
			p_fxpair->tnr[n_cnt].d_ask_lw_prc = p_fxpair->tnr[n_cnt].d_ask_hdof_prc;



		// BID(매도) 계산

		// 현물환
		if (strncmp(p_fxpair->tnr[n_cnt].s_tnr_tcd, "TOD", 3) == 0 ||
			strncmp(p_fxpair->tnr[n_cnt].s_tnr_tcd, "TOM", 3) == 0 ||
			strncmp(p_fxpair->tnr[n_cnt].s_tnr_tcd, "SPT", 3) == 0 )
		{
			// Cover마진가격 =  SPOT가격 + 현물환 Cover마진
			p_fxpair->tnr[n_cnt].d_bid_cvmg_prc = p_fxpair->d_bid_spt_prc - p_fxpair->tnr[n_cnt].d_bid_spt_cvmg;
		}
		// 선물환
		else
		{
			// Cover마진가격 =  SPOT가격 + 현물환 Cover마진 + 선물환 Cover마진
			p_fxpair->tnr[n_cnt].d_bid_cvmg_prc = p_fxpair->d_bid_spt_prc - p_fxpair->tnr[n_cnt].d_bid_spt_cvmg - p_fxpair->tnr[n_cnt].d_bid_fwd_cvmg;
		}
		l_round_x(&p_fxpair->tnr[n_cnt].d_bid_cvmg_prc, N_DGT);

		// 본점마진가격
		p_fxpair->tnr[n_cnt].d_bid_hdom_prc = p_fxpair->d_bid_spt_prc - p_fxpair->tnr[n_cnt].d_bid_hdom;

		// 시장가격
		// SPOT가격 + SWAP포인트 * D_DGT_VAL[0.01, 0.001, 0.0001]
		p_fxpair->tnr[n_cnt].d_bid_mrkt_prc = l_round_d(p_fxpair->d_bid_spt_prc + p_fxpair->tnr[n_cnt].d_bid_swap_pnt * D_DGT_VAL, N_DGT);

		// 본점가격
		// 시장가격 - 본점마진
		p_fxpair->tnr[n_cnt].d_bid_hdof_prc = p_fxpair->tnr[n_cnt].d_bid_mrkt_prc - p_fxpair->tnr[n_cnt].d_bid_hdom;
		l_round_x(&p_fxpair->tnr[n_cnt].d_bid_hdof_prc, N_DGT);

		// 전일대비증감
		p_fxpair->tnr[n_cnt].d_bid_ctpd_indc = p_fxpair->tnr[n_cnt].d_bid_pdcp - p_fxpair->tnr[n_cnt].d_bid_hdof_prc;

		// 시작가
		if (p_fxpair->tnr[n_cnt].d_bid_sttg_prc == 0.0)
			p_fxpair->tnr[n_cnt].d_bid_sttg_prc = p_fxpair->tnr[n_cnt].d_bid_hdof_prc;
		// 고가
		if (p_fxpair->tnr[n_cnt].d_bid_hgpr_prc < p_fxpair->tnr[n_cnt].d_bid_hdof_prc)
			p_fxpair->tnr[n_cnt].d_bid_hgpr_prc = p_fxpair->tnr[n_cnt].d_bid_hdof_prc;
		// 저가
		if (p_fxpair->tnr[n_cnt].d_bid_lw_prc > p_fxpair->tnr[n_cnt].d_bid_hdof_prc)
			p_fxpair->tnr[n_cnt].d_bid_lw_prc = p_fxpair->tnr[n_cnt].d_bid_hdof_prc;

		n_cnt++;
	}
	return (0);

}
// 재정통화Pair 시세 게산
// usd_pair  : USDKRW 시세
// std_pair  : USD기준 시세
// fnnl_pair : 재정통화 시세
int l_calc_fnnl_fxpair(p_usd_pair, p_std_pair, p_fnl_pair, s_errcd)
FXPAIR_ST *p_usd_pair;
FXPAIR_ST *p_std_pair;
FXPAIR_ST *p_fnl_pair;
char    *s_errcd;
{

	double    PX_UNIT = 0.0;
	int       N_DGT   = 0;
	int       n_cnt   = 0;
	
	FXTNR_ST *p_usd_tnr;
	FXTNR_ST *p_std_tnr;
	FXTNR_ST *p_fnl_tnr;
	

	// JPYKRW 100.0 나머지 1.0
	PX_UNIT  = p_fnl_pair->d_clc_unit;
	N_DGT    = p_fnl_pair->n_digit;

	
	p_fnl_pair->s_ask_orgn[0]     = p_usd_pair->s_ask_orgn[0];
	p_fnl_pair->s_bid_orgn[0]     = p_usd_pair->s_bid_orgn[0];

	memcpy(p_fnl_pair->s_rcv_ymd, p_usd_pair->s_rcv_ymd, sizeof(p_usd_pair->s_rcv_ymd));
	memcpy(p_fnl_pair->s_rcv_hms, p_usd_pair->s_rcv_hms, sizeof(p_usd_pair->s_rcv_hms));

	if (p_fnl_pair->s_clc_dsnc[0] == '2')
	{
		// 재정통화 ASK SPOT가격
		// round(USDKRW ASK SPOT가격 / 비재정 BID SPOT가격 * PX_UNIT, N_DGT)
		p_fnl_pair->d_ask_spt_prc = p_usd_pair->d_ask_spt_prc / p_std_pair->d_bid_spt_prc * PX_UNIT;
		p_fnl_pair->d_bid_spt_prc = p_usd_pair->d_bid_spt_prc / p_std_pair->d_ask_spt_prc * PX_UNIT;
	}
	else
	{
		p_fnl_pair->d_ask_spt_prc = p_usd_pair->d_ask_spt_prc * p_std_pair->d_bid_spt_prc;
		p_fnl_pair->d_bid_spt_prc = p_usd_pair->d_bid_spt_prc * p_std_pair->d_bid_spt_prc;
	}

	while (n_cnt < p_fnl_pair->n_tnr_cnt)
	{
		p_usd_tnr = (FXTNR_ST *)&p_usd_pair->tnr[n_cnt];
		p_std_tnr = (FXTNR_ST *)&p_std_pair->tnr[n_cnt];
		p_fnl_tnr = (FXTNR_ST *)&p_fnl_pair->tnr[n_cnt];

		// ASK(매입방향) 계산
		// DIV
		if (p_fnl_pair->s_clc_dsnc[0] == '2') 
		{
		
			// 재정통화 ASK 본점마진가격
			// USDKRW ASK 본점마진가격 / 비재정 BID 본점마진가격 * PX_UNIT(1.0 or 100.0)
			p_fnl_tnr->d_ask_hdom_prc = p_usd_tnr->d_ask_hdom_prc / p_std_tnr->d_bid_hdom_prc * PX_UNIT; 
		
			// ASK(매입) 본점마진
			// ASK 본점마진가격 - ASK SPOT가격
			p_fnl_tnr->d_ask_hdom = l_round_d(p_fnl_tnr->d_ask_hdom_prc, N_DGT) - p_fnl_pair->d_ask_spt_prc;
		
			// 현물환
			if (strncmp(p_fnl_tnr->s_tnr_tcd, "TOD", 3) == 0 || 
				strncmp(p_fnl_tnr->s_tnr_tcd, "TOM", 3) == 0 ||
				strncmp(p_fnl_tnr->s_tnr_tcd, "SPT", 3) == 0)
			{
				// ASK(매입) 현물환 Cover마진
				// round(USDKRW ASK Cover마진가격 / 비재정 BID Cover마진가격 * PX_UNIT(1.0 or 100.0), N_DGT) - 재정 ASK SPOT가격
				p_fnl_tnr->d_ask_spt_cvmg = l_round_d(p_usd_tnr->d_ask_cvmg_prc / p_std_tnr->d_bid_cvmg_prc * PX_UNIT, N_DGT) - p_fnl_pair->d_ask_spt_prc;	

				// ASK(매입) 현물환 Corp마진	
				// ASK 본점마진 - ASK Cover마진
				p_fnl_tnr->d_ask_spt_cpmg = p_fnl_tnr->d_ask_hdom - p_fnl_tnr->d_ask_fwd_cvmg;

				// ASK(매입) 선물환 Cover마진
				p_fnl_tnr->d_ask_fwd_cvmg = 0.0;
				// ASK(매입) 선물환 Corp마진
				p_fnl_tnr->d_ask_fwd_cpmg = 0.0;
			
			}
			// 선물환
			else 
			{
				p_fnl_tnr->d_ask_spt_cvmg = 0.0;
				p_fnl_tnr->d_ask_spt_cpmg = 0.0;

				// ASK(매입) Cover마진
				// round(USDKRW ASK Cover마진가격 / 비재정 BID Cover마진가격 * PX_UNIT(1.0 or 100.0), N_DGT) - 재정 ASK SPOT가격
				p_fnl_tnr->d_ask_fwd_cvmg = l_round_d(p_usd_tnr->d_ask_cvmg_prc / p_std_tnr->d_bid_cvmg_prc * PX_UNIT, N_DGT) - p_fnl_pair->d_ask_spt_prc;

				// ASK(매입) 본점 Corp마진
				// ASK 본점마진 - ASK Cover마진
				p_fnl_tnr->d_ask_fwd_cpmg = p_fnl_tnr->d_ask_hdom - p_fnl_tnr->d_ask_fwd_cvmg;
			}	
		
			// 재정통화Pair 본점가격
			// USDKRW ASK 본점가격 / 비재정 BID 본점가격 * PX_UNIT(1.0 or 100.0)
			p_fnl_tnr->d_ask_hdof_prc = p_usd_tnr->d_ask_hdof_prc / p_std_tnr->d_bid_hdof_prc * PX_UNIT;

	
		}
		// 곱하기
		else 
		{
		
			// 본점마진가격
			// USDKRW ASK 본점마진가격 * 비재정 ASK 본점마진가격
			p_fnl_tnr->d_ask_hdom_prc = p_usd_tnr->d_ask_hdom_prc * p_std_tnr->d_ask_hdom_prc; 
		
			// ASK(매입) 본점마진
			// 본점마진가격 - SPOT 가격
			p_fnl_tnr->d_ask_hdom = l_round_d(p_fnl_tnr->d_ask_hdom_prc, N_DGT) - p_fnl_pair->d_ask_spt_prc;
		
			// 현물환
			if (strncmp(p_fnl_tnr->s_tnr_tcd, "TOD", 3) == 0 || 
				strncmp(p_fnl_tnr->s_tnr_tcd, "TOM", 3) == 0 ||
				strncmp(p_fnl_tnr->s_tnr_tcd, "SPT", 3) == 0)
			{
				// ASK(매입) 현물환 Cover마진
				// round(USDKRW ASK Cover마진가격 * 비재정 ASK Cover마진가격, N_DGT) - 재정 ASK SPOT가격
				p_fnl_tnr->d_ask_spt_cvmg = l_round_d(p_usd_tnr->d_ask_cvmg_prc * p_std_tnr->d_bid_cvmg_prc, N_DGT) - p_fnl_pair->d_ask_spt_prc;

				// ASK(매입) 현물환 Corp마진
				// ASK 본점마진 - ASK Cover마진
				p_fnl_tnr->d_ask_spt_cpmg = p_fnl_tnr->d_ask_hdom - p_fnl_tnr->d_ask_fwd_cvmg;

				// ASK(매입) 선물환 Cover마진
				p_fnl_tnr->d_ask_fwd_cvmg = 0.0;
				
				// ASK(매입) 선물환 Corp마진
				p_fnl_tnr->d_ask_fwd_cpmg = 0.0;
			
			}
			// 선물환
			else 
			{
				p_fnl_tnr->d_ask_spt_cvmg = 0.0;
				p_fnl_tnr->d_ask_spt_cpmg = 0.0;

				// ASK(매입) 선물환 Cover마진
				// round( USDKRW ASK Cover마진가격 * 비재정 ASK Cover마진가격, N_DGT ) - 재정 ASK SPOT가격
				p_fnl_tnr->d_ask_fwd_cvmg = l_round_d(p_usd_tnr->d_ask_cvmg_prc * p_std_tnr->d_ask_cvmg_prc, N_DGT) - p_fnl_pair->d_ask_spt_prc;

				// ASK(매입) 선물환 Corp마진
				p_fnl_tnr->d_ask_fwd_cpmg = p_fnl_tnr->d_ask_hdom - p_fnl_tnr->d_ask_fwd_cvmg;
			
			}	
		
			// ASK 본점가격
			// USDKRW ASK 본점가격 * 비재정 ASK 본점가격
			p_fnl_tnr->d_ask_hdof_prc = p_usd_tnr->d_ask_hdof_prc * p_std_tnr->d_ask_hdof_prc;
		
		}
	
		// SWAP 포인트
		// ( round(본점 가격, 4) - round(본점마진가격,4) ) * 100.0
		p_fnl_tnr->d_ask_swap_pnt = (l_round_d(p_fnl_tnr->d_ask_hdof_prc, 4) - l_round_d(p_fnl_tnr->d_ask_hdom_prc, 4)) * 100.0;
		l_round_x(&p_fnl_tnr->d_ask_swap_pnt, 2);
        
	    // SWAP 포인트 계산 후 round 처리
		l_round_x(&p_fnl_tnr->d_ask_hdof_prc, 2);

		l_round_x(&p_fnl_tnr->d_ask_hdom_prc, 2);

	
		// 전일대비증감
		p_fnl_tnr->d_ask_ctpd_indc = p_fnl_tnr->d_ask_pdcp - p_fnl_tnr->d_ask_hdof_prc;
		
		// 시작가
		if (p_fnl_tnr->d_ask_sttg_prc == 0.0)
			p_fnl_tnr->d_ask_sttg_prc = p_fnl_tnr->d_ask_hdof_prc;
		// 고가
		if (p_fnl_tnr->d_ask_hgpr_prc < p_fnl_tnr->d_ask_hdof_prc)
			p_fnl_tnr->d_ask_hgpr_prc = p_fnl_tnr->d_ask_hdof_prc;
		// 저가	
		if (p_fnl_tnr->d_ask_lw_prc > p_fnl_tnr->d_ask_hdof_prc)
			p_fnl_tnr->d_ask_lw_prc = p_fnl_tnr->d_ask_hdof_prc;



		// BID(매도) 계산
		// DIV
		if (p_fnl_pair->s_clc_dsnc[0] == '2') 
		{
		
			// 재정통화 BID 본점마진가격
			// USDKRW BID 본점마진가격 / 비재정 ASK 본점마진가격 * PX_UNIT(1.0 or 100.0)
			p_fnl_tnr->d_bid_hdom_prc = p_usd_tnr->d_bid_hdom_prc / p_std_tnr->d_ask_hdom_prc * PX_UNIT; 
		
			// BID(매도) 본점마진
			// BID 본점마진가격 - BID SPOT가격 * -1.0
			p_fnl_tnr->d_bid_hdom = (l_round_d(p_fnl_tnr->d_bid_hdom_prc, N_DGT) - p_fnl_pair->d_bid_spt_prc) * -1.0;
		
			// 현물환
			if (strncmp(p_fnl_tnr->s_tnr_tcd, "TOD", 3) == 0 || 
				strncmp(p_fnl_tnr->s_tnr_tcd, "TOM", 3) == 0 ||
				strncmp(p_fnl_tnr->s_tnr_tcd, "SPT", 3) == 0)
			{
				// BID(매도) 현물환 Cover마진
				// 현물환 Cover마진가격 =  SPOT가격 - 현물환 Cover마진
				// (round(USDKRW BID Cover마진가격 / 비재정 ASK Cover마진가격 * PX_UNIT(1.0 or 100.0), N_DGT) - 재정 BID SPOT가격 ) * -1.0
				p_fnl_tnr->d_bid_spt_cvmg = (l_round_d(p_usd_tnr->d_bid_cvmg_prc / p_std_tnr->d_ask_cvmg_prc * PX_UNIT, N_DGT) - p_fnl_pair->d_bid_spt_prc) * -1.0;

				// BID(매도) 현물환 Corp마진
				// BID 본점마진 - BID Cover마진
				p_fnl_tnr->d_bid_spt_cpmg = p_fnl_tnr->d_bid_hdom - p_fnl_tnr->d_bid_fwd_cvmg;

				// BID(매도) 선물환 Cover마진
				p_fnl_tnr->d_bid_fwd_cvmg = 0.0;
				// BID(매도) 선물환 Corp마진
				p_fnl_tnr->d_bid_fwd_cpmg = 0.0;
			
			}
			// 선물환
			else 
			{
				p_fnl_tnr->d_bid_spt_cvmg = 0.0;
				p_fnl_tnr->d_bid_spt_cpmg = 0.0;

				// BID(매도) 선물환 Cover마진
				// 선물환 Cover마진가격 =  SPOT가격 + 현물환 Cover마진 + 선물환 Cover마진
				// round(USDKRW BID Cover마진가격 / 비재정 ASK Cover마진가격 * PX_UNIT(1.0 or 100.0), N_DGT) - 재정 BID SPOT가격
				p_fnl_tnr->d_bid_fwd_cvmg = (l_round_d(p_usd_tnr->d_bid_cvmg_prc / p_std_tnr->d_ask_cvmg_prc * PX_UNIT, N_DGT) - p_fnl_pair->d_bid_spt_prc) * -1.0;

				// BID(매도) 본점 Corp마진
				// BID 본점마진 - BID Cover마진
				p_fnl_tnr->d_bid_fwd_cpmg = p_fnl_tnr->d_bid_hdom - p_fnl_tnr->d_bid_fwd_cvmg;
			
			}	
		
			// 재정통화Pair 본점가격
			// USDKRW BID 본점가격 / 비재정 BID 본점가격 * PX_UNIT(1.0 or 100.0)
			p_fnl_tnr->d_bid_hdof_prc = p_usd_tnr->d_bid_hdof_prc / p_std_tnr->d_bid_hdof_prc * PX_UNIT;

	
		}
		// 곱하기
		else 
		{
		
			// 본점마진가격
			// USDKRW BID 본점마진가격 * 비재정 BID 본점마진가격
			p_fnl_tnr->d_bid_hdom_prc = p_usd_tnr->d_bid_hdom_prc * p_std_tnr->d_bid_hdom_prc; 
		
			// 본점마진
			// 본점마진가격 - SPOT 가격
			p_fnl_tnr->d_bid_hdom = l_round_d(p_fnl_tnr->d_bid_hdom_prc, N_DGT) - p_fnl_pair->d_bid_spt_prc;
		
			// 현물환
			if (strncmp(p_fnl_tnr->s_tnr_tcd, "TOD", 3) == 0 || 
				strncmp(p_fnl_tnr->s_tnr_tcd, "TOM", 3) == 0 ||
				strncmp(p_fnl_tnr->s_tnr_tcd, "SPT", 3) == 0)
			{
				// 현물환 Cover마진
				// 현물환 Cover마진가격 =  SPOT가격 + 현물환 Cover마진
				// (round(USDKRW Cover마진가격 * 비재정 Cover마진가격, N_DGT) - 재정 SPOT가격) * -1.0
				p_fnl_tnr->d_bid_spt_cvmg = (l_round_d(p_usd_tnr->d_bid_cvmg_prc * p_std_tnr->d_bid_cvmg_prc, N_DGT) - p_fnl_pair->d_bid_spt_prc) * -1.0;

				// BID(매도) 현물환 Corp마진
				// BID 본점마진 - BID Cover마진
				p_fnl_tnr->d_bid_spt_cpmg = p_fnl_tnr->d_bid_hdom - p_fnl_tnr->d_bid_fwd_cvmg;

				// BID(매도) 선물환 Cover마진
				p_fnl_tnr->d_bid_fwd_cvmg = 0.0;
				// BID(매도) 선물환 Corp마진
				p_fnl_tnr->d_bid_fwd_cpmg = 0.0;
			
			}
			// 선물환
			else 
			{
				// 현물환 Cover마진
				p_fnl_tnr->d_bid_spt_cvmg = 0.0;
				// 현물환 Corp마진
				p_fnl_tnr->d_bid_spt_cpmg = 0.0;
				// 선물환 Cover마진
				// (round( USDKRW Cover마진가격 * 비재정 Cover마진가격, N_DGT ) - 재정 SPOT가격) * -1.0
				p_fnl_tnr->d_bid_fwd_cvmg = (l_round_d(p_usd_tnr->d_bid_cvmg_prc * p_std_tnr->d_bid_cvmg_prc, N_DGT) - p_fnl_pair->d_bid_spt_prc) * -1.0;
				// 선물환 Corp마진
				// 본점마진 - 선물환 Cover마진
				p_fnl_tnr->d_bid_fwd_cpmg = p_fnl_tnr->d_bid_hdom - p_fnl_tnr->d_bid_fwd_cvmg;
				
			}	
			
			// BID 본점가격
			// USDKRW BID 본점가격 * 비재정 BID 본점가격
			p_fnl_tnr->d_bid_hdof_prc = p_usd_tnr->d_bid_hdof_prc * p_std_tnr->d_bid_hdof_prc;
			
		}
		
		// SWAP 포인트
		// ( round(본점 가격, 4) - round(본점마진가격,4) ) * 100.0
		p_fnl_tnr->d_bid_swap_pnt = (l_round_d(p_fnl_tnr->d_bid_hdof_prc, 4) - l_round_d(p_fnl_tnr->d_bid_hdom_prc, 4)) * 100.0;
		l_round_x(&p_fnl_tnr->d_bid_swap_pnt, 2);
   	     
		// SWAP 포인트 계산 후 round 처리
		l_round_x(&p_fnl_tnr->d_bid_hdof_prc, 2);	

		l_round_x(&p_fnl_tnr->d_bid_hdom_prc, 2);

	
		// 전일대비증감
		p_fnl_tnr->d_bid_ctpd_indc = p_fnl_tnr->d_bid_pdcp - p_fnl_tnr->d_bid_hdof_prc;
			
		// 시작가
		if (p_fnl_tnr->d_bid_sttg_prc == 0.0)
			p_fnl_tnr->d_bid_sttg_prc = p_fnl_tnr->d_bid_hdof_prc;
		// 고가
		if (p_fnl_tnr->d_bid_hgpr_prc < p_fnl_tnr->d_bid_hdof_prc)
			p_fnl_tnr->d_bid_hgpr_prc = p_fnl_tnr->d_bid_hdof_prc;
		// 저가	
		if (p_fnl_tnr->d_bid_lw_prc > p_fnl_tnr->d_bid_hdof_prc)
			p_fnl_tnr->d_bid_lw_prc = p_fnl_tnr->d_bid_hdof_prc;

		n_cnt++;
	}			
	return (0);

}





// 선물환 비표준 가격정보 계산
// fwd_qt1 : 이전 테너 시세
// fwd_qt2 : 이후 테너 시세
// fwd_qt3 : 비표준 테너 시세
int l_calc_unstd_fwd_quot(fwd_qt1, fwd_qt2, fwd_qt, s_errcd)
HDQOUT_ST   *fwd_qt1 ;
HDQOUT_ST   *fwd_qt2 ;
HDQOUT_ST   *fwd_qt  ;
char        *s_errcd ;
{
	int      rst          = 0;
	int      N_DGT           ;
	
	double   D_DGT_VAL       ;
	double   d_dldv          ;

	N_DGT       = fwd_qt1->pair.n_digit;
	D_DGT_VAL   = fwd_qt1->pair.d_digit_val;
	
	// 일보간 결제일에 대한 일할 계산
	d_dldv = l_calc_dldv(fwd_qt1->tnr.s_sldy, fwd_qt2->tnr.s_sldy, fwd_qt->tnr.s_sldy, s_errcd);

	// ASK(매입방향) 비표준가격 계산
	fwd_qt->pair.d_ask_spt_prc = fwd_qt1->pair.d_ask_spt_prc;
	
	// SPOT 마진을 변경 없음
	fwd_qt->tnr.d_ask_spt_cvmg = fwd_qt1->tnr.d_ask_spt_cvmg;
	fwd_qt->tnr.d_ask_spt_cpmg = fwd_qt1->tnr.d_ask_spt_cpmg;
	
	// 선물환 Cover마진 계산
	// 이전, 이후 테너의 선물환 Cover마진이 같으면 
	// 이전 테너 FWD Cover마진 + 일할 * (이후 테너 FWD Cover마진 - 이전 테너 FWD Cover마진)
	if (fwd_qt1->tnr.d_ask_fwd_cvmg != fwd_qt2->tnr.d_ask_fwd_cvmg )
		fwd_qt->tnr.d_ask_fwd_cvmg = fwd_qt1->tnr.d_ask_fwd_cvmg + (fwd_qt2->tnr.d_ask_fwd_cvmg - fwd_qt1->tnr.d_ask_fwd_cvmg) * d_dldv;
	else 
		fwd_qt->tnr.d_ask_fwd_cvmg = fwd_qt1->tnr.d_ask_fwd_cvmg;
	
	l_round_x(&fwd_qt->tnr.d_ask_fwd_cvmg, 6);
	
	// 선물환 Corp마진 계산
	// 이전 테너 FWD Corp마진 + 일할 * (이후 테너 FWD Corp마진 - 이전 테너 FWD Corp마진)
	if (fwd_qt1->tnr.d_ask_fwd_cpmg != fwd_qt2->tnr.d_ask_fwd_cpmg )
		fwd_qt->tnr.d_ask_fwd_cpmg = fwd_qt1->tnr.d_ask_fwd_cpmg + (fwd_qt2->tnr.d_ask_fwd_cpmg - fwd_qt1->tnr.d_ask_fwd_cpmg) * d_dldv;
	else
		fwd_qt->tnr.d_ask_fwd_cpmg = fwd_qt1->tnr.d_ask_fwd_cpmg;
	
	l_round_x(&fwd_qt->tnr.d_ask_fwd_cpmg, 6);	

	// ASK(매입) 본점 마진
	// SPOT COVER마진 + SPOT CORP마진 + FWD COVER마진 + FWD CORP마진 
	fwd_qt->tnr.d_ask_hdom = fwd_qt->tnr.d_ask_spt_cvmg + fwd_qt->tnr.d_ask_spt_cpmg + fwd_qt->tnr.d_ask_fwd_cvmg + fwd_qt->tnr.d_ask_fwd_cpmg;			
	
	// ASK 본점마진 가격
	// SPOT 가격 + ASK 본점마진
	fwd_qt->tnr.d_ask_hdom_prc = fwd_qt->pair.d_ask_spt_prc + fwd_qt->tnr.d_ask_hdom;
	
	// ASK 본점가격(일할계산)
	// 이전 테너 본점가격 + 일할 * (이후 테너 본점가격 - 이전 테너 본점가격)
	fwd_qt->tnr.d_ask_hdof_prc = fwd_qt1->tnr.d_ask_hdof_prc + (fwd_qt2->tnr.d_ask_hdof_prc - fwd_qt1->tnr.d_ask_hdof_prc) * d_dldv;
	
	// ASK SWAP 포인트
	// (round(ASK본점가격,4) - round(ASK본점마진가격,4)) * 100.0
	//fwd_qt3->d_ask_swap_pnt = fwd_qt1->d_ask_swap_pnt + (fwd2->d_ask_swap_pnt - fwd1->d_ask_swap_pnt) * d_dldv;
	 fwd_qt->tnr.d_ask_swap_pnt = (l_round_d(fwd_qt->tnr.d_ask_hdof_prc, 4) - l_round_d(fwd_qt->tnr.d_ask_hdom_prc, 4)) * 100.0;

	// 시장가격
	// SPOT 가격 + SWAP 포인트 * (1 / 포인트DGT)
	fwd_qt->tnr.d_ask_mrkt_prc = fwd_qt->pair.d_ask_spt_prc + fwd_qt->tnr.d_ask_swap_pnt * D_DGT_VAL;

	l_round_x(&fwd_qt->tnr.d_ask_mrkt_prc, N_DGT);
	l_round_x(&fwd_qt->tnr.d_ask_hdom_prc, N_DGT);
	l_round_x(&fwd_qt->tnr.d_ask_hdof_prc, N_DGT);



    // BID(매도방향) 비표준가격 계산
    
    fwd_qt->pair.d_bid_spt_prc = fwd_qt1->pair.d_bid_spt_prc;
    
	// SPOT 본점마진율 변경 없음
	fwd_qt->tnr.d_ask_spt_cvmg = fwd_qt1->tnr.d_ask_spt_cvmg;
	fwd_qt->tnr.d_ask_spt_cpmg = fwd_qt1->tnr.d_ask_spt_cpmg;
	
	// 선물환 Cover마진 계산
	// 이전 테너 FWD Cover마진 + 일할 * (이후 테너 FWD Cover마진 - 이전 테너 FWD Cover마진)
	if (fwd_qt1->tnr.d_bid_fwd_cvmg != fwd_qt2->tnr.d_bid_fwd_cvmg )
		fwd_qt->tnr.d_bid_fwd_cvmg = fwd_qt1->tnr.d_bid_fwd_cvmg + (fwd_qt2->tnr.d_bid_fwd_cvmg - fwd_qt1->tnr.d_bid_fwd_cvmg) * d_dldv ;
	else 
		fwd_qt->tnr.d_bid_fwd_cvmg = fwd_qt1->tnr.d_bid_fwd_cvmg;
	
	l_round_x(&fwd_qt->tnr.d_bid_fwd_cvmg, 6);
	
	// 선물환 Corp마진 계산
	// 이전 테너 FWD Corp마진 + 일할 * (이후 테너 FWD Corp마진 - 이전 테너 FWD Corp마진)
	if (fwd_qt1->tnr.d_bid_fwd_cpmg != fwd_qt2->tnr.d_bid_fwd_cpmg )
		fwd_qt->tnr.d_bid_fwd_cpmg = fwd_qt1->tnr.d_bid_fwd_cpmg + (fwd_qt2->tnr.d_bid_fwd_cpmg - fwd_qt1->tnr.d_bid_fwd_cpmg) * d_dldv ;
	else
		fwd_qt->tnr.d_bid_fwd_cpmg = fwd_qt1->tnr.d_bid_fwd_cpmg;
	
	l_round_x(&fwd_qt->tnr.d_bid_fwd_cpmg, 6);	

	// BID(매도방향) 본점 마진
	// SPOT COVER마진 + SPOT CORP마진 + FWD COVER마진 + FWD CORP마진 
	fwd_qt->tnr.d_bid_hdom = fwd_qt->tnr.d_bid_spt_cvmg + fwd_qt->tnr.d_bid_spt_cpmg + fwd_qt->tnr.d_bid_fwd_cvmg + fwd_qt->tnr.d_bid_fwd_cpmg;			
	
	// 본점마진 가격
	// BID SPOT 가격 - BID 본점마진
	fwd_qt->tnr.d_bid_hdom_prc = fwd_qt->pair.d_bid_spt_prc - fwd_qt->tnr.d_bid_hdom;
	
	// 본점가격
	// 이전 테너 본점가격 + 일할 * (이후 테너 본점가격 - 이전 테너 본점가격)
	fwd_qt->tnr.d_bid_hdof_prc = fwd_qt1->tnr.d_bid_hdof_prc + (fwd_qt2->tnr.d_bid_hdof_prc - fwd_qt1->tnr.d_bid_hdof_prc) * d_dldv;
	
	// SWAP 포인트
	// fwd_qt->d_bid_swap_pnt = fwd_qt1->d_bid_swap_pnt + (fwd2->d_bid_swap_pnt - fwd1->d_bid_swap_pnt) * d_dldv;
	// (round(ASK본점가격,4) - round(ASK본점마진가격,4)) * 100.0
	fwd_qt->tnr.d_bid_swap_pnt = (l_round_d(fwd_qt->tnr.d_bid_hdof_prc, 4) - l_round_d(fwd_qt->tnr.d_bid_hdom_prc, 4)) * 100.0;
	l_round_x(&fwd_qt->tnr.d_bid_swap_pnt, 2);

	// ASK 시장가격
	// SPOT 가격 + SWAP 포인트 * (1 / [100, 1000, 10000])
	fwd_qt->tnr.d_bid_mrkt_prc = fwd_qt->pair.d_bid_spt_prc + fwd_qt->tnr.d_bid_swap_pnt * D_DGT_VAL;
	l_round_x(&fwd_qt->tnr.d_bid_mrkt_prc, N_DGT);
	
	l_round_x(&fwd_qt->tnr.d_bid_hdom_prc, N_DGT);
	l_round_x(&fwd_qt->tnr.d_bid_hdof_prc, N_DGT);

	return 0;
}





// USD/KRW, 비재정통화 표준테너 시세 계산
// 시세 수신시 계산
int l_calc_usd_qout(sise, qout)
APSISE_ST *sise;
HDQOUT_ST *qout;
{
	int      N_DGT ;
	double   D_DGT_VAL;
	
	N_DGT     = qout->pair.n_digit;
	D_DGT_VAL = qout->pair.d_digit_val;

	// ASK(매입) 계산
	if (sise->offerprc != qout->pair.d_ask_spt_prc)
	{
		qout->pair.d_ask_spt_prc = sise->offerprc;
		qout->tnr.d_ask_hdom_prc = qout->pair.d_ask_spt_prc + qout->tnr.d_ask_hdom;
			
		// 본점마진가격
		// 현물환
		if (strncmp(qout->tnr.s_tnr_tcd, "TOD", 3) == 0 || 
			strncmp(qout->tnr.s_tnr_tcd, "TOM", 3) == 0 ||
			strncmp(qout->tnr.s_tnr_tcd, "SPT", 3) == 0 )
		{
			// SPOT가격 + 현물환 Cover마진
			qout->tnr.d_ask_cvmg_prc = qout->pair.d_ask_spt_prc + qout->tnr.d_ask_spt_cvmg;
		}
		// 선물환	
		else 
		{
			// SPOT가격 + 현물환 Cover마진 + 선물환 Cover마진
			qout->tnr.d_ask_cvmg_prc = qout->pair.d_ask_spt_prc + qout->tnr.d_ask_spt_cvmg + qout->tnr.d_ask_fwd_cvmg;
		}
	}
		
	// 시장가격
	// SPOT가격 + SWAP포인트 * D_DGT_VAL[0.01, 0.001, 0.0001]
	qout->tnr.d_ask_mrkt_prc = l_round_d(qout->pair.d_ask_spt_prc + qout->tnr.d_ask_swap_pnt * D_DGT_VAL, N_DGT);

	// 본점가격
	// 시장가격 + 본점마진
	qout->tnr.d_ask_hdof_prc = qout->tnr.d_ask_mrkt_prc + qout->tnr.d_ask_hdom;
		
	l_round_x(&qout->tnr.d_ask_hdof_prc, N_DGT);

	// 전일대비증감
	qout->tnr.d_ask_ctpd_indc = qout->tnr.d_ask_pdcp - qout->tnr.d_ask_hdof_prc;
		
	// 시작가
	if (qout->tnr.d_ask_sttg_prc == 0.0)
		qout->tnr.d_ask_sttg_prc = qout->tnr.d_ask_hdof_prc;
	// 고가
	if (qout->tnr.d_ask_hgpr_prc < qout->tnr.d_ask_hdof_prc)
		qout->tnr.d_ask_hgpr_prc = qout->tnr.d_ask_hdof_prc;
	// 저가	
	if (qout->tnr.d_ask_lw_prc > qout->tnr.d_ask_hdof_prc)
		qout->tnr.d_ask_lw_prc = qout->tnr.d_ask_hdof_prc;
	

	
	// BID(매도) 계산
	if (sise->bidprc != qout->pair.d_bid_spt_prc)
	{
		qout->pair.d_bid_spt_prc = sise->bidprc;
			
		// Cover마진가격
		// 현물환
		if (strncmp(qout->tnr.s_tnr_tcd, "TOD", 3) == 0 || 
			strncmp(qout->tnr.s_tnr_tcd, "TOM", 3) == 0 ||
			strncmp(qout->tnr.s_tnr_tcd, "SPT", 3) == 0 )
		{
			// Cover마진가격 =  SPOT가격 + 현물환 Cover마진
			qout->tnr.d_bid_cvmg_prc = qout->pair.d_bid_spt_prc - qout->tnr.d_bid_spt_cvmg;
		}
		// 선물환	
		else 
		{
			// Cover마진가격 =  SPOT가격 + 현물환 Cover마진 + 선물환 Cover마진
			qout->tnr.d_bid_cvmg_prc = qout->pair.d_bid_spt_prc - qout->tnr.d_bid_spt_cvmg - qout->tnr.d_bid_fwd_cvmg;
		}
		l_round_x(&qout->tnr.d_bid_cvmg_prc, N_DGT);
			// 본점마진금액
		qout->tnr.d_bid_hdom_prc = qout->pair.d_bid_spt_prc - qout->tnr.d_bid_hdom;
	}
		
	// 시장가격
	// SPOT가격 + SWAP포인트 * D_DGT_VAL[0.01, 0.001, 0.0001]
	qout->tnr.d_bid_mrkt_prc = l_round_d(qout->pair.d_bid_spt_prc + qout->tnr.d_bid_swap_pnt * D_DGT_VAL, N_DGT);

	// 본점가격
	// 시장가격 - 본점마진
	qout->tnr.d_bid_hdof_prc = qout->tnr.d_bid_mrkt_prc - qout->tnr.d_bid_hdom;
	l_round_x(&qout->tnr.d_bid_hdof_prc, N_DGT);

	// 전일대비증감
	qout->tnr.d_bid_ctpd_indc = qout->tnr.d_bid_pdcp - qout->tnr.d_bid_hdof_prc;
		
	// 시작가
	if (qout->tnr.d_bid_sttg_prc == 0.0)
		qout->tnr.d_bid_sttg_prc = qout->tnr.d_bid_hdof_prc;
	// 고가
	if (qout->tnr.d_bid_hgpr_prc < qout->tnr.d_bid_hdof_prc)
		qout->tnr.d_bid_hgpr_prc = qout->tnr.d_bid_hdof_prc;
	// 저가	
	if (qout->tnr.d_bid_lw_prc > qout->tnr.d_bid_hdof_prc)
		qout->tnr.d_bid_lw_prc = qout->tnr.d_bid_hdof_prc;

	
	return (0);	
		
}

// 재정통화Pair 표준테너 시세 게산
// usd_qt  : USDKRW 시세
// std_qt  : USD기준 시세
// fnnl_qt : 재정통화 시세
int l_calc_fnnl_qout(usd_qt, std_qt, fnnl_qt, s_errcd)
HDQOUT_ST *usd_qt;
HDQOUT_ST *std_qt;
HDQOUT_ST *fnnl_qt;
char    *s_errcd;
{

	double PX_UNIT = 0.0;
	int    N_DGT   = 0;
	
	// JPYKRW 100.0 나머지 1.0
	PX_UNIT  = fnnl_qt->pair.d_clc_unit;
	N_DGT    = fnnl_qt->pair.n_digit;
	
	
	// ASK(매입방향) 계산
	// DIV
	if (fnnl_qt->pair.s_clc_dsnc[0] == '2') 
	{
		// 재정통화 ASK SPOT가격
		// round(USDKRW ASK SPOT가격 / 비재정 BID SPOT가격 * PX_UNIT, N_DGT)
		fnnl_qt->pair.d_ask_spt_prc = l_round_d(usd_qt->pair.d_ask_spt_prc / std_qt->pair.d_bid_spt_prc * PX_UNIT, N_DGT);
		
		// 재정통화 ASK 본점마진가격
		// USDKRW ASK 본점마진가격 / 비재정 BID 본점마진가격 * PX_UNIT(1.0 or 100.0)
		fnnl_qt->tnr.d_ask_hdom_prc = usd_qt->tnr.d_ask_hdom_prc / std_qt->tnr.d_bid_hdom_prc * PX_UNIT; 
		
		// ASK(매입) 본점마진
		// ASK 본점마진가격 - ASK SPOT가격
		fnnl_qt->tnr.d_ask_hdom = l_round_d(fnnl_qt->tnr.d_ask_hdom_prc, N_DGT) - fnnl_qt->pair.d_ask_spt_prc;
		
		// 현물환
		if (strncmp(fnnl_qt->tnr.s_tnr_tcd, "TOD", 3) == 0 || 
			strncmp(fnnl_qt->tnr.s_tnr_tcd, "TOM", 3) == 0 ||
			strncmp(fnnl_qt->tnr.s_tnr_tcd, "SPT", 3) == 0)
		{
			// ASK(매입) 현물환 Cover마진
			// round(USDKRW ASK Cover마진가격 / 비재정 BID Cover마진가격 * PX_UNIT(1.0 or 100.0), N_DGT) - 재정 ASK SPOT가격
			fnnl_qt->tnr.d_ask_spt_cvmg = l_round_d(usd_qt->tnr.d_ask_cvmg_prc / std_qt->tnr.d_bid_cvmg_prc * PX_UNIT, N_DGT) - fnnl_qt->pair.d_ask_spt_prc;

			// ASK(매입) 현물환 Corp마진
			// ASK 본점마진 - ASK Cover마진
			fnnl_qt->tnr.d_ask_spt_cpmg = fnnl_qt->tnr.d_ask_hdom - fnnl_qt->tnr.d_ask_fwd_cvmg;

			// ASK(매입) 선물환 Cover마진
			fnnl_qt->tnr.d_ask_fwd_cvmg = 0.0;
			// ASK(매입) 선물환 Corp마진
			fnnl_qt->tnr.d_ask_fwd_cpmg = 0.0;
			
		}
		// 선물환
		else 
		{
			fnnl_qt->tnr.d_ask_spt_cvmg = 0.0;
			fnnl_qt->tnr.d_ask_spt_cpmg = 0.0;

			// ASK(매입) Cover마진
			// round(USDKRW ASK Cover마진가격 / 비재정 BID Cover마진가격 * PX_UNIT(1.0 or 100.0), N_DGT) - 재정 ASK SPOT가격
			fnnl_qt->tnr.d_ask_fwd_cvmg = l_round_d(usd_qt->tnr.d_ask_cvmg_prc / std_qt->tnr.d_bid_cvmg_prc * PX_UNIT, N_DGT) - fnnl_qt->pair.d_ask_spt_prc;

			// ASK(매입) 본점 Corp마진
			// ASK 본점마진 - ASK Cover마진
			fnnl_qt->tnr.d_ask_fwd_cpmg = fnnl_qt->tnr.d_ask_hdom - fnnl_qt->tnr.d_ask_fwd_cvmg;
		}	
		
		// 재정통화Pair 본점가격
		// USDKRW ASK 본점가격 / 비재정 BID 본점가격 * PX_UNIT(1.0 or 100.0)
		fnnl_qt->tnr.d_ask_hdof_prc = usd_qt->tnr.d_ask_hdof_prc / std_qt->tnr.d_bid_hdof_prc * PX_UNIT;

	
	}
	// 곱하기
	else 
	{
		// 재정통화Pair ASK SPOT가격
		// round(USDKRW ASK SPOT가격 * 비재정 ASK SPOT가격, N_DGT)
		fnnl_qt->pair.d_ask_spt_prc = l_round_d(usd_qt->pair.d_ask_spt_prc * std_qt->pair.d_ask_spt_prc, N_DGT);
		
		// 본점마진가격
		// USDKRW ASK 본점마진가격 * 비재정 ASK 본점마진가격
		fnnl_qt->tnr.d_ask_hdom_prc = usd_qt->tnr.d_ask_hdom_prc * std_qt->tnr.d_ask_hdom_prc; 
		
		// ASK(매입) 본점마진
		// 본점마진가격 - SPOT 가격
		fnnl_qt->tnr.d_ask_hdom = l_round_d(fnnl_qt->tnr.d_ask_hdom_prc, N_DGT) - fnnl_qt->pair.d_ask_spt_prc;
		
		// 현물환
		if (strncmp(fnnl_qt->tnr.s_tnr_tcd, "TOD", 3) == 0 || 
			strncmp(fnnl_qt->tnr.s_tnr_tcd, "TOM", 3) == 0 ||
			strncmp(fnnl_qt->tnr.s_tnr_tcd, "SPT", 3) == 0)
		{
			// ASK(매입) 현물환 Cover마진
			// round(USDKRW ASK Cover마진가격 * 비재정 ASK Cover마진가격, N_DGT) - 재정 ASK SPOT가격
			fnnl_qt->tnr.d_ask_spt_cvmg = l_round_d(usd_qt->tnr.d_ask_cvmg_prc * std_qt->tnr.d_bid_cvmg_prc, N_DGT) - fnnl_qt->pair.d_ask_spt_prc;

			// ASK(매입) 현물환 Corp마진
			// ASK 본점마진 - ASK Cover마진
			fnnl_qt->tnr.d_ask_spt_cpmg = fnnl_qt->tnr.d_ask_hdom - fnnl_qt->tnr.d_ask_fwd_cvmg;

			// ASK(매입) 선물환 Cover마진
			fnnl_qt->tnr.d_ask_fwd_cvmg = 0.0;
			// ASK(매입) 선물환 Corp마진
			fnnl_qt->tnr.d_ask_fwd_cpmg = 0.0;
			
		}
		// 선물환
		else 
		{
			fnnl_qt->tnr.d_ask_spt_cvmg = 0.0;
			fnnl_qt->tnr.d_ask_spt_cpmg = 0.0;
			// ASK(매입) 선물환 Cover마진
			// round( USDKRW ASK Cover마진가격 * 비재정 ASK Cover마진가격, N_DGT ) - 재정 ASK SPOT가격
			fnnl_qt->tnr.d_ask_fwd_cvmg = l_round_d(usd_qt->tnr.d_ask_cvmg_prc * std_qt->tnr.d_ask_cvmg_prc, N_DGT) - fnnl_qt->pair.d_ask_spt_prc;
			// ASK(매입) 선물환 Corp마진
			fnnl_qt->tnr.d_ask_fwd_cpmg = fnnl_qt->tnr.d_ask_hdom - fnnl_qt->tnr.d_ask_fwd_cvmg;
			
		}	
		
		// ASK 본점가격
		// USDKRW ASK 본점가격 * 비재정 ASK 본점가격
		fnnl_qt->tnr.d_ask_hdof_prc = usd_qt->tnr.d_ask_hdof_prc * std_qt->tnr.d_ask_hdof_prc;
		
	}
	
	// SWAP 포인트
	// ( round(본점 가격, 4) - round(본점마진가격,4) ) * 100.0
	fnnl_qt->tnr.d_ask_swap_pnt = (l_round_d(fnnl_qt->tnr.d_ask_hdof_prc, 4) - l_round_d(fnnl_qt->tnr.d_ask_hdom_prc, 4)) * 100.0;
	l_round_x(&fnnl_qt->tnr.d_ask_swap_pnt, 2);
        
    // SWAP 포인트 계산 후 round 처리
	l_round_x(&fnnl_qt->tnr.d_ask_hdof_prc, 2);

	l_round_x(&fnnl_qt->tnr.d_ask_hdom_prc, 2);

	
	// 전일대비증감
	fnnl_qt->tnr.d_ask_ctpd_indc = fnnl_qt->tnr.d_ask_pdcp - fnnl_qt->tnr.d_ask_hdof_prc;
		
	// 시작가
	if (fnnl_qt->tnr.d_ask_sttg_prc == 0.0)
		fnnl_qt->tnr.d_ask_sttg_prc = fnnl_qt->tnr.d_ask_hdof_prc;
	// 고가
	if (fnnl_qt->tnr.d_ask_hgpr_prc < fnnl_qt->tnr.d_ask_hdof_prc)
		fnnl_qt->tnr.d_ask_hgpr_prc = fnnl_qt->tnr.d_ask_hdof_prc;
	// 저가	
	if (fnnl_qt->tnr.d_ask_lw_prc > fnnl_qt->tnr.d_ask_hdof_prc)
		fnnl_qt->tnr.d_ask_lw_prc = fnnl_qt->tnr.d_ask_hdof_prc;



	// BID(매도) 계산
	// DIV
	if (fnnl_qt->pair.s_clc_dsnc[0] == '2') 
	{
		// 재정통화 BID SPOT가격
		// round(USDKRW BID SPOT가격 / 비재정 ASK SPOT가격 * PX_UNIT, N_DGT)
		fnnl_qt->pair.d_bid_spt_prc = l_round_d(usd_qt->pair.d_bid_spt_prc / std_qt->pair.d_ask_spt_prc * PX_UNIT, N_DGT);
		
		// 재정통화 BID 본점마진가격
		// USDKRW BID 본점마진가격 / 비재정 ASK 본점마진가격 * PX_UNIT(1.0 or 100.0)
		fnnl_qt->tnr.d_bid_hdom_prc = usd_qt->tnr.d_bid_hdom_prc / std_qt->tnr.d_ask_hdom_prc * PX_UNIT; 
		
		// BID(매도) 본점마진
		// BID 본점마진가격 - BID SPOT가격 * -1.0
		fnnl_qt->tnr.d_bid_hdom = (l_round_d(fnnl_qt->tnr.d_bid_hdom_prc, N_DGT) - fnnl_qt->pair.d_bid_spt_prc) * -1.0;
		
		// 현물환
		if (strncmp(fnnl_qt->tnr.s_tnr_tcd, "TOD", 3) == 0 || 
			strncmp(fnnl_qt->tnr.s_tnr_tcd, "TOM", 3) == 0 ||
			strncmp(fnnl_qt->tnr.s_tnr_tcd, "SPT", 3) == 0)
		{
			// BID(매도) 현물환 Cover마진
			// 현물환 Cover마진가격 =  SPOT가격 - 현물환 Cover마진
			// (round(USDKRW BID Cover마진가격 / 비재정 ASK Cover마진가격 * PX_UNIT(1.0 or 100.0), N_DGT) - 재정 BID SPOT가격 ) * -1.0
			fnnl_qt->tnr.d_bid_spt_cvmg = (l_round_d(usd_qt->tnr.d_bid_cvmg_prc / std_qt->tnr.d_ask_cvmg_prc * PX_UNIT, N_DGT) - fnnl_qt->pair.d_bid_spt_prc) * -1.0;

			// BID(매도) 현물환 Corp마진
			// BID 본점마진 - BID Cover마진
			fnnl_qt->tnr.d_bid_spt_cpmg = fnnl_qt->tnr.d_bid_hdom - fnnl_qt->tnr.d_bid_fwd_cvmg;

			// BID(매도) 선물환 Cover마진
			fnnl_qt->tnr.d_bid_fwd_cvmg = 0.0;
			// BID(매도) 선물환 Corp마진
			fnnl_qt->tnr.d_bid_fwd_cpmg = 0.0;
			
		}
		// 선물환
		else 
		{
			fnnl_qt->tnr.d_bid_spt_cvmg = 0.0;
			fnnl_qt->tnr.d_bid_spt_cpmg = 0.0;

			// BID(매도) 선물환 Cover마진
			// 선물환 Cover마진가격 =  SPOT가격 + 현물환 Cover마진 + 선물환 Cover마진
			// round(USDKRW BID Cover마진가격 / 비재정 ASK Cover마진가격 * PX_UNIT(1.0 or 100.0), N_DGT) - 재정 BID SPOT가격
			fnnl_qt->tnr.d_bid_fwd_cvmg = (l_round_d(usd_qt->tnr.d_bid_cvmg_prc / std_qt->tnr.d_ask_cvmg_prc * PX_UNIT, N_DGT) - fnnl_qt->pair.d_bid_spt_prc) * -1.0;

			// BID(매도) 본점 Corp마진
			// BID 본점마진 - BID Cover마진
			fnnl_qt->tnr.d_bid_fwd_cpmg = fnnl_qt->tnr.d_bid_hdom - fnnl_qt->tnr.d_bid_fwd_cvmg;
			
		}	
		
		// 재정통화Pair 본점가격
		// USDKRW BID 본점가격 / 비재정 BID 본점가격 * PX_UNIT(1.0 or 100.0)
		fnnl_qt->tnr.d_bid_hdof_prc = usd_qt->tnr.d_bid_hdof_prc / std_qt->tnr.d_bid_hdof_prc * PX_UNIT;

	
	}
	// 곱하기
	else 
	{
		// 재정통화Pair BID SPOT가격
		// round(USDKRW BID SPOT가격 * 비재정 BID SPOT가격, N_DGT)
		fnnl_qt->pair.d_bid_spt_prc = l_round_d(usd_qt->pair.d_bid_spt_prc * std_qt->pair.d_bid_spt_prc, N_DGT);
		
		// 본점마진가격
		// USDKRW BID 본점마진가격 * 비재정 BID 본점마진가격
		fnnl_qt->tnr.d_bid_hdom_prc = usd_qt->tnr.d_bid_hdom_prc * std_qt->tnr.d_bid_hdom_prc; 
		
		// 본점마진
		// 본점마진가격 - SPOT 가격
		fnnl_qt->tnr.d_bid_hdom = l_round_d(fnnl_qt->tnr.d_bid_hdom_prc, N_DGT) - fnnl_qt->pair.d_bid_spt_prc;
		
		// 현물환
		if (strncmp(fnnl_qt->tnr.s_tnr_tcd, "TOD", 3) == 0 || 
			strncmp(fnnl_qt->tnr.s_tnr_tcd, "TOM", 3) == 0 ||
			strncmp(fnnl_qt->tnr.s_tnr_tcd, "SPT", 3) == 0)
		{
			// 현물환 Cover마진
			// 현물환 Cover마진가격 =  SPOT가격 + 현물환 Cover마진
			// (round(USDKRW Cover마진가격 * 비재정 Cover마진가격, N_DGT) - 재정 SPOT가격) * -1.0
			fnnl_qt->tnr.d_bid_spt_cvmg = (l_round_d(usd_qt->tnr.d_bid_cvmg_prc * std_qt->tnr.d_bid_cvmg_prc, N_DGT) - fnnl_qt->pair.d_bid_spt_prc) * -1.0;

			// BID(매도) 현물환 Corp마진
			// BID 본점마진 - BID Cover마진
			fnnl_qt->tnr.d_bid_spt_cpmg = fnnl_qt->tnr.d_bid_hdom - fnnl_qt->tnr.d_bid_fwd_cvmg;

			// BID(매도) 선물환 Cover마진
			fnnl_qt->tnr.d_bid_fwd_cvmg = 0.0;
			// BID(매도) 선물환 Corp마진
			fnnl_qt->tnr.d_bid_fwd_cpmg = 0.0;
			
		}
		// 선물환
		else 
		{
			// 현물환 Cover마진
			fnnl_qt->tnr.d_bid_spt_cvmg = 0.0;
			// 현물환 Corp마진
			fnnl_qt->tnr.d_bid_spt_cpmg = 0.0;
			// 선물환 Cover마진
			// (round( USDKRW Cover마진가격 * 비재정 Cover마진가격, N_DGT ) - 재정 SPOT가격) * -1.0
			fnnl_qt->tnr.d_bid_fwd_cvmg = (l_round_d(usd_qt->tnr.d_bid_cvmg_prc * std_qt->tnr.d_bid_cvmg_prc, N_DGT) - fnnl_qt->pair.d_bid_spt_prc) * -1.0;
			// 선물환 Corp마진
			// 본점마진 - 선물환 Cover마진
			fnnl_qt->tnr.d_bid_fwd_cpmg = fnnl_qt->tnr.d_bid_hdom - fnnl_qt->tnr.d_bid_fwd_cvmg;
			
		}	
		
		// BID 본점가격
		// USDKRW BID 본점가격 * 비재정 BID 본점가격
		fnnl_qt->tnr.d_bid_hdof_prc = usd_qt->tnr.d_bid_hdof_prc * std_qt->tnr.d_bid_hdof_prc;
		
	}
	
	// SWAP 포인트
	// ( round(본점 가격, 4) - round(본점마진가격,4) ) * 100.0
	fnnl_qt->tnr.d_bid_swap_pnt = (l_round_d(fnnl_qt->tnr.d_bid_hdof_prc, 4) - l_round_d(fnnl_qt->tnr.d_bid_hdom_prc, 4)) * 100.0;
	l_round_x(&fnnl_qt->tnr.d_bid_swap_pnt, 2);
        
    // SWAP 포인트 계산 후 round 처리
	l_round_x(&fnnl_qt->tnr.d_bid_hdof_prc, 2);

	l_round_x(&fnnl_qt->tnr.d_bid_hdom_prc, 2);

	
	// 전일대비증감
	fnnl_qt->tnr.d_bid_ctpd_indc = fnnl_qt->tnr.d_bid_pdcp - fnnl_qt->tnr.d_bid_hdof_prc;
		
	// 시작가
	if (fnnl_qt->tnr.d_bid_sttg_prc == 0.0)
		fnnl_qt->tnr.d_bid_sttg_prc = fnnl_qt->tnr.d_bid_hdof_prc;
	// 고가
	if (fnnl_qt->tnr.d_bid_hgpr_prc < fnnl_qt->tnr.d_bid_hdof_prc)
		fnnl_qt->tnr.d_bid_hgpr_prc = fnnl_qt->tnr.d_bid_hdof_prc;
	// 저가	
	if (fnnl_qt->tnr.d_bid_lw_prc > fnnl_qt->tnr.d_bid_hdof_prc)
		fnnl_qt->tnr.d_bid_lw_prc = fnnl_qt->tnr.d_bid_hdof_prc;
		
	return (0);

}

// 재정통화Pair 비표준테너 계산
// fnnl_qt1 : 이전 테너 시세 정보
// fnnl_qt2 : 이후 테너 시세 정보
// fnnl_qt  : 비표준 테너 시세 정보
int l_calc_fnnl_unstd(fnnl_qt1, fnnl_qt2, fnnl_qt, s_errcd)
HDQOUT_ST *fnnl_qt1;
HDQOUT_ST *fnnl_qt2;
HDQOUT_ST *fnnl_qt;
char      *s_errcd;
{
	
	double  PX_UNIT = 0.0;
	double  d_dldv       ;
	
	int     N_DGT     = 0;
	
	// JPYKRW 100.0 나머지 1.0
	PX_UNIT  = fnnl_qt->pair.d_clc_unit;
	N_DGT    = fnnl_qt->pair.n_digit;
	
	// 일보간 결제일에 대한 일할 계산
	d_dldv = (double) l_calc_dldv(fnnl_qt1->tnr.s_sldy, fnnl_qt2->tnr.s_sldy, fnnl_qt->tnr.s_sldy, s_errcd);


	// ASK(매입) 계산
	// SPOT 가격
	fnnl_qt->pair.d_ask_spt_prc  = fnnl_qt1->pair.d_ask_spt_prc ;
	
	// 본점가격계산
	fnnl_qt->tnr.d_ask_hdof_prc  = fnnl_qt1->tnr.d_ask_hdof_prc + (fnnl_qt2->tnr.d_ask_hdof_prc - fnnl_qt1->tnr.d_ask_hdof_prc) * d_dldv ;
	
	// 시장가격
	fnnl_qt->tnr.d_ask_mrkt_prc  = fnnl_qt1->tnr.d_ask_mrkt_prc + (fnnl_qt2->tnr.d_ask_mrkt_prc - fnnl_qt1->tnr.d_ask_mrkt_prc) * d_dldv ;
	
	// 본점마진
	//fnnl_qt->d_ask_hdom      = l_round_d(fnnl_qt->d_ask_hdof_prc - fnnl_qt->d_ask_mrkt_prc, 6) ;
	fnnl_qt->tnr.d_ask_hdom      = fnnl_qt1->tnr.d_ask_hdom + (fnnl_qt2->tnr.d_ask_hdom - fnnl_qt1->tnr.d_ask_hdom) * d_dldv;
		
    // 본점마진가격
	fnnl_qt->tnr.d_ask_hdom_prc  = fnnl_qt->pair.d_ask_spt_prc + fnnl_qt->tnr.d_ask_hdom;

	// Cover마진
	if (fnnl_qt1->tnr.d_ask_fwd_cvmg != fnnl_qt2->tnr.d_ask_fwd_cvmg)
	{	
		fnnl_qt->tnr.d_ask_fwd_cvmg = fnnl_qt1->tnr.d_ask_fwd_cvmg + (fnnl_qt2->tnr.d_ask_fwd_cvmg - fnnl_qt1->tnr.d_ask_fwd_cvmg) * d_dldv ;
		l_round_x(&fnnl_qt->tnr.d_ask_fwd_cvmg, 6);
	}	
	else 
		fnnl_qt->tnr.d_ask_fwd_cvmg = fnnl_qt1->tnr.d_ask_fwd_cvmg ;

	// Corp마진
	fnnl_qt->tnr.d_ask_fwd_cpmg  = fnnl_qt->tnr.d_ask_hdom - fnnl_qt->tnr.d_ask_fwd_cvmg ;
	
	// SWAP 포인트
	fnnl_qt->tnr.d_ask_swap_pnt  = (l_round_d(fnnl_qt->tnr.d_ask_hdof_prc, 4) - l_round_d(fnnl_qt->tnr.d_ask_hdom_prc, 4)) * 100.0;
	
	l_round_x(&fnnl_qt->tnr.d_ask_hdof_prc, N_DGT) ;
	l_round_x(&fnnl_qt->tnr.d_ask_mrkt_prc, N_DGT) ;
	l_round_x(&fnnl_qt->tnr.d_ask_hdom_prc, N_DGT) ;

	
    // BID(매도방향) 계산
    // Spot가격
    fnnl_qt->pair.d_bid_spt_prc  = fnnl_qt1->pair.d_bid_spt_prc ;
    
    // 본점가격
	fnnl_qt->tnr.d_bid_hdof_prc = fnnl_qt1->tnr.d_bid_hdof_prc + (fnnl_qt2->tnr.d_bid_hdof_prc - fnnl_qt1->tnr.d_bid_hdof_prc) * d_dldv ;
	
	// 시장가격
	fnnl_qt->tnr.d_bid_mrkt_prc = fnnl_qt1->tnr.d_bid_mrkt_prc + (fnnl_qt2->tnr.d_bid_mrkt_prc - fnnl_qt1->tnr.d_bid_mrkt_prc) * d_dldv ;
	
	// 본점마진
	fnnl_qt->tnr.d_bid_hdom     = l_round_d(fnnl_qt->tnr.d_bid_mrkt_prc - fnnl_qt->tnr.d_bid_hdof_prc, 6) ;

	// 본점마진가격
	fnnl_qt->tnr.d_bid_hdom_prc = fnnl_qt1->pair.d_bid_spt_prc - fnnl_qt->tnr.d_bid_hdom ;
		

	// Cover마진	
	if (fnnl_qt1->tnr.d_bid_fwd_cvmg != fnnl_qt2->tnr.d_bid_fwd_cvmg)
	{	
		fnnl_qt->tnr.d_bid_fwd_cvmg = fnnl_qt1->tnr.d_bid_fwd_cvmg + (fnnl_qt2->tnr.d_bid_fwd_cvmg - fnnl_qt1->tnr.d_bid_fwd_cvmg) * d_dldv ;
		l_round_x(&fnnl_qt->tnr.d_bid_fwd_cvmg, 6);
	}	
	else 
		fnnl_qt->tnr.d_bid_fwd_cvmg = fnnl_qt1->tnr.d_bid_fwd_cvmg;

	fnnl_qt->tnr.d_bid_fwd_cpmg = fnnl_qt->tnr.d_bid_hdom_prc - fnnl_qt->tnr.d_bid_fwd_cvmg;
		
	// SWAP 포인트
	fnnl_qt->tnr.d_bid_swap_pnt  = (l_round_d(fnnl_qt->tnr.d_bid_hdof_prc, 4) - l_round_d(fnnl_qt->tnr.d_ask_hdom_prc, 4)) * 100.0;

	l_round_x(&fnnl_qt->tnr.d_bid_hdof_prc, N_DGT) ;
	l_round_x(&fnnl_qt->tnr.d_bid_mrkt_prc, N_DGT) ;
	l_round_x(&fnnl_qt->tnr.d_bid_hdom_prc, N_DGT) ;
		
	return 0;		

}




// 재정통화Pair 만기선택 체결 계산(USDKRW 제외)
// s_bysel_dcd : 매입매도구분코드(B:매입, S:매도)
// usd_qt1  : 결제일 USDKRW 시세
// usd_qt2  : 만기일 USDKRW 시세
// std_qt1  : 결제일 USD기준 시세
// std_qt2  : 만기일 USD기준 시세
// fnnl_qt  : 재정통화 시세
int l_calc_fnnl_expi_sclt(s_bysel_dcd, usd_qt1, usd_qt2, std_qt1, std_qt2, fnnl_qt, splt_qt, s_errcd)
char          *s_bysel_dcd;
HDQOUT_ST     *usd_qt1;
HDQOUT_ST     *usd_qt2;
HDQOUT_ST     *std_qt1;
HDQOUT_ST     *std_qt2;
HDQOUT_ST     *fnnl_qt;
SPLIT_OUT_ST  *splt_qt;
char          *s_errcd;
{

	HDQOUT_ST	usd_qt;	
	HDQOUT_ST	std_qt;
	
	double		PX_UNIT        = 0.0;
	
	double      d_hdof_prc1    = 0.0;
	double      d_hdof_prc2    = 0.0;
	double      d_hdof_prc3    = 0.0;
	double      d_hdof_prc4    = 0.0;
	
	double		d_hdom         = 0.0;
	double      d_hdom_prc     = 0.0;

	int			N_DGT = 0;
	
	// JPYKRW 100.0 나머지 1.0
	PX_UNIT   = fnnl_qt->pair.d_clc_unit;
	N_DGT     = fnnl_qt->pair.n_digit;

	
	// 매입
	if (s_bysel_dcd[0] == 'B')
	{
		// DIV
		if (fnnl_qt->pair.s_clc_dsnc[0] == '2')
		{
			d_hdof_prc1 = usd_qt1->tnr.d_ask_hdof_prc / std_qt1->tnr.d_bid_hdof_prc * PX_UNIT;
			d_hdof_prc2 = usd_qt1->tnr.d_ask_hdof_prc / std_qt2->tnr.d_bid_hdof_prc * PX_UNIT;
			d_hdof_prc3 = usd_qt2->tnr.d_ask_hdof_prc / std_qt1->tnr.d_bid_hdof_prc * PX_UNIT;
			d_hdof_prc4 = usd_qt2->tnr.d_ask_hdof_prc / std_qt2->tnr.d_bid_hdof_prc * PX_UNIT;

			// 매입의 경우 본점가격이 큰 경우를 계산한다			
			fnnl_qt->tnr.d_ask_hdof_prc = 0.0;
			if (d_hdof_prc1 >= fnnl_qt->tnr.d_ask_hdof_prc)
			{
				fnnl_qt->tnr.d_ask_hdof_prc = d_hdof_prc1;
				memcpy(&usd_qt, usd_qt1, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt1, sizeof(HDQOUT_ST));
			}	

			if (d_hdof_prc2 >= fnnl_qt->tnr.d_ask_hdof_prc)
			{
				fnnl_qt->tnr.d_ask_hdof_prc = d_hdof_prc2;
				memcpy(&usd_qt, usd_qt1, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt2, sizeof(HDQOUT_ST));
			}	
			
			if (d_hdof_prc3 >= fnnl_qt->tnr.d_ask_hdof_prc)
			{
				fnnl_qt->tnr.d_ask_hdof_prc = d_hdof_prc3;
				memcpy(&usd_qt, usd_qt2, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt1, sizeof(HDQOUT_ST));
			}	

			if (d_hdof_prc4 >= fnnl_qt->tnr.d_ask_hdof_prc)
			{
				fnnl_qt->tnr.d_ask_hdof_prc = d_hdof_prc4;
				memcpy(&usd_qt, usd_qt2, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt2, sizeof(HDQOUT_ST));
			}

			if (usd_qt.pair.s_ask_orgn[0] == 'S')
				sprintf(splt_qt->s_hoga_key, "%s%s%s%s", "SMB", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else if (usd_qt.pair.s_ask_orgn[0] == 'K')
				sprintf(splt_qt->s_hoga_key, "%s%s%s%s", "KMB", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else if (usd_qt.pair.s_ask_orgn[0] == 'E')
				sprintf(splt_qt->s_hoga_key, "%s%s%s%s", "EBS", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else if (usd_qt.pair.s_ask_orgn[0] == 'C')
				sprintf(splt_qt->s_hoga_key, "%s%s%s%s", "CMB", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else 
				sprintf(splt_qt->s_hoga_key, "%s%s%s%s", "XXX", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			
			splt_qt->rec[0].n_seq = 0;
			splt_qt->rec[0].n_seq = 1;
			splt_qt->rec[0].n_seq = 2;

			splt_qt->rec[0].s_bysel_dcd[0] = 'B';
			splt_qt->rec[1].s_bysel_dcd[0] = 'B';
			splt_qt->rec[2].s_bysel_dcd[0] = 'S';
			
			// 재정통화 ASK SPOT가격
			splt_qt->rec[0].d_spt_prc    = l_round_d(usd_qt.pair.d_ask_spt_prc / std_qt.pair.d_bid_spt_prc * PX_UNIT, N_DGT);
			splt_qt->rec[1].d_spt_prc 	 = usd_qt.pair.d_ask_spt_prc ;
			splt_qt->rec[2].d_spt_prc    = std_qt.pair.d_bid_spt_prc ;

			// 시장가격
			splt_qt->rec[0].d_mrkt_prc   = usd_qt.tnr.d_ask_mrkt_prc / std_qt.tnr.d_bid_mrkt_prc * PX_UNIT;
			splt_qt->rec[1].d_mrkt_prc   = usd_qt.tnr.d_ask_mrkt_prc ;
			splt_qt->rec[2].d_mrkt_prc   = std_qt.tnr.d_bid_mrkt_prc ;
			
			// 본점가격
			splt_qt->rec[0].d_hdof_prc   = usd_qt.tnr.d_ask_hdof_prc / std_qt.tnr.d_bid_hdof_prc * PX_UNIT;
			splt_qt->rec[1].d_hdof_prc   = usd_qt.tnr.d_ask_hdof_prc ;
			splt_qt->rec[2].d_hdof_prc   = std_qt.tnr.d_bid_hdof_prc ;
			
			// 본점마진
			d_hdom                       = l_round_d(splt_qt->rec[0].d_hdof_prc - splt_qt->rec[0].d_mrkt_prc, 6);
			d_hdom_prc                   = splt_qt->rec[0].d_spt_prc + d_hdom ;

			// ASK(매입) 선물환 Cover마진
			// Cover마진 소수점 확인 할 것
			// Cover마진 가격 - BID SPOT 가격
			splt_qt->rec[0].d_covr_sprd  = l_round_d(usd_qt.tnr.d_ask_cvmg_prc / std_qt.tnr.d_bid_cvmg_prc * PX_UNIT, N_DGT) - splt_qt->rec[0].d_spt_prc ;
			splt_qt->rec[1].d_covr_sprd  = usd_qt.tnr.d_ask_spt_cvmg + usd_qt.tnr.d_ask_fwd_cvmg ;
			splt_qt->rec[2].d_covr_sprd  = std_qt.tnr.d_bid_spt_cvmg + std_qt.tnr.d_bid_fwd_cvmg ;
			
			// ASK(매입) 선물환 Corp마진
			splt_qt->rec[0].d_corp_sprd  = d_hdom - splt_qt->rec[0].d_covr_sprd;
			splt_qt->rec[1].d_corp_sprd  = usd_qt.tnr.d_ask_spt_cpmg + usd_qt.tnr.d_ask_fwd_cpmg ;
			splt_qt->rec[2].d_corp_sprd  = std_qt.tnr.d_bid_spt_cpmg + std_qt.tnr.d_bid_fwd_cpmg ;
			
			
			// SWAP 포인트
			splt_qt->rec[0].d_swap_pnt   = (l_round_d(splt_qt->rec[0].d_hdof_prc, 4) - l_round_d(d_hdom_prc, 4)) * 100.0;
			splt_qt->rec[1].d_swap_pnt   = usd_qt.tnr.d_ask_swap_pnt ;
			splt_qt->rec[2].d_swap_pnt   = std_qt.tnr.d_bid_swap_pnt ;
	
        
	    	// SWAP 포인트 계산 후 round 처리
	    	l_round_x(&splt_qt->rec[0].d_mrkt_prc, PX_UNIT);
			l_round_x(&splt_qt->rec[0].d_hdof_prc, PX_UNIT);

			// 매도 마진 (-) 처리
			// USD기준통화
			splt_qt->rec[2].d_covr_sprd = splt_qt->rec[0].d_covr_sprd * -1.0 ;
			splt_qt->rec[2].d_corp_sprd = splt_qt->rec[0].d_corp_sprd * -1.0 ;
			
		}
		// 곱하기
		else 
		{
			d_hdof_prc1 = usd_qt1->tnr.d_ask_hdof_prc * std_qt1->tnr.d_ask_hdof_prc;
			d_hdof_prc2 = usd_qt1->tnr.d_ask_hdof_prc * std_qt2->tnr.d_ask_hdof_prc;
			d_hdof_prc3 = usd_qt2->tnr.d_ask_hdof_prc * std_qt1->tnr.d_ask_hdof_prc;
			d_hdof_prc4 = usd_qt2->tnr.d_ask_hdof_prc * std_qt2->tnr.d_ask_hdof_prc;
			
			fnnl_qt->tnr.d_ask_hdof_prc = 0.0;
			if (d_hdof_prc1 >= fnnl_qt->tnr.d_ask_hdof_prc)
			{
				fnnl_qt->tnr.d_ask_hdof_prc = d_hdof_prc1;
				memcpy(&usd_qt, usd_qt1, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt1, sizeof(HDQOUT_ST));
			}	

			if (d_hdof_prc2 >= fnnl_qt->tnr.d_ask_hdof_prc)
			{
				fnnl_qt->tnr.d_ask_hdof_prc = d_hdof_prc2;
				memcpy(&usd_qt, usd_qt1, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt2, sizeof(HDQOUT_ST));
			}	
			
			if (d_hdof_prc3 >= fnnl_qt->tnr.d_ask_hdof_prc)
			{
				fnnl_qt->tnr.d_ask_hdof_prc = d_hdof_prc3;
				memcpy(&usd_qt, usd_qt2, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt1, sizeof(HDQOUT_ST));
			}	

			if (d_hdof_prc4 >= fnnl_qt->tnr.d_ask_hdof_prc)
			{
				fnnl_qt->tnr.d_ask_hdof_prc = d_hdof_prc4;
				memcpy(&usd_qt, usd_qt2, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt2, sizeof(HDQOUT_ST));
			}	
			
			if (usd_qt.pair.s_ask_orgn[0] == 'S')
				sprintf(splt_qt->s_hoga_key, "%s%s%s%s", "SMB", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else if (usd_qt.pair.s_ask_orgn[0] == 'K')
				sprintf(splt_qt->s_hoga_key, "%s%s%s%s", "KMB", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else if (usd_qt.pair.s_ask_orgn[0] == 'E')
				sprintf(splt_qt->s_hoga_key, "%s%s%s%s", "EBS", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else if (usd_qt.pair.s_ask_orgn[0] == 'C')
				sprintf(splt_qt->s_hoga_key, "%s%s%s%s", "CMB", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else 
				sprintf(splt_qt->s_hoga_key, "%s%s%s%s", "XXX", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			
			splt_qt->rec[0].n_seq = 0;
			splt_qt->rec[0].n_seq = 1;
			splt_qt->rec[0].n_seq = 2;
			
			splt_qt->rec[0].s_bysel_dcd[0] = 'B';
			splt_qt->rec[1].s_bysel_dcd[0] = 'B';
			splt_qt->rec[2].s_bysel_dcd[0] = 'B';

			// 재정통화 ASK SPOT가격
			splt_qt->rec[0].d_spt_prc    = l_round_d(usd_qt.pair.d_ask_spt_prc * std_qt.pair.d_ask_spt_prc, N_DGT) ;
			splt_qt->rec[1].d_spt_prc 	 = usd_qt.pair.d_ask_spt_prc ;
			splt_qt->rec[2].d_spt_prc    = std_qt.pair.d_ask_spt_prc ;

			// 시장가격
			splt_qt->rec[0].d_mrkt_prc   = usd_qt.tnr.d_ask_mrkt_prc * std_qt.tnr.d_ask_mrkt_prc ;
			splt_qt->rec[1].d_mrkt_prc   = usd_qt.tnr.d_ask_mrkt_prc ;
			splt_qt->rec[2].d_mrkt_prc   = std_qt.tnr.d_ask_mrkt_prc ;
			
			// 본점가격
			splt_qt->rec[0].d_hdof_prc   = usd_qt.tnr.d_ask_hdof_prc * std_qt.tnr.d_ask_hdof_prc ;
			splt_qt->rec[1].d_hdof_prc   = usd_qt.tnr.d_ask_hdof_prc ;
			splt_qt->rec[2].d_hdof_prc   = std_qt.tnr.d_ask_hdof_prc ;
			
			// 본점마진
			d_hdom                       = l_round_d(splt_qt->rec[0].d_hdof_prc - splt_qt->rec[0].d_mrkt_prc, 6);
			d_hdom_prc                   = splt_qt->rec[0].d_spt_prc + d_hdom ;

			// ASK(매입) 선물환 Cover마진
			// Cover마진 소수점 확인 할 것
			splt_qt->rec[0].d_covr_sprd  = l_round_d(usd_qt.tnr.d_ask_cvmg_prc * std_qt.tnr.d_ask_cvmg_prc, N_DGT) - splt_qt->rec[0].d_spt_prc ;
			splt_qt->rec[1].d_covr_sprd  = usd_qt.tnr.d_ask_spt_cvmg + usd_qt.tnr.d_ask_fwd_cvmg ;
			splt_qt->rec[2].d_covr_sprd  = std_qt.tnr.d_ask_spt_cvmg + std_qt.tnr.d_ask_fwd_cvmg ;
			
			// ASK(매입) 선물환 Corp마진
			splt_qt->rec[0].d_corp_sprd  = d_hdom - splt_qt->rec[0].d_covr_sprd;
			splt_qt->rec[1].d_corp_sprd  = usd_qt.tnr.d_ask_spt_cpmg + usd_qt.tnr.d_ask_fwd_cpmg ;
			splt_qt->rec[2].d_corp_sprd  = std_qt.tnr.d_ask_spt_cpmg + std_qt.tnr.d_ask_fwd_cpmg ;
			
			
			// SWAP 포인트
			splt_qt->rec[0].d_swap_pnt   = (l_round_d(splt_qt->rec[0].d_hdof_prc, 4) - l_round_d(d_hdom_prc, 4)) * 100.0;
			splt_qt->rec[1].d_swap_pnt   = usd_qt.tnr.d_ask_swap_pnt ;
			splt_qt->rec[2].d_swap_pnt   = std_qt.tnr.d_bid_swap_pnt ;
	
        
			// SWAP 포인트 계산 후 round 처리
			l_round_x(&splt_qt->rec[0].d_mrkt_prc, PX_UNIT);
			l_round_x(&splt_qt->rec[0].d_hdof_prc, PX_UNIT);
			
		}	

	}
	// 매도
	else 
	{
		// DIV
		if (fnnl_qt->pair.s_clc_dsnc[0] == '2')
		{
		
			d_hdof_prc1 = usd_qt1->tnr.d_bid_hdof_prc / std_qt1->tnr.d_ask_hdof_prc * PX_UNIT;
			d_hdof_prc2 = usd_qt1->tnr.d_bid_hdof_prc / std_qt2->tnr.d_ask_hdof_prc * PX_UNIT;
			d_hdof_prc3 = usd_qt2->tnr.d_bid_hdof_prc / std_qt1->tnr.d_ask_hdof_prc * PX_UNIT;
			d_hdof_prc4 = usd_qt2->tnr.d_bid_hdof_prc / std_qt2->tnr.d_ask_hdof_prc * PX_UNIT;
			
			fnnl_qt->tnr.d_bid_hdof_prc = 999999.0;
			if (d_hdof_prc1 <= fnnl_qt->tnr.d_bid_hdof_prc)
			{
				fnnl_qt->tnr.d_bid_hdof_prc = d_hdof_prc1;
				memcpy(&usd_qt, usd_qt1, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt1, sizeof(HDQOUT_ST));
				
			}	
			if (d_hdof_prc2 <= fnnl_qt->tnr.d_bid_hdof_prc)
			{
				fnnl_qt->tnr.d_bid_hdof_prc = d_hdof_prc2;
				memcpy(&usd_qt, usd_qt1, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt2, sizeof(HDQOUT_ST));
				
			}	
			if (d_hdof_prc3 <= fnnl_qt->tnr.d_bid_hdof_prc)
			{
				fnnl_qt->tnr.d_bid_hdof_prc = d_hdof_prc3;
				memcpy(&usd_qt, usd_qt2, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt1, sizeof(HDQOUT_ST));
				
			}	
			if (d_hdof_prc4 <= fnnl_qt->tnr.d_bid_hdof_prc)
			{
				fnnl_qt->tnr.d_bid_hdof_prc = d_hdof_prc4;
				memcpy(&usd_qt, usd_qt2, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt2, sizeof(HDQOUT_ST));
				
			}
			
			if (usd_qt.pair.s_bid_orgn[0] == 'S')
				sprintf(splt_qt->s_hoga_key, "%s%s%s%s", "SMB", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else if (usd_qt.pair.s_bid_orgn[0] == 'K')
				sprintf(splt_qt->s_hoga_key, "%s%s%s", "KMB", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else if (usd_qt.pair.s_bid_orgn[0] == 'E')
				sprintf(splt_qt->s_hoga_key, "%s%s%s", "EBS", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else if (usd_qt.pair.s_bid_orgn[0] == 'C')
				sprintf(splt_qt->s_hoga_key, "%s%s%s", "CMB", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else 
				sprintf(splt_qt->s_hoga_key, "%s%s%s", "XXX", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			
			splt_qt->rec[0].n_seq = 0;
			splt_qt->rec[0].n_seq = 1;
			splt_qt->rec[0].n_seq = 2;
			
			splt_qt->rec[0].s_bysel_dcd[0] = 'S';
			splt_qt->rec[1].s_bysel_dcd[0] = 'S';
			splt_qt->rec[2].s_bysel_dcd[0] = 'B';

			// 재정통화 ASK SPOT가격
			splt_qt->rec[0].d_spt_prc    = l_round_d(usd_qt.pair.d_bid_spt_prc / std_qt.pair.d_ask_spt_prc * PX_UNIT, N_DGT);
			splt_qt->rec[1].d_spt_prc    = usd_qt.pair.d_bid_spt_prc ;
			splt_qt->rec[2].d_spt_prc    = std_qt.pair.d_ask_spt_prc ;

			// 시장가격
			splt_qt->rec[0].d_mrkt_prc   = usd_qt.tnr.d_bid_mrkt_prc / std_qt.tnr.d_ask_mrkt_prc * PX_UNIT;
			splt_qt->rec[1].d_mrkt_prc   = usd_qt.tnr.d_bid_mrkt_prc ;
			splt_qt->rec[2].d_mrkt_prc   = std_qt.tnr.d_ask_mrkt_prc ;
			
			// 본점가격
			splt_qt->rec[0].d_hdof_prc   = usd_qt.tnr.d_bid_hdof_prc / std_qt.tnr.d_ask_hdof_prc * PX_UNIT;
			splt_qt->rec[1].d_hdof_prc   = usd_qt.tnr.d_bid_hdof_prc ;
			splt_qt->rec[2].d_hdof_prc   = std_qt.tnr.d_ask_hdof_prc ;
			
			// 본점마진
			// 시장가격 - 본점가격
			d_hdom                       = l_round_d(splt_qt->rec[0].d_mrkt_prc - splt_qt->rec[0].d_hdof_prc, 6);
			d_hdom_prc                   = splt_qt->rec[0].d_spt_prc - d_hdom ;

			// BID(매도) 선물환 Cover마진
			// Cover마진 소수점 확인 할 것
			// SPOT 가격 - Cover마진 가격
			splt_qt->rec[0].d_covr_sprd  = splt_qt->rec[0].d_spt_prc - l_round_d(usd_qt.tnr.d_bid_cvmg_prc / std_qt.tnr.d_ask_cvmg_prc * PX_UNIT, N_DGT);
			splt_qt->rec[1].d_covr_sprd  = usd_qt.tnr.d_bid_spt_cvmg + usd_qt.tnr.d_bid_fwd_cvmg ;
			splt_qt->rec[2].d_covr_sprd  = std_qt.tnr.d_ask_spt_cvmg + std_qt.tnr.d_ask_fwd_cvmg ;
			
			// BID(매도) 선물환 Corp마진
			splt_qt->rec[0].d_corp_sprd  = d_hdom - splt_qt->rec[0].d_covr_sprd;
			splt_qt->rec[1].d_corp_sprd  = usd_qt.tnr.d_bid_spt_cpmg + usd_qt.tnr.d_bid_fwd_cpmg ;
			splt_qt->rec[2].d_corp_sprd  = std_qt.tnr.d_ask_spt_cpmg + std_qt.tnr.d_ask_fwd_cpmg ;
			
			
			// SWAP 포인트
			splt_qt->rec[0].d_swap_pnt   = (l_round_d(splt_qt->rec[0].d_hdof_prc, 4) - l_round_d(d_hdom_prc, 4)) * 100.0;
			splt_qt->rec[1].d_swap_pnt   = usd_qt.tnr.d_bid_swap_pnt ;
			splt_qt->rec[2].d_swap_pnt   = std_qt.tnr.d_ask_swap_pnt ;
	
        
			// SWAP 포인트 계산 후 round 처리
			l_round_x(&splt_qt->rec[0].d_mrkt_prc, PX_UNIT);
			l_round_x(&splt_qt->rec[0].d_hdof_prc, PX_UNIT);
			
			// 매도 마진 (-) 처리
			// 재정통화
			splt_qt->rec[0].d_covr_sprd = splt_qt->rec[0].d_covr_sprd * -1.0 ;
			splt_qt->rec[0].d_corp_sprd = splt_qt->rec[0].d_corp_sprd * -1.0 ;

			// USDKRW
			splt_qt->rec[1].d_covr_sprd = splt_qt->rec[0].d_covr_sprd * -1.0 ;
			splt_qt->rec[1].d_corp_sprd = splt_qt->rec[0].d_corp_sprd * -1.0 ;
		}
		// MUL
		else 
		{
			d_hdof_prc1 = usd_qt1->tnr.d_bid_hdof_prc * std_qt1->tnr.d_bid_hdof_prc;
			d_hdof_prc2 = usd_qt1->tnr.d_bid_hdof_prc * std_qt2->tnr.d_bid_hdof_prc;
			d_hdof_prc3 = usd_qt2->tnr.d_bid_hdof_prc * std_qt1->tnr.d_bid_hdof_prc;
			d_hdof_prc4 = usd_qt2->tnr.d_bid_hdof_prc * std_qt2->tnr.d_bid_hdof_prc;

			fnnl_qt->tnr.d_bid_hdof_prc = 99999999.0;
			if (d_hdof_prc1 <= fnnl_qt->tnr.d_bid_hdof_prc)
			{
				fnnl_qt->tnr.d_bid_hdof_prc = d_hdof_prc1;
				memcpy(&usd_qt, usd_qt1, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt1, sizeof(HDQOUT_ST));
				
			}	
			if (d_hdof_prc2 <= fnnl_qt->tnr.d_bid_hdof_prc)
			{
				fnnl_qt->tnr.d_bid_hdof_prc = d_hdof_prc2;
				memcpy(&usd_qt, usd_qt1, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt2, sizeof(HDQOUT_ST));
				
			}	
			if (d_hdof_prc3 <= fnnl_qt->tnr.d_bid_hdof_prc)
			{
				fnnl_qt->tnr.d_bid_hdof_prc = d_hdof_prc3;
				memcpy(&usd_qt, usd_qt2, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt1, sizeof(HDQOUT_ST));
				
			}
			if (d_hdof_prc4 <= fnnl_qt->tnr.d_bid_hdof_prc)
			{
				fnnl_qt->tnr.d_bid_hdof_prc = d_hdof_prc4;
				memcpy(&usd_qt, usd_qt2, sizeof(HDQOUT_ST));
				memcpy(&std_qt, std_qt2, sizeof(HDQOUT_ST));
				
			}
			if (usd_qt.pair.s_bid_orgn[0] == 'S')
				sprintf(splt_qt->s_hoga_key, "%s%s%s%s", "SMB", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else if (usd_qt.pair.s_bid_orgn[0] == 'K')
				sprintf(splt_qt->s_hoga_key, "%s%s%s", "KMB", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else if (usd_qt.pair.s_bid_orgn[0] == 'E')
				sprintf(splt_qt->s_hoga_key, "%s%s%s", "EBS", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else if (usd_qt.pair.s_bid_orgn[0] == 'C')
				sprintf(splt_qt->s_hoga_key, "%s%s%s", "CMB", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			else 
				sprintf(splt_qt->s_hoga_key, "%s%s%s", "XXX", fnnl_qt->pair.s_pair_id, usd_qt.pair.s_rcv_ymd, usd_qt.pair.s_rcv_hms);
			
			splt_qt->rec[0].n_seq = 0;
			splt_qt->rec[0].n_seq = 1;
			splt_qt->rec[0].n_seq = 2;

			splt_qt->rec[0].s_bysel_dcd[0] = 'S';
			splt_qt->rec[1].s_bysel_dcd[0] = 'S';
			splt_qt->rec[2].s_bysel_dcd[0] = 'S';

			// 재정통화 BID SPOT가격
			splt_qt->rec[0].d_spt_prc    = l_round_d(usd_qt.pair.d_bid_spt_prc * std_qt.pair.d_bid_spt_prc, N_DGT) ;
			splt_qt->rec[1].d_spt_prc    = usd_qt.pair.d_bid_spt_prc ;
			splt_qt->rec[2].d_spt_prc    = std_qt.pair.d_bid_spt_prc ;

			// 시장가격
			splt_qt->rec[0].d_mrkt_prc   = usd_qt.tnr.d_bid_mrkt_prc * std_qt.tnr.d_bid_mrkt_prc ;
			splt_qt->rec[1].d_mrkt_prc   = usd_qt.tnr.d_bid_mrkt_prc ;
			splt_qt->rec[2].d_mrkt_prc   = std_qt.tnr.d_bid_mrkt_prc ;
			
			// 본점가격
			splt_qt->rec[0].d_hdof_prc   = usd_qt.tnr.d_bid_hdof_prc * std_qt.tnr.d_bid_hdof_prc ;
			splt_qt->rec[1].d_hdof_prc   = usd_qt.tnr.d_bid_hdof_prc ;
			splt_qt->rec[2].d_hdof_prc   = std_qt.tnr.d_bid_hdof_prc ;
			
			// 본점마진
			// 본점가격 - 시장가격
			d_hdom                       = l_round_d(splt_qt->rec[0].d_mrkt_prc - splt_qt->rec[0].d_hdof_prc, 6);
			d_hdom_prc                   = splt_qt->rec[0].d_spt_prc - d_hdom ;

			// BID(매도) 선물환 Cover마진
			// Cover마진 소수점 확인 할 것
			// SPOT 가격 - Cover마진 가격
			splt_qt->rec[0].d_covr_sprd  = splt_qt->rec[0].d_spt_prc - l_round_d(usd_qt.tnr.d_bid_cvmg_prc * std_qt.tnr.d_bid_cvmg_prc, N_DGT) ;
			splt_qt->rec[1].d_covr_sprd  = usd_qt.tnr.d_bid_spt_cvmg + usd_qt.tnr.d_bid_fwd_cvmg ;
			splt_qt->rec[2].d_covr_sprd  = std_qt.tnr.d_bid_spt_cvmg + std_qt.tnr.d_bid_fwd_cvmg ;
			
			// BID(매도) 선물환 Corp마진
			splt_qt->rec[0].d_corp_sprd  = d_hdom - splt_qt->rec[0].d_covr_sprd;
			splt_qt->rec[1].d_corp_sprd  = usd_qt.tnr.d_bid_spt_cpmg + usd_qt.tnr.d_bid_fwd_cpmg ;
			splt_qt->rec[2].d_corp_sprd  = std_qt.tnr.d_bid_spt_cpmg + std_qt.tnr.d_bid_fwd_cpmg ;
			
			
			// SWAP 포인트
			splt_qt->rec[0].d_swap_pnt   = (l_round_d(splt_qt->rec[0].d_hdof_prc, 4) - l_round_d(d_hdom_prc, 4)) * 100.0;
			splt_qt->rec[1].d_swap_pnt   = usd_qt.tnr.d_bid_swap_pnt ;
			splt_qt->rec[2].d_swap_pnt   = std_qt.tnr.d_bid_swap_pnt ;
	
        
			// SWAP 포인트 계산 후 round 처리
			l_round_x(&splt_qt->rec[0].d_mrkt_prc, PX_UNIT);
			l_round_x(&splt_qt->rec[0].d_hdof_prc, PX_UNIT);
			
			// 매도 마진 (-) 처리
			splt_qt->rec[0].d_covr_sprd  = splt_qt->rec[0].d_covr_sprd * -1.0 ;
			splt_qt->rec[0].d_corp_sprd  = splt_qt->rec[0].d_corp_sprd * -1.0 ;

			splt_qt->rec[1].d_covr_sprd  = splt_qt->rec[0].d_covr_sprd * -1.0 ;
			splt_qt->rec[1].d_corp_sprd  = splt_qt->rec[0].d_corp_sprd * -1.0 ;
			
			splt_qt->rec[2].d_covr_sprd  = splt_qt->rec[0].d_covr_sprd * -1.0 ;
			splt_qt->rec[2].d_corp_sprd  = splt_qt->rec[0].d_corp_sprd * -1.0 ;
		}	
		
	}
	
	return (0);
}		

// USDKRW, 비재정 FWD 만기선택 
// 

int l_calc_usd_expi_sclt(s_bysel_dcd, fwd_qt1, fwd_qt2, splt_qt, s_errcd)
char          *s_bysel_dcd;
HDQOUT_ST     *fwd_qt1;
HDQOUT_ST     *fwd_qt2;
SPLIT_OUT_ST  *splt_qt;
char          *s_errcd;
{
	int     N_DGT   ;
	double  PX_UNIT ;

	// JPYKRW 100.0 나머지 1.0
	PX_UNIT  = fwd_qt1->pair.d_clc_unit;
	N_DGT    = fwd_qt1->pair.n_digit;

	memcpy(splt_qt->rec[0].s_crnc_pair_id, fwd_qt1->pair.s_pair_id, sizeof(splt_qt->rec[0].s_crnc_pair_id)-1);
	
	splt_qt->rec[0].s_bysel_dcd[0] = s_bysel_dcd[0];

	// 매입
	if (s_bysel_dcd[0] == 'B')
	{

		if (fwd_qt1->tnr.d_ask_hdof_prc > fwd_qt2->tnr.d_ask_hdof_prc)
		{
			memcpy(splt_qt->rec[0].s_sldy, fwd_qt1->tnr.s_sldy, sizeof(splt_qt->rec[0].s_sldy)-1);
			 
			splt_qt->rec[0].d_spt_prc      = fwd_qt1->pair.d_ask_spt_prc     ;  // ASK SPOT 가격

			splt_qt->rec[0].d_swap_pnt     = fwd_qt1->tnr.d_ask_swap_pnt     ;  // ASK SWAP 포인트
			splt_qt->rec[0].d_mrkt_prc     = fwd_qt1->tnr.d_ask_mrkt_prc     ;  // ASK 시장가격 
			
			splt_qt->rec[0].d_covr_sprd    = fwd_qt1->tnr.d_ask_fwd_cvmg     ;  // ASK 선물환 Cover딜러 마진
			splt_qt->rec[0].d_corp_sprd    = fwd_qt1->tnr.d_ask_fwd_cpmg     ;  // ASK 선물환 Corp딜러 마진
			splt_qt->rec[0].d_hdof_prc     = fwd_qt1->tnr.d_ask_hdof_prc     ;  // ASK 본점 가격 
		}
		else 
		{
			memcpy(splt_qt->rec[0].s_sldy, fwd_qt2->tnr.s_sldy, sizeof(splt_qt->rec[0].s_sldy)-1);

			splt_qt->rec[0].d_spt_prc      = fwd_qt2->pair.d_ask_spt_prc     ;  // ASK SPOT 가격
			
			splt_qt->rec[0].d_swap_pnt     = fwd_qt2->tnr.d_ask_swap_pnt     ;  // ASK SWAP 포인트
			splt_qt->rec[0].d_mrkt_prc     = fwd_qt2->tnr.d_ask_mrkt_prc     ;  // ASK 시장가격

			splt_qt->rec[0].d_covr_sprd    = fwd_qt2->tnr.d_ask_fwd_cvmg     ;  // ASK 선물환 Cover딜러 마진
			splt_qt->rec[0].d_corp_sprd    = fwd_qt2->tnr.d_ask_fwd_cpmg     ;  // ASK 선물환 Corp딜러 마진
			splt_qt->rec[0].d_hdof_prc     = fwd_qt2->tnr.d_ask_hdof_prc     ;  // ASK 본점 가격 
		}
	}
	// 매도
	else 
	{
		if (fwd_qt1->tnr.d_bid_hdof_prc > fwd_qt2->tnr.d_bid_hdof_prc)
		{
			memcpy(splt_qt->rec[0].s_sldy, fwd_qt1->tnr.s_sldy, sizeof(splt_qt->rec[0].s_sldy)-1);

			splt_qt->rec[0].d_spt_prc      = fwd_qt1->pair.d_bid_spt_prc         ;  // BID SPOT 가격
			
			splt_qt->rec[0].d_swap_pnt     = fwd_qt1->tnr.d_bid_swap_pnt         ;  // BID SWAP 포인트
			splt_qt->rec[0].d_mrkt_prc     = fwd_qt1->tnr.d_bid_mrkt_prc         ;  // BID 시장가격 

			splt_qt->rec[0].d_covr_sprd    = fwd_qt1->tnr.d_bid_fwd_cvmg * -1.0  ;  // BID 선물환 Cover딜러 마진
			splt_qt->rec[0].d_corp_sprd    = fwd_qt1->tnr.d_bid_fwd_cpmg * -1.0  ;  // BID 선물환 Corp딜러 마진
			splt_qt->rec[0].d_hdof_prc     = fwd_qt1->tnr.d_bid_hdof_prc         ;  // BID 본점 가격 
		}
		else 
		{
			memcpy(splt_qt->rec[0].s_sldy, fwd_qt2->tnr.s_sldy, sizeof(splt_qt->rec[0].s_sldy)-1);

			splt_qt->rec[0].d_spt_prc      = fwd_qt2->pair.d_bid_spt_prc          ;  // BID SPOT 가격
			
			splt_qt->rec[0].d_swap_pnt     = fwd_qt2->tnr.d_bid_swap_pnt          ;  // BID SWAP 포인트
			splt_qt->rec[0].d_mrkt_prc     = fwd_qt2->tnr.d_bid_mrkt_prc          ;  // BID 시장가격

			splt_qt->rec[0].d_covr_sprd    = fwd_qt2->tnr.d_bid_fwd_cvmg * -1.0   ;  // BID 선물환 Cover딜러 마진
			splt_qt->rec[0].d_corp_sprd    = fwd_qt2->tnr.d_bid_fwd_cpmg * -1.0   ;  // BID 선물환 Corp딜러 마진
			splt_qt->rec[0].d_hdof_prc     = fwd_qt2->tnr.d_bid_hdof_prc          ;  // BID 본점 가격 
		}
		
	}
	return (0);	
}	
