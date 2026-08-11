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

#include "mq.h"
#include "mymq.h"

#include "main.h"
#include "task.h"

#define	MAX_TEST	10000000

/** ***************************************************************************
**	GLOBAL
***************************************************************************** */
CFG			*Cfg;
MAP			*Map;
int			Continue = 1;

MyMQ			*MyMq;
struct content	*Content;
char			SendBuf[ 1024 * 256];

/** ***************************************************************************
**	PARAM
***************************************************************************** */
PARAM	ParamBuf =
{
	0,				/* argc */
	NULL,			/* argv */
	"",				/* argo */
	0,				/* args */
	"",				/* svcname */
	"",				/* host */
	0,				/* howmany */
	0,				/* size_s */
	"",				/* xchg */
	"",				/* rkey */
};
PARAM	*Param = &ParamBuf;

/** ***************************************************************************
**	Argument & Option
***************************************************************************** */
char	*OptStr = "h:n:s:";
char	Usage[] = 
"Usage: %s [-h host] [-n count] [-s size] rkey@xchg\n"
"Option: -o, --option                \n"
"  h, host                    host\n"
"  n, count                   config file name (default:./main.cfg)\n"
"  s, size                    map file name (default:./main.map)\n"
;
struct option LongOption[] = 
{
	{ "host",		required_argument,	0,	'h'},
	{ "count",		required_argument,	0,	'c'},
	{ "size",		required_argument,	0,	's'},
	{ 0,			0,					0,	0}
};
/** ***************************************************************************
**	
***************************************************************************** */
MY_DATA		MyDataBuf, *MyData = &MyDataBuf;

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
		printf( Usage, argv[ 0]);
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
	int		opt_ind = optind ? optind : 1;
	int		opt_idx = 0;
	char	*p;

	Param->argc = argc;
	Param->argv = argv;

	/************************************************/
	/* get parameter                                */
	/************************************************/
	while( 1)
	{
		opt = getopt_long( argc, argv, OptStr, LongOption, &opt_idx);
		if( opt < 0) break;

		long_opt:
		switch( opt)
		{
			case -1  :
				return 1;
			case 0   :
				break;
			case 'h' :
				memcpy( Param->host, optarg, strlen( optarg) +1);
				LogMsg( "Param->host=[%s]", Param->host);
			case 'n' :
				Param->howmany = atoi( optarg);
				LogMsg( "Param->howmany=[%d]", Param->howmany);
				break;
			case 's' :
				Param->send_s = atoi( optarg);
				Param->send_s *= 1024;
				LogMsg( "Param->send_s=[%d]", Param->send_s);
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
	int			log_flag = 0;
	char		*ptr;
	char		*path, *name;
	time_t		cur_time;

	char			*whoami;

	signal( SIGINT, SignalProcess);
	signal( SIGSTOP, SignalProcess);
	signal( SIGQUIT, SignalProcess);
	signal( SIGTERM, SignalProcess);

	if( argc < 2)
	{
		return -1;
	}

	whoami = argv[ 0];
	LogDbg( "whoami=[%s]", whoami);

	/************************************************/
	/* get key                                      */
	/************************************************/
	memcpy( Param->svcname, argv[ 1], strlen( argv[ 1]) +1);
	ptr = strchr( Param->svcname, '@');
	if( ptr == NULL)	return -1;
	*ptr = 0; ptr++;

	memcpy( Param->rkey, Param->svcname, strlen( Param->svcname) +1);
	if( strlen( Param->rkey) <= 0)	
	{
		LogDbg( "Param->rkey error. rkey=[%s]", Param->rkey);
		return -1;
	}
	memcpy( Param->xchg, ptr, strlen( ptr) +1);


	MyMq = mymq_open( Param->host, whoami);
	if( MyMq == NULL)
	{
		LogErr("mymq_open() error. host=[%s] whoami=[%s]", Param->host, whoami);
		return -1;
	}

	rtn = mymq_queue( MyMq, "", 256, QT_CLIENT);
	if( rtn != 0)
	{
		printf("mymq_queue() error. MyMq->errm=[%s]", MyMq->errm);
		return -1;
	}

	if( Param->howmany > MAX_TEST) Param->howmany = MAX_TEST;
	Content = content_alloc();


#if 0
	/************************************************/
	/* config file load                             */
	/************************************************/
	if( strchr( Param->argo, 'c') == NULL)
	{
		sprintf( Param->cfg_name, "%s/%s", ".", "main.cfg");
		LogDbg( "config file load.  Param->cfg_name = [%s]", Param->cfg_name);
	}
	Cfg = Cfg_Open( Param->cfg_name);
	if( Cfg == NULL)
	{
		LogCri( "config file open error. name=[%s]", Param->cfg_name);
		return -1;
	}

	if( strchr( Param->argo, 'f') == NULL)
	{
		rtn = Cfg_Get( Cfg, "map_name", Param->map_name, 512);
	}
	Param->interval = Cfg_GetInt( Cfg, "interval");

	rtn = Cfg_Get( Cfg, "log_name", Param->log_name, 512);
	if( rtn <= 0)
	{
		LogCri( "Config not found. name=[%s]", "log_name");
		return -1;
	}
	log_flag = Cfg_GetInt( Cfg, "log_flag");

	/************************************************/
	/* log file open                                */
	/************************************************/
	if( log_flag) 
	{
		LogMsg( "Redirect log message to file. name=[%s]", Param->log_name);
		LogFile( Param->log_name);
	}

	time( &cur_time);
	LogMsg( "######################################################");
	LogMsg( "##### PROCESS START at %s pid=[%d]", TtoS( cur_time), getpid());
	LogMsg( "######################################################");

	ParamPrint( Param);

	/************************************************/
	/* map file open                                */
	/************************************************/
	Map = Map_Open( Param->map_name);
	if( Map == NULL)
	{
		LogCri( "Map_Open error. map_name=[%s]", Param->map_name);
		return -1;
	}

	/************************************************/
	/* command tool 사용 - task.c                   */
	/************************************************/
	TaskInit();
	Cmd_Set( Cmd, CMD_EXTENAL_COMMAND_ON);
#endif

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
	int				rtn = 0;
	int				cnt = 0;
	char			send_c;
	int				sz = 512;
	int				send_sz = 512;
	struct timeval	cur_time;

	gettimeofday( &cur_time, NULL);
	send_c = '1';

	while( Continue)
	{
		LogDbg( "send ... mymq_send");
		send_sz = 8;
		sprintf( SendBuf, "%-8.8s", Param->rkey);
		memcpy( &SendBuf[ send_sz], "Hello!!!", 8);
		send_sz += 8;
		memset( &SendBuf[ send_sz], send_c, sz - send_sz);
		send_sz = sz;

		send_c++;
		if( send_c > 'z') send_c = '1';
		Content->ckey = cnt +1;
		sprintf( ( char *)Content->wkey, "%07d", cnt);
		memcpy( Content->xchg, Param->xchg, L_XCHG);
		memcpy( Content->rkey, Param->rkey, L_RKEY);
		memcpy( Content->destination.xchg, Param->xchg, L_XCHG);
		memcpy( Content->destination.rkey, Param->rkey, L_RKEY);

		LogDbg( "mymq_send call ... MyMq=[%p], Content=[%p] SendBuf=[%p] send_sz=[%d]", MyMq, Content, SendBuf, send_sz);
		rtn = mymq_send( MyMq, Content, SendBuf, send_sz);
		if( rtn != 0)
		{
			LogCri( "mymq_send() error. err=[%d:%s]", Content->errn, Content->errm);
			break;
		}
		LogDbg( "mymq_send rtn=[%d]", rtn);
		cnt++;
		if( cnt % 10000 == 0)
		{
			LogDbg( "sent %d ...", cnt);
		}
		LogDbg( "sent %d ...", cnt);
		sleep( 1);
	}

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
	time_t			cur_time;

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
	LogDbg( "param->svcname        = [%s]", param->svcname);
	LogDbg( "param->host           = [%s]", param->host);
	LogDbg( "param->howmany        = [%d]", param->howmany);
	LogDbg( "param->send_s         = [%d]", param->send_s);
	LogDbg( "param->xchg           = [%s]", param->xchg);
	LogDbg( "param->rkey           = [%s]", param->rkey);
	return 1;
}

