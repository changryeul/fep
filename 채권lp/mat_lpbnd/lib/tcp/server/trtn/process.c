#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "cfg.h"
#include "etc.h"
#include "tcp.h"
#include "xml.h"

extern	CFG				*Cfg;
#define MAX_CLIENT		1

/***** Module : server.c *****/
int         DataProcess( TCP *tp, char *rec, int sz);

static void XMLCALL StartElement( void *data, const char *name, const char **atts)
{
	int		i;
	int		*dp;

	dp = data;

	for( i = 0; i < *dp; i++)
	{
		putchar( '\t');
	}
	puts( name);
	*dp += 1;
}

static void XMLCALL EndElement( void *data, const char *name)
{
	int 	*dp;

	dp = data;
	*dp -= 1;
}

DataProcess( TCP *tp, char *rec, int sz)
{
	int			rtn, len;
	static int	stat = 0;
	char		send_data[ 8192];
	XML_Parser	*parser;
	int			depth = 0;

	rtn = Tcp_Recv( tp, rec, sz);
	if( rtn <= 0) return rtn;
	XmlProcess( rec, rtn);

	switch( stat)
	{
		case 0:
			len = Cfg_Get( Cfg, "heart_beat", send_data, 8192);
			break;
		default:
			len = Cfg_Get( Cfg, "data", send_data, 8192);
			break;
	}
	Tcp_Send( tp, send_data, len);
	stat++;

	return rtn;
}

XmlProcess( char *data, int sz)
{
	int			rtn;
	XML_DATA	*dp;

	dp = Xml_Open( NULL);
	rtn = Xml_Proc( dp, data, sz);
	Xml_Print( dp);

	Xml_Close( dp);

	return 1;
}


