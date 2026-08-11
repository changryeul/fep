#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include "log.h"


extern const char *const	sys_errlist[];
extern int			errno;

LOG				*LogPtr = NULL;


/* int		Log_Write( int LogLev, char *Format, ...); */
int		Log_Init( char *Fname, char *PgName, int LogLev);
LOG*	Log_Open( char *LogFileName);
int		Log_Type( int LogOpt, int LogSz);
int		Log_SetHead( char *LogBuffer);
int		Log_Check( char *LogFileName, int LogOption);
int		Log_MakeBackup( char *LogFileName);
int		Log_Display( FILE *LogFilePtr, char *Buffer, int BufLen);
int		Log_Create( char *LogFileName);

void		Log_HexDump( FILE *LogFilePtr, char *Buf, long BufLen);
void		Log_HexInitBuf( char *Buf, long Cnt);
char    	*StrTime( time_t *Time);

LOG	*Log_Open( char *f_name)
{
	LOG	*log = 0;
	char	*p;

	log = malloc( sizeof( LOG));
	memset( log, 0, sizeof( LOG));

	log->head |= LOG_HEAD_COLOR;
	log->head |= LOG_HEAD_BELL;

	log->fd = -1;

	if( f_name != NULL)
	{
		p = strrchr( f_name, '/');
		if( p != NULL)
		{
			log->f_name = malloc( strlen( p +1) +1);
			memcpy( log->f_name, p +1, strlen( p +1) +1);
			log->f_path = malloc( (int)( p - f_name) +2);
			memcpy( log->f_path, f_name, (int)( p - f_name) +1);
			log->f_path[ (int)( p - f_name) +2] = 0;
		}
		else
		{
			log->f_name = malloc( strlen( f_name) +1);
			memcpy( log->f_name, f_name, strlen( f_name) +1);
			log->f_path = NULL;
		}
		log->head &= ~LOG_HEAD_COLOR;
		log->type = 0;

		log->fd = open( f_name, O_APPEND + O_EXCL + O_RDWR, LOG_FILE_MODE);
		if( log->fd < 0)
		{
			if( errno == 2)
			{
				log->fd = open( f_name, O_CREAT + O_RDWR, LOG_FILE_MODE);
				if( log->fd < 0)
				{
					printf( "log file open error. name=[%s] err=[%d:%s]\n", f_name, errno, strerror( errno));
					if( log->f_name != NULL) 	free( log->f_name);
					if( log->f_path != NULL)	free( log->f_path);
					log->fd = 0;
					return 0;
				}
			}
			else
			{
				printf( "log file open error. name=[%s] err=[%d:%s]\n", f_name, errno, strerror( errno));
				if( log->f_name != NULL) 	free( log->f_name);
				if( log->f_path != NULL)	free( log->f_path);
				log->fd = 0;
				return 0;
			}
		}
	}

	log->buf = malloc( LOG_BUFFER_SIZE + LOG_HEAD_SIZE +1);
	memset( log->buf, 0, LOG_BUFFER_SIZE);

	log->tail = malloc( 2);
	memmove( log->tail, "\n\0", 2);

	return log;
}

int	Log_Close( LOG *log)
{
	if( log->fd > 0)			close( log->fd);
	if( log->tail	!= NULL)	free( log->tail);
	if( log->buf	!= NULL)	free( log->buf);
	if( log->f_name != NULL) 	free( log->f_name);
	if( log->f_path != NULL)	free( log->f_path);
	free( log);

	return 1;
}

int	Log_Write( const char *file, const char *func, int line, int level, LOG *log, char *format, ...)
{
	va_list		args;
	int			sz, rtn;
	int			hd_sz = 0;

	if( log == NULL)
	{
		log = LogPtr = Log_Open( NULL);
		if( log == NULL)
		{
			printf( "Log_Open error. err=[%d:%s]\n", errno, strerror( errno));
			return -1;
		}
	}
	if( level < log->level) return 0;

	hd_sz = Log_Head( log, file, func, line, level);
	sz = hd_sz;

	va_start( args, format);
	sz += vsnprintf( &log->buf[ sz], LOG_BUFFER_SIZE - hd_sz, format, args);
	va_end( args);

	if( level == LOG_LEV_ERR)
	{
		if( LOG_BUFFER_SIZE - sz > 0) 
		sz += snprintf( &log->buf[ sz], LOG_BUFFER_SIZE - sz, " Err=[%d:%s]", errno, strerror( errno));
	}

	if( log->tail != NULL)
	{
		sz += snprintf( &log->buf[ sz], LOG_BUFFER_SIZE - sz, "%s", log->tail);
	}
	log->buf[ sz] = 0;


	if( level == LOG_LEV_USR)
	{
		if( log->func != NULL) rtn = log->func( log->buf, sz);
	}
	else
	{
		Log_Out( log, log->buf, sz);
	}

	return sz;
}

int	Log_Head( LOG *log, const char *file, const char *func, int line, int level)
{
	int		sz = 0;
	int		sz_hd = 0;
	time_t		cur_time;
	struct tm	*tp;
	struct timeval	tv;
	char		*lv = "DMALWECB U     \0";
	int		color[16] = { 37, 37, 36, 32, 33, 31, 31, 31, 31, 37, 37};

	sz_hd = LOG_HEAD_SIZE;
	time( &cur_time);
	tp = localtime( &cur_time);
	gettimeofday( &tv, NULL);

	sz += sprintf( &log->buf[ sz], "%02d:%02d:%02d-%06d ", tp->tm_hour, tp->tm_min, tp->tm_sec, tv.tv_usec);
	if( log->head & LOG_HEAD_COLOR)
	{
		sz_hd += 9;
		sz += sprintf( &log->buf[ sz], "\033[%dm%c\033[0m ", color[ level], lv[ level]);
	}
	else
	{
		sz += sprintf( &log->buf[ sz], "%c ", lv[ level]);
	}
	sz += sprintf( &log->buf[ sz], "%s:%s(%d)  ", file, func, line);
	if( log->head & LOG_HEAD_BELL)
	{
		if( level == 7) log->buf[ sz++] = '';
	}
	if( sz <= sz_hd)
	{
		memset( &log->buf[ sz], 0x20, sz_hd - sz);
		sz = sz_hd;
	}

	return sz;
}

int Log_Tail( LOG *log, char *tail, int sz)
{
	if( log == NULL)
	{
		log = LogPtr = Log_Open( NULL);
		if( log == NULL)
		{
			printf( "Log_Open error. err=[%d:%s]\n", errno, strerror( errno));
			return -1;
		}
	}

	log->tail = realloc( log->tail, sz +1);
	if( log->tail == NULL)
	{
		printf( "Log_Tail: realloc error. Err=[%d:%s]\n", errno, strerror( errno));
		return -errno;
	}

	memcpy( log->tail, tail, sz);
	log->tail[ sz] = 0;

	return 1;
}

int Log_File( LOG *log, char *f_name)
{
	char		*p;

	if( log->fd > 0) return 0;
	if( f_name == NULL)
	{
		if( log->fd > 0)
		{
			close( log->fd);
			if( log->fd > 0)			close( log->fd);
			if( log->f_name != NULL) 	free( log->f_name);
			if( log->f_path != NULL)	free( log->f_path);
			log->fd = 0;
			log->f_name = NULL;
			log->f_path = NULL;
		}
		return 0;
	}

	p = strrchr( f_name, '/');
	if( p != NULL)
	{
		log->f_name = malloc( strlen( p +1) +1);
		memcpy( log->f_name, p +1, strlen( p +1) +1);
		log->f_path = malloc( (int)( p - f_name) +2);
		memcpy( log->f_path, f_name, (int)( p - f_name) +1);
		log->f_path[ (int)( p - f_name) +2] = 0;
	}
	else
	{
		log->f_name = malloc( strlen( f_name) +1);
		memcpy( log->f_name, f_name, strlen( f_name) +1);
		log->f_path = NULL;
	}
	log->head &= ~LOG_HEAD_COLOR;
	log->type = 0;

	log->fd = open( f_name, O_APPEND + O_EXCL + O_RDWR, LOG_FILE_MODE);
	if( log->fd < 0)
	{
		if( errno == 2)
		{
			log->fd = open( f_name, O_CREAT + O_RDWR, LOG_FILE_MODE);
			if( log->fd < 0)
			{
				printf( "log file open error. name=[%s] err=[%d:%s]\n", f_name, errno, strerror( errno));
				if( log->f_name != NULL) 	free( log->f_name);
				if( log->f_path != NULL)	free( log->f_path);
				log->fd = 0;
				return 0;
			}
		}
		else
		{
			printf( "log file open error. name=[%s] err=[%d:%s]\n", f_name, errno, strerror( errno));
			if( log->f_name != NULL) 	free( log->f_name);
			if( log->f_path != NULL)	free( log->f_path);
			log->fd = 0;
			return 0;
		}
	}

	return log->fd;
}

Log_Func( LOG *log, int (* func)( char *rec, int sz))
{
	log->func = func;
	return 1;
}

Log_Out( LOG *log, char *data, int sz)
{
	int		rtn;

	if( log->f_name == NULL)
	{
		rtn = write( 1, data, sz);
		return rtn;
	}

	rtn = Log_FileCheck( log);
	if( rtn < 0) return -1;
	if( log->fd < 0) return 0;

	rtn = write( log->fd, data, sz);
	if( rtn < 0)
	{
		printf( "log write error. err=[%d:%s]\n", errno, strerror( errno));
	}

	if( log->type == LOG_TYPE_APPEND)	
	{
		close( log->fd);
		log->fd = -1;
	}

	return rtn;
}

Log_Hex( LOG *log, char *data, int sz)
{
	int		pos = 0, base;
	char	rec[ 256];

	while( pos < sz)
	{
		memset( rec, 0x20, 256);
		sprintf( rec, "%08X", pos);
		rec[ 8] = 0x20;

		while( pos < sz)
		{
			sprintf( &rec[ 10 + ( pos % 16) * 3], "%02X ", data[ pos]);
			sprintf( &rec[ 60 + ( pos % 16)], "%c ", data[ pos]);
			rec[ 70] = '\n';
			pos++;
			if( pos % 16 == 0) break;
		}
		Log_Out( log, rec, 71);
	}
}

Log_Dec( LOG *log, char *data, int sz)
{
	int		pos = 0, base;
	char	rec[3][ 256];

	sprintf( rec[ 0], "          0         1         2         3         4         5        6        7        8         9\n");
	Log_Out( log, rec[ 0], 100);
	while( pos < sz)
	{
		memset( rec[ 0], 0x20, 256);
		memset( rec[ 1], 0x20, 256);
		memset( rec[ 2], 0x20, 256);
		sprintf( rec[ 0], "%06d", pos);
		rec[ 0][ 6] = 0x20;

		while( pos < sz)
		{
			if( isgraph( data[ pos])) 	sprintf( &rec[ 0][ pos % 100 + 10], "%c", data[ pos]);
			else 						sprintf( &rec[ 0][ pos % 100 + 10], ".");
			sprintf( &rec[ 1][ pos % 100 + 10], "%x", ( data[ pos] & 0xF0) >> 4);
			sprintf( &rec[ 2][ pos % 100 + 10], "%x", data[ pos] & 0x0F);
			pos++;
			if( pos % 100 == 0) break;
		}
		sprintf( &rec[ 0][ 100 + 10], "\n");
		sprintf( &rec[ 1][ 100 + 10], "\n");
		sprintf( &rec[ 2][ 100 + 10], "\n");
		Log_Out( log, rec[ 0], 111);
		Log_Out( log, rec[ 1], 111);
		Log_Out( log, rec[ 2], 111);
	}
	sprintf( rec[ 0], "          0         1         2         3         4         5        6        7        8         9\n");
	Log_Out( log, rec[ 0], 100);
}

Log_FileCheck( LOG *log)
{
	int			rtn;
	time_t		cur_time;
	struct tm	*tp;
	char		f_name[ 512];

	switch( log->type)
	{
		case LOG_TYPE_APPEND:
		case LOG_TYPE_STATIC:	/* static */
			if( log->fd >= 0) break;
			time( &cur_time);
			sprintf( f_name, "%s/%s", log->f_path, log->f_name);
			log->fd = open( f_name, O_APPEND | O_CREAT | O_RDWR, LOG_FILE_MODE);
			if( log->fd < 0)
			{
					printf( "log file open error. err=[%d:%s]\n", errno, strerror( errno));
					log->fd = -1;
					log->fd_time = 0;
			}
			log->fd_time = cur_time;
			return 1;
		case LOG_TYPE_DAILY:	/* daily */
			time( &cur_time);
			cur_time -= cur_time%( time_t)( 24*3600) + ( time_t)( 9*3600);
			if( log->fd_time == cur_time) break;
			if( log->fd > 3)
			{
				rtn = close( log->fd);
				if( rtn < 0)
				{
					printf( "log file close error. err=[%d:%s]\n", errno, strerror( errno));
				}
			}
			log->fd_time = cur_time;
			tp = localtime( &cur_time);
			sprintf( f_name, "%s/%s.%04d%02d%02d", log->f_path, log->f_name, 
					tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday);
			log->fd = open( f_name, O_APPEND | O_CREAT, LOG_FILE_MODE);
			if( log->fd < 0)
			{
					printf( "log file open error. err=[%d:%s]\n", errno, strerror( errno));
					log->fd = -1;
					log->fd_time = 0;
			}
			return 1;
		default:
			break;
			
	}

	return 0;

}

