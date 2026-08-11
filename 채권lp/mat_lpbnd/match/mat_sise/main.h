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
#include "sise.h"

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
	char	cfg_name[ 512];				/** cfg file name */
	char	log_name[ 512];				/** log file name */
	int		log_flag;					/** 0-console 1-file 2-mem */
	int		log_level;					/** log_level 10-debug, 100-error */
	char	addr[ 32];					/** sise server addr */
	int		port;						/** sise server port */
	int		interval;					/** main loop interval */
	int		timeout;					/** wait timeout */
	char	filter[ 32];				/** sise filter */
	char	file_name[ 512];			/** send sise data file */
	int		send_flag;					/** for test - read file */
	int		loop_flag;					/** for test - 반복 */
}	PARAM;
extern PARAM	*Param;

typedef struct _my_data_
{
	time_t		cur_time;
	int			alarm;
}	MY_DATA;
extern MY_DATA	*MyData;

#endif	/* MAIN_H */

/***** Module : print.c *****/
int         ORDER_RECV_Print( ORDER_RECV* ptr);
int         ORDER_SEND_Print( ORDER_SEND* ptr);
int         ORDER_Print( ORDER* ptr);

/***** Module : main.c *****/
int         main( int argc, char *argv[]);                                  /* GetOption   - 인수 분석 및 변수 초기화 */
int         GetOption( int argc, char *argv[]);                             /* 프로그램 시작시 받은 인수를 분석, 프로그램에서 사용할 변수를 초기화 한다. */
void        SignalProcess( int sig_id);                                     /* 프로그램에서 사용할 시그널 */
int         InitProcess( int argc, char *argv[]);                           /* 프로그램 초기화 */
int         MainProcess( int argc, char *argv[]);                           /* 프로그램 수행 */
int         RecvProcess( int argc, char *argv[]);                           /* 프로그램 수행 */
int         SendProcess( int argc, char *argv[]);                           /* 프로그램 수행 - sise.dat 파일을 읽어 전송 */
int         TermProcess( int argc, char *argv[]);                           /* 프로세스 종료 루틴 수행 */
int         ParamPrint( PARAM *param);                                      /* 프로그램에서 사용할 전역변수들의 구조체 값 표시 */
int         ReadProcess( APSISE *sise);                                     /* 프로그램에서 사용할 전역변수들의 구조체 값 표시 */

