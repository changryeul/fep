#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

main(int argc, char *argv[])
{
	int	i = 0;

	if( argc < 2) return 0;

	i = atoi( argv[ 1]);
	printf( "errno[%d]\t\t= %s\n", i, strerror( i));

	return 0;
}

