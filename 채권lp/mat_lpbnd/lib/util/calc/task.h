#ifndef TASK_H
#define	TASK_H	1

#include <stdio.h>
#include <stdlib.h>

#include "cmd.h"

typedef struct _param_
{
	char	cfg_file_path[ 512];
	char	cfg_file_name[ 512];
	int		file_cnt;
}   PARAM;

extern PARAM		Param;
extern CMD			*Cmd;
extern CMD_TBL		CmdTable[];

#endif /* TASK_H */

/***** Module : main.c *****/
int         GetOption( int argc, char *argv[]);
int         InitProcess( int argc, char *argv[]);
int         MainProcess( int argc, char *argv[]);
int         TermProcess( int argc, char *argv[]);
int         ParamPrint( PARAM *param);

