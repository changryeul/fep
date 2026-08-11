/** ***************************************************************************
**  @file		bat.h
**  @date		2022/08/25
**  @author		최동춘
**  @version	V1.0.20220825
**  @brif		
**	배치 테이블 관리
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

#include "log.h"
#include "mem.h"

#ifndef BAT_H
#define	BAT_H 1

#define BAT_MEM_KEY	0xfe010300
#define	BAT_MAX_TBL	32
#define	BAT_TBL_SZ	( sizeof( BAT_TBL))
#define	BAT_MEM_SZ	(( sizeof( BAT_TBL) * BAT_MAX_TBL) + sizeof( BAT_STAT))

#define	BAT_ERR_NONE	0
#define	BAT_ERR_NODATA	302
#define	BAT_ERR_FILE	303
#define	BAT_ERR_DB		304
#define	BAT_ERR_COMM	305

#define	BAT_STAT_CLEAR			0
#define	BAT_STAT_MAKE_START		1
#define	BAT_STAT_MAKE_END		2
#define	BAT_STAT_SEND_START		3
#define	BAT_STAT_SEND_END		4
#define	BAT_STAT_RECV_START		5
#define	BAT_STAT_RECV_END		6

#define	BAT_D2FS		11			/** @var  db to file start */
#define	BAT_D2FE		12			/** @var  db to file end */
#define	BAT_S2BS		13			/** @var  send to bok start */
#define	BAT_S2BE		14			/** @var  send to bok end */
#define	BAT_R2FS		21			/** @var  receive to file start */
#define	BAT_R2FE		22			/** @var  receive to file end */
#define	BAT_L2DS		23			/** @var  load to db start */
#define	BAT_L2DE		24			/** @var  load to db end */

/** ***************************************************************************
**  @struct		BAT_TBL
**  @brief		배치 테이블
***************************************************************************** */
typedef struct _bat_stat_
{
	time_t		create;					/* memory create time */
	int			daily;					/* daily job time */
	time_t		daily_at;				/* daily running time */
	char		path[ 512];				/* batch file path */
	char		swp_path[ 512];			/* shared memory swap file path */
	char		cfg_path[ 512];			/* config file path */
	char		version[ 64];			/* vsrsion string */
	int			cnt;					/* batch table count */
}	BAT_STAT;

/** ***************************************************************************
**  @struct		BAT_TBL
**  @brief		배치 테이블
***************************************************************************** */
typedef struct _bat_job_tbl_
{
	char	name[ 32];				/** @var batch file name */
	int		stat;					/** @var file create stat 0-none 1-start 2-end */
	time_t	st_time;				/** @var stat time */
	int		cnt;					/** @var record count */
	int		err;					/** @var file error */
	char	msg[ 128];				/** @var batch err message */
}	BAT_JOB_TBL;

typedef struct _bat_tbl_
{
	int			no;						/** @var job number */
	int			job;					/** @var 0=done 1=NEW 2=UPD 3=DEL 9=NON */
	char		id[ 32];				/** @var batch table id */
	char		name[ 32];				/** @var batch file name - YYYYMMDDHHMM */
	char		file[ 32];				/** @var last process batch file name */
	char		rcv_file[ 32];			/** @var last recv batch file name */
	int			rec_sz;					/** @var record size */
	int			stat;					/** @var batch job stat 0:미완료 1:생성시작 2:생성완료 3:읽기시작 4:읽기완료 */

#if 0
	BAT_JOB_TBL	file;
	BAT_JOB_TBL	send;
	BAT_JOB_TBL	recv;
#else
	char	nm_file[ 32];			/** @var batch file name */
	int		st_file;				/** @var file create stat 0-none 1-create */
	time_t	tm_file;				/** @var file create time */
	int		cnt_file;				/** @var record count */
	int		err_file;				/** @var file create error */
	char	msg_file[ 128];			/** @var batch err message */

	char	nm_send[ 32];			/** @var batch file name */
	int		st_send;				/** @var file send stat 0-none 1-send */
	time_t	tm_send;				/** @var file send time */
	int		cnt_send;				/** @var record count */
	int		err_send;				/** @var file send error */
	char	msg_send[ 128];			/** @var batch err message */

	char	nm_recv[ 32];			/** @var batch file name */
	int		st_recv;				/** @var file recv stat 0-none 1-recv */
	time_t	tm_recv;				/** @var file recv time */
	int		cnt_recv;				/** @var record count */
	int		err_recv;				/** @var file recv error */
	char	msg_recv[ 128];			/** @var batch err message */

	time_t	stat_time;				/** @var last process time */
	int		str_time;				/** @var batch start time HHMMSS - 배치 시작 시간 */
	int		end_time;				/** @bar batch end time HHMMSS - 이시간 이후에는 배치가 안돌아감 */
	int		err_time;				/** @ver cur_time > err_time 이면 error - hhmmss */
	int		bat_errno;				/** @var batch err number */
	char	bat_errmsg[ 128];		/** @var batch err message */
#endif
}	BAT_TBL;

/** ***************************************************************************
**  @struct		BAT
**  @brief		배치 관리 구조체
***************************************************************************** */
typedef struct _bat_
{
	int			opt;			/* 1-create 2-send 3-recv */
	MEM			*mem;
	BAT_TBL		*base;
	BAT_TBL		*curr;
	BAT_STAT	*stat;
	BAT_JOB_TBL	*job;
	char		job_date[ 20];	/* 배치 작업 일자 */
	int			fd;				/* data file descripter */
}	BAT;

#endif


/***** Module : bat.c *****/
size_t      Bat_GetSize( int opt);
BAT*        Bat_Malloc();
int         Bat_Free( BAT *bat);
BAT*        Bat_Create( char *id);
BAT*        Bat_Open( char *id);
BAT*        Bat_OpenByCreat( char *id);
BAT*        Bat_OpenBySend( char *id);
BAT*        Bat_OpenByRecv( char *id);
int         Bat_OpenDataFile( BAT *bat, BAT_TBL *tp);
int         Bat_Close( BAT *bat);
char*       Bat_GetFileName( BAT *bat, char *id);
int         Bat_LoadSwapFile( BAT *bat, char *f_name);
int         Bat_SaveSwapFile( BAT *bat);
int         Bat_LoadConfig( BAT *bat, char *f_name, int flag);
BAT_TBL*    Bat_MakeTbl( BAT *bat, char *str);
int         Bat_WriteTbl( BAT *bat, BAT_TBL *tbl);
BAT_TBL*    Bat_FindId( BAT *bat, char *id);
int         Bat_PrintStat( BAT *bat);
int         Bat_PrintTable( BAT *bat);
int         Bat_PrintBatTbl( BAT *bat, BAT_TBL *bp, int flag);
int         Bat_GetCnt( BAT *bat);
int         Bat_GetFileCnt( BAT *bat, BAT_TBL *bp);
int         Bat_GetSendCnt( BAT *bat, BAT_TBL *bp);
int         Bat_GetCntFile( BAT *bat);
int         Bat_SetBatTbl( BAT *bat, BAT_TBL *bp, char *value, int flag);
int         Bat_SetStat( BAT *bat, int stat, int err, char *format, ...);
int         Bat_CloseDataFile( BAT *bat, BAT_TBL *tp, int err_no, char *msg);
int         Bat_Write( BAT *bat, char *data, int sz);
int         Bat_Read( BAT *bat, char *data, int sz);
int         Bat_ReadDataFile( BAT *bat, BAT_TBL *tp, char *data, int sz);
int         Bat_ErrMessage( BAT *bat, BAT_TBL *bt, int err_no, char *msg);
BAT_TBL*    Bat_CheckSendStat( BAT *bat);
int         Bat_Daily( BAT *bat);

