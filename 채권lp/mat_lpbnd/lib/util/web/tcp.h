#ifndef TCP_H
#define	TCP_H 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "log.h"

#define	MAX_BACKLOG		5
#define MAX_CLIENT		10
#define	MAX_BUF_SZ		8192

#define TCP_MAX( x, y)		((( x > y) ? x : y))


typedef struct _tcp_
{
	int					sock;
	struct sockaddr_in	svr_addr;
	struct sockaddr_in	cli_addr;
}	TCP;

#endif

/***** Module : tcp.c *****/
int         Tcp_OpenServer( char *addr, int port);
int         Tcp_Connect( char *addr, int port);
int         Tcp_Server( int sock, int (*recv_func)( int sock, char *rec, int sz));
int         Tcp_Close( int sock);
TCP*        Tcp_MakeSock( TCP *sv_tcp);
int         Tcp_Accept( int sv_sock);
int         Tcp_SendN( int sock, char *rec, int sz);
int         Tcp_SendNT( int sock, char *rec, int sz, int to);
int         Tcp_RecvN( int sock, char *rec, int sz);
int         Tcp_RecvNT( int sock, char *rec, int sz, int to);

