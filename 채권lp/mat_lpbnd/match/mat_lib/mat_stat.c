/** ***************************************************************************
**  @file       mat.c
**  @date       2023/08/23
**  @author     cdc
**  @version    V2.0.20230823
**  @brif
**  ¸ÅÄª¿£Áø ¶óÀÌºê·¯¸®
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

char *StrCodeGubun[]	= { "¿Ï·á", "ÁÖ¹®", "Ã¼°á", "    ", "" };
char *StrCodeStat[]		= { "    ", "ÁÖ¹®", "Ã¼°á", "Ãë¼Ò", "°­Ãë", "Á¤Á¤", "±×·ì", "    ", "" };
char *StrCodeSide[]		= { "    ", "¸Å¼ö", "¸Åµµ", "" };
char *StrCodePrice[]	= { "    ", "½ÃÀå", "ÁöÁ¤", "¿¹¾à", "" };
char *StrCodeOrig[]		= { "    ", "°í°´", "³»ºÎ", "´ëÇà", "" };
char *StrCodeType[]		= { "    ", "½ÃÀå", "ÁöÁ¤", "¿¹¾à", "" };
char *StrCodePrty[]		= { "    ", "ÀÏ¹Ý", "°íÁ¤", "´ëÇà", "" };
char *StrCodeTran[]		= { "    ", "ÀÏ¹Ý", "MAR ", "RFQ ", "RFS", "¿¹¾à", "±â°£", "ÀÏ°ý", "", "", "ÇÞÁö", "" };

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  Åë°è setting
**	MAT_MAX_GAP ÀÌ»óÀÇ Åë°è´Â Á¦¿Ü
***************************************************************************** */
int Mat_StatisticsSet( MAT *mat, int pos, int opt)
{
	int				gap;
	MAT_STATIS		*stel;

	stel = &mat->map->stat.statis[ pos];

	switch( opt)
	{
		case MAT_STAT_COUNT:
			gettimeofday( &stel->end, NULL);
			stel->cnt++;
			return 1;

		case MAT_STAT_START:		/* ÃøÁ¤ ½ÃÀÛ */
			gettimeofday( &stel->srt, NULL);
			return 1;

		case MAT_STAT_END:		/* ÃøÁ¤ Á¾·á */
			if( stel->min <= 0 && stel->cnt == 0) stel->min = 999999999;
			gettimeofday( &stel->end, NULL);
			stel->cnt++;
			break;
	}

	/* Åë°è °è»ê */
	gap = Mat_TimeGap( mat, &stel->srt, &stel->end);
	if( gap < 0)				return 0;
	else if( gap > MAT_MAX_GAP)	return 0;
	else if( gap == 0)			gap = 1;
	stel->cur  = gap;
	stel->max  = MAX( stel->max, gap);
	stel->min  = MIN( stel->min, gap);
	stel->tot += gap;
	stel->avr  = stel->tot / stel->cnt;
	LogDel( "Åë°è pos[%d] cur[%6d] avr[%6d] max[%6d] min[%6d] tot[%9d]", 
			pos, stel->cur, stel->avr, stel->max, stel->min, stel->tot);


	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  ½Ã°£ Â÷ÀÌ ±¸ÇÏ±â
***************************************************************************** */
int Mat_TimeGap( MAT *mat, struct timeval *tv_1, struct timeval *tv_2)
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
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  Åë°è ÃÊ±âÈ­
***************************************************************************** */
int Mat_StatisReset( MAT *mat)
{
	int			i;
	MAT_STATIS	*stat;
	MAT_INDEX	*index;

	Mat_Lock( mat);
	for( i = 0; i < MAT_MAX_STATIS; i++)
	{
		stat = &mat->map->stat.statis[ i];
		memset( stat, 0x00, sizeof( MAT_STATIS));
	}

	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		index = &mat->map->index[ i];
		index->mat_cnt = 0;
		index->mat_time = 0;
		index->sis_cnt = 0;
		index->sis_time = 0;
	}
	Mat_Unlock( mat);

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  stat Ãâ·Â
***************************************************************************** */
int Mat_Stat( MAT *mat)
{
	int			i;
	int			cols = 80;

	MAT_INDEX	*index;

	printf( "[ ¸ÅÄª¿£Áø »óÅÂ ]\n");
	printf( "mat                  = [%p]\n", mat);
	printf( "key                  = [0x%08x]\n", mat->key);
	printf( "mem                  = [%p]\n", mat->mem);
	printf( "mem->id              = [%d]\n", mat->mem->id);
	printf( "sem                  = [%p]\n", mat->sem);
	printf( "sem->id              = [%d]\n", mat->sem->id);
	printf( "ord_fd               = [%d]\n", mat->ord_fd);
	printf( "mat_fd               = [%d]\n", mat->mat_fd);
	printf( "exe_fd               = [%d]\n", mat->exe_fd);
	printf( "base(mat->map)       = [%p]\n", ( char *)mat->map);
	printf( "---------------------------------------------\n");
	printf( "ctime                = [%s]\n", TtoS( mat->map->stat.ctime));
	printf( "service              = [%d]\n", mat->map->stat.service);
	printf( "rec_cnt              = [%d]\n", mat->map->stat.rec_cnt);
	printf( "exe_cnt              = [%d]\n", mat->map->stat.exe_cnt);
	printf( "wpos                 = [%d]\n", mat->map->stat.wpos);
	printf( "key                  = [%d]\n", mat->map->stat.key);
	printf( "ord_pipe             = [%s]\n", mat->map->stat.ord_pipe);
	printf( "mat_pipe             = [%s]\n", mat->map->stat.mat_pipe);
	printf( "exe_pipe             = [%s]\n", mat->map->stat.exe_pipe);
	printf( "---------------------------------------------\n");

	printf( "[ INDEX »óÅÂ ]\n");
	printf( "±âÁØÅëÈ­ ");
	printf( "»ó´ëÅëÈ­ ");
	printf( "¼Ò¼öÁ¡ ");
	printf( "¸Å¼ö½ÃÀÛ ");
	printf( "°¹¼ö ");
	printf( "¸Åµµ½ÃÀÛ ");
	printf( "°¹¼ö ");
	printf( "\n");
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		index = &mat->map->index[i];
		if( index->base_cur == 0 && index->cont_cur == 0) continue;
		printf( "%-8s ", mat->map->current[ index->base_cur].str);
		printf( "%-8s ", mat->map->current[ index->cont_cur].str);
		printf( "%6d ",	index->point);
		printf( "%8d ", index->start[ 0].start);
		printf( "%4d ", index->start[ 0].start_cnt);
		printf( "%8d ", index->start[ 1].start);
		printf( "%4d ", index->start[ 1].start_cnt);
		printf( "\n");
	}
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	Mat_StatIndex( mat);

	Mat_StatOrder( mat);
	Mat_StatExecute( mat);
	Mat_PrintStic( mat);
	Mat_PrintConform( mat);
	Mat_StatGroup( mat);

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  index stat Ãâ·Â
***************************************************************************** */
int Mat_StatIndex( MAT *mat)
{
	int			i;
	int			cols = 120;
	char		*ptr;

	MAT_INDEX	*index;

	printf( "[ INDEX »óÅÂ ]\n");
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");
	printf( " no ");
	printf( "pair    ");
	printf( "bid ");
	printf( "ask ");
	printf( "sis_cnt ");
	printf( "mat_cnt ");
	printf( "sis_time ");
	printf( "mat_time ");
	printf( "  curr bid ");
	printf( "  curr ask ");
	printf( "  base bid ");
	printf( "  base ask ");
	printf( "  cont bid ");
	printf( "  cont ask ");
	printf( "\n");
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	/* ÅëÈ­ index */
	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		index = &mat->map->index[i];
		if( index->base_cur == 0 && index->cont_cur == 0) continue;
		printf( "%3d ", index->no);
		printf( "%3s/%3s ", mat->map->current[ index->base_cur].str, mat->map->current[ index->cont_cur].str);
		printf( "%3d ", index->start[ 0].start_cnt);
		printf( "%3d ", index->start[ 1].start_cnt);
		printf( "%7d ", index->sis_cnt);
		printf( "%7d ", index->mat_cnt);

		ptr = TtoS( index->sis_time);
		printf( "%.8s ", &ptr[ 11]);
		ptr = TtoS( index->mat_time);
		printf( "%.8s ", &ptr[ 11]);

		printf( "%10.5f ", index->sise_curr.bidprc);
		printf( "%10.5f ", index->sise_curr.askprc);
		printf( "%10.5f ", index->sise_base.bidprc);
		printf( "%10.5f ", index->sise_base.askprc);
		printf( "%10.5f ", index->sise_cont.bidprc);
		printf( "%10.5f ", index->sise_cont.askprc);
		/*
		printf( "%-3s(%2d)   ", mat->map->current[ index->base_cur].str, index->base_cur);
		printf( "%-3s(%2d)   ", mat->map->current[ index->cont_cur].str, index->cont_cur);
		*/
		printf( "\n");
	}
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  index stat Ãâ·Â
***************************************************************************** */
int Mat_StatIndexPos( MAT *mat, int idx_pos)
{
	int			i;
	int			cols = 80;
	int			pos, cnt = 0;

	MAT_INDEX	*index;
	MAT_RECORD	*rec;
	MAT_HEAD	*head;
	ORDER		*obook;

	printf( "[ INDEX (%d) »óÅÂ ]\n", idx_pos);

	index = &mat->map->index[ idx_pos];
	if( index->base_cur == 0 && index->cont_cur == 0)
	{
		printf( "ÇØ´ç À§Ä¡¿¡ INDEX°¡ ¾ø½À´Ï´Ù. idx_pos=[%d]\n", idx_pos);
		return 1;
	}

	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	MAT_INDEX_PrintFile( index, stdout);


	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	pos = index->start[ 0].start;
	while( pos > 0)
	{
		if( cnt == 0)
		{
			printf( "[ %.3s/%.3s ¸Å¼ö ÁÖ¹® ]\n", 
				mat->map->current[ index->base_cur].str, mat->map->current[ index->cont_cur].str);
			for( i = 0; i < cols; i++) printf( "-"); 
			printf( "\n");
			printf( " À§Ä¡ ");
			printf( " prev ");
			printf( " next ");
			printf( "ÁÖ¹® ¹øÈ£   ");
			printf( " ÁÖ¹® °¡°Ý ");
			printf( "¼ö¼ö·á°¡°Ý ");
			printf( " ½Ã¼¼ °¡°Ý ");
			printf( "\n");
			for( i = 0; i < cols; i++) printf( "-"); 
			printf( "\n");
		}
		rec   = &mat->map->rec[ pos];
		head  = &rec->head;
		obook = ( ORDER *)rec->ord;
		printf( "%5d ", pos);
		printf( "%5d ", rec->head.prev);
		printf( "%5d ", rec->head.next);
		printf( "%.11s ", obook->ClOrdID);
		printf( "%10.5f ", obook->Price);
		printf( "%10.5f ", head->fee_out.rec[ 0].d_fx_csac_prc);
		printf( "%10.5f ", index->sise_curr.askprc);
		printf( "\n");
		pos = rec->head.next;
		cnt++;
	}
	if( cnt > 0)
	{
		for( i = 0; i < cols; i++) printf( "-"); 
		printf( "\n");
		cnt = 0;
	}

	pos = index->start[ 1].start;
	while( pos > 0)
	{
		if( cnt == 0)
		{
			printf( "[ %.3s/%.3s ¸Åµµ ÁÖ¹® ]\n", 
				mat->map->current[ index->base_cur].str, mat->map->current[ index->cont_cur].str);
			for( i = 0; i < cols; i++) printf( "-"); 
			printf( "\n");
			printf( " À§Ä¡ ");
			printf( " prev ");
			printf( " next ");
			printf( "ÁÖ¹® ¹øÈ£   ");
			printf( " ÁÖ¹® °¡°Ý ");
			printf( "¼ö¼ö·á°¡°Ý ");
			printf( " ½Ã¼¼ °¡°Ý ");
			printf( "\n");
			for( i = 0; i < cols; i++) printf( "-"); 
			printf( "\n");
		}
		rec   = &mat->map->rec[ pos];
		head  = &rec->head;
		obook = ( ORDER *)rec->ord;
		printf( "%5d ", pos);
		printf( "%5d ", rec->head.prev);
		printf( "%5d ", rec->head.next);
		printf( "%.11s ", obook->ClOrdID);
		printf( "%10.5f ", obook->Price);
		printf( "%10.5f ", head->fee_out.rec[ 0].d_fx_csac_prc);
		printf( "%10.5f ", index->sise_curr.bidprc);
		printf( "\n");
		pos = rec->head.next;
		cnt++;
	}
	if( cnt > 0)
	{
		for( i = 0; i < cols; i++) printf( "-"); 
		printf( "\n");
		cnt = 0;
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  °øÀ¯¸Þ¸ð¸®¿¡ ³²¾Æ ÀÖ´Â ÁÖ¹® Ãâ·Â
***************************************************************************** */
int Mat_StatOrder( MAT *mat)
{
	int			i;
	int			cols = 80;
	MAT_RECORD	*rec;
	MAT_HEAD	*head;
	ORDER		*obook;

	printf( "[ ³²¾ÆÀÖ´Â ÁÖ¹® ]\n");
	printf( "[ÁÖ¹®] rec_cnt    = [%d]\n", mat->map->stat.rec_cnt);
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	printf( " pos  ");
	printf( "bas ");
	printf( "con ");
	printf( "%15s ", "price");
	printf( "\n");
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	for( i = 0; i < MAT_MAX_RECORD; i++)
	{
		rec   = &mat->map->rec[ i];
		head  = &rec->head;
		obook = ( ORDER *)&rec->ord;
		if( head->gubun != 1) continue;
		printf( "%5d ", i);
		printf( "%.3s ", &obook->Symbol[ 0]);
		printf( "%.3s ", &obook->Symbol[ 4]);
		printf( "%15f ", rec->head.price);
		printf( "\n");
	}
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  °øÀ¯¸Þ¸ð¸®¿¡ ³²¾Æ ÀÖ´Â ÁÖ¹® Ãâ·Â
***************************************************************************** */
int Mat_StatRecord( MAT *mat, int filter)
{
	int			i;
	int			cols = 80;
	MAT_RECORD	*rec;
	MAT_HEAD	*head;
	ORDER		*obook;

	printf( "[ÁÖ¹®] rec_cnt    = [%d]\n", mat->map->stat.rec_cnt);
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	printf( " pos  ");
	printf( "g ");
	printf( " prev ");
	printf( " next ");
	printf( "waitp ");
	printf( "waitn ");
	printf( "grp_p ");
	printf( "grp_n ");
	printf( "bas ");
	printf( "con ");
	printf( "%15s ", "price");
	printf( "\n");
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	for( i = 0; i < MAT_MAX_RECORD; i++)
	{
		rec   = &mat->map->rec[ i];
		head  = &rec->head;
		obook = ( ORDER *)&rec->ord;
		if( head->gubun != filter) continue;
		printf( "%5d ", i);
		printf( "%1d ", head->gubun);
		printf( "%5d ", head->prev);
		printf( "%5d ", head->next);
		printf( "%5d ", head->wait_prev);
		printf( "%5d ", head->wait_next);
		printf( "%5d ", head->grp_prev);
		printf( "%5d ", head->grp_next);
		printf( "%.3s ", &obook->Symbol[ 0]);
		printf( "%.3s ", &obook->Symbol[ 4]);
		printf( "%15f ", rec->head.price);
		printf( "\n");
	}
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  °øÀ¯¸Þ¸ð¸®¿¡ ³²¾Æ ÀÖ´Â Ã¼°á Ãâ·Â
***************************************************************************** */
int Mat_StatExecute( MAT *mat)
{
	int			i;
	int			cols = 80;
	MAT_RECORD	*rec;
	MAT_HEAD	*head;
	ORDER		*obook;

	printf( "[ ³²¾ÆÀÖ´Â Ã¼°á ]\n");
	printf( "[Ã¼°á] exe_cnt    = [%d]\n", mat->map->stat.exe_cnt);
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	printf( " pos  ");
	printf( "bas ");
	printf( "con ");
	printf( "%15s ", "price");
	printf( "\n");
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	for( i = 0; i < MAT_MAX_RECORD; i++)
	{
		rec   = &mat->map->rec[ i];
		head  = &rec->head;
		obook = ( ORDER *)&rec->ord;
		if( head->gubun != 2) continue;
		printf( "%5d ", i);
		printf( "%.3s ", &obook->Symbol[ 0]);
		printf( "%.3s ", &obook->Symbol[ 4]);
		printf( "%15f ", rec->head.price);
		printf( "\n");
	}
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  °øÀ¯¸Þ¸ð¸®¿¡ ³²¾Æ ÀÖ´Â Ã¼°á Ãâ·Â
***************************************************************************** */
int Mat_StatJangCurr( MAT *mat, char *key[])
{
	int				i, pos = 0, n_find = 0;
	int				cols = 80;
	char			*key_ptr, *ptr;
	/*
	char			*hoga_str[ 3] = { "½ÃÀå", "ÁöÁ¤", NULL};
	char			*prod_str[ 6] = { "BAR", "SPT", "FWD", "SWP", "MAR", NULL};
	char			*tnr_str[ 5]  = { "ALL", "TOD", "TOM", "SPT", NULL};
	*/
	MAT_JANG		*jang;
	MAT_JANG_REC	*jp;
	time_t			cur_time;

	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");
	printf( "id   C   H P   T   ½ÃÀÛ½Ã°£            Á¾·á½Ã°£\n");
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");


	jang = &mat->map->jang;
	time( &cur_time);

	for( i = 1; i < jang->cnt; i++)
	{
		jp = &jang->rec[ i];
		if( jp->used != 1) continue;

		pos = 0;
		n_find = 0;
		key_ptr = key[ pos++];
		while( key_ptr != NULL)
		{
			ToUpper( key_ptr);
			ptr = strstr( jp->key, key_ptr);
			if( ptr == NULL) 
			{
				n_find = 1;
				break;
			}
			key_ptr = key[ pos++];
		}
		if( n_find) continue;

		printf( "%3d ", i);
		printf( "%.s ", jp->key);
		printf( "%.3s ", &jp->key[ 0]);
		printf( "%.1s ", &jp->key[ 3]);
		printf( "%.3s ", &jp->key[ 4]);
		printf( "%.3s ", &jp->key[ 7]);
		if( jp->start == 0)				printf( "[31m");
		else if( cur_time < jp->start)	printf( "[33m");
		else							printf( "[32m");
		printf( "%s ", TtoS( jp->start));
		if( cur_time > jp->end)			printf( "[33m");
		else							printf( "[32m");
		printf( "%s ", TtoS( jp->end));
		printf( "[0m");
		printf( "\n");
	}

	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	printf( "jang count = [%d]\n", jang->cnt);

	return 1;

}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  °øÀ¯¸Þ¸ð¸®¿¡ ³²¾Æ ÀÖ´Â Ã¼°á Ãâ·Â
***************************************************************************** */
int Mat_StatJangPair( MAT *mat)
{
#if JANG_OLD
	int				i, j;
	int				first = 1;
	int				cols = 80;
	MAT_INDEX		*index;
	MAT_JANG_REC	*jang;
	char			*jang_id[] = { "   ", "MAR", "TOD", "TOM", "SPT", "FWD", "SWP", NULL, NULL};

	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");
	printf( "ÅëÈ­    »óÇ° ½ÃÀÛ½Ã°£            Á¾·á½Ã°£\n");
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");
	
	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		index = &mat->map->index[i];
		if( index->base_cur == 0 && index->cont_cur == 0) continue;

		printf( "%3s/%3s ", Mat_GetCurrentString( mat, index->base_cur), Mat_GetCurrentString( mat, index->cont_cur));
		first = 1;

		for( j = 1; j < 10; j++)
		{
			jang = &index->jang[ j];
			if( jang->used == 0) continue;
			if( jang->id == 0) continue;

			if( first)		first = 0;
			else			printf( "        ");
			printf( "%s  ", jang_id[ j]);
			printf( "%s ", TtoS( jang->s_time));
			printf( "%s ", TtoS( jang->e_time));
			printf( "\n");
		}
		printf( "\n");
	}

	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");
#endif
	return 1;

}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  ÇÑ°³ÀÇ index table Ãâ·Â
***************************************************************************** */
int Mat_PrintIndex( MAT *mat, MAT_INDEX *index)
{
	printf( "±âÁØÅëÈ­             = [%s][%d]\n", mat->map->current[ index->base_cur].str, index->base_cur);
	printf( "»ó´ëÅëÈ­             = [%s][%d]\n", mat->map->current[ index->cont_cur].str, index->cont_cur);
	printf( "´ÜÀ§                 = [%d]\n",     index->point);
	printf( "¸Å¼ö½ÃÀÛÀ§Ä¡         = [%d]\n",     index->start[ 0].start);
	printf( "¸Å¼öÁÖ¹®¼ö·®         = [%d]\n",     index->start[ 0].start_cnt);
	printf( "¸Åµµ½ÃÀÛÀ§Ä¡         = [%d]\n",     index->start[ 1].start);
	printf( "¸ÅµµÁÖ¹®¼ö·®         = [%d]\n",     index->start[ 1].start_cnt);

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  Åë°è Ãâ·Â
***************************************************************************** */
int Mat_PrintStic( MAT *mat)
{
	int			i;
	int			cols = 80;
	char		*gubun[ 20] = { "¼ö½Å", "°ÅºÎ", "Á¢¼ö", "ÁÖ¹®", "Ãë¼Ò", "Ã¼°á", "¸¶Áø", "¸ÅÄª", "½Ã¼¼", NULL, NULL};

	MAT_STATIS	*stel;

	printf( "[¸ÅÄª¿£Áø Åë°è]\n");

	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");
	printf( "%6s ", "[±¸ºÐ]");
	printf( "%12s ", "count");
	printf( "%12s ", "  max");
	printf( "%12s ", "  min");
	printf( "%12s ", "  tot");
	printf( "%12s ", "  avr");
	printf( "\n");

	stel = &mat->map->stat.statis[ 0];

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
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  Åë°è Ãâ·Â to log file
***************************************************************************** */
int Mat_PrintSticRaw( MAT *mat)
{
	int			i;
	int			cols = 80;
	char		*gubun[ 20] = { "¼ö½Å", "°ÅºÎ", "Á¢¼ö", "ÁÖ¹®", "Ãë¼Ò", "Ã¼°á", "¸¶Áø", "¸ÅÄª", "½Ã¼¼", NULL, NULL};

	MAT_STATIS	*stel;


	for( i = 0; i < cols; i++) LogRaw( "-"); 
	LogRaw( "\n");
	LogRaw( "%6s ", "[±¸ºÐ]");
	LogRaw( "%12s ", "count");
	LogRaw( "%12s ", "  max");
	LogRaw( "%12s ", "  min");
	LogRaw( "%12s ", "  tot");
	LogRaw( "%12s ", "  avr");
	LogRaw( "\n");
	for( i = 0; i < cols; i++) LogRaw( "-"); 
	LogRaw( "\n");

	stel = &mat->map->stat.statis[ 0];

	for( i = 0; gubun[ i] != NULL; i++)
	{
		LogRaw( " %.4s  ", gubun[ i]);
		LogRaw( "%12d ", stel->cnt);
		LogRaw( "%12d ", stel->max);
		LogRaw( "%12d ", stel->min);
		LogRaw( "%12d ", stel->tot);
		LogRaw( "%12d ", stel->avr);
		LogRaw( "\n");
		stel++;
	}
	for( i = 0; i < cols; i++) LogRaw( "-"); 
	LogRaw( "\n");

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  Åë°è Ãâ·Â
***************************************************************************** */
int Mat_PrintConform( MAT *mat)
{
	int			i;
	int			col = 80;

	printf( "[Ã¼°á ¿ªÀü ¹æÁö »óÅÂ]\n");
	printf( "conform cnt=[%d]\n", mat->map->conform_cnt);
	for( i = 0; i < col; i++) printf( "-"); 
	printf( "\n");
	for( i = 0; i < MAT_MAX_CONFORM; i++)
	{
		if( mat->map->conform[ i] < 0) continue;
		printf( "i=[%2d] pos=[%7d]\n", i, mat->map->conform[ i]);
	}
	for( i = 0; i < col; i++) printf( "-"); 
	printf( "\n");

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  Åë°è Ãâ·Â
***************************************************************************** */
int Mat_StatGroup( MAT *mat)
{
	int			i;
	int			col = 80;
	MAT_GROUP	*grp;

	printf( "[±×·ìÁÖ¹® »óÅÂ]\n");
	printf( "grp_pos    = [%d]\n", mat->map->grp_pos);
	for( i = 0; i < col; i++) printf( "-"); 
	printf( "\n");

	printf( " pos  ");
	printf( "srt ");
	printf( "id          ");
	printf( " seq  ");
	printf( " tot  ");
	printf( " cnt  ");
	printf( "\n");
	for( i = 0; i < col; i++) printf( "-"); 
	printf( "\n");

	for( i = 1; i < MAT_MAX_GROUP; i++)
	{
		grp = &mat->map->grp[ i];

		printf( "%5d ", i);
		printf( "%3d ", grp->start);
		printf( "%11.11s ", grp->id);
		printf( "%5d ", grp->seq);
		printf( "%5d ", grp->tot);
		printf( "%5d ", grp->cnt);
		printf( "\n");
	}
	for( i = 0; i < col; i++) printf( "-"); 
	printf( "\n");

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  Åë°è Ãâ·Â
***************************************************************************** */
int Mat_PrintRecord( MAT *mat, int pos)
{
	ORDER		*order;

	order = ( ORDER *)&mat->map->rec[ pos].ord;
	ORDER_Print( order);

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  Åë°è Ãâ·Â
***************************************************************************** */
int Mat_RecordToFile( MAT *mat, int pos, FILE *fp)
{
	MAT_RECORD	*rec;
	MAT_HEAD	*head;
	ORDER		*order;

	rec   = &mat->map->rec[ pos];
	head  = &rec->head;
	order = ( ORDER *)&rec->ord;

	MAT_HEAD_PrintFile( head, fp);
	ORDER_PrintFile( order, fp);

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     ¼º°ø	- 1
**  @retval     ½ÇÆÐ	- -1
**  @brief      
**  ÅëÈ­ Á¶È¸
***************************************************************************** */
int Mat_OrderList( MAT *mat)
{
	int				i, col = 120;
	MAT_RECORD		*rec;
	MAT_HEAD		*head;
	ORDER			*obook;
	/* SPLIT_OUT_ST	*fee; */
#if 0
	char			*gubun[] = { "¿Ï·á", "ÁÖ¹®", "Ã¼°á", "    ", "" };
	char			*ord_stat[] = { "    ", "ÁÖ¹®", "Ã¼°á", "Ãë¼Ò", "°­Ãë", "Á¤Á¤", "±×·ì", "    ", "" };
	char			*side[] = { "¸Å¼ö", "¸Åµµ", "" };
	char			*type[] = { "½ÃÀå", "ÁöÁ¤", "¿¹¾à", "" };
	char			*prty[] = { "ÀÏ¹Ý", "°íÁ¤", "´ëÇà", "" };
	char			*orig[] = { "°í°´", "³»ºÎ", "´ëÇà", "" };
	char			*tran[] = { "ÀÏ¹Ý", "MAR ", "RFQ ", "RFS", "¿¹¾à", "±â°£", "ÀÏ°ý", "", "", "ÇÞÁö", "" };
#endif

	if( mat == NULL)
	{
		LogMsg( "mat == NULL ...  ÁøÇà ÇÒ ¼ö ¾ø½À´Ï´Ù.");
		return 0;
	}

	/*
	LogRaw( "g = 0:empty, 1:ÁÖ¹®, 2:Ã¼°á \n");
	LogRaw( "S = 1:¸Å¼ö, 2:¸Åµµ \n");
	LogRaw( "O = 1:½ÃÀå°¡, 2:ÁöÁ¤°¡, 3:¿¹¾àÁÖ¹® \n");
	*/

	for( i = 0; i < col; i++) LogRaw( "-"); 
	LogRaw( "\n");


	LogRaw( " pos  ");
	LogRaw( "±¸ºÐ ");
	LogRaw( "»óÅÂ ");
	LogRaw( "recv_time           ");
	LogRaw( "ClOrdID     ");
	LogRaw( "Symbol  ");
	LogRaw( "¸Å¸Å ");
	LogRaw( "À¯Çü ");
	LogRaw( "  ÁÖ¹®°¡°Ý      ");
	LogRaw( "  Ã¼°á°¡°Ý      ");
	LogRaw( "    ");
	LogRaw( "»óÇ° ");
	LogRaw( "¿øÃµ ");
	LogRaw( "Å¸ÀÔ ");
	LogRaw( "À¯Çü ");
	LogRaw( "\n");
	for( i = 0; i < col; i++) LogRaw( "-"); 
	LogRaw( "\n");

	for( i = 1; i < MAT_MAX_RECORD; i++)
	{
		rec    = &mat->map->rec[ i];
		head   = &rec->head;
		obook  = ( ORDER *)&rec->ord;
		/* fee    = ( SPLIT_OUT_ST *)&head->fee_out; */
		if( head->ord_stat == 0) continue;
		/* if( head->rcv_time.tv_sec == 0) continue; */

		LogRaw( "%5d ", i);
		LogRaw( "%s ", StrCodeGubun[ head->gubun]);
		LogRaw( "%s ", StrCodeStat[ head->ord_stat]);
		if( head->rcv_time.tv_sec == 0)
		{
			LogRaw( "%s ", TtoS( head->ord_time.tv_sec));
		}
		else
		{
			LogRaw( "%s ", TtoS( head->rcv_time.tv_sec));
		}
		LogRaw( "%.*s ", ( int)sizeof( obook->ClOrdID), obook->ClOrdID);
		LogRaw( "%.*s ", ( int)sizeof( obook->Symbol), obook->Symbol);
		LogRaw( "%s ", StrCodeSide[ Mat_StrCode( obook->Side[ 0] - '0')] );
		LogRaw( "%s ", StrCodeType[ Mat_StrCode( obook->OrdType[ 0] - '0')]);
		LogRaw( "%15f ", obook->Price);
		LogRaw( "%15f ", head->exe_price);
		/* LogRaw( "%15f ", fee->rec[ 0].d_fx_csac_prc); */
		LogRaw( "    ");
		LogRaw( "%.*s  ", ( int)sizeof( obook->SettType), obook->SettType);
		LogRaw( "%s ", StrCodeOrig[ Mat_StrCode( obook->OrgnGb[ 0] - '0')]);
		LogRaw( "%s ", StrCodePrty[ Mat_StrCode( obook->TrdTypeDcd[ 0] - '0')]);
		LogRaw( "%s ", StrCodeTran[ Mat_StrCode( obook->TranPtrnCd[ 0] - '0')]);
		LogRaw( "\n");
	}
	for( i = 0; i < col; i++) LogRaw( "-"); 
	LogRaw( "\n");

	return 1;
}

int Mat_StrCode( int code)
{
	if( code < 1 || code > 10)		return 0;
	else							return code;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - ¸ÅÄª struct pointer
**  @return     ¼º°ø    - 1
**  @retval     ½ÇÆÐ    - -1
**  @brief
**  Åë°è Ãâ·Â
***************************************************************************** */
int MAT_HEAD_File( MAT_HEAD* ptr, FILE *fp)
{
    fprintf( fp, "%s", "----[ MAT_HEAD ]------------------------------------------------------------------------\n");
    fprintf( fp, "0-ºó record, 1-ÁÖ¹® record,   gubun                  4    0 = [%d]\n", 	ptr->gubun);
    fprintf( fp, "ÁÖ¹® Á¢¼ö ½Ã°£                rcv_time               8    8 = [%s:%06ld]\n", 	TtoS( ptr->rcv_time.tv_sec), ptr->rcv_time.tv_usec);
    fprintf( fp, "ÁÖ¹® È®ÀÎ ¼Û½Å ½Ã°£           con_time               8   16 = [%s:%06ld]\n", 	TtoS( ptr->con_time.tv_sec), ptr->con_time.tv_usec);
    fprintf( fp, "ÁÖ¹® È®ÀÎ ´ë±â ½Ã°£           dly_time               8   24 = [%s:%06ld]\n", 	TtoS( ptr->dly_time.tv_sec), ptr->dly_time.tv_usec);
    fprintf( fp, "ÁÖ¹® ÀúÀå ½Ã°£                ord_time               8   32 = [%s:%06ld]\n", 	TtoS( ptr->ord_time.tv_sec), ptr->ord_time.tv_usec);
    fprintf( fp, "ÁÖ¹® Ã¼°á ½Ã°£                mat_time               8   40 = [%s:%06ld]\n", 	TtoS( ptr->mat_time.tv_sec), ptr->mat_time.tv_usec);
    fprintf( fp, "Ã¼°á ¼Û½Å ½Ã°£                snd_time               8   48 = [%s:%06ld]\n", 	TtoS( ptr->snd_time.tv_sec), ptr->snd_time.tv_usec);
	fprintf( fp, ">>> °è»êµÈ ½Ã¼¼ ±âº»ÅëÈ­/»ó´ëÅëÈ­\n");
    MATSISE_PrintFile( &ptr->sise_curr, fp);
	fprintf( fp, ">>> ±âº»ÅëÈ­ ½Ã¼¼ USD/±âº»ÅëÈ­\n");
    MATSISE_PrintFile( &ptr->sise_base, fp);
	fprintf( fp, ">>> »ó´ëÅëÈ­ ½Ã¼¼ USD/»ó´ëÅëÈ­\n");
    MATSISE_PrintFile( &ptr->sise_cont, fp);
    SPLIT_IN_ST_File( &ptr->fee_in, fp);
    SPLIT_OUT_ST_File( &ptr->fee_out, fp);
    fprintf( fp, "ÁÖ¹® °¡°Ý - ¸¶Å©¾÷À» »« °¡°Ý  price                  8    4 = [%f]\n", 	ptr->price);
    fprintf( fp, "Ã¼°á°¡°Ý                      exe_price              8   12 = [%f]\n", 	ptr->exe_price);
    fprintf( fp, "matching type ÀÏ¹Ý = 0        mat_type               4   20 = [%d]\n", 	ptr->mat_type);
    fprintf( fp, "trailling stop ÁÖ¹® pips gap  ts_gap                 4   24 = [%d]\n", 	ptr->ts_gap);
    fprintf( fp, "ÀÌÀü record À§Ä¡, ½ÃÀÛ recor  prev                   4   28 = [%d]\n", 	ptr->prev);
    fprintf( fp, "´ÙÀ½ record À§Ä¡, ³¡ record   next                   4   32 = [%d]\n", 	ptr->next);
    fprintf( fp, "ÁÖ¹® ´ë±â¿­ ÀÌÀü record À§Ä¡  wait_prev              4   36 = [%d]\n", 	ptr->wait_prev);
    fprintf( fp, "ÁÖ¹® ´ë±â¿­ ´ÙÀ½ record À§Ä¡  wait_next              4   40 = [%d]\n", 	ptr->wait_next);
    fprintf( fp, "group ÁÖ¹® ÀÌÀü               grp_prev               4   44 = [%d]\n", 	ptr->grp_prev);
    fprintf( fp, "group ÁÖ¹® ´ÙÀ½               grp_next               4   48 = [%d]\n", 	ptr->grp_next);
    fprintf( fp, "%s", "------------------------------------------------------------------------[ MAT_HEAD ]----\n");

    return sizeof( MAT_HEAD);
}




