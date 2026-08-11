/** ***************************************************************************
**  @file       mcp.c
**  @date       2022/08/23
**  @author     최동춘
**  @version    V1.0.20220823
**  @brif
**  MCP/IP 소켓 통신 프로그램
***************************************************************************** */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "log.h"
#include "mcp.h"

/** ***************************************************************************
**  @fn			MCP *Mcp_Open()
**  @param      none
**  @return     MCP 구조체
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**	Mcp_OpenServer, Mcp_OpenClient가 사용할 socket을 open, 구조체 malloc
***************************************************************************** */
MCP *Mcp_Open( char *addr, int port)
{
	MCP		*mcp;

	mcp = malloc( sizeof( MCP));
	if( mcp == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( MCP));
		goto error;
	}
	memset( mcp, 0x00, sizeof( MCP));

	if( addr == NULL)	mcp->addr = NULL;
	else
	{
		mcp->addr = malloc( strlen( addr) +1);
		if( mcp->addr == NULL)
		{
			LogErr( "malloc error. sz=[%d]", strlen( addr) +1);
			goto error_1;
		}
		memcpy( mcp->addr, addr, strlen( addr) +1);
	}
	mcp->port = port;

/*
	mcp->sock = socket( AF_INET, SOCK_DGRAM, 0);
*/
	mcp->sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if( mcp->sock < 0)
	{
		LogErr( "socket open error.");
		goto error_1;
	}
	LogLib( "socket open. sock=[%d]", mcp->sock);


	return mcp;

	error_1:
		free( mcp);
	error:
		LogErr( "Mcp_Open error. addr=[%s] port=[%d]", addr, port);
		return NULL;
}

/** ***************************************************************************
**  @fn			int Mcp_Close( MCP *mcp)
**  @param      MCP *mcp
**  @return     1	성공
**  @exception
**  @remark
**  @brief
**	socket close and MCP 구조체 free
***************************************************************************** */
int Mcp_Close( MCP *mcp)
{
	if( mcp->addr != NULL) free( mcp->addr);
	LogLib( "close socket. mcp->sock=[%d]", mcp->sock);
	close( mcp->sock);
	free( mcp);

	return 1;
}

/** ***************************************************************************
**  @fn			MCP* Mcp_OpenServer( char *addr, int port)
**  @param      char *addr	- server bind address, NULL이면 bind 하지 않는다
**  @param      int port 	- wait port
**  @return     MCP 구조체
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  MCP SERVER 소켓 OPEN
**  addr 주소로 bind
**  이후 반환된 MCP로 listen, accept 한다
***************************************************************************** */
MCP *Mcp_OpenServer( char *addr, int port)
{
	int						rtn, so_flag = 1;
	in_addr_t				in_addr;
	MCP						*mcp;

	mcp = Mcp_Open( addr, port);
	if( mcp == NULL)
	{
		LogCri( "Mcp_Open error.");
		goto error;
	}

	in_addr = inet_addr( addr);
	if( in_addr == -1)
	{
		LogLib( "address convert error. addr=[%s] in_addr=[%d] ... set in_addr=[0]", addr, in_addr);
		in_addr = 0;
	}

#if 0
	mcp->svr_addr.sin_family = AF_INET;
	mcp->svr_addr.sin_addr.s_addr = in_addr;
	mcp->svr_addr.sin_port = htons( port);
#else
	mcp->svr_addr.sin_family = AF_INET;
	mcp->svr_addr.sin_addr.s_addr = htonl( INADDR_ANY);
	mcp->svr_addr.sin_port = htons( port);
#endif
	setsockopt( mcp->sock, SOL_SOCKET, SO_REUSEADDR, ( const char *)&so_flag, sizeof( so_flag));
	rtn = bind( mcp->sock, ( struct sockaddr *)&mcp->svr_addr, sizeof( mcp->svr_addr));
	if( rtn < 0)
	{
		LogErr( "bind error. addr=[%s] port=[%d]", 
				inet_ntoa( mcp->svr_addr.sin_addr), ntohs( mcp->svr_addr.sin_port));
		goto error_1;
	}
	LogLib( "bind success. addr=[%s] port=[%d]", 
			inet_ntoa( mcp->svr_addr.sin_addr), ntohs( mcp->svr_addr.sin_port));

	mcp->mreq.imr_multiaddr.s_addr = inet_addr( addr);
	mcp->mreq.imr_interface.s_addr = htonl( INADDR_ANY);
	// mcp->mreq.imr_interface.s_addr = INADDR_ANY;
 
	// setsockopt( mcp->sock, IPPROTO_IP, IP_MULTICAST_LOOP, ( char *)&mcp->mreq, sizeof( mcp->mreq));
	setsockopt( mcp->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, ( char *)&mcp->mreq, sizeof( mcp->mreq));


	mcp->backlog = 5;

	return mcp;

	error_1:
		Mcp_Close( mcp);
	error:
		LogErr( "Mcp_OpenServer error. addr=[%s] port=[%d]", addr, port);
		return NULL;
}

/** ***************************************************************************
**  @fn			MCP *Mcp_OpenClient( char *addr, int port)
**  @param      char *addr	- server connect address
**  @param      int port 	- connect port
**  @return     MCP 구조체
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  MCP CLIENT 소켓 OPEN
**  addr 주소로 connect
***************************************************************************** */
MCP *Mcp_OpenClient( char *addr, int port)
{
	int						rtn;
	int						so_flag = 1;
	int						ttl = 1;
	MCP						*mcp;

	mcp = Mcp_Open( addr, port);
	if( mcp == NULL)
	{
		LogCri( "Mcp_Open error.");
		goto error;
	}
	LogMsg( "Mcp_Open get mcp=[%p] sock=[%d]", mcp, mcp->sock);

	mcp->mreq.imr_multiaddr.s_addr = inet_addr( addr);
	mcp->mreq.imr_interface.s_addr = htonl( INADDR_ANY);
	// mcp->mreq.imr_interface.s_addr = INADDR_ANY;

	mcp->cli_addr.sin_family = AF_INET;
	mcp->cli_addr.sin_addr.s_addr = htonl( INADDR_ANY);
	mcp->cli_addr.sin_port = htons( port);

	mcp->svr_addr.sin_family = AF_INET;
	mcp->svr_addr.sin_addr.s_addr = inet_addr( addr);
	mcp->svr_addr.sin_port = htons( port);

#if 0
	setsockopt( mcp->sock, SOL_SOCKET, SO_REUSEADDR, ( const char *)&so_flag, sizeof( so_flag));

	rtn = bind( mcp->sock, (struct sockaddr *)&mcp->cli_addr, sizeof( mcp->svr_addr));
	if( rtn < 0)
	{
		LogErr( "bind error. addr=[%s] port=[%d]", 
				inet_ntoa( mcp->svr_addr.sin_addr), ntohs( mcp->svr_addr.sin_port));
		goto error_1;
	}

	// setsockopt( mcp->sock, IPPROTO_IP, IP_MULTICAST_LOOP, ( char *)&mcp->mreq, sizeof( mcp->mreq));
	// setsockopt( mcp->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, ( char *)&mcp->mreq, sizeof( mcp->mreq));
	setsockopt( mcp->sock, IPPROTO_IP, IP_MULTICAST_IF, ( char *)&mcp->mreq, sizeof( mcp->mreq));
#endif
	setsockopt( mcp->sock, IPPROTO_IP, IP_MULTICAST_TTL, ( void *)&ttl, sizeof( ttl));

	return mcp;

	error_1:
		Mcp_Close( mcp);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fn			int Mcp_Listen( int sock, int backlog)
**  @param      MCP * - MCP structure
**  @return     error 여부
**  @retval     listen의 return 값
**  @exception
**  @remark
**  @brief
**  listen 함수 call
***************************************************************************** */
int Mcp_Listen( MCP *mcp)
{
	LogLib( "Listen ... sock=[%d] backlog=[%d]", mcp->sock, mcp->backlog);
	return listen( mcp->sock, mcp->backlog);
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  client의 접속을 허가 ... mcp->cli_addr에 client 정보 setting
***************************************************************************** */
MCP *Mcp_Accept( MCP *mcp)
{
	int				sock;
	unsigned int	addr_len;
	MCP				*new_mcp;

	addr_len = sizeof( mcp->cli_addr);
	sock = accept( mcp->sock, ( struct sockaddr *)&mcp->cli_addr, &addr_len);
	if( sock < 0)
	{
		LogErr( "accept error. sock=[%d]", sock);
		goto error;
	}
	LogLib( "accepted. sock=[%d] addr=[%s] port=[%d]", 
			sock, inet_ntoa( mcp->cli_addr.sin_addr), ntohs( mcp->cli_addr.sin_port));

	new_mcp = malloc( sizeof( MCP));
	if( new_mcp == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( MCP));
		goto error;
	}
	memcpy( new_mcp, mcp, sizeof( MCP));
	new_mcp->sock = sock;

	return new_mcp;;

	error:
		return NULL;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  client의 접속을 허가 ... mcp->cli_addr에 client 정보 setting
***************************************************************************** */
int Mcp_Info( MCP *mcp)
{
	/*
	rtn = getsockname( mcp->sock, ( struct sockaddr *)&mcp->cli_addr, &sz);
	if( rtn < 0)
	{
		LogErr( "getsockname error. rtn=[%d]", rtn);
	}
	rtn = getpeername( mcp->sock, ( struct sockaddr *)&mcp->svr_addr, &sz);
	if( rtn < 0)
	{
		LogErr( "getsockname error. rtn=[%d]", rtn);
	}
	*/

	LogDbg( "mcp           = [%p]", mcp);
	LogDbg( "socket        = [%d]", mcp->sock);
	LogDbg( "addr          = [%s]", mcp->addr);
	LogDbg( "port          = [%d]", mcp->port);
	LogDbg( "client -------------------------");
	LogDbg( "addr          = [%s]", inet_ntoa( mcp->cli_addr.sin_addr));
	LogDbg( "port          = [%d]", ntohs( mcp->cli_addr.sin_port));
	LogDbg( "server -------------------------");
	LogDbg( "addr          = [%s]", inet_ntoa( mcp->svr_addr.sin_addr));
	LogDbg( "port          = [%d]", ntohs( mcp->svr_addr.sin_port));

	return 1;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
int Mcp_SetTimeout( MCP *mcp, int to)
{
	LogMsg( "SetTimeout ... mcp->to=[%d]", to);
	mcp->to = to;

	return mcp->to;
}

int Mcp_SetSelectTimeout( MCP *mcp, int to)
{
	LogDbg( "SetSelectTimeout ... mcp->sel_to=[%d]", to);
	mcp->sel_to = to;

	return mcp->sel_to;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
char *Mcp_GetAddr( MCP *mcp)
{
	static char rec[ 512];
	sprintf( rec, "%s", inet_ntoa( mcp->svr_addr.sin_addr));
	LogDbg( "Mcp_GetAddr=[%s]", rec);
	return rec;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
int Mcp_GetPort( MCP *mcp)
{
	return ntohs( mcp->cli_addr.sin_port);
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
char *Mcp_GetServerAddr( MCP *mcp)
{
	LogDbg( "Mcp_GetServerAddr=[%s]", mcp->addr);
	return mcp->addr;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
int Mcp_GetServerPort( MCP *mcp)
{
	return mcp->port;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
char *Mcp_GetServAddr( MCP *mcp)
{
	return mcp->addr;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
int Mcp_GetServPort( MCP *mcp)
{
	return mcp->port;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
char *Mcp_GetPeerAddr( MCP *mcp)
{
	static char rec[ 512];
	sprintf( rec, "%s", inet_ntoa( mcp->svr_addr.sin_addr));
	LogDbg( "Mcp_GetAddr=[%s]", rec);
	return rec;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
int Mcp_GetPeerPort( MCP *mcp)
{
	return ntohs( mcp->cli_addr.sin_port);
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
int Mcp_Send( MCP *mcp, char *rec, int sz)
{
	int 	rtn, ssz = 0;

	rtn = send( mcp->sock, rec, sz, 0);
	if( rtn < 0)
	{
		LogErr( "send error.");
		goto error_1;
	}
	LogDump( &rec[ ssz], rtn, "SEND %d/%d", ssz +rtn, sz);

	return rtn;

	error_1:
		return -errno;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
int Mcp_SendTo( MCP *mcp, char *rec, int sz)
{
	int 	rtn, ssz = 0;

	rtn = sendto( mcp->sock, rec, sz, 0, ( struct sockaddr *)&mcp->svr_addr, sizeof( mcp->svr_addr));
	if( rtn < 0)
	{
		LogErr( "send error.");
		goto error_1;
	}
	LogDump( &rec[ ssz], rtn, "SEND %d/%d", ssz +rtn, sz);

	return rtn;

	error_1:
		return -errno;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
int Mcp_SendN( MCP *mcp, char *rec, int sz)
{
	int 	rtn, ssz = 0;

	while( ssz < sz)
	{
		rtn = send( mcp->sock, &rec[ ssz], sz - ssz, 0);
		if( rtn < 0)
		{
			LogErr( "send error.");
			goto error_1;
		}
		LogDump( &rec[ ssz], rtn, "SEND %d/%d", ssz +rtn, sz);
		ssz += rtn;
	}

	return ssz;

	error_1:
		return -errno;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
int Mcp_SendNT( MCP *mcp, char *rec, int sz, int to)
{
	int 			rtn, ssz = 0;
	fd_set			wfds;
	struct timeval	tv;

	mcp->to = to;

	while( ssz < sz)
	{
		tv.tv_sec = mcp->to / 1000000;
		tv.tv_usec = mcp->to % 1000000;

		FD_ZERO( &wfds);
		FD_SET( mcp->sock, &wfds);

		rtn = select( mcp->sock +1, NULL, &wfds, NULL, &tv);
		if( rtn < 0)
		{
			LogErr( "select error. sock=[%d] rtn=[%d]", mcp->sock, rtn);
			return rtn;
		}
		else if( rtn == 0)
		{
			LogLib( "send timeout. sock=[%d] sec=[%d.%06d]", mcp->sock, tv.tv_sec, tv.tv_usec);
			continue;
		}

		if( FD_ISSET( mcp->sock, &wfds))
		{
			rtn = send( mcp->sock, &rec[ ssz], sz - ssz, 0);
			if( rtn < 0)
			{
				LogErr( "send error.");
				goto error_1;
			}
			LogDump( &rec[ ssz], rtn, "SEND %d/%d", ssz +rtn, sz);
			ssz += rtn;
		}
	}

	return ssz;

	error_1:
		return -errno;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
int Mcp_Recv( MCP *mcp, char *rec, int sz)
{
	int 			rtn, rsz = 0;
	unsigned int	addr_sz = 0;

#if 0
	rtn = recvfrom( mcp->sock, &rec[ rsz], sz - rsz, 0, (struct sockaddr *)&mcp->cli_addr, &addr_sz);
	if( rtn < 0)
	{
		LogErr( "recv error.");
		goto error_1;
	}
#else
	rtn = recvfrom( mcp->sock, &rec[ rsz], sz - rsz, 0, (struct sockaddr *)NULL, &addr_sz);
	if( rtn < 0)
	{
		LogErr( "recv error.");
		goto error_1;
	}
#endif
	LogDddd( &rec[ rsz], rtn, "RECV %d/%d", rsz +rtn, sz);
	rsz += rtn;

	return rsz;

	error_1:
		return -errno;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
int Mcp_RecvN( MCP *mcp, char *rec, int sz)
{
	int 	rtn, rsz = 0;

	while( rsz < sz)
	{
		rtn = recv( mcp->sock, &rec[ rsz], sz - rsz, 0);
		if( rtn <= 0)
		{
			LogErr( "recv error. rec=[%p] sz=[%d/%d] rtn=[%d]", rec, rsz, sz, rtn);
			goto error_1;
		}
		LogDump( &rec[ rsz], rtn, "RECV %d/%d", rsz +rtn, sz);
		rsz += rtn;
	}

	return rsz;

	error_1:
		return -errno;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
int Mcp_RecvNT( MCP *mcp, char *rec, int sz, int to)
{
	int 			rtn, rsz = 0;
	int				start = 1;
	fd_set			rfds;
	struct timeval	tv, *tp = &tv;

	/* to(timeout)이 0보다 크면 argument 적용, 아니면 Mcp_SetTimeout으로 정의된 값 적용 */
	if( to > 0)
	{
		mcp->to = to;
	}

	LogDbg( "mcp->to       =[%d]", mcp->to);
	LogDbg( "mcp->sel_to   =[%d]", mcp->sel_to);

	while( rsz < sz)
	{
		if( start == 1)
		{
			tv.tv_sec = mcp->to / 1000000;
			tv.tv_usec = mcp->to % 1000000;
			if( mcp->to == 0) tp = NULL;
			start = 0;
		}
		else
		{
			if( mcp->sel_to <= 0) mcp->sel_to = mcp->to;
			tv.tv_sec = mcp->sel_to / 1000000;
			tv.tv_usec = mcp->sel_to % 1000000;
			if( mcp->to == 0) tp = NULL;
			start = 0;
		}

		FD_ZERO( &rfds);
		FD_SET( mcp->sock, &rfds);

		LogDbg( "select wait... sock=[%d] timeout=[%d.%06d] data=[%d/%d]", mcp->sock, tv.tv_sec, tv.tv_usec, rsz, sz);
		rtn = select( mcp->sock +1, &rfds, NULL, NULL, tp);
		if( rtn < 0)
		{
			LogErr( "select error. sock=[%d] rtn=[%d]", mcp->sock, rtn);
			return rtn;
		}
		else if( rtn == 0)
		{
			LogDbg( "recv timeout. sock=[%d] sec=[%d.%06d] rsz=[%d]", mcp->sock, mcp->to / 1000000, mcp->to % 1000000, rsz);
			return rsz;
		}

		if( FD_ISSET( mcp->sock, &rfds))
		{
			rtn = recv( mcp->sock, &rec[ rsz], sz - rsz, 0);
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
			LogDump( &rec[ rsz], rtn, "RECV %d/%d/%d", rtn, rsz +rtn, sz);
			rsz += rtn;
		}
	}

	return rsz;

	error_1:
		return -errno;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
int Mcp_RecvT( MCP *mcp, char *rec, int sz, int to)
{
	int 			rtn;
	fd_set			rfds;
	struct timeval	tv;

	mcp->to = to;
	tv.tv_sec = mcp->to / 1000000;
	tv.tv_usec = mcp->to % 1000000;

	LogDel( "Mcp_RecvT ... sock=[%d] rec=[%p] sz=[%d] to=[%d.%06d]", mcp->sock, rec, sz, mcp->to/1000000, mcp->to%1000000);

	FD_ZERO( &rfds);
	FD_SET( mcp->sock, &rfds);

	rtn = select( mcp->sock +1, &rfds, NULL, NULL, &tv);
	if( rtn < 0)
	{
		LogErr( "select error. sock=[%d] rtn=[%d]", mcp->sock, rtn);
		return rtn;
	}
	else if( rtn == 0)
	{
		LogDel( "recv timeout. sock=[%d] sec=[%d.%06d]", mcp->sock, mcp->to / 1000000, mcp->to % 1000000);
		return 0;
	}

	if( FD_ISSET( mcp->sock, &rfds))
	{
		rtn = recv( mcp->sock, rec, sz, 0);
		if( rtn < 0)
		{
			LogErr( "recv error.");
			goto error_1;
		}
		if( rtn == 0)
		{
			LogMsg( "recv 0 byte.... sock=[%d]", mcp->sock);
			return -1;
		}
		LogDddd( rec, rtn, "RECV %d/%d", rtn, sz);
		return rtn;
	}

	LogDbg( "no event...");
	return 0;

	error_1:
		return -errno;
}

/** ***************************************************************************
**  @fn			int Mcp_Accept( MCP *mcp)
**  @param      MCP * - MCP structure
**  @return     MCP * - 접속한 client의 MCP structure
**  @exception
**  @remark
**  @brief
**  multicast library
***************************************************************************** */
int Mcp_GetEvent( MCP *mcp, int to)
{
	int 			rtn, rsz = 0;
	fd_set			rfds;
	struct timeval	tv, *tp = &tv;

	mcp->to = to;

	tv.tv_sec = mcp->to / 1000000;
	tv.tv_usec = mcp->to % 1000000;
	if( mcp->to == 0) tp = NULL;

	FD_ZERO( &rfds);
	FD_SET( mcp->sock, &rfds);

	LogDbg( "select wait... sock=[%d] timeout=[%d.%06d]", mcp->sock, tv.tv_sec, tv.tv_usec);
	rtn = select( mcp->sock +1, &rfds, NULL, NULL, tp);
	if( rtn < 0)
	{
		LogErr( "select error. sock=[%d] rtn=[%d]", mcp->sock, rtn);
		return rtn;
	}
	else if( rtn == 0)
	{
		LogDbg( "recv timeout. sock=[%d] sec=[%d.%06d] rsz=[%d]", mcp->sock, mcp->to / 1000000, mcp->to % 1000000, rsz);
		return rsz;
	}

	if( FD_ISSET( mcp->sock, &rfds))
	{
		return 1;
	}
	return 0;
}



