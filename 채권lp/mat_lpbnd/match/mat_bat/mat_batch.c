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
**	mat_group.c		- 그룹주문 처리
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
**  @fu         int Mat_Create()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - mat pointer
**  @retval     실패    - NULL
**  @brief
**  매칭엔진에 필요한 ipc를 생성
**	shared memory, semaphore 생성
***************************************************************************** */
int Mat_BatchStart( MAT *mat)
{
	int			rtn, i;
	MAT_INDEX	*index;

	LogMsg( "Mat_BatchStart. batch start ");

	LogMsg( "통계를 초기화 합니다. [Mat_StatisReset]");
	/* Mat_StatisReset 안에서 Lock 수행 */
	rtn = Mat_StatisReset( mat);
	if( rtn < 0)
	{
		LogCri( "Mat_StatisReset error. rtn=[%d]", rtn);
	}

	Mat_Lock( mat);
	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		index = &mat->map->index[ i];
		if( index->base_cur == 0 && index->cont_cur == 0) continue;

		LogMsg( "시세를 초기화 합니다. [%.3s/%.3s]", 
				&mat->map->current[ index->base_cur].str, &mat->map->current[ index->cont_cur].str);
		memset( &index->sise_curr, 0x00, sizeof( MATSISE));
		memset( &index->sise_base, 0x00, sizeof( MATSISE));
		memset( &index->sise_cont, 0x00, sizeof( MATSISE));
	}
	Mat_Unlock( mat);

	return rtn;
}

/** ***************************************************************************
**  @fu         int Mat_Create()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - mat pointer
**  @retval     실패    - NULL
**  @brief
**  매칭엔진에 필요한 ipc를 생성
**	shared memory, semaphore 생성
***************************************************************************** */
int Mat_BatchEnd( MAT *mat)
{
	int			i, j;
	time_t		cur_time;
	MAT_RECORD	*rec;
	MAT_HEAD	*head;
	MAT_INDEX	*index;
	MAT_START	*start;
	MAT_GROUP	*grp;
	MAT_CURRENT	*base, *cont;
	ORDER		*obook;

	time( &cur_time);
	LogMsg( "Matching system end batch start. at=[%s]", TtoS( cur_time));

	/* 주문 List 출력 */
	Mat_OrderList( mat);

	/* 통계 출력 */
	Mat_PrintSticRaw( mat);

	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		index = &mat->map->index[ i];
		if( index->base_cur == index->cont_cur) continue;

		base  = &mat->map->current[ index->base_cur];
		cont  = &mat->map->current[ index->cont_cur];

		LogMsg( "Check index ... pos=[%d] current=[%s(%d)/%s(%d)]", 
				i, base->str, base->num, cont->str, cont->num);
		for( j = 0; j < 2; j++)
		{
			start = &index->start[ j];
			LogMsg( "buy(0)/sell(1)=[%d] cnt=[%d]", j, start->start_cnt);
			while( start->start > 0)
			{
				LogMsg( "####################################################################");
				LogMsg( "##### 주문 record를 삭제 합니다. pos=[%d]", start->start);
				LogMsg( "####################################################################");

				rec    = &mat->map->rec[ start->start];
				head   = &rec->head;
				obook  = ( ORDER *)&rec->ord;
				/*
				MAT_HEAD_Print( head);
				ORDER_Print( obook);
				*/

				Mat_Delete( mat, start, start->start);
				head->gubun = 0;
				memset( rec, 0x00, sizeof( MAT_RECORD));
				rec->pos = i;
			}
		}
	}

	LogMsg( "##### 모든 record를 clear합니다. count=[%d]", MAT_MAX_RECORD);
	for( i = 1; i < MAT_MAX_RECORD; i++)
	{
		rec    = &mat->map->rec[ i];
		memset( rec, 0x00, sizeof( MAT_RECORD));
		rec->pos = i;
	}

	LogMsg( "##### 모든 group 저장소를 clear합니다. count=[%d]", MAT_MAX_GROUP);
	for( i = 1; i < MAT_MAX_GROUP; i++)
	{
		grp = &mat->map->grp[ i];
		memset( grp, 0x00, sizeof( MAT_GROUP));
	}
	mat->map->grp_pos = 1;


	LogMsg( "##### 신규 저장 위치를 1로 setting합니다. old=[%d]", mat->map->stat.wpos);
	mat->map->stat.wpos = 1;
	mat->map->stat.rec_cnt = 0;
	mat->map->stat.exe_cnt = 0;

	return 1;
}


/** ***************************************************************************
**  @fu         int Mat_Create()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - mat pointer
**  @retval     실패    - NULL
**  @brief
**  주문 강제 취소
***************************************************************************** */
int Mat_ForceCancel( MAT *mat)
{
	int			rtn;
	int			i, j;
	int			pos = 0, cnt = 0;
	time_t		cur_time;
	MAT_RECORD	*rec;
	MAT_HEAD	*head;
	MAT_INDEX	*index;
	MAT_START	*start;
	MAT_GROUP	*grp;
	MAT_CURRENT	*base, *cont;
	ORDER		*obook;

	time( &cur_time);
	LogMsg( "유효주문 강제취소 시작. at=[%s]", TtoS( cur_time));

	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		index = &mat->map->index[ i];
		if( index->base_cur == index->cont_cur) continue;

		base  = &mat->map->current[ index->base_cur];
		cont  = &mat->map->current[ index->cont_cur];

		LogDel( "Check index ... pos=[%d] current=[%s(%d)/%s(%d)]", 
				i, base->str, base->num, cont->str, cont->num);
		for( j = 0; j < 2; j++)
		{
			start = &index->start[ j];
			LogDel( "buy(0)/sell(1)=[%d] cnt=[%d]", j, start->start_cnt);
			while( start->start > 0)
			{
				LogMsg( "##### 주문 record를 강제 취소 합니다. pos=[%d]", start->start);

				pos    = start->start;
				rec    = &mat->map->rec[ start->start];
				head   = &rec->head;
				obook  = ( ORDER *)&rec->ord;
				/*
				MAT_HEAD_Print( head);
				ORDER_Print( obook);
				*/

				Mat_Delete( mat, start, start->start);
				head->error = 82210;	/* 82210  :엔진에 의해 강제 취소 되었습니다. */
				mat->map->stat.exe_cnt++;
				cnt++;

				retry:
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
			}	/* while */
		}	/* for */
	}
	LogMsg( "유효주문 강제취소 종료. cnt=[%d]", cnt);

	return 1;
}





