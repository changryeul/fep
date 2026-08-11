#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "task.h"

int	CmdReport( int argc, char *argv[]);
int	CmdMon( int argc, char *argv[]);
int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);
int	AlarmSet( int argc, char *argv[]);
int	Margin( int argc, char *argv[]);
int	Batch( int argc, char *argv[]);

int	TaskWarning( int argc, char *argv[]);
int	TaskError( int argc, char *argv[]);
int	TaskCritical( int argc, char *argv[]);

extern int 	LogPrint;
extern int	SbrsMarginView;
extern int	SbrsBatchView;

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	5,		"help",			CmdHelp,		"none", 		"command display"		},
	{	99,		"quit",			CmdQuit,		"none", 		"stop process"		},
	{	-1,		"\0",			NULL,			"\0",			"\0"				}
};

int CmdHelp( int argc, char *argv[])
{
	Cmd_IntHelp( Cmd, argc, argv);

	return 1;
}

int CmdQuit( int argc, char *argv[])
{
	LogDel( "quit.............");

	exit( 1);
	return 1;
}

