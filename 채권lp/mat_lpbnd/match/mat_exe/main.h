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
	char	cfg_name[ 512];				/** config file name */
	char	log_name[ 512];				/** log file name */
	int		log_flag;					/** 0-console, 1-file, 2-mem */
	int		log_level;					/** Dev=0,Trc=1.Tst=5,Dbg=10,App=20,Msg=30,Lib=40,War=50 */
	char	sq_name[ 32];				/** send queue name */
	char	rq_name[ 32];				/** recv queue name */
	int		interval;					/** loop interval */
	int		timeout;					/** recv timeout */
}	PARAM;
extern PARAM	*Param;

#endif	/* MAIN_H */

/***** Module : proc_swap.c *****/
int         main( int argc, char *argv[]);
int         GetOption( int argc, char *argv[]);
void        SignalProcess( int sig_id);
int         InitProcess( int argc, char *argv[]);
int         MainProcess( int argc, char *argv[]);
int         TermProcess( int argc, char *argv[]);
int         ParamPrint( PARAM *param);

/***** Module : proc.c *****/
int         Proc_Execute();                                                 /* 주문 정합 check */
int         Proc_OrderPreCheck( ORDER *order);                              /* 주문 사전 check */
int         Proc_OrderCheck( ORDER *order);                                 /* 주문 check */

/***** Module : import *****/
int			l_dbconnect();
int			l_dbdisconnect();
int			Db_TRG005LInsert( MAT_RECORD *rec);

/***** Module : mat_execute.c *****/
int         Mat_Execute( MAT *mat, int timeout);                            /* 체결처리 - mat_exe main */
int         Mat_ReadPipe( MAT *mat, int timeout);                           /* pipe로 부터 체결 record position을 수신 */

