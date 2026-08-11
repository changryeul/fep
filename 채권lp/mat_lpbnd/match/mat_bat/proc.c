/** ***************************************************************************
**  @file       main.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  프로그램 초기화/프로세싱/종료
**  파라메터 세팅 및 환경파일 로드
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>

#include "mat.h"
#include "smq.h"
#include "order.h"
#include "sise.h"
#include "main.h"

/** ***************************************************************************
**	GLOBAL
***************************************************************************** */
extern int			Continue;
extern MAT			*Mat;
extern PARAM		*Param;

extern MAT_REJECT	MatReject[];

/** ***************************************************************************
**	
***************************************************************************** */
/** ***************************************************************************
**  @fn         int Proc_()
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  주문 정합 check
***************************************************************************** */
int	Proc_LoadMessage( MAT *mat)
{
	int		rtn;
	CFG		*cfg;
	char	cfg_name[ 1024];
	char	*line;

	LogDbg( "cfg_name=[%s]", Param->cfg_name);
	cfg = Cfg_Open( Param->cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", Param->cfg_name);
		return -1;
	}

	Mat_MessageReset( mat);
	LogMsg( "Message load. cfg=[%s]", Param->cfg_name);

	line = Cfg_GetFirstNamePtr( cfg, "Message");
	while( line != NULL)
	{
		LogDel( "line=[%s]", line);
		rtn = Proc_LineMessage( mat, line);
		line = Cfg_GetNextNamePtr( cfg, "Message");
	}

	Cfg_Close( cfg);

	return rtn;
}

/** ***************************************************************************
**  @fn         int Proc_()
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  주문 정합 check
***************************************************************************** */
int	Proc_LineMessage( MAT *mat, char *line)
{
	int		rtn;
	int		stat = 0;
	char	*token = ":";
	char	rec[ 1024];
	char	*ptr = rec, *end = ptr;

	int		err;
	char	msg[ 512];

	memcpy( rec, line, strlen( line) +1);

	while( end != NULL)
	{
		end = strpbrk( ptr, token);
		if( end)	*end = 0;

		switch( stat)
		{
			case 0:		/* error number */
				err = atoi( ptr);
				break;
			case 1:		/* message */
				memcpy( msg, ptr, strlen( ptr) +1);
				break;
				
		}
		stat++;
		ptr = end +1;
	}
	if( stat < 2) return -1;

	LogMsg( "[%5d:%s]", err, msg);
	rtn = Mat_MessageAdd( mat, err, msg);

	return rtn;
}

/** ***************************************************************************
**  @fn         int Proc_()
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  주문 정합 check
***************************************************************************** */
int	Proc_LoadCurrent()
{
	int		rtn;
	CFG		*cfg;
	char	*line;

	cfg = Cfg_Open( Param->cfg_name);
	if( cfg == NULL)
	{
		LogErr( "Cfg_Open error. name=[%s]", Param->cfg_name);
		return -1;
	}

	Cfg_Set( cfg, "CurrentInt");

	line = Cfg_GetFirstNamePtr( cfg, "CurrentInt");
	while( line != NULL)
	{
		LogDel( "line=[%s]", line);
		rtn = Proc_LineCurrent( Mat, line);
		line = Cfg_GetNextNamePtr( cfg, "CurrentInt");
	}

	Cfg_Close( cfg);

	return rtn;
}

/** ***************************************************************************
**  @fn         int Proc_()
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  주문 정합 check
***************************************************************************** */
int	Proc_LineCurrent( MAT *mat, char *line)
{
	int			stat = 0, pos;
	char		*token = ":";
	char		rec[ 8192];
	MAT_CURRENT	*current = NULL;
	char		*ptr = rec, *end = ptr;

	memcpy( rec, line, strlen( line) +1);

	while( end != NULL)
	{
		end = strpbrk( ptr, token);
		if( end) *end = 0;

		switch( stat)
		{
			case 0:		/* position */
				pos = atoi( ptr);
				current = &mat->map->current[ pos];
				current->num = pos;
				break;
			case 1:		/* 통화 표시 */
				TrimR( ptr);
				memcpy( current->str, ptr, strlen( ptr) +1);
				break;
			case 2:		/* 소수점 */
				break;
			case 3:		/* comment */
				memcpy( current->comment, ptr, strlen( ptr) +1);
			default:
				break;
		}
		ptr = end +1;
		stat++;
	}
	LogDbg( "[%2d][%s][%s]",
		current->num,
		current->str,
		current->comment
	);

	return 1;
}

/** ***************************************************************************
**  @fn         int Proc_()
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  주문 정합 check
***************************************************************************** */
int	Proc_OrderInsert( MAT *mat, ORDER *obook)
{
	int			rtn;
	int			pos;
	int			side;
	int			base_cur, cont_cur;
	int			jang_id;
	MAT_INDEX	*index;
	MAT_HEAD	*head;
	MAT_RECORD	*rec;

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

	pos = Mat_FindRecordByClOrdID( mat, index, obook);
	if( pos > 0)
	{
		LogMsg( "같은 주문번호의 주문이 테이블에 있습니다. pos=[%d] ClOrdID=[%.*s]", 
				pos, sizeof( obook->ClOrdID), obook->ClOrdID);
		return 0;
	}
	LogMsg( "주문 INSERT ...");

	pos = Mat_GetEmptyRecordPos( mat);

	// jang_id = Mat_JangCheckBatch( mat, index, obook);

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
	// head->jang_id = jang_id;
	head->price = obook->Price;
	Mat_MakeHead( mat, head, obook);
#if 0
	gettimeofday( &head->rcv_time, NULL);
#endif
	/* 장운영 정보 check */
	head->jang_id = Mat_GetJangId( mat, obook, 1);
	if( head->jang_id <= 0)
	{
		LogCri( "주문의 장운영 정보가 없습니다. head->jang_id=[%d]", head->jang_id);
		return -1;
	}

	rtn = Mat_Insert( mat, &index->start[ side], pos);
	if( rtn < 0)
	{
		LogCri( "주문 INSERT error. rtn=[%d]", rtn);
		return 0;
	}

	return 1;
}

#if JANG_OLD
/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공 - jang_id 장운영 상품 구분 코드
**  @retval     실패 - 0       장시간 아님
**  @brief
**  원주문번호와 일치하는 주문 찾기
***************************************************************************** */
int Mat_JangCheckBatch( MAT *mat, MAT_INDEX *index, ORDER *order)
{
	int				jang_id = -1;			/* SPC:0 MAR:1 TOD:2 TOM:3 SPT:4 FWD:5 */
	time_t			cur_time;
	MAT_JANG_REC	*jang;

	/* 장운영 상품 구분 코드 등록 */
	if( order->TranPtrnCd[ 0] == '2')				jang_id = 1;	/* MAR */
	else
	if( !memcmp( order->SettType, "SPT", 3))
	{
		if( !memcmp( order->TnrId, "TOD", 3))		jang_id = 2;	/* TOD */
		else
		if( !memcmp( order->TnrId, "TOM", 3))		jang_id = 3;	/* TOM */
		else
		if( !memcmp( order->TnrId, "SPT", 3))		jang_id = 4;	/* SPT */
		else
		{
			jang_id = 0;
		}
	}
	else
	if( !memcmp( order->SettType, "FWD", 3))		jang_id = 5;	/* FWD */
	else
	{
		jang_id = 0;
	}

	return jang_id;
}
#endif










