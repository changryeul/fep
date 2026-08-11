/*------------------------------------------------------------------------
#	Module	: operate shared memory - create, attach, detach and remove
#	File	: shmipc.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
int		SHM_Creat (key_t, size_t);
int		SHM_Creat_Excl (key_t, size_t);
char	*SHM_Attach (int);
void	SHM_Detach (char *);
int		SHM_Remove (int);

/*************************************************************************
	Function		: . create shared memory
	Parameters IN	: . p_shmkey	: shared memory key
					  . p_shmsize	: size of shared memory segment
	Parameters OUT	: .
	Return Code		: . int (shared memory identifier: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		SHM_Creat (key_t p_shmkey, size_t p_shmsize)
/*----------------------------------------------------------------------*/
{
	int     rt;

	rt = shmget (p_shmkey, p_shmsize, 0664 | IPC_CREAT);

	if (rt == -1)
		return (SHM_Creat_Excl (p_shmkey, p_shmsize));

	return (rt);
}	/* End of SHM_Creat ()	*/

/*----------------------------------------------------------------------*/
int		SHM_Creat_Excl (key_t p_shmkey, size_t p_shmsize)
/*----------------------------------------------------------------------*/
{
	int     rt;

	rt = shmget (p_shmkey, p_shmsize, 0664 | IPC_CREAT | IPC_EXCL);

	return (rt);
}	/* End of SHM_Creat_Excl ()	*/

/*************************************************************************
	Function		: . attach shared memory
	Parameters IN	: . p_shmid	: shared memory identifier
	Parameters OUT	: .
	Return Code		: . char * (data segment start address of the attached
						shared memory segment: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
char	*SHM_Attach (int p_shmid)
/*----------------------------------------------------------------------*/
{
	char    *rt;

	rt = shmat (p_shmid, (char *)0, 0);

	return (rt);
}	/* End of SHM_Attach ()	*/

/*************************************************************************
	Function		: . detach shared memory
	Parameters IN	: . p_shmptr	: shared memory segment start address
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	SHM_Detach (char *p_shmptr)
/*----------------------------------------------------------------------*/
{
	if (p_shmptr != NULL)
		shmdt (p_shmptr);
	
	return;
}	/* End of SHM_Detach ()	*/

/*************************************************************************
	Function		: . remove shared memory identifier
	Parameters IN	: . p_shmid	: shared memory identifier
	Parameters OUT	: .
	Return Code		: . int (0: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		SHM_Remove (int p_shmid)
/*----------------------------------------------------------------------*/
{
	int		rt;

	rt = shmctl (p_shmid, IPC_RMID, (struct shmid_ds *)0);
	
	return (rt);
}	/* End of SHM_Remove ()	*/

/*************************************************************************
	End of Program (shmipc.c)
*************************************************************************/
