#ifndef SEM_H
#define	SEM_H	1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union SemUnion
{
	int 				val;
	struct semid_ds		*buf;
	unsigned short int	*array;
	struct seminfo		*info;
};

typedef struct _sem_
{
	key_t		key;
	int			id;
	int			nsems;
	int			lock_cnt;
	struct sembuf		*lock;
	struct sembuf		*unlock;
}   SEM;

#endif /* SEM_H */

/***** Module : sem.c *****/
SEM*        Sem_Create( key_t key);                                         /* semaphore create */
SEM*        Sem_Open( key_t key);                                           /* semaphore create */
int         Sem_Remove( SEM *sem);                                          /* semaphore create */
int         Sem_RemoveByKey( int key);                                      /* semaphore create */
int         Sem_Close( SEM *sem);                                           /* semaphore create */
int         Sem_GetId( SEM *sem);                                           /* semaphore create */
int         Sem_Lock( SEM *sem);                                            /* semaphore create */
int         Sem_LockT( SEM *sem, int timeout);                              /* semaphore lock - timeout(micro second) */
int         Sem_Unlock( SEM *sem);                                          /* semaphore create */

/***** Module : semlog.c *****/
SEM*        SemLog_Create( key_t key);                                      /* semaphore create */
SEM*        SemLog_Open( key_t key);                                        /* semaphore create */
int         SemLog_Remove( SEM *sem);                                       /* semaphore create */
int         SemLog_RemoveByKey( int key);                                   /* semaphore create */
int         SemLog_Close( SEM *sem);                                        /* semaphore create */
int         SemLog_GetId( SEM *sem);                                        /* semaphore create */
int         SemLog_Lock( SEM *sem);                                         /* semaphore create */
int         SemLog_LockT( SEM *sem, int timeout);                           /* semaphore lock - timeout(micro second) */
int         SemLog_Unlock( SEM *sem);                                       /* semaphore create */

