#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "udp.h"
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

extern UDP	*Udp;

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"recv",			CmdRecv,		"none",				"process to server"},
	{	2,		"send",			CmdSend,		"none",				"process to client"},
	{	100,	"help",			CmdHelp,		"none",				"stop process"},
	{	101,	"quit",			CmdQuit,		"none",				"stop process"},
	{	-1,		"\0",			NULL,			"\0",				"\0"}
};

int TaskInit()
{
	return 1;
}

int CmdRecv( int argc, char *argv[])
{
	int		rtn;
	char	rec[ 8192];

	rtn = Udp_RecvFrom( Udp, rec, 8192);
	if( rtn < 0)
	{
		LogErr( "Udp_Recv error.");
		return rtn;
	}
	LogMsg( "Udp_Recv rtn=[%d]", rtn);
	
	return 1;
}

int CmdSend( int argc, char *argv[])
{
	int		rtn, sz;
	char	rec[ 8192];

	sz = sprintf( rec, "%s", "Hello ...");

	rtn = Udp_SendTo( Udp, rec, sz);
	if( rtn < 0)
	{
		LogErr( "Udp_Recv error.");
		return rtn;
	}
	LogMsg( "Udp_Recv rtn=[%d]", rtn);
	
	return 1;
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

