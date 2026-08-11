#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "cmd.h"
#include "task.h"
#include "bok_fx.h"

int	CmdProc( int argc, char *argv[]);
int	CmdConnect( int argc, char *argv[]);
int	CmdSend( int argc, char *argv[]);
int	CmdRecv( int argc, char *argv[]);
int	CmdClose( int argc, char *argv[]);

int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

int	BokProtocol( TCP *tcp);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"proc",			CmdProc,		"none",				"process to server"},
	{	1,		"connect",		CmdConnect,		"[addr [port]]",	"connect to server"},
	{	2,		"send",			CmdSend,		"[packet_name]",	"send packet"},
	{	3,		"recv",			CmdRecv,		"[none]",			"receive packet"},
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

	CmdConnect( argc, argv);
	CmdSend( 2, cmd);

	while( 1)
	{
		sleep( 20);
		cmd[ 1] = "poll";
		CmdSend( 2, cmd);
	}
	return 1;
}

int CmdConnect( int argc, char *argv[])
{
	int		rtn;
	int		rs_flag = 0;
	TCP		*tcp;

	LogDbg( "tcp server .............");

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

	Client->tcp = Tcp_OpenClient( Param->addr, Param->port);
	if( Client->tcp == NULL)
	{
		LogCri( "Tcp_OpenClient error. addr=[%s] port=[%d]", Param->addr, Param->port);
		return -1;
	}
	LogMsg( "Tcp_OpenClient success. tcp=[%p]", Client->tcp);
	Tcp_Info( Client->tcp);
	Client->stat = 0;

	return 1;
}

int CmdClose( int argc, char *argv[])
{
	Tcp_Close( Client->tcp);
	Client->tcp == NULL;

	return 1;
}


int CmdSend( int argc, char *argv[])
{
	int		rtn;

	Cfg_Set( Cfg, "DATA");
	Client->s_sz = Cfg_Get( Cfg, argv[ 1], Client->send, sizeof( Client->send));
	rtn = Tcp_Send( Client->tcp, Client->send, Client->s_sz);
	if( rtn <= 0)
	{
		return rtn;
	}
	LogDbg( "send data=[%d:%s]", Client->s_sz, Client->send);

	rtn = CmdRecv( argc, argv);

	return rtn;
}

int CmdRecv( int argc, char *argv[])
{
	int		rtn, rcv_sz;

	Client->r_sz = 0;

	rtn = Tcp_Recv( Client->tcp, &Client->recv[ Client->r_sz], 4);
	if( rtn < 4)
	{
		LogCri( "Tcp_RecvN error. rtn=[%d] ... close socket sock=[%d]", rtn, Client->tcp->sock);
		Tcp_Close( Client->tcp);
		return 0;
	}
	Client->r_sz += rtn;

	rcv_sz = AtoI( Client->recv, 4);

	rtn = Tcp_Recv( Client->tcp, &Client->recv[ Client->r_sz], rcv_sz);
	if( rtn < rcv_sz)
	{
		LogCri( "Tcp_RecvN error. rtn=[%d] ... close socket sock=[%d]", rtn, Client->tcp->sock);
		Tcp_Close( Client->tcp);
		return 0;
	}
	Client->r_sz += rtn;
	LogDbg( "recv data=[%d:%s]", Client->r_sz, Client->recv);

	return Client->r_sz;
}












int CmdHelp( int argc, char *argv[])
{
	int		rtn;

	rtn = Cmd_IntHelp( Cmd, argc, argv);
	return rtn;
}

int CmdQuit( int argc, char *argv[])
{
	LogDbg( "quit.............");

	exit( 1);
	return 1;
}

