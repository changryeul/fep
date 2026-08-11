/** ***************************************************************************
**  @file       blp.c
**  @date       2025/09/01
**  @author     cdc
**  @version    V0.0.20250901
**  @brif
**  채권 시장조성 라이브러리
**	blp.c			- 
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#define		DATA_SIZE	2048
#include "shm_memory.h"
#include "pa_struct.h"
#include "buf_struct.h"

#include "mem.h"
#include "sem.h"
#include "etc.h"
#ifndef _OMS_SOURCE_
#include "log.h"
#else
#include "log_conv.h"
#endif
#include "blp.h"

/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/
extern int		Continue;
extern SHM_NOTE	*Shm_Note;

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  거래소 체결 시세 처리
***************************************************************************** */
int Blp_ConvertExe( BLP *blp, BLP_SISE *sise, CO_G701K *g701k)
{
	int				int_time, ms_time;
	// int				wait;

	/***********************************/
	/* 시간 covert                     */
	/***********************************/
	memcpy( &sise->tv_old, &sise->tv, sizeof( struct timeval));
	int_time = AtoI( &g701k->trade_time[ 0], 6);
	ms_time  = AtoI( &g701k->trade_time[ 6], 6);
	LogDbg( "int_time=[%06d.%06d]", int_time, ms_time);
	gettimeofday( &sise->tv, NULL);
	sise->tv.tv_sec = sise->tv.tv_sec - sise->tv.tv_sec % 86400 - 32400;	/* 하루 시작 */
	sise->tv.tv_sec = sise->tv.tv_sec + ( int_time / 10000) * 3600 + (( int_time % 10000) / 100) * 60 + int_time % 100;
	sise->tv.tv_usec = ms_time;
	/*
	wait = ( sise->tv.tv_sec - sise->tv_old.tv_sec) * 1000000 + sise->tv.tv_usec - sise->tv_old.tv_usec;
	LogDbg( "usleep[%d]", wait);
	if( wait > 0 && wait < 10000000) usleep( wait / 10);
	*/


	/***********************************/
	/* 체결 convert                    */
	/***********************************/
	sise->price = AtoD( g701k->crprc, sizeof( g701k->crprc));
	sise->price = AtoD( g701k->volume, sizeof( g701k->volume));

	/***********************************/
	/* 호가 convert                    */
	/***********************************/
	sise->ask[ 0][ 0] = AtoD( g701k->ask1_price, sizeof( g701k->ask1_price));
	sise->ask[ 0][ 1] = AtoD( g701k->ask2_price, sizeof( g701k->ask2_price));
	sise->ask[ 0][ 2] = AtoD( g701k->ask3_price, sizeof( g701k->ask3_price));
	sise->ask[ 0][ 3] = AtoD( g701k->ask4_price, sizeof( g701k->ask4_price));
	sise->ask[ 0][ 4] = AtoD( g701k->ask5_price, sizeof( g701k->ask5_price));
	sise->bid[ 0][ 0] = AtoD( g701k->bid1_price, sizeof( g701k->bid1_price));
	sise->bid[ 0][ 1] = AtoD( g701k->bid2_price, sizeof( g701k->bid2_price));
	sise->bid[ 0][ 2] = AtoD( g701k->bid3_price, sizeof( g701k->bid3_price));
	sise->bid[ 0][ 3] = AtoD( g701k->bid4_price, sizeof( g701k->bid4_price));
	sise->bid[ 0][ 4] = AtoD( g701k->bid5_price, sizeof( g701k->bid5_price));

	sise->ask[ 1][ 0] = AtoD( g701k->bond_ask1_remain_vol, sizeof( g701k->bond_ask1_remain_vol));
	sise->ask[ 1][ 1] = AtoD( g701k->bond_ask2_remain_vol, sizeof( g701k->bond_ask2_remain_vol));
	sise->ask[ 1][ 2] = AtoD( g701k->bond_ask3_remain_vol, sizeof( g701k->bond_ask3_remain_vol));
	sise->ask[ 1][ 3] = AtoD( g701k->bond_ask4_remain_vol, sizeof( g701k->bond_ask4_remain_vol));
	sise->ask[ 1][ 4] = AtoD( g701k->bond_ask5_remain_vol, sizeof( g701k->bond_ask5_remain_vol));
	sise->bid[ 1][ 0] = AtoD( g701k->bond_bid1_remain_vol, sizeof( g701k->bond_bid1_remain_vol));
	sise->bid[ 1][ 1] = AtoD( g701k->bond_bid2_remain_vol, sizeof( g701k->bond_bid2_remain_vol));
	sise->bid[ 1][ 2] = AtoD( g701k->bond_bid3_remain_vol, sizeof( g701k->bond_bid3_remain_vol));
	sise->bid[ 1][ 3] = AtoD( g701k->bond_bid4_remain_vol, sizeof( g701k->bond_bid4_remain_vol));
	sise->bid[ 1][ 4] = AtoD( g701k->bond_bid5_remain_vol, sizeof( g701k->bond_bid5_remain_vol));

	return sizeof( CO_G701K);
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  거래소 호가 시세 처리
***************************************************************************** */
int Blp_ConvertHoga( BLP *blp, BLP_SISE *sise, CO_B601K *b601k)
{
	int				int_time, ms_time;
	double			prc = 0.0;
	// int				wait;

	CO_B601K_Print( b601k);

	/***********************************/
	/* check 1 호가                    */
	/***********************************/
	prc += AtoD( b601k->ask1_price, sizeof( b601k->ask1_price));
	prc += AtoD( b601k->bid1_price, sizeof( b601k->bid1_price));
	if( prc <= 0) return 0;

	/***********************************/
	/* 시간 covert                     */
	/***********************************/
	memcpy( &sise->tv_old, &sise->tv, sizeof( struct timeval));
	int_time = AtoI( &b601k->trade_time[ 0], 6);
	ms_time  = AtoI( &b601k->trade_time[ 6], 6);
	LogDbg( "int_time =[%06d.%06d]", int_time, ms_time);
	gettimeofday( &sise->tv, NULL);
	sise->tv.tv_sec = sise->tv.tv_sec - sise->tv.tv_sec % 86400 - 32400;	/* 하루 시작 */
	sise->tv.tv_sec = sise->tv.tv_sec + ( int_time / 10000) * 3600 + (( int_time % 10000) / 100) * 60 + int_time % 100;
	sise->tv.tv_usec = ms_time;
	LogDbg( "sise time=[%s]", TtoS( sise->tv.tv_sec));
	/*
	wait = ( sise->tv.tv_sec - sise->tv_old.tv_sec) * 1000000 + sise->tv.tv_usec - sise->tv_old.tv_usec;
	LogDbg( "usleep[%d]", wait);
	if( wait > 0 && wait < 10000000) usleep( wait / 10);
	*/

	/***********************************/
	/* hoga convert                    */
	/***********************************/
	sise->ask[ 0][ 0] = AtoD( b601k->ask1_price, sizeof( b601k->ask1_price));
	sise->ask[ 0][ 1] = AtoD( b601k->ask2_price, sizeof( b601k->ask2_price));
	sise->ask[ 0][ 2] = AtoD( b601k->ask3_price, sizeof( b601k->ask3_price));
	sise->ask[ 0][ 3] = AtoD( b601k->ask4_price, sizeof( b601k->ask4_price));
	sise->ask[ 0][ 4] = AtoD( b601k->ask5_price, sizeof( b601k->ask5_price));
	sise->bid[ 0][ 0] = AtoD( b601k->bid1_price, sizeof( b601k->bid1_price));
	sise->bid[ 0][ 1] = AtoD( b601k->bid2_price, sizeof( b601k->bid2_price));
	sise->bid[ 0][ 2] = AtoD( b601k->bid3_price, sizeof( b601k->bid3_price));
	sise->bid[ 0][ 3] = AtoD( b601k->bid4_price, sizeof( b601k->bid4_price));
	sise->bid[ 0][ 4] = AtoD( b601k->bid5_price, sizeof( b601k->bid5_price));

	sise->ask[ 1][ 0] = AtoD( b601k->bond_ask1_remain_vol, sizeof( b601k->bond_ask1_remain_vol));
	sise->ask[ 1][ 1] = AtoD( b601k->bond_ask2_remain_vol, sizeof( b601k->bond_ask2_remain_vol));
	sise->ask[ 1][ 2] = AtoD( b601k->bond_ask3_remain_vol, sizeof( b601k->bond_ask3_remain_vol));
	sise->ask[ 1][ 3] = AtoD( b601k->bond_ask4_remain_vol, sizeof( b601k->bond_ask4_remain_vol));
	sise->ask[ 1][ 4] = AtoD( b601k->bond_ask5_remain_vol, sizeof( b601k->bond_ask5_remain_vol));
	sise->bid[ 1][ 0] = AtoD( b601k->bond_bid1_remain_vol, sizeof( b601k->bond_bid1_remain_vol));
	sise->bid[ 1][ 1] = AtoD( b601k->bond_bid2_remain_vol, sizeof( b601k->bond_bid2_remain_vol));
	sise->bid[ 1][ 2] = AtoD( b601k->bond_bid3_remain_vol, sizeof( b601k->bond_bid3_remain_vol));
	sise->bid[ 1][ 3] = AtoD( b601k->bond_bid4_remain_vol, sizeof( b601k->bond_bid4_remain_vol));
	sise->bid[ 1][ 4] = AtoD( b601k->bond_bid5_remain_vol, sizeof( b601k->bond_bid5_remain_vol));

	return sizeof( CO_B601K);
}


/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  거래소 호가 시세 처리
***************************************************************************** */
int	Blp_ConvertArg( BLP_ARG *arg, char *rec)
{
	BLP_ARG_IF	*arg_if = ( BLP_ARG_IF *)rec;
	BLP_TIME	*mkt;

	BLP_ARG_IF_Print( arg_if);

	memcpy( arg->exe_ymd                 , arg_if->exe_ymd              , sizeof( arg_if->exe_ymd              ));
	arg->exe_no                   = AtoI(  arg_if->exe_no               , sizeof( arg_if->exe_no               ));
	memcpy( arg->proc_stus_dstcd         , arg_if->proc_stus_dstcd      , sizeof( arg_if->proc_stus_dstcd      ));
	memcpy( arg->item_cd                 , arg_if->item_cd              , sizeof( arg_if->item_cd              ));
	memcpy( arg->mm_item_dstcd           , arg_if->mm_item_dstcd        , sizeof( arg_if->mm_item_dstcd        ));
	arg->ord_qanty_unit           = AtoI(  arg_if->ord_qanty_unit       , sizeof( arg_if->ord_qanty_unit       ));
	memcpy( arg->dspratio_taget_dstcd    , arg_if->dspratio_taget_dstcd , sizeof( arg_if->dspratio_taget_dstcd ));
	memcpy( arg->dspratio_dstcd          , arg_if->dspratio_dstcd       , sizeof( arg_if->dspratio_dstcd       ));
	arg->dspratio                 = AtoD(  arg_if->dspratio             , sizeof( arg_if->dspratio             ));
	arg->tick_unit                = AtoD(  arg_if->tick_unit            , sizeof( arg_if->tick_unit            ));
	memcpy( arg->trdr_uno                , arg_if->trdr_uno             , sizeof( arg_if->trdr_uno             ));
	memcpy( arg->trdr_no                 , arg_if->trdr_no              , sizeof( arg_if->trdr_no              ));
	arg->prv_yild                 = AtoD(  arg_if->prv_yild             , sizeof( arg_if->prv_yild             ));
	arg->prv_prc                  = AtoD(  arg_if->prv_prc              , sizeof( arg_if->prv_prc              ));
	arg->clsng_prc                = AtoD(  arg_if->clsng_prc            , sizeof( arg_if->clsng_prc            ));
	arg->clsng_yild               = AtoD(  arg_if->clsng_yild           , sizeof( arg_if->clsng_yild           ));
	memcpy( arg->account_no              , arg_if->account_no           , sizeof( arg_if->account_no           ));
	arg->mk_stat                  = AtoI(  arg_if->mk_stat              , sizeof( arg_if->mk_stat              ));

	mkt = &arg->mk_time[ 0];
	mkt->start                    = AtoT( arg_if->start_1              , sizeof( arg_if->start_1              ));
	mkt->end                      = AtoT( arg_if->end_1                , sizeof( arg_if->end_1                ));
	mkt->lp_time                  = AtoI( arg_if->lp_time_1            , sizeof( arg_if->lp_time_1            )) * 60;
	mkt->exe_delay                = AtoI( arg_if->exe_delay_1          , sizeof( arg_if->exe_delay_1          ));
	mkt->rev_wait                 = AtoI( arg_if->rev_wait_1           , sizeof( arg_if->rev_wait_1           ));
	mkt->submit_limit             = AtoI( arg_if->submit_limit_1       , sizeof( arg_if->submit_limit_1       ));
	mkt->sped_prc[ 0]             = AtoD( arg_if->sped_prc_11          , sizeof( arg_if->sped_prc_11          ));
	mkt->sped_prc[ 1]             = AtoD( arg_if->sped_prc_12          , sizeof( arg_if->sped_prc_12          ));
	mkt->sped_prc[ 2]             = AtoD( arg_if->sped_prc_13          , sizeof( arg_if->sped_prc_13          ));
	mkt->ord_qanty[ 0]            = AtoD( arg_if->ord_qanty_11         , sizeof( arg_if->ord_qanty_11         ));
	mkt->ord_qanty[ 1]            = AtoD( arg_if->ord_qanty_12         , sizeof( arg_if->ord_qanty_12         ));
	mkt->ord_qanty[ 2]            = AtoD( arg_if->ord_qanty_13         , sizeof( arg_if->ord_qanty_13         ));

	mkt = &arg->mk_time[ 1];
	mkt->start                    = AtoT( arg_if->start_2              , sizeof( arg_if->start_2              ));
	mkt->end                      = AtoT( arg_if->end_2                , sizeof( arg_if->end_2                ));
	mkt->lp_time                  = AtoI( arg_if->lp_time_2            , sizeof( arg_if->lp_time_2            )) * 60;
	mkt->exe_delay                = AtoI( arg_if->exe_delay_2          , sizeof( arg_if->exe_delay_2          ));
	mkt->rev_wait                 = AtoI( arg_if->rev_wait_2           , sizeof( arg_if->rev_wait_2           ));
	mkt->submit_limit             = AtoI( arg_if->submit_limit_2       , sizeof( arg_if->submit_limit_2       ));
	mkt->sped_prc[ 0]             = AtoD( arg_if->sped_prc_21          , sizeof( arg_if->sped_prc_21          ));
	mkt->sped_prc[ 1]             = AtoD( arg_if->sped_prc_22          , sizeof( arg_if->sped_prc_22          ));
	mkt->sped_prc[ 2]             = AtoD( arg_if->sped_prc_23          , sizeof( arg_if->sped_prc_23          ));
	mkt->ord_qanty[ 0]            = AtoD( arg_if->ord_qanty_21         , sizeof( arg_if->ord_qanty_21         ));
	mkt->ord_qanty[ 1]            = AtoD( arg_if->ord_qanty_22         , sizeof( arg_if->ord_qanty_22         ));
	mkt->ord_qanty[ 2]            = AtoD( arg_if->ord_qanty_23         , sizeof( arg_if->ord_qanty_23         ));
	
	mkt = &arg->mk_time[ 2];
	mkt->start                    = AtoT( arg_if->start_3              , sizeof( arg_if->start_3              ));
	mkt->end                      = AtoT( arg_if->end_3                , sizeof( arg_if->end_3                ));
	mkt->lp_time                  = AtoI( arg_if->lp_time_3            , sizeof( arg_if->lp_time_3            )) * 60;
	mkt->exe_delay                = AtoI( arg_if->exe_delay_3          , sizeof( arg_if->exe_delay_3          ));
	mkt->rev_wait                 = AtoI( arg_if->rev_wait_3           , sizeof( arg_if->rev_wait_3           ));
	mkt->submit_limit             = AtoI( arg_if->submit_limit_3       , sizeof( arg_if->submit_limit_3       ));
	mkt->sped_prc[ 0]             = AtoD( arg_if->sped_prc_31          , sizeof( arg_if->sped_prc_31          ));
	mkt->sped_prc[ 1]             = AtoD( arg_if->sped_prc_32          , sizeof( arg_if->sped_prc_32          ));
	mkt->sped_prc[ 2]             = AtoD( arg_if->sped_prc_33          , sizeof( arg_if->sped_prc_33          ));
	mkt->ord_qanty[ 0]            = AtoD( arg_if->ord_qanty_31         , sizeof( arg_if->ord_qanty_31         ));
	mkt->ord_qanty[ 1]            = AtoD( arg_if->ord_qanty_32         , sizeof( arg_if->ord_qanty_32         ));
	mkt->ord_qanty[ 2]            = AtoD( arg_if->ord_qanty_33         , sizeof( arg_if->ord_qanty_33         ));

	return 1;
}

