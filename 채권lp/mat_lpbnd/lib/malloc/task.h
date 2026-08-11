#ifndef	_TASK_H_
#define	_TASK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "cmd.h"
#include "cfg.h"

typedef struct _param_
{
	int		argc;
	char	**argv;
	char	**envp;
	char	cfg_name[ 512];
}	PARAM;

extern CMD		*Cmd;
extern CMD_TBL	CmdTable[];

#endif 	/* _TASK_H_ */

/***** Module : task.c *****/
int         CmdOpen( int argc, char *argv[]);
int         CmdHelp( int argc, char *argv[]);
int         CmdQuit( int argc, char *argv[]);

