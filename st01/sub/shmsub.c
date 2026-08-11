/*------------------------------------------------------------------------
#   Module  : attach shared memory
#   File    : shmsub.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*************************************************************************
    Function        : . map SHM segment to SHM_MEMORY structure
    Parameters IN   : . base    : SHM start address
                      . info    : ALL_DAEMON_INFO (count info)
    Parameters OUT  : . mem     : SHM_MEMORY pointers
    Return Code     : . char * (next address after mapped region)
*************************************************************************/
/*----------------------------------------------------------------------*/
char    *Shm_Map_SubDaemon(char *base, ALL_DAEMON_INFO *info,
        SHM_MEMORY *mem)
/*----------------------------------------------------------------------*/
{
    char    *ptr = base;

    mem->Daemon = (SUB_DAEMON_INFO *)ptr;
    ptr += sizeof (SUB_DAEMON_INFO);
    mem->Proc = (PROCESS_INFO *)ptr;
    ptr += sizeof (PROCESS_INFO) * info->process_count;
    mem->File = (FILE_INFO *)ptr;
    ptr += sizeof (FILE_INFO) * info->file_count;
    mem->DShm = (DSHM_INFO *)ptr;
    ptr += sizeof (DSHM_INFO) * info->dshm_count;
#if defined ISAM_INCL
    mem->Cisam = (CISAM_INFO *)ptr;
    ptr += sizeof (CISAM_INFO) * info->cisam_count;
#endif
    mem->Tcp1 = (TCP1_INFO *)ptr;
    ptr += sizeof (TCP1_INFO) * info->tcp1_count;
    mem->Tcp2 = (TCP2_INFO *)ptr;
    ptr += sizeof (TCP2_INFO) * info->tcp2_count;
    mem->Udpip = (UDPIP_INFO *)ptr;
    ptr += sizeof (UDPIP_INFO) * info->udpip_count;

    return (ptr);
}   /* End of Shm_Map_SubDaemon ()  */

/*************************************************************************
    Function        : . calculate sub-daemon SHM total size
    Parameters IN   : . info    : ALL_DAEMON_INFO (count info)
    Parameters OUT  : .
    Return Code     : . size_t (total SHM size)
*************************************************************************/
/*----------------------------------------------------------------------*/
size_t  Shm_Calc_SubDaemon_Size(ALL_DAEMON_INFO *info)
/*----------------------------------------------------------------------*/
{
    size_t  sz;

    sz = sizeof (SUB_DAEMON_INFO);
    sz += sizeof (PROCESS_INFO) * info->process_count;
    sz += sizeof (FILE_INFO)    * info->file_count;
    sz += sizeof (DSHM_INFO)    * info->dshm_count;
#if defined ISAM_INCL
    sz += sizeof (CISAM_INFO)   * info->cisam_count;
#endif
    sz += sizeof (TCP1_INFO)    * info->tcp1_count;
    sz += sizeof (TCP2_INFO)    * info->tcp2_count;
    sz += sizeof (UDPIP_INFO)   * info->udpip_count;

    if (info->data_count != 0)
        sz += SHM_DATA_SIZE * info->data_count;

    if (info->shm_log == 1)
        sz += SHM_LOG_SIZE * SHM_LOG_MAX;

    return (sz);
}   /* End of Shm_Calc_SubDaemon_Size ()    */

/*************************************************************************
    Function        : . check SHM version marker
    Parameters IN   : . dk      : daemon key
    Parameters OUT  : .
    Return Code     : . int (version number, 0 if legacy)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Shm_Check_Version(int dk)
/*----------------------------------------------------------------------*/
{
    int     mag, ver;

    memcpy(&mag, &DAEMON(dk).process_info[32], 4);
    memcpy(&ver, &DAEMON(dk).process_info[36], 4);

    if (mag != SHM_MAGIC) {
        Log(USR_WARN, "SHM[%d] no version marker(legacy)", dk);
        return (0);
    }

    Log(USR_OK, "SHM[%d] version=%d", dk, ver);
    return (ver);
}   /* End of Shm_Check_Version ()  */

/*************************************************************************
    Function        : . attach daemon SHM (INFO)
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Sub_SHM(void)
/*----------------------------------------------------------------------*/
{
    key_t   shm_key = BASE_SHM_KEY;

    if (memcmp(_FEP_DIV, "TEST", 4) == 0)
        shm_key += 0x01000000L;

    Log(USR_OK, "00. SubDaemon Attach SHM[%#x]", shm_key);
    Shmptr = Shm_Attach(shm_key, &SHM_Shmid);
    SHM_All_Daemon_Info = (ALL_DAEMON_INFO *)Shmptr;

    return;
}   /* End of Sub_SHM ()    */

/*************************************************************************
    Function       : . attach sub SHM (Shm_Mem)
    Parameters IN  : . flag : flag (0: all, 1: one sub)
    Parameters OUT : .
    Return Code    : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Mem_SHM(int flag, int dk)
/*----------------------------------------------------------------------*/
{
    int     i, j, k;
    char    key[32];
    key_t   base_key, shm_key;

    base_key = BASE_SHM_KEY;

    if (memcmp(_FEP_DIV, "TEST", 4) == 0)
        base_key += 0x01000000L;

    for (i = 0; i < Process_Count; i ++) {
        Shmsize = 0;

        sprintf(key, "0x00%02d0000", i + 1);
        shm_key = base_key + strtol(key, NULL, 16);

        if ((flag == 1 && i == dk) || (flag == 0 && INFO(i).process_id[0] != 0)) {
            Shmsize = sizeof (PROCESS_INFO) * INFO(i).process_count +
            sizeof (FILE_INFO) * INFO(i).file_count +
            sizeof (DSHM_INFO) * INFO(i).dshm_count +
#if defined ISAM_INCL
            sizeof (CISAM_INFO) * INFO(i).cisam_count +
#endif
            sizeof (TCP1_INFO) * INFO(i).tcp1_count +
            sizeof (TCP2_INFO) * INFO(i).tcp2_count +
            sizeof (UDPIP_INFO) * INFO(i).udpip_count;

            if (!Shmsize)
                continue;

            Shmsize += sizeof (SUB_DAEMON_INFO);

            if (INFO(i).data_count != 0)
                Shmsize += SHM_DATA_SIZE * INFO(i).data_count;

            if (INFO(i).shm_log == 1)
                Shmsize += SHM_LOG_SIZE * SHM_LOG_MAX;

            Shmptr = Shm_Attach(shm_key, &Mem_Shmid[i]);
            SHM_Mem[i] = (char *)Shmptr;

            Shmptr = Shm_Map_SubDaemon(Shmptr, &INFO(i), &Shm_Mem[i]);

            if (INFO(i).data_count != 0) {
                for (j = 0; j < INFO(i).data_count; j ++) {
                    Data_Ptr[j] = Shmptr;
                    Shmptr += SHM_DATA_SIZE;
                }

                for (k = j = 0; j < DAEMON(i).p_count; j ++) {
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
}   /* End of Mem_SHM ()    */

/*************************************************************************
    Function       : . attach sise SHM
    Parameters IN  : .
    Parameters OUT : .
    Return Code    : . void
*************************************************************************/
/*-----------------------------------------------------------------------*/
void    Sise_SHM(void)
/*-----------------------------------------------------------------------*/
{
    key_t   k;

    if (memcmp(_FEP_DIV, "TEST", 4) == 0)
        k = 0x01000000L;
    else
        k = 0x00000000L;

    /* 1. 채권    , 0x21000001L */
    Log(USR_OK, "01. SHM[%#x]", NOTE_SHM_KEY);
    Shm_Note    = (SHM_NOTE *)Shm_Attach(NOTE_SHM_KEY + k, &SISE_Note_Shmid);

    /* 2. 채권    , 0x21000001L */
    Log(USR_OK, "02. SHM[%#x]", RDS01_SHM_KEY);
    Shm_Rds01   = (CO_A001_RDS01 *)Shm_Attach(RDS01_SHM_KEY + k, &RDS_01_Shmid);

    /* 3. 금융파생, 0x21000002L */
    Log(USR_OK, "03. SHM[%#x]", FF_SHM_KEY);
    Shm_FinFut  = (SHM_FIN_FUT *)Shm_Attach(FF_SHM_KEY + k, &SISE_FF_Shmid);

    /* 12. 계좌별 전략 영역 , 0x21000012L */
    Log(USR_OK, "04. SHM[%#x]", STRRG_SHM_KEY);
    Shm_Strrg = (STRRG *)Shm_Attach(STRRG_SHM_KEY + k, &SISE_Strrg_Shmid);

    /* 13. 시장별 미체결관리 , 0x21000013L */
    Log(USR_OK, "05. SHM[%#x]", MK_PM_SHM_KEY);
    Shm_Mk_PreMatch = (MK_PREMATCH *)Shm_Attach(MK_PM_SHM_KEY + k, &SISE_Mk_Prematch_Shmid);

    /* 14. 한도관리영역 , 0x21000014L */
    Log(USR_OK, "06. SHM[%#x]", RISK_SHM_KEY);
    Shm_Risk = (RISK *)Shm_Attach(RISK_SHM_KEY + k, &SISE_Risk_Shmid);

    /* 30. item code qsort 처리용 , 0x21000030L */
    Log(USR_OK, "07. SHM[%#x]", ITEM_SHM_KEY);
    Shm_Item = (SHM_KEY_ARRY *)Shm_Attach(ITEM_SHM_KEY + k, &SISE_Item_Shmid);

    return;
}   /* End of Sise_SHM ()   */

/*************************************************************************
    Function        : . attach shared memory
    Parameters IN   : . p_key   : shared memory key
    Parameters OUT  : . p_id    : shared memory ID
    Return Code     : . char * (data segment start address of the attached
                        shared memory segment: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
char    *Shm_Attach(key_t p_key, int *p_id)
/*----------------------------------------------------------------------*/
{
    char    *ptr = (char *)-1;

    /* create shared memory - shmget    */
    *p_id = SHM_Creat (p_key, 0);

    if (*p_id == -1) {
        Log(SYS_FATAL, "Shm_Attach:cannot create SHM[%#x] {%d:%s}",
                p_key, SYS_NO, SYS_STR);
        Exit_Process();
    }

    /* attach shared memory - shmat */
    ptr = SHM_Attach(*p_id);

    if (ptr == (char *)-1) {
        Log(SYS_FATAL, "Shm_Attach:cannot attach SHM {%d:%s}",
                SYS_NO, SYS_STR);
        Exit_Process();
    }

    return (ptr);
}   /* End of Shm_Attach () */

/*************************************************************************
    Function        : . create and attach the shared memory
    Parameters IN   : . p_key   : SHM key
                      . p_size  : size of SHM segment
    Parameters OUT  : . p_id    : SHM id
    Return Code     : . char *
*************************************************************************/
/*----------------------------------------------------------------------*/
char    *SHM_Creat_Attach(key_t p_key, size_t p_size, int *p_id)
/*----------------------------------------------------------------------*/
{
    int     i, ret;
    char    cmd[256];
    char    *ptr = (char *)-1;

    i = 0;
    while (1) {
        i++;
        *p_id = SHM_Creat_Excl (p_key, p_size);

        if (*p_id == -1) {
            *p_id = SHM_Creat (p_key, 0);

            if (*p_id == -1) {
                Log(SYS_FATAL, "cannot create SHM[%#x,%d] {%d:%s}",
                        p_key, p_size, SYS_NO, SYS_STR);
                exit(FAIL);
            }

            if (i == 1) {
                SHM_Remove(*p_id);
            }
            if (i == 2) {
                memset(cmd, 0, sizeof (cmd));
                sprintf(cmd, "ipcrm -m %d", *p_id);
                ret = system(cmd);
                if (ret != 0) {
                    Log(SAM_WARN, "system() 명령어 실패: [%d][%s]", ret, cmd);
                }
                *p_id = SHM_Creat_Excl (p_key, p_size);
                Log(USR_OK, "system ipcrm [%s] rt[%d]", cmd, *p_id);
                sleep(1);
            }
            else if (i > 2)
                break;
            continue;
        }
        else
            break;
    }

    Log(SYS_OK, "SHM created[%d,%#x,%d]", *p_id, p_key, p_size);

    /* 2025FIX: verify actual SHM size before memset to prevent overrun */
    {
        struct shmid_ds shm_stat;
        if (shmctl(*p_id, IPC_STAT, &shm_stat) == 0) {
            if ((size_t)shm_stat.shm_segsz < p_size) {
                Log(SYS_FATAL,
                        "SHM size mismatch[%#x]: actual=%d requested=%d "
                        "(run ipcrm.sh and retry)",
                        p_key, (int)shm_stat.shm_segsz, (int)p_size);
                exit(FAIL);
            }
        }
    }

    ptr = SHM_Attach(*p_id);

    if (ptr == (char *)-1) {
        Log(SYS_FATAL, "cannot attach SHM[%d] {%d:%s}",
                *p_id, SYS_NO, SYS_STR);
        exit(FAIL);
    }

    memset(ptr, 0, p_size);

    return (ptr);
}   /* End of SHM_Creat_Attach ()   */

/*************************************************************************
    End of Program (shmsub.c)
*************************************************************************/
