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
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  주문 insert,update,delete - mat_rcv main
***************************************************************************** */
int Mat_Order( MAT *mat, ORDER *obook, int opt)
{
	int			rtn;
	int			pos;
	int 		base_cur;
	int			cont_cur;
	time_t		cur_time;

	MAT_INDEX	*index;
	MATSISE		*sise_curr;
	MATSISE		*sise_base;
	MATSISE		*sise_cont;
	int			side;
	int			code;					/* error code */
	double		price = 0.0;
	int			pee_check  = 1;			/* 수수료 체크 */
	int			sise_check = 1;			/* 시세 체크 */
	int			slippage = 0;			/* 슬립피지 체크 */
	int			jang_check = 1;			/* 통화별 장운영 check */
	int			jang_id = -1;			/* 장운영 ID SPC:0 MAR:1 TOD:2 TOM:3 SPT:4 FWD:5 */

	/* 기준통화 */
	base_cur = Mat_GetCurrentInt( mat, &obook->Symbol[ 0]);
	if( base_cur < 0)
	{
		LogMsg( "통화정보가 올바르지 않습니다. base_cur=[%.3s]", &obook->Symbol);
		return -82050;
	}

	/* 상대통화 */
	cont_cur = Mat_GetCurrentInt( mat, &obook->Symbol[ 4]);
	if( cont_cur < 0)
	{
		LogMsg( "통화정보가 올바르지 않습니다. cont_cur=[%.3s]", &obook->Symbol[ 4]);
		return -82050;
	}

	/* search index */
	index = Mat_GetIndex( mat, base_cur, cont_cur);
	if( index == NULL)
	{
		LogMsg( "거래 가능한 통화가 아닙니다. 기준통화=[%d] 상대통화=[%d]", base_cur, cont_cur);
		return -82051;
	}

	LogDel( "opt=[%d] base=[%s:%d] cont=[%s:%d] price=[%12f]", 
			opt, mat->map->current[ base_cur].str, base_cur, mat->map->current[ cont_cur].str, 
			cont_cur, obook->Price);


	/*************************************************/
	/* 시세 check - 시장가의 경우만                  */
	/* 대행주문 RFQ 주문은 시세체크 하지 않는다      */
	/* MAR의 경우는 시세 check를 하지 않는다 - 시세가 없을때는 수수료 계산에서 전일 종가로 거래 */
	/*********************************************** */
	if( obook->OrdType[ 0] == '1')			{ sise_check = 1;	slippage = 1; }		/* 시장가 */
	else									{ sise_check = 0;	slippage = 0; }		/* 지정가 예약가 */
	if( obook->TrdTypeDcd[ 0] == '3')		{ sise_check = 0;	slippage = 0; }		/* RFQ */
	if( obook->TranPtrnCd[ 0] == '2')		{ sise_check = 0;	slippage = 0; }		/* MAR */

	if( obook->MsgType[ 0] == 'F')			jang_check = 0;							/* 취소주문은 장 check를 하지 않음 */

	if( jang_check)
	{
		jang_id = Mat_GetJangId( mat, obook, 0);
		if( jang_id < 0)
		{
			LogCri( "장운영 error. 매매 가능 시간이 아닙니다. jang_id=[%d]", jang_id);
			return jang_id;
		}
		/* 바로환전은 장check만 하고 SPT으로 변경 */
		/* 체결에서는 jang_id로 check 하므로 STOT-TOD로 변경해도 문제 없음 */
		if( !memcmp( obook->SettType, "BAR", 3))
		{
			memcpy( obook->SettType, "SPT", 3);
		}
	}

	if( sise_check)
	{
		/*************************************************/
		/* 시세 유효시간 check - 시장가의 경우만         */
		/*********************************************** */
		sise_curr = &index->sise_curr;
		sise_base = &index->sise_base;
		sise_cont = &index->sise_cont;

		/* 시세 가격 get */
		side = obook->Side[ 0] - '1';
		if( side == 0)	price = sise_curr->askprc;
		else			price = sise_curr->bidprc;

		time( &cur_time);
		if( sise_curr->price_time > 0)
		{
			if( cur_time - sise_curr->ctime >= sise_curr->price_time && sise_curr->ctime != 0)
			{
				/* 시세 유효기간 만료 */
				LogMsg( "현재통화 시세 유효기간이 만료 되었습니다.");
				LogMsg( "cur_time                =[%ld]", cur_time);
				LogMsg( "sise_curr->ctime        =[%ld]", sise_curr->ctime);
				LogMsg( "sise_curr->price_time   =[%ld]", sise_curr->price_time);
				LogMsg( "gap                     =[%ld]", cur_time - sise_curr->ctime);
				LogDel( "##### current");
				MATSISE_Print( sise_curr);
				LogDel( "##### base");
				MATSISE_Print( sise_base);
				LogDel( "##### cont");
				MATSISE_Print( sise_cont);
				return -82011;
			}
		}

		if( sise_base->price_time > 0)
		{
			if( cur_time - sise_base->ctime >= sise_base->price_time && sise_base->ctime != 0)
			{
				/* 시세 유효기간 만료 */
				LogMsg( "기준통화 시세 유효기간이 만료 되었습니다.");
				LogMsg( "cur_time                =[%ld]", cur_time);
				LogMsg( "sise_base->ctime        =[%ld]", sise_base->ctime);
				LogMsg( "sise_base->price_time   =[%ld]", sise_base->price_time);
				LogMsg( "gap                     =[%ld]", cur_time - sise_base->ctime);
				return -82011;
			}
		}

		if( sise_cont->price_time > 0)
		{
			if( cur_time - sise_cont->ctime >= sise_cont->price_time && sise_cont->ctime != 0)
			{
				/* 시세 유효기간 만료 */
				LogMsg( "상대통화 시세 유효기간이 만료 되었습니다.");
				LogMsg( "cur_time                =[%ld]", cur_time);
				LogMsg( "sise_cont->ctime        =[%ld]", sise_cont->ctime);
				LogMsg( "sise_cont->price_time   =[%ld]", sise_cont->price_time);
				LogMsg( "gap                     =[%ld]", cur_time - sise_cont->ctime);
				return -82011;
			}
		}

		/*************************************************/
		/* 시세 형성 check - 시장가의 경우만             */
		/*********************************************** */
		/* 시세가 형성되지 않았을시 시장가 주문은 거부로 */
		if( price <= 0)
		{
			LogMsg( "obook->OrdType[ 0]=[%c](시장가) price=[%f](시세형성 안됨)", obook->OrdType[ 0], price);
			return -82011;
		}
	}	/* sise_chech */

	/*****************************************************/
	/* 수수료 check                                      */
	/* 수수료 check를 하지 않는 경우                     */
	/* 취소주문                                          */
	/* 대행주문 및 RFQ 주문                              */
	/* SPOT 고정 시장가(보류)                            */
	/*************************************************** */
	if( opt == MAT_DELETE)				pee_check = 0;		/* 취소 주문 */
	if( obook->TrdTypeDcd[ 0] == '3')	pee_check = 0;		/* 대행 및 RFQ */
	/*
	if( obook->TrdTypeDcd[ 0] == '2' && obook->OrdType[ 0] == '1')	pee_check = 0;
	*/

	if( pee_check)
	{
		LogMsg( "수수료 정보 check");
		rtn = Mat_CheckFee( mat, index, obook);
		if( rtn < 0)
		{
			LogCri( "Mat_CheckFee error.");
			return rtn;
		}
	}

	/*************************************************/
	/* 슬립피지허용가격 check                        */
	/*********************************************** */
	/* if( obook->OrdType[ 0] == '1') */
	if( slippage)
	{
		if( obook->SlipCmpPrice != 0.0 && obook->SlipPip != 0.0)
		{
			if( price > ( obook->SlipCmpPrice + obook->SlipPip))
			{
				LogCri( "슬립피지허용가격 오류!!!");
				LogMsg( "price                  =[%15f]", price);
				LogMsg( "SlipCmpPrice           =[%15f]", obook->SlipCmpPrice);
				LogMsg( "SlipPip                =[%15f]", obook->SlipPip);
				LogMsg( "SlipCmpPrice + SlipPip =[%15f]", obook->SlipCmpPrice + obook->SlipPip);
				ORDER_Print( obook);
				return -82012;
			}
			if( price < ( obook->SlipCmpPrice - obook->SlipPip))
			{
				LogCri( "슬립피지허용가격 오류!!!");
				LogMsg( "price                  =[%15f]", price);
				LogMsg( "SlipCmpPrice           =[%15f]", obook->SlipCmpPrice);
				LogMsg( "SlipPip                =[%15f]", obook->SlipPip);
				LogMsg( "SlipCmpPrice - SlipPip =[%15f]", obook->SlipCmpPrice - obook->SlipPip);
				ORDER_Print( obook);
				return -82012;
			}
		}
	}

	/*************************************************/
	/* 주문 매칭 (TEST)                              */
	/* 주문수신 후 다음 틱에서 체결되는것을 방지하기 */
	/* 위해 주문 저장전에 매칭 체크를 한번 한다      */
	/*********************************************** */
	/* Mat_OrderMatching( mat, index, obook);        */

	switch( opt)
	{
		case MAT_INSERT:
			pos = Mat_RecordInsert( mat, index, obook, jang_id);
			break;
		case MAT_UPDATE:
			pos = Mat_RecordDelete( mat, index, obook);
			if( pos <= 0)
			{
				LogMsg( "Mat_RecordDelete error. pos=[%d]", pos);
				return -1;
			}
			pos = Mat_RecordInsert( mat, index, obook, jang_id);
			if( pos <= 0)
			{
				LogMsg( "Mat_RecordInsert error. pos=[%d]", pos);
				return -1;
			}
			break;
		case MAT_DELETE:
			pos = Mat_RecordDelete( mat, index, obook);
			if( pos <= 0)
			{
				LogMsg( "Mat_RecordDelete error. pos=[%d]", pos);
				return -1;
			}
			break;
		case MAT_INSERT_GROUP:
			LogDel( "MAT_INSERT_GROUP start");
			pos = Mat_RecordInsertGroup( mat, index, obook, jang_id, 0);
			if( pos <= 0)
			{
				LogMsg( "Mat_RecordInsertGroup error. index=[%p] obook=[%p]", index, obook);
				ORDER_Print( obook);
				return -1;
			}
			break;
		default:
			LogMsg( "unknown option. opt=[%d]", opt);
			break;
	}

	return pos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  주문 insert 
**	무조건 첫번째에 주문 레코드 삽입
**	추후 index가 확정 되면 key에 따라 해당 위치에 삽입
***************************************************************************** */
int Mat_RecordInsert( MAT *mat, MAT_INDEX *index, ORDER	 *obook, int jang_id)
{
	int			rtn;
	int			pos;
	int			side;
	MAT_HEAD	*head;
	MAT_RECORD	*rec;

	char		ord_id[ 128];
	int			id_sz;
	time_t		cur_time;
	struct tm	_tp, *tp = &_tp;

	/* 빈레코드를 찾아 가져올때부터 lock을 해야 두개의 process가 떴을때도 중복이 안생김 */
	Mat_Lock( mat);

	pos = Mat_GetEmptyRecordPos( mat);
	if( pos <= 0)
	{
		LogMsg( "Mat_GetEmptyRecordPos error.");
		goto error;
	}

	/* 접수번호 OrdID 생성 YYYYMMDDHHMMSSPPPPPP */
	time( &cur_time);
	localtime_r( &cur_time, tp);
	id_sz = sprintf( ord_id, "1%04d%02d%02d%02d%02d%02d%06d",
			tp->tm_year +1900,
			tp->tm_mon +1,
			tp->tm_mday,
			tp->tm_hour,
			tp->tm_min,
			tp->tm_sec,
			pos);
	memcpy( obook->OrdID, ord_id, id_sz);
	LogDel( "ord_id       =[%s]", ord_id);
	LogDel( "obook->OrdID =[%.*s]", sizeof( obook->OrdID), obook->OrdID);

	/* 공유메모리 index 테이블 매수(0) 매도(1)로 주문 side 매수('1') 매도('2')를 변환 */
	side = ( int)(obook->Side[0] - '1');
	if( side < 0 || side > 1)
	{
		LogCri( "매수/매도 구분 오류 ... side=[%d]", side);
		return -1;
	}

	rec = &mat->map->rec[ pos];
	memset( rec, 0x00, sizeof( MAT_RECORD));
	rec->pos = pos;
	head = &rec->head;
	memcpy( &rec->ord, obook, sizeof( ORDER	));
	head->gubun = 1;
	head->ord_stat = 1;
	head->idx_no = index->no;
	head->jang_id = jang_id;
	head->price = obook->Price;
	Mat_MakeHead( mat, head, obook);
	/* 주문 접수시간 - 통계에서 copy 한다. 대기시간이 있으므로 정확힌 접수 시간을 알기위함 */
	memcpy( &head->rcv_time, &mat->map->stat.statis[ MAT_STAT_RCV].end, sizeof( struct timeval));
	/* gettimeofday( &head->rcv_time, NULL); */

	rtn = Mat_Insert( mat, &index->start[ side], pos);

	Mat_Unlock( mat);

	/* pipe write */
#if 0
	rtn = write( mat->ord_fd, &pos, sizeof( int));
	if( rtn < 0)
	{
		LogErr( "pipe write error. fd=[%d] rtn=[%d]", mat->ord_fd, rtn);
	}
#else
	rtn = write( mat->mat_fd, &index->no, sizeof( int));
	if( rtn < sizeof( int))
	{
		LogErr( "pipe write error. fd=[%d] rtn=[%d]", mat->mat_fd, rtn);
	}
	LogDbg( "send to pipe. idx_pos=[%d]", index->no);
#endif
	return pos;

	error:
		Mat_Unlock( mat);
		return -1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  주문 delete 
***************************************************************************** */
int Mat_RecordDelete( MAT *mat, MAT_INDEX *index, ORDER	 *obook)
{
	int			rtn;
	int			pos;
	int			side;
	MAT_RECORD	*rec;

	Mat_StatisticsSet( mat, MAT_STAT_CAN, MAT_STAT_START);
	Mat_Lock( mat);

	/* 정정/취소 주문은 매수/매도, 통화코드가 같아야 하므로 같은 index 내에서 원 주문을 찾는다. */
	side = ( int)(obook->Side[0] - '1');

	/* 삭제할 주문 찾기 */
	pos = Mat_FindRecordByOrigClOrdID( mat, index, obook);
	if( pos <= 0)
	{
		LogMsg( "원주문이 없습니다. OrigClOrdID=[%.*s]", sizeof( obook->OrigClOrdID), obook->OrigClOrdID);
		goto error;
	}

	rec  = &mat->map->rec[ pos];
	rec->head.gubun = 0;
	/* 삭제 record가 취소에 의한 것인지 정정에 의한것인지 표시 */
	switch( obook->MsgType[ 0])
	{
		case 'G':	/* 정정 */
			rec->head.ord_stat = 5;
			break;
		default:
		case 'F':	/* 취소 */
			rec->head.ord_stat = 3;
			break;
	}

	rtn = Mat_Delete( mat, &index->start[ side], pos);
	if( rtn < 0)
	{
		LogCri( "Mat_Delete error. rtn=[%d] cur=[%d%d] side=[%d] pos=[%d]", 
				rtn, index->base_cur, index->cont_cur, side, pos);
		goto error;
	}

	/* 삭제시 clear 하지 않고 gubun만 0으로 set하고 insert 할때 clear 
	memset( rec, 0x00, sizeof( MAT_RECORD));
	*/

	Mat_Unlock( mat);
	Mat_StatisticsSet( mat, MAT_STAT_CAN, MAT_STAT_END);
	return pos;

	error:
		Mat_Unlock( mat);
		return -1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @param      int opt  - 1 = 그룹주문의 마지막 주문이 거부일때
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  그룹주문 insert 
**  그룹 주문 첫번째 record는 index의 wait의 첫번째에 등록
**  마지막 record가 들어 오면 index의 start에 등록
***************************************************************************** */
int Mat_RecordInsertGroup( MAT *mat, MAT_INDEX *index, ORDER *obook, int jang_id, int opt)
{
	int			rtn, err_no;
	int			pos, grp_pos;			/* record position, next record position */
	int			side;					/* 매수=0 / 매도=1 */
	int			idx_no = -1;			/* pipe send index_no */
	MAT_GROUP	_grp, *grp = &_grp;		/* group buffer */
	MAT_GROUP	*gp;					/* shm group pointer */
	MAT_RECORD	*rec;					/* record pointer, next record pointer */
	MAT_HEAD	*head;					/* head pointer, next head pointer */

	MAT_INDEX	*gi;					/* group index pointer */
	MAT_START	*gs;					/* group start pointer */

	/* for order_rd */
	time_t		cur_time;
	struct tm	_tp, *tp = &_tp;
	char		ord_id[ 128];
	int			id_sz;

	/* 빈레코드를 찾아 가져올때부터 lock을 해야 두개의 process가 떴을때도 중복이 안생김 */
	Mat_Lock( mat);

	/* 거부주문의 마지막 record - 주문을 insert 하지 않고 저장한 주문은 index로 이동 */
	if( opt == 1)
	{
		grp->seq = AtoI( obook->GrpOrdnSeq, sizeof( obook->GrpOrdnSeq));
		grp->tot = AtoI( obook->GrpOrdnCnt, sizeof( obook->GrpOrdnCnt));
		memcpy( grp->id, obook->GrpOrdnNo,  sizeof( obook->GrpOrdnNo));

		/* group 주문 찾기 */
		rtn = Mat_FindGroup( mat, grp, 1);
		if( rtn < 0)
		{
			LogCri( "그룹주문번호 없음");
			MAT_GROUP_Print( grp);
			err_no = -82009;
			goto error;
		}
		gp = &mat->map->grp[ rtn];

		goto last_order;
	}

	pos = Mat_GetEmptyRecordPos( mat);
	if( pos <= 0)
	{
		LogMsg( "Mat_GetEmptyRecordPos error.");
		goto error;
	}
	LogDel( "Group insert. pos=[%d]", pos);

	/* 접수번호 OrdID 생성 YYYYMMDDHHMMSSPPPPPP */
	time( &cur_time);
	localtime_r( &cur_time, tp);
	id_sz = sprintf( ord_id, "1%04d%02d%02d%02d%02d%02d%06d",
			tp->tm_year +1900,
			tp->tm_mon +1,
			tp->tm_mday,
			tp->tm_hour,
			tp->tm_min,
			tp->tm_sec,
			pos);
	memcpy( obook->OrdID, ord_id, id_sz);
	LogDel( "ord_id       =[%s]", ord_id);

	LogDel( "obook->OrdID =[%.*s]", sizeof( obook->OrdID), obook->OrdID);
	/* 공유메모리 index 테이블 매수(0) 매도(1)로 주문 side 매수('1') 매도('2')를 변환 */
	side = ( int)(obook->Side[0] - '1');
	if( side < 0 || side > 1)
	{
		LogCri( "매수/매도 구분 오류 ... side=[%d]", side);
		err_no = -82101;
		goto error;
	}

	rec = &mat->map->rec[ pos];
	head = &rec->head;
	rec->pos = pos;
	memcpy( ( char *)&rec->ord, obook, sizeof( ORDER	));
	Mat_SetHead( mat, rec, MAT_HEAD_ORDER);

	head->mat_type = 2;	/* 그룹주문 매칭 타입 */
	head->ord_stat = 6;
	head->idx_no = index->no;
	head->jang_id = jang_id;
	head->price = obook->Price;

	/* 주문 접수시간 - 통계에서 copy 한다. 대기시간이 있으므로 정확힌 접수 시간을 알기위함 */
	memcpy( &head->rcv_time, &mat->map->stat.statis[ MAT_STAT_RCV].end, sizeof( struct timeval));
	/* gettimeofday( &head->rcv_time, NULL); */

	grp->seq = AtoI( obook->GrpOrdnSeq, sizeof( obook->GrpOrdnSeq));
	grp->tot = AtoI( obook->GrpOrdnCnt, sizeof( obook->GrpOrdnCnt));
	memcpy( grp->id, obook->GrpOrdnNo,  sizeof( obook->GrpOrdnNo));

	LogDel( "GroupOrder seq/tot=[%d/%d]", grp->seq, grp->tot);


	/* 그룹 주문번호 찾기 */
	if( grp->seq == 1)		/* 첫번째 주문 */
	{
		LogDel( "group order new pos=[%d]", pos);
		rtn = Mat_FindGroup( mat, grp, 0);
		if( rtn < 0)
		{
			LogCri( "그룹 record를 저장할 공간이 없습니다.");
			MAT_GROUP_Print( grp);
			err_no = -82061;
			goto error;
		}
		gp = &mat->map->grp[ rtn];

		head->grp_next = -1;
		memset( gp, 0x00, sizeof( MAT_GROUP));
		gp->start = pos;
		memcpy( gp->id, grp->id, sizeof( grp->id));
		gp->tot = grp->tot;
		gp->seq = grp->seq;
		gp->cnt++;
	}
	else
	{
		/* 그룹주문 처음에 추가 */
		rtn = Mat_FindGroup( mat, grp, 1);
		if( rtn < 0)
		{
			LogCri( "그룹주문번호 없음");
			MAT_GROUP_Print( grp);
			err_no = -82009;
			goto error;
		}
		gp = &mat->map->grp[ rtn];

		head->grp_next = gp->start;
		gp->start = pos;
		gp->seq   = grp->seq;
		gp->cnt++;
	}

	last_order:
	if( grp->seq == grp->tot)		/* 마지막 주문 */
	{
#if 0
		그룹주문의 거부 주문이 있을 수 있으므로 갯수 check는 무시
		if( gp->cnt != gp->tot)
		{
			LogCri( "그룹 주문 갯수가 맞지 않습니다.");
			MAT_GROUP_Print( gp);
			err_no = -82062;
			goto error;
		}
#endif

		grp_pos = gp->start;
		while( grp_pos >= 0)
		{
			rec  = &mat->map->rec[ grp_pos];
			head = &rec->head;
			side = ( int)(obook->Side[0] - '1');
			gi   = &mat->map->index[ head->idx_no];
			gs   = &gi->start[ side];

			gettimeofday( &head->ord_time, NULL);
			LogDel( "그룹주문 INSERT ... gp->id=[%.11s] grp_pos=[%d]", gp->id, grp_pos);
			Mat_Insert( mat, gs, grp_pos);

			/* pipe write */
			/* 그룹주문 각각이 다른 index 일 수 있으므로 loop 돌면서 idx_pos를 mat_mat에 send */
			if( idx_no != head->idx_no)
			{
				rtn = write( mat->mat_fd, &head->idx_no, sizeof( int));
				if( rtn < sizeof( int))
				{
					LogErr( "pipe write error. fd=[%d] rtn=[%d]", mat->mat_fd, rtn);
				}
				LogDbg( "send to pipe. idx_pos=[%d]", head->idx_no);
				idx_no = head->idx_no;
			}

			grp_pos = head->grp_next;
		}
		gp->start = -1;
	}

	/* 그룹주문은 한번에 체결 시켜야 하기 때문에 신호를 보낸후 락 해제 - 상관 없을듯 */
	Mat_Unlock( mat);

#if 0
	/* pipe write */
	rtn = write( mat->mat_fd, &index->no, sizeof( int));
	if( rtn < sizeof( int))
	{
		LogErr( "pipe write error. fd=[%d] rtn=[%d]", mat->mat_fd, rtn);
	}
	LogDbg( "send to pipe. idx_pos=[%d]", index->no);
#endif

	return pos;

	error:
		Mat_Unlock( mat);
		return err_no;
}

#if 0
/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  주문 delete 
***************************************************************************** */
int Mat_CheckFee( MAT *mat, MAT_INDEX *index, ORDER	 *obook)
{
	int				rtn;

    SPLIT_IN_ST     _pee_in, *pee_in = &_pee_in;
    SPLIT_OUT_ST    _pee_out, *pee_out = &_pee_out;
    MAT_E_MSG       e_msg;
    char            symbol[ 16];
    char            cust_id[ sizeof( obook->CustID) +1], fund_no[ sizeof( obook->FundNO) +1];
	char			cust_buf[ sizeof( obook->CustID) + sizeof( obook->FundNO) +1];
    double          price = 0.0, pee_price = 0.0;

	Mat_StatisticsSet( mat, MAT_STAT_PEE, MAT_STAT_START);
	memset( &e_msg, 0x00, sizeof( MAT_E_MSG));

	/**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**************/
	/* 수수료 Call                                                                  */
	/* 수수료쪽 체결 error가 나는것을 방지하기위해 사전에 call하여 오류 여부를 판단 */
	/************++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++****/
	memset( pee_in,                  0x00,              sizeof( SPLIT_IN_ST));
	memset( pee_out,                 0x00,              sizeof( SPLIT_OUT_ST));
	memset( &e_msg,                  0x00,              sizeof( MAT_E_MSG));

	/* 고객번호 CustID(16) + FundNO(12) */
	Mat_CustCp( mat, pee_in, obook);
	/* 원천구분 */
	memcpy( pee_in->s_csac_orgn_gb,  obook->OrgnGb,     sizeof( pee_in->s_csac_orgn_gb) -1);
	/* 통화페어 */
	sprintf( symbol, "%.3s/%.3s", &obook->Symbol[ 0], &obook->Symbol[ 4]);
	memcpy( pee_in->s_pair_id,       symbol,            sizeof( pee_in->s_pair_id) -1);
	if( !memcmp( obook->SettType, "SWP", 3))
	{
		/* 상품구분코드 */
		memcpy( pee_in->s_sett_type,     obook->NearSettType,	sizeof( pee_in->s_sett_type) -1);
		/* 테너유형구분코드(S:표준,U:비표준) */
		memcpy( pee_in->s_tnr_ptrn_dcd,  obook->NearTnrPtrnDcd, sizeof( pee_in->s_tnr_ptrn_dcd) -1);
		/* 테너ID */
		memcpy( pee_in->s_tnr_id,        obook->NearTnrId,      sizeof( pee_in->s_tnr_id) -1);
		LogDel( "sizeof( obook->NearSettType)=[%d]", sizeof( obook->NearSettType) -1);
		/* 매입매도구분코드 */
		memcpy( pee_in->s_bysel_dcd,     obook->NearLegSide,    sizeof( pee_in->s_bysel_dcd) -1);
		/* 만기종료년월일 */
		memcpy( pee_in->s_expi_fnsh_ymd, obook->NearLegSettlDate, sizeof( pee_in->s_expi_fnsh_ymd) -1);
		/* 만기시작년월일 */
		memcpy( pee_in->s_expi_sttg_ymd, obook->NearLegSettlDate, sizeof( pee_in->s_expi_sttg_ymd) -1);
		/* 주문가격조건코드 */
		memcpy( pee_in->s_ordn_prc_cncd, obook->OrdType,    sizeof( pee_in->s_ordn_prc_cncd) -1);
		if( obook->TranPtrnCd[ 0] != '1') /* SWAP인경우 무조건 시장가 */
		{
			Mat_SetMessage( mat, obook, 82020);
			MAT_E_MSG_Print( &e_msg);
			ORDER_Print( obook);
			return -1;
		}
	}
	else
	{
		/* 상품구분코드 */
		memcpy( pee_in->s_sett_type,     obook->SettType,   sizeof( pee_in->s_sett_type) -1);
		/* 테너유형구분코드(S:표준,U:비표준) */
		memcpy( pee_in->s_tnr_ptrn_dcd,  obook->TnrPtrnDcd, sizeof( pee_in->s_tnr_ptrn_dcd) -1);
		/* 테너ID */
		memcpy( pee_in->s_tnr_id,        obook->TnrId,      sizeof( pee_in->s_tnr_id) -1);
		LogDel( "sizeof( obook->SettType)=[%d]", sizeof( obook->SettType) -1);
		/* 매입매도구분코드 */
		memcpy( pee_in->s_bysel_dcd,     obook->Side,       sizeof( pee_in->s_bysel_dcd) -1);
		/* 만기종료년월일 */
		memcpy( pee_in->s_expi_fnsh_ymd, obook->ValueDate2, sizeof( pee_in->s_expi_fnsh_ymd) -1);
		/* 만기시작년월일 */
		memcpy( pee_in->s_expi_sttg_ymd, obook->ValueDate1, sizeof( pee_in->s_expi_sttg_ymd) -1);
		/* 주문가격조건코드 */
		memcpy( pee_in->s_ordn_prc_cncd, obook->OrdType,    sizeof( pee_in->s_ordn_prc_cncd) -1);
		if( obook->TranPtrnCd[ 0] == '2') /* MAR의 경우 - 시세가 0인경우  수수료에서 전일종가로 계산 */
		{
			memcpy( pee_in->s_ordn_prc_cncd, "4",    sizeof( pee_in->s_ordn_prc_cncd) -1);
		}
	}
#if 0
	else
	{
		/* MAR가 아닌경우 기준통화와 상대통화 중 하나라도 시세가 형성되지 않으면 체결 금지 */
		if( index->sise_cont.bidprc == 0.0) return 0.0;
		if( index->sise_cont.askprc == 0.0) return 0.0;

		if( index->sise_base.bidprc == 0.0) return 0.0;
		if( index->sise_base.askprc == 0.0) return 0.0;
	}
#endif


#if 0
	pee_in->d_fx_ordn_prc  = obook->Price;

	if( index->sise_cont.bidprc == 0.0)
	{
		pee_in->d_bid_usd_prc  = index->sise_curr.bidprc;
		pee_in->d_ask_usd_prc  = index->sise_curr.askprc;
	}
	else
	{
		pee_in->d_bid_usd_prc  = index->sise_cont.bidprc;
		pee_in->d_ask_usd_prc  = index->sise_cont.askprc;
	}
	if( index->sise_base.bidprc == 0.0)
	{
		pee_in->d_bid_std_prc  = index->sise_curr.bidprc;
		pee_in->d_ask_std_prc  = index->sise_curr.askprc;
	}
	else
	{
		pee_in->d_bid_std_prc  = index->sise_base.bidprc;
		pee_in->d_ask_std_prc  = index->sise_base.askprc;
	}
#endif

    LogDev( "index->base_cur=[%d] index->cont_cur=[%d]", index->base_cur, index->cont_cur);
    if( index->base_cur == 1 && index->cont_cur == 0)   /* 기본통화 - USD/KRW */
    {
        LogDev( "기본통화");
        pee_in->d_bid_usd_prc  = index->sise_curr.bidprc;   /* USD/KRW */
        pee_in->d_ask_usd_prc  = index->sise_curr.askprc;
        pee_in->d_bid_std_prc  = 0.0;
        pee_in->d_ask_std_prc  = 0.0;
        pee_in->d_bid_fnl_prc  = 0.0;
        pee_in->d_ask_fnl_prc  = 0.0;
    }
    else
    if( index->cont_cur == 0)                           /* 재정통화 - JPY/KRW */
    {
        LogDev( "재정통화");
        pee_in->d_bid_usd_prc  = index->sise_cont.bidprc;       /* USD/KRW */
        pee_in->d_ask_usd_prc  = index->sise_cont.askprc;
        pee_in->d_bid_std_prc  = index->sise_base.bidprc;       /* USD/JPY */
        pee_in->d_ask_std_prc  = index->sise_base.askprc;
        pee_in->d_bid_fnl_prc  = index->sise_curr.bidprc;       /* JPY/KRW */
        pee_in->d_ask_fnl_prc  = index->sise_curr.askprc;
    }
    else                                                /* 이종통화 - USD/JPY */
    {
        LogDev( "이종통화");
        pee_in->d_bid_usd_prc  = 0.0;                   /* USD/JPY */
        pee_in->d_ask_usd_prc  = 0.0;
        pee_in->d_bid_std_prc  = index->sise_curr.bidprc;
        pee_in->d_ask_std_prc  = index->sise_curr.askprc;
        pee_in->d_bid_fnl_prc  = 0.0;
        pee_in->d_ask_fnl_prc  = 0.0;
    }

    LogDev( "index->sise_cont.bidprc =[%12f]", index->sise_cont.bidprc);
    LogDev( "index->sise_cont.askprc =[%12f]", index->sise_cont.askprc);
    LogDev( "index->sise_base.bidprc =[%12f]", index->sise_base.bidprc);
    LogDev( "index->sise_base.askprc =[%12f]", index->sise_base.askprc);
    LogDev( "index->sise_curr.bidprc =[%12f]", index->sise_curr.bidprc);
    LogDev( "index->sise_curr.askprc =[%12f]", index->sise_curr.askprc);

#if 0
	/* 원본 */
	pee_in->d_bid_usd_prc  = index->sise_cont.bidprc;
	pee_in->d_ask_usd_prc  = index->sise_cont.askprc;
	pee_in->d_bid_std_prc  = index->sise_base.bidprc;
	pee_in->d_ask_std_prc  = index->sise_base.askprc;
	pee_in->d_bid_fnl_prc  = index->sise_curr.bidprc;
	pee_in->d_ask_fnl_prc  = index->sise_curr.askprc;
#endif

	if( Param->log_level < 5) SPLIT_IN_ST_Print( pee_in);

	sprintf( e_msg.code, "%d", 82031);
	sprintf( e_msg.mesg, "%s", "수수료 계산중 오류가 발생했습니다.");
	/******************************/
	/* 수수료 Call f_get_cust_prc */
	/******************************/
	rtn = f_get_cust_prc( pee_in, pee_out, &e_msg);
	if( rtn < 0)
	{
		LogCri( "f_get_cust_prc error. rtn=[%d]", rtn);
#if 1
		Mat_SetEmsg( mat, obook, &e_msg);
#else
		Mat_SetMessage( mat, obook, 82030);
#endif
		MAT_E_MSG_Print( &e_msg);
		ORDER_Print( obook);
		return -1;
	}
	if( Param->log_level < 5) SPLIT_OUT_ST_Print( pee_out);
	if( pee_out->n_rec_cnt <= 0) return 0;

	pee_price = pee_out->rec[ 0].d_fx_csac_prc;
	LogDel( "고객 마진 가격 =[%12f] 체결가", pee_price);
	LogDel( "시세 가격      =[%12f]", price);
	LogDel( "주문 가격      =[%12f]", obook->Price);
	Mat_StatisticsSet( mat, MAT_STAT_PEE, MAT_STAT_END);

	if( pee_price <= 0.0)
	{
		LogCri( "수수료 계산 오류!!! rtn=[%d] pee_price=[%f]", rtn, pee_price);
		LogCri( "고객 마진 가격      =[%12f] 체결가", pee_price);
		LogCri( "시세 가격           =[%12f]", price);
		LogCri( "주문 가격           =[%12f]", obook->Price);
		if( obook->OrdType[ 0] == '1')		/* 시장가 */
		{
			return -1;
		}
		else
		{
			LogCri( "지정가 주문 ... 주문 유효");
		}
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  주문 delete 
***************************************************************************** */
int Mat_CheckFeeFar( MAT *mat, MAT_INDEX *index, ORDER	 *obook)
{
	int				rtn;

    SPLIT_IN_ST     _pee_in, *pee_in = &_pee_in;
    SPLIT_OUT_ST    _pee_out, *pee_out = &_pee_out;
    MAT_E_MSG       e_msg;
    char            symbol[ 16];
    double          price = 0.0, pee_price = 0.0;

	Mat_StatisticsSet( mat, MAT_STAT_PEE, MAT_STAT_START);
	memset( &e_msg, 0x00, sizeof( MAT_E_MSG));

	/**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**************/
	/* 수수료 Call                                                                  */
	/* 수수료쪽 체결 error가 나는것을 방지하기위해 사전에 call하여 오류 여부를 판단 */
	/************++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++****/
	memset( pee_in,                  0x00,              sizeof( SPLIT_IN_ST));
	memset( pee_out,                 0x00,              sizeof( SPLIT_OUT_ST));
	memset( &e_msg,                  0x00,              sizeof( MAT_E_MSG));

#if 0
	/* 고객번호 */
	memcpy( pee_in->s_csac_idnt_no,  obook->CustID,     sizeof( pee_in->s_csac_idnt_no) -1);
#endif
	/* 고객번호 CustID(16) + FundNO(12) */
	Mat_CustCp( mat, pee_in, obook);
	/* 원천구분 */
	memcpy( pee_in->s_csac_orgn_gb,  obook->OrgnGb,     sizeof( pee_in->s_csac_orgn_gb) -1);
	/* 통화페어 */
	sprintf( symbol, "%.3s/%.3s", &obook->Symbol[ 0], &obook->Symbol[ 4]);
	memcpy( pee_in->s_pair_id,       symbol,            sizeof( pee_in->s_pair_id) -1);
	if( !memcmp( obook->SettType, "SWP", 3))
	{
		/* 상품구분코드 */
		memcpy( pee_in->s_sett_type,     obook->FarSettType,	sizeof( pee_in->s_sett_type) -1);
		/* 테너유형구분코드(S:표준,U:비표준) */
		memcpy( pee_in->s_tnr_ptrn_dcd,  obook->FarTnrPtrnDcd, sizeof( pee_in->s_tnr_ptrn_dcd) -1);
		/* 테너ID */
		memcpy( pee_in->s_tnr_id,        obook->FarTnrId,      sizeof( pee_in->s_tnr_id) -1);
		LogDel( "sizeof( obook->FarSettType)=[%d]", sizeof( obook->FarSettType) -1);
		/* 매입매도구분코드 */
		memcpy( pee_in->s_bysel_dcd,     obook->FarLegSide,    sizeof( pee_in->s_bysel_dcd) -1);
		/* 만기종료년월일 */
		memcpy( pee_in->s_expi_fnsh_ymd, obook->FarLegSettlDate, sizeof( pee_in->s_expi_fnsh_ymd) -1);
		/* 만기시작년월일 */
		memcpy( pee_in->s_expi_sttg_ymd, obook->FarLegSettlDate, sizeof( pee_in->s_expi_sttg_ymd) -1);
		/* 주문가격조건코드 */
		memcpy( pee_in->s_ordn_prc_cncd, obook->OrdType,    sizeof( pee_in->s_ordn_prc_cncd) -1);
		if( obook->TranPtrnCd[ 0] != '1') /* SWAP인경우 무조건 시장가 */
		{
			Mat_SetMessage( mat, obook, 82020);
			MAT_E_MSG_Print( &e_msg);
			ORDER_Print( obook);
			return -1;
		}
	}
	else
	{
		/* 상품구분코드 */
		memcpy( pee_in->s_sett_type,     obook->SettType,   sizeof( pee_in->s_sett_type) -1);
		/* 테너유형구분코드(S:표준,U:비표준) */
		memcpy( pee_in->s_tnr_ptrn_dcd,  obook->TnrPtrnDcd, sizeof( pee_in->s_tnr_ptrn_dcd) -1);
		/* 테너ID */
		memcpy( pee_in->s_tnr_id,        obook->TnrId,      sizeof( pee_in->s_tnr_id) -1);
		LogDel( "sizeof( obook->SettType)=[%d]", sizeof( obook->SettType) -1);
		/* 매입매도구분코드 */
		memcpy( pee_in->s_bysel_dcd,     obook->Side,       sizeof( pee_in->s_bysel_dcd) -1);
		/* 만기종료년월일 */
		memcpy( pee_in->s_expi_fnsh_ymd, obook->ValueDate2, sizeof( pee_in->s_expi_fnsh_ymd) -1);
		/* 만기시작년월일 */
		memcpy( pee_in->s_expi_sttg_ymd, obook->ValueDate1, sizeof( pee_in->s_expi_sttg_ymd) -1);
		/* 주문가격조건코드 */
		memcpy( pee_in->s_ordn_prc_cncd, obook->OrdType,    sizeof( pee_in->s_ordn_prc_cncd) -1);
		if( obook->TranPtrnCd[ 0] == '2') /* MAR의 경우 - 시세가 0인경우  수수료에서 전일종가로 계산 */
		{
			memcpy( pee_in->s_ordn_prc_cncd, "4",    sizeof( pee_in->s_ordn_prc_cncd) -1);
		}
	}
#if 0
	else
	{
		/* MAR가 아닌경우 기준통화와 상대통화 중 하나라도 시세가 형성되지 않으면 체결 금지 */
		if( index->sise_cont.bidprc == 0.0) return 0.0;
		if( index->sise_cont.askprc == 0.0) return 0.0;

		if( index->sise_base.bidprc == 0.0) return 0.0;
		if( index->sise_base.askprc == 0.0) return 0.0;
	}
#endif


#if 0
	pee_in->d_fx_ordn_prc  = obook->Price;

	if( index->sise_cont.bidprc == 0.0)
	{
		pee_in->d_bid_usd_prc  = index->sise_curr.bidprc;
		pee_in->d_ask_usd_prc  = index->sise_curr.askprc;
	}
	else
	{
		pee_in->d_bid_usd_prc  = index->sise_cont.bidprc;
		pee_in->d_ask_usd_prc  = index->sise_cont.askprc;
	}
	if( index->sise_base.bidprc == 0.0)
	{
		pee_in->d_bid_std_prc  = index->sise_curr.bidprc;
		pee_in->d_ask_std_prc  = index->sise_curr.askprc;
	}
	else
	{
		pee_in->d_bid_std_prc  = index->sise_base.bidprc;
		pee_in->d_ask_std_prc  = index->sise_base.askprc;
	}
#endif

    LogDev( "index->base_cur=[%d] index->cont_cur=[%d]", index->base_cur, index->cont_cur);
    if( index->base_cur == 1 && index->cont_cur == 0)   /* 기본통화 - USD/KRW */
    {
        LogDev( "기본통화");
        pee_in->d_bid_usd_prc  = index->sise_curr.bidprc;   /* USD/KRW */
        pee_in->d_ask_usd_prc  = index->sise_curr.askprc;
        pee_in->d_bid_std_prc  = 0.0;
        pee_in->d_ask_std_prc  = 0.0;
        pee_in->d_bid_fnl_prc  = 0.0;
        pee_in->d_ask_fnl_prc  = 0.0;
    }
    else
    if( index->cont_cur == 0)                           /* 재정통화 - JPY/KRW */
    {
        LogDev( "재정통화");
        pee_in->d_bid_usd_prc  = index->sise_cont.bidprc;       /* USD/KRW */
        pee_in->d_ask_usd_prc  = index->sise_cont.askprc;
        pee_in->d_bid_std_prc  = index->sise_base.bidprc;       /* USD/JPY */
        pee_in->d_ask_std_prc  = index->sise_base.askprc;
        pee_in->d_bid_fnl_prc  = index->sise_curr.bidprc;       /* JPY/KRW */
        pee_in->d_ask_fnl_prc  = index->sise_curr.askprc;
    }
    else                                                /* 이종통화 - USD/JPY */
    {
        LogDev( "이종통화");
        pee_in->d_bid_usd_prc  = 0.0;                   /* USD/JPY */
        pee_in->d_ask_usd_prc  = 0.0;
        pee_in->d_bid_std_prc  = index->sise_curr.bidprc;
        pee_in->d_ask_std_prc  = index->sise_curr.askprc;
        pee_in->d_bid_fnl_prc  = 0.0;
        pee_in->d_ask_fnl_prc  = 0.0;
    }

    LogDev( "index->sise_cont.bidprc =[%12f]", index->sise_cont.bidprc);
    LogDev( "index->sise_cont.askprc =[%12f]", index->sise_cont.askprc);
    LogDev( "index->sise_base.bidprc =[%12f]", index->sise_base.bidprc);
    LogDev( "index->sise_base.askprc =[%12f]", index->sise_base.askprc);
    LogDev( "index->sise_curr.bidprc =[%12f]", index->sise_curr.bidprc);
    LogDev( "index->sise_curr.askprc =[%12f]", index->sise_curr.askprc);

#if 0
	/* 원본 */
	pee_in->d_bid_usd_prc  = index->sise_cont.bidprc;
	pee_in->d_ask_usd_prc  = index->sise_cont.askprc;
	pee_in->d_bid_std_prc  = index->sise_base.bidprc;
	pee_in->d_ask_std_prc  = index->sise_base.askprc;
	pee_in->d_bid_fnl_prc  = index->sise_curr.bidprc;
	pee_in->d_ask_fnl_prc  = index->sise_curr.askprc;
#endif

	if( Param->log_level < 5) SPLIT_IN_ST_Print( pee_in);

	sprintf( e_msg.code, "%d", 82031);
	sprintf( e_msg.mesg, "%s", "수수료 계산중 오류가 발생했습니다.");
	/******************************/
	/* 수수료 Call f_get_cust_prc */
	/******************************/
	rtn = f_get_cust_prc( pee_in, pee_out, &e_msg);
	if( rtn < 0)
	{
		LogCri( "f_get_cust_prc error. rtn=[%d]", rtn);
#if 1
		Mat_SetEmsg( mat, obook, &e_msg);
#else
		Mat_SetMessage( mat, obook, 82030);
#endif
		MAT_E_MSG_Print( &e_msg);
		ORDER_Print( obook);
		return -1;
	}
	if( Param->log_level < 5) SPLIT_OUT_ST_Print( pee_out);
	if( pee_out->n_rec_cnt <= 0) return 0;

	pee_price = pee_out->rec[ 0].d_fx_csac_prc;
	LogDel( "고객 마진 가격 =[%12f] 체결가", pee_price);
	LogDel( "시세 가격      =[%12f]", price);
	LogDel( "주문 가격      =[%12f]", obook->Price);
	Mat_StatisticsSet( mat, MAT_STAT_PEE, MAT_STAT_END);

	if( pee_price <= 0.0)
	{
		LogCri( "수수료 계산 오류!!! rtn=[%d] pee_price=[%f]", rtn, pee_price);
		LogCri( "고객 마진 가격      =[%12f] 체결가", pee_price);
		LogCri( "시세 가격           =[%12f]", price);
		LogCri( "주문 가격           =[%12f]", obook->Price);
		if( obook->OrdType[ 0] == '1')		/* 시장가 */
		{
			return -1;
		}
		else
		{
			LogCri( "지정가 주문 ... 주문 유효");
		}
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  고객번호 CustID(16) + FundNO(12)를 수수료 pee_in->s_csac_idnt_no로 copy
***************************************************************************** */
int Mat_CustCp( MAT *mat, SPLIT_IN_ST *pee_in, ORDER	 *obook)
{
	int		sz;
	char	cust_id[ sizeof( obook->CustID) +1], fund_no[ sizeof( obook->FundNO) +1];
	char	cust_buf[ sizeof( obook->CustID) + sizeof( obook->FundNO) +1];

	memset( cust_buf, 0x20, sizeof( obook->CustID) + sizeof( obook->FundNO));

    /* 고객번호 CustID(16) + FundNO(12) */
    memcpy( cust_id, obook->CustID, sizeof( obook->CustID));
    memcpy( fund_no, obook->FundNO, sizeof( obook->FundNO));
    TrimNR( cust_id, sizeof( obook->CustID));
    TrimNR( fund_no, sizeof( obook->FundNO));
    sz = sprintf( cust_buf, "%s%s", cust_id, fund_no);
	cust_buf[ sz] = 0x20;
    memcpy( pee_in->s_csac_idnt_no,  cust_buf,     sizeof( pee_in->s_csac_idnt_no) -1);

	return 1;
}
#endif
