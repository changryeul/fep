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
int Mon_SmqMain( MAT *mat, SMQ *smq)
{
	int			id;
	char		*title;
	MAP			*map;
	MAP_MENU	*menu;

	int			field, line;
	MATSISE		*sise;
	int			start_pos;

	map = Map_Open( Param->map_smq);
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", Param->map_smq);
		goto error;
	}

	Mon_SmqMainInit( map, mat, smq);

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
int Mon_SmqMainInit( MAP *map, MAT *mat, SMQ *smq)
{
	int			i, line = 0;
	char		name[ 32];
	SMQ_INDEX	*q_index;
	void		*ptr;

	Map_SetField( map, "time",	Mon_TimeSet, &Mon->cur_time);

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

	return 1;
}

