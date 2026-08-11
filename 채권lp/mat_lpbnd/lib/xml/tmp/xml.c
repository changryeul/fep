#include <stdio.h>

#include "log.h"
#include "xml.h"

XML_DATA *Xml_Open( const char *incode)
{
	XML_DATA	*dp;

	dp = malloc( sizeof( XML_DATA));
	if( dp == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( XML_DATA));
		return NULL;
	}
	memset( dp, 0x00, sizeof( XML_DATA));

	dp->parser = XML_ParserCreate( incode);
	XML_SetUserData( dp->parser, dp);
	XML_SetElementHandler( dp->parser, Xml_Start, Xml_End);
	XML_SetCharacterDataHandler( dp->parser, Xml_Value);

/*
	dp->key = malloc( sizeof( char *) * XML_MAX_DEPTH);
	if( dp->key == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( char *) * XML_MAX_DEPTH);
		return NULL;
	}
	memset( dp->key, 0x00, sizeof( XML_DATA));
*/

	return dp;
}

int	Xml_AddName( XML_DATA *dp, const char *name, int sz)
{
	char			key[ 512];
	int				i, key_sz = 0;
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

	dp->key = realloc( dp->key, sizeof( char *) * ( mp->depth +1));
	if( dp->key == NULL)
	{
		LogErr( "realloc error. ptr=[%p] sz=[%d]", dp->key, sizeof( char *) * ( mp->depth +1));
		goto error_2;
	}
	dp->key[ mp->depth] = mp->name;

	for( i = 0; i <= mp->depth; i++)
	{
		LogDel( "%s", dp->key[i]);
		key_sz += sprintf( &key[ key_sz], "%s/", dp->key[ i]);
	}
	if( key_sz > 0)
	{
		mp->key = malloc( key_sz);
		if( mp->key == NULL)
		{
			LogErr( "malloc error. key_sz=[%d]", key_sz);
			goto error_2;
		}
		memcpy( mp->key, key, key_sz -1);
		mp->key[ key_sz -1] = 0;
		LogDel( "depth=[%d] sz=[%d] key=[%s]", mp->depth, key_sz, mp->key);
	}
	else
	{
		mp->key = NULL;
	}

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

int Xml_AddValue( XML_DATA *dp, const char *value, int sz)
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

void Xml_Start(void *userData, const XML_Char *name, const XML_Char **atts) 
{
	int			i;
	XML_DATA	*dp = ( XML_DATA *)userData;

	Xml_AddName( dp, name, strlen( name));
	dp->depth++;
}

void Xml_End(void *userData, const XML_Char *name) 
{
	XML_DATA	*dp = ( XML_DATA *)userData;

	dp->depth--;
	LogDbg( "depth=[%d]", dp->depth);
	if( dp->depth == 0) XML_StopParser( dp->parser, 1);
}

void Xml_Value( void *userData, const XML_Char *val, int len)
{
	int 		i;
	char		cpy[ 128] = {};
	XML_DATA	*dp = ( XML_DATA *)userData;

	Xml_AddValue( dp, val, len);
}


int Xml_Close( XML_DATA *dp)
{
	int			i, col = 100;
	XML_MEMBER	*mp;

	for( i = 0; i < dp->cnt; i++)
	{
		mp = dp->member[ i];
		if( mp == NULL) continue;
		if( mp->key != NULL) free( mp->key);
		if( mp->name != NULL) free( mp->name);
		if( mp->value != NULL) free( mp->value);
		free( mp);
	}
	XML_ParserFree( dp->parser);

	free( dp->member);
	free( dp->key);
	free( dp);

	return 1;
}

int Xml_Print( XML_DATA *dp)
{
	int			i, col = 100;
	XML_MEMBER	*mp;

	LogRaw( "cnt=[%d]\n", dp->cnt);
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");
	LogRaw( "%-5s", "depth ");
	LogRaw( "%-32s ", "name");
	LogRaw( "%3s ", " sz");
	LogRaw( "[%s] ", "value");
	LogRaw( "\n");
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");

	for( i = 0; i < dp->cnt; i++)
	{
		mp = dp->member[ i];
		LogRaw( "%5d ", mp->depth);
		LogRaw( "%-32s ", mp->name);
		LogRaw( "%3d ", mp->vsz);
		LogRaw( "[%s] ", mp->value);
		LogRaw( "\n");
	}
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");

	return 1;
}

int Xml_Print2( XML_DATA *dp)
{
	int			i, j, col = 100;
	XML_MEMBER	*mp;
	char		*np[ 256];
	int			sz;
	char		name[ 512];

	LogRaw( "cnt=[%d]\n", dp->cnt);
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");
	LogRaw( "%-5s", "depth ");
	LogRaw( "%-64s ", "name");
	LogRaw( "%3s ", " sz");
	LogRaw( "[%s] ", "value");
	LogRaw( "\n");
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");

	for( i = 0; i < dp->cnt; i++)
	{
		mp = dp->member[ i];
		np[ mp->depth] = mp->name;
		LogRaw( "%5d ", mp->depth);
		sz = 0;
		for( j = 0; j <= mp->depth; j++)
			sz += sprintf( &name[ sz], "%s/", np[ j]);
		name[ sz -1] = 0;
		LogRaw( "%-64s ", name);
		LogRaw( "%3d ", mp->vsz);
		LogRaw( "[%s] ", mp->value);
		LogRaw( "\n");
	}
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");

	return 1;
}

int Xml_Proc( XML_DATA *dp, char *data, int sz, int f)
{
	int		rtn;

	dp->data = XML_GetBuffer( dp->parser, dp->len +sz);
	dp->len += sz;
	memcpy( &dp->data[ dp->len -sz], data, sz);

	rtn = XML_ParseBuffer( dp->parser, dp->len, f);
	if( rtn == XML_STATUS_ERROR)
	{
		LogCri( "XML_Parse error. at=[%d:%d] pos=[%d] err=[%d:%s]",
				XML_GetCurrentLineNumber( dp->parser), XML_GetCurrentColumnNumber( dp->parser),
				XML_GetCurrentByteCount( dp->parser),
				XML_GetErrorCode(dp->parser), XML_ErrorString( XML_GetErrorCode(dp->parser)));
	}

	return rtn;
}

int Xml_FindKey( XML_DATA *dp, char *dest, char *key)
{
	int			i, sz;
	XML_MEMBER	*mp;

	sz = strlen( key);

	LogDbg( "Find key=[%s]", key);

	for( i = 0; i < dp->cnt; i++)
	{
		mp = dp->member[ i];
		LogDel( "    compare mp->key=[%s]", mp->key);
		if( !memcmp( mp->key, key, sz))
		{
			memcpy( dest, mp->value, mp->vsz);
			dest[ mp->vsz] = 0;
			return mp->vsz;
		}
	}

	LogDbg( "data not found. key=[%s]", key);
	return 0;
}

int XML_FindKey( XML_DATA *dp, char *key, char *dest)
{
	return Xml_FindKey( dp, dest, key);
}


