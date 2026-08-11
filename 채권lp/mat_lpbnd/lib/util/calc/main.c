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
#include "calc.h"
#include "task.h"

CFG		*Cfg;

char	CfgFilePath[ 512] = ".";
char	CfgFileName[ 512] = "packet.cfg";


PARAM	Param = 
{
	".",										/* cfg_file_path					*/
	"cmd.cfg",									/* cfg_file_name					*/
	0											/* file_cnt							*/
};


char	*OptStr = "ho:";
char	Usage[] = 
"usage: %s -h -o out_file_name [header_file ... ]\n"
"\to: output file name (default:stdout);\n"
"\th: help message\n";

int main( int argc, char *argv[])
{
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

	rtn = MainProcess( argc, argv);
	if( rtn < 0)
	{
		LogCri( "MainProcess error. rtn=[%d]", rtn);
		return rtn;
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

	while( 1)
	{
		opt = getopt( argc, argv, OptStr);
		if( opt < 0) break;

		switch( opt)
		{
			case -1  :
				return 1;
			case 'c' :
				memcpy( Param.cfg_file_name, optarg, strlen( optarg) +1);
				printf( "config file name = [%s]\n", Param.cfg_file_name);
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
	int		rtn = 1;

	/*
	Cmd = Cmd_Open( CmdTable);
	Cmd_SetPrompt( Cmd, "[cmd] ");
	ParamPrint( &Param);
	*/
	return rtn;
}

int MainProcess( int argc, char *argv[])
{
	int		rtn;
	char	rec[ 65535], *ptr;
	CALC	*cal;

	cal = Calc_Open();

	while( 1)
	{
		ptr = fgets( rec, 512, stdin);
		if( ptr == NULL) break;
		if( strlen( rec) <= 1) break;
		rtn = Calc_Process( cal, rec);
		if( rtn <= 0) break;
	}
	Calc_Close( cal);
	/*
	Cmd_Main( Cmd);
	*/

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
	LogMsg( "cfg_file_path          = [%s]", param->cfg_file_path);
	LogMsg( "cfg_file_name          = [%s]", param->cfg_file_name);

	return 1;
}

