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

#include "udp.h"
#include "main.h"
#include "task.h"

/********** USER FUNCTION **********/
int	CmdServer( int argc, char *argv[]);
int	CmdClient( int argc, char *argv[]);
int	CmdPort( int argc, char *argv[]);
int	CmdAddr( int argc, char *argv[]);
/***********************************/
int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"server",		CmdServer,		"none",					"command test"},
	{	2,		"client",		CmdClient,		"none",					"command test"},
	{	3,		"port",			CmdPort,		"<port_no>",			"change udp port"},
	{	4,		"addr",			CmdAddr,		"<ip_addr>",			"change udp address"},
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
int CmdServer( int argc, char *argv[])
{
	int		rtn;
	UDP		*udp;
	char	rec[ 8192];
	int		sz;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		goto error;
	}

	LogMsg( "Udp open by server. addr=[%s] port=[%d]", Param->addr, Param->port);
	udp = Udp_OpenServer( Param->addr, Param->port);
	if( udp == NULL)
	{
		LogErr( "Udp_OpenServer error. addr=[%s] port=[%d]", Param->addr, Param->port);
		goto error;
	}

	while( Continue)
	{
		LogMsg( "Udp_RecvFrom ... timeout=[%d.%d]", Param->timeout/1000000, Param->timeout%1000000);
		rtn = Udp_RecvT( udp, rec, 8192, Param->timeout);
		if( rtn < 0)
		{
			LogErr( "Udp_RecvFrom error.");
			goto error_1;
		}

		/*usleep( Param->interval);*/
	}

	Udp_Close( udp);
	Continue = 1;

	return 1;

	error_1:
		Udp_Close( udp);
		Continue = 1;
	error:
		return 0;
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
int CmdClient( int argc, char *argv[])
{
	int		rtn;
	UDP		*udp;
	char	rec[ 8192];
	int		sz;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}
	else
	if( argc >= 2)
	{
		sz = sprintf( rec, "%s", argv[ 1]);
	}
	else
	{
		sz = sprintf( rec, "%s", "Hello.");
	}

	LogMsg( "Udp open by client. addr=[%s] port=[%d]", Param->addr, Param->port);
	udp = Udp_OpenClient( Param->addr, Param->port);
	if( udp == NULL)
	{
		LogErr( "Udp_OpenServer error. addr=[%s] port=[%d]", Param->addr, Param->port);
		goto error;
	}

	while( Continue)
	{
		LogMsg( "Udp_SendTo ... interval=[%d.%d] data=[%s]", Param->interval/1000000, Param->interval%1000000, rec);
		rtn = Udp_SendTo( udp, rec, sz);
		if( rtn < 0)
		{
			LogErr( "Udp_SendTo ...");
			goto error_1;
		}

		usleep( Param->interval);
	}

	Udp_Close( udp);
	Continue = 1;
	return 1;

	error_1:
		Udp_Close( udp);
		Continue = 1;
	error:
		return 0;
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
int CmdPort( int argc, char *argv[])
{
	int		port;

	switch( argc)
	{
		default:
		case 2:
			port = atoi( argv[ 1]);
			Param->port = port;
			break;
		case 1:
			printf( "argument error.\n");
			Cmd_HelpCommand( Cmd, argv[ 0]);
			return 0;
	}

	sprintf( Prompt, "[%s:%d]", Param->addr, Param->port);
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
int CmdAddr( int argc, char *argv[])
{
	switch( argc)
	{
		default:
		case 2:
			sprintf( Param->addr, "%s", argv[ 1]);
			break;
		case 1:
			printf( "argument error.\n");
			Cmd_HelpCommand( Cmd, argv[ 0]);
			return 0;
	}

	sprintf( Prompt, "[%s:%d]", Param->addr, Param->port);
	Cmd_SetPrompt( Cmd, Prompt);

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

