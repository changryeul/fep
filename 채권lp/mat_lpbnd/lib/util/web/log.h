#ifndef LOG_H
#define	LOG_H 1
#include <time.h>

#define		LOG_BUFFER_SIZE		65000
#define		LOG_HEAD_SIZE		50				/* color +9 */
#define		LOG_HEAD_TYPE		0
#define		LOG_HEAD_COLOR		0x00000100
#define		LOG_HEAD_BELL		0x00000200
#define		LOG_TYPE_DAILY		1				/* file_name.YYYYMMDD */
#define		LOG_TYPE_STATIC		0				/* file_name */
#define		LOG_TYPE_APPEND		2				/* open/close นÝบน */
#define		LOG_FILE_MODE		0644				/* create mode  -rw-r--r-- */

#define		LOG_LEV_DBG		0
#define		LOG_LEV_MSG		1
#define		LOG_LEV_APP		2
#define		LOG_LEV_LIB		3
#define		LOG_LEV_WAR		4
#define		LOG_LEV_ERR		5
#define		LOG_LEV_CRI		6
#define		LOG_LEV_BELL	7
#define		LOG_LEV_USR		9

typedef struct _log_
{
	int		dump;
	int		head;
	int		level;
	int		fd;
	time_t	fd_time;
	int		type;
	char	*f_path;
	char	*f_name;
	char	*buf;
	char	*tail;
	int		(*func)( char *msg, int sz);
}	LOG;

extern LOG	*LogPtr;

#define 	LogUsr( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, 9, LogPtr, format, ##args)
#define 	LogBel( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, 7, LogPtr, format, ##args)
#define 	LogCri( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, 6, LogPtr, format, ##args)
#define 	LogErr( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, 5, LogPtr, format, ##args)
#define 	LogWar( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, 4, LogPtr, format, ##args)
#define 	LogLib( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, 3, LogPtr, format, ##args)
#define 	LogApp( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, 2, LogPtr, format, ##args)
#define 	LogMsg( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, 1, LogPtr, format, ##args)
#define 	LogDbg( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, 0, LogPtr, format, ##args)
#define 	LogTst( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, 0, LogPtr, format, ##args)
#define 	LogDel( format, args...)	/* Log_Write( __FILE__, __FUNCTION__, __LINE__, 0, LogPtr, format, ##args) */
#define		LogOpen( f_name)			LogPtr = Log_Open( f_name);
#define		LogFile( f_name)			Log_File( LogPtr, f_name);
#define		LogClose()					Log_Close( LogPtr);
#define		LogFunc( func)				Log_Func( LogPtr, func);
#define		LogDbgFlag					1
#define		LogDelFlag					0

#endif


/***** Module : log.c *****/
LOG*        Log_Open( char *f_name);
int         Log_Close( LOG *log);
int         Log_Write( const char *file, const char *func, int line, int level, LOG *log, char *format, ...);
int         Log_Head( LOG *log, const char *file, const char *func, int line, int level);
int         Log_Tail( LOG *log, char *tail, int sz);
int         Log_File( LOG *log, char *f_name);
int         Log_Func( LOG *log, int (* func)( char *rec, int sz));
int         Log_Put( LOG *log, int sz);
int         Log_Out( LOG *log, char *data, int sz);
int         Log_Hex( LOG *log, char *data, int sz);
int         Log_Dec( LOG *log, char *data, int sz);
int         Log_FileCheck( LOG *log);

