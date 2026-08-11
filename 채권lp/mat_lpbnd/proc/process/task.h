#ifndef TASK_H
#define	TASK_H	1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "cfg.h"
#include "cmd.h"
#include "proc.h"
#include "etc.h"
#include "map.h"

typedef struct _param_
{
	int		argc;
	char	**argv;
	char	argo[ 32];
	int		args;
	char	cfg_name[ 512];
	char	log_name[ 512];
	char	map_name[ 512];
	char	holiday_file[ 512];
	int		file_cnt;
	int		mon_interval;
}   PARAM;

extern PARAM		*Param;
extern CMD			*Cmd;
extern CMD_TBL		CmdTable[];

#endif /* TASK_H */

/***** Module : main.c *****/
int         GetOption( int argc, char *argv[]);
int         InitProcess( int argc, char *argv[]);
int         MainProcess( int argc, char *argv[]);
int         TermProcess( int argc, char *argv[]);
int         ParamPrint( PARAM *param);

/***** Module : task.c *****/
int         TaskInit();
int         TaskStat( int argc, char *argv[]);
int         TaskSet( int argc, char *argv[]);
int         TaskMon( int argc, char *argv[]);
int         TaskStop( int argc, char *argv[]);
int         TaskRun( int argc, char *argv[]);
int         TaskDaemon( int argc, char *argv[]);
int         TaskHelp( int argc, char *argv[]);
int         TaskQuit( int argc, char *argv[]);

/***** Module : mon.c *****/
int         Mon( int argc, char *argv[]);
int         MonMain();
int         MonProcessStat( MAP *map, PROC *proc, char *name);
int         Map_AlarmStat( MAP *map, MAP_FIELD *field);
int         Map_StatCheck( MAP *map, MAP_FIELD *field);
int         Map_TimeProc( MAP *map, MAP_FIELD *field);
int         Map_TimeField( MAP *map, MAP_FIELD *field);
int         Map_TimeConvert( MAP *map, MAP_FIELD *field);
int         MonInit( MAP *map, PROC *proc);
int         AlarmProcess( MAP *map, char *format, ...);

