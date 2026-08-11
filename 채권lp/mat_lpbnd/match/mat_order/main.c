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
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>

#include "mat.h"
#include "smq.h"
#include "order.h"

#include "proc.h"

#include "main.h"

#define	MAX_TEST	10000000

/** ***************************************************************************
**	GLOBAL
***************************************************************************** */
CFG			*Cfg;
MAT_ORDER	_Mo;
MAT_ORDER	*Mo = &_Mo;
DATA_STRUCT	_DataStruct, *DataStruct = &_DataStruct;
FILE		*FilePtr = NULL;
int			FileDesc = -1;
char		ClOrdId[ 11 +1];
int			OrderNo = 0;
int			Continue = 1;

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
	"",				/* que_name */
	"",				/* file_name */
	"",				/* seq_file */
	0,				/* interval */
	0,				/* timeout */
	0,				/* loop */
	0,				/* type */
};
PARAM	*Param = &ParamBuf;

/** ***************************************************************************
**	Argument & Option
***************************************************************************** */
char	*OptStr = "hc:f:";
char	Usage[] = 
"Usage: %s [-h] [-c config_name] [-f order_file_name]\n"
"Option: -o, --option                \n"
"  h, help                    help\n"
"  c, config                  config file name (default:./main.cfg)\n"
"  f, file                    order file name (default:./order.dat)\n"
;
struct option LongOption[] = 
{
	{ "help",		no_argument,		0,	'h'},
	{ "config",		required_argument,	0,	'c'},
	{ "file",		required_argument,	0,	'f'},
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
			case 'c' :
				sprintf( Param->cfg_name, optarg, strlen( optarg) +1);
				LogMsg( "Param->cfg_name=[%s]", Param->cfg_name);
				break;
			case 'f' :
				sprintf( Param->file_name, optarg, strlen( optarg) +1);
				LogMsg( "Param->file_name=[%s]", Param->file_name);
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
		sprintf( Param->cfg_name, "%s/%s", ".", "main.cfg" /* getenv( "MAT_CFG"), "mat_snd.cfg" */);
		LogDbg( "config file load.  Param->cfg_name = [%s]", Param->cfg_name);
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

	rtn = Cfg_Get( Cfg, "que_name", Param->que_name, 32);
	rtn = Cfg_Get( Cfg, "seq_file", Param->seq_file, 512);
	Param->interval  = Cfg_GetInt( Cfg, "interval");
	Param->timeout   = Cfg_GetInt( Cfg, "interval");
	Param->loop      = Cfg_GetInt( Cfg, "loop");
	Param->type      = Cfg_GetInt( Cfg, "type");

	if( strchr( Param->argo, 'f') == NULL)
	{
		rtn = Cfg_Get( Cfg, "file_name", Param->file_name, 512);
	}

	ParamPrint( Param);

	/************************************************/
	/* log file open                                */
	/************************************************/
	LogMsg( "log file. name=[%s] flag=[%d] level=[%d]", Param->log_name, Param->log_flag, Param->log_level);
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
	LogSetDump( LOG_DUMP_DATA);

	/************************************************/
	/* smq open for client                          */
	/************************************************/
	Mo->smq_send = Smq_Open( Param->que_name);
	if( Mo->smq_send == NULL)
	{
		LogCri( "Smq_Open error. name=[%s]", Param->que_name);
		return -1;
	}

	/************************************************/
	/* input order file open                        */
	/************************************************/
	FilePtr = fopen( Param->file_name, "r");
	if( FilePtr == NULL)
	{
		LogErr( "fopen ... name=[%s]", Param->file_name);
		return -1;
	}
	LogMsg( "File open success. name=[%s] FilePtr=[%p]", Param->file_name, FilePtr);

	/************************************************/
	/* Get ClOrdId seq                              */
	/************************************************/
	FileDesc = open( Param->seq_file, O_RDWR);
	if( FileDesc < 0)
	{
		LogErr( "fopen ... name=[%s]", Param->seq_file);
		return -1;
	}
	LogMsg( "File open success. name=[%s] FileDesc=[%d]", Param->seq_file, FileDesc);
	rtn = read( FileDesc, ClOrdId, sizeof( ClOrdId) -1);
	if( rtn < sizeof( ClOrdId) -1)
	{
		LogErr( "File read error. name=[%s] fd=[%d]", Param->seq_file, FileDesc);
		return -1;
	}
	OrderNo = atoi( &ClOrdId[ 4]) +1;
	LogMsg( "Start order no ClOrdId=[%s] OrderNo=[%d]", ClOrdId, OrderNo);

#if 0
	/************************************************/
	/* 매칭 엔진 open                               */
	/************************************************/
	Mat = Mat_Open();
	if( Mat == NULL)
	{
		LogCri( "매칭 엔진 open error.");
		return -1;
	}
#endif

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
	int				cnt = 0;
	char			*ptr;
	ORDER_RECV		*order;

	LogDbg( "Start MainProcess ...");

	if( FilePtr == NULL)
	{
		FilePtr = fopen( Param->file_name, "r");
		if( FilePtr == NULL)
		{
			LogErr( "fopen error. name=[%s]", Param->file_name);
			return -1;
		}
		LogMsg( "File open. name=[%s] FilePtr=[%p]", Param->file_name, FilePtr);
	}

	while( Continue)
	{
		ptr = fgets( DataStruct->buf, sizeof( DataStruct->buf), FilePtr);
		if( ptr == NULL)
		{
			if( Param->loop)
			{
				fseek( FilePtr, 0, SEEK_SET);
				continue;
			}
			LogMsg( "EOF ... name=[%s] FilePtr=[%p]", Param->file_name, FilePtr);
			return 1;
		}
		DataStruct->line++;
		DataStruct->buf_sz = strlen( DataStruct->buf);
		LogDel( "LINE[%4d] DATA[%4d:%s]", 
				DataStruct->line, DataStruct->buf_sz, DataStruct->buf);

		switch( Param->type)
		{
			case 1:			/* line Brace([]) data */
				rtn = LineProcess( DataStruct);
				LogDel( "DATA[%4d:%s]", DataStruct->rec_sz, DataStruct->rec);
				if( rtn <= 0)
				{
					continue;
				}
				break;
			case 2:			/* struct print format data */
				rtn = StructProcess( DataStruct);
				LogDel( "DATA[%4d:%s]", DataStruct->rec_sz, DataStruct->rec);
				if( rtn <= 0)
				{
					continue;
				}
				break;
			case 0:			/* line data */
			default:
				break;
		}

		order = ( ORDER_RECV *)DataStruct->rec;

		sprintf( &ClOrdId[ 4], "%07d", OrderNo++);
		memcpy( order->ClOrdID, ClOrdId, sizeof( order->ClOrdID));
		LogDbg( "[%.7s][%.1s][%.20s][%.*s]", 
			order->Symbol,
			order->Side,
			order->Price,
			sizeof( order->ClOrdID), order->ClOrdID);
#if 0
		ORDER_RECV_Print( ( ORDER_RECV *)DataStruct->rec);
#endif

		retry:
		rtn = Smq_Send( Mo->smq_send, DataStruct->rec, DataStruct->rec_sz);
		if( rtn < 0)
		{
			if( rtn == SMQ_TIMEOUT)	/* 빈 record가 없는경우 */
			{
				sleep( 1);
				goto retry;
			}
			LogCri( "Smq_Send error. rtn=[%d]", rtn);
			break;
		}
		cnt++;

		lseek( FileDesc, 0, SEEK_SET);
		ClOrdId[ 11] = 0;
		rtn = write( FileDesc, ClOrdId, strlen( ClOrdId));
		if( rtn < strlen( ClOrdId))
		{
			LogErr( "Write error. name=[%s] FileDesc=[%d]", Param->seq_file, FileDesc);
		}

		usleep( Param->interval);
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

	close( FileDesc);
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

	LogDbg( "param                 = [%p]", param);
	LogDbg( "param->argc           = [%d]", param->argc);
	for( i = 0; i < param->argc; i++)
	LogDbg( "param->argv[%3d]      = [%s]", i, param->argv[i]);
	LogDbg( "param->argo           = [%s]", param->argo);
	LogDbg( "param->args           = [%d]", param->args);
	LogDbg( "param->cfg_name       = [%s]", param->cfg_name);
	LogDbg( "param->log_name       = [%s]", param->log_name);
	LogDbg( "param->log_flag       = [%d]", param->log_flag);
	LogDbg( "param->log_level      = [%d]", param->log_level);
	LogDbg( "param->que_name       = [%s]", param->que_name);
	LogDbg( "param->file_name      = [%s]", param->file_name);
	LogDbg( "param->seq_file       = [%s]", param->seq_file);
	LogDbg( "param->interval       = [%d]", param->interval);
	LogDbg( "param->timeout        = [%d]", param->timeout);
	LogDbg( "param->loop           = [%d]", param->loop);
	LogDbg( "param->type           = [%d]", param->type);
	return 1;
}

/*********************************************************************************************************
*
*********************************************************************************************************/
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
int LineProcess( DATA_STRUCT *ds)
{
	int		i;
	int		start = 0;

	ds->rec_sz = 0;
	for( i = 0; i < ds->buf_sz; i++)
	{
		switch( ds->buf[ i])
		{
			case '[':	/* start */
				start = 1;
				break;
			case ']':	/* end */
				start = 0;
				break;
			default:
				if( start) ds->rec[ ds->rec_sz++] = ds->buf[ i];
				break;
		}
	}

	return ds->rec_sz;
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
int StructProcess( DATA_STRUCT *ds)
{
	int		i;
	int		start = 0;
	char	field[ 8192];
	int		pos = 0;

	for( i = 0; i < ds->buf_sz; i++)
	{
		switch( ds->buf[ i])
		{
			case '[':	/* start */
				start = 1;
				break;
			case ']':	/* end */
				start = 0;
				break;
			default:
				if( start) field[ pos++] = ds->buf[ i];
				break;
		}
	}
	field[ pos] = 0;

	if( !memcmp( field, " ORDER_RECV ", 12 +1))
	{
		if( ds->field == 0)		/* start */
		{
			LogDel( "field start .... ");
			ds->field++;
			ds->rec_sz = 0;
		}
		else					/* end */
		{
			LogDel( "field end .... ");
			ds->field = 0;
			return ds->rec_sz;
		}
	}
	else
	{
		memcpy( &ds->rec[ ds->rec_sz], field, pos);
		ds->rec_sz += pos;
	}

	LogDel( "DATA=[%4d:%s]", ds->rec_sz, ds->rec);
	
	return 0;
}


