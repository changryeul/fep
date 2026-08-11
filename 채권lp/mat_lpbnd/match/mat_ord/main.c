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
#include <signal.h>

#include "mat.h"
#include "smq.h"
#include "udp.h"
#include "order.h"
#include "sise.h"

#include "main.h"

#define	MYMQ_TEST	0
#define	MAX_TEST	10000000

/** ***************************************************************************
**	GLOBAL
***************************************************************************** */
CFG			*Cfg;
UDP			*Udp;
MAT			*Mat;
SMQ			*SmqRecv;
SMQ			*SmqSend;
int			Continue = 1;

#if 0
MAT_ORDER		_Mo;
MAT_ORDER		*Mo = &_Mo;
#endif
FILE			*FilePtr;
char			RecvBuf[ 1024 * 256];

/** ***************************************************************************
**	PARAM
***************************************************************************** */
PARAM	ParamBuf =
{
	0,				/* argc */
	NULL,			/* argv */
	"",				/* argo */
	0,				/* args */
	"",				/* cfg_name */
	"",				/* log_name */
	0,				/* log_flag */
	0,				/* log_level */
	"",				/* sq_name */
	"",				/* rq_name */
	0,				/* interval */
	0,				/* timeout */
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
		case SIGSTOP:
		case SIGQUIT:
		case SIGTERM:
			Continue = 0;
			signal( sig_id, SignalProcess);
			LogMsg( "signal=[%d] Continue=[%d]", sig_id, Continue);
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

	signal( SIGINT, SignalProcess);
	signal( SIGSTOP, SignalProcess);
	signal( SIGQUIT, SignalProcess);
	signal( SIGTERM, SignalProcess);

	if( argc < 1)
	{
		return -1;
	}

	/************************************************/
	/* config file load                             */
	/************************************************/
	if( strchr( Param->argo, 'c') == NULL)
	{
		sprintf( Param->cfg_name, "%s/%s", getenv( "MAT_CFG"), "mat_ord.cfg");
		LogDel( "config file load.  Param->cfg_name = [%s]", Param->cfg_name);
	}
	Cfg = Cfg_Open( Param->cfg_name);
	if( Cfg == NULL)
	{
		LogCri( "config file open error. name=[%s]", Param->cfg_name);
		return -1;
	}

	rtn = Cfg_Get( Cfg, "log_name", Param->log_name, 512);
	if( rtn <= 0)
	{
		LogCri( "Config not found. name=[%s]", "log_name");
		return -1;
	}
	Param->log_flag  = Cfg_GetInt( Cfg, "log_flag");
	Param->log_level = Cfg_GetInt( Cfg, "log_level");

	Cfg_Get( Cfg, "sq_name", Param->sq_name, 32);
	Cfg_Get( Cfg, "rq_name", Param->rq_name, 32);
	Param->interval = Cfg_GetInt( Cfg, "interval");
	Param->timeout = Cfg_GetInt( Cfg, "timeout");

	/************************************************/
	/* log file open                                */
	/************************************************/
	LogLevel( Param->log_level);
	if( Param->log_flag == 1) 
	{
		LogMsg( "Redirect log message to file. name=[%s]", Param->log_name);
		LogFile( Param->log_name);
	}
	else
	if( Param->log_flag == 2) 
	{
		LogMsg( "Redirect log message to mem. name=[%s]", Param->log_name);
		LogMem( Param->log_name);
	}

	/************************************************/
	/* SMQ queue open                               */
	/************************************************/
	SmqRecv = Smq_Open( Param->rq_name);
	if( SmqRecv == NULL)
	{
		LogCri( "Smq_Open error. name=[%s]", Param->rq_name);
		return -1;
	}

	SmqSend = Smq_Open( Param->sq_name);
	if( SmqSend == NULL)
	{
		LogCri( "Smq_Open error. name=[%s]", Param->sq_name);
		return -1;
	}

	/************************************************/
	/* 매칭 엔진 open                               */
	/************************************************/
	Mat = Mat_Open( 0);
	if( Mat == NULL)
	{
		LogCri( "매칭 엔진 open error.");
		return -1;
	}

	time( &cur_time);
	LogMsg( "######################################################");
	LogMsg( "##### PROCESS START at %s pid=[%d]", TtoS( cur_time), getpid());
	LogMsg( "######################################################");

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
	int				rtn = 0;
	int				rcv_sz = 512;
	char			rec[ 8192];
	ORDER_RECV		*order = ( ORDER_RECV *)rec;

	while( Continue)
	{
		/************************************************/
		/* Queue Order Receive                          */
		/************************************************/
		LogDel( "Smq_GetRecord call...");
		rtn = Smq_GetRecord( SmqRecv, rec, 8192, Param->timeout);
		if( rtn < 0)
		{
			if( rtn == SMQ_TIMEOUT) 
			{
				LogMsg( "receive timeout. timeout=[%d.%d]", Param->timeout / 1000000, Param->timeout % 1000000);
				continue;
			}
			LogCri( "Smq_GetRecord error. Mo->rsz=[%d]", rtn);
			break;
		}
		rcv_sz = rtn;
		LogDddd( rec, rcv_sz, "recv %d byte(s).", rcv_sz);
		LogDev( "MsgType=[%.1s] Symbol=[%.7s] Side=[%.1s] Price=[%.20s]",
			order->MsgType, order->Symbol, order->Side, order->Price);

		
		/************************************************/
		/* Dead lock이 걸려서 Proc_Order 앞으로 뺀것임  */
		/************************************************/
		Smq_Commit( SmqRecv);

		/************************************************/
		/* Order check and Write shared memory          */
		/************************************************/
		rtn = Proc_Order( rec, rcv_sz);
		LogDel( "Proc_Order rtn=[%d]", rtn);
		if( rtn < 0)
		{
			LogMsg( "order error. rtn=[%d]", rtn);
		}
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

	LogMsg( "param                 = [%p]", param);
	LogMsg( "param->argc           = [%d]", param->argc);
	for( i = 0; i < param->argc; i++)
	LogMsg( "param->argv[%3d]      = [%s]", i, param->argv[i]);
	LogMsg( "param->argo           = [%s]", param->argo);
	LogMsg( "param->args           = [%d]", param->args);
	LogMsg( "param->cfg_name       = [%s]", param->cfg_name);
	LogMsg( "param->log_name       = [%s]", param->log_name);
	LogMsg( "param->log_flag       = [%d]", param->log_flag);
	LogMsg( "param->log_level      = [%d]", param->log_level);
	LogMsg( "param->sq_name        = [%s]", param->sq_name);
	LogMsg( "param->rq_name        = [%s]", param->rq_name);
	return 1;
}

/*********************************************************************************************************
* PROCESS
*********************************************************************************************************/

