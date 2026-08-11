/** ***************************************************************************
**  @file       udp.c
**  @date       2022/08/23
**  @author     최동춘
**  @version    V1.0.20220823
**  @brif
**  UDP/IP 소켓 통신 프로그램
***************************************************************************** */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "log.h"
#include "udp.h"

/** ***************************************************************************
**  @fn			UDP *Udp_Open()
**  @param      none
**  @return     UDP 구조체
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**	Udp_OpenServer, Udp_OpenClient가 사용할 socket을 open, 구조체 malloc
***************************************************************************** */
UDP *Udp_Open( char *addr, int port)
{
	UDP		*udp;

	udp = malloc( sizeof( UDP));
	if( udp == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( UDP));
		goto error;
	}
	memset( udp, 0x00, sizeof( UDP));

	if( addr == NULL)	udp->addr = NULL;
	else
	{
		udp->addr = malloc( strlen( addr) +1);
		if( udp->addr == NULL)
		{
			LogErr( "malloc error. sz=[%d]", strlen( addr) +1);
			goto error_1;
		}
		memcpy( udp->addr, addr, strlen( addr) +1);
	}
	udp->port = port;

	udp->sock = socket( PF_INET /*AF_INET*/, SOCK_DGRAM, 0);
	if( udp->sock < 0)
	{
		LogErr( "socket open error.");
		goto error_1;
	}
	LogLib( "socket open. sock=[%d]", udp->sock);


	return udp;

	error_1:
		free( udp);
	error:
		LogErr( "Udp_Open error. addr=[%s] port=[%d]", addr, port);
		return NULL;
}

/** ***************************************************************************
**  @fn			int Udp_Close( UDP *udp)
**  @param      UDP *udp
**  @return     1	성공
**  @exception
**  @remark
**  @brief
**	socket close and UDP 구조체 free
***************************************************************************** */
int Udp_Close( UDP *udp)
{
	if( udp->addr != NULL) free( udp->addr);
	LogLib( "close socket. udp->sock=[%d]", udp->sock);
	close( udp->sock);
	free( udp);

	return 1;
}

/** ***************************************************************************
**  @fn			UDP* Udp_OpenServer( char *addr, int port)
**  @param      char *addr	- server bind address, NULL이면 bind 하지 않는다
**  @param      int port 	- wait port
**  @return     UDP 구조체
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  UDP SERVER 소켓 OPEN
**  addr 주소로 bind
**  이후 반환된 UDP로 listen, accept 한다
***************************************************************************** */
UDP *Udp_OpenServer( char *addr, int port)
{
	int						rtn, so_flag = 1;
	in_addr_t				in_addr;
	UDP						*udp;

	udp = Udp_Open( addr, port);
	if( udp == NULL)
	{
		LogCri( "Udp_Open error.");
		goto error;
	}

	in_addr = inet_addr( addr);
	if( in_addr == -1)
	{
		LogLib( "address convert error. addr=[%s] in_addr=[%d] ... set in_addr=[0]", addr, in_addr);
		in_addr = 0;
	}

	udp->svr_addr.sin_family = AF_INET;
#if 0
	udp->svr_addr.sin_addr.s_addr = in_addr;
#else
	udp->svr_addr.sin_addr.s_addr = INADDR_ANY;
#endif
	udp->svr_addr.sin_port = htons( port);
#if 0
	setsockopt( udp->sock, SOL_SOCKET, SO_REUSEADDR, ( const char *)&so_flag, sizeof( so_flag));
#endif
	rtn = bind( udp->sock, ( struct sockaddr *)&udp->svr_addr, sizeof( udp->svr_addr));
	if( rtn < 0)
	{
		LogErr( "bind error. addr=[%s] port=[%d]", 
				inet_ntoa( udp->svr_addr.sin_addr), ntohs( udp->svr_addr.sin_port));
		goto error_1;
	}
	LogLib( "bind success. addr=[%s] port=[%d]", 
			inet_ntoa( udp->svr_addr.sin_addr), ntohs( udp->svr_addr.sin_port));

	return udp;

	error_1:
		Udp_Close( udp);
	error:
		LogErr( "Udp_OpenServer error. addr=[%s] port=[%d]", addr, port);
		return NULL;
}

/** ***************************************************************************
**  @fn			UDP *Udp_OpenClient( char *addr, int port)
**  @param      char *addr	- server connect address
**  @param      int port 	- connect port
**  @return     UDP 구조체
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  UDP CLIENT 소켓 OPEN
**  addr 주소로 connect
***************************************************************************** */
UDP *Udp_OpenClient( char *addr, int port)
{
	UDP						*udp;

	udp = Udp_Open( addr, port);
	if( udp == NULL)
	{
		LogCri( "Udp_Open error.");
		goto error;
	}
	LogMsg( "Udp_Open get udp=[%p] sock=[%d] addr=[%s] port=[%d]", udp, udp->sock, addr, port);

	udp->svr_addr.sin_family = AF_INET;
	udp->svr_addr.sin_addr.s_addr = inet_addr( addr);
	udp->svr_addr.sin_port = htons( port);

	udp->cli_addr.sin_family = AF_INET;
	udp->cli_addr.sin_addr.s_addr = INADDR_ANY;
	udp->cli_addr.sin_port = htons( port);

#if 0
	rtn = bind( udp->sock, (struct sockaddr *)&udp->cli_addr, sizeof( udp->svr_addr));
	if( rtn < 0)
	{
		LogErr( "bind error. addr=[%s] port=[%d]", 
				inet_ntoa( udp->svr_addr.sin_addr), ntohs( udp->svr_addr.sin_port));
		goto error_1;
	}
#endif

	return udp;

#if 0
	error_1:
		Udp_Close( udp);
#endif
	error:
		return NULL;
}

/** ***************************************************************************
**  @fn			int Udp_Listen( int sock, int backlog)
**  @param      UDP * - UDP structure
**  @return     error 여부
**  @retval     listen의 return 값
**  @exception
**  @remark
**  @brief
**  listen 함수 call
***************************************************************************** */
int Udp_SetClient( UDP *udp, char *addr)
{
	udp->cli_addr.sin_family = AF_INET;
	udp->cli_addr.sin_addr.s_addr = inet_addr( addr);
	udp->cli_addr.sin_port = udp->svr_addr.sin_port;

	return 1;
}

/** ***************************************************************************
**  @fn			int Udp_Accept( UDP *udp)
**  @param      UDP * - UDP structure
**  @return     UDP * - 접속한 client의 UDP structure
**  @exception
**  @remark
**  @brief
**  client의 접속을 허가 ... udp->cli_addr에 client 정보 setting
***************************************************************************** */
int Udp_Info( UDP *udp)
{
	/*
	rtn = getsockname( udp->sock, ( struct sockaddr *)&udp->cli_addr, &sz);
	if( rtn < 0)
	{
		LogErr( "getsockname error. rtn=[%d]", rtn);
	}
	rtn = getpeername( udp->sock, ( struct sockaddr *)&udp->svr_addr, &sz);
	if( rtn < 0)
	{
		LogErr( "getsockname error. rtn=[%d]", rtn);
	}
	*/

	LogDbg( "udp           = [%p]", udp);
	LogDbg( "socket        = [%d]", udp->sock);
	LogDbg( "addr          = [%s]", udp->addr);
	LogDbg( "port          = [%d]", udp->port);
	LogDbg( "client -------------------------");
	LogDbg( "addr          = [%s]", inet_ntoa( udp->cli_addr.sin_addr));
	LogDbg( "port          = [%d]", ntohs( udp->cli_addr.sin_port));
	LogDbg( "server -------------------------");
	LogDbg( "addr          = [%s]", inet_ntoa( udp->svr_addr.sin_addr));
	LogDbg( "port          = [%d]", ntohs( udp->svr_addr.sin_port));

	return 1;
}

int Udp_SetTimeout( UDP *udp, int to)
{
	LogMsg( "SetTimeout ... udp->to=[%d]", to);
	udp->to = to;

	return udp->to;
}

int Udp_SetSelectTimeout( UDP *udp, int to)
{
	LogDbg( "SetSelectTimeout ... udp->sel_to=[%d]", to);
	udp->sel_to = to;

	return udp->sel_to;
}

char *Udp_GetAddr( UDP *udp)
{
	static char rec[ 512];
	sprintf( rec, "%s", inet_ntoa( udp->svr_addr.sin_addr));
	LogDbg( "Udp_GetAddr=[%s]", rec);
	return rec;
}

int Udp_GetPort( UDP *udp)
{
	return ntohs( udp->cli_addr.sin_port);
}

char *Udp_GetServerAddr( UDP *udp)
{
	LogDbg( "Udp_GetServerAddr=[%s]", udp->addr);
	return udp->addr;
}

int Udp_GetServerPort( UDP *udp)
{
	return udp->port;
}

char *Udp_GetServAddr( UDP *udp)
{
	return udp->addr;
}

int Udp_GetServPort( UDP *udp)
{
	return udp->port;
}

char *Udp_GetPeerAddr( UDP *udp)
{
	static char rec[ 512];
	sprintf( rec, "%s", inet_ntoa( udp->svr_addr.sin_addr));
	LogDbg( "Udp_GetAddr=[%s]", rec);
	return rec;
}

int Udp_GetPeerPort( UDP *udp)
{
	return ntohs( udp->cli_addr.sin_port);
}

int Udp_SendTo( UDP *udp, char *rec, int sz)
{
	int 	rtn;

	LogDel( "sendto sock=[%d] rec=[%p] sz=[%d] to=[%s:%d]", 
			udp->sock, rec, sz, inet_ntoa( udp->svr_addr.sin_addr), ntohs( udp->svr_addr.sin_port));
	rtn = sendto( udp->sock, rec, sz, 0, (struct sockaddr *)&udp->svr_addr, sizeof( udp->svr_addr));
	if( rtn < 0)
	{
		LogErr( "sendto error.");
		goto error_1;
	}
	LogDump( rec, rtn, "SEND %d/%d", rtn, sz);

	return rtn;

	error_1:
		return -errno;
}

int Udp_Send( UDP *udp, char *rec, int sz)
{
	int 	rtn;

	LogDel( "sendto sock=[%d] rec=[%p] sz=[%d] to=[%s:%d]", 
			udp->sock, rec, sz, inet_ntoa( udp->svr_addr.sin_addr), ntohs( udp->svr_addr.sin_port));
	rtn = sendto( udp->sock, rec, sz, 0, (struct sockaddr *)&udp->svr_addr, sizeof( udp->svr_addr));
	if( rtn < 0)
	{
		LogErr( "sendto error.");
		goto error_1;
	}
	LogDump( rec, rtn, "SEND %d/%d", rtn, sz);

	return rtn;

	error_1:
		return -errno;
}

int Udp_SendNT( UDP *udp, char *rec, int sz, int to)
{
	int 			rtn, ssz = 0;
	fd_set			wfds;
	struct timeval	tv;

	udp->to = to;

	while( ssz < sz)
	{
		tv.tv_sec = udp->to / 1000000;
		tv.tv_usec = udp->to % 1000000;

		FD_ZERO( &wfds);
		FD_SET( udp->sock, &wfds);

		rtn = select( udp->sock +1, NULL, &wfds, NULL, &tv);
		if( rtn < 0)
		{
			LogErr( "select error. sock=[%d] rtn=[%d]", udp->sock, rtn);
			return rtn;
		}
		else if( rtn == 0)
		{
			LogLib( "send timeout. sock=[%d] sec=[%d.%06d]", udp->sock, tv.tv_sec, tv.tv_usec);
			continue;
		}

		if( FD_ISSET( udp->sock, &wfds))
		{
			rtn = send( udp->sock, &rec[ ssz], sz - ssz, 0);
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

int Udp_RecvFrom( UDP *udp, char *rec, int sz)
{
	int 			rtn, rsz = 0;
	unsigned int	addr_sz = 0;

	addr_sz = sizeof( udp->cli_addr);

	rtn = recvfrom( udp->sock, &rec[ rsz], sz - rsz, 0, (struct sockaddr *)&udp->cli_addr, &addr_sz);
	if( rtn < 0)
	{
		LogErr( "recv error.");
		goto error_1;
	}
	LogDump( &rec[ rsz], rtn, "RECV %d/%d", rsz +rtn, sz);
	rsz += rtn;

	return rsz;

	error_1:
		return -errno;
}

int Udp_Recv( UDP *udp, char *rec, int sz)
{
	int 			rtn, rsz = 0;
	unsigned int	addr_sz = 0;

	rtn = recvfrom( udp->sock, &rec[ rsz], sz - rsz, 0, (struct sockaddr *)&udp->cli_addr, &addr_sz);
	if( rtn < 0)
	{
		LogErr( "recv error.");
		goto error_1;
	}
	LogDump( &rec[ rsz], rtn, "RECV %d/%d", rsz +rtn, sz);
	rsz += rtn;

	return rsz;

	error_1:
		return -errno;
}

int Udp_RecvNT( UDP *udp, char *rec, int sz, int to)
{
	int 			rtn, rsz = 0;
	int				start = 1;
	unsigned int	addr_sz = 0;
	fd_set			rfds;
	struct timeval	tv, *tp = &tv;

	/* to(timeout)이 0보다 크면 argument 적용, 아니면 Udp_SetTimeout으로 정의된 값 적용 */
	if( to > 0)
	{
		udp->to = to;
	}

	LogDbg( "udp->to       =[%d]", udp->to);
	LogDbg( "udp->sel_to   =[%d]", udp->sel_to);

	while( rsz < sz)
	{
		if( start == 1)
		{
			tv.tv_sec = udp->to / 1000000;
			tv.tv_usec = udp->to % 1000000;
			if( udp->to == 0) tp = NULL;
			start = 0;
		}
		else
		{
			if( udp->sel_to <= 0) udp->sel_to = udp->to;
			tv.tv_sec = udp->sel_to / 1000000;
			tv.tv_usec = udp->sel_to % 1000000;
			if( udp->to == 0) tp = NULL;
			start = 0;
		}

		FD_ZERO( &rfds);
		FD_SET( udp->sock, &rfds);

		LogDbg( "select wait... sock=[%d] timeout=[%d.%06d] data=[%d/%d]", udp->sock, tv.tv_sec, tv.tv_usec, rsz, sz);
		rtn = select( udp->sock +1, &rfds, NULL, NULL, tp);
		if( rtn < 0)
		{
			LogErr( "select error. sock=[%d] rtn=[%d]", udp->sock, rtn);
			return rtn;
		}
		else if( rtn == 0)
		{
			LogDbg( "recv timeout. sock=[%d] sec=[%d.%06d] rsz=[%d]", udp->sock, udp->to / 1000000, udp->to % 1000000, rsz);
			return rsz;
		}

		if( FD_ISSET( udp->sock, &rfds))
		{
			rtn = recvfrom( udp->sock, &rec[ rsz], sz - rsz, 0, (struct sockaddr *)&udp->cli_addr, &addr_sz);
			/* rtn = recv( udp->sock, &rec[ rsz], sz - rsz, 0); */
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

int Udp_RecvT( UDP *udp, char *rec, int sz, int to)
{
	int 			rtn;
	unsigned int	addr_sz = 0;
	fd_set			rfds;
	struct timeval	tv;

	udp->to = to;
	tv.tv_sec = udp->to / 1000000;
	tv.tv_usec = udp->to % 1000000;

	LogDel( "Udp_RecvT ... sock=[%d] rec=[%p] sz=[%d] to=[%d.%06d]", 
			udp->sock, rec, sz, udp->to/1000000, udp->to%1000000);
	LogDel( "              from=[%s:%d]", 
			inet_ntoa( udp->svr_addr.sin_addr), ntohs( udp->svr_addr.sin_port));

	FD_ZERO( &rfds);
	FD_SET( udp->sock, &rfds);

	rtn = select( udp->sock +1, &rfds, NULL, NULL, &tv);
	if( rtn < 0)
	{
		LogErr( "select error. sock=[%d] rtn=[%d]", udp->sock, rtn);
		return rtn;
	}
	else if( rtn == 0)
	{
		LogDel( "recv timeout. sock=[%d] sec=[%d.%06d]", udp->sock, udp->to / 1000000, udp->to % 1000000);
		return 0;
	}

	if( FD_ISSET( udp->sock, &rfds))
	{
		LogDel( "Udp_RecvT ... sock=[%d] rec=[%p] sz=[%d] from=[%s:%d]", 
				udp->sock, rec, sz, udp->cli_addr, udp->to%1000000);
		rtn = recvfrom( udp->sock, rec, sz, 0, (struct sockaddr *)&udp->cli_addr, &addr_sz);
		if( rtn < 0)
		{
			LogErr( "recv error.");
			goto error_1;
		}
		if( rtn == 0)
		{
			LogMsg( "recv 0 byte.... sock=[%d]", udp->sock);
			return -1;
		}
		LogDump( rec, rtn, "RECV %d/%d", rtn, sz);
		return rtn;
	}

	LogDbg( "no event...");
	return 0;

	error_1:
		return -errno;
}

int Udp_GetEvent( UDP *udp, int to)
{
	int 			rtn, rsz = 0;
	fd_set			rfds;
	struct timeval	tv, *tp = &tv;

	udp->to = to;

	tv.tv_sec = udp->to / 1000000;
	tv.tv_usec = udp->to % 1000000;
	if( udp->to == 0) tp = NULL;

	FD_ZERO( &rfds);
	FD_SET( udp->sock, &rfds);

	LogDbg( "select wait... sock=[%d] timeout=[%d.%06d]", udp->sock, tv.tv_sec, tv.tv_usec);
	rtn = select( udp->sock +1, &rfds, NULL, NULL, tp);
	if( rtn < 0)
	{
		LogErr( "select error. sock=[%d] rtn=[%d]", udp->sock, rtn);
		return rtn;
	}
	else if( rtn == 0)
	{
		LogDbg( "recv timeout. sock=[%d] sec=[%d.%06d] rsz=[%d]", udp->sock, udp->to / 1000000, udp->to % 1000000, rsz);
		return rsz;
	}

	if( FD_ISSET( udp->sock, &rfds))
	{
		return 1;
	}
	return 0;
}



