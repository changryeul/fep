#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "task.h"

CFG		*Cfg;
char	Prompt[ 512];
CLIENT	_Client;
CLIENT	*Client = &_Client;

int		Task = 0;
char	CfgFilePath[ 512] = ".";
char	CfgFileName[ 512] = "packet.cfg";


PARAM	ParamBuf = 
{
	"./main.cfg",	/* cfg_name					*/
	"./main.log",	/* log_name					*/
	"./batch.cmd",	/* bat_name					*/
	"127.0.0.1",	/* addr */
	10001			/* port */
};
PARAM	*Param = &ParamBuf;


char	*OptStr = "ho:b:";
char	Usage[] = 
"usage: %s -h -o out_file_name [header_file ... ]\n"
"\to: output file name (default:stdout);\n"
"\tb: batch process (default:cmd.bat);\n"
"\th: help message\n";

int main( int argc, char *argv[])
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

	switch( Task)
	{
		case 2:		/* daemon process */
			break;
		case 1:		/* batch process */
			BatchProcess( argc, argv);
			break;
		case 0:		/* command process */
		default:
			rtn = CommandProcess( argc, argv);
			if( rtn < 0)
			{
				LogCri( "MainProcess error. rtn=[%d]", rtn);
				return rtn;
			}
			break;
	}

	rtn = TermProcess( argc, argv);
	if( rtn < 0)
	{
		LogCri( "MainProcess error. rtn=[%d]", rtn);
		return rtn;
	}

	return 1;
}

int GetOption( int argc, char *argv[])
{
	int		opt;
	char	*p;

	while( 1)
	{
		opt = getopt( argc, argv, OptStr);
		if( opt < 0) break;

		switch( opt)
		{
			case -1  :
				return 1;
			case 'o' :
				memcpy( Param->log_name, optarg, strlen( optarg) +1);
				printf( "log file name = [%s]\n", Param->log_name);
				break;
			case 'c' :
				memcpy( Param->cfg_name, optarg, strlen( optarg) +1);
				printf( "config file name = [%s]\n", Param->cfg_name);
				break;
			case 'b' :
				memcpy( Param->bat_name, optarg, strlen( optarg) +1);
				Task = 1;
				break;
			case 'h' :
				return 0;
			default  :
				break;
		}
	}
	argc -=optind;
	argv = &argv[ optind];

	return 1;
}

int InitProcess( int argc, char *argv[])
{
	int		rtn;

	Cfg = Cfg_Open( Param->cfg_name);
	if( Cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", Param->cfg_name);
		return -1;
	}

	Cfg_Get( Cfg, "addr", Param->addr, 32);
	Param->port = Cfg_GetInt( Cfg, "port");

	Cmd = Cmd_Open( CmdTable);
	sprintf( Prompt, "[%s:%d]", Param->addr, Param->port);
	Cmd_SetPrompt( Cmd, Prompt);
	ParamPrint( Param);

	Client->udp = Udp_OpenClient( Param->addr, Param->port);
	if( Client->udp == NULL)
	{
		LogCri( "Udp_OpenClient error. addr=[%s] port=[%d]", Param->addr, Param->port);
		return -1;
	}

	return 1;
}

int CommandProcess( int argc, char *argv[])
{
	int		rtn = 1;

	while( rtn >= 0)
	{
		rtn = Cmd_Main( Cmd);
	}

	return 1;
}

int BatchProcess( int argc, char *argv[])
{
	int		rtn;
	FILE	*fp;
	char	*ptr, rec[ 512];

	fp = fopen( Param->bat_name, "r");
	if( fp == NULL)
	{
		LogErr( "fopen error. name=[%s]", Param->bat_name);
		return -1;
	}

	while( 1)
	{
		ptr = fgets( rec, 512, fp);
		if( ptr == NULL) break;
		if( rec[ 0] == '#') continue;
		rtn = Cmd_RunCommand( Cmd, rec);
		if( rtn < 0) break;
	}

	fclose( fp);

	return 1;
}

int TermProcess( int argc, char *argv[])
{
	return 1;
}

/*********************************************************************************************************
*
*********************************************************************************************************/
int ParamPrint( PARAM *param)
{
	int		cnt = 0;
	char	**ptr;

	LogMsg( "cfg_name          = [%s]", param->cfg_name);
	LogMsg( "bat_name          = [%s]", param->bat_name);
	LogMsg( "addr              = [%s]", param->addr);
	LogMsg( "port              = [%d]", param->port);

	return 1;
}

