#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <semaphore.h>

#include "log.h"
#include "psem.h"

PSEM *Psem_Create( char *name)
{
	PSEM				*sem;

	sem = ( PSEM *)malloc( sizeof( PSEM));
	if( sem == NULL)
	{
		LogErr( "malloc error. sem size=[%d]", sizeof( PSEM));
		goto error;
	}
	memset( sem, 0x00, sizeof( PSEM));

	sprintf( sem->name, "/%s", name);

	sem->sem = sem_open( sem->name, O_CREAT | O_EXCL, 0666, 1);
	if( sem->sem == SEM_FAILED)
	{
		LogErr( "sem_open error. name=[%s]", sem->name);
		goto error_1;
	}

	LogDel( "semaphore created. name=[%s]", sem->name);
	return sem;

	error_1:
		free( sem);
	error:
		return NULL;
}

PSEM *Psem_Open( char *name)
{
	PSEM		*sem;

	sem = ( PSEM *)malloc( sizeof( PSEM));
	if( sem == NULL)
	{
		LogErr( "malloc error. sem size=[%d]", sizeof( PSEM));
		goto error;
	}
	memset( sem, 0x00, sizeof( PSEM));

	sprintf( sem->name, "/%s", name);

	sem->sem = sem_open( sem->name, O_RDWR);
	if( sem->sem == SEM_FAILED)
	{
		LogErr( "sem_open error. name=[%s]", sem->name);
		goto error_1;
	}

	return sem;

	error_1:
		free( sem);
	error:
		return NULL;
}

int Psem_Remove( PSEM *sem)
{
	int				rtn;

	if( sem->sem != NULL) sem_close( sem->sem);
	rtn = sem_unlink( sem->name);
	free( sem);

	return rtn;
}

int Psem_RemoveName( char *name)
{
	int				rtn;

	rtn = sem_unlink( name);

	return rtn;
}

int Psem_Close( PSEM *sem)
{
	int		rtn;

	if( sem->sem != NULL) sem_close( sem->sem);
	return 1;
}

sem_t *Psem_GetPtr( PSEM *sem)
{
	return sem->sem;
}

int Psem_Lock( PSEM *sem)
{
	int		rtn = -1;
	int		sval;
	struct timespec	ts;

	sem_getvalue( sem->sem, &sval);
	LogDbg( "sem_wait ... try     getvalue=[%d]", sval);

#if 1
	while( rtn < 0)
	{
		clock_gettime( CLOCK_REALTIME, &ts);
		ts.tv_sec += 1;
		ts.tv_nsec += 0;
		rtn = sem_timedwait( sem->sem, &ts);
		if( rtn < 0)
		{
			LogErr( "sem_wait error. name=[%s] sem=[%p]", sem->name, sem->sem);
		}
		sem_getvalue( sem->sem, &sval);
		LogDbg( "rtn=[%d] getvalue=[%d]", rtn, sval);
	}
#else
	rtn = sem_wait( sem->sem);
	if( rtn < 0)
	{
		LogErr( "sem_wait error. name=[%s] sem=[%p]", sem->name, sem->sem);
	}
#endif

	sem_getvalue( sem->sem, &sval);
	LogDbg( "sem_wait ... success getvalue=[%d]", sval);
	return rtn;
}

int Psem_Unlock( PSEM *sem)
{
	int		rtn;
	int		sval;

	sem_getvalue( sem->sem, &sval);
	LogDbg( "sem_post ... try     getvalue=[%d]", sval);

	rtn = sem_post( sem->sem);
	if( rtn < 0)
	{
		LogErr( "sem_post error.");
	}

	sem_getvalue( sem->sem, &sval);
	LogDbg( "sem_post ... success  getvalue=[%d]", sval);

	return rtn;
}

