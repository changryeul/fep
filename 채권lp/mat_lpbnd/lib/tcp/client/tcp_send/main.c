/** ***************************************************************************
**  @file       main.c
**  @date       2025/01/02
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

#include "task.h"

/** ***************************************************************************
**  GLOBAL
***************************************************************************** */
CFG		*Cfg;
CLIENT	_Client;
CLIENT	*Client = &_Client;

int		Task = 0;
int		Continue = 1;
char	CfgFilePath[ 512] = ".";
char	CfgFileName[ 512] = "packet.cfg";

/** ***************************************************************************
**  PARAM
***************************************************************************** */
PARAM	ParamBuf = 
{
	"./main.cfg",	/* cfg_name					*/
	"./main.log",	/* log_name					*/
	"./batch.cmd",	/* bat_name					*/
	"127.0.0.1",	/* addr */
	10001			/* port */
};
PARAM	*Param = &ParamBuf;


/** ***************************************************************************
**  Argument & Option
***************************************************************************** */
char	*OptStr = "ho:b:";
char	Usage[] = 
"usage: %s -h -o out_file_name [header_file ... ]\n"
"\to: output file name (default:stdout);\n"
"\tb: batch process (default:cmd.bat);\n"
"\th: help message\n";


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
	int		i;
	int		rtn;

	rtn = GetOption( argc, argv);
	if( rtn <= 0)
	{
		printf( Usage, argv[ 0]);
		return -1;
	}

	rtn = InitProcess( argc, argv);
	if( rtn < 0)
	{
		LogCri( "InitProcess error. rtn=[%d]", rtn);
		return rtn;
	}

	switch( Task)
	{
		case 2:		/* daemon process */
			break;
		case 1:		/* batch process */
			BatchProcess( argc, argv);
			break;
		case 0:		/* command process */
		default:
			rtn = CommandProcess( argc, argv);
			if( rtn < 0)
			{
				LogCri( "MainProcess error. rtn=[%d]", rtn);
				return rtn;
			}
			break;
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
	char	*p;

	while( 1)
	{
		opt = getopt( argc, argv, OptStr);
		if( opt < 0) break;

		switch( opt)
		{
			case -1  :
				return 1;
			case 'o' :
				memcpy( Param->log_name, optarg, strlen( optarg) +1);
				printf( "log file name = [%s]\n", Param->log_name);
				break;
			case 'c' :
				memcpy( Param->cfg_name, optarg, strlen( optarg) +1);
				printf( "config file name = [%s]\n", Param->cfg_name);
				break;
			case 'b' :
				memcpy( Param->bat_name, optarg, strlen( optarg) +1);
				Task = 1;
				break;
			case 'h' :
				return 0;
			default  :
				break;
		}
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
            Continue = 0;
            break;
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
	int		rtn;
	time_t	cur_time;

	Cfg = Cfg_Open( Param->cfg_name);
	if( Cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", Param->cfg_name);
		return -1;
	}

	Cfg_Get( Cfg, "addr", Param->addr, 32);
	Param->port = Cfg_GetInt( Cfg, "port");

    time( &cur_time);
    LogMsg( "######################################################");
    LogMsg( "##### PROCESS START at %s pid=[%d]", TtoS( cur_time), getpid());
    LogMsg( "######################################################");
	ParamPrint( Param);

	Cmd = Cmd_Open( CmdTable);
	Cmd_SetPrompt( Cmd, "[cmd] ");
	Process( argc, argv);
	return 1;
}

int CommandProcess( int argc, char *argv[])
{
	int		rtn;

	rtn = Cmd_Main( Cmd);

	return 1;
}

int Process( int argc, char *argv[])
{
	int		rtn = 0;

	while( Continue)
	{
		rtn = Cmd_Main( Cmd);
		if( rtn < 0) break;
	}
	return 1;
}

int BatchProcess( int argc, char *argv[])
{
	int		rtn;
	FILE	*fp;
	char	*ptr, rec[ 512];

	fp = fopen( Param->bat_name, "r");
	if( fp == NULL)
	{
		LogErr( "fopen error. name=[%s]", Param->bat_name);
		return -1;
	}

	while( 1)
	{
		ptr = fgets( rec, 512, fp);
		if( ptr == NULL) break;
		if( rec[ 0] == '#') continue;
		rtn = Cmd_RunCommand( Cmd, rec);
		if( rtn < 0) break;
	}

	fclose( fp);

	return 1;
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
    time_t      cur_time;

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
	int		cnt = 0;
	char	**ptr;

	LogMsg( "cfg_name          = [%s]", param->cfg_name);
	LogMsg( "bat_name          = [%s]", param->bat_name);
	LogMsg( "addr              = [%s]", param->addr);
	LogMsg( "port              = [%d]", param->port);

	return 1;
}

