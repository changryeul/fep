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
int Mon_StatisMain( MAT *mat, SMQ *smq)
{
	int			id;
	char		*title;
	MAP			*map;
	MAP_MENU	*menu;

	int			field, line;
	MATSISE		*sise;
	int			start_pos;

	map = Map_Open( Param->map_statis);
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", Param->map_statis);
		goto error;
	}

	Mon_StatisMainInit( map, mat, smq);

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
int Mon_StatisMainInit( MAP *map, MAT *mat, SMQ *smq)
{
	int			i, line = 0;
	char		name[ 32];
	MAT_INDEX	*index;
	SMQ_INDEX	*q_index;
	void		*ptr;
	char		*title[ 20] = { " 수신 ", " 거부 ", " 접수 ", " 주문 ", " 취소 ", " 체결 ", " 마진 ", " 매칭 ", " 시세 ", NULL};
	MAT_STATIS	*stel;

	Map_SetField( map, "time",	Mon_TimeSet, &Mon->cur_time);

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
		sprintf( name, "p_time_%d", i);     Map_SetField(   map, name, Mon_TimeStr, &stel->end);
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
		sprintf( name, "p_time_%d", i);		Map_SetDataPtr( map, name, NULL);
		i++;
	}

	return 1;
	
}

