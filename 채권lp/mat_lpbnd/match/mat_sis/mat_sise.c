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
**  mat_mat 매인 루틴
***************************************************************************** */
int Mat_Sise( MAT *mat, MATSISE *sise)
{
	int			rtn;
	int			base_cur;
	int			cont_cur;

	LogDel( "Mat_Match ... start");

	/* 기준통화 */
	base_cur = Mat_GetCurrentInt( mat, &sise->symb[ 0]);
	if( base_cur < 0)
	{
		LogMsg( "통화정보가 올바르지 않습니다. base_cur=[%.3s]", &sise->symb[ 0]);
		return -1;
	}

	/* 상대통화 */
	cont_cur = Mat_GetCurrentInt( mat, &sise->symb[ 4]);
	if( cont_cur < 0)
	{
		LogMsg( "통화정보가 올바르지 않습니다. cont_cur=[%.3s]", &sise->symb[ 4]);
		return -1;
	}
	LogDel( "메칭 시작 ... 기준통화/상대통화=[%s(%2d)/%s(%2d)]", 
			mat->map->current[ base_cur].str, base_cur, 
			mat->map->current[ cont_cur].str, cont_cur);
	LogDel( "              bid=[%f] ask=[%f]", sise->bidprc, sise->askprc);

#if 0
	Mat_Lock( mat);
#endif
	rtn = Mat_SiseProcess( mat, base_cur, cont_cur, sise);
	if( rtn < 0)
	{
		return rtn;
	}
#if 0
	Mat_Unlock( mat);
#endif

	LogDel( "Mat_Match ... end");
	return 0;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  시세처리 - 재정,기준/상대 통화 관련된 시세 UPDATE
***************************************************************************** */
int Mat_SiseProcess( MAT *mat, int base, int cont, MATSISE *sise)
{
	int			i;
	MAT_INDEX	*index;

	LogDel( "시세 PROCESS ... sise=[%s(%2d)/%s(%2d)]", 
			mat->map->current[ base].str, base, mat->map->current[ cont].str, cont);

	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		index = &mat->map->index[ i];
		LogDel( "index ... sise=[%s(%2d)/%s(%2d)]", 
				mat->map->current[ index->base_cur].str, index->base_cur, 
				mat->map->current[ index->cont_cur].str, index->cont_cur);

		/* 빈 index */
		if( index->base_cur == 0 && index->cont_cur == 0) continue;

		/* 시세 일치 통화 */
		if( base == index->base_cur && cont == index->cont_cur)
		{
			LogDel( "index->sise_curr update ... bid=[%f] ask=[%f]", sise->bidprc, sise->askprc);
			Mat_SiseUpdate( mat, index, sise, 0);
		}

		/* USD/KRW는 시세 일치만 update */
		if( index->base_cur == 1 && index->cont_cur == 0)	continue;

		if( base == 1 && cont == 0)								/* USD/KRW */
		{
			if( index->cont_cur == 0)											/* 재정통화 cont */
			{
				LogDel( "index->sise_cont update ... bid=[%f] ask=[%f]", sise->bidprc, sise->askprc);
				Mat_SiseUpdate( mat, index, sise, 1);
			}
		}
		else
		if( base == 1 && cont == index->base_cur)				/* USD/@@@ */
		{
			if( index->cont_cur == 0)											/* 재정통화 base */
			{
				LogDel( "index->sise_base update ... bid=[%f] ask=[%f]", sise->bidprc, sise->askprc);
				Mat_SiseUpdate( mat, index, sise, 2);
			}
		}
		else
		if( cont == 1 && base == index->base_cur)				/* @@@/USD */
		{
			if( index->cont_cur == 0)											/* 재정통화 base */
			{
				LogDel( "index->sise_base update ... bid=[%f] ask=[%f]", sise->bidprc, sise->askprc);
				Mat_SiseUpdate( mat, index, sise, 3);
			}
		}
	} /* for */

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat         - 매칭 struct pointer
**  @param      MAT_INDEX *index - index pointer
**  @param      MATSISE *sise    - 수신된 시세, 내용은 APSISE
**  @param      int type         - 0:현재시세 
**                                 1:재정통화시세 
**                                 2:비재정통화시세중 USD:@@@ 
**                                 3:비재정통화시세중 @@@:USD
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  이종통화 시세 계산 
***************************************************************************** */
int Mat_SiseUpdate( MAT *mat, MAT_INDEX *index, MATSISE *sise, int type)
{
	int				rtn;
	double			unit, point;
	MATSISE			*curr, *base, *cont;

	curr = &index->sise_curr;
	base = &index->sise_base;
	cont = &index->sise_cont;

#if 0
	struct timeval	tv;
	struct tm		*tp;
	gettimeofday( &tv, NULL);
	tp = localtime( &tv.tv_sec);
#endif

	LogDel( "MAT  [%.3s%.3s] ... ", mat->map->current[ index->base_cur].str, mat->map->current[ index->cont_cur].str);

	Mat_Lock( mat);

	index->sis_cnt++;
	time( &index->sis_time);

	if( type)			/* index->sise_curr update가 아닌경우 MATSISE의 field update */
	{
#if 0
		char	time_str[ 32];
		sprintf( time_str, "%04d%02d%02d%02d%02d%02d%03ld", 
				tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday,
				tp->tm_hour, tp->tm_min, tp->tm_sec,
				tv.tv_usec / 1000);
#endif
		curr->excode[ 0] = 'M';
		curr->bidex[ 0]  = ' ';
		curr->askex[ 0]  = ' ';
		memcpy( &curr->symb[ 0], &base->symb[ 4], 3);
		memcpy( &curr->symb[ 4], &cont->symb[ 4], 3);
		curr->symb[ 3] = '/';
		memcpy( curr->id, sise->id, sizeof( curr->id));
		memcpy( curr->date, sise->date, sizeof( curr->date) + sizeof( curr->time));
		curr->ctime = sise->ctime;
		LogDel( "ctime=[%ld]", curr->ctime);
		/* 시세 유효기간은 상대통화 시간을 update 한다 */
		/* - 현재 통화는 시세가 들어와 update 했으므로 유효시간은 다른 통화 시세 시간에 따라 체결 여부를 결정.. */
		/* 
		 * 현재/기준/상대 통화 일일이 비교 하는것으로 변경
		if( type == 1)	
		{
			curr->ctime = index->sise_base.ctime;
			curr->price_time = index->sise_base.price_time;
		}
		else 			
		{
			curr->ctime = index->sise_cont.ctime;
			curr->price_time = index->sise_cont.price_time;
		}
		*/
		unit  = ( double)index->unit;
		point = ( double)index->point;
	}

	switch( type)
	{
		case 0:			/* current update */
			Proc_SiseConvert( &index->sise_curr, sise);
#if 0
			/*
			 * USD/JPY 경우 sise->askprc = 151.407의 경우 문제가 있음
			 * ceil 함수가 151407.000을 151408.000으로 변환함 
			 */
			d = ( double)index->point;
			e = exp10( d);
			curr->bidprc = floor( sise->bidprc * e) / e;
			curr->askprc = ceil ( sise->askprc * e) / e;
			LogDel( "d                  =[%f]", d);
			LogDel( "e                  =[%f]", e);
			LogDel( "curr/sise bid      =[%f][%f]", curr->bidprc, sise->bidprc);
			LogDel( "curr/sise ask      =[%f][%f]", curr->askprc, sise->askprc);
#endif
			break;
		case 1:			/* 재정통화 USD/KRW update */
			Proc_SiseConvert( &index->sise_cont, sise);
			if( !memcmp( &base->symb[ 0], "USD", 3))
			{
				if( cont->bidprc != 0.0 && base->askprc != 0.0)	
				{
					curr->bidprc = ( cont->bidprc / base->askprc) * unit;
					curr->bidprc = Mat_Round( mat, curr->bidprc, point);
					LogDel( "curr->bidprc=[%15f][%f]", curr->bidprc, point);
				}
				else { /* curr->bidprc = 0.0; 가격이 0일때 update 안함 20231124 */ }
				if( cont->askprc != 0.0 && base->bidprc != 0.0)	
				{
					curr->askprc = ( cont->askprc / base->bidprc) * unit;
					curr->askprc = Mat_Round( mat, curr->askprc, point);
				}
				else { /* curr->askprc = 0.0; */ }
			}
			else								
			{
				if( cont->bidprc != 0.0 && base->bidprc != 0.0)	
				{
					curr->bidprc = ( cont->bidprc * base->bidprc) * unit;
					curr->bidprc = Mat_Round( mat, curr->bidprc, point);
				}
				else { /* curr->bidprc = 0.0; */ }
				if( cont->askprc != 0.0 && base->askprc != 0.0)	
				{
					curr->askprc = ( cont->askprc * base->askprc) * unit;
					curr->askprc = Mat_Round( mat, curr->askprc, point);
				}
				else { /* curr->askprc = 0.0; */ }
			}
			break;
		case 2:			/* base update USD/@@@ */
			Proc_SiseConvert( &index->sise_base, sise);
			if( cont->bidprc != 0.0 && base->askprc != 0.0)	
			{
				curr->bidprc = ( cont->bidprc / base->askprc) * unit;
				curr->bidprc = Mat_Round( mat, curr->bidprc, point);
			}
			/* else											curr->bidprc = 0.0; */
			if( cont->askprc != 0.0 && base->bidprc != 0.0)	
			{
				curr->askprc = ( cont->askprc / base->bidprc) * unit;
				curr->askprc = Mat_Round( mat, curr->askprc, point);
			}
			/* else											curr->askprc = 0.0; */
			break;
		case 3:			/* base update @@@/USD */
			Proc_SiseConvert( &index->sise_base, sise);
			if( cont->bidprc != 0.0 && base->bidprc != 0.0)	
			{
				curr->bidprc = ( cont->bidprc * base->bidprc) * unit;
				curr->bidprc = Mat_Round( mat, curr->bidprc, point);
			}
			/* else											curr->bidprc = 0.0; */
			if( cont->askprc != 0.0 && base->askprc != 0.0)	
			{
				curr->askprc = ( cont->askprc * base->askprc) * unit;
				curr->askprc = Mat_Round( mat, curr->askprc, point);
			}
			/* else											curr->askprc = 0.0; */
			break;
	}
	Mat_Unlock( mat);

#if 0
	rtn = Mat_Matching( mat, index);
#else
	rtn = Mat_SendIdxPos( mat, index->no);
	if( rtn < 0)
	{
		LogCri( "Mat_SendIdxPos error. mat=[%p] idx_pos=[%d]", mat, index->no);
		return -1;
	}
#endif

	return rtn;
}

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
int Mat_SendIdxPos( MAT *mat, int idx_pos)
{
	int				rtn;

	rtn = write( mat->mat_fd, &idx_pos, sizeof( int));
	if( rtn < sizeof( int))
	{
		LogErr( "write pipe error. rtn=[%d] sz=[%d]", rtn, sizeof( int));
		return -1;
	}
	LogDel( "send data %d byte(s). idx_pos=[%2d]", rtn, idx_pos);

	return rtn;
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
double Mat_Round( MAT *mat, double value, double point)
{
	double	e;

	e = exp10( point);

	return round( value * e) / e;
}


