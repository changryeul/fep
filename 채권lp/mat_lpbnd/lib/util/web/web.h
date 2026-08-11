#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef WEB_H
#define	WEB_H

#define	WEB_MAX_BUFF	8192

typedef struct _web_struct_
{
	char	*addr;
	int		port;
	int		sock;
	int		client;
	char	recv[ WEB_MAX_BUFF];
	int		recv_sz;
	char	path[ 512];
	int		path_sz;
}	WEB;


#endif	/* WEB_H */

/***** Module : web.c *****/
WEB*        Web_Open( char *addr, int port);
int         Web_Server( WEB *web);
int         Web_WaitChild( WEB *web);
int         Web_Service( WEB *web, int sock);
int         Web_GetRequest( WEB *web);
int         Web_PutMessage( WEB *web, int msg_no);
int         Web_ServiceHtml( WEB *web, char *name, int sz);

