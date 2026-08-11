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

#ifndef STP_H
#define	STP_H	1

#define		STP_MEM_KEY		0xfe011100
#define		MAX_STP_TBL		256
#define		STP_STAT_NONE(x)	Stp_Stat( x, 0)
#define		STP_STAT_WAIT(x)	Stp_Stat( x, 1)
#define		STP_STAT_CONN(x)	Stp_Stat( x, 2)
#define		STP_STAT_END(x)		Stp_Stat( x, 3)
#define		STP_STAT_ERR(x)		Stp_Stat( x, 9)

/* get pointer */
#define		STP_BIND_ADDR		1
#define		STP_CONN_ADDR		2
#define		STP_FORM_TYPE		3
/* get int */
#define		STP_SVR_PORT		11
#define		STP_CLI_PORT		12
#define		STP_READ_POS		13
#define		STP_WRITE_POS		14
/* get/set SEQ_T */
#define		STP_READ_SEQ		21
#define		STP_WRITE_SEQ		22
#define		STP_SEND_SEQ		23

#define		STPTBL_UPDATE	0
#define		STPTBL_INSERT	1
#define		STPTBL_DELETE	2

typedef		long long		SEQ_T;

typedef struct _stp_head_
{
	char	seq[ 10];							/* sequence number */
	char	len[ 6];							/* record length */
	char	time[ 19];							/* write time - time stemp */
	char	spc[ 1];							/* space */
}	STP_HEAD;

typedef struct _stp_table_
{
	int			pos;								/* record pos ... last=record count */
	char		id[ 32];							/* stp file id */
	int			used;								/* 0:not used, 1:used */
	int			type;								/* protocol type */
	time_t		uptime;								/* last update time */
	time_t		daily;								/* daily update time */
	key_t		key;								/* sem 접근 키 */
	SEQ_T		bseq;								/* base sequence */
	SEQ_T		wseq;								/* write sequence */
	SEQ_T		rseq;								/* read sequence */
	SEQ_T		sseq;								/* send sequence */
	time_t		wtime;								/* last write time */
	time_t		rtime;								/* last read time */
	time_t		stime;								/* last send time */
	time_t		ctime;								/* connect time */
	time_t		dtime;								/* disconnect time */
	time_t		etime;								/* last error time */
	int			wpos;								/* stp file write pos */
	int			rpos;								/* stp file read pos */
	pid_t		wpid;								/* last write process id */
	pid_t		rpid;								/* last read process id */
	char		baddr[ 32];							/* server bind address or server address */
	char		caddr[ 32];							/* connect addr or peer addr */
	int			sport;								/* server port */
	int			cport;								/* client port */
	int			stat;								/* session stat */
	char		name[ 64];							/* stp file name ... *.fifo *.stp */
	int			rlen;								/* stp file record length 고정길이 */
	char		dm[ 4];								/* delimiter - \r\n or \n or \0 - record length = head + rlen + strlen( dm) */
	char		fm[ 12];							/* 응답 전문 업체 정보 */
													/* format 2     fm[0]=recv, fm[1]=send, 값('0')-head size, ('1')-250, */
													/* format 3     fm[0]=recv, fm[1]=send, 값('0')-head size, ('1')-295 */
													/* push format  fm[0]='0' async mode, fm[0]='1' sync mode */
	char		sub_id[ 20];						/* sub id */
	char		com_name[ 32];						/* 회사명 */
	char		filler[ 64];						/* filler */
}	STP_TBL;

/*
                       wseq,rseq     base_seq     seek_pos(file)
nomal type (type=0)  - A             0            A
old-format (type=1)  - A             A+           A
new-format2 (type-2) - B             B+           A
new_format3 (type-3) - B             B+           A
	0 - 항상 0
	A - 일주일에 한번 clear   (+) - wseq + base_seq
	B - 하루 한번 clear       (+) - wseq + base_seq
*/

typedef struct _stp_stat_
{
	char	ver_string[ 64];
	time_t	create;
	char	swap_name[ 512];
	char	path[ 512];
	char	backup_path[ 512];
	int		cnt;
}	STP_STAT;

typedef struct _stp_
{
	MEM			*mem;							/* shared memory pointer 	*/
	SEM			*sem;							/* semaphore pointer 		*/
	int			fifo;							/* fifo desc */
	int			fd;								/* stp file desc */
	STP_TBL		*base;							/* shared memory STP_TABLE base pointer */
	STP_TBL		*curr;							/* shared memory STP_TABLE current pointer */
	STP_STAT	*stat;							/* STP_TBLE stat */
}   STP;

typedef struct _stp_write_
{
	MEM				*mem;							/* shared memory pointer 	*/
	STP			*stp[ MAX_STP_TBL];
	int				cnt;
	STP_TBL		*base;							/* shared memory STP_TABLE base pointer */
	STP_TBL		*curr;							/* shared memory STP_TABLE current pointer */
	STP_STAT		*stat;							/* STP_TBLE stat */
	char			path[ 512];						/* stp file path - config(stp_file_path) */
	char			bak_path[ 512];					/* stp file backup path - config(bak_file_path) */
}	STP_WRITE;

#endif /* STP_H */

/***** Module : stp.c *****/
STP*        Stp_Create( char *cfg_name);
int         Stp_Remove( STP *stp);
int         Stp_Clear( STP *stp);
STP*        Stp_Open( char *id);
STP*        Stp_OpenBySubId( char *subid);
int         Stp_OpenIpc( STP *stp, char *id);
STP*        Stp_MakeSub( STP *stpp, char *id);
STP_WRITE*  Stp_OpenWrite( STP *stp);
int         Stp_CloseWrite( STP_WRITE *swp);
int         Stp_Close( STP *stp);
int         Stp_LoadSwap( STP *stp, char *cfg_name);
STP*        Stp_GetStpPtr( STP_WRITE *swp, char *id);
int         Stp_LoadCfg( STP *stp, char *cfg_name);
int         Stp_LoadCfgForce( STP *stp, char *cfg_name);
int         Stp_CfgCheck( STP *stp, char *cfg_name);
int         Stp_CfgLoad( STP *stp, char *cfg_name);
int         Stp_CheckCfg( STP *stp, CFG *cfg);
int         Stp_SwapFile( STP *stp, char *swap_file_name);
int         Stp_LoadFile( STP *stp, char *swap_file_name);
STP_TBL*    Stp_MakeTbl( STP *stp, char *rec);
int         Stp_MakeIpc( STP *stp);
int         Stp_UpdateTbl( STP *stp, STP_TBL *tbl);
STP_TBL*    Stp_FindId( STP *stp, char *id);
STP_TBL*    Stp_FindSubId( STP *stp, char *subid);
int         Stp_Write( STP *stp, char *rec, int sz);
int         Stp_WriteId( STP_WRITE *stp_write, char *id, char *rec, int sz);
int         Stp_Read( STP *stp, char *rec, int sz);
int         Stp_ReadT( STP *stp, char *rec, int sz, int timeout);
int         Stp_ReadST( STP *stp, SEQ_T seq, char *rec, int sz, int timeout);
int         Stp_WaitFifo( STP *stp, int timeout);
int         Stp_GetReadPos( STP *stp);
int         Stp_GetRecLen( STP *stp);
off_t       Stp_GetPos( STP *stp, int seq);
int         Stp_DailyProcess( STP *stp);
int         Stp_DailyProcess( STP *stp);
int         Stp_WeeklyProcess( STP *stp);
int         Stp_WeeklyProcess( STP *stp);
int         Stp_Print( STP *stp);
int         Stp_PrintList( STP *stp);
int         Stp_PrintTblRaw( STP *stp, STP_TBL *tp);
int         Stp_PrintTbl( STP *stp, STP_TBL *tp);
int         Stp_InfoField( STP *stp, STP_TBL *tp, char *name);
int         Stp_EditField( STP *stp, STP_TBL *tp, char *name, char *val);
int         Stp_Stat( STP *stp, int stat);
void*       Stp_GetPtr( STP *stp, int opt);
int         Stp_GetInt( STP *stp, int opt);
SEQ_T       Stp_GetSeq( STP *stp, int opt);
int         Stp_SetSeq( STP *stp, int opt, SEQ_T seq);

