#ifndef TASK_H
#define	TASK_H	1

#include <stdio.h>
#include <stdlib.h>

#include "cmd.h"
#include "cfg.h"
#include "etc.h"
#include "udp.h"

typedef struct _param_
{
	char	cfg_name[ 512];
	char	log_name[ 512];		/* log file name - output file name */
	char	bat_name[ 512];		/* batch command file */
	char	addr[ 32];
	int		port;
}   PARAM;

typedef struct _client_
{
	UDP		*udp;
	char	recv[ 8192];
	char	send[ 8192];
	int		r_sz;
	int		s_sz;
	int		stat;
	int		seq;
}	CLIENT;

extern PARAM		*Param;
extern CMD			*Cmd;
extern CMD_TBL		CmdTable[];

extern CFG			*Cfg;

#endif /* TASK_H */

/***** Module : main.c *****/
int         main( int argc, char *argv[]);
int         GetOption( int argc, char *argv[]);
int         InitProcess( int argc, char *argv[]);
int         CommandProcess( int argc, char *argv[]);
int         BatchProcess( int argc, char *argv[]);
int         TermProcess( int argc, char *argv[]);
int         ParamPrint( PARAM *param);

