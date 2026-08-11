/** ***************************************************************************
**  @file		log.h
**  @date		2022/08/23
**  @author		최동춘
**  @version	V2.0.20220823
**  @brif		
**	로그 파일 관리 라이브러리
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>

#include "mem.h"
#include "sem.h"

#ifndef LOG_H
#define	LOG_H 1

#ifdef		ERRMSG
#define		ERRMSG_FUNC							/* gloval error log - error_msg function */
#endif
#define		LOG_DBG				0				/* for trace log library */

#define		LOG_BUFFER_SIZE		65000
#define		LOG_HEAD_SIZE		55				/* color +9 */
#define		LOG_HEAD_0			0				/* HH:MM:SS-SSSSSS L file:func(line)                      - 55 byte, L=level */
#define		LOG_HEAD_COLOR		0x00000100		/* log level clolr 표현 */
#define		LOG_HEAD_BELL		0x00000200		/* LOG_LEV_BELL 소리 --  */
#define		LOG_TYPE_NONE		0				/* 새파일을 만들지 않고 계속 사용 */
#define		LOG_TYPE_DAILY		1				/* file_name.YYYYMMDD */
#define		LOG_TYPE_SIZE		4				/* log->sz 까지 사용후 새파일 open log->sz=0 이면 계속 사용 */
#define		LOG_FILE_MODE		0644			/* create mode  -rw-r--r-- */
#define		LOG_DIR_MODE		0755			/* create mode  -rwxr-xr-x */

#define		LOG_TAIL_STR		"\n\0"

#define		LOG_DUMP_HEX		1				/* LogDump = HexDump(LogHex) */
#define		LOG_DUMP_DEC		2				/* LogDump = DecimalDump(LogDec) */
#define		LOG_DUMP_CHA		3				/* LogDump = LogCha - addr [DATA] */
#define		LOG_DUMP_DATA		4				/* LogDump = [sz:data] */
#define		LOG_DUMP_NONE		5				/* LogDump를 사용하지 않음 */
#define		LOG_DUMP_DEL		10				/* LogDump를 사용하지 않음 */

// 로그레벨보다 크거나 같으면 적용됨.  ( MSG 30 -> 30~ 101까지 적용, 0~20까지 미적용), 낮을수록 DEB 레벨임.
#define		LOG_LEV_DEV		0  // 0 to all  ... for DEV 가장 낮은 레벨의 logging
#define		LOG_LEV_DMP		1  // 1 to all  ... for test
#define		LOG_LEV_DBG		5  // 5 to all  ... for debug 레벨의 logging
#define		LOG_LEV_TRC		7  // 7 to all  ... for test
#define		LOG_LEV_TST		10 // 10 to all ...		no all dump message
#define		LOG_LEV_RAW		10  // 5 to all  ... for debug 레벨의 logging

#define		LOG_LEV_APP		20 // 20 to all ... for UAT
#define		LOG_LEV_MSG		30 // 30 to all ... for PRD
#define		LOG_LEV_LIB		40
#define		LOG_LEV_WAR		50
#define		LOG_LEV_CRI		70
#define		LOG_LEV_ERR		75
#define		LOG_LEV_BELL	80
#define		LOG_LEV_USR		90
#define		LOG_LEV_ERRMSG	100					/* for kmb errmsg log */
#define		LOG_LEV_CRIMSG	101					/* for kmb errmsg log */



/** LSM ********************************************************************/
/** LSM ********************************************************************/
/** LSM ********************************************************************/
#define		LOG_MAX_REC				100					/* 최대 Log 저장 건수 */
#define		LOG_REC_SZ				1024				/* 최대 record size */
#define		LOG_IPC_KEY				0xfa004001			/* IPC 접근 key */
#define		LOG_PIPE_NAME			"/app/fxwin/mat/dat/log.fifo"	/* 20241224 안씀 $MAT_DAT/log.fifo로 대체 named pipe name */
#define		LOG_FILE_PATH			"/log/fxwin/mat"	/* 안씀... $MAT_LOG로 대체 log file path - log daemon이 떨굴 파일의 위치 */
#define		LOG_TIMEOUT				-9999
#define		LOG_NODATA				-9998				/* Log_GetPos return */
#define		LOG_MAX_IDX				256					/* max file name */

#ifndef MAX
#define		MAX(x,y)				((x>y)?x:y)
#define		MIN(x,y)				((x<y)?x:y)
#endif

typedef struct _log_index_
{
	int		pos;						/* position - -1:empty pos:used */
	char	name[ 64];
	int		cnt;						/* current data cnt */
	int		sum;						/* total data cnt */
	time_t	w_time;						/* data send time */
	char	comment[ 64];				/* log comment */
}	LOG_INDEX;

typedef struct _log_mem_head_
{
	int				gubun;				/* 0-빈 record, 1-data record */
	int				index_pos;			/* index no */
	char			name[ 64];			/* log file name */
	char			file[ 64];			/* file name */
	char			func[ 64];			/* function name */
	int				line;				/* line number */
	int				level;				/* log level */
	int				sz;					/* data record size */
	struct timeval	w_time;				/* write 시간  */
	struct timeval	r_time;				/* read 시간 */
	int				prev;				/* 이전 record 위치, 시작 record = -1 */
	int				next;				/* 다음 record 위치, 끝 record = -1 */
}	LSM_HEAD;

typedef struct _log_data_area_
{
	int				pos;				/* shared memory position */
	LSM_HEAD		head;				/* logching head */
	char			rec[ LOG_REC_SZ];	/* data record */
	char			filler[ 100];		/* filler - 512 byte 맞추기 */
}	LOG_RECORD;

typedef struct _log_status_
{
	int			rec_cnt;			/* 공유메모리에 저장된 data 건수 */
	int			wpos;				/* write position - 빈 record 찾기 시작 위치 */
	time_t		ctime;				/* shared memory create time */
	time_t		rtime;				/* daily run time */
	time_t		btime;				/* next daily run time */
	int			dtime;				/* daily clear time - HHMMSS */
	pid_t		d_pid;				/* daemon pid */
	key_t		key;				/* semaphore, shared_memory 접근 key */
	char		log_path[ 512];		/* log file path */
	char		pipe_name[ 512];	/* named pipe full name */
}	LOG_STATUS;

typedef struct _log_shm_map_
{
	LOG_RECORD	rec[ LOG_MAX_REC];	/* 주문 record */
	LOG_STATUS	stat;				/* 관리 struct */
	int			start;				/* dll - log start position */
	int			end;				/* dll - log end position */
	int			cnt;				/* record count */
	int			dcnt;				/* data count */
	int			spid;
	int			rpid;
	time_t		stime;
	time_t		rtime;
	LOG_INDEX	index[ LOG_MAX_IDX];
}	LOG_MAP;

typedef struct _log_mem_
{
	int			idx;				/* index position */
	MEM			*mem;				/* shared memory struct */
	SEM			*sem;				/* semaphore struct */
	int			fd;					/* fifo file desc */
	LOG_MAP		*map;				/* shared memory base pointer */
	int			pos;				/* empty record position - set Log_SetRecord/Log_GetRecord */
	void		*ptr;				/* user record pointer */
	int			sz;					/* user record size */
	int			flag;				/* 0-commit, 1-write, 2-read */
	char		name[ 64];			/* log file name */
	int			index_pos;			/* file index pos */
	int			name_sz;			/* file_name size */
}	LSM;
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/

/** ***************************************************************************
**  @struct		LOG
**  @brief		로그 파일 관리 구조체
***************************************************************************** */
typedef struct _log_
{
	int			dump;					/** @var int dump	dump type 1:hex,2:dec,3:cha			*/
	int			head;					/** @var int head	head type */
	int			level;					/** @var int level	log level */
	char		*p_name;				/** @var process name */
	pid_t		pid;					/** *var process id */
	int			fd;						/** @var log file desc */
	int			fd_type;				/** @var console=0 regular_file=1 */
	time_t		fd_time;				/** @var daily log time - 유효 연한 at open */
	int			type;
	int			last;					/** @var last charector - 로그 마지막 문자:개행문자 확인용 */
	size_t		sz;						/** @var log file size - LOG_TYPE_SIZE */
	char		*f_path;				/** @var log file path */
	char		*b_path;				/** @var log backup path */
	char		*f_name;				/** @var log file name */
	char		*buf;					/** @var log buffer */
	char		*tail;					/** @var log dilimiter */
	int			tail_sz;				/** @var log tail size */
	struct stat	sb;						/** @var log stat buffer - at open */
	char		*file;					/** @var log call file name */
	char		*func;					/** @var log call function name */
	int			line;					/** @var log call line number */
	void		*user_data;				/** @user_data call user function data pointer */
	int			(*user)( void *user_data, char *msg, int sz);		/* @var LOG_LEV_USR call function */
	LSM			*mem;
}	LOG;

extern LOG	*LogPtr;
extern LOG	*LsmPtr;

#ifndef LOG_PROC
#define 	LogHex( data, sz)			Log_Hex( __FILE__, __FUNCTION__, __LINE__, LogPtr, data, sz)
#define 	LogDec( data, sz)			Log_Dec( __FILE__, __FUNCTION__, __LINE__, LogPtr, data, sz)
#define 	LogCha( data, sz)			Log_Cha( __FILE__, __FUNCTION__, __LINE__, LogPtr, data, sz)
#define 	LogDump( data, sz, format, args...)	\
				Log_Dump( __FILE__, __FUNCTION__, __LINE__, LogPtr, data, sz, ( char *)format, ##args)  // 에러레벨없이 무조껀 찍힘
#define 	LogDddd( data, sz, format, args...)
#define 	LogDdel( data, sz, format, args...)
#define 	LogSetDump( opt)			Log_SetDump( LogPtr, opt)

#define 	Log_Usr( log_ptr, format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, 9, log_ptr, ( char *)format, ##args)
#define 	LogUsr( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_USR, LogPtr, ( char *)format, ##args)
#define 	LogBel( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_BELL, LogPtr, ( char *)format, ##args)
#define 	LogCri( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_CRI, LogPtr, ( char *)format, ##args)
#define 	LogCRI( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_CRIMSG, LogPtr, ( char *)format, ##args) /* KMB 대문자는 에러서버 전송 (에러 메세지만 첨부) - 어플리케이션 에러시 */
#define 	LogErr( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_ERR, LogPtr, ( char *)format, ##args)
#define 	LogERR( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_ERRMSG, LogPtr, ( char *)format, ##args)  /* KMB 에러메시지, 에러넘버 첨부 - 시스템콜 에러시 (read, bind ..) */
#define 	LogWar( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_WAR, LogPtr, ( char *)format, ##args)
#define 	LogLib( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_LIB, LogPtr, ( char *)format, ##args)
#define 	LogMsg( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_MSG, LogPtr, ( char *)format, ##args) /* for PRD */
#define 	LogApp( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_APP, LogPtr, ( char *)format, ##args) /* for UAT */
#define 	LogDbg( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_DBG, LogPtr, ( char *)format, ##args) /* for DEV */
#define 	LogDev( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__,  LOG_LEV_DEV, LogPtr, ( char *)format, ##args)
#define 	LogDmp( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__,  LOG_LEV_DMP, LogPtr, ( char *)format, ##args)
#define 	LogDbg( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__,  LOG_LEV_DBG, LogPtr, ( char *)format, ##args)
#define 	LogTrc( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__,  LOG_LEV_TRC, LogPtr, ( char *)format, ##args)
#define 	LogTst( format, args...)	Log_Write( __FILE__, __FUNCTION__, __LINE__,  LOG_LEV_TST, LogPtr, ( char *)format, ##args)
#define 	LogDel( format, args...)	/* Log_Write( __FILE__, __FUNCTION__, __LINE__, 0, LogPtr, ( char *)format, ##args) */
#define 	LogDdd( format, args...)	/* Log_Write( __FILE__, __FUNCTION__, __LINE__, 0, LogPtr, ( char *)format, ##args) */
#define 	LogRem( format, args...)	/* Log_Write( __FILE__, __FUNCTION__, __LINE__, 0, LogPtr, ( char *)format, ##args) */
#define 	LogNoh( format, args...)	Log_NoHeadWrite( __FILE__, __FUNCTION__, __LINE__, 0, LogPtr, ( char *)format, ##args)
#define 	LogRaw( format, args...)	Log_RawWrite( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_RAW, LogPtr, ( char *)format, ##args)
#define 	LogRa2( format, args...)	Log_RawWrite( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_APP, LogPtr, ( char *)format, ##args)
#define 	LogRAW( format, args...)	Log_RawWrite( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_MSG, LogPtr, ( char *)format, ##args)
#define 	LogDat( format, args...)	Log_RawWrite( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_MSG, LogPtr, ( char *)format, ##args)
#define 	LogMEM( format, args...)	Log_WriteMem( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_MSG, LogPtr, ( char *)format, ##args)
#define		LogOpen( f_name)			LogPtr = Log_Open( f_name);
#define		LogFile( f_name)			Log_File( LogPtr, f_name);
#define		LogMem( f_name)				Log_Mem( LogPtr, f_name);
#define		LogClose()					Log_Close( LogPtr);
#define		LogFunc( data_ptr, func)	Log_Func( LogPtr, data_ptr, func);
#define		LogType( type)				Log_Type( LogPtr, type);
#define		LogSize( size)				Log_Size( LogPtr, size);
#define		LogLevel( level)			Log_Level( LogPtr, level);
#define		LogGetFd()					Log_GetFd( LogPtr);
#define		LogName( p_name)			Log_Name( LogPtr, p_name);
#define		LogDbgFlag					1
#define		LogDelFlag					0


#define LsmCri( format, args...) Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_CRI, LsmPtr, ( char *)format, ##args)
#define LsmErr( format, args...) Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_ERR, LsmPtr, ( char *)format, ##args)
#define LsmWar( format, args...) Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_WAR, LsmPtr, ( char *)format, ##args)
#define LsmLib( format, args...) Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_LIB, LsmPtr, ( char *)format, ##args)
#define LsmMsg( format, args...) Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_MSG, LsmPtr, ( char *)format, ##args)
#define LsmApp( format, args...) Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_APP, LsmPtr, ( char *)format, ##args)
#define LsmDbg( format, args...) Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_DBG, LsmPtr, ( char *)format, ##args)
#define LsmTst( format, args...) Log_Write( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_TST, LsmPtr, ( char *)format, ##args)
#define LsmRaw( format, args...) Log_RawWrite( __FILE__, __FUNCTION__, __LINE__, LOG_LEV_TST, LsmPtr, ( char *)format, ##args)
#define LsmDel( format, args...) ;
#endif	/* LOG_PROC */

#endif


/***** Module : log.c *****/
LOG*        Log_Open( char *f_name);                                        /* 로그 파일을 사용하기위한 초기화 작업 */
int         Log_Close( LOG *log);
int         Log_Type( LOG *log, int type);
int         Log_Level( LOG *log, int level);
size_t      Log_Size( LOG *log, size_t sz);
int         Log_Write( const char *file, const char *func, int line, int level, LOG *log, char *format, ...);
int         Log_WriteMem( const char *file, const char *func, int line, int level, LOG *log, char *format, ...);
int         Log_NoHeadWrite( const char *file, const char *func, int line, int level, LOG *log, char *format, ...);
int         Log_RawWrite( const char *file, const char *func, int line, int level, LOG *log, char *format, ...);
int         Log_Head( LOG *log, const char *file, const char *func, int line, int level);
int         Log_Tail( LOG *log, char *tail, int sz);
int         Log_GetFd( LOG *log);
int         Log_Name( LOG *log, char *p_name);
int         Log_Func( LOG *log, void *data_ptr, int (* user)( void *data_ptr, char *rec, int sz));
int         Log_Out( LOG *log, char *data, int sz);
int         Log_Append( LOG *log, char *data, int sz);
int         Log_SetDump( LOG *log, int opt);
int         Log_Dump( const char *file, const char *func, int line, LOG *log, char *data, int sz, char *format, ...);
int         Log_Hex( const char *file, const char *func, int line, LOG *log, char *data, int sz);
int         Log_Cha( const char *file, const char *func, int line, LOG *log, char *data, int sz);
int         Log_Dec( const char *file, const char *func, int line, LOG *log, char *data, int sz);
int         NotLog_Dec( const char *file, const char *func, int line, LOG *log, char *data, int sz);
int         Log_MemTest( LOG *log, char *f_name);
int         Log_Mem( LOG *log, char *f_name);
int         Log_File( LOG *log, char *f_name);
int         Log_FileCheck( LOG *log);
int         Log_FileBack( LOG *log);
int         Log_FileOpen( LOG *log);
int         Log_FilePath( LOG *log, char *f_name);

/***** Module : log_mem.c *****/
LSM*    Lsm_CreateForce();                                           /* 매칭엔진에 필요한 ipc를 생성 */
LSM*    Lsm_Create();                                                /* 매칭엔진에 필요한 ipc를 생성 */
int         Lsm_Remove( LSM *mem);                                   /* 매칭엔진에서 생성한 ipc를 삭제 */
int         Lsm_RemoveForce( char *cfg_name);                            /* 매칭엔진에서 생성한 ipc를 삭제 */
LSM*    Lsm_Open( char *name);                                       /* 매칭엔진 Open */
int         Lsm_Close( LSM *mem);                                    /* 매칭엔진 Close */
int         Lsm_Init( LSM *mem);                                     /* index initial */
int         Lsm_LoadConfig( LSM *mem);                               /* find index pos by name */
int         Lsm_Write( const char *file, const char *func, int line, int level, LOG *log, char *format, ...);/* find index pos by name */
int         Lsm_FindIdx( LSM *mem, char *name);                      /* find index pos by name */
int         Lsm_WritePipe( LSM*mem);                                 /* pipe로 부터 체결 record position을 수신 */
int         Lsm_ReadPipe( LSM*mem, int timeout);                     /* pipe로 부터 체결 record position을 수신 */
int         Lsm_Send( LSM *mem, char *rec, int sz);                  /* 주문 insert  */
int         Lsm_Recv( LSM*mem, char *rec, int sz, int timeout);      /* queue read */
int         Lsm_GetPos( LSM*mem, int timeout);                       /* queue read */
int         Lsm_Insert( LSM *mem);                                   /* Lsm_SetRecord 이후 Lock 해제 및 insert */
int         Lsm_InsertRecord( LSM *mem);                             /* Lsm_SetRecord 이후 Lock 해제 및 insert */
int         Lsm_GetEmptyRecordPos( LSM *mem, int timeout);           /* 빈 record 찾기 */
LOG_RECORD* Lsm_GetRecordByPos( LSM*mem, int pos);                   /* position을 입력하여 record 찾기 */
int         Lsm_Lock( LSM *mem, int timeout);                        /* semaphore lock 수행 */
int         Lsm_Unlock( LSM *mem);                                   /* semaphore lock 해제 */
int         Lsm_Stat( LSM*mem);                                      /* stat 출력 */

/***** Module : lsm.c *****/
LSM*        Lsm_CreateForce();                                              /* 매칭엔진에 필요한 ipc를 생성 */
LSM*        Lsm_Create();                                                   /* 매칭엔진에 필요한 ipc를 생성 */
int         Lsm_Remove( LSM *mem);                                          /* 매칭엔진에서 생성한 ipc를 삭제 */
int         Lsm_RemoveForce( char *cfg_name);                               /* 매칭엔진에서 생성한 ipc를 삭제 */
LSM*        Lsm_Open( char *name);                                          /* 매칭엔진 Open */
int         Lsm_Close( LSM *mem);                                           /* 매칭엔진 Close */
int         Lsm_Init( LSM *mem);                                            /* index initial */
int         Lsm_LoadConfig( LSM *mem);                                      /* find index pos by name */
int         Lsm_Write( const char *file, const char *func, int line, int level, LOG *log, char *format, ...);/* find index pos by name */
int         Lsm_FindIdx( LSM *mem, char *name);                             /* find index pos by name */
int         Lsm_WritePipe( LSM*mem);                                        /* pipe로 부터 체결 record position을 수신 */
int         Lsm_ReadPipe( LSM*mem, int timeout);                            /* pipe로 부터 체결 record position을 수신 */
int         Lsm_Send( LSM *mem, char *rec, int sz);                         /* 주문 insert  */
int         Lsm_Recv( LSM*mem, char *rec, int sz, int timeout);             /* queue read */
int         Lsm_GetPos( LSM*mem, int timeout);                              /* queue read */
int         Lsm_Insert( LSM *mem);                                          /* Lsm_SetRecord 이후 Lock 해제 및 insert */
int         Lsm_InsertRecord( LSM *mem);                                    /* Lsm_SetRecord 이후 Lock 해제 및 insert */
int         Lsm_GetEmptyRecordPos( LSM *mem, int timeout);                  /* 빈 record 찾기 */
LOG_RECORD* Lsm_GetRecordByPos( LSM*mem, int pos);                          /* position을 입력하여 record 찾기 */
int         Lsm_Head( LSM *lsm, char *buf, LOG_INDEX *index, LSM_HEAD *head);/* position을 입력하여 record 찾기 */
int         Lsm_Lock( LSM *mem, int timeout);                               /* semaphore lock 수행 */
int         Lsm_Unlock( LSM *mem);                                          /* semaphore lock 해제 */
int         Lsm_Stat( LSM*mem);                                             /* stat 출력 */

