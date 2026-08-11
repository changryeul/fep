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

