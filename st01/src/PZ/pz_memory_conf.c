#if !defined(_LARGE_FILES)
#define     _LARGE_FILES
#endif
/*------------------------------------------------------------------------
#   Module  : read configuration files and load shared memory
#   File    : pz_memory_conf.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "daemon.h"

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    Daemon_Config_Read(int);
void    File_Config_Read(int);
void    Dshm_Config_Read(int);
#if defined ISAM_INCL
void    Cisam_Config_Read(int);
void    Create_Cisam(int, int);
#endif
void    Tcp1_Config_Read(int);
void    Tcp2_Config_Read(int);
void    Udpip_Config_Read(int);
/* 202201
void    SiseTr_Config_Read (int);
void    Accno_Config_Read (int);
*/
void    Proc_Config_Read(int);

/*************************************************************************
    Function        : . read daemon.ini and set the daemon SHM
    Parameters IN   : . flag (0:set daemon SHM (INFO),
                              1:set sub daemon SHM (DAEMON),
                              2:set temporary daemon buffer (Info),
                              3:daemon SHM (INFO)�� �� �ι��� set)
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Daemon_Config_Read(int flag)
/*----------------------------------------------------------------------*/
{
    int         hh, mm, tmp_ss, start_ss, end_ss, sub_flag;
    int         c1 = '=';
    int         c2 = ' ';
    int         c3 = '\t';
    int         cnt = 'A';
    char        buf[1024], file_name[256], dt[10], bumun[4];
    char        tmp1[40], tmp2[40], tmp3[40];
    char        *sp, *sp1, *sp2, *sp3;
    FILE        *fp;
    time_t      t;
    struct tm   *tp, *date;

    sub_flag = 0;

    sprintf(buf, "%s/daemon.ini", _FEP_CFG);
    if ((fp = fopen(buf, "r")) == NULL) {
        Log(SAM_FATAL, "fopen failure[%s] {%d:%s}", buf, SYS_NO, SYS_STR);
        sleep(3);
        exit(FAIL);
    }

    memset(buf, 0, sizeof (buf));

    while ((sp = fgets(buf, sizeof (buf), fp)) != NULL) {
        buf[strlen(buf)-1] = 0;
        if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
            continue;

        sprintf(tmp3, "%s", "Daemon_End");
        if (memcmp(buf, tmp3, strlen(tmp3)) == 0) {
            if (sub_flag == 1) {
                if (flag == 1) {
                    INFO(cnt-'A').process_count = DAEMON(cnt-'A').process_count;
                    INFO(cnt-'A').file_count = DAEMON(cnt-'A').file_count;
                    INFO(cnt-'A').dshm_count = DAEMON(cnt-'A').dshm_count;
#if defined ISAM_INCL
                    INFO(cnt-'A').cisam_count = DAEMON(cnt-'A').cisam_count;
#endif
                    INFO(cnt-'A').tcp1_count = DAEMON(cnt-'A').tcp1_count;
                    INFO(cnt-'A').tcp2_count = DAEMON(cnt-'A').tcp2_count;
                    INFO(cnt-'A').udpip_count = DAEMON(cnt-'A').udpip_count;
                    /* 202201
                                        INFO(cnt-'A').sisetr_count = DAEMON(cnt-'A').sisetr_count;
                                        INFO(cnt-'A').accno_count = DAEMON(cnt-'A').accno_count;
                    */
                    INFO(cnt-'A').data_count = DAEMON(cnt-'A').data_count;
                }
                Log(USR_OK, "... end daemon config");
                break;
            }
            cnt ++;
            continue;
        }

        sprintf(tmp3, "%s", "DAEMON_CONF_START");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            Log(USR_OK, "start daemon config ...");
            continue;
        }

        sprintf(tmp3, "%s", "DAEMON_CONF_END");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            Log(USR_OK, "... end daemon config");
            break;
        }

        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            Log(USR_FATAL, "daemon(%c):[%s] len(%d) line(%d)", cnt, buf, (int)strlen(buf), __LINE__);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }
        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1 + 1, c2);
        sp3 = strchr(sp1 + 1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)    strncpy(tmp2, sp1+1, strlen(sp1)-1);
        else                memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        sprintf(tmp3, "DAEMON_%c_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if ((flag == 1 || flag == 3) && D_K == cnt - 'A') {
                if (flag == 1) {
                    sprintf(DAEMON(cnt-'A').process_info, "%s", sp1 + 1);
                    DAEMON(cnt-'A').system_status = ON;
                    INFO(cnt-'A').system_status = ON;
                }
                sub_flag = 1;
            }
            continue;
        }

        sprintf(tmp3, "DAEMON_%c_ID", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A')) {
                /* 2025EDIT,START */
                strncpy(INFO(cnt-'A').process_id, tmp2, sizeof(INFO(cnt-'A').process_id));
                /* 2025EDIT,END */
            }
            else if (flag == 1 && D_K == cnt - 'A') {
                /* 2025EDIT,START */
                strncpy(DAEMON(cnt-'A').process_id, tmp2, sizeof(DAEMON(cnt-'A').process_id));
                /* 2025EDIT,END */
                sprintf(DAEMON(cnt-'A').process_path, "%s/%s", _FEP_BIN, tmp2);
                /* 2025EDIT,START */
                strncpy(INFO(cnt-'A').process_id, tmp2, sizeof(INFO(cnt-'A').process_id));
                /* 2025EDIT,END */
            }
            continue;
        }

        sprintf(tmp3, "DAEMON_%c_START_TIME", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A')) {
                /* 2025EDIT */
                strncpy(INFO(cnt-'A').start_time, tmp2, sizeof(INFO(cnt-'A').start_time));
                /* 2025EDIT */
            }
            else if (flag == 1 && D_K == cnt - 'A') {
                /* 2025EDIT */
                strncpy(DAEMON(cnt-'A').start_time, tmp2, sizeof(DAEMON(cnt-'A').start_time));
                strncpy(INFO(cnt-'A').start_time,  tmp2, sizeof(INFO(cnt-'A').start_time));
                /* 2025EDIT */
            }
            continue;
        }

        sprintf(tmp3, "DAEMON_%c_END_TIME", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A')) {
                /* 2025EDIT */
                strncpy(INFO(cnt-'A').end_time, tmp2, sizeof(INFO(cnt-'A').end_time));
                /* 2025EDIT */
            }
            else if (flag == 1 && D_K == cnt - 'A') {
                /* 2025EDIT */
                strncpy(DAEMON(cnt-'A').end_time, tmp2, sizeof(DAEMON(cnt-'A').end_time));
                strncpy(INFO(cnt-'A').end_time,  tmp2, sizeof(INFO(cnt-'A').end_time));
                /* 2025EDIT */
            }
            continue;
        }

        sprintf(tmp3, "DAEMON_%c_DATE_FLAG", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A')) {
                INFO(cnt-'A').date_flag = atoi(tmp2);

                /* set work date    */
                start_ss = AtoIf(INFO(cnt-'A').start_time, 2) * 60 * 60 +
                AtoIf(INFO(cnt-'A').start_time+2, 2) * 60;
                end_ss = AtoIf(INFO(cnt-'A').end_time, 2) * 60 * 60 +
                AtoIf(INFO(cnt-'A').end_time+2, 2) * 60;

                if ((start_ss < end_ss && (INFO(cnt-'A').date_flag == 2 ||
                        INFO(cnt-'A').date_flag == 4)) ||
                        (start_ss > end_ss && (INFO(cnt-'A').date_flag == 1 ||
                        INFO(cnt-'A').date_flag == 3 ||
                        INFO(cnt-'A').date_flag == 5)) ||
                        (start_ss == end_ss && INFO(cnt-'A').date_flag != 9) ||
                        (start_ss != end_ss && INFO(cnt-'A').date_flag == 9)) {
                    Log(USR_FATAL, "daemon(%c):%s[%s] line(%d)", cnt, tmp1, tmp2, __LINE__);
                    sleep(3);
                    exit(FAIL);
                }

                if (INFO(cnt-'A').date_flag != 9) {
                    memset(dt, 0, sizeof (dt));
                    time(&t);
                    date = localtime(&t);
                    tmp_ss = (date->tm_hour * 60 * 60)
                    + (date->tm_min * 60) + date->tm_sec;

                    switch (INFO(cnt-'A').date_flag) {
                        case    1:
                            break;
                        case    2:
                            if (tmp_ss <= end_ss)
                                t -= 86400L;
                            break;
                        case    3:
                            t -= 86400L;
                            break;
                        case    4:
                            if (tmp_ss >= start_ss)
                                t += 86400L;
                            break;
                        case    5:
                            t += 86400L;
                            break;
                    }

                    tp = localtime(&t);
                    strftime(dt, 10, "%Y%m%d", tp);
                    memcpy(INFO(cnt-'A').date, dt, 8);
                    Log(USR_OK, "set work date(INFO) [%c%c,%s]",
                            _System_Name[0], cnt, INFO(cnt-'A').date);
                }
            }
            else if (flag == 1 && D_K == cnt - 'A') {
                DAEMON(cnt-'A').date_flag = atoi(tmp2);
                INFO(cnt-'A').date_flag = atoi(tmp2);

                start_ss = AtoIf(DAEMON(cnt-'A').start_time, 2) * 60 * 60 +
                AtoIf(DAEMON(cnt-'A').start_time+2, 2) * 60;
                end_ss = AtoIf(DAEMON(cnt-'A').end_time, 2) * 60 * 60 +
                AtoIf(DAEMON(cnt-'A').end_time+2, 2) * 60;

                if (DAEMON(cnt-'A').date_flag != 9) {
                    memset(dt, 0, sizeof (dt));
                    time(&t);
                    date = localtime(&t);
                    tmp_ss = (date->tm_hour * 60 * 60)
                    + (date->tm_min * 60) + date->tm_sec;

                    switch (DAEMON(cnt-'A').date_flag) {
                        case    1:
                            break;
                        case    2:
                            if (tmp_ss <= end_ss)
                                t -= 86400L;
                            break;
                        case    3:
                            t -= 86400L;
                            break;
                        case    4:
                            if (tmp_ss >= start_ss)
                                t += 86400L;
                            break;
                        case    5:
                            t += 86400L;
                            break;
                    }

                    tp = localtime(&t);
                    strftime(dt, 10, "%Y%m%d", tp);
                    memcpy(DAEMON(cnt-'A').date, dt, 8);
                    memcpy(INFO(cnt-'A').date, dt, 8);

                    Log(USR_OK, "set work date(DAEMON) [%c%c,%s]",
                            _System_Name[0], cnt, DAEMON(cnt-'A').date);
                }
            }
            continue;
        }

        sprintf(tmp3, "DAEMON_%c_COMPACT_DAYS", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A'))
                INFO(cnt-'A').compact_days = atoi(tmp2);
            else if (flag == 1 && D_K == cnt - 'A') {
                DAEMON(cnt-'A').compact_days = atoi(tmp2);
                INFO(cnt-'A').compact_days = atoi(tmp2);
            }
            continue;
        }

        sprintf(tmp3, "DAEMON_%c_STATUS", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A' && tmp2[0] == '0'))
                INFO(cnt-'A').process_status = atoi(tmp2);
            else if (flag == 1 && D_K == cnt - 'A') {
                DAEMON(cnt-'A').process_status = atoi(tmp2);
                INFO(cnt-'A').process_status = atoi(tmp2);
            }
            continue;
        }

        sprintf(tmp3, "DAEMON_%c_FIFO", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A')) {
                sprintf(bumun, "%-2.2s", tmp2);
                LtoU(bumun, 2);

                /* 2025EDIT,START */
                strncpy(INFO(cnt-'A').start_FIFO_name, tmp2, sizeof(INFO(cnt-'A').start_FIFO_name));
                /* 2025EDIT,END */
                sprintf(file_name, "%s/%-2.2s/%s", _FEP_FIFO, bumun, tmp2);
                Create_FIFO(file_name);

                /* 2025EDIT,START */
                {
                    char temp_buf01[sizeof(INFO(cnt - 'A').exit_FIFO_name)];
                    size_t max_tmp_len01 = sizeof(temp_buf01) - strlen("_exit");

                    strncpy(temp_buf01, tmp2, max_tmp_len01);
                    strcat(temp_buf01, "_exit");

                    strncpy(INFO(cnt - 'A').exit_FIFO_name, temp_buf01, sizeof(INFO(cnt - 'A').exit_FIFO_name) - 1);
                    INFO(cnt - 'A').exit_FIFO_name[sizeof(INFO(cnt - 'A').exit_FIFO_name) - 1] = '\0';
                }
                /* 2025EDIT,END */
                sprintf(file_name, "%s/%-2.2s/%s_exit",
                        _FEP_FIFO, bumun, tmp2);
                Create_FIFO(file_name);

                /* 2025EDIT,START */
                {
                    char temp_buf001[sizeof(INFO(cnt - 'A').daemon_FIFO_name)];
                    size_t max_tmp_len001 = sizeof(temp_buf001) - strlen("_ctrl");

                    strncpy(temp_buf001, tmp2, max_tmp_len001);
                    strcat(temp_buf001, "_ctrl");

                    strncpy(INFO(cnt - 'A').daemon_FIFO_name, temp_buf001, sizeof(INFO(cnt - 'A').daemon_FIFO_name) - 1);
                    INFO(cnt - 'A').daemon_FIFO_name[sizeof(INFO(cnt - 'A').daemon_FIFO_name) - 1] = '\0';
                }
                /* 2025EDIT,END */
                sprintf(file_name, "%s/%-2.2s/%s_ctrl",
                        _FEP_FIFO, bumun, tmp2);
                Create_FIFO(file_name);

                sprintf(file_name, "%s/%-2.2s/%s_slog",
                        _FEP_FIFO, bumun, tmp2);
                Create_FIFO(file_name);
            }
            else if (flag == 1 && D_K == cnt - 'A') {
                sprintf(bumun, "%-2.2s", tmp2);
                LtoU(bumun, 2);

                /* 2025EDIT */
                strncpy(DAEMON(cnt-'A').start_FIFO_name, tmp2, sizeof(DAEMON(cnt-'A').start_FIFO_name));
                strncpy( INFO(cnt-'A').start_FIFO_name, tmp2, sizeof(INFO(cnt-'A').start_FIFO_name));
                /* 2025EDIT */

                sprintf(file_name, "%s/%-2.2s/%s", _FEP_FIFO, bumun, tmp2);
                Create_FIFO(file_name);

                /* 2025EDIT,START */
                {
                    char temp_buf02[sizeof(DAEMON(cnt - 'A').exit_FIFO_name)];
                    size_t max_tmp_len02 = sizeof(temp_buf02) - strlen("_exit");

                    strncpy(temp_buf02, tmp2, max_tmp_len02);
                    strcat(temp_buf02, "_exit");

                    strncpy(DAEMON(cnt - 'A').exit_FIFO_name, temp_buf02, sizeof(DAEMON(cnt - 'A').exit_FIFO_name) - 1);
                    DAEMON(cnt - 'A').exit_FIFO_name[sizeof(DAEMON(cnt - 'A').exit_FIFO_name) - 1] = '\0';
                    strncpy(INFO(cnt-'A').exit_FIFO_name, temp_buf02, sizeof(INFO(cnt-'A').exit_FIFO_name) - 1);
                    INFO(cnt-'A').exit_FIFO_name[sizeof(INFO(cnt-'A').exit_FIFO_name) - 1] = '\0';
                }
                /* 2025EDIT,END */
                sprintf(file_name,
                        "%s/%-2.2s/%s_exit", _FEP_FIFO, bumun, tmp2);
                Create_FIFO(file_name);

                /* 2025EDIT,START */
                {
                    char temp_buf03[sizeof(DAEMON(cnt - 'A').daemon_FIFO_name)];
                    size_t max_tmp_len03 = sizeof(temp_buf03) - strlen("_ctrl");

                    strncpy(temp_buf03, tmp2, max_tmp_len03);
                    strcat(temp_buf03, "_ctrl");

                    strncpy(DAEMON(cnt - 'A').daemon_FIFO_name, temp_buf03, sizeof(DAEMON(cnt - 'A').daemon_FIFO_name) - 1);
                    DAEMON(cnt - 'A').daemon_FIFO_name[sizeof(DAEMON(cnt - 'A').daemon_FIFO_name) - 1] = '\0';
                    strncpy(INFO(cnt-'A').daemon_FIFO_name, temp_buf03, sizeof(INFO(cnt-'A').daemon_FIFO_name) - 1);
                    INFO(cnt-'A').daemon_FIFO_name[sizeof(INFO(cnt-'A').daemon_FIFO_name) - 1] = '\0';
                }
                /* 2025EDIT,END */
                sprintf(file_name,
                        "%s/%-2.2s/%s_ctrl", _FEP_FIFO, bumun, tmp2);
                Create_FIFO(file_name);
            }
            continue;
        }

        sprintf(tmp3, "DAEMON_%c_PROC_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A'))
                INFO(cnt-'A').process_count = atoi(tmp2);
            else if (flag == 1 && D_K == cnt - 'A') {
                DAEMON(cnt-'A').process_count = atoi(tmp2);
                INFO(cnt-'A').process_count = atoi(tmp2);
            }
            else if (flag == 2)
                Info[cnt-'A'].process_count = atoi(tmp2);
            continue;
        }

        sprintf(tmp3, "DAEMON_%c_FILE_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A'))
                INFO(cnt-'A').file_count = atoi(tmp2);
            else if (flag == 1 && D_K == cnt - 'A') {
                DAEMON(cnt-'A').file_count = atoi(tmp2);
                INFO(cnt-'A').file_count = atoi(tmp2);
            }
            else if (flag == 2)
                Info[cnt-'A'].file_count = atoi(tmp2);
            continue;
        }

        sprintf(tmp3, "DAEMON_%c_DSHM_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A'))
                INFO(cnt-'A').dshm_count = atoi(tmp2);
            else if (flag == 1 && D_K == cnt - 'A') {
                DAEMON(cnt-'A').dshm_count = atoi(tmp2);
                INFO(cnt-'A').dshm_count = atoi(tmp2);
            }
            else if (flag == 2)
                Info[cnt-'A'].dshm_count = atoi(tmp2);
            continue;
        }

#if defined ISAM_INCL
        sprintf(tmp3, "DAEMON_%c_CISAM_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A'))
                INFO(cnt-'A').cisam_count = atoi(tmp2);
            else if (flag == 1 && D_K == cnt - 'A') {
                DAEMON(cnt-'A').cisam_count = atoi(tmp2);
                INFO(cnt-'A').cisam_count = atoi(tmp2);
            }
            else if (flag == 2)
                Info[cnt-'A'].cisam_count = atoi(tmp2);
            continue;
        }
#endif

        sprintf(tmp3, "DAEMON_%c_TCP1_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A'))
                INFO(cnt-'A').tcp1_count = atoi(tmp2);
            else if (flag == 1 && D_K == cnt - 'A') {
                DAEMON(cnt-'A').tcp1_count = atoi(tmp2);
                INFO(cnt-'A').tcp1_count = atoi(tmp2);
            }
            else if (flag == 2)
                Info[cnt-'A'].tcp1_count = atoi(tmp2);
            continue;
        }

        sprintf(tmp3, "DAEMON_%c_TCP2_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A'))
                INFO(cnt-'A').tcp2_count = atoi(tmp2);
            else if (flag == 1 && D_K == cnt - 'A') {
                DAEMON(cnt-'A').tcp2_count = atoi(tmp2);
                INFO(cnt-'A').tcp2_count = atoi(tmp2);
            }
            else if (flag == 2)
                Info[cnt-'A'].tcp2_count = atoi(tmp2);
            continue;
        }

        sprintf(tmp3, "DAEMON_%c_UDPIP_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A'))
                INFO(cnt-'A').udpip_count = atoi(tmp2);
            else if (flag == 1 && D_K == cnt - 'A') {
                DAEMON(cnt-'A').udpip_count = atoi(tmp2);
                INFO(cnt-'A').udpip_count = atoi(tmp2);
            }
            else if (flag == 2)
                Info[cnt-'A'].udpip_count = atoi(tmp2);
            continue;
        }

        sprintf(tmp3, "DAEMON_%c_DATA_COUNT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A'))
                INFO(cnt-'A').data_count = atoi(tmp2);
            else if (flag == 1 && D_K == cnt - 'A') {
                DAEMON(cnt-'A').data_count = atoi(tmp2);
                INFO(cnt-'A').data_count = atoi(tmp2);
            }
            else if (flag == 2)
                Info[cnt-'A'].data_count = atoi(tmp2);
            continue;
        }

        sprintf(tmp3, "DAEMON_%c_SHM_LOG", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 3 && D_K == cnt - 'A' && tmp2[0] == '0'))
                INFO(cnt-'A').shm_log = atoi(tmp2);
            else if (flag == 1 && D_K == cnt - 'A') {
                DAEMON(cnt-'A').shm_log = atoi(tmp2);
                INFO(cnt-'A').shm_log = atoi(tmp2);
            }
            else if (flag == 2)
                Info[cnt-'A'].shm_log = atoi(tmp2);
            continue;
        }

        /* 202201
                sprintf (tmp3, "DAEMON_%c_SISE_COUNT", cnt);
                if (memcmp (tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    if (flag == 0 || (flag == 3 && D_K == cnt - 'A'))
                        INFO(cnt-'A').sisetr_count = atoi (tmp2);
                    else if (flag == 1 && D_K == cnt - 'A') {
                        DAEMON(cnt-'A').sisetr_count = atoi (tmp2);
                        INFO(cnt-'A').sisetr_count = atoi (tmp2);
                    }
                    else if (flag == 2)
                        Info[cnt-'A'].sisetr_count = atoi (tmp2);
                    continue;
                }

                sprintf (tmp3, "DAEMON_%c_ACCNO_COUNT", cnt);
                if (memcmp (tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    if (flag == 0 || (flag == 3 && D_K == cnt - 'A'))
                        INFO(cnt-'A').accno_count = atoi (tmp2);
                    else if (flag == 1 && D_K == cnt - 'A') {
                        DAEMON(cnt-'A').accno_count = atoi (tmp2);
                        INFO(cnt-'A').accno_count = atoi (tmp2);
                    }
                    else if (flag == 2)
                        Info[cnt-'A'].accno_count = atoi (tmp2);
                    continue;
                }
        */

        if (flag == 0 || ((flag == 1 || flag == 3) && sub_flag == 1)) {
            Log(USR_FATAL, "daemon(%c):%s[%s] line(%d)", cnt, tmp1, tmp2, __LINE__);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }
        memset(buf, 0, sizeof (buf));
    }
    fclose(fp);

    return;
}   /* End of Daemon_Config_Read () */

/*************************************************************************
    Function        : . read file.ini and set the file SHM (FILEM)
    Parameters IN   : . flag (0:set all, 1:set one)
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    File_Config_Read(int flag)
/*----------------------------------------------------------------------*/
{
    int         fd, rt, cnt = 0, i, j, itemcnt, len, pt, idx_flag, sub_flag;
    int         c1 = '=';
    int         c2 = ' ';
    int         c3 = '\t';
    int         p_cnt = 0;
    int         g_cnt = 'A';
    char        buf[1024];  /* buf[100] */
    char        file_name[256], seq_buf[400];
    char        tmp1[40], tmp2[40], tmp3[40], bumun[4], ppath[100];
    char        *sp, *sp1, *sp2, *sp3;
    FILE        *fp;
    struct stat64   f_info;

    sub_flag = 0;

    memset(buf, 0, sizeof (buf));
    sprintf(buf, "%s/file.ini", _FEP_CFG);

    if ((fp = fopen(buf, "r")) == NULL) {
        Log(SAM_FATAL, "fopen failure[%s] {%d:%s}", buf, SYS_NO, SYS_STR);
        sleep(3);
        exit(FAIL);
    }

    memset(buf, 0, sizeof (buf));

    while ((sp = fgets(buf, sizeof (buf), fp)) != NULL) {
        buf[strlen(buf)-1] = 0;
        if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
            continue;

        sprintf(tmp3, "%s", "File_End");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                if (memcmp(&FILEM(p_cnt,cnt-1).file_name[3], "log", 3) == 0)
                    sprintf(ppath, "%s", _FEP_LOG);
                else
                    sprintf(ppath, "%s", _FEP_DAT);

                sprintf(bumun, "%-2.2s", FILEM(p_cnt,cnt-1).file_name);
                sprintf(file_name, "%s/%s/00000000", ppath, LtoU(bumun, 2));
                Create_Dir(file_name);

                sprintf(file_name, "%s/%s/00000000/%s_seq",
                        ppath, bumun, FILEM(p_cnt,cnt-1).file_name);
                fd = open(file_name, O_RDWR|O_APPEND|O_CREAT, 0664);
                fchmod(fd, 0664);
                close(fd);

                sprintf(file_name, "%s/%s/00000000/%s.idx",
                        ppath, bumun, FILEM(p_cnt,cnt-1).file_name);
                if (access(file_name, F_OK) == 0)
                    idx_flag = 1;
                else {
                    idx_flag = 0;
                    sprintf(file_name, "%s/%s/00000000/%s",
                            ppath, bumun, FILEM(p_cnt,cnt-1).file_name);
                }

                fd = open64(file_name, O_RDWR|O_APPEND|O_CREAT, 0664);
                fchmod(fd, 0664);
                close(fd);
                f_info.st_size = 0;
                rt = stat64(file_name, &f_info);
                if (f_info.st_size > 0) {
                    if (memcmp(FILEM(p_cnt,cnt-1).file_name+3,
                            "seq", 3) != 0 &&
                            memcmp(FILEM(p_cnt,cnt-1).file_name+3, "log", 3) != 0) {
                        if (idx_flag)
                            FILEM(p_cnt,cnt-1).w_cnt[0] = f_info.st_size / 23;
                        else
                            FILEM(p_cnt,cnt-1).w_cnt[0] = f_info.st_size /
                        (int)(FILEM(p_cnt,cnt-1).record_size +
                                sizeof (FILE_RW_HEAD) + 1);
                    }
                    sprintf(file_name, "%s/%s/00000000/%s_seq",
                            ppath, bumun, FILEM(p_cnt,cnt-1).file_name);
                    Log(USR_OK, "file [%s]", file_name);
                    fd = open(file_name, O_RDWR | O_APPEND | O_CREAT, 0664);
                    if (fd < 0)
                        Log(SAM_FATAL, "cannot open file[%d,%s] {%d:%s}",
                                fd, file_name, SYS_NO, SYS_STR);
                    else {
                        fchmod(fd, 0664);
                        memset(seq_buf, 0, sizeof (seq_buf));
                        len = 72;
                        pt = 0;

                        while (1) {
                            rt = read(fd, seq_buf+pt, len);
                            if (rt < 0) {
                                Log(SAM_FATAL,
                                        "cannot read file[%d,%s] {%d:%s}",
                                        fd, file_name, SYS_NO, SYS_STR);
                                break;
                            }
                            else if (rt == 0 || rt == len)
                                break;
                            len -= rt;
                            pt += rt;
                        }

                        if (rt > 0) {
                            for (i = 0; i < 9; i ++)
                                FILEM(p_cnt,cnt-1).r_cnt[i] =
                            AtoIf(&seq_buf[8*i], 8);
                        }
                        close(fd);
                    }
                }

                for (i = 1; i <= FILEM(p_cnt,cnt-1).fifo_count; i ++) {
                    sprintf(file_name, "%s/%-2.2s/%s%d",
                            _FEP_FIFO, bumun, FILEM(p_cnt,cnt-1).file_name, i);
                    Create_FIFO(file_name);
                }
            }
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
                    if (flag == 0 || (flag == 1 && D_K == p_cnt))
                        Log(USR_OK, "start %c%c file config ...",
                                _System_Name[0], g_cnt);

                    if (flag == 1 && D_K == p_cnt)
                        sub_flag = 1;

                    break;
                }
            }
            continue;
        }

        sprintf(tmp3, "%c%c_CONF_END", _System_Name[0], g_cnt);
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                Log(USR_OK, "... end %c%c file config",
                        _System_Name[0], g_cnt);

            if (sub_flag == 1)
                break;

            continue;
        }

        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            Log(USR_FATAL, "file(%c):[%s]", g_cnt, buf);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }
        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1+1, c2);
        sp3 = strchr(sp1+1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)    strncpy(tmp2, sp1+1, strlen(sp1)-1);
        else                memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        sprintf(tmp3, "%s", "FILE_COUNT");
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            itemcnt = atoi(tmp2);
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                if (DAEMON(p_cnt).file_count < itemcnt) {
                    Log(USR_FATAL,
                            "file(%c):%s[%s]:edit file_count in daemon.ini",
                            g_cnt, tmp1, tmp2);
                    fclose(fp);
                    sleep(3);
                    exit(FAIL);
                }
                else
                    DAEMON(p_cnt).f_count = itemcnt;
            }
            continue;
        }

        if (itemcnt < cnt)
            continue;

        sprintf(tmp3, "FILE_%d_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                sprintf(FILEM(p_cnt,cnt-1).file_info, "%s", sp1+1);
            continue;
        }
        sprintf(tmp3, "FILE_%d_NAME", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                /* 2025EDIT, START */
                strncpy(FILEM(p_cnt,cnt-1).file_name, tmp2, sizeof(FILEM(p_cnt,cnt-1).file_name));
                /* 2025EDIT, START */
            }
            continue;
        }
        sprintf(tmp3, "FILE_%d_FIFO", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                FILEM(p_cnt,cnt-1).fifo_count = atoi(tmp2);
            continue;
        }
        sprintf(tmp3, "FILE_%d_SIZE", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                FILEM(p_cnt,cnt-1).record_size = atoi(tmp2);
            continue;
        }

        if (flag == 0 || (flag == 1 && sub_flag == 1)) {
            Log(USR_FATAL, "file(%c,%d):%s[%s]", g_cnt, cnt, tmp1, tmp2);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }
        memset(buf, 0, sizeof (buf));
    }
    fclose(fp);

    return;
}   /* End of File_Config_Read ()   */

/*************************************************************************
    Function        : . read dshm.ini and set the data SHM (DSHM)
    Parameters IN   : . flag (0:set all, 1:set one)
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Dshm_Config_Read(int flag)
/*----------------------------------------------------------------------*/
{
    int             fd, rt, cnt = 0, i, j, itemcnt, len, pt, sub_flag;
    int             shmid, rec_size, max, offset;
    int             c1 = '=';
    int             c2 = ' ';
    int             c3 = '\t';
    int             p_cnt = 0;
    int             g_cnt = 'A';
    char            buf[1024];  /* buf[100] */
    char            file_name[256], seq_buf[400], key[12];
    char            tmp1[40], tmp2[40], tmp3[40], bumun[4], ppath[100];
    char            *sp, *sp1, *sp2, *sp3;
    key_t           base_key, shm_key;
    FILE            *fp;
    struct stat     f_info;

    sub_flag = offset = 0;

    sprintf(buf, "%s/dshm.ini", _FEP_CFG);

    if ((fp = fopen(buf, "r")) == NULL) {
        Log(SAM_FATAL, "fopen failure[%s] {%d:%s}", buf, SYS_NO, SYS_STR);
        sleep(3);
        exit(FAIL);
    }

    memset(buf, 0, sizeof (buf));

    while ((sp = fgets(buf, sizeof (buf), fp)) != NULL) {
        buf[strlen(buf)-1] = 0;

        if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
            continue;

        sprintf(tmp3, "%s", "Dshm_End");

        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                rec_size = sizeof (FILE_RW_HEAD) +
                DSHM(p_cnt,cnt-1).data_size + 1;
                max = DSHM(p_cnt,cnt-1).max_rec;

                /* start section in a SHM segment   */
                if (DSHM(p_cnt,cnt-1).key_info[5] == 'S') {
                    DSHM(p_cnt,cnt-1).offset = 0;
                    offset += (rec_size * max);
                }
                /* a section in between start and end sections  */
                else if (DSHM(p_cnt,cnt-1).key_info[5] == 'C') {
                    DSHM(p_cnt,cnt-1).offset = offset;
                    offset += (rec_size * max);
                }
                else {
                    sprintf(key, "0x0000%2.2s00", DSHM(p_cnt,cnt-1).key_info);
                    errno = 0;
                    shm_key = base_key + strtol(key, NULL, 16);

                    /* end section in a SHM segment */
                    if (DSHM(p_cnt,cnt-1).key_info[5] == 'E') {
                        Shmptr = SHM_Creat_Attach(shm_key, offset +
                                (rec_size * max), &shmid);
                        Log(USR_OK, "DSHM created: key[%#10x] size[%d]",
                                shm_key, offset + (rec_size * max));
                        DSHM(p_cnt,cnt-1).offset = offset;
                    }
                    /* SHM segment has only one section */
                    else if (DSHM(p_cnt,cnt-1).key_info[5] == 'O') {
                        Shmptr = SHM_Creat_Attach(shm_key, rec_size * max,
                                &shmid);
                        Log(USR_OK, "DSHM created: key[%#10x] size[%d]",
                                shm_key, offset + (rec_size * max));
                        DSHM(p_cnt,cnt-1).offset = 0;
                    }

                    offset = 0;
                }

                sprintf(ppath, "%s", _FEP_DAT);
                sprintf(bumun, "%-2.2s", DSHM(p_cnt,cnt-1).data_name);
                sprintf(file_name, "%s/%s/00000000", ppath, LtoU(bumun, 2));
                Create_Dir(file_name);
                sprintf(file_name, "%s/%s/00000000/%s_dseq",
                        ppath, bumun, DSHM(p_cnt,cnt-1).data_name);

                fd = open(file_name, O_RDWR|O_APPEND|O_CREAT, 0664);
                fchmod(fd, 0664);
                close(fd);

                sprintf(file_name, "%s/%s/00000000/%s",
                        ppath, bumun, DSHM(p_cnt,cnt-1).data_name);

                fd = open(file_name, O_RDWR|O_APPEND|O_CREAT, 0664);
                fchmod(fd, 0664);
                close(fd);
                f_info.st_size = 0;
                rt = stat(file_name, &f_info);

                if (f_info.st_size > 0) {
                    DSHM(p_cnt,cnt-1).w_cnt[0] = f_info.st_size /
                    (int)(sizeof (FILE_RW_HEAD) +
                            DSHM(p_cnt,cnt-1).data_size + 1);
                }

                sprintf(file_name, "%s/%s/00000000/%s_dseq",
                        ppath, bumun, DSHM(p_cnt,cnt-1).data_name);
                fd = open(file_name, O_RDWR|O_APPEND|O_CREAT, 0664);

                if (fd < 0)
                    Log(SAM_FATAL, "cannot open file[%d,%s] {%d:%s}",
                            fd, file_name, SYS_NO, SYS_STR);
                else {
                    fchmod(fd, 0664);
                    memset(seq_buf, 0, sizeof (seq_buf));
                    len = 80;
                    pt = 0;

                    while (1) {
                        rt = read(fd, seq_buf+pt, len);

                        if (rt < 0) {
                            Log(SAM_FATAL,
                                    "cannot read file[%d,%s] {%d:%s}",
                                    fd, file_name, SYS_NO, SYS_STR);
                            break;
                        }
                        else if (rt == 0 || rt == len)
                            break;

                        len -= rt;
                        pt += rt;
                    }

                    if (rt > 0) {
                        for (i = 0; i < 9; i ++)
                            DSHM(p_cnt,cnt-1).r_cnt[i] =
                        AtoIf(&seq_buf[8*i], 8);

                        DSHM(p_cnt,cnt-1).sm_r_cnt =
                        AtoIf(&seq_buf[72], 8);
                    }

                    close(fd);
                }

                for (i = 0; i <= DSHM(p_cnt,cnt-1).fifo_count; i ++) {
                    sprintf(file_name, "%s/%-2.2s/%s%d",
                            _FEP_FIFO, bumun, DSHM(p_cnt,cnt-1).data_name, i);
                    Create_FIFO(file_name);
                }
            }

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

                    if (flag == 0 || (flag == 1 && D_K == p_cnt))
                        Log(USR_OK, "start %c%c dshm config ...",
                                _System_Name[0], g_cnt);

                    if (flag == 1 && D_K == p_cnt)
                        sub_flag = 1;

                    /* set data SHM base key    */
                    base_key = BASE_SHM_KEY;

                    if (memcmp(_FEP_DIV, "TEST", 4) == 0)
                        base_key += 0x01000000L;

                    sprintf(key, "0x00%02d0000", p_cnt + 1);
                    errno = 0;
                    base_key += strtol(key, NULL, 16);
                    break;
                }
            }

            continue;
        }

        sprintf(tmp3, "%c%c_CONF_END", _System_Name[0], g_cnt);

        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                Log(USR_OK, "... end %c%c dshm config",
                        _System_Name[0], g_cnt);

            if (sub_flag == 1)
                break;

            continue;
        }

        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);

        if (sp1 == NULL) {
            Log(USR_FATAL, "dshm(%c):[%s]", g_cnt, buf);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }

        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1+1, c2);
        sp3 = strchr(sp1+1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)    strncpy(tmp2, sp1+1, strlen(sp1)-1);
        else                memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        sprintf(tmp3, "%s", "DSHM_COUNT");

        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            itemcnt = atoi(tmp2);

            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                if (DAEMON(p_cnt).dshm_count < itemcnt) {
                    Log(USR_FATAL,
                            "dshm(%c):%s[%s]:edit dshm_count in daemon.ini",
                            g_cnt, tmp1, tmp2);
                    fclose(fp);
                    sleep(3);
                    exit(FAIL);
                }
                else
                    DAEMON(p_cnt).d_count = itemcnt;
            }

            continue;
        }

        if (itemcnt < cnt)
            continue;

        sprintf(tmp3, "DSHM_%d_COMMENT", cnt);

        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                sprintf(DSHM(p_cnt,cnt-1).info, "%s", sp1+1);

            continue;
        }

        sprintf(tmp3, "DSHM_%d_NAME", cnt);

        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                /* 2025EDIT,START */
                strncpy(DSHM(p_cnt,cnt-1).data_name, tmp2, sizeof(DSHM(p_cnt,cnt-1).data_name));
            /* 2025EDIT,END */

            continue;
        }

        sprintf(tmp3, "DSHM_%d_KEY", cnt);

        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                /* 2025EDIT,START */
                strncpy(DSHM(p_cnt,cnt-1).key_info, tmp2, sizeof(DSHM(p_cnt,cnt-1).key_info));
            /* 2025EDIT,START */

            continue;
        }

        sprintf(tmp3, "DSHM_%d_FIFO", cnt);

        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                DSHM(p_cnt,cnt-1).fifo_count = atoi(tmp2);

            continue;
        }

        sprintf(tmp3, "DSHM_%d_SIZE", cnt);

        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                DSHM(p_cnt,cnt-1).data_size = atoi(tmp2);

            continue;
        }

        sprintf(tmp3, "DSHM_%d_MAX", cnt);

        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                DSHM(p_cnt,cnt-1).max_rec = atoi(tmp2);

            continue;
        }

        if (flag == 0 || (flag == 1 && sub_flag == 1)) {
            Log(USR_FATAL, "dshm(%c,%d):%s[%s]", g_cnt, cnt, tmp1, tmp2);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }

        memset(buf, 0, sizeof (buf));
    }

    fclose(fp);

    return;
}   /* End of Dshm_Config_Read ()   */

#if defined ISAM_INCL
/*************************************************************************
    Function        : . read cisam.ini and set the file SHM (CISAM)
    Parameters IN   : . flag (0:set all, 1:set one)
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Cisam_Config_Read(int flag)
/*----------------------------------------------------------------------*/
{
    int     cnt, i, j, itemcnt, sub_flag;
    int     c1 = '=';
    int     c2 = ' ';
    int     c3 = '\t';
    int     p_cnt = 0;
    int     g_cnt = 'A';
    char    buf[1024];  /* buf[100] */
    char    tmp1[40], tmp2[40], tmp3[40];
    char    *sp, *sp1, *sp2, *sp3;
    FILE    *fp;

    sub_flag = 0;

    sprintf(buf, "%s/cisam.ini", _FEP_CFG);
    if ((fp = fopen(buf, "r")) == NULL) {
        Log(SAM_FATAL, "fopen failure[%s] {%d:%s}", buf, SYS_NO, SYS_STR);
        sleep(3);
        exit(FAIL);
    }

    memset(buf, 0, sizeof (buf));

    while ((sp = fgets(buf, sizeof (buf), fp)) != NULL) {
        buf[strlen(buf)-1] = 0;
        if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
            continue;

        sprintf(tmp3, "%s", "Cisam_End");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                Create_Cisam(p_cnt, cnt - 1);

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
                    if (flag == 0 || (flag == 1 && D_K == p_cnt))
                        Log(USR_OK, "start %c%c cisam config ...",
                                _System_Name[0], g_cnt);

                    if (flag == 1 && D_K == p_cnt)
                        sub_flag = 1;

                    break;
                }
            }
            continue;
        }

        sprintf(tmp3, "%c%c_CONF_END", _System_Name[0], g_cnt);
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                Log(USR_OK, "end %c%c cisam config ...",
                        _System_Name[0], g_cnt);

            if (sub_flag == 1)
                break;

            continue;
        }

        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            Log(USR_FATAL, "cisam(%c):[%s]", g_cnt, buf);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }
        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1+1, c2);
        sp3 = strchr(sp1+1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)    strncpy(tmp2, sp1+1, strlen(sp1)-1);
        else                memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        sprintf(tmp3, "%s", "CISAM_COUNT");
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            itemcnt = atoi(tmp2);
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                if (DAEMON(p_cnt).cisam_count < itemcnt) {
                    Log(USR_FATAL,
                            "cisam(%c):%s[%s]:edit cisam_count in daemon.ini",
                            g_cnt, tmp1, tmp2);
                    fclose(fp);
                    sleep(3);
                    exit(FAIL);
                }
                else
                    DAEMON(p_cnt).c_count = itemcnt;
            }
            continue;
        }

        if (itemcnt < cnt)
            continue;

        sprintf(tmp3, "CISAM_%d_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                sprintf(CISAM(p_cnt,cnt-1).file_info, "%s", sp1+1);
            continue;
        }
        sprintf(tmp3, "CISAM_%d_NAME", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                sprintf(CISAM(p_cnt,cnt-1).file_name, "%s", tmp2);
            continue;
        }
        sprintf(tmp3, "CISAM_%d_KEY_SIZE", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                CISAM(p_cnt,cnt-1).key_size = atoi(tmp2);
            continue;
        }
        sprintf(tmp3, "CISAM_%d_RECORD_SIZE", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                CISAM(p_cnt,cnt-1).record_size = atoi(tmp2);
            continue;
        }

        if (flag == 0 || (flag == 1 && sub_flag == 1)) {
            Log(USR_FATAL, "cisam(%c,%d):%s[%s]", g_cnt, cnt, tmp1, tmp2);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }
        memset(buf, 0, sizeof (buf));
    }
    fclose(fp);

    return;
}   /* End of Cisam_Config_Read ()  */

/*************************************************************************
    Function        : . create c-isam files
    Parameters IN   : . dk  : daemon key
                      . pk  : process key
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Create_Cisam(int dk, int pk)
/*----------------------------------------------------------------------*/
{
    int             rt, key_size, rec_size, fd;
    char            bumun[4], ppath[100], buf[1024], file_name[256], cmd[256];
    char            *sp;
    FILE            *fp;
    struct keydesc  key;

    sprintf(bumun, "%-2.2s", CISAM(dk,pk).file_name);
    sprintf(ppath, "%s/%s/00000000", _FEP_DAT, LtoU(bumun, 2));
    Create_Dir(ppath);

    key_size = CISAM(dk,pk).key_size;
    rec_size = key_size + CISAM(dk,pk).record_size;

    sprintf(file_name, "%s/%s.dat", ppath, CISAM(dk,pk).file_name);
    if (access(file_name, F_OK) == 0) {
        sprintf(file_name, "%s/%s.idx", ppath, CISAM(dk,pk).file_name);
        if (access(file_name, F_OK) == -1) {
            sprintf(cmd, "mv %s/%s.dat %s/%s.bak",
                    ppath, CISAM(dk,pk).file_name, ppath, CISAM(dk,pk).file_name);
            rt = system(cmd);
#if defined(__hpux) || defined(sun) || defined(_AIX)
            if (rt < 0) {
                Log(SYS_ERROR, "system call failure[%s] {%d:%s}",
                        cmd, SYS_NO, SYS_STR);
                sleep(3);
                exit(FAIL);
            }
#endif

            key.k_flags = ISNODUPS;
            key.k_nparts = 1;
            key.k_part[0].kp_start = 0;
            key.k_part[0].kp_leng = key_size;
            key.k_part[0].kp_type = CHARTYPE;

            sprintf(file_name, "%s/%s", ppath, CISAM(dk,pk).file_name);
            fd = isbuild(file_name, rec_size, &key, ISINOUT + ISEXCLLOCK);
            if (fd < OK) {
                Log(SAM_ERROR, "cannot build ISAM[%d] {%d:%s}",
                        fd, ISYS_NO, ISYS_STR);
            }

            sprintf(file_name, "%s/%s.bak", ppath, CISAM(dk,pk).file_name);
            if ((fp = fopen(file_name, "r")) == NULL) {
                Log(SAM_ERROR, "fopen failure[%s] {%d:%s}",
                        file_name, SYS_NO, SYS_STR);

                Isam_Close(fd);
                sleep(3);
                exit(FAIL);
            }

            while (1) {
                memset(buf, 0, sizeof (buf));
                sp = fgets(buf, sizeof (buf), fp);
                if (sp == NULL || strlen(buf) < rec_size)
                    break;

                rt = iswrite(fd, buf);
                if (rt < OK && ISYS_NO != EDUPL) {
                    Log(SAM_ERROR, "cannot write ISAM[%d] {%d:%s}",
                            rt, ISYS_NO, ISYS_STR);
                    Isam_Close(fd);
                    break;
                }
            }

            Isam_Close(fd);
            fclose(fp);

            rt = unlink(file_name);
            if (rt < 0) {
                Log(SAM_ERROR, "unlink failure[%s] {%d:%s}",
                        file_name, SYS_NO, SYS_STR);
                sleep(3);
                exit(FAIL);
            }
        }
    }
    else {
        sprintf(file_name, "%s/%s.idx", ppath, CISAM(dk,pk).file_name);
        if (access(file_name, F_OK) == 0) {
            rt = unlink(file_name);
            if (rt < 0) {
                Log(SAM_ERROR, "unlink failure[%s] {%d:%s}",
                        file_name, SYS_NO, SYS_STR);
                sleep(3);
                exit(FAIL);
            }
        }

        key.k_flags = ISNODUPS;
        key.k_nparts = 1;
        key.k_part[0].kp_start = 0;
        key.k_part[0].kp_leng = key_size;
        key.k_part[0].kp_type = CHARTYPE;

        sprintf(file_name, "%s/%s", ppath, CISAM(dk,pk).file_name);
        fd = isbuild(file_name, rec_size, &key, ISINOUT + ISEXCLLOCK);
        if (fd < OK && ISYS_NO != 17) {
            Log(SAM_ERROR, "cannot build ISAM[%d] {%d:%s}",
                    fd, ISYS_NO, ISYS_STR);
        }

        Isam_Close(fd);
    }

    return;
}   /* Create_Cisam ()  */
#endif

/*************************************************************************
    Function        : . read tcp1.ini and set the TCP/IP 1 SHM (TCP1)
    Parameters IN   : . flag (0:set all, 1:set one)
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Tcp1_Config_Read(int flag)
/*----------------------------------------------------------------------*/
{
    int     i, cnt = 0, itemcnt = 0, sub_flag;
    int     c1 = '=';
    int     c2 = ' ';
    int     c3 = '\t';
    int     p_cnt = 0;
    int     g_cnt = 'A';
    char    buf[1024], tmp1[40], tmp2[40], tmp3[40];
    char    *sp, *sp1, *sp2, *sp3;
    FILE    *fp;

    sub_flag = 0;

    sprintf(buf, "%s/tcp1.ini", _FEP_CFG);
    if ((fp = fopen(buf, "r")) == NULL) {
        Log(SAM_FATAL, "fopen failure[%s] {%d:%s}", buf, SYS_NO, SYS_STR);
        sleep(3);
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
                    if (flag == 0 || (flag == 1 && D_K == p_cnt))
                        Log(USR_OK, "start %c%c tcp1 config ...",
                                _System_Name[0], g_cnt);

                    if (flag == 1 && D_K == p_cnt)
                        sub_flag = 1;

                    break;
                }
            }
            continue;
        }

        sprintf(tmp3, "%c%c_CONF_END", _System_Name[0], g_cnt);
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                Log(USR_OK, "... end %c%c tcp1 config",
                        _System_Name[0], g_cnt);

            if (sub_flag == 1)
                break;

            continue;
        }
        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            Log(USR_FATAL, "tcp1(%c):[%s]", g_cnt, buf);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }
        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1+1, c2);
        sp3 = strchr(sp1+1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)    strncpy(tmp2, sp1+1, strlen(sp1)-1);
        else                memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        sprintf(tmp3, "%s", "TCP1_COUNT");
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            itemcnt = atoi(tmp2);
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                if (DAEMON(p_cnt).tcp1_count < itemcnt) {
                    Log(USR_FATAL,
                            "tcp1(%c):%s[%s]:edit tcp1_count in daemon.ini",
                            g_cnt, tmp1, tmp2);
                    fclose(fp);
                    sleep(3);
                    exit(FAIL);
                }
                else
                    DAEMON(p_cnt).t1_count = itemcnt;
            }
            continue;
        }

        if (itemcnt < cnt)
            continue;

        sprintf(tmp3, "TCP1_%d_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                sprintf(TCP1(p_cnt,cnt-1).tcp_info, "%s", sp1+1);
            continue;
        }

        sprintf(tmp3, "TCP1_%d_IP", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                if (inet_pton(AF_INET, tmp2, TCP1(p_cnt,cnt-1).ip_addr) != 1) {
                    Log(USR_FATAL, "tcp1(%c,%d):malformed IP address:%s[%s]",
                            g_cnt, cnt, tmp1, tmp2);
                    sleep(3);
                    exit(FAIL);
                }
                else {
                    Log(USR_OK, "tcp1(%c,%d):IP address:%s[%s]",
                            g_cnt, cnt, tmp1, tmp2);
                }
            }
            continue;
        }

        sprintf(tmp3, "TCP1_%d_PORT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                TCP1(p_cnt,cnt-1).port_no = atoi(tmp2);
            continue;
        }

        sprintf(tmp3, "TCP1_%d_SPORT01", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                TCP1(p_cnt,cnt-1).service_port_no[0] = atoi(tmp2);
                if (!TCP1(p_cnt,cnt-1).service_port_no[0])
                    TCP1(p_cnt,cnt-1).service_status[0] = 9;
            }
            continue;
        }

        sprintf(tmp3, "TCP1_%d_SPORT02", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                TCP1(p_cnt,cnt-1).service_port_no[1] = atoi(tmp2);
                if (!TCP1(p_cnt,cnt-1).service_port_no[1])
                    TCP1(p_cnt,cnt-1).service_status[1] = 9;
            }
            continue;
        }

        sprintf(tmp3, "TCP1_%d_SPORT03", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                TCP1(p_cnt,cnt-1).service_port_no[2] = atoi(tmp2);
                if (!TCP1(p_cnt,cnt-1).service_port_no[2])
                    TCP1(p_cnt,cnt-1).service_status[2] = 9;
            }
            continue;
        }

        sprintf(tmp3, "TCP1_%d_SPORT04", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                TCP1(p_cnt,cnt-1).service_port_no[3] = atoi(tmp2);
                if (!TCP1(p_cnt,cnt-1).service_port_no[3])
                    TCP1(p_cnt,cnt-1).service_status[3] = 9;
            }
            continue;
        }

        sprintf(tmp3, "TCP1_%d_SPORT05", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                TCP1(p_cnt,cnt-1).service_port_no[4] = atoi(tmp2);
                if (!TCP1(p_cnt,cnt-1).service_port_no[4])
                    TCP1(p_cnt,cnt-1).service_status[4] = 9;
            }
            continue;
        }

        sprintf(tmp3, "TCP1_%d_SPORT06", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                TCP1(p_cnt,cnt-1).service_port_no[5] = atoi(tmp2);
                if (!TCP1(p_cnt,cnt-1).service_port_no[5])
                    TCP1(p_cnt,cnt-1).service_status[5] = 9;
            }
            continue;
        }

        sprintf(tmp3, "TCP1_%d_SPORT07", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                TCP1(p_cnt,cnt-1).service_port_no[6] = atoi(tmp2);
                if (!TCP1(p_cnt,cnt-1).service_port_no[6])
                    TCP1(p_cnt,cnt-1).service_status[6] = 9;
            }
            continue;
        }

        sprintf(tmp3, "TCP1_%d_SPORT08", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                TCP1(p_cnt,cnt-1).service_port_no[7] = atoi(tmp2);
                if (!TCP1(p_cnt,cnt-1).service_port_no[7])
                    TCP1(p_cnt,cnt-1).service_status[7] = 9;
            }
            continue;
        }

        sprintf(tmp3, "TCP1_%d_SPORT09", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                TCP1(p_cnt,cnt-1).service_port_no[8] = atoi(tmp2);
                if (!TCP1(p_cnt,cnt-1).service_port_no[8])
                    TCP1(p_cnt,cnt-1).service_status[8] = 9;
            }
            continue;
        }

        if (flag == 0 || (flag == 1 && sub_flag == 1)) {
            Log(USR_FATAL, "tcp1(%c,%d):%s[%s]", g_cnt, cnt, tmp1, tmp2);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }
        memset(buf, 0, sizeof (buf));
    }
    fclose(fp);

    return;
}   /* End of Tcp1_Config_Read ()   */

/*************************************************************************
    Function        : . read tcp2.ini and set the TCP/IP 2 SHM (TCP2)
    Parameters IN   : . flag (0:set all, 1:set one)
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Tcp2_Config_Read(int flag)
/*----------------------------------------------------------------------*/
{
    int     i, cnt = 0, itemcnt = 0, sub_flag;
    int     c1 = '=';
    int     c2 = ' ';
    int     c3 = '\t';
    int     p_cnt = 0;
    int     g_cnt = 'A';
    char    buf[1024], tmp1[40], tmp2[40], tmp3[40];
    char    *sp, *sp1, *sp2, *sp3;
    FILE    *fp;

    sub_flag = 0;

    sprintf(buf, "%s/tcp2.ini", _FEP_CFG);
    if ((fp = fopen(buf, "r")) == NULL) {
        Log(SAM_FATAL, "fopen failure[%s] {%d:%s}", buf, SYS_NO, SYS_STR);
        sleep(3);
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
                    if (flag == 0 || (flag == 1 && D_K == p_cnt))
                        Log(USR_OK, "start %c%c tcp2 config ...",
                                _System_Name[0], g_cnt);

                    if (flag == 1 && D_K == p_cnt)
                        sub_flag = 1;

                    break;
                }
            }
            continue;
        }

        sprintf(tmp3, "%c%c_CONF_END", _System_Name[0], g_cnt);
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                Log(USR_OK, "... end %c%c tcp2 config",
                        _System_Name[0], g_cnt);

            if (sub_flag == 1)
                break;

            continue;
        }
        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            Log(USR_FATAL, "tcp2(%c):[%s]", g_cnt, buf);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }
        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1+1, c2);
        sp3 = strchr(sp1+1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)        strncpy(tmp2, sp1+1, strlen(sp1)-1);
        else                    memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        sprintf(tmp3, "%s", "TCP2_COUNT");
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            itemcnt = atoi(tmp2);
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                if (DAEMON(p_cnt).tcp2_count < itemcnt) {
                    Log(USR_FATAL,
                            "tcp2(%c):%s[%s]:edit tcp2_count in daemon.ini",
                            g_cnt, tmp1, tmp2);
                    fclose(fp);
                    sleep(3);
                    exit(FAIL);
                }
                else
                    DAEMON(p_cnt).t2_count = itemcnt;
            }
            continue;
        }

        if (itemcnt < cnt)
            continue;

        sprintf(tmp3, "TCP2_%d_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                sprintf(TCP2(p_cnt,cnt-1).tcp_info, "%s", sp1+1);
            continue;
        }

        sprintf(tmp3, "TCP2_%d_IP", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                if (inet_pton(AF_INET, tmp2, TCP2(p_cnt,cnt-1).ip_addr) != 1) {
                    Log(USR_FATAL, "tcp2(%c,%d):malformed IP address:%s tmp2[%s][%d]",
                            g_cnt, cnt, tmp1, tmp2, strlen(tmp2));
                    sleep(3);
                    exit(FAIL);
                }
                else {
                    Log(USR_OK, "tcp2(%c,%d):IP address:%s tmp2[%s][%d]",
                            g_cnt, cnt, tmp1, tmp2, strlen(tmp2));
                }
            }
            continue;
        }

        sprintf(tmp3, "TCP2_%d_DUP_ID", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                TCP2(p_cnt,cnt-1).dup_id = atoi(tmp2);
            continue;
        }

        sprintf(tmp3, "TCP2_%d_PORT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                TCP2(p_cnt,cnt-1).port_no = atoi(tmp2);
            continue;
        }

        if (flag == 0 || (flag == 1 && sub_flag == 1)) {
            Log(USR_FATAL, "tcp2(%c,%d):%s[%s]", g_cnt, cnt, tmp1, tmp2);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }
        memset(buf, 0, sizeof (buf));
    }
    fclose(fp);

    return;
}   /* End of Tcp2_Config_Read ()   */

/*************************************************************************
    Function        : . read udpip.ini and set the UDP/IP SHM (UDPIP)
    Parameters IN   : . flag (0:set all, 1:set one)
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*-----------------------------------------------------------------------*/
void    Udpip_Config_Read(int flag)
/*-----------------------------------------------------------------------*/
{
    int     i, j, cnt = 0, itemcnt = 0, sub_flag, continue_flag;
    int     c1 = '=';
    int     c2 = ' ';
    int     c3 = '\t';
    int     p_cnt = 0;
    int     g_cnt = 'A';
    char    buf[1024], tmp1[40], tmp2[40], tmp3[40];
    char    *sp, *sp1, *sp2, *sp3;
    FILE    *fp;

    sub_flag = 0;

    sprintf(buf, "%s/udpip.ini", _FEP_CFG);
    if ((fp = fopen(buf, "r")) == NULL) {
        Log(SAM_FATAL, "fopen failure[%s] {%d:%s}", buf, SYS_NO, SYS_STR);
        sleep(3);
        exit(FAIL);
    }

    memset(buf, 0, sizeof (buf));

    while ((sp = fgets(buf, sizeof (buf), fp)) != NULL) {
        buf[strlen(buf)-1] = 0;
        if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
            continue;

        sprintf(tmp3, "%s", "Udpip_End");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            cnt++;
            continue;
        }

        sprintf(tmp3, "%cA_CONF_START", _System_Name[0]);
        if (memcmp(buf+2, tmp3+2, Max(strlen(buf)-2,strlen(tmp3)-2)) == 0) {
            for (g_cnt = 'A'; g_cnt <= 'Z'; g_cnt++) {
                sprintf(tmp3, "%c%c_CONF_START", _System_Name[0], g_cnt);
                if (memcmp(buf, tmp3, strlen(tmp3)) == 0) {
                    cnt = 1;
                    p_cnt = g_cnt - 'A';
                    if (flag == 0 || (flag == 1 && D_K == p_cnt))
                        Log(USR_OK, "start %c%c udpip config ...",
                                _System_Name[0], g_cnt);

                    if (flag == 1 && D_K == p_cnt)
                        sub_flag = 1;

                    break;
                }
            }
            continue;
        }

        sprintf(tmp3, "%c%c_CONF_END", _System_Name[0], g_cnt);
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                Log(USR_OK, "... end %c%c udpip config",
                        _System_Name[0], g_cnt);

            if (sub_flag == 1)
                break;

            continue;
        }
        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            Log(USR_FATAL, "udpip(%c)[%s]", g_cnt, buf);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }
        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1 + 1, c2);
        sp3 = strchr(sp1 + 1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)    strncpy(tmp2, sp1+1, strlen(sp1)-1);
        else                memcpy(tmp2, sp1 + 1, sp2 - sp1 - 1);

        sprintf(tmp3, "%s", "UDPIP_COUNT");
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            itemcnt = atoi(tmp2);
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                if (DAEMON(p_cnt).udpip_count < itemcnt) {
                    Log(USR_FATAL,
                            "udpip(%c):%s[%s]:edit udpip_count in daemon.ini",
                            g_cnt, tmp1, tmp2);
                    fclose(fp);
                    sleep(3);
                    exit(FAIL);
                }
                else
                    DAEMON(p_cnt).u_count = itemcnt;
            }
            continue;
        }

        if (itemcnt < cnt)
            continue;

        sprintf(tmp3, "UDPIP_%d_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                sprintf(UDPIP(p_cnt,cnt-1).info, "%s", sp1+1);
            continue;
        }

        continue_flag = 0;

        for (j = 0; j < 20; j ++) {
            sprintf(tmp3, "UDPIP_%d_IP_%d", cnt, j + 1);
            if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                    if (inet_pton(AF_INET, tmp2, UDPIP(p_cnt,cnt-1).ip_addr[j]) != 1) {
                        Log(USR_FATAL,
                                "udpip(%c,%d):malformed IP address:%s tmp2[%s][%d]",
                                g_cnt, cnt, tmp1, tmp2, strlen(tmp2));
                        sleep(3);
                        exit(FAIL);
                    }
                    else {
                        Log(USR_OK, "udpip(%c,%d):IP address:%s tmp2[%s][%d]",
                                g_cnt, cnt, tmp1, tmp2, strlen(tmp2));
                    }
                }
                continue_flag = 1;
                break;
            }
        }

        if (continue_flag == 1)
            continue;

        sprintf(tmp3, "UDPIP_%d_DUP_ID", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                UDPIP(p_cnt,cnt-1).dup_id = atoi(tmp2);
            continue;
        }

        continue_flag = 0;

        for (j = 0; j < 20; j ++) {
            sprintf(tmp3, "UDPIP_%d_PORT_%d", cnt, j + 1);
            if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                if (flag == 0 || (flag == 1 && D_K == p_cnt))
                    UDPIP(p_cnt,cnt-1).port[j] = atoi(tmp2);
                continue_flag = 1;
                break;
            }
        }

        if (continue_flag == 1)
            continue;

        if (flag == 0 || (flag == 1 && sub_flag == 1)) {
            Log(USR_FATAL, "udpip(%c,%d):%s[%s]", g_cnt, cnt, tmp1, tmp2);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }
        memset(buf, 0, sizeof (buf));
    }
    fclose(fp);

    return;
}   /* End of Udpip_Config_Read ()  */

/*************************************************************************
    Function        : . read proc.ini and set the process SHM (PROC)
    Parameters IN   : . flag (0:set all, 1:set one)
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Proc_Config_Read(int flag)
/*----------------------------------------------------------------------*/
{
    int     len, pt, fd, rt, hh, mm, i, j, cnt = 0;
    int     itemcnt, l1, l2, sub_flag, start_ss, end_ss, ss;
    int     c1 = '=';
    int     c2 = ' ';
    int     c3 = '\t';
    int     p_cnt = 0;
    int     g_cnt = 'A';
    short   ip_tmp[4], dupid;
    char    buf[1024], tmp1[40], tmp2[40], tmp3[40], n1[40], n2[40];
    char    file_name[256], ppath[100], bumun[4], d_time[16];
    char    stat_buf[1024];
    char    dir_name[128], continue_flag;
    char    *sp, *sp1, *sp2, *sp3;
    FILE    *fp;

    sub_flag = 0;
    dupid = 0;

    sprintf(buf, "%s/proc.ini", _FEP_CFG);
    if ((fp = fopen(buf, "r")) == NULL) {
        Log(SAM_FATAL, "fopen failure[%s] {%d:%s}", buf, SYS_NO, SYS_STR);
        sleep(3);
        exit(FAIL);
    }

    memset(buf, 0, sizeof (buf));

    while ((sp = fgets(buf, sizeof (buf), fp)) != NULL) {
        buf[strlen(buf)-1] = 0;
        if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
            continue;

        sprintf(tmp3, "%s", "Proc_End");
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                /* 2025DEBUG: fflush to capture crash point */
                if (cnt <= 3) {
                    Log(USR_OK, "proc_dbg: >>> Proc_End ENTER cnt=%d _FEP_DAT=[%s]",
                            cnt, _FEP_DAT ? _FEP_DAT : "(NULL)");
                    fflush(NULL);
                }
                Log(USR_OK, "proc(%c) Proc_End cnt=%d id=[%s] type=%d",
                        g_cnt, cnt, PROC(p_cnt,cnt-1).process_id,
                        (int)PROC(p_cnt,cnt-1).type);
                if (cnt <= 3) fflush(NULL);
                memcpy(PROC(p_cnt,cnt-1).date, INFO(p_cnt).date, 8);
                if (_FEP_DAT == NULL) {
                    Log(USR_FATAL, "proc(%c,%d): _FEP_DAT is NULL!", g_cnt, cnt);
                    fflush(NULL);
                    sleep(3);
                    exit(FAIL);
                }
                sprintf(ppath, "%s", _FEP_DAT);
                sprintf(bumun, "%-2.2s", PROC(p_cnt,cnt-1).process_id);
                LtoU(bumun, 2);

                if (DAEMON(p_cnt).date_flag == 9) {
                    if (g_cnt == 'W')                   /* TCP��������  */ {
                        cnt ++;
                        continue;
                    }

                    Get_DateTime(d_time);
                    sprintf(dir_name, "%s/%s/%8.8s", ppath, bumun, d_time);
                }
                else
                    sprintf(dir_name, "%s/%s/00000000", ppath, bumun);
                if (cnt <= 3) { Log(USR_OK, "proc_dbg: pre-CreateDir dir=[%s]", dir_name); fflush(NULL); }
                Create_Dir(dir_name);
                if (cnt <= 3) { Log(USR_OK, "proc_dbg: post-CreateDir OK"); fflush(NULL); }

                memset(file_name, 0, sizeof(file_name));
                sprintf(file_name, "%s/%s_stat",
                        dir_name, PROC(p_cnt,cnt-1).process_id);
                if (cnt <= 3) { Log(USR_OK, "proc_dbg: stat_file=[%s]", file_name); fflush(NULL); }
                fd = open(file_name, O_RDWR|O_APPEND|O_CREAT, 0664);
                if (cnt <= 3) { Log(USR_OK, "proc_dbg: stat fd=%d", fd); fflush(NULL); }

                if (fd < 0)
                    Log(SAM_FATAL, "cannot open file[%d,%s] {%d:%s}",
                            fd, file_name, SYS_NO, SYS_STR);

                fchmod(fd, 0664);
                memset(stat_buf, 0, sizeof (stat_buf));
                len = 28;
                pt = 0;
                while (1) {
                    rt = read(fd, stat_buf+pt, len);
                    if (rt < 0) {
                        Log(SAM_FATAL, "cannot read file[%d,%s,%d] {%d:%s}",
                                fd, file_name, len, SYS_NO, SYS_STR);
                        close(fd);
                        break;
                    }
                    else if (rt == 0 || rt == len)
                        break;
                    len -= rt;
                    pt += rt;
                }

                PROC(p_cnt,cnt-1).if_seq = AtoIf(stat_buf, 8);
                /* 2025, ��
                                PROC(p_cnt,cnt-1).start_status = AtoIf (stat_buf+8, 1);
                */
                if (PROC(p_cnt,cnt-1).type == TY_TRS1)          /* TCP1 */ {
                    PROC(p_cnt,cnt-1).start_status = AtoIf(stat_buf+8, 1);
                    PROC(p_cnt,cnt-1).l.t1.network_status =
                    AtoIf(stat_buf+9, 1);
                }
                else if (PROC(p_cnt,cnt-1).type == TY_TRS2)     /* TCP2 */ {
                    PROC(p_cnt,cnt-1).start_status = AtoIf(stat_buf+8 +80, 1);     /* 8*10 */
                    /* 2025 FEP�� ������ ME�� 10��� Seq���� ����� ��, START */
                    int     new_opset = 0;
                    for (i = 0; i < 10; i ++) {
                        new_opset = 8*i;
                        PROC(p_cnt,cnt-1).if_meg_seq[i] = AtoIf(stat_buf+8+new_opset, 8);
                    }
                    /* 2025 FEP�� ������ ME�� 10��� Seq���� ����� ��, END   */
                    /* 2025, ��
                                        PROC(p_cnt,cnt-1).l.t2.line_gubun = AtoIf (stat_buf+9, 1);
                    */
                    PROC(p_cnt,cnt-1).l.t2.line_gubun = AtoIf(stat_buf+9 +80, 1);  /* 8*10 */

                    for (i = 0; i < 2; i ++) {
                        if (PROC(p_cnt,cnt-1).l.t2.l[i]) {
                            TCP2_LSTAT(p_cnt,cnt-1,i) =
                            AtoIf(&stat_buf[10+80+i*3], 1);
                            TCP2_PSTAT(p_cnt,cnt-1,i) =
                            AtoIf(&stat_buf[11+80+i*3], 1);
                            TCP2_NSTAT(p_cnt,cnt-1,i) =
                            AtoIf(&stat_buf[12+80+i*3], 1);
                        }
                    }

                    memcpy(PROC(p_cnt,cnt-1).last_tr , &stat_buf[17+80], 11);
                }
                close(fd);
            }
            if (cnt <= 3) { Log(USR_OK, "proc_dbg: Proc_End cnt=%d done OK", cnt); fflush(NULL); }
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
                    if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                        Log(USR_OK, "start %c%c proc config ... "
                                "pcnt=%d dflag=%d pcount=%d fcount=%d",
                                _System_Name[0], g_cnt, p_cnt,
                                (int)DAEMON(p_cnt).date_flag,
                                DAEMON(p_cnt).process_count,
                                DAEMON(p_cnt).file_count);
                        start_ss = AtoIf(DAEMON(p_cnt).start_time, 2) * 60 *
                        60 + AtoIf(DAEMON(p_cnt).start_time+2, 2) * 60;
                        end_ss = AtoIf(DAEMON(p_cnt).end_time, 2) * 60 *
                        60 + AtoIf(DAEMON(p_cnt).end_time+2, 2) * 60;
                    }

                    if (flag == 1 && D_K == p_cnt)
                        sub_flag = 1;

                    break;
                }
            }
            continue;
        }

        sprintf(tmp3, "%c%c_CONF_END", _System_Name[0], g_cnt);
        if (memcmp(buf, tmp3, Max(strlen(buf),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                Log(USR_OK, "... end %c%c proc config",
                        _System_Name[0], g_cnt);

            if (sub_flag == 1)
                break;

            continue;
        }
        memset(tmp1, 0, sizeof (tmp1));
        memset(tmp2, 0, sizeof (tmp2));

        sp1 = strchr(buf, c1);
        if (sp1 == NULL) {
            Log(USR_FATAL, "proc(%c):[%s]", g_cnt, buf);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }
        memcpy(tmp1, buf, sp1 - buf);
        LtoU(tmp1, sp1 - buf);
        sp2 = strchr(sp1+1, c2);
        sp3 = strchr(sp1+1, c3);

        if (sp3 && sp2 > sp3)   sp2 = sp3;

        if (sp2 == NULL)    strncpy(tmp2, sp1+1, strlen(sp1)-1);
        else                memcpy(tmp2, sp1+1, sp2 - sp1 - 1);

        /* 2025DEBUG: trace each parsed line to find crash point */
        if (cnt <= 3)
            Log(USR_OK, "proc_dbg: cnt=%d key=[%s] val=[%.30s](%d)",
                    cnt, tmp1, tmp2, (int)strlen(tmp2));

        sprintf(tmp3, "%s", "PROC_COUNT");
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            itemcnt = atoi(tmp2);
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                if (DAEMON(p_cnt).process_count < itemcnt) {
                    Log(USR_FATAL,
                            "proc(%c):%s[%s]:edit proc_count in daemon.ini",
                            g_cnt, tmp1, tmp2);
                    fclose(fp);
                    sleep(3);
                    exit(FAIL);
                }
                else {
                    DAEMON(p_cnt).p_count = itemcnt;
                    Log(USR_OK, "proc(%c) PROC_COUNT=%d OK (max=%d) [DBG_V4]",
                            g_cnt, itemcnt, DAEMON(p_cnt).process_count);
                }
            }
            continue;
        }

        if (itemcnt < cnt)
            continue;

        sprintf(tmp3, "PROC_%d_COMMENT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                sprintf(PROC(p_cnt,cnt-1).process_info, "%s", sp1+1);
            continue;
        }

        sprintf(tmp3, "PROC_%d_ID", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                /* 2025EDIT,START */
                strncpy(PROC(p_cnt,cnt-1).process_id, tmp2, sizeof(PROC(p_cnt,cnt-1).process_id));
                /* 2025EDIT,END */
                sprintf(PROC(p_cnt,cnt-1).process_path, "%s/%s",
                        _FEP_BIN, tmp2);
            }
            continue;
        }

        sprintf(tmp3, "PROC_%d_STATUS", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                if (tmp2[0] == 'R')
                    PROC(p_cnt,cnt-1).process_status = 1;
                else
                    PROC(p_cnt,cnt-1).process_status = 9;
            }
            continue;
        }

        /* input file   */
        sprintf(tmp3, "PROC_%d_IFN_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            continue_flag = 0;

            for (j = 0; j < 3; j ++) {
                sprintf(tmp3, "PROC_%d_IFN_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                        for (i = 0; i < DAEMON(p_cnt).f_count; i ++) {
                            if (memcmp(tmp2, FILEM(p_cnt,i).file_name,
                                    Max(strlen(tmp2),
                                    strlen(FILEM(p_cnt,i).file_name))) == 0) {
                                PROC(p_cnt,cnt-1).in_f[j] = i + 1;
                                break;
                            }

                            if (i == DAEMON(p_cnt).f_count - 1) {
                                Log(USR_FATAL, "proc(%c,%d):IFN%d:%s[%s]",
                                        g_cnt, cnt, j + 1, tmp1, tmp2);
                                sleep(3);
                                exit(FAIL);
                            }
                        }
                    }
                    continue_flag = 1;
                    break;
                }
            }

            if (continue_flag == 1)
                continue;
        }

        /* input data SHM   */
        sprintf(tmp3, "PROC_%d_IDN_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            continue_flag = 0;

            for (j = 0; j < 3; j ++) {
                sprintf(tmp3, "PROC_%d_IDN_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                        for (i = 0; i < DAEMON(p_cnt).d_count; i ++) {
                            if (memcmp(tmp2, DSHM(p_cnt,i).data_name,
                                    Max(strlen(tmp2),
                                    strlen(DSHM(p_cnt,i).data_name))) == 0) {
                                PROC(p_cnt,cnt-1).in_d[j] = i + 1;
                                break;
                            }

                            if (i == DAEMON(p_cnt).d_count - 1) {
                                Log(USR_FATAL, "proc(%c,%d):IDN%d:%s[%s]",
                                        g_cnt, cnt, j + 1, tmp1, tmp2);
                                sleep(3);
                                exit(FAIL);
                            }
                        }
                    }

                    continue_flag = 1;
                    break;
                }
            }

            if (continue_flag == 1)
                continue;
        }

        /* input fifo name  */
        sprintf(tmp3, "PROC_%d_FFN_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            continue_flag = 0;

            for (j = 0; j < 3; j ++) {
                sprintf(tmp3, "PROC_%d_FFN_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    if (flag == 0 || (flag == 1 && D_K == p_cnt))
                        sprintf(PROC(p_cnt,cnt-1).fifo_f[j], "%s", tmp2);

                    continue_flag = 1;
                    break;
                }
            }

            if (continue_flag == 1)
                continue;
        }

#if defined ISAM_INCL
        /* input C-ISAM */
        sprintf(tmp3, "PROC_%d_ICN_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            continue_flag = 0;

            for (j = 0; j < 3; j ++) {
                sprintf(tmp3, "PROC_%d_ICN_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                        for (i = 0; i < DAEMON(p_cnt).c_count; i ++) {
                            if (memcmp(tmp2, CISAM(p_cnt,i).file_name,
                                    Max(strlen(tmp2),
                                    strlen(CISAM(p_cnt,i).file_name))) == 0) {
                                PROC(p_cnt,cnt-1).in_c[j] = i + 1;
                                break;
                            }

                            if (i == DAEMON(p_cnt).c_count - 1) {
                                Log(USR_FATAL, "proc(%c,%d):ICN%d:%s[%s]",
                                        g_cnt, cnt, j + 1, tmp1, tmp2);
                                sleep(3);
                                exit(FAIL);
                            }
                        }
                    }

                    continue_flag = 1;
                    break;
                }
            }

            if (continue_flag == 1)
                continue;
        }
#endif

        /* output file  */
        sprintf(tmp3, "PROC_%d_OFN_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            continue_flag = 0;

            for (j = 0; j < 99; j ++) {
                sprintf(tmp3, "PROC_%d_OFN_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                        for (i = 0; i < DAEMON(p_cnt).f_count; i ++) {
                            if (memcmp(tmp2, FILEM(p_cnt,i).file_name,
                                    Max(strlen(tmp2),
                                    strlen(FILEM(p_cnt,i).file_name))) == 0) {
                                PROC(p_cnt,cnt-1).out_f[j] = i + 1;
                                break;
                            }

                            if (i == DAEMON(p_cnt).f_count - 1) {
                                Log(USR_FATAL, "proc(%c,%d):OFN%d:%s[%s]",
                                        g_cnt, cnt, j + 1, tmp1, tmp2);
                                sleep(3);
                                exit(FAIL);
                            }
                        }
                    }

                    continue_flag = 1;
                    break;
                }
            }

            if (continue_flag == 1)
                continue;
        }

        /* output data SHM  */
        sprintf(tmp3, "PROC_%d_ODN_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            continue_flag = 0;

            for (j = 0; j < 99; j ++) {
                sprintf(tmp3, "PROC_%d_ODN_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                        for (i = 0; i < DAEMON(p_cnt).d_count; i ++) {
                            if (memcmp(tmp2, DSHM(p_cnt,i).data_name,
                                    Max(strlen(tmp2),
                                    strlen(DSHM(p_cnt,i).data_name))) == 0) {
                                PROC(p_cnt,cnt-1).out_d[j] = i + 1;
                                break;
                            }

                            if (i == DAEMON(p_cnt).d_count - 1) {
                                Log(USR_FATAL, "proc(%c,%d):ODN%d:%s[%s]",
                                        g_cnt, cnt, j + 1, tmp1, tmp2);
                                sleep(3);
                                exit(FAIL);
                            }
                        }
                    }

                    continue_flag = 1;
                    break;
                }
            }

            if (continue_flag == 1)
                continue;
        }

#if defined ISAM_INCL
        /* output C-ISAM    */
        sprintf(tmp3, "PROC_%d_OCN_", cnt);
        if (memcmp(tmp1, tmp3, strlen(tmp3)) == 0) {
            continue_flag = 0;

            for (j = 0; j < 3; j ++) {
                sprintf(tmp3, "PROC_%d_OCN_%d", cnt, j + 1);
                if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
                    if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                        for (i = 0; i < DAEMON(p_cnt).c_count; i ++) {
                            if (memcmp(tmp2, CISAM(p_cnt,i).file_name,
                                    Max(strlen(tmp2),
                                    strlen(CISAM(p_cnt,i).file_name))) == 0) {
                                PROC(p_cnt,cnt-1).out_c[j] = i + 1;
                                break;
                            }

                            if (i == DAEMON(p_cnt).c_count - 1) {
                                Log(USR_FATAL, "proc(%c,%d):OCN%d:%s[%s]",
                                        g_cnt, cnt, j + 1, tmp1, tmp2);
                                sleep(3);
                                exit(FAIL);
                            }
                        }
                    }

                    continue_flag = 1;
                    break;
                }
            }

            if (continue_flag == 1)
                continue;
        }
#endif

        sprintf(tmp3, "PROC_%d_TYPE", cnt);

        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                if (tmp2[0] == 'M')
                    PROC(p_cnt,cnt-1).type = TY_MP;
                else if (tmp2[0] == 'D')
                    PROC(p_cnt,cnt-1).type = TY_DD;
                else if (tmp2[0] == 'T' && tmp2[2] == '1')
                    PROC(p_cnt,cnt-1).type = TY_TRS1;
                else if (tmp2[0] == 'T' && tmp2[2] == '2')
                    PROC(p_cnt,cnt-1).type = TY_TRS2;
                else if (tmp2[0] == 'U')
                    PROC(p_cnt,cnt-1).type = TY_URS;
                else if (tmp2[0] == 'B')
                    PROC(p_cnt,cnt-1).type = TY_BRS;
                /* 2025EDIT,START */
                strncpy(PROC(p_cnt,cnt-1).process_type, tmp2, sizeof(PROC(p_cnt,cnt-1).process_type));
                /* 2025EDIT,START */
            }
            continue;
        }

        sprintf(tmp3, "PROC_%d_START_TIME", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                hh = AtoIf(tmp2, 2);
                mm = AtoIf(tmp2+2, 2);
                if (hh < 0 || hh > 23 || mm < 0 || mm > 59) {
                    Log(USR_FATAL,
                            "proc(%c,%d):invalid process start time[%s]",
                            g_cnt, cnt, tmp2);
                    sleep(3);
                    exit(FAIL);
                }

                ss = hh * 60 * 60 + mm * 60;

                if (((DAEMON(p_cnt).date_flag == 2 ||
                        DAEMON(p_cnt).date_flag == 4) &&
                        (ss < start_ss && ss >= end_ss)) ||
                        ((DAEMON(p_cnt).date_flag == 1 ||
                        DAEMON(p_cnt).date_flag == 3 ||
                        DAEMON(p_cnt).date_flag == 5) &&
                        (ss < start_ss || ss >= end_ss))) {
                    Log(USR_FATAL,
                            "proc(%c,%d):invalid process start time[%s:%s]",
                            g_cnt, cnt, tmp2, DAEMON(p_cnt).start_time);

                    memcpy(PROC(p_cnt,cnt-1).end_time, DAEMON(p_cnt).end_time,
                            sizeof (DAEMON(p_cnt).end_time));
                    /*
                                        sprintf (PROC(p_cnt,cnt-1).start_time, "%.4s",
                                            DAEMON(p_cnt).start_time);
                    */
                }
                else {
                    /* 2025EDIT,START */
                    strncpy(PROC(p_cnt,cnt-1).start_time, tmp2, sizeof(PROC(p_cnt,cnt-1).start_time));
                    /* 2025EDIT,END */
                }
            }
            continue;
        }

        sprintf(tmp3, "PROC_%d_END_TIME", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                hh = AtoIf(tmp2, 2);
                mm = AtoIf(tmp2+2, 2);
                if (hh < 0 || hh > 23 || mm < 0 || mm > 59) {
                    Log(USR_FATAL,
                            "proc(%c,%d):invalid process end time[%s]",
                            g_cnt, cnt, tmp2);
                    sleep(3);
                    exit(FAIL);
                }

                ss = hh * 60 * 60 + mm * 60;

                if (((DAEMON(p_cnt).date_flag == 2 ||
                        DAEMON(p_cnt).date_flag == 4) &&
                        (ss <= start_ss && ss > end_ss)) ||
                        ((DAEMON(p_cnt).date_flag == 1 ||
                        DAEMON(p_cnt).date_flag == 3 ||
                        DAEMON(p_cnt).date_flag == 5) &&
                        (ss <= start_ss || ss > end_ss))) {
                    Log(USR_FATAL,
                            "proc(%c,%d):invalid process end time[%s:%s]",
                            g_cnt, cnt, tmp2, DAEMON(p_cnt).end_time);
                    /*
                                        sprintf (PROC(p_cnt,cnt-1).end_time, "%.4s",
                                            DAEMON(p_cnt).end_time);
                    */
                    memcpy(PROC(p_cnt,cnt-1).end_time, DAEMON(p_cnt).end_time,
                            sizeof (DAEMON(p_cnt).end_time));
                }
                else {
                    /* 2025EDIT,START */
                    strncpy(PROC(p_cnt,cnt-1).end_time, tmp2, sizeof(PROC(p_cnt,cnt-1).end_time));
                    /* 2025EDIT,START */
                }
            }
            continue;
        }

        sprintf(tmp3, "PROC_%d_TIME_OUT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                PROC(p_cnt,cnt-1).timeout = atoi(tmp2);
            continue;
        }

        sprintf(tmp3, "PROC_%d_DELAY", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt))
                PROC(p_cnt,cnt-1).delay = atoi(tmp2);
            continue;
        }

        sprintf(tmp3, "PROC_%d_DATA_BUF", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                if (tmp2[0] == 'Y' && Data_I < DATA_BUF_CNT) {
                    PROC(p_cnt,cnt-1).data_flag = 1;
                    PROC(p_cnt,cnt-1).data = (char *)Data_Ptr[Data_I++];
                }
            }
            continue;
        }

        sprintf(tmp3, "PROC_%d_TCP1_TYPE", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                if (PROC(p_cnt,cnt-1).type == TY_TRS1) {
                    if (tmp2[0] == 'M')
                        PROC(p_cnt,cnt-1).l.t1.port_type = 1;
                    else if (tmp2[0] == 'S')
                        PROC(p_cnt,cnt-1).l.t1.port_type = 2;
                }
            }
            continue;
        }

        sprintf(tmp3, "PROC_%d_TCP1_PORT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                if (PROC(p_cnt,cnt-1).type == TY_TRS1) {
                    for (i = 0; i < DAEMON(p_cnt).t1_count; i ++) {
                        if (PROC(p_cnt,cnt-1).l.t1.port_type == 1) {
                            if (atoi(tmp2) == TCP1(p_cnt,i).port_no) {
                                PROC(p_cnt,cnt-1).l.t1.line_gubun = i + 1;
                                PROC(p_cnt,cnt-1).l.t1.port_type = 0;
                                break;
                            }
                        }
                        else {
                            for (j = 0; j < 9; j ++) {
                                if (atoi(tmp2) ==
                                        TCP1(p_cnt,i).service_port_no[j]) {
                                    PROC(p_cnt,cnt-1).l.t1.line_gubun = i + 1;
                                    PROC(p_cnt,cnt-1).l.t1.port_type = j + 1;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            continue;
        }

        sprintf(tmp3, "PROC_%d_TCP2_PRIMARY", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if ((flag == 0 || (flag == 1 && D_K == p_cnt)) &&
                    PROC(p_cnt,cnt-1).type == TY_TRS2) {
                for (j = 0; j < strlen(tmp2) && tmp2[j] != ','; j ++)
                    ;

                for (i = 0; i < DAEMON(p_cnt).t2_count; i ++) {
                    if (AtoIf(tmp2, j) == TCP2(p_cnt,i).dup_id &&
                            atoi(&tmp2[j+1]) == TCP2(p_cnt,i).port_no) {
                        PROC(p_cnt,cnt-1).l.t2.l[0] = i + 1;
                        break;
                    }

                    if (i == DAEMON(p_cnt).t2_count - 1) {
                        Log(USR_FATAL, "proc(%c,%d):tcp2 primary:%s[%s]",
                                g_cnt, cnt, tmp1, tmp2);
                        sleep(3);
                        exit(FAIL);
                    }
                }
            }
            continue;
        }

        sprintf(tmp3, "PROC_%d_TCP2_BACKUP", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if ((flag == 0 || (flag == 1 && D_K == p_cnt)) &&
                    PROC(p_cnt,cnt-1).type == TY_TRS2) {
                for (j = 0; j < strlen(tmp2) && tmp2[j] != ','; j ++)
                    ;

                for (i = 0; i < DAEMON(p_cnt).t2_count; i ++) {
                    if (AtoIf(tmp2, j) == TCP2(p_cnt,i).dup_id &&
                            atoi(&tmp2[j+1]) == TCP2(p_cnt,i).port_no) {
                        PROC(p_cnt,cnt-1).l.t2.l[1] = i + 1;
                        PROC(p_cnt,cnt-1).backup = 1;
                        break;
                    }

                    if (i == DAEMON(p_cnt).t2_count - 1) {
                        Log(USR_FATAL, "proc(%c,%d):tcp2 backup:%s[%s]",
                                g_cnt, cnt, tmp1, tmp2);
                        sleep(3);
                        exit(FAIL);
                    }
                }
            }
            continue;
        }

        sprintf(tmp3, "PROC_%d_UDP_PORT", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if ((flag == 0 || (flag == 1 && D_K == p_cnt)) &&
                    PROC(p_cnt,cnt-1).type == TY_URS) {
                for (j = 0; j < strlen(tmp2) && tmp2[j] != ','; j ++)
                    ;

                for (i = 0; i < DAEMON(p_cnt).u_count; i ++) {
                    if (AtoIf(tmp2, j) == UDPIP(p_cnt,i).dup_id &&
                            atoi(&tmp2[j+1]) == UDPIP(p_cnt,i).port[0]) {
                        PROC(p_cnt,cnt-1).l.u = i + 1;
                        break;
                    }

                    if (i == DAEMON(p_cnt).u_count - 1) {
                        Log(USR_FATAL, "proc(%c,%d):udpip:%s[%s]",
                                g_cnt, cnt, tmp1, tmp2);
                        sleep(3);
                        exit(FAIL);
                    }
                }
            }
            continue;
        }

        sprintf(tmp3, "PROC_%d_LOGONID", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                /* 2025EDIT,START */
                strncpy(PROC(p_cnt,cnt-1).logon_id, tmp2, sizeof(PROC(p_cnt,cnt-1).logon_id));
                /* 2025EDIT,END */
            }
            continue;
        }

        sprintf(tmp3, "PROC_%d_LOGONPW", cnt);
        if (memcmp(tmp1, tmp3, Max(strlen(tmp1),strlen(tmp3))) == 0) {
            if (flag == 0 || (flag == 1 && D_K == p_cnt)) {
                /* 2025EDIT,START */
                strncpy(PROC(p_cnt,cnt-1).logon_pw, tmp2, sizeof(PROC(p_cnt,cnt-1).logon_pw));
                /* 2025EDIT,END */
            }
            continue;
        }

        if (flag == 0 || (flag == 1 && sub_flag == 1)) {
            Log(USR_FATAL, "proc(%c,%d):%s[%s]", g_cnt, cnt, tmp1, tmp2);
            fclose(fp);
            sleep(3);
            exit(FAIL);
        }
        memset(buf, 0, sizeof (buf));
    }
    fclose(fp);

    return;
}   /* End of Proc_Config_Read ()   */

/*************************************************************************
    End of Program (pz_memory_conf.c)
*************************************************************************/
