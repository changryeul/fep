/** ***************************************************************************
**  @file       mon.c
**  @date       2023/08/23
**  @author     cdc
**  @version    V2.0.20230901
**  @brif
**  모니터링
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <ctype.h>

#include "map.h"
#include "log.h"
#include "etc.h"
#include "mat.h"
#include "smq.h"
#include "order.h"

#include "main.h"
#include "task.h"

extern int	Continue;
extern MON	*Mon;

/** ***************************************************************************
**  @fu         int CmdInsert( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int Mon_SmallMain( MAT *mat, SMQ *smq)
{
	int			id;
	char		*title;
	MAP			*map;
	MAP_MENU	*menu;

	int			field, line;
	MATSISE		*sise;
	int			start_pos;

	map = Map_Open( Param->map_small);
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", Param->map_small);
		goto error;
	}

	Mon_SmallMainInit( map, mat, smq);

	menu = Map_MenuOpen( map, '1');
	if( menu == NULL) goto error_1;

	Map_DisplayMap( map);
	Map_Message( map, "loop start");

	while( Continue)
	{
		Map_CursorOff( map);
		Map_DisplayField( map);
		Map_CursorOn( map);
		id = Map_Menu( map, menu, Param->timeout);
		field =  id / 1000;
		line  =  id % 1000;
		if( toupper( menu->key) == 'R')
		{
			Mon_MainInit( map, mat, smq);
			Map_DisplayMap( map);
			Map_Message( map, "Reload screen.");
			continue;
		}

		switch( id)
		{
			case 0: /* timeout */
				continue;
			case -1:	/* stop monitor */
				break;
			default:
				switch( field)
				{
					case 104:
						start_pos = mat->map->index[ line].start[ 0].start;
						if( start_pos < 0)
						{
							Map_Message( map, "주문이 없습니다.");
							LogMsg( "field=[%d] line=[%d] start_pos=[%d]", field, line, start_pos);
							break;
						}
						Map_CursorOff( map);
						Mon_ProcessRecord( map, "record2.map", mat, line, 0);
						Map_DisplayMap( map);
						break;
					case 106:
						start_pos = mat->map->index[ line].start[ 1].start;
						if( start_pos < 0)
						{
							Map_Message( map, "주문이 없습니다.");
							LogMsg( "field=[%d] line=[%d] start_pos=[%d]", field, line, start_pos);
							LogMsg( "cnt=[%d]", mat->map->index[ line].start[ 1].start_cnt);
							break;
						}
						Map_CursorOff( map);
						Mon_ProcessRecord( map, "record2.map", mat, line, 1);
						Map_DisplayMap( map);
						break;
					case 108:
					case 109:
						sise = &mat->map->index[ line].sise_curr;
						title = " 현재 sise ";
						Map_CursorOff( map);
						Mon_ProcessSise( map, "sise2.map", sise, title);
						Map_DisplayMap( map);
						break;
					case 110:
					case 111:
						sise = &mat->map->index[ line].sise_base;
						title = " 기준통화 sise ";
						Mon_ProcessSise( map, "sise2.map", sise, title);
						Map_DisplayMap( map);
						Map_CursorOff( map);
						break;
					case 112:
					case 113:
						sise = &mat->map->index[ line].sise_cont;
						title = " 상대통화 sise ";
						Map_CursorOff( map);
						Mon_ProcessSise( map, "sise2.map", sise, title);
						Map_DisplayMap( map);
						break;
					default:
						continue;
				}
				continue;
		}
		break;
	}

	Map_CursorOn( map);
	Map_Close( map);
	return 1;

	error_1:
		Map_Close( map);
	error:
		return 0;
}

/** ***************************************************************************
**  @func       int Mon_()
**  @param      MAP	*map
**  @return     성공    - +
**  @retval     실패    - -
**  @brief
**	모니터링맵 초기화
***************************************************************************** */
int Mon_SmallMainInit( MAP *map, MAT *mat, SMQ *smq)
{
	int			i, line = 0;
	char		name[ 32];
	MAT_INDEX	*index;
	SMQ_INDEX	*q_index;
	void		*ptr;
	char		*title[ 20] = { " 수신 ", " 거부 ", " 접수 ", " 주문 ", " 취소 ", " 체결 ", " 전송 ", " 직결 ", " 시세 ", NULL};
	MAT_STATIS	*stel;

	Map_SetField( map, "time",	Mon_TimeSet, &Mon->cur_time);

    /************************************************/
    /* Mat Index                                    */
    /************************************************/
	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		sprintf( name, "base_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		index = &mat->map->index[ i];
		if( index->base_cur == 0 && index->cont_cur == 0) continue;

        sprintf( name, "base_%d", line);    Map_SetDataPtr( map, name, &mat->map->current[ index->base_cur].str);
        sprintf( name, "cont_%d", line);    Map_SetDataPtr( map, name, &mat->map->current[ index->cont_cur].str);
        sprintf( name, "unit_%d", line);    Map_SetDataPtr( map, name, &index->unit);
        sprintf( name, "point_%d", line);   Map_SetDataPtr( map, name, &index->point);
        sprintf( name, "bid_c_%d", line);   Map_SetDataPtr( map, name, &index->start[ 0].start_cnt);
        sprintf( name, "ask_c_%d", line);   Map_SetDataPtr( map, name, &index->start[ 1].start_cnt);
        sprintf( name, "mat_cnt_%d", line); Map_SetDataPtr( map, name, &index->mat_cnt);
        sprintf( name, "mat_gap_%d", line); Map_SetField  ( map, name, Mon_GapProc, index);
        sprintf( name, "m_time_%d", line);  Map_SetField  ( map, name, Mon_GapTime, &index->mat_time);

		sprintf( name, "SiCuBi_%d", line);	Map_SetField( map, name, Mon_Current, index);
		sprintf( name, "SiCuOf_%d", line);	Map_SetField( map, name, Mon_Current, index);
		sprintf( name, "SiBaBi_%d", line);	Map_SetField( map, name, Mon_Current, index);
		sprintf( name, "SiBaOf_%d", line);	Map_SetField( map, name, Mon_Current, index);
		sprintf( name, "SiCoBi_%d", line);	Map_SetField( map, name, Mon_Current, index);
		sprintf( name, "SiCoOf_%d", line);	Map_SetField( map, name, Mon_Current, index);

		line++;
	}

	while( 1)
	{
		sprintf( name, "base_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

        sprintf( name, "base_%d", line);    Map_SetDataPtr( map, name, NULL);
        sprintf( name, "cont_%d", line);    Map_SetDataPtr( map, name, NULL);
        sprintf( name, "unit_%d", line);    Map_SetDataPtr( map, name, NULL);
        sprintf( name, "point_%d", line);   Map_SetDataPtr( map, name, NULL);
        sprintf( name, "bid_c_%d", line);   Map_SetDataPtr( map, name, NULL);
        sprintf( name, "ask_c_%d", line);   Map_SetDataPtr( map, name, NULL);
        sprintf( name, "mat_cnt_%d", line); Map_SetDataPtr( map, name, NULL);
        sprintf( name, "m_time_%d", line);  Map_SetDataPtr( map, name, NULL);
        sprintf( name, "sis_cnt_%d", line); Map_SetDataPtr( map, name, NULL);
        sprintf( name, "s_time_%d", line);  Map_SetDataPtr( map, name, NULL);

		sprintf( name, "SiCuBi_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "SiCuOf_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "SiBaBi_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "SiBaOf_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "SiCoBi_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "SiCoOf_%d", line);	Map_SetDataPtr( map, name, NULL);
		line++;
	}

    /************************************************/
    /* SMQ                                          */
    /************************************************/
	line = 0;
	for( i = 0; i < SMQ_MAX_IDX; i++)
	{
		sprintf( name, "mq_name_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		q_index = &smq->map->index[ i];
		if( strlen( q_index->name) <= 0) continue;

		sprintf( name, "mq_name_%d", line);		Map_SetDataPtr( map, name, &q_index->name);
		sprintf( name, "mq_start_%d", line);	Map_SetDataPtr( map, name, &q_index->start);
		sprintf( name, "mq_end_%d", line);		Map_SetDataPtr( map, name, &q_index->end);
		sprintf( name, "mq_dcount_%d", line);	Map_SetDataPtr( map, name, &q_index->dcnt);
		sprintf( name, "mq_count_%d", line);	Map_SetDataPtr( map, name, &q_index->cnt);
		sprintf( name, "mq_spid_%d", line);		Map_SetDataPtr( map, name, &q_index->spid);
		sprintf( name, "mq_rpid_%d", line);		Map_SetDataPtr( map, name, &q_index->rpid);
		sprintf( name, "mq_stime_%d", line);	Map_SetField(   map, name, Mon_TimeStr, &q_index->stime);
		sprintf( name, "mq_rtime_%d", line);	Map_SetField(   map, name, Mon_TimeStr, &q_index->rtime);
		sprintf( name, "mq_node_%d", line);		Map_SetDataPtr( map, name, &q_index->key);
		line++;
	}

	while( 1)
	{
		sprintf( name, "mq_name_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;


		sprintf( name, "mq_name_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_start_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_end_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_dcount_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_count_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_spid_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_rpid_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_rtime_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_stime_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_node_%d", line);		Map_SetDataPtr( map, name, NULL);
		line++;
	}


    /************************************************/
    /* Statistics                                   */
    /************************************************/
	stel = &mat->map->stat.statis[ MAT_STAT_RCV];
	for( i = 0; title[ i] != NULL; i++)
	{
		sprintf( name, "st_name_%d", i);	Map_SetDataPtr( map, name, title[ i]);
		sprintf( name, "st_cnt_%d", i);		Map_SetDataPtr( map, name, &stel->cnt);
		sprintf( name, "st_cur_%d", i);		Map_SetDataPtr( map, name, &stel->cur);
		sprintf( name, "st_max_%d", i);		Map_SetDataPtr( map, name, &stel->max);
		sprintf( name, "st_min_%d", i);		Map_SetDataPtr( map, name, &stel->min);
		sprintf( name, "st_tot_%d", i);		Map_SetDataPtr( map, name, &stel->tot);
		sprintf( name, "st_avr_%d", i);		Map_SetDataPtr( map, name, &stel->avr);
		stel++;
	}
	while( 1)
	{
		sprintf( name, "st_name_%d", i);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "st_name_%d", i);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "st_cnt_%d", i);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "st_cur_%d", i);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "st_max_%d", i);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "st_min_%d", i);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "st_tot_%d", i);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "st_avr_%d", i);		Map_SetDataPtr( map, name, NULL);
		i++;
	}

	/*
	Map_SetField( map, "name", myfunc, mat->aa);
	Map_SetDataPtr( map, "name", mat->aa);
	*/

	return 1;
	
}

