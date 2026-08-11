/***** Module : psem.c *****/
PSEM*       Psem_Create( char *name);
PSEM*       Psem_Open( char *name);
int         Psem_Remove( PSEM *sem);
int         Psem_RemoveName( char *name);
int         Psem_Close( PSEM *sem);
sem_t*      Psem_GetPtr( PSEM *sem);
int         Psem_Lock( PSEM *sem);
int         Psem_Unlock( PSEM *sem);

