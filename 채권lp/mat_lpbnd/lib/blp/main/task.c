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
int	CmdCreate( int argc, char *argv[]);
int	CmdRemove( int argc, char *argv[]);
int	CmdOpen( int argc, char *argv[]);
int	CmdClose( int argc, char *argv[]);
int	CmdService( int argc, char *argv[]);
int	CmdMon( int argc, char *argv[]);
int	CmdStat( int argc, char *argv[]);
int	CmdList( int argc, char *argv[]);
int	CmdProcess( int argc, char *argv[]);
int	CmdSise( int argc, char *argv[]);
int	CmdTime( int argc, char *argv[]);
int	CmdSend( int argc, char *argv[]);
int	CmdArg( int argc, char *argv[]);
int	CmdRecv( int argc, char *argv[]);
int	CmdSize( int argc, char *argv[]);
/***********************************/
int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"test",			CmdTest,		"none",					"command test"},
	{	1,		"create",		CmdCreate,		"none",					"command test"},
	{	1,		"remove",		CmdRemove,		"none",					"command test"},
	{	1,		"open",			CmdOpen,		"none",					"command test"},
	{	1,		"close",		CmdClose,		"none",					"command test"},
	{	1,		"service",		CmdService,		"none",					"command test"},
	{	1,		"mon",			CmdMon,			"none",					"command test"},
	{	1,		"stat",			CmdStat,		"none",					"command test"},
	{	1,		"list",			CmdList,		"none",					"command test"},
	{	1,		"process",		CmdProcess,		"none",					"command test"},
	{	1,		"sise",			CmdSise,		"none",					"command test"},
	{	1,		"arg",			CmdArg,			"none",					"command test"},
	{	1,		"send",			CmdSend,		"none",					"command test"},
	{	1,		"time",			CmdTime,		"none",					"command test"},
	{	1,		"recv",			CmdRecv,		"none",					"시세 수신 테스트"},
	{	1,		"size",			CmdSize,		"none",					"command test"},
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
	char			*data = "00000000037TTRODP4130201KTSG100501031950300000022          KR103501GF9721319510210003001000000000009969.00000000.00000010000000000164104000      00 110010020030040005056BF93920251024092830670                              AST0102000000100 0101         0928306820000000000000000070100000000000                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          ";

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	LogDbg( "test.............");

	KRX_NOTE_SETTLE_RESP_DATA_Print( data);

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
int CmdCreate( int argc, char *argv[])
{
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	Blp = Blp_Create( Param->key);
	if( Blp == NULL)
	{
			LogErr( "Blp_Create error. key=[0x%08x]", Param->key);
			return 0;
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
int CmdRemove( int argc, char *argv[])
{
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	Blp_Remove( Blp);

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
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	Blp = Blp_Open( Param->key, 1);
	if( Blp == NULL)
	{
			LogErr( "Blp_Create error. key=[0x%08x]", Param->key);
			return 0;
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
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	Blp_Close( Blp);

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
int CmdService( int argc, char *argv[])
{
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	Blp->map->stat.service = atoi( argv[ 1]);
	LogMsg( "Blp->map->stat.service=[%d]", Blp->map->stat.service);


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
int CmdMon( int argc, char *argv[])
{
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	MapInit();
	Mon_Main( Blp, "./main.map");
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
int CmdStat( int argc, char *argv[])
{
	static char	item_code[ 32] = "\0\0";

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	switch( argc)
	{
		case 3:
			if( memcmp( argv[ 1], "set", 4))	memcpy( item_code, argv[ 2], 13);
			break;
		case 2:
			{
				memcpy( item_code, argv[ 1], 13);
				Blp_Stat( Blp, argv[ 1]);
			}
			break;
		case 1:
			Blp_Stat( Blp, item_code);
			break;
		default:
			printf( "argument error.\n");
			Cmd_HelpCommand( Cmd, argv[ 0]);
			return 0;
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
int CmdList( int argc, char *argv[])
{
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	Blp_List( Blp);

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
int CmdProcess( int argc, char *argv[])
{
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	Blp_Process( Blp, argv[ 1]);

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
int CmdSise( int argc, char *argv[])
{
	int			rtn;
	int			sleep_time = 1000000;
	CO_B601K	_b601k, *b601k = &_b601k;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}
	if( argc >= 2)	sleep_time = atoi( argv[ 1]);

	while( Continue)
	{
		memset( b601k, 0x00, sizeof( CO_B601K));
		rtn = Blp_GetSiseFromFile( Blp, b601k, Param->dat_file);
		if( rtn < 0) break;
		// CO_B601K_Print( b601k);

		// rtn = Blp_ProcessSise( Blp, b601k);
		rtn = Blp_Process( Blp, b601k);
		if( rtn <= 0) 
		{
			LogDbg( "skip ...");
		}
		usleep( sleep_time);
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
int CmdArg( int argc, char *argv[])
{
	int			rtn;
	char		arg1[ 8192] = "0000010001000050101A                              20250909000121                                                        KR103501GF970200000002.0000000004.0000000006.001000000000100000000010000000000000101100000005.0000000000.5000012136200000002.4000000000.0000010163.5000000002.4031951021000                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             ";
	char		arg2[ 8192] = "0000010001000050101A                              20250909000121                                                        KR103502GF620200000004.0000000008.0000000012.001000000000100000000010000000000000101100000005.0000000001.0000012136200000002.4000000000.0000010163.5000000002.4031951021000                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             ";
	char		arg3[ 8192] = "500100541200005010 A00000000000000000000000000000020250930000041KR103501GF97020000101100000000.0000000000.0000012136200000002.3700000000.0000000000.0000000000.00319510210000090000120000000600015000150000000002.0000000004.0000000006.0000010000000004000000000500000130000150000000600015000150000000002.0000000004.0000000006.0000010000000004000000000500000150000153000000000003000150000000002.0000000004.0000000006.00000100000000040000000005000000 ";
	char		*rec = arg3;
	char		buffer[ 8192];

	if( argc < 2)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	FileProcess( "arg.dat", 2, buffer);

	switch( argv[ 1][ 0] - '0')
	{
		case 2:	rec = buffer;	break;
		default:				break;
	}

	Blp_SetArg( Blp, rec, "TE50101");

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
int CmdTime( int argc, char *argv[])
{
	int		rtn;

	rtn = Blp_TimeCheck( Blp, &Blp->map->tbl[ Blp->tbl_pos]);

	LogDbg( "rtn = [%d]", rtn);

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
int CmdRecv( int argc, char *argv[])
{
	int		rtn;
	char	buf[ 8192];

	rtn = Blp_SiseOpen( Blp);

	LogDbg( "rtn = [%d]", rtn);

	while( Continue)
	{
		rtn = Blp_SiseRecv( Blp, buf, sizeof( buf));
		if( rtn <= 0) 
		{	
			LogDbg( "Blp_SiseRecv rtn=[%d]", rtn);
			continue;
		}
		buf[ rtn] = 0;
		LogDbg( "Blp_SiseRecv rtn=[%d:%s]", rtn, buf);
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
int CmdSend( int argc, char *argv[])
{
	int		sam_fd, fifo_fd;
	char 	sam_name[ 512]  = "/fsfxwin/fep/st02/DAT/PA/00000000/pa_50101mp";
	char	fifo_name[ 512] = "/fsfxwin/fep/st02/FIFO/PA/pa_50101mp1";
	char	buf[ 512];

	sam_fd = open( sam_name, O_RDWR);
	if( sam_fd < 0)
	{
		LogErr( "sam file open error. name=[%s]", sam_name);
		return 0;
	}

	fifo_fd = open( fifo_name, O_RDWR);
	if( fifo_fd < 0)
	{
		LogErr( "fifo open error. name=[%s]", fifo_name);
		return 0;
	}

	write( sam_fd, "111111111111111111111111111111111111111111111111111111111111111111111111111111111", 70);
	write( fifo_fd, "1", 1);
	sleep( 1);
	read( fifo_fd, buf, 512);

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
int CmdSize( int argc, char *argv[])
{
	LogDbg( "BLP_TBL        = [%d]", sizeof( BLP_TBL));

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

