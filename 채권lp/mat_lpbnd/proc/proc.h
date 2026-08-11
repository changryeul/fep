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
#define		PROC_MEM_KEY		0xfa003001
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
	int			pre_week;						/* yesterday flag */
	int			nex_week;						/* tomorrow flag */
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
MEM*        ProcTbl_Create( char *cfg_name);                                /* 프로세스 테이블 공유메모리 생성 */
MEM*        ProcTbl_Open( char *cfg_name);                                  /* 프로세스 공유메모리 연결 */
int         ProcTbl_Close( MEM *mem);                                       /* 프로세스 공유메모리연결을 끊는다. */
int         ProcTbl_Info( MEM *mem);                                        /* 프로세스 공유메모리 테이블을 출력(stdout) */
int         ProcTbl_InfoSub( int pos, PROC_TBL *rec);                       /* 프로세스 테이블 항목을 출력 */
int         ProcTbl_RegiMe( MEM *mem, int cmd, char *id);                   /* 프로세스 테이블에 상태 등록 */
int         ProcTbl_SwapLoad( MEM *mem, char *swap_file_name);              /* 프로세스 관리 */
int         ProcTbl_SwapProc( MEM *mem, char *swap_file_name);              /* 프로세스 관리 */
int         ProcTbl_Compare( const void *a1, const void *a2);               /* 프로세스 관리 */
int         ProcTbl_CheckCfg( MEM *mem, char *cfg_file_name, int opt);      /* 프로세스 관리 */
int         ProcTbl_MakeRec( MEM *mem, PROC_TBL *proc_tbl, char *proc_rec); /* 프로세스 관리 */
int         ProcTbl_LoadRec( MEM *mem, PROC_TBL *proc);                     /* 프로세스 관리 */
int         ProcTbl_ClearRec( MEM *mem);                                    /* 프로세스 관리 */
int         ProcTbl_GetHolidayInt( MEM *mem, char *line);                   /* 프로세스 관리 */
int         ProcTbl_LoadHolidayTbl( MEM *mem, char *cfg_name);              /* 프로세스 관리 */
PROC*       Proc_Open( char *cfg_name);                                     /* 프로세스 관리 */
int         Proc_Close( PROC *proc);                                        /* 프로세스 관리 */
int         Proc_HealthCheck( PROC_TBL *tbl);                               /* 모니터링을 위한 장애여부 판단 */
int         Proc_CheckPid( PROC *proc);                                     /* 프로세스 테이블에 등록된 pid를 check하여(kill function) 실제 떠 있는지 검사 */
int         Proc_Run( PROC *proc);                                          /* 프로세스 관리 */
int         Proc_RunCheck( PROC *proc, PROC_TBL *tbl, int int_time);        /* 프로세스를 실행해야 하는지 검사 */
int         Proc_RunProc( PROC *proc, PROC_TBL *tbl);                       /* 프로세스 관리 */
int         Proc_Stop( PROC *proc);                                         /* 프로세스 관리 */
int         Proc_StopCheck( PROC *proc, PROC_TBL *tbl, int int_time);       /* 프로세스 중지해야 할지 check */
int         Proc_StopProc( PROC *proc, PROC_TBL *tbl);                      /* 프로세스 관리 */
int         Proc_StopAll( PROC *proc);                                      /* 프로세스 관리 */
int         Proc_WaitAll( PROC *proc);                                      /* 프로세스 관리 */
int         Proc_Check( PROC *proc);                                        /* 프로세스 관리 */
PROC_TBL*   Proc_FindPid( PROC *proc, pid_t pid);                           /* 프로세스 관리 */
PROC_TBL*   Proc_FindId( PROC *proc, char *id);                             /* 프로세스 관리 */
int         Proc_StopMake( PROC *proc, PROC_TBL *curr, int stat);           /* 프로세스 관리 */
int         Proc_StopCmd( PROC *proc, char *id);                            /* 프로세스 관리 */
int         Proc_RunCmd( PROC *proc, char *id);                             /* 프로세스 관리 */
int         Proc_GetDaemonPid( PROC *proc);                                 /* proc_d pid get */
int         Proc_InfoProc( PROC *proc, char *id);                           /* 프로세스 관리 */
int         Proc_InfoProcSub( PROC *proc, PROC_TBL *curr);                  /* 프로세스 관리 */
int         Proc_SetProc( PROC *proc, char *id, char *name, char *value);   /* 프로세스 관리 */
int         Proc_Infofield( PROC *proc, PROC_TBL *curr, char *name);        /* 프로세스 관리 */
int         Proc_SetField( PROC *proc, PROC_TBL *curr, char *name, char *value);/* 프로세스 관리 */
int         Proc_DailyJob( PROC *proc);                                     /* 프로세스 관리 */

