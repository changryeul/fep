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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "log.h"
#include "cfg.h"
#include "etc.h"
#include "tcp.h"

#ifndef MAIN_H
#define	MAIN_H	1

#define	MAX_CLIENT		1

typedef struct _param_
{
	int		argc;						/** @var command line argument count */
	char	**argv;						/** @var command line argument */
	char	argo[ 32];					/** @var argument option buffer */
	int		args;						/** @var argument option size */
	char 	cfg_name[ 512];				/* config file name */
	char	log_name[ 512];				/* log file name */
	char	addr[ 32];					/* bind address */
	int		port;						/* wait port */
	int		interval;					/* loop interval */
}	PARAM;

#endif	/* MAIN_H */

extern TCP		*Server;
extern TCP		*Client[ MAX_CLIENT];
extern CFG		*Cfg;
extern int		Continue;
extern PARAM	*Param;

/***** Module : main.c *****/
int         main( int argc, char *argv[]);
int         GetOption( int argc, char *argv[]);
void        SignalProcess( int sig_id);
int         InitProcess( int argc, char *argv[]);
int         MainProcess( int argc, char *argv[]);
int         TermProcess( int argc, char *argv[]);
int         ParamPrint( PARAM *param);
int         ServerProcess( char *addr, int port);
int         DataProcess( TCP *tcp);

