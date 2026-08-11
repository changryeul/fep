/***** Module : sem.c *****/
SEM*        Sem_Create( char *name);
SEM*        Sem_Open( char *name);
int         Sem_Remove( SEM *sem);
int         Sem_Close( SEM *sem);
sem_t*      Sem_GetPtr( SEM *sem);
int         Sem_Lock( SEM *sem);
int         Sem_Unlock( SEM *sem);

