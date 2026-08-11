#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char	FileName[ 512] = "./test.log";

char *Line( char *str)
{
	int				rtn, sz;
	char			*ptr;
	static char 	rec[ 8192];
	static FILE		*fp = NULL;

	if( fp == NULL)
	{
		fp = fopen( FileName, "r");
		if( fp == NULL)
		{
			printf( "file open error. name=[%s] err=[%d:%s]\n", errno, strerror( errno));
			return NULL;
		}
	}

	while( 1)
	{
		ptr = fgets( rec, 8192, fp);
		if( ptr == NULL) return NULL;

		ptr = strstr( rec, str);
		if( ptr != NULL) return rec;
	}

	return NULL;
}

