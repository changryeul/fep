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
int Mon_Main( MAT *mat, SMQ *smq, char *map_name)
{
	int			id;
	char		*title;
	MAP			*map;
	MAP_MENU	*menu;

	int			field, line;
	MATSISE		*sise;
	int			start_pos;

	if( map_name != NULL)
	{
		sprintf( Param->map_name, "%s/map/%s", getenv( "MAT_CFG"), map_name);
	}

	map = Map_Open( Param->map_name);
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", Param->map_name);
		goto error;
	}


	Mon_MainInit( map, mat, smq);

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
							LogDel( "field=[%d] line=[%d] start_pos=[%d]", field, line, start_pos);
							break;
						}
						Map_CursorOff( map);
						Mon_ProcessRecord( map, "record.map", mat, line, 0);
						Map_DisplayMap( map);
						break;
					case 106:
						start_pos = mat->map->index[ line].start[ 1].start;
						if( start_pos < 0)
						{
							Map_Message( map, "주문이 없습니다.");
							LogDel( "field=[%d] line=[%d] start_pos=[%d]", field, line, start_pos);
							LogDel( "cnt=[%d]", mat->map->index[ line].start[ 1].start_cnt);
							break;
						}
						Map_CursorOff( map);
						Mon_ProcessRecord( map, "record.map", mat, line, 1);
						Map_DisplayMap( map);
						break;
					case 108:
					case 109:
						sise = &mat->map->index[ line].sise_curr;
						title = " 현재 sise ";
						Map_CursorOff( map);
						Mon_ProcessSise( map, "sise.map", sise, title);
						Map_DisplayMap( map);
						break;
					case 110:
					case 111:
						sise = &mat->map->index[ line].sise_base;
						title = " 기준통화 sise ";
						Mon_ProcessSise( map, "sise.map", sise, title);
						Map_DisplayMap( map);
						Map_CursorOff( map);
						break;
					case 112:
					case 113:
						sise = &mat->map->index[ line].sise_cont;
						title = " 상대통화 sise ";
						Map_CursorOff( map);
						Mon_ProcessSise( map, "sise.map", sise, title);
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
int Mon_MainInit( MAP *map, MAT *mat, SMQ *smq)
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
    /* Matching Engine                              */
    /************************************************/
	Map_SetField( map,   "ctime",		Mon_TimeStr, &mat->map->stat.ctime);
	Map_SetDataPtr( map, "max_rec",		&mat->map->stat.max_rec);
	Map_SetDataPtr( map, "rec_cnt",		&mat->map->stat.rec_cnt);
	Map_SetDataPtr( map, "exe_cnt",		&mat->map->stat.exe_cnt);
	Map_SetDataPtr( map, "wpos",		&mat->map->stat.wpos);
	Map_SetDataPtr( map, "key",			&mat->key);
	Map_SetDataPtr( map, "ordp",		&mat->map->stat.mat_pipe);
	Map_SetDataPtr( map, "exep",		&mat->map->stat.exe_pipe);
	Map_SetDataPtr( map, "conform",		&mat->map->conform_cnt);

    /************************************************/
    /* SMQ                                          */
    /************************************************/
	Map_SetField( map,   "q_ctime",		Mon_TimeStr, &smq->map->stat.ctime);
	Map_SetDataPtr( map, "q_max_rec",	&smq->map->stat.max_rec);
	Map_SetDataPtr( map, "q_rec_cnt",	&smq->map->stat.rec_cnt);
	Map_SetDataPtr( map, "q_wpos",		&smq->map->stat.wpos);
	Map_SetDataPtr( map, "q_key",		&smq->map->stat.key);
	Map_SetDataPtr( map, "q_shm",		&smq->mem->id);
	Map_SetDataPtr( map, "q_sem",		&smq->sem_mem->id);
	Map_SetDataPtr( map, "q_pipe",		&smq->map->stat.pipe_dir);

    /************************************************/
    /* 장운영                                       */
    /************************************************/
	Map_SetField( map,   "s_time",		Mon_TimeStr, &mat->map->stat.start);
	Map_SetField( map,   "e_time",		Mon_TimeStr, &mat->map->stat.end);
	Map_SetDataPtr( map, "l_day",		&mat->map->stat.last_day);
	Map_SetField( map,   "l_time",		Mon_TimeStr, &mat->map->stat.last_time);

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
		sprintf( name, "base_%d", line);	Map_SetDataPtr( map, name, &mat->map->current[ index->base_cur].str);
		sprintf( name, "cont_%d", line);	Map_SetDataPtr( map, name, &mat->map->current[ index->cont_cur].str);
		sprintf( name, "unit_%d", line);	Map_SetDataPtr( map, name, &index->unit);
		sprintf( name, "point_%d", line);	Map_SetDataPtr( map, name, &index->point);
		sprintf( name, "bid_c_%d", line);	Map_SetDataPtr( map, name, &index->start[ 0].start_cnt);
		sprintf( name, "ask_c_%d", line);	Map_SetDataPtr( map, name, &index->start[ 1].start_cnt);
		sprintf( name, "mat_cnt_%d", line);	Map_SetDataPtr( map, name, &index->mat_cnt);
		sprintf( name, "mat_gap_%d", line);	Map_SetField  ( map, name, Mon_GapProc, index);
		sprintf( name, "m_time_%d", line);	Map_SetField  ( map, name, Mon_GapTime, &index->mat_time);

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

		sprintf( name, "base_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "cont_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "unit_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "point_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "bid_c_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ask_c_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mat_cnt_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "m_time_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sis_cnt_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "s_time_%d", line);	Map_SetDataPtr( map, name, NULL);

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
		sprintf( name, "p_time_%d", i);		Map_SetField(   map, name, Mon_TimeStr, &stel->end.tv_sec);
		sprintf( name, "p_msec_%d", i);		Map_SetDataPtr( map, name, &stel->end.tv_usec);
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

	/*
	Map_SetField( map, "name", myfunc, mat->aa);
	Map_SetDataPtr( map, "name", mat->aa);
	*/

	return 1;
	
}

int Mon_ProcessRecord( MAP *main_map, char *map_file, MAT *mat, int line, int side)
{
	int			rtn;
	char		map_name[ 512];
	MAP			*map;
	MAP_FIELD	*field;
	MAP_MENU	*menu;
	char		title[ 64] = "INDEX VIEW";

	MAT_INDEX	*index;

	index = &mat->map->index[ line];

	sprintf( map_name, "%s/map/%s", getenv( "MAT_CFG"), map_file);
	map = Map_Open( map_name);
	if( map == NULL)
	{
		Map_Message( main_map, "Map_Open error. name=[%s]", map_name);
		LogCri( "Map_Open error. name=[%s]", map_name);
		return -1;
	}

	field = Map_GetFieldPtr( map, "title");
	LogDbg( "title=[%s] field=[%p]", title, field);
	if( field != NULL)
	{
		if( side == 0)	
		{
			field->vsz = sprintf( field->data, " [%s/%s] %s 주문 ", 
					Mat_GetCurrentString( mat, index->base_cur), 
					Mat_GetCurrentString( mat, index->cont_cur),
					"매수" 
					);
		}
		else			
		{
			field->vsz = sprintf( field->data, " [%s/%s] %s 주문 ", 
					Mat_GetCurrentString( mat, index->base_cur), 
					Mat_GetCurrentString( mat, index->cont_cur),
					"매도" 
					);
		}
	}

	Mon_ProcessRecordInit( map, mat, line, side);

	menu = Map_MenuOpen( map, '1');
	if( menu == NULL)
	{
		LogCri( "Map_EditOpen error.");
		goto error;
	}

	Map_DisplayMap( map);

	while( Continue)
	{
		Map_DisplayField( map);
		rtn = Map_Menu( map, menu, Param->timeout);

		switch( rtn)
		{
			case 0:
				break;
			default:
				break;
			case -1:
				goto end;
		}
		switch( menu->key)
		{
			case 'r':
			case 'R':
				Mon_ProcessRecordInit( map, mat, line, side);
				Map_Message( main_map, "reload data");
				break;
			/* edit file를 움직이면 core dump */
			case WIN_KEY_ENTER:
				LogDbg( "pos=[%d]", *( int *)menu->select->data);
				Mon_ProcessFileVi( main_map, mat, *( int *)menu->select->data, side);
				Mon_ProcessRecordInit( map, mat, line, side);
				Map_CursorOff( map);
				Map_DisplayMap( map);
				break;
		}
	}

	end:
	Map_MenuClose( map, menu);
	Map_Close( map);

	return 1;

	error:
		Map_Close( map);
		return -1;
}

int Mon_ProcessRecordInit( MAP *map, MAT *mat, int line, int side)
{
	char		name[ 16];
	char		*ptr;

	MAT_RECORD	*rec;
	MAT_INDEX	*index;
	ORDER		*book;
	int			pos;
	int			cnt = 0;

	cnt = 0;
	index = &mat->map->index[ line];
	pos = index->start[ side].start;
	while( pos >= 0)
	{
		rec  = &mat->map->rec[ pos];
		book = ( ORDER *)&rec->ord;
		sprintf( name, "pos_%d", 	cnt); 		Map_SetDataPtr( map, name,		&rec->pos);
		sprintf( name, "prev_%d", 	cnt); 		Map_SetDataPtr( map, name,		&rec->head.prev);
		sprintf( name, "next_%d", 	cnt); 		Map_SetDataPtr( map, name,		&rec->head.next);

		sprintf( name, "sise_%d", 	cnt); 		
		if( side == 0)	Map_SetDataPtr( map, name,		&index->sise_curr.askprc);
		else			Map_SetDataPtr( map, name,      &index->sise_curr.bidprc);

		sprintf( name, "price_%d", 		cnt); 		Map_SetDataPtr( map, name,		&book->Price);
		sprintf( name, "gubun_%d", 		cnt); 		Map_SetDataPtr( map, name,		&rec->head.gubun);
		sprintf( name, "type_%d", 		cnt); 		Map_SetDataPtr( map, name,		&rec->head.mat_type);
		sprintf( name, "OrdType_%d", 	cnt); 		Map_SetDataPtr( map, name,		&book->OrdType);
		sprintf( name, "OrderQty_%d", 	cnt); 		Map_SetDataPtr( map, name,		&book->OrderQty);
		sprintf( name, "ClOrdID_%d", 	cnt); 		Map_SetDataPtr( map, name,		&book->ClOrdID);

		pos = rec->head.next;
		cnt++;
	}

	while( 1)
	{
		sprintf( name, "pos_%d", cnt);
		ptr = ( char *)Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "pos_%d", 		cnt); 		Map_SetDataPtr( map, name,		NULL);
		sprintf( name, "prev_%d", 		cnt); 		Map_SetDataPtr( map, name,		NULL);
		sprintf( name, "next_%d", 		cnt); 		Map_SetDataPtr( map, name,		NULL);
		sprintf( name, "sise_%d", 		cnt); 		Map_SetDataPtr( map, name,		NULL);
		sprintf( name, "price_%d", 		cnt); 		Map_SetDataPtr( map, name,		NULL);
		sprintf( name, "gubun_%d", 		cnt); 		Map_SetDataPtr( map, name,		NULL);
		sprintf( name, "type_%d", 		cnt); 		Map_SetDataPtr( map, name,		NULL);
		sprintf( name, "OrdType_%d", 	cnt); 		Map_SetDataPtr( map, name,		NULL);
		sprintf( name, "OrderQty_%d", 	cnt); 		Map_SetDataPtr( map, name,		NULL);
		sprintf( name, "ClOrdID_%d", 	cnt); 		Map_SetDataPtr( map, name,		NULL);
		cnt++;
		
	}

	return 1;
}

int Mon_ProcessSise( MAP *main_map, char *map_sise, MATSISE *arg_sise, char *title)
{
	int			rtn;
	char		map_name[ 512];
	MAP			*map;
	MAP_FIELD	*field;
	MAP_EDIT	*edit;

	MATSISE		_sise, *sise = &_sise;

	memcpy( sise, arg_sise, sizeof( MATSISE));

	sprintf( map_name, "%s/map/%s", getenv( "MAT_CFG"), map_sise);
	map = Map_Open( map_name);
	if( map == NULL)
	{
		Map_Message( main_map, "Map_Open error. name=[%s]", map_name);
		LogCri( "Map_Open error. name=[%s]", map_name);
		return -1;
	}

	field = Map_GetFieldPtr( map, "title");
	LogDel( "title=[%s] field=[%p]", title, field);
	if( field != NULL)
	{
		field->vsz = sprintf( field->data, " [ %s ] ", title);
	}

	Map_SetDataPtr( map, "type",		&sise->type);
	Map_SetDataPtr( map, "excode",		&sise->excode);
	Map_SetDataPtr( map, "bidex",		&sise->bidex );
	Map_SetDataPtr( map, "askex",		&sise->askex);
	Map_SetDataPtr( map, "symb",		&sise->symb  );
	Map_SetDataPtr( map, "id",			&sise->id  );
	Map_SetDataPtr( map, "date",		&sise->date  );
	Map_SetDataPtr( map, "time",		&sise->time  );
	Map_SetDataPtr( map, "usdbid",		&sise->usdbid);
	Map_SetDataPtr( map, "usdask",		&sise->usdask);
	Map_SetDataPtr( map, "bidprc",		&sise->bidprc);
	Map_SetDataPtr( map, "askprc",		&sise->askprc);
	Map_SetDataPtr( map, "bidqty",		&sise->bidqty);
	Map_SetDataPtr( map, "askqty",		&sise->askqty);
	Map_SetDataPtr( map, "midprc",		&sise->midprc);
	Map_SetDataPtr( map, "fillprc",		&sise->fillprc);
	Map_SetField  ( map, "ctime",		Mon_TimeStr, &sise->ctime);
	Map_SetDataPtr( map, "price_time",	&sise->price_time);

	Map_Message( map, "%s", "edit ");

	edit = Map_EditOpen( map, '1', NULL);
	if( edit == NULL)
	{
		LogCri( "Map_EditOpen error.");
		goto error;
	}

	Map_DisplayMap( map);

	while( Continue)
	{

		Map_DisplayField( map);
#if 0
		rtn = Map_GetKey( map, Param->mon_interval);
#else
		rtn = Map_Edit( map, edit);
#endif
		Map_Message( map, "key=[%d]", rtn);
		switch( rtn)
		{
			case 0:
				break;
			default:
				goto end;
				break;
		}
	}

	end:
	Map_EditClose( map, edit);
	Map_Close( map);

	return 1;

	error:
		Map_Close( map);
		return -1;
}

int Mon_ProcessFileVi( MAP *main_map, MAT *mat, int pos, int side)
{
	FILE		*fp;
	char		file_name[ 512] = "/tmp/file.dat";
	char		cmd[ 1024];

	Map_Message( main_map, "pos=[%d]", pos);

	fp = fopen( file_name, "w+");
	if( fp == NULL)
	{
		LogErr( "file open error. name=[%s]", file_name);
		return -1;
	}

	Mat_RecordToFile( mat, pos, fp);
	fclose( fp);

	sprintf( cmd, "vi %s", file_name);
	system( cmd);

	return 1;
}

int Mon_ProcessFile( MAP *main_map, MAT *mat, int pos, int side)
{
	int			rtn;
	FILE		*fp;
	char		file_name[ 512] = "/tmp/file.dat";
	char		map_name[ 512];
	MAT_RECORD	*rec;

	Map_Message( main_map, "pos=[%d]", pos);

	rec = &mat->map->rec[ pos];

	fp = fopen( file_name, "w+");
	if( fp == NULL)
	{
		LogErr( "file open error. name=[%s]", file_name);
		return -1;
	}

	ORDER_PrintFile( ( ORDER *)&rec->ord, fp);
	fclose( fp);

	sprintf( map_name, "%s/map/%s", getenv( "MAT_CFG"), "file.map");
	rtn = Map_FileEdit( map_name, file_name);
	if( rtn < 0)
	{
		return rtn;
	}

	return 1;
}

int Map_FileEdit( char *map_name, char *file_name)
{
	int			rtn;
	MAP			*map;
	MAP_EDIT	*edit;
	MAP_FIELD	*field;
	FILE		*fp;

	char		*ptr;
	char		rec[ 512];
	char		name[ 32];
	int			line = 0, rec_sz;

	map = Map_Open( map_name);
	if( map == NULL)
	{
		LogCri( "Map_Open error. name=[%s]", map_name);
		return 0;
	}

	edit = Map_EditOpen( map, '1', NULL);
	if( edit == NULL)
	{
		LogCri( "Map_EditOpen error.");
		goto error;
	}

	fp = fopen( file_name, "r");
	if( fp == NULL)
	{
		LogErr( "file open error. name=[%s]", file_name);
		goto error;
	}
	LogDbg( "fp=[%p]", fp);

	while( 1)
	{
		sprintf( name, "line_%d", line);
		field = Map_GetFieldPtr( map, name);
		if( field == NULL) 
		{
			LogDbg( "End of field. line=[%d]", line);
			line--;
			break;
		}

		ptr = fgets( rec, 512, fp);
		if( ptr == NULL) 
		{
			LogDbg( "EOF line=[%d]", line);
			break;
		}

		rec_sz = strlen( rec);
		memcpy( field->data, rec, rec_sz -1);
		field->data[ rec_sz -1] = 0;

		line++;
		if( line >= 512) break;
	}

	LogDbg( "line=[%d]", line);

	Map_DisplayMap( map);

	while( Continue)
	{
		Map_DisplayField( map);
		rtn = Map_Edit( map, edit);
		Map_Message( map, "key=[%d]", rtn);
		switch( rtn)
		{
			case 0:
				continue;
			default:
				break;
		}
		break;
	}

	fclose( fp);
#if 0
	while( line >= 0)
	{
		LogDbg( "line=[%2d] ptr=[%p]", line, file_line[ line]);
		free( file_line[ line]);
		line--;
	}
	free( file_line);
#endif

	Map_EditClose( map, edit);
	Map_Close( map);
	return 1;

#if 0
	error_1:
		fclose( fp);
#endif
	error:
		Map_Close( map);
		return 1;
}

#if 0
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
		sprintf( name, "base_%d", line);	Map_SetDataPtr( map, name, &mat->map->current[ index->base_cur].str);
		sprintf( name, "cont_%d", line);	Map_SetDataPtr( map, name, &mat->map->current[ index->cont_cur].str);
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
#endif

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
int Mon_GapProc( MAP *map, MAP_FIELD *field)
{
	MAT_INDEX	*index;
	int			gap;

	index = field->ptr;

	gap = index->mat_cnt - index->sis_cnt;

	sprintf( field->data, "%4d", gap);

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
int Mon_GapTime( MAP *map, MAP_FIELD *field)
{
	time_t	cur_time;
	time_t	*set_time;
	time_t	gap_time;


	time( &cur_time);
	set_time = field->ptr;

	gap_time = ( cur_time - *set_time);

	if( gap_time >= 10000)	sprintf( field->data, "%4d", 9999);
	else					sprintf( field->data, "%4d", ( int)gap_time);

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
int Mon_Current( MAP *map, MAP_FIELD *field)
{
	MAT_INDEX	*index;
	MATSISE		*sise;
	double		price = 0.0;
	int			point = 0;
	time_t		cur_time;

#if 0
	price = *( double *)field->ptr;
	if( price == 0.0D)	sprintf( field->data, "         ");
	else				sprintf( field->data, "%9f", price);
#else


	index = field->ptr;

	if( !memcmp( field->name,      "SiCuBi", 6))	{ sise = &index->sise_curr; price = sise->bidprc; point = index->point; }
	else if( !memcmp( field->name, "SiCuOf", 6))	{ sise = &index->sise_curr; price = sise->askprc; point = index->point; }
	else if( !memcmp( field->name, "SiBaBi", 6))	{ sise = &index->sise_base; price = sise->bidprc; }
	else if( !memcmp( field->name, "SiBaOf", 6))	{ sise = &index->sise_base; price = sise->askprc; }
	else if( !memcmp( field->name, "SiCoBi", 6))	{ sise = &index->sise_cont; price = sise->bidprc; point = 2; }
	else if( !memcmp( field->name, "SiCoOf", 6))	{ sise = &index->sise_cont; price = sise->askprc; point = 2; }
	else 											return 0;

	if( price == 0.0D)	
	{
		sprintf( field->data, "         ");
		return 0;
	}
	if( point == 0)
	{
		sprintf( field->data, "%10.5f", price);
	}
	else
	{
		/*
		sprintf( field->data, "%10.5f", price);
		*/
		sprintf( field->data, "%*.*f", 10, point, price);
	}

#endif

	if( Param->color)
	{
		time( &cur_time);
		if( sise->ctime == 0)				
		{
			field->attr[ MAP_ATTR_FORE] = MAP_ATTR_WHITE;
			field->attr[ MAP_ATTR_BACK] = MAP_ATTR_BLACK;
			field->attr[ MAP_ATTR_BRIGHT] = '0';
			field->attr[ MAP_ATTR_BOLD] = '0';
		}
		else
		if( cur_time - sise->ctime < 2)		
		{
			field->attr[ MAP_ATTR_FORE] = MAP_ATTR_CYAN;
			field->attr[ MAP_ATTR_BACK] = MAP_ATTR_NONE;
			field->attr[ MAP_ATTR_BRIGHT] = '1';
			field->attr[ MAP_ATTR_BOLD] = '1';
		}
		else
		if( cur_time - sise->ctime < 5)		
		{
			field->attr[ MAP_ATTR_FORE] = MAP_ATTR_WHITE;
			field->attr[ MAP_ATTR_BACK] = MAP_ATTR_NONE;
			field->attr[ MAP_ATTR_BRIGHT] = '1';
			field->attr[ MAP_ATTR_BOLD] = '1';
		}
		else
		if( cur_time - sise->ctime < 30)		
		{
			field->attr[ MAP_ATTR_FORE] = MAP_ATTR_WHITE;
			field->attr[ MAP_ATTR_BACK] = MAP_ATTR_NONE;
			field->attr[ MAP_ATTR_BRIGHT] = '1';
			field->attr[ MAP_ATTR_BOLD] = '0';
		}
		else
		if( cur_time - sise->ctime < 60)	
		{
			field->attr[ MAP_ATTR_FORE] = MAP_ATTR_WHITE;
			field->attr[ MAP_ATTR_BACK] = MAP_ATTR_BLACK;
			field->attr[ MAP_ATTR_BRIGHT] = '0';
			field->attr[ MAP_ATTR_BOLD] = '0';
		}
		else								
		{
			if( sise->price_time != 0)
			{
				if( cur_time - sise->ctime >= sise->price_time)
				{
					field->attr[ MAP_ATTR_FORE] = MAP_ATTR_RED;
					field->attr[ MAP_ATTR_BACK] = MAP_ATTR_BLACK;
					field->attr[ MAP_ATTR_BRIGHT] = '0';
					field->attr[ MAP_ATTR_BOLD] = '0';
				}
			}
			else
			{
				field->attr[ MAP_ATTR_FORE] = MAP_ATTR_BLACK;
				field->attr[ MAP_ATTR_BACK] = MAP_ATTR_NONE;
				field->attr[ MAP_ATTR_BRIGHT] = '1';
				field->attr[ MAP_ATTR_BOLD] = '0';
			}
		}
	}

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
#if 0
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
		MAT_FX_QUOTE_T_Print( &index->sise_curr);
		LogRaw( "\n---base------------------------------------------------------------------------\n");
		MAT_FX_QUOTE_T_Print( &index->sise_base);
		LogRaw( "\n---cont------------------------------------------------------------------------\n");
		MAT_FX_QUOTE_T_Print( &index->sise_cont);
		LogRaw( "\n---------------------------------------------------------------------------\n");
		cnt++;
	}
	else
	{
		sprintf( field->data, "%s", "O");
	}

	return 1;
}
#endif

