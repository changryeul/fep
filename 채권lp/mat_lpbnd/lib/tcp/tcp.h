/** ***************************************************************************
**  @file       log.h
**  @date       2022/08/23
**  @author     최동춘
**  @version    V2.0.20220823
**  @brif
**  TCP/IP socket 통신 라이브러리
**	TCP header 정의
***************************************************************************** */
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
// #define MAX_CLIENT		10
#define	MAX_BUF_SZ		8192

#define TCP_FD_ZERO( x)				FD_ZERO( x)
#define TCP_FD_SET( x, y)			FD_SET( x->sock, y)
#define TCP_FD_ISSET( x, y)			FD_ISSET( x->sock, y)
#define	TCP_SOCK( x)				x->sock

/** ***************************************************************************
**  @struct		TCP
**  @brief      TCP/IP 기본 구조체
**              20230117 - stp_trtn을 위해서 sel_to을 추가 
**                         연속된 heartbeat가 오는것을 끊기 위해서 timout을 짧게 구현
***************************************************************************** */
typedef struct _tcp_
{
	int					sock;		/** @var socker descripter */
	char				*addr;		/** @var argument addr */
	int					port;		/** @var argument port */
	struct sockaddr_in	svr_addr;	/** @var 서버 정보 */
	struct sockaddr_in	cli_addr;	/** @var 클라이언트 정보 */
	int					to;			/** @var timeout - micro second */
	int					sel_to;		/** @var select timeout - 연속 데이타 수신시 타임 아웃 */
	int					backlog;	/** @var listen backlog */
	int					debug;		/** @val tcp debug level 1:data 2:cha 3:dec 4:hex */
	char				rec[ 512];	/** @var temp buffer */
}	TCP;

#endif

/***** Module : tcp.c *****/
TCP*        Tcp_Open( char *addr, int port);                                /* Tcp_OpenServer, Tcp_OpenClient가 사용할 socket을 open, 구조체 malloc */
int         Tcp_Close( TCP *tcp);                                           /* socket close and TCP 구조체 free */
TCP*        Tcp_OpenServer( char *addr, int port);                          /* TCP SERVER 소켓 OPEN */
TCP*        Tcp_OpenClient( char *addr, int port);                          /* TCP CLIENT 소켓 OPEN */
int         Tcp_Listen( TCP *tcp);                                          /* listen 함수 call */
TCP*        Tcp_Accept( TCP *tcp);                                          /* client의 접속을 허가 ... tcp->cli_addr에 client 정보 setting */
int         Tcp_SetDebug( TCP *tcp, int opt);                               /* debug set --- 0:no_debug 1:debug */
int         Tcp_Info( TCP *tcp);                                            /* client의 접속을 허가 ... tcp->cli_addr에 client 정보 setting */
int         Tcp_SetTimeout( TCP *tcp, int to);
int         Tcp_SetSelectTimeout( TCP *tcp, int to);
char*       Tcp_GetAddr( TCP *tcp);
int         Tcp_GetPort( TCP *tcp);
char*       Tcp_GetServerAddr( TCP *tcp);
int         Tcp_GetServerPort( TCP *tcp);
char*       Tcp_GetServAddr( TCP *tcp);
int         Tcp_GetServPort( TCP *tcp);
char*       Tcp_GetPeerAddr( TCP *tcp);
int         Tcp_GetPeerPort( TCP *tcp);
int         Tcp_Send( TCP *tcp, char *rec, int sz);
int         Tcp_SendN( TCP *tcp, char *rec, int sz);
int         Tcp_SendNT( TCP *tcp, char *rec, int sz, int to);
int         Tcp_Recv( TCP *tcp, char *rec, int sz);
int         Tcp_RecvN( TCP *tcp, char *rec, int sz);
int         Tcp_RecvNT( TCP *tcp, char *rec, int sz, int to);
int         Tcp_RecvT( TCP *tcp, char *rec, int sz, int to);
int         Tcp_GetEvent( TCP *tcp, int to);

