#ifndef PSEM_H
#define	PSEM_H	1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <semaphore.h>

typedef struct _psem_
{
	char		name[ 512];
	sem_t		*sem;
}   PSEM;

#endif /* PSEM_H */

#include "psem_func.h"
