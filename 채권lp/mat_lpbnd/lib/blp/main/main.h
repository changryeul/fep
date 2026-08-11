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

#include "shm_memory.h"

#include "log.h"
#include "cfg.h"
#include "etc.h"
#include "map.h"
#include "blp.h"

#ifndef MAIN_H
#define	MAIN_H	1

extern CFG		*Cfg;
extern MAP		*Map;
extern BLP		*Blp;
extern int		Continue;
extern SHM_NOTE	*Shm_Note;

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
	key_t	key;						/* ipc key */
	key_t	oms_key;					/* OMS SHM_NOTE key */
	char	dat_file[ 512];				/* krx sise file name */
	char	map_name[ 512];				/* for monitor */
}	PARAM;
extern PARAM	*Param;

typedef struct _my_data_
{
	time_t		cur_time;
	int			alarm;
}	MY_DATA;
extern MY_DATA	*MyData;

typedef struct _data_struct
{
    char        buf[ 8192];             /* fgets recv buffer */
    int         buf_sz;
    char        rec[ 65535];            /* data */
    int         rec_sz;
    int         line;
    int         field;
}   DATA_STRUCT;


#endif	/* MAIN_H */

/***** Module : main.c *****/
int         main( int argc, char *argv[]);                                  /* GetOption   - 인수 분석 및 변수 초기화 */
int         GetOption( int argc, char *argv[]);                             /* 프로그램 시작시 받은 인수를 분석, 프로그램에서 사용할 변수를 초기화 한다. */
void        SignalProcess( int sig_id);                                     /* 프로그램에서 사용할 시그널 */
int         InitProcess( int argc, char *argv[]);                           /* 프로그램 초기화 */
int         MainProcess( int argc, char *argv[]);                           /* 프로그램 수행 */
int         TermProcess( int argc, char *argv[]);                           /* 프로세스 종료 루틴 수행 */
int         ParamPrint( PARAM *param);                                      /* 프로그램에서 사용할 전역변수들의 구조체 값 표시 */
int         FileProcess( char *file_name, int file_type, char *buffer);     /* 프로그램 수행 */
int         LineProcess( DATA_STRUCT *ds);                                  /* 프로그램 수행 */
int         StructProcess( DATA_STRUCT *ds);                                /* 프로그램 수행 */

