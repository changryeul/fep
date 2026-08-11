#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#ifndef _OMS_SOURCE_
#include "log.h"
#endif
#include "mem.h"

MEM *Mem_Create( key_t key, size_t sz)
{
	MEM		*mem;

	mem = ( MEM *)malloc( sizeof( MEM));
	if( mem == NULL)
	{
		LogErr( "malloc error. mem size=[%d]", sizeof( MEM));
		goto error;
	}
	memset( mem, 0x00, sizeof( MEM));

	mem->key = key;
	mem->sz = sz;

	mem->id = shmget( mem->key, mem->sz, IPC_CREAT | IPC_EXCL | 0666);
	if( mem->id < 0)
	{
		LogErr( "shmget error. key=[0x%08x], sz=[%d]", mem->key, mem->sz);
		goto error_1;
	}

	mem->ptr = shmat( mem->id, NULL, IPC_CREAT | 0666);
	if( mem->ptr == (void *)-1)
	{
		LogErr( "shmat error. id=[%d]", mem->id);
		goto error_1;
	}

	LogDel( "shard memory created. key=[0x%08x] id=[%d] sz=[%d]", key, mem->id, sz);
	return mem;

	error_1:
		free( mem);
	error:
		return NULL;
}

MEM *Mem_Open( key_t key)
{
	MEM		*mem;

	mem = ( MEM *)malloc( sizeof( MEM));
	if( mem == NULL)
	{
		LogErr( "malloc error. mem size=[%d]", sizeof( MEM));
		goto error;
	}
	memset( mem, 0x00, sizeof( MEM));

	mem->key = key;
	mem->sz = 0;

	mem->id = shmget( key, mem->sz, 0666);
	if( mem->id < 0)
	{
		LogErr( "shmget error. key=[0x%08x], sz=[%d]", mem->key, mem->sz);
		goto error_1;
	}

	mem->ptr = shmat( mem->id, NULL, 0666);
	if( mem->ptr == (void *)-1)
	{
		LogErr( "shmat error. id=[%d]", mem->id);
		goto error_1;
	}

	LogDel( "shard memory created. key=[0x%08x] id=[%d] sz=[%d]", key, mem->id, mem->sz);
	return mem;

	error_1:
		free( mem);
	error:
		return NULL;
}

int Mem_Remove( MEM *mem)
{
	int				rtn;
	struct shmid_ds	buf;

	rtn  = shmdt( mem->ptr);
	if( rtn < 0)
	{
		LogErr( "shmdt error. ptr=[%p]", mem->ptr);
		return rtn;
	}

	rtn = shmctl( mem->id, IPC_RMID, &buf);
	if( rtn < 0)
	{
		LogErr( "shmem remove error. id=[%d]", mem->id);
		return -1;
	}

	return rtn;
}

int Mem_RemoveByKey( key_t key)
{
	int				rtn;
	int				id;
	struct shmid_ds	buf;

	id = shmget( key, 0, 0666);
	if( id < 0)
	{
		LogErr( "shmget error. key=[0x%08x]", key);
		return -1;
	}

	rtn = shmctl( id, IPC_RMID, &buf);
	if( rtn < 0)
	{
		LogErr( "shmem remove error. id=[%d]", id);
		return -1;
	}

	return rtn;
}

int Mem_Close( MEM *mem)
{
	int		rtn;

	rtn = shmdt( mem->ptr);
	if( rtn < 0)
	{
		LogErr( "shmdt error. ptr=[%p]", mem->ptr);
		return -1;
	}

	free( mem);
	mem = NULL;
	
	return 1;
}

void *Mem_GetPtr( MEM *mem)
{
	return mem->ptr;
}

