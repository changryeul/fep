#if !defined(_LARGE_FILES)
#define     _LARGE_FILES
#endif
/*------------------------------------------------------------------------
#   Module  : SQLite DB access layer for FEP configuration
#   File    : config_db.c
#   Description : Provides cfg_db_*() functions for DB-based config
#                 loading as alternative to .ini file parsing.
#                 Phase 1: Connection management + utility functions.
#                 Phase 2: cfg_db_load_tcp1/tcp2/udpip/proc/ip_whitelist.
#                 Phase 2b: cfg_db_load_daemon/file/dshm (all stubs done).
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"
#include    "config_db.h"

/*------------------------------------------------------------------------
    Static Variables
------------------------------------------------------------------------*/
static sqlite3      *g_db = NULL;       /* DB connection handle         */
static char         g_env_id[10];       /* cached environment ID        */
static int          g_env_detected = 0; /* env detection flag           */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
static void cfg_db_proc_init_stat(int p_cnt, int proc_idx, int g_cnt);

int     cfg_db_open(const char *db_path);
void    cfg_db_close(void);
int     cfg_db_is_open(void);
const char* cfg_db_detect_env(void);

int     cfg_db_load_daemon(int flag);
int     cfg_db_load_file(int flag);
int     cfg_db_load_dshm(int flag);
int     cfg_db_load_tcp1(int flag);
int     cfg_db_load_tcp2(int flag);
int     cfg_db_load_udpip(int flag);
int     cfg_db_load_sisetr(int flag);
int     cfg_db_load_proc(int flag);
int     cfg_db_load_ip_whitelist(char ip_list[][20], int max_cnt,
        const char *ip_type);

int     cfg_db_set_value(const char *env_id, const char *group_id,
        const char *section, const char *key_name,
        const char *new_value, const char *changed_by);

int     cfg_db_export_ini(const char *group_id, const char *ini_path);
int     cfg_db_sync_all_ini(void);

int     cfg_db_encrypt_pw(const char *plain, char *cipher);
int     cfg_db_decrypt_pw(const char *cipher, char *plain);

int     cfg_db_get_value(const char *env_id, const char *group_id,
        const char *section, const char *key_name,
        char *out_buf, int buf_len);
int     cfg_db_get_int(const char *env_id, const char *group_id,
        const char *section, const char *key_name,
        int default_val);
int     cfg_db_get_key_count(const char *env_id, const char *group_id);

/*************************************************************************
    Function        : . open SQLite DB file
    Parameters IN   : . db_path (NULL = $_FEP_CFG/fep_config.db)
    Parameters OUT  : .
    Return Code     : . CFG_DB_OK or CFG_DB_ERR_OPEN
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_open(const char *db_path)
/*----------------------------------------------------------------------*/
{
    char    path[256];
    int     rc;

    /* already open */
    if (g_db != NULL)
        return CFG_DB_OK;

    /* determine DB path */
    if (db_path != NULL && db_path[0] != '\0') {
        strncpy(path, db_path, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    }
    else {
        char    *db_env;

        db_env = getenv("_FEP_DB");
        if (db_env != NULL && db_env[0] != '\0') {
            strncpy(path, db_env, sizeof(path) - 1);
            path[sizeof(path) - 1] = '\0';
        }
        else {
            snprintf(path, sizeof(path), "%s/fep_config.db", _FEP_CFG);
        }
    }

    /* check file existence */
    if (access(path, R_OK) != 0) {
        Log(ORA_WARN, "cfg_db_open: DB file not found [%s]", path);
        return CFG_DB_ERR_OPEN;
    }

    /* open with read-only + WAL mode */
    rc = sqlite3_open_v2(path, &g_db,
            SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX, NULL);
    if (rc != SQLITE_OK) {
        Log(ORA_ERROR, "cfg_db_open: sqlite3_open failed [%s] rc=%d: %s",
                path, rc, sqlite3_errmsg(g_db));
        sqlite3_close(g_db);
        g_db = NULL;
        return CFG_DB_ERR_OPEN;
    }

    /* set busy timeout (100ms) */
    sqlite3_busy_timeout(g_db, 100);

    /* verify schema version */
    {
        sqlite3_stmt    *stmt = NULL;
        const char      *sql;
        const char      *ver;

        sql = "SELECT key_value FROM fep_db_meta "
        "WHERE key_name='schema_version'";
        rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK || sqlite3_step(stmt) != SQLITE_ROW) {
            Log(ORA_WARN, "cfg_db_open: schema version check failed");
            sqlite3_finalize(stmt);
            sqlite3_close(g_db);
            g_db = NULL;
            return CFG_DB_ERR_OPEN;
        }

        ver = (const char *) sqlite3_column_text(stmt, 0);
        if (ver == NULL || strcmp(ver, "1.0") != 0) {
            Log(ORA_WARN, "cfg_db_open: unsupported schema version [%s]",
                    ver ? ver : "NULL");
            sqlite3_finalize(stmt);
            sqlite3_close(g_db);
            g_db = NULL;
            return CFG_DB_ERR_OPEN;
        }
        sqlite3_finalize(stmt);
    }

    /* update last_loaded_at */
    {
        sqlite3_stmt    *stmt = NULL;
        const char      *sql;
        char            dt[20];

        sql = "UPDATE fep_db_meta SET key_value=?, updated_at=? "
        "WHERE key_name='last_loaded_at'";

        /* re-open as read-write for meta update */
        /* skip meta update if read-only - not critical */
    }

    Log(ORA_OK, "cfg_db_open: DB opened [%s]", path);
    return CFG_DB_OK;

}   /* End of cfg_db_open () */

/*************************************************************************
    Function        : . close SQLite DB file
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    cfg_db_close(void)
/*----------------------------------------------------------------------*/
{
    if (g_db != NULL) {
        sqlite3_close(g_db);
        g_db = NULL;
        Log(ORA_OK, "cfg_db_close: DB closed");
    }

    return;
}   /* End of cfg_db_close () */

/*************************************************************************
    Function        : . check DB connection status
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . 1 (connected) or 0 (not connected)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_is_open(void)
/*----------------------------------------------------------------------*/
{
    return (g_db != NULL) ? 1 : 0;

}   /* End of cfg_db_is_open () */

/*************************************************************************
    Function        : . detect current environment ID based on hostname
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . env_id string pointer
*************************************************************************/
/*----------------------------------------------------------------------*/
const char* cfg_db_detect_env(void)
/*----------------------------------------------------------------------*/
{
    char    hostname[64];

    /* return cached value */
    if (g_env_detected)
        return g_env_id;

    memset(hostname, 0, sizeof(hostname));

    if (gethostname(hostname, sizeof(hostname) - 1) != 0) {
        Log(USR_WARN, "cfg_db_detect_env: gethostname failed, using TEST");
        strncpy(g_env_id, "TEST", sizeof(g_env_id) - 1);
        g_env_detected = 1;
        return g_env_id;
    }

    if (strcmp(hostname, "podm11") == 0)
        strncpy(g_env_id, "REAL1", sizeof(g_env_id) - 1);
    else if (strcmp(hostname, "podm12") == 0)
        strncpy(g_env_id, "REAL2", sizeof(g_env_id) - 1);
    else
        strncpy(g_env_id, "TEST", sizeof(g_env_id) - 1);

    g_env_id[sizeof(g_env_id) - 1] = '\0';
    g_env_detected = 1;

    Log(ORA_OK, "cfg_db_detect_env: hostname=[%s] env_id=[%s]",
            hostname, g_env_id);

    return g_env_id;

}   /* End of cfg_db_detect_env () */

/*========================================================================
    Config Load Functions (DB -> SHM)
    Phase 2: tcp1, tcp2, udpip, proc, ip_whitelist - full implementations
    Phase 2b: daemon, file, dshm - fully implemented
========================================================================*/

/*************************************************************************
    Function        : . load daemon.ini config from DB -> INFO()/DAEMON()
    Parameters IN   : . flag (0:INFO all, 1:DAEMON+INFO current, 3:reload)
    Parameters OUT  : . INFO(), DAEMON() SHM structs + FIFO files
    Return Code     : . CFG_DB_OK or error code
    DB key_name     : COMMENT, ID, START_TIME, END_TIME, DATE_FLAG,
                      COMPACT_DAYS, STATUS, FIFO,
                      PROC_COUNT, FILE_COUNT, DSHM_COUNT,
                      TCP1_COUNT, TCP2_COUNT, UDPIP_COUNT,
                      DATA_COUNT, SHM_LOG
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_load_daemon(int flag)
/*----------------------------------------------------------------------*/
{
    sqlite3_stmt    *stmt = NULL;
    int             rc, p_cnt;
    int             start_ss, end_ss, tmp_ss;
    char            g_cnt;
    const char      *env_id, *section, *key, *val;
    time_t          t;
    struct tm       *tp, *date;
    char            dt[10], bumun[4], file_name[256];

    if (g_db == NULL)
        return CFG_DB_ERR_OPEN;

    /* flag 2: temp buffer mode, not supported in DB path */
    if (flag == 2)
        return CFG_DB_OK;

    env_id = cfg_db_detect_env();

    rc = sqlite3_prepare_v2(g_db,
            "SELECT section, key_name, key_value FROM fep_config "
            "WHERE env_id=? AND group_id='daemon' "
            "ORDER BY section, sort_order, key_name",
            -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        Log(ORA_WARN, "cfg_db_load_daemon: prepare failed(%d)", rc);
        return CFG_DB_ERR_QUERY;
    }

    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        section = (const char *) sqlite3_column_text(stmt, 0);
        key     = (const char *) sqlite3_column_text(stmt, 1);
        val     = (const char *) sqlite3_column_text(stmt, 2);

        if (!section || !key || !val)
            continue;

        g_cnt = section[0];
        p_cnt = g_cnt - 'A';

        if (p_cnt < 0 || p_cnt > 25)
            continue;

        /* flag filtering: skip non-matching daemon */
        if ((flag == 1 || flag == 3) && D_K != p_cnt)
            continue;

        /*-- COMMENT --*/
        if (strcmp(key, "COMMENT") == 0) {
            if (flag == 1) {
                strncpy(DAEMON(p_cnt).process_info, val,
                        sizeof (DAEMON(p_cnt).process_info));
                DAEMON(p_cnt).system_status = ON;
                INFO(p_cnt).system_status = ON;
            }
            continue;
        }

        /*-- ID --*/
        if (strcmp(key, "ID") == 0) {
            if (flag == 0 || flag == 3) {
                strncpy(INFO(p_cnt).process_id, val,
                        sizeof (INFO(p_cnt).process_id));
            }
            else if (flag == 1) {
                strncpy(DAEMON(p_cnt).process_id, val,
                        sizeof (DAEMON(p_cnt).process_id));
                sprintf(DAEMON(p_cnt).process_path, "%s/%s",
                        _FEP_BIN, val);
                strncpy(INFO(p_cnt).process_id, val,
                        sizeof (INFO(p_cnt).process_id));
            }
            continue;
        }

        /*-- START_TIME --*/
        if (strcmp(key, "START_TIME") == 0) {
            if (flag == 0 || flag == 3) {
                strncpy(INFO(p_cnt).start_time, val,
                        sizeof (INFO(p_cnt).start_time));
            }
            else if (flag == 1) {
                strncpy(DAEMON(p_cnt).start_time, val,
                        sizeof (DAEMON(p_cnt).start_time));
                strncpy(INFO(p_cnt).start_time, val,
                        sizeof (INFO(p_cnt).start_time));
            }
            continue;
        }

        /*-- END_TIME --*/
        if (strcmp(key, "END_TIME") == 0) {
            if (flag == 0 || flag == 3) {
                strncpy(INFO(p_cnt).end_time, val,
                        sizeof (INFO(p_cnt).end_time));
            }
            else if (flag == 1) {
                strncpy(DAEMON(p_cnt).end_time, val,
                        sizeof (DAEMON(p_cnt).end_time));
                strncpy(INFO(p_cnt).end_time, val,
                        sizeof (INFO(p_cnt).end_time));
            }
            continue;
        }

        /*-- DATE_FLAG (includes work date calculation) --*/
        if (strcmp(key, "DATE_FLAG") == 0) {
            if (flag == 0 || flag == 3) {
                INFO(p_cnt).date_flag = atoi(val);

                if (INFO(p_cnt).date_flag != 9) {
                    start_ss = AtoIf(INFO(p_cnt).start_time, 2) * 3600 +
                    AtoIf(INFO(p_cnt).start_time+2, 2) * 60;
                    end_ss = AtoIf(INFO(p_cnt).end_time, 2) * 3600 +
                    AtoIf(INFO(p_cnt).end_time+2, 2) * 60;

                    memset(dt, 0, sizeof (dt));
                    time(&t);
                    date = localtime(&t);
                    tmp_ss = (date->tm_hour * 3600)
                    + (date->tm_min * 60) + date->tm_sec;

                    switch (INFO(p_cnt).date_flag) {
                        case 1:
                            break;
                        case 2:
                            if (tmp_ss <= end_ss)
                                t -= 86400L;
                            break;
                        case 3:
                            t -= 86400L;
                            break;
                        case 4:
                            if (tmp_ss >= start_ss)
                                t += 86400L;
                            break;
                        case 5:
                            t += 86400L;
                            break;
                    }

                    tp = localtime(&t);
                    strftime(dt, 10, "%Y%m%d", tp);
                    memcpy(INFO(p_cnt).date, dt, 8);
                    Log(USR_OK,
                            "set work date(INFO) [%c%c,%s]",
                            _System_Name[0], g_cnt, INFO(p_cnt).date);
                }
            }
            else if (flag == 1) {
                DAEMON(p_cnt).date_flag = atoi(val);
                INFO(p_cnt).date_flag = atoi(val);

                if (DAEMON(p_cnt).date_flag != 9) {
                    start_ss =
                    AtoIf(DAEMON(p_cnt).start_time, 2) * 3600 +
                    AtoIf(DAEMON(p_cnt).start_time+2, 2) * 60;
                    end_ss =
                    AtoIf(DAEMON(p_cnt).end_time, 2) * 3600 +
                    AtoIf(DAEMON(p_cnt).end_time+2, 2) * 60;

                    memset(dt, 0, sizeof (dt));
                    time(&t);
                    date = localtime(&t);
                    tmp_ss = (date->tm_hour * 3600)
                    + (date->tm_min * 60) + date->tm_sec;

                    switch (DAEMON(p_cnt).date_flag) {
                        case 1:
                            break;
                        case 2:
                            if (tmp_ss <= end_ss)
                                t -= 86400L;
                            break;
                        case 3:
                            t -= 86400L;
                            break;
                        case 4:
                            if (tmp_ss >= start_ss)
                                t += 86400L;
                            break;
                        case 5:
                            t += 86400L;
                            break;
                    }

                    tp = localtime(&t);
                    strftime(dt, 10, "%Y%m%d", tp);
                    memcpy(DAEMON(p_cnt).date, dt, 8);
                    memcpy(INFO(p_cnt).date, dt, 8);
                    Log(USR_OK,
                            "set work date(DAEMON) [%c%c,%s]",
                            _System_Name[0], g_cnt,
                            DAEMON(p_cnt).date);
                }
            }
            continue;
        }

        /*-- COMPACT_DAYS --*/
        if (strcmp(key, "COMPACT_DAYS") == 0) {
            if (flag == 0 || flag == 3)
                INFO(p_cnt).compact_days = atoi(val);
            else if (flag == 1) {
                DAEMON(p_cnt).compact_days = atoi(val);
                INFO(p_cnt).compact_days = atoi(val);
            }
            continue;
        }

        /*-- STATUS --*/
        if (strcmp(key, "STATUS") == 0) {
            if (flag == 0 || (flag == 3 && val[0] == '0'))
                INFO(p_cnt).process_status = atoi(val);
            else if (flag == 1) {
                DAEMON(p_cnt).process_status = atoi(val);
                INFO(p_cnt).process_status = atoi(val);
            }
            continue;
        }

        /*-- FIFO (creates FIFO files on filesystem) --*/
        if (strcmp(key, "FIFO") == 0) {
            sprintf(bumun, "%-2.2s", val);
            LtoU(bumun, 2);

            if (flag == 0 || flag == 3) {
                strncpy(INFO(p_cnt).start_FIFO_name, val,
                        sizeof (INFO(p_cnt).start_FIFO_name));

                /* exit FIFO name */
                {
                    char tbuf[20];
                    size_t mlen = sizeof (tbuf) - 6;
                    strncpy(tbuf, val, mlen);
                    tbuf[mlen] = '\0';
                    strncat(tbuf, "_exit", sizeof (tbuf) - strlen(tbuf) - 1);
                    strncpy(INFO(p_cnt).exit_FIFO_name, tbuf, sizeof(INFO(p_cnt).exit_FIFO_name) - 1);
                    INFO(p_cnt).exit_FIFO_name[sizeof(INFO(p_cnt).exit_FIFO_name) - 1] = '\0';
                }

                /* ctrl FIFO name */
                {
                    char tbuf[20];
                    size_t mlen = sizeof (tbuf) - 6;
                    strncpy(tbuf, val, mlen);
                    tbuf[mlen] = '\0';
                    strncat(tbuf, "_ctrl", sizeof (tbuf) - strlen(tbuf) - 1);
                    strncpy(INFO(p_cnt).daemon_FIFO_name, tbuf, sizeof(INFO(p_cnt).daemon_FIFO_name) - 1);
                    INFO(p_cnt).daemon_FIFO_name[sizeof(INFO(p_cnt).daemon_FIFO_name) - 1] = '\0';
                }

                /* create 4 FIFOs */
                sprintf(file_name, "%s/%-2.2s/%s",
                        _FEP_FIFO, bumun, val);
                Create_FIFO(file_name);

                sprintf(file_name, "%s/%-2.2s/%s_exit",
                        _FEP_FIFO, bumun, val);
                Create_FIFO(file_name);

                sprintf(file_name, "%s/%-2.2s/%s_ctrl",
                        _FEP_FIFO, bumun, val);
                Create_FIFO(file_name);

                sprintf(file_name, "%s/%-2.2s/%s_slog",
                        _FEP_FIFO, bumun, val);
                Create_FIFO(file_name);
            }
            else if (flag == 1) {
                strncpy(DAEMON(p_cnt).start_FIFO_name, val,
                        sizeof (DAEMON(p_cnt).start_FIFO_name));
                strncpy(INFO(p_cnt).start_FIFO_name, val,
                        sizeof (INFO(p_cnt).start_FIFO_name));

                /* exit FIFO name */
                {
                    char tbuf[20];
                    size_t mlen = sizeof (tbuf) - 6;
                    strncpy(tbuf, val, mlen);
                    tbuf[mlen] = '\0';
                    strncat(tbuf, "_exit", sizeof (tbuf) - strlen(tbuf) - 1);
                    strncpy(DAEMON(p_cnt).exit_FIFO_name, tbuf, sizeof(DAEMON(p_cnt).exit_FIFO_name) - 1);
                    DAEMON(p_cnt).exit_FIFO_name[sizeof(DAEMON(p_cnt).exit_FIFO_name) - 1] = '\0';
                    strncpy(INFO(p_cnt).exit_FIFO_name, tbuf, sizeof(INFO(p_cnt).exit_FIFO_name) - 1);
                    INFO(p_cnt).exit_FIFO_name[sizeof(INFO(p_cnt).exit_FIFO_name) - 1] = '\0';
                }

                /* ctrl FIFO name */
                {
                    char tbuf[20];
                    size_t mlen = sizeof (tbuf) - 6;
                    strncpy(tbuf, val, mlen);
                    tbuf[mlen] = '\0';
                    strncat(tbuf, "_ctrl", sizeof (tbuf) - strlen(tbuf) - 1);
                    strncpy(DAEMON(p_cnt).daemon_FIFO_name, tbuf, sizeof(DAEMON(p_cnt).daemon_FIFO_name) - 1);
                    DAEMON(p_cnt).daemon_FIFO_name[sizeof(DAEMON(p_cnt).daemon_FIFO_name) - 1] = '\0';
                    strncpy(INFO(p_cnt).daemon_FIFO_name, tbuf, sizeof(INFO(p_cnt).daemon_FIFO_name) - 1);
                    INFO(p_cnt).daemon_FIFO_name[sizeof(INFO(p_cnt).daemon_FIFO_name) - 1] = '\0';
                }

                /* create 3 FIFOs (no slog for flag=1) */
                sprintf(file_name, "%s/%-2.2s/%s",
                        _FEP_FIFO, bumun, val);
                Create_FIFO(file_name);

                sprintf(file_name, "%s/%-2.2s/%s_exit",
                        _FEP_FIFO, bumun, val);
                Create_FIFO(file_name);

                sprintf(file_name, "%s/%-2.2s/%s_ctrl",
                        _FEP_FIFO, bumun, val);
                Create_FIFO(file_name);
            }
            continue;
        }

        /*-- PROC_COUNT --*/
        if (strcmp(key, "PROC_COUNT") == 0) {
            if (flag == 0 || flag == 3)
                INFO(p_cnt).process_count = atoi(val);
            else if (flag == 1) {
                DAEMON(p_cnt).process_count = atoi(val);
                INFO(p_cnt).process_count = atoi(val);
            }
            continue;
        }

        /*-- FILE_COUNT --*/
        if (strcmp(key, "FILE_COUNT") == 0) {
            if (flag == 0 || flag == 3)
                INFO(p_cnt).file_count = atoi(val);
            else if (flag == 1) {
                DAEMON(p_cnt).file_count = atoi(val);
                INFO(p_cnt).file_count = atoi(val);
            }
            continue;
        }

        /*-- DSHM_COUNT --*/
        if (strcmp(key, "DSHM_COUNT") == 0) {
            if (flag == 0 || flag == 3)
                INFO(p_cnt).dshm_count = atoi(val);
            else if (flag == 1) {
                DAEMON(p_cnt).dshm_count = atoi(val);
                INFO(p_cnt).dshm_count = atoi(val);
            }
            continue;
        }

        /*-- TCP1_COUNT --*/
        if (strcmp(key, "TCP1_COUNT") == 0) {
            if (flag == 0 || flag == 3)
                INFO(p_cnt).tcp1_count = atoi(val);
            else if (flag == 1) {
                DAEMON(p_cnt).tcp1_count = atoi(val);
                INFO(p_cnt).tcp1_count = atoi(val);
            }
            continue;
        }

        /*-- TCP2_COUNT --*/
        if (strcmp(key, "TCP2_COUNT") == 0) {
            if (flag == 0 || flag == 3)
                INFO(p_cnt).tcp2_count = atoi(val);
            else if (flag == 1) {
                DAEMON(p_cnt).tcp2_count = atoi(val);
                INFO(p_cnt).tcp2_count = atoi(val);
            }
            continue;
        }

        /*-- UDPIP_COUNT --*/
        if (strcmp(key, "UDPIP_COUNT") == 0) {
            if (flag == 0 || flag == 3)
                INFO(p_cnt).udpip_count = atoi(val);
            else if (flag == 1) {
                DAEMON(p_cnt).udpip_count = atoi(val);
                INFO(p_cnt).udpip_count = atoi(val);
            }
            continue;
        }

        /*-- DATA_COUNT --*/
        if (strcmp(key, "DATA_COUNT") == 0) {
            if (flag == 0 || flag == 3)
                INFO(p_cnt).data_count = atoi(val);
            else if (flag == 1) {
                DAEMON(p_cnt).data_count = atoi(val);
                INFO(p_cnt).data_count = atoi(val);
            }
            continue;
        }

        /*-- SHM_LOG --*/
        if (strcmp(key, "SHM_LOG") == 0) {
            if (flag == 0 || (flag == 3 && val[0] == '0'))
                INFO(p_cnt).shm_log = atoi(val);
            else if (flag == 1) {
                DAEMON(p_cnt).shm_log = atoi(val);
                INFO(p_cnt).shm_log = atoi(val);
            }
            continue;
        }
    }

    sqlite3_finalize(stmt);

    Log(USR_OK, "cfg_db_load_daemon: loaded(flag=%d)", flag);
    return CFG_DB_OK;

}   /* End of cfg_db_load_daemon () */

/*************************************************************************
    Function        : . load file.ini config from DB -> FILEM() SHM
    Parameters IN   : . flag (0:all daemons, 1:current daemon only)
    Parameters OUT  : . FILEM(), DAEMON().f_count + dirs/seq/FIFOs
    Return Code     : . CFG_DB_OK or error code
    DB key_name     : FILE_COUNT, FILE_N_COMMENT, FILE_N_NAME,
                      FILE_N_FIFO, FILE_N_SIZE
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_load_file(int flag)
/*----------------------------------------------------------------------*/
{
    sqlite3_stmt    *stmt = NULL;
    int             rc, p_cnt, n, i, fd, rt, len, pt, idx_flag;
    int             itemcnt, count = 0;
    char            g_cnt, suffix[40];
    const char      *env_id, *section, *key, *val;
    char            file_name[256], seq_buf[400], bumun[4], ppath[100];
    struct stat64   f_info;

    if (g_db == NULL)
        return CFG_DB_ERR_OPEN;

    env_id = cfg_db_detect_env();

    rc = sqlite3_prepare_v2(g_db,
            "SELECT section, key_name, key_value FROM fep_config "
            "WHERE env_id=? AND group_id='file' "
            "ORDER BY section, sort_order, key_name",
            -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        Log(ORA_WARN, "cfg_db_load_file: prepare failed(%d)", rc);
        return CFG_DB_ERR_QUERY;
    }

    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        section = (const char *) sqlite3_column_text(stmt, 0);
        key     = (const char *) sqlite3_column_text(stmt, 1);
        val     = (const char *) sqlite3_column_text(stmt, 2);

        if (!section || !key || !val)
            continue;

        g_cnt = section[0];
        p_cnt = g_cnt - 'A';

        if (p_cnt < 0 || p_cnt > 25)
            continue;

        /* flag filtering */
        if (flag == 1 && D_K != p_cnt)
            continue;

        /*-- FILE_COUNT --*/
        if (strcmp(key, "FILE_COUNT") == 0) {
            itemcnt = atoi(val);

            if (DAEMON(p_cnt).file_count < itemcnt) {
                Log(USR_FATAL,
                        "file(%c):FILE_COUNT[%s]:edit file_count in daemon",
                        g_cnt, val);
            }
            else
                DAEMON(p_cnt).f_count = itemcnt;

            count++;
            continue;
        }

        /*-- FILE_N_COMMENT, FILE_N_NAME, FILE_N_FIFO, FILE_N_SIZE --*/
        if (sscanf(key, "FILE_%d_%39s", &n, suffix) == 2 && n >= 1) {
            if (strcmp(suffix, "COMMENT") == 0)
                strncpy(FILEM(p_cnt,n-1).file_info, val,
                        sizeof (FILEM(p_cnt,n-1).file_info));
            else if (strcmp(suffix, "NAME") == 0)
                strncpy(FILEM(p_cnt,n-1).file_name, val,
                        sizeof (FILEM(p_cnt,n-1).file_name));
            else if (strcmp(suffix, "FIFO") == 0)
                FILEM(p_cnt,n-1).fifo_count = atoi(val);
            else if (strcmp(suffix, "SIZE") == 0)
                FILEM(p_cnt,n-1).record_size = atoi(val);

            count++;
            continue;
        }
    }

    sqlite3_finalize(stmt);

    /*================================================================
        Post-processing: File_End equivalent
        For each loaded file entry, create directories, seq files,
        check data file size, read r_cnt, create FIFOs
    ================================================================*/
    for (p_cnt = 0; p_cnt < 26; p_cnt++) {
        int f_count = DAEMON(p_cnt).f_count;

        if (f_count <= 0)
            continue;

        if (flag == 1 && D_K != p_cnt)
            continue;

        for (n = 0; n < f_count; n++) {
            if (FILEM(p_cnt,n).file_name[0] == '\0')
                continue;

            /* determine path: log -> _FEP_LOG, else _FEP_DAT */
            if (memcmp(&FILEM(p_cnt,n).file_name[3], "log", 3) == 0)
                sprintf(ppath, "%s", _FEP_LOG);
            else
                sprintf(ppath, "%s", _FEP_DAT);

            sprintf(bumun, "%-2.2s", FILEM(p_cnt,n).file_name);

            /* create directory */
            sprintf(file_name, "%s/%s/00000000",
                    ppath, LtoU(bumun, 2));
            Create_Dir(file_name);

            /* create _seq file */
            sprintf(file_name, "%s/%s/00000000/%s_seq",
                    ppath, bumun, FILEM(p_cnt,n).file_name);
            fd = open(file_name, O_RDWR | O_APPEND | O_CREAT, 0664);
            fchmod(fd, 0664);
            close(fd);

            /* check .idx file */
            sprintf(file_name, "%s/%s/00000000/%s.idx",
                    ppath, bumun, FILEM(p_cnt,n).file_name);

            if (access(file_name, F_OK) == 0)
                idx_flag = 1;
            else {
                idx_flag = 0;
                sprintf(file_name, "%s/%s/00000000/%s",
                        ppath, bumun, FILEM(p_cnt,n).file_name);
            }

            /* create/open data file, get size */
            fd = open64(file_name,
                    O_RDWR | O_APPEND | O_CREAT, 0664);
            fchmod(fd, 0664);
            close(fd);

            f_info.st_size = 0;
            rt = stat64(file_name, &f_info);

            if (f_info.st_size > 0) {
                if (memcmp(FILEM(p_cnt,n).file_name+3,
                        "seq", 3) != 0 &&
                        memcmp(FILEM(p_cnt,n).file_name+3,
                        "log", 3) != 0) {
                    if (idx_flag)
                        FILEM(p_cnt,n).w_cnt[0] =
                    f_info.st_size / 23;
                    else
                        FILEM(p_cnt,n).w_cnt[0] =
                    f_info.st_size /
                    (int)(FILEM(p_cnt,n).record_size +
                            sizeof (FILE_RW_HEAD) + 1);
                }

                /* read seq data (r_cnt[0..8]) */
                sprintf(file_name, "%s/%s/00000000/%s_seq",
                        ppath, bumun, FILEM(p_cnt,n).file_name);

                fd = open(file_name,
                        O_RDWR | O_APPEND | O_CREAT, 0664);

                if (fd >= 0) {
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
                        for (i = 0; i < 9; i++)
                            FILEM(p_cnt,n).r_cnt[i] =
                        AtoIf(&seq_buf[8*i], 8);
                    }

                    close(fd);
                }
            }

            /* create FIFOs */
            for (i = 1; i <= FILEM(p_cnt,n).fifo_count; i++) {
                sprintf(file_name, "%s/%-2.2s/%s%d",
                        _FEP_FIFO, bumun,
                        FILEM(p_cnt,n).file_name, i);
                Create_FIFO(file_name);
            }
        }
    }

    Log(USR_OK, "cfg_db_load_file: loaded %d keys(flag=%d)",
            count, flag);
    return CFG_DB_OK;

}   /* End of cfg_db_load_file () */

/*************************************************************************
    Function        : . load dshm.ini config from DB -> DSHM() SHM
    Parameters IN   : . flag (0:all daemons, 1:current daemon only)
    Parameters OUT  : . DSHM(), DAEMON().d_count + SHM segments
                      + dirs/dseq/FIFOs
    Return Code     : . CFG_DB_OK or error code
    DB key_name     : DSHM_COUNT, DSHM_N_COMMENT, DSHM_N_NAME,
                      DSHM_N_KEY, DSHM_N_FIFO, DSHM_N_SIZE, DSHM_N_MAX
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_load_dshm(int flag)
/*----------------------------------------------------------------------*/
{
    sqlite3_stmt    *stmt = NULL;
    int             rc, p_cnt, n, i, fd, rt, len, pt;
    int             count = 0, shmid, rec_size, max;
    char            g_cnt, suffix[40];
    const char      *env_id, *section, *key, *val;
    char            file_name[256], seq_buf[400], bumun[4], ppath[100];
    char            key_buf[12];
    key_t           base_key, shm_key;
    struct stat     f_info;
    int             offset;
    char            *fep_div;

    if (g_db == NULL)
        return CFG_DB_ERR_OPEN;

    env_id = cfg_db_detect_env();

    rc = sqlite3_prepare_v2(g_db,
            "SELECT section, key_name, key_value FROM fep_config "
            "WHERE env_id=? AND group_id='dshm' "
            "ORDER BY section, sort_order, key_name",
            -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        Log(ORA_WARN, "cfg_db_load_dshm: prepare failed(%d)", rc);
        return CFG_DB_ERR_QUERY;
    }

    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        section = (const char *) sqlite3_column_text(stmt, 0);
        key     = (const char *) sqlite3_column_text(stmt, 1);
        val     = (const char *) sqlite3_column_text(stmt, 2);

        if (!section || !key || !val)
            continue;

        g_cnt = section[0];
        p_cnt = g_cnt - 'A';

        if (p_cnt < 0 || p_cnt > 25)
            continue;

        /* flag filtering */
        if (flag == 1 && D_K != p_cnt)
            continue;

        /*-- DSHM_COUNT --*/
        if (strcmp(key, "DSHM_COUNT") == 0) {
            int itemcnt = atoi(val);

            if (DAEMON(p_cnt).dshm_count < itemcnt) {
                Log(USR_FATAL,
                        "dshm(%c):DSHM_COUNT[%s]:edit dshm_count in daemon",
                        g_cnt, val);
            }
            else
                DAEMON(p_cnt).d_count = itemcnt;

            count++;
            continue;
        }

        /*-- DSHM_N_COMMENT, NAME, KEY, FIFO, SIZE, MAX --*/
        if (sscanf(key, "DSHM_%d_%39s", &n, suffix) == 2 && n >= 1) {
            if (strcmp(suffix, "COMMENT") == 0)
                strncpy(DSHM(p_cnt,n-1).info, val,
                        sizeof (DSHM(p_cnt,n-1).info));
            else if (strcmp(suffix, "NAME") == 0)
                strncpy(DSHM(p_cnt,n-1).data_name, val,
                        sizeof (DSHM(p_cnt,n-1).data_name));
            else if (strcmp(suffix, "KEY") == 0)
                strncpy(DSHM(p_cnt,n-1).key_info, val,
                        sizeof (DSHM(p_cnt,n-1).key_info));
            else if (strcmp(suffix, "FIFO") == 0)
                DSHM(p_cnt,n-1).fifo_count = atoi(val);
            else if (strcmp(suffix, "SIZE") == 0)
                DSHM(p_cnt,n-1).data_size = atoi(val);
            else if (strcmp(suffix, "MAX") == 0)
                DSHM(p_cnt,n-1).max_rec = atoi(val);

            count++;
            continue;
        }
    }

    sqlite3_finalize(stmt);

    /*================================================================
        Post-processing: Dshm_End equivalent
        For each loaded dshm entry:
        1. SHM segment creation based on key_info[5] (S/C/E/O)
        2. Create directories and data files
        3. Read dseq (r_cnt + sm_r_cnt)
        4. Create FIFOs
    ================================================================*/
    for (p_cnt = 0; p_cnt < 26; p_cnt++) {
        int d_count = DAEMON(p_cnt).d_count;

        if (d_count <= 0)
            continue;

        if (flag == 1 && D_K != p_cnt)
            continue;

        /* compute base_key for this daemon */
        base_key = BASE_SHM_KEY;

        fep_div = (char *) getenv("_FEP_DIV");
        if (fep_div != NULL &&
                memcmp(fep_div, "TEST", 4) == 0)
        base_key += 0x01000000L;

        sprintf(key_buf, "0x00%02d0000", p_cnt + 1);
        errno = 0;
        base_key += strtol(key_buf, NULL, 16);

        Log(USR_OK, "start %c%c dshm config ...",
                _System_Name[0], (char)('A' + p_cnt));

        offset = 0;

        for (n = 0; n < d_count; n++) {
            if (DSHM(p_cnt,n).data_name[0] == '\0')
                continue;

            rec_size = sizeof (FILE_RW_HEAD) +
            DSHM(p_cnt,n).data_size + 1;
            max = DSHM(p_cnt,n).max_rec;

            /*-- SHM segment management --*/
            if (DSHM(p_cnt,n).key_info[5] == 'S') {
                DSHM(p_cnt,n).offset = 0;
                offset += (rec_size * max);
            }
            else if (DSHM(p_cnt,n).key_info[5] == 'C') {
                DSHM(p_cnt,n).offset = offset;
                offset += (rec_size * max);
            }
            else {
                sprintf(key_buf, "0x0000%2.2s00",
                        DSHM(p_cnt,n).key_info);
                errno = 0;
                shm_key = base_key +
                strtol(key_buf, NULL, 16);

                if (DSHM(p_cnt,n).key_info[5] == 'E') {
                    Shmptr = SHM_Creat_Attach(shm_key,
                            offset + (rec_size * max), &shmid);
                    Log(USR_OK,
                            "DSHM created: key[%#10x] size[%d]",
                            (unsigned int)shm_key,
                            offset + (rec_size * max));
                    DSHM(p_cnt,n).offset = offset;
                }
                else if (DSHM(p_cnt,n).key_info[5] == 'O') {
                    Shmptr = SHM_Creat_Attach(shm_key,
                            rec_size * max, &shmid);
                    Log(USR_OK,
                            "DSHM created: key[%#10x] size[%d]",
                            (unsigned int)shm_key,
                            rec_size * max);
                    DSHM(p_cnt,n).offset = 0;
                }

                offset = 0;
            }

            /*-- directory and file creation --*/
            sprintf(ppath, "%s", _FEP_DAT);
            sprintf(bumun, "%-2.2s",
                    DSHM(p_cnt,n).data_name);

            sprintf(file_name, "%s/%s/00000000",
                    ppath, LtoU(bumun, 2));
            Create_Dir(file_name);

            /* create _dseq file */
            sprintf(file_name, "%s/%s/00000000/%s_dseq",
                    ppath, bumun, DSHM(p_cnt,n).data_name);
            fd = open(file_name,
                    O_RDWR | O_APPEND | O_CREAT, 0664);
            fchmod(fd, 0664);
            close(fd);

            /* create data file */
            sprintf(file_name, "%s/%s/00000000/%s",
                    ppath, bumun, DSHM(p_cnt,n).data_name);
            fd = open(file_name,
                    O_RDWR | O_APPEND | O_CREAT, 0664);
            fchmod(fd, 0664);
            close(fd);

            f_info.st_size = 0;
            rt = stat(file_name, &f_info);

            if (f_info.st_size > 0) {
                DSHM(p_cnt,n).w_cnt[0] = f_info.st_size /
                (int)(sizeof (FILE_RW_HEAD) +
                        DSHM(p_cnt,n).data_size + 1);
            }

            /* read dseq (r_cnt[0..8] + sm_r_cnt) */
            sprintf(file_name, "%s/%s/00000000/%s_dseq",
                    ppath, bumun, DSHM(p_cnt,n).data_name);

            fd = open(file_name,
                    O_RDWR | O_APPEND | O_CREAT, 0664);

            if (fd >= 0) {
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
                    for (i = 0; i < 9; i++)
                        DSHM(p_cnt,n).r_cnt[i] =
                    AtoIf(&seq_buf[8*i], 8);

                    DSHM(p_cnt,n).sm_r_cnt =
                    AtoIf(&seq_buf[72], 8);
                }

                close(fd);
            }

            /* create FIFOs (note: starts from 0) */
            for (i = 0; i <= DSHM(p_cnt,n).fifo_count; i++) {
                sprintf(file_name, "%s/%-2.2s/%s%d",
                        _FEP_FIFO, bumun,
                        DSHM(p_cnt,n).data_name, i);
                Create_FIFO(file_name);
            }
        }

        Log(USR_OK, "... end %c%c dshm config",
                _System_Name[0], (char)('A' + p_cnt));
    }

    Log(USR_OK, "cfg_db_load_dshm: loaded %d keys(flag=%d)",
            count, flag);
    return CFG_DB_OK;

}   /* End of cfg_db_load_dshm () */

/*************************************************************************
    Function        : . load tcp1.ini config from DB -> TCP1() SHM
    Parameters IN   : . flag (0:all daemons, 1:current daemon only)
    Parameters OUT  : . TCP1(), DAEMON().t1_count
    Return Code     : . CFG_DB_OK or error code
    DB key_name     : TCP1_COUNT, TCP1_N_COMMENT, TCP1_N_IP,
                      TCP1_N_PORT, TCP1_N_SPORT01..09
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_load_tcp1(int flag)
/*----------------------------------------------------------------------*/
{
    sqlite3_stmt    *stmt = NULL;
    int             rc, count = 0, p_cnt, n, sport_idx;
    char            g_cnt, suffix[40];
    const char      *env_id, *section, *key, *val;

    if (g_db == NULL)
        return CFG_DB_ERR_OPEN;

    env_id = cfg_db_detect_env();

    rc = sqlite3_prepare_v2(g_db,
            "SELECT section, key_name, key_value FROM fep_config "
            "WHERE env_id=? AND group_id='tcp1' "
            "ORDER BY section, sort_order, key_name",
            -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        Log(ORA_ERROR, "cfg_db_load_tcp1: prepare: %s",
                sqlite3_errmsg(g_db));
        return CFG_DB_ERR_QUERY;
    }

    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        section = (const char *) sqlite3_column_text(stmt, 0);
        key     = (const char *) sqlite3_column_text(stmt, 1);
        val     = (const char *) sqlite3_column_text(stmt, 2);

        if (section == NULL || key == NULL)
            continue;
        if (val == NULL)
            val = "";

        g_cnt = section[0];
        p_cnt = g_cnt - 'A';
        if (p_cnt < 0 || p_cnt > 25)
            continue;
        if (flag == 1 && D_K != p_cnt)
            continue;

        /* TCP1_COUNT */
        if (strcmp(key, "TCP1_COUNT") == 0) {
            int     itemcnt = atoi(val);

            if (DAEMON(p_cnt).tcp1_count < itemcnt) {
                Log(USR_FATAL,
                        "cfg_db tcp1(%c):TCP1_COUNT[%s]:edit daemon.ini",
                        g_cnt, val);
                sqlite3_finalize(stmt);
                return CFG_DB_ERR_TYPE;
            }
            DAEMON(p_cnt).t1_count = itemcnt;
            count++;
            continue;
        }

        /* TCP1_N_XXX */
        memset(suffix, 0, sizeof(suffix));
        if (sscanf(key, "TCP1_%d_%39s", &n, suffix) == 2 && n >= 1) {
            if (n > DAEMON(p_cnt).t1_count)
                continue;

            if (strcmp(suffix, "COMMENT") == 0) {
                snprintf(TCP1(p_cnt,n-1).tcp_info,
                        sizeof(TCP1(p_cnt,n-1).tcp_info), "%s", val);
            }
            else if (strcmp(suffix, "IP") == 0) {
                if (inet_pton(AF_INET, val, TCP1(p_cnt,n-1).ip_addr) != 1) {
                    Log(USR_FATAL,
                            "cfg_db tcp1(%c,%d):bad IP[%s]",
                            g_cnt, n, val);
                    sqlite3_finalize(stmt);
                    return CFG_DB_ERR_TYPE;
                }
            }
            else if (strcmp(suffix, "PORT") == 0) {
                TCP1(p_cnt,n-1).port_no = atoi(val);
            }
            else if (sscanf(suffix, "SPORT%2d", &sport_idx) == 1 &&
                    sport_idx >= 1 && sport_idx <= 9) {
                TCP1(p_cnt,n-1).service_port_no[sport_idx-1] =
                atoi(val);
                if (!TCP1(p_cnt,n-1).service_port_no[sport_idx-1])
                    TCP1(p_cnt,n-1).service_status[sport_idx-1] = 9;
            }

            count++;
        }
    }

    sqlite3_finalize(stmt);

    if (count > 0) {
        Log(ORA_OK, "cfg_db_load_tcp1: loaded %d keys(flag=%d)",
                count, flag);
        return CFG_DB_OK;
    }

    return CFG_DB_ERR_NODATA;

}   /* End of cfg_db_load_tcp1 () */

/*************************************************************************
    Function        : . load tcp2.ini config from DB -> TCP2() SHM
    Parameters IN   : . flag (0:all daemons, 1:current daemon only)
    Parameters OUT  : . TCP2(), DAEMON().t2_count
    Return Code     : . CFG_DB_OK or error code
    DB key_name     : TCP2_COUNT, TCP2_N_COMMENT, TCP2_N_IP,
                      TCP2_N_DUP_ID, TCP2_N_PORT
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_load_tcp2(int flag)
/*----------------------------------------------------------------------*/
{
    sqlite3_stmt    *stmt = NULL;
    int             rc, count = 0, p_cnt, n;
    char            g_cnt, suffix[40];
    const char      *env_id, *section, *key, *val;

    if (g_db == NULL)
        return CFG_DB_ERR_OPEN;

    env_id = cfg_db_detect_env();

    rc = sqlite3_prepare_v2(g_db,
            "SELECT section, key_name, key_value FROM fep_config "
            "WHERE env_id=? AND group_id='tcp2' "
            "ORDER BY section, sort_order, key_name",
            -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        Log(ORA_ERROR, "cfg_db_load_tcp2: prepare: %s",
                sqlite3_errmsg(g_db));
        return CFG_DB_ERR_QUERY;
    }

    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        section = (const char *) sqlite3_column_text(stmt, 0);
        key     = (const char *) sqlite3_column_text(stmt, 1);
        val     = (const char *) sqlite3_column_text(stmt, 2);

        if (section == NULL || key == NULL)
            continue;
        if (val == NULL)
            val = "";

        g_cnt = section[0];
        p_cnt = g_cnt - 'A';
        if (p_cnt < 0 || p_cnt > 25)
            continue;
        if (flag == 1 && D_K != p_cnt)
            continue;

        /* TCP2_COUNT */
        if (strcmp(key, "TCP2_COUNT") == 0) {
            int     itemcnt = atoi(val);

            if (DAEMON(p_cnt).tcp2_count < itemcnt) {
                Log(USR_FATAL,
                        "cfg_db tcp2(%c):TCP2_COUNT[%s]:edit daemon.ini",
                        g_cnt, val);
                sqlite3_finalize(stmt);
                return CFG_DB_ERR_TYPE;
            }
            DAEMON(p_cnt).t2_count = itemcnt;
            count++;
            continue;
        }

        /* TCP2_N_XXX (N=line number, XXX=field) */
        memset(suffix, 0, sizeof(suffix));
        if (sscanf(key, "TCP2_%d_%39s", &n, suffix) == 2 && n >= 1) {
            if (n > DAEMON(p_cnt).t2_count)
                continue;

            if (strcmp(suffix, "COMMENT") == 0) {
                snprintf(TCP2(p_cnt,n-1).tcp_info,
                        sizeof(TCP2(p_cnt,n-1).tcp_info), "%s", val);
            }
            else if (strcmp(suffix, "IP") == 0) {
                if (inet_pton(AF_INET, val, TCP2(p_cnt,n-1).ip_addr) != 1) {
                    Log(USR_FATAL,
                            "cfg_db tcp2(%c,%d):bad IP[%s]",
                            g_cnt, n, val);
                    sqlite3_finalize(stmt);
                    return CFG_DB_ERR_TYPE;
                }
            }
            else if (strcmp(suffix, "DUP_ID") == 0) {
                TCP2(p_cnt,n-1).dup_id = atoi(val);
            }
            else if (strcmp(suffix, "PORT") == 0) {
                TCP2(p_cnt,n-1).port_no = atoi(val);
            }

            count++;
        }
    }

    sqlite3_finalize(stmt);

    if (count > 0) {
        Log(ORA_OK, "cfg_db_load_tcp2: loaded %d keys(flag=%d)",
                count, flag);
        return CFG_DB_OK;
    }

    return CFG_DB_ERR_NODATA;

}   /* End of cfg_db_load_tcp2 () */

/*************************************************************************
    Function        : . load udpip.ini config from DB -> UDPIP() SHM
    Parameters IN   : . flag (0:all daemons, 1:current daemon only)
    Parameters OUT  : . UDPIP(), DAEMON().u_count
    Return Code     : . CFG_DB_OK or error code
    DB key_name     : UDPIP_COUNT, UDPIP_N_COMMENT, UDPIP_N_IP_M,
                      UDPIP_N_DUP_ID, UDPIP_N_PORT_M
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_load_udpip(int flag)
/*----------------------------------------------------------------------*/
{
    sqlite3_stmt    *stmt = NULL;
    int             rc, count = 0, p_cnt, n, m;
    char            g_cnt, suffix[40];
    const char      *env_id, *section, *key, *val;

    if (g_db == NULL)
        return CFG_DB_ERR_OPEN;

    env_id = cfg_db_detect_env();

    rc = sqlite3_prepare_v2(g_db,
            "SELECT section, key_name, key_value FROM fep_config "
            "WHERE env_id=? AND group_id='udpip' "
            "ORDER BY section, sort_order, key_name",
            -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        Log(ORA_ERROR, "cfg_db_load_udpip: prepare: %s",
                sqlite3_errmsg(g_db));
        return CFG_DB_ERR_QUERY;
    }

    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        section = (const char *) sqlite3_column_text(stmt, 0);
        key     = (const char *) sqlite3_column_text(stmt, 1);
        val     = (const char *) sqlite3_column_text(stmt, 2);

        if (section == NULL || key == NULL)
            continue;
        if (val == NULL)
            val = "";

        g_cnt = section[0];
        p_cnt = g_cnt - 'A';
        if (p_cnt < 0 || p_cnt > 25)
            continue;
        if (flag == 1 && D_K != p_cnt)
            continue;

        /* UDPIP_COUNT */
        if (strcmp(key, "UDPIP_COUNT") == 0) {
            int     itemcnt = atoi(val);

            if (DAEMON(p_cnt).udpip_count < itemcnt) {
                Log(USR_FATAL,
                        "cfg_db udpip(%c):UDPIP_COUNT[%s]:edit daemon.ini",
                        g_cnt, val);
                sqlite3_finalize(stmt);
                return CFG_DB_ERR_TYPE;
            }
            DAEMON(p_cnt).u_count = itemcnt;
            count++;
            continue;
        }

        /* UDPIP_N_XXX */
        memset(suffix, 0, sizeof(suffix));
        if (sscanf(key, "UDPIP_%d_%39s", &n, suffix) == 2 && n >= 1) {
            if (n > DAEMON(p_cnt).u_count)
                continue;

            if (strcmp(suffix, "COMMENT") == 0) {
                snprintf(UDPIP(p_cnt,n-1).info,
                        sizeof(UDPIP(p_cnt,n-1).info), "%s", val);
            }
            else if (sscanf(suffix, "IP_%d", &m) == 1 &&
                    m >= 1 && m <= 20) {
                if (inet_pton(AF_INET, val, UDPIP(p_cnt,n-1).ip_addr[m-1]) != 1) {
                    Log(USR_FATAL,
                            "cfg_db udpip(%c,%d):bad IP_%d[%s]",
                            g_cnt, n, m, val);
                    sqlite3_finalize(stmt);
                    return CFG_DB_ERR_TYPE;
                }
            }
            else if (strcmp(suffix, "DUP_ID") == 0) {
                UDPIP(p_cnt,n-1).dup_id = atoi(val);
            }
            else if (sscanf(suffix, "PORT_%d", &m) == 1 &&
                    m >= 1 && m <= 20) {
                UDPIP(p_cnt,n-1).port[m-1] = atoi(val);
            }

            count++;
        }
    }

    sqlite3_finalize(stmt);

    if (count > 0) {
        Log(ORA_OK, "cfg_db_load_udpip: loaded %d keys(flag=%d)",
                count, flag);
        return CFG_DB_OK;
    }

    return CFG_DB_ERR_NODATA;

}   /* End of cfg_db_load_udpip () */

/*************************************************************************
    Function        : . load sisetr.ini config from DB
    Parameters IN   : . flag
    Parameters OUT  : .
    Return Code     : . CFG_DB_OK or error code
    Note            : Phase 2 - full implementation pending
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_load_sisetr(int flag)
/*----------------------------------------------------------------------*/
{
    /* sisetr disabled since 202201 - return OK (no-op) */
    if (g_db == NULL)
        return CFG_DB_ERR_OPEN;

    Log(ORA_DEBUG, "cfg_db_load_sisetr: disabled since 202201");
    return CFG_DB_OK;

}   /* End of cfg_db_load_sisetr () */

/*************************************************************************
    Static helper   : . initialize process stat file (runtime state)
    Description     : Replicates Proc_End block from Proc_Config_Read.
                      Reads _stat file for if_seq, start_status, etc.
                      This is runtime state, not config, but needed
                      for SHM byte-level compatibility.
*************************************************************************/
static void cfg_db_proc_init_stat(int p_cnt, int proc_idx, int g_cnt) {
    int     fd, rt, pt, len, i;
    char    ppath[100], bumun[4], d_time[16];
    char    dir_name[128], file_name[256], stat_buf[1024];

    memcpy(PROC(p_cnt,proc_idx).date, INFO(p_cnt).date, 8);

    sprintf(ppath, "%s", _FEP_DAT);
    sprintf(bumun, "%-2.2s", PROC(p_cnt,proc_idx).process_id);
    LtoU(bumun, 2);

    if (DAEMON(p_cnt).date_flag == 9) {
        if (g_cnt == 'W')       /* TCP window daemon skip */
            return;

        Get_DateTime(d_time);
        sprintf(dir_name, "%s/%s/%8.8s", ppath, bumun, d_time);
    }
    else {
        sprintf(dir_name, "%s/%s/00000000", ppath, bumun);
    }

    Create_Dir(dir_name);

    memset(file_name, 0, sizeof(file_name));
    sprintf(file_name, "%s/%s_stat",
            dir_name, PROC(p_cnt,proc_idx).process_id);

    fd = open(file_name, O_RDWR | O_APPEND | O_CREAT, 0664);
    if (fd < 0) {
        Log(SAM_FATAL, "cfg_db proc stat: cannot open [%s] {%d:%s}",
                file_name, SYS_NO, SYS_STR);
        return;
    }

    fchmod(fd, 0664);
    memset(stat_buf, 0, sizeof(stat_buf));
    len = 28;
    pt = 0;

    while (1) {
        rt = read(fd, stat_buf + pt, len);
        if (rt < 0) {
            Log(SAM_FATAL,
                    "cfg_db proc stat: cannot read [%s,%d] {%d:%s}",
                    file_name, len, SYS_NO, SYS_STR);
            close(fd);
            return;
        }
        if (rt == 0 || rt == len)
            break;
        len -= rt;
        pt += rt;
    }

    PROC(p_cnt,proc_idx).if_seq = AtoIf(stat_buf, 8);

    if (PROC(p_cnt,proc_idx).type == TY_TRS1)           /* TCP1 */ {
        PROC(p_cnt,proc_idx).start_status = AtoIf(stat_buf+8, 1);
        PROC(p_cnt,proc_idx).l.t1.network_status =
        AtoIf(stat_buf+9, 1);
    }
    else if (PROC(p_cnt,proc_idx).type == TY_TRS2)      /* TCP2 */ {
        /* 2025: +80 offset for ME group 10 Seq area */
        PROC(p_cnt,proc_idx).start_status =
        AtoIf(stat_buf+8+80, 1);

        /* ME group 10 Seq recovery */
        for (i = 0; i < 10; i++)
            PROC(p_cnt,proc_idx).if_meg_seq[i] =
        AtoIf(stat_buf+8+(8*i), 8);

        PROC(p_cnt,proc_idx).l.t2.line_gubun =
        AtoIf(stat_buf+9+80, 1);

        for (i = 0; i < 2; i++) {
            if (PROC(p_cnt,proc_idx).l.t2.l[i]) {
                TCP2_LSTAT(p_cnt,proc_idx,i) =
                AtoIf(&stat_buf[10+80+i*3], 1);
                TCP2_PSTAT(p_cnt,proc_idx,i) =
                AtoIf(&stat_buf[11+80+i*3], 1);
                TCP2_NSTAT(p_cnt,proc_idx,i) =
                AtoIf(&stat_buf[12+80+i*3], 1);
            }
        }

        memcpy(PROC(p_cnt,proc_idx).last_tr,
                &stat_buf[17+80], 11);
    }

    close(fd);

}   /* End of cfg_db_proc_init_stat () */

/*************************************************************************
    Function        : . load proc.ini config from DB -> PROC() SHM
    Parameters IN   : . flag (0:all daemons, 1:current daemon only)
    Parameters OUT  : . PROC(), DAEMON().p_count
    Return Code     : . CFG_DB_OK or error code
    Description     : Loads process config + initializes stat files.
                      LogonPW auto-decrypted via cfg_db_decrypt_pw().
                      Cross-refs (IFN/IDN/OFN/ODN) matched against
                      FILEM/DSHM names (requires file/dshm loaded first).
                      TCP1/TCP2/UDP line matching against loaded TCP SHM.
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_load_proc(int flag)
/*----------------------------------------------------------------------*/
{
    sqlite3_stmt    *stmt = NULL;
    int             rc, count = 0, p_cnt, n, m, i, j;
    int             start_ss, end_ss;
    char            g_cnt, suffix[40];
    const char      *env_id, *section, *key, *val;
    /* track loaded process counts per daemon for stat init */
    int             proc_loaded[26];

    memset(proc_loaded, 0, sizeof(proc_loaded));

    if (g_db == NULL)
        return CFG_DB_ERR_OPEN;

    env_id = cfg_db_detect_env();

    rc = sqlite3_prepare_v2(g_db,
            "SELECT section, key_name, key_value FROM fep_config "
            "WHERE env_id=? AND group_id='proc' "
            "ORDER BY section, sort_order, key_name",
            -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        Log(ORA_ERROR, "cfg_db_load_proc: prepare: %s",
                sqlite3_errmsg(g_db));
        return CFG_DB_ERR_QUERY;
    }

    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        section = (const char *) sqlite3_column_text(stmt, 0);
        key     = (const char *) sqlite3_column_text(stmt, 1);
        val     = (const char *) sqlite3_column_text(stmt, 2);

        if (section == NULL || key == NULL)
            continue;
        if (val == NULL)
            val = "";

        g_cnt = section[0];
        p_cnt = g_cnt - 'A';
        if (p_cnt < 0 || p_cnt > 25)
            continue;
        if (flag == 1 && D_K != p_cnt)
            continue;

        /* PROC_COUNT */
        if (strcmp(key, "PROC_COUNT") == 0) {
            int     itemcnt = atoi(val);

            if (DAEMON(p_cnt).process_count < itemcnt) {
                Log(USR_FATAL,
                        "cfg_db proc(%c):PROC_COUNT[%s]:edit daemon.ini",
                        g_cnt, val);
                sqlite3_finalize(stmt);
                return CFG_DB_ERR_TYPE;
            }
            DAEMON(p_cnt).p_count = itemcnt;
            proc_loaded[p_cnt] = itemcnt;
            count++;
            continue;
        }

        /* PROC_N_XXX (N=process number, XXX=field) */
        memset(suffix, 0, sizeof(suffix));
        if (sscanf(key, "PROC_%d_%39s", &n, suffix) != 2 || n < 1)
            continue;

        if (n > DAEMON(p_cnt).p_count)
            continue;

        /*-------- simple fields --------*/

        if (strcmp(suffix, "COMMENT") == 0) {
            snprintf(PROC(p_cnt,n-1).process_info,
                    sizeof(PROC(p_cnt,n-1).process_info), "%s", val);
        }
        else if (strcmp(suffix, "ID") == 0) {
            strncpy(PROC(p_cnt,n-1).process_id, val,
                    sizeof(PROC(p_cnt,n-1).process_id) - 1);
            sprintf(PROC(p_cnt,n-1).process_path, "%s/%s",
                    _FEP_BIN, val);
        }
        else if (strcmp(suffix, "STATUS") == 0) {
            if (val[0] == 'R')
                PROC(p_cnt,n-1).process_status = 1;
            else
                PROC(p_cnt,n-1).process_status = 9;
        }
        else if (strcmp(suffix, "TYPE") == 0) {
            if (val[0] == 'M')
                PROC(p_cnt,n-1).type = TY_MP;
            else if (val[0] == 'D')
                PROC(p_cnt,n-1).type = TY_DD;
            else if (val[0] == 'T' && strlen(val) >= 3 && val[2] == '1')
                PROC(p_cnt,n-1).type = TY_TRS1;
            else if (val[0] == 'T' && strlen(val) >= 3 && val[2] == '2')
                PROC(p_cnt,n-1).type = TY_TRS2;
            else if (val[0] == 'U')
                PROC(p_cnt,n-1).type = TY_URS;
            else if (val[0] == 'B')
                PROC(p_cnt,n-1).type = TY_BRS;

            strncpy(PROC(p_cnt,n-1).process_type, val,
                    sizeof(PROC(p_cnt,n-1).process_type));
        }
        else if (strcmp(suffix, "START_TIME") == 0) {
            int     hh, mm, ss;

            hh = AtoIf((char *)val, 2);
            mm = AtoIf((char *)val+2, 2);

            if (hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59) {
                ss = hh * 3600 + mm * 60;
                start_ss = AtoIf(DAEMON(p_cnt).start_time, 2) * 3600
                + AtoIf(DAEMON(p_cnt).start_time+2, 2) * 60;
                end_ss = AtoIf(DAEMON(p_cnt).end_time, 2) * 3600
                + AtoIf(DAEMON(p_cnt).end_time+2, 2) * 60;

                if (((DAEMON(p_cnt).date_flag == 2 ||
                        DAEMON(p_cnt).date_flag == 4) &&
                        (ss < start_ss && ss >= end_ss)) ||
                        ((DAEMON(p_cnt).date_flag == 1 ||
                        DAEMON(p_cnt).date_flag == 3 ||
                        DAEMON(p_cnt).date_flag == 5) &&
                        (ss < start_ss || ss >= end_ss))) {
                    Log(USR_FATAL,
                            "cfg_db proc(%c,%d):invalid start_time[%s]",
                            g_cnt, n, val);
                    memcpy(PROC(p_cnt,n-1).end_time,
                            DAEMON(p_cnt).end_time,
                            sizeof(DAEMON(p_cnt).end_time));
                }
                else {
                    strncpy(PROC(p_cnt,n-1).start_time, val,
                            sizeof(PROC(p_cnt,n-1).start_time));
                }
            }
        }
        else if (strcmp(suffix, "END_TIME") == 0) {
            int     hh, mm, ss;

            hh = AtoIf((char *)val, 2);
            mm = AtoIf((char *)val+2, 2);

            if (hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59) {
                ss = hh * 3600 + mm * 60;
                start_ss = AtoIf(DAEMON(p_cnt).start_time, 2) * 3600
                + AtoIf(DAEMON(p_cnt).start_time+2, 2) * 60;
                end_ss = AtoIf(DAEMON(p_cnt).end_time, 2) * 3600
                + AtoIf(DAEMON(p_cnt).end_time+2, 2) * 60;

                if (((DAEMON(p_cnt).date_flag == 2 ||
                        DAEMON(p_cnt).date_flag == 4) &&
                        (ss <= start_ss && ss > end_ss)) ||
                        ((DAEMON(p_cnt).date_flag == 1 ||
                        DAEMON(p_cnt).date_flag == 3 ||
                        DAEMON(p_cnt).date_flag == 5) &&
                        (ss <= start_ss || ss > end_ss))) {
                    Log(USR_FATAL,
                            "cfg_db proc(%c,%d):invalid end_time[%s]",
                            g_cnt, n, val);
                    memcpy(PROC(p_cnt,n-1).end_time,
                            DAEMON(p_cnt).end_time,
                            sizeof(DAEMON(p_cnt).end_time));
                }
                else {
                    strncpy(PROC(p_cnt,n-1).end_time, val,
                            sizeof(PROC(p_cnt,n-1).end_time));
                }
            }
        }
        else if (strcmp(suffix, "TIME_OUT") == 0) {
            PROC(p_cnt,n-1).timeout = atoi(val);
        }
        else if (strcmp(suffix, "DELAY") == 0) {
            PROC(p_cnt,n-1).delay = atoi(val);
        }
        else if (strcmp(suffix, "DATA_BUF") == 0) {
            if (val[0] == 'Y' && Data_I < DATA_BUF_CNT) {
                PROC(p_cnt,n-1).data_flag = 1;
                PROC(p_cnt,n-1).data = (char *)Data_Ptr[Data_I++];
            }
        }

        /*-------- cross-reference fields --------*/

        /* IFN - input file name (matches FILEM().file_name) */
        else if (sscanf(suffix, "IFN_%d", &m) == 1 &&
                m >= 1 && m <= 3) {
            for (i = 0; i < DAEMON(p_cnt).f_count; i++) {
                if (memcmp(val, FILEM(p_cnt,i).file_name,
                        Max(strlen(val),
                        strlen(FILEM(p_cnt,i).file_name))) == 0) {
                    PROC(p_cnt,n-1).in_f[m-1] = i + 1;
                    break;
                }
            }
        }
        /* IDN - input data SHM name (matches DSHM().data_name) */
        else if (sscanf(suffix, "IDN_%d", &m) == 1 &&
                m >= 1 && m <= 3) {
            for (i = 0; i < DAEMON(p_cnt).d_count; i++) {
                if (memcmp(val, DSHM(p_cnt,i).data_name,
                        Max(strlen(val),
                        strlen(DSHM(p_cnt,i).data_name))) == 0) {
                    PROC(p_cnt,n-1).in_d[m-1] = i + 1;
                    break;
                }
            }
        }
        /* FFN - input FIFO file name */
        else if (sscanf(suffix, "FFN_%d", &m) == 1 &&
                m >= 1 && m <= 3) {
            snprintf(PROC(p_cnt,n-1).fifo_f[m-1],
                    sizeof(PROC(p_cnt,n-1).fifo_f[m-1]), "%s", val);
        }
        /* OFN - output file name (matches FILEM().file_name) */
        else if (sscanf(suffix, "OFN_%d", &m) == 1 &&
                m >= 1 && m <= 99) {
            for (i = 0; i < DAEMON(p_cnt).f_count; i++) {
                if (memcmp(val, FILEM(p_cnt,i).file_name,
                        Max(strlen(val),
                        strlen(FILEM(p_cnt,i).file_name))) == 0) {
                    PROC(p_cnt,n-1).out_f[m-1] = i + 1;
                    break;
                }
            }
        }
        /* ODN - output data SHM name (matches DSHM().data_name) */
        else if (sscanf(suffix, "ODN_%d", &m) == 1 &&
                m >= 1 && m <= 99) {
            for (i = 0; i < DAEMON(p_cnt).d_count; i++) {
                if (memcmp(val, DSHM(p_cnt,i).data_name,
                        Max(strlen(val),
                        strlen(DSHM(p_cnt,i).data_name))) == 0) {
                    PROC(p_cnt,n-1).out_d[m-1] = i + 1;
                    break;
                }
            }
        }

        /*-------- TCP/UDP line matching --------*/

        /* TCP1_TYPE (M=master, S=service) */
        else if (strcmp(suffix, "TCP1_TYPE") == 0) {
            if (PROC(p_cnt,n-1).type == TY_TRS1) {
                if (val[0] == 'M')
                    PROC(p_cnt,n-1).l.t1.port_type = 1;
                else if (val[0] == 'S')
                    PROC(p_cnt,n-1).l.t1.port_type = 2;
            }
        }
        /* TCP1_PORT (matches TCP1().port_no or service_port_no) */
        else if (strcmp(suffix, "TCP1_PORT") == 0) {
            if (PROC(p_cnt,n-1).type == TY_TRS1) {
                for (i = 0; i < DAEMON(p_cnt).t1_count; i++) {
                    if (PROC(p_cnt,n-1).l.t1.port_type == 1) {
                        if (atoi(val) == TCP1(p_cnt,i).port_no) {
                            PROC(p_cnt,n-1).l.t1.line_gubun = i+1;
                            PROC(p_cnt,n-1).l.t1.port_type = 0;
                            break;
                        }
                    }
                    else {
                        for (j = 0; j < 9; j++) {
                            if (atoi(val) ==
                                    TCP1(p_cnt,i).service_port_no[j]) {
                                PROC(p_cnt,n-1).l.t1.line_gubun = i+1;
                                PROC(p_cnt,n-1).l.t1.port_type = j+1;
                                break;
                            }
                        }
                    }
                }
            }
        }
        /* TCP2_PRIMARY (val: "dup_id,port") */
        else if (strcmp(suffix, "TCP2_PRIMARY") == 0) {
            if (PROC(p_cnt,n-1).type == TY_TRS2) {
                for (j = 0; j < (int)strlen(val) && val[j] != ','; j++)
                    ;
                for (i = 0; i < DAEMON(p_cnt).t2_count; i++) {
                    if (AtoIf((char *)val, j) ==
                            TCP2(p_cnt,i).dup_id &&
                            atoi(&val[j+1]) == TCP2(p_cnt,i).port_no) {
                        PROC(p_cnt,n-1).l.t2.l[0] = i + 1;
                        break;
                    }
                }
            }
        }
        /* TCP2_BACKUP (val: "dup_id,port") */
        else if (strcmp(suffix, "TCP2_BACKUP") == 0) {
            if (PROC(p_cnt,n-1).type == TY_TRS2) {
                for (j = 0; j < (int)strlen(val) && val[j] != ','; j++)
                    ;
                for (i = 0; i < DAEMON(p_cnt).t2_count; i++) {
                    if (AtoIf((char *)val, j) ==
                            TCP2(p_cnt,i).dup_id &&
                            atoi(&val[j+1]) == TCP2(p_cnt,i).port_no) {
                        PROC(p_cnt,n-1).l.t2.l[1] = i + 1;
                        PROC(p_cnt,n-1).backup = 1;
                        break;
                    }
                }
            }
        }
        /* UDP_PORT (val: "dup_id,port") */
        else if (strcmp(suffix, "UDP_PORT") == 0) {
            if (PROC(p_cnt,n-1).type == TY_URS) {
                for (j = 0; j < (int)strlen(val) && val[j] != ','; j++)
                    ;
                for (i = 0; i < DAEMON(p_cnt).u_count; i++) {
                    if (AtoIf((char *)val, j) ==
                            UDPIP(p_cnt,i).dup_id &&
                            atoi(&val[j+1]) == UDPIP(p_cnt,i).port[0]) {
                        PROC(p_cnt,n-1).l.u = i + 1;
                        break;
                    }
                }
            }
        }

        /*-------- logon fields --------*/

        else if (strcmp(suffix, "LOGONID") == 0) {
            strncpy(PROC(p_cnt,n-1).logon_id, val,
                    sizeof(PROC(p_cnt,n-1).logon_id) - 1);
        }
        /* LOGONPW - auto-decrypt from DB ciphertext */
        else if (strcmp(suffix, "LOGONPW") == 0) {
            char    pw[20];

            memset(pw, 0, sizeof(pw));
            if (cfg_db_decrypt_pw(val, pw) == CFG_DB_OK)
                strncpy(PROC(p_cnt,n-1).logon_pw, pw,
                        sizeof(PROC(p_cnt,n-1).logon_pw) - 1);
            else
                strncpy(PROC(p_cnt,n-1).logon_pw, val,
                        sizeof(PROC(p_cnt,n-1).logon_pw) - 1);
        }

        count++;
    }

    sqlite3_finalize(stmt);

    /* Post-processing: stat file initialization for each loaded proc */
    for (p_cnt = 0; p_cnt < 26; p_cnt++) {
        if (proc_loaded[p_cnt] == 0)
            continue;
        if (flag == 1 && D_K != p_cnt)
            continue;

        g_cnt = 'A' + p_cnt;

        for (n = 1; n <= proc_loaded[p_cnt]; n++)
            cfg_db_proc_init_stat(p_cnt, n - 1, g_cnt);
    }

    if (count > 0) {
        Log(ORA_OK, "cfg_db_load_proc: loaded %d keys(flag=%d)",
                count, flag);
        return CFG_DB_OK;
    }

    return CFG_DB_ERR_NODATA;

}   /* End of cfg_db_load_proc () */

/*************************************************************************
    Function        : . load IP whitelist from fep_ip_whitelist table
    Parameters IN   : . ip_list (caller-allocated array)
                      max_cnt (array capacity)
                      ip_type ("CLIENT" or "ADMIN")
    Parameters OUT  : . ip_list (filled with IP addresses)
    Return Code     : . loaded count (>=0) or negative error code
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_load_ip_whitelist(char ip_list[][20], int max_cnt,
        const char *ip_type)
/*----------------------------------------------------------------------*/
{
    sqlite3_stmt    *stmt = NULL;
    int             rc, count = 0;
    const char      *env_id, *ip;

    if (g_db == NULL)
        return CFG_DB_ERR_OPEN;

    env_id = cfg_db_detect_env();

    rc = sqlite3_prepare_v2(g_db,
            "SELECT ip_address FROM fep_ip_whitelist "
            "WHERE env_id=? AND ip_type=? AND is_active=1 "
            "ORDER BY ip_address",
            -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        Log(ORA_ERROR, "cfg_db_load_ip_whitelist: prepare: %s",
                sqlite3_errmsg(g_db));
        return CFG_DB_ERR_QUERY;
    }

    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, ip_type, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_cnt) {
        ip = (const char *) sqlite3_column_text(stmt, 0);
        if (ip != NULL) {
            strncpy(ip_list[count], ip, 19);
            ip_list[count][19] = '\0';
            count++;
        }
    }

    sqlite3_finalize(stmt);

    Log(ORA_OK, "cfg_db_load_ip_whitelist: loaded %d IPs(type=%s)",
            count, ip_type);

    return count;

}   /* End of cfg_db_load_ip_whitelist () */

/*========================================================================
    Config Save Functions
========================================================================*/

/*************************************************************************
    Function        : . save single config value
    Parameters IN   : . env_id, group_id, section, key_name, new_value,
                        changed_by
    Parameters OUT  : .
    Return Code     : . CFG_DB_OK or error code
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_set_value(const char *env_id, const char *group_id,
        const char *section, const char *key_name,
        const char *new_value, const char *changed_by)
/*----------------------------------------------------------------------*/
{
    sqlite3_stmt    *stmt = NULL;
    int             rc;
    const char      *sql;

    if (g_db == NULL)
        return CFG_DB_ERR_OPEN;

    sql = "UPDATE fep_config SET key_value=?, updated_by=?, "
    "updated_at=datetime('now','localtime') "
    "WHERE env_id=? AND group_id=? AND section=? AND key_name=?";

    rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        Log(ORA_ERROR, "cfg_db_set_value: prepare failed: %s",
                sqlite3_errmsg(g_db));
        return CFG_DB_ERR_QUERY;
    }

    sqlite3_bind_text(stmt, 1, new_value, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, changed_by, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, env_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, group_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, section, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, key_name, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        Log(ORA_ERROR, "cfg_db_set_value: step failed: %s",
                sqlite3_errmsg(g_db));
        return CFG_DB_ERR_QUERY;
    }

    if (sqlite3_changes(g_db) == 0) {
        Log(ORA_WARN, "cfg_db_set_value: no row updated "
                "[%s.%s.%s.%s]", env_id, group_id, section, key_name);
        return CFG_DB_ERR_NODATA;
    }

    return CFG_DB_OK;

}   /* End of cfg_db_set_value () */

/*========================================================================
    DB -> .ini Sync Functions
========================================================================*/

/*************************************************************************
    Function        : . export specific group to .ini file
    Parameters IN   : . group_id, ini_path
    Parameters OUT  : .
    Return Code     : . CFG_DB_OK or error code
    Note            : Phase 3 - full implementation pending
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_export_ini(const char *group_id, const char *ini_path)
/*----------------------------------------------------------------------*/
{
    sqlite3_stmt    *stmt = NULL;
    FILE            *fp = NULL;
    const char      *env_id;
    int             rc, count = 0;
    static const char *sql =
    "SELECT section, key_name, key_value "
    "FROM fep_config "
    "WHERE env_id=? AND group_id=? "
    "ORDER BY section, sort_order";

    if (g_db == NULL)
        return CFG_DB_ERR_OPEN;

    env_id = cfg_db_detect_env();

    rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        Log(ORA_WARN, "cfg_db_export_ini: prepare failed: %s",
                sqlite3_errmsg(g_db));
        return CFG_DB_ERR_QUERY;
    }

    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, group_id, -1, SQLITE_STATIC);

    fp = fopen(ini_path, "w");
    if (fp == NULL) {
        Log(ORA_WARN, "cfg_db_export_ini: cannot create [%s]", ini_path);
        sqlite3_finalize(stmt);
        return CFG_DB_ERR_OPEN;
    }

    /* simple key=value export (one per line, section header) */
    {
        char    prev_sec[4];
        memset(prev_sec, 0, sizeof (prev_sec));

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char  *sec = (const char *) sqlite3_column_text(stmt, 0);
            const char  *key = (const char *) sqlite3_column_text(stmt, 1);
            const char  *val = (const char *) sqlite3_column_text(stmt, 2);

            if (sec == NULL || key == NULL)
                continue;

            /* section change: write header comment */
            if (strcmp(prev_sec, sec) != 0) {
                fprintf(fp, "# [%s:%s]\n", group_id, sec);
                strncpy(prev_sec, sec, sizeof (prev_sec) - 1);
            }

            fprintf(fp, "%s=%s\n", key,
                    val != NULL ? val : "");
            count++;
        }
    }

    fclose(fp);
    sqlite3_finalize(stmt);

    Log(ORA_OK, "cfg_db_export_ini: group=%s keys=%d path=%s",
            group_id, count, ini_path);
    return CFG_DB_OK;

}   /* End of cfg_db_export_ini () */

/*************************************************************************
    Function        : . export all groups to .ini files
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . CFG_DB_OK or error code
    Note            : Phase 3 - full implementation pending
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_sync_all_ini(void)
/*----------------------------------------------------------------------*/
{
    sqlite3_stmt    *stmt = NULL;
    int             rc, count = 0;
    char            ini_path[256];
    char            *cfg_dir;
    static const char *sql =
    "SELECT group_id, ini_file FROM fep_config_group "
    "ORDER BY load_order";

    if (g_db == NULL)
        return CFG_DB_ERR_OPEN;

    /* determine config directory */
    cfg_dir = getenv("_FEP_CFG");
    if (cfg_dir == NULL) {
        Log(ORA_WARN, "cfg_db_sync_all_ini: _FEP_CFG not set, skip");
        return CFG_DB_OK;
    }

    rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        Log(ORA_WARN, "cfg_db_sync_all_ini: prepare failed: %s",
                sqlite3_errmsg(g_db));
        return CFG_DB_ERR_QUERY;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char  *grp = (const char *) sqlite3_column_text(stmt, 0);
        const char  *ini = (const char *) sqlite3_column_text(stmt, 1);

        if (grp == NULL || ini == NULL)
            continue;

        snprintf(ini_path, sizeof (ini_path),
                "%s/%s.dbsync", cfg_dir, ini);

        rc = cfg_db_export_ini(grp, ini_path);
        if (rc == CFG_DB_OK)
            count++;
    }

    sqlite3_finalize(stmt);

    Log(ORA_OK, "cfg_db_sync_all_ini: exported %d groups", count);
    return CFG_DB_OK;

}   /* End of cfg_db_sync_all_ini () */

/*========================================================================
    Password Encryption Functions
========================================================================*/

/*************************************************************************
    Function        : . encrypt LogonPW (plaintext -> ciphertext)
    Parameters IN   : . plain (plaintext password)
    Parameters OUT  : . cipher (output buffer, 64 bytes min)
    Return Code     : . CFG_DB_OK or CFG_DB_ERR_ENCRYPT
    Note            : Phase 2 - placeholder, currently passthrough
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_encrypt_pw(const char *plain, char *cipher)
/*----------------------------------------------------------------------*/
{
    /* TODO Phase 2: implement AES-256-CBC encryption
       using INISAFENET library or OpenSSL */

    /* placeholder: copy plaintext as-is */
    if (plain == NULL || cipher == NULL)
        return CFG_DB_ERR_ENCRYPT;

    strncpy(cipher, plain, 63);
    cipher[63] = '\0';

    return CFG_DB_OK;

}   /* End of cfg_db_encrypt_pw () */

/*************************************************************************
    Function        : . decrypt LogonPW (ciphertext -> plaintext)
    Parameters IN   : . cipher (ciphertext)
    Parameters OUT  : . plain (output buffer, 20 bytes min)
    Return Code     : . CFG_DB_OK or CFG_DB_ERR_ENCRYPT
    Note            : Phase 2 - placeholder, currently passthrough
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_decrypt_pw(const char *cipher, char *plain)
/*----------------------------------------------------------------------*/
{
    /* TODO Phase 2: implement AES-256-CBC decryption
       using INISAFENET library or OpenSSL */

    /* placeholder: copy ciphertext as-is */
    if (cipher == NULL || plain == NULL)
        return CFG_DB_ERR_ENCRYPT;

    strncpy(plain, cipher, 19);
    plain[19] = '\0';

    return CFG_DB_OK;

}   /* End of cfg_db_decrypt_pw () */

/*========================================================================
    Utility Functions
========================================================================*/

/*************************************************************************
    Function        : . get DB config value as string
    Parameters IN   : . env_id, group_id, section, key_name
    Parameters OUT  : . out_buf
    Return Code     : . CFG_DB_OK or error code
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_get_value(const char *env_id, const char *group_id,
        const char *section, const char *key_name,
        char *out_buf, int buf_len)
/*----------------------------------------------------------------------*/
{
    sqlite3_stmt    *stmt = NULL;
    int             rc;
    const char      *sql;
    const char      *val;

    if (g_db == NULL)
        return CFG_DB_ERR_OPEN;

    sql = "SELECT key_value FROM fep_config "
    "WHERE env_id=? AND group_id=? AND section=? AND key_name=?";

    rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        Log(ORA_ERROR, "cfg_db_get_value: prepare failed: %s",
                sqlite3_errmsg(g_db));
        return CFG_DB_ERR_QUERY;
    }

    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, group_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, section, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, key_name, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return CFG_DB_ERR_NODATA;
    }

    val = (const char *) sqlite3_column_text(stmt, 0);
    if (val != NULL) {
        strncpy(out_buf, val, buf_len - 1);
        out_buf[buf_len - 1] = '\0';
    }
    else {
        out_buf[0] = '\0';
    }

    sqlite3_finalize(stmt);
    return CFG_DB_OK;

}   /* End of cfg_db_get_value () */

/*************************************************************************
    Function        : . get DB config value as integer
    Parameters IN   : . env_id, group_id, section, key_name, default_val
    Parameters OUT  : .
    Return Code     : . integer value, or default_val on error
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_get_int(const char *env_id, const char *group_id,
        const char *section, const char *key_name,
        int default_val)
/*----------------------------------------------------------------------*/
{
    char    buf[64];
    int     rc;

    rc = cfg_db_get_value(env_id, group_id, section, key_name,
            buf, sizeof(buf));

    if (rc != CFG_DB_OK)
        return default_val;

    return atoi(buf);

}   /* End of cfg_db_get_int () */

/*************************************************************************
    Function        : . get loaded key count for a specific group
    Parameters IN   : . env_id, group_id
    Parameters OUT  : .
    Return Code     : . key count or 0
*************************************************************************/
/*----------------------------------------------------------------------*/
int     cfg_db_get_key_count(const char *env_id, const char *group_id)
/*----------------------------------------------------------------------*/
{
    sqlite3_stmt    *stmt = NULL;
    int             rc;
    int             count = 0;
    const char      *sql;

    if (g_db == NULL)
        return 0;

    sql = "SELECT COUNT(*) FROM fep_config "
    "WHERE env_id=? AND group_id=?";

    rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return 0;

    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, group_id, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
        count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return count;

}   /* End of cfg_db_get_key_count () */

/*************************************************************************
    End of Program (config_db.c)
*************************************************************************/
