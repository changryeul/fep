#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#ifndef	PROC_DAEMON_H
#define	PROC_DAEMON_H	1

#include "cfg.h"
#include "etc.h"
#include "proc.h"

extern CFG		*Cfg;
extern PROC		*Proc;

typedef struct _param_
{
	int			argc;						/* command line argument count */
	char		**argv;						/* command line argument */
	char		**envp;						/* daemon envp */
	char		argo[ 32];					/* argument option buffer */
	int			args;						/* argument option size */
	char		cfg_file_name[ 512];		/* config file name */
	char		log_file_name[ 512];		/* log file name */
	char		swp_file_name[ 512];		/* swp file name */
	char		holiday_file[ 512];			/* swp file name */
	char		curr_work_dir[ 512];		/* current work directory */
	char		stdout_file[ 512];			/* child process stdout/stderror */
	char		ora_connect[ 32];			/* oracle connect env name */
	char		svc_name[ 32];				/* oracle db key */
	int			daemon_mode;				/* daemon mode flag */
	int			force;						/* 이미 pid가 등록되어 있어도 실행 */
	int			swap_load;					/* swap file load flag */
	int			config_load;				/* config file load flag - load_flag 값에 관계없이 로드 */
	int			daily_job;					/* daily job flag - SIGUSR1 */
	char		daily_name[ 32];			/* daily job name - daily clear 작업으로 등록된 id */
	int			check_flag;					/* process check flag - SIGUSR2 */
	int			interval;					/* sleep interval */
}	PARAM;
extern PARAM *Param;

#endif /* PROC_DAEMON_H */

/***** Module : fep_daemon.c *****/
int         main( int argc, char *argv[], char *envp[]);
int         GetOption( int argc, char *argv[], char *envp[]);
void        SignalProcess( int sig_id);
int         InitProcess( int argc, char *argv[], char *envp[]);
int         MainProcess( int argc, char *argv[]);
int         TermProcess( int argc, char *argv[]);
int         LoaderRun( char *cmd);
int         MakeDaemon();
int			ParamPrint( PARAM *Param);

