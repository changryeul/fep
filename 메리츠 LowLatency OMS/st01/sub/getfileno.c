/*------------------------------------------------------------------------
#	Module	: out file search
#	File	: getfileno.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
int		GETFILENO (int, int);
int		GETFILENO_NOLOCK (int, int);

/*************************************************************************
	Function        : . out file search
	Parameters IN   : . p_out   : DELAY_TIME(out dfile count)
					  . offset  : process별 offset value
	Parameters OUT  : . 처리가능 process 지정.
	Return Code     : . int
						> 0 : success (write process)
						-1  : failure
*************************************************************************/
/*----------------------------------------------------------------------*/
int		GETFILENO (int p_out, int offset)
/*----------------------------------------------------------------------*/
{
	int			s_k, i, rt, chk_loop, j;
	union   semun
    {
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
		SEM_Lock (SemId[s_k]);

#if 0	// 202109
	if ((ODW(D_K,P_K,s_k,0) - ODR(D_K,P_K,s_k,0)) == 0)
	{
		for (i = 0; i < s_k; i ++)
		{
			chk_loop = (i + offset >= s_k ? i + offset - s_k : i + offset);

			if (	((ODW(D_K,P_K,chk_loop,0) - ODR(D_K,P_K,chk_loop,0)) == 0)
				&&	(ODR(D_K,P_K,chk_loop,8) != ON)	)
			{
				rt = 0;

				for (j = 0; j < AtoIf(Shm_Futures[0].Futures_A0.cnt, 5); j++)
				{
					if ( (Shm_Futures[j].Futures_CURR_Arry.auto_run == 1) &&
						 (Shm_Futures[j].Futures_CURR_Arry.auto_write == chk_loop + 1) )
					{
						rt = 1;
						break;
					}
				}

				if (rt == 0)
				{
					arg.val = 1;
					if (semctl(SemId[chk_loop], 0, GETVAL, arg) >= 1)
					{
						SEM_Lock (SemId[chk_loop]);
						break;
					}
				}
			}

			if (i >= (s_k - 1))
			{
				chk_loop = -1;
				break;
			}
		}
	}
	else
		chk_loop = -1;
#endif

	if (SemId[s_k] != -1)
		SEM_UnLock (SemId[s_k]);

	return (chk_loop);
}   /* End of GETFILENO () */

/*************************************************************************
	Function        : . out file search
	Parameters IN   : . p_out   : DELAY_TIME(out dfile count)
					  . offset  : process별 offset value
	Parameters OUT  : . 처리가능 process 지정.
	Return Code     : . int
						> 0 : success (write process)
						-1  : failure
*************************************************************************/
/*----------------------------------------------------------------------*/
int		GETFILENO_NOLOCK (int p_out, int offset)
/*----------------------------------------------------------------------*/
{
	int			s_k, i, rt, chk_loop, j;
	union   semun
    {
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
		SEM_Lock (SemId[s_k]);

#if 0	// 202109
	if ((ODW(D_K,P_K,s_k,0) - ODR(D_K,P_K,s_k,0)) == 0)
	{
		for (i = 0; i < s_k; i ++)
		{
			chk_loop = (i + offset >= s_k ? i + offset - s_k : i + offset);

			if ((ODW(D_K,P_K,chk_loop,0) - ODR(D_K,P_K,chk_loop,0)) == 0)
			{
				rt = 0;

				for (j = 0; j < AtoIf(Shm_Futures[0].Futures_A0.cnt, 5); j++)
				{
					if ( (Shm_Futures[j].Futures_CURR_Arry.auto_run == 1) &&
						 (Shm_Futures[j].Futures_CURR_Arry.auto_write == chk_loop + 1) )
					{
						rt = 1;
						break;
					}
				}

				if (rt == 0)
				{
					arg.val = 1;
					if (semctl(SemId[chk_loop], 0, GETVAL, arg) >= 1)
						break;
				}
			}

			if (i >= (s_k - 1))
			{
				chk_loop = -1;
				break;
			}
		}
	}
	else
		chk_loop = -1;
#endif

	if (SemId[s_k] != -1)
		SEM_UnLock (SemId[s_k]);

	return (chk_loop);
}   /* End of GETFILENO_NOLOCK () */

/*************************************************************************
	End of Program (getfileno.c)
*************************************************************************/
