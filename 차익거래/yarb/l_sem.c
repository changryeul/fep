/** ***************************************************************************
**  @file       sem.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  ipc semaphore lock 구현
***************************************************************************** */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <time.h>

#ifndef _OMS_SOURCE_
#endif
#include "sem.h"
#include "def.h"

/** ***************************************************************************
**  @fn         int Sem_(SEM *sem, )
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
int Sem_Create(key_t key)
{
	int				rtn;
	int             created = 0, sem_id;
	union SemUnion	sem_union;

	sem_id = semget(key, 1, IPC_CREAT|IPC_EXCL|0666);
	if (sem_id >= 0)
		created = 1;
	else if (sem_id < 0)
	{
		if (errno == EEXIST) {
			sem_id = semget(key, 1, 0666);
		}
	}

	if (sem_id < 0) 
	{
		l_dbg(L_ERR, "semget error. key=[0x%08x]", key);
		return(-1);
	}

	if (created) 
	{
		sem_union.val = 1;
		rtn = semctl(sem_id, 0, SETVAL, sem_union);
		if (rtn < 0)
		{
			l_dbg(L_ERR, "semctl error.");
			return(-2);
		}
	}
	
	l_dbg(L_DBG, "semaphore created. key=[0x%08x] id=[%d]", key, sem_id);
	
	return(sem_id);
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
int Sem_RemoveByKey(int key)
{
	int				rtn;
	int				id;
	union SemUnion	arg;

	id = semget(key, 0, 0666);
	if (id < 0)
	{
		l_dbg(L_ERR, "semget error. key=[0x%08x], nsems=[%d]", key, 0);
		return -1;
	}

	arg.val = 1;
	rtn = semctl(id, 0, IPC_RMID, arg);
	if (rtn < 0)
	{
		l_dbg(L_ERR, "semem remove error. id=[%d]", id);
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
int Sem_Lock(int sem_id)
{
	int	   rtn;
	struct sembuf op;

	op.sem_num = 0;
	op.sem_op  = -1;
	op.sem_flg = SEM_UNDO;

	rtn = semop(sem_id, &op, 1);
	return(rtn);
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
int Sem_Unlock(int sem_id)
{
	int	   rtn;
	struct sembuf op;

	op.sem_num = 0;
	op.sem_op  = 1;
	op.sem_flg = SEM_UNDO;

	rtn = semop(sem_id, &op, 1);
	return(rtn);
}
