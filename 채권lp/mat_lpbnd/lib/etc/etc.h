#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifndef TRIM
#define	TRIM( x)		( TrimNR( x, sizeof( x)))
#endif
#ifndef LLONG
#define	LLONG		long long
#endif

double exp10(double x);		/*  warning: incompatible implicit declaration of built-in function 'exp10' <--- warning 때문에 추가 */

/***** Module : etc.c *****/
int         ToUpper( char *dst);                                            /* convert */
char*       AtoA( char *dst, char *src, int sz);                            /* convert */
char*       AtoAT( char *dst, int dst_sz, char *src, int src_sz);           /* convert */
char*       ItoA( char *rec, int value, int sz);                            /* convert */
char*       ItoA2( char *rec, int sz, int value);                           /* convert */
double      Dfloor( double dPrice, int ndecimal);                           /* 내림 ... 2진수 보정: Client 변환 함수 가져옴 */
double      NotDfloor( double value, int point);                            /* 내림 ... 2진수 보정 */
char*       DtoA( char *rec, double value, int sz);                         /* double to string */
char*       DtoAN( char *rec, double value, int sz, int n);                 /* double to string - 소숫점 자리, 반올림 (%sz.nf) */
char*       DtoAND( char *rec, double value, int sz, int n);                /* double to string - 소숫점 자리, 버림 (%sz.nf) */
char*       DtoANU( char *rec, double value, int sz, int n);                /* double to string - 소숫점 자리, 올림 */
char*       DtoALN( char *rec, double value, int sz, int n);                /* double to string - 소숫점 자리, 반올림 (%sz.nf), Left 정렬 */
char*       DtoALND( char *rec, double value, int sz, int n);               /* double to string - 소숫점 자리, 버림 (%sz.nf), Left 정렬 */
char*       DtoALNU( char *rec, double value, int sz, int n);               /* double to string - 소숫점 자리, 올림, Left 정렬 */
char*       DtoA0ND( char *rec, double value, int sz, int n);               /* double to string - 소숫점자리,버림(%sz.nf),0 채움 */
char*       DtoA0NU( char *rec, double value, int sz, int n);               /* double to string - 소숫점 자리,올림,0 채움 */
char*       DtoA0N( char *rec, double value, int sz);                       /* double to string - 0 채움 */
int         AtoI( char *rec, int sz);                                       /* convert */
long        AtoL( char *rec, int sz);                                       /* convert */
double      AtoF( char *rec, int sz);                                       /* convert */
double      AtoD( char *rec, int sz);                                       /* convert */
char*       LLtoA( char *rec, LLONG value, int sz);  	                   /* convert */
LLONG       AtoLL( char *rec, int sz);                                      /* convert */
char*       TtoS( time_t atime);                                            /* convert */
char*       TtoSP( time_t atime, char *buf);                                /* convert */
int         TtoA( char *str, time_t atime);                                 /* convert */
int         TtoAF( char *str, time_t atime, char *format);                  /* convert */
time_t      StoT( char *tstr);                                              /* convert YYYY/MM/DD-hh:mm:ss to time_t */
time_t      AtoT( char *tstr, int sz);                                      /* convert YYYYMMDDhhmmss to time_t */
int         TtoD( time_t cur_time);                                         /* convert time_t to YYMMDD */
int         TtoI( time_t cur_time);                                         /* convert time_t to HHMMSS */
char*       GetTimeStr();                                                   /* convert */
unsigned    StoI( char *str);                                               /* convert */
int         _Max( int a, int b);                                            /* convert */
int         _Min( int a, int b);                                            /* convert */
int         TrimR( char *rec);                                              /* convert */
int         TrimNR( char *rec, int len);                                    /* convert */
int         TrimN( char *rec, int len);                                     /* convert */
int         StrSetN( char *rec, int org, int dst, int len);                 /* convert */
int         IsHanGul( unsigned char *data, int pos);                        /* convert */
char*       GetPrevWeekDay( char *date);                                    /* 전일 날짜를 (YYYYMMDD) 형식으로 산출 */

/***** Module : conv.c *****/
char*       STRtoSTR( char *dst, char *src, int sz);                        /* string1 -> string2 */
char*       STRtoSTT( char *dst, int dst_sz, char *src, int src_sz);        /* string1 -> string2 */
char*       INTtoSTR( char *rec, int value, int sz);                        /* int -> string */
char*       INTtoST0( char *rec, int sz, int value);                        /* int -> string (0 채움) */
char*       DBLtoSTR( char *rec, double value, int sz);                     /* double to string %*.4f */
char*       DBLtoST2( char *rec, double value, int sz);                     /* double to string %*.2f */
char*       DBLtoST0( char *rec, double value, int sz);                     /* double to string %0*.0f */
char*       DBLtoS02( char *rec, double value, int sz);                     /* double to string %*.2f */
char*       DBLtoS00( char *rec, double value, int sz);                     /* double to string %0*.0f */

