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
#include "cmd.h"
#include "etc.h"
#include "map.h"

#ifndef MAIN_H
#define	MAIN_H	1

extern CFG		*Cfg;
extern CMD		*Cmd;
extern MAP		*Map;
extern int		Continue;

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
	int		timeout;					/* map loop timeout */
	int		interval;					/* loop interval */
}	PARAM;
extern PARAM	*Param;

typedef struct _my_data_
{
	time_t		cur_time;
	int			alarm;
}	MY_DATA;
extern MY_DATA	*MyData;

#endif	/* MAIN_H */

/***** Module : mon.c *****/
int         Mon_Main();                                                     /* 함수 작성    */
int         Mon_MainInit( MAP *map);                                        /* 모니터링맵 초기화 */
int         Mon_TimeSet( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_TimeStr( MAP *map, MAP_FIELD *field);                       /* time stemp convert */

/***** Module : main.c *****/
int         main( int argc, char *argv[]);                                  /* GetOption   - 인수 분석 및 변수 초기화 */
int         GetOption( int argc, char *argv[]);                             /* 프로그램 시작시 받은 인수를 분석, 프로그램에서 사용할 변수를 초기화 한다. */
void        SignalProcess( int sig_id);                                     /* 프로그램에서 사용할 시그널 */
int         InitProcess( int argc, char *argv[]);                           /* 프로그램 초기화 */
int         MainProcess( int argc, char *argv[]);                           /* 프로그램 수행 */
int         TermProcess( int argc, char *argv[]);                           /* 프로세스 종료 루틴 수행 */
int         ParamPrint( PARAM *param);                                      /* 프로그램에서 사용할 전역변수들의 구조체 값 표시 */

/***** Module : task.c *****/
int         TaskInit();                                                     /*  */
int         CmdTest( int argc, char *argv[]);                               /* 1. 함수 프로토타입 정의 */
int         CmdMon( int argc, char *argv[]);                                /* 함수 작성    */
int         CmdHelp( int argc, char *argv[]);                               /*  */
int         CmdQuit( int argc, char *argv[]);

