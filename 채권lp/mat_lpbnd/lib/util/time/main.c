#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main( int argc, char *argv[])
{
	time_t		arg_time, cur_time;
	struct tm	*tp;

	printf( "sz=[%d]\n", sizeof( time_t));
	time( &cur_time);
	printf( "cur_time=[%ld]\n", cur_time);
	if( argc >= 2) arg_time = atoi( argv[ 1]);
	tp = localtime( &arg_time);
	if( tp == NULL)
	{
	}

	printf( "%04d/%02d/%02d-%02d:%02d:%02d\n", tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);

	return 1;
}
