/** ***************************************************************************
**  @file       mat.c
**  @date       2023/08/23
**  @author     cdc
**  @version    V2.0.20230823
**  @brif
**  매칭엔진 라이브러리
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "log.h"
#include "etc.h"
#include "mat.h"

#include "order.h"
#include "sise.h"
#include "main.h"

extern int		Continue;

int f_get_cust_prc( SPLIT_IN_ST *p_in, SPLIT_OUT_ST *p_out, MAT_E_MSG *msg);
int f_get_cust_swap_prc( SPLIT_IN_ST *p_in, SPLIT_IN_ST *p_in_f, SPLIT_OUT_ST *p_out, SPLIT_OUT_ST *p_out_f, MAT_E_MSG *msg);

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @param      int opt  - 0:일반주문 1:SPOT고정주문
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  일반주문 수수료 계산 lib call
***************************************************************************** */
double Mat_FeeProcess( MAT *mat, MAT_INDEX *index, int side, int pos)
{
	int			rtn = 1;
	time_t		cur_time;
	MAT_RECORD	*rec;		/* curr record */
	MAT_HEAD	*head;
	ORDER		*obook;
	MATSISE		*sise_curr = &index->sise_curr;

	double			price;				/* 시세 가격 */
	double			fee_price;			/* 수수료 가격 */
	double			book_price = 0.0;	/* 주문 가격 */

	rec     = &mat->map->rec[ pos];
	head    = &rec->head;
	obook   = ( ORDER *)&rec->ord;

	/* 매수주문 > 매도시세, 매도주문 < 매수시세 */
	if( side == 0)	price = sise_curr->askprc;
	else			price = sise_curr->bidprc;

	if( obook->TranPtrnCd[ 0] != '2') /* MAR의 경우 - 시세가 0인경우  수수료에서 전일종가로 계산 */
	{
		if( price <= 0)
		{
			return 0;
		}
	}
	book_price = obook->Price;

	switch( obook->TrdTypeDcd[ 0])
	{
		default:		/* 일반 주문 */
			break;
		case '2':		/* SPOT 고정주문 */
			break;
		case '3':		/* 대행 주문 */
			break;
	}


	if( obook->TrdTypeDcd[ 0] == '2') /* SPOT 고정 주문 */
	{
		/* SPOT 고정 주문의 고정가 경우 오류 - SPOT 고정은 시장가만 가능 */
		if( obook->OrdType[ 0] != '1') return -1;

		LogDbg( "SPOT 고정 주문  obook->TrdTypeDcd[ 0]=[%.1s]", obook->TrdTypeDcd);
		fee_price = Mat_GetFee( mat, index, side, pos, 0);
		if( fee_price < 0.0)
		{
			return -head->error;
		}
		else
		if( fee_price == 0.0)
		{
			return 0;
		}
	}
	else 
	if( obook->TrdTypeDcd[ 0] == '3') /* 대행주문의 경우 SPOT 가격은 주문 가격 그대로 체결 */
	{
		LogDev( "대행 주문  obook->TrdTypeDcd[ 0]=[%.1s]", obook->TrdTypeDcd);
		fee_price = obook->Price;
	}
	else							 /* 일반주문 */
	{
		if( memcmp( obook->SettType, "SWP", 3))	/* 일반주문 */
		{
			LogDev( "일반주문 obook->TrdTypeDcd[ 0]=[%.1s]", obook->TrdTypeDcd);
			fee_price = Mat_GetFee( mat, index, side, pos, 0);
			if( fee_price < 0.0)	/* 오류 */
			{
				return -head->error;
			}
			else
			if( fee_price == 0.0)	/* 미체결 */
			{
				return 0;
			}
		}
		else									/* SWAP 주문 */
		{
			LogDev( "SWAP 주문 obook->TrdTypeDcd[ 0]=[%.1s]", obook->TrdTypeDcd);
			fee_price = Mat_GetFeeSwap( mat, index, side, pos, 0);
			LogMsg( "시장가 FAR     =[%12f] 체결 pos=[%d]", fee_price, pos);
		}
	}

	/* 장운영 check */
	time( &cur_time);
	if( ( cur_time < mat->map->stat.start) || ( cur_time > mat->map->stat.end))
	{
		if( obook->TrdTypeDcd[ 0] != '3') /* 대행주문의 경우 장시간 외에도 체결 */
		{
			LogDev( "장 시간 아님 ... pos=[%d]", pos);
			LogDev( "    start    = [%s]", TtoS( mat->map->stat.start));
			LogDev( "    curr     = [%s]", TtoS( cur_time));
			LogDev( "    end      = [%s]", TtoS( mat->map->stat.end));
			LogDev( "    지정가   = [%12f] 미체결 pos=[%d]", book_price, pos);
			return 0;
		}
	}

	/* 시장가 */
	if( obook->OrdType[ 0] == '1')
	{
		/* 체결가는 head에 세팅후 체결 프로세스에서 처리 */
		head->exe_price = fee_price; /* obook->LastPx = fee_price; */

		LogMsg( "Matching ... [%s/%s] [%d]", mat->map->current[ index->base_cur].str, mat->map->current[ index->cont_cur].str, side);
		LogMsg( "시장가         =[%12f] 체결 pos=[%d]", fee_price, pos);
		return 1;
	}

	/* 매칭 비교 */
	if( ( side == 0) && ( book_price < fee_price)) 
	{
		LogDev( "지정가         =[%12f] < [%12f] 미체결 pos=[%d]", book_price, fee_price, pos);
		return 0;
	}
	if( ( side == 1) && ( book_price > fee_price)) 
	{
		LogDev( "지정가         =[%12f] > [%12f] 미체결 pos=[%d]", book_price, fee_price, pos);
		return 0;
	}

	/* 체결가는 head에 세팅후 체결 프로세스에서 처리 */
	if( obook->TrdTypeDcd[ 0] == '2') /* SPOT 고정 주문은 SpotPrc에 체결 */
	{
		head->exe_price = book_price;
	}
	else
	{
		head->exe_price = fee_price; /* obook->LastPx = fee_price; */
	}

	LogDev( "execute ... pos=[%d] price=[%12f] my=[%12f]", pos, price, book_price);
	LogMsg( "Matching ... [%s/%s] [%d]", mat->map->current[ index->base_cur].str, mat->map->current[ index->cont_cur].str, side);
	LogMsg( "지정가 체결 ...  side=[%d] pos=[%d]", side, pos);
	LogMsg( "                 book_price   =[%15f] 주문가", book_price);
	LogMsg( "                 fee_price    =[%15f] 체결가", fee_price);

	return rtn;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @param      int opt  - 0:일반주문 1:SPOT고정주문
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  일반주문 수수료 계산 lib call
***************************************************************************** */
int Mat_CheckFee( MAT *mat, MAT_INDEX *index, ORDER *obook)
{
	int				rtn;

    SPLIT_IN_ST     _fee_in, *fee_in = &_fee_in;
    SPLIT_OUT_ST    _fee_out, *fee_out = &_fee_out;
    SPLIT_IN_ST     _fee_in_far, *fee_in_far = &_fee_in_far;
    SPLIT_OUT_ST    _fee_out_far, *fee_out_far = &_fee_out_far;
    MAT_E_MSG       e_msg;
    double          fee_price = 0.0;

	Mat_StatisticsSet( mat, MAT_STAT_PEE, MAT_STAT_START);
	memset( &e_msg, 0x00, sizeof( MAT_E_MSG));

	/**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**************/
	/* 수수료 Call                                                                  */
	/* 수수료쪽 체결 error가 나는것을 방지하기위해 사전에 call하여 오류 여부를 판단 */
	/************++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++****/
	memset( fee_in,                  0x00,              sizeof( SPLIT_IN_ST));
	memset( fee_in_far,              0x00,              sizeof( SPLIT_IN_ST));
	memset( fee_out,                 0x00,              sizeof( SPLIT_OUT_ST));
	memset( fee_out_far,             0x00,              sizeof( SPLIT_OUT_ST));
	memset( &e_msg,                  0x00,              sizeof( MAT_E_MSG));

	/******************************/
	/* fee_in set                 */
	/******************************/
	/* get obook  */
	if( !memcmp( obook->SettType, "SWP", 3))			/* SWAP 주문 */
	{
		LogDbg( "SWAP 주문");
		if( obook->TranPtrnCd[ 0] != '1') /* SWAP인경우 무조건 시장가 */
		{
			Mat_SetMessage( mat, obook, 82020);
			MAT_E_MSG_Print( &e_msg);
			ORDER_Print( obook);
			return -1;
		}
		rtn = Mat_SetFeeOrder( mat, obook, fee_in, 2);
		if( rtn < 0)
		{
			Mat_SetMessage( mat, obook, 82031);
			ORDER_Print( obook);
			return -1;
		}
		rtn = Mat_SetFeeOrder( mat, obook, fee_in_far, 3);
		if( rtn < 0)
		{
			Mat_SetMessage( mat, obook, 82031);
			ORDER_Print( obook);
			return -1;
		}
		rtn = Mat_SetFeeSise( mat, index, fee_in);
		rtn = Mat_SetFeeSise( mat, index, fee_in_far);

		if( Param->log_level < 5) 
		{
			LogDbg( "SWAP in parameter check");
			SPLIT_IN_ST_Print( fee_in);
			SPLIT_IN_ST_Print( fee_in_far);
		}
	}
	else											/* 일반 주문 */
	{
		LogDbg( "일반 주문");
		rtn = Mat_SetFeeOrder( mat, obook, fee_in, 1);
		if( rtn < 0)
		{
			Mat_SetMessage( mat, obook, 82031);
			ORDER_Print( obook);
			return -1;
		}
		rtn = Mat_SetFeeSise( mat, index, fee_in);

		if( Param->log_level < 5) 
		{
			LogDbg( "일반 in parameter check");
			SPLIT_IN_ST_Print( fee_in);
		}
	}

	sprintf( e_msg.code, "%d", 82031);
	sprintf( e_msg.mesg, "%s", "수수료 계산중 오류가 발생했습니다.");
	/******************************/
	/* 수수료 Call f_get_cust_prc */
	/******************************/
	if( !memcmp( obook->SettType, "SWP", 3))			/* SWAP 주문 */
	{
		rtn = f_get_cust_swap_prc( fee_in, fee_in_far, fee_out, fee_out_far, &e_msg);
		if( rtn < 0)
		{
			LogCri( "f_get_cust_swap_prc error. rtn=[%d]", rtn);
			Mat_SetEmsg( mat, obook, &e_msg);
			MAT_E_MSG_Print( &e_msg);
			ORDER_Print( obook);
			return -1;
		}
		LogDbg( "f_get_cust_swap_prc success ... rtn=[%d]", rtn);
		if( Param->log_level < 5) 
		{
			LogDbg( "near out");
			SPLIT_OUT_ST_Print( fee_out);
			LogDbg( "far out");
			SPLIT_OUT_ST_Print( fee_out_far);
		}
		/* 수수료 output record 갯수가 0이면 거부 */
		if( fee_out->n_rec_cnt <= 0) return -1;
		if( fee_out_far->n_rec_cnt <= 0) return -1;

		fee_price = fee_out_far->rec[ 0].d_fx_csac_prc;
		LogDel( "Near 고객 마진 가격 =[%12f] 체결가", fee_out->rec[ 0].d_fx_csac_prc);
		LogDel( "Far  고객 마진 가격 =[%12f] 체결가", fee_out_far->rec[ 0].d_fx_csac_prc);
		/* 시장가 -- SWAP 주문은 시장가 밖에 없음: 시세 형성 안되면 오류 */
		if( fee_out->rec[ 0].d_fx_csac_prc <= 0) return -1;
		if( fee_out_far->rec[ 0].d_fx_csac_prc <= 0) return -1;
	}
	else											/* 일반 주문 */
	{
		rtn = f_get_cust_prc( fee_in, fee_out, &e_msg);
		if( rtn < 0)
		{
			LogCri( "f_get_cust_prc error. rtn=[%d]", rtn);
			Mat_SetEmsg( mat, obook, &e_msg);
			MAT_E_MSG_Print( &e_msg);
			ORDER_Print( obook);
			return -1;
		}
		LogDbg( "f_get_cust_prc success ... rtn=[%d]", rtn);
		if( Param->log_level < 5) SPLIT_OUT_ST_Print( fee_out);
		if( fee_out->n_rec_cnt <= 0) return 0;

		fee_price = fee_out->rec[ 0].d_fx_csac_prc;
		if( fee_price <= 0.0)
		{
			LogCri( "수수료 계산 오류!!! rtn=[%d] fee_price=[%f]", rtn, fee_price);
			LogCri( "고객 마진 가격      =[%12f] 체결가", fee_price);
			LogCri( "주문 가격           =[%12f]", obook->Price);
			if( obook->OrdType[ 0] == '1')		/* 시장가 */
			{
				return -1;
			}
			else
			{
				LogCri( "수수료 check ... 지정가 주문 ... 주문 유효");
			}
		}
	}
	Mat_StatisticsSet( mat, MAT_STAT_PEE, MAT_STAT_END);

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @param      int opt  - 0:일반주문 1:SPOT고정주문
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  일반주문 수수료 계산 lib call
***************************************************************************** */
double Mat_GetFee( MAT *mat, MAT_INDEX *index, int side, int pos, int opt)
{
	int			rtn;
	MAT_RECORD	*rec;		/* curr record */
	MAT_HEAD	*head;
	ORDER		*obook;

	SPLIT_IN_ST		*fee_in;
	SPLIT_OUT_ST	*fee_out;
	MAT_E_MSG		e_msg;
	double			fee_price;
	double			price = 0.0;

	Mat_StatisticsSet( mat, MAT_STAT_PEE, MAT_STAT_START);
	rec     = &mat->map->rec[ pos];
	head    = &rec->head;
	fee_in  = &head->fee_in;
	fee_out = &head->fee_out;
	obook   = ( ORDER *)&rec->ord;

	LogDev( "Matching[%s/%s] ... ", mat->map->current[ index->base_cur].str, mat->map->current[ index->cont_cur].str);
	LogDev( "    side=[%d] pos=[%d]", side, pos);
	LogDev( "    price=[%12f] my=[%12f]", price, obook->Price);

	/***************/
	/* 수수료 Call */
	/***************/
	memset( fee_in,                  0x00,              sizeof( SPLIT_IN_ST));
	memset( fee_out,                 0x00,              sizeof( SPLIT_OUT_ST));
	memset( &e_msg,                  0x00,              sizeof( MAT_E_MSG));

	/************************/
	/* fill fee_in by obook */
	/************************/
	rtn = Mat_SetFeeOrder( mat, obook, fee_in, 1 /* 일반주문 */);
	if( rtn < 0)
	{
		return ( double)rtn;
	}
	/* 테너ID - SPOT 고정 주문 때문에 여기서 setting */
	if( opt == 0)	/* 일반 주문 */
	{
		memcpy( fee_in->s_tnr_id,        obook->TnrId,      sizeof( fee_in->s_tnr_id) -1);
		LogDev( "sizeof( obook->SettType)=[%d]", sizeof( obook->SettType) -1);
	}
	else			/* SPOT 고정 주문 */
	{
		memcpy( fee_in->s_tnr_id,        "SPT",      		sizeof( fee_in->s_tnr_id) -1);
		LogDev( "sizeof( obook->SettType)=[%d]", sizeof( obook->SettType) -1);
	}

	/************************/
	/* fill fee_in by sise  */
	/************************/
	rtn = Mat_SetFeeSise( mat, index, fee_in);
	if( rtn < 0)
	{
		return ( double)rtn;
	}

	price = fee_in->d_fx_ordn_prc  = obook->Price;
	LogDev( "price=[%12f]", price);

	LogDev( "index->sise_cont.bidprc =[%12f]", index->sise_cont.bidprc);
	LogDev( "index->sise_cont.askprc =[%12f]", index->sise_cont.askprc);
	LogDev( "index->sise_base.bidprc =[%12f]", index->sise_base.bidprc);
	LogDev( "index->sise_base.askprc =[%12f]", index->sise_base.askprc);
	LogDev( "index->sise_curr.bidprc =[%12f]", index->sise_curr.bidprc);
	LogDev( "index->sise_curr.askprc =[%12f]", index->sise_curr.askprc);

	if( Param->log_level < 5) SPLIT_IN_ST_Print( fee_in);

    sprintf( e_msg.code, "%d", 82031);
    sprintf( e_msg.mesg, "%s", "수수료 계산중 오류가 발생했습니다.");
	/******************************/
	/* 수수료 Call f_get_cust_prc */
	/******************************/
	rtn = f_get_cust_prc( fee_in, fee_out, &e_msg);
	if( rtn < 0)
	{
		LogCri( "f_get_cust_prc error. rtn=[%d]", rtn);
        Mat_SetEmsg( mat, obook, &e_msg);
		MAT_E_MSG_Print( &e_msg);
		if( Param->log_level < 5) ORDER_Print( obook);
		head->error = atoi( e_msg.code);
		return -1.0;
	}
	if( Param->log_level < 5) SPLIT_OUT_ST_Print( fee_out);
	if( fee_out->n_rec_cnt <= 0) return 0;

	fee_price = fee_out->rec[ 0].d_fx_csac_prc;
	LogDev( "pos            =[%d]", pos);
	LogDev( "고객 마진 가격 =[%12f] 체결가", fee_price);
	LogDev( "시세 가격      =[%12f]", price);
	LogDev( "주문 가격      =[%12f]", obook->Price);

	if( fee_price <= 0.0)
	{
		LogCri( "수수료 계산오류!!!. rtn=[%d] fee_price=[%f]", rtn, fee_price);
		LogCri( "pos            =[%d]", pos);
		LogCri( "고객 마진 가격 =[%12f] 체결가", fee_price);
		LogCri( "시세 가격      =[%12f]", price);
		LogCri( "주문 가격      =[%12f]", obook->Price);
		head->error = 82031;
		return -1.0;
	}

	Mat_StatisticsSet( mat, MAT_STAT_PEE, MAT_STAT_END);
	return fee_price;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @param      int opt  - 0:일반주문 1:SPOT고정주문
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  SWAP 주문 수수료 계산 lib call
***************************************************************************** */
double Mat_GetFeeSwap( MAT *mat, MAT_INDEX *index, int side, int pos, int opt)
{
	int			rtn;
	MAT_RECORD	*rec;		/* curr record */
	MAT_HEAD	*head;
	ORDER		*obook;
	/*
	MATSISE		*sise_curr = &index->sise_curr;
	MATSISE		*sise_base = &index->sise_base;
	MATSISE		*sise_cont = &index->sise_cont;
	*/

	SPLIT_IN_ST		*fee_in;
	SPLIT_OUT_ST	*fee_out;
	SPLIT_IN_ST		*fee_in_far;
	SPLIT_OUT_ST	*fee_out_far;
	MAT_E_MSG		e_msg;
	double			fee_price;
	double			fee_price_near, fee_price_far;
	double			price = 0.0;
	double			ord_price_near = 0.0, ord_price_far = 0.0;
	/* double			point;  소수점 처리용 */

	Mat_StatisticsSet( mat, MAT_STAT_PEE, MAT_STAT_START);
	rec         = &mat->map->rec[ pos];
	head        = &rec->head;
	fee_in      = &head->fee_in;
	fee_out     = &head->fee_out;
	fee_in_far  = &head->fee_in_far;
	fee_out_far = &head->fee_out_far;
	obook   = ( ORDER *)&rec->ord;

	if( side == 0)	price = index->sise_cont.bidprc;		/* 매수 */
	else			price = index->sise_cont.askprc;		/* 매도 */
	ord_price_near = obook->NearLegPrice;
	ord_price_far  = obook->FarLegPrice;

	LogDev( "Matching[%s/%s] ... ", mat->map->current[ index->base_cur].str, mat->map->current[ index->cont_cur].str);
	/*
	LogDev( "    sise->id=[%.32s]", sise_curr->id);
	*/
	LogDev( "    side=[%d] pos=[%d]", side, pos);
	LogDev( "    price=[%12f] near=[%12f] far=[%12f]", price, ord_price_near, ord_price_far);

	/***************/
	/* 수수료 Call */
	/***************/
	memset( fee_in,                  0x00,              sizeof( SPLIT_IN_ST));
	memset( fee_out,                 0x00,              sizeof( SPLIT_OUT_ST));
	memset( fee_in_far,              0x00,              sizeof( SPLIT_IN_ST));
	memset( fee_out_far,             0x00,              sizeof( SPLIT_OUT_ST));
	memset( &e_msg,                  0x00,              sizeof( MAT_E_MSG));

	/************************/
	/* fill fee_in by obook */
	/************************/
	rtn = Mat_SetFeeOrder( mat, obook, fee_in, 2 /* SWAP NEAR */);
	if( rtn < 0)
	{
		return ( double)rtn;
	}
	rtn = Mat_SetFeeOrder( mat, obook, fee_in_far, 3 /* SWAP FAR */);
	if( rtn < 0)
	{
		return ( double)rtn;
	}

	/************************/
	/* fill fee_in by sise  */
	/************************/
	rtn = Mat_SetFeeSise( mat, index, fee_in);
	if( rtn < 0)
	{
		return ( double)rtn;
	}
	rtn = Mat_SetFeeSise( mat, index, fee_in_far);
	if( rtn < 0)
	{
		return ( double)rtn;
	}

	fee_in->d_fx_ordn_prc      = obook->NearLegPrice;
	fee_in_far->d_fx_ordn_prc  = obook->FarLegPrice;
	LogDev( "near price=[%12f]", fee_in->d_fx_ordn_prc);
	LogDev( "far  price=[%12f]", fee_in_far->d_fx_ordn_prc);

	LogDev( "index->sise_cont.bidprc =[%12f]", index->sise_cont.bidprc);
	LogDev( "index->sise_cont.askprc =[%12f]", index->sise_cont.askprc);
	LogDev( "index->sise_base.bidprc =[%12f]", index->sise_base.bidprc);
	LogDev( "index->sise_base.askprc =[%12f]", index->sise_base.askprc);
	LogDev( "index->sise_curr.bidprc =[%12f]", index->sise_curr.bidprc);
	LogDev( "index->sise_curr.askprc =[%12f]", index->sise_curr.askprc);

	if( Param->log_level < 5) 
	{
		LogDbg( "SWAP 수수료 in check");
		SPLIT_IN_ST_Print( fee_in);
		SPLIT_IN_ST_Print( fee_in_far);
	}

    sprintf( e_msg.code, "%d", 82031);
    sprintf( e_msg.mesg, "%s", "수수료 계산중 오류가 발생했습니다.");
	/******************************/
	/* 수수료 Call f_get_cust_prc */
	/******************************/
	rtn = f_get_cust_swap_prc( fee_in, fee_in_far, fee_out, fee_out_far, &e_msg);
	if( rtn < 0)
	{
		LogCri( "f_get_cust_prc error. rtn=[%d]", rtn);
#if 1
        Mat_SetEmsg( mat, obook, &e_msg);
#else
        Mat_SetMessage( mat, obook, 82030);
#endif
		MAT_E_MSG_Print( &e_msg);
		if( Param->log_level < 5) ORDER_Print( obook);
		head->error = atoi( e_msg.code);
		return -1.0;
	}
	LogMsg( "f_get_cust_swap_prc call success ... rtn=[%d]", rtn);

	if( Param->log_level < 5) 
	{
		LogDbg( "SWAP 수수료 out check");
		SPLIT_OUT_ST_Print( fee_out);
		SPLIT_OUT_ST_Print( fee_out_far);
	}
	if( fee_out->n_rec_cnt <= 0) return 0;
	if( fee_out_far->n_rec_cnt <= 0) return 0;

	fee_price_near = fee_out->rec[ 0].d_fx_csac_prc;
	fee_price_far  = fee_out_far->rec[ 0].d_fx_csac_prc;
	LogDev( "pos            =[%d]", pos);
	LogDev( "Near 고객 마진 가격 =[%12f] 체결가", fee_price_near);
	LogDev( "Near 시세 가격      =[%12f]", price);
	LogDev( "Near 주문 가격      =[%12f]", ord_price_near);
	LogDev( "Far  고객 마진 가격 =[%12f] 체결가", fee_price_far);
	LogDev( "Far  시세 가격      =[%12f]", price);
	LogDev( "Far  주문 가격      =[%12f]", ord_price_far);

	if( fee_price_near <= 0.0)
	{
		LogCri( "Near 수수료 계산오류!!!. rtn=[%d] fee_price=[%f]", rtn, fee_price_near);
		LogCri( "pos            =[%d]", pos);
		LogCri( "고객 마진 가격 =[%12f] 체결가", fee_price_near);
		LogCri( "시세 가격      =[%12f]", price);
		LogCri( "주문 가격      =[%12f]", ord_price_near);
		head->error = 82031;
		return -1.0;
	}

	if( fee_price_far <= 0.0)
	{
		LogCri( "Far 수수료 계산오류!!!. rtn=[%d] fee_price_far=[%f]", rtn, fee_price_far);
		LogCri( "pos            =[%d]", pos);
		LogCri( "고객 마진 가격 =[%12f] 체결가", fee_price_far);
		LogCri( "시세 가격      =[%12f]", price);
		LogCri( "주문 가격      =[%12f]", ord_price_far);
		head->error = 82031;
		return -1.0;
	}

	obook->NearLegMktPrc = fee_out->rec[ 0].d_mrkt_spt_prc;
	obook->NearLegCvPrc  = fee_out->rec[ 0].d_cvr_spr;
	obook->NearLegCoPrc  = fee_out->rec[ 0].d_sls_spr;
	obook->NearLegCusPrc = fee_out->rec[ 0].d_fx_csac_prc;
	obook->NearLegCusSpr = fee_out->rec[ 0].d_cus_spr;

	obook->FarLegMktPrc  = fee_out_far->rec[ 0].d_mrkt_spt_prc;
	obook->FarLegCvPrc   = fee_out_far->rec[ 0].d_cvr_spr;
	obook->FarLegCoPrc   = fee_out_far->rec[ 0].d_sls_spr;
	obook->FarLegCusPrc  = fee_out_far->rec[ 0].d_fx_csac_prc;
	obook->FarLegCusSpr  = fee_out_far->rec[ 0].d_cus_spr;

	/* 체결가격은 far_swap_point - far_swap_point는 이미 수수료에서 near_swap_point를 빼고 넘겨줌 */
	// fee_price = fee_out_far->rec[ 0].d_mrkt_swap_prc /* - fee_out->rec[ 0].d_mrkt_swap_prc */;
	/* 20250421 변경  - Far고객마진가격 - Near고객마진가격 */
	fee_price = fee_price_far - fee_price_near;
	LogDev( "far price     = [%f]", fee_out_far->rec[ 0].d_mrkt_swap_prc);
	LogDev( "near price    = [%f]", fee_out->rec[ 0].d_mrkt_swap_prc);
	LogDev( "fee_price     = [%f]", fee_price);

	/* 소수점 처리 */
	/* point = exp10( index->point); */
	fee_price = Dfloor( fee_price, index->point);
	LogDev( "fee_price     = [%f]", fee_price);

	Mat_StatisticsSet( mat, MAT_STAT_PEE, MAT_STAT_END);

	return fee_price;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @param      int opt  - 0:일반주문 1:SPOT고정주문
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  SWAP NEAR 주문 수수료 계산 lib call
***************************************************************************** */
double Mat_GetFeeNear( MAT *mat, MAT_INDEX *index, int side, int pos, int opt)
{
	int			rtn;
	MAT_RECORD	*rec;		/* curr record */
	MAT_HEAD	*head;
	ORDER		*obook;
	/*
	MATSISE		*sise_curr = &index->sise_curr;
	MATSISE		*sise_base = &index->sise_base;
	MATSISE		*sise_cont = &index->sise_cont;
	*/

	SPLIT_IN_ST		*fee_in;
	SPLIT_OUT_ST	*fee_out;
	MAT_E_MSG		e_msg;
	double			fee_price;
	double			price = 0.0;
#if 0
#endif

	Mat_StatisticsSet( mat, MAT_STAT_PEE, MAT_STAT_START);
	rec     = &mat->map->rec[ pos];
	head    = &rec->head;
	fee_in  = &head->fee_in;
	fee_out = &head->fee_out;
	obook   = ( ORDER *)&rec->ord;

	LogDev( "Matching[%s/%s] ... ", mat->map->current[ index->base_cur].str, mat->map->current[ index->cont_cur].str);
	/*
	LogDev( "    sise->id=[%.32s]", sise_curr->id);
	*/
	LogDev( "    side=[%d] pos=[%d]", side, pos);
	LogDev( "    price=[%12f] my=[%12f]", price, obook->Price);

	/***************/
	/* 수수료 Call */
	/***************/
	memset( fee_in,                  0x00,              sizeof( SPLIT_IN_ST));
	memset( fee_out,                 0x00,              sizeof( SPLIT_OUT_ST));
	memset( &e_msg,                  0x00,              sizeof( MAT_E_MSG));

	/************************/
	/* fill fee_in by obook */
	/************************/
	rtn = Mat_SetFeeOrder( mat, obook, fee_in, 2 /* SWAP NEAR */);
	if( rtn < 0)
	{
		return ( double)rtn;
	}

	/************************/
	/* fill fee_in by sise  */
	/************************/
	rtn = Mat_SetFeeSise( mat, index, fee_in);
	if( rtn < 0)
	{
		return ( double)rtn;
	}

	price = fee_in->d_fx_ordn_prc  = obook->NearLegPrice;
	LogDev( "price=[%12f]", price);

	LogDev( "index->sise_cont.bidprc =[%12f]", index->sise_cont.bidprc);
	LogDev( "index->sise_cont.askprc =[%12f]", index->sise_cont.askprc);
	LogDev( "index->sise_base.bidprc =[%12f]", index->sise_base.bidprc);
	LogDev( "index->sise_base.askprc =[%12f]", index->sise_base.askprc);
	LogDev( "index->sise_curr.bidprc =[%12f]", index->sise_curr.bidprc);
	LogDev( "index->sise_curr.askprc =[%12f]", index->sise_curr.askprc);

	if( Param->log_level < 5) SPLIT_IN_ST_Print( fee_in);

    sprintf( e_msg.code, "%d", 82031);
    sprintf( e_msg.mesg, "%s", "수수료 계산중 오류가 발생했습니다.");
	/******************************/
	/* 수수료 Call f_get_cust_prc */
	/******************************/
	rtn = f_get_cust_prc( fee_in, fee_out, &e_msg);
	if( rtn < 0)
	{
		LogCri( "f_get_cust_prc error. rtn=[%d]", rtn);
#if 1
        Mat_SetEmsg( mat, obook, &e_msg);
#else
        Mat_SetMessage( mat, obook, 82030);
#endif
		MAT_E_MSG_Print( &e_msg);
		if( Param->log_level < 5) ORDER_Print( obook);
		head->error = atoi( e_msg.code);
		return -1.0;
	}
	if( Param->log_level < 5) SPLIT_OUT_ST_Print( fee_out);
	if( fee_out->n_rec_cnt <= 0) return 0;

	fee_price = fee_out->rec[ 0].d_fx_csac_prc;
	LogDev( "pos            =[%d]", pos);
	LogDev( "고객 마진 가격 =[%12f] 체결가", fee_price);
	LogDev( "시세 가격      =[%12f]", price);
	LogDev( "주문 가격      =[%12f]", obook->Price);

	if( fee_price <= 0.0)
	{
		LogCri( "수수료 계산오류!!!. rtn=[%d] fee_price=[%f]", rtn, fee_price);
		LogCri( "pos            =[%d]", pos);
		LogCri( "고객 마진 가격 =[%12f] 체결가", fee_price);
		LogCri( "시세 가격      =[%12f]", price);
		LogCri( "주문 가격      =[%12f]", obook->Price);
		head->error = 82031;
		return -1.0;
	}
	obook->NearLegMktPrc = fee_out->rec[ 0].d_mrkt_spt_prc;
	obook->NearLegCvPrc  = fee_out->rec[ 0].d_cvr_spr;
	obook->NearLegCoPrc  = fee_out->rec[ 0].d_sls_spr;
	obook->NearLegCusPrc = fee_out->rec[ 0].d_fx_csac_prc;
	obook->NearLegCusSpr = fee_out->rec[ 0].d_cus_spr;

	Mat_StatisticsSet( mat, MAT_STAT_PEE, MAT_STAT_END);
	return fee_price;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @param      int opt  - 0:일반주문 1:SPOT고정주문
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  SWAP NEAR 주문 수수료 계산 lib call
***************************************************************************** */
double Mat_GetFeeFar( MAT *mat, MAT_INDEX *index, int side, int pos, int opt)
{
	int			rtn;
	MAT_RECORD	*rec;		/* curr record */
	MAT_HEAD	*head;
	ORDER		*obook;

	SPLIT_IN_ST		*fee_in;
	SPLIT_OUT_ST	*fee_out;
	MAT_E_MSG		e_msg;
	double			fee_price;
	double			price = 0.0;

	Mat_StatisticsSet( mat, MAT_STAT_PEE, MAT_STAT_START);
	rec      = &mat->map->rec[ pos];
	head     = &rec->head;
	fee_in   = &head->fee_in_far;
	fee_out  = &head->fee_out_far;
	obook    = ( ORDER *)&rec->ord;

	LogDev( "Matching[%s/%s] ... ", mat->map->current[ index->base_cur].str, mat->map->current[ index->cont_cur].str);
	LogDev( "    side=[%d] pos=[%d]", side, pos);
	LogDev( "    price=[%12f] my=[%12f]", price, obook->Price);

	/***************/
	/* 수수료 Call */
	/***************/
	memset( fee_in,                  0x00,              sizeof( SPLIT_IN_ST));
	memset( fee_out,                 0x00,              sizeof( SPLIT_OUT_ST));
	memset( &e_msg,                  0x00,              sizeof( MAT_E_MSG));

	/************************/
	/* fill fee_in by obook */
	/************************/
	rtn = Mat_SetFeeOrder( mat, obook, fee_in, 3 /* SWAP FAR */);
	if( rtn < 0)
	{
		return ( double)rtn;
	}

	/************************/
	/* fill fee_in by sise  */
	/************************/
	rtn = Mat_SetFeeSise( mat, index, fee_in);
	if( rtn < 0)
	{
		return ( double)rtn;
	}

	/* SWAP 주문은 FarLegPrice + Near의 swap point */
	price = fee_in->d_fx_ordn_prc  = obook->FarLegPrice;
	LogDev( "price=[%12f]", price);

	if( Param->log_level < 5) SPLIT_IN_ST_Print( fee_in);

    sprintf( e_msg.code, "%d", 82031);
    sprintf( e_msg.mesg, "%s", "수수료 계산중 오류가 발생했습니다.");
	/******************************/
	/* 수수료 Call f_get_cust_prc */
	/******************************/
	rtn = f_get_cust_prc( fee_in, fee_out, &e_msg);
	if( rtn < 0)
	{
		LogCri( "f_get_cust_prc error. rtn=[%d]", rtn);
        Mat_SetEmsg( mat, obook, &e_msg);
		MAT_E_MSG_Print( &e_msg);
		if( Param->log_level < 5) ORDER_Print( obook);
		head->error = atoi( e_msg.code);
		return -1.0;
	}
	if( Param->log_level < 5) SPLIT_OUT_ST_Print( fee_out);
	if( fee_out->n_rec_cnt <= 0) return 0;

	fee_price = fee_out->rec[ 0].d_fx_csac_prc;
	LogDev( "pos            =[%d]", pos);
	LogDev( "고객 마진 가격 =[%12f] 체결가", fee_price);
	LogDev( "시세 가격      =[%12f]", price);
	LogDev( "주문 가격      =[%12f]", obook->Price);

	if( fee_price <= 0.0)
	{
		LogCri( "수수료 계산오류!!!. rtn=[%d] fee_price=[%f]", rtn, fee_price);
		LogCri( "pos            =[%d]", pos);
		LogCri( "고객 마진 가격 =[%12f] 체결가", fee_price);
		LogCri( "시세 가격      =[%12f]", price);
		LogCri( "주문 가격      =[%12f]", obook->Price);
		head->error = 82031;
		return -1.0;
	}
	obook->FarLegMktPrc = fee_out->rec[ 0].d_mrkt_spt_prc;
	obook->FarLegCvPrc  = fee_out->rec[ 0].d_cvr_spr;
	obook->FarLegCoPrc  = fee_out->rec[ 0].d_sls_spr;
	obook->FarLegCusPrc = fee_out->rec[ 0].d_fx_csac_prc;
	obook->FarLegCusSpr = fee_out->rec[ 0].d_cus_spr;

	Mat_StatisticsSet( mat, MAT_STAT_PEE, MAT_STAT_END);
	return fee_price;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @param      int opt  - 0:일반주문 1:SPOT고정주문
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  SWAP NEAR 주문 수수료 계산 lib call
***************************************************************************** */
int Mat_SetFeeSise( MAT *mat, MAT_INDEX *index, SPLIT_IN_ST *fee_in)
{

	LogDev( "index->base_cur=[%d] index->cont_cur=[%d]", index->base_cur, index->cont_cur);
	if( index->base_cur == 1 && index->cont_cur == 0)	/* 기본통화 - USD/KRW */
	{
		LogDev( "기본통화");
		fee_in->d_bid_usd_prc  = index->sise_curr.bidprc;	/* USD/KRW */
		fee_in->d_ask_usd_prc  = index->sise_curr.askprc;
		fee_in->d_bid_std_prc  = 0.0;
		fee_in->d_ask_std_prc  = 0.0;
		fee_in->d_bid_fnl_prc  = 0.0;
		fee_in->d_ask_fnl_prc  = 0.0;
	}
	else
	if( index->cont_cur == 0)							/* 재정통화 - JPY/KRW */
	{
		LogDev( "재정통화");
		fee_in->d_bid_usd_prc  = index->sise_cont.bidprc;		/* USD/KRW */
		fee_in->d_ask_usd_prc  = index->sise_cont.askprc;
		fee_in->d_bid_std_prc  = index->sise_base.bidprc;		/* USD/JPY */
		fee_in->d_ask_std_prc  = index->sise_base.askprc;
		fee_in->d_bid_fnl_prc  = index->sise_curr.bidprc;		/* JPY/KRW */
		fee_in->d_ask_fnl_prc  = index->sise_curr.askprc;
	}
	else												/* 이종통화 - USD/JPY */
	{
		LogDev( "이종통화");
		fee_in->d_bid_usd_prc  = 0.0;					/* USD/JPY */
		fee_in->d_ask_usd_prc  = 0.0;
		fee_in->d_bid_std_prc  = index->sise_curr.bidprc;
		fee_in->d_ask_std_prc  = index->sise_curr.askprc;
		fee_in->d_bid_fnl_prc  = 0.0;
		fee_in->d_ask_fnl_prc  = 0.0;
	}

	LogDev( "index->sise_cont.bidprc =[%12f]", index->sise_cont.bidprc);
	LogDev( "index->sise_cont.askprc =[%12f]", index->sise_cont.askprc);
	LogDev( "index->sise_base.bidprc =[%12f]", index->sise_base.bidprc);
	LogDev( "index->sise_base.askprc =[%12f]", index->sise_base.askprc);
	LogDev( "index->sise_curr.bidprc =[%12f]", index->sise_curr.bidprc);
	LogDev( "index->sise_curr.askprc =[%12f]", index->sise_curr.askprc);

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  고객번호 CustID(16) + FundNO(12)를 수수료 fee_in->s_csac_idnt_no로 copy
***************************************************************************** */
int Mat_CustCp( MAT *mat, SPLIT_IN_ST *fee_in, ORDER	 *obook)
{
	int		sz;
	char	cust_id[ sizeof( obook->CustID) +1], fund_no[ sizeof( obook->FundNO) +1];
	char	cust_buf[ sizeof( obook->CustID) + sizeof( obook->FundNO) +1];

	memset( cust_buf, 0x20,  sizeof( obook->CustID) + sizeof( obook->FundNO));

    /* 고객번호 CustID(16) + FundNO(12) */
    memcpy( cust_id, obook->CustID, sizeof( obook->CustID));
    memcpy( fund_no, obook->FundNO, sizeof( obook->FundNO));
    TrimNR( cust_id, sizeof( obook->CustID));
    TrimNR( fund_no, sizeof( obook->FundNO));
    sz = sprintf( cust_buf, "%s%s", cust_id, fund_no);
	cust_buf[ sz] = 0x20;
    memcpy( fee_in->s_csac_idnt_no,  cust_buf,     sizeof( fee_in->s_csac_idnt_no) -1);

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @param      int opt  - 1:일반 2:SWAP NEAR 3:SWAP FAR
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  SWAP NEAR 주문 수수료 계산 lib call
***************************************************************************** */
int Mat_SetFeeOrder( MAT *mat, ORDER *obook, SPLIT_IN_ST *fee_in, int opt)
{
	char	symbol[ 16];

	switch( opt)
	{
		case 1:		/* 일반주문 */
			/* 고객번호 CustID(16) + FundNO(12) */
			Mat_CustCp( mat, fee_in, obook);
			/* 원천구분 */
			memcpy( fee_in->s_csac_orgn_gb,  obook->OrgnGb,     sizeof( fee_in->s_csac_orgn_gb) -1);
			/* 통화페어 */
			sprintf( symbol, "%.3s/%.3s", &obook->Symbol[ 0], &obook->Symbol[ 4]);
			memcpy( fee_in->s_pair_id,       symbol,            sizeof( fee_in->s_pair_id) -1);
			/* 상품구분코드 */
			memcpy( fee_in->s_sett_type,     obook->SettType,   sizeof( fee_in->s_sett_type) -1);
			/* 테너유형구분코드(S:표준,U:비표준) */
			memcpy( fee_in->s_tnr_ptrn_dcd,  obook->TnrPtrnDcd, sizeof( fee_in->s_tnr_ptrn_dcd) -1);
			/* 테너ID */
			/* 테너 ID는 나중에 부른 함수에서 setting - 스팟고정주문 때문 */
			/* 매입매도구분코드 */
			memcpy( fee_in->s_bysel_dcd,     obook->Side,       sizeof( fee_in->s_bysel_dcd) -1);
			/* 만기종료년월일 */
			memcpy( fee_in->s_expi_fnsh_ymd, obook->ValueDate2, sizeof( fee_in->s_expi_fnsh_ymd) -1);
			/* 만기시작년월일 */
			memcpy( fee_in->s_expi_sttg_ymd, obook->ValueDate1, sizeof( fee_in->s_expi_sttg_ymd) -1);
			/* 주문가격조건코드 */
			memcpy( fee_in->s_ordn_prc_cncd, obook->OrdType,    sizeof( fee_in->s_ordn_prc_cncd) -1);
    		if( obook->TranPtrnCd[ 0] == '2') /* MAR의 경우 - 시세가 0인경우  수수료에서 전일종가로 계산 */
    		{
        		memcpy( fee_in->s_ordn_prc_cncd, "4",    sizeof( fee_in->s_ordn_prc_cncd) -1);
    		}
			/* 주문 수량 */
			fee_in->d_fx_ordn_amt = obook->OrderQty;
			/* USD 환산 주문 수량 */
			fee_in->d_usd_amt = obook->UsdQty;
			break;
		case 2:		/* SWAP NEAR */
			/* 고객번호 CustID(16) + FundNO(12) */
			Mat_CustCp( mat, fee_in, obook);
			/* 원천구분 */
			memcpy( fee_in->s_csac_orgn_gb,  obook->OrgnGb,           sizeof( fee_in->s_csac_orgn_gb) -1);
			/* 통화페어 */
			sprintf( symbol, "%.3s/%.3s", &obook->Symbol[ 0], &obook->Symbol[ 4]);
			memcpy( fee_in->s_pair_id,       symbol,                  sizeof( fee_in->s_pair_id) -1);
			/* 상품구분코드 */
			memcpy( fee_in->s_sett_type,     obook->NearSettType,     sizeof( fee_in->s_sett_type) -1);
			/* 테너유형구분코드(S:표준,U:비표준) */
			memcpy( fee_in->s_tnr_ptrn_dcd,  obook->NearTnrPtrnDcd,   sizeof( fee_in->s_tnr_ptrn_dcd) -1);
			/* 테너ID */
			memcpy( fee_in->s_tnr_id,        obook->NearTnrId,        sizeof( fee_in->s_tnr_id) -1);
			/* 매입매도구분코드 */
			memcpy( fee_in->s_bysel_dcd,     obook->NearLegSide,      sizeof( fee_in->s_bysel_dcd) -1);
			/* 만기종료년월일 */
			memcpy( fee_in->s_expi_fnsh_ymd, obook->NearLegSettlDate, sizeof( fee_in->s_expi_fnsh_ymd) -1);
			/* 만기시작년월일 */
			memcpy( fee_in->s_expi_sttg_ymd, obook->NearLegSettlDate, sizeof( fee_in->s_expi_sttg_ymd) -1);

			/* 주문가격조건코드 */
			memcpy( fee_in->s_ordn_prc_cncd, obook->OrdType,          sizeof( fee_in->s_ordn_prc_cncd) -1);
    		if( obook->TranPtrnCd[ 0] == '2') /* MAR의 경우 - 시세가 0인경우  수수료에서 전일종가로 계산 */
    		{
        		memcpy( fee_in->s_ordn_prc_cncd, "4",                 sizeof( fee_in->s_ordn_prc_cncd) -1);
    		}
			/* 주문 수량 */
			fee_in->d_fx_ordn_amt = obook->OrderQty;
			/* USD 환산 주문 수량 */
			fee_in->d_usd_amt = obook->UsdQty;
			break;
		case 3:		/* SWAP FAR */
			/* 고객번호 CustID(16) + FundNO(12) */
			Mat_CustCp( mat, fee_in, obook);
			/* 원천구분 */
			memcpy( fee_in->s_csac_orgn_gb,  obook->OrgnGb,           sizeof( fee_in->s_csac_orgn_gb) -1);
			/* 통화페어 */
			sprintf( symbol, "%.3s/%.3s", &obook->Symbol[ 0], &obook->Symbol[ 4]);
			memcpy( fee_in->s_pair_id,       symbol,                  sizeof( fee_in->s_pair_id) -1);
			/* 상품구분코드 */
			memcpy( fee_in->s_sett_type,     obook->FarSettType,      sizeof( fee_in->s_sett_type) -1);
			/* 테너유형구분코드(S:표준,U:비표준) */
			memcpy( fee_in->s_tnr_ptrn_dcd,  obook->FarTnrPtrnDcd,    sizeof( fee_in->s_tnr_ptrn_dcd) -1);
			/* 테너ID */
			memcpy( fee_in->s_tnr_id,        obook->FarTnrId,         sizeof( fee_in->s_tnr_id) -1);
			/* 매입매도구분코드 */
			memcpy( fee_in->s_bysel_dcd,     obook->FarLegSide,       sizeof( fee_in->s_bysel_dcd) -1);
			/* 만기종료년월일 */
			memcpy( fee_in->s_expi_fnsh_ymd, obook->FarLegSettlDate,  sizeof( fee_in->s_expi_fnsh_ymd) -1);
			/* 만기시작년월일 */
			memcpy( fee_in->s_expi_sttg_ymd, obook->FarLegSettlDate,  sizeof( fee_in->s_expi_sttg_ymd) -1);
		
			/* 주문가격조건코드 */
			memcpy( fee_in->s_ordn_prc_cncd, obook->OrdType,          sizeof( fee_in->s_ordn_prc_cncd) -1);
    		if( obook->TranPtrnCd[ 0] == '2') /* MAR의 경우 - 시세가 0인경우  수수료에서 전일종가로 계산 */
    		{
        		memcpy( fee_in->s_ordn_prc_cncd, "4",                 sizeof( fee_in->s_ordn_prc_cncd) -1);
    		}
			/* 주문 수량 */
			fee_in->d_fx_ordn_amt = obook->OrderQty;
			/* USD 환산 주문 수량 */
			fee_in->d_usd_amt = obook->UsdQty;
			break;
		default:
			return -1;
	}

	return 1;
}


