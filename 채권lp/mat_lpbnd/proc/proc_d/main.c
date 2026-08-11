/** ***************************************************************************
**  @file       fep_deaemon.c
**  @date       2022/08/23
**  @author     최동춘
**  @version    V2.0.20220823
**  @brif
**  FEP PROCESS 관리 SUPER DAEMON
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>

#include "main.h"

/** ***************************************************************************
**  GLOBAL
***************************************************************************** */
CFG		*Cfg;
MEM		*Mem;
PROC	*Proc = NULL;
int		Continue = 1;
int		ProcFlag = 0;						/* 데몬 프로세스 테이블에 pid가 등록되어 있어도 실행 */
char	ConnName[ 32] = "FEP_DAEMON";

/** ***************************************************************************
**  PARAM
***************************************************************************** */
PARAM	ParamBuf =
{
	0,										/* argc */
	NULL,									/* argv */
	NULL,									/* envp */
	"",										/* argo */
	0,										/* args */
	"process.cfg",							/* cfg_file_name[ 512]              */
	"proc_daemon.log",						/* log_file_name[ 512]              */
	"",										/* swp_file_name[ 512]              */
	"",										/* holiday_file[ 512]               */
	".",									/* curr_work_dir[ 512]              */
	"",										/* stdout_file						*/
	"",										/* ora_connect 						*/
	"",										/* svc_name							*/
	0,										/* daemon_mode						*/
	0,										/* force */
	0,										/* swap_load                        */
	0,										/* config_load						*/
	0,										/* daily_job                        */
	"",										/* daily_name						*/
	0,										/* check_flag                       */
	1										/* interval                         */
};
PARAM	*Param = &ParamBuf;


/** ***************************************************************************
**  Argument & Option
***************************************************************************** */
char	*OptStr = "hdfsljkc:";
char	Usage[] = 
"Usage: %s [-h] [-c config_file_name]\n"
"Option: -o, --option                \n"
"  h, help                    this help message\n"
"  d, daemon                  데몬 모드 - 데몬 프로세스로 수행 ... \n"
"  f, force                   데몬 프로세스 테이블의 pid를 무시하고 실행\n"
"  s, swap                    swap 파일을 읽어 공유메모리에 로드\n"
"  l, load                    config 파일을 읽어 공유메모리에 로드 - config의 load_flag값에 상관없이 로드\n"
"  j, job                     일일 클리어 작업 - 데몬모드로 실행 하여도 프로그램이 종료 합니다.\n"
"  k, kill                    프로세스 체크 - 아이디가 등록되어 있지만 떠있지 않는 프로세스 테이블 클리어\n"
"  c, config=<cfg_name>       config file name (default:./main.cfg)\n"
"\n"
"  kill -SIGTERM <proc_daemon_pid> :전체 프로세스를 종료 하고 daemon도 종료합니다. ( shutdown )\n"
"  kill -SIGUSR1 <proc_daemon_pid> :일일 클리어 작업을 daemon이 직접 수행 합니다. \n"
"  kill -SIGUSR2 <proc_daemon_pid> :프로세스가 죽었지만 pid가 등록되어 있는 table을 정리합니다. 데몬의 pid를 등록합니다.\n"
;
struct option LongOption[] =
{
	{ "help",		no_argument,		0,	'h'},
	{ "daemon",		no_argument,		0,	'd'},
	{ "force",		no_argument,		0,	'f'},
	{ "swap",		no_argument,		0,	's'},
	{ "load",		no_argument,		0,	'l'},
	{ "job",		no_argument,		0,	'j'},
	{ "config",		required_argument,	0,	'c'},
	{ 0,			0,					0,	0}
};

/** ***************************************************************************
**  @fn         int main( int argc, char *argv[], char *envp[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @param      char *envp[] - 프로그램 인수
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
int main( int argc, char *argv[], char *envp[])
{
	int		rtn;

	rtn = GetOption( argc, argv, envp);
	if( rtn <= 0)
	{
		printf( Usage, argv[ 0]);
		return -1;
	}

	rtn = InitProcess( argc, argv, envp);
	if( rtn < 0)
	{
		LogCri( "InitProcess error. rtn=[%d]", rtn);
		printf( Usage, argv[ 0]);
		return rtn;
	}

	if( rtn > 0)
	{
		rtn = MainProcess( argc, argv);
		if( rtn < 0)
		{
			LogCri( "MainProcess error. rtn=[%d]", rtn);
			return rtn;
		}
	}

	rtn = TermProcess( argc, argv);
	if( rtn < 0)
	{
		LogCri( "MainProcess error. rtn=[%d]", rtn);
		return rtn;
	}

	return 1;
}

/** ***************************************************************************
**  @fn         int GetOption( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패 종료
**  @retval     양수 성공 종료
**  @exception
**  @remark
**  @brief
**  프로그램 시작시 받은 인수를 분석, 프로그램에서 사용할 변수를 초기화 한다.
**  Param->argc  - 인프로그램에서 받은 인수 갯수
**  Param->argv  - 프로그램에서 받은 인수
**  Param->argo  - 받은 옵션 저장, -x 의 x를 저장한다. - 환경에서 읽은 변수와 
                    중복되지 않게 이미 파라메터에서 세팅된 변수가 어느것인지 
					정보 표시를 위함
**  Param->args  - 받은 옵션 저장 갯수
***************************************************************************** */
int GetOption( int argc, char *argv[], char *envp[])
{
	int		opt;
	int		opt_idx = 0;

	Param->argc = argc;
	Param->argv = argv;
	Param->envp = envp;

	/************************************************/
	/* get parameter                                */
	/************************************************/
	while( 1)
	{
		opt = getopt_long( argc, argv, OptStr, LongOption, &opt_idx);
		if( opt < 0) break;

		switch( opt)
		{
			case -1  :
				return 1;
			case 0   :
				break;
			case 'c' :
				LogDbg( "config load");
				sprintf( Param->cfg_file_name, optarg, strlen( optarg) +1);
				LogMsg( "Param->cfg_file_name=[%s]", Param->cfg_file_name);
				break;
			case 'd' :
				Param->daemon_mode = 1;
				break;
			case 'f' :
				Param->force = 1;
				break;
			case 's' :
				Param->swap_load = 1;
				break;
			case 'l' :
				Param->config_load = 1;
				break;
			case 'k' :
				Param->check_flag = 1;
				break;
			case 'j' :
				Param->daily_job = 1;
				break;
			case 'h' :
				return 0;
			default  :
				break;
		}
		if( opt != 0) Param->argo[ Param->args++] = opt;
	}
	argc -=optind;
	argv = &argv[ optind];

	return 1;
}

/** ***************************************************************************
**  @fn         void SignalProcess( int sig_id) 
**  @param      int sig_id - 시그널 번호 
**  @return     없음    
**  @exception
**  @remark 
**  @brief  
**  프로그램에서 사용할 시그널  
***************************************************************************** */
void SignalProcess( int sig_id)
{
	LogMsg( "receive signal sig_id=[%d]", sig_id);

	switch( sig_id)
	{
		case SIGINT:
		case SIGQUIT:
		case SIGILL:
		case SIGABRT:
		case SIGFPE:
		case SIGKILL:
		case SIGPIPE:
		case SIGTERM:
			Continue = 0;
			LogMsg( "Set Contine=[%d]", Continue);
			break;
		case SIGUSR1:
			Param->daily_job = 1;
			LogMsg( "Set Param->daily_job=[%d]", Param->daily_job);
			break;
		case SIGUSR2:
			Param->check_flag = 1;
			LogMsg( "Set Param->check_flag=[%d]", Param->check_flag);
			break;
		default:
			LogMsg( "none processing. sig_id=[%d]", sig_id);
			break;
	}
	signal( sig_id, SignalProcess);

	return;
}


/** ***************************************************************************
**  @fn         int InitProcess( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  프로그램 초기화
        config file 로드
        로그파일 오픈
        전역변수 Param 값 세팅
**  전역변수 Param->argo의 값에 따라 세팅할 변수를 선택
***************************************************************************** */
int InitProcess( int argc, char *argv[], char *envp[])
{
	int			rtn;
	int			create_flag= 0;
	char		*path, *name;
	char		*ptr;

	signal( SIGINT,  SignalProcess);
	signal( SIGQUIT, SignalProcess);
	signal( SIGILL,  SignalProcess);
	signal( SIGABRT, SignalProcess);
	signal( SIGFPE,  SignalProcess);
	signal( SIGKILL, SignalProcess);
	signal( SIGPIPE, SignalProcess);
	signal( SIGTERM, SignalProcess);
	signal( SIGUSR1, SignalProcess);
	signal( SIGUSR2, SignalProcess);

	/************************************************/
	/* config file load                             */
	/************************************************/
	if( strchr( Param->argo, 'c') == NULL)
	{
		ptr = getenv( "MAT_CFG");
		if( ptr == NULL)
		{
			LogMsg( "getenv error. \"MAT_CFG\" name not found.");
			return -1;
		}
		sprintf( Param->cfg_file_name, "%s/%s", ptr, "process.cfg");
	}
	LogMsg( "Cfg_Open ... name=[%s]", Param->cfg_file_name);
	Cfg = Cfg_Open( Param->cfg_file_name);
	if( Cfg == NULL)
	{
		LogCri( "config file open error. name=[%s]", Param->cfg_file_name);
		return -1;
	}

	Cfg_Get( Cfg, "ora_connect", Param->ora_connect, 512);
	Cfg_Get( Cfg, "svc_name", Param->svc_name, 512);

	Cfg_Get( Cfg, "log_file_name", Param->log_file_name, 512);
	LogMsg( "LogFileName ... name=[%s]", Param->log_file_name);

	path = Cfg_GetPtr( Cfg, "swap_file_path");
	name = Cfg_GetPtr( Cfg, "swap_file_name");
	sprintf( Param->swp_file_name, "%s/%s", path, name);
	LogMsg( "SwpFileName ... name=[%s]", Param->swp_file_name);

	Cfg_Get( Cfg, "holiday_file", Param->holiday_file, 512);
	LogMsg( "holiday_file ... name=[%s]", Param->holiday_file);

	Cfg_Get( Cfg, "pwd", Param->curr_work_dir, 512);
	LogMsg( "Current work directory ... pwd=[%s]", Param->curr_work_dir);

	Cfg_Get( Cfg, "daily_name", Param->daily_name, 32);
	LogMsg( "Current work directory ... pwd=[%s]", Param->curr_work_dir);

	Cfg_Get( Cfg, "stdout_file", Param->stdout_file, 512);
	LogMsg( "stdout for child process ... stdout/stderr=[%s]", Param->stdout_file);

	Param->interval = Cfg_GetInt( Cfg, "interval");

	/* config 파일의 글로벌 변수를 환경에 등록한다 */
	Cfg_PutEnv( Cfg);

	ParamPrint( Param);
	sleep( 1);

	/************************************************/
	/* process table open                           */
	/************************************************/
	LogMsg( "Process table 공유메모리를 연결합니다. ...");
	Mem = ProcTbl_Open( Param->cfg_file_name);
	if( Mem == NULL)
	{
		LogMsg( "Process table 공유메모리가 없습니다. 공유메모리를 생성합니다.");
		Mem = ProcTbl_Create( Param->cfg_file_name);
		if( Mem == NULL)
		{
			LogMsg( "Process table 공유메모리 생성오류!!!");
			return -1;
		}
		LogMsg( "Process table 공유메모리가 생성되었습니다.");
		create_flag = 1;
	}
	LogMsg( "Process table 공유메모리를 연결했습니다. key=[0x%08x]", Mem->key);

	/************************************************/
	/* swap load                                    */
	/************************************************/
	sleep( 1);
	if( Param->swap_load)
	{
		LogMsg( "Swap file을 공유메모리로 load합니다. name=[%s]", Param->swp_file_name);
		rtn = ProcTbl_SwapLoad( Mem, Param->swp_file_name);
		if( rtn < 0)
		{
			LogMsg( "Swap file load error. 스왑파일을 로드하지 못했습니다. name=[%s]", Param->swp_file_name);
			LogMsg( "다음 작업을 수행합니다.");
		}
	}
	else
	{
		LogMsg( "Swap file을 공유메모리로 load 하지 않습니다.. name=[%s]", Param->swp_file_name);
		LogMsg( "Swap file load option=[-s] or [--swap]");
	}

	/************************************************/
	/* config load                                  */
	/************************************************/
	sleep( 1);
	if( Param->config_load || create_flag)
	{
		LogMsg( "Config file을 공유메모리로 load합니다. name=[%s]", Param->cfg_file_name);
		rtn = ProcTbl_CheckCfg( Mem, Param->cfg_file_name, 1);
		if( rtn < 0)
		{
			LogMsg( "Config file을 공유메모리로 load하지 못했습니다. name=[%s]", Param->cfg_file_name);
			LogMsg( "다음 작업을 수행합니다.");
		}

		LogMsg( "휴일테이블을 공유메모리로 load합니다. name=[%s]", Param->holiday_file);
		rtn = ProcTbl_LoadHolidayTbl( Mem, Param->holiday_file);
		if( rtn < 0)
		{
			LogMsg( "휴일테이블을 공유메모리로 load하지 못했습니다. name=[%s]", Param->holiday_file);
			LogMsg( "다음 작업을 수행합니다.");
		}
	}
	else
	{
		LogMsg( "Config file을 공유메모리로 load하지 않습니다. name=[%s]", Param->cfg_file_name);
		LogMsg( "Config file load option=[-l] or [--load]");
	}

	/************************************************/
	/* daily clear                                  */
	/************************************************/
	sleep( 1);
	if( Param->daily_job || create_flag)
	{
		LogFile( Param->log_file_name);
		LogMsg( "일일 클리어 작업을 수행합니다.");

		LogMsg( "휴일테이블 로드 작업을 수행합니다.");
		ProcTbl_LoadHolidayTbl( Mem, Param->holiday_file);
		LogMsg( "휴일테이블 로드 작업을 완료하였습니다.");

		ProcTbl_ClearRec( Mem);
		LogMsg( "일일 클리어 작업을 완료하였습니다.");

		if( strchr( Param->argo, 'd') == NULL)  /* daemon mode 일때는 다음 작업을 수행 */
		{
			/* 배치 종료시간까지 대기 - 두번 실행되는것 방지 */
			while( Continue) 
			{
				sleep( 1);
			}

			ProcTbl_RegiMe( Mem, 3, Param->daily_name);
			Param->daily_job = 0;

			return 0;
		}
	}
	else
	{
		LogMsg( "일일 클리어 작업을 수행하지 않습니다.");
		LogMsg( "daily job option=[-j] or [--job]");
	}

	/************************************************/
	/* ProcTbl Close & Proc Open                    */
	/************************************************/
	ProcTbl_Close( Mem);
	Proc = Proc_Open( Param->cfg_file_name);
	if( Proc == NULL)
	{
		LogCri( "Proc_Open error.");
		return -1;
	}

	/************************************************/
	/* check process                                */
	/************************************************/
	sleep( 1);
	if( Param->check_flag)
	{
		LogMsg( "프로세스 체크 작업을 수행합니다.");
		Proc_CheckPid( Proc);
		Param->check_flag = 0;
	}
	else
	{
		LogMsg( "프로세스 체크 작업을 수행하지 않습니다.");
		LogMsg( "process check option=[-k] or [--kill]");
	}

	/************************************************/
	/* daemon mode process                          */
	/************************************************/
	if( Param->daemon_mode <= 0)
	{
		LogMsg( "데몬모드가 아니라 프로세스를 종료합니다.");
		LogMsg( "Daemon mode option=[-d] or [--daemon]");
		return 0;
	}

	/************************************************/
	/* check other daemon process                   */
	/************************************************/
	sleep( 1);
	LogMsg( "다른 데몬 프로세스가 떠 있는지 검사합니다.");
	if( Proc->daemon->pid != 0)
	{
		LogMsg( "이미 다른 데몬 프로세스가 떠 있습니다. pid=[%d]", Proc->daemon->pid);
		if( Param->force <=  0)
		{
			LogMsg( "프로세스를 종료 합니다. 강제로 실행 하려면 [-f] or [--force] 옵션을 추가 해 주십시요.");
			return -1;
		}
		LogMsg( "Param->force = [%d] 이므로 강제로 수행합니다.", Param->force);
	}

	LogMsg( "데몬모드로 프로세스를 수행합니다.");
	sleep( 1);
	rtn = MakeDaemon( Param);

	ParamPrint( Param);

	return 1;
}

int MainProcess( int argc, char *argv[])
{
	int		rtn;
	time_t	cur_time;
	int		fd;

	LogMsg( "Register my process id. pid=[%d]", getpid());
	ProcTbl_RegiMe( Proc->mem, 1, "proc_d");

	/* 등록되어 있는 pid가 실제 살아 있는지 check 하여 clear */
	Proc_CheckPid( Proc);

	/************************************************/
	/* process loop                                 */
	/************************************************/
	LogMsg( "Processing loop start.");
	while( Continue)
	{
		time( &cur_time);
		if( cur_time % 60 == 0) 
		{
			LogMsg( "[Check process] at=[%s] interval=[%d.%d] job=[%ld]", 
				TtoS( cur_time), 
				Param->interval / 1000000, 
				Param->interval % 1000000, 
				( 24 *3600) - (( cur_time + ( 9 * 3600)) % ( 24 * 3600)));
		}
		if( ( cur_time + ( 9 * 3600)) % ( 24 * 3600) == 0)
		{
			LogMsg( "daily log initial ...");
			LogFile( NULL);
			LogFile( Param->log_file_name);

			fd = open( Param->stdout_file, O_WRONLY | O_CREAT, 0666);
			if( fd <= 0) 
			{
				LogErr( "file open error. name=[%s]", Param->stdout_file);
			}
			LogMsg( "stdout, stderr redirect ... name=[%s] fd=[%d]", Param->stdout_file, fd);
			close( STDOUT_FILENO);
			close( STDERR_FILENO);
			dup2( fd, STDOUT_FILENO);
			dup2( fd, STDERR_FILENO);
		}

		rtn = Proc_Run( Proc);
		rtn = Proc_Stop( Proc);
		rtn = Proc_Check( Proc);
		if( rtn < 0)
		{
		}

		if( Param->daily_job)
		{
			Proc_DailyJob( Proc);
			Param->daily_job = 0;
		}

		if( Param->check_flag)
		{
			/* 0 - stop, 1 - run, 2 -> pid 등록 */
			ProcTbl_RegiMe( Proc->mem, 2, "proc_d");
			Proc_CheckPid( Proc);
			Param->check_flag = 0;
		}

		ProcTbl_CheckCfg( Proc->mem, Param->cfg_file_name, 0);
		/*
		ProcTbl_SwapProc( Proc->mem, Param->swp_file_name);
		*/

		usleep( Param->interval);
	}
	LogMsg( "Processing loop end.");

	return 1;
}

int TermProcess( int argc, char *argv[])
{
	int		rtn = 1;
	time_t	cur_time;

	if( Param->daemon_mode)
	{
		Proc_StopAll( Proc);
		Proc_WaitAll( Proc);
		LogDbg( "Set Table .....");
		ProcTbl_RegiMe( Proc->mem, 0, "proc_d");
	}

	time( &cur_time);
	LogMsg( "### FEP_DAEMON END at=[%s] pid=[%d]", TtoS( cur_time), getpid());
	return rtn;
}

/*********************************************************************************************************
*
*********************************************************************************************************/
int LoaderRun( char *cmd)
{
	int			p_stat;
	pid_t		pid, child = 0;

	LogApp( "Loader(proc_swap) run ... cmd=[%s]", cmd);
	pid = fork();
	if( pid < 0)
	{
		LogErr( "fork error.");
		return -1;
	}
	else if( pid > 0)
	{
		while( pid != child)
		{
			LogApp( "Loader term wait. pid=[%d]", pid);
			sleep( 1);
			child = wait( &p_stat);
			LogApp( "Loader process terminated. pid=[%d] stat=[0x%08x]", child);
		}
		return pid;
	}

	system( cmd);
	exit( 0);
}

int MakeDaemon( PARAM *param)
{
	int		rtn;
	int		fd;
	pid_t	pid;
	time_t	cur_time;

	LogMsg( "DAEMON(background)으로 프로세스를 수행 합니다.");
	pid = fork();
	if( pid < 0)
	{
		LogErr( "fork error.");
		return -1;
	}
	else if( pid > 0)
	{
		LogMsg( "fork child process... pid=[%d], and terminated my_pid=[%d]", pid, getpid());
		exit( EXIT_SUCCESS);
	}

	LogMsg( "작업 디렉토리를 변경합니다. pwd=[%s]", Param->curr_work_dir);
	rtn = chdir( Param->curr_work_dir);
	if( rtn < 0)
	{
		LogErr( "작업디렉토리 변경 오류. pwd=[%s]", Param->curr_work_dir);
		return -1;
	}

	LogMsg( "로그 파일을 변경합니다. [console] -> [%s]", Param->log_file_name);

	LogFile( Param->log_file_name);

	fd = open( param->stdout_file, O_WRONLY | O_CREAT, 0666);
	if( fd <= 0) 
	{
		LogErr( "file open error. name=[%s]", Param->stdout_file);
	}

	close( STDOUT_FILENO);
	close( STDERR_FILENO);
	dup2( fd, STDOUT_FILENO);
	dup2( fd, STDERR_FILENO);

	time( &cur_time);
	LogMsg( "###########################################");
	LogMsg( "### FEP_DAEMON START at=[%s] pid=[%d]", TtoS( cur_time), getpid());
	LogMsg( "###########################################");

	return 1;
}

/** ***************************************************************************
**  @fn         int ParamPrint( PARAM *param)
**  @param      PARAM *param - 프로그램 전역에서 사용할 변수
**  @return     항상 1
**  @exception
**  @remark
**  @brief
**  프로그램에서 사용할 전역변수들의 구조체 값 표시
***************************************************************************** */
int ParamPrint( PARAM *param)
{
	int		i;

	LogDbg( "--------------------------------------------------------------------");
	for( i = 0; param->envp[ i] != NULL; i++)
	LogDbg( "param->envp[%3d]      = [%s]", i, param->envp[i]);
	LogDbg( "--------------------------------------------------------------------");
	LogDbg( "param                 = [%p]", param);
	LogDbg( "param->argc           = [%d]", param->argc);
	for( i = 0; i < param->argc; i++)
	LogDbg( "param->argv[%3d]      = [%s]", i, param->argv[i]);
	LogDbg( "param->argo           = [%s]", param->argo);
	LogDbg( "param->args           = [%d]", param->args);

	LogDbg( "param->cfg_file_name  = [%s]", param->cfg_file_name);
	LogDbg( "param->log_file_name  = [%s]", param->log_file_name);
	LogDbg( "param->swp_file_name  = [%s]", param->swp_file_name);
	LogDbg( "param->holiday_file   = [%s]", param->holiday_file);
	LogDbg( "param->curr_workdir   = [%s]", param->curr_work_dir);
	LogDbg( "param->ora_connect    = [%s]", param->ora_connect);
	LogDbg( "param->svc_name       = [%s]", param->svc_name);
	LogDbg( "param->daily_name     = [%s]", param->daily_name);

	LogDbg( "param->daemon_mode    = [%d]", param->daemon_mode);
	LogDbg( "param->force          = [%d]", param->force);
	LogDbg( "param->swap_load      = [%d]", param->swap_load);

	LogDbg( "param->daily_job      = [%d]", param->daily_job);
	LogDbg( "param->check_flag     = [%d]", param->check_flag);

	LogDbg( "param->interval       = [%d]", param->interval);
	return 1;
}

