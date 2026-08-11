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
#include "tcp.h"
#include "smq.h"
#include "order.h"
#include "sise.h"
#include "proc.h"

#include "main.h"
#include "task.h"
#include "comm.h"

#define	MAX_TEST		10000000
#define MAX_CLIENT		1

/** ***************************************************************************
**	GLOBAL
***************************************************************************** */
CFG			*Cfg;
SMQ			*Smq;
TCP			*TcpServer;
TCP			*TcpClient;		/* one client */
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
	"",				/* que_name */
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
"  c, config                  config file name (default:$MAT_CFG/mat_ord.cfg)\n"
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
		sprintf( Param->cfg_name, "%s/%s", getenv( "MAT_CFG"), "mat_snd.cfg");
		LogDel( "config file load.  Param->cfg_name = [%s]", Param->cfg_name);
	}
	Cfg = Cfg_Open( Param->cfg_name);
	if( Cfg == NULL)
	{
		LogCri( "config file open error. name=[%s]", Param->cfg_name);
		return -1;
	}

	Cfg_Get( Cfg, "que_name", Param->que_name, 32);

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

	/************************************************/
	/* log file open                                */
	/************************************************/
	LogLevel( Param->log_level);
	LogSetDump( LOG_DUMP_DEL);
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
	/* smq open for client                          */
	/************************************************/
	LogMsg( "Smq_Open ... name=[%s]", Param->que_name);
	Smq = Smq_Open( Param->que_name);
	if( Smq == NULL)
	{
		LogCri( "Smq_Open error. name=[%s]", Param->que_name);
		return -1;
	}
	LogMsg( "Smq_Open ... success");

	/************************************************/
	/* TCP open                                     */
	/************************************************/
	LogMsg( "Tcp_OpenClient ... ");
	TcpServer = Tcp_OpenServer( Param->addr, Param->port);
	if( TcpServer == NULL)
	{
		LogCri( "Tcp_OpenClient error. addr=[%s] port=[%d]", Param->addr, Param->port);
		return -1;
	} 
	Tcp_Listen( TcpServer);
	LogMsg( "Tcp_OpenClient ... success");

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
	int		rtn;
	TCP		*tcp;

	int				nfds = 0;
	fd_set			rfds;
	struct timeval	tv;

	LogDel( "ServerProc start.");

	LogDel( "listen ...");
	rtn = Tcp_Listen( TcpServer);
	if( rtn < 0)
	{
		LogCri( "Tcp_Listen error. rtn=[%d]", rtn);
		return -1;
	}

	while( Continue)
	{
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		nfds = 0;
		FD_ZERO( &rfds);
		FD_SET( TCP_SOCK( TcpServer), &rfds);
		nfds = TCP_SOCK( TcpServer);

		if( TcpClient != NULL)
		{
			FD_SET( TCP_SOCK( TcpClient), &rfds);
			nfds = Max( nfds, TCP_SOCK( TcpClient));
		}

		LogDel( "select ... wait=[%d.%dsec]", tv.tv_sec, tv.tv_usec);
		rtn = select( nfds +1, &rfds, NULL, NULL, &tv);
		if( rtn < 0)
		{
			LogErr( "select error. rtn=[%d]", rtn);
			goto error_1;
		}
		else
		if( rtn == 0)
		{
			LogDel( "timeout ... ");
			continue;
		}

		LogDel( "socket event ... ");
		if( FD_ISSET( TCP_SOCK( TcpServer), &rfds))
		{
			LogDel( "Server sock event ...");
			if( TcpClient != NULL)
			{
				LogCri( "too many connections!!! clinet=[%p]", TcpClient); 
				Tcp_Close( TcpClient);
				TcpClient = NULL;
			}

			LogDel( "accept ...");
			tcp = Tcp_Accept( TcpServer);
			LogDel( "accept tmp_cli=[%p] ", tcp);

			TcpClient = tcp;
		}
		if( FD_ISSET( TCP_SOCK( TcpClient), &rfds))
		{
			LogDel( "get event ... sock=[%d]", TCP_SOCK( TcpClient));
			rtn = DataProcess( TcpClient);
			if( rtn <= 0)
			{
				LogDel( "DataProcess error. close TcpClient=[%p]", TcpClient);
				Tcp_Close( TcpClient);
				TcpClient = NULL;
			}
		}
		
	}

	return 1;

	error_1:
		if( TcpClient != NULL) Tcp_Close( TcpClient);
		return -1;
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
	LogMsg( "param->que_name       = [%s]", param->que_name);
	LogMsg( "param->addr           = [%s]", param->addr);
	LogMsg( "param->port           = [%d]", param->port);
	LogMsg( "param->interval       = [%d]", param->interval);
	LogMsg( "param->timeout        = [%d]", param->timeout);
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
int DataProcess( TCP *tcp)
{
	int			rtn, sz;
	MAT_PACKET	_packet, *packet = &_packet;

	memset( packet, 0x00, sizeof( MAT_PACKET));
	rtn = RecvPacket( tcp, packet);
	if( rtn < 0)
	{
		LogCri( "RecvPacket error. rtn=[%d]", rtn);
		return -1;
	}

	if( !strncmp( packet->comm_head.type, "LINK", 4))
	{
		memcpy( packet->comm_head.type, "LIOK", 4);
		sz  = sizeof( MAT_COMM_HEAD);
		ItoA( packet->comm_head.len, sz - sizeof( packet->comm_head.len), sizeof( packet->comm_head.len));
		SendPacket( tcp, packet, sz);
		SmqProcess( tcp);
	}
	else
	if( !strncmp( packet->comm_head.type, "HTBT", 4))
	{
		memcpy( packet->comm_head.type, "HTOK", 4);
		sz  = sizeof( MAT_COMM_HEAD);
		ItoA( packet->comm_head.len, sz - sizeof( packet->comm_head.len), sizeof( packet->comm_head.len));
		SendPacket( tcp, packet, sz);
	}
	else
	if( !strncmp( packet->comm_head.type, "DATA", 4))
	{
		sz = rtn - sizeof( MAT_COMM_HEAD) - sizeof( MAT_DATA_HEAD);
		LogDel( "Smq_Send ... data=[%d:%s]", sz, packet->data);
		rtn = Smq_Send( Smq, packet->data, sz);
		if( rtn < 0)
		{
			LogCri( "Smq_Send error. rtn=[%d] data=[%d:%s]", sz, packet->data);
			return -1;
		}
		memcpy( packet->comm_head.type, "DAOK", 4);
		sz  = sizeof( MAT_COMM_HEAD);
		ItoA( packet->comm_head.len, sz - sizeof( packet->comm_head.len), sizeof( packet->comm_head.len));
		SendPacket( tcp, packet, sz);
	}

	return rtn;
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
int PollProcess( TCP *tcp)
{
	int			rtn, sz;
	time_t		cur_time;
	MAT_PACKET	_packet, *packet = &_packet;

	memset( packet, 0x00, sizeof( MAT_PACKET));

	/* set comm_head */
	sz  = sizeof( MAT_COMM_HEAD);
	ItoA( packet->comm_head.len, sz - sizeof( packet->comm_head.len), sizeof( packet->comm_head.len));
	memcpy( packet->comm_head.type, "HTBT", 4);
	ItoA( packet->comm_head.seq,   0, sizeof( packet->comm_head.seq));
	ItoA( packet->comm_head.block, 0, sizeof( packet->comm_head.block));
	ItoA( packet->comm_head.code,  0, sizeof( packet->comm_head.code));
	time( &cur_time);
	memcpy( packet->comm_head.date, "YYYYMMDDhhmmss      ", 20);
	TtoA( packet->comm_head.date, cur_time);
	memset( packet->comm_head.filler, 0x20, sizeof( packet->comm_head.filler));

	SendPacket( tcp, packet, sz);

	rtn = RecvPacket( tcp, packet);
	if( rtn < 0)
	{
		LogCri( "RecvPacket error. rtn=[%d]", rtn);
		return -1;
	}

	if( !strncmp( packet->comm_head.type, "HTOK", 4))
	{
		return rtn;
	}
	else
	if( !strncmp( packet->comm_head.type, "DAOK", 4))
	{
		return rtn;
	}

	return rtn;
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
int RecvPacket( TCP *tcp, MAT_PACKET *packet)
{
	int			rtn, sz;
	int			rcv_sz = 0;

	/* data length */
	rtn = Tcp_Recv( tcp, packet->comm_head.len, sizeof( packet->comm_head.len));
	if( rtn < sizeof( packet->comm_head.len))
	{
		LogCri( "Tcp_Recv error. rtn=[%d] size=[%d]", rtn, sizeof( packet->comm_head.len));
		return -1;
	}
	rcv_sz += rtn;
	sz = AtoI( packet->comm_head.len, sizeof( packet->comm_head.len));
	LogDel( "receive data size=[%d]", sz);

	/* data */
	rtn = Tcp_Recv( tcp, packet->comm_head.type, sz);
	if( rtn < sz)
	{
		LogCri( "Tcp_Recv error. rtn=[%d] size=[%d]", rtn, sizeof( packet->comm_head.len));
		return -1;
	}
	rcv_sz += rtn;

	LogMsg( "recv data. data=[%04d:%.80s]", rcv_sz, ( char *)packet);
#if 0
	LogDump( ( char *)packet, rcv_sz, "receive data. data=[%04d:%.80s]", rcv_sz, ( char *)packet);
	MAT_PACKET_Print( packet);
#endif

	return rcv_sz;
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
int SendPacket( TCP *tcp, MAT_PACKET *packet, int sz)
{
	int			rtn;

	/* data length */
	rtn = Tcp_Send( tcp, ( char *)packet, sz);
	if( rtn < sz)
	{
		LogCri( "Tcp_Send error. rtn=[%d] size=[%d]", rtn, sz);
		return -1;
	}
	LogMsg( "send data. data=[%04d:%.50s]", sz, ( char *)packet);
#if 0
	LogDump( ( char *)packet, sz, "send data. data=[%04d:%.80s]", sz, ( char *)packet);
	MAT_PACKET_Print( packet);
#endif

	return sz;
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
int SmqProcess( TCP *tcp)
{
	int			rtn, sz;
	time_t		cur_time;
	MAT_PACKET _packet, *packet = &_packet;
	MAT_PACKET _rcv_packet, *rcv_packet = &_rcv_packet;

	while( Continue)
	{
		memset( packet, 0x00, sizeof( MAT_PACKET));
		rtn = Smq_GetRecord( Smq, packet->data, sizeof( packet->data), Param->timeout);
		if( rtn < 0)
		{
			if( rtn == SMQ_TIMEOUT)
			{
				rtn = PollProcess( tcp);
				if( rtn <= 0)
				{
					LogCri( "PollProcess error. rtn=[%d]", rtn);
					return -1;
				}
				continue;
			}
			return -1;
		}
#if 0
		LogDump( packet->data, rtn, "Smq read data ... rtn=[%d]", rtn);
		ORDER_SEND_Print( ( ORDER_SEND *)packet->data);
#endif
		/* set comm_head */
		sz  = sizeof( MAT_COMM_HEAD) + sizeof( MAT_DATA_HEAD) + rtn;
		ItoA( packet->comm_head.len, sz - sizeof( packet->comm_head.len), sizeof( packet->comm_head.len));
		memcpy( packet->comm_head.type, "DATA", 4);
		ItoA( packet->comm_head.seq,   0, sizeof( packet->comm_head.seq));
		ItoA( packet->comm_head.block, 0, sizeof( packet->comm_head.block));
		ItoA( packet->comm_head.code,  0, sizeof( packet->comm_head.code));
		time( &cur_time);
		memcpy( packet->comm_head.date, "YYYYMMDDhhmmss      ", 20);
		TtoA( packet->comm_head.date, cur_time);
		memset( packet->comm_head.filler, 0x20, sizeof( packet->comm_head.filler));
		memset( &packet->data_head,       0x20, sizeof( packet->data_head));

		rtn = SendPacket( tcp, packet, sz);
		if( rtn < sz)
		{
			LogCri( "SendPacket error. rtn=[%d] sz=[%d]", rtn, sz);
			ORDER_SEND_Print( ( ORDER_SEND *)packet->data);
			return -1;
		}
		ORDER_SEND_Print( ( ORDER_SEND *)packet->data);
		rtn = RecvPacket( tcp, rcv_packet);
		if( rtn <= 0)
		{
			LogCri( "RecvPacket error. rtn=[%d]", rtn);
			ORDER_SEND_Print( ( ORDER_SEND *)packet->data);
			return -1;
		}
		Smq_Commit( Smq);
	}

	return 1;
}






