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

PARAM	Param = 
{
	"./frame.cfg",								/* cfg_file_name					*/
	0											/* file_cnt							*/
};

int MainProcess( int argc, char *argv[]);

char	*OptStr = "ho:";
char	Usage[] = 
"usage: %s -h -o out_file_name [header_file ... ]\n"
"\to: output file name (default:stdout);\n"
"\th: help message\n";

main( int argc, char *argv[])
{
	int		i;
	int		rtn;

#if 0
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
#endif

	rtn = MainProcess( argc, argv);
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
			case -1  :
				return 1;
			case 'c' :
				memcpy( Param.cfg_name, optarg, strlen( optarg) +1);
				printf( "config file name = [%s]\n", Param.cfg_name);
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

InitProcess( int argc, char *argv[])
{
	int		rtn;

	rtn = GetConfig( Param.cfg_name);
	if( rtn < 0) return rtn;

	Cmd = Cmd_Open( CmdTable);
	Cmd_SetPrompt( Cmd, "[cmd] ");
	ParamPrint( &Param);
	return 1;
}

typedef struct _lat_
{
    int                 id;
    struct timeval      order_rtt;
    struct timeval      order_rec1;
    struct timeval      order_msg;
    struct timeval      order_rec2;
    struct timeval      order_rat_ins;
    struct timeval      order_rat_acc;
}   JBRS_LATENCY;

MainProcess( int argc, char *argv[])
{
	int		rtn;
    FILE    *fep_fp, sb_fp;
    char    rec[ 8192];
    char    *ptr;

    JBRS_LATENCY    lat[ 50000];

    LogDbg( "jbrs.............");

    fep_fp = fopen( "./sb_20191224.log", "r");
    if( fep_fp == NULL) return -1;

    while( 1)
    {
        ptr = fgets( rec, 8192, fep_fp);
        if( ptr == NULL) break;
        if( strstr( rec, "ORDER_RTT")) 				Get_OrderRtt( lat, rec);
        else if( strstr( rec, "ORDER_RTT")) 		Get_OrderRec1( lat, rec);
    }

    fclose( fep_fp);

    return 1;
}

Get_OrderRtt( JBRS_LATENCY *lat, char *rec)
{
	char	*ptr;
	unsigned int		id;

	ptr = strstr( rec, "29800");

	id = atoi( ptr +3);
	lat[ id].id = id;
	GetTime( &lat[ id].order_rtt, rec);
    printf( "%d %d.%d %s", id, lat[ id].order_rtt.tv_sec, lat[ id].order_rtt.tv_usec, rec);
	sleep( 1);
}

Get_OrderRec1( JBRS_LATENCY *lat, char *rec)
{
	char	*ptr;
	unsigned int		id;

	ptr = strstr( rec, "29800");

	id = atoi( ptr +3);
	lat[ id].id = id;
	GetTime( &lat[ id].order_rtt, rec);
    printf( "%d %d.%d %s", id, lat[ id].order_rtt.tv_sec, lat[ id].order_rtt.tv_usec, rec);
	sleep( 1);
}

GetTime( struct timeval *tv, char *rec)
{
	time_t		cur_time;
	struct tm	*tp;

	time( &cur_time);
	tp = localtime( &cur_time);

	tp->tm_hour = atoi( &rec[ 0]);
	tp->tm_min =  atoi( &rec[ 3]);
	tp->tm_sec =  atoi( &rec[ 6]);
	tv->tv_sec = mktime( tp);
	tv->tv_usec = atoi( &rec[ 9]);

	return 1;
}

TermProcess( int argc, char *argv[])
{
	return 1;
}

/*********************************************************************************************************
*
*********************************************************************************************************/
GetConfig( char *cfg_name)
{
	Cfg = Cfg_Open( cfg_name);
	if( Cfg == NULL)
	{
		LogCri( "config open error. name=[%s]", cfg_name);
		return -1;
	}

}

ParamPrint( PARAM *param)
{
	int		cnt = 0;
	char	**ptr;

	LogMsg( "cfg_file_name          = [%s]", param->cfg_name);

	return 1;
}

