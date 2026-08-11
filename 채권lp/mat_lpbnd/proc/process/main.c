#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>

#include "task.h"

/** ***************************************************************************
**  GLOBAL
***************************************************************************** */
CFG		*Cfg;
PROC	*Proc;
int		Continue = 1;

/** ***************************************************************************
**  PARAM
***************************************************************************** */
PARAM	ParamBuf =
{
	0,							/* argc */
	NULL,						/* argv */
	"",							/* argo */
	0,							/* args */
	"",							/* cfg_name[ 512] - from getenv() */
	"",							/* log_name[ 512]             */
	"",							/* map_name[ 512]             */
	"",							/* holiday_file[ 512]         */
	0,							/* file_cnt                   */
	0							/* mon_interval					*/
};
PARAM	*Param = &ParamBuf;

/** ***************************************************************************
**  Argument & Option
***************************************************************************** */
char	*OptStr = "hc:";
char	Usage[] = 
"Usage: %s [-h] [-c config_file_name]\n"
"Option: -o, --option                \n"
"  h, help                    this help message\n"
"  c, config=<cfg_name>       config file name (default:$PROC_CFG_NAME)\n"
;
struct option LongOption[] =
{
	{ "help",       no_argument,        0,  'h'},
	{ "config",     required_argument,  0,  'c'},
	{ 0,            0,                  0,  0}
};

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
		printf( Usage, argv[ 0]);
		return -1;
	}

	rtn = InitProcess( argc, argv);
	if( rtn < 0)
	{
		LogCri( "InitProcess error. rtn=[%d]", rtn);
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
	char	*ptr;

	/************************************************/
	/* config file load                             */
	/************************************************/
	if( strchr( Param->argo, 'c') == NULL)
	{
		ptr = getenv( "MAT_CFG");
		if( ptr == NULL)
		{
			LogMsg( "getenv error. name \"MAT_CFG\" not found");
			return -1;
		}
		sprintf( Param->cfg_name, "%s/%s", ptr, "process.cfg");
	}
	Cfg = Cfg_Open( Param->cfg_name);
	if( Cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", Param->cfg_name);
		return -1;
	}
	Cfg_Get( Cfg, "log_file_name", Param->log_name, 512);
	Cfg_Get( Cfg, "map_file_name", Param->map_name, 512);
	Cfg_Get( Cfg, "holiday_file", Param->holiday_file, 512);
	Param->mon_interval = Cfg_GetInt( Cfg, "mon_interval");

	ParamPrint( Param);

	/************************************************/
	/* command open                                 */
	/************************************************/
	TaskInit();

	LogDbg( "Cmd_SetLog log_file_name=[%s]", Param->log_name);
	Cmd_SetLog( Cmd, Param->log_name);
#if 1
	LogFile( Param->log_name);
#endif

	/************************************************/
	/* process table open                           */
	/************************************************/
	Proc = Proc_Open( Param->cfg_name);
	if( Proc == NULL)
	{
		printf( "프로세스 테이블이 없습니다. 데몬[proc_d -d] 프로그램을 실행해야 합니다.\n");
		printf( "프로세스 테이블이 없습니다. 프로그램을 종료합니다.\n");
		return -1;
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
int MainProcess( int argc, char *argv[])
{
	int		rtn = 0;

	while( Continue)
	{
		rtn = Cmd_Main( Cmd);
		if( rtn < 0)
		{
			if( rtn == -9999) rtn = 0;
			break;
		}
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
	if( Proc != NULL) Proc_Close( Proc);
	if( Cmd != NULL) Cmd_Close( Cmd);
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

	LogMsg( "param->cfg_name       = [%s]", param->cfg_name);
	LogMsg( "param->log_name       = [%s]", param->log_name);
	LogMsg( "param->map_name       = [%s]", param->map_name);
	LogMsg( "param->file_cnt       = [%d]", param->file_cnt);
	LogMsg( "param->mon_interval   = [%d]", param->mon_interval);
	return 1;
}

