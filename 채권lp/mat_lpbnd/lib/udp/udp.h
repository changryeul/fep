/** ***************************************************************************
**  @file       log.h
**  @date       2022/08/23
**  @author     최동춘
**  @version    V2.0.20220823
**  @brif
**  UDP/IP socket 통신 라이브러리
**	UDP header 정의
***************************************************************************** */
#ifndef UDP_H
#define	UDP_H 1

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

#define UDP_FD_ZERO( x)				FD_ZERO( x)
#define UDP_FD_SET( x, y)			FD_SET( x->sock, y)
#define UDP_FD_ISSET( x, y)			FD_ISSET( x->sock, y)
#define	UDP_SOCK( x)				x->sock

/** ***************************************************************************
**  @struct		UDP
**  @brief      UDP/IP 기본 구조체
**              20230117 - stp_trtn을 위해서 sel_to을 추가 
**                         연속된 heartbeat가 오는것을 끊기 위해서 timout을 짧게 구현
***************************************************************************** */
typedef struct _udp_
{
	int					sock;		/** @var socker descripter */
	char				*addr;		/** @var argument addr */
	int					port;		/** @var argument port */
	struct sockaddr_in	svr_addr;	/** @var 서버 정보 */
	struct sockaddr_in	cli_addr;	/** @var 클라이언트 정보 */
	int					to;			/** @var timeout - micro second */
	int					sel_to;		/** @var select timeout - 연속 데이타 수신시 타임 아웃 */
	char				rec[ 512];	/** @var temp buffer */
}	UDP;

#endif

/***** Module : udp.c *****/
UDP*        Udp_Open( char *addr, int port);
int         Udp_Close( UDP *udp);
UDP*        Udp_OpenServer( char *addr, int port);
UDP*        Udp_OpenClient( char *addr, int port);
int         Udp_Listen( UDP *udp);
UDP*        Udp_Accept( UDP *udp);
int         Udp_Info( UDP *udp);
int         Udp_SetTimeout( UDP *udp, int to);
int         Udp_SetSelectTimeout( UDP *udp, int to);
char*       Udp_GetAddr( UDP *udp);
int         Udp_GetPort( UDP *udp);
char*       Udp_GetServerAddr( UDP *udp);
int         Udp_GetServerPort( UDP *udp);
char*       Udp_GetServAddr( UDP *udp);
int         Udp_GetServPort( UDP *udp);
char*       Udp_GetPeerAddr( UDP *udp);
int         Udp_GetPeerPort( UDP *udp);
int         Udp_Send( UDP *udp, char *rec, int sz);
int         Udp_SendN( UDP *udp, char *rec, int sz);
int         Udp_SendNT( UDP *udp, char *rec, int sz, int to);
int         Udp_Recv( UDP *udp, char *rec, int sz);
int         Udp_RecvN( UDP *udp, char *rec, int sz);
int         Udp_RecvNT( UDP *udp, char *rec, int sz, int to);
int         Udp_RecvT( UDP *udp, char *rec, int sz, int to);
int         Udp_GetEvent( UDP *udp, int to);

