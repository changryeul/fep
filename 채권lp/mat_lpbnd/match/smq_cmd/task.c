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
#include "smq.h"

#include "main.h"
#include "task.h"

/********** USER FUNCTION **********/
int	CmdTest( int argc, char *argv[]);
int	CmdCreate( int argc, char *argv[]);
int	CmdRemove( int argc, char *argv[]);
int	CmdOpen( int argc, char *argv[]);
int	CmdClose( int argc, char *argv[]);
int	CmdStat( int argc, char *argv[]);
int	CmdList( int argc, char *argv[]);
int	CmdRecord( int argc, char *argv[]);
int	CmdSend( int argc, char *argv[]);
int	CmdRecv( int argc, char *argv[]);
int	CmdPipe( int argc, char *argv[]);
int	CmdLock( int argc, char *argv[]);
int	CmdMon( int argc, char *argv[]);
int	CmdDump( int argc, char *argv[]);
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
	{	15,		"list",			CmdList,		"[start [end]]",		"data list"},
	{	15,		"rec",			CmdRecord,		"[start [end]]",		"data dump"},
	{	16,		"send",			CmdSend,		"<message>",			"Send 테스트"},
	{	16,		"recv",			CmdRecv,		"none",					"Recv 테스트"},
	{	15,		"pipe",			CmdPipe,		"[pos [pos [...]]]",	"체결 pipe send"},
	{	17,		"lock",			CmdLock,		"none",					"Lock - All stop!!!"},
	{	17,		"mon",			CmdMon,			"[file_name]",			"모니터링"},
	{	17,		"dump",			CmdDump,		"[type]",				"set dump format. hex|dec|cha|dat|no"},
	{	100,	"help",			CmdHelp,		"none",					"help message"},
	{	101,	"quit",			CmdQuit,		"none",					"stop process"},
	{	102,	"exit",			CmdQuit,		"none",					"stop process"},
	{	-1,		"\0",			NULL,			"\0",					"\0"}
};

/********** USER DEFINE VALIABLE **********/
SMQ		*Smq;
/******************************************/

/** ***************************************************************************
**  @function   CMD 사용을 위한 초기화  ... 변경 불필요
**  @brief      HELP/QUIT
***************************************************************************** */
int TaskInit()
{
	char	prompt[ 32];

	Cmd = Cmd_Open( CmdTable);
	if( Cmd == NULL)
	{
		LogCri( "Cmd_Open error. cmd_table=[%p]", CmdTable);
		return -1;
	}

	sprintf( prompt, "[%s:0x%08x]", "smq", SMQ_IPC_KEY);
	Cmd_SetPrompt( Cmd, prompt);

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

	if( Smq != NULL)
	{
		LogMsg( "Smq != NULL ... 이미 열려 있습니다.");
		return 0;
	}

	Smq = Smq_Create( Param->cfg_name);
	if( Smq == NULL)
	{
		LogMsg( "Queue 생성 오류");
		return 0;
	}

	LogMsg( "Queue 생성... Smq=[%p]", Smq);

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

	if( Smq == NULL)
	{
		LogMsg( "Smq == NULL ... 삭제할 수 없습니다.");
		return 0;
	}

	rtn = Smq_Remove( Smq);
	if( rtn < 0)
	{
		LogMsg( "Queue 삭제 오류");
		return 0;
	}
	Smq = NULL;

	LogMsg( "Queue 삭제...");

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
	char	*name = NULL;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Smq != NULL)
	{
		LogMsg( "Smq != NULL ... 이미 열려 있습니다.");
		return 0;
	}

	if( argc > 1)	name = argv[ 1];

	Smq = Smq_Open( name);
	if( Smq == NULL)
	{
		LogMsg( "Queue Open 오류. name=[%s]", name);
		return 0;
	}

	LogMsg( "Queue Open ... Smq=[%p] name=[%s]", Smq, name);

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

	if( Smq == NULL)
	{
		LogMsg( "Smq == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	LogMsg( "Queue close... name=[%s]", Smq->name);
	rtn = Smq_Close( Smq);
	if( rtn < 0)
	{
		LogMsg( "Queue 닫기 오류");
		return 0;
	}
	Smq = NULL;
	LogMsg( "Queue close... success");


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
int CmdStat( int argc, char *argv[])
{
	int		rtn;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Smq == NULL)
	{
		LogMsg( "Smq == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	rtn = Smq_Stat( Smq);
	if( rtn < 0)
	{
		LogMsg( "Smq_Stat error.");
		return 0;
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
int CmdRecord( int argc, char *argv[])
{
	// int			rtn;
	int			i, start, end;
	int			gap;

	// SMQ_INDEX	*index;
	SMQ_RECORD	*rec;
	SMQ_HEAD	*head;
	SMQ_STATUS	*stat = &Smq->map->stat;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Smq == NULL)
	{
		LogMsg( "Smq == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	start = stat->wpos - 50;
	if( start <= 0) start = 1;
	end = stat->wpos -1;

	switch( argc)
	{
		default:
			end = atoi( argv[ 2]);
		case 2:
			start = atoi( argv[ 1]);
		case 1:
			break;
		case 0:
			printf( "argument error.\n");
			Cmd_HelpCommand( Cmd, argv[ 0]);
			return 0;
	}

	for( i = start; i <= end; i++)
	{
		rec  = &Smq->map->rec[ i];
		head = &rec->head;

		printf( "%5d ", i);
		printf( "%1d ", head->gubun);
		printf( "%4d ", head->sz);
		printf( "%s ", TtoS( head->w_time.tv_sec));
		printf( "%06ld ", head->w_time.tv_usec);
		gap = head->r_time.tv_sec - head->w_time.tv_sec;
		gap *= 1000000;
		gap += head->r_time.tv_usec - head->w_time.tv_usec;
		if( gap < 0 || gap > 10000000) gap = 0;
		printf( "%7d ", gap);
		printf( "%.70s", rec->rec);
		printf( "\n");
		LogDump( rec->rec, head->sz, "data sz=[%d]", head->sz);
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
int CmdList( int argc, char *argv[])
{
	// int			rtn;
	int			i, start, end;
	int			gap;

	// SMQ_INDEX	*index;
	SMQ_RECORD	*rec;
	SMQ_HEAD	*head;
	SMQ_STATUS	*stat = &Smq->map->stat;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Smq == NULL)
	{
		LogMsg( "Smq == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	start = stat->wpos - 50;
	if( start <= 0) start = 1;
	end = stat->wpos -1;

	switch( argc)
	{
		default:
			end = atoi( argv[ 2]);
		case 2:
			start = atoi( argv[ 1]);
		case 1:
			break;
		case 0:
			printf( "argument error.\n");
			Cmd_HelpCommand( Cmd, argv[ 0]);
			return 0;
	}

	for( i = start; i <= end; i++)
	{
		rec  = &Smq->map->rec[ i];
		head = &rec->head;

		printf( "%5d ", i);
		printf( "%1d ", head->gubun);
		printf( "%4d ", head->sz);
		printf( "%s ", TtoS( head->w_time.tv_sec));
		printf( "%06ld ", head->w_time.tv_usec);
		gap = head->r_time.tv_sec - head->w_time.tv_sec;
		gap *= 1000000;
		gap += head->r_time.tv_usec - head->w_time.tv_usec;
		if( gap < 0 || gap > 10000000) gap = 0;
		printf( "%7d ", gap);
		printf( "%.70s", rec->rec);
		printf( "\n");
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
int CmdSend( int argc, char *argv[])
{
	int				rtn;

	if( argc < 2)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Smq == NULL)
	{
		LogMsg( "Smq == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	rtn = Smq_Send( Smq, argv[ 1], strlen( argv[ 1]));
	if( rtn < 0)
	{
		LogMsg( "Smq_Stat error.");
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
int CmdRecv( int argc, char *argv[])
{
	int				rtn, i;
	int				cnt;
	char			rec[ 65535];
	SMQ				*smq;

	if( argc < 3)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	smq = Smq_Open( argv[ 1]);
	if( smq == NULL)
	{
		LogCri( "Smq_Open error. name=[%s]", argv[ 1]);
		return 0;
	}
	LogMsg( "Smq_Open success. ptr=[%p]", smq);

	cnt = atoi( argv[ 2]);


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

	LogSetDump( LOG_DUMP_DEC);

	for( i = 0; i < cnt; i++)
	{
		rtn = Smq_Recv( smq, rec, 65535, Param->timeout);
		if( rtn <= 0)
		{
			LogMsg( "Smq_Recv stop. rtn=[%d]", rtn);
			if( rtn == 0) continue;
			if( rtn == SMQ_TIMEOUT) break;
			LogMsg( "Smq_Recv error. rtn=[%d]", rtn);
			break;
		}
		LogDump( rec, rtn, "recv data. sz=[%d] cnt=[%d/%d]", rtn, i +1, cnt);
	}


	Smq_Close( smq);

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
	char	*ptr;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Smq == NULL)
	{
		LogMsg( "Smq == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	Smq_MemLock( Smq);

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
				Smq_MemUnlock( Smq);
				Smq_MemLock( Smq);
				continue;
			case '3':
				Smq_MemUnlock( Smq);
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
	char			*ptr, rec[ 512];
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

	if( Smq == NULL)
	{
		LogMsg( "Smq == NULL ...  진행 할 수 없습니다.");
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

		rtn = Smq_Send( Smq, rec, 512);
		if( rtn < 0)
		{
			LogMsg( "Smq_Stat error.");
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
int CmdPipe( int argc, char *argv[])
{
	int				rtn;
	int				pos = 1, cnt = 1;

	if( Smq == NULL)
	{
		LogMsg( "Smq == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	if( argc < 2)
	{
		Smq_Stat( Smq);
		return 0;
	}

	while( cnt < argc)
	{
		pos = atoi( argv[ cnt]);
		if( pos < 0 && pos >= SMQ_MAX_REC) return 0;
		
		rtn = write( Smq->fd, ( char *)&pos, sizeof( int));
		if( rtn < sizeof( int))
		{
			return 0;
		}
		cnt++;
	}

	sleep( 1);
	Smq_Stat( Smq);

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

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Smq == NULL)
	{
		LogMsg( "Smq == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	MapInit();
	rtn = Mon_Main( Smq);
	MapEnd();

	return rtn;
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
int CmdDump( int argc, char *argv[])
{
	// int				rtn;
	// static int		dump_type;

	if( argc < 2)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( !memcmp( argv[ 1], "hex", 3))		LogSetDump( LOG_DUMP_HEX);
	else if( !memcmp( argv[ 1], "dec", 3))	LogSetDump( LOG_DUMP_DEC);
	else if( !memcmp( argv[ 1], "cha", 3))	LogSetDump( LOG_DUMP_CHA);
	else if( !memcmp( argv[ 1], "dat", 3))	LogSetDump( LOG_DUMP_DATA);
	else if( !memcmp( argv[ 1], "no", 2))	LogSetDump( LOG_DUMP_NONE);

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

