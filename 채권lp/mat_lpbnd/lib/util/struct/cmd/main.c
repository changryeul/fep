#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "cfg.h"
#include "cmd.h"
#include "task.h"

CFG		*Cfg;
CMD		*Cmd;
extern CMD_TBL	CmdTable[];


PARAM	Param = 
{
	NULL,									/* file_list				*/
	0										/* file_cnt					*/
};


char	*OptStr = "ho:";
char	Usage[] = 
"usage: %s -h -o out_file_name [header_file ... ]\n"
"\to: output file name (default:stdout);\n"
"\th: help message\n";

main( int argc, char *argv[])
{
	int		i;
	int		rtn;

	rtn = GetOption( argc, argv);
	if( rtn <= 0)
	{
		printf( Usage, argv[ 0]);
		return -1;
	}

	rtn = InitProcess( argc, argv);
	if( rtn < 0)
	{
		LogCri( "InitProcess error. rtn=[%d]", rtn);
		return rtn;
	}

	rtn = MainProcess( argc, argc);
	if( rtn < 0)
	{
		LogCri( "MainProcess error. rtn=[%d]", rtn);
		return rtn;
	}

	rtn = TermProcess( argc, argc);
	if( rtn < 0)
	{
		LogCri( "MainProcess error. rtn=[%d]", rtn);
		return rtn;
	}

	return 1;
}

GetOption( int argc, char *argv[])
{
	int		opt;
	char	*p;

	while( 1)
	{
		opt = getopt( argc, argv, OptStr);
		if( opt < 0) break;

		switch( opt)
		{
			case 'h' :
				return 0;
			default  :
				break;
		}
	}
	return 1;
}

InitProcess( int argc, char *argv[])
{
	int		rtn;

	Cmd = Cmd_Open( CmdTable);
	if( Cmd == NULL)
	{
		LogCri( "Cmd_Open error. CmdTable=[%p]", CmdTable);
		goto error_1;
	}
	Cmd_SetPrompt( Cmd, "[struct] ");

	ParamPrint( &Param);
	return 1;

	error_1:
		return -1;
}

MainProcess( int argc, char *argv[])
{
	int		rtn;
	int		sz, tot_sz = 0;;
	char	rec[ 65535];

	while( 1)
	{
		rtn = Cmd_Main( Cmd);
		if( rtn < 0)
		{
			break;
		}
	}
	return 1;
}

TermProcess( int argc, char *argv[])
{
	return 1;
}

/*********************************************************************************************************
*
*********************************************************************************************************/
ParamPrint( PARAM *param)
{
	return 1;
}

