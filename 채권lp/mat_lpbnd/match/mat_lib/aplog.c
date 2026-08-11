/** ***************************************************************************
**  @file       main.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  수수료 라이브러리(libwwslm03.a)에서 쓰는 log함수 APLog를 위한 function
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

int APLog( char *pname, int level, char *format, ...)
{
	va_list		args;
	int			sz = 0;
	char		msg[ 65535];

	sz += sprintf( &msg[ sz], "%10s[%4d]", pname, level);

	va_start( args, format);
	sz += vsnprintf( &msg[ sz], 65535 - sz, format, args);
	va_end( args);

	sz += sprintf( &msg[ sz], "\n");

	LogRaw( msg);

	return sz;
}


