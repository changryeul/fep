#ifndef SAM_H
#define	SAM_H	1

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "mem.h"
#include "sem.h"
#include "cfg.h"

#if 0
#define		SAM_MEM_KEY		0xfe010100
#endif
#define		MAX_SAM_TBL		256
#define		SAM_USER_SZ		512

#define		SAMTBL_UPDATE	0
#define		SAMTBL_INSERT	1
#define		SAMTBL_DELETE	2

#define		SAM_GET_RPOS( x)			x->curr->rpos

typedef struct _sam_head_
{
	char	seq[ 10];							/* sequence number */
	char	len[ 6];							/* record length */
	char	time[ 19];							/* write time - time stemp */
	char	spc[ 1];							/* space */
}	SAM_HEAD;

typedef struct _sam_table_
{
	int			pos;								/* record pos ... last=record count */
	char		id[ 32];							/* sam file id */
	int			used;								/* 0:not_used,1:used */
	time_t		uptime;								/* last update time */
	time_t		daily;								/* daily update time */
	int			job_time;							/* JDDHHMMSS J:0=daily,1:weekly,2:monthly DD:daily는 무시, week는 요일넘버, month는 날짜 */
	key_t		key;								/* sem 접근 키 */
	int			wpos;								/* sam file write pos */
	int			rpos;								/* sam file read pos */
	pid_t		wpid;								/* last write process id */
	pid_t		rpid;								/* last read process id */
	time_t		wtime;								/* last write time */
	time_t		rtime;								/* last read time */
	char		name[ 64];							/* sam file name ... *.fifo *.sam */
	int			rlen;								/* sam file record length 고정길이 */
	char		dm[ 4];								/* delimiter - \r\n or \n or \0  rec_len = head + rlen + strlen( dm) */
	char		user[ SAM_USER_SZ];					/* user area */
	int			user_sz;							/* config load user field size */
	char		filler[ 64];						/* filler */
}	SAM_TBL;

typedef struct _sam_stat_
{
	char	ver_string[ 64];
	time_t	create;
	key_t	key;
	char	swp_name[ 512];
	char	sam_path[ 512];
	char	bak_path[ 512];
	char	cfg_name[ 512];
	int		cnt;
}	SAM_STAT;

typedef struct _sam_
{
	key_t		key;							/* shared memoty key */
	MEM			*mem;							/* shared memory pointer 	*/
	SEM			*sem;							/* semaphore pointer 		*/
	int			fifo;							/* fifo desc */
	int			fd;								/* sam file desc */
	SAM_TBL		*base;							/* shared memory SAM_TABLE base pointer */
	SAM_TBL		*curr;							/* shared memory SAM_TABLE current pointer */
	SAM_STAT	*stat;							/* SAM_TBLE stat */
}   SAM;

typedef struct _sam_write_
{
	MEM			*mem;							/* shared memory pointer 	*/
	SAM			*sam[ MAX_SAM_TBL];
	int			cnt;
	SAM_TBL		*base;							/* shared memory SAM_TABLE base pointer */
	SAM_TBL		*curr;							/* shared memory SAM_TABLE current pointer */
	SAM_STAT	*stat;							/* SAM_TBLE stat */
	char		path[ 512];						/* sam file path - config(sam_file_path) */
	char		bak_path[ 512];					/* sam file backup path - config(bak_file_path) */
}	SAM_WRITE;

#endif /* SAM_H */

/***** Module : sam.c *****/
SAM*        Sam_Create( char *cfg_name);
int         Sam_Remove( SAM *sam);
int         Sam_RemoveSub( SAM *sam, SAM_TBL *tp);
int         Sam_RemoveAll( SAM *sam);
int         Sam_ClearSub( SAM *sam, SAM_TBL *tp);
int         Sam_Clear( SAM *sam, SAM_TBL *tp);
SAM*        Sam_Open( key_t key, char *id);
int         Sam_OpenIpc( SAM *sam, char *id);
SAM*        Sam_MakeSub( SAM *samp, char *id);
SAM_WRITE*  Sam_OpenWrite( SAM *sam);
int         Sam_CloseWrite( SAM_WRITE *swp);
int         Sam_Close( SAM *sam);
int         Sam_LoadSwap( SAM *sam);
SAM*        Sam_GetSamPtr( SAM_WRITE *swp, char *id);
int         Sam_GetWritePos( SAM *Sam);
int         Sam_GetReadPos( SAM *Sam);
int         Sam_LoadCfg( SAM *sam, char *cfg_name);
int         Sam_CfgCheck( SAM *sam, char *cfg_name);
int         Sam_CheckCfg( SAM *sam, CFG *cfg);
int         Sam_Swap( SAM *sam);
int         Sam_SwapFile( SAM *sam, char *file_name);
int         Sam_LoadFile( SAM *sam, char *swap_file_name);
SAM_TBL*    Sam_MakeTbl( SAM *sam, char *rec);
int         Sam_MakeIpcAll( SAM *sam);
int         Sam_MakeIpc( SAM *sam);
int         Sam_UpdateTbl( SAM *sam, SAM_TBL *tbl);
void*       Sam_GetUserPtr( SAM *sam);
int         Sam_GetUserArea( SAM *sam, SAM_TBL *stp, char *rec);
SAM_TBL*    Sam_FindId( SAM *sam, char *id);
int         Sam_WriteId( SAM_WRITE *sw, char *id, char *rec, int sz);
int         Sam_Write( SAM *sam, char *rec, int sz);
int         Sam_Read( SAM *sam, char *rec, int sz);
int         Sam_ReadT( SAM *sam, char *rec, int sz, int timeout);
int         Sam_ReadPT( SAM *sam, int pos, char *rec, int sz, int timeout);
int         Sam_WaitFifo( SAM *sam, int timeout);
int         Sam_GetRecLen( SAM *sam);
off_t       Sam_GetPos( SAM *sam, int pos);
int         Sam_JobProcess( SAM *sam, int opt);
int         Sam_DailyProcess( SAM *sam, int opt);
int         Sam_WeeklyProcess( SAM *sam, int opt);
int         Sam_Print( SAM *sam);
int         Sam_PrintList( SAM *sam);
int         Sam_PrintTbl( SAM *sam, SAM_TBL *tp);
int         Sam_InfoField( SAM *sam, SAM_TBL *tp, char *name);
int         Sam_EditField( SAM *sam, SAM_TBL *tp, char *name, char *val);
int         Sam_PrintTblFp( SAM *sam, SAM_TBL *tp, FILE *fp);
int         Sam_PrintTblFile( SAM *sam, SAM_TBL *tp, char *fname);
int         Sam_EditRecFile( SAM *sam, char *f_name);
int         Sam_LoadRecFile( SAM *sam, SAM_TBL *sam_tbl, char *f_name);
int         Sam_GetEditLine( SAM *sam, SAM_TBL *sp, char *rec);

