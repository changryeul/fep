/***** Module : etc.c *****/
char*       AtoA( char *dst, char *src, int sz);                            /* convert */
char*       ItoA( char *rec, int value, int sz);                            /* convert */
char*       ItoA2( char *rec, int sz, int value);                           /* convert */
char*       DtoA( char *rec, double value, int sz);                         /* double to string */
char*       DtoAN( char *rec, double value, int sz, int n);                 /* double to string - 수숫점 자리, 반올림 (%sz.nf) */
char*       DtoAND( char *rec, double value, int sz, int n);                /* double to string - 수숫점 자리, 버림 (%sz.nf) */
char*       DtoANU( char *rec, double value, int sz, int n);                /* double to string - 수숫점 자리, 올림 */
char*       DtoALN( char *rec, double value, int sz, int n);                /* double to string - 수숫점 자리, 반올림 (%sz.nf), Left 정렬 */
char*       DtoALND( char *rec, double value, int sz, int n);               /* double to string - 수숫점 자리, 버림 (%sz.nf), Left 정렬 */
char*       DtoALNU( char *rec, double value, int sz, int n);               /* double to string - 수숫점 자리, 올림, Left 정렬 */
char*       DtoA0ND( char *rec, double value, int sz, int n);               /* double to string - 수숫점자리,버림(%sz.nf),0 채움 */
char*       DtoA0NU( char *rec, double value, int sz, int n);               /* double to string - 수숫점 자리,올림,0 채움 */
int         AtoI( char *rec, int sz);                                       /* convert */
long        AtoL( char *rec, int sz);                                       /* convert */
double      AtoF( char *rec, int sz);                                       /* convert */
double      AtoD( char *rec, int sz);                                       /* convert */
char*       LLtoA( char *rec, long long value, int sz);                     /* convert */
long        AtoLL( char *rec, int sz);                                      /* convert */
char*       TtoS( time_t atime);                                            /* convert */
int         TtoA( char *str, time_t atime);                                 /* convert */
int         TtoAF( char *str, time_t atime, char *format);                  /* convert */
time_t      StoT( char *tstr);                                              /* convert */
int         TtoD( time_t cur_time);                                         /* convert */
int         TtoI( time_t cur_time);                                         /* convert */
char*       GetTimeStr();                                                   /* convert */
unsigned    StoI( char *str);                                               /* convert */
int         Max( int a, int b);                                             /* convert */
int         Min( int a, int b);                                             /* convert */
int         Trim( char *rec);                                               /* convert */
int         TrimNR( char *rec, int len);                                    /* convert */
int         TrimN( char *rec, int len);                                     /* convert */
int         StrSetN( char *rec, int org, int dst, int len);                 /* convert */
int         IsHanGul( unsigned char *data, int pos);                        /* convert */
char*       GetPrevWeekDay( char *date);                                    /* 전일 날짜를 (YYYYMMDD) 형식으로 산출 */

