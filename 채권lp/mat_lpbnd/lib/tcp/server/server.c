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
	TCP		*client[ MAX_CLIENT], *tmp_cli = NULL;

	int				sock;
	int				nfds = 0;
	fd_set			rfds;
	struct timeval	tv;
	struct sockaddr	client_addr;

	LogDel( "ServerProc start.");

	for( i = 0; i < MAX_CLIENT; i++) client[ i] == NULL;

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

		for( i = 0; i < MAX_CLIENT; i++)
		{
			if( client[ i] == NULL) 	continue;
			FD_SET( TCP_SOCK( client[ i]), &rfds);
			nfds = Max( nfds, TCP_SOCK( client[ i]));
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
			LogDel( "accept ...");
			tmp_cli = Tcp_Accept( server);
			LogDel( "accept tmp_cli=[%p] ", tmp_cli);

			for( i = 0; i < MAX_CLIENT; i++) if( client[ i] == NULL) break;
			if( i >= MAX_CLIENT)
			{
				LogCri( "too many connections!!! MAX=[%d] connection=[%d]", MAX_CLIENT, i); 
				Tcp_Close( tmp_cli);
				continue;
			}
			client[ i] = tmp_cli;
			tmp_cli = NULL;
		}
		for( i = 0; i < MAX_CLIENT; i++)
		{
			if( FD_ISSET( TCP_SOCK( client[ i]), &rfds))
			{
				rtn = DataProcess( client[ i], rec, 65535);
				if( rtn <= 0)
				{
					Tcp_Close( client[ i]);
					client[ i] == NULL;
				}
			}
		}
		
	}

	return 1;

	error_1:
		for( i = 0; i < MAX_CLIENT; i++) if( client[ i] != NULL) Tcp_Close( client[ i]);
		return -1;
}

DataProcess( TCP *client, char *rec, int sz)
{
	int		rtn;

	rtn = Tcp_Recv( client, rec, sz);
	if( rtn < 0)
	{
		return -1;
	}
	rec[ rtn] = 0;

	LogRaw( "%s\n", rec);

	return rtn;
}

DataSendRecv( TCP *svr, char *data, int sz)
{
}





