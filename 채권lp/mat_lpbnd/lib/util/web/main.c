#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "log.h"
#include "tcp.h"
#include "web.h"

struct _param_
{
	char	cfg_name[ 512];
	char	home[ 512];
}	
Param =
{
	"./web.cfg",
	"./home"
};

WEB		*Web;

char	*ServerAddr	= "0.0.0.0";
int		ServerPort = 10001;
int		ServerSock = -1;

int Recv( int sock, char *rec, int sz);

char	OptStr[] = "h?b:p:";
char	Usage[] = 
"usage: %s -b bind_address -p port_no\n"
"\tb: tcp server bind address(default=0.0.0.0)\n"
"\tp: tcp server port(default=10001)\n";

main( int argc, char *argv[])
{
	int		rtn;

	rtn = GetOption( argc, argv);
	if( rtn < 0)
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

	while( 1)
	{
		rtn = MainProcess( argc, argc);
		if( rtn < 0)
		{
			LogCri( "MainProcess error. rtn=[%d]", rtn);
			return rtn;
		}
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

	while( 1)
	{
		opt = getopt( argc, argv, OptStr);
		if( opt < 0) break;

		switch( opt)
		{
			case 'b' :
				ServerAddr = malloc( strlen( optarg) +1);
				if( ServerAddr == NULL)
				{
					LogErr( "malloc error. size=[%d]", strlen( optarg) +1);
					return -1;
				}
				memcpy( ServerAddr, optarg, strlen( optarg) +1);
				break;
			case 'p' :
				ServerPort = atoi( optarg);
				break;
			case '?' :
			case 'h' :
				return -1;
			default  :
				break;
		}
	}

	LogDel( "ServerAddr    = [%s]", ServerAddr);
	LogDel( "ServerPort    = [%d]", ServerPort);
	return 1;
}

InitProcess( int argc, char *argv[])
{
	int		rtn;
	char	f_name[ 512];
	char	*ptr;
	char	name[ 512], value[ 512];

	Web = Web_Open( NULL, 15000);
	if( Web == NULL)
	{
		LogCri( "Web server open error.");
		goto error_1;
	}

	LogFile( "./web.log");
	LogDbg( "web service started. port=[%d]", 15000);

	return 1;

	error_x:
	error_1:
		return -1;
}

MainProcess( int argc, char *argv[])
{
	int				rtn;
	int				stat, nfds;
	struct timeval	tv;
	fd_set			rfds;
	pid_t			pid;

	Web_Server( Web);

	return 1;
}

TermProcess( int argc, char *argv[])
{
	return 1;
}

