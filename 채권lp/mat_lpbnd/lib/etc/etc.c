#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <ctype.h>

#include <math.h>

#ifndef _OMS_SOURCE_
#include "log.h"
#endif
#include "etc.h"

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int ToUpper( char *dst)
{
	int		i;
	int		sz = 0;

	sz = strlen( dst);

	for( i = 0; i < sz; i++)	dst[ i] = toupper( ( int)dst[ i]);
	
	return sz;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
char *AtoA( char *dst, char *src, int sz)
{
	char buf[ 512];

	sprintf( buf, "%-*s", sz, src);
	memcpy( dst, buf, sz);
	return dst;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
char *AtoAT( char *dst, int dst_sz, char *src, int src_sz)
{
	char buf[ 512];

	sprintf( buf, "%-*.*s", dst_sz, src_sz, src);
	memcpy( dst, buf, dst_sz);
	TrimNR( dst, dst_sz);
	return dst;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
char *ItoA( char *rec, int value, int sz)
{
	char buf[ 512];

	sprintf( buf, "%0*d", sz, value);
	memcpy( rec, buf, sz);
	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
char *ItoA2( char *rec, int sz, int value)
{
	char buf[ 512];

	sprintf( buf, "%0*d", sz, value);
	memcpy( rec, buf, sz);
	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  내림 ... 2진수 보정: Client 변환 함수 가져옴
***************************************************************************** */
double Dfloor( double dPrice, int ndecimal)
{
	double dValue = 0.0;
	double epsilon = 1e-8;
	char   strValue[ 512], *p;

	double dfactor = pow(10.0, ndecimal);
	if (dPrice > 0)
	{	
		dValue = dPrice * dfactor + epsilon;		
	}
	else if (dPrice < 0)
	{
		sprintf( strValue, "%.*f", ndecimal, dPrice * dfactor - epsilon);
		dValue = strtod( strValue, &p);
		/* if( p != NULL) LogMsg( "convert remain. p=[%p:%s]", p, p); */
	}
	else
	{
		return 0.0;
	}

	int nValue = (int)dValue;

	if (dPrice < 0 && (nValue - dValue) > 0.0)
		nValue -= 1;


	return nValue / dfactor;
}


/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  내림 ... 2진수 보정
***************************************************************************** */
double NotDfloor( double value, int point)
{
	double	e, d, d1;
	char 	buf[ 512];
	char	*p;

	e = exp10( point);
	d = value * e;
	sprintf( buf, "%15.*f", point, d);

	p = strchr( buf, '.');
	*p = 0;
	d1 = strtod( buf, &p);
	LogDbg( "d2=[%f]", d1);
	d = floor( d);
	LogDbg( "d3=[%f]", d);

	return d / e;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string
***************************************************************************** */
char *DtoA( char *rec, double value, int sz)
{
	char buf[ 512];

	sprintf( buf, "%*.4f", sz, value);
	memcpy( rec, buf, sz);
	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점 자리, 반올림 (%sz.nf)
***************************************************************************** */
char *DtoAN( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%*.*f", sz, n, round( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점 자리, 버림 (%sz.nf)
***************************************************************************** */
char *DtoAND( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%*.*f", sz, n, floor( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점 자리, 올림
***************************************************************************** */
char *DtoANU( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%*.*f", sz, n, ceil( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점 자리, 반올림 (%sz.nf), Left 정렬
***************************************************************************** */
char *DtoALN( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%-*.*f", sz, n, round( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점 자리, 버림 (%sz.nf), Left 정렬
***************************************************************************** */
char *DtoALND( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%-*.*f", sz, n, floor( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점 자리, 올림, Left 정렬
***************************************************************************** */
char *DtoALNU( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%-*.*f", sz, n, ceil( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점자리,버림(%sz.nf),0 채움
***************************************************************************** */
char *DtoA0ND( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%0*.*f", sz, n, floor( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 소숫점 자리,올림,0 채움
***************************************************************************** */
char *DtoA0NU( char *rec, double value, int sz, int n)
{
	char	buf[ 1024];
	double	e, d = ( double)n;

	e = exp10( d);
	sprintf( buf, "%0*.*f", sz, n, ceil( value * e) / e);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  double to string - 0 채움
***************************************************************************** */
char *DtoA0N( char *rec, double value, int sz)
{
	char	buf[ 1024];

	sprintf( buf, "%0*f", sz, value);
	memcpy( rec, buf, sz);

	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int AtoI( char *rec, int sz)
{
	int		i;
	char	buf[ 512];

	memcpy( buf, rec, sz);
	buf[ sz] = 0;
	i = atoi( buf);
	return i;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
long AtoL( char *rec, int sz)
{
	long	l;
	char	buf[ 512];

	memcpy( buf, rec, sz);
	buf[ sz] = 0;
	l = atol( buf);
	return l;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
double AtoF( char *rec, int sz)
{
	double	d;
	char	buf[ 512];

	memcpy( buf, rec, sz);
	buf[ sz] = 0;
	d = atof( buf);
	return d;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
double AtoD( char *rec, int sz)
{
	double	d;
	char	buf[ 512];
	char	*ptr;

	memcpy( buf, rec, sz);
	buf[ sz] = 0;
	LogDel( "buf=[%s]", buf);
	d = strtod( buf, &ptr);
	LogDel( "d=[%f]", d);
	return d;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
char *LLtoA( char *rec, long long value, int sz)
{
	char buf[ 512];

	sprintf( buf, "%0*lld", sz, value);
	memcpy( rec, buf, sz);
	return rec;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
long long AtoLL( char *rec, int sz)
{
	long long		l;
	char			buf[ 512];

	memcpy( buf, rec, sz);
	buf[ sz] = 0;
	l = atoll( buf);
	return l;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
char *TtoS( time_t atime)
{
	static char tstr[ 512];
	struct tm	_tm, *tp = &_tm;
	struct tm	*tmp;

	if( atime == 0)
	{
		sprintf( tstr, "0000/00/00-00:00:00");
		return tstr;
	}
	tmp = localtime_r( &atime, tp);
	if( tmp == NULL)
	{
		LogErr( "localtime error. atime=[%ld]", atime);
		sprintf( tstr, "0000/00/00-00:00:00");
		return tstr;
	}

	sprintf( tstr, "%04d/%02d/%02d-%02d:%02d:%02d", 
		tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday,
			tp->tm_hour, tp->tm_min, tp->tm_sec);

	return tstr;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
char *TtoSP( time_t atime, char *buf)
{
	struct tm	_tm, *tp = &_tm;
	struct tm	*tmp;

	if( atime == 0)
	{
		sprintf( buf, "0000/00/00-00:00:00");
		return buf;
	}
	tmp = localtime_r( &atime, tp);
	if( tmp == NULL)
	{
		LogErr( "localtime error. atime=[%ld]", atime);
		sprintf( buf, "0000/00/00-00:00:00");
		return buf;
	}

	sprintf( buf, "%04d/%02d/%02d-%02d:%02d:%02d", 
		tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday,
			tp->tm_hour, tp->tm_min, tp->tm_sec);

	return buf;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int TtoA( char *str, time_t atime)
{
	struct tm	*tp;
	char		*ptr;

	tp = localtime( &atime);
	if( tp == NULL)
	{
		LogErr( "localtime error. atime=[%ld]", atime);
		return 0;
	}

	ptr = strstr( str, "ss");	if( ptr) ItoA( ptr, tp->tm_sec, 2);
	ptr = strstr( str, "mm");	if( ptr) ItoA( ptr, tp->tm_min, 2);
	ptr = strstr( str, "hh");	if( ptr) ItoA( ptr, tp->tm_hour, 2);
	ptr = strstr( str, "DD");	if( ptr) ItoA( ptr, tp->tm_mday, 2);
	ptr = strstr( str, "MM");	if( ptr) ItoA( ptr, tp->tm_mon +1, 2);
	ptr = strstr( str, "YYYY");	
	if( ptr) 
	{
		ItoA( ptr, tp->tm_year + 1900, 4);
	}
	else 
	{
		ptr = strstr( str, "YY"); 
		if( ptr)	ItoA( ptr, tp->tm_year % 100, 2);
	}

	return 1;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int TtoAF( char *str, time_t atime, char *format)
{
	struct tm	*tp;
	char		*ptr;

	memcpy( str, format, strlen( format));

	tp = localtime( &atime);
	if( tp == NULL)
	{
		LogErr( "localtime error. atime=[%ld]", atime);
		return 0;
	}

	ptr = strstr( str, "ss");	if( ptr) ItoA( ptr, tp->tm_sec, 2);
	ptr = strstr( str, "mm");	if( ptr) ItoA( ptr, tp->tm_min, 2);
	ptr = strstr( str, "hh");	if( ptr) ItoA( ptr, tp->tm_hour, 2);
	ptr = strstr( str, "DD");	if( ptr) ItoA( ptr, tp->tm_mday, 2);
	ptr = strstr( str, "MM");	if( ptr) ItoA( ptr, tp->tm_mon +1, 2);
	ptr = strstr( str, "YYYY");	
	if( ptr) 
	{
		ItoA( ptr, tp->tm_year + 1900, 4);
	}
	else 
	{
		ptr = strstr( str, "YY"); 
		if( ptr)	ItoA( ptr, tp->tm_year % 100, 2);
	}

	return 1;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert YYYY/MM/DD-hh:mm:ss to time_t
***************************************************************************** */
/* convert time string to time_t - YYYY/MM/DD-hh:mm:ss */
time_t StoT( char *tstr)
{
	int			len = 19, stat = 0, i;
	char 		*token = "/:- ";
	char		str[ 512];
	char		*ptr;
	struct tm	tm_buf;
	time_t		rtn_time;

	memcpy( str, tstr, len);
	str[ len] = 0;

	memset( &tm_buf, 0x00, sizeof( struct tm));
	ptr = strtok( str, token);
	while( ptr != NULL)
	{
		i = atoi( ptr);
		switch( stat)
		{
			case 0:
				if( i < 1900) return 0;
				tm_buf.tm_year = i -1900;
				break;
			case 1:
				if( i <= 0) i = 1;
				tm_buf.tm_mon = i -1;
				break;
			case 2:
				if( i <= 0) i = 1;
				tm_buf.tm_mday = i;
				break;
			case 3:
				tm_buf.tm_hour = atoi( ptr);
				break;
			case 4:
				tm_buf.tm_min = atoi( ptr);
				break;
			case 5:
				tm_buf.tm_sec = atoi( ptr);
				break;
				
		}
		ptr = strtok( NULL, token);
		stat++;
	}

	rtn_time = mktime( &tm_buf);

	return rtn_time;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert YYYYMMDDhhmmss to time_t
***************************************************************************** */
/* convert time string to time_t - YYYYMMDDhhmmss */
time_t AtoT( char *tstr, int len)
{
	int			pos = 0;
	time_t		cur_time;
	char		str[ 512];
	struct tm	tm_buf;
	time_t		rtn_time;

	memcpy( str, tstr, len);
	str[ len] = 0;
	time( &cur_time);
	localtime_r( &cur_time, &tm_buf);
	LogDel( "str=[%s]", str);
	LogDel( "len=[%d]", len);

	switch( len)
	{
		case 14:
			tm_buf.tm_year = AtoI( &str[ pos], 4) - 1900;
			pos += 4;
		case 10:
			tm_buf.tm_mon = AtoI( &str[ pos], 2) -1;
			pos += 2;
		case 8 :
			tm_buf.tm_mday = AtoI( &str[ pos], 2);
			pos += 2;
		case 6 :
			tm_buf.tm_hour = AtoI( &str[ pos], 2);
			LogDel( "tm_buf.tm_hour=[%d] [%.2s]", tm_buf.tm_hour, &str[ pos]);
			pos += 2;
		case 4 :
			tm_buf.tm_min = AtoI( &str[ pos], 2);
			LogDel( "tm_buf.tm_min=[%d] [%.2s]", tm_buf.tm_min, &str[ pos]);
			pos += 2;
		case 2 :
			tm_buf.tm_sec = AtoI( &str[ pos], 2);
			LogDel( "tm_buf.tm_sec=[%d] [%.2s]", tm_buf.tm_sec, &str[ pos]);
			pos += 2;
			break;
		default:
			return 0;
	}

	rtn_time = mktime( &tm_buf);

	return rtn_time;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert time_t to YYMMDD
***************************************************************************** */
int	TtoD( time_t cur_time)
{
	int			int_day = 0;  /* YYYYMMDD */
	struct tm	*tp;

	tp = localtime( &cur_time);

	int_day  = ( tp->tm_year + 1900) * 10000;
	int_day += ( tp->tm_mon + 1) * 100;
	int_day += tp->tm_mday;

	return int_day;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert time_t to HHMMSS
***************************************************************************** */
int	TtoI( time_t cur_time)
{
	int			int_time = 0;  /* HHMMSS */
	struct tm	*tp;

	tp = localtime( &cur_time);

	int_time  = tp->tm_hour * 10000;
	int_time += tp->tm_min * 100;
	int_time += tp->tm_sec;

	return int_time;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
char *GetTimeStr()
{
	static char	time_str[ 32];
	time_t		cur_time;
	struct tm	*tp;

	time( &cur_time);
	tp = localtime( &cur_time);

	sprintf( time_str, "%04d/%02d/%02d-%02d:%02d:%02d", 
			tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);

	return time_str;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
unsigned int StoI( char *str)
{
	long long		rtn;

	if( !memcmp( str, "0x", 2)) 			rtn = strtoll( str, NULL, 16);
	else if( !memcmp( str, "0X", 2)) 		rtn = strtoll( str, NULL, 16);
	else if( !memcmp( str, "0",  1)) 		rtn = strtoll( str, NULL, 8);
	else									rtn = strtoll( str, NULL, 0);
	return ( unsigned int)rtn;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int _Max( int a, int b)
{
	return ((a)>(b)) ? (a):(b);
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int _Min( int a, int b)
{
	return ((a)<(b)) ? (a):(b);
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int TrimR( char *rec)
{
	int		pos;

	pos = strlen( rec) -1;
	if( pos < 0) return pos;

	while( rec[ pos] != 0)
	{
		switch( rec[ pos])
		{
			case ' ':		/* space */
			case '\t':		/* tab */
			case '\n':		/* line feed */
			case '\r':		/* carriage return */
			case '\v':		/* vertical tab */
			case '\f':		/* feed */
				rec[ pos] = 0;
				break;
			default:
				return pos +1;
		}
		pos--;
		if( pos < 0) return 0;
	}
	return 0;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int TrimNR( char *rec, int len)
{
	int		pos = 0;

	pos = len -1;
	while( pos >= 0)
	{
		switch( rec[ pos])
		{
			case 0x00:
			case ' ' :		/* space */
			case '\t':		/* tab */
			case '\n':		/* line feed */
			case '\r':		/* carriage return */
			case '\v':		/* vertical tab */
			case '\f':		/* feed */
				rec[ pos] = 0;
				break;
			default:
				return pos +1;
		}
		pos--;
	}

	return 0;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int TrimN( char *rec, int len)
{
	int		pos = 0;

	while( pos < len)
	{
		switch( rec[ pos])
		{
			case ' ':		/* space */
			case '\t':		/* tab */
			case '\n':		/* line feed */
			case '\r':		/* carriage return */
			case '\v':		/* vertical tab */
			case '\f':		/* feed */
				rec[ pos] = 0;
				break;
			default:
				break;
		}
		pos++;
	}

	return len;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int StrSetN( char *rec, int org, int dst, int len)
{
	int		pos = 0;

	while( pos < len)
	{
		if( rec[ pos] == org) rec[ pos] = dst;
		pos++;
	}

	return len;
}

/** ***************************************************************************
**  @fn         int A()
**  @param      char 	dst
**  @param      char 	src
**  @param      int		size
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert
***************************************************************************** */
int IsHanGul( unsigned char *data, int pos)
{
	int				cnt = 0;
	unsigned char 	c;

	while( 1)
	{
		c = data[ pos];

		if( c < 0x80) 
		{
			if( cnt == 0)	return 0;
			if( cnt % 2)	return 1;
			else			return 2;
		}
		cnt++;
		pos--;
	}
}

/** ***************************************************************************
**  @fn         int GetPrevWeekDay( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패 종료
**  @retval     양수 성공 종료
**  @exception
**  @remark
**  @brief
**  전일 날짜를 (YYYYMMDD) 형식으로 산출
**  전일 - 토요일,일요일 제외 공휴일 포함
***************************************************************************** */
char *GetPrevWeekDay( char *date)
{
	time_t		cur_time;
	struct tm	*tp;

	time( &cur_time);
	tp = localtime( &cur_time);

	switch( tp->tm_wday)
	{
		/* 어제 */
		default:
			cur_time -= ( 3600 * 24 );
			break;
		/* 그제 */
		case 0:
			cur_time -= ( 3600 * 24 * 2);
			break;
		/* 3일 전 */
		case 1:
			cur_time -= ( 3600 * 24 * 3);
			break;
	}
	tp = localtime( &cur_time);

	sprintf( date, "%04d%02d%02d", tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday);

	return date;
}






















