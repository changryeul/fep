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
int Mon_Order( MAT *mat, int pos)
{
	int			id;
	char		*title;
	MAP			*map;
	MAP_MENU	*menu;

	int			field, line = 0;
	MATSISE		*sise;
	int			start_pos;

	char		name[ 32];
	MAP_FIELD	*fp = NULL;

	if( pos <= 0) 					pos = 1;
	else 
	if( pos >= mat->map->stat.wpos) pos = mat->map->stat.wpos - (( mat->map->stat.wpos % 50) -1);

	map = Map_Open( Param->map_order);
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", Param->map_name);
		goto error;
	}

	menu = Map_MenuOpen( map, '1');
	if( menu == NULL) goto error_1;

	Mon_OrderInit( map, mat, pos);
	Map_DisplayMap( map);
	Map_Message( map, "loop start");

	while( Continue)
	{
		switch( menu->select->id / 1000)
		{
			case 100:	/* pos */
				if( menu->select->data != NULL) Map_Message( map, "start=[%d]", pos);
				else							Map_Message( map, "start=[%d]", pos);
				break;
			case 101:	/* gubun */
				Map_Message( map, "구분 = [완료] [주문] [체결]");
				break;
			case 102:	/* stat */
				Map_Message( map, "주문상태 = [주문] [체결] [취소] [강취:강제취소] [정정]");
				break;
			case 103:	/* rcv_time */
				Map_Message( map, "주문수신시간");
				break;
			case 104:	/* clordid */
				Map_Message( map, "주문번호");
				break;
			case 105:	/* symbol */
				Map_Message( map, "종목번호");
				break;
			case 106:	/* meme */
				Map_Message( map, "매매구분 = [매수] [매도]");
				break;
			case 107:	/* price */
				Map_Message( map, "주문유형 = [시장가] [지정가] [예약주문]");
				break;
		}
		Mon_OrderInit( map, mat, pos);
		Map_CursorOff( map);
		Map_DisplayField( map);
		Map_CursorOn( map);
		id = Map_Menu( map, menu, Param->timeout);
		field =  id / 1000;
		line  =  id % 1000;

		switch( menu->key)
		{
			case 'R':	/* refrash */
			case 'r':
				Mon_OrderInit( map, mat, pos);
				Map_DisplayMap( map);
				Map_Message( map, "Reload screen.");
				continue;
			case WIN_KEY_ENTER:	/* record 상세 */
				sprintf( name, "pos_%d", line);
				fp = Map_GetFieldPtr( map, name);
				if( fp->data != NULL) Mon_ProcessFileVi( map, mat, *( int *)fp->data, 0);
				break;
			case WIN_KEY_PGUP:
				pos -= 50;
				if( pos <  1) pos = 1;
				Mon_OrderInit( map, mat, pos);
				Map_DisplayMap( map);
				break;
			case WIN_KEY_PGDN:
				pos += 50;
				if( pos >=  mat->map->stat.wpos) pos -= 50;
				Mon_OrderInit( map, mat, pos);
				Map_DisplayMap( map);
				break;
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
int Mon_OrderInit( MAP *map, MAT *mat, int line)
{
	int			i;
	char		name[ 32];
	MAT_STATUS	*stat;
	MAT_RECORD	*rec;
	MAT_HEAD	*head;
	ORDER		*obook;
	MAP_FIELD	*ptr;

#if 0
	static char	*gubun[]    = { "완료", "주문", "체결", "    ", "" };
	static char	*ord_stat[] = { "    ", "주문", "체결", "취소", "강취", "정정", "그룹", "    ", "" };
	static char	*side[]     = { "    ", "매수", "매도", "" };
	static char	*price[]    = { "    ", "시장", "지정", "예약", "" };
	static char	*orig[]     = { "    ", "고객", "내부", "대행", "" };
	static char	*type[]     = { "    ", "시장", "지정", "예약", "" };
	static char	*prty[]     = { "    ", "일반", "고정", "대행", "" };
	static char	*tran[]     = { "    ", "일반", "MAR ", "RFQ ", "RFS", "예약", "기간", "일괄", "", "", "햇지", "" };
#endif

	Map_SetField( map, "time",	Mon_TimeSet, &Mon->cur_time);

	stat = &mat->map->stat;

    /************************************************/
    /* Mat Index                                    */
    /************************************************/
	for( i = 0; ( i + line) < stat->wpos; i++)
	{
		sprintf( name, "pos_%d", i);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		rec   = &mat->map->rec[ i + line];
		head  = &rec->head;
		obook = ( ORDER *)&rec->ord;

		sprintf( name, "pos_%d", i);			Map_SetDataPtr( map, name, &rec->pos);
												Map_SetUserPtr( map, name, 0, &rec);
		sprintf( name, "gubun_%d", i);			Map_SetDataPtr( map, name, StrCodeGubun[ Mat_StrCode( head->gubun)]);

		sprintf( name, "stat_%d", i);			Map_SetDataPtr( map, name, StrCodeStat[ Mat_StrCode( head->ord_stat)]);
		ptr = Map_GetFieldPtr( map, name);
		switch( head->ord_stat)
		{
			case 6:		/* 그룹 */
				ptr->attr[ MAP_ATTR_FORE] = MAP_ATTR_YELLOW;
				break;
			case 5:		/* 정정 */
				ptr->attr[ MAP_ATTR_FORE] = MAP_ATTR_CYAN;
				break;
			case 4:		/* 강취 */
				ptr->attr[ MAP_ATTR_FORE] = MAP_ATTR_RED;
				break;
			case 3:		/* 취소 */
				ptr->attr[ MAP_ATTR_FORE] = MAP_ATTR_BLUE;
				break;
			case 2:		/* 체결 */
				ptr->attr[ MAP_ATTR_FORE] = MAP_ATTR_GREEN;
				break;
			case 1:		/* 주문 */
				ptr->attr[ MAP_ATTR_FORE] = MAP_ATTR_YELLOW;
				break;
			default:
			case 0:		/* none */
				ptr->attr[ MAP_ATTR_FORE] = MAP_ATTR_WHITE;
				break;
		}

		if( head->rcv_time.tv_sec == 0)
		{
			sprintf( name, "rcv_time_%d", i);	Map_SetField( map, name, Mon_TimeStr, &head->ord_time.tv_sec);
		}
		else
		{
			sprintf( name, "rcv_time_%d", i);	Map_SetField( map, name, Mon_TimeStr, &head->rcv_time.tv_sec);
		}
		sprintf( name, "clordid_%d", i);	Map_SetDataPtr( map, name, &obook->ClOrdID);
		sprintf( name, "symbol_%d", i);		Map_SetDataPtr( map, name, &obook->Symbol);
		sprintf( name, "meme_%d", i);		Map_SetDataPtr( map, name, StrCodeSide[ Mat_StrCode( obook->Side[ 0] - '0')]);
		sprintf( name, "price_%d", i);		Map_SetDataPtr( map, name, StrCodePrice[ Mat_StrCode(obook->OrdType[ 0] - '0')]);
		sprintf( name, "ord_price_%d", i);	Map_SetDataPtr( map, name, &obook->Price);
		if( head->ord_stat == 1 || head->ord_stat == 6)
		{
			sprintf( name, "exe_price_%d", i);	
			ptr = Map_GetFieldPtr( map, name);
			ptr->attr[ MAP_ATTR_FORE] = MAP_ATTR_BLACK;
			ptr->attr[ MAP_ATTR_BACK] = MAP_ATTR_NONE;
			ptr->attr[ MAP_ATTR_BRIGHT] = '1';
			ptr->attr[ MAP_ATTR_BOLD] = '0';
			Map_SetDataPtr( map, name, &head->fee_out.rec[ 0].d_fx_csac_prc);
		}
		else
		{
			sprintf( name, "exe_price_%d", i);	
			ptr = Map_GetFieldPtr( map, name);
			ptr->attr[ MAP_ATTR_FORE] = MAP_ATTR_WHITE;
			ptr->attr[ MAP_ATTR_BACK] = MAP_ATTR_BLACK;
			ptr->attr[ MAP_ATTR_BRIGHT] = '0';
			ptr->attr[ MAP_ATTR_BOLD] = '0';
			Map_SetDataPtr( map, name, &head->exe_price);
		}
		sprintf( name, "sett_%d", i);		Map_SetDataPtr( map, name, &obook->SettType);
		sprintf( name, "orig_%d", i);		Map_SetDataPtr( map, name, StrCodeOrig[ Mat_StrCode( obook->OrgnGb[ 0] - '0')]);
		sprintf( name, "type_%d", i);		Map_SetDataPtr( map, name, StrCodePrty[ Mat_StrCode( obook->TrdTypeDcd[ 0] - '0')]);
		sprintf( name, "tran_%d", i);		Map_SetDataPtr( map, name, StrCodeTran[ Mat_StrCode( obook->TranPtrnCd[ 0] - '0')]);
	}

	while( 1)
	{
		sprintf( name, "pos_%d", i);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "pos_%d", i);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "gubun_%d", i);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "stat_%d", i);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "rcv_time_%d", i);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "clordid_%d", i);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "symbol_%d", i);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "meme_%d", i);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "price_%d", i);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ord_price_%d", i);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "exe_price_%d", i);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sett_%d", i);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "orig_%d", i);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "type_%d", i);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "tran_%d", i);		Map_SetDataPtr( map, name, NULL);

		i++;
	}

	return 1;
	
}

int Mon_GetCode( int value)
{
	if( value < 1 || value > 10)	return 0;
	else							return value;
}

#if 0
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
	char		file_name[ 512];
	char		cmd[ 1024];
	time_t		cur_time;

	Map_Message( main_map, "pos=[%d]", pos);

	time( &cur_time);
	sprintf( file_name, "/tmp/mat_%d_%ld.dat", pos, cur_time);
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
	remove( file_name);

	return 1;
}

int Mon_ProcessFile( MAP *main_map, MAT *mat, int pos, int side)
{
	int			rtn;
	FILE		*fp;
	time_t		cur_time;
	char		file_name[ 512];
	char		map_name[ 512];
	MAT_RECORD	*rec;

	Map_Message( main_map, "pos=[%d]", pos);

	rec = &mat->map->rec[ pos];

	time( &cur_time);
	sprintf( file_name, "/tmp/mat_%d_%ld.dat", pos, cur_time);
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
	remove( file_name);

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
		if( cur_time - sise->ctime < 10)		
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
#endif
