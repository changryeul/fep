#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "cmd.h"
#include "task.h"
#include "bok_fx.h"

int	CmdProc( int argc, char *argv[]);
int	CmdBind( int argc, char *argv[]);
int	CmdSend( int argc, char *argv[]);
int	CmdRecv( int argc, char *argv[]);
int	CmdClose( int argc, char *argv[]);

int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

int	BokProtocol( UDP *udp);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"proc",			CmdProc,		"none",				"process to server"},
	{	1,		"bind",			CmdBind,		"[addr [port]]",	"connect to server"},
	{	2,		"send",			CmdSend,		"[packet_name]",	"send packet"},
	{	3,		"recv",			CmdRecv,		"[count]",			"receive packet"},
	{	4,		"close",		CmdClose,		"[packet]",			"receive packet"},
	{	100,	"help",			CmdHelp,		"none",				"stop process"},
	{	101,	"quit",			CmdQuit,		"none",				"stop process"},
	{	-1,		"\0",			NULL,			"\0",				"\0"}
};

extern CLIENT	*Client;

int TaskInit()
{
	Cmd = Cmd_Open( CmdTable);
	if( Cmd == NULL)
	{
		LogCri( "Cmd_Open error. cmd_table=[%p]", CmdTable);
		return -1;
	}

	return 1;
}

int CmdProc( int argc, char *argv[])
{
	char	*cmd[ 10] = { "send", "link" , 0, 0};

	CmdBind( Param->addr, Param->port);

	while( 1)
	{
		sleep( 20);
		cmd[ 1] = "poll";
		CmdRecv( 1, recv);
	}
	return 1;
}

int CmdBind( int argc, char *argv[])
{
	int		rtn;
	int		rs_flag = 0;
	UDP		*udp;

	LogDbg( "udp server .............");

	switch( argc)
	{
		case 3:
			Param->port = atoi( argv[ 2]);
		case 2:
			sprintf( Param->addr, "%s", argv[ 1]);
		case 1:
		default:
			break;
	}

	Client->udp = Udp_OpenClient( Param->addr, Param->port);
	if( Client->udp == NULL)
	{
		LogCri( "Udp_OpenClient error. addr=[%s] port=[%d]", Param->addr, Param->port);
		return -1;
	}
	LogMsg( "Udp_OpenClient success. udp=[%p]", Client->udp);
	Udp_Info( Client->udp);
	Client->stat = 0;

	return 1;
}

int CmdClose( int argc, char *argv[])
{
	Udp_Close( Client->udp);
	Client->udp == NULL;

	return 1;
}


int CmdSend( int argc, char *argv[])
{
	int		rtn;

	Cfg_Set( Cfg, "DATA");
	Client->s_sz = Cfg_Get( Cfg, argv[ 1], Client->send, sizeof( Client->send));
	rtn = Udp_Send( Client->udp, Client->send, Client->s_sz);
	if( rtn <= 0)
	{
		return rtn;
	}

	rtn = CmdRecv( argc, argv);

	return rtn;
}

int CmdRecv( int argc, char *argv[])
{
	int		rtn, rcv_sz;
	int		cnt = 100;

	if( argc < 1)
	{
		printf( "Argument error.");
		return 0;
	}

	if( argc >= 2)	cnt = atoi( argv[ 1]);
	else			cnt = 0x7fffff;

	Client->r_sz = 0;

	while( cnt > 0)
	{
		rtn = Udp_Recv( Client->udp, &Client->recv[ Client->r_sz], 8192);
		if( rtn < 0)
		{
			LogCri( "Udp_RecvN error. rtn=[%d] ... close socket sock=[%d]", rtn, Client->udp->sock);
			return 0;
		}
		Client->r_sz = 0;
		cnt--;
	}

	return Client->r_sz;
}












int CmdHelp( int argc, char *argv[])
{
	int		rtn = 1;

	rtn = Cmd_IntHelp( Cmd, argc, argv);
	return rtn;
}

int CmdQuit( int argc, char *argv[])
{
	LogDbg( "quit.............");

	exit( 1);
	return 1;
}

