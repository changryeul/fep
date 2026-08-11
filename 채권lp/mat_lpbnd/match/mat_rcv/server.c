#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "cfg.h"
#include "etc.h"
#include "tcp.h"

#define MAX_CLIENT		1

/***** Module : server.c *****/
int         ServerProc( TCP *server);

int ServerProcess( TCP *server)
{
	int		rtn, i;
	int		sz, tot_sz = 0;;
	char	rec[ 65535];
	TCP		*client = NULL, *tmp_cli = NULL;

	int				sock;
	int				nfds = 0;
	fd_set			rfds;
	struct timeval	tv;
	struct sockaddr	client_addr;

	LogDel( "ServerProc start.");

	LogDel( "listen ...");
	rtn = Tcp_Listen( server);
	if( rtn < 0)
	{
		LogCri( "Tcp_Listen error. rtn=[%d]", rtn);
		return -1;
	}

	while( 1)
	{
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		nfds = 0;
		FD_ZERO( &rfds);
		FD_SET( TCP_SOCK( server), &rfds);
		nfds = TCP_SOCK( server);

		if( client != NULL)
		{
			FD_SET( TCP_SOCK( client), &rfds);
			nfds = Max( nfds, TCP_SOCK( client));
		}

		LogDel( "select ... wait=[%d.%dsec]", tv.tv_sec, tv.tv_usec);
		rtn = select( nfds +1, &rfds, NULL, NULL, &tv);
		if( rtn < 0)
		{
			LogErr( "select error. rtn=[%d]", rtn);
			goto error_1;
		}
		else
		if( rtn == 0)
		{
			LogDel( "timeout ... ");
			continue;
		}

		LogDel( "socket event ... ");
		if( FD_ISSET( TCP_SOCK( server), &rfds))
		{
			LogDel( "Server sock event ...");
			if( client != NULL)
			{
				LogCri( "too many connections!!! clinet=[%p]", client); 
				Tcp_Close( client);
				client = NULL;
			}

			LogDel( "accept ...");
			tmp_cli = Tcp_Accept( server);
			LogDel( "accept tmp_cli=[%p] ", tmp_cli);

			client = tmp_cli;
		}
		if( FD_ISSET( TCP_SOCK( client), &rfds))
		{
			LogDel( "get event ... sock=[%d]", TCP_SOCK( client));
			rtn = DataProcess( client, rec, 65535);
			if( rtn <= 0)
			{
				LogDel( "DataProcess error. close client=[%p]", client);
				Tcp_Close( client);
				client = NULL;
			}
		}
		
	}

	return 1;

	error_1:
		Tcp_Close( client);
		return -1;
}

DataProcess( TCP *client, char *rec, int sz)
{
	int		rtn;

	rtn = Tcp_Recv( client, rec, sz);
	LogDump( rec, sz, "receive data. data=[%04d:%.80s]", sz, rec);

	return rtn;
}


