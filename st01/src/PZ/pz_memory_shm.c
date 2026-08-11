/*------------------------------------------------------------------------
#   Module  : initialize shared memory
#   File    : pz_memory_shm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "daemon.h"

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    Sub_SHM_Creat(void);
void    Mem_SHM_Creat(void);
void    Sise_SHM_Creat(void);

/*************************************************************************
    Function        : . initialize daemon SHM (INFO)
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Sub_SHM_Creat(void)
/*----------------------------------------------------------------------*/
{
    key_t   shm_key = BASE_SHM_KEY;

    if (memcmp(_FEP_DIV, "TEST", 4) == 0)
        shm_key += 0x01000000L;

    Shmsize = sizeof (ALL_DAEMON_INFO) * SHM_MAX_SUB;

    if (!Shmsize)
        return;

    Log(USR_OK, "00. SuperDaemon SHM[%#x] Size[%d]", shm_key, Shmsize);
    Shmptr = SHM_Creat_Attach(shm_key, Shmsize, &SHM_Shmid);
    SHM_All_Daemon_Info = (ALL_DAEMON_INFO *)Shmptr;
    Log(USR_OK, "daemon SHM[%#x,%d]", shm_key, SHM_Shmid);

    return;
}   /* End of Sub_SHM_Creat ()  */

/*************************************************************************
    Function        : . initialize sub SHM
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Mem_SHM_Creat(void)
/*----------------------------------------------------------------------*/
{
    int     i, j;
    char    key[12];
    key_t   base_key, shm_key;

    base_key = BASE_SHM_KEY;

    if (memcmp(_FEP_DIV, "TEST", 4) == 0)
        base_key += 0x01000000L;

    memset(Info, 0, sizeof (Info));

    /* read daemon.ini and set temporary daemon buffer (Info)   */
    Daemon_Config_Read(2);

    for (i = 0; i < Process_Count; i++) {
        Shmsize = 0;
        sprintf(key, "0x00%02d0000", i + 1);
        errno = 0;
        shm_key = base_key + strtol(key, NULL, 16);

        if (D_K == -1 || D_K == i) {
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

            if (Info[i].shm_log == 1)       /* use SHM (delayed) log    */
                Shmsize += SHM_LOG_SIZE * SHM_LOG_MAX;

            Log(USR_OK, "00. pz_memory_shm i[%d] SHM[%#x] size[%d] Start!!", i, shm_key, Shmsize);
            Shmptr = SHM_Creat_Attach(shm_key, Shmsize, &Mem_Shmid[i]);
            Log(USR_OK, "01. pz_memory_shm i[%d] SHM[%#x] size[%d] End!!", i, shm_key, Shmsize);
            SHM_Mem[i] = (char *)Shmptr;

            /* copy counts to INFO(i) for Shm_Map_SubDaemon */
            INFO(i).process_count = Info[i].process_count;
            INFO(i).file_count = Info[i].file_count;
            INFO(i).dshm_count = Info[i].dshm_count;
#if defined ISAM_INCL
            INFO(i).cisam_count = Info[i].cisam_count;
#endif
            INFO(i).tcp1_count = Info[i].tcp1_count;
            INFO(i).tcp2_count = Info[i].tcp2_count;
            INFO(i).udpip_count = Info[i].udpip_count;

            Shmptr = Shm_Map_SubDaemon(Shmptr, &INFO(i), &Shm_Mem[i]);

            if (Info[i].data_count != 0) {
                for (j = 0; j < Info[i].data_count; j++) {
                    Data_Ptr[j] = Shmptr;
                    Shmptr += SHM_DATA_SIZE;
                }
            }

            if (Info[i].shm_log == 1)
                ShmLogPtr = Shmptr;

            /* write SHM version marker */
            {
                int ver = SHM_VERSION;
                int mag = SHM_MAGIC;
                memcpy(&DAEMON(i).process_info[32], &mag, 4);
                memcpy(&DAEMON(i).process_info[36], &ver, 4);
            }

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

            /* read daemon.ini and set daemon SHM (DAEMON, INFO)    */
            Log(USR_OK, "%c%c sub SHM BEFORE", _System_Name[0], i + 'A');
            Daemon_Config_Read(1);

            Log(USR_OK, "%c%c sub SHM initialized[%#x]",
                    _System_Name[0], i + 'A', shm_key);

            if (D_K == i)
                break;
        }
    }

    return;
}   /* End of Mem_SHM_Creat ()  */

/**************************************************************************
    Function        : . initialize sise SHM
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
**************************************************************************/
/*-----------------------------------------------------------------------*/
void    Sise_SHM_Creat(void)
/*-----------------------------------------------------------------------*/
{
    key_t   k;

    if (memcmp(_FEP_DIV, "TEST", 4) == 0)
        k = 0x01000000L;
    else
        k = 0x00000000L;

    /* 1.채권KTS */
    Log(USR_OK, "1. SHM[%#x]", NOTE_SHM_KEY);
    Shmsize = sizeof (SHM_NOTE) * SHM_MAX_NOTE;         /* 10,000 */
    Shm_Note = (SHM_NOTE *)SHM_Creat_Attach(NOTE_SHM_KEY + k, Shmsize,
            &SISE_Note_Shmid);

    /* 2.채권RDS01 */
    Log(USR_OK, "2. SHM[%#x]", RDS01_SHM_KEY);
    Shmsize = sizeof (CO_A001_RDS01) * SHM_MAX_RDS01;   /* 100,000 */
    Shm_Rds01 = (CO_A001_RDS01 *)SHM_Creat_Attach(RDS01_SHM_KEY + k, Shmsize,
            &RDS_01_Shmid);

    /* 3.금융파생 */
    Log(USR_OK, "3. SHM[%#x]", FF_SHM_KEY);
    Shmsize = sizeof (SHM_FIN_FUT) * SHM_MAX_DEV_FIF;   /* 1,000 */
    Shm_FinFut = (SHM_FIN_FUT *)SHM_Creat_Attach(FF_SHM_KEY + k, Shmsize,
            &SISE_FF_Shmid);

    /* 22. 계좌별 전략 영역 */
    Log(USR_OK, "4. SHM[%#x]", STRRG_SHM_KEY);
    Shmsize = sizeof (STRRG) * SHM_MAX_STRRG;
    Shm_Strrg = (STRRG *)SHM_Creat_Attach(STRRG_SHM_KEY + k, Shmsize,
            &SISE_Strrg_Shmid);

    /* 23. 시장뼐 미체결관리 */
    Log(USR_OK, "5. SHM[%#x]", MK_PM_SHM_KEY);
    Shmsize = sizeof (MK_PREMATCH) * SHM_MAX_PREMATCH;
    Shm_Mk_PreMatch = (MK_PREMATCH *)SHM_Creat_Attach(MK_PM_SHM_KEY + k, Shmsize,
            &SISE_Mk_Prematch_Shmid);

    /* 24. 한도관리영역 */
    Log(USR_OK, "6. SHM[%#x]", RISK_SHM_KEY);
    Shmsize = sizeof (RISK) * 1;
    Shm_Risk = (RISK *)SHM_Creat_Attach(RISK_SHM_KEY + k, Shmsize,
            &SISE_Risk_Shmid);

    /* 30. 종목코드 sort */
    Log(USR_OK, "7. SHM[%#x]", ITEM_SHM_KEY);
    Shmsize = sizeof (SHM_KEY_ARRY) * 1;
    Shm_Item = (SHM_KEY_ARRY *)SHM_Creat_Attach(ITEM_SHM_KEY + k, Shmsize,
            &SISE_Item_Shmid);

    return;
}   /* End of Sise_SHM_Creat () */

/*************************************************************************
    End of Program (pz_memory_shm.c)
*************************************************************************/
