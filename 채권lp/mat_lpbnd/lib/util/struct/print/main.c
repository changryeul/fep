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

CFG				*Cfg;
CMD				*Cmd;
extern CMD_TBL	CmdTable[];

char	CfgFilePath[ 512] = ".";
char	CfgFileName[ 512] = "packet.cfg";


PARAM	Param = 
{
	"./pp.cfg"									/* cfg_file_name				*/
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

	Cfg = Cfg_Open( Param.cfg_file_name);
	if( Cfg == NULL)
	{
		LogCri( "config file open error. name=[%s]", Param.cfg_file_name);
		goto error_1;
	}

	Cmd = Cmd_Open( CmdTable);
	if( Cmd == NULL)
	{
		LogCri( "Cmd_Open error. CmdTable=[%p]", CmdTable);
		goto error_2;
	}
	Cmd_SetPrompt( Cmd, "[PrintPacket] ");

	ParamPrint( &Param);
	return 1;

	error_2:
		Cfg_Close( Cfg);
	error_1:
		return -1;
}

MainProcess( int argc, char *argv[])
{
	int		rtn = 0;

	while( rtn >= 0)
	{
		rtn = Cmd_Main( Cmd);
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
	LogDbg( "cfg_file_name            = [%s] ", param->cfg_file_name);
	return 1;
}

