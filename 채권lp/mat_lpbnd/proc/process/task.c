#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "map.h"
#include "etc.h"
#include "proc.h"
#include "task.h"

int	TaskStat( int argc, char *argv[]);
int	TaskEdit( int argc, char *argv[]);
int	TaskSet( int argc, char *argv[]);
int	TaskMon( int argc, char *argv[]);
int	TaskStop( int argc, char *argv[]);
int	TaskRun( int argc, char *argv[]);
int	TaskHoliday( int argc, char *argv[]);
int	TaskWeek( int argc, char *argv[]);
int	TaskReset( int argc, char *argv[]);
int	TaskAlarm( int argc, char *argv[]);
int	TaskInterval( int argc, char *argv[]);
int	TaskMap( int argc, char *argv[]);
int	TaskShutdown( int argc, char *argv[]);

int	TaskHelp( int argc, char *argv[]);
int	TaskQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	  1,	"stat",			TaskStat,		"[pos|id]",		"process table stat"},
	{	  1,	"edit",			TaskEdit,		"[pos|id]",		"process table 수정"},
	{	  1,	"set",			TaskSet,		"<id> <name> <value>",	"process table 수정"},
	{	  1,	"mon",			TaskMon,		"<filter=id>",	"process table monitor filter=(strstr function)"},
	{	  1,	"stop",			TaskStop,		"<id|all>",		"stop process - id로 시작하는 모든 프로세스"},
	{	  1,	"run",			TaskRun,		"<id>",			"exec process - id로 시작 하는 모든 프로세스"},
	{	  1,	"holiday",		TaskHoliday,	"[load]",		"휴일 테이블 조회/LOAD"},
	{	  1,	"week",			TaskWeek,		"[week_num]",	"요일 테이블 조회/수정"},
	{	  1,	"reset",		TaskReset,		"none",			"실행 count 초기화"},
	{	  1,	"alarm",		TaskAlarm,		"<on|off>",		"Alarm on or off"},
	{	  1,	"interval",		TaskInterval,	"<micro second>","usleep interval time"},
	{	  1,	"map",			TaskMap,		"<map_name>",	"change map name"},
	{	  1,	"shutdown",		TaskShutdown,	"none",			"데몬을 종료하고 공유메모리를 삭제"},

	{	100,	"help",			TaskHelp,		"none",		"help message"},
	{	101,	"quit",			TaskQuit,		"none",		"exit process"},
	{	102,	"exit",			TaskQuit,		"none",		"exit process"},
	{	-1,		"\0",			NULL,			"\0",		"\0"}
};

extern PROC		*Proc;
extern int		Continue;
extern int		Alarm;


int TaskInit()
{
	char	prompt[ 32];

	Cmd = Cmd_Open( CmdTable);
	if( Cmd == NULL)
	{
		LogCri( "Cmd_Open error. cmd_table=[%p]", CmdTable);
		return -1;
	}

	sprintf( prompt, "[process:0x%08x]", 0xfa110001);
	Cmd_SetPrompt( Cmd, prompt);
	Cmd_Set( Cmd, CMD_EXTENAL_COMMAND_ON);

	return 1;
}

int TaskStat( int argc, char *argv[])
{
	if( Proc == NULL)
	{
		printf( "Proc table not opened.\n");
		Proc = Proc_Open( Param->cfg_name);
		if( Proc == NULL)
		{
			LogCri( "Proc_Open error. ");
			return 0;
		}
	}

	if( argc <= 1) 
	{
		ProcTbl_Info( Proc->mem);
		return 1;
	}

	Proc_InfoProc( Proc, argv[ 1]);

	return 1;
}

int TaskEdit( int argc, char *argv[])
{
	int			len;
	PROC_TBL	proc_buf, *proc_ptr = &proc_buf;
	PROC_TBL	*curr;
	char		*ptr, cmd[ 512], val[ 512];

	if( Proc == NULL)
	{
		printf( "Proc table not opened.\n");
		Proc = Proc_Open( Param->cfg_name);
		if( Proc == NULL)
		{
			LogCri( "Proc_Open error. ");
			return 0;
		}
	}

	if( argc <= 1) 
	{
		printf( "Argument error.\n");
		Cmd_HelpCommand( Cmd, argv[0]);
		return 1;
	}

	curr = Proc_FindId( Proc, argv[ 1]);
	if( curr == NULL)
	{
		LogApp( "Proc_FindId error. id not found. id=[%s]", argv[ 1]);
		return 0;
	}
	memcpy( proc_ptr, curr, sizeof( PROC_TBL));

	while( 1)
	{
		Proc_InfoProcSub( Proc, proc_ptr);

		printf( "Select field(\'s\'ave/\'c\'encel/\'enter\'=cencel): ");
		ptr = fgets( cmd, 512, stdin);
		if( ptr == NULL) break;
		len = strlen( cmd) -1;
		cmd[ len] = 0;
		if( len <= 0) return 0;
		if( len == 1)
		{
			if( cmd[ 0] == 'c') return 0;
			if( cmd[ 0] == 's')
			{
				memcpy( curr, proc_ptr, sizeof( PROC_TBL));
				Proc_InfoProcSub( Proc, curr);
				return 1;
			}
		}
		Proc_Infofield( Proc, proc_ptr, cmd);
		printf( "Change select value                       =  ");
		ptr = fgets( val, 512, stdin);
		if( ptr == NULL) break;
		val[ strlen( val) -1] = 0;
		if( strlen( val) <= 0) break;
		Proc_SetField( Proc, proc_ptr, cmd, val);
		printf( "\n\n\n\n\n");
	}

	return 1;
}

int TaskSet( int argc, char *argv[])
{
	int			rtn;

	if( Proc == NULL)
	{
		printf( "Proc table not opened.\n");
		Proc = Proc_Open( Param->cfg_name);
		if( Proc == NULL)
		{
			LogCri( "Proc_Open error. ");
			return 0;
		}
	}

	if( argc < 4)
	{
		printf( "명령 인수가 부족합니다.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	rtn = Proc_SetProc( Proc, argv[ 1], argv[ 2], argv[ 3]);
	if( rtn <= 0) return rtn;

	Proc_InfoProc( Proc, argv[ 1]);

	return 1;
}

int TaskMon( int argc, char *argv[])
{
	int rtn;

	rtn = Mon( argc, argv);

	return rtn;
}

int TaskStop( int argc, char *argv[])
{
	int		rtn;


	if( argc < 2)
	{
		printf( "종료할 프로세스 id를 입력하십시요.\n");
		return 0;
	}

	rtn = Proc_StopCmd( Proc, argv[ 1]);
	if( rtn <= 0)
	{
		printf( "프로세스 id가 없습니다. id=[%s]\n", argv[ 1]);
		return 0;
	}

	return 1;
}

int TaskRun( int argc, char *argv[])
{
	int		rtn;


	if( argc < 2)
	{
		printf( "실행할 프로세스 id를 입력하십시요.\n");
		return 0;
	}

	rtn = Proc_RunCmd( Proc, argv[ 1]);
	if( rtn <= 0)
	{
		printf( "프로세스 id가 없습니다. id=[%s]\n", argv[ 1]);
		return 0;
	}

	return 1;
}

int TaskHoliday( int argc, char *argv[])
{
	int			rtn, i;
	PROC_TBL	*base;
	PROC_STAT	*stat;


	if( argc < 1)
	{
		return 0;
	}

	base = Mem_GetPtr( Proc->mem);
	stat = ( PROC_STAT *)&base[ MAX_PROC_TBL +1];

	if( argc > 1)
	{
		rtn = ProcTbl_LoadHolidayTbl( Proc->mem, Param->holiday_file);
		if( rtn < 0)
		{
			printf( "ProcTbl_LoadHolidayTbl error. rtn=[%d]", rtn);
			return 0;
		}
	}

	for( i = 0; stat->holiday[ i] > 0; i++)
		printf( "Holiday[%3d]           = [%d]\n", i, stat->holiday[ i]);

	return 1;
}

int TaskWeek( int argc, char *argv[])
{
	int			rtn, i;
	int			week = -1;
	PROC_TBL	*base;
	PROC_STAT	*stat;
	char		*week_tbl[] = { "일요일", "월요일", "화요일", "수요일", "목요일", "금요일", "토요일", "공휴일", NULL, NULL};
	char		msg[ 512];


	if( argc < 1)
	{
		return 0;
	}

	base = Mem_GetPtr( Proc->mem);
	stat = ( PROC_STAT *)&base[ MAX_PROC_TBL +1];

	switch( argc)
	{
		default:
		case 2:
			week = atoi( argv[ 1]);
		case 1:
		case 0:
			break;
	}

	if( week >= 0)
	{
		sprintf( msg, "요일테이블을 수정합니다. old=[%d:%s] new=[%d:%s]", 
				stat->week, week_tbl[ stat->week], week, week_tbl[ week]);
		rtn = Conform( msg);
		if( rtn )
		{
			stat->week = week;
		}
	}

	printf( "요일테이블 오늘=[%d:%s]\n", stat->week, week_tbl[ stat->week]);

	return 1;
}

int TaskReset( int argc, char *argv[])
{
	int		rtn;
	pid_t	pid;


	if( argc < 1)
	{
		return 0;
	}

	pid = Proc_GetDaemonPid( Proc);
	if( pid <= 0)
	{
		LogCri( "Proc_GetDaemonPid error. pid=[%d]", pid);
		return 0;
	}

	LogMsg( "send signal \"SIGUSR1\" to daemon process. pid=[%d]", pid);
	rtn = kill( pid, SIGUSR1);
	if( rtn < 0)
	{
		LogErr( "kill");
		return 0;
	}

	return 1;
}

int TaskAlarm( int argc, char *argv[])
{
	int		rtn = 1;


	if( argc < 2)
	{
		if( Alarm)
		{
			printf( "알람이 켜진 상태입니다.\n");
		}
		else
		{
			printf( "알람이 꺼진 상태입니다.\n");
		}
		return 1;
	}

	if( !memcmp( argv[ 1], "on", 2))
	{
		Alarm = 1;
		printf( "알람이 켜진 상태입니다.\n");
	}
	if( !memcmp( argv[ 1], "off", 3))
	{
		Alarm = 0;
		printf( "알람이 꺼진 상태입니다.\n");
	}

	return rtn;
}

int TaskInterval( int argc, char *argv[])
{
	int		rtn = 1;
	int		interval;


	if( argc < 2)
	{
		printf( "모니터링 간격은 [%d.%06d]초 입니다.\n", Param->mon_interval / 1000000, Param->mon_interval % 1000000);
		return 1;
	}

	interval = atoi( argv[ 1]);
	if( interval <= 100000)
	{
		printf( "모니터링 간격 값이 허용범위를 초과했습니다. interval=[%d.%06d]\n", interval / 1000000, interval % 1000000);
		printf( "모니터링 간격은 [%d.%06d]초 입니다.\n", Param->mon_interval / 1000000, Param->mon_interval % 1000000);
		return 0;
	}

	Param->mon_interval = interval;
	printf( "모니터링 간격은 [%d.%06d]초 입니다.\n", Param->mon_interval / 1000000, Param->mon_interval % 1000000);


	return rtn;
}

int TaskMap( int argc, char *argv[])
{
	int		rtn = 1;
	int		num = 0;

	if( argc < 2)
	{
		printf( "map name=[%s]\n", Param->map_name);
		return 0;
	}

	num = atoi( argv[ 1]);

	switch( num)
	{
		case 20:
		case 40:
		case 50:
		case 60:
			sprintf( Param->map_name, "%s/%s.%d", getenv( "MAP_DIR"), "process.map", num);
			printf( "map name=[%s]\n", Param->map_name);
			break;
		default:
			sprintf( Param->map_name, "%s", argv[ 1]);
			printf( "map name=[%s]\n", Param->map_name);
	}

	return rtn;
}

/*
int TaskFilter( int argc, char *argv[])
{
	int		rtn;


	if( argc < 2)
	{
		Filter[ 0] = 0;
		return 1;
	}

	sprintf( Filter, "%s", argv[ 1]);

	return 1;
}
*/

int TaskShutdown( int argc, char *argv[])
{
	int			rtn;
	PROC_TBL	*ptbl;


	if( argc < 1)
	{
		printf( "종료할 프로세스 id를 입력하십시요.\n");
		return 0;
	}

	ptbl = &Proc->base[ MAX_PROC_TBL];
	rtn = Proc_StopCmd( Proc, "proc_d");
	if( rtn <= 0)
	{
		printf( "프로세스 id가 없습니다. id=[%s]\n", "proc_d");
		Continue = 0;
	}

	while( Continue)
	{
		if( ptbl->pid == 0) break;
		printf( "wait stop daemon  ... pid=[%d]\n", ptbl->pid);
		rtn = kill( ptbl->pid, 0);
		if( rtn < 0)
		{
			printf( "check pid=[%d] ... error=[%d:%s]\n", ptbl->pid, errno, strerror( errno));
			Continue = 0;
			break;
		}
		sleep( 1);
	}
	printf( "데몬 process 종료 ... pid=[%d]\n", ptbl->pid);

	rtn = Mem_Remove( Proc->mem);
	printf( "공유메모리 삭제 ... key=[0x%08x]\n", Proc->mem->key);

	printf( "\"process\" 프로그램을 종료 합니다.\n");
	sleep( 1);

	return -1;
}






















int TaskDaemon( int argc, char *argv[])
{
	return 1;
}

int TaskHelp( int argc, char *argv[])
{
	int		rtn;

	rtn = Cmd_IntHelp( Cmd, argc, argv);
	return rtn;
}

int TaskQuit( int argc, char *argv[])
{
	LogDbg( "Program End");

	return CMD_EXIT;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  통화 조회
***************************************************************************** */
int Conform( char *msg)
{
    int     rtn;
    char    buf[ 512];

    printf( "%s (y/n):", msg);
    fflush( stdout);
    fgets( buf, 512, stdin);

    if( buf[ 0] == 'Y' || buf[ 0] == 'y')   rtn = 1;
    else                                    rtn = 0;

    return rtn;
}
