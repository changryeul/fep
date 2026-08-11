/*------------------------------------------------------------------------
#	Module	: create semaphore
#	File	: semipc.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
int		SEM_Creat (key_t);
int		SEM_Creat_Excl (key_t);
int		SEM_Lock (int);
int		SEM_UnLock (int);
int		SEM_Remove (int);

/*************************************************************************
	Function		: . create semaphore
	Parameters IN	: . p_semkey	: semaphore key
	Parameters OUT	: .
	Return Code		: . int (semaphore ID: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		SEM_Creat (key_t p_semkey)
/*----------------------------------------------------------------------*/
{
	int     rt;

	rt = semget (p_semkey, 1, 0);

	return (rt);
}	/* End of SEM_Creat ()	*/

/*----------------------------------------------------------------------*/
int		SEM_Creat_Excl (key_t p_semkey)
/*----------------------------------------------------------------------*/
{
	int		rt, semid;
	union	semun   
	{
		int				val;
		struct semid_ds	*buff;
		ushort			*array;
	}	arg;

	rt = semget (p_semkey, 1, 0666|IPC_CREAT|IPC_EXCL);

	if (rt == -1) 
		return (-1);

	semid = rt;
	arg.val = 1;

	rt = semctl (semid, 0, SETVAL, arg);

	if (rt == -1)
		return (-1);

	return (semid);
}	/* End of SEM_Creat_Excl ()	*/

/*************************************************************************
	Function		: . lock semaphore operation
	Parameters IN	: . p_semid	: semaphore ID
	Parameters OUT	: .
	Return Code		: . int (0: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		SEM_Lock (int p_semid)
/*----------------------------------------------------------------------*/
{
	int				rt;
	struct sembuf	lock = { 0, -1, SEM_UNDO };

	rt = semop (p_semid, &lock, 1);

	return (rt);
}	/* End of SEM_Lock ()	*/

/*************************************************************************
	Function		: . unlock semaphore operation
	Parameters IN	: . p_semid	: semaphore ID
	Parameters OUT	: .
	Return Code		: . int (0: success, 1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		SEM_UnLock (int p_semid)
/*----------------------------------------------------------------------*/
{
	int				rt;
	struct sembuf	unlock = { 0, 1, SEM_UNDO };

	rt = semop (p_semid, &unlock, 1);

	return (rt);
}	/* End of SEM_UnLock ()	*/

/*************************************************************************
	Function		: . remove semaphore control operation
	Parameters IN	: . p_semid	: semaphore ID
	Parameters OUT	: .
	Return Code		: . int (0: success, 1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		SEM_Remove (int p_semid)
/*----------------------------------------------------------------------*/
{
	int     rt;

	rt = semctl (p_semid, 0, IPC_RMID, 0);

	return (rt);
}	/* End of SEM_Remove ()	*/

/*************************************************************************
	End of Program (semipc.c)
*************************************************************************/
