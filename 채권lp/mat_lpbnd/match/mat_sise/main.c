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
#include "udp.h"
#include "order.h"
#include "sise.h"
#include "proc.h"

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
	0,				/* log_lavel */
	"",				/* addr */
	0,				/* port */
	0,				/* interval */
	0,				/* timeout */
	"",				/* filter */
	"",				/* file_name */
	0,				/* send_flag */
	0,				/* loop_flag */
};
PARAM	*Param = &ParamBuf;

/** ***************************************************************************
**	Argument & Option
***************************************************************************** */
char	*OptStr = "hrsc:";
char	Usage[] = 
"Usage: %s [-c config_name]\n"
"Option: -o, --option                \n"
"  h, help                    this help message\n"
"  c, config                  config file name (default:./main.cfg)\n"
"  s, send                    send mode\n"
"  r, recv                    receive mode\n"
;
struct option LongOption[] = 
{
	{ "help",		no_argument,		0,	'h'},
	{ "config",		required_argument,	0,	'c'},
	{ "recv",		no_argument,		0,	'r'},
	{ "send",		no_argument,		0,	's'},
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
				break;
			case 'r' :
				Param->send_flag = 0;
				LogMsg( "set Param->send_flag=[%d]", Param->send_flag);
				break;
			case 's' :
				Param->send_flag = 1;
				LogMsg( "set Param->send_flag=[%d]", Param->send_flag);
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
		sprintf( Param->cfg_name, "%s/%s", "./" /* getenv( "MAT_CFG") */, "main.cfg" /* "mat_sis.cfg" */);
		LogDel( "config file load.  Param->cfg_name = [%s]", Param->cfg_name);
	}

	Cfg = Cfg_Open( Param->cfg_name);
	if( Cfg == NULL)
	{
		LogCri( "config file open error. name=[%s]", Param->cfg_name);
		return -1;
	}

	if( strchr( Param->argo, 's') == NULL && strchr( Param->argo, 'r') == NULL)
	{
		Param->send_flag  = Cfg_GetInt( Cfg, "send_flag");
		LogDel( "cfg get. Param->send_flag=[%d]", Param->send_flag);
	}
	if( Param->send_flag) Param->loop_flag  = Cfg_GetInt( Cfg, "loop_flag");

	Cfg_Get( Cfg, "filter", Param->filter, 32);

	Cfg_Get( Cfg, "addr", Param->addr, 32);
	Param->port  = Cfg_GetInt( Cfg, "port");

	Param->interval   = Cfg_GetInt( Cfg, "interval");
	Param->timeout    = Cfg_GetInt( Cfg, "timeout");

	rtn = Cfg_Get( Cfg, "log_name", Param->log_name, 512);
	if( rtn <= 0)
	{
		LogCri( "Config not found. name=[%s]", "log_name");
		return -1;
	}
	Param->log_flag  = Cfg_GetInt( Cfg, "log_flag");
	Param->log_level = Cfg_GetInt( Cfg, "log_level");

	rtn = Cfg_Get( Cfg, "file_name", Param->file_name, 512);

	/************************************************/
	/* log file open                                */
	/************************************************/
	if( Param->send_flag <= 0)
	{
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
	}
	LogSetDump( LOG_DUMP_DEL);

	/************************************************/
	/* MCP open                                     */
	/************************************************/
	if( Param->send_flag > 0)
	{
		LogMsg( "Mcp_OpenClient ... ");
		Mcp = Mcp_OpenClient( Param->addr, Param->port);
		if( Mcp == NULL)
		{
			LogCri( "Mcp_OpenClient error. addr=[%s] port=[%d]", Param->addr, Param->port);
			return -1;
		} 
		LogMsg( "Mcp_OpenClient ... success");
	}
	else
	{
		LogMsg( "Mcp_OpenClient ... ");
		Mcp = Mcp_OpenServer( Param->addr, Param->port);
		if( Mcp == NULL)
		{
			LogCri( "Mcp_OpenClient error. addr=[%s] port=[%d]", Param->addr, Param->port);
			return -1;
		} 
		LogMsg( "Mcp_OpenServer ... success");
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
	if( Param->send_flag > 0)
	{
		SendProcess( argc, argv);
	}
	else
	{
		RecvProcess( argc, argv);
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
int Filter( APSISE *sise)
{
	int		pos = 0;

	while( Param->filter[ pos] != 0)
	{
		if( Param->filter[ pos] == '*')					return 1;
		if( sise->excode[ 0] == Param->filter[ pos])	return 1;
		pos++;
	}

	return 0;
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
int RecvProcess( int argc, char *argv[])
{
	int				rtn = 0;
	int				cnt = 0;
	char			buf[ 8192];
	APSISE			*sise = ( APSISE *)buf;

	FILE			*fp;

	LogDel( "Receive process start.");
	fp = fopen( Param->log_name, "a+");
	if( fp == NULL)
	{
		LogErr( "fopen error. name=[%s]", Param->log_name);
		goto error;
	}

	while( Continue)
	{
		if( cnt++ % 1000 == 0) LogDel( "send %d packet(s).", cnt);

		rtn = Mcp_RecvT( Mcp, buf, 8192, Param->timeout);
		if( rtn <= 0)
		{
			if( rtn == 0)
			{
				LogMsg( "Mcp_Recv timeout. timeout=[%d.%d]", Param->timeout / 1000000, Param->timeout % 1000000);
				continue;
			}
			LogMsg( "Mcp_Recv error. rtn=[%d]", rtn);
			goto error_1;
		}
		LogDump( buf, rtn, "receive udp sise.... sz=[%d]", rtn);
		LogDel( "recv %d bytes", rtn);
		LogDel( "[%.1s%.1s%.1s][%.7s][%15f][%15f]",
				sise->excode,
				sise->bidex,
				sise->offerex,
				sise->symb,
				sise->bidprc,
				sise->offerprc);
		rtn = Filter( sise);
		if( rtn)
		{
			APSISE_PrintFile( sise, fp);
		}
	}

	fclose( fp);
	return 1;

	error_1:
		fclose( fp);
	error:
		return -1;
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
**  프로그램 수행 - sise.dat 파일을 읽어 전송
***************************************************************************** */
int SendProcess( int argc, char *argv[])
{
	int				rtn = 0;
	int				sz;
	char			buf[ 8192];
	APSISE			*sise = ( APSISE *)buf;

#if 0
	LogSetDump( LOG_DUMP_DEC);
#endif

	while( Continue)
	{
		rtn = ReadProcess( sise);
		if( rtn < 0)
		{
			LogCri( "ReadProcess error. rtn=[%d]", rtn);
			return -1;
		}
		LogDel( "ReadProcess rtn=[%d]", rtn);
		sz = sizeof( APSISE);

#if 0
		LogDump( buf, rtn, "receive udp sise.... sz=[%d]", rtn);
		LogDel( "recv %d bytes", rtn);
#endif
		printf( "[%.1s%.1s%.1s][%.7s][%15f][%15f][%.9s]\n",
				sise->excode,
				sise->bidex,
				sise->offerex,
				sise->symb,
				sise->bidprc,
				sise->offerprc,
				sise->time
				);

		/* APSISE_PrintFile( sise, stdout); */
#if 1
		rtn = Mcp_SendTo( Mcp, buf, sz);
		if( rtn <= 0)
		{
			LogCri( "Udp_Send error. rtn=[%d]", rtn);
			sleep( 1);
		}
#endif
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
	LogMsg( "param->send_flag      = [%d]", param->send_flag);
	LogMsg( "param->loop_flag      = [%d]", param->loop_flag);
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
int ReadProcess( APSISE *sise)
{
	int			i;
	int			flag = 0, start = 0;
	static FILE	*fp = NULL;
	static int	stat = 0;
	static int	line = 0;
	char		rec[ 8192], buf[ 512];
	char		*ptr;
	int			sz, bsz;

	if( fp == NULL)
	{
		fp = fopen( Param->log_name, "r");
		if( fp == NULL)
		{
			LogErr( "file open error. name=[%s]", Param->log_name);
			return -1;
		}
		LogMsg( "Data file opened. name=[%s] fp=[%p]", Param->log_name, fp);
	}

	memset( sise, 0x00, sizeof( APSISE));

	while( Continue)
	{
		ptr = fgets( rec, 8192, fp);
		if( ptr == NULL)
		{
			LogMsg( "End of file. fp=[%p]", fp);
			/* sleep( 1); */
			if( Param->loop_flag)	fseek( fp, 0, SEEK_SET);
			else					return -1;
			continue;
		}
		line++;
		LogDel( "line=[%d]", line);
	
		bsz = 0;
		sz = strlen( rec);
		for( i = 0; i < sz; i++)
		{
			switch( rec[ i])
			{
				case '[':	flag = 1;	break;
				case ']':	flag = 0;	break;
				default:
					if( flag) buf[ bsz++] = rec[ i];
					break;
			}
		}
		buf[ bsz] = 0;
		if( bsz <= 0) continue;
		LogDel( "buf=[%s]", buf);

		LogDel( "line=[%d] stat=[%d] buf=[%s]", line, stat, buf);
	
		switch( stat)
		{
			case 0:		/* title */
				if( memcmp( buf, " APSISE ", 8)) continue;
				start = 1;
				break;
			case 17:	/* title */
				stat = 0;
				LogDel( "buf=[%s]", buf);
				if( memcmp( buf, " APSISE ", 8)) return -1;
				return sizeof( APSISE);
				break;
			case 1:		/* type */
				memcpy( sise->type, buf, sizeof( sise->type));
				break;
			case 2:		/* excode */
				memcpy( sise->excode, buf, sizeof( sise->excode));
				break;
			case 3:		/* bidex */
				memcpy( sise->bidex, buf, sizeof( sise->bidex));
				break;
			case 4:		/* offerex */
				memcpy( sise->offerex, buf, sizeof( sise->offerex));
				break;
			case 5:		/* symb */
				memcpy( sise->symb, buf, sizeof( sise->symb));
				break;
			case 6:		/* id */
				memcpy( sise->id, buf, sizeof( sise->id));
				break;
			case 7:		/* date */
				memcpy( sise->date, buf, sizeof( sise->date));
				break;
			case 8:		/* time */
				memcpy( sise->time, buf, sizeof( sise->time));
				break;
			case 9:		/* usdbid */
				sise->usdbid = strtod( buf, NULL);
				break;
			case 10:		/* usdoffer */
				sise->usdoffer = strtod( buf, NULL);
				break;
			case 11:		/* bidprc */
				sise->bidprc = strtod( buf, NULL);
				break;
			case 12:	/* offerprc */
				sise->offerprc = strtod( buf, NULL);
				break;
			case 13:	/* bidqty */
				sise->bidqty = strtod( buf, NULL);
				break;
			case 14:	/* offerqty */
				sise->offerqty = strtod( buf, NULL);
				break;
			case 15:	/* midprc */
				sise->offerqty = strtod( buf, NULL);
				break;
			case 16:	/* fillprc */
				sise->offerqty = strtod( buf, NULL);
				break;
		}
		if( start) stat++;
		LogDel( "stat=[%d]", stat);
	}
	return -1;
}

