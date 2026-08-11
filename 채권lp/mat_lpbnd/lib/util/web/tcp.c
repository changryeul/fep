#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "log.h"
#include "tcp.h"


Tcp_OpenServer( char *addr, int port)
{
	int						rtn;
	int						sock;
	struct sockaddr_in		svr_addr;

	sock = socket( PF_INET, SOCK_STREAM, 0);
	if( sock < 0)
	{
		LogErr( "socket open error.");
		goto error_1;
	}
	LogDel( "socket open. sock=[%d]", sock);

	if( 1 /*addr != NULL*/)
	{
		svr_addr.sin_family = AF_INET;
		if( addr == NULL)		svr_addr.sin_addr.s_addr = 0;
		else					svr_addr.sin_addr.s_addr = inet_addr( addr);
		svr_addr.sin_port = htons( port);
		rtn = bind( sock, ( struct sockaddr *)&svr_addr, sizeof( svr_addr));
		if( rtn < 0)
		{
			LogErr( "bind error. addr=[%s] port=[%d]", 
					inet_ntoa( svr_addr.sin_addr), ntohs( svr_addr.sin_port));
			goto error_2;
		}
		LogDel( "bind success. addr=[%s] port=[%d]", 
				inet_ntoa( svr_addr.sin_addr), ntohs( svr_addr.sin_port));
	}

	return sock;

	error_x:
	error_2:
		close( sock);
	error_1:
		return -errno;
}

Tcp_Connect( char *addr, int port)
{
	int						rtn;
	int						sock;
	struct sockaddr_in		svr_addr;

	sock = socket( PF_INET, SOCK_STREAM, 0);
	if( sock < 0)
	{
		LogErr( "socket open error.");
		goto error_1;
	}
	LogDel( "socket open. sock=[%d]", sock);

	svr_addr.sin_family = AF_INET;
	svr_addr.sin_addr.s_addr = inet_addr( addr);
	svr_addr.sin_port = htons( port);
	rtn = connect( sock, ( struct sockaddr *)&svr_addr, sizeof( svr_addr));
	if( rtn < 0)
	{
		LogErr( "connect error. addr=[%s] port=[%d]", 
				inet_ntoa( svr_addr.sin_addr), ntohs( svr_addr.sin_port));
		goto error_2;
	}
	LogDel( "connect success. addr=[%s] port=[%d]", 
			inet_ntoa( svr_addr.sin_addr), ntohs( svr_addr.sin_port));

	return sock;

	error_x:
	error_2:
		close( sock);
	error_1:
		return -errno;
}

Tcp_Listen( int sock, int backlog)
{
	return listen( sock, backlog);
}

Tcp_Server( int sock,  int (*recv_func)( int sock, char *rec, int sz))
{
	int				rtn, addr_len;
	int				i, fds;
	fd_set			rfds;
	struct timeval	tv;
	int				peer_tbl[ MAX_CLIENT];
	int				peer_cnt = 0;
	char			rec[ MAX_BUF_SZ];

	LogDel( "listen. sock=[%d]", sock);

	memset( peer_tbl, 0xFF, sizeof( int) * MAX_CLIENT);

	rtn = listen( sock, MAX_BACKLOG);
	if( rtn < 0)
	{
		LogErr( "listen error. sock=[%d]", sock);
		return -1;
	}

	while( 1)
	{
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		FD_ZERO( &rfds);
		FD_SET( sock, &rfds);
		fds = TCP_MAX( fds, sock);
		for( i = 0; i < MAX_CLIENT; i++)
		{
			if( peer_tbl[ i] > 0)
			{
				FD_SET( peer_tbl[ i], &rfds);
				fds = TCP_MAX( fds, peer_tbl[ i]);
				LogDel( "add select. fds=[%d] sock=[%d]", fds, peer_tbl[ i]);
			}
		}

		LogDel( "select wait...");
		rtn = select( fds +1, &rfds, NULL, NULL, &tv);
		LogDel( "select rtn=[%d]", rtn);
		if( rtn == 0)
		{
			LogDel( "select timeout.");
		}
		if( FD_ISSET( sock, &rfds))
		{
			for( i = 0; i < MAX_CLIENT; i++) if( peer_tbl[ i] < 0) break;
			peer_tbl[ i] = Tcp_Accept( sock);
		}
		for( i = 0; i < MAX_CLIENT; i++)
		{
			if( peer_tbl[ i] >= 0)
			{
				if( FD_ISSET( peer_tbl[ i], &rfds))
				{
					if( recv_func == NULL)
					{
						rtn = recv( peer_tbl[ i], rec, MAX_BUF_SZ, 0);
						if( rtn <= 0)
						{
							LogDel( "close socket. sock=[%d]", peer_tbl[ i]);
							Tcp_Close( peer_tbl[ i]);
							peer_tbl[ i] = -1;
							continue;
						}
						rec[ rtn] = 0;
						LogDel( "recv. data=[%d:%s]", rtn, rec);
					}
					else
					{
						rtn = recv_func( peer_tbl[ i], rec, MAX_BUF_SZ);
						if( rtn <= 0)
						{
							LogDel( "close socket. sock=[%d]", peer_tbl[ i]);
							Tcp_Close( peer_tbl[ i]);
							peer_tbl[ i] = -1;
							continue;
						}
						rec[ rtn] = 0;
						LogDel( "recv. data=[%d:%s]", rtn, rec);
					}
				}
			}
		}
	}
}

int	Tcp_Close( int sock)
{
	if( sock >= 0) close( sock);

	return 1;
}

TCP *Tcp_MakeSock( TCP *sv_tcp)
{
	int		rtn;
	TCP		*tcp;

	tcp = malloc( sizeof( TCP));
	if( tcp == NULL)
	{
		LogErr( "malloc error. size=[%d]", sizeof( TCP));
		return NULL;
	}
	LogDel( "malloc. tcp=[%p]", tcp);
	if( sv_tcp != NULL)	memcpy( tcp, sv_tcp, sizeof( TCP));
	else				memset( tcp, 0, sizeof( TCP));

	return tcp;
}

Tcp_Accept( int sv_sock)
{
	int					rtn, addr_len;
	struct sockaddr_in	cli_addr;
	int					sock;

	addr_len = sizeof( cli_addr);
	sock = accept( sv_sock, ( struct sockaddr *)&cli_addr, &addr_len);
	if( sock < 0)
	{
		LogErr( "accept error. sock=[%d]", sock);
		goto error_1;
	}
	LogDel( "accepted. sock=[%d] addr=[%s] port=[%d]", 
			sock, inet_ntoa( cli_addr.sin_addr), ntohs( cli_addr.sin_port));

	return sock;

	error_x:
	error_1:
		return -errno;
}

Tcp_SendN( int sock, char *rec, int sz)
{
	int 	rtn, ssz = 0;

	while( ssz < sz)
	{
		rtn = send( sock, &rec[ ssz], sz - ssz, 0);
		if( rtn < 0)
		{
			LogErr( "send error.");
			goto error_1;
		}
		LogDel( "send %d %d/%d [%d:%.*s]", rtn, ssz +rtn, sz, rtn, rtn, &rec[ ssz]);
		// Log_Hex( LogPtr, &rec[ ssz], rtn);
		ssz += rtn;
	}

	return ssz;

	error_1:
		return -errno;
}

Tcp_SendNT( int sock, char *rec, int sz, int to)
{
	int 			rtn, ssz = 0;
	int				i, fds;
	fd_set			wfds;
	struct timeval	tv;

	while( ssz < sz)
	{
		tv.tv_sec = to / 1000000;
		tv.tv_usec = to % 1000000;

		FD_ZERO( &wfds);
		FD_SET( sock, &wfds);

		rtn = select( sock +1, NULL, &wfds, NULL, &tv);
		if( rtn < 0)
		{
			LogErr( "select error. sock=[%d] rtn=[%d]", sock, rtn);
		}
		else if( rtn == 0)
		{
			LogDel( "send timeout. sock=[%d] sec=[%d] usec=[%d]", sock, tv.tv_sec, tv.tv_usec);
			continue;
		}

		if( FD_ISSET( sock, &wfds))
		{
			rtn = send( sock, &rec[ ssz], sz - ssz, 0);
			if( rtn < 0)
			{
				LogErr( "send error.");
				goto error_1;
			}
			LogDel( "send %d %d/%d [%d:%.*s]", rtn, ssz +rtn, sz, rtn, rtn, &rec[ ssz]);
			// Log_Hex( LogPtr, &rec[ ssz], rtn);
			ssz += rtn;
		}
	}

	return ssz;

	error_1:
		return -errno;
}

Tcp_RecvN( int sock, char *rec, int sz)
{
	int 	rtn, rsz = 0;

	while( rsz < sz)
	{
		rtn = recv( sock, &rec[ rsz], sz - rsz, 0);
		if( rtn < 0)
		{
			LogErr( "recv error.");
			goto error_1;
		}
		LogDel( "recv %d byte(s). %d/%d [%d:%.*s]", rtn, rsz +rtn, sz, rtn, rtn, &rec[ rsz]);
		// Log_Hex( LogPtr, &rec[ rsz], rtn);
		rsz += rtn;
	}

	return rsz;

	error_1:
		return -errno;
}

Tcp_RecvNT( int sock, char *rec, int sz, int to)
{
	int 			rtn, rsz = 0;
	int				i, fds;
	fd_set			rfds;
	struct timeval	tv;

	while( rsz < sz)
	{
		tv.tv_sec = to / 1000000;
		tv.tv_usec = to % 1000000;

		FD_ZERO( &rfds);
		FD_SET( sock, &rfds);

		rtn = select( sock +1, &rfds, NULL, NULL, &tv);
		if( rtn < 0)
		{
			LogErr( "select error. sock=[%d] rtn=[%d]", sock, rtn);
		}
		else if( rtn == 0)
		{
			LogDel( "recv timeout. sock=[%d] sec=[%d] usec=[%d]", sock, to / 1000000, to % 1000000);
			continue;
		}

		if( FD_ISSET( sock, &rfds))
		{
			rtn = recv( sock, &rec[ rsz], sz - rsz, 0);
			if( rtn < 0)
			{
				LogErr( "recv error.");
				goto error_1;
			}
			if( rtn == 0)
			{
				LogDel( "recv 0 byte.");
				return rsz;
			}
			LogDel( "recv %d byte(s). %d/%d ", rtn, rsz +rtn, sz);
			// Log_Hex( LogPtr, &rec[ rsz], rtn);
			rsz += rtn;
		}
	}

	return rsz;

	error_1:
		return -errno;
}



