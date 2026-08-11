/** ***************************************************************************
**  @file       log.h
**  @date       2022/08/23
**  @author     최동춘
**  @version    V2.0.20220823
**  @brif
**  MCP/IP socket 통신 라이브러리
**	MCP header 정의
***************************************************************************** */
#ifndef MCP_H
#define	MCP_H 1

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

#define	MAX_BACKLOG		5
// #define MAX_CLIENT		10
#define	MAX_BUF_SZ		8192

#define MCP_FD_ZERO( x)				FD_ZERO( x)
#define MCP_FD_SET( x, y)			FD_SET( x->sock, y)
#define MCP_FD_ISSET( x, y)			FD_ISSET( x->sock, y)
#define	MCP_SOCK( x)				x->sock

/** ***************************************************************************
**  @struct		MCP
**  @brief      MCP/IP 기본 구조체
**              20230117 - stp_trtn을 위해서 sel_to을 추가 
**                         연속된 heartbeat가 오는것을 끊기 위해서 timout을 짧게 구현
***************************************************************************** */
typedef struct _mcp_
{
	int					sock;		/** @var socker descripter */
	char				*addr;		/** @var argument addr */
	int					port;		/** @var argument port */
	struct sockaddr_in	svr_addr;	/** @var 서버 정보 */
	struct sockaddr_in	cli_addr;	/** @var 클라이언트 정보 */
	struct ip_mreq		mreq;		/** @var 멀티캐스트 그룹 정보 */
	int					to;			/** @var timeout - micro second */
	int					sel_to;		/** @var select timeout - 연속 데이타 수신시 타임 아웃 */
	int					backlog;	/** @var listen backlog */
	char				rec[ 512];	/** @var temp buffer */
}	MCP;

#endif

/***** Module : mcp.c *****/
MCP*        Mcp_Open( char *addr, int port);                                /* Mcp_OpenServer, Mcp_OpenClient가 사용할 socket을 open, 구조체 malloc */
int         Mcp_Close( MCP *mcp);                                           /* socket close and MCP 구조체 free */
MCP*        Mcp_OpenServer( char *addr, int port);                          /* MCP SERVER 소켓 OPEN */
MCP*        Mcp_OpenClient( char *addr, int port);                          /* MCP CLIENT 소켓 OPEN */
int         Mcp_Listen( MCP *mcp);                                          /* listen 함수 call */
MCP*        Mcp_Accept( MCP *mcp);                                          /* client의 접속을 허가 ... mcp->cli_addr에 client 정보 setting */
int         Mcp_Info( MCP *mcp);                                            /* client의 접속을 허가 ... mcp->cli_addr에 client 정보 setting */
int         Mcp_SetTimeout( MCP *mcp, int to);                              /* multicast library */
int         Mcp_SetSelectTimeout( MCP *mcp, int to);
char*       Mcp_GetAddr( MCP *mcp);                                         /* multicast library */
int         Mcp_GetPort( MCP *mcp);                                         /* multicast library */
char*       Mcp_GetServerAddr( MCP *mcp);                                   /* multicast library */
int         Mcp_GetServerPort( MCP *mcp);                                   /* multicast library */
char*       Mcp_GetServAddr( MCP *mcp);                                     /* multicast library */
int         Mcp_GetServPort( MCP *mcp);                                     /* multicast library */
char*       Mcp_GetPeerAddr( MCP *mcp);                                     /* multicast library */
int         Mcp_GetPeerPort( MCP *mcp);                                     /* multicast library */
int         Mcp_Send( MCP *mcp, char *rec, int sz);                         /* multicast library */
int         Mcp_SendTo( MCP *mcp, char *rec, int sz);                       /* multicast library */
int         Mcp_SendN( MCP *mcp, char *rec, int sz);                        /* multicast library */
int         Mcp_SendNT( MCP *mcp, char *rec, int sz, int to);               /* multicast library */
int         Mcp_Recv( MCP *mcp, char *rec, int sz);                         /* multicast library */
int         Mcp_RecvN( MCP *mcp, char *rec, int sz);                        /* multicast library */
int         Mcp_RecvNT( MCP *mcp, char *rec, int sz, int to);               /* multicast library */
int         Mcp_RecvT( MCP *mcp, char *rec, int sz, int to);                /* multicast library */
int         Mcp_GetEvent( MCP *mcp, int to);                                /* multicast library */

