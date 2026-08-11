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

#include "log.h"
#include "mcp.h"
#include "order.h"
#include "sise.h"

#include "main.h"
#include "task.h"

#define	MAX_TEST	10000000

/** ***************************************************************************
**	GLOBAL
***************************************************************************** */
CFG			*Cfg;
MCP			*Mcp;
int			Continue = 1;

MAT				*Mat;
struct content	*Content;
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
	"",				/* addr */
	0,				/* port */
	"",				/* filter */
	0,				/* interval */
	0,				/* timeout */
};
PARAM	*Param = &ParamBuf;

/** ***************************************************************************
**	Argument & Option
***************************************************************************** */
char	*OptStr = "hc:";
char	Usage[] = 
"Usage: %s [-c config_name]\n"
"Option: -o, --option                \n"
"  h, help                    this help message\n"
"  c, config                  config file name (default:./main.cfg)\n"
;
struct option LongOption[] = 
{
	{ "help",		no_argument,		0,	'h'},
	{ "config",		required_argument,	0,	'c'},
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
			case 'h' :
				return 0;
			case 'c' :
				sprintf( Param->cfg_name, optarg, strlen( optarg) +1);
				LogMsg( "Param->cfg_name=[%s]", Param->cfg_name);
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

	signal( SIGINT, SignalProcess);
	signal( SIGSTOP, SignalProcess);
	signal( SIGQUIT, SignalProcess);
	signal( SIGTERM, SignalProcess);

	/************************************************/
	/* config file load                             */
	/************************************************/
	if( strchr( Param->argo, 'c') == NULL)
	{
#if 0
		sprintf( Param->cfg_name, "%s/%s", ".", "main.cfg");
#else
		sprintf( Param->cfg_name, "%s/%s", getenv( "MAT_CFG"), "mat_sis.cfg");
#endif
		LogDel( "config file load.  Param->cfg_name = [%s]", Param->cfg_name);
	}
	Cfg = Cfg_Open( Param->cfg_name);
	if( Cfg == NULL)
	{
		LogCri( "config file open error. name=[%s]", Param->cfg_name);
		return -1;
	}

	Cfg_Get( Cfg, "addr", Param->addr, 32);
	Param->port = Cfg_GetInt( Cfg, "port");

	Param->interval = Cfg_GetInt( Cfg, "interval");
	Param->timeout  = Cfg_GetInt( Cfg, "timeout");

	rtn = Cfg_Get( Cfg, "filter", Param->filter, 16);
	Param->filter[ rtn] = 0;

	rtn = Cfg_Get( Cfg, "log_name", Param->log_name, 512);
	if( rtn <= 0)
	{
		LogCri( "Config not found. name=[%s]", "log_name");
		return -1;
	}
	Param->log_flag  = Cfg_GetInt( Cfg, "log_flag");
	Param->log_level = Cfg_GetInt( Cfg, "log_level");

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
		LogMsg( "Redirect log message to file. name=[%s]", Param->log_name);
		LogMem( Param->log_name);
	}

	/************************************************/
	/* MCP open                                     */
	/************************************************/
	LogMsg( "Mcp_OpenServer ... ");
	Mcp = Mcp_OpenServer( Param->addr, Param->port);
	if( Mcp == NULL)
	{
		LogCri( "Mcp_OpenClient error. addr=[%s] port=[%d]", Param->addr, Param->port);
		return -1;
	} 
	LogMsg( "Mcp_OpenServer ... success");

	/************************************************/
	/* 매칭 엔진 open                               */
	/************************************************/
	LogMsg( "Mat_Open ... ");
	Mat = Mat_Open( 0);
	if( Mat == NULL)
	{
		LogCri( "매칭 엔진 open error.");
		LogMsg( "Mat_Open ... fail");
		return -1;
	}
	LogMsg( "Mat_Open ... success");

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
	int				i, f_len = strlen( Param->filter);
	char			buf[ 8192];
	struct timeval	timeout;
	APSISE			*sise = ( APSISE *)&buf;

	time_t			cur_time, old_time = 0;
	int				cnt = 0, old = 0;
	int				u_cnt = 0, u_old = 0;

	fd_set			rfds;

	while( Continue)
	{
		time( &cur_time);
		if( cur_time / 60 != old_time)
		{
			LogMsg( "loop ... cnt=[%7d/%7d][%7d/%7d]", u_cnt, cnt, u_cnt -u_old, cnt -old);
			old_time = cur_time / 60;
			old = cnt;
			u_old = u_cnt;
		}
		if( Mat->map->stat.sise)
		{
			LogMsg( "시세수신 작업이 중단되었습니다. Mat->map->stat.sise=[%d]", Mat->map->stat.sise);
			sleep( 1);
			continue;
		}

		cnt++;

		FD_ZERO( &rfds);
		FD_SET( Mcp->sock, &rfds);

		timeout.tv_sec  = Param->timeout / 1000000;
		timeout.tv_usec = Param->timeout % 1000000;

		rtn = select( Mcp->sock +1, &rfds, NULL, NULL, &timeout);
		if( rtn < 0)
		{
			LogErr( "select error. sock=[%d] fd=[%d]", Mcp->sock);
			switch( errno)
			{
				case EBUSY:
				case ECANCELED:
				case EINTR:
					continue;
				default:
					return -1;

			}
		}
		else
		if( rtn == 0)
		{
			LogMsg( "select timeout. timeout=[%d:%06d]", Param->timeout / 1000000, Param->timeout % 1000000);
			continue;
		}

		if( FD_ISSET( Mcp->sock, &rfds))
		{
			Mat_StatisticsSet( Mat, MAT_STAT_SIS, MAT_STAT_START);
			rtn = Mcp_Recv( Mcp, buf, 8192);
			if( rtn <= 0)
			{
				if( rtn == 0)
				{
					/*
					LogMsg( "Mcp_Recv timeout. timeout=[%d.%d]", Param->timeout / 1000000, Param->timeout % 1000000);
					rtn = Proc_Timeout( Mat);
					*/
					continue;
				}
				LogMsg( "Mcp_Recv error. rtn=[%d]", rtn);
				return -1;
			}
			if( Param->log_level < 5)
				LogDev( "[%.2s][%.3s][%.7s][%15f][%15f]", sise->type, sise->excode, sise->symb, sise->bidprc, sise->offerprc);
#if 0
			LogMsg( "[%.2s][%.3s][%.7s][%15f][%15f]", sise->type, sise->excode, sise->symb, sise->bidprc, sise->offerprc);
			LogDddd( buf, rtn, "receive mcp sise.... sz=[%d]", rtn);
			/*
			APSISE_Print( buf);
			if( sise->excode[ 0] != 'B') continue;
			*/
#endif

			/************************************************/
			/* CNH 통화는 일반 고객에게는 없는 통화         */
			/* 따라서 CNH 통화가 틀어오면 CNY로 바꿔서 전송 */
			/* 엔진에서 CNYKRW 통화 매칭을 위해             */
			/************************************************/
			if( memcmp( sise->type, "FA", 2))	/* FA가 아님 무시, FB는 swap rete 이므로 거름 */
			{
				continue;
			}

			/************************************************/
			/* filter 적용                                  */
			/************************************************/
			if( f_len > 0)
			{
				for( i = 0; i < f_len; i++)
				{
					if( sise->excode[ 0] == Param->filter[ i]) break;
				}
				if( i >= f_len) continue;
			}
#if 0
			APSISE_Print( sise);
			LogDev( "[%.2s][%.3s][%.7s][%15f][%15f]", sise->type, sise->excode, sise->symb, sise->bidprc, sise->offerprc);
			LogMsg( "[%.2s][%.3s][%.7s][%15f][%15f]", sise->type, sise->excode, sise->symb, sise->bidprc, sise->offerprc);
			LogMsg( "excode=[%c] Param->filter[ %d]=[%c]", sise->excode[ 0], i, Param->filter[ i]);
#endif

#if 0
			20240314 - CNH 시세는 더이상 들어오지 않음
			else
			if( !memcmp( sise->symb, "USD/CNH", 7))
			{
				/* USD/CNH -> USD/CNY convert는 송신매체에서 변환하므로 여기선 USD/CNH는 무시 20240109 */
				continue;
				memcpy( sise->symb, "USD/CNY", 7);
			}
			else
			if( !memcmp( sise->symb, "CNH/KRW", 7)) /* CNH/KRW는 CNY/KRW 거래와는 상관 없는통화 - 무시 */
			{
				LogDel( "sise->symb=[%.7s] 무시", sise->symb);
				continue;
			}
#endif

			rtn = SiseProcess( Mat, sise);
			if( rtn < 0)
			{
				LogMsg( "Proc_Match error. rtn=[%d]", rtn);
			}
			u_cnt++;

			Mat_StatisticsSet( Mat, MAT_STAT_SIS, MAT_STAT_END);
		}	/* if( FD_ISSET( Mcp->sock)) */
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
	LogMsg( "##### PROCESS END at %s pid=[%d]", TtoS( cur_time));
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
	LogMsg( "param->addr           = [%s]", param->addr);
	LogMsg( "param->port           = [%d]", param->port);
	LogMsg( "param->filter         = [%s]", param->filter);
	LogMsg( "param->interval       = [%d]", param->interval);
	LogMsg( "param->timeout        = [%d]", param->timeout);
	return 1;
}

/*********************************************************************************************************
*
*********************************************************************************************************/
/** ***************************************************************************
**  @fn         int Proc_()
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  매칭
***************************************************************************** */
int SiseProcess( MAT *mat, APSISE *sise)
{
	int			rtn = 1;
	MATSISE		*mat_sise = ( MATSISE *)sise;

#if 0
	너무 오래걸림 (100 micro seconds 이상)
	struct tm	_tb, *tp = &_tb;

	tp->tm_year = AtoI( &mat_sise->date[ 0], 4) -1900;
	tp->tm_mon  = AtoI( &mat_sise->date[ 4], 2) -1;
	tp->tm_mday = AtoI( &mat_sise->date[ 6], 2);
	tp->tm_hour = AtoI( &mat_sise->time[ 0], 2);
	tp->tm_min  = AtoI( &mat_sise->time[ 2], 2);
	tp->tm_sec  = AtoI( &mat_sise->time[ 4], 2);
	mat_sise->ctime = mktime( tp);
#else
	time( &mat_sise->ctime);
#endif

#if 0
	MAT_INDEX	*index;
	int			base, cont;
	time_t		cur_time;
	struct tm	_tp, *tp = &_tp;

	LogDel( "Match Symb=[%.6s] bid=[%9f] ask=[%9f]", 
		sise->symb, sise->bidprc, sise->offerprc);

	/* convert APSISE -> MATSISE */
	time( &cur_time);
	localtime_r( &cur_time, tp);
	tp->tm_year = AtoI( &sise->date[ 0], 4) -1900;
	tp->tm_mon = AtoI( &sise->date[ 4], 2) -1;
	tp->tm_mday = AtoI( &sise->date[ 6], 2);
	tp->tm_hour = AtoI( &sise->time[ 0], 2);
	tp->tm_min = AtoI( &sise->time[ 2], 2);
	tp->tm_sec = AtoI( &sise->time[ 4], 2);
	mat_sise->ctime = mktime( tp);
	base = Mat_GetCurrentInt( mat, &sise->symb[ 0]);
	if( base < 0) { rtn = -1; goto error; }
	cont = Mat_GetCurrentInt( mat, &sise->symb[ 4]);
	if( cont < 0) { rtn = -2; goto error; }
	index = Mat_GetIndex( mat, base, cont);
	if( index == NULL) { rtn = -3; goto error; }
	mat_sise->price_time = index->price_time;
	LogDel( "[%d][%d][%d]", tp->tm_year, tp->tm_mon, tp->tm_mday);
	LogDel( "[%d][%d][%d]", tp->tm_hour, tp->tm_min, tp->tm_sec);
	LogDel(  "sym=[%.7s][%ld]", mat_sise->symb, mat_sise->ctime);
#endif

#if 0
	MATSISE_Print( mat_sise);
#endif
	rtn = Mat_Sise( mat, mat_sise);
	if( rtn < 0)
	{
		LogCri( "Mat_Match error. rtn=[%d]", rtn);
		rtn = -4;
		goto error;
	}
	LogDel( "Mat_Match ... rtn = [%d]", rtn);

	return 1;

	error:
		switch( rtn)
		{
			case -1:
				LogCri( "base current not found. current=[%.3s]", &sise->symb[ 0]);
				break;
			case -2:
				LogCri( "cont current not found. current=[%.3s]", &sise->symb[ 4]);
				break;
			case -3:
				LogCri( "index not found. current=[%.7s]", sise->symb);
				break;
			case -4:
				break;
		}
		MATSISE_Print( mat_sise);
		return rtn;
}


