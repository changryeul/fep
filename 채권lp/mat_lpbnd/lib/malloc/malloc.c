/** ***************************************************************************
**  @file       malloc.c
**  @date       2022/08/23
**  @author     최동춘
**  @version    V2.0.20220823
**  @brif
**  메모리 테스트 프로그램
**  메모리 해다(malloc.h)를 include 하여 컴파일
**  malloc/free 함수의 짝을 찾아 메모리 누수 부분을 찾는 모듈
***************************************************************************** */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define	MALLOC_DEFINE	1
#include "malloc.h"
#include "log.h"

typedef struct _st_malloc__
{
	int		flag;			/* 0 - malloc 1 - free */
	void	*ptr;			/* memory pointer - malloc return pointer */
	int		aline;			/* malloc call line */
	int		dline;			/* free call line */
}	ST_MALLOC;

ST_MALLOC	StMalloc[ 65535];
int			StMallocCnt = 0;

void *Malloc( size_t sz, const char *file, int line)
{
    void 	*ptr;

    ptr = malloc( sz);
   	LogDbg( "malloc ptr=[%p] sz=[%d] file=[%s] line=[%d]", ptr, sz, file, line);

	StMalloc_Add( ptr, line);

    return ptr;
}

void *Realloc( void *arg, size_t sz, const char *file, int line)
{
	// int		i;
    void 	*ptr;

	StMalloc_Del( arg, line);

    ptr = realloc( arg, sz);
    LogDbg( "realloc ptr=[%p] arg=[%p] sz=[%d] file=[%s] line=[%d]", ptr, arg, sz, file, line);

	StMalloc_Add( ptr, line);

    return ptr;
}

void Free( void *ptr, const char *file, int line)
{
    LogDbg( "free  ptr=[%p] file=[%s] line=[%d]", ptr, file, line);
	StMalloc_Del( ptr, line);

    free( ptr);
}

int StMalloc_Add( void *ptr, int line)
{
	int		pos;

	pos = StMalloc_FindPtr( ptr, line);
	if( pos >= 0)
	{
		LogDbg( "pointer already alloc. ptr=[%p] line=[%d]", ptr, line);
		LogDbg( "%04d ptr=[%p] flag=[%d] aline=[%4d] dline=[%4d]", 
				pos, StMalloc[ pos].ptr, StMalloc[ pos].flag, StMalloc[ pos].aline, StMalloc[ pos].dline);
		return 0;
	}

	pos = StMalloc_FindSpace( ptr, line);
	if( pos >= 0)
	{
		memset( &StMalloc[ pos], 0x00, sizeof( ST_MALLOC));
		StMalloc[ pos].ptr   = ptr;
		StMalloc[ pos].aline = line;
		return 0;
	}

	StMalloc[ StMallocCnt].ptr   = ptr;
	StMalloc[ StMallocCnt].aline = line;
	StMallocCnt++;

	return 1;
}

int StMalloc_FindSpace( void *ptr, int line)
{
	int	pos = 0;

	while( pos < StMallocCnt)
	{
		if( StMalloc[ pos].flag == 1)
		{
			return pos;
		}
		pos++;
	}
	return -1;
}

int StMalloc_FindPtr( void *ptr, int line)
{
	int	pos = 0;

	while( pos < StMallocCnt)
	{
		if( StMalloc[ pos].ptr == ptr)
		{
			if( StMalloc[ pos].flag == 0)
			{
				return pos;
			}
		}
		pos++;
	}
	return -1;
}

int StMalloc_Del( void *ptr, int line)
{
	int pos = 0;

	pos = StMalloc_FindPtr( ptr, line);
	if( pos < 0)
	{
		LogDbg( "pointer not found. ptr=[%p] line=[%d]", ptr, line);
		return 0;
	}
	else
	{
		StMalloc[ pos].flag	 = 1;
		StMalloc[ pos].dline = line;
		LogDel( "pointer del found. ptr=[%p] line=[%d]", ptr, line);
		return 1;
	}

	return 0;
}

int Memory()
{
	int		i;

	if( StMallocCnt <= 0) 
	{
		LogDbg( "StMallocCnt=[%d]", StMallocCnt);
		return 0;
	}
	LogDbg( "-----[ malloc report ]-----------------------------------------");
	LogDbg( "---------------------------------------------------------------");
	for( i = 0; i < StMallocCnt; i++)
	{
		/*
		if( StMalloc[ i].flag == 1) continue;
		*/
		LogDbg( "%04d ptr=[%p] flag=[%d] aline=[%4d] dline=[%4d]", 
				i, StMalloc[ i].ptr, StMalloc[ i].flag, StMalloc[ i].aline, StMalloc[ i].dline);
	}
	LogDbg( "-----------------------------------------[ malloc report ]-----");
	LogDbg( "malloc count                 = [%d]", StMallocCnt);
	/*
	StMallocCnt = 0;
	*/

	return 1;
}


