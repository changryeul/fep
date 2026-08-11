#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "malloc.h"
#include "map.h"
#include "task.h"

int	CmdOpen( int argc, char *argv[]);

int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"open",			CmdOpen,		"none",		"command test"},
	{	2,		"help",			CmdHelp,		"none",		"stop process"},
	{	3,		"quit",			CmdQuit,		"none",		"stop process"},
	{	-1,		"\0",			NULL,			"\0",		"\0"}
};

extern CFG		*Cfg;
extern MAP		*Map;
extern PARAM	Param;


int CmdOpen( int argc, char *argv[])
{
	int		i = 0;
	char	*ptr;

	if( argc < 2) return -1;

	printf( "%8d ptr=[%p]\n", i, ptr);

	MapInit();
	while( 1)
	{
		Map = Map_Open( argv[ 1]);
		LogDbg( "map=[%p]", Map);

		/*
		Map_DisplayMap( Map);
		while( 1)
		{
			Map_PrintName( Map, "name8", "test %d", i);
			Map_DisplayUpdate( Map);
			sleep( 1);
		}
		*/
		Map_Close( Map);
		LogDbg( "cnt=[%d]----------------------------------------------------------------------------", i++);
		Memory();
		usleep( 10000);
	}

	MapEnd();
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

