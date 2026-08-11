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
#include "struct.h"
#include "task.h"

CFG		*Cfg;

char	CfgFilePath[ 512] = ".";
char	CfgFileName[ 512] = "packet.cfg";


PARAM	Param = 
{
	"",										/* out_file					*/
	NULL,									/* file_list				*/
	0										/* file_cnt					*/
};


char	*OptStr = "ho:";
char	Usage[] = 
"usage: %s -h -o out_file_name [header_file ... ]\n"
"\to: output file name (default:stdout);\n"
"\th: help message\n";

/***** Module : main.c *****/
int         GetOption( int argc, char *argv[]);
int         InitProcess( int argc, char *argv[]);
int         MainProcess( int argc, char *argv[]);
int         TermProcess( int argc, char *argv[]);
int         ParamPrint( PARAM *param);

int main( int argc, char *argv[])
{
	int		i;
	int		rtn;

	if( argc < 2)
	{
		printf( Usage, argv[ 0]);
		return 1;
	}

	rtn = GetOption( argc, argv);
	if( rtn < 0)
	{
		printf( Usage, argv[ 0]);
		return -1;
	}

	for( i = 0; i < rtn; i++)
	{
		argv++;
		argc--;
	}
	Param.file_list = &argv[ rtn];
	Param.file_cnt = argc;

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
	int		opt, cnt = 0;
	char	*p;

	while( 1)
	{
		opt = getopt( argc, argv, OptStr);
		if( opt < 0) break;

		switch( opt)
		{
			case 'o' :
				memcpy( Param.out_file, optarg, strlen( optarg) +1);
				break;
			case 'h' :
				return 0;
			default  :
				break;
		}
		cnt++;
	}
	return cnt;
}

int InitProcess( int argc, char *argv[])
{
	int		rtn;

	ParamPrint( &Param);
	return 1;
}

int MainProcess( int argc, char *argv[])
{
	int		rtn;
	int		sz, tot_sz = 0;;
	char	rec[ 65535];

	StLoader();
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

	ptr = param->file_list;

	LogDbg( "output file name          = [%s]", *ptr);

	while( 1)
	{
		ptr++;
		if( *ptr == NULL)  break;
		LogDbg( "input file name           = [%s]", *ptr);
	}
	LogDbg( "file_cnt                  = [%d]", Param.file_cnt -1);

	return 1;
}


