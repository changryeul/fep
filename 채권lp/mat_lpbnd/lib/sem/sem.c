/** ***************************************************************************
**  @file       sem.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  ipc semaphore lock 구현
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#ifndef _OMS_SOURCE_
#include "log.h"
#endif
#include "sem.h"

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
SEM *Sem_Create( key_t key)
{
	int				rtn;
	SEM				*sem;
	union SemUnion	sem_union;

	sem = ( SEM *)malloc( sizeof( SEM));
	if( sem == NULL)
	{
		LogErr( "malloc error. sem size=[%d]", sizeof( SEM));
		goto error_3;
	}
	memset( sem, 0x00, sizeof( SEM));

	sem->lock = malloc( sizeof( struct sembuf));
	if( sem->lock == NULL)
	{
		LogErr( "malloc error. sem->lock size=[%d]", sizeof( struct sembuf));
		goto error_2;
	}
	sem->lock->sem_num = 0;
	sem->lock->sem_op = -1;
	sem->lock->sem_flg = SEM_UNDO;

	sem->unlock = malloc( sizeof( struct sembuf));
	if( sem->lock == NULL)
	{
		LogErr( "malloc error. sem->lock size=[%d]", sizeof( struct sembuf));
		goto error_1;
	}
	sem->unlock->sem_num = 0;
	sem->unlock->sem_op = 1;
	sem->unlock->sem_flg = SEM_UNDO;

	sem->key = key;
	sem->nsems = 1;
	sem->lock_cnt = 0;

	sem->id = semget( sem->key, sem->nsems, IPC_CREAT | IPC_EXCL | 0666);
	if( sem->id < 0)
	{
		LogErr( "semget error. key=[0x%08x]", sem->key);
		goto error;
	}

	sem_union.val = 1;
	rtn = semctl( sem->id, 0, SETVAL, sem_union);
	if( rtn < 0)
	{
		LogErr( "semctl error.");
		goto error;
	}

	LogDel( "semaphore created. key=[0x%08x] id=[%d] nsems=[%d]", key, sem->id, sem->nsems);
	return sem;

	error:
		free( sem->unlock);
	error_1:
		free( sem->lock);
	error_2:
		free( sem);
	error_3:
		return NULL;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
SEM *Sem_Open( key_t key)
{
	SEM		*sem;

	sem = ( SEM *)malloc( sizeof( SEM));
	if( sem == NULL)
	{
		LogErr( "malloc error. sem size=[%d]", sizeof( SEM));
		goto error_3;
	}
	memset( sem, 0x00, sizeof( SEM));

	sem->lock = malloc( sizeof( struct sembuf));
	if( sem->lock == NULL)
	{
		LogErr( "malloc error. sem->lock size=[%d]", sizeof( struct sembuf));
		goto error_2;
	}
	sem->lock->sem_num = 0;
	sem->lock->sem_op = -1;
	sem->lock->sem_flg = SEM_UNDO;

	sem->unlock = malloc( sizeof( struct sembuf));
	if( sem->unlock == NULL)
	{
		LogErr( "malloc error. sem->lock size=[%d]", sizeof( struct sembuf));
		goto error_1;
	}
	sem->unlock->sem_num = 0;
	sem->unlock->sem_op = 1;
	sem->unlock->sem_flg = SEM_UNDO;

	sem->key = key;
	sem->nsems = 0;
	sem->lock_cnt = 0;

	sem->id = semget( key, sem->nsems, 0666);
	if( sem->id < 0)
	{
		LogErr( "semget error. key=[0x%08x], nsems=[%d]", sem->key, sem->nsems);
		goto error;
	}

	LogDel( "semaphore opened. key=[0x%08x] id=[%d] nsems=[%d]", key, sem->id, sem->nsems);
	return sem;

	error:
		free( sem->lock);
	error_1:
		free( sem->unlock);
	error_2:
		free( sem);
	error_3:
		return NULL;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
int Sem_Remove( SEM *sem)
{
	int				rtn;
	union SemUnion	arg;

	arg.val = 1;
	rtn = semctl( sem->id, 0, IPC_RMID, arg);
	if( rtn < 0)
	{
		LogErr( "semem remove error. id=[%d]", sem->id);
		return -1;
	}

	free( sem->lock);
	free( sem->unlock);
	free( sem);

	return rtn;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
int Sem_RemoveByKey( int key)
{
	int				rtn;
	int				id;
	union SemUnion	arg;

	id = semget( key, 0, 0666);
	if( id < 0)
	{
		LogErr( "semget error. key=[0x%08x], nsems=[%d]", key, 0);
		return -1;
	}

	arg.val = 1;
	rtn = semctl( id, 0, IPC_RMID, arg);
	if( rtn < 0)
	{
		LogErr( "semem remove error. id=[%d]", id);
		return -1;
	}

	return rtn;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
int Sem_Close( SEM *sem)
{
	free( sem->lock);
	free( sem->unlock);
	free( sem);
	
	return 1;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
int Sem_GetId( SEM *sem)
{
	return sem->id;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
int Sem_Lock( SEM *sem)
{
	int		rtn;

	LogDel( "id=[%d] sem->loc_cnt=[%d]", sem->id, sem->lock_cnt);
	if( sem->lock_cnt > 0) 
	{
		/* LogWar( "already locked. sem->lock_cnt=[%d]", sem->lock_cnt); */
		return 0;
	}

	rtn = semop( sem->id, sem->lock, 1);
	sem->lock_cnt++;

	return rtn;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore lock - timeout(micro second)
***************************************************************************** */
int Sem_LockT( SEM *sem, int timeout)
{
	int				rtn;
	struct timespec	ts;

	LogDel( "id=[%d] sem->loc_cnt=[%d]", sem->id, sem->lock_cnt);

	if( sem->lock_cnt > 0) 
	{
		LogWar( "already locked. sem->id=[%d] sem->lock_cnt=[%d]", sem->id, sem->lock_cnt);
		usleep( timeout);
		return -1;
	}

	/* clock_gettime( CLOCK_REALTIME, &ts); */
	ts.tv_sec  = timeout / 1000000;
	ts.tv_nsec = ( timeout % 1000000) * 1000;

	rtn = semtimedop( sem->id, sem->lock, 1, &ts);
	if( rtn < 0)
	{
		LogErr( "semtimedop error. id=[%d] timeout=[%d.%d]", sem->id, ts.tv_sec, ts.tv_nsec);
		return rtn;
	}
	sem->lock_cnt++;

	return 1;
}

/** ***************************************************************************
**  @fn         int Sem_( SEM *sem, )
**  @param      SEM *sem     - semaphore pointer
**  @param      
**  @return     프로그램 수행 결과
**  @retval     실패 - -
**  @retval     성공 - +
**  @exception
**  @remark
**  @brief
**	semaphore create
***************************************************************************** */
int Sem_Unlock( SEM *sem)
{
	int		rtn;

	LogDel( "id=[%d] sem->loc_cnt=[%d]", sem->id, sem->lock_cnt);
	if( sem->lock_cnt == 0)
	{
		LogCri( "no locked action. sem->lock_cnt=[%d]", sem->lock_cnt);
		return -1;
	}

	rtn = semop( sem->id, sem->unlock, 1);
	if( rtn < 0)
	{
		LogErr( "semop error. id=[%d]", sem->id);
		return rtn;
	}

	sem->lock_cnt--;
	return 1;
}





