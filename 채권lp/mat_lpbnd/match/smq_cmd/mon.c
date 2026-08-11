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

#include "map.h"
#include "log.h"
#include "etc.h"
#include "smq.h"
#include "order.h"

#include "main.h"
#include "task.h"

extern int	Continue;

typedef struct _mon_
{
	int		alarm;
	time_t	cur_time;
}	MON;

MON		_Mon = 
{
	0,
	0
};
MON		*Mon = &_Mon;


/** ***************************************************************************
**  @fu         int CmdInsert( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int Mon_Main( SMQ *smq)
{
	int			id;
	MAP			*map;
	MAP_MENU	*menu;

	map = Map_Open( Param->map_name);
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", Param->map_name);
		goto error;
	}

	Mon_MainInit( map, smq);

	menu = Map_MenuOpen( map, '1');
	if( menu == NULL) goto error_1;

	Map_DisplayMap( map);
	Map_Message( map, "loop start");

	Map_CursorOff( map);
	while( Continue)
	{
		Map_DisplayField( map);
		id = Map_Menu( map, menu, 1000000);

		switch( id)
		{
			case 0: /* timeout */
			default:
				switch( menu->key)
				{
					case 0: /* timeout */
						continue;
					case WIN_KEY_ENTER:
					case WIN_KEY_ESC:
						goto end;
				}
				continue;
		}
	}

	end:
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
int Mon_MainInit( MAP *map, SMQ *smq)
{
	int			i, line = 0;
	char		name[ 32];
	SMQ_INDEX	*index;
	void		*ptr;

	Map_SetField( map, "time",	Mon_TimeSet, &Mon->cur_time);
	Map_SetField( map, "ctime",	Mon_TimeStr, &smq->map->stat.ctime);

	Map_SetDataPtr( map, "rec_cnt",	&smq->map->stat.rec_cnt);
	Map_SetDataPtr( map, "wpos",	&smq->map->stat.wpos);
	Map_SetDataPtr( map, "key",		&smq->map->stat.key);
	Map_SetDataPtr( map, "shm",		&smq->mem->id);
	Map_SetDataPtr( map, "sem",		&smq->sem_mem->id);
	Map_SetDataPtr( map, "pipe",	&smq->map->stat.pipe_dir);

	for( i = 0; i < SMQ_MAX_IDX; i++)
	{
		sprintf( name, "name_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		index = &smq->map->index[ i];
		if( strlen( index->name) <= 0) continue;
		sprintf( name, "name_%d", line);		Map_SetDataPtr( map, name, &index->name);
		sprintf( name, "start_%d", line);		Map_SetDataPtr( map, name, &index->start);
		sprintf( name, "end_%d", line);			Map_SetDataPtr( map, name, &index->end);
		sprintf( name, "dcount_%d", line);		Map_SetDataPtr( map, name, &index->dcnt);
		sprintf( name, "count_%d", line);		Map_SetDataPtr( map, name, &index->cnt);
		sprintf( name, "spid_%d", line);		Map_SetDataPtr( map, name, &index->spid);
		sprintf( name, "rpid_%d", line);		Map_SetDataPtr( map, name, &index->rpid);
		sprintf( name, "stime_%d", line);		Map_SetField(   map, name, Mon_TimeStr, &index->stime);
		sprintf( name, "rtime_%d", line);		Map_SetField(   map, name, Mon_TimeStr, &index->rtime);
		sprintf( name, "node_%d", line);		Map_SetDataPtr( map, name, &index->key);
		line++;
	}

	while( 1)
	{
		sprintf( name, "name_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "name_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "start_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "end_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "dcount_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "count_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "spid_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "rpid_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "stime_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "rtime_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "node_%d", line);	Map_SetDataPtr( map, name, NULL);
		line++;
	}

	Map_SetField( map, "time", Mon_TimeSet, &Mon->cur_time);

	/*
	Map_SetField( map, "name", myfunc, smq->aa);
	Map_SetDataPtr( map, "name", smq->aa);
	*/

	return 1;
	
}

/** ***************************************************************************
**  @func       int Mon_()
**  @param      MAP	*map
**  @return     성공    - +
**  @retval     실패    - -
**  @brief
**	time stemp convert
***************************************************************************** */
int Mon_TimeSet( MAP *map, MAP_FIELD *field)
{
	time_t	*cur_time;

	cur_time = field->ptr;
	time( cur_time);
	sprintf( field->data, "%s", TtoS( *cur_time));

	return 1;
}

/** ***************************************************************************
**  @func       int Mon_()
**  @param      MAP	*map
**  @return     성공    - +
**  @retval     실패    - -
**  @brief
**	time stemp convert
***************************************************************************** */
int Mon_TimeStr( MAP *map, MAP_FIELD *field)
{
	char	buf[ 32];
	time_t	cur_time, *ptr_time;

	time( &cur_time);
	ptr_time = field->ptr;
	sprintf( buf, "%s", TtoS( *ptr_time));

	if( cur_time - *ptr_time > ( time_t)( 24 * 3600))	sprintf( field->data, "%.8s", &buf[ 2]);
	else												sprintf( field->data, "%.8s", &buf[ 11]);

	return 1;
}



