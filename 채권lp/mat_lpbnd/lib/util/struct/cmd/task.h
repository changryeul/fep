#ifndef TASK_H
#define	TASK_H	1

#include <stdio.h>
#include <stdlib.h>

#include "cmd.h"

typedef struct _param_
{
	char	**file_list;
	int		file_cnt;
}   PARAM;

extern PARAM	Param;

#endif /* TASK_H */

