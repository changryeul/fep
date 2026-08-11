/** ***************************************************************************
**  @file       tcp.c
**  @date       2022/08/23
**  @author     최동춘
**  @version    V1.0.20220823
**  @brif
**  TCP/IP 소켓 통신 프로그램
***************************************************************************** */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "log.h"
#include "tcp.h"

/** ***************************************************************************
**  @fn			TCP *Tcp_Open()
**  @param      none
**  @return     TCP 구조체
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**	Tcp_OpenServer, Tcp_OpenClient가 사용할 socket을 open, 구조체 malloc
***************************************************************************** */
TCP *Tcp_Open( char *addr, int port)
{
	TCP		*tcp;

	tcp = malloc( sizeof( TCP));
	if( tcp == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( TCP));
		goto error;
	}
	memset( tcp, 0x00, sizeof( TCP));

	if( addr == NULL)	tcp->addr = NULL;
	else
	{
		tcp->addr = malloc( strlen( addr) +1);
		if( tcp->addr == NULL)
		{
			LogErr( "malloc error. sz=[%d]", strlen( addr) +1);
			goto error_1;
		}
		memcpy( tcp->addr, addr, strlen( addr) +1);
	}
	tcp->port = port;

	tcp->sock = socket( AF_INET, SOCK_STREAM, 0);
	if( tcp->sock < 0)
	{
		LogErr( "socket open error.");
		goto error_1;
	}
	LogLib( "socket open. sock=[%d]", tcp->sock);


	return tcp;

	error_1:
		free( tcp);
	error:
		LogErr( "Tcp_Open error. addr=[%s] port=[%d]", addr, port);
		return NULL;
}

/** ***************************************************************************
**  @fn			int Tcp_Close( TCP *tcp)
**  @param      TCP *tcp
**  @return     1	성공
**  @exception
**  @remark
**  @brief
**	socket close and TCP 구조체 free
***************************************************************************** */
int Tcp_Close( TCP *tcp)
{
	if( tcp->addr != NULL) 
	{
		free( tcp->addr);
		tcp->addr = NULL;
	}
	LogLib( "close socket. tcp->sock=[%d]", tcp->sock);
	close( tcp->sock);
	free( tcp);

	return 1;
}

/** ***************************************************************************
**  @fn			TCP* Tcp_OpenServer( char *addr, int port)
**  @param      char *addr	- server bind address, NULL이면 bind 하지 않는다
**  @param      int port 	- wait port
**  @return     TCP 구조체
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  TCP SERVER 소켓 OPEN
**  addr 주소로 bind
**  이후 반환된 TCP로 listen, accept 한다
***************************************************************************** */
TCP *Tcp_OpenServer( char *addr, int port)
{
	int						rtn, so_flag = 1;
	in_addr_t				in_addr;
	TCP						*tcp;

	tcp = Tcp_Open( addr, port);
	if( tcp == NULL)
	{
		LogCri( "Tcp_Open error.");
		goto error;
	}

	in_addr = inet_addr( addr);
	if( in_addr == -1)
	{
		LogLib( "address convert error. addr=[%s] in_addr=[%d] ... set in_addr=[0]", addr, in_addr);
		in_addr = 0;
	}

	tcp->svr_addr.sin_family = AF_INET;
	tcp->svr_addr.sin_addr.s_addr = in_addr;
	tcp->svr_addr.sin_port = htons( port);
	setsockopt( tcp->sock, SOL_SOCKET, SO_REUSEADDR, ( const char *)&so_flag, sizeof( so_flag));
	rtn = bind( tcp->sock, ( struct sockaddr *)&tcp->svr_addr, sizeof( tcp->svr_addr));
	if( rtn < 0)
	{
		LogErr( "bind error. addr=[%s] port=[%d]", 
				inet_ntoa( tcp->svr_addr.sin_addr), ntohs( tcp->svr_addr.sin_port));
		goto error_1;
	}
	LogLib( "bind success. addr=[%s] port=[%d]", 
			inet_ntoa( tcp->svr_addr.sin_addr), ntohs( tcp->svr_addr.sin_port));

	tcp->backlog = 5;

	return tcp;

	error_1:
		Tcp_Close( tcp);
	error:
		LogErr( "Tcp_OpenServer error. addr=[%s] port=[%d]", addr, port);
		return NULL;
}

/** ***************************************************************************
**  @fn			TCP *Tcp_OpenClient( char *addr, int port)
**  @param      char *addr	- server connect address
**  @param      int port 	- connect port
**  @return     TCP 구조체
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  TCP CLIENT 소켓 OPEN
**  addr 주소로 connect
***************************************************************************** */
TCP *Tcp_OpenClient( char *addr, int port)
{
	int						rtn;
	unsigned int			sz;
	TCP						*tcp;

	tcp = Tcp_Open( addr, port);
	if( tcp == NULL)
	{
		LogCri( "Tcp_Open error.");
		goto error;
	}
	LogMsg( "Tcp_Open get tcp=[%p] sock=[%d]", tcp, tcp->sock);

	tcp->svr_addr.sin_family = AF_INET;
	tcp->svr_addr.sin_addr.s_addr = inet_addr( addr);
	tcp->svr_addr.sin_port = htons( port);
	LogMsg( "connect to sock=[%d] addr=[%s] port=[%d]", tcp->sock, addr, port);
	rtn = connect( tcp->sock, ( struct sockaddr *)&tcp->svr_addr, sizeof( tcp->svr_addr));
	if( rtn < 0)
	{
		LogErr( "connect error. addr=[%s] port=[%d]", 
				inet_ntoa( tcp->svr_addr.sin_addr), ntohs( tcp->svr_addr.sin_port));
		goto error_1;
	}
	LogLib( "connect success. addr=[%s] port=[%d]", 
			inet_ntoa( tcp->svr_addr.sin_addr), ntohs( tcp->svr_addr.sin_port));

	sz = sizeof( struct sockaddr);
	rtn = getsockname( tcp->sock, ( struct sockaddr *)&tcp->cli_addr, &sz);
	if( rtn < 0)
	{
		LogErr( "getsockname error. rtn=[%d]", rtn);
	}
	tcp->backlog = 0;

	return tcp;

	error_1:
		Tcp_Close( tcp);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fn			int Tcp_Listen( int sock, int backlog)
**  @param      TCP * - TCP structure
**  @return     error 여부
**  @retval     listen의 return 값
**  @exception
**  @remark
**  @brief
**  listen 함수 call
***************************************************************************** */
int Tcp_Listen( TCP *tcp)
{
	LogLib( "Listen ... sock=[%d] backlog=[%d]", tcp->sock, tcp->backlog);
	return listen( tcp->sock, tcp->backlog);
}

/** ***************************************************************************
**  @fn			int Tcp_Accept( TCP *tcp)
**  @param      TCP * - TCP structure
**  @return     TCP * - 접속한 client의 TCP structure
**  @exception
**  @remark
**  @brief
**  client의 접속을 허가 ... tcp->cli_addr에 client 정보 setting
***************************************************************************** */
TCP *Tcp_Accept( TCP *tcp)
{
	int				sock;
	unsigned int	addr_len;
	TCP				*new_tcp;

	addr_len = sizeof( tcp->cli_addr);
	sock = accept( tcp->sock, ( struct sockaddr *)&tcp->cli_addr, &addr_len);
	if( sock < 0)
	{
		LogErr( "accept error. sock=[%d]", sock);
		goto error;
	}
	LogLib( "accepted. sock=[%d] addr=[%s] port=[%d]", 
			sock, inet_ntoa( tcp->cli_addr.sin_addr), ntohs( tcp->cli_addr.sin_port));

	new_tcp = malloc( sizeof( TCP));
	if( new_tcp == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( TCP));
		goto error;
	}
	memcpy( new_tcp, tcp, sizeof( TCP));
	new_tcp->addr = malloc( strlen( tcp->addr) +1);
	memcpy( new_tcp->addr, tcp->addr, strlen( tcp->addr) +1);
	new_tcp->sock = sock;

	return new_tcp;;

	error:
		return NULL;
}

/** ***************************************************************************
**  @fn			int Tcp_Accept( TCP *tcp)
**  @param      TCP * - TCP structure
**  @return     TCP * - 접속한 client의 TCP structure
**  @exception
**  @remark
**  @brief
**  debug set --- 0:no_debug 1:debug
***************************************************************************** */
int Tcp_SetDebug( TCP *tcp, int opt)
{
	tcp->debug = opt;

	return opt;
}

/** ***************************************************************************
**  @fn			int Tcp_Accept( TCP *tcp)
**  @param      TCP * - TCP structure
**  @return     TCP * - 접속한 client의 TCP structure
**  @exception
**  @remark
**  @brief
**  client의 접속을 허가 ... tcp->cli_addr에 client 정보 setting
***************************************************************************** */
int Tcp_Info( TCP *tcp)
{
	/*
	rtn = getsockname( tcp->sock, ( struct sockaddr *)&tcp->cli_addr, &sz);
	if( rtn < 0)
	{
		LogErr( "getsockname error. rtn=[%d]", rtn);
	}
	rtn = getpeername( tcp->sock, ( struct sockaddr *)&tcp->svr_addr, &sz);
	if( rtn < 0)
	{
		LogErr( "getsockname error. rtn=[%d]", rtn);
	}
	*/

	LogDbg( "tcp           = [%p]", tcp);
	LogDbg( "socket        = [%d]", tcp->sock);
	LogDbg( "addr          = [%s]", tcp->addr);
	LogDbg( "port          = [%d]", tcp->port);
	LogDbg( "client -------------------------");
	LogDbg( "addr          = [%s]", inet_ntoa( tcp->cli_addr.sin_addr));
	LogDbg( "port          = [%d]", ntohs( tcp->cli_addr.sin_port));
	LogDbg( "server -------------------------");
	LogDbg( "addr          = [%s]", inet_ntoa( tcp->svr_addr.sin_addr));
	LogDbg( "port          = [%d]", ntohs( tcp->svr_addr.sin_port));

	return 1;
}

int Tcp_SetTimeout( TCP *tcp, int to)
{
	LogMsg( "SetTimeout ... tcp->to=[%d]", to);
	tcp->to = to;

	return tcp->to;
}

int Tcp_SetSelectTimeout( TCP *tcp, int to)
{
	LogDbg( "SetSelectTimeout ... tcp->sel_to=[%d]", to);
	tcp->sel_to = to;

	return tcp->sel_to;
}

char *Tcp_GetAddr( TCP *tcp)
{
	static char rec[ 512];
	sprintf( rec, "%s", inet_ntoa( tcp->svr_addr.sin_addr));
	LogDbg( "Tcp_GetAddr=[%s]", rec);
	return rec;
}

int Tcp_GetPort( TCP *tcp)
{
	return ntohs( tcp->cli_addr.sin_port);
}

char *Tcp_GetServerAddr( TCP *tcp)
{
	LogDbg( "Tcp_GetServerAddr=[%s]", tcp->addr);
	return tcp->addr;
}

int Tcp_GetServerPort( TCP *tcp)
{
	return tcp->port;
}

char *Tcp_GetServAddr( TCP *tcp)
{
	return tcp->addr;
}

int Tcp_GetServPort( TCP *tcp)
{
	return tcp->port;
}

char *Tcp_GetPeerAddr( TCP *tcp)
{
	static char rec[ 512];
	sprintf( rec, "%s", inet_ntoa( tcp->svr_addr.sin_addr));
	LogDbg( "Tcp_GetAddr=[%s]", rec);
	return rec;
}

int Tcp_GetPeerPort( TCP *tcp)
{
	return ntohs( tcp->cli_addr.sin_port);
}

int Tcp_Send( TCP *tcp, char *rec, int sz)
{
	int 	rtn, ssz = 0;

	rtn = send( tcp->sock, rec, sz, 0);
	if( rtn < 0)
	{
		LogErr( "send error.");
		goto error_1;
	}
	if( tcp->debug)	LogDump( &rec[ ssz], rtn, "SEND %d/%d", ssz +rtn, sz);

	return rtn;

	error_1:
		return -errno;
}

int Tcp_SendN( TCP *tcp, char *rec, int sz)
{
	int 	rtn, ssz = 0;

	while( ssz < sz)
	{
		rtn = send( tcp->sock, &rec[ ssz], sz - ssz, 0);
		if( rtn < 0)
		{
			LogErr( "send error.");
			goto error_1;
		}
		if( tcp->debug) LogDump( &rec[ ssz], rtn, "SEND %d/%d", ssz +rtn, sz);
		ssz += rtn;
	}

	return ssz;

	error_1:
		return -errno;
}

int Tcp_SendNT( TCP *tcp, char *rec, int sz, int to)
{
	int 			rtn, ssz = 0;
	fd_set			wfds;
	struct timeval	tv;

	tcp->to = to;

	while( ssz < sz)
	{
		tv.tv_sec = tcp->to / 1000000;
		tv.tv_usec = tcp->to % 1000000;

		FD_ZERO( &wfds);
		FD_SET( tcp->sock, &wfds);

		rtn = select( tcp->sock +1, NULL, &wfds, NULL, &tv);
		if( rtn < 0)
		{
			LogErr( "select error. sock=[%d] rtn=[%d]", tcp->sock, rtn);
			return rtn;
		}
		else if( rtn == 0)
		{
			LogLib( "send timeout. sock=[%d] sec=[%d.%06d]", tcp->sock, tv.tv_sec, tv.tv_usec);
			continue;
		}

		if( FD_ISSET( tcp->sock, &wfds))
		{
			rtn = send( tcp->sock, &rec[ ssz], sz - ssz, 0);
			if( rtn < 0)
			{
				LogErr( "send error.");
				goto error_1;
			}
			if( tcp->debug) LogDump( &rec[ ssz], rtn, "SEND %d/%d", ssz +rtn, sz);
			ssz += rtn;
		}
	}

	return ssz;

	error_1:
		return -errno;
}

int Tcp_Recv( TCP *tcp, char *rec, int sz)
{
	int 	rtn, rsz = 0;

	rtn = recv( tcp->sock, &rec[ rsz], sz - rsz, 0);
	if( rtn < 0)
	{
		LogErr( "recv error.");
		goto error_1;
	}
	if( tcp->debug) LogDump( &rec[ rsz], rtn, "RECV %d/%d", rsz +rtn, sz);
	rsz += rtn;

	return rsz;

	error_1:
		return -errno;
}

int Tcp_RecvN( TCP *tcp, char *rec, int sz)
{
	int 	rtn, rsz = 0;

	while( rsz < sz)
	{
		rtn = recv( tcp->sock, &rec[ rsz], sz - rsz, 0);
		if( rtn <= 0)
		{
			LogErr( "recv error. rec=[%p] sz=[%d/%d] rtn=[%d]", rec, rsz, sz, rtn);
			goto error_1;
		}
		if( tcp->debug) LogDump( &rec[ rsz], rtn, "RECV %d/%d", rsz +rtn, sz);
		rsz += rtn;
	}

	return rsz;

	error_1:
		return -errno;
}

int Tcp_RecvNT( TCP *tcp, char *rec, int sz, int to)
{
	int 			rtn, rsz = 0;
	int				start = 1;
	fd_set			rfds;
	struct timeval	tv, *tp = &tv;

	/* to(timeout)이 0보다 크면 argument 적용, 아니면 Tcp_SetTimeout으로 정의된 값 적용 */
	if( to > 0)
	{
		tcp->to = to;
	}

	LogDbg( "tcp->to       =[%d]", tcp->to);
	LogDbg( "tcp->sel_to   =[%d]", tcp->sel_to);

	while( rsz < sz)
	{
		if( start == 1)
		{
			tv.tv_sec = tcp->to / 1000000;
			tv.tv_usec = tcp->to % 1000000;
			if( tcp->to == 0) tp = NULL;
			start = 0;
		}
		else
		{
			if( tcp->sel_to <= 0) tcp->sel_to = tcp->to;
			tv.tv_sec = tcp->sel_to / 1000000;
			tv.tv_usec = tcp->sel_to % 1000000;
			if( tcp->to == 0) tp = NULL;
			start = 0;
		}

		FD_ZERO( &rfds);
		FD_SET( tcp->sock, &rfds);

		LogDbg( "select wait... sock=[%d] timeout=[%d.%06d] data=[%d/%d]", tcp->sock, tv.tv_sec, tv.tv_usec, rsz, sz);
		rtn = select( tcp->sock +1, &rfds, NULL, NULL, tp);
		if( rtn < 0)
		{
			LogErr( "select error. sock=[%d] rtn=[%d]", tcp->sock, rtn);
			return rtn;
		}
		else if( rtn == 0)
		{
			LogDbg( "recv timeout. sock=[%d] sec=[%d.%06d] rsz=[%d]", tcp->sock, tcp->to / 1000000, tcp->to % 1000000, rsz);
			return rsz;
		}

		if( FD_ISSET( tcp->sock, &rfds))
		{
			rtn = recv( tcp->sock, &rec[ rsz], sz - rsz, 0);
			if( rtn < 0)
			{
				LogErr( "recv error.");
				goto error_1;
			}
			if( rtn == 0)
			{
				LogDbg( "recv 0 byte.");
				return -1;
			}
			if( tcp->debug) LogDump( &rec[ rsz], rtn, "RECV %d/%d/%d", rtn, rsz +rtn, sz);
			rsz += rtn;
		}
	}

	return rsz;

	error_1:
		return -errno;
}

int Tcp_RecvT( TCP *tcp, char *rec, int sz, int to)
{
	int 			rtn;
	fd_set			rfds;
	struct timeval	tv;

	tcp->to = to;
	tv.tv_sec = tcp->to / 1000000;
	tv.tv_usec = tcp->to % 1000000;

	LogDbg( "Tcp_RecvT ... sock=[%d] rec=[%p] sz=[%d] to=[%d.%06d]", tcp->sock, rec, sz, tcp->to/1000000, tcp->to%1000000);

	FD_ZERO( &rfds);
	FD_SET( tcp->sock, &rfds);

	rtn = select( tcp->sock +1, &rfds, NULL, NULL, &tv);
	if( rtn < 0)
	{
		LogErr( "select error. sock=[%d] rtn=[%d]", tcp->sock, rtn);
		return rtn;
	}
	else if( rtn == 0)
	{
		LogDbg( "recv timeout. sock=[%d] sec=[%d.%06d]", tcp->sock, tcp->to / 1000000, tcp->to % 1000000);
		return 0;
	}

	if( FD_ISSET( tcp->sock, &rfds))
	{
		rtn = recv( tcp->sock, rec, sz, 0);
		if( rtn < 0)
		{
			LogErr( "recv error.");
			goto error_1;
		}
		if( rtn == 0)
		{
			LogMsg( "recv 0 byte.... sock=[%d]", tcp->sock);
			return -1;
		}
		if( tcp->debug) LogDump( rec, rtn, "RECV %d/%d", rtn, sz);
		return rtn;
	}

	LogDbg( "no event...");
	return 0;

	error_1:
		return -errno;
}

int Tcp_GetEvent( TCP *tcp, int to)
{
	int 			rtn, rsz = 0;
	fd_set			rfds;
	struct timeval	tv, *tp = &tv;

	tcp->to = to;

	tv.tv_sec = tcp->to / 1000000;
	tv.tv_usec = tcp->to % 1000000;
	if( tcp->to == 0) tp = NULL;

	FD_ZERO( &rfds);
	FD_SET( tcp->sock, &rfds);

	LogDbg( "select wait... sock=[%d] timeout=[%d.%06d]", tcp->sock, tv.tv_sec, tv.tv_usec);
	rtn = select( tcp->sock +1, &rfds, NULL, NULL, tp);
	if( rtn < 0)
	{
		LogErr( "select error. sock=[%d] rtn=[%d]", tcp->sock, rtn);
		return rtn;
	}
	else if( rtn == 0)
	{
		LogDbg( "recv timeout. sock=[%d] sec=[%d.%06d] rsz=[%d]", tcp->sock, tcp->to / 1000000, tcp->to % 1000000, rsz);
		return rsz;
	}

	if( FD_ISSET( tcp->sock, &rfds))
	{
		return 1;
	}
	return 0;
}



