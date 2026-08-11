/** ***************************************************************************
**	@file		log.c
**	@date		2022/08/23
**	@author		최동춘
**	@version	V2.0.20220823
**	@brif		
**	로그 파일 관리 라이브러리
***************************************************************************** */
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
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

#if 0 /* ERRMSG */
#include "errmsg.h"
#endif
#include "log.h"
#include "etc.h"


extern const char *const	sys_errlist[];
extern int			errno;

LOG				*LogPtr = NULL;
extern LOG		*LsmPtr;


/* int		Log_Write( int LogLev, char *Format, ...); */
int		Log_Init( char *Fname, char *PgName, int LogLev);
LOG*	Log_Open( char *LogFileName);
int		Log_SetHead( char *LogBuffer);
int		Log_Check( char *LogFileName, int LogOption);
int		Log_MakeBackup( char *LogFileName);
int		Log_Display( FILE *LogFilePtr, char *Buffer, int BufLen);
int		Log_Create( char *LogFileName);

void		Log_HexDump( FILE *LogFilePtr, char *Buf, long BufLen);
void		Log_HexInitBuf( char *Buf, long Cnt);
char    	*StrTime( time_t *Time);


/** ***************************************************************************
**	@fn			LOG *Log_Open( char *f_name)
**	@param		char *f_name - 파일이름 NULL:stdout
**	@return		LOG 구조체 pointer
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	로그 파일을 사용하기위한 초기화 작업
**	AP에서 LogOpen(헤더에 define 되어있는)을 호출하지 않으면 
**	전역 변수 LogPtr을 사용 - stdout으로 로그 출력
**	LogOpen을 AP에서 호출하면 그때부터 f_name에 로그 출력
***************************************************************************** */
LOG	*Log_Open( char *f_name)
{
	int			rtn;
	char		*ptr;
	LOG			*log = 0;

	log = ( LOG *)malloc( sizeof( LOG));
	if( log == NULL) 
	{
		LogErr( "log malloc error. sz=[%d] err=[%d:%s]", sizeof( LOG), errno, strerror( errno));
		goto error;
	}
	ptr = memset( log, 0, sizeof( LOG));
	if( ptr == NULL)
	{
		goto error_1;
	}

	log->type = LOG_TYPE_DAILY;
	log->head = LOG_HEAD_0;

	log->fd = -1;
	log->f_name = NULL;
	log->f_path = NULL;
	log->b_path = NULL;
	log->mem    = NULL;

	if( f_name == NULL)
	{
		log->fd = STDOUT_FILENO;
		log->fd_type = 0;
		log->sz = 0;
		log->fd_time = 0;
#if 0
		log->head |= LOG_HEAD_COLOR;
		log->head |= LOG_HEAD_BELL;
#endif
	}
	else
	{
		rtn = Log_FilePath( log, f_name);
		if( rtn < 0)
		{
			goto error_1;
		}
		rtn = Log_FileOpen( log);
	}

	log->buf = ( char *)malloc( LOG_BUFFER_SIZE + LOG_HEAD_SIZE +1);
	memset( log->buf, 0, LOG_BUFFER_SIZE);

#if 1
	log->tail = ( char *)malloc( 2);
	memmove( log->tail, LOG_TAIL_STR, 2);
	log->tail_sz = strlen( LOG_TAIL_STR);
#endif

	return log;

	error_1:
		free( log);
	error:
		return NULL;
}

int	Log_Close( LOG *log)
{
	if( log->fd > 0)			close( log->fd);
	if( log->p_name != NULL)	free( log->p_name);
	if( log->tail	!= NULL)	free( log->tail);
	if( log->buf	!= NULL)	free( log->buf);
	if( log->f_name != NULL) 	free( log->f_name);
	if( log->f_path != NULL)	free( log->f_path);
	if( log->b_path != NULL)	free( log->b_path);
	free( log);

	return 1;
}

int Log_Type( LOG *log, int type)
{
	if( log == NULL)
	{
		log = Log_Open( NULL);
		if( log == NULL)
		{
			LogCri( "Log_Open error. err=[%d:%s]", errno, strerror( errno));
			return -1;
		}
		if( LogPtr == NULL) LogPtr = log;
	}

	log->type = type;

	return log->type;
}

int	Log_Level( LOG *log, int level)
{
	if( log == NULL)
	{
		log = Log_Open( NULL);
		if( log == NULL)
		{
			LogCri( "Log_Open error. err=[%d:%s]", errno, strerror( errno));
			return -1;
		}
		if( LogPtr == NULL) LogPtr = log;
	}

	log->level = level;

	return log->level;
}

size_t Log_Size( LOG *log, size_t sz)
{
	if( log == NULL)
	{
		log = Log_Open( NULL);
		if( log == NULL)
		{
			LogCri( "Log_Open error. err=[%d:%s]", errno, strerror( errno));
			return -1;
		}
		if( LogPtr == NULL) LogPtr = log;
	}

	log->type = LOG_TYPE_SIZE;
	log->sz = sz;
	/* LogDbg( "LOGDBG Log_Size .... log->type=[0x%08x] sz=[%lld]\n", log->type, log->sz); */

	return log->sz;
}

int	Log_Write( const char *file, const char *func, int line, int level, LOG *log, char *format, ...)
{
	va_list		args;
	int			sz, rtn, msg_sz;
	int			hd_sz = 0;

	if( log == NULL)
	{
		log = Log_Open( NULL);
		if( log == NULL)
		{
			LogCri( "Log_Open error. err=[%d:%s]", errno, strerror( errno));
			return -1;
		}
		if( LogPtr == NULL) LogPtr = log;
	}
#if 0
	if( log->mem != NULL)
	{
		sz = Lsm_Write( file, func, line, level, log, format, args);
		return sz;
	}
#endif

	/* log level에 따라 거름 */
	if( level < log->level) return 0;

#if 0
	/* 마지막 로그 문자가 개행문자가 아니면 개행문자를 입력하여 정상로그를 첫 컬럼에서부터 쓴다 */
	if( log->last != log->tail[ log->tail_sz -1])		Log_Out( log, log->tail, log->tail_sz);
#endif

	log->file = ( char *)file;
	log->func = ( char *)func;
	log->line = line;

	hd_sz = Log_Head( log, file, func, line, level);
	sz = hd_sz;

	va_start( args, format);
	sz += vsnprintf( &log->buf[ sz], LOG_BUFFER_SIZE - hd_sz, format, args);
	va_end( args);

	if( sz > 60000)
	{
		msg_sz = sz;
		sz = hd_sz;
		sz += sprintf( &log->buf[ sz], "data size too long. sz=[%d]", msg_sz);
	}

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
		if( log->user != NULL) 
		{
			rtn = log->user( log->user_data, log->buf, sz);
			if( rtn < 0)
			{
				return -1;
			}
		}
	}
	else
	{
		Log_Out( log, log->buf, sz);
	}

	return sz;
}

#if 0
int	Log_WriteMem( const char *file, const char *func, int line, int level, LOG *log, char *format, ...)
{
	va_list		args;
	int			sz, rtn, msg_sz;
	int			hd_sz = 0;

	int				pos;
	LOG_RECORD		*rec;
	LSM_HEAD		*head;
	LOG_INDEX		*index;

	if( log == NULL)
	{
		log = Log_Open( NULL);
		if( log == NULL)
		{
			LogCri( "Log_Open error. err=[%d:%s]", errno, strerror( errno));
			return -1;
		}
		if( LogPtr == NULL) LogPtr = log;
	}

	/* log level에 따라 거름 */
	if( level < log->level) return 0;

	if( log->mem != NULL)
	{
		pos = 0;
		while( pos <= 0) pos = Lsm_GetEmptyRecordPos( log->mem, 1000);
		rec  = &log->mem->map->rec[ pos];
		head = &rec->head;
		memcpy( head->file, file, strlen( file) +1);
		memcpy( head->func, func, strlen( func) +1);
		head->line      = line;
		head->level     = level;
		head->index_pos = log->mem->index_pos;
		index			= &log->mem->map->index[ log->mem->index_pos];
		gettimeofday( &head->w_time, NULL);

		va_start( args, format);
		head->sz = vsnprintf( rec->rec, LOG_REC_SZ, format, args);
		va_end( args);
		if( level == LOG_LEV_ERR)
		{
			head->sz += snprintf( &rec->rec[ head->sz], LOG_REC_SZ - head->sz, " Err=[%d:%s]", errno, strerror( errno));
		}
		if( log->tail != NULL)
		{
			head->sz += snprintf( &rec->rec[ head->sz], LOG_REC_SZ - head->sz, "%s", log->tail);
		}

		log->mem->pos = pos;
		index->cnt++;
		index->sum++;
		/* write time은 daemon에서 stat로 구해서 write 
		time( &index->w_time);
		*/

		Lsm_Insert( log->mem);

		return head->sz;
	}
	else
	{
		log->file = ( char *)file;
		log->func = ( char *)func;
		log->line = line;

		hd_sz = Log_Head( log, file, func, line, level);
		sz = hd_sz;

		va_start( args, format);
		sz += vsnprintf( &log->buf[ sz], LOG_BUFFER_SIZE - hd_sz, format, args);
		va_end( args);
	}

#if 0
	/* 마지막 로그 문자가 개행문자가 아니면 개행문자를 입력하여 정상로그를 첫 컬럼에서부터 쓴다 */
	if( log->last != log->tail[ log->tail_sz -1])		Log_Out( log, log->tail, log->tail_sz);
#endif

	if( sz > 60000)
	{
		msg_sz = sz;
		sz = hd_sz;
		sz += sprintf( &log->buf[ sz], "data size too long. sz=[%d]", msg_sz);
	}

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
		if( log->user != NULL) 
		{
			rtn = log->user( log->user_data, log->buf, sz);
			if( rtn < 0)
			{
				return -1;
			}
		}
	}
	else
	{
		Log_Out( log, log->buf, sz);
	}

	return sz;
}
#endif

int	Log_NoHeadWrite( const char *file, const char *func, int line, int level, LOG *log, char *format, ...)
{
	va_list		args;
	int			sz, rtn;
	int			hd_sz = 0;

	if( log == NULL)
	{
		log = Log_Open( NULL);
		if( log == NULL)
		{
			LogCri( "Log_Open error. err=[%d:%s]", errno, strerror( errno));
			return -1;
		}
		if( LogPtr == NULL) LogPtr = log;
	}
	if( level < log->level) return 0;

	log->file = ( char *)file;
	log->func = ( char *)func;
	log->line = line;

	/*
	hd_sz = Log_Head( log, file, func, line, level);
	*/
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
		if( log->user != NULL) rtn = log->user( log->user_data, log->buf, sz);
	}
	else
	{
		rtn = Log_Out( log, log->buf, sz);
		if( rtn < 0)
		{
			return rtn;
		}
	}

	return sz;
}

int	Log_RawWrite( const char *file, const char *func, int line, int level, LOG *log, char *format, ...)
{
	va_list		args;
	int			sz, rtn;
	int			hd_sz = 0;

	if( log == NULL)
	{
		log = Log_Open( NULL);
		if( log == NULL)
		{
			LogCri( "Log_Open error. err=[%d:%s]", errno, strerror( errno));
			return -1;
		}
		if( LogPtr == NULL) LogPtr = log;
	}
	if( level < log->level) return 0;

	log->file = ( char *)file;
	log->func = ( char *)func;
	log->line = line;

	sz = hd_sz;

	va_start( args, format);
	sz += vsnprintf( &log->buf[ sz], LOG_BUFFER_SIZE - hd_sz, format, args);
	va_end( args);

	if( level == LOG_LEV_USR)
	{
		if( log->user != NULL) 
		{
			rtn = log->user( log->user_data, log->buf, sz);
			if( rtn < 0)
			{
				return rtn;
			}
		}
	}
	else
	{
		rtn = Log_Out( log, log->buf, sz);
		if( rtn < 0)
		{
			return rtn;
		}
	}

	return sz;
}

int	Log_Head( LOG *log, const char *file, const char *func, int line, int level)
{
	int				sz = 0;
	int				sz_hd = 0;
	int				pos;
	struct tm		*tp;
	struct timeval	tv;
	char			lv[128] = "TDAMLWECB U                               \0";
	int				color[16] = { 37, 37, 36, 32, 33, 31, 31, 31, 31, 37, 37};

	pos = level / 10;

	switch( log->head)
	{
		default:
		case LOG_HEAD_0 :
			sz_hd = LOG_HEAD_SIZE;
			gettimeofday( &tv, NULL);
			tp = localtime( &tv.tv_sec);
	
			sz += sprintf( &log->buf[ sz], "%02d:%02d:%02d-%06ld ", tp->tm_hour, tp->tm_min, tp->tm_sec, tv.tv_usec);
			if( log->head & LOG_HEAD_COLOR)
			{
				sz_hd += 9;
				sz += sprintf( &log->buf[ sz], "\033[%dm%c\033[0m ", color[ pos], lv[ pos]);
			}
			else
			{
				sz += sprintf( &log->buf[ sz], "%c ", lv[ pos]);
			}
			sz += sprintf( &log->buf[ sz], "%s:%s(%d)  ", file, func, line);
			if( log->head & LOG_HEAD_BELL)
			{
				if( level == LOG_LEV_BELL) log->buf[ sz++] = '';
			}
			if( sz <= sz_hd)
			{
				memset( &log->buf[ sz], 0x20, sz_hd - sz);
				sz = sz_hd;
			}
			break;
	}

	return sz;
}

int Log_Tail( LOG *log, char *tail, int sz)
{
	if( log == NULL)
	{
		log = Log_Open( NULL);
		if( log == NULL)
		{
			LogCri( "Log_Open error. err=[%d:%s]", errno, strerror( errno));
			return -1;
		}
		if( LogPtr == NULL) LogPtr = log;
	}

	log->tail = ( char *)realloc( log->tail, sz +1);
	if( log->tail == NULL)
	{
		LogErr( "Log_Tail: realloc error. Err=[%d:%s]", errno, strerror( errno));
		return -errno;
	}

	memcpy( log->tail, tail, sz);
	log->tail[ sz] = 0;

	return 1;
}

int Log_GetFd( LOG *log)
{
	return log->fd;
}

int Log_Name( LOG *log, char *p_name)
{
	int		sz;

	sz = strlen( p_name) +1;
	if( log->p_name != NULL)
	{
		log->p_name = realloc( log->p_name, sz);
		if( log->p_name == NULL)
		{
			LogErr( "Log_Name( log=[%p], p_name=[%p]) realloc error. sz=[%d] err=[%d:%s]", 
					log, p_name, sz, errno, strerror( errno));
			return -errno;
		}
	}
	else
	{
		log->p_name = malloc( sz);
		if( log->p_name == NULL)
		{
			LogErr( "Log_Name( log=[%p], p_name=[%p]) malloc error. sz=[%d] err=[%d:%s]", 
					log, p_name, sz, errno, strerror( errno));
			return -errno;
		}
	}
	memcpy( log->p_name, p_name, sz);
	log->pid = getpid();

	return log->pid;
}

int Log_Func( LOG *log, void *data_ptr, int (* user)( void *data_ptr, char *rec, int sz))
{
	log->user_data = data_ptr;
	log->user = user;
	return 1;
}

int Log_Out( LOG *log, char *data, int sz)
{
	int			rtn;

	/* printf( "log->mem=[%p] mem_flag=[%d] data=[%.60s]\n", log->mem, mem_flag, data); */
	/*
	if( log == LsmPtr) printf( "log=[%p] LsmPtr=[%p] LsmPtr->mem=[%p] log->mem=[%p]\n", 
			log, LsmPtr, LsmPtr->mem, log->mem);
	*/

#if 0
	if( log->mem != NULL)
	{
		rtn = Lsm_Send( log->mem, data, sz);
		if( rtn < 0)
		{
			LsmCri( "Lsm_Send error. mem=[%p] rtn=[%d] sz=[%d]", log->mem, rtn, sz);
			log->mem = NULL;
		}
		return rtn;
	}
#endif

	log->last = data[ sz -1];
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
		LogErr( "log write error. fd=[%d] err=[%d:%s]", log->fd, errno, strerror( errno));
	}

	return rtn;
}

int Log_Append( LOG *log, char *data, int sz)
{
	int			rtn;

	/* printf( "log->mem=[%p] mem_flag=[%d] data=[%.60s]\n", log->mem, mem_flag, data); */
	/*
	if( log == LsmPtr) printf( "log=[%p] LsmPtr=[%p] LsmPtr->mem=[%p] log->mem=[%p]\n", 
			log, LsmPtr, LsmPtr->mem, log->mem);
	*/

	printf( "log->f_path=[%s]\n", log->f_path);
	printf( "log->f_name=[%s]\n", log->f_name);
	printf( "log->fd=[%d]\n", log->fd);


	log->last = data[ sz -1];
	if( log->f_name == NULL || log->fd < 2)
	{
		rtn = write( 1, data, sz);
		return rtn;
	}

	lseek( log->fd, 0L, SEEK_END);


	rtn = write( log->fd, data, sz);
	if( rtn < 0)
	{
		printf( "log write error. fd=[%d] err=[%d:%s]\n", log->fd, errno, strerror( errno));
	}

	return rtn;
}

int Log_SetDump( LOG *log, int opt)
{
	log->dump = opt;
	return log->dump;
}

int Log_Dump( const char *file, const char *func, int line, LOG *log, char *data, int sz, char *format, ...)
{
	va_list		args;
	char		rec[ 8192], buf[ 512];
	int			rec_sz = 0;
	int			rtn;
	char		*type[ 8] = { "NONE", "HEX", "DEC", "CHA", "DATA", ""};
	int			dump = log->dump;
	int			level = ( dump) ? LOG_LEV_MSG : LOG_LEV_TST;
	int			osz;


	if( dump >= 10) return 0;
	va_start( args, format);
	rec_sz += vsnprintf( rec, 8192, format, args);
	va_end( args);
	Log_Write( file, func, line, level, log, "%s", rec);

	if( dump == 0)
	{
		if( log->level <= LOG_LEV_DEV)		dump = LOG_DUMP_HEX;
		else if( log->level < LOG_LEV_TRC)	dump = LOG_DUMP_DEC;
		else if( log->level < LOG_LEV_TST)	dump = LOG_DUMP_CHA;
		else if( log->level < 9)			dump = LOG_DUMP_DATA;
		else if( log->level < LOG_LEV_DBG)	dump = LOG_DUMP_NONE;
		else								return 0;
	}

	osz = sprintf( buf, "DUMP %s ptr=[%p] sz=[%d]\n", type[ dump], data, sz);
	switch( dump)
	{
		case LOG_DUMP_HEX:
			rtn = Log_Out( log, buf, osz);
			rtn = Log_Hex( file, func, line, log, data, sz);
			return rtn;
		default:
		case LOG_DUMP_DEC:
			rtn = Log_Out( log, buf, osz);
			rtn = Log_Dec( file, func, line, log, data, sz);
			return rtn;
		case LOG_DUMP_CHA:
			rtn = Log_Out( log, buf, osz);
			rtn = Log_Cha( file, func, line, log, data, sz);
			return rtn;
		case LOG_DUMP_DATA:
			osz -= 1;
			osz += sprintf( &buf[ osz], "%s", " data=[");
			rtn = Log_Out( log, buf, osz);
			rtn = Log_Out( log, data, sz);
			osz = sprintf( buf, "%s", "]\n");
			rtn = Log_Out( log, buf, osz);
			return rtn;
		case LOG_DUMP_NONE:
			osz -= 1;
			osz += sprintf( &buf[ osz], " data=[%.50s]\n", data);
			rtn = Log_Out( log, buf, osz);
			break;
	}

	return rtn;
}

int Log_Hex( const char *file, const char *func, int line, LOG *log, char *data, int sz)
{
	unsigned int		pos = 0;
	unsigned char		rec[ 256], *ptr;

	ptr = ( unsigned char *)data;

	while( pos < sz)
	{
		memset( rec, 0x20, 256);
		sprintf( ( char *)rec, "%08X", pos);
		rec[ 8] = 0x20;

		while( pos < sz)
		{
			sprintf( ( char *)( &rec[ 10 + ( pos % 16) * 3]), "%02X ", ptr[ pos]);
			if( isprint( ptr[ pos]))	sprintf( ( char *)( &rec[ 60 + ( pos % 16)]), "%c ", ptr[ pos]);
			else						sprintf( ( char *)( &rec[ 60 + ( pos % 16)]), "%c ", '.');
			rec[ 79] = '\n';
			pos++;
			if( pos % 16 == 0) break;
		}
		Log_Out( log, ( char *)rec, 80);
	}

	return sz;
}

int Log_Cha( const char *file, const char *func, int line, LOG *log, char *data, int sz)
{
	int		pos = 0, out_sz;
	char	rec[ 256];

	out_sz = sprintf( rec, "         [0---------1---------2---------3---------4---------5---------6---------7---------8---------9---------]\n");
	Log_Out( log, rec, out_sz);

	while( pos < sz)
	{
		memset( rec, 0x20, 256);
		sprintf( rec, "%06d   [", pos);

		while( pos < sz)
		{
			rec[ pos % 100 + 10] = data[ pos];
			pos++;
			if( pos % 100 == 0) break;
		}
		if( pos % 100 == 0)		rec[ 100 +10] = ']';
		else 					rec[ pos % 100 +10] = ']';
		rec[ 111] = '\n';
		Log_Out( log, rec, 112);
	}
	return sz;
}

int Log_Dec( const char *file, const char *func, int line, LOG *log, char *data, int sz)
{
	int		pos = 0, lsz, col, han;
	char	rec[3][ 256];
	char	temp[ 16];
	unsigned char	c;

	lsz = sprintf( rec[ 0], "         [0----.----1----.----2----.----3----.----4----.----5----.----6----.----7----.----8----.----9----.----]\n");
	Log_Out( log, rec[ 0], lsz);
	while( pos < sz)
	{

		memset( rec[ 0], 0x20, 256);
		memset( rec[ 1], 0x20, 256);
		memset( rec[ 2], 0x20, 256);
		sprintf( rec[ 0], "%6d", pos);
		/* sprintf( rec[ 1], "0x%04x", pos); */
		rec[ 0][ 6] = 0x20;
		/* rec[ 1][ 6] = 0x20; */
		/*
		sprintf( &rec[ 0][ 114], "%06d", pos);
		sprintf( &rec[ 1][ 114], "0x%04x", pos);
		rec[ 0][ 120] = 0x20;
		rec[ 1][ 120] = 0x20;
		*/

		rec[ 0][ 100 + 11] = ' ';
		while( pos < sz)
		{
			if( pos == 0) 	sprintf( &rec[ 0][ pos % 100 + 10 -1], "%c", '[');

			c = ( unsigned char)data[ pos];
			han = IsHanGul( ( unsigned char *)data, pos);
			col = pos % 100 + 10;

			if( isgraph( c)) 		rec[ 0][ col] = c;
			else if( c == 0x20) 	rec[ 0][ col] = c;
			else if( han)		
			{
				if( col == 10)
				{
					if( han == 2)	rec[ 0][ col] = '<';
					else			rec[ 0][ col] = c;
				}
				else
				if( col == 109)
				{
					if( han == 1)			
					{
						rec[ 0][ col] = c;
						rec[ 0][ col +1] = ( unsigned char)data[ pos +1];
					}
					else
					{
						rec[ 0][ col] = c;
					}
				}
				else
				{
					rec[ 0][ col] = c;
				}
			}
			else 					rec[ 0][ col] = '.';

			sprintf( temp, "%02x", c);
			rec[ 1][ pos % 100 + 10] = temp[0];
			rec[ 2][ pos % 100 + 10] = temp[1];
			pos++;
			if( pos >= sz) 	rec[ 0][ pos % 100 + 10] = ']';
			if( pos % 100 == 0) break;
		}
		rec[ 0][ 100 + 12] = '\n';
		rec[ 1][ 100 + 11] = '\n';
		rec[ 2][ 100 + 11] = '\n';
		Log_Out( log, rec[ 0], 113);
		Log_Out( log, rec[ 1], 112);
		Log_Out( log, rec[ 2], 112);
		lsz = sprintf( rec[ 0], "         [0----^----1----^----2----^----3----^----4----^----5----^----6----^----7----^----8----^----9----^----]\n");
		Log_Out( log, rec[ 0], lsz);
	}
	/*
	lsz = sprintf( rec[ 0], "         [0---------1---------2---------3---------4---------5---------6---------7---------8---------9---------]\n");
	lsz = sprintf( rec[ 1], "         [0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789]\n");
	Log_Out( log, rec[ 0], lsz);
	*/
	return sz;
}

int NotLog_Dec( const char *file, const char *func, int line, LOG *log, char *data, int sz)
{
	int		pos = 0, lsz;
	char	rec[3][ 256];

	lsz = sprintf( rec[ 0], "         [0---------1---------2---------3---------4---------5---------6---------7---------8---------9---------]\n");
	Log_Out( log, rec[ 0], lsz);
	while( pos < sz)
	{
		memset( rec[ 0], 0x20, 256);
		memset( rec[ 1], 0x20, 256);
		memset( rec[ 2], 0x20, 256);
		sprintf( rec[ 0], "%06d", pos);
		sprintf( rec[ 1], "0x%04x", pos);
		rec[ 0][ 6] = 0x20;
		rec[ 1][ 6] = 0x20;
		/*
		sprintf( &rec[ 0][ 114], "%06d", pos);
		sprintf( &rec[ 1][ 114], "0x%04x", pos);
		rec[ 0][ 120] = 0x20;
		rec[ 1][ 120] = 0x20;
		*/

		while( pos < sz)
		{
			if( pos == 0) 	sprintf( &rec[ 0][ pos % 100 + 10 -1], "%c", '[');
			if( isgraph( data[ pos])) 		sprintf( &rec[ 0][ pos % 100 + 10], "%c", data[ pos]);
			else if( data[ pos] == 0x20) 	sprintf( &rec[ 0][ pos % 100 + 10], "%c", data[ pos]);
			else 							sprintf( &rec[ 0][ pos % 100 + 10], ".");
			sprintf( &rec[ 1][ pos % 100 + 10], "%x", ( data[ pos] & 0xF0) >> 4);
			sprintf( &rec[ 2][ pos % 100 + 10], "%x", data[ pos] & 0x0F);
			pos++;
			if( pos >= sz) 	sprintf( &rec[ 0][ pos % 100 + 10], "%c", ']');
			if( pos % 100 == 0) break;
		}
		sprintf( &rec[ 0][ 100 + 11], "\n");
		sprintf( &rec[ 1][ 100 + 11], "\n");
		sprintf( &rec[ 2][ 100 + 11], "\n");
		Log_Out( log, rec[ 0], 112);
		Log_Out( log, rec[ 1], 112);
		Log_Out( log, rec[ 2], 112);
	}
	lsz = sprintf( rec[ 0], "         [0---------1---------2---------3---------4---------5---------6---------7---------8---------9---------]\n");
	Log_Out( log, rec[ 0], lsz);
	return sz;
}

#if 0
int Log_MemTest( LOG *log, char *f_name)
{
	int			rtn;

	printf( "Log_MemTest start. log=[%p] f_name=[%s]\n", log, f_name);
	if( log == NULL) log = Log_Open( f_name);
	if( log->mem == NULL)
	{
		if( LsmPtr != NULL) Log_Close( LsmPtr);
		LsmPtr = Log_Open( f_name);
		Log_File( LsmPtr, f_name);
		LsmPtr->mem = NULL;
#if LOG_DBG
	printf( "f_name=[%s]\n", f_name);
#endif

		log->mem = Lsm_Open( f_name);
		if( log->mem == NULL)
		{
			LogCri( "Lsm_Open error. f_name=[%s]", f_name);
			LogMsg( "direct open file. f_name=[%s]", f_name);
			rtn = Log_File( log, f_name);
			return rtn;
		}
		LogDbg( "LogPtr f_name=[%s]", log->f_name);

		log->fd_type = 0;
		log->head &= ~LOG_HEAD_COLOR;
		log->head &= ~LOG_HEAD_BELL;
	}
	return 0;
}
#endif

int Log_Mem( LOG *log, char *f_name)
{
	int			rtn;

	return Log_File( log, f_name);

	if( log == NULL) log = Log_Open( f_name);
	if( log->mem == NULL)
	{
		if( LsmPtr != NULL) Log_Close( LsmPtr);
		LsmPtr = Log_Open( f_name);
		Log_File( LsmPtr, f_name);
		LsmPtr->mem = NULL;
#if LOG_DBG
	printf( "f_name=[%s]\n", f_name);
#endif

		log->mem = Lsm_Open( f_name);
		if( log->mem == NULL)
		{
			LogCri( "Lsm_Open error. f_name=[%s]", f_name);
			LogMsg( "direct open file. f_name=[%s]", f_name);
			rtn = Log_File( log, f_name);
			return rtn;
		}
		LogDbg( "LogPtr f_name=[%s]", log->f_name);

		log->fd_type = 0;
		log->head &= ~LOG_HEAD_COLOR;
		log->head &= ~LOG_HEAD_BELL;
	}
	return 0;
}

int Log_File( LOG *log, char *f_name)
{
	int			rtn;
	char		rec[ 65535];
	char		name[ 512];
	int			out_sz, sz;

#if LOG_DBG
	printf( "LogPtr=[%p] log=[%p]\n", LogPtr, log);
#endif
	if( log == NULL)
	{
		log = Log_Open( f_name);
		if( log == NULL)
		{
			LogCri( "Log_Open error. err=[%d:%s]", errno, strerror( errno));
			return -1;
		}
		if( LogPtr == NULL) LogPtr = log;
		log->fd_type = 1;
#if LOG_DBG
		printf( "create log=[%p]\n", log);
#endif
	}
#if LOG_DBG
	printf( "LOGDBG Log_File log=[%p] f_name=[%s] log->fd=[%d] log->fd_type=[%d]\n", log, f_name, log->fd, log->fd_type);
#endif

	if( log->f_name != NULL) 	free( log->f_name);
	if( log->f_path != NULL)	free( log->f_path);
	log->f_name = NULL;
	log->f_path = NULL;

	if( f_name == NULL)
	{
		if( log->fd > 0 && log->fd != STDOUT_FILENO)
		{
			close( log->fd);
		}

		log->fd = STDOUT_FILENO;
		log->fd_type = 0;
		log->sz = 0;
		log->fd_time = 0;
		log->head |= LOG_HEAD_COLOR;
		log->head |= LOG_HEAD_BELL;
		return log->fd;
	}

	sz = sprintf( name, "%s/%s", log->f_path, log->f_name);
	if( !memcmp( name, f_name, sz)) 
	{
		if( log->fd >= 3) return log->fd;
	}

	if( log->fd_type == 1 && log->fd >= 3) 
	{
		out_sz = sprintf( rec, "LOG -- log change new file. old=[%s/%s] new=[%s]\n", log->f_path, log->f_name, f_name);
		write( log->fd, rec, out_sz);
		close( log->fd);
		log->fd = -1;
	}
		
	rtn = Log_FilePath( log, f_name);
	if( rtn < 0)
	{
		return rtn;
	}
	Log_FileOpen( log);
#if LOG_DBG
		printf( "return log->fd=[%d]\n", log->fd);
#endif

	return log->fd;
}

int Log_FileCheck( LOG *log)
{
	int			rtn;
	time_t		cur_time;
	size_t		seek_pos;
	char		rec[ 65535];
	int			sz;

	if( log->fd < 0)		Log_FileOpen( log);

	/* not regular_file */
	if( log->fd_type != 1)	return 0;		

	seek_pos = lseek( log->fd, 0L, SEEK_END);
	if( seek_pos < 0LL)
	{
		printf( "LOG -- lseek error. fd=[%d] err=[%d%s]\n", log->fd, errno, strerror( errno));
		return 0;
	}
	/* printf( "LOGDBG seek_pos=[%lld]\n", seek_pos); */

	switch( log->type)
	{
		case LOG_TYPE_NONE:
			break;

		case LOG_TYPE_SIZE :
			if( log->sz <= 0) return 0;
			if( seek_pos >= log->sz)
			{
#if LOG_DBG
				printf( "seek_pos=[%d] log->sz=[%d]\n", seek_pos, log->sz);
#endif
				sz = sprintf( rec, "LOG -- log file end. fd=[%d] sz=[%ld] log->sz=[%ld]\n", log->fd, seek_pos, log->sz);
				write( log->fd, rec, sz);
				close( log->fd);
				log->fd = -1;
				Log_FileBack( log);
				Log_FileOpen( log);
			}
			break;

		case LOG_TYPE_DAILY:	/* daily */
			if( log->fd_time == 0) return 0;
			time( &cur_time);
			if( cur_time < log->fd_time) break;

			if( log->fd_type == 1)
			{
				sz = sprintf( rec, "LOG -- log file end. fd=[%d] at=[%s] log->fd_time=[%s]\n", 
						log->fd, TtoS( cur_time), TtoS( log->fd_time));
				write( log->fd, rec, sz);
				close( log->fd);
				log->fd = -1;
				/* Log_FileBack( log); */
				rtn = Log_FileOpen( log);
				if( rtn < 0)
				{
					return rtn;
				}
			}
			break;

		default:
			break;
			
	}

	return 0;
}

int Log_FileBack( LOG *log)
{
	char		b_name[ 1024];
	char		b_path[ 512];
	char		f_name[ 1024];
	time_t		cur_time;
	struct tm	*tp;

	if( log->fd >= 0) return 0;

	time( &cur_time);

	if( log->type == LOG_TYPE_SIZE)
	{
		sprintf( f_name, "%s/%s", log->f_path, log->f_name);
		sprintf( b_name, "%s/%s.bak", log->f_path, log->f_name);
		rename( f_name, b_name);
		return 1;
	}

	sprintf( f_name, "%s/%s", log->f_path, log->f_name);
	if( log->b_path != NULL)
	{
		memcpy( b_path, log->b_path, strlen( log->b_path) +1);
		TtoA( b_path, cur_time);
		mkdir( b_path, LOG_DIR_MODE);
		sprintf( b_name, "%s/%s", b_path, log->f_name);
	}
	else
	{
		tp = localtime( &cur_time);
		sprintf( b_name, "%s/%s.%04d%02d%02d%02d", 
				log->f_path, log->f_name, tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday, tp->tm_hour);
	}
	rename( f_name, b_name);

	return 1;
}

int Log_FileOpen( LOG *log)
{
	time_t		cur_time;
	struct tm	tb;
	char		f_name[ 1024], d_name[ 512];
	char		rec[ 65535];
	int			sz = 0;

	/*
	printf( "LOGDBG Log_FileOpen - log=[%p] log->fd=[%d] log->fd_type=[%d] log->f_path=[%s] log->f_name=[%s]\n", 
			log, log->fd, log->fd_type, log->f_path, log->f_name);
	*/

	if( log->fd >= 0 && log->fd_type == 1)	return 0;
	if( log->f_name == NULL)				return 0;

	time( &cur_time);

	memcpy( d_name, log->f_name, strlen( log->f_name) +1);
	TtoA( d_name, cur_time);

	sprintf( f_name, "%s/%s", log->f_path, d_name);

	log->fd = open( f_name, O_APPEND | O_CREAT | O_RDWR, LOG_FILE_MODE);
	if( log->fd < 0)
	{
		printf( "log file open error. name=[%s] err=[%d:%s]\n", f_name, errno, strerror( errno));
		log->fd = -1;
		log->fd_time = 0;
		return 0;
	}


	cur_time += 86400;
	localtime_r( &cur_time, &tb);
	tb.tm_hour = 0;
	tb.tm_min = 0;
	tb.tm_sec = 0;
	log->fd_time = mktime( &tb);
	fstat( log->fd, &log->sb);

	if( log->sb.st_size == 0)
	{
		sz += sprintf( &rec[ sz], "LOG -- file create.      at=[%s]\n", TtoS( cur_time - 86400));
		sz += sprintf( &rec[ sz], "LOG -- inode number.     no=[%ld]\n", log->sb.st_ino);
		sz += sprintf( &rec[ sz], "LOG -- backup path.      at=[%s]\n", log->b_path);
		sz += sprintf( &rec[ sz], "LOG -- file size limit.  sz=[%ld]\n", log->sz);
		sz += sprintf( &rec[ sz], "LOG -- file time limit.  at=[%s]\n", TtoS( log->fd_time));
		write( log->fd, rec, sz);
	}

	if( S_ISREG( log->sb.st_mode))	
	{
		log->fd_type = 1;
		log->head &= ~LOG_HEAD_COLOR;
		log->head &= ~LOG_HEAD_BELL;
	}
	else							
	{
		log->fd_type = 0;
		log->head |= ~LOG_HEAD_COLOR;
		log->head |= ~LOG_HEAD_BELL;
	}
	/*
	printf( "LOGDBG Log_FileOpen - stat f_name=[%s] log->fd_type=[%d]", f_name, log->fd_type);
	*/

	return log->fd;
}

int Log_FilePath( LOG *log, char *f_name)
{
	char		*p, name[ 512];
	int			sz;

	/* printf( "LOGDBG log=[%p] f_name=[%s]\n", log, f_name); */
	memcpy( name, f_name, strlen( f_name) +1);
	p = strrchr( name, '/');
	if( p != NULL)
	{
		*p = 0;
		p++;
		sz = strlen( p);
		log->f_name = ( char *)malloc( sz +1);
		memcpy( log->f_name, p, sz +1);
		sz = strlen( name);
		log->f_path = ( char *)malloc( sz +1);
		memcpy( log->f_path, name, sz +1);
	}
	else
	{
		sz = strlen( name);
		log->f_name = ( char *)malloc( sz +1);
		memcpy( log->f_name, name, sz +1);
		sz = 7;
		log->f_path = ( char *)malloc( sz +1);
		memcpy( log->f_path, "./", 3);
	}
	if( log->b_path == NULL)
	{
		sz = strlen( log->f_path) +1;
		sz += 32;
		log->b_path = malloc( sz);
		sprintf( log->b_path, "%s/YYYYMMDD", log->f_path);
	}
	/* printf( "LOGDBG log=[%p] log->f_path=[%s] log->f_name=[%s]\n", log, log->f_path, log->f_name); */

	return 1;
}


