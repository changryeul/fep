#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "task.h"

int	CmdJbrs( int argc, char *argv[]);
int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"jbrs",			CmdJbrs,		"none",		"command test"},
	{	2,		"help",			CmdHelp,		"none",		"stop process"},
	{	3,		"quit",			CmdQuit,		"none",		"stop process"},
	{	-1,		"\0",			NULL,			"\0",		"\0"}
};

TaskInit()
{
	Cmd = Cmd_Open( CmdTable);
	if( Cmd == NULL)
	{
		LogCri( "Cmd_Open error. cmd_table=[%p]", CmdTable);
		return -1;
	}

	return 1;
}


CmdJbrs( int argc, char *argv[])
{
	return 1;
}

CmdHelp( int argc, char *argv[])
{
	int		rtn;

	rtn = Cmd_IntHelp( Cmd, argc, argv);
	return rtn;
}

CmdQuit( int argc, char *argv[])
{
	LogDbg( "quit.............");

	exit( 1);
	return 1;
}

