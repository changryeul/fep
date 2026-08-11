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
int        Sem_Create(key_t key);                        
int        Sem_RemoveByKey(int key);                        
int        Sem_Lock(int sem_id);                           
int        Sem_Unlock(int sem_id);                         

