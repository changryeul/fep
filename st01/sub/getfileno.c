/*------------------------------------------------------------------------
#   Module  : out file search
#   File    : getfileno.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
int     GETFILENO(int, int);
int     GETFILENO_NOLOCK(int, int);

/*************************************************************************
    Function        : . out file search
    Parameters IN   : . p_out   : DELAY_TIME(out dfile count)
                      . offset  : process�� offset value
    Parameters OUT  : . ó������ process ����.
    Return Code     : . int
                        > 0 : success (write process)
                        -1  : failure
*************************************************************************/
/*----------------------------------------------------------------------*/
int     GETFILENO(int p_out, int offset)
/*----------------------------------------------------------------------*/
{
    int         s_k, i, rt, chk_loop, j;
    union   semun {
        int             val;
        struct semid_ds *buff;
        ushort          *array;
    }   arg;

    s_k = p_out - 1;
    if (offset >= s_k)
        chk_loop = 0;
    else
        chk_loop = offset;

    if (SemId[s_k] != -1)
        SEM_Lock(SemId[s_k]);

    if (SemId[s_k] != -1)
        SEM_UnLock(SemId[s_k]);

    return (chk_loop);
}   /* End of GETFILENO () */

/*************************************************************************
    Function        : . out file search
    Parameters IN   : . p_out   : DELAY_TIME(out dfile count)
                      . offset  : process�� offset value
    Parameters OUT  : . ó������ process ����.
    Return Code     : . int
                        > 0 : success (write process)
                        -1  : failure
*************************************************************************/
/*----------------------------------------------------------------------*/
int     GETFILENO_NOLOCK(int p_out, int offset)
/*----------------------------------------------------------------------*/
{
    int         s_k, i, rt, chk_loop, j;
    union   semun {
        int             val;
        struct semid_ds *buff;
        ushort          *array;
    }   arg;

    s_k = p_out - 1;
    if (offset >= s_k)
        chk_loop = 0;
    else
        chk_loop = offset;

    if (SemId[s_k] != -1)
        SEM_Lock(SemId[s_k]);

    if (SemId[s_k] != -1)
        SEM_UnLock(SemId[s_k]);

    return (chk_loop);
}   /* End of GETFILENO_NOLOCK () */

/*************************************************************************
    End of Program (getfileno.c)
*************************************************************************/
