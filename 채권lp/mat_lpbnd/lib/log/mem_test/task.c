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

#include "log.h"
#include "main.h"
#include "task.h"

/********** USER FUNCTION **********/
int	CmdFile( int argc, char *argv[]);
int	CmdClose( int argc, char *argv[]);
int	CmdDaily( int argc, char *argv[]);
int	CmdTest( int argc, char *argv[]);
/***********************************/
int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"file",			CmdFile,		"[log_file]",			"log file open - 지정한 파일로 로그 저장"},
	{	2,		"close",		CmdClose,		"none",					"log file close - 콘솔로 로그 전송"},
	{	3,		"daily",		CmdDaily,		"none",					"데일리 파일 변경 테스트"},
	{	10,		"test",			CmdTest,		"none",					"command test"},
	{	100,	"help",			CmdHelp,		"none",					"help message"},
	{	101,	"quit",			CmdQuit,		"none",					"stop process"},
	{	102,	"exit",			CmdQuit,		"none",					"stop process"},
	{	-1,		"\0",			NULL,			"\0",					"\0"}
};

/********** USER DEFINE VALIABLE **********/
int MyMsg( void *data, char *rec, int sz);

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
int CmdFile( int argc, char *argv[])
{
	int     rtn;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( argc > 1)
	{
		LogDbg( "log file open... name=[%s]", argv[ 1]);
		LogFile( argv[ 1]);
	}
	else
	{
		LogDbg( "log file close... ");
		LogFile( NULL);
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
int CmdClose( int argc, char *argv[])
{
	int     rtn;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	LogClose();

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
int CmdDaily( int argc, char *argv[])
{
	time_t	cur_time;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	LogType( LOG_TYPE_DAILY);

	LogDbg( "test.............");
	printf( "log->fd_time=[%s]\n", TtoS( LogPtr->fd_time));

	printf( "데일리 수행을 10초후로 바꿈.. \n");
	time( &cur_time);
	LogPtr->fd_time = cur_time + 10;
	printf( "log->fd_time=[%s]\n", TtoS( LogPtr->fd_time));

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
int CmdTest( int argc, char *argv[])
{
	int     rtn;
	char    *file = "./test.log";
	char    rec[ 512] = "12345678901234567890";

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	LogName( Param->argv[0]);
	LogDbg( "log file test.............");

	LogTst( "debug", file);
	rtn = test_log();
	LogSetDump( LOG_DUMP_DEC);
	LogDump( rec, strlen( rec), "dump test %d", 1);

	LogFunc( NULL, MyMsg);
	LogUsr( "test...");
	LogDbg( "Here...");

	rtn = test_log();
	LogUsr( "test...");

	return 1;
}

int test_log()
{
	LogDbg( "debug");
	LogMsg( "message");
	LogApp( "application");
	LogLib( "Library");
	LogWar( "warning");
	LogErr( "error");
	LogCri( "critical");
	LogBel( "bell");

}

int MyMsg( void *data, char *rec, int sz)
{
	printf( "MyMsg..... %s", rec);
	printf( "%.*s\n", sz, rec);

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

