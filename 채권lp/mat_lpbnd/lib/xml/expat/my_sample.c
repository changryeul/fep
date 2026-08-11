#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "expat.h"

void XmlStart( void *data, const XML_Char *name, const XML_Char **atts)
{
}

void XmlEnd( void *data, const XML_Char *name)
{
}

int sample( int argc, char *argv[])
{
	int			rtn;
	int			done;
	XML_Parser	pp;
	char		data[ 65535];

	fgets( data, 65535, stdin);

	pp = XML_ParserCreate( NULL);
	if( pp == NULL)
	{
		LogErr( "XML_ParserCreate error.");
		goto error;
	}

#if 0
	XML_SetStartElementHandler( pp, XmlStart);
	XML_SetEndElementHandler( pp, XmlEnd);
#else
	XML_SetElementHandler( pp, XmlStart, XmlEnd);
#endif

	rtn = XML_Parse( pp, data, strlen( data), done);
	if( rtn == XML_STATUS_ERROR)
	{
		LogCri( "XML_Parse error. at=[%d:%d] pos=[%d] err=[%d:%s]", 
				XML_GetCurrentLineNumber( pp), XML_GetCurrentColumnNumber( pp), 
				XML_GetCurrentByteCount( pp),
				XML_GetErrorCode(pp), XML_ErrorString( XML_GetErrorCode(pp)));
		goto error_1;
	}



	return 1;

	error_1:
		XML_ParserFree( pp);
	error:
		return -1;
}


