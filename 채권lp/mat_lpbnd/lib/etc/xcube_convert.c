#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include "cfg.h"
#include "log.h"
#include "xcube_convert.h"

static ENV	*DataEnv;

int FwClear( char *dst, int sz)
{
	LogDel( "FwClear dst=[%p] sz=[%d]", dst, sz);
	memset( dst, 0x00, sz);
	return sz;
}

int FwSpace( char *dst, int sz)
{
	LogDel( "FwSpace dst=[%p] sz=[%d]", dst, sz);
	memset( dst, 0x20, sz);
	return sz;
}

int FwFilcpy( char *dst, int dst_sz, char *src, int src_sz)
{
	int		sz, len;
	char	buf[ 512];

	memset( dst, 0x20, dst_sz);
	len = strlen( src);

	sz = ( dst_sz > len) ? len : dst_sz;
	memcpy( dst, src, sz);

	return sz;
}

int FwSprintf( char *dst, int dst_sz, char *format, ...)
{
	int			sz = 0;
	va_list		args;

	va_start( args, format);
	sz += vsprintf( dst, format, args);
	va_end( args);

	return sz;
}

void *FwMemcpy( char *dst, int dst_sz, char *src, int src_sz, int cpy_sz)
{
	void *rtn;
	int	sz;

	/*
	LogDel( "FwMemcpy dst=[%s] dst_sz=[%d] src=[%s] src_sz=[%d]", dst, dst_sz, src, src_sz);
	sz = ( dst_sz < cpy_sz) ? dst_sz : cpy_sz;
	LogDel( "sz = [%d]", sz);
	*/
	sz = cpy_sz;
	rtn =  memcpy( dst, src, sz);
	LogDel( "dst=[%s] src=[%s]", dst, src);

	return rtn;
}

void *FwMemcpyN( char *dst, int dst_sz, char *src, int src_sz, int cp_sz)
{
	void *rtn;
	int	sz;

	LogDel( "FwMemcpy dst=[%s] dst_sz=[%d] src=[%s] src_sz=[%d] cp_sz=[%d]", dst, dst_sz, src, src_sz, cp_sz);
	sz = ( dst_sz > cp_sz) ? cp_sz : dst_sz;
	LogDel( "cp_sz = [%d]", sz);
	rtn =  memcpy( dst, src, sz);
	LogDel( "dst=[%s] src=[%s]", dst, src);

	return rtn;
}

int FwMemcmp( char *dst, int dst_sz, char *src, int src_sz, int sz)
{
	int		rtn;
	rtn = memcmp( dst, src, sz);
	LogDel( "FwMemcmp dst=[%s] src=[%s] sz=[%d] rtn=[%d]", dst, src, sz, rtn);

	return rtn;
}

int FwStrcmp( char *dst, int dst_sz, char *src, int src_sz)
{
	int		rtn;
	int		sz;

	/* sz = ( dst_sz < sz) ? dst_sz : sz; */
	sz = strlen( src);
	rtn = memcmp( dst, src, sz);
	LogDel( "FwMemcmp dst=[%s] src=[%s] sz=[%d] rtn=[%d]", dst, src, sz, rtn);

	return rtn;
}

void *FwStrcpy( char *dst, int dst_sz, char *src, int src_sz)
{
	int		sz;

	/*
	if( dst == NULL) return 0;
	memset( dst, 0x00, dst_sz);
	if( src == NULL) return 0;

	sz = ( ( dst_sz -1) <= strlen( src)) ? dst_sz -1 : strlen( src);
	*/
	sz = strlen( src);
	return memcpy( dst, src, sz);
}


double FwAtoF( char *dst, int dst_sz)
{
	double	d;
	char	buf[ 512];

	memcpy( buf, dst, dst_sz);
	buf[ dst_sz] = 0;
	d = atof( buf);
	return d;
}

int FwAtoI( char *dst, int dst_sz)
{
	char	buf[ 512];

	memcpy( buf, dst, dst_sz);
	buf[ dst_sz] = 0;

	return atoi( buf);
}

int FwZItoA( char *dst, int dst_sz, int val)
{
	int		len;
	char	buf[ 512];

	len = sprintf( buf, "%0*d", dst_sz, val);
	memcpy( dst, buf, len);

	return len;
}


int GET_DATE( char *dst)
{
	time_t		cur_time;
	struct tm	*tp;

	time( &cur_time);
	tp = localtime( &cur_time);

	sprintf( dst, "%04d%02d%02d", tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday);

	return 1;
}

int IntToAsc( char *rec, int sz, int val)
{
	int		len;
	char	buf[ 512];

	len = sprintf( buf, "%0*d", sz, val);
	memcpy( rec, buf, len +1);

	return len;
}

int AscToInt( char *rec, int sz)
{
	int		val;
	char	buf[ 512];

	memcpy( buf, rec, sz);
	buf[ sz] = 0;
	val = atoi( buf);

	return val;
}

char *GetEnv( char *name, char *file)
{
	char	*val;

	return val;
}

int EnvOpen( char *file)
{
	LogWar( "not make EnvOpen");
#if 0
	LogDel( "EnvOpen. file=[%s]", file);
	DataEnv->cfg = Cfg_Open( file);
#endif
}

ENV *Env( ENV *env)
{
	DataEnv = malloc( sizeof( ENV));

	DataEnv->open = EnvOpen;

	env = DataEnv;
	return env;
}

char *PrgEnv( char *file)
{
}

int ToUpper( int arg)
{
	return toupper( arg);
}

int GetDate( char *rec)
{
	int			sz;
	time_t		cur_time;
	struct tm	*tp;

	time( &cur_time);
	tp = localtime( &cur_time);
	sz= sprintf( rec, "%04d%02d%02d", tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday);

	return sz;
}

int GetTime( char *rec)
{
	int			sz;
	time_t		cur_time;
	struct tm	*tp;

	time( &cur_time);
	tp = localtime( &cur_time);
	sz= sprintf( rec, "%02d%02d%02d", tp->tm_hour, tp->tm_min, tp->tm_sec);

	return sz;
}

int GetDateTime( char *rec)
{
	int			sz;
	time_t		cur_time;
	struct tm	*tp;

	time( &cur_time);
	tp = localtime( &cur_time);
	sz= sprintf( rec, "%04d%02d%02d%02d%02d%02d", 
			tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);

	LogDbg( "GetDateTime rec=[%s]", rec);
	return sz;
}

int GetDateTimeMsec( char *rec)
{
	int				sz;
	struct timeval	tv;
	struct tm		*tp;

	gettimeofday( &tv, NULL);
	tp = localtime( &tv.tv_sec);
	sz= sprintf( rec, "%04d%02d%02d%02d%02d%02d%06d", 
			tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec, tv.tv_usec);

	return sz;
}

char *GetFileName( char *name)
{
	char 	*ptr;

	ptr = strrchr( name, '/');
	if( ptr == NULL) return name;
	ptr++;
	return ptr;
}



