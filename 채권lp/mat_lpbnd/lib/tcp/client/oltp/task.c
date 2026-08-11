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
int	CmdTest1( int argc, char *argv[]);
int	CmdTest2( int argc, char *argv[]);
int	CmdTest3( int argc, char *argv[]);
/***********************************/
int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"test1",		CmdTest1,		"none",					"command test"},
	{	1,		"test2",		CmdTest2,		"none",					"command test"},
	{	1,		"test3",		CmdTest3,		"none",					"command test"},
	{	100,	"help",			CmdHelp,		"none",					"help message"},
	{	101,	"quit",			CmdQuit,		"none",					"stop process"},
	{	102,	"exit",			CmdQuit,		"none",					"stop process"},
	{	-1,		"\0",			NULL,			"\0",					"\0"}
};

/********** USER DEFINE VALIABLE **********/

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
int CmdTest1( int argc, char *argv[])
{
	char	rec[ 8192] = 	"000184                                S                    OCM9507P01                            KR "
							"                              0           SUSD/KRW00000000012.3000000000000000045.60000000"; /* 190 */
	int		len = 194;
	int		uid = 0x0d;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	Tcp_Send( Client, ( char *)&len, 4);
	Tcp_Send( Client, ( char *)&uid, 4);
	Tcp_Send( Client, rec, 190);

	LogDbg( "test.............");

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
int CmdTest2( int argc, char *argv[])
{
	char	rec[ 8192] = 	"000184                                S                    OCM9507P02                            KR "
							"                              0           SUSD/KRW00000000012.3000000000000000045.60000000"; /* 190 */
	int		len = 194;
	int		uid = 0x0d;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	Tcp_Send( Client, ( char *)&len, 4);
	Tcp_Send( Client, ( char *)&uid, 4);
	Tcp_Send( Client, rec, 190);

	LogDbg( "test.............");

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
int CmdTest3( int argc, char *argv[])
{
	char	rec[ 8192] = 	"000184                                S                    OCM9507Q02                            KR "
							"                              0           SUSD/KRW00000000012.3000000000000000045.60000000"; /* 190 */
	int		len = 194;
	int		uid = 0x0d;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	Tcp_Send( Client, ( char *)&len, 4);
	Tcp_Send( Client, ( char *)&uid, 4);
	Tcp_Send( Client, rec, 190);

	LogDbg( "test.............");

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

