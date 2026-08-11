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
#include <fcntl.h>

#include "map.h"
#include "mat.h"
#include "order.h"
#include "sise.h"

#include "main.h"
#include "task.h"

/********** USER FUNCTION **********/
int	CmdTest( int argc, char *argv[]);
int	CmdCreate( int argc, char *argv[]);
int	CmdRemove( int argc, char *argv[]);
int	CmdOpen( int argc, char *argv[]);
int	CmdClose( int argc, char *argv[]);
int	CmdStat( int argc, char *argv[]);
int	CmdInsert( int argc, char *argv[]);
int	CmdUpdate( int argc, char *argv[]);
int	CmdDelete( int argc, char *argv[]);
int	CmdOrder( int argc, char *argv[]);
int	CmdRecord( int argc, char *argv[]);
int	CmdMatch( int argc, char *argv[]);
int	CmdExecute( int argc, char *argv[]);
int	CmdPipe( int argc, char *argv[]);
int	CmdLock( int argc, char *argv[]);
int	CmdMon( int argc, char *argv[]);
int	CmdReset( int argc, char *argv[]);
/***********************************/
char*	GetInput( char *msg);
/***********************************/
int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"test",			CmdTest,		"none",					"command test"},
	{	10,		"create",		CmdCreate,		"none",					"ipc 생성"},
	{	11,		"remove",		CmdRemove,		"none",					"ipc 삭제"},
	{	12,		"open",			CmdOpen,		"none",					"open"},
	{	13,		"close",		CmdClose,		"none",					"close"},
	{	14,		"stat",			CmdStat,		"none",					"status"},
	{	16,		"insert",		CmdInsert,		"none",					"Insert 테스트"},
	{	16,		"update",		CmdUpdate,		"none",					"Update 테스트"},
	{	16,		"delete",		CmdDelete,		"none",					"Delete 테스트"},
	{	15,		"order",		CmdOrder,		"[usleep_time]",		"접수 테스트"},
	{	15,		"record",		CmdRecord,		"[position]",			"주문 조회"},
	{	15,		"match",		CmdMatch,		"[usleep_time]",		"매칭 테스트"},
	{	15,		"execute",		CmdExecute,		"[usleep_time]",		"체결 테스트"},
	{	15,		"pipe",			CmdPipe,		"[pos [pos [...]]]",	"체결 pipe send"},
	{	17,		"lock",			CmdLock,		"none",					"Lock - All stop!!!"},
	{	17,		"mon",			CmdMon,			"[file_name]",			"모니터링"},
	{	17,		"reset",		CmdReset,		"none",					"통계 초기화"},
	{	100,	"help",			CmdHelp,		"none",					"help message"},
	{	101,	"quit",			CmdQuit,		"none",					"stop process"},
	{	102,	"exit",			CmdQuit,		"none",					"stop process"},
	{	-1,		"\0",			NULL,			"\0",					"\0"}
};

/********** USER DEFINE VALIABLE **********/
extern MAT		*Mat;
extern SMQ		*Smq;
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
**  @fu         int CmdCreate( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
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

	if( Mat != NULL)
	{
		LogMsg( "Mat != NULL ... 이미 열려 있습니다.");
		return 0;
	}

	Mat = Mat_Create();
	if( Mat == NULL)
	{
		LogMsg( "매칭엔진 생성 오류");
		return 0;
	}

	LogMsg( "매칭엔진 생성... Mat=[%p]", Mat);

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdCreate( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdRemove( int argc, char *argv[])
{
	int		rtn;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ... 삭제할 수 없습니다.");
		return 0;
	}

	rtn = Mat_Remove( Mat);
	if( rtn < 0)
	{
		LogMsg( "매칭엔진 삭제 오류");
		return 0;
	}
	Mat = NULL;

	LogMsg( "매칭엔진 삭제...");

	return CMD_EXIT;
}

/** ***************************************************************************
**  @fu         int CmdCreate( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
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

	if( Mat != NULL)
	{
		LogMsg( "Mat != NULL ... 이미 열려 있습니다.");
		return 0;
	}

	Mat = Mat_Open();
	if( Mat == NULL)
	{
		LogMsg( "매칭엔진 Open 오류");
		return 0;
	}

	LogMsg( "매칭엔진 Open ... Mat=[%p]", Mat);

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdCreate( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdClose( int argc, char *argv[])
{
	int		rtn;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	rtn = Mat_Close( Mat);
	if( rtn < 0)
	{
		LogMsg( "매칭엔진 닫기 오류");
		return 0;
	}
	Mat = NULL;

	LogMsg( "매칭엔진 close...");

	return CMD_EXIT;
}

/** ***************************************************************************
**  @fu         int CmdStat( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdStat( int argc, char *argv[])
{
	int		rtn;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	rtn = Mat_Stat( Mat);
	if( rtn < 0)
	{
		LogMsg( "Mat_Stat error.");
		return 0;
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
int CmdInsert( int argc, char *argv[])
{
	int				rtn;
	char			*ptr;
	char			rec[ 512];
	ORDER		_obook, *obook = &_obook;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

#if 0
	ptr = GetInput( "기준통화(USD)   "); 
	memcpy( obook->base_cur, 	ptr,	3);
	ptr = GetInput( "상대통화(KRW)   "); 
	memcpy( obook->cont_cur, 	ptr,	3);
	ptr = GetInput( "매수(1)매도(2)  "); 
	memcpy( obook->side, 		ptr, 	1);
	ptr = GetInput( "가격(0000.00)   "); 
	obook->price = strtod( ptr, NULL);
	obook->update = -1;
#endif

	rtn = Mat_Order( Mat, obook, MAT_INSERT);
	if( rtn < 0)
	{
		LogMsg( "Mat_Stat error.");
		return 0;
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdUpdate( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  delete and insert
***************************************************************************** */
int CmdUpdate( int argc, char *argv[])
{
	int				rtn;
	char			*ptr;
	char			rec[ 512];
	ORDER		_obook, *obook = &_obook;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

#if 0
	ptr = GetInput( "기준통화(USD)   "); 
	memcpy( obook->base_cur, 	ptr,	3);
	ptr = GetInput( "상대통화(KRW)   "); 
	memcpy( obook->cont_cur, 	ptr,	3);
	ptr = GetInput( "매수(1)매도(2)  "); 
	memcpy( obook->side, 		ptr, 	1);
	ptr = GetInput( "가격(0000.00)   "); 
	obook->price = strtod( ptr, NULL);
	ptr = GetInput( "update pos      "); 
	obook->update = atoi( ptr);
#endif

	rtn = Mat_Order( Mat, obook, MAT_UPDATE);
	if( rtn < 0)
	{
		LogMsg( "Mat_Stat error.");
		return 0;
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdUpdate( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  delete and insert
***************************************************************************** */
int CmdDelete( int argc, char *argv[])
{
	int				rtn;
	char			*ptr;
	char			rec[ 512];
	ORDER		_obook, *obook = &_obook;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

#if 0
	ptr = GetInput( "기준통화(USD)   "); 
	memcpy( obook->base_cur, 	ptr,	3);
	ptr = GetInput( "상대통화(KRW)   "); 
	memcpy( obook->cont_cur, 	ptr,	3);
	ptr = GetInput( "매수(1)매도(2)  "); 
	memcpy( obook->side, 		ptr, 	1);
	ptr = GetInput( "delete pos      "); 
	obook->update = atoi( ptr);
#endif

	rtn = Mat_Order( Mat, obook, MAT_DELETE);
	if( rtn < 0)
	{
		LogMsg( "Mat_Stat error.");
		return 0;
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
int CmdLock( int argc, char *argv[])
{
	int		rtn;
	char	*ptr;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	Mat_Lock( Mat);

	while( 1)
	{
		printf( "    1. lock 계속\n");
		printf( "    2. unlock 후 다시 lock\n");
		printf( "    3. unlock 후 종료\n");

		ptr = GetInput( "input number (1/2/3) "); 
		switch( ptr[ 0])
		{
			default:
			case '1':
				continue;
			case '2':
				Mat_Unlock( Mat);
				Mat_Lock( Mat);
				continue;
			case '3':
				Mat_Unlock( Mat);
				return 1;
		}
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdStat( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdOrder( int argc, char *argv[])
{
	int				rtn;
	int				usleep_time = 1000000;
	char			*ptr, rec[ 512], *cmd;
	char			token[ 16] = " \t\n";
	FILE			*fp;
	ORDER		_obook, *obook = &_obook;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}
	else if( argc > 1)
	{
		usleep_time = atoi( argv[ 1]);
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	retry:

	fp = fopen( "order.dat", "r+");
	if( fp == NULL)
	{
		LogErr( "file open error. name=[%s]", "order.dat");
		return 0;
	}

	while( Continue)
	{
		ptr = fgets( rec, 512, fp);
		if( ptr == NULL) break;
		if( strlen( ptr) < 4) continue;

		if( rec[ 0] == '#') continue;

#if 0
		cmd = strtok( rec, token);

		ptr = strtok( NULL, token);
		obook->order = atoi( ptr);
		
		ptr = strtok( NULL, token);
		memcpy( obook->side, ptr, 2);
		
		ptr = strtok( NULL, token);
		memcpy( obook->base_cur, ptr, 3);
		
		ptr = strtok( NULL, token);
		memcpy( obook->cont_cur, ptr, 3);
		
		ptr = strtok( NULL, token);
		obook->price = strtod( ptr, NULL);
		
		ptr = strtok( NULL, token);
		obook->update = atoi( ptr);
#endif

		if( !strcmp( cmd, "insert")) 		rtn = Mat_Order( Mat, obook, MAT_INSERT);
		else if( !strcmp( cmd, "update"))	rtn = Mat_Order( Mat, obook, MAT_UPDATE);
		else if( !strcmp( cmd, "delete"))	rtn = Mat_Order( Mat, obook, MAT_DELETE);
		else						 continue;
		if( rtn < 0)
		{
			LogMsg( "Mat_Stat error.");
			return 0;
		}

		usleep( usleep_time);
	}

	fclose( fp);

	if( !Continue) return 1;

	goto retry;

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdStat( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdRecord( int argc, char *argv[])
{
	int				rtn;
	int				pos;
	char			*ptr, rec[ 512], *cmd;
	char			token[ 16] = " \t\n";
	FILE			*fp;
	ORDER		_obook, *obook = &_obook;

	if( argc < 2)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	pos = atoi( argv[ 1]);

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	Mat_PrintRecord( Mat, pos);

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdStat( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdMatch( int argc, char *argv[])
{
	int				rtn;
	int				usleep_time = 1000000;
	char			*ptr, rec[ 512], *cmd;
	char			token[ 16] = " \t\n";
	FILE			*fp;
	FX_QUOTE_T		_sise, *sise = &_sise;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}
	else if( argc > 1)
	{
		usleep_time = atoi( argv[ 1]);
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	retry:

#if 0
	fp = fopen( "sise.dat", "r+");
	if( fp == NULL)
	{
		LogErr( "file open error. name=[%s]", "order.dat");
		return 0;
	}

	while( Continue)
	{
		ptr = fgets( rec, 512, fp);
		if( ptr == NULL) break;
		if( strlen( ptr) < 4) continue;

		if( rec[ 0] == '#') continue;

		ptr = strtok( rec, token);
		memcpy( sise->base_cur, ptr, 3);
		
		ptr = strtok( NULL, token);
		memcpy( sise->cont_cur, ptr, 3);
		
		ptr = strtok( NULL, token);
		sise->ask_price = strtod( ptr, NULL);

		ptr = strtok( NULL, token);
		sise->bid_price = strtod( ptr, NULL);

		LogDbg( "sise=[%.3s][%.3s][%12f][%12f]", sise->base_cur, sise->cont_cur, sise->ask_price, sise->bid_price);
		rtn = Mat_Match( Mat, sise);
		
		usleep( usleep_time);
	}

	fclose( fp);
#endif

	if( !Continue) return 1;

	goto retry;

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdStat( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdExecute( int argc, char *argv[])
{
	int				rtn;
	int				usleep_time = 1000000;
	char			*ptr, rec[ 512], *cmd;
	char			token[ 16] = " \t\n";
	FILE			*fp;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}
	else if( argc > 1)
	{
		usleep_time = atoi( argv[ 1]);
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	while( Continue)
	{
		rtn = Mat_Execute( Mat, Param->timeout);
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdStat( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdPipe( int argc, char *argv[])
{
	int				rtn;
	int				usleep_time = 1000000;
	int				pos = 1, cnt = 1;

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	if( argc < 2)
	{
		Mat_StatExecute( Mat);
		return 0;
	}

	while( cnt < argc)
	{
		pos = atoi( argv[ cnt]);
		if( pos < 0 && pos >= MAT_MAX_RECORD) return 0;
		
		rtn = write( Mat->fd, ( char *)&pos, sizeof( int));
		if( rtn < sizeof( int))
		{
			return 0;
		}
		cnt++;
	}

	sleep( 1);
	Mat_StatExecute( Mat);

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
char *GetInput( char *msg)
{
	static char rec[ 512];

	memset( rec, 0, 512);
	printf( "%s:", msg);
	fgets( rec, 512, stdin);

	return rec;
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
int CmdMon( int argc, char *argv[])
{
	int			rtn;
	int			key;
	char		rec[ 512];


	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	MapInit();
	rtn = Mon_Main( Mat, Smq);
	MapEnd();

	return rtn;

	error_1:
	error:
		MapEnd();
		return 0;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통계 초기화
***************************************************************************** */
int CmdReset( int argc, char *argv[])
{
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	Mat_StatisReset( Mat);

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

