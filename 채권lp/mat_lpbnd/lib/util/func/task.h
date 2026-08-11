#ifndef TASK_H
#define	TASK_H	1

#include <stdio.h>
#include <stdlib.h>

#include "cmd.h"

typedef struct _param_
{
	char	out_file[ 512];
	char	**file_list;
	int		file_cnt;
}   PARAM;

extern PARAM	Param;

#endif /* TASK_H */

/***** Module : main.c *****/
int         GetOption( int argc, char *argv[]);
int         InitProcess( int argc, char *argv[]);
int         MainProcess( int argc, char *argv[]);
int         TermProcess( int argc, char *argv[]);
int         ParamPrint( PARAM *param);

