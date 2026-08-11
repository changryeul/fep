/*------------------------------------------------------------------------
#	Module	: attach shared memory
#	File	: shmsub.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	Sub_SHM (void);
void    Mem_SHM (int, int);
void	Sise_SHM (void);
char    *Shm_Attach (key_t, int *);
char    *SHM_Creat_Attach (key_t, size_t, int *);

/*************************************************************************
	Function		: . attach daemon SHM (INFO)
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Sub_SHM (void)
/*----------------------------------------------------------------------*/
{
	key_t	shm_key = BASE_SHM_KEY;

	if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
		shm_key += 0x01000000L;

    Shmptr = Shm_Attach (shm_key, &SHM_Shmid);
	SHM_All_Daemon_Info = (ALL_DAEMON_INFO *)Shmptr;

	return;
}	/* End of Sub_SHM ()	*/

/*************************************************************************
	Function	   : . attach sub SHM (Shm_Mem)
	Parameters IN  : . flag	: flag (0: all, 1: one sub)
	Parameters OUT : .
	Return Code	   : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Mem_SHM (int flag, int dk)
/*----------------------------------------------------------------------*/
{
	int		i, j, k;
	char	key[12];
	key_t	base_key, shm_key;

	base_key = BASE_SHM_KEY;

	if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
		base_key += 0x01000000L;

	for (i = 0; i < Process_Count; i ++)
	{
		Shmsize = 0;
		sprintf (key, "0x00%02d0000", i + 1);
		shm_key = base_key + strtol (key, NULL, 16);

		if ((flag == 1 && i == dk) || (flag == 0 && INFO(i).process_id[0] != 0))
		{
			Shmsize = sizeof (PROCESS_INFO) * INFO(i).process_count +
				sizeof (FILE_INFO) * INFO(i).file_count +
				sizeof (DSHM_INFO) * INFO(i).dshm_count +
#if defined ISAM_INCL
				sizeof (CISAM_INFO) * INFO(i).cisam_count +
#endif
				sizeof (TCP1_INFO) * INFO(i).tcp1_count +
				sizeof (TCP2_INFO) * INFO(i).tcp2_count +
				sizeof (UDPIP_INFO) * INFO(i).udpip_count;
/* 202201
				sizeof (SISETR_INFO) * INFO(i).sisetr_count +
				sizeof (ACCNO_INFO) * INFO(i).accno_count;
*/

			if (!Shmsize)
				continue;

			Shmsize += sizeof (SUB_DAEMON_INFO);

			if (INFO(i).data_count != 0)
				Shmsize += SHM_DATA_SIZE * INFO(i).data_count;

			if (INFO(i).shm_log == 1)		/* use SHM (delayed) log	*/
				Shmsize += SHM_LOG_SIZE * SHM_LOG_MAX;

			Shmptr = Shm_Attach (shm_key, &Mem_Shmid[i]);
			SHM_Mem[i] = (char *)Shmptr;

			Shm_Mem[i].Daemon = (SUB_DAEMON_INFO *)Shmptr;
			Shmptr += sizeof (SUB_DAEMON_INFO);
			Shm_Mem[i].Proc = (PROCESS_INFO *)Shmptr;
			Shmptr += sizeof (PROCESS_INFO) * INFO(i).process_count;
			Shm_Mem[i].File = (FILE_INFO *)Shmptr;
			Shmptr += sizeof (FILE_INFO) * INFO(i).file_count;
			Shm_Mem[i].DShm = (DSHM_INFO *)Shmptr;
			Shmptr += sizeof (DSHM_INFO) * INFO(i).dshm_count;
#if defined ISAM_INCL
			Shm_Mem[i].Cisam = (CISAM_INFO *)Shmptr;
			Shmptr += sizeof (CISAM_INFO) * INFO(i).cisam_count;
#endif
			Shm_Mem[i].Tcp1 = (TCP1_INFO *)Shmptr;
			Shmptr += sizeof (TCP1_INFO) * INFO(i).tcp1_count;
			Shm_Mem[i].Tcp2 = (TCP2_INFO *)Shmptr;
			Shmptr += sizeof (TCP2_INFO) * INFO(i).tcp2_count;
			Shm_Mem[i].Udpip = (UDPIP_INFO *)Shmptr;
			Shmptr += sizeof (UDPIP_INFO) * INFO(i).udpip_count;
/* 202201
			Shm_Mem[i].Sisetr = (SISETR_INFO *)Shmptr;
			Shmptr += sizeof (SISETR_INFO) * INFO(i).sisetr_count;
			Shm_Mem[i].Accno = (ACCNO_INFO *)Shmptr;
			Shmptr += sizeof (ACCNO_INFO) * INFO(i).accno_count;
*/

			if (INFO(i).data_count != 0)
			{
				for (j = 0; j < INFO(i).data_count; j ++)
				{
					Data_Ptr[j] = Shmptr;
					Shmptr += SHM_DATA_SIZE;
				}

				for (k = j = 0; j < DAEMON(i).p_count; j ++)
				{
					if (PROC(i,j).data_flag == 1)
						PROC(i,j).data = (char *)Data_Ptr[k++];
				}
			}

			if (INFO(i).shm_log == 1)
				ShmLogPtr = Shmptr;

			if (flag == 1)
				break;
		}
	}

	return;
}	/* End of Mem_SHM ()	*/

/*************************************************************************
	Function	   : . attach sise SHM
	Parameters IN  : .
	Parameters OUT : .
	Return Code	   : . void
*************************************************************************/
/*-----------------------------------------------------------------------*/
void	Sise_SHM (void)
/*-----------------------------------------------------------------------*/
{
	key_t	k;

	if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
		k = 0x01000000L;
	else
		k = 0x00000000L;

	/* 1. 지수선물	*/
	Shm_Futures = (SHM_FUTURES *)Shm_Attach (F_SHM_KEY + k, &SISE_F_Shmid);

	/* 2. 지수옵션	*/
	Shm_Options = (SHM_OPTIONS *)Shm_Attach (O_SHM_KEY + k, &SISE_O_Shmid);

	/* 3. 주식선물	*/
	Shm_SFutures = (SHM_STOCK_FUTURES *)Shm_Attach (SF_SHM_KEY + k, &SISE_SF_Shmid);

	/* 4. 주식옵션	*/
	Shm_SOptions = (SHM_STOCK_OPTIONS *)Shm_Attach (SO_SHM_KEY + k, &SISE_SO_Shmid);

	/* 5. 유가증권	*/
	Shm_Stock = (SHM_STOCK *)Shm_Attach (S_SHM_KEY + k, &SISE_S_Shmid);

	/* 6. 코스닥	*/
	Shm_Kosdaq = (SHM_KOSDAQ *)Shm_Attach (K_SHM_KEY + k, &SISE_K_Shmid);

#if 0
	/* 7. ELW 시세	*/
	Shm_Elw = (SHM_ELW *)Shm_Attach (ELW_SHM_KEY + k, &SISE_ELW_Shmid);

	/* 17. ETN 시세	*/
	Shm_Etn = (SHM_ETN *)Shm_Attach (ETN_SHM_KEY + k, &SISE_ETN_Shmid);
#endif

	/* 8. KRX300	*/
	Shm_K300 = (SHM_K300 *)Shm_Attach (K300_SHM_KEY + k, &SISE_K300_Shmid);

	/* 9. Kosdaq150F	*/
	Shm_K150F = (SHM_K150F *)Shm_Attach (K150F_SHM_KEY + k, &SISE_K150F_Shmid);

	/* 10. Kosdaq150S	*/
	Shm_K150O = (SHM_K150O *)Shm_Attach (K150O_SHM_KEY + k, &SISE_K150O_Shmid);

	/* 20. 지수	*/
	Shm_Jisu = (SHM_JISU *)Shm_Attach (J_SHM_KEY + k, &SISE_J_Shmid);

	/* 22. 계좌별 전략 영역 */
	Shm_Strrg = (STRRG *)Shm_Attach (STRRG_SHM_KEY + k, &SISE_Strrg_Shmid);

	/* 23. 시장별 미체결관리 */
	Shm_Mk_PreMatch = (MK_PREMATCH *)Shm_Attach (MK_PM_SHM_KEY + k, &SISE_Mk_Prematch_Shmid);

	/* 24. 한도관리영역 */
	Shm_Risk = (RISK *)Shm_Attach (RISK_SHM_KEY + k, &SISE_Risk_Shmid);

	/* 30. item code qsort 처리용 */
	Shm_Item = (SHM_KEY_ARRY *)Shm_Attach (ITEM_SHM_KEY + k, &SISE_Item_Shmid);

	/* 31. CME */
	Shm_CME = (SHM_CME *)Shm_Attach (CME_KEY + k, &SISE_CME_Shmid);

	/* 32. CMX */
	Shm_CMX = (SHM_CMX *)Shm_Attach (CMX_KEY + k, &SISE_CMX_Shmid);

	/* 33. SGX */
	Shm_SGX = (SHM_SGX *)Shm_Attach (SGX_KEY + k, &SISE_SGX_Shmid);

	/* 34. ERX */
	Shm_ERX = (SHM_ERX *)Shm_Attach (ERX_KEY + k, &SISE_ERX_Shmid);

	/* 35. HKE */
	Shm_HKE = (SHM_HKE *)Shm_Attach (HKE_KEY + k, &SISE_HKE_Shmid);

	return;
}	/* End of Sise_SHM ()	*/

/*************************************************************************
	Function		: . attach shared memory
	Parameters IN	: . p_key	: shared memory key
	Parameters OUT	: . p_id	: shared memory ID
	Return Code		: . char * (data segment start address of the attached
						shared memory segment: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
char	*Shm_Attach (key_t p_key, int *p_id)
/*----------------------------------------------------------------------*/
{
    char	*ptr = (char *)-1;

	/* create shared memory	- shmget	*/
	*p_id = SHM_Creat (p_key, 0);

    if (*p_id == -1) 
    {
        Log (SYS_FATAL, "Shm_Attach:cannot create SHM[%#x] {%d:%s}",
			p_key, SYS_NO, SYS_STR);
		Exit_Process ();
    }

#if (0)
    Log (USR_OK, "shared memory created[%d]", *p_id);
#endif

	/* attach shared memory - shmat	*/
	ptr = SHM_Attach (*p_id);

    if (ptr == (char *)-1) 
    {
		Log (SYS_FATAL, "Shm_Attach:cannot attach SHM {%d:%s}",
			SYS_NO, SYS_STR);
		Exit_Process ();
	}

#if (0)
	Log (USR_OK, "shared memory attached[%d,%#x]", ptr, ptr);
#endif

    return (ptr);
}	/* End of Shm_Attach ()	*/

/*************************************************************************
	Function		: . create and attach the shared memory
	Parameters IN	: . p_key	: SHM key
					  . p_size	: size of SHM segment
	Parameters OUT	: . p_id	: SHM id
	Return Code		: . char *
*************************************************************************/
/*----------------------------------------------------------------------*/
char	*SHM_Creat_Attach (key_t p_key, size_t p_size, int *p_id)
/*----------------------------------------------------------------------*/
{
	int		i;
	char	cmd[256];
	char	*ptr = (char *)-1;

	i = 0;
	while (1)
	{
		i++;
		*p_id = SHM_Creat_Excl (p_key, p_size);

		if (*p_id == -1)
		{
			*p_id = SHM_Creat (p_key, 0);

			if (*p_id == -1)
			{
				Log (SYS_FATAL, "cannot create SHM[%#x,%d] {%d:%s}",
					p_key, p_size, SYS_NO, SYS_STR);
				exit (FAIL);
			}

			if (i == 1)
			{
				SHM_Remove (*p_id);
			}
			if (i == 2)
			{
				memset (cmd, 0, sizeof (cmd));
				sprintf (cmd, "ipcrm -m %d", *p_id);
				system (cmd);
				*p_id = SHM_Creat_Excl (p_key, p_size);
				Log (USR_OK, "system ipcrm [%s] rt[%d]", cmd, *p_id);
				sleep (1);
			}
			else if (i > 2)
				break;
			continue;
		}
		else
			break;
	}

	Log (SYS_OK, "SHM created[%d,%#x,%d]", *p_id, p_key, p_size);

	ptr = SHM_Attach (*p_id);

	if (ptr == (char *)-1)
	{
		Log (SYS_FATAL, "cannot attach SHM[%d] {%d:%s}",
			*p_id, SYS_NO, SYS_STR);
		exit (FAIL);
	}

	memset (ptr, 0, p_size);

	return (ptr);
}	/* End of SHM_Creat_Attach ()	*/

/*************************************************************************
	End of Program (shmsub.c)
*************************************************************************/
