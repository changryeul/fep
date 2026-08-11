/** ***************************************************************************
**  @file       map.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  map library module
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "etc.h"
#include "map.h"

MAP	__MapBuf, *__Map = &__MapBuf;

/** ***************************************************************************
**  @func       int MapInit()
**  @param      none
**  @return     0 - 성공
**  @brief
**  맵 파용하기 위한 터미널 초기화
***************************************************************************** */
int MapInit()
{
	WinInit();
	__Map->win = __Win;
	return 0;
}

/** ***************************************************************************
**  @func       int MapEnd()
**  @param      none
**  @return     0 - 성공
**  @brief
**  터미널 세팅을 끝냄
***************************************************************************** */
int MapEnd()
{
	WinEnd();
	printf( "\n");
	return 0;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      map file name
**  @return     성공 - MAP pointer
**  @retval     실패 - NULL
**  @brief
**  맵 파일을 열어 출력을위한 초기화
***************************************************************************** */
MAP	*Map_Open( char *f_name)
{
	int			rtn, sz;
	MAP			*map;

	map = ( MAP *)malloc( sizeof( MAP));
	if( map == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( MAP));
		goto error_1;
	}
	memset( map, 0, sizeof( MAP));
	map->win = Win_Open( NULL);

	sz = strlen( f_name);
	map->f_name = ( char *)malloc( sz +1);
	if( map->f_name == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sz +1);
		goto error_2;
	}
	memcpy( map->f_name, f_name, sz +1);

	map->title = Dll_Open( 0);
	if( map->title == NULL)
	{
		LogCri( "Dll_Open error. map->title");
		goto error_2;
	}

	map->field = Dll_Open( 0);
	if( map->field == NULL)
	{
		LogCri( "Dll_Open error. map->field");
		goto error_2;
	}

	rtn = Map_Load( map);
	if( rtn < 0)
	{
		LogCri( "Map load error. name=[%s]", map->f_name);
		goto error_3;
	}

	return map;

	error_3:
		if( map->title != NULL)
		{
			Dll_Close( map->title);
		}
		if( map->field != NULL)
		{
			Dll_Close( map->field);
		}
	error_2:
		free( map);
	error_1:
		return NULL;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_Delete( MAP *map)
{
	int 	i;
	char	space[ 512];

	memset( space, 0x20, map->sz_x);
	space[ map->sz_x] = 0;

	for( i = map->y; i < map->y + map->sz_y; i++)
	{
		Win_PrintXYA( map->win, map->x, i, MAP_NONE_ATTR, "%*.*s", map->sz_x, map->sz_x, space);
	}

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_Close( MAP *map)
{
	MAP_FIELD		*field;
	int				i;

	if( map->field != NULL)
	{
		field = ( MAP_FIELD *)Dll_GetFirstPtr( map->field);
		while( field != NULL)
		{
			if( field->format != NULL) { free( field->format); field->format = NULL; } /* 20220808 */
			if( field->name != NULL) { free( field->name); field->name = NULL; }
			if( !( field->type[ MAP_TYPE_POINTER] == '1'))	
			if( field->data != NULL) 
			{ 
				if( field->set == 0)
				{
					free( field->data); 
					field->data = NULL; 
				}
			}
			free( field);
			field = NULL;
			field = ( MAP_FIELD *)Dll_GetNextPtr( map->field);
		}
		Dll_Close( map->field);
	}

	for( i = 0; i < map->s_cnt; i++)	free( map->scr[ i]);
	free( map->scr);

	if( map->f_name != NULL) free( map->f_name);


	Win_Close( map->win);
	free( map);

	return 0;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_SetBuffer( MAP *map, char *buffer, int sz)
{
	int		rtn;

	rtn = Win_SetBuffer( map->win, buffer, sz);
	return rtn;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_GetBuffer( MAP *map)
{
	return Win_GetBuffer( map->win);
}

/*
FILE *Map_SetOutput( MAP *map, FILE *ptr)
{
	FILE	*old_ptr;

	old_ptr = Win_SetOutput( map->win, ptr);

	return old_ptr;
}
*/

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_DisplayMap( MAP *map)
{
	int			rtn;
	int			x, y, vsz, sz; 
	char		*attr;
	int			pos = 0;
	MAP_FIELD	*title;

	/* screen */
	while( pos < map->s_cnt)
	{
		x = map->x +1;
		y = map->y + pos +1;
		vsz		= map->sz_y;
		sz		= strlen( map->scr[ pos]);;
		attr	= WIN_ATTR_MAP;
		Win_PrintXYA( map->win, x, y, attr, "%-*.*s", vsz, sz, map->scr[ pos]);
		pos++;
	}

	/* scr_field */
	if( map->title != NULL)
	{
		title = ( MAP_FIELD *)Dll_GetFirstPtr( map->title);
		while( title != NULL)
		{
			LogDel( "title=[%p] title->data=[%s]", map->title, title->data);
			if( title->data != NULL)
			{
				if( title->func != NULL) 
				{
					rtn = title->func( map, title);
					if( rtn < 0)
					{
						return rtn;
					}
				}
				x		= title->x + map->x;
				y		= title->y + map->y;
				vsz		= title->vsz;
				sz		= title->sz;
				attr	= title->attr;
				Win_PrintXYA( map->win, x, y, attr, title->format, title->data);
			}
			title = ( MAP_FIELD *)Dll_GetNextPtr( map->title);
		}
	}
	LogDel( "title ... end");

	Map_DisplayField( map);

	Win_Flush( map->win);

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_DisplayField( MAP *map)
{
	int			rtn;
	MAP_FIELD	*field;

	/* field */
	if( map->field != NULL)
	{
		field = ( MAP_FIELD *)Dll_GetFirstPtr( map->field);
		while( field != NULL)
		{
			LogDel( "field=[%p] field->name=[%s] select=[%d]", field, field->name, field->type[ MAP_TYPE_SELECT]);
#if 1
			if( field->type[ MAP_TYPE_SELECT] == '1')		rtn = Map_DisplayFieldDataA( map, field, MAP_MENU_ATTR);
			else											rtn = Map_DisplayFieldData( map, field);
#else
			/* menu의 select field는 Map_Menu 함수에서 출력하는걸로 바꿈 */
			if( field->type[ MAP_TYPE_SELECT] != '1') 	rtn = Map_DisplayFieldData( map, field);
#endif
			if( rtn < 0) return rtn;
			field = ( MAP_FIELD *)Dll_GetNextPtr( map->field);
		}
	}

	Win_Flush( map->win);

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  한개의 필드 출력
***************************************************************************** */
int Map_DisplayFieldData( MAP *map, MAP_FIELD *field)
{
	int			rtn;

	rtn = Map_DisplayFieldDataA( map, field, field->attr);

	return rtn;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  한개의 필드를 인수의 attribute로  출력
***************************************************************************** */
int Map_DisplayFieldDataA( MAP *map, MAP_FIELD *field, char *attr)
{
	int			rtn;
	int			x, y, vsz/*, sz */;
	char		*a;

	/*
	Map_Message( map, "Map_DisplayFieldData name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
	*/
	LogDel( "Map_DisplayFieldData name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
	LogDel( "                                    x=[%d] y=[%d]", field->x, field->y);
	if( field->data != NULL)
	{
		if( field->func != NULL) 
		{
			rtn = field->func( map, field);
			LogDel( "name=[%s] rtn=[%d] ", field->name, rtn);
			if( rtn <= 0)	return rtn;
		}
		x		= field->x + map->x;
		y		= field->y + map->y;
		vsz		= field->vsz;
		a		= attr;

		LogDel( "field->type[ MAP_TYPE_TYPE]=[%c]", field->type[ MAP_TYPE_TYPE]);
		switch( field->type[ MAP_TYPE_TYPE])
		{
			case MAP_TYPE_INT:
				Win_PrintNXYA( map->win, vsz, x, y, a, field->format, *( int *)field->data);
				LogDel( "int type  name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
				break;
			case MAP_TYPE_FLOAT:
				Win_PrintNXYA( map->win, vsz, x, y, a, field->format, *( float *)field->data);
				LogDel( "float type  name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
				break;
			case MAP_TYPE_LONG:
				Win_PrintNXYA( map->win, vsz, x, y, a, field->format, *( long *)field->data);
				LogDel( "long type  name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
				break;
			case MAP_TYPE_LONG_LONG:
				Win_PrintNXYA( map->win, vsz, x, y, a, field->format, *( long long *)field->data);
				LogDel( "logn long type  name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
				break;
			case MAP_TYPE_DOUBLE:
				Win_PrintNXYA( map->win, vsz, x, y, a, field->format, *( double *)field->data);
				LogDel( "double type  name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
				break;
			default:
				Win_PrintNXYA( map->win, vsz, x, y, a, field->format, field->data);
				LogDel( "default type  name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
				break;
		}
		field->update = 0;
	}
	else
	{
	}
	return 0;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  한개의 필드를 인수의 attribute로  출력
***************************************************************************** */
int Map_DisplayFieldDataNA( MAP *map, MAP_FIELD *field, int d_sz, char *attr)
{
	int			rtn;
	int			x, y, vsz /*, sz */;
	char		*a;

	/*
	Map_Message( map, "Map_DisplayFieldData name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
	*/
	LogDel( "Map_DisplayFieldData name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
	LogDel( "                                    x=[%d] y=[%d]", field->x, field->y);
	if( field->data != NULL)
	{
		if( field->func != NULL) 
		{
			rtn = field->func( map, field);
			LogDel( "name=[%s] rtn=[%d] ", field->name, rtn);
			if( rtn <= 0)	return rtn;
		}
		x		= field->x + map->x;
		y		= field->y + map->y;
		vsz		= field->vsz;
		/* sz		= field->sz; */
		a		= attr;

		LogDel( "field->type[ MAP_TYPE_TYPE]=[%c]", field->type[ MAP_TYPE_TYPE]);
		switch( field->type[ MAP_TYPE_TYPE])
		{
			case MAP_TYPE_INT:
				Win_PrintNXYA( map->win, vsz, x, y, a, field->format, *( int *)field->data);
				LogDel( "int type  name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
				break;
			case MAP_TYPE_FLOAT:
				Win_PrintNXYA( map->win, vsz, x, y, a, field->format, *( float *)field->data);
				LogDel( "float type  name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
				break;
			case MAP_TYPE_LONG:
				Win_PrintNXYA( map->win, vsz, x, y, a, field->format, *( long *)field->data);
				LogDel( "long type  name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
				break;
			case MAP_TYPE_LONG_LONG:
				Win_PrintNXYA( map->win, vsz, x, y, a, field->format, *( long long *)field->data);
				LogDel( "logn long type  name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
				break;
			case MAP_TYPE_DOUBLE:
				Win_PrintNXYA( map->win, vsz, x, y, a, field->format, *( double *)field->data);
				LogDel( "double type  name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
				break;
			default:
				Win_PrintNXYA( map->win, vsz, x, y, a, field->format, field->data);
				LogDel( "default type  name=[%s] data=[%p] func=[%p] start", field->name, field->data, field->func);
				break;
		}
		field->update = 0;
	}
	return 0;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_DisplayUpdate( MAP *map)
{
	int			x, y, vsz, sz; 
	char		*attr;
	MAP_FIELD	*field;

	/* field */
	if( map->field != NULL)
	{
		field = ( MAP_FIELD *)Dll_GetFirstPtr( map->field);
		while( field != NULL)
		{
			if( field->update)
			{
				if( field->data != NULL)
				{
					x		= field->x + map->x;
					y		= field->y + map->y;
					vsz		= field->vsz;
					sz		= field->sz;
					attr	= field->attr;
					Win_PrintXYA( map->win, x, y, attr, "%-*.*s", vsz, sz, field->data);
					field->update = 0;
				}
			}
			field = ( MAP_FIELD *)Dll_GetNextPtr( map->field);
		}
	}

	Win_Flush( map->win);

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_Load( MAP *map)
{
	CFG		*cfg;
	char	*ptr, rec[ 8192];

	cfg = Cfg_Open( map->f_name);
	if( cfg == NULL)
	{
		LogCri( "Map open error. name=[%s]", map->f_name);
		return -1;
	}

	Cfg_Set( cfg, "map");

	ptr = Cfg_GetPtr( cfg, "win_size");
	if( ptr == NULL)
	{
		LogCri( "\"win_size\" field not found at=[%s]", map->f_name);
		goto error_1;
	}
	memcpy( rec, ptr, strlen( ptr) +1);
	Map_GetSize( map, rec);

	map->timeout = Cfg_GetInt( cfg, "timeout");
	if( map->timeout <= 0) map->timeout = 1000000;

	/* get screen */
	Cfg_Set( cfg, "screen");
	ptr = Cfg_GetFirstPtr( cfg);
	while( ptr != NULL)
	{
		memcpy( rec, ptr, strlen( ptr) +1);
		Map_GetScreen( map, rec);
		ptr = Cfg_GetNextPtr( cfg);
	}

	/* get screen_field */
	/*
	Cfg_Set( cfg, NULL);
	Cfg_Set( cfg, "screen_field");
	ptr = Cfg_GetFirstPtr( cfg);
	while( ptr != NULL)
	{
		memcpy( rec, ptr, strlen( ptr) +1);
		Map_GetField( map, rec);
		ptr = Cfg_GetNextPtr( cfg);
	}
	*/

	/* get field */
	Cfg_Set( cfg, "field");
	ptr = Cfg_GetFirstPtr( cfg);
	while( ptr != NULL)
	{
		memcpy( rec, ptr, strlen( ptr) +1);
		if( !memcmp( rec, "base", 4))		Map_GetBase( map, rec);
		else if( !memcmp( rec, "line", 4))	Map_GetLine( map, rec);
		else								Map_GetField( map, rec);
		ptr = Cfg_GetNextPtr( cfg);
	}

	Cfg_Close( cfg);

	return 1;

	error_1:
		Cfg_Close( cfg);
		return -1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_GetSize( MAP *map, char *line)
{
	int		stat = 0;
	char	*token = ",";
	char	rec[ 8192];
	char	*ptr = rec, *end = ptr;

	memcpy( rec, line, strlen( line) +1);

	while( end != NULL)
	{
		end = strpbrk( ptr, token);
		if( end) *end = 0;

		switch( stat)
		{
			case 0:
				map->x = atoi( ptr);
				break;
			case 1:
				map->y = atoi( ptr);
				break;
			case 2:
				map->sz_x = atoi( ptr);
				break;
			case 3:
				map->sz_y = atoi( ptr);
				break;
			default:
				break;
		}

		ptr = end +1;
		stat++;
		/*
		ptr = strtok( NULL, token);
		*/
	}

	LogDel( "x                 =[%d]", map->x);
	LogDel( "y                 =[%d]", map->y);
	LogDel( "sz_x              =[%d]", map->sz_x);
	LogDel( "sz_y              =[%d]", map->sz_y);
	
	return stat;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_GetScreen( MAP *map, char *line)
{
	int		sz;

	sz = sizeof( char *) * ( map->s_cnt + 2);
	map->scr = realloc( map->scr, sz);
	if( map->scr == NULL)
	{
		LogErr( "map->scr realloc error. sz=[%d]", sz);
		goto error_1;
	}

	sz = strlen( line);
	map->scr[ map->s_cnt] = malloc( sz +1);
	if( map->scr[ map->s_cnt] == NULL)
	{
		LogErr( "screen melloc error. sz=[%d]", sz);
		goto error_1;
	}
	memcpy( map->scr[ map->s_cnt], line, sz +1);
	map->s_cnt++;
	
	return 1;

	error_1:
		return -1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_GetBase( MAP *map, char *line)
{
	int		stat = 0;
	char	*token = ":";
	char	rec[ 8192];
	char	*ptr = rec, *end = ptr;

	memcpy( rec, line, strlen( line) +1);

	/* ptr = strtok( line, token); */
	while( end != NULL)
	{
		end = strpbrk( ptr, token);
		if( end) *end = 0;
		switch( stat)
		{
			case 0:		/* id */
				if( memcmp( ptr, "base", 4)) return -1;
				break;
			case 1:		/* name */
				break;
			case 2:		/* base x */
				map->base_x = atoi( ptr);
				break;
			case 3:		/* base y */
				map->base_y = atoi( ptr);
				break;
			default:
				break;
		}
		stat++;
		ptr = end +1;
		/* ptr = strtok( NULL, token); */
	}

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_GetLine( MAP *map, char *line)
{
	int		stat = 0;
	int		start_flag = 0;
	char	*token = ":";
	char	rec[ 8192];
	char	*ptr = rec, *end = ptr;

	memcpy( rec, line, strlen( line) +1);

	/* ptr = strtok( line, token); */
	while( end != NULL)
	{
		end = strpbrk( ptr, token);
		if( end) *end = 0;
		switch( stat)
		{
			case 0:		/* id */
				if( memcmp( ptr, "line", 4)) return -1;
				break;
			case 1:		/* name */
				if( !memcmp( ptr, "start", 5))	start_flag = 1;
				else							start_flag = 0;
				break;
			case 2:		/* start y */
				if( start_flag)	map->line_s = atoi( ptr);
				else			map->line_s = 0;
				break;
			case 3:		/* end y */
				if( start_flag)	map->line_e = atoi( ptr);
				else			map->line_e = 0;
				break;
			case 4:
				if( start_flag)	map->line_n = atoi( ptr);
				break;
			default:
				break;
		}
		stat++;
		ptr = end +1;
		/* ptr = strtok( NULL, token); */
	}

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_GetField( MAP *map, char *line)
{
	int			rtn;
	int			sz, stat = 0;
	char		*token = ":";
	char		rec[ 8192];
	char		*ptr = rec, *end = ptr;
	MAP_FIELD	*field;

	LogDel( "line=[%s]", line);

	field = malloc( sizeof( MAP_FIELD));
	if( field == NULL)
	{
		LogCri( "malloc error. sz=[%d]", sizeof( MAP_FIELD));
		goto error_1;
	}
	memset( field, 0, sizeof( MAP_FIELD));

	memcpy( rec, line, strlen( line) +1);

	/* ptr = strtok( line, token); */
	while( end != NULL)
	{
		end = strpbrk( ptr, token);
		if( end) *end = 0;
		switch( stat)
		{
			case 0:		/* id */
				field->id = atoi( ptr);
				break;
			case 1:		/* name */
				Map_Trim( map, ptr);
				sz = strlen( ptr);
				field->name = malloc( sz + 1);
				if( field->name == NULL) goto error_3;
				memcpy( field->name, ptr, sz +1);
				break;
			case 2:		/* x position */
				field->x = atoi( ptr) + map->base_x;
				break;
			case 3:		/* y position */
				field->y = atoi( ptr) + map->base_y;
				break;
			case 4:		/* view size */
				field->vsz = atoi( ptr);
				break;
			case 5:		/* data size */
				field->sz = atoi( ptr);
				break;
			case 6:		/* attribute */
				memcpy( field->attr, ptr, sizeof( field->attr));
				break;
			case 7:		/* type */
				memcpy( field->type, ptr, sizeof( field->type));
				break;
			case 8:		/* format */
				sz = strlen( ptr);
				field->format = malloc( sz + 1);
				if( field->format == NULL) goto error_3;
				memcpy( field->format, ptr, sz +1);
				break;
			case 9:		/* filler */
				break;
			case 10:	/* data */
#if 20231102
				/* pointer type 이라도 우선은 allocation 하고 function에서 free 한다 */	
				sz = strlen( ptr);
				field->data = malloc( field->sz + 1);
				if( field->data == NULL) goto error_3;
				memcpy( field->data, ptr, sz +1);
#else
				if( field->type[ MAP_TYPE_POINTER] == '1')
				{
					field->data = NULL;
					field->sz = 0;
				}
				else
				{
					sz = strlen( ptr);
					field->data = malloc( field->sz + 1);
					if( field->data == NULL) goto error_3;
					memcpy( field->data, ptr, sz +1);
				}
#endif
				break;
			default:
				break;
		}
		stat++;
		ptr = end +1;
		/* ptr = strtok( NULL, token); */
	}

	/*
	LogDel( "field             =[%p]", field);
	LogDel( "field.id          =[%d]", field->id);
	LogDel( "field.name        =[%s]", field->name);
	LogDel( "field.x           =[%d]", field->x);
	LogDel( "field.y           =[%d]", field->y);
	LogDel( "field.vsz         =[%d]", field->vsz);
	LogDel( "field.sz          =[%d]", field->sz);
	LogDel( "field.attr        =[%s]", field->attr);
	LogDel( "field.type        =[0x%08x]", field->type);
	LogDel( "field.data        =[%s]", field->data);
	*/

	if( field->type[ MAP_TYPE_MAP] == '1')
	{
		Dll_Add( map->title, field, sizeof( MAP_FIELD));
	}
	else
	{
		if( map->line_s != 0)
		{
			rtn = Map_LineField( map, field);
			if( rtn < 0) goto error_3;
		}
		else
		{
			Dll_Add( map->field, field, sizeof( MAP_FIELD));
		}
	}

	return 1;

	error_3:
		if( field->format != NULL) free( field->format);
		if( field->name != NULL) free( field->name);
		if( field->data != NULL) free( field->data);
#if 0
	error_2:
#endif
		free( field);
	error_1:
		return -1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_LineField( MAP *map, MAP_FIELD *field)
{
	int			i, y;
	int			base;	/* for multi line */
	MAP_FIELD	*line_field;

	LogDel( "Map_LineField start ... map=[%p] field=[%p]", map, field);
	
	if( map->line_n <= 0) map->line_n = 1;

	base = field->y /* - map->line_s ... 20230831 해당위치에 안찍혀서 */;

	for( y = map->line_s, i = 0; y <= map->line_e; y += map->line_n, i++)
	{
		LogDel( "field->name=[%s] y=[%d] i=[%d] line_s=[%d] line_e=[%d]", field->name, y, i, map->line_s, map->line_e);
		line_field = malloc( sizeof( MAP_FIELD));
		if( line_field == NULL)
		{
			LogCri( "malloc error. sz=[%d]", sizeof( MAP_FIELD));
			return -1;
		}
		memset( line_field, 0, sizeof( MAP_FIELD));
		memcpy( line_field, field, sizeof( MAP_FIELD));

		/* set id */
		line_field->id = ( line_field->id * 1000) + i;

		/* set name */
		line_field->name = malloc( strlen( field->name) + 5);
		if( line_field->name == NULL) 
		{
			LogCri( "malloc error. sz=[%d]", sizeof( MAP_FIELD));
			free( line_field);
			return -1;
		}
		sprintf( line_field->name, "%s_%d", field->name, i);
		LogDel( "add field->name=[%s]", line_field->name);

		line_field->y = y + base;
		LogDel( "line_field->y = [%d]", line_field->y);

		LogDel( "line_field->type=[%s]", line_field->type);
		LogDel( "line_field->type[ MAP_TYPE_POINTER]=[%c]", line_field->type[ MAP_TYPE_POINTER]);
#if 20231102
		line_field->data = malloc( line_field->sz +1);
		if( line_field->data == NULL) 
		{
			LogCri( "malloc error. sz=[%d]", sizeof( MAP_FIELD));
			free( line_field->name);
			free( line_field);
			return -1;
		}
		memset( line_field->data, 0x00, line_field->sz +1);
		memcpy( line_field->data, field->data, _Min( line_field->sz +1, strlen( field->data) +1));
#else
		if( line_field->type[ MAP_TYPE_POINTER] == '1')
		{
			line_field->data = NULL;
			line_field->sz = 0;
		}
		else
		{
			line_field->data = malloc( line_field->sz +1);
			if( line_field->data == NULL) 
			{
				LogCri( "malloc error. sz=[%d]", sizeof( MAP_FIELD));
				free( line_field->name);
				free( line_field);
				return -1;
			}
			memset( line_field->data, 0x00, line_field->sz +1);
			memcpy( line_field->data, field->data, _Min( line_field->sz +1, strlen( field->data) +1));
		}
#endif

		LogDel( "len=[%d]", strlen( field->format));
		line_field->format = malloc( strlen( field->format) +1);
		if( line_field->format == NULL)
		{
			LogCri( "malloc error. sz=[%d]", sizeof( MAP_FIELD));
			free( line_field->name);
			free( line_field->data);
			free( line_field);
			return -1;
		}
		memcpy( line_field->format, field->format, strlen( field->format) +1);
		Dll_Add( map->field, line_field, sizeof( MAP_FIELD));
	}
	if( field->format != NULL) free( field->format);
	if( field->name != NULL) free( field->name);
	if( field->data != NULL) free( field->data);
	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_AddField( MAP *map, int id, char *name, int x, int y, int vsz, int sz, char *attr, char *type, char *data)
{
	int				size;
	MAP_FIELD		*field;

	size = sizeof( MAP_FIELD);
	field = malloc( size);
	if( field == NULL)
	{
		LogErr( "field malloc error. sz=[%d]", size);
		goto error_1;
	}
	memset( field, 0, size);

	field->id		= id;
	field->x		= x;
	field->y		= y;
	field->vsz		= vsz;
	field->sz		= sz;
	memcpy( field->attr, attr, sizeof( field->attr));
#if 1
	memcpy( field->type, type, sizeof( field->type));
#else
	field->type.i	= type;
#endif

	size = strlen( name);
	field->name = malloc( size +1);
	if( field->name == NULL) goto error_2;
	memcpy( field->name, name, size +1);

#if 20231102
	size = strlen( data);
	field->data = malloc( size +1);
	if( field->data == NULL) goto error_3;
	memcpy( field->data, data, size +1);
#else
	if( field->type[ MAP_TYPE_POINTER] == '1')
	{
		if( field->data != NULL)
		{
			free( field->data);
			field->data = NULL;
			field->sz = 0;
		}
		field->data = data;
	}
	else
	{
		size = strlen( data);
		field->data = malloc( size +1);
		if( field->data == NULL) goto error_3;
		memcpy( field->data, data, size +1);
	}
#endif

	return 1;

#if 0
	error_5:
	error_4:
		if( field->type[ MAP_TYPE_POINTER] != '1') 
		{
			if( field->data != NULL) free( field->data);
		}
#endif
	error_3:
		free( field->name);
	error_2:
		free( field);
	error_1:
		return -1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_SetDataPtr( MAP *map, char *name, void *ptr)
{
	MAP_FIELD	*field;

	field = Dll_GetFirstPtr( map->field);
	while( field != NULL)
	{
		if( !strncmp( field->name, name, strlen( name) +1))
		{
			if( field->data != NULL)
			{
				if( field->set == 0) free( field->data);
				field->data = NULL;
			}
			field->data = ptr;
			field->set = 1;
		}
		field = Dll_GetNextPtr( map->field);
	}

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  field->uptr을 setting - Map_GetUserPtr 에서 반환
***************************************************************************** */
int Map_SetUserPtr( MAP *map, char *name, int num, void *ptr)
{
	MAP_FIELD	*field;

	if( num >= 8 || num < 0) return -1;

	field = Dll_GetFirstPtr( map->field);
	while( field != NULL)
	{
		if( !strncmp( field->name, name, strlen( name) +1))
		{
			field->uptr[ num] = ptr;
		}
		field = Dll_GetNextPtr( map->field);
	}

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  field->uptr을 반환 - Map_SetUserPtr 에서 set
***************************************************************************** */
void* Map_GetUserPtr( MAP *map, int num, MAP_FIELD *field)
{
	if( num >= 8 || num < 0) return NULL;
	return field->uptr[ num];
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_PrintName( MAP *map, char *name, char *format, ...)
{
	va_list		args;
	int			sz, dat_sz;
	char		buf[ 8192];
	MAP_FIELD	*field;

	va_start( args, format);
	sz = vsnprintf( buf, 8192, format, args);
	va_end( args);

	/* field */
	field = Dll_GetFirstPtr( map->field);
	while( field != NULL)
	{
		if( !strncmp( field->name, name, strlen( name)))
		{
			dat_sz = MAP_MIN( field->sz, sz);
			memcpy( field->data, buf, dat_sz);
			field->data[ dat_sz] = 0;
			field->update = 1;
		}
		field = Dll_GetNextPtr( map->field);
	}

	return sz;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_PrintNameA( MAP *map, char *name, char *attr, char *format, ...)
{
	va_list		args;
	int			sz, dat_sz;
	char		buf[ 8192];
	MAP_FIELD	*field;

	va_start( args, format);
	sz = vsnprintf( buf, 8192, format, args);
	va_end( args);

	/* field */
	field = Dll_GetFirstPtr( map->field);
	while( field != NULL)
	{
		if( !strncmp( field->name, name, strlen( name)))
		{
			memcpy( field->attr, attr, sizeof( field->attr));
			dat_sz = MAP_MIN( field->sz, sz);
			memcpy( field->data, buf, dat_sz);
			field->data[ dat_sz] = 0;
			field->update = 1;
		}
		field = Dll_GetNextPtr( map->field);
	}

	return sz;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_SetField( MAP *map, char *name, int ( *func)( MAP *map, MAP_FIELD *field), void *ptr)
{
	MAP_FIELD	*field;

	/* field */
	field = Dll_GetFirstPtr( map->field);
	while( field != NULL)
	{
		if( !strncmp( field->name, name, strlen( name) +1))
		{
			field->func = func;
			field->ptr = ptr;

			if( field->data == NULL)
			{
				field->data = malloc( field->sz +1);
				if( field->data == NULL)
				{
					LogCri( "malloc error. sz=[%d]", field->sz +1);
					return -1;
				}
			}
		}
		field = Dll_GetNextPtr( map->field);
	}

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_PrintField( MAP *map, MAP_FIELD *field, char *format, ...)
{
	va_list		args;
	int			sz, dat_sz;
	char		buf[ 8192];

	va_start( args, format);
	sz = vsnprintf( buf, 8192, format, args);
	va_end( args);

	if( field == NULL) return 0;
	/* field */
	dat_sz = MAP_MIN( field->sz, sz);
	memcpy( field->data, buf, dat_sz);
	field->data[ dat_sz] = 0;
	field->update = 1;

	return sz;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_PrintFieldA( MAP *map, MAP_FIELD *field, char *attr, char *format, ...)
{
	va_list		args;
	int			sz, dat_sz;
	char		buf[ 8192];

	va_start( args, format);
	sz = vsnprintf( buf, 8192, format, args);
	va_end( args);

	if( field == NULL) return 0;
	/* field */
	memcpy( field->attr, attr, sizeof( field->attr));
	dat_sz = MAP_MIN( field->sz, sz);
	memcpy( field->data, buf, dat_sz);
	field->data[ dat_sz] = 0;
	field->update = 1;

	return sz;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  find 필드
***************************************************************************** */
MAP_FIELD *Map_GetFieldPtr( MAP *map, char *name)
{
	MAP_FIELD	*field;

	/* field */
	field = Dll_GetFirstPtr( map->field);
	while( field != NULL)
	{
		if( !strncmp( field->name, name, strlen( name)))
		{
			return field;
		}
		field = Dll_GetNextPtr( map->field);
	}

	return NULL;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_Trim( MAP *map, char *str)
{
	char	*ptr;

	ptr = str;
	while( *ptr != 0)
	{
		switch( *ptr)
		{
			case 0x09:  /* horizontal tab */
			case 0x20:  /* space */
				*ptr = 0;
			default:
				break;
		}
		ptr++;
	}

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_GotoXY( MAP *map, int x, int y)
{
	Win_GotoXY( map->win, x, y);
	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_GotoEnd( MAP *map)
{
	Win_GotoXY( map->win, map->x + map->sz_x, map->y + map->sz_y);
	Win_Flush( map->win);

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_CursorOff( MAP *map)
{
	return Win_CursorOff( __Map->win);
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_CursorOn( MAP *map)
{
	return Win_CursorOn( __Map->win);
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_PrintMap( MAP *map)
{
	MAP_FIELD	*field;

	/* field */
	if( map->field != NULL)
	{
		printf( "-------------------------------------------------------------------------------------------\n");
		field = Dll_GetFirstPtr( map->field);
		while( field != NULL)
		{
			printf( "%3d ", field->id);
			printf( "%3d ", field->x);
			printf( "%3d ", field->y);
			printf( "%3d ", field->vsz);
			printf( "%3d ", field->sz);
			printf( "%-10.10s ", field->name);
			printf( "%s ", field->attr);
			printf( "%s ", field->type);
			printf( "%p ", field->data);
			switch( field->type[ MAP_TYPE_TYPE])
			{
				case MAP_TYPE_INT:
					printf( "%d", *( int *)field->data);
					break;
				default:
					printf( "%s", field->data);
					break;
			}
			printf( "%s ", field->format);
			printf( "\n");
			field = Dll_GetNextPtr( map->field);
		}
	}

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
unsigned int Map_GetKey( MAP *map, int timeout)
{
	unsigned int	key;
	key = Win_GetKey( map->win, map->timeout);

	return key;
}


/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_Message( MAP *map, char *format, ...)
{
	va_list		args;
	int			sz = 0;
	char		buf[ 8192];

	va_start( args, format);
	sz += vsnprintf( &buf[ sz], 8100, format, args);
	va_end( args);

/*	Win_Message( map->win, buf, sz); */
	Win_MessageXYZ( map->win, map->x +1, map->y + map->sz_y, map->sz_x, buf, sz);

	return sz;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_AlarmMessage( MAP *map, char *format, ...)
{
	va_list		args;
	int			sz = 0;
	char		buf[ 8192];

	buf[ 0] = 0x07;
	sz += 1;

	va_start( args, format);
	sz += vsnprintf( &buf[ sz], 8100, format, args);
	va_end( args);

/*	Win_Message( map->win, buf, sz); */
	Win_MessageXYZ( map->win, map->x +1, map->y + map->sz_y, map->sz_x, buf, sz);

	return sz;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  맵 파일
***************************************************************************** */
int Map_GetLineSize( MAP *map)
{
	return map->sz_y;
}


























