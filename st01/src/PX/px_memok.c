#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : read configuration file and compare it with current shared memory
#   File    : px_memok.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
extern  void        Mem_SHM(int, int);
void    Daemon_Config_Read(void);
void    File_Config_Read(void);
void    Dshm_Config_Read(void);
#if defined ISAM_INCL
void    Cisam_Config_Read(void);
#endif
void    Tcp1_Config_Read(void);
void    Tcp2_Config_Read(void);
void    Udpip_Config_Read(void);
void    SiseTr_Config_Read(void);
void    Accno_Config_Read(void);
void    Proc_Config_Read(void);

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
char    tmpbuf[100];

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    Init_Mana(argc, argv);
    Mem_SHM(0, -1);

    File_Config_Read();
    Dshm_Config_Read();
#if defined ISAM_INCL
    Cisam_Config_Read();
#endif
    Tcp1_Config_Read();
    Tcp2_Config_Read();
    Udpip_Config_Read();
    SiseTr_Config_Read();
    Proc_Config_Read();

    exit(OK);
}   /* End of main ()   */

/*************************************************************************
    Function        : . read daemon.ini and check the daemon SHM
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Daemon_Config_Read(void)
/*----------------------------------------------------------------------*/
{
    int         c1 = '=';
    int         c2 = ' ';
    int         c3 = '\t';
    int         cnt = 'A';
    char        buf[100], tmp1[40], tmp2[40], tmp3[40];
    char        *sp, *sp1, *sp2, *sp3;
    FILE        *fp;

    sprintf(buf, "%s/daemon.ini", _FEP_CFG);
    if ((fp = fopen(buf, "r")) == 0) {
        printf("fopen failure[%s] {%d:%s}\n", buf, SYS_NO, SYS_STR);
        exit(FAIL);
    }

    memset(buf, 0, sizeof (buf));

    while ((sp = fgets(buf, sizeof (buf), fp)) != NULL) {
        buf[strlen(buf)-1] = 0;
        if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
            continue;

        sprintf(tmp3, "%s", "Daemon_End");
        if (memcmp(buf, tmp3, strlen(tmp3)) == 0) {
            cnt ++;
            continue;
        }

        sprintf(tmp3, "%s", "DAEMON_CONF_START");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0)
            continue;

        sprintf(tmp3, "%s", "DAEMON_CONF_END");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0)
            break;

        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            printf("daemon(%c):[%s]\n", cnt, buf);
            fclose(fp);
            exit(FAIL);
        }
        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1 + 1, c2);
        sp3 = strchr(sp1 + 1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)    { strncpy(tmp2, sp1+1, sizeof(tmp2) - 1); tmp2[sizeof(tmp2) - 1] = '\0'; }
            else                memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        sprintf(tmp3, "DAEMON_%c_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(DAEMON(cnt-'A').process_info, sp1+1,
                    strlen(sp1+1)) != 0)
            printf("%s [%s:%s]\n", tmp3,
                    DAEMON(cnt-'A').process_info, sp1+1);

            continue;
        }

        sprintf(tmp3, "DAEMON_%c_ID", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(DAEMON(cnt-'A').process_id, tmp2, strlen(tmp2)) != 0)
                printf("%s [%s:%s]\n", tmp3, DAEMON(cnt-'A').process_id, tmp2);

            continue;
        }

        sprintf(tmp3, "DAEMON_%c_START_TIME", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(DAEMON(cnt-'A').start_time, tmp2, strlen(tmp2)) != 0)
                printf("%s [%s:%s]\n", tmp3, DAEMON(cnt-'A').start_time, tmp2);

            continue;
        }

        sprintf(tmp3, "DAEMON_%c_END_TIME", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(DAEMON(cnt-'A').end_time, tmp2, strlen(tmp2)) != 0)
                printf("%s [%s:%s]\n", tmp3, DAEMON(cnt-'A').end_time, tmp2);

            continue;
        }

        sprintf(tmp3, "DAEMON_%c_DATE_FLAG", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (DAEMON(cnt-'A').date_flag != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, DAEMON(cnt-'A').date_flag, tmp2);

            continue;
        }

        sprintf(tmp3, "DAEMON_%c_COMPACT_DAYS", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (DAEMON(cnt-'A').compact_days != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3,
                        DAEMON(cnt-'A').compact_days, tmp2);

            continue;
        }

        sprintf(tmp3, "DAEMON_%c_STATUS", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (DAEMON(cnt-'A').process_status != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3,
                        DAEMON(cnt-'A').process_status, tmp2);

            continue;
        }

        sprintf(tmp3, "DAEMON_%c_FIFO", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(DAEMON(cnt-'A').start_FIFO_name,
                    tmp2, strlen(tmp2)) != 0)
            printf("%s [%s:%s]\n", tmp3,
                    DAEMON(cnt-'A').start_FIFO_name, tmp2);

            sprintf(tmpbuf, "%s_exit", tmp2);
            if (memcmp(DAEMON(cnt-'A').exit_FIFO_name,
                    tmp2, strlen(tmp2)) != 0)
            printf("%s [%s:%s]\n", tmp3,
                    DAEMON(cnt-'A').exit_FIFO_name, tmpbuf);

            sprintf(tmpbuf, "%s_ctrl", tmp2);
            if (memcmp(DAEMON(cnt-'A').daemon_FIFO_name,
                    tmp2, strlen(tmp2)) != 0)
            printf("%s [%s:%s]\n", tmp3,
                    DAEMON(cnt-'A').daemon_FIFO_name, tmpbuf);

            continue;
        }

        sprintf(tmp3, "DAEMON_%c_PROC_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (DAEMON(cnt-'A').process_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3,
                        DAEMON(cnt-'A').process_count, tmp2);

            continue;
        }

        sprintf(tmp3, "DAEMON_%c_FILE_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (DAEMON(cnt-'A').file_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, DAEMON(cnt-'A').file_count, tmp2);

            continue;
        }

#if defined ISAM_INCL
        sprintf(tmp3, "DAEMON_%c_CISAM_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (DAEMON(cnt-'A').cisam_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3,
                        DAEMON(cnt-'A').cisam_count, tmp2);

            continue;
        }
#endif

        sprintf(tmp3, "DAEMON_%c_TCP1_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (DAEMON(cnt-'A').tcp1_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, DAEMON(cnt-'A').tcp1_count, tmp2);

            continue;
        }

        sprintf(tmp3, "DAEMON_%c_TCP2_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (DAEMON(cnt-'A').tcp2_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, DAEMON(cnt-'A').tcp2_count, tmp2);

            continue;
        }

        sprintf(tmp3, "DAEMON_%c_UDPIP_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (DAEMON(cnt-'A').udpip_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, DAEMON(cnt-'A').udpip_count,
                        tmp2);

            continue;
        }

        sprintf(tmp3, "DAEMON_%c_DATA_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (DAEMON(cnt-'A').data_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, DAEMON(cnt-'A').data_count, tmp2);

            continue;
        }

        sprintf(tmp3, "DAEMON_%c_SISE_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (DAEMON(cnt-'A').sisetr_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, DAEMON(cnt-'A').sisetr_count,
                        tmp2);

            continue;
        }

        memset(buf, 0, sizeof (buf));
    }

    fclose(fp);

    return;
}   /* End of Daemon_Config_Read () */

/*************************************************************************
    Function        : . read file.ini and check the file SHM (FILEM)
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    File_Config_Read(void)
/*----------------------------------------------------------------------*/
{
    int         cnt, itemcnt;
    int         c1 = '=';
    int         c2 = ' ';
    int         c3 = '\t';
    int         p_cnt = 0;
    int         g_cnt = 'A';
    char        buf[100], tmp1[40], tmp2[40], tmp3[40];
    char        *sp, *sp1, *sp2, *sp3;
    FILE        *fp;

    sprintf(buf, "%s/file.ini", _FEP_CFG);
    if ((fp = fopen(buf, "r")) == 0) {
        printf("fopen failure[%s] {%d:%s}\n", buf, SYS_NO, SYS_STR);
        exit(FAIL);
    }

    memset(buf, 0, sizeof (buf));

    while ((sp = fgets(buf, sizeof (buf), fp)) != NULL) {
        buf[strlen(buf)-1] = 0;
        if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
            continue;

        sprintf(tmp3, "%s", "File_End");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            cnt ++;
            continue;
        }

        sprintf(tmp3, "%cA_CONF_START", _System_Name[0]);
        if (memcmp(buf+2, tmp3+2, Max(strlen(buf)-2,strlen(tmp3)-2)) == 0) {
            for (g_cnt = 'A'; g_cnt <= 'Z'; g_cnt ++) {
                sprintf(tmp3, "%c%c_CONF_START", _System_Name[0], g_cnt);
                if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
                    cnt = 1;
                    p_cnt = g_cnt - 'A';
                    break;
                }
            }

            continue;
        }

        sprintf(tmp3, "%c%c_CONF_END", _System_Name[0], g_cnt);
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0)
            continue;

        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            printf("file(%c):[%s]\n", g_cnt, buf);
            fclose(fp);
            exit(FAIL);
        }

        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1+1, c2);
        sp3 = strchr(sp1+1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)    { strncpy(tmp2, sp1+1, sizeof(tmp2) - 1); tmp2[sizeof(tmp2) - 1] = '\0'; }
            else                memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        sprintf(tmp3, "%s", "FILE_COUNT");
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            itemcnt = atoi(tmp2);

            if (DAEMON(p_cnt).f_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, DAEMON(p_cnt).f_count, tmp2);

            continue;
        }

        if (itemcnt < cnt)
            continue;

        sprintf(tmp3, "FILE_%d_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(FILEM(p_cnt,cnt-1).file_info,
                    sp1+1, strlen(sp1+1)) != 0)
            printf("%s [%s:%s]\n", tmp3,
                    FILEM(p_cnt,cnt-1).file_info, sp1+1);

            continue;
        }

        sprintf(tmp3, "FILE_%d_NAME", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(FILEM(p_cnt,cnt-1).file_name, tmp2, strlen(tmp2)) != 0)
                printf("%s [%s:%s]\n", tmp3,
                        FILEM(p_cnt,cnt-1).file_name, tmp2);

            continue;
        }

        sprintf(tmp3, "FILE_%d_FIFO", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (FILEM(p_cnt,cnt-1).fifo_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3,
                        FILEM(p_cnt,cnt-1).fifo_count, tmp2);

            continue;
        }

        sprintf(tmp3, "FILE_%d_SIZE", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (FILEM(p_cnt,cnt-1).record_size != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3,
                        FILEM(p_cnt,cnt-1).record_size, tmp2);

            continue;
        }

        memset(buf, 0, sizeof (buf));
    }

    fclose(fp);

    return;
}   /* End of File_Config_Read ()   */

/*************************************************************************
    Function        : . read dshm.ini and check the data SHM (DSHM)
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Dshm_Config_Read(void)
/*----------------------------------------------------------------------*/
{
    int         cnt, itemcnt;
    int         c1 = '=';
    int         c2 = ' ';
    int         c3 = '\t';
    int         p_cnt = 0;
    int         g_cnt = 'A';
    char        buf[100], tmp1[40], tmp2[40], tmp3[40];
    char        *sp, *sp1, *sp2, *sp3;
    FILE        *fp;

    sprintf(buf, "%s/dshm.ini", _FEP_CFG);
    if ((fp = fopen(buf, "r")) == 0) {
        printf("fopen failure[%s] {%d:%s}\n", buf, SYS_NO, SYS_STR);
        exit(FAIL);
    }

    memset(buf, 0, sizeof (buf));

    while ((sp = fgets(buf, sizeof (buf), fp)) != NULL) {
        buf[strlen(buf)-1] = 0;
        if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
            continue;

        sprintf(tmp3, "%s", "Dshm_End");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            cnt ++;
            continue;
        }

        sprintf(tmp3, "%cA_CONF_START", _System_Name[0]);
        if (memcmp(buf+2, tmp3+2, Max(strlen(buf)-2,strlen(tmp3)-2)) == 0) {
            for (g_cnt = 'A'; g_cnt <= 'Z'; g_cnt ++) {
                sprintf(tmp3, "%c%c_CONF_START", _System_Name[0], g_cnt);
                if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
                    cnt = 1;
                    p_cnt = g_cnt - 'A';
                    break;
                }
            }

            continue;
        }

        sprintf(tmp3, "%c%c_CONF_END", _System_Name[0], g_cnt);
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0)
            continue;

        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            printf("file(%c):[%s]\n", g_cnt, buf);
            fclose(fp);
            exit(FAIL);
        }

        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1+1, c2);
        sp3 = strchr(sp1+1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)    { strncpy(tmp2, sp1+1, sizeof(tmp2) - 1); tmp2[sizeof(tmp2) - 1] = '\0'; }
            else                memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        sprintf(tmp3, "%s", "DSHM_COUNT");
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            itemcnt = atoi(tmp2);

            if (DAEMON(p_cnt).d_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, DAEMON(p_cnt).d_count, tmp2);

            continue;
        }

        if (itemcnt < cnt)
            continue;

        sprintf(tmp3, "DSHM_%d_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(DSHM(p_cnt,cnt-1).info, sp1+1, strlen(sp1+1)) != 0)
                printf("%s [%s:%s]\n", tmp3, DSHM(p_cnt,cnt-1).info, sp1+1);

            continue;
        }

        sprintf(tmp3, "DSHM_%d_NAME", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(DSHM(p_cnt,cnt-1).data_name, tmp2, strlen(tmp2)) != 0)
                printf("%s [%s:%s]\n", tmp3,
                        DSHM(p_cnt,cnt-1).data_name, tmp2);

            continue;
        }

        sprintf(tmp3, "DSHM_%d_KEY", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(DSHM(p_cnt,cnt-1).key_info, tmp2, strlen(tmp2)) != 0)
                printf("%s [%s:%s]\n", tmp3, DSHM(p_cnt,cnt-1).key_info, tmp2);

            continue;
        }

        sprintf(tmp3, "DSHM_%d_FIFO", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (DSHM(p_cnt,cnt-1).fifo_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3,
                        DSHM(p_cnt,cnt-1).fifo_count, tmp2);

            continue;
        }

        sprintf(tmp3, "DSHM_%d_SIZE", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (DSHM(p_cnt,cnt-1).data_size != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3,
                        DSHM(p_cnt,cnt-1).data_size, tmp2);

            continue;
        }

        sprintf(tmp3, "DSHM_%d_MAX", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (DSHM(p_cnt,cnt-1).max_rec != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, DSHM(p_cnt,cnt-1).max_rec, tmp2);

            continue;
        }

        memset(buf, 0, sizeof (buf));
    }

    fclose(fp);

    return;
}   /* End of Dshm_Config_Read ()   */

#if defined ISAM_INCL
/*************************************************************************
    Function        : . read cisam.ini and check the file SHM (CISAM)
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Cisam_Config_Read(void)
/*----------------------------------------------------------------------*/
{
    int     cnt, i, j, itemcnt;
    int     c1 = '=';
    int     c2 = ' ';
    int     c3 = '\t';
    int     p_cnt = 0;
    int     g_cnt = 'A';
    char    buf[100], tmp1[40], tmp2[40], tmp3[40];
    char    *sp, *sp1, *sp2, *sp3;
    FILE    *fp;

    sprintf(buf, "%s/cisam.ini", _FEP_CFG);
    if ((fp = fopen(buf, "r")) == 0) {
        printf("fopen failure[%s] {%d:%s}\n", buf, SYS_NO, SYS_STR);
        exit(FAIL);
    }

    memset(buf, 0, sizeof (buf));

    while ((sp = fgets(buf, sizeof (buf), fp)) != NULL) {
        buf[strlen(buf)-1] = 0;
        if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
            continue;

        sprintf(tmp3, "%s", "Cisam_End");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            cnt ++;
            continue;
        }

        sprintf(tmp3, "%cA_CONF_START", _System_Name[0]);
        if (memcmp(buf+2, tmp3+2, Max(strlen(buf)-2,strlen(tmp3)-2)) == 0) {
            for (g_cnt = 'A'; g_cnt <= 'Z'; g_cnt ++) {
                sprintf(tmp3, "%c%c_CONF_START", _System_Name[0], g_cnt);
                if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
                    cnt = 1;
                    p_cnt = g_cnt - 'A';
                    break;
                }
            }

            continue;
        }

        sprintf(tmp3, "%c%c_CONF_END", _System_Name[0], g_cnt);
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0)
            continue;

        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            printf("cisam(%c):[%s]\n", g_cnt, buf);
            fclose(fp);
            exit(FAIL);
        }

        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1+1, c2);
        sp3 = strchr(sp1+1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)    { strncpy(tmp2, sp1+1, sizeof(tmp2) - 1); tmp2[sizeof(tmp2) - 1] = '\0'; }
            else                memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        sprintf(tmp3, "%s", "CISAM_COUNT");
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            itemcnt = atoi(tmp2);

            if (DAEMON(p_cnt).c_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, DAEMON(p_cnt).c_count, tmp2);

            continue;
        }

        if (itemcnt < cnt)
            continue;

        sprintf(tmp3, "CISAM_%d_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(CISAM(p_cnt,cnt-1).file_info, sp1+1,
                    strlen(sp1+1)) != 0)
            printf("%s [%s:%s]\n", tmp3,
                    CISAM(p_cnt,cnt-1).file_info, sp1+1);

            continue;
        }

        sprintf(tmp3, "CISAM_%d_NAME", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(CISAM(p_cnt,cnt-1).file_name, tmp2, strlen(tmp2)) != 0)
                printf("%s [%s:%s]\n", tmp3,
                        CISAM(p_cnt,cnt-1).file_name, tmp2);

            continue;
        }

        sprintf(tmp3, "CISAM_%d_KEY_SIZE", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (CISAM(p_cnt,cnt-1).key_size != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3,
                        CISAM(p_cnt,cnt-1).key_size, tmp2);

            continue;
        }

        sprintf(tmp3, "CISAM_%d_RECORD_SIZE", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (CISAM(p_cnt,cnt-1).record_size != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3,
                        CISAM(p_cnt,cnt-1).record_size, tmp2);

            continue;
        }

        memset(buf, 0, sizeof (buf));
    }

    fclose(fp);

    return;
}   /* End of Cisam_Config_Read ()  */
#endif

/*************************************************************************
    Function        : . read tcp1.ini and check the TCP/IP 1 SHM (TCP1)
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Tcp1_Config_Read(void)
/*----------------------------------------------------------------------*/
{
    int     i, j, cnt, itemcnt, continue_flag = 0;
    int     c1 = '=';
    int     c2 = ' ';
    int     c3 = '\t';
    int     p_cnt = 0;
    int     g_cnt = 'A';
    char    buf[100], tmp1[40], tmp2[40], tmp3[40];
    char    *sp, *sp1, *sp2, *sp3;
    FILE    *fp;

    sprintf(buf, "%s/tcp1.ini", _FEP_CFG);
    if ((fp = fopen(buf, "r")) == 0) {
        printf("fopen failure[%s] {%d:%s}\n", buf, SYS_NO, SYS_STR);
        exit(FAIL);
    }

    memset(buf, 0, sizeof (buf));

    while ((sp = fgets(buf, sizeof (buf), fp)) != NULL) {
        buf[strlen(buf)-1] = 0;
        if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
            continue;

        sprintf(tmp3, "%s", "Tcp1_End");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            cnt ++;
            continue;
        }

        sprintf(tmp3, "%cA_CONF_START", _System_Name[0]);
        if (memcmp(buf+2, tmp3+2, Max(strlen(buf)-2,strlen(tmp3)-2)) == 0) {
            for (g_cnt = 'A'; g_cnt <= 'Z'; g_cnt ++) {
                sprintf(tmp3, "%c%c_CONF_START", _System_Name[0], g_cnt);
                if (memcmp(buf, tmp3, strlen(tmp3)) == 0) {
                    cnt = 1;
                    p_cnt = g_cnt - 'A';
                    break;
                }
            }

            continue;
        }

        sprintf(tmp3, "%c%c_CONF_END", _System_Name[0], g_cnt);
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0)
            continue;

        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            printf("tcp1(%c):[%s]\n", g_cnt, buf);
            fclose(fp);
            exit(FAIL);
        }

        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1+1, c2);
        sp3 = strchr(sp1+1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)    { strncpy(tmp2, sp1+1, sizeof(tmp2) - 1); tmp2[sizeof(tmp2) - 1] = '\0'; }
            else                memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        sprintf(tmp3, "%s", "TCP1_COUNT");
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            itemcnt = atoi(tmp2);

            if (DAEMON(p_cnt).t1_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, DAEMON(p_cnt).t1_count, tmp2);

            continue;
        }

        if (itemcnt < cnt)
            continue;

        sprintf(tmp3, "TCP1_%d_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(TCP1(p_cnt,cnt-1).tcp_info,
                    sp1+1, strlen(sp1+1)) != 0)
            printf("%s [%s:%s]\n", tmp3,
                    TCP1(p_cnt,cnt-1).tcp_info, sp1+1);

            continue;
        }

        sprintf(tmp3, "TCP1_%d_IP", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            {
                struct in_addr tmp_ia;
                if (inet_pton(AF_INET, tmp2, &tmp_ia) != 1)
                    printf("tcp1(%c,%d):malformed IP address:%s[%s]\n",
                            g_cnt, cnt, tmp1, tmp2);
                else {
                    if (memcmp(TCP1(p_cnt,cnt-1).ip_addr, &tmp_ia, sizeof (tmp_ia)) != 0)
                        printf("%s [%d.%d.%d.%d:%s]\n", tmp3,
                                TCP1(p_cnt,cnt-1).ip_addr[0],
                                TCP1(p_cnt,cnt-1).ip_addr[1],
                                TCP1(p_cnt,cnt-1).ip_addr[2],
                                TCP1(p_cnt,cnt-1).ip_addr[3], tmp2);
                }
            }

            continue;
        }

        sprintf(tmp3, "TCP1_%d_PORT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (TCP1(p_cnt,cnt-1).port_no != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, TCP1(p_cnt,cnt-1).port_no, tmp2);

            continue;
        }

        continue_flag = 0;
        for (j = 0; j < 9; j ++) {
            sprintf(tmp3, "TCP1_%d_SPORT%02d", cnt, j + 1);
            if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                if (TCP1(p_cnt,cnt-1).service_port_no[j] != atoi(tmp2))
                    printf("%s [%d:%s]\n", tmp3,
                            TCP1(p_cnt,cnt-1).service_port_no[j], tmp2);

                continue_flag = 1;
                break;
            }
        }

        if (continue_flag == 1)
            continue;

        memset(buf, 0, sizeof (buf));
    }

    fclose(fp);

    return;
}   /* End of Tcp1_Config_Read ()   */

/*************************************************************************
    Function        : . read tcp2.ini and check the TCP/IP 2 SHM (TCP2)
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Tcp2_Config_Read(void)
/*----------------------------------------------------------------------*/
{
    int     i, cnt, itemcnt;
    int     c1 = '=';
    int     c2 = ' ';
    int     c3 = '\t';
    int     p_cnt = 0;
    int     g_cnt = 'A';
    char    buf[100], tmp1[40], tmp2[40], tmp3[40];
    char    *sp, *sp1, *sp2, *sp3;
    FILE    *fp;

    sprintf(buf, "%s/tcp2.ini", _FEP_CFG);
    if ((fp = fopen(buf, "r")) == 0) {
        printf("fopen failure[%s] {%d:%s}\n", buf, SYS_NO, SYS_STR);
        exit(FAIL);
    }

    memset(buf, 0, sizeof (buf));

    while ((sp = fgets(buf, sizeof (buf), fp)) != NULL) {
        buf[strlen(buf)-1] = 0;
        if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
            continue;

        sprintf(tmp3, "%s", "Tcp2_End");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            cnt ++;
            continue;
        }

        sprintf(tmp3, "%cA_CONF_START", _System_Name[0]);
        if (memcmp(buf+2, tmp3+2, Max(strlen(buf)-2,strlen(tmp3)-2)) == 0) {
            for (g_cnt = 'A'; g_cnt <= 'Z'; g_cnt ++) {
                sprintf(tmp3, "%c%c_CONF_START", _System_Name[0], g_cnt);
                if (memcmp(buf, tmp3, strlen(tmp3)) == 0) {
                    cnt = 1;
                    p_cnt = g_cnt - 'A';
                    break;
                }
            }

            continue;
        }

        sprintf(tmp3, "%c%c_CONF_END", _System_Name[0], g_cnt);
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0)
            continue;

        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            printf("tcp2(%c):[%s]\n", g_cnt, buf);
            fclose(fp);
            exit(FAIL);
        }

        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1+1, c2);
        sp3 = strchr(sp1+1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)        { strncpy(tmp2, sp1+1, sizeof(tmp2) - 1); tmp2[sizeof(tmp2) - 1] = '\0'; }
            else                    memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        sprintf(tmp3, "%s", "TCP2_COUNT");
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            itemcnt = atoi(tmp2);

            if (DAEMON(p_cnt).t2_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, DAEMON(p_cnt).t2_count, tmp2);

            continue;
        }

        if (itemcnt < cnt)
            continue;

        sprintf(tmp3, "TCP2_%d_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(TCP2(p_cnt,cnt-1).tcp_info,
                    sp1+1, strlen(sp1+1)) != 0)
            printf("%s [%s:%s]\n", tmp3,
                    TCP2(p_cnt,cnt-1).tcp_info, sp1+1);

            continue;
        }

        sprintf(tmp3, "TCP2_%d_IP", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            {
                struct in_addr tmp_ia;
                if (inet_pton(AF_INET, tmp2, &tmp_ia) != 1)
                    printf("tcp2(%c,%d):malformed IP address:%s[%s]\n",
                            g_cnt, cnt, tmp1, tmp2);
                else {
                    if (memcmp(TCP2(p_cnt,cnt-1).ip_addr, &tmp_ia, sizeof (tmp_ia)) != 0)
                        printf("%s [%d.%d.%d.%d:%s]\n", tmp3,
                                TCP2(p_cnt,cnt-1).ip_addr[0],
                                TCP2(p_cnt,cnt-1).ip_addr[1],
                                TCP2(p_cnt,cnt-1).ip_addr[2],
                                TCP2(p_cnt,cnt-1).ip_addr[3], tmp2);
                }
            }

            continue;
        }

        sprintf(tmp3, "TCP2_%d_DUP_ID", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (TCP2(p_cnt,cnt-1).dup_id != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, TCP2(p_cnt,cnt-1).dup_id, tmp2);

            continue;
        }

        sprintf(tmp3, "TCP2_%d_PORT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (TCP2(p_cnt,cnt-1).port_no != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, TCP2(p_cnt,cnt-1).port_no, tmp2);

            continue;
        }

        memset(buf, 0, sizeof (buf));
    }

    fclose(fp);

    return;
}   /* End of Tcp2_Config_Read ()   */

/*************************************************************************
    Function        : . read udpip.ini and check the UDP SHM (UDPIP)
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Udpip_Config_Read(void)
/*----------------------------------------------------------------------*/
{
    int     i, j, cnt, itemcnt, flag;
    int     c1 = '=';
    int     c2 = ' ';
    int     c3 = '\t';
    int     p_cnt = 0;
    int     g_cnt = 'A';
    char    buf[100], tmp1[40], tmp2[40], tmp3[40];
    char    *sp, *sp1, *sp2, *sp3;
    FILE    *fp;

    sprintf(buf, "%s/udpip.ini", _FEP_CFG);
    if ((fp = fopen(buf, "r")) == 0) {
        printf("fopen failure[%s] {%d:%s}\n", buf, SYS_NO, SYS_STR);
        exit(FAIL);
    }

    memset(buf, 0, sizeof (buf));

    while ((sp = fgets(buf, sizeof (buf), fp)) != NULL) {
        buf[strlen(buf)-1] = 0;
        if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
            continue;

        sprintf(tmp3, "%s", "Udpip_End");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            cnt ++;
            continue;
        }

        sprintf(tmp3, "%cA_CONF_START", _System_Name[0]);
        if (memcmp(buf+2, tmp3+2, Max(strlen(buf)-2,strlen(tmp3)-2)) == 0) {
            for (g_cnt = 'A'; g_cnt <= 'Z'; g_cnt ++) {
                sprintf(tmp3, "%c%c_CONF_START", _System_Name[0], g_cnt);
                if (memcmp(buf, tmp3, strlen(tmp3)) == 0) {
                    cnt = 1;
                    p_cnt = g_cnt - 'A';
                    break;
                }
            }

            continue;
        }

        sprintf(tmp3, "%c%c_CONF_END", _System_Name[0], g_cnt);
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0)
            continue;

        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            printf("udpip(%c):[%s]\n", g_cnt, buf);
            fclose(fp);
            exit(FAIL);
        }

        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1+1, c2);
        sp3 = strchr(sp1+1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)        { strncpy(tmp2, sp1+1, sizeof(tmp2) - 1); tmp2[sizeof(tmp2) - 1] = '\0'; }
            else                    memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        sprintf(tmp3, "%s", "UDPIP_COUNT");
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            itemcnt = atoi(tmp2);

            if (DAEMON(p_cnt).u_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, DAEMON(p_cnt).u_count, tmp2);

            continue;
        }

        if (itemcnt < cnt)
            continue;

        sprintf(tmp3, "UDPIP_%d_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(UDPIP(p_cnt,cnt-1).info, sp1+1, strlen(sp1+1)) != 0)
                printf("%s [%s:%s]\n", tmp3,
                        UDPIP(p_cnt,cnt-1).info, sp1+1);

            continue;
        }

        sprintf(tmp3, "UDPIP_%d_IP_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            flag = 0;

            for (j = 0; j < 20; j ++) {
                sprintf(tmp3, "UDPIP_%d_IP_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    {
                        struct in_addr tmp_ia;
                        if (inet_pton(AF_INET, tmp2, &tmp_ia) != 1)
                            printf("udpip(%c,%d):malformed IP address:%s[%s]\n",
                                    g_cnt, cnt, tmp1, tmp2);
                        else {
                            if (memcmp(UDPIP(p_cnt,cnt-1).ip_addr[j], &tmp_ia,
                                    sizeof (tmp_ia)) != 0)
                            printf("%s [%d.%d.%d.%d:%s]\n", tmp3,
                                    UDPIP(p_cnt,cnt-1).ip_addr[j][0],
                                    UDPIP(p_cnt,cnt-1).ip_addr[j][1],
                                    UDPIP(p_cnt,cnt-1).ip_addr[j][2],
                                    UDPIP(p_cnt,cnt-1).ip_addr[j][3], tmp2);
                        }
                    }

                    flag = 1;
                    break;
                }
            }

            if (flag == 1)
                continue;
        }

        sprintf(tmp3, "UDPIP_%d_DUP_ID", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (UDPIP(p_cnt,cnt-1).dup_id != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, UDPIP(p_cnt,cnt-1).dup_id, tmp2);

            continue;
        }

        sprintf(tmp3, "UDPIP_%d_PORT_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            flag = 0;

            for (j = 0; j < 20; j ++) {
                sprintf(tmp3, "UDPIP_%d_PORT_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    if (UDPIP(p_cnt,cnt-1).port[j] != atoi(tmp2))
                        printf("%s [%d:%s]\n",
                                tmp3, UDPIP(p_cnt,cnt-1).port[j], tmp2);

                    flag = 1;
                    break;
                }
            }

            if (flag == 1)
                continue;
        }

        memset(buf, 0, sizeof (buf));
    }

    fclose(fp);

    return;
}   /* End of Udpip_Config_Read ()  */

/*************************************************************************
    Function        : . read sisetr.ini and check the SISE TR SHM (SISETR)
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*-----------------------------------------------------------------------*/
void    SiseTr_Config_Read(void)
/*-----------------------------------------------------------------------*/
{
    int     cnt, itemcnt;
    int     c1 = '=';
    int     c2 = ' ';
    int     c3 = '\t';
    int     p_cnt = 0;
    int     g_cnt = 'A';
    char    buf[100], tmp1[40], tmp2[40], tmp3[40];
    char    *sp, *sp1, *sp2, *sp3;
    FILE    *fp;

    sprintf(buf, "%s/sisetr.ini", _FEP_CFG);
    if ((fp = fopen(buf, "r")) == 0) {
        printf("fopen failure[%s] {%d:%s}\n", buf, SYS_NO, SYS_STR);
        exit(FAIL);
    }

    memset(buf, 0, sizeof (buf));

    while ((sp = fgets(buf, sizeof (buf), fp)) != NULL) {
        buf[strlen(buf)-1] = 0;
        if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
            continue;

        sprintf(tmp3, "%s", "Sise_End");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            cnt++;
            continue;
        }

        sprintf(tmp3, "%cA_CONF_START", _System_Name[0]);
        if (memcmp(buf+2, tmp3+2, Max(strlen(buf)-2,strlen(tmp3)-2)) == 0) {
            for (g_cnt = 'A'; g_cnt <= 'Z'; g_cnt ++) {
                sprintf(tmp3, "%c%c_CONF_START", _System_Name[0], g_cnt);
                if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
                    cnt = 1;
                    p_cnt = g_cnt - 'A';
                    break;
                }
            }

            continue;
        }

        sprintf(tmp3, "%c%c_CONF_END", _System_Name[0], g_cnt);
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0)
            continue;

        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            printf("sisetr(%c):[%s]\n", g_cnt, buf);
            fclose(fp);
            exit(FAIL);
        }

        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1+1, c2);
        sp3 = strchr(sp1+1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)    { strncpy(tmp2, sp1+1, sizeof(tmp2) - 1); tmp2[sizeof(tmp2) - 1] = '\0'; }
            else                memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        sprintf(tmp3, "%s", "SISE_COUNT");
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            itemcnt = atoi(tmp2);

            if (DAEMON(p_cnt).s_count != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, DAEMON(p_cnt).s_count, tmp2);

            continue;
        }

        if (itemcnt < cnt)
            continue;

        sprintf(tmp3, "SISE_%d_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(SISETR(p_cnt,cnt-1).tr_info, sp1+1,
                    strlen(sp1+1)) != 0)
            printf("%s [%s:%s]\n", tmp3,
                    SISETR(p_cnt,cnt-1).tr_info, sp1+1);

            continue;
        }

        sprintf(tmp3, "SISE_%d_TR", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(SISETR(p_cnt,cnt-1).tr, tmp2, strlen(tmp2)) != 0)
                printf("%s [%s:%s]\n", tmp3, SISETR(p_cnt,cnt-1).tr, tmp2);

            continue;
        }

        sprintf(tmp3, "SISE_%d_LENGTH", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (SISETR(p_cnt,cnt-1).length != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, SISETR(p_cnt,cnt-1).length, tmp2);
            continue;
        }

        sprintf(tmp3, "SISE_%d_QUEUE", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (SISETR(p_cnt,cnt-1).queue != atoi(tmp2))
                printf("%s [%d:%s]\n", tmp3, SISETR(p_cnt,cnt-1).queue, tmp2);

            continue;
        }

        memset(buf, 0, sizeof (buf));
    }

    fclose(fp);

    return;
}   /* End of SiseTr_Config_Read () */

/*************************************************************************
    Function        : . read proc.ini and check the process SHM (PROC)
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Proc_Config_Read(void)
/*----------------------------------------------------------------------*/
{
    int     len, pt, fd, rt, hh, mm, i, j, cnt, flag;
    int     itemcnt, l1, l2, sub_flag, start_ss, end_ss, ss;
    int     c1 = '=';
    int     c2 = ' ';
    int     c3 = '\t';
    int     p_cnt = 0;
    int     g_cnt = 'A';
    short   ip_tmp[4], dupid;
    char    buf[100], tmp1[40], tmp2[40], tmp3[40], n1[40], n2[40];
    char    file_name[100], stat_buf[60], ppath[100], bumun[4], d_time[16];
    char    dir_name[128];
    char    *sp, *sp1, *sp2, *sp3;
    FILE    *fp;

    dupid = 0;

    sprintf(buf, "%s/proc.ini", _FEP_CFG);
    if ((fp = fopen(buf, "r")) == 0) {
        printf("fopen failure[%s] {%d:%s}\n", buf, SYS_NO, SYS_STR);
        exit(FAIL);
    }

    memset(buf, 0, sizeof (buf));

    while ((sp = fgets(buf, sizeof (buf), fp)) != NULL) {
        buf[strlen(buf)-1] = 0;
        if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
            continue;

        sprintf(tmp3, "%s", "Proc_End");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            cnt ++;
            continue;
        }

        sprintf(tmp3, "%cA_CONF_START", _System_Name[0]);
        if (memcmp(buf+2, tmp3+2, Max(strlen(buf)-2,strlen(tmp3)-2)) == 0) {
            for (g_cnt = 'A'; g_cnt <= 'Z'; g_cnt ++) {
                sprintf(tmp3, "%c%c_CONF_START", _System_Name[0], g_cnt);
                if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
                    cnt = 1;
                    p_cnt = g_cnt - 'A';
                    break;
                }
            }

            continue;
        }

        sprintf(tmp3, "%c%c_CONF_END", _System_Name[0], g_cnt);
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0)
            continue;

        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            printf("proc(%c):[%s]\n", g_cnt, buf);
            fclose(fp);
            exit(FAIL);
        }

        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1+1, c2);
        sp3 = strchr(sp1+1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)    { strncpy(tmp2, sp1+1, sizeof(tmp2) - 1); tmp2[sizeof(tmp2) - 1] = '\0'; }
            else                memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        sprintf(tmp3, "%s", "PROC_COUNT");
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            itemcnt = atoi(tmp2);

            if (DAEMON(p_cnt).p_count != atoi(tmp2))
                printf("%c %s [%d:%s]\n", g_cnt, tmp3,
                        DAEMON(p_cnt).p_count, tmp2);

            continue;
        }

        if (itemcnt < cnt)
            continue;

        sprintf(tmp3, "PROC_%d_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(PROC(p_cnt,cnt-1).process_info, sp1+1,
                    strlen(sp1+1)) != 0)
            printf("%c %s [%s:%s]\n", g_cnt, tmp3,
                    PROC(p_cnt,cnt-1).process_info, sp1+1);

            continue;
        }

        sprintf(tmp3, "PROC_%d_ID", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(PROC(p_cnt,cnt-1).process_id,
                    tmp2, strlen(tmp2)) != 0)
            printf("%c %s [%s:%s]\n", g_cnt, tmp3,
                    PROC(p_cnt,cnt-1).process_id, tmp2);

            continue;
        }

        sprintf(tmp3, "PROC_%d_IFN_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            flag = 0;

            for (j = 0; j < 3; j ++) {
                sprintf(tmp3, "PROC_%d_IFN_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    for (i = 0; i < DAEMON(p_cnt).f_count; i ++) {
                        if (memcmp(tmp2, FILEM(p_cnt,i).file_name,
                                Max(strlen(tmp2),
                                strlen(FILEM(p_cnt,i).file_name))) == 0) {
                            if (PROC(p_cnt,cnt-1).in_f[j] != i + 1)
                                printf("%c %s [%d:%d]\n", g_cnt, tmp3,
                                        PROC(p_cnt,cnt-1).in_f[j], i + 1);

                            break;
                        }

                        if (i == DAEMON(p_cnt).f_count - 1)
                            printf("proc(%c,%d):IFN%d:%s[%s]\n",
                                    g_cnt, cnt, j + 1, tmp1, tmp2);
                    }

                    flag = 1;
                    break;
                }
            }

            if (flag == 1)
                continue;
        }

        sprintf(tmp3, "PROC_%d_IDN_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            flag = 0;

            for (j = 0; j < 3; j ++) {
                sprintf(tmp3, "PROC_%d_IDN_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    for (i = 0; i < DAEMON(p_cnt).d_count; i ++) {
                        if (memcmp(tmp2, DSHM(p_cnt,i).data_name,
                                Max(strlen(tmp2),
                                strlen(DSHM(p_cnt,i).data_name))) == 0) {
                            if (PROC(p_cnt,cnt-1).in_d[j] != i + 1)
                                printf("%c %s [%d:%d]\n", g_cnt, tmp3,
                                        PROC(p_cnt,cnt-1).in_d[j], i + 1);

                            break;
                        }

                        if (i == DAEMON(p_cnt).d_count - 1)
                            printf("proc(%c,%d):IDN%d:%s[%s]\n",
                                    g_cnt, cnt, j + 1, tmp1, tmp2);
                    }

                    flag = 1;
                    break;
                }
            }

            if (flag == 1)
                continue;
        }

        sprintf(tmp3, "PROC_%d_FFN_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            flag = 0;

            for (j = 0; j < 3; j ++) {
                sprintf(tmp3, "PROC_%d_FFN_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    if (memcmp(PROC(p_cnt,cnt-1).fifo_f[j], tmp2,
                            strlen(tmp2)) != 0)
                    printf("%c %s [%s:%s]\n", g_cnt, tmp3,
                            PROC(p_cnt,cnt-1).fifo_f[j], tmp2);

                    flag = 1;
                    break;
                }
            }

            if (flag == 1)
                continue;
        }

#if defined ISAM_INCL
        sprintf(tmp3, "PROC_%d_ICN_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            flag = 0;

            for (j = 0; j < 3; j ++) {
                sprintf(tmp3, "PROC_%d_ICN_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    for (i = 0; i < DAEMON(p_cnt).c_count; i ++) {
                        if (memcmp(tmp2, CISAM(p_cnt,i).file_name,
                                Max(strlen(tmp2),
                                strlen(CISAM(p_cnt,i).file_name))) == 0) {
                            if (PROC(p_cnt,cnt-1).in_c[j] != i + 1)
                                printf("%c %s [%d:%d]\n", g_cnt, tmp3,
                                        PROC(p_cnt,cnt-1).in_c[j], i + 1);

                            break;
                        }

                        if (i == DAEMON(p_cnt).c_count - 1)
                            printf("proc(%c,%d):ICN%d:%s[%s]\n",
                                    g_cnt, cnt, j + 1, tmp1, tmp2);
                    }

                    flag = 1;
                    break;
                }
            }

            if (flag == 1)
                continue;
        }
#endif

        sprintf(tmp3, "PROC_%d_OFN_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            flag = 0;

            for (j = 0; j < 99; j ++) {
                sprintf(tmp3, "PROC_%d_OFN_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    for (i = 0; i < DAEMON(p_cnt).f_count; i ++) {
                        if (memcmp(tmp2, FILEM(p_cnt,i).file_name,
                                Max(strlen(tmp2),
                                strlen(FILEM(p_cnt,i).file_name))) == 0) {
                            if (PROC(p_cnt,cnt-1).out_f[j] != i + 1)
                                printf("%c %s [%d:%d]\n", g_cnt, tmp3,
                                        PROC(p_cnt,cnt-1).out_f[j], i + 1);

                            break;
                        }

                        if (i == DAEMON(p_cnt).f_count - 1)
                            printf("proc(%c,%d):OFN%d:%s[%s]\n",
                                    g_cnt, cnt, j + 1, tmp1, tmp2);
                    }

                    flag = 1;
                    break;
                }
            }

            if (flag == 1)
                continue;
        }

        sprintf(tmp3, "PROC_%d_ODN_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            flag = 0;

            for (j = 0; j < 99; j ++) {
                sprintf(tmp3, "PROC_%d_ODN_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    for (i = 0; i < DAEMON(p_cnt).d_count; i ++) {
                        if (memcmp(tmp2, DSHM(p_cnt,i).data_name,
                                Max(strlen(tmp2),
                                strlen(DSHM(p_cnt,i).data_name))) == 0) {
                            if (PROC(p_cnt,cnt-1).out_d[j] != i + 1)
                                printf("%c %s [%d:%d]\n", g_cnt, tmp3,
                                        PROC(p_cnt,cnt-1).out_d[j], i + 1);

                            break;
                        }

                        if (i == DAEMON(p_cnt).d_count - 1)
                            printf("proc(%c,%d):ODN%d:%s[%s]\n",
                                    g_cnt, cnt, j + 1, tmp1, tmp2);
                    }

                    flag = 1;
                    break;
                }
            }

            if (flag == 1)
                continue;
        }

#if defined ISAM_INCL
        sprintf(tmp3, "PROC_%d_OCN_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            flag = 0;

            for (j = 0; j < 3; j ++) {
                sprintf(tmp3, "PROC_%d_OCN_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    for (i = 0; i < DAEMON(p_cnt).c_count; i ++) {
                        if (memcmp(tmp2, CISAM(p_cnt,i).file_name,
                                Max(strlen(tmp2),
                                strlen(FILEM(p_cnt,i).file_name))) == 0) {
                            if (PROC(p_cnt,cnt-1).out_c[j] != i + 1)
                                printf("%c %s [%d:%d]\n", g_cnt, tmp3,
                                        PROC(p_cnt,cnt-1).out_c[j], i + 1);

                            break;
                        }

                        if (i == DAEMON(p_cnt).c_count - 1)
                            printf("proc(%c,%d):OCN%d:%s[%s]\n",
                                    g_cnt, cnt, j + 1, tmp1, tmp2);
                    }

                    flag = 1;
                    break;
                }
            }

            if (flag == 1)
                continue;
        }
#endif

        sprintf(tmp3, "PROC_%d_TYPE", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (tmp2[0] == 'M') {
                if (PROC(p_cnt,cnt-1).type != TY_MP)
                    printf("%s [%c:%d]\n", tmp3,
                            PROC(p_cnt,cnt-1).type, TY_MP);
            }
            else if (tmp2[0] == 'D') {
                if (PROC(p_cnt,cnt-1).type != TY_DD)
                    printf("%c %s [%c:%d]\n", g_cnt, tmp3,
                            PROC(p_cnt,cnt-1).type, TY_DD);
            }
            else if (tmp2[0] == 'T' && tmp2[2] == '1') {
                if (PROC(p_cnt,cnt-1).type != TY_TRS1)
                    printf("%c %s [%c:%d]\n", g_cnt, tmp3,
                            PROC(p_cnt,cnt-1).type, TY_TRS1);
            }
            else if (tmp2[0] == 'T' && tmp2[2] == '2') {
                if (PROC(p_cnt,cnt-1).type != TY_TRS2)
                    printf("%c %s [%c:%d]\n", g_cnt, tmp3,
                            PROC(p_cnt,cnt-1).type, TY_TRS2);
            }
            else if (tmp2[0] == 'B') {
                if (PROC(p_cnt,cnt-1).type != TY_BRS)
                    printf("%c %s [%c:%d]\n", g_cnt, tmp3,
                            PROC(p_cnt,cnt-1).type, TY_BRS);
            }

            continue;
        }

        sprintf(tmp3, "PROC_%d_START_TIME", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(PROC(p_cnt,cnt-1).start_time, tmp2, strlen(tmp2)) != 0)
                printf("%c %s [%s:%s]\n", g_cnt, tmp3,
                        PROC(p_cnt,cnt-1).start_time, tmp2);

            continue;
        }

        sprintf(tmp3, "PROC_%d_END_TIME", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (memcmp(PROC(p_cnt,cnt-1).end_time, tmp2, strlen(tmp2)) != 0)
                printf("%c %s [%s:%s]\n", g_cnt, tmp3,
                        PROC(p_cnt,cnt-1).end_time, tmp2);

            continue;
        }

        sprintf(tmp3, "PROC_%d_DATA_BUF", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (tmp2[0] == 'Y' && Data_I < DATA_BUF_CNT) {
                if (PROC(p_cnt,cnt-1).data_flag != 1)
                    printf("%c %s [%c:%s]\n", g_cnt, tmp3,
                            PROC(p_cnt,cnt-1).data_flag, tmp2);
            }

            continue;
        }

        sprintf(tmp3, "PROC_%d_TCP1_TYPE", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (PROC(p_cnt,cnt-1).type == TY_TRS1) {
                if (tmp2[0] == 'M') {
                    if (PROC(p_cnt,cnt-1).l.t1.port_type != 0)
                        printf("%c %s [%d:0]\n", g_cnt, tmp3,
                                PROC(p_cnt,cnt-1).l.t1.port_type);
                }
                else if (tmp2[0] == 'S') {
                    if (PROC(p_cnt,cnt-1).l.t1.port_type < 1)
                        printf("%c %s [%d:1~9]\n", g_cnt, tmp3,
                                PROC(p_cnt,cnt-1).l.t1.port_type);
                }
            }

            continue;
        }

        sprintf(tmp3, "PROC_%d_TCP1_PORT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            rt = NOTOK;

            if (PROC(p_cnt,cnt-1).type == TY_TRS1) {
                for (i = 0; i < DAEMON(p_cnt).t1_count; i ++) {
                    if (PROC(p_cnt,cnt-1).l.t1.port_type == 0) {
                        if (atoi(tmp2) == TCP1(p_cnt,i).port_no) {
                            rt = OK;
                            break;
                        }
                    }
                    else {
                        for (j = 0; j < 9; j ++) {
                            if (atoi(tmp2) ==
                                    TCP1(p_cnt,i).service_port_no[j]) {
                                rt = OK;
                                break;
                            }
                        }
                    }
                }

                if (rt == NOTOK) {
                    if (PROC(p_cnt,cnt-1).l.t1.port_type == 1)
                        printf("%c %s [%d:%s]\n", g_cnt, tmp3,
                                TCP1_PORT(p_cnt,cnt-1), tmp2);
                    else
                        printf("%c %s [%d:%s]\n", g_cnt, tmp3,
                                TCP1_PORT(p_cnt,cnt-1), tmp2);
                }
            }

            continue;
        }

        sprintf(tmp3, "PROC_%d_TCP2_PRIMARY", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (PROC(p_cnt,cnt-1).type == TY_TRS2) {
                for (j = 0; j < strlen(tmp2) && tmp2[j] != ','; j ++)
                    ;

                for (i = 0; i < DAEMON(p_cnt).t2_count; i ++) {
                    if (AtoIf(tmp2, j) == TCP2(p_cnt,i).dup_id &&
                            atoi(&tmp2[j+1]) == TCP2(p_cnt,i).port_no) {
                        if (PROC(p_cnt,cnt-1).l.t2.l[0] != i + 1)
                            printf("%c %s [%d:%d]\n", g_cnt, tmp3,
                                    PROC(p_cnt,cnt-1).l.t2.l[0], i + 1);

                        break;
                    }
                }
            }

            continue;
        }

        sprintf(tmp3, "PROC_%d_TCP2_BACKUP", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (PROC(p_cnt,cnt-1).type == TY_TRS2) {
                for (j = 0; j < strlen(tmp2) && tmp2[j] != ','; j ++)
                    ;

                for (i = 0; i < DAEMON(p_cnt).t2_count; i ++) {
                    if (AtoIf(tmp2, j) == TCP2(p_cnt,i).dup_id &&
                            atoi(&tmp2[j+1]) == TCP2(p_cnt,i).port_no) {
                        if (PROC(p_cnt,cnt-1).l.t2.l[1] != i + 1)
                            printf("%c %s [%d:%d]\n", g_cnt, tmp3,
                                    PROC(p_cnt,cnt-1).l.t2.l[1], i + 1);

                        break;
                    }
                }
            }

            continue;
        }

        sprintf(tmp3, "PROC_%d_UDP_PORT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (PROC(p_cnt,cnt-1).type == TY_URS) {
                for (j = 0; j < strlen(tmp2) && tmp2[j] != ','; j ++)
                    ;

                for (i = 0; i < DAEMON(p_cnt).u_count; i ++) {
                    if (AtoIf(tmp2, j) == UDPIP(p_cnt,i).dup_id &&
                            atoi(&tmp2[j+1]) == UDPIP(p_cnt,i).port[0]) {
                        if (PROC(p_cnt,cnt-1).l.u != i + 1)
                            printf("%c %s [%d:%d]\n", g_cnt, tmp3,
                                    PROC(p_cnt,cnt-1).l.u, i + 1);

                        break;
                    }
                }
            }

            continue;
        }

        sprintf(tmp3, "PROCESS_%d_DUP_ID", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            dupid = atoi(tmp2);
            continue;
        }

        memset(buf, 0, sizeof (buf));
    }

    fclose(fp);

    return;
}   /* End of Proc_Config_Read ()   */

/*************************************************************************
    End of Program (px_memok.c)
*************************************************************************/
