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

/********** USER FUNCTION **********/
int	CmdTest( int argc, char *argv[]);
int	CmdOpen( int argc, char *argv[]);
int	CmdClose( int argc, char *argv[]);
int	CmdInit( int argc, char *argv[]);
int	CmdProc( int argc, char *argv[]);
int	CmdEdit( int argc, char *argv[]);
/***********************************/
int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"test",			CmdTest,		"none",					"command test"},
	{	1,		"open",			CmdOpen,		"<map_file_name>",		"open map file"},
	{	1,		"close",		CmdClose,		"",						"close map file"},
	{	1,		"init",			CmdInit,		"",						"open map file"},
	{	1,		"mon",			CmdProc,		"",						"map process by opend map"},
	{	1,		"edit",			CmdEdit,		"<file_name>",			"edit file"},
	{	1,		"proc",			CmdProc,		"",						"map process by opend map"},
	{	100,	"help",			CmdHelp,		"none",					"help message"},
	{	101,	"quit",			CmdQuit,		"none",					"stop process"},
	{	102,	"exit",			CmdQuit,		"none",					"stop process"},
	{	-1,		"\0",			NULL,			"\0",					"\0"}
};

/********** USER DEFINE VALIABLE **********/
char	Prompt[ 32];
/******************************************/

/** ***************************************************************************
**  @function   CMD 사용을 위한 초기화  ... 변경 불필요
**  @brief      HELP/QUIT
***************************************************************************** */
int TaskInit()
{
	Cmd = Cmd_Open( CmdTable);
	if( Cmd == NULL)
	{
		LogCri( "Cmd_Open error. cmd_table=[%p]", CmdTable);
		return -1;
	}
	sprintf( Prompt, "[%s]", "file");
	Cmd_SetPrompt( Cmd, Prompt);

	return 1;
}

/** ***************************************************************************
**  @function   기능별 함수 작성
**  @brief      TODO
**  1. 함수 프로토타입 정의
**  2. CmdTable에 등록
**  3. 함수 작성
***************************************************************************** */
/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdTest( int argc, char *argv[])
{
	int		i;
	char 	**file;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	LogDbg( "test.............");

	file = FileLoad( "file.dat");

	for( i = 0; file[ i] != NULL; i++)
	{
		printf( "%s", file[ i]);
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
int CmdOpen( int argc, char *argv[])
{
	char	*map_name;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}
	if( argc < 2)	map_name = Param->map_name;
	else			map_name = argv[ 1];

	Map = Map_Open( map_name);
	if( Map == NULL)
	{
		LogCri( "Map_Open( map_name=[%s]) error.", map_name);
		return 0;
	}

	sprintf( Prompt, "[file:%s]", map_name);
	Cmd_SetPrompt( Cmd, Prompt);
	LogDbg( "map opened. name=[%s]", map_name);

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
int CmdClose( int argc, char *argv[])
{
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Map == NULL)
	{
		LogMsg( "map not opened. Map=(null)");
		return 0;
	}

	Map_Close( Map);
	Map = NULL;
	sprintf( Prompt, "[%s]", "file");
	Cmd_SetPrompt( Cmd, Prompt);

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
int CmdInit( int argc, char *argv[])
{
	int		key;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Map == NULL) 
	{
		LogMsg( "Map not opened.");
		return 0;
	}

	Proc_Init( Map, MyData);

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
int CmdEdit( int argc, char *argv[])
{
	int		key;

	if( argc < 2)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	MapEditFile( argv[ 1]);

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
int CmdProc( int argc, char *argv[])
{
	int		key;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Map == NULL) 
	{
		LogMsg( "Map not opened.");
		return 0;
	}

	Process( Map);

	return 1;
}

/** ***************************************************************************
**  @function   정의된 함수 ... 변경 불필요
**  @brief      HELP/QUIT
***************************************************************************** */
int CmdHelp( int argc, char *argv[])
{
	int		rtn;

	if( argc >= 2)
	{
		rtn = Cmd_HelpCommand( Cmd, argv[ 1]);
		return rtn;
	}

	rtn = Cmd_IntHelp( Cmd, argc, argv);
	return rtn;
}

int CmdQuit( int argc, char *argv[])
{
	LogDbg( "quit.............");

	exit( 1);
	return 1;
}

