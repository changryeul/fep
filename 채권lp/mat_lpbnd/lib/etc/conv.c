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
**  string1 -> string2
***************************************************************************** */
char *STRtoSTR( char *dst, char *src, int sz)
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
**  string1 -> string2
***************************************************************************** */
char *STRtoSTT( char *dst, int dst_sz, char *src, int src_sz)
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
**  int -> string
***************************************************************************** */
char *INTtoSTR( char *rec, int value, int sz)
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
**  int -> string (0 채움)
***************************************************************************** */
char *INTtoST0( char *rec, int sz, int value)
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
**  double to string %*.4f
***************************************************************************** */
char *DBLtoSTR( char *rec, double value, int sz)
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
**  double to string %*.2f
***************************************************************************** */
char *DBLtoST2( char *rec, double value, int sz)
{
	char buf[ 512];

	sprintf( buf, "%*.2f", sz, value);
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
**  double to string %0*.0f
***************************************************************************** */
char *DBLtoST0( char *rec, double value, int sz)
{
	char buf[ 512];

	sprintf( buf, "%0*.0f", sz, value);
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
**  double to string %*.2f
***************************************************************************** */
char *DBLtoS02( char *rec, double value, int sz)
{
	char buf[ 512];

	sprintf( buf, "%0*.2f", sz, value);
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
**  double to string %0*.0f
***************************************************************************** */
char *DBLtoS00( char *rec, double value, int sz)
{
	char buf[ 512];

	sprintf( buf, "%0*.0f", sz, value);
	memcpy( rec, buf, sz);
	return rec;
}






















