/*------------------------------------------------------------------------
#   Module  : operate shared memory - create, attach, detach and remove
#   File    : shmipc.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*************************************************************************
    Function        : . create shared memory
    Parameters IN   : . p_shmkey    : shared memory key
                      . p_shmsize   : size of shared memory segment
    Parameters OUT  : .
    Return Code     : . int (shared memory identifier: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     SHM_Creat(key_t p_shmkey, size_t p_shmsize)
/*----------------------------------------------------------------------*/
{
    int     rt;

    rt = shmget(p_shmkey, p_shmsize, 0600 | IPC_CREAT);

    if (rt == -1)
        return (SHM_Creat_Excl(p_shmkey, p_shmsize));

    return (rt);
}   /* End of SHM_Creat ()  */

/*----------------------------------------------------------------------*/
int     SHM_Creat_Excl(key_t p_shmkey, size_t p_shmsize)
/*----------------------------------------------------------------------*/
{
    int     rt;

    rt = shmget(p_shmkey, p_shmsize, 0600 | IPC_CREAT | IPC_EXCL);

    return (rt);
}   /* End of SHM_Creat_Excl () */

/*************************************************************************
    Function        : . attach shared memory
    Parameters IN   : . p_shmid : shared memory identifier
    Parameters OUT  : .
    Return Code     : . char * (data segment start address of the attached
                        shared memory segment: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
char    *SHM_Attach(int p_shmid)
/*----------------------------------------------------------------------*/
{
    char    *rt;

    rt = shmat(p_shmid, (char *)0, 0);

    return (rt);
}   /* End of SHM_Attach () */

/*************************************************************************
    Function        : . attach shared memory with size verification
    Parameters IN   : . p_shmid         : shared memory identifier
                      . expected_size   : expected SHM size (0 to skip)
    Parameters OUT  : .
    Return Code     : . char * (data segment start address: success,
                        -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
char    *SHM_Attach_Verify(int p_shmid, size_t expected_size)
/*----------------------------------------------------------------------*/
{
    char            *rt;
    struct shmid_ds shm_stat;

    rt = shmat(p_shmid, (char *)0, 0);

    if (rt == (char *)-1)
        return (rt);

    if (expected_size > 0) {
        if (shmctl(p_shmid, IPC_STAT, &shm_stat) == 0) {
            if (shm_stat.shm_segsz < expected_size) {
                Log(SAM_WARN,
                        "SHM size mismatch: actual=%d expected=%d",
                        (int)shm_stat.shm_segsz, (int)expected_size);
            }
        }
    }

    return (rt);
}   /* End of SHM_Attach_Verify ()  */

/*************************************************************************
    Function        : . detach shared memory
    Parameters IN   : . p_shmptr    : shared memory segment start address
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    SHM_Detach(char *p_shmptr)
/*----------------------------------------------------------------------*/
{
    if (p_shmptr != NULL)
        shmdt(p_shmptr);

    return;
}   /* End of SHM_Detach () */

/*************************************************************************
    Function        : . remove shared memory identifier
    Parameters IN   : . p_shmid : shared memory identifier
    Parameters OUT  : .
    Return Code     : . int (0: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     SHM_Remove(int p_shmid)
/*----------------------------------------------------------------------*/
{
    int     rt;

    rt = shmctl(p_shmid, IPC_RMID, (struct shmid_ds *)0);

    return (rt);
}   /* End of SHM_Remove () */

/*************************************************************************
    End of Program (shmipc.c)
*************************************************************************/
