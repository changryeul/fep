#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"

#include "mrg.h"
#include "proc.h"

int f_hdomgrp_init(HDOMGRP_SHM_ST *p_hdomgrp, char *s_errcd);
int f_hdomgrp_prt(HDOMGRP_SHM_ST *p_hdomgrp, char *s_errcd);

int ShmProcess( MRG *mrg)
{

	int rtn = 0;
	char s_errcd[6+1];

	HDOMGRP_SHM_ST *p_hdomgrp_shm;

	


#if 0
	LogDel("shm remove");
	rtn = f_remove_hdomgrp(s_errcd);
#endif

#if 0
	LogDel("shm 생성");
	rtn = f_create_hdomgrp(p_hdomgrp_shm, s_errcd);
	if (rtn < 0)
	{
		rtn = f_remove_hdomgrp(s_errcd);
		exit(0);
	}
/*
	else
	{
		rtn = f_wget_hdomgrp(p_hdomgrp_shm, s_errcd);
		if (rtn < 0)
		{
			rtn = f_remove_hdomgrp(s_errcd);
			exit(0);
		}
	}
*/
#endif
	LogDel("size [%d]", HDOMGRP_SHM_ST_SZ);
	p_hdomgrp_shm->n_grp_cnt = 0;
	LogDel("본점마진그룹 그룹 count[%d]", p_hdomgrp_shm->n_grp_cnt);
	

//	memset(p_hdomgrp_shm->test, 0x00, HDOMGRP_SHM_ST_SZ);
	p_hdomgrp_shm = ( HDOMGRP_SHM_ST *)mrg->map;
	f_hdomgrp_prt(p_hdomgrp_shm, s_errcd);

	f_hdomgrp_init(p_hdomgrp_shm, s_errcd);

	LogDel("본점마진그룹 그룹 count[%d]", p_hdomgrp_shm->n_grp_cnt);


	f_hdomgrp_prt(p_hdomgrp_shm, s_errcd);


#if 0
	rtn = f_remove_hdomgrp(s_errcd);
	exit(0);
#endif
}

int f_hdomgrp_prt(p_hdomgrp, s_errcd)
HDOMGRP_SHM_ST *p_hdomgrp;
char           *s_errcd;
{
	int i = 0;
	int j = 0;
	int k = 0;

	FXPAIR_ST *pair;
	FXTNR_ST  *tnr;

	LogDel("group count[%d]", p_hdomgrp->n_grp_cnt);

	while(i < p_hdomgrp->n_grp_cnt)
	{
		LogDel("[%s][%d]", p_hdomgrp->grp[i].s_hdom_grp_id, p_hdomgrp->grp[i].n_pair_cnt);

		j = 0;
		while (j < p_hdomgrp->grp[i].n_pair_cnt)
		{
			pair = (FXPAIR_ST *)&p_hdomgrp->grp[i].pair[j];


			LogRaw("        [%s][%s][%d][%.4f][%3f][%.2f][%.2f]\n", pair->s_pair_id, pair->s_clc_dsnc, pair->n_digit, pair->d_digit_val, pair->d_clc_unit, pair->d_ask_spt_prc, pair->d_bid_spt_prc);
			k = 0;
			while(k < pair->n_tnr_cnt)
			{
				tnr = (FXTNR_ST *)&p_hdomgrp->grp[i].pair[j].tnr[k];
				if (pair->n_digit == 2)
					LogRaw("           [%s][%s][%3.0f][%8.2f][%8.02f][%8.2f][%8.02f][%8.02f][%8.02f][%8.02f][%8.02f][%8.02f][%8.02f][%8.02f][%8.02f]\n", 
							tnr->s_tnr_tcd, tnr->s_sldy, tnr->d_ndd, 
							tnr->d_ask_spt_cvmg + tnr->d_ask_fwd_cvmg,
							tnr->d_ask_hdom,
							tnr->d_ask_cvmg_prc,
							tnr->d_ask_hdom_prc,
							tnr->d_ask_swap_pnt,
							tnr->d_ask_hdof_prc,
							tnr->d_bid_spt_cvmg + tnr->d_bid_fwd_cvmg,
							tnr->d_bid_hdom,
							tnr->d_bid_cvmg_prc,
							tnr->d_bid_hdom_prc,
							tnr->d_bid_swap_pnt,
							tnr->d_bid_hdof_prc);
				if (pair->n_digit == 3)
					LogRaw("           [%s][%s][%3.0f][%8.03f][%8.03f][%8.3f][%8.03f][%8.03f][%8.03f][%8.03f][%8.03f][%8.03f][%8.03f][%8.03f][%8.03f]\n", 
							tnr->s_tnr_tcd, tnr->s_sldy, tnr->d_ndd, 
							tnr->d_ask_spt_cvmg + tnr->d_ask_fwd_cvmg,
							tnr->d_ask_hdom,
							tnr->d_ask_cvmg_prc,
							tnr->d_ask_hdom_prc,
							tnr->d_ask_swap_pnt,
							tnr->d_ask_hdof_prc,
							tnr->d_bid_spt_cvmg + tnr->d_bid_fwd_cvmg,
							tnr->d_bid_hdom,
							tnr->d_bid_cvmg_prc,
							tnr->d_bid_hdom_prc,
							tnr->d_bid_swap_pnt,
							tnr->d_bid_hdof_prc);
				if (pair->n_digit == 4)
					LogRaw("           [%s][%s][%3.0f][%8.04f][%8.04f][%8.4f][%8.04f][%8.04f][%8.04f][%8.04f][%8.04f][%8.04f][%8.04f][%8.04f][%8.04f]\n", 
							tnr->s_tnr_tcd, tnr->s_sldy, tnr->d_ndd, 
							tnr->d_ask_spt_cvmg + tnr->d_ask_fwd_cvmg,
							tnr->d_ask_hdom,
							tnr->d_ask_cvmg_prc,
							tnr->d_ask_hdom_prc,
							tnr->d_ask_swap_pnt,
							tnr->d_ask_hdof_prc,
							tnr->d_bid_spt_cvmg + tnr->d_bid_fwd_cvmg,
							tnr->d_bid_hdom,
							tnr->d_bid_cvmg_prc,
							tnr->d_bid_hdom_prc,
							tnr->d_bid_swap_pnt,
							tnr->d_bid_hdof_prc);
				k++;
			}
			j++;
		}
		i++;
	}
	return 0;
}


int f_hdomgrp_init(p_hdomgrp, s_errcd)
HDOMGRP_SHM_ST *p_hdomgrp;
char           *s_errcd;
{

	int i;

	FXGRP_ST  *fxgrp;
	FXPAIR_ST *fxpair;
	FXTNR_ST  *fxtnr;

	memset(p_hdomgrp->grp[0].s_hdom_grp_id, 0x00, sizeof(p_hdomgrp->grp[0].s_hdom_grp_id));

	LogDel("grp id [%s]", p_hdomgrp->grp[0].s_hdom_grp_id);

	fxgrp = (FXGRP_ST *)&p_hdomgrp->grp[0];

	
	memset(&p_hdomgrp->grp[0].s_hdom_grp_id, 0x00, sizeof(p_hdomgrp->grp[0].s_hdom_grp_id));

	memcpy(p_hdomgrp->grp[0].s_hdom_grp_id, "001", 3);

	p_hdomgrp->grp[0].n_pair_cnt = 0;


	LogDel("group id [%s]", p_hdomgrp->grp[0].s_hdom_grp_id);

	fxpair = (FXPAIR_ST *)&p_hdomgrp->grp[0].pair[0]; 

//	memset(fxpair, 0x00, FXPAIR_ST_SZ);

	LogDel("group id [%s]", fxgrp->s_hdom_grp_id);

	memcpy(fxpair->s_pair_id, "USDKRW", 6);

	LogDel("pair id [%s]", fxpair->s_pair_id);

	memset(p_hdomgrp->grp[0].pair[0].s_clc_dsnc, 0x00, 2);

	fxpair->s_clc_dsnc[0] = '1';
	fxpair->n_digit = 2;
	fxpair->d_digit_val = 0.01;
	fxpair->d_clc_unit  = 1.0;
	fxpair->d_ask_spt_prc  = 1328.5;
	fxpair->d_bid_spt_prc  = 1328.5;
	fxpair->d_ask_usd_spt_prc = 1328.5;
	fxpair->d_bid_usd_spt_prc = 1328.5;
	fxpair->s_ask_orgn[0] = 'X';
	fxpair->s_bid_orgn[0] = 'X';
	memcpy(fxpair->s_rcv_ymd, "20230919", 8);
	memcpy(fxpair->s_rcv_hms, "000000000", 9);

	fxpair->n_tnr_cnt = 0;
	for (i = 0; i < MAX_TNR_CNT; i++)
	{
//		fxtnr = (FXTNR_ST *)&fxpair->tnr[i];
		fxtnr = (FXTNR_ST *)&p_hdomgrp->grp[0].pair[0].tnr[i];
		

		memcpy(fxtnr->s_tnr_tcd, "TOD", 3);
		fxtnr->s_tnr_dsnc[0] = 'S';

		fxtnr->d_ask_spt_cvmg = 0.1;
		fxtnr->d_ask_spt_cpmg = 0.0;

		fxtnr->d_bid_spt_cvmg = 0.1;
		fxtnr->d_bid_spt_cpmg = 0.1;

		memset(fxtnr->s_tnr_tcd, 0x00, sizeof(fxtnr->s_tnr_tcd));
		memset(fxtnr->s_sldy, 0x00, sizeof(fxtnr->s_sldy));


		switch(i)
		{
		case 0: // TOD
			memcpy(fxtnr->s_tnr_tcd, "TOD", 3);
			memcpy(fxtnr->s_sldy, "20230919", 8);
			fxtnr->d_ndd = 0.0;
			fxtnr->d_ask_fwd_cvmg = 0.0;
			fxtnr->d_ask_fwd_cpmg = 0.0;
			fxtnr->d_ask_swap_pnt = 0.0;

			fxtnr->d_bid_fwd_cvmg = 0.0;
			fxtnr->d_bid_fwd_cpmg = 0.0;
			fxtnr->d_bid_swap_pnt = 0.0;
			break;
		case 1: // TOM
			memcpy(fxtnr->s_tnr_tcd, "TOM", 3);
			memcpy(fxtnr->s_sldy, "20230920", 8);
			fxtnr->d_ndd = 0.0;
			fxtnr->d_ask_fwd_cvmg = 0.0;
			fxtnr->d_ask_fwd_cpmg = 0.0;
			fxtnr->d_ask_swap_pnt = 0.0;

			fxtnr->d_bid_fwd_cvmg = 0.0;
			fxtnr->d_bid_fwd_cpmg = 0.0;
			fxtnr->d_bid_swap_pnt = 0.0;
			break;
		case 2:
			memcpy(fxtnr->s_tnr_tcd, "SPT", 3);
			memcpy(fxtnr->s_sldy, "20230921", 8);
			fxtnr->d_ndd = 0.0;
			fxtnr->d_ask_fwd_cvmg = 0.0;
			fxtnr->d_ask_fwd_cpmg = 0.0;
			fxtnr->d_ask_swap_pnt = 0.0;

			fxtnr->d_bid_fwd_cvmg = 0.0;
			fxtnr->d_bid_fwd_cpmg = 0.0;
			fxtnr->d_bid_swap_pnt = 0.0;
			break;
		case 3:
			memcpy(fxtnr->s_tnr_tcd, "W01", 3);
			memcpy(fxtnr->s_sldy, "20231004", 8);
			fxtnr->d_ndd = 13.0;
			fxtnr->d_ask_fwd_cvmg = 0.0;
			fxtnr->d_ask_fwd_cpmg = 0.6;
			fxtnr->d_ask_swap_pnt = -86.0;

			fxtnr->d_bid_fwd_cvmg = 0.0;
			fxtnr->d_bid_fwd_cpmg = 0.5;
			fxtnr->d_bid_swap_pnt = -96.0;
			break;
		case 4:
			memcpy(fxtnr->s_tnr_tcd, "M01", 3);
			memcpy(fxtnr->s_sldy, "20231023", 8);
			fxtnr->d_ndd = 32.0;
			fxtnr->d_ask_fwd_cvmg = 0.0;
			fxtnr->d_ask_fwd_cpmg = 1.1;
			fxtnr->d_ask_swap_pnt = -217.0;

			fxtnr->d_bid_fwd_cvmg = 0.0;
			fxtnr->d_bid_fwd_cpmg = 0.9;
			fxtnr->d_bid_swap_pnt = -263.0;
			break;
		case 5:
			memcpy(fxtnr->s_tnr_tcd, "M02", 3);
			memcpy(fxtnr->s_sldy, "20231121", 8);
			fxtnr->d_ndd = 61.0;
			fxtnr->d_ask_fwd_cvmg = 0.0;
			fxtnr->d_ask_fwd_cpmg = 1.2;
			fxtnr->d_ask_swap_pnt = -433.0;

			fxtnr->d_bid_fwd_cvmg = 0.0;
			fxtnr->d_bid_fwd_cpmg = 1.1;
			fxtnr->d_bid_swap_pnt = -497.0;
			break;
		case 6:
			memcpy(fxtnr->s_tnr_tcd, "M03", 3);
			memcpy(fxtnr->s_sldy, "20231221", 8);
			fxtnr->d_ndd = 91.0;
			fxtnr->d_ask_fwd_cvmg = 0.0;
			fxtnr->d_ask_fwd_cpmg = 1.6;
			fxtnr->d_ask_swap_pnt = -643.0;

			fxtnr->d_bid_fwd_cvmg = 0.0;
			fxtnr->d_bid_fwd_cpmg = 1.5;
			fxtnr->d_bid_swap_pnt = -727.0;
			break;
		case 7:
			memcpy(fxtnr->s_tnr_tcd, "M06", 3);
			memcpy(fxtnr->s_sldy, "20240321", 8);
			fxtnr->d_ndd = 162.0;
			fxtnr->d_ask_fwd_cvmg = 0.0;
			fxtnr->d_ask_fwd_cpmg = 2.1;
			fxtnr->d_ask_swap_pnt = -1423.0;

			fxtnr->d_bid_fwd_cvmg = 0.0;
			fxtnr->d_bid_fwd_cpmg = 2.0;
			fxtnr->d_bid_swap_pnt = -1554.0;
			break;
		case 8:
			memcpy(fxtnr->s_tnr_tcd, "Y01", 3);
			memcpy(fxtnr->s_sldy, "20240923", 8);
			fxtnr->d_ndd = 368.0;
			fxtnr->d_ask_fwd_cvmg = 0.0;
			fxtnr->d_ask_fwd_cpmg = 2.1;
			fxtnr->d_ask_swap_pnt = -2845.0;

			fxtnr->d_bid_fwd_cvmg = 0.0;
			fxtnr->d_bid_fwd_cpmg = 2.0;
			fxtnr->d_bid_swap_pnt = -3015.0;
			break;
		}
		LogDel("tnr_tcd [%s]  s_sldy[%s]", fxtnr->s_tnr_tcd, fxtnr->s_sldy);

		fxtnr->d_ask_cvmg_prc = 0.0;
		fxtnr->d_ask_hdom     = 0.0;
		fxtnr->d_ask_hdom_prc = 0.0;
		fxtnr->d_ask_mrkt_prc = 0.0;
		fxtnr->d_ask_hdof_prc = 0.0;

		fxtnr->d_bid_cvmg_prc = 0.0;
		fxtnr->d_bid_hdom     = 0.0;
		fxtnr->d_bid_hdom_prc = 0.0;
		fxtnr->d_bid_mrkt_prc = 0.0;
		fxtnr->d_bid_hdof_prc = 0.0;

		fxtnr->d_ask_cvmg_prc = fxpair->d_ask_spt_prc + fxtnr->d_ask_spt_cvmg + fxtnr->d_ask_fwd_cvmg;
		fxtnr->d_ask_hdom     = fxtnr->d_ask_spt_cvmg + fxtnr->d_ask_spt_cpmg + fxtnr->d_ask_fwd_cvmg + fxtnr->d_ask_fwd_cpmg;
		fxtnr->d_ask_hdom_prc = fxpair->d_ask_spt_prc + fxtnr->d_ask_hdom;
		fxtnr->d_ask_mrkt_prc = fxpair->d_ask_spt_prc + fxtnr->d_ask_swap_pnt / 100.0;
		fxtnr->d_ask_hdof_prc = fxtnr->d_ask_mrkt_prc + fxtnr->d_ask_hdom;

		fxtnr->d_bid_cvmg_prc = fxpair->d_bid_spt_prc - fxtnr->d_bid_spt_cvmg - fxtnr->d_bid_fwd_cvmg;
		fxtnr->d_bid_hdom     = fxtnr->d_bid_spt_cvmg + fxtnr->d_bid_spt_cpmg + fxtnr->d_bid_fwd_cvmg + fxtnr->d_bid_fwd_cpmg;
		fxtnr->d_bid_hdom_prc = fxpair->d_bid_spt_prc - fxtnr->d_bid_hdom;
		fxtnr->d_bid_mrkt_prc = fxpair->d_bid_spt_prc + fxtnr->d_bid_swap_pnt / 100.0;
		fxtnr->d_bid_hdof_prc = fxtnr->d_bid_mrkt_prc - fxtnr->d_bid_hdom;

		l_round_x(&fxtnr->d_ask_mrkt_prc, fxpair->n_digit);
		l_round_x(&fxtnr->d_ask_hdof_prc, fxpair->n_digit);

		l_round_x(&fxtnr->d_bid_mrkt_prc, fxpair->n_digit);
		l_round_x(&fxtnr->d_bid_hdof_prc, fxpair->n_digit);

		fxtnr->d_ask_pdcp = fxtnr->d_ask_hdof_prc;
		fxtnr->d_bid_pdcp = fxtnr->d_bid_hdof_prc;

		LogDel("테너 [%s] 적재 완료", fxtnr->s_tnr_tcd);

		fxpair->n_tnr_cnt++;
	}

	LogDel("[%s] tnr_cnt[%d]", fxpair->s_pair_id, fxpair->n_tnr_cnt);
	p_hdomgrp->grp[0].n_pair_cnt++;

//	fxpair = (FXPAIR_ST *)&fxgrp->pair[1];
	fxpair = (FXPAIR_ST *)&p_hdomgrp->grp[0].pair[1]; 

//	memset(fxpair, 0x00, FXPAIR_ST_SZ);


	memset(fxpair->s_pair_id, 0x00, sizeof(fxpair->s_pair_id));
	memcpy(fxpair->s_pair_id, "JPYKRW", 6);

	LogDel("pair id [%s]", fxpair->s_pair_id);

	fxpair->s_clc_dsnc[0] = '2';
	fxpair->n_digit = 2;
	fxpair->d_digit_val = 0.01;
	fxpair->d_clc_unit  = 100.0;
	fxpair->d_ask_spt_prc  = 898.61;
	fxpair->d_bid_spt_prc  = 898.3; 
	fxpair->d_ask_usd_spt_prc = 1328.5;
	fxpair->d_bid_usd_spt_prc = 1328.5;
	fxpair->s_ask_orgn[0] = 'X';
	fxpair->s_bid_orgn[0] = 'X';

	memcpy(fxpair->s_rcv_ymd, "20230919", 8);
	memcpy(fxpair->s_rcv_hms, "000000000", 9);


	fxpair->n_tnr_cnt = 0;
	for (i = 0; i < MAX_TNR_CNT; i++)
	{
//		fxtnr = (FXTNR_ST *)&fxpair->tnr[i];
		fxtnr = (FXTNR_ST *)&p_hdomgrp->grp[0].pair[1].tnr[i];

		LogDel( "AAAA for[%d] fxtnr=[%p]", i, fxtnr);

		memset(fxtnr->s_tnr_dsnc, 0x00, sizeof(fxtnr->s_tnr_dsnc));
		fxtnr->s_tnr_dsnc[0] = 'S';
		LogDel( "1 p=[%p]", fxtnr);

		fxtnr->d_ask_spt_cvmg = 0.25;
		LogDel( "2 p=[%p]", fxtnr);
		fxtnr->d_ask_spt_cpmg = 0.0;
		LogDel( "3 p=[%p]", fxtnr);

		fxtnr->d_bid_spt_cvmg = 0.25;
		LogDel( "4 p=[%p]", fxtnr);
		fxtnr->d_bid_spt_cpmg = 0.1;
		LogDel( "5 p=[%p]", fxtnr);

		memset(fxtnr->s_tnr_tcd, 0x00, 8);
		memset(fxtnr->s_sldy, 0x00, 9);

		switch(i)
		{
		case 0: // TOD
			memcpy(fxtnr->s_tnr_tcd, "TOD", 3);
			memcpy(fxtnr->s_sldy, "20230919", 8);
			fxtnr->d_ndd = 0.0;
			fxtnr->d_ask_fwd_cvmg = 0.0;
			fxtnr->d_ask_fwd_cpmg = 0.0;
			fxtnr->d_ask_swap_pnt = 0.0;

			fxtnr->d_bid_fwd_cvmg = 0.0;
			fxtnr->d_bid_fwd_cpmg = 0.0;
			fxtnr->d_bid_swap_pnt = 0.0;
			break;
		case 1: // TOM
			memcpy(fxtnr->s_tnr_tcd, "TOM", 3);
			memcpy(fxtnr->s_sldy, "20230920", 8);
			fxtnr->d_ndd = 0.0;
			fxtnr->d_ask_fwd_cvmg = 0.0;
			fxtnr->d_ask_fwd_cpmg = 0.0;
			fxtnr->d_ask_swap_pnt = 0.0;

			fxtnr->d_bid_fwd_cvmg = 0.0;
			fxtnr->d_bid_fwd_cpmg = 0.0;
			fxtnr->d_bid_swap_pnt = 0.0;
			break;
		case 2:
			memcpy(fxtnr->s_tnr_tcd, "SPT", 3);
			memcpy(fxtnr->s_sldy, "20230921", 8);
			fxtnr->d_ndd = 0.0;
			fxtnr->d_ask_fwd_cvmg = 0.0;
			fxtnr->d_ask_fwd_cpmg = 0.0;
			fxtnr->d_ask_swap_pnt = 0.0;

			fxtnr->d_bid_fwd_cvmg = 0.0;
			fxtnr->d_bid_fwd_cpmg = 0.0;
			fxtnr->d_bid_swap_pnt = 0.0;
			break;
		case 3:
			memcpy(fxtnr->s_tnr_tcd, "W01", 3);
			memcpy(fxtnr->s_sldy, "20231004", 8);
			fxtnr->d_ndd = 13.0;
			fxtnr->d_ask_spt_cvmg = 0.0;
			fxtnr->d_ask_spt_cpmg = 0.0;

			fxtnr->d_ask_fwd_cvmg = 0.25;
			fxtnr->d_ask_fwd_cpmg = 0.71;
			fxtnr->d_ask_swap_pnt = 39.24;

			fxtnr->d_bid_spt_cvmg = 0.0;
			fxtnr->d_bid_spt_cpmg = 0.0;

			fxtnr->d_bid_fwd_cvmg = 0.55;
			fxtnr->d_bid_fwd_cpmg = 0.41;
			fxtnr->d_bid_swap_pnt = 32.18;
			break;
		case 4:
			memcpy(fxtnr->s_tnr_tcd, "M01", 3);
			memcpy(fxtnr->s_sldy, "20231023", 8);
			fxtnr->d_ndd = 32.0;
			fxtnr->d_ask_spt_cvmg = 0.0;
			fxtnr->d_ask_spt_cpmg = 0.0;

			fxtnr->d_ask_fwd_cvmg = 0.25;
			fxtnr->d_ask_fwd_cpmg = 1.05;
			fxtnr->d_ask_swap_pnt = 317.63;

			fxtnr->d_bid_spt_cvmg = 0.0;
			fxtnr->d_bid_spt_cpmg = 0.0;

			fxtnr->d_bid_fwd_cvmg = 0.55;
			fxtnr->d_bid_fwd_cpmg = 0.68;
			fxtnr->d_bid_swap_pnt = 284.48;
			break;
		case 5:
			memcpy(fxtnr->s_tnr_tcd, "M02", 3);
			memcpy(fxtnr->s_sldy, "20231121", 8);
			fxtnr->d_ndd = 61.0;
			fxtnr->d_ask_spt_cvmg = 0.0;
			fxtnr->d_ask_spt_cpmg = 0.0;

			fxtnr->d_ask_fwd_cvmg = 0.25;
			fxtnr->d_ask_fwd_cpmg = 1.12;
			fxtnr->d_ask_swap_pnt = 589.78;

			fxtnr->d_bid_spt_cvmg = 0.0;
			fxtnr->d_bid_spt_cpmg = 0.0;

			fxtnr->d_bid_fwd_cvmg = 0.55;
			fxtnr->d_bid_fwd_cpmg = 0.82;
			fxtnr->d_bid_swap_pnt = 542.27;
			break;
		case 6:
			memcpy(fxtnr->s_tnr_tcd, "M03", 3);
			memcpy(fxtnr->s_sldy, "20231221", 8);
			fxtnr->d_ndd = 91.0;
			fxtnr->d_ask_spt_cvmg = 0.0;
			fxtnr->d_ask_spt_cpmg = 0.0;

			fxtnr->d_ask_fwd_cvmg = 0.25;
			fxtnr->d_ask_fwd_cpmg = 1.39;
			fxtnr->d_ask_swap_pnt = 881.42;

			fxtnr->d_bid_spt_cvmg = 0.0;
			fxtnr->d_bid_spt_cpmg = 0.0;

			fxtnr->d_bid_fwd_cvmg = 0.55;
			fxtnr->d_bid_fwd_cpmg = 1.09;
			fxtnr->d_bid_swap_pnt = 817.31;
			break;
		case 7:
			memcpy(fxtnr->s_tnr_tcd, "M06", 3);
			memcpy(fxtnr->s_sldy, "20240321", 8);
			fxtnr->d_ndd = 182.0;
			fxtnr->d_ask_spt_cvmg = 0.0;
			fxtnr->d_ask_spt_cpmg = 0.0;

			fxtnr->d_ask_fwd_cvmg = 0.25;
			fxtnr->d_ask_fwd_cpmg = 1.73;
			fxtnr->d_ask_swap_pnt = 1730.95;

			fxtnr->d_bid_spt_cvmg = 0.0;
			fxtnr->d_bid_spt_cpmg = 0.0;

			fxtnr->d_bid_fwd_cvmg = 0.550;
			fxtnr->d_bid_fwd_cpmg = 1.42;
			fxtnr->d_bid_swap_pnt = 1620.10;
			break;
		case 8:
			memcpy(fxtnr->s_tnr_tcd, "Y01", 3);
			memcpy(fxtnr->s_sldy, "20240923", 8);
			fxtnr->d_ask_spt_cvmg = 0.0;
			fxtnr->d_ask_spt_cpmg = 0.0;

			fxtnr->d_ndd = 368.0;
			fxtnr->d_ask_fwd_cvmg = 0.25;
			fxtnr->d_ask_fwd_cpmg = 1.73;
			fxtnr->d_ask_swap_pnt = 3393.54;

			fxtnr->d_bid_spt_cvmg = 0.0;
			fxtnr->d_bid_spt_cpmg = 0.0;

			fxtnr->d_bid_fwd_cvmg = 0.55;
			fxtnr->d_bid_fwd_cpmg = 1.42;
			fxtnr->d_bid_swap_pnt = 3221.04;
			break;
		}
		LogDel("tnr_tcd [%s]  s_sldy[%s]", fxtnr->s_tnr_tcd, fxtnr->s_sldy);

		fxtnr->d_ask_cvmg_prc = 0.0;
		fxtnr->d_ask_hdom     = 0.0;
		fxtnr->d_ask_hdom_prc = 0.0;
		fxtnr->d_ask_mrkt_prc = 0.0;
		fxtnr->d_ask_hdof_prc = 0.0;

		fxtnr->d_bid_cvmg_prc = 0.0;
		fxtnr->d_bid_hdom     = 0.0;
		fxtnr->d_bid_hdom_prc = 0.0;
		fxtnr->d_bid_mrkt_prc = 0.0;
		fxtnr->d_bid_hdof_prc = 0.0;

		fxtnr->d_ask_cvmg_prc = fxpair->d_ask_spt_prc + fxtnr->d_ask_spt_cvmg + fxtnr->d_ask_fwd_cvmg;
		fxtnr->d_ask_hdom     = fxtnr->d_ask_spt_cvmg + fxtnr->d_ask_spt_cpmg + fxtnr->d_ask_fwd_cvmg + fxtnr->d_ask_fwd_cpmg;
		fxtnr->d_ask_hdom_prc = fxpair->d_ask_spt_prc + fxtnr->d_ask_hdom;
		fxtnr->d_ask_mrkt_prc = fxpair->d_ask_spt_prc + fxtnr->d_ask_swap_pnt / 100.0;
		fxtnr->d_ask_hdof_prc = fxtnr->d_ask_mrkt_prc + fxtnr->d_ask_hdom;

		fxtnr->d_bid_cvmg_prc = fxpair->d_bid_spt_prc - fxtnr->d_bid_spt_cvmg - fxtnr->d_bid_fwd_cvmg;
		fxtnr->d_bid_hdom     = fxtnr->d_bid_spt_cvmg + fxtnr->d_bid_spt_cpmg + fxtnr->d_bid_fwd_cvmg + fxtnr->d_bid_fwd_cpmg;
		fxtnr->d_bid_hdom_prc = fxpair->d_bid_spt_prc - fxtnr->d_bid_hdom;
		fxtnr->d_bid_mrkt_prc = fxpair->d_bid_spt_prc + fxtnr->d_bid_swap_pnt / 100.0;
		fxtnr->d_bid_hdof_prc = fxtnr->d_bid_mrkt_prc - fxtnr->d_bid_hdom;

		l_round_x(&fxtnr->d_ask_mrkt_prc, fxpair->n_digit);
		l_round_x(&fxtnr->d_ask_hdof_prc, fxpair->n_digit);

		l_round_x(&fxtnr->d_bid_mrkt_prc, fxpair->n_digit);
		l_round_x(&fxtnr->d_bid_hdof_prc, fxpair->n_digit);

		fxtnr->d_ask_pdcp = fxtnr->d_ask_hdof_prc;
		fxtnr->d_bid_pdcp = fxtnr->d_bid_hdof_prc;

		LogDel("테너 [%s] 적재 완료", fxtnr->s_tnr_tcd);

		fxpair->n_tnr_cnt++;
	}

	p_hdomgrp->grp[0].n_pair_cnt++;

	LogDel("[%s] tnr_cnt[%d]", fxpair->s_pair_id, fxpair->n_tnr_cnt);


	fxpair = (FXPAIR_ST *)&p_hdomgrp->grp[0].pair[2]; 

//	fxpair = (FXPAIR_ST *)&fxgrp->pair[2];
//	memset(fxpair, 0x00, FXPAIR_ST_SZ);

	LogDel( "fxpair->s_pair_id =[%p]", fxpair->s_pair_id);

	memcpy(fxpair->s_pair_id, "USDJPY", 6);

	LogDel("pair id [%s]", fxpair->s_pair_id);

	fxpair->s_clc_dsnc[0] = '2';
	fxpair->n_digit = 2;
	fxpair->d_digit_val = 0.01;
	fxpair->d_clc_unit  = 100.0;
	fxpair->d_ask_spt_prc  = 147.89;
	fxpair->d_bid_spt_prc  = 147.84; 
	fxpair->d_ask_usd_spt_prc = 1328.5;
	fxpair->d_bid_usd_spt_prc = 1328.5;
	fxpair->s_ask_orgn[0] = 'X';
	fxpair->s_bid_orgn[0] = 'X';

	memcpy(fxpair->s_rcv_ymd, "20230919", 8);

	memcpy(fxpair->s_rcv_hms, "000000000", 9);


	fxpair->n_tnr_cnt = 0;
	for (i = 0; i < MAX_TNR_CNT; i++)
	{

//		fxtnr = (FXTNR_ST *)&fxpair->tnr[i];
		fxtnr = (FXTNR_ST *)&p_hdomgrp->grp[0].pair[2].tnr[i];

		LogDel( "for[%d] ... fxtnr=[%p]", i, fxtnr);
		fxtnr->s_tnr_dsnc[0] = 'S';

		LogDel( "11 fxtnr=[%p]", fxtnr);
		fxtnr->d_ask_spt_cvmg = 0.03;
		LogDel( "12 fxtnr=[%p]", fxtnr);
		fxtnr->d_ask_spt_cpmg = 0.0;
		LogDel( "13 fxtnr=[%p]", fxtnr);
		fxtnr->d_ask_fwd_cvmg = 0.0;
		LogDel( "14 fxtnr=[%p]", fxtnr);
		fxtnr->d_ask_fwd_cpmg = 0.0;
		LogDel( "15 fxtnr=[%p]", fxtnr);

		fxtnr->d_bid_spt_cvmg = 0.03;
		LogDel( "16 fxtnr=[%p]", fxtnr);
		fxtnr->d_bid_spt_cpmg = 0.0;
		LogDel( "17 fxtnr=[%p]", fxtnr);
		fxtnr->d_bid_fwd_cvmg = 0.0;
		fxtnr->d_bid_fwd_cpmg = 0.0;

		memset(fxtnr->s_tnr_tcd, 0x00, sizeof(fxtnr->s_tnr_tcd));
		memset(fxtnr->s_sldy, 0x00, sizeof(fxtnr->s_sldy));


		switch(i)
		{
		case 0: // TOD
			memcpy(fxtnr->s_tnr_tcd, "TOD", 3);
			memcpy(fxtnr->s_sldy, "20230919", 8);
			fxtnr->d_ndd = 0.0;
			
			fxtnr->d_ask_swap_pnt = -2.25;

			fxtnr->d_bid_swap_pnt = -2.65;
			break;
		case 1: // TOM
			memcpy(fxtnr->s_tnr_tcd, "TOM", 3);
			memcpy(fxtnr->s_sldy, "20230920", 8);
			fxtnr->d_ndd = 0.0;
			fxtnr->d_ask_swap_pnt = -2.45;

			fxtnr->d_bid_swap_pnt = -2.48;
			break;
		case 2:
			memcpy(fxtnr->s_tnr_tcd, "SPT", 3);
			memcpy(fxtnr->s_sldy, "20230921", 8);
			fxtnr->d_ndd = 0.0;
			fxtnr->d_ask_swap_pnt = 0.0;

			fxtnr->d_bid_swap_pnt = 0.0;
			break;
		case 3:
			memcpy(fxtnr->s_tnr_tcd, "W01", 3);
			memcpy(fxtnr->s_sldy, "20231004", 8);
			fxtnr->d_ndd = 13.0;

			fxtnr->d_ask_fwd_cvmg = 0.05;
			fxtnr->d_ask_swap_pnt = -16.31;

			fxtnr->d_bid_fwd_cvmg = 0.05;
			fxtnr->d_bid_swap_pnt = -16.46;
			break;
		case 4:
			memcpy(fxtnr->s_tnr_tcd, "M01", 3);
			memcpy(fxtnr->s_sldy, "20231023", 8);
			fxtnr->d_ndd = 32.0;

			fxtnr->d_ask_fwd_cvmg = 0.05;
			fxtnr->d_ask_swap_pnt = -76.1;


			fxtnr->d_bid_fwd_cvmg = 0.05;
			fxtnr->d_bid_swap_pnt = -76.21;
			break;
		case 5:
			memcpy(fxtnr->s_tnr_tcd, "M02", 3);
			memcpy(fxtnr->s_sldy, "20231121", 8);
			fxtnr->d_ndd = 61.0;

			fxtnr->d_ask_fwd_cvmg = 0.05;
			fxtnr->d_ask_swap_pnt = -143.86;

			fxtnr->d_bid_fwd_cvmg = 0.05;
			fxtnr->d_bid_swap_pnt = -144.06; 
			break;
		case 6:
			memcpy(fxtnr->s_tnr_tcd, "M03", 3);
			memcpy(fxtnr->s_sldy, "20231221", 8);
			fxtnr->d_ndd = 91.0;
			fxtnr->d_ask_fwd_cvmg = 0.05;
			fxtnr->d_ask_swap_pnt = -213.92;

			fxtnr->d_bid_fwd_cvmg = 0.05;
			fxtnr->d_bid_swap_pnt = -214.26;
			break;
		case 7:
			memcpy(fxtnr->s_tnr_tcd, "M06", 3);
			memcpy(fxtnr->s_sldy, "20240321", 8);
			fxtnr->d_ndd = 182.0;

			fxtnr->d_ask_fwd_cvmg = 0.05;
			fxtnr->d_ask_swap_pnt = -432.7;

			fxtnr->d_bid_fwd_cvmg = 0.05;
			fxtnr->d_bid_swap_pnt = -433.85;
			break;
		case 8:
			memcpy(fxtnr->s_tnr_tcd, "Y01", 3);
			memcpy(fxtnr->s_sldy, "20240923", 8);
			fxtnr->d_ndd = 368.0;

			fxtnr->d_ask_fwd_cvmg = 0.05;
			fxtnr->d_ask_swap_pnt = -838.2;


			fxtnr->d_bid_fwd_cvmg = 0.05;
			fxtnr->d_bid_swap_pnt = -841.35;
			break;
		}
		LogDel("tnr_tcd [%s]  s_sldy[%s]", fxtnr->s_tnr_tcd, fxtnr->s_sldy);

		fxtnr->d_ask_cvmg_prc = 0.0;
		fxtnr->d_ask_hdom     = 0.0;
		fxtnr->d_ask_hdom_prc = 0.0;
		fxtnr->d_ask_mrkt_prc = 0.0;
		fxtnr->d_ask_hdof_prc = 0.0;

		fxtnr->d_bid_cvmg_prc = 0.0;
		fxtnr->d_bid_hdom     = 0.0;
		fxtnr->d_bid_hdom_prc = 0.0;
		fxtnr->d_bid_mrkt_prc = 0.0;
		fxtnr->d_bid_hdof_prc = 0.0;

		fxtnr->d_ask_cvmg_prc = fxpair->d_ask_spt_prc + fxtnr->d_ask_spt_cvmg + fxtnr->d_ask_fwd_cvmg;
		fxtnr->d_ask_hdom     = fxtnr->d_ask_spt_cvmg + fxtnr->d_ask_spt_cpmg + fxtnr->d_ask_fwd_cvmg + fxtnr->d_ask_fwd_cpmg;
		fxtnr->d_ask_hdom_prc = fxpair->d_ask_spt_prc + fxtnr->d_ask_hdom;
		fxtnr->d_ask_mrkt_prc = fxpair->d_ask_spt_prc + fxtnr->d_ask_swap_pnt / 100.0;
		fxtnr->d_ask_hdof_prc = fxtnr->d_ask_mrkt_prc + fxtnr->d_ask_hdom;

		fxtnr->d_bid_cvmg_prc = fxpair->d_bid_spt_prc - fxtnr->d_bid_spt_cvmg - fxtnr->d_bid_fwd_cvmg;
		fxtnr->d_bid_hdom     = fxtnr->d_bid_spt_cvmg + fxtnr->d_bid_spt_cpmg + fxtnr->d_bid_fwd_cvmg + fxtnr->d_bid_fwd_cpmg;
		fxtnr->d_bid_hdom_prc = fxpair->d_bid_spt_prc - fxtnr->d_bid_hdom;
		fxtnr->d_bid_mrkt_prc = fxpair->d_bid_spt_prc + fxtnr->d_bid_swap_pnt / 100.0;
		fxtnr->d_bid_hdof_prc = fxtnr->d_bid_mrkt_prc - fxtnr->d_bid_hdom;

		l_round_x(&fxtnr->d_ask_mrkt_prc, fxpair->n_digit);
		l_round_x(&fxtnr->d_ask_hdof_prc, fxpair->n_digit);

		l_round_x(&fxtnr->d_bid_mrkt_prc, fxpair->n_digit);
		l_round_x(&fxtnr->d_bid_hdof_prc, fxpair->n_digit);

		fxtnr->d_ask_pdcp = fxtnr->d_ask_hdof_prc;
		fxtnr->d_bid_pdcp = fxtnr->d_bid_hdof_prc;


		LogDel("테너 [%s] 적재 완료", fxtnr->s_tnr_tcd);

		fxpair->n_tnr_cnt++;
	}

	p_hdomgrp->grp[0].n_pair_cnt++;

	LogDel("[%s] tnr_cnt[%d]", fxpair->s_pair_id, fxpair->n_tnr_cnt);

	LogDel("적재 완료");
	p_hdomgrp->n_grp_cnt = 1;
}



/*
int f_calc_sise_shm(p_sise, p_hdomgrp, s_errcd)
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

				}
			}
			n_pcnt++;
		}
		n_gcnt++;
	}
	return 0;
}
*/
