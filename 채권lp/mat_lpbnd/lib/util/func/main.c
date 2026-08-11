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
#include "func.h"
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
"usage: %s [-h] [-o out_header_file_name] [source_file.c ... ]\n"
"\to: output file name (default:stdout);\n"
"\th: help message\n";

/** ***************************************************************************
**  @fu         int main( int argc, char *argv[])
**  @param      int argc	- argument count
**  @param      char argv[]	- argument list
**  @return     성공    	- record position
**  @retval     실패    	- -1
**  @brief
**  main function
***************************************************************************** */
int main( int argc, char *argv[])
{
	int		rtn;

	if( argc < 2)
	{
		printf( Usage, argv[ 0]);
		return 1;
	}

	rtn = GetOption( argc, argv);
	if( rtn <= 0)
	{
		printf( Usage, argv[ 0]);
		return -1;
	}

	Param.file_cnt = argc -optind;
	if( Param.file_cnt <= 0)
	{
		Param.file_cnt = 0;
		Param.file_list = NULL;

	}
	else
	{
		Param.file_list = &argv[ optind];
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

	return 0;
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
			case 'o' :
				memcpy( Param.out_file, optarg, strlen( optarg) +1);
				printf( "out file name = [%s]\n", Param.out_file);
				break;
			case 'h' :
				return 0;
			default  :
				break;
		}
	}

	return 1;
}

int InitProcess( int argc, char *argv[])
{
	ParamPrint( &Param);
	return 1;
}

int MainProcess( int argc, char *argv[])
{
	Func();
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

	LogDel( "output file name          = [%s]\n", param->out_file);

	if( param->file_list == NULL) return 1;

	while( param->file_list[ cnt] != NULL)
	{
		LogDel( "file_name                 = [%s]\n", param->file_list[ cnt]);
		cnt++;
	}

	return 1;
}

