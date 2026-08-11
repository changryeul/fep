/** ***************************************************************************
**  @file       main.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  프로그램 초기화/프로세싱/종료
**  파라메터 세팅 및 환경파일 로드
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>

#include "main.h"
#include "task.h"

/** ***************************************************************************
**	GLOBAL
***************************************************************************** */
extern CFG			*Cfg;
extern MAP			*Map;
extern int			Continue;
extern PARAM		*Param;

/** ***************************************************************************
**	
***************************************************************************** */

/** ***************************************************************************
**  @fn         int main( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패 종료
**  @retval     양수 성공 종료
**  @exception
**  @remark
**  @brief
**  GetOption   - 인수 분석 및 변수 초기화
**  InitProcess - 프로세스 초기상태 세팅 환경로드(config) 연관 모듈 오픈(open)
**  MainProcess - 프로세스 주요 작업
**  TermProcess - 포로세스 종료 작업 오픈모듈 정리
***************************************************************************** */
char **FileLoad( char *file_name)
{
	FILE	*fp;
	char	**file, *line_ptr;
	char	rec[ 8192], *ptr;
	int		line = 0, line_sz;

	fp = fopen( file_name, "r");
	if( fp == NULL)
	{
		LogErr( "file open error. name=[%s]", file_name);
		goto error;
	}

	file = malloc( sizeof( char *) * ( line +1));
	if( file == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( char *) * ( line +1));
		goto error_1;
	}

	while( Continue)
	{
		ptr = fgets( rec, 8192, fp);
		if( ptr == NULL)
		{
			LogMsg( "end of file. ptr=[%p]", ptr);
			break;
		}
		LogDbg( "line=[%d] rec=[%.20s]", line, rec);

		line_sz = strlen( rec);
		line_ptr = malloc( line_sz +1);
		if( line_ptr == NULL)
		{
			LogErr( "malloc error. sz=[%d]", line_sz +1);
			goto error_2;
		}
		LogDbg( "line_ptr[%d]=[%p]", line, line_ptr);
		memcpy( line_ptr, rec, line_sz +1);
		line_ptr[ line_sz -1] = 0;		/* \n 제거 */
		file = realloc( file, sizeof( char *) * ( line +1));
		if( file == NULL)
		{
			LogErr( "realloc error. sz=[%d]", sizeof( char *) * ( line +1));
			goto error_2;
		}
		file[ line] = line_ptr;
		line++;
	}

	file = realloc( file, sizeof( char *) * ( line +1));
	if( file == NULL)
	{
		LogErr( "realloc error. sz=[%d]", sizeof( char *) * ( line +1));
		goto error_2;
	}
	file[ line] = NULL;

	fclose( fp);

	return file;

	error_2:
		while( line)
		{
			free( file[ line]);
			line--;
		}
		free( file);
	error_1:
		fclose( fp);
	error:
		return NULL;
}

int FileFree( char **file)
{
	int		i;

	for( i = 0; file[ i] != NULL; i++)
	{
		LogDbg( "file[ %d]=[%p]", i, file[ i]);
		free( file[ i]);
	}
	free( file);

	return 1;
}

