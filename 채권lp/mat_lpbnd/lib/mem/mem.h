#ifndef MEM_H
#define	MEM_H	1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>

typedef struct _mem_
{
	key_t		key;
	int			id;
	size_t		sz;
	void		*ptr;
}   MEM;

#endif /* MEM_H */

/***** Module : mem.c *****/
MEM*        Mem_Create( key_t key, size_t sz);
MEM*        Mem_Open( key_t key);
int         Mem_Remove( MEM *mem);
int         Mem_RemoveByKey( key_t key);
int         Mem_Close( MEM *mem);
void*       Mem_GetPtr( MEM *mem);

/***** Module : memlog.c *****/
MEM*        MemLog_Create( key_t key, size_t sz);
MEM*        MemLog_Open( key_t key);
int         MemLog_Remove( MEM *mem);
int         MemLog_RemoveByKey( key_t key);
int         MemLog_Close( MEM *mem);
void*       MemLog_GetPtr( MEM *mem);

