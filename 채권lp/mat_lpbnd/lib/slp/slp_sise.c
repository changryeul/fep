/* ***************************************************************************
**  @file       slp.h
**  @date       2024/02/01
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  double linked list position manager library
**  double linked list를 포지션으로 관리 - 공유 메모리용
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "log.h"
#include "cfg.h"
#include "etc.h"
#include "slp.h"

extern int	Continue;

/** ***************************************************************************
*  @fu         int Slp_( SLP *slp)
**  @param      SLP *slp - 매칭 struct pointer
**  @return     성공    - index position
**  @retval     실패    - -1
**  @brief
**  시세처리 - 재정,기준/상대 통화 관련된 시세 UPDATE
***************************************************************************** */
int Slp_Sise( SLP *slp, int base, int cont, SLP_SISE *sise)
{
	int			i;
	SLP_INDEX	*index;

	LogDel( "시세 PROCESS ... sise=[%s(%2d)/%s(%2d)]", 
			slp->map->current[ base].str, base, slp->map->current[ cont].str, cont);

	for( i = 0; i < SLP_MAX_IDX( slp); i++)
	{
		index = SLP_GET_IDX( slp, i);
		LogDel( "index ... sise=[%s(%2d)/%s(%2d)]", 
				slp->map->current[ index->pair.base].str, index->pair.base, 
				slp->map->current[ index->pair.cont].str, index->pair.cont);

		/* 빈 index */
		if( index->pair.base == 0 && index->pair.cont == 0) break;

		/* 시세 일치 통화 */
		if( base == index->pair.base && cont == index->pair.cont)
		{
			LogDel( "index->sise_curr update ... bid=[%f] ask=[%f]", sise->bidprc, sise->askprc);
			Slp_SiseUpdate( slp, index, sise, 0);
		}

		/* USD/KRW는 시세 일치만 update */
		if( index->pair.base == 1 && index->pair.cont == 0)	continue;

		if( base == 1 && cont == 0)								/* USD/KRW */
		{
			if( index->pair.cont == 0)											/* 재정통화 cont */
			{
				LogDel( "index->sise_cont update ... bid=[%f] ask=[%f]", sise->bidprc, sise->askprc);
				Slp_SiseUpdate( slp, index, sise, 1);
			}
		}
		else
		if( base == 1 && cont == index->pair.base)				/* USD/@@@ */
		{
			if( index->pair.cont == 0)											/* 재정통화 base */
			{
				LogDel( "index->sise_base update ... bid=[%f] ask=[%f]", sise->bidprc, sise->askprc);
				Slp_SiseUpdate( slp, index, sise, 2);
			}
		}
		else
		if( cont == 1 && base == index->pair.base)				/* @@@/USD */
		{
			if( index->pair.cont == 0)											/* 재정통화 base */
			{
				LogDel( "index->sise_base update ... bid=[%f] ask=[%f]", sise->bidprc, sise->askprc);
				Slp_SiseUpdate( slp, index, sise, 3);
			}
		}
	} /* for */

	return 1;
}

/** ***************************************************************************
**  @fu         int Slp_( SLP *slp)
**  @param      SLP *slp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  이종통화 시세 계산 
***************************************************************************** */
int Slp_SiseUpdate( SLP *slp, SLP_INDEX *index, SLP_SISE *sise, int type)
{
	int				rtn;
	struct timeval	tv;
	struct tm		*tp;
	char			time_str[ 32];
	double			unit, point;
	SLP_SISE			*curr, *base, *cont;

	curr = &index->sise[ 0];
	base = &index->sise[ 1];
	cont = &index->sise[ 2];

	gettimeofday( &tv, NULL);
	tp = localtime( &tv.tv_sec);

	LogDel( "SLP  [%.3s%.3s] ... ", slp->map->current[ index->pair.base].str, slp->map->current[ index->pair.cont].str);

	Slp_Lock( slp);

	if( type)			/* index->sise_curr update가 아닌경우 SLP_SISE의 field update */
	{
		sprintf( time_str, "%04d%02d%02d%02d%02d%02d%03ld", 
				tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday,
				tp->tm_hour, tp->tm_min, tp->tm_sec,
				tv.tv_usec / 1000);
		curr->excode[ 0] = 'M';
		curr->bidex[ 0]  = ' ';
		curr->askex[ 0]  = ' ';
		memcpy( &curr->symb[ 0], &base->symb[ 4], 3);
		memcpy( &curr->symb[ 4], &cont->symb[ 4], 3);
		curr->symb[ 3] = '/';
		memcpy( curr->id, sise->id, sizeof( curr->id));
		memcpy( curr->date, sise->date, sizeof( curr->date) + sizeof( curr->time));
		LogDel( "ctime=[%ld]", curr->ctime);
		unit  = ( double)index->pair.unit;
		point = ( double)index->pair.point;
	}

	switch( type)
	{
		case 0:			/* current update */
			rtn = Slp_SiseConvert( &index->sise[ 0], sise);
			break;
		case 1:			/* 재정통화 USD/KRW update */
			rtn = Slp_SiseConvert( &index->sise[ 2], sise);
			if( !memcmp( &base->symb[ 0], "USD", 3))
			{
				if( cont->bidprc != 0.0 && base->askprc != 0.0)	
				{
					curr->bidprc = ( cont->bidprc / base->askprc) * unit;
					curr->bidprc = Slp_Round( slp, curr->bidprc, point);
					LogDel( "curr->bidprc=[%15f][%f]", curr->bidprc, point);
				}
				else { /* curr->bidprc = 0.0; 가격이 0일때 update 안함 20231124 */ }
				if( cont->askprc != 0.0 && base->bidprc != 0.0)	
				{
					curr->askprc = ( cont->askprc / base->bidprc) * unit;
					curr->askprc = Slp_Round( slp, curr->askprc, point);
				}
				else { /* curr->askprc = 0.0; */ }
			}
			else								
			{
				if( cont->bidprc != 0.0 && base->bidprc != 0.0)	
				{
					curr->bidprc = ( cont->bidprc * base->bidprc) * unit;
					curr->bidprc = Slp_Round( slp, curr->bidprc, point);
				}
				else { /* curr->bidprc = 0.0; */ }
				if( cont->askprc != 0.0 && base->askprc != 0.0)	
				{
					curr->askprc = ( cont->askprc * base->askprc) * unit;
					curr->askprc = Slp_Round( slp, curr->askprc, point);
				}
				else { /* curr->askprc = 0.0; */ }
			}
			break;
		case 2:			/* 비재정통화 update USD/@@@ */
			rtn = Slp_SiseConvert( &index->sise[ 1], sise);
			if( cont->bidprc != 0.0 && base->askprc != 0.0)	
			{
				curr->bidprc = ( cont->bidprc / base->askprc) * unit;
				curr->bidprc = Slp_Round( slp, curr->bidprc, point);
			}
			/* else											curr->bidprc = 0.0; */
			if( cont->askprc != 0.0 && base->bidprc != 0.0)	
			{
				curr->askprc = ( cont->askprc / base->bidprc) * unit;
				curr->askprc = Slp_Round( slp, curr->askprc, point);
			}
			/* else											curr->askprc = 0.0; */
			break;
		case 3:			/* 비재정통화 update @@@/USD */
			rtn = Slp_SiseConvert( &index->sise[ 1], sise);
			if( cont->bidprc != 0.0 && base->bidprc != 0.0)	
			{
				curr->bidprc = ( cont->bidprc * base->bidprc) * unit;
				curr->bidprc = Slp_Round( slp, curr->bidprc, point);
			}
			/* else											curr->bidprc = 0.0; */
			if( cont->askprc != 0.0 && base->askprc != 0.0)	
			{
				curr->askprc = ( cont->askprc * base->askprc) * unit;
				curr->askprc = Slp_Round( slp, curr->askprc, point);
			}
			/* else											curr->askprc = 0.0; */
			break;
	}

	Slp_Unlock( slp);
	rtn = write( slp->mat_fd, ( char *)&index->pos, sizeof( int));
	if( rtn < sizeof( int))
	{
		LogErr( "pipe write error. rtn=[%d] fd=[%d]", rtn, slp->mat_fd);
		return 0;
	}

	return rtn;
}

/** ***************************************************************************
**  @fu         int Slp_( SLP *slp)
**  @param      SLP *slp - 매칭 struct pointer
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  신규 주문 매칭
**  다음 틱 까지 기다려 매칭 되는것을 방지하기 위함
***************************************************************************** */
double Slp_Round( SLP *slp, double value, double point)
{
	double	e;

	e = exp10( point);

	return round( value * e) / e;
}

