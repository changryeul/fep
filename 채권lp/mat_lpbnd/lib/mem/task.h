#ifndef TASK_H
#define	TASK_H	1

#include <stdio.h>
#include <stdlib.h>

#include "cmd.h"

typedef struct _param_
{
	char	cfg_name[ 512];
	int		file_cnt;
}   PARAM;

extern PARAM		Param;
extern CMD			*Cmd;
extern CMD_TBL		CmdTable[];

#endif /* TASK_H */

