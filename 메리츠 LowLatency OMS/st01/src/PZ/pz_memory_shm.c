/*------------------------------------------------------------------------
#	Module	: initialize shared memory
#	File	: pz_memory_shm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"daemon.h"

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void    Sub_SHM_Creat (void);
void    Mem_SHM_Creat (void);
void	Sise_SHM_Creat (void);

/*************************************************************************
	Function		: . initialize daemon SHM (INFO)
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Sub_SHM_Creat (void)
/*----------------------------------------------------------------------*/
{
	int		i;
	key_t	shm_key = BASE_SHM_KEY;

	if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
		shm_key += 0x01000000L;

	Shmsize = sizeof (ALL_DAEMON_INFO) * SHM_MAX_SUB;

	if (!Shmsize)
		return;

	Shmptr = SHM_Creat_Attach (shm_key, Shmsize, &SHM_Shmid);
	SHM_All_Daemon_Info = (ALL_DAEMON_INFO *)Shmptr;
	Log (USR_OK, "daemon SHM[%#x,%d]", shm_key, SHM_Shmid);

	return;
}	/* End of Sub_SHM_Creat ()	*/

/*************************************************************************
	Function		: . initialize sub SHM
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Mem_SHM_Creat (void)
/*----------------------------------------------------------------------*/
{
	int		i, j;
	char	key[12];
	key_t	base_key, shm_key;

	base_key = BASE_SHM_KEY;

	if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
		base_key += 0x01000000L;

	memset (Info, 0, sizeof (Info));

	/* read daemon.ini and set temporary daemon buffer (Info)	*/
	Daemon_Config_Read (2);

	for (i = 0; i < Process_Count; i++)
	{
		Shmsize = 0;
		sprintf (key, "0x00%02d0000", i + 1);
		errno = 0;
		shm_key = base_key + strtol (key, NULL, 16);

		if (D_K == -1 || D_K == i)
		{
			Info[i].Shmsize = sizeof (PROCESS_INFO) * Info[i].process_count;
			Shmsize += Info[i].Shmsize;
			Info[i].FShmsize = sizeof (FILE_INFO) * Info[i].file_count;
			Shmsize += Info[i].FShmsize;
			Info[i].DShmsize = sizeof (DSHM_INFO) * Info[i].dshm_count;
			Shmsize += Info[i].DShmsize;
#if defined ISAM_INCL
			Info[i].CShmsize = sizeof (CISAM_INFO) * Info[i].cisam_count;
			Shmsize += Info[i].CShmsize;
#endif
			Info[i].T1Shmsize = sizeof (TCP1_INFO) * Info[i].tcp1_count;
			Shmsize += Info[i].T1Shmsize;
			Info[i].T2Shmsize = sizeof (TCP2_INFO) * Info[i].tcp2_count;
			Shmsize += Info[i].T2Shmsize;
			Info[i].UShmsize = sizeof (UDPIP_INFO) * Info[i].udpip_count;
			Shmsize += Info[i].UShmsize;
/* 202201
			Info[i].SShmsize = sizeof (SISETR_INFO) * Info[i].sisetr_count;
			Shmsize += Info[i].SShmsize;
			Info[i].AShmsize = sizeof (ACCNO_INFO) * Info[i].accno_count;
            Shmsize += Info[i].AShmsize;
*/

			if (!Shmsize)
				continue;

			Shmsize += sizeof (SUB_DAEMON_INFO);

			if (Info[i].data_count != 0)
				Shmsize += SHM_DATA_SIZE * Info[i].data_count;

			if (Info[i].shm_log == 1)		/* use SHM (delayed) log	*/
				Shmsize += SHM_LOG_SIZE * SHM_LOG_MAX;

			Shmptr = SHM_Creat_Attach (shm_key, Shmsize, &Mem_Shmid[i]);
			SHM_Mem[i] = (char *)Shmptr;

			Shm_Mem[i].Daemon = (SUB_DAEMON_INFO *)Shmptr;
			Shmptr += sizeof (SUB_DAEMON_INFO);
			Shm_Mem[i].Proc = (PROCESS_INFO *)Shmptr;
			Shmptr += Info[i].Shmsize;
			Shm_Mem[i].File = (FILE_INFO *)Shmptr;
			Shmptr += Info[i].FShmsize;
			Shm_Mem[i].DShm = (DSHM_INFO *)Shmptr;
			Shmptr += Info[i].DShmsize;
#if defined ISAM_INCL
			Shm_Mem[i].Cisam = (CISAM_INFO *)Shmptr;
			Shmptr += Info[i].CShmsize;
#endif
			Shm_Mem[i].Tcp1 = (TCP1_INFO *)Shmptr;
			Shmptr += Info[i].T1Shmsize;
			Shm_Mem[i].Tcp2 = (TCP2_INFO *)Shmptr;
			Shmptr += Info[i].T2Shmsize;
			Shm_Mem[i].Udpip = (UDPIP_INFO *)Shmptr;
			Shmptr += Info[i].UShmsize;
/* 202201
            Shm_Mem[i].Sisetr = (SISETR_INFO *)Shmptr;
            Shmptr += Info[i].SShmsize;
			Shm_Mem[i].Accno = (ACCNO_INFO *)Shmptr;
            Shmptr += Info[i].AShmsize;
*/

			if (Info[i].data_count != 0)
			{
				for (j = 0; j < Info[i].data_count; j++)
				{
					Data_Ptr[j] = Shmptr;
					Shmptr += SHM_DATA_SIZE;
				}
			}

			if (Info[i].shm_log == 1)
				ShmLogPtr = Shmptr;

			DAEMON(i).Shmsize = Info[i].Shmsize;
			DAEMON(i).FShmsize = Info[i].FShmsize;
			DAEMON(i).DShmsize = Info[i].DShmsize;
#if defined ISAM_INCL
			DAEMON(i).CShmsize = Info[i].CShmsize;
#endif
			DAEMON(i).T1Shmsize = Info[i].T1Shmsize;
			DAEMON(i).T2Shmsize = Info[i].T2Shmsize;
			DAEMON(i).UShmsize = Info[i].UShmsize;
/* 202201
            DAEMON(i).SShmsize = Info[i].SShmsize;
            DAEMON(i).AShmsize = Info[i].AShmsize;
*/

			/* read daemon.ini and set daemon SHM (DAEMON, INFO)	*/
			Daemon_Config_Read (1);

			Log (USR_OK, "%c%c sub SHM initialized[%#x]",
				_System_Name[0], i + 'A', shm_key);

			if (D_K == i)
				break;
		}
	}

	return;
}	/* End of Mem_SHM_Creat ()	*/

/**************************************************************************
	Function		: . initialize sise SHM
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
**************************************************************************/
/*-----------------------------------------------------------------------*/
void	Sise_SHM_Creat (void)
/*-----------------------------------------------------------------------*/
{
	key_t	k;

	if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
		k = 0x01000000L;
	else
		k = 0x00000000L;

	/* 1.지수선물 */
    Shmsize = sizeof (SHM_FUTURES) * SHM_MAX_FUTURES;
    Shm_Futures = (SHM_FUTURES *)SHM_Creat_Attach (F_SHM_KEY + k, Shmsize,
        &SISE_F_Shmid);

    /* 2.지수옵션 */
    Shmsize = sizeof (SHM_OPTIONS) * SHM_MAX_OPTIONS;
    Shm_Options = (SHM_OPTIONS *)SHM_Creat_Attach (O_SHM_KEY + k, Shmsize,
        &SISE_O_Shmid);

    /* 3.주식선물 */
    Shmsize = sizeof (SHM_STOCK_FUTURES) * SHM_MAX_S_FUTURES;
    Shm_SFutures = (SHM_STOCK_FUTURES *)SHM_Creat_Attach (SF_SHM_KEY + k, Shmsize,
        &SISE_SF_Shmid);

    /* 4.주식옵션 */
    Shmsize = sizeof (SHM_STOCK_OPTIONS) * SHM_MAX_S_OPTIONS;
    Shm_SOptions = (SHM_STOCK_OPTIONS *)SHM_Creat_Attach (SO_SHM_KEY + k, Shmsize,
        &SISE_SO_Shmid);

    /* 5. 유가증권 */
    Shmsize = sizeof (SHM_STOCK) * SHM_MAX_STOCK;
    Shm_Stock = (SHM_STOCK *)SHM_Creat_Attach (S_SHM_KEY + k, Shmsize,
        &SISE_S_Shmid);

    /* 6. 코스닥 */
    Shmsize = sizeof (SHM_KOSDAQ) * SHM_MAX_KOSDAQ;
    Shm_Kosdaq = (SHM_KOSDAQ *)SHM_Creat_Attach (K_SHM_KEY + k, Shmsize,
        &SISE_K_Shmid);

#if 0
    /* 7. ELW */
    Shmsize = sizeof (SHM_ELW) * SHM_MAX_ELW;
	Shm_Elw = (SHM_ELW *)SHM_Creat_Attach (ELW_SHM_KEY + k, Shmsize,
        &SISE_ELW_Shmid);

    /* 17. ETN */
    Shmsize = sizeof (SHM_ETN) * SHM_MAX_ETN;
    Shm_Etn = (SHM_ETN *)SHM_Creat_Attach (ETN_SHM_KEY + k, Shmsize,
        &SISE_ETN_Shmid);
#endif

    /* 8. KRX300 */
    Shmsize = sizeof (SHM_K300) * SHM_MAX_K300;
    Shm_K300 = (SHM_K300 *)SHM_Creat_Attach (K300_SHM_KEY + k, Shmsize,
        &SISE_K300_Shmid);

    /* 9. KOSDAQ150F */
    Shmsize = sizeof (SHM_K150F) * SHM_MAX_K150F;
    Shm_K150F = (SHM_K150F *)SHM_Creat_Attach (K150F_SHM_KEY + k, Shmsize,
        &SISE_K150F_Shmid);

    /* 10. KOSDAQ150O */
    Shmsize = sizeof (SHM_K150O) * SHM_MAX_K150O;
    Shm_K150O = (SHM_K150O *)SHM_Creat_Attach (K150O_SHM_KEY + k, Shmsize,
        &SISE_K150O_Shmid);

	/* 11.미니선물 */
    Shmsize = sizeof (SHM_MF) * SHM_MAX_MF;
    Shm_MF = (SHM_MF *)SHM_Creat_Attach (MF_SHM_KEY + k, Shmsize,
        &SISE_MF_Shmid);

    /* 12.미니옵션 */
    Shmsize = sizeof (SHM_MO) * SHM_MAX_MO;
    Shm_MO = (SHM_MO *)SHM_Creat_Attach (MO_SHM_KEY + k, Shmsize,
        &SISE_MO_Shmid);

	/* 20. 지수 */
    Shmsize = sizeof (SHM_JISU) * SHM_MAX_JISU;
    Shm_Jisu = (SHM_JISU *)SHM_Creat_Attach (J_SHM_KEY + k, Shmsize,
        &SISE_J_Shmid);

    /* 22. 계좌별 전략 영역 */
    Shmsize = sizeof (STRRG) * SHM_MAX_STRRG;
    Shm_Strrg = (STRRG *)SHM_Creat_Attach (STRRG_SHM_KEY + k, Shmsize,
        &SISE_Strrg_Shmid);

    /* 23. 시장뼐 미체결관리 */
    Shmsize = sizeof (MK_PREMATCH) * SHM_MAX_PREMATCH;
    Shm_Mk_PreMatch = (MK_PREMATCH *)SHM_Creat_Attach (MK_PM_SHM_KEY + k, Shmsize,
        &SISE_Mk_Prematch_Shmid);

    /* 24. 한도관리영역 */
	Shmsize = sizeof (RISK) * 1;
    Shm_Risk = (RISK *)SHM_Creat_Attach (RISK_SHM_KEY + k, Shmsize,
        &SISE_Risk_Shmid);

    /* 30. 종목코드 sort */
    Shmsize = sizeof (SHM_KEY_ARRY) * 1;
    Shm_Item = (SHM_KEY_ARRY *)SHM_Creat_Attach (ITEM_SHM_KEY + k, Shmsize,
        &SISE_Item_Shmid);

    /* 31. CME */
    Shmsize = sizeof (SHM_CME) * SHM_MAX_CME;
    Shm_CME = (SHM_CME *)SHM_Creat_Attach (CME_KEY + k, Shmsize,
        &SISE_CME_Shmid);

    /* 32. CMX */
    Shmsize = sizeof (SHM_CMX) * SHM_MAX_CMX;
    Shm_CMX = (SHM_CMX *)SHM_Creat_Attach (CMX_KEY + k, Shmsize,
        &SISE_CMX_Shmid);

    /* 33. SGX */
    Shmsize = sizeof (SHM_SGX) * SHM_MAX_SGX;
    Shm_SGX = (SHM_SGX *)SHM_Creat_Attach (SGX_KEY + k, Shmsize,
        &SISE_SGX_Shmid);

    /* 34. ERX */
    Shmsize = sizeof (SHM_ERX) * SHM_MAX_ERX;
    Shm_ERX = (SHM_ERX *)SHM_Creat_Attach (ERX_KEY + k, Shmsize,
        &SISE_ERX_Shmid);

    /* 35. HKE */
    Shmsize = sizeof (SHM_HKE) * SHM_MAX_HKE;
    Shm_HKE = (SHM_HKE *)SHM_Creat_Attach (HKE_KEY + k, Shmsize,
        &SISE_HKE_Shmid);

#if 0
    /* SHM Backup */
    Shmsize = sizeof (SHM_BACKOFFICE) * SHM_MAX_BACKOFFICE;
    Shm_BackOffice = (SHM_BACKOFFICE *)SHM_Creat_Attach (B_SHM_KEY + k,
        Shmsize, &SISE_B_Shmid);
#endif
	
	return;
}	/* End of Sise_SHM_Creat ()	*/

/*************************************************************************
	End of Program (pz_memory_shm.c)
*************************************************************************/
