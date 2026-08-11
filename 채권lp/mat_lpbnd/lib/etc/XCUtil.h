#ifndef _XCUTIL_H_
#define _XCUTIL_H_
#include "67ab1daf27abc93ece43dcdd2aee7ddcec0deb35.OS.h"
#include "9222da1e889ba0998d171b7041b49bfb5a34bde0.XCube_Basic_Types.h"
#include "9222da1e889ba0998d171b7041b49bfb5a34bde0.XCUBE_Export.h"

#include <stdio.h>

#define XDBL_PRECISION_VALUE        (1.0E-10)
#define IS_DBL_ZERO(X)              ( fabs( (X) ) <= XDBL_PRECISION_VALUE ? true : false         )
#define IS_DBL_NONZERO(X)           ( fabs( (X) ) <= XDBL_PRECISION_VALUE ? false : true )
#define IS_DBL_EQ(X,Y)              ( fabs( (X) - (Y) ) <= XDBL_PRECISION_VALUE ? true   : false )
#define IS_DBL_NEQ(X,Y)             ( XDBL_PRECISION_VALUE < fabs( (X) - (Y) ) ? true    : false )
#define IS_DBL_GT(X,Y)              ( XDBL_PRECISION_VALUE < ( (X) - (Y) ) ? true        : false )
#define IS_DBL_GEQ(X,Y)             ( ( (Y) - (X) ) <= XDBL_PRECISION_VALUE      ? true : false )
#define IS_DBL_LT(X,Y)              ( XDBL_PRECISION_VALUE < ( (Y) - (X) )       ? true : false )
#define IS_DBL_LEQ(X,Y)             ( ( (X) - (Y) ) <= XDBL_PRECISION_VALUE      ? true : false )

#define GET_DATE(x)                 XCUtil::get_date(x)                  // YYYYMMDD
#define GET_TIME(x)                 XCUtil::get_time(x)                  // HHMMSS
#define GET_DATETIME(x)             XCUtil::get_datetime(x)              // YYYYMMDDHHMMSS
#define GET_TWDATE(x)               XCUtil::get_twdate(x)                // TAIWAN DATE...
#define MEMCPY(a,b)                 (ACE_OS::memcpy (a, b, ((sizeof(a)) > (sizeof(b))) ? sizeof(b) : sizeof(a))) // �����ʸ�ŭ copy
#define STRCPY(a,b)                 (ACE_OS::memcpy (a, b, ACE_OS::strlen(b))) // strcopy (���� �Ⱦ��°�)
#define FW_GET_DATE(x)              XCUtil::get_date (x)                 // YYYYMMDD
#define FW_GET_TIME(x)              XCUtil::get_time (x)                 // HHMMSS
#define FW_GET_DTIME(x)             XCUtil::get_datetime (x)             // YYYYMMDDHHMMSS
#define FW_GET_DTIMEMS(x)           XCUtil::get_datetimemsec (x)         // YYYYMMDDHHMMSSmmmmmm

#define FW_GET_GMDATE(x)            XCUtil::get_gmdate (x)               // YYYYMMDD
#define FW_GET_GMTIME(x)            XCUtil::get_gmtime (x)               // HHMMSS
#define FW_GET_GMDTIME(x)           XCUtil::get_gmdatetime (x)           // YYYYMMDDHHMMSS

// 2010-01-26 ecchoe New Function Wow olle!!

#define FW_ATOL(x)           XCUtil::atol (x, sizeof(x))                     // ���� <-> long
#define FW_ATOI(x)           XCUtil::atoi (x, sizeof(x))                     // int <-> ����  ex) val = 1234, len = 10, c = #
#define FW_ATOLN(x,y)        XCUtil::atol (x, y)
#define FW_ATOIN(x,y)        XCUtil::atoi (x, y)
#define FW_ITOA(x, val)      XCUtil::itoa (x, sizeof(x), val)                // '1234\n?????'   NULL ���� ���ڿ�
#define FW_ZITOA(x,val)      XCUtil::zero_itoa (x, sizeof(x), val)           // '000001234\n'   NULL ���� ���ڿ�
#define FW_FITOA(x,val)      XCUtil::lpad_itoa (x, sizeof(x), val)           // '     1234\n'   NULL ���� ���ڿ�
#define FW_SITOA(x,val)      XCUtil::rpad_itoa (x, sizeof(x), val)           // '1234     \n'   NULL ���� ���ڿ�
#define FW_LITOA(x,val,c)    XCUtil::lpad_itoa (x, sizeof(x), val, c)        // '#####1234\n'   NULL ���� ���ڿ�
#define FW_RITOA(x,val,c)    XCUtil::rpad_itoa (x, sizeof(x), val, c)        // '1234#####\n'   NULL ���� ���ڿ�
#define FW_ZITOAS(x,val)     XCUtil::zero_itoas (x, sizeof(x), val)          // '0000001234'    NULL ����
#define FW_FITOAS(x,val)     XCUtil::lpad_itoas (x, sizeof(x), val)          // '      1234'    NULL ����
#define FW_SITOAS(x,val)     XCUtil::rpad_itoas (x, sizeof(x), val)          // '1234      '    NULL ����
#define FW_LITOAS(x,val,c)   XCUtil::lpad_itoas (x, sizeof(x), val, c)       // '######1234'    NULL ����
#define FW_RITOAS(x,val,c)   XCUtil::rpad_itoas (x, sizeof(x), val, c)       // '1234######'    NULL ����

#define FW_ATOF(x)           XCUtil::atof (x, sizeof(x))                                //  dobule <-> ����  ex) val = 12.34, len = 10, d(decimal) = 3, c = #
#define FW_FTOA(x,val)       XCUtil::ftoa (x, sizeof(x), val)                // '12.34\n????'   NULL ���� ���ڿ�
#define FW_FTOAD(x,val,d)    XCUtil::ftoa (x, sizeof(x), val,d)              // '12.340\n???'   NULL ���� ���ڿ�
#define FW_ZFTOA(x,val,d)    XCUtil::zero_ftoa (x, sizeof(x), val, d)        // '00012.340\n'   NULL ���� ���ڿ�
#define FW_FFTOA(x,val,d)    XCUtil::lpad_ftoa (x, sizeof(x), val, d)        // '   12.340\n'   NULL ���� ���ڿ�
#define FW_SFTOA(x,val,d)    XCUtil::rpad_ftoa (x, sizeof(x), val, d)        // '12.340   \n'   NULL ���� ���ڿ�
#define FW_LFTOA(x,val,d,c)  XCUtil::lpad_ftoa (x, sizeof(x), val, d, c)     // '###12.340\n'   NULL ���� ���ڿ�
#define FW_RFTOA(x,val,d,c)  XCUtil::rpad_ftoa (x, sizeof(x), val, d, c)     // '12.340###\n'   NULL ���� ���ڿ�
#define FW_ZFTOAS(x,val,d)   XCUtil::zero_ftoas (x, sizeof(x), val, d)       // '000012.340'    NULL ����
#define FW_FFTOAS(x,val,d)   XCUtil::lpad_ftoas (x, sizeof(x), val, d)       // '    12.340'    NULL ����
#define FW_SFTOAS(x,val,d)   XCUtil::rpad_ftoas (x, sizeof(x), val, d)       // '12.340    '    NULL ����
#define FW_LFTOAS(x,val,d,c) XCUtil::lpad_ftoas (x, sizeof(x), val, d, c)    // '####12.340'    NULL ����
#define FW_RFTOAS(x,val,d,c) XCUtil::rpad_ftoas (x, sizeof(x), val, d, c)    // '12.340####'    NULL ����

#define FW_Z0FTOA(x,val)     XCUtil::zero_ftoa (x, sizeof(x), val, 0)        // '00000000000012\n'   NULL ���� ���ڿ� (len 15)
#define FW_Z2FTOA(x,val)     XCUtil::zero_ftoa (x, sizeof(x), val, 2)        // '00000000012.34\n'   NULL ���� ���ڿ� (len 15)
#define FW_Z6FTOA(x,val)     XCUtil::zero_ftoa (x, sizeof(x), val, 6)        // '0000012.340000\n'   NULL ���� ���ڿ� (len 15)
#define FW_Z9FTOA(x,val)     XCUtil::zero_ftoa (x, sizeof(x), val, 9)        // '0012.340000000\n'   NULL ���� ���ڿ� (len 15)
#define FW_Z0FTOAS(x,val)    XCUtil::zero_ftoas (x, sizeof(x), val, 0)       // '000000000000012'    NULL ����
#define FW_Z2FTOAS(x,val)    XCUtil::zero_ftoas (x, sizeof(x), val, 2)       // '000000000012.34'    NULL ����
#define FW_Z6FTOAS(x,val)    XCUtil::zero_ftoas (x, sizeof(x), val, 6)       // '00000012.340000'    NULL ���� (len 15)
#define FW_Z9FTOAS(x,val)    XCUtil::zero_ftoas (x, sizeof(x), val, 9)       // '00012.340000000'    NULL ���� (len 15)

#define FW_F0FTOA(x,val)     XCUtil::lpad_ftoa (x, sizeof(x), val, 0)        // '            12\n'   NULL ���� ���ڿ� (len 15)
#define FW_F2FTOA(x,val)     XCUtil::lpad_ftoa (x, sizeof(x), val, 2)        // '         12.34\n'   NULL ���� ���ڿ� (len 15)
#define FW_F6FTOA(x,val)     XCUtil::lpad_ftoa (x, sizeof(x), val, 6)        // '     12.340000\n'   NULL ���� ���ڿ� (len 15)
#define FW_F9FTOA(x,val)     XCUtil::lpad_ftoa (x, sizeof(x), val, 9)        // '  12.340000000\n'   NULL ���� ���ڿ� (len 15)
#define FW_F0FTOAS(x,val)    XCUtil::lpad_ftoas (x, sizeof(x), val, 2)       // '             12'    NULL ����
#define FW_F2FTOAS(x,val)    XCUtil::lpad_ftoas (x, sizeof(x), val, 2)       // '          12.34'    NULL ����
#define FW_F6FTOAS(x,val)    XCUtil::lpad_ftoas (x, sizeof(x), val, 6)       // '      12.340000'    NULL ���� (len 15)
#define FW_F9FTOAS(x,val)    XCUtil::lpad_ftoas (x, sizeof(x), val, 9)       // '   12.340000000'    NULL ���� (len 15)

#define FW_S0FTOA(x,val)     XCUtil::rpad_ftoa (x, sizeof(x), val, 0)        // '12            \n'   NULL ���� ���ڿ� (len 15)
#define FW_S2FTOA(x,val)     XCUtil::rpad_ftoa (x, sizeof(x), val, 2)        // '12.34         \n'   NULL ���� ���ڿ� (len 15)
#define FW_S6FTOA(x,val)     XCUtil::rpad_ftoa (x, sizeof(x), val, 6)        // '12.340000     \n'   NULL ���� ���ڿ� (len 15)
#define FW_S9FTOA(x,val)     XCUtil::rpad_ftoa (x, sizeof(x), val, 9)        // '12.340000000  \n'   NULL ���� ���ڿ� (len 15)
#define FW_S0FTOAS(x,val)    XCUtil::rpad_ftoas (x, sizeof(x), val, 0)       // '12             '    NULL ����
#define FW_S2FTOAS(x,val)    XCUtil::rpad_ftoas (x, sizeof(x), val, 2)       // '12.34          '    NULL ����
#define FW_S6FTOAS(x,val)    XCUtil::rpad_ftoas (x, sizeof(x), val, 6)       // '12.340000      '    NULL ���� (len 15)
#define FW_S9FTOAS(x,val)    XCUtil::rpad_ftoas (x, sizeof(x), val, 9)       // '12.340000000   '    NULL ���� (len 15)

// 2010-01-26 ecchoe New Function Wow olle!!

#define FW_CLEAR(a)                 memset((char *)&a, 0x00, sizeof(a))   // bzero
#define FW_CLSTR(a)                 memset(a, 0x00, sizeof(a))   // bzero
#define FW_SPACE(a)                 memset((char *)&a, 0x20, sizeof(a))   // space�� ä��� 
#define FW_FCHAR(a,x)               memset((char *)a, x , sizeof(a))      // x �� ä���
#define FW_FSPAC(a,n)               memset((char *)a, 0x20, n)            // space �� ä���
#define FW_FZERO(a)                 memset((char *)a, '0' , sizeof(a))    // 0���� ä���
#define FW_MEMCPY(a,b)              (memcpy(a,b, ((sizeof(a)) > (sizeof(b))) ? sizeof(b) : sizeof(a)))  //���������� copy
#define FW_FILCPY(a,b)              {memset (a,0x20,sizeof(a)); memcpy(a,b, ((strlen(b)) < (sizeof(a))) ? strlen(b) : sizeof(a));} // copy �� space ä���
#define FW_STRCPY(a,b)              {memset (a,0x00,sizeof(a)); memcpy(a,b, ((strlen(b)) < (sizeof(a)-1)) ? strlen(b) : sizeof(a)-1);} // data �� copy �� \0 .
#define FW_VALCPY(a,b)              {memset (a,0x00,sizeof(a));XCUtil::xc_valcpy(a,b, ((sizeof(b)) < (sizeof(a)-1)) ? sizeof(b) : sizeof(a)-1);}                  // data �� copy
#define FW_ZFLCPY(a,b)              {memset (a,'0',sizeof(a)); memcpy(&a[sizeof(a) - (((strlen(b)) < (sizeof(a))) ? strlen(b) : sizeof(a)) ],b, ((strlen(b)) < (sizeof(a))) ? strlen(b) : sizeof(a));} // copy �� space ä���

#define FW_TRIM(STR)                XCUtil::Trim ((STR), NULL)
#define FW_LTRIM(STR)               XCUtil::LTrim((STR), NULL)
#define FW_RTRIM(STR)               XCUtil::RTrim((STR), NULL)

/* //////////////////////////////////////////////////////////////////////////////////// */
#define FW_TOUPPER(STR)             XCUtil::ToUpper(STR)
#define FW_TOLOWER(STR)             XCUtil::ToLower(STR)

#define FW_UCASE(STR)               XCUtil::ToUpper(STR)
#define FW_LCASE(STR)               XCUtil::ToLower(STR)

/* //////////////////////////////////////////////////////////////////////////////////// */
// #ifdef FW_CLEAR
// #  undef FW_CLEAR
// #endif
// #define FW_CLEAR(BUF)            memset((char *)&(BUF), 0x00, sizeof(BUF))

/* //////////////////////////////////////////////////////////////////////////////////// */
#ifdef FW_SPACE
#  undef FW_SPACE
#endif
#define FW_SPACE(BUF)               memset((char *)&(BUF), 0x20, sizeof(BUF))

/* //////////////////////////////////////////////////////////////////////////////////// */
#ifdef FW_BLANK
#  undef FW_BLANK
#endif
#define FW_BLANK(BUF)               memset((char *)&(BUF), 0x20, sizeof(BUF))

/* //////////////////////////////////////////////////////////////////////////////////// */
#define FW_VALMEM(DEST, SRC)                                                              \
  do {                                                                                  \
    memcpy( (DEST), (SRC), Min(sizeof(DEST)-1,sizeof(SRC)) );                         \
    ((char *)&(DEST))[ Min(sizeof(DEST)-1,sizeof(SRC)) - 1 ] = 0x00;                  \
  }                                                                                     \
  while (0)

/* //////////////////////////////////////////////////////////////////////////////////// */
#define FW_VALSTR(DEST, SRC)                 XCUtil::FetchRTrimmed(Min(sizeof(DEST)-1,sizeof(SRC)), (DEST), (SRC))
#define FW_VALDBL(DEST, SRC)                 XCUtil::FetchValue(sizeof(SRC), (DEST), (SRC))
#define FW_VALINT(DEST, SRC)                 XCUtil::FetchValue(sizeof(SRC), (DEST), (SRC))

/* //////////////////////////////////////////////////////////////////////////////////// */
#define FW_FILSTR(DEST, SRC)                 XCUtil::FillCopy(sizeof(DEST), (DEST), (SRC))
#define FW_FILDBL(DEST, SRC, PREC)           XCUtil::FillCopy(sizeof(DEST), (DEST), (SRC), (PREC))
#define FW_FILINT(DEST, SRC)                 XCUtil::FillCopy(sizeof(DEST), (DEST), (SRC))

/* //////////////////////////////////////////////////////////////////////////////////// */
#define FW_RMZERO(SRC)                       XCUtil::ReplaceDeciZero(sizeof(SRC), (SRC))
#define FW_COMMA(SRC)                        XCUtil::PutComma(sizeof(SRC), (SRC))

/* //////////////////////////////////////////////////////////////////////////////////// */
#define FW_DBLx20(DEST, SRC)                 ( RxIsZero(SRC) ? memset((char *)&(DEST), 0x20, sizeof(DEST)) : (void *)NULL )
#define FW_INTx20(DEST, SRC)                 ( (SRC) == 0    ? memset((char *)&(DEST), 0x20, sizeof(DEST)) : (void *)NULL )

/* //////////////////////////////////////////////////////////////////////////////////// */
#define FW_MINCPY(DEST, SRC)                 strncpy((DEST),(SRC),Min(sizeof(DEST),sizeof(SRC)))
#define FW_ASSIGN(DEST, SRC)                 ((DEST) = (SRC))

/* //////////////////////////////////////////////////////////////////////////////////// */
#define FW_SQLCAT                            RxSTR_SqlCat

#define FW_UUID(X)                           XCUtil::generate_UUID (X, sizeof (X))
  
/* //////////////////////////////////////////////////////////////////////////////////// */
// ��¥��� ��ũ��
  
#define FW_ISYOON(year)                        XCUtil::IsYoon(year)
#define FW_COUNTDAYS(year, mon, day)           XCUtil::CountDays(year, mon, day)
#define FW_GETDATEVALUE(count, year, mon, day) XCUtil::GetDateValue(count, &year, &mon, &day)
#define FW_GETWEEKS(year, mon, day)            XCUtil::GetWeeks(year, mon, day)                            
#define FW_GETYEARMONTHWEEKS(buf, date)        XCUtil::GetYearMonthWeeks((char *)&buf, (char *)&date)                     
#define FW_GETWEEKSSTARTDATE(buf, date)        XCUtil::GetWeekStartDate((char *)&buf, (char *)&date)                      
#define FW_GETEVALWEEKDAY(buf, date, interval) XCUtil::GetEvalWeekDay((char *)&buf, (char *)&date, interval)          
#define FW_GETWEEKDAYS(date)                   XCUtil::GetWeekDays(date)                                        
 

/* //////////////////////////////////////////////////////////////////////////////////// */
typedef struct STRING_UNIT_S {
    char *   ptr      ;
    int      len      ;
  } StrUnit_S;

#ifndef IsSpace
#  define IsSpace(X)                         isspace(((unsigned char)(X)))
#endif

#ifndef Max
#  define Max(a,b)                           (((a)>(b))?(a):(b))
#endif
#ifndef Min
#  define Min(a,b)                           (((a)<(b))?(a):(b))
#endif

//API that EAI supported for IBK
#define FW_FTOSA0(x,val,d)        XCUtil::FTOSA0(x,sizeof(x),val,d) 
#define FW_ITOSA0(x,val)          XCUtil::ITOSA0(x,sizeof(x),val) 

#define FW_SA0TOF(val, size, d)   XCUtil::SA0TOF(val, size, d)
#define FW_SA0TOI(cal, size)      XCUtil::SA0TOI(val, size)


#define FW_ROUND(x, dig) ((float)(int)(x * pow(10.0, dig) + 0.5f))/ pow(10.0, dig)
#define FW_CEIL(x, dig)  (ceil ((x) * pow(10.0, dig)) / pow(10.0, dig))
#define FW_TRUNC(x, dig) (floor((x) * pow(10.0, dig)) / pow(10.0, dig))

#define FW_MAX(x,y) (((x)>(y)) ? (x) : (y))
#define FW_MIN(x,y) (((x)>(y)) ? (y) : (x))


class XCUBE_Export XCUtil
{
public:
// ���ں�ȯ ���
  static int       atoi (char *src, int len = -1);                                        // int <-> ���ڿ�
  static long      atol (char *src, int len = -1);
  static char *    itoa (char *buf, int len, int val);                                    // NULL ���� ���ڿ�
  static int       make_itoa (char *buf, int len, int val);                               // NULL ���� ���ڿ�, ���� = ũ��, ���� < 0
  static char *    zero_itoa (char *buf, int len, int val);                               // ũ�� ��ŭ �տ� '0' ä��, NULL ���� ���ڿ�
  static char *    lpad_itoa (char *buf, int len, int val, char ch=' ');                  // ũ�� ��ŭ �տ� ch  ä��, NULL ���� ���ڿ�
  static char *    rpad_itoa (char *buf, int len, int val, char ch=' ');                  // ũ�� ��ŭ �ڿ� ch  ä��, NULL ���� ���ڿ�
  static char *    zero_itoas (char *buf, int len, int val);                              // ũ�� ��ŭ �տ� '0' ä��, NULL ����
  static char *    lpad_itoas (char *buf, int len, int val, char ch=' ');                 // ũ�� ��ŭ �տ� ch  ä��, NULL ����
  static char *    rpad_itoas (char *buf, int len, int val, char ch=' ');                 // ũ�� ��ŭ �ڿ� ch  ä��, NULL ����

  static double    atof (char *src, int len = -1);                                         // double <-> ���ڿ�
  static char *    ftoa (char *buf, int len, double val, int dec = -1);                    // NULL ���� ���ڿ�
  static int       make_ftoa (char *buf, int len, double val, int dec = -1);               // NULL ���� ���ڿ�, ���� = ũ��, ���� < 0
  static char *    zero_ftoa (char *buf, int len, double val, int dec = -1);               // ũ�� ��ŭ �տ� '0' ä��, NULL ���� ���ڿ�
  static char *    lpad_ftoa (char *buf, int len, double val, int dec = -1, char ch=' ');  // ũ�� ��ŭ �տ� ch  ä��, NULL ���� ���ڿ�
  static char *    rpad_ftoa (char *buf, int len, double val, int dec = -1, char ch=' ');  // ũ�� ��ŭ �ڿ� ch  ä��, NULL ���� ���ڿ�
  static char *    zero_ftoas (char *buf, int len, double val, int dec = -1);              // ũ�� ��ŭ �տ� '0' ä��, NULL ����
  static char *    lpad_ftoas (char *buf, int len, double val, int dec = -1, char ch=' '); // ũ�� ��ŭ �տ� ch  ä��, NULL ����
  static char *    rpad_ftoas (char *buf, int len, double val, int dec = -1, char ch=' '); // ũ�� ��ŭ �ڿ� ch  ä��, NULL ����

  static double    xtof (char *src, int len = -1, int dec = -1);                           // double <-> �����ڸ������ڿ�  ( 8.2 -> 1.23 = 00000000123)
  static char *    ftox (char *buf, int len, double val, int dec = -1);                    // NULL ���� ���ڿ�
  static int       make_ftox (char *buf, int len, double val, int dec = -1);               // NULL ���� ���ڿ�, ���� = ũ��, ���� < 0
  static char *    zero_ftox (char *buf, int len, double val, int dec = -1);               // ũ�� ��ŭ �տ� '0' ä��, NULL ���� ���ڿ�
  static char *    lpad_ftox (char *buf, int len, double val, int dec = -1, char ch=' ');  // ũ�� ��ŭ �տ� ch  ä��, NULL ���� ���ڿ�
  static char *    rpad_ftox (char *buf, int len, double val, int dec = -1, char ch=' ');  // ũ�� ��ŭ �ڿ� ch  ä��, NULL ���� ���ڿ�
  static char *    zero_ftoxs (char *buf, int len, double val, int dec = -1);              // ũ�� ��ŭ �տ� '0' ä��, NULL ����
  static char *    lpad_ftoxs (char *buf, int len, double val, int dec = -1, char ch=' '); // ũ�� ��ŭ �տ� ch  ä��, NULL ����
  static char *    rpad_ftoxs (char *buf, int len, double val, int dec = -1, char ch=' '); // ũ�� ��ŭ �ڿ� ch  ä��, NULL ����

  static double SA0TOF(const char *val, int size, int d);
  static int    SA0TOI(const char *val, int size);

  static void   FTOSA0(char *out, int size, double val, int d);
  static void   ITOSA0(char *out, int size, int val);


// ��¥��ȯ ���
  static char*     get_date (char *date);
  static char*     get_time (char *date);
  static char*     get_datetime (char *date);
  static char*     get_datetimemsec (char *date);
  static char*     get_twdate (char *date);
  static char*     get_dateyy (char *date);
  static char*     get_gmdate (char *date);
  static char*     get_gmtime (char *date);
  static char*     get_gmdatetime (char *date);
  static int       twDate_to_weDate (char *tw, char *we);
  static int       weDate_to_twDate (char *we, char *tw);
  static char *    wdate_2_tdate (char *wdate, char *tdate, int len);
  static char *    tdate_2_wdate (char *tdate, char *wdate, int len);

// ��Ÿ
  static void      sleep (int seconds, int useconds);

// ���ں�ȯ ���
  static int       xtrim (char *src, int len);
  static char*     xrtrim (char *str);
  static int       xrtrimn (char *s, int s_len);
  static int       xrtrimn2 (char *dest, char *src, int n, char ch);
  static char *    xrtrimn3 (char *dest, char *src, int n, char ch);
  static char *    xrtrimn4 (char *dest, char *src, int n, char ch);
  static int       xstrcpy (char *tar,  char *src);
  static int       xstrncpy (char *tar,  char *src, int len);
  static char*     xpadncpy (char *tar,  char *src, int len);
  static char*     xpadncpy2 (char *tar, char *src, int len, char sep);
  static char*     xc_valcpy (char *tar, char *src, int len);

// �߰�
  static char * RTrim           (char *pSrc, int *npSrcLen=NULL);
  static char * LTrim           (char *pSrc, int *npSrcLen=NULL);
  static char * Trim            (char *pSrc, int *npSrcLen=NULL);

  static char * RemoveChar      (char *pSrc, const char nCh)          ;
  static char * RemoveDupSpace  (char *pSrc, int *npSrcLen=NULL)      ;
  static char * ToNoSpecialChar (char *pSrc, char nNewCh)             ;
  static char * SpaceToChar     (char *pSrc, int nSrcLen, char nNewCh);
  static char * ToLower         (char *pSrc)                          ;
  static char * ToUpper         (char *pSrc)                          ;

  static char * PutComma        (const char *pSrc, char *pSave);
  static char * HyphenAccount   (const char *pSrc, char *pSave);
  static char * HyphenDate      (const char *pSrc, char *pSave);
  static char * ColonTime       (const char *pSrc, char *pSave);

  static int    CountFound      (const char *pSrc, const char *pTgt)                                           ;
  static int    Split           (char *pSrc, const char *pDelimiter, StrUnit_S *pSUnitV, const int nSUnitVSz);

  static char * DoubleToNthSystemString   (double dValue, int nSystem, char *pSave)    ;
  static double NthSystemStringToDouble   (const char *pSrc, int nSystem)              ;
  static char * NthSystemToDecimalString  (const char *pSrc, int nSystem, char *pSave) ;

  static int    FillCopy        (const int nSpan, char *pTgt, const char           *pSrc                  );
  static int    FillCopy        (const int nSpan, char *pTgt, const long double     dSrc, const int nPrec );
  static int    FillCopy        (const int nSpan, char *pTgt, const double          dSrc, const int nPrec );
  static int    FillCopy        (const int nSpan, char *pTgt, const float           dSrc, const int nPrec );
  static int    FillCopy        (const int nSpan, char *pTgt, const long            nSrc                  );
  static int    FillCopy        (const int nSpan, char *pTgt, const unsigned long   nSrc                  );
  static int    FillCopy        (const int nSpan, char *pTgt, const int             nSrc                  );
  static int    FillCopy        (const int nSpan, char *pTgt, const unsigned int    nSrc                  );
  static int    FillCopy        (const int nSpan, char *pTgt, const short           nSrc                  );
  static int    FillCopy        (const int nSpan, char *pTgt, const unsigned short  nSrc                  );
  static int    FillCopy        (const int nSpan, char *pTgt, const char            nCh                   );
  static int    FillCopy        (const int nSpan, char *pTgt, const unsigned char   nCh                   );

  static int    FetchRTrimmed   (const int nSpan, unsigned char *pDest, const char *pSrc);
  static int    FetchRTrimmed   (const int nSpan, char          *pDest, const char *pSrc);
  static int    FetchValue      (const int nSpan, double        *pDest, const char *pSrc);
  static int    FetchValue      (const int nSpan, double        &mDest, const char *pSrc);
  static int    FetchValue      (const int nSpan, int           *pDest, const char *pSrc);
  static int    FetchValue      (const int nSpan, int           &mDest, const char *pSrc);

  static int    ReplaceDeciZero (const int nSpan, char *pSrc);
  static int    PutComma        (const int nSpan, char *pSrc);
  
  static char  *generate_UUID   (char *UUID, int size);
  
  
// ��¥ ��� �Լ�(��������).
  static int           IsYoon            (int year);                                          // ���޿���, �����̸� 1, �ƴϸ� 0
  static unsigned long CountDays         (int year,int month,int day);                        // AD1����� �Է¹��� ��¥������ �ϼ��� ����.
  static void          GetDateValue      (unsigned long count,int *year,int *month,int *day); // AD1����� �Էµ� �ϼ���(count) ���� �� ��¥�� year, month, day�� ������.
  
  static int           GetWeeks          (int year,int month,int day);                        // AD1����� �ش��ϱ����� �ּ��� ����.
  static char         *GetYearMonthWeeks (char *pBuff, char *pDate);                          // pDate���� �����ִ� �����(week)�� pBuff�� ����.
  static char         *GetWeekStartDate  (char *pBuff, char *pDate);                          // pDate���� �����ִ� ���� ù���� pBuff�� ����.
  static char         *GetEvalWeekDay    (char *pBuff, char *pDate, int interval);            // pDate���� interval�� �Ⱓ�� ���� ��¥(����)�� ���� pBuff�� ����. 
                                                                                              // ���� ���� �Ͽ����� ��� +interval+1 Ȥ�� -interval-1 �� ��.
  static int           GetWeekDays       (char *pDate);                                       // pDate�� ������ ����. 0 - sun .. 6 - sat


};
#endif  // _XCUTIL_H_


