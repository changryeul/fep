/** ***************************************************************************
**  @file       main.h
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  메인 모듈 관련 헤더
**  프로그램 사용 파라메터 정의
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "log.h"
#include "cfg.h"
#include "etc.h"
#include "map.h"

#include "mat.h"
#include "smq.h"

#ifndef MAIN_H
#define	MAIN_H	1

#define DB_USED	0

extern CFG		*Cfg;
extern MAP		*Map;
extern int		Continue;
extern MAT		*Mat;
extern int		DbCon;

typedef struct _param_
{
	int		argc;						/** @var command line argument count */
	char	**argv;						/** @var command line argument */
	char	argo[ 32];					/** @var argument option buffer */
	int		args;						/** @var argument option size */
	int		ipc_flag;					/* 0-none, 1-create, 2-remove */
	char 	cfg_name[ 512];				/* config file name */
	char	log_name[ 512];				/* log file name */
	char	map_name[ 512];				/* map file name */
	char	map_small[ 512];			/* map file name */
	char	map_index[ 512];			/* map file name */
	char	map_smq[ 512];				/* map file name */
	char	map_statis[ 512];			/* map file name */
	char	map_order[ 512];			/* map file name */
	int		timeout;					/* map loop timeout */
	int		interval;					/* loop interval */
	int		color;						/* map color 표시 */

	int		max_record;					/* shm max record */
	int		max_curr;					/* shm index 갯수 */
	int		max_conform;				/* 체결 역전 방지 position */
	key_t	ipc_key;					/* shm,sem 접근 ipc key */
	char	ord_pipe[ 512];				/* order pipe full path */
	char	exe_pipe[ 512];				/* execute pipe full path */
}	PARAM;
extern PARAM	*Param;

typedef struct _my_data_
{
	time_t		cur_time;
	int			alarm;
}	MY_DATA;
extern MY_DATA	*MyData;

typedef struct _mon_
{
	int		alarm;
	time_t	cur_time;
}	MON;

#endif	/* MAIN_H */

/***** Module : proc_swap.c *****/
int         main( int argc, char *argv[]);
int         GetOption( int argc, char *argv[]);
void        SignalProcess( int sig_id);
int         InitProcess( int argc, char *argv[]);
int         MainProcess( int argc, char *argv[]);
int         TermProcess( int argc, char *argv[]);
int         ParamPrint( PARAM *param);

/***** Module : process.c *****/
int         Proc_Init( MAP *map, MY_DATA *my);
int         Process( MAP *map);
int         Proc_AlarmStat( MAP *map, MAP_FIELD *field);
int         Proc_TimeProc( MAP *map, MAP_FIELD *field);

/***** Module : ../print.c *****/
int         ORDER_HEAD_Print( ORDER_HEAD* ptr);
int         ORDER_RECV_Print( ORDER_RECV* ptr);
int         ORDER_SEND_Print( ORDER_SEND* ptr);
int         ORDER_Print( ORDER* ptr);
int         ORDER_FilePrint( FILE *fp, ORDER* ptr);
int         APSISE_Print( APSISE* ptr);
int         SISE_ENTRY_Print( SISE_ENTRY* ptr);
int         FX_QUOTE_T_Print( FX_QUOTE_T* ptr);

/***** Module : mon_index.c *****/
int         Mon_IndexMain( MAT *mat, SMQ *smq);                             /* 함수 작성    */
int         Mon_IndexMainInit( MAP *map, MAT *mat, SMQ *smq);               /* 모니터링맵 초기화 */

/***** Module : mon.c *****/
int         Mon_Main( MAT *mat, SMQ *smq, char *map_name);                  /* 함수 작성    */
int         Mon_MainInit( MAP *map, MAT *mat, SMQ *smq);                    /* 모니터링맵 초기화 */
int         Mon_ProcessRecord( MAP *main_map, char *map_file, MAT *mat, int line, int side);
int         Mon_ProcessRecordInit( MAP *map, MAT *mat, int line, int side);
int         Mon_ProcessSise( MAP *main_map, char *map_sise, MATSISE *arg_sise, char *title);
int         Mon_ProcessFileVi( MAP *main_map, MAT *mat, int pos, int side);
int         Mon_ProcessFile( MAP *main_map, MAT *mat, int pos, int side);
int         Map_FileEdit( char *map_name, char *file_name);
int         Mon_List( MAT *mat, char *base_cur, char *cont_cur);            /* 함수 작성    */
int         Mon_ListInit( MAP *map, MAT *mat, char *base_cur, char *cont_cur);/* 모니터링맵 초기화 */
int         Mon_Index( MAT *mat);                                           /* 함수 작성    */
int         Mon_IndexInit( MAP *map, MAT *mat);                             /* 모니터링맵 초기화 */
int         Mon_TimeSet( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_TimeStr( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_GapProc( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_GapTime( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_Current( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_IndexChk( MAP *map, MAP_FIELD *field);                      /* time stemp convert */

