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

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @param      MAT_INDEX *index - index pointer
**  @param      int side - 0=매수,1=매도
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  index에 등록된 주문을 검색하여 매칭
***************************************************************************** */
int Mat_Matching( MAT *mat, int idx_pos)
{
	int			rtn;
	int			pos;
	int			side;
	MAT_HEAD	*head;
	MAT_RECORD	*rec;		/* curr record */
	MAT_INDEX	*index;

	if( ( idx_pos < 0) || ( idx_pos >= MAT_MAX_CURR))
	{
		LogCri( "idx_pos error. idx_pos = [%d]", idx_pos);
		return -1;
	}

	index = &mat->map->index[ idx_pos];
	index->mat_cnt++;
	time( &index->mat_time);

	if( ( index->start[ 0].start_cnt > 0) ||  ( index->start[ 1].start_cnt > 0))
	{
		LogTrc( "Matching ... [%s/%s] bid=[%d] ask=[%d]", 
				mat->map->current[ index->base_cur].str, mat->map->current[ index->cont_cur].str, 
				index->start[ 0].start_cnt, index->start[ 1].start_cnt);
	}
	else
	{
		return 0;
	}

	for( side = 0; side < 2; side++)
	{
		for( pos = index->start[ side].start; pos > 0; pos = head->next)
		{
			rtn = Mat_MatchRecord( mat, index, side, pos);
			if( rtn < 0)
			{
				LogCri( "Mat_MatchRecord error. rtn=[%d]", rtn);
			}
			rec     = &mat->map->rec[ pos];
			head    = &rec->head;
			if( rtn == 0)	continue;

			/* duble linked list 에서 삭제 */
			Mat_RecordUnlink( mat, index, rec, pos, side);
			LogDev( "index->start[ side].start =[%d]", index->start[ side].start);

			LogDev( "Mat_MatchExecute call ...");
			rtn = Mat_MatchExecute( mat, index, rec, pos, side, pos, 0);
			if( rtn < 0)
			{
				LogCri( "Mat_MatchExecute error. rtn=[%d]", rtn);
				return -1;
			}
		}	/* for pos */
	}	/* for side */

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @param      MAT_INDEX *index - index pointer
**  @param      int side - 0=매수,1=매도
**  @return     0 - 매칭 실패
**  @retval     1 - 매칭 성공
**  @brief
**  가격비교하여 매수/매도체결
***************************************************************************** */
int Mat_MatchRecord( MAT *mat, MAT_INDEX *index, int side, int pos)
{
	int			rtn;
	time_t		cur_time;
	MAT_RECORD	*rec;		/* curr record */
	MAT_HEAD	*head;
	ORDER		*obook;
	MATSISE		*sise_curr = &index->sise_curr;
	int			jang_check = 1;

	double			price;				/* 시세 가격 */
	double			fee_price;			/* 수수료 가격 */
	double			book_price = 0.0;	/* 주문 가격 */

	LogDev( "Matching ... [%s/%s] bid=[%d] ask=[%d]", 
			mat->map->current[ index->base_cur].str, mat->map->current[ index->cont_cur].str, 
			index->start[ 0].start_cnt, index->start[ 1].start_cnt);

	rec     = &mat->map->rec[ pos];
	head    = &rec->head;
	obook   = ( ORDER *)&rec->ord;

	/*
	ORDER_Print( obook);
	*/

	/* 월말 매매 가능 시간 check */
	if( mat->map->stat.last_day)
	{
		time( &cur_time);
		if( mat->map->stat.last_time > 0)
		{
			if( cur_time >= mat->map->stat.last_time)
			{
				return 0;
			}
		}
	}

	/* 통화별 장운영 check */
	if( jang_check)
	{
		rtn = Mat_JangCheck( mat, obook, head->jang_id);
		if( rtn < 0)
		{
			LogDev( "pos=[%d] 장시간 아님 ", pos);
			return 0;
		}
		else
		{
			LogDev( "체결 try ... pos=[%d]", pos);
		}
	}

	/* 매수주문 > 매도시세, 매도주문 < 매수시세 */
	if( side == 0)	price = sise_curr->askprc;
	else			price = sise_curr->bidprc;

	if( obook->TranPtrnCd[ 0] != '2') /* MAR의 경우 - 시세가 0인경우  수수료에서 전일종가로 계산 */
	{
		if( price <= 0)
		{
			LogDbg( "시세 없음 ... price=[%f] 미체결", price);
			return 0;
		}
	}
	book_price = obook->Price;

	if( obook->TrdTypeDcd[ 0] == '2') /* SPOT 고정 주문 */
	{
		if( obook->OrdType[ 0] == '1') /* SPOT 고정 주문의 시장가인 경우 */
		{
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
		else	/* SPOT 고정 주문의 지정가인 경우 SpotPrc 가격이 SPOT fee 가격에 충족하는경우 SPOT 가격에 체결 */
		{
#if 0
			fee_price = Mat_GetFee( mat, index, side, pos, 1); /* 선물(FWD)이라도 스팟 가격으로 처리 opt=1 */
			if( fee_price < 0.0)
			{
				head->error = -( int)(fee_price);
				return -82030;
			}
			else
			if( fee_price == 0.0)
			{
				return 0;
			}
			book_price = obook->SpotPrc;
#else
			/* 스팟고정주문은 시장가로만 - 이 루틴을 탈 일은 없어짐 - 일반 주문과 같이 처리함 */
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
			book_price = obook->SpotPrc;
#endif
		}
	}
	else 
	if( obook->TrdTypeDcd[ 0] == '3') /* 대행주문의 경우 SPOT 가격은 주문 가격 그대로 체결 */
	{
		LogDbg( "대행 주문  obook->TrdTypeDcd[ 0]=[%.1s]", obook->TrdTypeDcd);
		fee_price = obook->Price;
		if( !memcmp( obook->SettType, "SWP", 3)) /* SWAP 대행 주문  */
		{
			LogDbg( "SWAP 대행 ");
			obook->NearLegMktPrc = obook->NearLegPrice;
			obook->FarLegMktPrc  = obook->FarLegPrice;
		}
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

	/* 시장가 */
	if( obook->OrdType[ 0] == '1')
	{
		/* 체결가는 head에 세팅후 체결 프로세스에서 처리 */
		head->exe_price = fee_price; /* obook->LastPx = fee_price; */

#if 0
		슬립피지 check는 주문확인(mat_ord)에서 check
		/* 슬립피지허용가격 check */
		if( obook->SlipCmpPrice != 0.0 && obook->SlipPip != 0.0)
		{
			if( fee_price > ( obook->SlipCmpPrice + obook->SlipPip))
			{
				LogMsg( "fee_price              =[%15f]", fee_price);
				LogMsg( "SlipCmpPrice           =[%15f]", obook->SlipCmpPrice);
				LogMsg( "SlipPip                =[%15f]", obook->SlipPip);
				LogMsg( "SlipCmpPrice + SlipPip =[%15f]", obook->SlipCmpPrice + obook->SlipPip);
				head->error = 82012;
				ORDER_Print( obook);
				return -82012;
			}
			if( fee_price < ( obook->SlipCmpPrice - obook->SlipPip))
			{
				LogMsg( "fee_price              =[%15f]", fee_price);
				LogMsg( "SlipCmpPrice           =[%15f]", obook->SlipCmpPrice);
				LogMsg( "SlipPip                =[%15f]", obook->SlipPip);
				LogMsg( "SlipCmpPrice - SlipPip =[%15f]", obook->SlipCmpPrice - obook->SlipPip);
				ORDER_Print( obook);
				head->error = 82012;
				return -82012;
			}
		}
#endif
		LogMsg( "Matching ... [%s/%s] [%d]", mat->map->current[ index->base_cur].str, mat->map->current[ index->cont_cur].str, side);
		LogMsg( "시장가         =[%12f] 체결 pos=[%d]", fee_price, pos);
		return 1;
	}

	/* 매칭 비교 */
	/*
	if( side == 0) LogMsg( "지정가 비교... side=[%d] fee_price=[%f] <= book_price=[%f]", side, fee_price, book_price);
	else           LogMsg( "지정가 비교... side=[%d] fee_price=[%f] >= book_price=[%f]", side, fee_price, book_price);
	*/

	if( ( side == 0) && ( book_price < fee_price)) 
	{
		LogDev( "지정가         =[%12f] < [%12f] 미체결 pos=[%d]", book_price, fee_price, pos);
		return 0;
	}
	if( side == 1 && book_price > fee_price) 
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

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  매칭 체결 처리
**  base_pos    - 체결 수수료 및 DB 처리를 위해 head data 값을 그룹주문에 반영하기 위해
**  group_check - 여러번 수행하는것 방지 ... count에 문제가 생김
***************************************************************************** */
int Mat_MatchExecute( MAT *mat, MAT_INDEX *index, MAT_RECORD *rec, int pos, int side, int base_pos, int group_check)
{
	int			rtn;
	MAT_HEAD	*head;

	MAT_RECORD	*base_rec;
	MAT_HEAD	*base_head;
	/*
	MATSISE		*sise = &index->sise_curr;
	*/

	LogDev( "Mat_MatchExecute ... pos=[%d]", pos);

	head  = &rec->head;

	/* 체결가 set ... 그룹주문은 아직 체결가가 setting 되지 않았으므로 20240116 */
	if( group_check)
	{
		base_rec  = &mat->map->rec[ base_pos];
		base_head = &base_rec->head;
		/* 수수료 */
		head->exe_price = base_head->exe_price;
		memcpy( &head->fee_in,  &base_head->fee_in,  sizeof( SPLIT_IN_ST));
		memcpy( &head->fee_out, &base_head->fee_out, sizeof( SPLIT_OUT_ST));
	}

	LogDev( "체결 전송");

	/* 체결 sise copy */
	memcpy( &head->sise_curr, &index->sise_curr, sizeof( MATSISE) * 3);
	Mat_SetHead( mat, rec, MAT_HEAD_EXECUTE);

	/* 체결가 */
	/* 이미 위에서 setting 
	if( side == 0)	head->exe_price = sise->bidprc;
	else			head->exe_price = sise->askprc;
	*/

	/* record update - gubun을 2로 setting하여 체결 레코드로 표시 */
	rec->head.gubun = 2;

	mat->map->stat.exe_cnt++;
	LogDev( "exe_cnt=[%d]", mat->map->stat.exe_cnt);

	/**** 체결 record position을 pipe에 전송 *****/
	retry:
	LogDev( "write to pipe... fd=[%d] pos=[%d]", mat->exe_fd, pos);
	rtn = write( mat->exe_fd, ( char *)&pos, sizeof( int));
	if( rtn < sizeof( int))
	{
		switch( errno)
		{
			case EINTR:
			case EPIPE:
				LogErr( "pipe write error. retry ... rtn=[%d]", rtn);
				goto retry;
				
		}
		/* pipe full일경우 해소 될때 까지 대기 */
		if( rtn >= 0)
		{
			LogMsg( "pipe가 full입니다. rtn=[%d]", rtn);
			sleep( 1);
			goto retry;
		}
		LogErr( "pipe write error. fd=[%d] rtn=[%d]", mat->exe_fd, rtn);
		return -1;
	}
	LogDev( "pipe write ... rtn=[%d]", rtn);

	LogDev( "execute send ... pos[%d]", pos);

	LogDev( "GROUP ORDER check ... mat_type=[%d]", head->mat_type);

	return pos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  신규 주문 매칭
**  다음 틱 까지 기다려 매칭 되는것을 방지하기 위함
***************************************************************************** */
int Mat_MatchTimeout( MAT *mat)
{
	int			i;
	int			rtn;
	int			pos;
	MAT_HEAD	*head;
	MAT_RECORD	*rec;		/* curr record */
	MAT_INDEX	*index;

	int			side;

	Mat_Lock( mat);

	/*
	LogDev( "Matching ... [%s/%s] count[0]=[%d] count[1]=[%d]", 
			mat->map->current[ index->base_cur].str, mat->map->current[ index->cont_cur].str, 
			index->start[ 0].start_cnt, index->start[ 1].start_cnt);
	*/
	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		index = &mat->map->index[ i];
		if( index->base_cur == 0 && index->cont_cur == 0) continue;

		for( side = 0; side < 2; side++)
		{
			for( pos = index->start[ side].start; pos > 0; pos = head->next)
			{
				rtn = Mat_MatchRecord( mat, index, side, pos);
	
				rec     = &mat->map->rec[ pos];
				head    = &rec->head;

				if( rtn == 0)	continue;
	
				LogDev( "execute ... pos=[%d]", pos);
	
				/* duble linked list 에서 삭제 */
				Mat_RecordUnlink( mat, index, rec, pos, side);
				LogDev( "index->start[ side].start =[%d]", index->start[ side].start);
	
				LogDev( "Mat_MatchExecute call ...");
				rtn = Mat_MatchExecute( mat, index, rec, pos, side, pos, 0);
				if( rtn < 0)
				{
					Mat_Unlock( mat);
					LogCri( "Mat_MatchExecute error. rtn=[%d]", rtn);
					return -1;
				}
			}	/* for pos */
		}	/* for side */
	}	/* for i */

	Mat_Unlock( mat);
	return 0;
}

#if 0
/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  신규 주문 매칭
**  다음 틱 까지 기다려 매칭 되는것을 방지하기 위함
***************************************************************************** */
double Mat_Round( MAT *mat, double value, double point)
{
	double	e;

	e = exp10( point);

	return round( value * e) / e;
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

#if 0
	if( index->sise_cont.bidprc == 0.0)
	{
		fee_in->d_bid_usd_prc  = index->sise_curr.bidprc;
		fee_in->d_ask_usd_prc  = index->sise_curr.askprc;
	}
	else
	{
		fee_in->d_bid_usd_prc  = index->sise_cont.bidprc;
		fee_in->d_ask_usd_prc  = index->sise_cont.askprc;
	}

	if( index->sise_base.bidprc == 0.0)
	{
		fee_in->d_bid_std_prc  = index->sise_curr.bidprc;
		fee_in->d_ask_std_prc  = index->sise_curr.askprc;
	}
	else
	{
		fee_in->d_bid_std_prc  = index->sise_base.bidprc;
		fee_in->d_ask_std_prc  = index->sise_base.askprc;
	}
#endif

	LogDev( "index->sise_cont.bidprc =[%12f]", index->sise_cont.bidprc);
	LogDev( "index->sise_cont.askprc =[%12f]", index->sise_cont.askprc);
	LogDev( "index->sise_base.bidprc =[%12f]", index->sise_base.bidprc);
	LogDev( "index->sise_base.askprc =[%12f]", index->sise_base.askprc);
	LogDev( "index->sise_curr.bidprc =[%12f]", index->sise_curr.bidprc);
	LogDev( "index->sise_curr.askprc =[%12f]", index->sise_curr.askprc);

#if 0
	/* 원본 */
	fee_in->d_bid_usd_prc  = index->sise_cont.bidprc;
	fee_in->d_ask_usd_prc  = index->sise_cont.askprc;
	fee_in->d_bid_std_prc  = index->sise_base.bidprc;
	fee_in->d_ask_std_prc  = index->sise_base.askprc;
	fee_in->d_bid_fnl_prc  = index->sise_curr.bidprc;
	fee_in->d_ask_fnl_prc  = index->sise_curr.askprc;
#endif

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
	double			fee_price_far;
	double			price = 0.0;
	double			price_far = 0.0;
#if 0
#endif

	Mat_StatisticsSet( mat, MAT_STAT_PEE, MAT_STAT_START);
	rec         = &mat->map->rec[ pos];
	head        = &rec->head;
	fee_in      = &head->fee_in;
	fee_out     = &head->fee_out;
	fee_in_far  = &head->fee_in_far;
	fee_out_far = &head->fee_out_far;
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

	if( Param->log_level < 5) SPLIT_IN_ST_Print( fee_in);
	if( Param->log_level < 5) SPLIT_IN_ST_Print( fee_in_far);

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

	if( fee_out->n_rec_cnt <= 0) return 0;
	if( fee_out_far->n_rec_cnt <= 0) return 0;
	if( Param->log_level < 5) SPLIT_OUT_ST_Print( fee_out);
	if( Param->log_level < 5) SPLIT_OUT_ST_Print( fee_out_far);

	fee_price = fee_out_far->rec[ 0].d_fx_csac_prc;
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

	obook->FarLegMktPrc  = fee_out_far->rec[ 0].d_mrkt_spt_prc;
	obook->FarLegCvPrc   = fee_out_far->rec[ 0].d_cvr_spr;
	obook->FarLegCoPrc   = fee_out_far->rec[ 0].d_sls_spr;
	obook->FarLegCusPrc  = fee_out_far->rec[ 0].d_fx_csac_prc;
	obook->FarLegCusSpr  = fee_out_far->rec[ 0].d_cus_spr;

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
			break;
		default:
			return -1;
	}

	return 1;
}

#endif

