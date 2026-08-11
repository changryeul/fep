#pragma	once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#ifndef XCUBE_CONVERT_H
#define XCUBE_CONVERT_H	1

extern char	ConnName[64];
extern char	TnsName[ 64];

typedef struct _svc_info_
{
	char		test[ 128];
	char		svcname[ 128];
	char		tr_cd[ 128];
	char		con_name[ 128];
}	SVC_INFO;
typedef struct _svc_info_	TR_SVCINFO;
typedef struct _svc_info_	XCube_User_Message_Service;

/********************************** XCUBE define ****************************************************************/
#define	XCTCP_ETIMEOUT		1
#define	EL_INFO				1
#define     FW_SPRINTF( x, format, args...)     FwSprintf( x, sizeof( x), format, ##args)
#define		_FL_					__FUNCTION__, __LINE__
#define		SQLCODE					sqlca.sqlcode
#define		SQLNOTFOUND				1403
#define		SQLERRMSG				sqlca.sqlerrm.sqlerrmc
#define		RET_FAIL				-1
#define		RET_SUCCESS				0
#define		SQLSUCCESS				0
#define		SQLDUPLICATE			-1	/* or -2601 */
#define		FW_CLEAR( x)			FwClear( x, sizeof( x))
#define		FW_SPACE( x)			FwSpace( x, sizeof( x))
#define		FW_FILCPY( x, y)		FwFilcpy( x, sizeof( x), y, sizeof( y))
#define		FW_MEMCPY( x, y, z)		FwMemcpy( x, sizeof( x), y, sizeof( y), z)
#define		FW_MEMCMP( x, y, z)		FwMemcmp( x, sizeof( x), y, sizeof( y), z)
#define		FW_MEMSET( x, y, z)		memset( x, y, z)
#define		FW_STRCMP( x, y)		FwStrcmp( x, sizeof( x), y, sizeof( y))
#define		FW_STRCPY( x, y)		FwStrcpy( x, sizeof( x), y, sizeof( y))
#define		FW_ATOF( x)				FwAtoF( x, sizeof( x))
#define		FW_ATOI( x)				FwAtoI( x, sizeof( x))
#define		FW_ATOIN( x, y)			FwAtoI( x, y)
#define		FW_ZITOAS( x, y)		FwZItoA( x, sizeof( x), y)
#define		FW_STRNCMP( x, y, z)	strncmp( x, y, z)
#define		GET_FILE_NAME( x)		GetFileName( x)

#define FW_GET_DATE(x)			GetDate(x)								// YYYYMMDD
#define FW_GET_TIME(x)			GetTime(x)								// HHMMSS
#define FW_GET_DTIME(x)			GetDateTime(x)							// YYYYMMDDHHMMSS
#define FW_GET_DTIMEMS(x)		GetDateTimeMsec(x)						// YYYYMMDDHHMMSSmmmmmm
#define	get_datetime( x)		GetDateTime(x)
/***************************************************************************************************************/


typedef struct _tcp_server_
{
	int		(* IP_ADDR)();
	int		(* PORT_NO)();
	int		(* SVR_IP)();
	int		(* SVR_PORT)();
	int		(* connect)( );
	int		(* open)( );
	int		(* close)( );
	int		(* send)( );
	int		(* recv)( );
}	TCP_SERVER;

typedef struct _tcp_server_ TCP_CLIENT;

typedef struct _env_
{
	int		(* open)( char *file);
	char*	(* get)( char *x, char *y);
}	ENV;

typedef struct _dt_
{
	int		(* year)();
	int		(* month)();
	int		(* day)();
}	DT;
typedef DT	ACE_Date_Time;

#endif /* XCUBE_CONVERT_H */

/***** Module : xcube_convert.c *****/
int         FwClear( char *dst, int sz);
int         FwSpace( char *dst, int sz);
int         FwFilcpy( char *dst, int dst_sz, char *src, int src_sz);
int         FwSprintf( char *dst, int dst_sz, char *format, ...);
void*       FwMemcpy( char *dst, int dst_sz, char *src, int src_sz, int cpy_sz);
void*       FwMemcpyN( char *dst, int dst_sz, char *src, int src_sz, int cp_sz);
int         FwMemcmp( char *dst, int dst_sz, char *src, int src_sz, int sz);
int         FwStrcmp( char *dst, int dst_sz, char *src, int src_sz);
void*       FwStrcpy( char *dst, int dst_sz, char *src, int src_sz);
double      FwAtoF( char *dst, int dst_sz);
int         FwAtoI( char *dst, int dst_sz);
int         FwZItoA( char *dst, int dst_sz, int val);
int         GET_DATE( char *dst);
int         IntToAsc( char *rec, int sz, int val);
int         AscToInt( char *rec, int sz);
char*       GetEnv( char *name, char *file);
int         EnvOpen( char *file);
ENV*        Env( ENV *env);
char*       PrgEnv( char *file);
int         ToUpper( int arg);
int         GetDate( char *rec);
int         GetTime( char *rec);
int         GetDateTime( char *rec);
int         GetDateTimeMsec( char *rec);
char*       GetFileName( char *name);

