/** ***************************************************************************
**  @file       task.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  명령어 수행 프로그램
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "task.h"

/** ***************************************************************************
**  @function   기능별 함수 작성
**  @brief      TODO
**  1. 함수 프로토타입 정의
**  2. CmdTable에 등록
**  3. 함수 작성
***************************************************************************** */
char	MapFile[ 512] = "file.map";

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int MapEditFile( char *file_name)
{
	int			loop = 1;
	MAP			*map;
	MAP_EDIT	*edit;
	char		**file;
	int			id;

	file = FileLoad( file_name);
	if( file == NULL)
	{
		LogCri( "file load fail. name=[%s]", file_name);
		goto error;
	}

	MapInit();

	map = Map_Open( MapFile);
	if( map == NULL)
	{
		LogCri( "Map_Open error. name=[%s]", MapFile);
		goto error_1;
	}

	edit = Map_EditOpen( map, '1', NULL);
	if( edit == NULL)
	{
		LogCri( "Map_EditOpen error. map=[%p]", map);
		goto error_2;
	}

	Map_DisplayMap( map);
	Map_Message( map, "loop start");

#if 1
	MapEditFileInit( map, file);
#else
#endif

	while( loop)
	{
		Map_DisplayField( map);
		id = Map_EditFile( map, edit);

		switch( edit->key)
		{
			case WIN_KEY_ESC:
				loop = 0;
				break;
			default:
				break;
		}
	}

	Map_EditClose( map, edit);
	Map_Close( map);
	MapEnd();
	FileFree( file);
	return 1;

	error_2:
		Map_Close( map);
	error_1:
		MapEnd();
		FileFree( file);
	error:
		return -1;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int MapEditFileInit( MAP *map, char **file)
{
	char	name[ 32];
	char	*ptr;
	int		line = 0;

	while( Continue)
	{
		sprintf( name, "line_%d", line);
		ptr = ( char *)Map_GetFieldPtr( map, name);
		if( ptr == NULL || file[ line] == NULL) break;

		Map_SetDataPtr( map, name, file[ line]);
		line++;
	}

	while( ptr != NULL)
	{
		Map_SetDataPtr( map, name, NULL);

		line++;
		sprintf( name, "line_%d", line);
		ptr = ( char *)Map_GetFieldPtr( map, name);
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int Process( MAP *map)
{
	int		key;

	MapInit();

	Map_DisplayMap( map);
	Map_Message( map, "loop start");

	while( Continue)
	{
		time( &MyData->cur_time);

		Map_DisplayField( map);

		key = Map_GetKey( map, 1000000);
		if( key != 0) Map_Message( map, "get key=[0x%08x]", key);
		switch( key)
		{
			case 0:
				continue;
			case 0x0D:
			case 'q':
				MapEnd();
				return 1;
			case 'a':
				if( MyData->alarm)	
				{
					MyData->alarm = 0;
					Map_Message( map, "alarm off. alarm=[%d]", MyData->alarm);
				}
				else				
				{
					MyData->alarm = 1;
					Map_Message( map, "alarm on. alarm=[%d]", MyData->alarm);
				}
				break;
		}
	}

	MapEnd();

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int Proc_Init( MAP *map, MY_DATA *my)
{
	Map_SetField( map, "alarm", Proc_AlarmStat, &my->alarm);
	Map_SetField( map, "time", Proc_TimeProc, &my->cur_time);
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int Proc_AlarmStat( MAP *map, MAP_FIELD *field)
{
	if( *(int *)field->ptr)
	{
		sprintf( field->data, "%s", "ON");
		field->attr[ MAP_ATTR_FORE] = WIN_ATTR_GREEN;
	}
	else
	{
		sprintf( field->data, "%s", "OFF");
		field->attr[ MAP_ATTR_FORE] = WIN_ATTR_RED;
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int Proc_TimeProc( MAP *map, MAP_FIELD *field)
{
	time_t		cur_time, *ptr_time;
	int			gap_time;
	struct tm	*tp;

	time( &cur_time);
	tp = localtime( &cur_time);
	sprintf( field->data, "%02d:%02d:%02d", tp->tm_hour, tp->tm_min, tp->tm_sec);

	return 1;
}



