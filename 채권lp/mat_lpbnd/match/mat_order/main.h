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
	char 	cfg_name[ 32];				/* config file name */
	char	log_name[ 32];				/* log file name */
	int		log_flag;					/* 0-console 1-file 2-mem */
	int		log_level;					/* 0-debug, 100-error */
	char	que_name[ 32];				/* smq name */
	char	file_name[ 512];			/* input file name */
	char	seq_file[ 512];					/* order_no 관리 file */
	int		interval;					/* usleep interval */
	int		timeout;					/* send timeout */
	int		loop;						/* end of file loop */
	int		type;						/* data type */
}	PARAM;
extern PARAM	*Param;

typedef struct _data_struct
{
	char		buf[ 8192];				/* fgets recv buffer */
	int			buf_sz;
	char		rec[ 65535];			/* data */
	int			rec_sz;
	int			line;
	int			field;
}	DATA_STRUCT;

#endif	/* MAIN_H */

/***** Module : order.c *****/
int         OrderProcess( ORDER_RECV *recv);                                /* 프로세스 종료 루틴 수행 */

/***** Module : main.c *****/
int         main( int argc, char *argv[]);                                  /* GetOption   - 인수 분석 및 변수 초기화 */
int         GetOption( int argc, char *argv[]);                             /* 프로그램 시작시 받은 인수를 분석, 프로그램에서 사용할 변수를 초기화 한다. */
void        SignalProcess( int sig_id);                                     /* 프로그램에서 사용할 시그널 */
int         InitProcess( int argc, char *argv[]);                           /* 프로그램 초기화 */
int         MainProcess( int argc, char *argv[]);                           /* 프로그램 수행 */
int         TermProcess( int argc, char *argv[]);                           /* 프로세스 종료 루틴 수행 */
int         ParamPrint( PARAM *param);                                      /* 프로그램에서 사용할 전역변수들의 구조체 값 표시 */
int         LineProcess( DATA_STRUCT *ds);                                  /* 프로그램 수행 */
int         StructProcess( DATA_STRUCT *ds);                                /* 프로그램 수행 */

