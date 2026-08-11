#include <stdio.h>
#include <stdlib.h>

#include "tcp.h"
#include "main.h"
#include "process.h"

/** ***************************************************************************
**  @fn         int ServerProcess( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  프로그램 수행
***************************************************************************** */
int ServerProcess( char *addr, int port)
{
	int		rtn, i;
	int		sz, tot_sz = 0;;
	char	rec[ 65535];
	TCP		*client;

	int				sock;
	int				nfds = 0;
	fd_set			rfds;
	struct timeval	tv;
	struct sockaddr	client_addr;

	LogDbg( "MainProcess start.");

	for( i = 0; i < MAX_CLIENT; i++) Client[ i] = NULL;

	Server = Tcp_OpenServer( addr, port);
	if( Server == NULL)
	{
		LogCri( "Tcp_OpenServer error. addr=[%s] port=[%d]", Param->addr, Param->port);
		return -1;
	}
	LogMsg( "Tcp_OpenServer success. addr=[%s] port=[%d] sock=[%p]", Param->addr, Param->port, TCP_SOCK( Server));

	LogDbg( "listen ...");
	rtn = Tcp_Listen( Server);
	if( rtn < 0)
	{
		LogCri( "Tcp_Listen error. rtn=[%d]", rtn);
		return -1;
	}

	while( Continue)
	{
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		nfds = 0;
		TCP_FD_ZERO( &rfds);
		TCP_FD_SET( Server, &rfds);
		nfds = TCP_SOCK( Server);

		for( i = 0; i < MAX_CLIENT; i++)
		{
			if( Client[ i] == NULL) 	continue;
			TCP_FD_SET( Client[ i], &rfds);
			nfds = Max( nfds, TCP_SOCK( Client[ i]));
		}

		LogDbg( "select ... wait=[%d.%dsec]", tv.tv_sec, tv.tv_usec);
		rtn = select( nfds +1, &rfds, NULL, NULL, &tv);
		if( rtn < 0)
		{
			LogErr( "select error. rtn=[%d]", rtn);
			goto error_1;
		}
		else
		if( rtn == 0)
		{
			LogDbg( "timeout ... ");
			continue;
		}

		if( TCP_FD_ISSET( Server, &rfds))
		{
			LogDbg( "socket event ... server socket");
			LogDbg( "accept ...");
			client = Tcp_Accept( Server);
			if( client == NULL)
			{
				LogCri( "Tcp_Accept error. sock=[%d]", TCP_SOCK( Server));
				continue;
			}
			LogDbg( "accepted new socket=[%d] ", TCP_SOCK( client));

			for( i = 0; i < MAX_CLIENT; i++) 
			{
				if( Client[ i] == NULL) break;
				LogMsg( "already connect sock[%d] = [%d]", TCP_SOCK( Client[i]));
			}

			if( i >= MAX_CLIENT)
			{
				LogCri( "too many connections!!! MAX=[%d] connection=[%d]", MAX_CLIENT, i); 
				Tcp_Close( client);
				continue;
			}

			Client[ i] = client;
			// clrscr();
		}
		for( i = 0; i < MAX_CLIENT; i++)
		{
			if( TCP_FD_ISSET( Client[ i], &rfds))
			{
				LogDbg( "socket event ... client socket. sock=[%d]", TCP_SOCK( Client[ i]));
				rtn = DataProcess( Client[ i]);
				if( rtn <= 0)
				{
					Tcp_Close( Client[ i]);
					Client[ i] = NULL;

				}
			}
		}
		
	}

	return 1;

	error_1:
		for( i = 0; i < MAX_CLIENT; i++) 
		{
			if( Client[ i] != NULL) 
			{
				Tcp_Close( Client[ i]);
				Client[ i] = NULL;
			}
		}
		Tcp_Close( Server);
		return -1;
}

/** ***************************************************************************
**  @fn         int ServerProcess( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  프로그램 수행
***************************************************************************** */
int DataProcess( TCP *tcp)
{
	int				rtn, sz;
	FIX_COMM_HEAD	*head = Recv;

	rtn = RecvProcess( tcp);
	if( rtn <= 0)
	{
		return rtn;
	}

	if( !memcmp( head->message_type, "LINK", 4))
	{
		Cfg_Get( Cfg, "link", Send, 65535);
		sz = strlen( Send);
	}
	else
	if( !memcmp( head->message_type, "POLL", 4))
	{
		Cfg_Get( Cfg, "poll", Send, 65535);
		sz = strlen( Send);
	}
	else
	{
		sz = 0;
	}

	if( sz <= 0) return 1;

	rtn = Tcp_SendN( tcp, Send, sz);
	if( rtn < sz)
	{
		LogCri( "Tcp_SendN error. sz=[%d] rtn=[%d]", sz, rtn);
		return -1;
	}

	return rtn;
}


int RecvProcess( TCP *tcp)
{
	int		rtn;
	int		sz;

	rtn = Tcp_RecvN( tcp, Recv, 4);
	if( rtn < 4)
	{
		LogCri( "Tcp_RecvN error. sz=[4] rtn=[%d]", rtn);
		return -1;
	}
	Recv[ 4] = 0;

	sz = atoi( Recv);

	rtn = Tcp_RecvN( tcp, &Recv[ 4], sz);
	if( rtn < sz)
	{
		LogCri( "Tcp_RecvN error. sz=[%d] rtn=[%d]", sz, rtn);
		return -1;
	}


	return sz +4;
}

