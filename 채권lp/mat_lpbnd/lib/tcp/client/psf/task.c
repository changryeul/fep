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
	int		rtn;
	SEND_PACKET		*recv = Client->recv;
	SEND_PACKET		*send = Client->send;
	unsigned short	task;

	char	*cmd [ 10] = { "recv", 0, 0, 0};
	char	*link[ 10] = { "recv", "link" , 0, 0};
	char	*data[ 10] = { "recv", "link" , 0, 0};

	CmdConnect( argc, argv);

	while( 1)
	{
		rtn = CmdRecv( 1, cmd);
		if( rtn <= 0) break;
		SEND_PACKET_Print( recv);
		memcpy( ( char *)&task, recv->TELG_FG, 2);

		switch( task)
		{
			case 'A0':
				LogDbg( "개시전문 Receive");
				Client->s_sz = GetConfigData( "link", send, 512);
				rtn = CmdSend( 2, link);
				if( rtn <= 0) 
				{
					return -1;
				}
				break;

			case 'DN':
				LogDbg( "데이타 전문 Receive");
				Client->seq = AtoI( recv->TELG_SEQ, sizeof( recv->TELG_SEQ));

				Client->s_sz = GetConfigData( "data", send, 512);
				ItoA( send->TELG_SEQ, Client->seq, sizeof( send->TELG_SEQ));
				rtn = CmdSend( 2, link);
				if( rtn <= 0) 
				{
					return -1;
				}
				break;
			default:
				break;
		}
		usleep( Param->interval);
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

	LogDbg( "Tcp_Send( Client->tcp=[%p], Client->send=[%.32s], Client->s_sz=[%d]) call ...", 
			Client->tcp, Client->send, Client->s_sz);
	rtn = Tcp_Send( Client->tcp, Client->send, Client->s_sz);
	if( rtn <= 0)
	{
		return rtn;
	}

	return rtn;
}

int CmdRecv( int argc, char *argv[])
{
	int		rtn, rcv_sz;

	Client->r_sz = 0;

	rtn = Tcp_Recv( Client->tcp, &Client->recv[ Client->r_sz], 1);
	if( rtn < 1)
	{
		LogCri( "Tcp_RecvN error. rtn=[%d] ... close socket sock=[%d]", rtn, Client->tcp->sock);
		Tcp_Close( Client->tcp);
		return 0;
	}
	Client->r_sz += rtn;
	if( Client->recv[ 0] != 0x02)
	{
		LogCri( "STX recv error.");
		return -1;
	}

	rtn = Tcp_Recv( Client->tcp, &Client->recv[ Client->r_sz], 6);
	if( rtn < 6)
	{
		LogCri( "Tcp_RecvN error. rtn=[%d] ... close socket sock=[%d]", rtn, Client->tcp->sock);
		Tcp_Close( Client->tcp);
		return 0;
	}
	Client->r_sz += rtn;

	rcv_sz = AtoI( &Client->recv[ 1], 6);

	rtn = Tcp_Recv( Client->tcp, &Client->recv[ Client->r_sz], rcv_sz);
	if( rtn < rcv_sz)
	{
		LogCri( "Tcp_RecvN error. rtn=[%d] ... close socket sock=[%d]", rtn, Client->tcp->sock);
		Tcp_Close( Client->tcp);
		return 0;
	}
	Client->r_sz += rtn;

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













int SEND_PACKET_Print( SEND_PACKET* ptr)
{
    LogRaw( "%s", "----[ SEND_PACKET ]---------------------------------------------------------------------\n");
    LogRaw( "STX:0x02                      STX                    1    0 = [%.1s]\n",   ptr->STX);
    LogRaw( "전문길이                      TELG_LNGT              6    1 = [%.6s]\n",   ptr->TELG_LNGT);
    LogRaw( "MASTER                        MASTER                 1    7 = [%.1s]\n",   ptr->MASTER);
    LogRaw( "전문구분                      TELG_FG                2    8 = [%.2s]\n",   ptr->TELG_FG);
    LogRaw( "전문번호                      TELG_SEQ               6   10 = [%.6s]\n",   ptr->TELG_SEQ);
    LogRaw( "전송일시                      TELG_SND_DT           14   16 = [%.14s]\n",  ptr->TELG_SND_DT);
    LogRaw( "응답코드                      RESP_CD                2   30 = [%.2s]\n",   ptr->RESP_CD);
    LogRaw( "예약1                         RESV1                 10   32 = [%.10s]\n",  ptr->RESV1);
    LogRaw( "PGM DATA                      pgmd                 1024   42 = [%.1024s]\n",       ptr->pgmd);
    LogRaw( "%s", "---------------------------------------------------------------------[ SEND_PACKET ]----\n");

    return sizeof( SEND_PACKET);
}

int GetConfigData( char *name, char *rec, int sz)
{
	Cfg_Set( Cfg, "DATA");
	LogDbg( "get config data. name=[%s]", name);
	Client->s_sz = Cfg_Get( Cfg, name, Client->send, sz);

	return Client->s_sz;
}


