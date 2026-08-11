#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "log.h"
#include "tcp.h"
#include "web.h"

#define	STDIO	0

extern WEB	*Web;

WEB *Web_Open( char *addr, int port)
{
	int		sz;
	WEB		*web;

	web = malloc( sizeof( WEB));
	if( web == NULL)
	{
		LogErr( "Web malloc error. sz=[%d]", sizeof( WEB));
		goto error_1;
	}
	memset( web, 0x00, sizeof( WEB));


	if( addr != NULL)
	{
		sz = strlen( addr) +1;
		web->addr = malloc( sz);
		if( web->addr == NULL)
		{
			LogErr( "Web->addr malloc error. sz=[%d]", sz);
			goto error_2;
		}
		memcpy( web->addr, addr, sz); 
	}
	else web->addr = NULL;
	web->port = port;

	web->sock = Tcp_OpenServer( web->addr, web->port);
	if( web->sock < 0)
	{
		LogCri( "Tcp_OpenServer error.");
		goto error_3;
	}

	return web;

	error_3:
		free( web->addr);
	error_2:
		free( web);
	error_1:
		return NULL;
}

Web_Server( WEB *web)
{
	int				rtn;

	int				fds, sock;
	fd_set			rfds;
	struct timeval	tv;

	while( 1)
	{
		Web_WaitChild( web);

		tv.tv_sec = 1;
		tv.tv_usec = 0;
		FD_ZERO( &rfds);
		FD_SET( web->sock, &rfds);
		fds = web->sock +1;
		LogDel( "select wait. timeout=[%d.%06d]", tv.tv_sec, tv.tv_usec);
		rtn = select( fds, &rfds, NULL, NULL, &tv);
		if( rtn < 0)
		{
			LogMsg( "select error. fds=[%d], rfds=[%p]", fds, &rfds);
			return -1;
		}
		else
		if( rtn == 0)
		{
			LogDel( "select timeout.");
			continue;
		}

		if( !FD_ISSET( web->sock, &rfds)) continue;

		rtn = listen( web->sock, MAX_BACKLOG);
		if( rtn < 0)
		{
			LogErr( "listen error. rtn=[%d]", rtn);
			return -1;
		}

		sock = Tcp_Accept( web->sock);
		if( sock < 0)
		{
			LogErr( "Tcp_Accept error. sock=[%d]", web->sock);
			return -1;
		}
		LogDbg( "accept. new sock=[%d]", sock);

		rtn = Web_Service( web, sock);
	}

	// rtn = Tcp_Server( web->sock, Web_Service, Web_Timeout);

	return 1;
}

Web_WaitChild( WEB *web)
{
	int				rtn, cnt = 10;
	pid_t			pid;
	int				stat;

	while( cnt)
	{
		pid = wait3( ( int *)&stat, WNOHANG, ( struct rusage *)0);
		if( pid < 0) return -1;
		if( pid > 0)
		{
			LogMsg( "Stop process. pid=[%d]", pid);
			continue;
		}
		cnt--;
	}

	return 1;
}

Web_Service( WEB *web, int sock)
{
	int		rtn, s_sz;
	char	*head = "HTTP/1.1 200 OK!\r\n"
					"Server: KIS Server\r\n"
					"Content-Type: text/html\r\n"
					"Date: The, 20 Feb 2020 09:00:00 GMT\r\n"
					"Connection: close\r\n";
	pid_t	pid;

	/*
	s_sz = strlen( head);
	printf( "send %d bytes.\n", s_sz);
	Tcp_SendN( sock, head, s_sz);
	*/

	pid = fork();
	if( pid < 0)
	{
		LogErr( "fork error. rtn=[%d]", pid);
		return -1;
	}
	else if( pid > 0)
	{
		/* close socket */
		close( sock);
		return 0;
	}

#if STDIO
	close( 1);
	close( 0);
	dup2( sock, 1);
	dup2( sock, 0);
#endif

	LogDbg( "--------------------- service started. pid=[%d] --------------------------", getpid());

	web->client = sock;
	rtn = Web_GetRequest( web);
	if( rtn > 0)
	{
		Web_ServiceHtml( web, web->path, web->path_sz);
	}

	close( sock);
	LogDbg( "--------------------- service end.     pid=[%d] ---------------------------", getpid());
	exit( 1);
}

Web_GetRequest( WEB *web)
{
	int		rtn;
	char	*ptr;
	char	rec[ WEB_MAX_BUFF];

#if STDIO
	rtn = read( 0, web->recv, WEB_MAX_BUFF);
#else
	rtn = Tcp_RecvT( web->client, web->recv, WEB_MAX_BUFF, 3000000);
	if( rtn < 0) return -1;
	if( rtn == 0) 
	{
		LogDbg( "recv timeout. sock=[%d]", web->client);
		return -1;
	}
#endif
	LogDbg( "recv %d byte(s).", rtn);
	LogDbg( "%s", web->recv);

	ptr = strstr( web->recv, "GET");
	if( ptr == NULL)
	{
		Web_PutMessage( web, 400);
		return -1;
	}

	ptr += 4;
	while( *ptr != ' ')
	{
		web->path[ web->path_sz++] = *ptr;
		ptr++;
	}
	web->path[ web->path_sz] = 0;

	if( web->path_sz <= 0)
	{
		Web_PutMessage( web, 400);
		return -1;
	}

	return rtn;
}

Web_PutMessage( WEB *web, int msg_no)
{
	char	*head = "HTTP/1.1 200 OK!\r\n"
					"Server: KIS Server\r\n"
					"Content-Type: text/html\r\n"
					"Date: The, 20 Feb 2020 09:00:00 GMT\r\n"
					"Connection: close\r\n";
	char	snd_buf[ 8192];
	int		snd_sz;

	switch( msg_no)
	{
		case 200:
			snd_sz = sprintf( snd_buf, "HTTP/1.1 %d OK!\r\n", msg_no);
			break;
		default:
			snd_sz = sprintf( snd_buf, "HTTP/1.1 %d NOT FOUND!\r\n", msg_no);
			break;
	}
#if STDIO
	write( 1, snd_buf, snd_sz);
#else
	send( web->client, snd_buf, snd_sz, 0);
#endif
	LogDbg( "send message [%d:%s]", snd_sz, snd_buf);

	return snd_sz;
}

Web_ServiceHtml( WEB *web, char *name, int sz)
{
	int		rtn, s_sz, t_sz;
	FILE	*fp;
	char	f_name[ 512], *ptr;
	char	rec[ 512];

	int		i;

	LogDbg( "Html service start. name=[%s] sz=[%d]", name, sz);
	if( !memcmp( name, "/", 2))
	{
		strcpy( f_name, "./home/index.html");
	}
	else
	{
		/*
		for( i = 0; i < sz; i++) LogDbg( "name[ %d] = %02x %c", i, name[i], name[ i]);
		*/
		ptr = strchr( name, '?');
		if( ptr != NULL)
		{
			LogDbg( "more information...");
			ptr = 0;
			sprintf( f_name, "./home/%s", name);
#if STDIO
			ptr = fgets( rec, 512, stdin);
			LogDbg( "fgets ...");
			LogDbg( "%s", rec);
#else
			rtn = Tcp_RecvNT( web->client, web->recv, WEB_MAX_BUFF, 1000000);
			LogDbg( "recv %d byte(s).", rtn);
			LogDbg( "%s", web->recv);
#endif
		}
		else sprintf( f_name, "./home/%s", name);
	}

	LogDbg( "file open. name=[%s]", f_name);
	fp = fopen( f_name, "r");
	if( fp == NULL)
	{
		LogDbg( "file open error. name=[%s] error=[%d:%s]\n", f_name, errno, strerror( errno));
		Web_PutMessage( web, 404);
		return -1;
	}

	while( 1)
	{
		ptr = fgets( rec, 512, fp);
		if( ptr == NULL) break;
		s_sz = strlen( rec);
#if STDIO
		write( 1, rec, s_sz);
#else
		send( web->client, rec, s_sz, 0);
#endif
		t_sz += s_sz;
	}
	LogDbg( "send html %d byte(s).", t_sz);

	fclose( fp);
	LogDbg( "service end. name=[%s] sz=[%d]", name, sz);

#if STDIO
	rtn = read( 0, web->recv, WEB_MAX_BUFF);
#else
	rtn = Tcp_RecvNT( web->client, web->recv, WEB_MAX_BUFF, 1000);
#endif
	LogDbg( "recv %d byte(s).", rtn);
	LogDbg( "%s", web->recv);



	return 1;
}

