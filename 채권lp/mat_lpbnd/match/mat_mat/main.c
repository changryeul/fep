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
#include "order.h"
#include "sise.h"

#include "main.h"
#include "task.h"

#define	MAX_TEST	10000000

/** ***************************************************************************
**	GLOBAL
***************************************************************************** */
CFG			*Cfg;
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
		sprintf( Param->cfg_name, "%s/%s", getenv( "MAT_CFG"), "mat_mat.cfg");
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

	/************************************************/
	/* NONBLOCK mode set                            */
	/************************************************/
	/* NONBLOCK mode - read에 들어가 무한정 기다리는것 방지 SIG_TERM도 안먹음 */
	/* multi process - select에서 event를 받았으나 다른 process가 가져가 버려 read에서 무한정 기다림 방지 */
	rtn = fcntl( Mat->mat_fd, F_SETFL, O_NONBLOCK);
	if( rtn < 0)
	{
		LogErr( "fcntl error. rtn=[%d] opt=[%s]", rtn, "O_NONBLOCK");
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
	struct timeval	timeout;

	time_t			cur_time, old_time = 0;
	int				cnt = 0, old = 0;
	int				loop = 0, old_loop = 0;

	fd_set			rfds;
	int				idx_pos;

	while( Continue)
	{
		time( &cur_time);
		if( cur_time / 60 != old_time)
		{
			LogMsg( "loop=[%8d][%6d] cnt=[%7d][%5d] pid=[%d]", 
					loop, loop -old_loop, 
					cnt, cnt -old, 
					getpid());
			old_time = cur_time / 60;
			old = cnt;
			old_loop = loop;
		}
		loop++;

		FD_ZERO( &rfds);
		FD_SET( Mat->mat_fd, &rfds);

		timeout.tv_sec  = Param->timeout / 1000000;
		timeout.tv_usec = Param->timeout % 1000000;

		rtn = select( Mat->mat_fd +1, &rfds, NULL, NULL, &timeout);
		if( rtn < 0)
		{
			LogErr( "select error. Mat->mat_fd=[%d]", Mat->mat_fd);
			switch( errno)
			{
				case EBUSY:
				case ECANCELED:
				case EINTR:
				case EAGAIN:
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

		if( FD_ISSET( Mat->mat_fd, &rfds))
		{
			/* read 전에 Lock을 걸어 1개 이상의 process가 read 하는것 방지 */
			idx_pos = -1;

			rtn = read( Mat->mat_fd, &idx_pos, sizeof( int));
			if( rtn < ( int)sizeof( int))
			{
				LogDel( "read pipe idx_pos=[%2d] rtn=[%d]", idx_pos, rtn);
				if( rtn == 0)
				{
					/* non_block mode */
					continue;
				}
				switch( errno)
				{
					case EBUSY:
					case ECANCELED:
					case EINTR:
					case EAGAIN:
						continue;
					default:
						LogErr( "pipe read error. rtn=[%d] Mat->mat_fd=[%d]", rtn, Mat->mat_fd);
						return -1;
				}
			}
			LogDel( "read pipe idx_pos=[%2d] rtn=[%d]", idx_pos, rtn);

			if( ( idx_pos < 0) || ( idx_pos >= MAT_MAX_CURR)) 
			{
				LogCri( "read idx_pos error. idx_pos=[%d] rtn=[%d]", idx_pos, rtn);
				return -1;
			}

			Mat_Lock( Mat);
			Mat_StatisticsSet( Mat, MAT_STAT_DIR, MAT_STAT_START);

			rtn = Mat_Matching( Mat, idx_pos);
			if( rtn < 0)
			{
				Mat_Unlock( Mat);
				LogCri( "Mat_Matching error. rtn=[%d]", rtn);
				return -1;
			}
			Mat_StatisticsSet( Mat, MAT_STAT_DIR, MAT_STAT_END);
			Mat_Unlock( Mat);
			cnt++;
		}	/* if( FD_ISSET( Mat->mat_fd)) */
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
	LogMsg( "param->interval       = [%d]", param->interval);
	LogMsg( "param->timeout        = [%d]", param->timeout);
	return 1;
}

