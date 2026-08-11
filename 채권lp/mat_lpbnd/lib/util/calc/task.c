#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "calc.h"
#include "task.h"

int	CmdCalc( int argc, char *argv[]);
int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"cal",			CmdCalc,		"none",		"command test"},
	{	2,		"help",			CmdHelp,		"none",		"stop process"},
	{	3,		"quit",			CmdQuit,		"none",		"stop process"},
	{	-1,		"\0",			NULL,			"\0",		"\0"}
};

CALC	*Calc;

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

int CmdCalc( int argc, char *argv[])
{
	int		rtn, i, sz = 0;
	char	rec[ 512];

	Calc = Calc_Open();

	for( i = 1; i < argc; i++)
	{
		sz += sprintf( &rec[ sz], "%s ", argv[ i]);
	}
	rtn = Calc_Process( Calc, rec);

	Calc_Close( Calc);

	return 1;
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

