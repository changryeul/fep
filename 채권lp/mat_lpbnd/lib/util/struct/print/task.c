#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "task.h"

int	TaskData( int argc, char *argv[]);
int	TaskLine( int argc, char *argv[]);

int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,			"data",			TaskData,		"none", 		"command display"		},
	{	1,			"line",			TaskLine,		"none", 		"command display"		},
	{	101,		"help",			CmdHelp,		"none", 		"command display"		},
	{	102,		"quit",			CmdQuit,		"none", 		"stop process"			},
	{	-1,			"\0",			NULL,			"\0",			"\0"					}
};


/*********************************************************************************************************
 *
*********************************************************************************************************/
TaskData( int argc, char *argv[])
{
	int		pos = 0, sz;
	char	*ptr;
	char	rec[ 8192];
	char	data[ 65535];

	printf( "input data ... ");
	printf( "[\"Enter\" key = 입력종료]\n");

	while( 1)
	{
		ptr = fgets( rec, 8192, stdin);
		if( rec[ 0] == '\n') break;
		ptr = strchr( rec, '\n');
		*ptr = 0;
		sz = strlen( rec);
		memcpy( &data[ pos], rec, sz +1);
		pos += sz;
	}

	PacketPrint( data);

	return 1;
}

/*********************************************************************************************************
 *
*********************************************************************************************************/
TaskLine( int argc, char *argv[])
{
	int		pos = 0, sz;
	char	*ptr;
	char	rec[ 8192];

	printf( "input data ... ");
	printf( "[\"Enter\" key = 입력종료]\n");

	ptr = fgets( rec, 8192, stdin);
	ptr = strchr( rec, '\n');
	*ptr = 0;

	PacketPrint( rec);

	return 1;
}

/*********************************************************************************************************
 *
*********************************************************************************************************/
CmdHelp( int argc, char *argv[])
{
	Cmd_IntHelp( Cmd, argc, argv);

	return 1;
}

CmdQuit( int argc, char *argv[])
{
	LogDel( "quit.............");

	exit( 1);
	return 1;
}

