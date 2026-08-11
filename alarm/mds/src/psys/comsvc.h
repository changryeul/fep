
#ifndef _COMSVC_H_
#define _COMSVC_H_

#if defined(LINUX)

//#ifdef  __USE_BSD
# ifndef __uchar_t_defined
typedef unsigned char   uchar_t;
typedef unsigned short  ushort_t;
typedef unsigned int    uint_t;
typedef unsigned long   ulong_t;
#  define __uchar_t_defined
# endif
//#endif

#endif

#define SUNDAY      0
#define MONDAY      1
#define TUESDAY     2
#define WEDNESDAY   3
#define THURSDAY    4
#define FRIDAY      5
#define SATURDAY    6

typedef struct  {
    uchar_t yy;     /* year [1900(0) ~ ]        */
    uchar_t mm;     /* month [1 - 12]       */
    uchar_t dd;     /* day [1 - 31]         */
    uchar_t hh;     /* hour [0 - 23]        */
    uchar_t mi;     /* minute [0 - 59]      */
    uchar_t ss;     /* second [0 - 59]      */
    uchar_t ms;     /* mili second [0 - 99]     */
    uchar_t xx;     /* reserved         */
} YMDT;

typedef struct {
    short   yy;     /* year (YYYY)          */
    char    mm;     /* month [1 - 12]       */
    char    dd;     /* day [1 - 31]         */
} YMD;

typedef struct {
    char    hh;     /* hour [0 - 23]        */
    char    mm;     /* minute [0 - 59]      */
    char    ss;     /* second [0 - 59]      */
    char    ms;     /* mili second [0 - 99]     */
} HMSM;

typedef struct {
    char    hh;     /* hour [0 - 23]        */
    char    mm;     /* minute [0 - 59]      */
    char    ss;     /* second [0 - 59]      */
} HMS;

typedef struct  {
    char    hh;     /* hour [0 - 23]        */
    char    mm;     /* minute [0 - 59]      */
} HM;

#endif
