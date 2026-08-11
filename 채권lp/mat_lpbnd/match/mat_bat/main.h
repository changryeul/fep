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

#if 0
#include "WCM001.h"
#endif 

#include "order.h"

#ifndef MAIN_H
#define	MAIN_H	1

extern CFG		*Cfg;
extern MAP		*Map;
extern int		Continue;

typedef struct _param_
{
	int		argc;						/** @var command line argument count */
	char	**argv;						/** @var command line argument */
	char	argo[ 32];					/** @var argument option buffer */
	int		args;						/** @var argument option size */
	char 	cfg_name[ 512];				/* config file name */
	char 	holiday_file[ 512];			/* 공휴일 data file name */
	char	log_name[ 512];				/* log file name */
	int		log_flag;					/* 0-console 1-file 2-mem */
	int		log_level;					/* log_level */
	int		interval;					/* loop interval */
	int		timeout;					/* recv timeout */
	int		start;						/* open batch start time */
	int		end;						/* end batch start time */
	int		last_time;					/* 월말 영업일 마감 시간 */
	int		force;						/* 강제 batch 수행 1-start batch, 2-end batch */
}	PARAM;
extern PARAM	*Param;

#endif	/* MAIN_H */

/***** Module : ------- *****/
int GetLastDay();
int HolidayProcess();


/***** Module : db_access.pc *****/
int         Db_Process();
int         Db_GetCodFxPair();

/***** Module : main.c *****/
int         main( int argc, char *argv[]);                                  /* GetOption   - 인수 분석 및 변수 초기화 */
int         GetOption( int argc, char *argv[]);                             /* 프로그램 시작시 받은 인수를 분석, 프로그램에서 사용할 변수를 초기화 한다. */
void        SignalProcess( int sig_id);                                     /* 프로그램에서 사용할 시그널 */
int         InitProcess( int argc, char *argv[]);                           /* 프로그램 초기화 */
int         MainProcess( int argc, char *argv[]);                           /* 프로그램 수행 */
int         TermProcess( int argc, char *argv[]);                           /* 프로세스 종료 루틴 수행 */
int         ParamPrint( PARAM *param);                                      /* 프로그램에서 사용할 전역변수들의 구조체 값 표시 */
int         StartBatchProcess();                                            /* 프로그램에서 사용할 전역변수들의 구조체 값 표시 */
int         EndBatchProcess();                                              /* 프로그램에서 사용할 전역변수들의 구조체 값 표시 */

/***** Module : proc.c *****/
int         Proc_LoadMessage( MAT *mat);                                    /* 주문 정합 check */
int         Proc_LineMessage( MAT *mat, char *line);                        /* 주문 정합 check */
int         Proc_LoadCurrent();                                             /* 주문 정합 check */
int         Proc_LineCurrent( MAT *mat, char *line);                        /* 주문 정합 check */

/***** Module : mat_batch.c *****/
int         Mat_BatchStart( MAT *mat);                                      /* 매칭엔진에 필요한 ipc를 생성 */
int         Mat_BatchEnd( MAT *mat);                                        /* 매칭엔진에 필요한 ipc를 생성 */

