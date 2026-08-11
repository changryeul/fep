/** ***************************************************************************
**  @file       mat.c
**  @date       2023/08/23
**  @author     cdc
**  @version    V2.0.20230823
**  @brif
**  매칭엔진 라이브러리
**	mat.c			- 매칭 시스템 주요 모듈 생성/삭제/오픈/클로즈
**	mat_sise.c		- 매칭시스템 시세 처리
**	mat_stat.c		- 통계처리 상태출력
**	mat_order.c		- 주문 처리
**	mat_match.c		- 매칭 처리
**	mat_execute.c	- 체결 처리
**	convert.c		- 주문 구조체 변환
**	print.c			- 매칭관련 구조체 출력
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

extern int		Continue;

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record pointer
**  @retval     실패    - NULL
**  @brief
**  일치하는 그룹주문을 wait에서 삭제
***************************************************************************** */
int Mat_InsertGroup( MAT *mat, ORDER *obook, int pos)
{
	int			rtn;
	int			i;
	int			seq, max;
	MAT_GROUP	*grp;
	MAT_RECORD	*rec;
	MAT_HEAD	*head;

	rec  = &mat->map->rec[ pos];
	head = &rec->head;

	seq = AtoI( obook->GrpOrdnSeq, sizeof( obook->GrpOrdnSeq));
	max = AtoI( obook->GrpOrdnCnt, sizeof( obook->GrpOrdnCnt));

	if( seq == 1)
	{
		rtn = Mat_FindGroup( mat, obook, 0);
		if( rtn < 0)
		{
			LogCri( "그룹 record를 저장할 공간이 없습니다.");
			return -82061;
		}
		grp = &mat->map->grp[ rtn];

		head->grp_next = -1;
		memset( grp, 0x00, sizeof( MAT_GROUP));
		grp->start = pos;
		memcpy( grp->id, obook->GrpOrdnNo, sizeof( obook->GrpOrdnNo));
		grp->tot   = max;
		grp->cnt++;
	}
	else
	{
		rtn = Mat_FindGroup( mat, obook, 1);
		if( rtn < 0)
		{
			LogCri( "그룹주문번호 없음");
			return -82009;
		}
		grp = &mat->map->grp[ rtn];

		head->grp_next = grp->start;
		grp->start = pos;
		grp->cnt++;
	}

	return pos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record pointer
**  @retval     실패    - NULL
**  @brief
**  일치하는 그룹주문을 wait에서 삭제
***************************************************************************** */
int Mat_GroupDelete( MAT *mat, MAT_START *start, int del_pos)
{
	int			pos, cnt = 0;
	MAT_RECORD	*rec = NULL;
	MAT_RECORD	*p_rec, *n_rec;		/* prev/next record pointer */

	pos = start->wait;
	while( pos > 0)
	{
		rec  = &mat->map->rec[ pos];

		if( pos == del_pos) break;

		pos = rec->head.wait_next;
		cnt++;
	}

	if( pos <= 0) /* delete record mot found */
	{
		LogCri( "delete record not found. del_pos=[%d]", del_pos);
		return -1;
	}

	start->wait_cnt--;

	if( cnt == 0)	/* 처음 */
	{
		LogDbg( "first record deleted. pos=[%d]", del_pos);
		start->wait = rec->head.wait_next;
		return del_pos;
	}

	p_rec = &mat->map->rec[ rec->head.wait_prev];
	p_rec->head.wait_next = rec->head.wait_next;

	if( rec->head.wait_next >= 0)
	{
		n_rec = &mat->map->rec[ rec->head.wait_next];
		n_rec->head.wait_prev = p_rec->pos;
	}

	return del_pos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record pointer
**  @retval     실패    - NULL
**  @brief
**  그룹주문을 wait index 추가 
***************************************************************************** */
int Mat_GroupNew( MAT *mat, MAT_GROUP *grp, int pos)
{
	int			rtn;
	int			seq, max;
	MAT_RECORD	*rec;
	MAT_HEAD	*head;

	rec  = &mat->map->rec[ pos];
	head = &rec->head;

#if 0
	seq = AtoI( obook->GrpOrdnSeq, sizeof( obook->GrpOrdnSeq));
	max = AtoI( obook->GrpOrdnCnt, sizeof( obook->GrpOrdnCnt));

	LogDbg( "New group record. pos=[%d]", pos);
	rtn = Mat_FindGroup( mat, obook, 1);
	if( rtn > 0) /* 그룹 주문번호 중복 에러 */
	{
		LogCri( "그룹 주문번호 중복 에러. order=[%.*s]", sizeof( obook->GrpOrdnNo), obook->GrpOrdnNo);
		return -1;
	}

	rtn = Mat_InsertGroup( mat, obook, pos);
	if( rtn <= 0)
	{
		LogCri( "그룹주문 INSERT error. pos=[%d]", pos);
		return rtn;
	}

	rec  = &mat->map->rec[ pos];
	head = &rec->head;
	head->grp_prev = -1;
	head->grp_next = -1;
#endif

	return rtn;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record pointer
**  @retval     실패    - NULL
**  @brief
**  그룹주문 double linked list 추가 
***************************************************************************** */
int Mat_GroupAdd( MAT *mat, MAT_START *start, ORDER *obook, int pos)
{
	int			find_pos, last_pos = -1;;
	MAT_RECORD	*rec, *prev_rec, *next_rec;
	MAT_HEAD	*head, *prev_head, *next_head;
	ORDER		*book;
	int			seq, rec_seq, tot;

	find_pos = Mat_WaitFind( mat, start, obook);
	if( find_pos <= 0) /* 그룹 주문번호 없음 에러 */
	{
		LogCri( "그룹 주문번호 없음 에러. order=[%.*s]", sizeof( obook->GrpOrdnNo), obook->GrpOrdnNo);
		return -1;
	}

	seq = AtoI( obook->GrpOrdnSeq, sizeof( obook->GrpOrdnSeq));
	tot = AtoI( obook->GrpOrdnCnt, sizeof( obook->GrpOrdnCnt));
	LogDbg( "GroupOrder seq/tot=[%d/%d]", seq, tot);

	LogDbg( "그룹주문 add.  find_pos=[%d] pos=[%d] seq=[%d/%d]", find_pos, pos, seq, tot);

	/* 그룹주문내의 insert할 위치를 찾는다 */
	while( find_pos > 0)
	{
		next_rec  = &mat->map->rec[ find_pos];
		next_head = &next_rec->head;
		book = ( ORDER  *)&next_rec->ord;
		rec_seq = AtoI( book->GrpOrdnSeq, sizeof( book->GrpOrdnSeq));
		LogDbg( "find_pos=[%d] seq=[%d] my=[%d]", find_pos, rec_seq, seq);
		if( rec_seq > seq) break;
		last_pos = find_pos;

		find_pos = next_head->grp_next;
	}

	if( find_pos <= 0)	/* append */
	{
		LogDbg( "그룹주문 append.  pos=[%d] prev_pos=[%d] seq=[%d/%d]", pos, last_pos, seq, tot);
		prev_rec  = &mat->map->rec[ last_pos];
		prev_head = &prev_rec->head;
		prev_head->grp_next = pos;

		rec  = &mat->map->rec[ pos];
		head = &rec->head;
		head->grp_prev = last_pos;
		head->grp_next = -1;
	}
	else				/* insert */
	{
		LogDbg( "그룹주문 insert.  pos=[%d] prev_pos=[%d] next_pos=[%d] seq=[%d/%d]", pos, last_pos, find_pos, seq, tot);
		prev_rec  = &mat->map->rec[ last_pos];
		prev_head = &prev_rec->head;
		prev_head->grp_next = pos;
		next_head->grp_prev = pos;

		rec  = &mat->map->rec[ pos];
		head = &rec->head;
		head->grp_prev = last_pos;
		head->grp_next = find_pos;
	}

	mat->map->stat.rec_cnt++;

	return pos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record pointer
**  @retval     실패    - NULL
**  @brief
**  그룹주문 double linked list 추가 
***************************************************************************** */
int Mat_GroupEnd( MAT *mat, MAT_START *start, ORDER *obook, int pos)
{
	int			rtn;
	int			start_pos;

	start_pos = Mat_WaitFind( mat, start, obook);
	if( start_pos <= 0) /* 그룹 주문번호 없음 에러 */
	{
		LogCri( "그룹 주문번호 없음 에러. order=[%.*s]", sizeof( obook->GrpOrdnNo), obook->GrpOrdnNo);
		return -82009;
	}
	LogDbg( "그룹주문 End.  pos=[%d] at start_pos=[%d]", pos, start_pos);

	/* 마지막 그룹주문 추가 */
	rtn = Mat_GroupAdd( mat, start, obook, pos);
	if( rtn < 0)
	{
		LogCri( "Mat_GroupAdd error.");
		return -1;
	}

	/* 일반 주문에 추가 - add index */
	rtn = Mat_Insert( mat, start, start_pos);
	if( rtn < 0)
	{
		LogCri( "Mat_Insert error.");
		return -1;
	}
	LogDbg( "INSERT index->start[ side].start_cnt=[%d] Group Add", start->start_cnt);

	/* 주문대기열에서 삭제 */
	LogDbg( "delete wait group position. pos=[%d]", pos);
	rtn = Mat_DeleteWait( mat, start, start_pos);
	if( rtn < 0)
	{
		LogCri( "Mat_DeleteWait error.");
		return -1;
	}

	return pos;
}


