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

#include "xml.h"
#include "main.h"
#include "task.h"

/********** USER FUNCTION **********/
int	CmdTest( int argc, char *argv[]);
int	CmdXml( int argc, char *argv[]);
/***********************************/
int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"test",			CmdTest,		"none",					"command test"},
	{	2,		"xml",			CmdXml,			"none",					"command test"},
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
int CmdTest( int argc, char *argv[])
{
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

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
int CmdXml( int argc, char *argv[])
{
	int			rtn, sz, len = 0;
	int			fd;
	XML_DATA	*dp;
	char		rec[ 8192];
	char		*ptr;
	int			line, column, byte;
	int			offset, size;
	int			fin = 0;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	fd = open( Param->xml_file, O_RDONLY);
	if( fd < 0)
	{
		LogErr( "file open error. name=[%s]", Param->xml_file);
		goto error;
	}
	LogDbg( "file open success. name=[%s] fd=[%d]", Param->xml_file, fd);

	dp = Xml_Open( NULL);
	if( dp == NULL)
	{
		LogCri( "Xml_Open error.");
		goto error_1;
	}

	while( 1)
	{
		sz = read( fd, rec, 8192);
		len += sz;
		LogDbg( "file read data sz=[%d] len=[%d] fd=[%d]", sz, len, fd);
		if( sz < 0)
		{
			LogDbg( "file read end. fd=[%d] rtn=[%d]", fd, sz);
			break;
		}

		rtn = Xml_Proc( dp, rec, sz, fin);
		if( rtn > 0)
		{
			break;
		}
		LogDbg( "fin=[%d]", fin);
		LogDbg( "Xml_Proc success. rtn=[%d]", rtn);
		if( !fin) break;
		if( sz < 8192) break;
	}

	Xml_Print2( dp);
	sz = XML_FindKey( dp, "RWP_1/CACHED_DEALS/DEAL/FX_SWAP_DETAILS/FAR_LEG/CONTRA_AMOUNT", rec);
	LogDbg( "rec=[%s] sz=[%d]", rec, sz);
	sz = Xml_GetRemain( dp, rec, 8192);
	LogDbg( "remain=[%d:%s]", sz, rec);

	Xml_Close( dp);
	close( fd);

	/*
	while( 1)
	{
		dp = Xml_Open( NULL);
		rtn = Xml_Proc( dp, rec, sz, 0);
		Xml_Print2( dp);
		sz = Xml_GetRemain( dp, rec, 8192);
		LogDbg( "remain=[%d:%s]", sz, rec);
		Xml_Close( dp);
		if( sz <= 0) break;
	}
	*/


	return 1;

	error_2:
		Xml_Close( dp);
	error_1:
		close( fd);
	error:
		return 0;
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

