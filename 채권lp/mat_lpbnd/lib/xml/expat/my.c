/* This is simple demonstration of how to use expat. This program
   reads an XML document from standard input and writes a line with
   the name of each element to standard output indenting child
   elements by one tab stop more than their parent element.
   It must be used with Expat compiled for UTF-8 output.
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
                        \___/_/\_\ .__/ \__,_|\__|
                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2001-2003 Fred L. Drake, Jr. <fdrake@users.sourceforge.net>
   Copyright (c) 2004-2006 Karl Waclawek <karl@waclawek.net>
   Copyright (c) 2005-2007 Steven Solie <steven@solie.ca>
   Copyright (c) 2016-2019 Sebastian Pipping <sebastian@pipping.org>
   Copyright (c) 2017      Rhodri James <rhodri@wildebeest.org.uk>
   Copyright (c) 2019      Zhongyuan Zhou <zhouzhongyuan@huawei.com>
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the
   "Software"),  to  deal in  the  Software  without restriction,  including
   without  limitation the  rights  to use,  copy,  modify, merge,  publish,
   distribute, sublicense, and/or sell copies of the Software, and to permit
   persons  to whom  the Software  is  furnished to  do so,  subject to  the
   following conditions:

   The above copyright  notice and this permission notice  shall be included
   in all copies or substantial portions of the Software.

   THE  SOFTWARE  IS  PROVIDED  "AS  IS",  WITHOUT  WARRANTY  OF  ANY  KIND,
   EXPRESS  OR IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO  THE WARRANTIES  OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
   NO EVENT SHALL THE AUTHORS OR  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
   DAMAGES OR  OTHER LIABILITY, WHETHER  IN AN  ACTION OF CONTRACT,  TORT OR
   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
   USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <expat.h>
#include "log.h"

#ifdef XML_LARGE_SIZE
#  define XML_FMT_INT_MOD "ll"
#else
#  define XML_FMT_INT_MOD "l"
#endif

#ifdef XML_UNICODE_WCHAR_T
#  include <wchar.h>
#  define XML_FMT_STR "ls"
#else
#  define XML_FMT_STR "s"
#endif

typedef struct _xml_stack_
{
	int		depth;				/* xml depth */
	char	*name;				/* xml name */
	int		nsz;				/* xml name size */
	char	*value;				/* xml value */
	int		vsz;				/* xml value size */
}	XML_MEMBER;

typedef struct _xml_data_
{
	int			cnt;			/* member count */
	XML_MEMBER	**member;		/* member pointer */
	int			depth;
}	XML_DATA;

XML_DATA	*XmlData;

XML_DATA *Xml_Open()
{
	XML_DATA	*dp;

	dp = malloc( sizeof( XML_DATA));
	if( dp == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( XML_DATA));
		return NULL;
	}
	memset( dp, 0x00, sizeof( XML_DATA));

	return dp;
}

int	Xml_AddName( XML_DATA *dp, char *name, int sz)
{
	XML_MEMBER		*mp;

	mp = malloc( sizeof( XML_MEMBER));
	if( mp == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( XML_MEMBER));
		goto error;
	}
	memset( mp, 0x00, sizeof( XML_MEMBER));

	mp->name = malloc( sz +1);
	if( mp->name == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sz +1);
		goto error_1;
	}
	memcpy( mp->name, name, sz);
	mp->name[ sz] = 0;
	mp->nsz = sz;
	mp->depth = dp->depth;

	dp->cnt++;
	dp->member = realloc( dp->member, sizeof( XML_MEMBER *) * dp->cnt);
	if( dp->member == NULL)
	{
		LogErr( "realloc error. dp->member=[%p] sz=[%d]", dp->member, sizeof( XML_MEMBER *) * dp->cnt);
		dp->cnt--;
		goto error_1;
	}
	dp->member[ dp->cnt -1] = mp;

	return 1;

	error_2:
		free( mp->name);
	error_1:
		free( mp);
	error:
		return -1;

}

int Xml_AddValue( XML_DATA *dp, char *value, int sz)
{
	XML_MEMBER	*mp;

	mp = dp->member[ dp->cnt -1];

	mp->value = malloc( sz +1);
	if( mp->value == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sz +1);
		goto error;
	}
	memcpy( mp->value, value, sz);
	mp->value[ sz] = 0;
	mp->vsz = sz;

	return 1;

	error:
		return -1;

}

int Xml_Close( XML_DATA *dp)
{
	int			i, col = 100;
	XML_MEMBER	*mp;

	for( i = 0; i < dp->cnt; i++)
	{
		mp = dp->member[ i];
		if( mp == NULL) continue;
		if( mp->name != NULL) free( mp->name);
		if( mp->value != NULL) free( mp->value);
		free( mp);
	}
	free( dp);

	return 1;
}

int Xml_Print( XML_DATA *dp)
{
	int			i, col = 100;
	XML_MEMBER	*mp;

	LogRaw( "cnt=[%d]\n", dp->cnt);
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");

	for( i = 0; i < dp->cnt; i++)
	{
		mp = dp->member[ i];
		LogRaw( "%d ", mp->depth);
		LogRaw( "%-32s ", mp->name);
		LogRaw( "%s ", mp->value);
		LogRaw( "\n");
	}
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");

	return 1;
}

void XmlStart(void *userData, const XML_Char *name, const XML_Char **atts) 
{
	int			i;
	XML_DATA	*dp = ( XML_DATA *)userData;

	dp->depth++;
	Xml_AddName( dp, name, strlen( name));
}

void XmlEnd(void *userData, const XML_Char *name) 
{
	XML_DATA	*dp = ( XML_DATA *)userData;

	dp->depth--;
}

void XmlValue( void *userData, const XML_Char *val, int len)
{
	int 		i;
	char		cpy[ 128] = {};
	XML_DATA	*dp = ( XML_DATA *)userData;

	Xml_AddValue( dp, val, len);
}

int sample(int argc, char *argv[]) 
{
	int			rtn;
	char		*ptr;
	char 		buf[8192];
	XML_Parser 	pb;
	XML_DATA	*dp;
	int 		done;
	int 		depth = 0;

	dp = Xml_Init();
	if( dp == NULL)
	{
		LogCri( "Xml_Init error. ");
		goto error;
	}

	pb = XML_ParserCreate( NULL);

	XML_SetUserData( pb, dp);
	XML_SetElementHandler( pb, XmlStart, XmlEnd);
	XML_SetCharacterDataHandler( pb, XmlValue);
	do 
	{
		ptr = fgets( buf, 8192, stdin);
		if( ptr == NULL) break;

		rtn = XML_Parse( pb, buf, strlen( buf), done);
		if( rtn == XML_STATUS_ERROR) 
		{
			LogCri( "XML_Parse error. at=[%d:%d] pos=[%d] err=[%d:%s]",
				XML_GetCurrentLineNumber( pb), XML_GetCurrentColumnNumber( pb),
				XML_GetCurrentByteCount( pb),
				XML_GetErrorCode(pb), XML_ErrorString( XML_GetErrorCode(pb)));
			goto error_1;
		}
		Xml_Print( dp);
	} while (! done);

	XML_ParserFree( pb);

	return 1;

	error_1:
		XML_ParserFree( pb);
	error:
		LogDbg( "sample end by error. ");
		return -1;
}

