/** ***************************************************************************
**  @file       main.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  프로그램 초기화/프로세싱/종료
**  파라메터 세팅 및 환경파일 로드
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>

#include "tree.h"
#include "main.h"
#include "task.h"

/** ***************************************************************************
**	GLOBAL
***************************************************************************** */
CFG			*Cfg;
int			Continue = 1;
int			Task = 0;				/* 0 - find tree */
									/* 1 - tree view */
									/* 2 - scan(update tree data file) */
char		DirList[ 64][512];		/* scan directory list */
int			DirListCnt = 0;			/* scan directory list cnt */

char		DirListName[ 512];
char		ScanFileName[ 512];
extern char	FindList[ 1024][ 512];
extern int	FindCnt;

/** ***************************************************************************
**	PARAM
***************************************************************************** */
PARAM	ParamBuf =
{
	0,				/* argc */
	NULL,			/* argv */
	"",				/* argo */
	0,				/* args */
	"./main.cfg",	/* cfg_name[ 512] - from 환경변수 */
	"./main.log",	/* log_name[ 512]            */
	0,				/* log_level */
	0,				/* log_flag */
	1000,			/* log_size */
	"",				/* dat_file */
	"",				/* tmp_file */
	1,				/* timeout */
	10,				/* interval - loop interval       */
};
PARAM	*Param = &ParamBuf;

/** ***************************************************************************
**	Argument & Option
***************************************************************************** */
char	*OptStr = "hstc:f:";
char	Usage[] = 
"Usage: %s [-h] [-s] [-t] <directory name> ...\n"
"Option: -o, --option                \n"
"  h, help                    this help message\n"
"  t, tree                    tree view\n"
"  s, scan                    tree data file update\n"
"\n\n"
"tree - need set    tree.sh --> cd `tree $*`\n"
"                   alias   --> alias tree='. tree.sh'\n"
"                   env set --> export TREE_PATH=\"$HOME/.tree\"\n"
"       argument    dir_name dir_name ... : search and move directory\n"
"                   .           tree view - sub directory at cwd\n"
"                   -t          tree view - all directory\n"
"                   -s          scan tree\n"
"                   -h          this help\n"
"                   [dir list]\n"
"       file        $TREE_PATH/list.dat - scan directory list\n"
"                   $TREE_PATH/scan.dat - scan file name\n";
;
struct option LongOption[] = 
{
	{ "help",		no_argument,		0,	'h'},
	{ "tree",		no_argument,		0,	't'},
	{ "scan",		no_argument,		0,	's'},
	{ 0,			0,					0,	0}
};
/** ***************************************************************************
**	
***************************************************************************** */

/** ***************************************************************************
**  @fn         int main( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
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
int main( int argc, char *argv[])
{
	int		rtn;

	rtn = GetOption( argc, argv);
	if( rtn <= 0)
	{
		fprintf( stderr, Usage, argv[ 0]);
		fprintf( stdout, ".");
		return -1;
	}

	rtn = InitProcess( argc, argv);
	if( rtn < 0)
	{
		LogCri( "InitProcess error. rtn=[%d]", rtn);
		fprintf( stderr, Usage, argv[ 0]);
		fprintf( stdout, ".");
	}

	if( rtn > 0)
	{
		rtn = MainProcess( argc, argv);
		if( rtn < 0)
		{
			LogCri( "MainProcess error. rtn=[%d]", rtn);
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
int GetOption( int argc, char *argv[])
{
	int		opt;
	int		opt_idx = 0;

	Param->argc = argc;
	Param->argv = argv;

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
				sprintf( Param->cfg_name, optarg, strlen( optarg) +1);
				LogMsg( "Param->cfg_name=[%s]", Param->cfg_name);
				break;
			case 'h' :
				return 0;
			case 't' :
				Task = 1;		/* tree view */
				break;
			case 's' :
				Task = 2;		/* scan - update tree data file */
				break;
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
	LogMsg( "Received signal=[%d]", sig_id);
	switch( sig_id)
	{
		case SIGINT:
			exit( 0);
		case SIGSTOP:
		case SIGQUIT:
		case SIGTERM:
			Continue = 0;
			LogMsg( "signal=[%d] Continue=[%d]", sig_id, Continue);
			signal( sig_id, SignalProcess);
			break;
		default:
			LogMsg( "no action. signal=[%d]", sig_id);
			break;
	}

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
int InitProcess( int argc, char *argv[])
{
	int			rtn;
	time_t		cur_time;
	char		*ptr;

	signal( SIGINT, SignalProcess);
	signal( SIGSTOP, SignalProcess);
	signal( SIGQUIT, SignalProcess);
	signal( SIGTERM, SignalProcess);

	/************************************************/
	/* config file load                             */
	/************************************************/
	if( strchr( Param->argo, 'c') == NULL)
	{
		sprintf( Param->cfg_name, "%s/%s", getenv( "HOME"), ".tree/tree.cfg");
		/* LogDbg( "config file load.  Param->cfg_name = [%s]", Param->cfg_name); */
	}
	Cfg = Cfg_Open( Param->cfg_name);
	if( Cfg == NULL)
	{
		LogCri( "config file open error. name=[%s]", Param->cfg_name);
		return -1;
	}

	Param->interval = Cfg_GetInt( Cfg, "interval");
	Param->timeout  = Cfg_GetInt( Cfg, "timeout");

	rtn = Cfg_Get( Cfg, "log_name", Param->log_name, 512);
	if( rtn <= 0)
	{
		LogCri( "Config not found. name=[%s]", "log_name");
		return -1;
	}
	Param->log_flag  = Cfg_GetInt( Cfg, "log_flag");
	Param->log_size  = Cfg_GetInt( Cfg, "log_size");
	Param->log_level = Cfg_GetInt( Cfg, "log_level");

	/************************************************/
	/* log file open                                */
	/************************************************/
	if( Param->log_flag == 1) 
	{
		/* LogMsg( "Redirect log message to file. name=[%s]", Param->log_name); */
		LogFile( NULL);
		LogFile( Param->log_name);
		LogSize( Param->log_size);
	}
	LogLevel( Param->log_level);

	Cfg_Get( Cfg, "dat_file", Param->dat_file, 512);
	Cfg_Get( Cfg, "tmp_file", Param->tmp_file, 512);

	time( &cur_time);
	LogMsg( "######################################################");
	LogMsg( "##### PROCESS START at %s pid=[%d]", TtoS( cur_time), getpid());
	LogMsg( "######################################################");

	/************************************************/
	/* TREE list load                               */
	/************************************************/
	ptr = Cfg_GetFirstNamePtr( Cfg, "directory_list");
	while( ptr != NULL)
	{
		memcpy( DirList[ DirListCnt], ptr, strlen( ptr) +1);
		DirListCnt++;
		ptr = Cfg_GetNextNamePtr( Cfg, "directory_list");
	}

	ParamPrint( Param);

	return 1;
}

/** ***************************************************************************
**  @fn         int MainProcess( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  프로그램 수행
***************************************************************************** */
int MainProcess( int argc, char *argv[])
{
	int		rtn;

	switch( Task)
	{
		case 0:		/* search directory */
			if( argc < 2 || argv[ 1][ 0] == '.')    /* tree view current directory */
			{
				rtn = TaskCurrentTreeView( argc, argv);
				if( rtn < 0) return -1;
			}
			else
			{
				rtn = TaskFindDir( argc, argv);
				if( rtn < 0) return -1;
			}
			break;
		case 1:     /* tree view */
			rtn = TaskTreeView( argc, argv);
			if( rtn < 0) return -1;
			break;
		case 2:		/* scan directory */
			fprintf( stderr, "scanning directory ... ");
			rtn = TaskScanDir( argc, argv);
			if( rtn < 0) return -1;
			fprintf( stderr, "done.\n");
			break;
		default:	/* unknow job */
			break;
	}
	return 1;
}

/** ***************************************************************************
**  @fn         int MainProcess( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  프로그램 수행
***************************************************************************** */
int Not2MainProcess( int argc, char *argv[])
{
	int		rtn = 0;
	int		i;
	FILE	*fp;
	char	**ptr;
	char	cwd[ 512];
	char	rec[ 512];
	int		no, nfind = 0;

	switch( Task)
	{
		case 0:		/* search directory */
			if( argc < 2 || argv[ 1][ 0] == '.')	/* tree view current directory */
			{
				fp = fopen( Param->tmp_file, "w+");
				if( fp == NULL)
				{
					LogErr( "fopen error. name=[%s]", Param->tmp_file);
					return -1;
				}
				getcwd( cwd, 512);
				LogDbg( "cwd=[%s]", cwd);
				fprintf( fp, "%03d %s\n", 0, cwd);
				Tree_ScanDir( cwd, fp);

				fclose( fp);

				rtn = Tree_View( Param->tmp_file);
				if( rtn < 0)
				{
					LogErr( "Tree_View error.");
					return -1;
				}
				fprintf( stdout, ".");
			}
			else						/* search directory */
			{
				ptr = Tree_Search( Param->dat_file, argc, argv, &nfind);
				if( ptr == NULL)
				{
					fprintf( stderr, "directory not found. name=[%s]\n", argv[ 1]);
					return -1;
				}

				if( nfind <= 0)
				{
					fprintf( stdout, ".");
					return 1;
				}
				else
				if( nfind > 1)
				{
					for( i = 0; ptr[ i] != NULL; i++)
					{
						fprintf( stderr, "%3d %s\n", i +1, ptr[ i]);
					}
					fprintf( stderr, "select change directory no:");
					fgets( rec, 512, stdin);
					no = atoi( rec) -1;
					if( no < 0)		sprintf( rec, ".");
					else 			memcpy( rec, ptr[ no], strlen( ptr[ no]) +1);
				}
				else
				{
					memcpy( rec, ptr[ 0], strlen( ptr[ 0]) +1);
				}

				fprintf( stdout, "%s", rec);

				for( i = 0; ptr[ i] != NULL; i++) free( ptr[ i]);
				free( ptr);
			}
			break;
		case 1:		/* tree view */
			rtn = Tree_View( Param->dat_file);
			if( rtn < 0)
			{
				LogErr( "Tree_View error.");
				return -1;
			}
			fprintf( stdout, ".");
			break;
		case 2:		/* scan directory */
			fp = fopen( Param->dat_file, "w+");
			if( fp == NULL)
			{
				LogErr( "fopen error. name=[%s]", Param->dat_file);
				return -1;
			}

			for( i = 0; i < DirListCnt; i++)
			{
				fprintf( fp, "%03d %s\n", 0, DirList[ i]);
				Tree_ScanDir( DirList[ i], fp);
			}

			fclose( fp);

			fprintf( stdout, ".");
			break;
		default:	/* unknown */
			break;
	}


	return rtn;
}

/** ***************************************************************************
**  @fn         int TermProcess( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패 종료
**  @retval     양수 성공 종료
**  @exception
**  @remark
**  @brief
**  프로세스 종료 루틴 수행
***************************************************************************** */
int TermProcess( int argc, char *argv[])
{
	time_t		cur_time;

	time( &cur_time);
	LogMsg( "##### PROCESS END at %s pid=[%d]", TtoS( cur_time), getpid());
	return 1;
}

/*********************************************************************************************************
*
*********************************************************************************************************/
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

	LogDbg( "param                 = [%p]", param);
	LogDbg( "param->argc           = [%d]", param->argc);
	for( i = 0; i < param->argc; i++)
	LogDbg( "param->argv[%3d]      = [%s]", i, param->argv[i]);
	LogDbg( "param->argo           = [%s]", param->argo);
	LogDbg( "param->args           = [%d]", param->args);
	LogDbg( "param->cfg_name       = [%s]", param->cfg_name);
	LogDbg( "param->log_name       = [%s]", param->log_name);
	LogDbg( "param->log_level      = [%d]", param->log_level);
	LogDbg( "param->log_flag       = [%d]", param->log_flag);
	LogDbg( "param->log_size       = [%d]", param->log_size);
	LogDbg( "param->dat_file       = [%s]", param->dat_file);
	LogDbg( "param->tmp_file       = [%s]", param->tmp_file);
	LogDbg( "param->timeout        = [%d]", param->timeout);
	LogDbg( "param->interval       = [%d]", param->interval);
	return 1;
}

