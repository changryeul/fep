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
#include "mem.h"

#ifndef MAIN_H
#define	MAIN_H	1


typedef struct _mds_mem_
{
	int			no;					/* used=not 0, unused = 0 */
	char		name[ 4];			/* 3글자 요약 */
	char		full_name[ 64];		/* 수신 시세 기관명 */
	MEM			*mem;				/* 공유메모리 mem */
	key_t		key;				/* 공유메모리 접근 key */
	void		*base;				/* 공유메모리 pointer */
}	MDS_MEM;

typedef struct _param_
{
	int		argc;						/** @var command line argument count */
	char	**argv;						/** @var command line argument */
	char	argo[ 32];					/** @var argument option buffer */
	int		args;						/** @var argument option size */
	char 	cfg_name[ 512];				/* config file name */
	char	log_name[ 512];				/* log file name */
	int		log_level;					/* log_level - # Dev=0,Trc=1.Tst=5,Dbg=10,App=20,Msg=30,Lib=40,War=50 */
	int		log_flag;					/* log_flag 0-console,1-file,2-memory */
	int		timeout;					/* map loop timeout */
	int		interval;					/* loop interval */

	key_t	mem_key;					/* mds shared memory key */
	char	main_map[512];				/* main map name */
	char	market_map[512];			/* market map name */
	char	fold_map[512];				/* fold map name */
	char	quot_map[512];				/* fold map name */
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
    int     alarm;
    time_t  cur_time;
}   MON;


extern CFG		*Cfg;
extern MAP		*Map;
extern char		Prompt[ 256];
extern int		Continue;

extern MDS_MEM	MdsMem[ 10];
extern int		MdsMemCurr;

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

/***** Module : print.c *****/
#if 0
int         XCHG_Print( XCHG* ptr);
int         MDARCH_Print( MDARCH* ptr);
int         kswitch_t_Print( kswitch_t* ptr);
int         MDCANDLE_Print( MDCANDLE* ptr);
int         mdside_t_Print( mdside_t* ptr);
int         mdquot_t_Print( mdquot_t* ptr);
int         MDFOLD_Print( MDFOLD* ptr);
#endif

