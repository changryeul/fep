#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

char *ItoA( char *buf, int value, int sz)
{
	sprintf( buf, "%0*d", sz, value);
	return buf;
}

int AtoI( char *rec, int sz)
{
	int		i;
	char	buf[ 512];

	memcpy( buf, rec, sz);
	buf[ sz] = 0;
	i = atoi( buf);
	return i;
}

char *TtoS( time_t atime)
{
	static char tstr[ 512];
	struct tm	*tp;

	tp = localtime( &atime);

	sprintf( tstr, "%04d/%02d/%02d-%02d:%02d:%02d", 
		tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday,
			tp->tm_hour, tp->tm_min, tp->tm_sec);

	return tstr;
}

time_t StoT( char *tstr)
{
	int			len, stat = 0;
	char 		*token = "/:-";
	char		str[ 512];
	char		*ptr;
	struct tm	tm_buf;
	time_t		rtn_time;

	len = strlen( tstr) +1;
	if( len > 512) len = 512;

	memcpy( str, tstr, len);

	memset( &tm_buf, 0x00, sizeof( struct tm));
	ptr = strtok( str, token);
	while( ptr != NULL)
	{
		printf( "ptr =[%s]\n", ptr);
		switch( stat)
		{
			case 0:
				tm_buf.tm_year = atoi( ptr) -1900;
				printf( "===%d\n", tm_buf.tm_year);
				break;
			case 1:
				tm_buf.tm_mon = atoi( ptr) -1;
				break;
			case 2:
				tm_buf.tm_mday = atoi( ptr);
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

	printf( "rtn_time  = [%lld]\n", rtn_time);

	return rtn_time;
}

unsigned int StoI( char *str)
{
	long long		rtn;

	if( !memcmp( str, "0x", 2)) 			rtn = strtoll( str, NULL, 16);
	else if( !memcmp( str, "0X", 2)) 		rtn = strtoll( str, NULL, 16);
	else if( !memcmp( str, "0",  1)) 		rtn = strtoll( str, NULL, 8);
	else									rtn = strtoll( str, NULL, 0);
	return ( unsigned int)rtn;
}

Max( int a, int b)
{
	return ((a)>(b)) ? (a):(b);
}

Min( int a, int b)
{
	return ((a)<(b)) ? (a):(b);
}


