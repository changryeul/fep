/** ***************************************************************************
**  @file       malloc.h
**  @date       2022/08/23
**  @author     최동춘
**  @version    V2.0.20220823
**  @brif
**  메모리 테스트 프로그램
**  메모리 해더(malloc.h)를 include 하여 컴파일
**  malloc/free 함수의 짝을 찾아 메모리 누수 부분을 찾는 모듈
**  결과는 Memory() 함수를 불러 확인
***************************************************************************** */
#include <stdlib.h>
#ifndef MALLOC
#define	MALLOC	1

#ifndef MALLOC_DEFINE
#define MALLOC_DEFINE	1
#define	malloc( x)				Malloc( x, __FILE__, __LINE__)
#define	realloc( x, y)			Realloc( x, y, __FILE__, __LINE__)
#define	free( x)				Free( x, __FILE__, __LINE__)
#endif

#endif

/***** Module : malloc.c *****/
void*       Malloc( size_t sz, const char *file, int line);
void*       Realloc( void *arg, size_t sz, const char *file, int line);
void        Free( void *ptr, const char *file, int line);
int         StMalloc_Add( void *ptr, int line);
int         StMalloc_FindSpace( void *ptr, int line);
int         StMalloc_FindPtr( void *ptr, int line);
int         StMalloc_Del( void *ptr, int line);
int         Memory();

