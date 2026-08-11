#include <stdio.h>

#include "log.h"
#include "etc.h"
#include "xml.h"

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      char * incode - NULL(UTF-8)
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering을 위한 open
***************************************************************************** */
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
	dp->data = NULL;
	dp->len = 0;

	return dp;
}

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
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

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
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

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
int Xml_AddTrimValue( XML_DATA *dp, const char *value, int sz)
{
	XML_MEMBER	*mp;
	char		trim[ 8192];
	int			t_sz;

	memcpy( trim, value, sz +1);
	trim[ sz] = 0;
	t_sz = TrimN( trim, sizeof( trim));
	mp = dp->member[ dp->cnt -1];

	mp->value = malloc( t_sz +1);
	if( mp->value == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sz +1);
		goto error;
	}
	memcpy( mp->value, trim, t_sz);
	mp->value[ t_sz] = 0;
	mp->vsz = t_sz;

	return 1;

	error:
		return -1;

}

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
void Xml_Start(void *userData, const XML_Char *name, const XML_Char **atts) 
{
	// int			i;
	XML_DATA	*dp = ( XML_DATA *)userData;

	Xml_AddName( dp, name, strlen( name));
	dp->depth++;
}

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
void Xml_End(void *userData, const XML_Char *name) 
{
	XML_DATA	*dp = ( XML_DATA *)userData;

	dp->depth--;
	LogDel( "depth=[%d]", dp->depth);
	if( dp->depth == 0) 
	{
		dp->end = 1;
	}
}

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
void Xml_Value( void *userData, const XML_Char *val, int len)
{
	// int 		i;
	// char		cpy[ 128] = {};
	XML_DATA	*dp = ( XML_DATA *)userData;

	/* Xml_AddValue( dp, val, len); */
	Xml_AddTrimValue( dp, val, len);
}


/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
int Xml_Close( XML_DATA *dp)
{
	int			i; 
	// int			col = 100;
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

	if( dp->data != NULL) free( dp->data);
	if( dp->remain != NULL) free( dp->remain);
	free( dp->member);
	free( dp->key);
	free( dp);

	return 1;
}

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
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

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
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

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
int Xml_PrintFile( XML_DATA *dp, FILE *fp)
{
	int			i, j, col = 100;
	XML_MEMBER	*mp;
	char		*np[ 256];
	int			sz;
	char		name[ 512];

	fprintf( fp, "cnt=[%d]\n", dp->cnt);
	for( i = 0; i < col; i++) fprintf( fp, "-");
	fprintf( fp, "\n");

	fprintf( fp, "%-6s", " cnt ");
	fprintf( fp, "%-5s", "depth ");
	fprintf( fp, "%-64s ", "name");
	fprintf( fp, "%3s ", " sz");
	fprintf( fp, "[%s] ", "value");
	fprintf( fp, "\n");
	for( i = 0; i < col; i++) fprintf( fp, "-");
	fprintf( fp, "\n");

	for( i = 0; i < dp->cnt; i++)
	{
		mp = dp->member[ i];
		np[ mp->depth] = mp->name;
		fprintf( fp, "%5d ", i);
		fprintf( fp, "%5d ", mp->depth);
		sz = 0;
		for( j = 0; j <= mp->depth; j++)
			sz += sprintf( &name[ sz], "%s/", np[ j]);
		name[ sz -1] = 0;
		fprintf( fp, "%-64s ", name);
		fprintf( fp, "%3d ", mp->vsz);
		fprintf( fp, "[%s] ", mp->value);
		fprintf( fp, "\n");
	}
	for( i = 0; i < col; i++) fprintf( fp, "-");
	fprintf( fp, "\n");
	fflush( fp);

	return 1;
}

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @param      char *data
**  @param      int sz
**  @param      int f -- final 여부
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
int Xml_Proc( XML_DATA *dp, char *data, int sz, int f)
{
	// char	*ptr;
	int		rtn;
	XML_ParsingStatus	status;

	LogDbg( "realloc call ... dp->data=[%p] dp->len=[%d]", dp->data, dp->len +sz);
	dp->data = realloc( dp->data, dp->len +sz);
	if( dp->data == NULL)
	{
		LogErr( "realloc error. sz=[%d]", dp->len +sz);
		return -1;
	}
	dp->len += sz;
	memcpy( &dp->data[ dp->len -sz], data, sz);

	rtn = XML_Parse( dp->parser, data, sz, f);
	if( rtn == XML_STATUS_ERROR)
	{
		LogMsg( "XML_Parse error. at=[%d:%d] pos=[%d] err=[%d:%s]",
				XML_GetCurrentLineNumber( dp->parser), XML_GetCurrentColumnNumber( dp->parser),
				XML_GetCurrentByteCount( dp->parser),
				XML_GetErrorCode(dp->parser), XML_ErrorString( XML_GetErrorCode(dp->parser)));
		switch( XML_GetErrorCode(dp->parser))
		{
			case 9:	/* junk after document element */
				Xml_StopProc( dp);
				return 1;
			default:
				return -1;
		}
	}

	LogDbg( "===== f           =[%d]", f);
	LogDbg( "===== dp->depth   =[%d]", dp->depth);
	if( ( f == 1) || ( dp->depth == 0))
	{
		XML_GetParsingStatus( dp->parser, &status);
		LogDbg( "parsing       =[%d]", status.parsing);
		LogDbg( "finalBuffer   =[%d]", status.finalBuffer);
		Xml_StopProc( dp);
		return 1;
	}

	return 0;
}

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
int Xml_StopProc( XML_DATA *dp)
{
	// XML_ParsingStatus status;

	dp->pos = ( int)XML_GetCurrentByteIndex( dp->parser);
	LogDel( "len=[%d]", dp->len);
	LogDel( "GetCurrentByteIndex=[%d]", dp->pos);

	dp->remain_len = dp->len - dp->pos;

	dp->remain = malloc( dp->remain_len +1);
	if( dp->remain == NULL)
	{
		LogErr( "malloc error. sz=[%d]", dp->remain_len +1);
		return -1;
	}
	memcpy( dp->remain, &dp->data[ dp->pos], dp->remain_len +1);

	dp->remain[ dp->remain_len] = 0;
	dp->data[ dp->len] = 0;

	LogDel( "remain=[%d:%s]", dp->remain_len, dp->remain);

	return 1;

}

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
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
			LogDbg( "Data found ... value=[%s] sz=[%d]", mp->value, mp->vsz);
			memcpy( dest, mp->value, mp->vsz);
			dest[ mp->vsz] = 0;
			return mp->vsz;
		}
	}

	LogDbg( "data not found. key=[%s]", key);
	return -1;
}

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
int XML_FindKey( XML_DATA *dp, char *key, char *dest)
{
	return Xml_FindKey( dp, dest, key);
}

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
int Xml_GetRemainLen( XML_DATA *dp)
{
	return dp->remain_len;
}

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
int Xml_GetRemain( XML_DATA *dp, char *data, int len)
{
	int		sz;

	sz = ( dp->remain_len <= len) ? dp->remain_len : len;

	memcpy( data, dp->remain, sz);
	data[ sz] = 0;
	return dp->remain_len;
}

/** ***************************************************************************
**  @fn         int Xml_()
**  @param      XML_DATA *dp
**  @return     XML_DATA pointer
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  HTTP parsering library
***************************************************************************** */
int Xml_GetData( XML_DATA *dp, char *data, int sz)
{
	if( dp->len >= sz)
	{
		LogCri( "size error. data_sz=%d] sz=[%d]", dp->len, sz);
		return -1;
	}
	memcpy( data, dp->data, dp->len);
	data[ dp->len] = 0;

	return dp->len;
}






