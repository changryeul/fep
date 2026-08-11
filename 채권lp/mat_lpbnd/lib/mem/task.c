#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "mem.h"
#include "task.h"

int	TaskOpen( int argc, char *argv[]);

int	TaskHelp( int argc, char *argv[]);
int	TaskQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"open",			TaskOpen,		"none",		"command test"},
	{	2,		"help",			TaskHelp,		"none",		"stop process"},
	{	3,		"quit",			TaskQuit,		"none",		"stop process"},
	{	-1,		"\0",			NULL,			"\0",		"\0"}
};

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

int TaskOpen( int argc, char *argv[])
{
	MEM	*mem;

	mem = Mem_Open( 0xcdc00001);
	LogDbg( "open............. mem=[%p]", mem);

	return 1;
}

int TaskHelp( int argc, char *argv[])
{
	int		rtn;

	rtn = Cmd_IntHelp( Cmd, argc, argv);
	return rtn;
}

int TaskQuit( int argc, char *argv[])
{
	LogDbg( "quit.............");

	exit( 1);
	return 1;
}

