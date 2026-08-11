#ifndef PROC_H
#define	PROC_H	1

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "mem.h"

/*
#define		PROC_MEM_KEY		0xfa110001
*/
#define		MAX_PROC_TBL		256
#define		MAX_HOLI_TBL		256
#define		DAEMON_POS			(MAX_PROC_TBL +1)
#define		SWAP_POS			MAX_PROC_TBL

/* memory map  ***********************************
	- PROC_TBL[ MAX_PROC_TBL]     - Process table
	- PROC_TBL at MAX_PROC_TBL    - Daemon table
	- PROC_STAT                
**************************************************/

typedef struct _proc_tbl_
{
	char		id[ 32];						/* 프로그램 ID */
	int			cmd;							/* command - 프로세스 실행/종료 등의 명령 */
	int			cmd_cnt;						/* kill 명령 횟수 - 10이 넘으면 SIGKILL로 수행 */
	int			used;							/* record 사용여부 */
	int			stat;							/* 상태 0:none 1:run 9:kill */
	pid_t		pid;							/* process id */
	int			s_time;							/* 시작시간 HHMMSS */
	int			e_time;							/* 종료시간 HHMMSS */
	char		week[ 10];						/* 서비스 수행 요일  일월화수목금토공 - 시작시간을 기준으로 한다 (공-공휴일) */
	char		path[ 512];						/* 프로그램 위치 - 실행위치 */
	char		name[ 256];						/* 프로그램 이름 - argument 포함 */
	int			option;							/* job option - 0:한번만 수행 1:종료시 다시 살림 */
	int			r_cnt;							/* run count 수행 횟수 */
	int			max_run;						/* 최대 run 수행 횟수 */
	time_t		sr_time;						/* process 시작 시간 */
	time_t		er_time;						/* process 종료 시간 */
}	PROC_TBL;

typedef struct _proc_stat_
{
	key_t		key;							/* shared memory key */
	char		ver_string[ 64];
	time_t		create;							/* make(create) time */
	int			cnt;							/* PROC_TBL counter */
	int			week;							/* today flag */
	int			holiday[ MAX_HOLI_TBL];			/* 0:end, Holiday array - YYYYMMDD */
}	PROC_STAT;

typedef struct _proc_
{
	MEM			*mem;							/* shared memory pointer 	*/
	int			holiday;						/* 1=holiday, 0=not holiday */
	PROC_TBL	*base; 							/* shared memory PROC_TABLE pointer */
	PROC_TBL	*curr;
	PROC_TBL	*daemon;
	PROC_STAT	*stat;
}   PROC;

#endif /* PROC_H */

/***** Module : proc.c *****/
MEM*        ProcTbl_Create( char *cfg_name);
MEM*        ProcTbl_Open( char *cfg_name);
int         ProcTbl_Close( MEM *mem);
int         ProcTbl_Info( MEM *mem);
int         ProcTbl_InfoSub( int pos, PROC_TBL *rec);
int         ProcTbl_RegiMe( MEM *mem, int cmd, char *id);
int         ProcTbl_SwapLoad( MEM *mem, char *swap_file_name);
int         ProcTbl_SwapProc( MEM *mem, char *swap_file_name);
int         ProcTbl_Compare( const void *a1, const void *a2);
int         ProcTbl_CheckCfg( MEM *mem, char *cfg_file_name, int opt);
int         ProcTbl_MakeRec( MEM *mem, PROC_TBL *proc_tbl, char *proc_rec);
int         ProcTbl_LoadRec( MEM *mem, PROC_TBL *proc);
int         ProcTbl_ClearRec( MEM *mem);
PROC*       Proc_Open( char *cfg_name);
int         Proc_Close( PROC *proc);
int         Proc_Run( PROC *proc);
int         Proc_HealthCheck( PROC_TBL *tbl);
int         Proc_CheckPid( PROC *proc);
int         Proc_RunCheck( PROC *proc, PROC_TBL *tbl, int int_time);
int         Proc_RunProc( PROC *proc, PROC_TBL *tbl);
int         Proc_Stop( PROC *proc);
int         Proc_StopCheck( PROC *proc, PROC_TBL *tbl, int int_time);
int         Proc_StopProc( PROC *proc, PROC_TBL *tbl);
int         Proc_StopAll( PROC *proc);
int         Proc_WaitAll( PROC *proc);
int         Proc_Check( PROC *proc);
PROC_TBL*   Proc_FindPid( PROC *proc, pid_t pid);
PROC_TBL*   Proc_FindId( PROC *proc, char *id);
int         Proc_StopMake( PROC *proc, PROC_TBL *curr, int stat);
int         Proc_StopCmd( PROC *proc, char *id);
int         Proc_RunCmd( PROC *proc, char *id);
int         Proc_InfoProc( PROC *proc, char *id);
int         Proc_InfoProcSub( PROC *proc, PROC_TBL *curr);
int         Proc_SetProc( PROC *proc, char *id, char *name, char *value);
int         Proc_Infofield( PROC *proc, PROC_TBL *curr, char *name);
int         Proc_SetField( PROC *proc, PROC_TBL *curr, char *name, char *value);
int         Proc_DailyJob( PROC *proc);

