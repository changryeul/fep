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
**  @fu         int Slp_( SLP *slp)
**  @param      SLP *slp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  통계 setting
**	SLP_MAX_GAP 이상의 통계는 제외
***************************************************************************** */
int Slp_StatisticsSet( SLP *slp, int pos, int opt)
{
	int				gap;
	SLP_STATIS		*stel;

	stel = &slp->status->statis[ pos];

	switch( opt)
	{
		case SLP_STAT_COUNT:
			gettimeofday( &stel->end, NULL);
			stel->cnt++;
			return 1;

		case SLP_STAT_START:		/* 측정 시작 */
			gettimeofday( &stel->srt, NULL);
			return 1;

		case SLP_STAT_END:		/* 측정 종료 */
			if( stel->min <= 0 && stel->cnt == 0) stel->min = 999999999;
			gettimeofday( &stel->end, NULL);
			stel->cnt++;
			break;
	}

	/* 통계 계산 */
	gap = Slp_TimeGap( slp, &stel->srt, &stel->end);
	if( gap < 0)				return 0;
	if( gap > SLP_STAT_MAX_GAP)	return 0;
	stel->cur  = gap;
	stel->max  = MAX( stel->max, gap);
	stel->min  = MIN( stel->min, gap);
	stel->tot += gap;
	stel->avr  = stel->tot / stel->cnt;
	LogDel( "통계 pos[%d] cur[%6d] avr[%6d] max[%6d] min[%6d] tot[%9d]", 
			pos, stel->cur, stel->avr, stel->max, stel->min, stel->tot);


	return 1;
}

/** ***************************************************************************
**  @fu         int Slp_( SLP *slp)
**  @param      SLP *slp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  시간 차이 구하기
***************************************************************************** */
int Slp_TimeGap( SLP *slp, struct timeval *tv_1, struct timeval *tv_2)
{
	int		int_gap;
	time_t	sec_gap;
	time_t	usec_gap;

	sec_gap = tv_2->tv_sec - tv_1->tv_sec;
	usec_gap = tv_2->tv_usec - tv_1->tv_usec;

	if( usec_gap < 0)
	{
		sec_gap--;
		usec_gap = 1000000 + usec_gap;
	}

	int_gap = sec_gap * 1000000 + usec_gap;

	return int_gap;
}



/** ***************************************************************************
**  @fu         int Slp_( SLP *slp)
**  @param      SLP *slp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  통계 초기화
***************************************************************************** */
int Slp_StatisReset( SLP *slp)
{
	int			i;
	SLP_STATIS	*stat;

	Slp_Lock( slp);
	for( i = 0; i < SLP_MAX_STATIS; i++)
	{
		stat = &slp->status->statis[ i];
		memset( stat, 0x00, sizeof( SLP_STATIS));
	}
	Slp_Unlock( slp);

	return 1;
}

/** ***************************************************************************
**  @fu         int Slp_( SLP *slp)
**  @param      SLP *slp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  통계 출력
***************************************************************************** */
int Slp_PrintStic( SLP *slp)
{
	int			i;
	int			cols = 80;
	char		*gubun[ 20] = { "수신", "거부", "접수", "주문", "취소", "체결", "전송", "직결", "시세", NULL, NULL};

	SLP_STATIS	*stel;

	printf( "[매칭엔진 통계]\n");

	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");
	printf( "%6s ", "[구분]");
	printf( "%12s ", "count");
	printf( "%12s ", "  max");
	printf( "%12s ", "  min");
	printf( "%12s ", "  tot");
	printf( "%12s ", "  avr");
	printf( "\n");

	stel = &slp->status->statis[ 0];

	for( i = 0; gubun[ i] != NULL; i++)
	{
		printf( " %.4s  ", gubun[ i]);
		printf( "%12d ", stel->cnt);
		printf( "%12d ", stel->max);
		printf( "%12d ", stel->min);
		printf( "%12d ", stel->tot);
		printf( "%12d ", stel->avr);
		printf( "\n");
		stel++;
	}
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	return 1;
}


/** ***************************************************************************
**  @fn         SLP *Slp_()
**  @param      none
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp_sub library
***************************************************************************** */
int Slp_StatCurr( SLP *slp)
{
	int			i, num;
	int			col = 80;
	SLP_CURR	*curr;
	
	LogRaw( "Current status\n");
	for( i = 0; i < col; i++) LogRaw( "-");
	LogRaw( "\n");

	for( num = 0; num < SLP_MAX_CURR; num++)
	{
		curr = &slp->status->curr[ num];
		if( curr->num < 0) break;
		LogRaw( " %2d ", curr->num);
		LogRaw( " %s ", curr->str);
		LogRaw( " %1d ", curr->point);
		LogRaw( " %s ", curr->comment);
		LogRaw( "\n");
	}
	for( i = 0; i < col; i++) LogRaw( "-");
	LogRaw( "\n");

	return i;
}

/** ***************************************************************************
**  @fn         SLP *Slp_()
**  @param      none
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp_sub library
***************************************************************************** */
int Slp_StatIndex( SLP *slp)
{
	int			i, num;
	int			col = 125;
	SLP_INDEX	*index;
	
	LogRaw( "Index status\n");

	for( i = 0; i < col; i++) LogRaw( "-");
	LogRaw( "\n");

	LogRaw( "            current                start       count        curr                  base                  cont   ");
	LogRaw( "\n");
	LogRaw( "pos ");
	LogRaw( "symbol  ");
	LogRaw( "base ");
	LogRaw( "cont ");
	LogRaw( "point ");
	LogRaw( "unit ");
	LogRaw( "  bid ");
	LogRaw( "  ask ");
	LogRaw( "  bid ");
	LogRaw( "  ask ");

	LogRaw( "   bid     ");
	LogRaw( "   ask     ");
	LogRaw( "   bid     ");
	LogRaw( "   ask     ");
	LogRaw( "   bid     ");
	LogRaw( "   ask     ");
	LogRaw( "\n");
	for( i = 0; i < col; i++) LogRaw( "-");
	LogRaw( "\n");

	for( num = 0; num < SLP_MAX_IDX( slp); num++)
	{
		index = SLP_GET_IDX( slp, num);
		if( index->pair.num < 0) break;
		LogRaw( " %2d ", index->pos);
		LogRaw( "%.7s ", index->pair.symbol);
		LogRaw( "%4d ", index->pair.base);
		LogRaw( "%4d ", index->pair.cont);
		LogRaw( "%5d ", index->pair.point);
		LogRaw( "%4d ", index->pair.unit);
		LogRaw( "%5d ", index->start[ 0]);
		LogRaw( "%5d ", index->start[ 1]);
		LogRaw( "%5d ", index->cnt[ 0]);
		LogRaw( "%5d ", index->cnt[ 1]);

		LogRaw( "%10.5f ", index->sise[ 0].bidprc);
		LogRaw( "%10.5f ", index->sise[ 0].askprc);
		LogRaw( "%10.5f ", index->sise[ 1].bidprc);
		LogRaw( "%10.5f ", index->sise[ 1].askprc);
		LogRaw( "%10.5f ", index->sise[ 2].bidprc);
		LogRaw( "%10.5f ", index->sise[ 2].askprc);

		LogRaw( "%s ", index->pair.comment);
		LogRaw( "\n");
	}
	for( i = 0; i < col; i++) LogRaw( "-");
	LogRaw( "\n");

	return i;
}

