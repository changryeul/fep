#ifndef TASK_H
#define	TASK_H	1

#include <stdio.h>
#include <stdlib.h>

#include "cmd.h"

typedef struct _param_
{
	char	cfg_file_name[ 512];
}   PARAM;

extern PARAM	Param;

#endif /* TASK_H */

