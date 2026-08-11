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
#include "mat.h"
#include "smq.h"
#include "order.h"

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
int Mon_Main( MAT *mat, SMQ *smq)
{
	int			rtn;
	int			id;
	char		rec[ 512];
	MAP			*map;
	MAP_MENU	*menu;

	map = Map_Open( "main.map");
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", "main.map");
		goto error;
	}

	Mon_MainInit( map, mat, smq);

	menu = Map_MenuOpen( map, '1');
	if( menu == NULL) goto error_1;

	Map_DisplayMap( map);
	Map_Message( map, "loop start");

	Map_CursorOff( map);
	while( Continue)
	{
		Map_DisplayField( map);
		id = Map_Menu( map, menu, 1000000);
		LogDel( "Map_Menu rtn=[%d]", id);

		switch( id)
		{
			case 0: /* timeout */
				continue;
			case -1:	/* stop monitor */
				break;
			default:
				LogDbg( "select field id=[%d]", id);
				continue;
		}
		break;
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
int Mon_MainInit( MAP *map, MAT *mat, SMQ *smq)
{
	int			rtn, i, line = 0;
	char		name[ 32];
	extern char	MatCurrent[ 32][ 4];
	MAT_INDEX	*index;
	SMQ_INDEX	*q_index;
	void		*ptr;
	char	*title[ 20] = { " 수신 ", " 거부 ", " 접수 ", " 주문 ", " 체결 ", " 전송 ", " 시세 ", NULL, NULL};
	STEL	*stel;

	Map_SetField( map, "time",	Mon_TimeSet, &Mon->cur_time);
	Map_SetField( map, "ctime",	Mon_TimeStr, &mat->map->stat.ctime);

	Map_SetDataPtr( map, "max_rec",	&mat->map->stat.max_rec);
	Map_SetDataPtr( map, "rec_cnt",	&mat->map->stat.rec_cnt);
	Map_SetDataPtr( map, "exe_cnt",	&mat->map->stat.exe_cnt);
	Map_SetDataPtr( map, "wpos",	&mat->map->stat.wpos);
	Map_SetDataPtr( map, "key",		&mat->key);
	Map_SetDataPtr( map, "shm",		&mat->mem->id);
	Map_SetDataPtr( map, "sem",		&mat->sem->id);
	Map_SetDataPtr( map, "pipe",	&mat->map->stat.pipe_name);
	Map_SetDataPtr( map, "conform",	&mat->map->conform_cnt);

	Map_SetField( map, "time", Mon_TimeSet, &Mon->cur_time);

    /************************************************/
    /* Mat Index                                    */
    /************************************************/
	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		sprintf( name, "base_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		index = &mat->map->stat.index[ i];
		if( index->base_cur == 0 && index->cont_cur == 0) continue;
		sprintf( name, "base_%d", line);	Map_SetDataPtr( map, name, MatCurrent[ index->base_cur]);
		sprintf( name, "cont_%d", line);	Map_SetDataPtr( map, name, MatCurrent[ index->cont_cur]);
		sprintf( name, "unit_%d", line);	Map_SetDataPtr( map, name, &index->unit);
		sprintf( name, "point_%d", line);	Map_SetDataPtr( map, name, &index->point);
		sprintf( name, "bid_s_%d", line);	Map_SetDataPtr( map, name, &index->start[ 0]);
		sprintf( name, "bid_c_%d", line);	Map_SetDataPtr( map, name, &index->cnt[ 0]);
		sprintf( name, "ask_s_%d", line);	Map_SetDataPtr( map, name, &index->start[ 1]);
		sprintf( name, "ask_c_%d", line);	Map_SetDataPtr( map, name, &index->cnt[ 1]);

		sprintf( name, "bp_%d", line);	Map_SetDataPtr( map, name, &index->sise_curr.entry[ 0].MDEntry_Px);
		sprintf( name, "ap_%d", line);	Map_SetDataPtr( map, name, &index->sise_curr.entry[ 1].MDEntry_Px);
		sprintf( name, "bbp_%d", line);	Map_SetDataPtr( map, name, &index->sise_base.entry[ 0].MDEntry_Px);
		sprintf( name, "bap_%d", line);	Map_SetDataPtr( map, name, &index->sise_base.entry[ 1].MDEntry_Px);
		sprintf( name, "cbp_%d", line);	Map_SetDataPtr( map, name, &index->sise_cont.entry[ 0].MDEntry_Px);
		sprintf( name, "cap_%d", line);	Map_SetDataPtr( map, name, &index->sise_cont.entry[ 1].MDEntry_Px);

		line++;
	}

	while( 1)
	{
		sprintf( name, "base_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "base_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "cont_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "unit_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "point_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "bid_s_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "bid_c_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ask_s_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ask_c_%d", line);	Map_SetDataPtr( map, name, NULL);

		sprintf( name, "bp_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ap_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "bbp_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "bap_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "cbp_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "cap_%d", line);	Map_SetDataPtr( map, name, NULL);
		line++;
	}

    /************************************************/
    /* SMQ                                          */
    /************************************************/
	for( i = 0; i < SMQ_MAX_IDX; i++)
	{
		sprintf( name, "mq_name_%d", i);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		q_index = &smq->map->index[ i];
		if( strlen( q_index->name) <= 0) continue;

		sprintf( name, "mq_name_%d", i);	Map_SetDataPtr( map, name, &q_index->name);
		sprintf( name, "mq_start_%d", i);	Map_SetDataPtr( map, name, &q_index->start);
		sprintf( name, "mq_end_%d", i);		Map_SetDataPtr( map, name, &q_index->end);
		sprintf( name, "mq_dcount_%d", i);	Map_SetDataPtr( map, name, &q_index->dcnt);
		sprintf( name, "mq_count_%d", i);	Map_SetDataPtr( map, name, &q_index->cnt);
		sprintf( name, "mq_spid_%d", i);	Map_SetDataPtr( map, name, &q_index->spid);
		sprintf( name, "mq_rpid_%d", i);	Map_SetDataPtr( map, name, &q_index->rpid);
		sprintf( name, "mq_stime_%d", i);	Map_SetField(   map, name, Mon_TimeStr, &q_index->stime);
		sprintf( name, "mq_rtime_%d", i);	Map_SetField(   map, name, Mon_TimeStr, &q_index->rtime);
		sprintf( name, "mq_node_%d", i);	Map_SetDataPtr( map, name, &q_index->node);
	}

	while( 1)
	{
		sprintf( name, "mq_name_%d", i);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "mq_name_%d", i);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_start_%d", i);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_end_%d", i);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_count_%d", i);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_spid_%d", i);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_rpid_%d", i);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_rtime_%d", i);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_stime_%d", i);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mq_node_%d", i);	Map_SetDataPtr( map, name, NULL);
		line++;
	}


    /************************************************/
    /* Statistics                                   */
    /************************************************/
	stel = &mat->map->stat.stat.rcv;
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

/** ***************************************************************************
**  @fu         int CmdInsert( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int Mon_List( MAT *mat, char *base_cur, char *cont_cur)
{
	int			rtn;
	int			key, id;
	char		rec[ 512];
	char		map_name[ 512] = "main.map";
	MAP			*map;
	MAP_MENU	*menu;

	map = Map_Open( "list.map");
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", map_name);
		goto error;
	}

	menu = Map_MenuOpen( map, '1');
	if( menu == NULL) goto error_1;

	rtn = Mon_ListInit( map, mat, base_cur, cont_cur);
	if( rtn < 0)
	{
		LogMsg( "mat init error. name=[%s]", map_name);
		goto error_1;
	}

	Map_DisplayMap( map);

	Map_CursorOff( map);
	while( Continue)
	{
		Map_DisplayField( map);
		id = Map_Menu( map, menu, 1000000);

		switch( menu->key)
		{
			case WIN_KEY_ESC:
			case WIN_KEY_ENTER:
				goto end;
			default:
				break;
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
int Mon_ListInit( MAP *map, MAT *mat, char *base_cur, char *cont_cur)
{
	int			rtn, i;
	int			line = 0;
	char		*ptr;
	char		name[ 32];

	int			base, cont;
	char		symbol[ 8];
	MAT_HEAD	*head;
	ORDER		*obook;
	MAT_RECORD	_rec, *rec;
	MAT_INDEX	*index = NULL;

	Map_SetField( map, "time", Mon_TimeSet, &Mon->cur_time);


	for( i = '1'; i <= '2'; i++)
	{
		rec = &_rec;
		sprintf( symbol, "%.3s/%.3s", base_cur, cont_cur);
		memcpy( rec->book.Symbol, symbol, sizeof( rec->book.Symbol));
		rec->book.Side[ 0] = i;

		rec = Mat_GetRecord( mat, rec, MAT_FIRST);
		while( rec != NULL)
		{
			LogDbg( "rec=[%p]");
			sprintf( name, "curr_%d", line);	Map_SetDataPtr( map, name, &rec->book.Symbol);
			sprintf( name, "side_%d", line);	Map_SetDataPtr( map, name, &rec->book.Side);
			sprintf( name, "id_%d", line);		Map_SetDataPtr( map, name, &rec->book.ClOrdID);
			sprintf( name, "org_id_%d", line);	Map_SetDataPtr( map, name, &rec->book.OrigClOrdID);
			sprintf( name, "price_%d", line);	Map_SetDataPtr( map, name, &rec->book.Price);
			line++;
			rec = Mat_GetRecord( mat, rec, MAT_NEXT);
		}
	}

	while( 1)
	{
		sprintf( name, "curr_%d", line);
		ptr = ( char *)Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "curr_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "side_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "id_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "org_id_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "price_%d", line);	Map_SetDataPtr( map, name, NULL);
		line++;
	}

	return 1;
	
}

/** ***************************************************************************
**  @fu         int CmdInsert( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int Mon_Index( MAT *mat)
{
	int			rtn;
	int			key, id;
	char		rec[ 512];
	MAP			*map;
	MAP_MENU	*menu;

	map = Map_Open( "index.map");
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", "index.map");
		goto error;
	}

	menu = Map_MenuOpen( map, '1');
	if( menu == NULL) goto error_1;

	rtn = Mon_IndexInit( map, mat);

	Map_DisplayMap( map);

	Map_CursorOff( map);
	while( Continue)
	{
		Map_DisplayField( map);
		id = Map_Menu( map, menu, 1000000);

		switch( menu->key)
		{
			case WIN_KEY_ESC:
			case WIN_KEY_ENTER:
				goto end;
			default:
				break;
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
int Mon_IndexInit( MAP *map, MAT *mat)
{
	int			rtn, i, line = 0;
	char		name[ 32];
	extern char	MatCurrent[ 32][ 4];
	MAT_INDEX	*index;
	void		*ptr;


	Map_SetField( map, "time", Mon_TimeSet, &Mon->cur_time);

	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		sprintf( name, "base_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		index = &mat->map->stat.index[ i];
		if( index->base_cur == 0 && index->cont_cur == 0) continue;
		sprintf( name, "base_%d", line);	Map_SetDataPtr( map, name, MatCurrent[ index->base_cur]);
		sprintf( name, "cont_%d", line);	Map_SetDataPtr( map, name, MatCurrent[ index->cont_cur]);
		sprintf( name, "pips_%d", line);	Map_SetDataPtr( map, name, &index->unit);
		sprintf( name, "bid_s_%d", line);	Map_SetDataPtr( map, name, &index->start[ 0]);
		sprintf( name, "bid_c_%d", line);	Map_SetDataPtr( map, name, &index->cnt[ 0]);
		sprintf( name, "ask_s_%d", line);	Map_SetDataPtr( map, name, &index->start[ 1]);
		sprintf( name, "ask_c_%d", line);	Map_SetDataPtr( map, name, &index->cnt[ 1]);
		line++;
	}

	while( 1)
	{
		sprintf( name, "base_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "base_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "cont_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "pips_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "bid_s_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "bid_c_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ask_s_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ask_c_%d", line);	Map_SetDataPtr( map, name, NULL);
		line++;
	}

	/*
	Map_SetField( map, "name", myfunc, mat->aa);
	Map_SetDataPtr( map, "name", mat->aa);
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
	time_t	*cur_time;

	cur_time = field->ptr;
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
int Mon_IndexChk( MAP *map, MAP_FIELD *field)
{
	MAT_INDEX	*index;
	double		bid, ask;
	static int	cnt = 0;

	index = field->ptr;
	bid = AtoD( index->sise_curr.entry[ 0].MDEntry_Px, sizeof( index->sise_curr.entry[ 0].MDEntry_Px));
	ask = AtoD( index->sise_curr.entry[ 1].MDEntry_Px, sizeof( index->sise_curr.entry[ 1].MDEntry_Px));

	if( bid <= 0) return 0;
	if( ask <= 0) return 0;

	if( bid > ask)
	{
		sprintf( field->data, "%s", "X");
		Map_Message( map, "[%d] [%.*s] [%.*s][%.*s]", cnt,
			sizeof( index->sise_curr.Symb), &index->sise_curr.Symb,
			sizeof( index->sise_curr.entry[ 0].MDEntry_Px), index->sise_curr.entry[ 0].MDEntry_Px,
			sizeof( index->sise_curr.entry[ 1].MDEntry_Px), index->sise_curr.entry[ 1].MDEntry_Px);
		LogDbg( "[%d] [%.*s] [%.*s][%.*s]", cnt,
			sizeof( index->sise_curr.Symb), &index->sise_curr.Symb,
			sizeof( index->sise_curr.entry[ 0].MDEntry_Px), index->sise_curr.entry[ 0].MDEntry_Px,
			sizeof( index->sise_curr.entry[ 1].MDEntry_Px), index->sise_curr.entry[ 1].MDEntry_Px);
		LogDbg( "[%d] [%.*s] [%.*s][%.*s]", cnt,
			sizeof( index->sise_base.Symb), &index->sise_base.Symb,
			sizeof( index->sise_base.entry[ 0].MDEntry_Px), index->sise_base.entry[ 0].MDEntry_Px,
			sizeof( index->sise_base.entry[ 1].MDEntry_Px), index->sise_base.entry[ 1].MDEntry_Px);
		LogDbg( "[%d] [%.*s] [%.*s][%.*s]", cnt,
			sizeof( index->sise_cont.Symb), &index->sise_cont.Symb,
			sizeof( index->sise_cont.entry[ 0].MDEntry_Px), index->sise_cont.entry[ 0].MDEntry_Px,
			sizeof( index->sise_cont.entry[ 1].MDEntry_Px), index->sise_cont.entry[ 1].MDEntry_Px);
		LogRaw( "\n---curr------------------------------------------------------------------------\n");
		FX_QUOTE_T_Print( &index->sise_curr);
		LogRaw( "\n---base------------------------------------------------------------------------\n");
		FX_QUOTE_T_Print( &index->sise_base);
		LogRaw( "\n---cont------------------------------------------------------------------------\n");
		FX_QUOTE_T_Print( &index->sise_cont);
		LogRaw( "\n---------------------------------------------------------------------------\n");
		cnt++;
	}
	else
	{
		sprintf( field->data, "%s", "O");
	}

	return 1;
}


