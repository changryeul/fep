/*------------------------------------------------------------------------
#   Module  : SHM vs DB configuration verification tool
#   File    : cfg_verify.c
#   Description : Attaches to existing FEP SHM, opens SQLite DB,
#                 compares key config fields, reports mismatches.
#                 Used for TC-02: DB load ↔ INI load equivalence.
#
#   Build   : cc -o cfg_verify cfg_verify.c -I$_FEP_INC -L$_FEP_LIB \
#             -lfepP -lsqlite3
#   Usage   : cfg_verify [-e ENV_ID] [-d DB_PATH] [-v]
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <sqlite3.h>

#include    "fep_sub.h"

/*------------------------------------------------------------------------
    Constants
------------------------------------------------------------------------*/
#define     MAX_VAL     256
#define     VF_OK       0
#define     VF_MISMATCH 1

/*------------------------------------------------------------------------
    Static Variables
------------------------------------------------------------------------*/
static int  g_total = 0;        /* total fields checked     */
static int  g_pass  = 0;        /* fields that matched      */
static int  g_fail  = 0;        /* fields that mismatched   */
static int  g_skip  = 0;        /* fields skipped (no DB)   */
static int  g_verbose = 0;      /* verbose mode             */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
static int      db_get(sqlite3 *db, const char *env_id,
        const char *group_id, const char *section,
        const char *key_name, char *out, int out_len);
static void     chk_str(sqlite3 *db, const char *env, const char *grp,
        const char *sec, const char *key,
        const char *shm_val, const char *label);
static void     chk_int(sqlite3 *db, const char *env, const char *grp,
        const char *sec, const char *key,
        int shm_val, const char *label);
static void     chk_ip4(sqlite3 *db, const char *env, const char *grp,
        const char *sec, const char *key,
        unsigned char ip[4], const char *label);
static int      verify_daemon(sqlite3 *db, const char *env, int i);
static int      verify_file(sqlite3 *db, const char *env, int i);
static int      verify_dshm(sqlite3 *db, const char *env, int i);
static int      verify_tcp1(sqlite3 *db, const char *env, int i);
static int      verify_tcp2(sqlite3 *db, const char *env, int i);
static int      verify_udpip(sqlite3 *db, const char *env, int i);
static const char*  detect_env(void);

/*************************************************************************
    Function        : . query single value from fep_config
    Parameters IN   : . db, env_id, group_id, section, key_name
    Parameters OUT  : . out (value string)
    Return Code     : . 0 (found), -1 (not found)
*************************************************************************/
/*----------------------------------------------------------------------*/
static int      db_get(sqlite3 *db, const char *env_id,
        const char *group_id, const char *section,
        const char *key_name, char *out, int out_len)
/*----------------------------------------------------------------------*/
{
    sqlite3_stmt    *stmt = NULL;
    int             rc;
    const char      *sql =
    "SELECT key_value FROM fep_config "
    "WHERE env_id=? AND group_id=? AND section=? AND key_name=?";

    memset(out, 0, out_len);

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, group_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, section, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, key_name, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const char  *val = (const char *)sqlite3_column_text(stmt, 0);
        if (val != NULL)
            strncpy(out, val, out_len - 1);
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return -1;
}

/*************************************************************************
    Comparison Helpers
*************************************************************************/
/*----------------------------------------------------------------------*/
static void     chk_str(sqlite3 *db, const char *env, const char *grp,
        const char *sec, const char *key,
        const char *shm_val, const char *label)
/*----------------------------------------------------------------------*/
{
    char    db_val[MAX_VAL];

    g_total++;

    if (db_get(db, env, grp, sec, key, db_val, sizeof(db_val)) != 0) {
        g_skip++;
        if (g_verbose)
            printf("  SKIP  %-40s(no DB key: %s.%s.%s)\n",
                    label, grp, sec, key);
        return;
    }

    if (strncmp(shm_val, db_val, strlen(db_val)) == 0) {
        g_pass++;
        if (g_verbose > 1)
            printf("  PASS  %-40s [%s]\n", label, shm_val);
    }
    else {
        g_fail++;
        printf("  FAIL  %-40s  SHM=[%.40s]  DB=[%.40s]\n",
                label, shm_val, db_val);
    }
}

/*----------------------------------------------------------------------*/
static void     chk_int(sqlite3 *db, const char *env, const char *grp,
        const char *sec, const char *key,
        int shm_val, const char *label)
/*----------------------------------------------------------------------*/
{
    char    db_val[MAX_VAL];
    int     db_int;

    g_total++;

    if (db_get(db, env, grp, sec, key, db_val, sizeof(db_val)) != 0) {
        g_skip++;
        if (g_verbose)
            printf("  SKIP  %-40s(no DB key: %s.%s.%s)\n",
                    label, grp, sec, key);
        return;
    }

    db_int = atoi(db_val);

    if (shm_val == db_int) {
        g_pass++;
        if (g_verbose > 1)
            printf("  PASS  %-40s [%d]\n", label, shm_val);
    }
    else {
        g_fail++;
        printf("  FAIL  %-40s  SHM=[%d]  DB=[%d]\n",
                label, shm_val, db_int);
    }
}

/*----------------------------------------------------------------------*/
static void     chk_ip4(sqlite3 *db, const char *env, const char *grp,
        const char *sec, const char *key,
        unsigned char ip[4], const char *label)
/*----------------------------------------------------------------------*/
{
    char    db_val[MAX_VAL];
    char    shm_ip[20];
    int     a, b, c, d;

    g_total++;

    sprintf(shm_ip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    if (db_get(db, env, grp, sec, key, db_val, sizeof(db_val)) != 0) {
        g_skip++;
        return;
    }

    /* normalize DB IP: parse and reformat */
    if (sscanf(db_val, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        char    norm[20];
        sprintf(norm, "%d.%d.%d.%d", a, b, c, d);

        if (strcmp(shm_ip, norm) == 0) {
            g_pass++;
            if (g_verbose > 1)
                printf("  PASS  %-40s [%s]\n", label, shm_ip);
        }
        else {
            g_fail++;
            printf("  FAIL  %-40s  SHM=[%s]  DB=[%s]\n",
                    label, shm_ip, norm);
        }
    }
    else {
        g_fail++;
        printf("  FAIL  %-40s  SHM=[%s]  DB=[%s] (parse error)\n",
                label, shm_ip, db_val);
    }
}

/*************************************************************************
    Per-Group Verification Functions
*************************************************************************/
/*----------------------------------------------------------------------*/
static int      verify_daemon(sqlite3 *db, const char *env, int i)
/*----------------------------------------------------------------------*/
{
    char    sec[4], label[80];

    sprintf(sec, "%c", i + 'A');

    if (INFO(i).process_id[0] == 0)
        return 0;

    printf("\n  [Daemon %c]\n", i + 'A');

    sprintf(label, "Daemon_%c.ID", i + 'A');
    chk_str(db, env, "daemon", sec, "ID",
            INFO(i).process_id, label);

    sprintf(label, "Daemon_%c.START_TIME", i + 'A');
    chk_str(db, env, "daemon", sec, "START_TIME",
            INFO(i).start_time, label);

    sprintf(label, "Daemon_%c.END_TIME", i + 'A');
    chk_str(db, env, "daemon", sec, "END_TIME",
            INFO(i).end_time, label);

    sprintf(label, "Daemon_%c.DATE_FLAG", i + 'A');
    chk_int(db, env, "daemon", sec, "DATE_FLAG",
            (int)INFO(i).date_flag, label);

    sprintf(label, "Daemon_%c.COMPACT_DAYS", i + 'A');
    chk_int(db, env, "daemon", sec, "COMPACT_DAYS",
            (int)INFO(i).compact_days, label);

    sprintf(label, "Daemon_%c.PROC_COUNT", i + 'A');
    chk_int(db, env, "daemon", sec, "PROC_COUNT",
            (int)INFO(i).process_count, label);

    sprintf(label, "Daemon_%c.FILE_COUNT", i + 'A');
    chk_int(db, env, "daemon", sec, "FILE_COUNT",
            (int)INFO(i).file_count, label);

    sprintf(label, "Daemon_%c.DSHM_COUNT", i + 'A');
    chk_int(db, env, "daemon", sec, "DSHM_COUNT",
            (int)INFO(i).dshm_count, label);

    sprintf(label, "Daemon_%c.TCP1_COUNT", i + 'A');
    chk_int(db, env, "daemon", sec, "TCP1_COUNT",
            (int)INFO(i).tcp1_count, label);

    sprintf(label, "Daemon_%c.TCP2_COUNT", i + 'A');
    chk_int(db, env, "daemon", sec, "TCP2_COUNT",
            (int)INFO(i).tcp2_count, label);

    sprintf(label, "Daemon_%c.UDPIP_COUNT", i + 'A');
    chk_int(db, env, "daemon", sec, "UDPIP_COUNT",
            (int)INFO(i).udpip_count, label);

    sprintf(label, "Daemon_%c.DATA_COUNT", i + 'A');
    chk_int(db, env, "daemon", sec, "DATA_COUNT",
            (int)INFO(i).data_count, label);

    return 0;
}

/*----------------------------------------------------------------------*/
static int      verify_file(sqlite3 *db, const char *env, int i)
/*----------------------------------------------------------------------*/
{
    char    sec[4], key[40], label[80];
    int     j, cnt;

    sprintf(sec, "%c", i + 'A');
    cnt = (int)DAEMON(i).f_count;

    for (j = 0; j < cnt; j++) {
        sprintf(label, "File_%c_%d.NAME", i + 'A', j + 1);
        sprintf(key, "FILE_%d_NAME", j + 1);
        chk_str(db, env, "file", sec, key,
                FILEM(i, j).file_name, label);

        sprintf(label, "File_%c_%d.FIFO", i + 'A', j + 1);
        sprintf(key, "FILE_%d_FIFO", j + 1);
        chk_int(db, env, "file", sec, key,
                (int)FILEM(i, j).fifo_count, label);

        sprintf(label, "File_%c_%d.SIZE", i + 'A', j + 1);
        sprintf(key, "FILE_%d_SIZE", j + 1);
        chk_int(db, env, "file", sec, key,
                (int)FILEM(i, j).record_size, label);
    }

    return 0;
}

/*----------------------------------------------------------------------*/
static int      verify_dshm(sqlite3 *db, const char *env, int i)
/*----------------------------------------------------------------------*/
{
    char    sec[4], key[40], label[80];
    int     j, cnt;

    sprintf(sec, "%c", i + 'A');
    cnt = (int)DAEMON(i).d_count;

    for (j = 0; j < cnt; j++) {
        sprintf(label, "Dshm_%c_%d.NAME", i + 'A', j + 1);
        sprintf(key, "DSHM_%d_NAME", j + 1);
        chk_str(db, env, "dshm", sec, key,
                DSHM(i, j).data_name, label);

        sprintf(label, "Dshm_%c_%d.KEY", i + 'A', j + 1);
        sprintf(key, "DSHM_%d_KEY", j + 1);
        chk_str(db, env, "dshm", sec, key,
                DSHM(i, j).key_info, label);

        sprintf(label, "Dshm_%c_%d.FIFO", i + 'A', j + 1);
        sprintf(key, "DSHM_%d_FIFO", j + 1);
        chk_int(db, env, "dshm", sec, key,
                (int)DSHM(i, j).fifo_count, label);

        sprintf(label, "Dshm_%c_%d.SIZE", i + 'A', j + 1);
        sprintf(key, "DSHM_%d_SIZE", j + 1);
        chk_int(db, env, "dshm", sec, key,
                (int)DSHM(i, j).data_size, label);

        sprintf(label, "Dshm_%c_%d.MAX", i + 'A', j + 1);
        sprintf(key, "DSHM_%d_MAX", j + 1);
        chk_int(db, env, "dshm", sec, key,
                DSHM(i, j).max_rec, label);
    }

    return 0;
}

/*----------------------------------------------------------------------*/
static int      verify_tcp1(sqlite3 *db, const char *env, int i)
/*----------------------------------------------------------------------*/
{
    char    sec[4], key[40], label[80];
    int     j, cnt;

    sprintf(sec, "%c", i + 'A');
    cnt = (int)DAEMON(i).t1_count;

    for (j = 0; j < cnt; j++) {
        sprintf(label, "Tcp1_%c_%d.PORT", i + 'A', j + 1);
        sprintf(key, "TCP1_%d_PORT", j + 1);
        chk_int(db, env, "tcp1", sec, key,
                (int)TCP1(i, j).port_no, label);

        sprintf(label, "Tcp1_%c_%d.IP", i + 'A', j + 1);
        sprintf(key, "TCP1_%d_IP", j + 1);
        chk_ip4(db, env, "tcp1", sec, key,
                TCP1(i, j).ip_addr, label);
    }

    return 0;
}

/*----------------------------------------------------------------------*/
static int      verify_tcp2(sqlite3 *db, const char *env, int i)
/*----------------------------------------------------------------------*/
{
    char    sec[4], key[40], label[80];
    int     j, cnt;

    sprintf(sec, "%c", i + 'A');
    cnt = (int)DAEMON(i).t2_count;

    for (j = 0; j < cnt; j++) {
        sprintf(label, "Tcp2_%c_%d.ID", i + 'A', j + 1);
        sprintf(key, "TCP2_%d_ID", j + 1);
        chk_int(db, env, "tcp2", sec, key,
                (int)TCP2(i, j).dup_id, label);

        sprintf(label, "Tcp2_%c_%d.PORT", i + 'A', j + 1);
        sprintf(key, "TCP2_%d_PORT", j + 1);
        chk_int(db, env, "tcp2", sec, key,
                (int)TCP2(i, j).port_no, label);

        sprintf(label, "Tcp2_%c_%d.IP", i + 'A', j + 1);
        sprintf(key, "TCP2_%d_IP", j + 1);
        chk_ip4(db, env, "tcp2", sec, key,
                TCP2(i, j).ip_addr, label);
    }

    return 0;
}

/*----------------------------------------------------------------------*/
static int      verify_udpip(sqlite3 *db, const char *env, int i)
/*----------------------------------------------------------------------*/
{
    char    sec[4], key[40], label[80];
    int     j, cnt;

    sprintf(sec, "%c", i + 'A');
    cnt = (int)DAEMON(i).u_count;

    for (j = 0; j < cnt; j++) {
        sprintf(label, "Udpip_%c_%d.ID", i + 'A', j + 1);
        sprintf(key, "UDPIP_%d_ID", j + 1);
        chk_int(db, env, "udpip", sec, key,
                (int)UDPIP(i, j).dup_id, label);

        sprintf(label, "Udpip_%c_%d.PORT", i + 'A', j + 1);
        sprintf(key, "UDPIP_%d_PORT", j + 1);
        chk_int(db, env, "udpip", sec, key,
                (int)UDPIP(i, j).port[0], label);

        sprintf(label, "Udpip_%c_%d.IP", i + 'A', j + 1);
        sprintf(key, "UDPIP_%d_IP", j + 1);
        chk_ip4(db, env, "udpip", sec, key,
                UDPIP(i, j).ip_addr[0], label);
    }

    return 0;
}

/*************************************************************************
    Environment Detection
*************************************************************************/
/*----------------------------------------------------------------------*/
static const char*      detect_env(void)
/*----------------------------------------------------------------------*/
{
    char    hostname[64];

    memset(hostname, 0, sizeof(hostname));
    gethostname(hostname, sizeof(hostname) - 1);

    if (strcmp(hostname, "podm11") == 0) return "REAL1";
    if (strcmp(hostname, "podm12") == 0) return "REAL2";
    return "TEST";
}

/*************************************************************************
    Main
*************************************************************************/
/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    sqlite3     *db = NULL;
    char        db_path[256];
    char        env_id[16];
    int         i, rc;

    /* defaults */
    memset(db_path, 0, sizeof(db_path));
    memset(env_id, 0, sizeof(env_id));

    /* parse arguments */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            strncpy(env_id, argv[++i], sizeof(env_id) - 1);
        }
        else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            strncpy(db_path, argv[++i], sizeof(db_path) - 1);
        }
        else if (strcmp(argv[i], "-v") == 0) {
            g_verbose = 1;
        }
        else if (strcmp(argv[i], "-vv") == 0) {
            g_verbose = 2;
        }
        else if (strcmp(argv[i], "-h") == 0) {
            printf("Usage: cfg_verify [-e ENV_ID] [-d DB_PATH] [-v|-vv]\n");
            printf("  -e ENV_ID   : environment ID(auto-detect if omitted)\n");
            printf("  -d DB_PATH  : SQLite DB path(default: $_FEP_CFG/fep_config.db)\n");
            printf("  -v          : verbose(show skipped fields)\n");
            printf("  -vv         : very verbose(show all fields)\n");
            return 0;
        }
    }

    /* detect environment */
    if (env_id[0] == '\0')
        strncpy(env_id, detect_env(), sizeof(env_id) - 1);

    /* determine DB path */
    if (db_path[0] == '\0') {
        char    *cfg_env;

        cfg_env = getenv("_FEP_CFG");
        if (cfg_env != NULL && cfg_env[0] != '\0')
            snprintf(db_path, sizeof(db_path), "%s/fep_config.db", cfg_env);
        else {
            fprintf(stderr, "ERROR: _FEP_CFG not set and no -d option\n");
            return 1;
        }
    }

    printf("==========================================================\n");
    printf("  cfg_verify: SHM vs DB Configuration Verification\n");
    printf("==========================================================\n");
    printf("  Environment : %s\n", env_id);
    printf("  DB Path     : %s\n", db_path);
    printf("==========================================================\n");

    /* 1. attach to daemon SHM (INFO) */
    Sub_SHM();
    printf("  SHM attached(daemon INFO)\n");

    /* 2. attach to sub-daemon SHMs (Shm_Mem) */
    Mem_SHM(0, -1);
    printf("  SHM attached(sub-daemon Shm_Mem)\n");

    /* 3. open DB */
    rc = sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READONLY, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "ERROR: cannot open DB [%s]: %s\n",
                db_path, sqlite3_errmsg(db));
        if (db) sqlite3_close(db);
        return 1;
    }
    printf("  DB opened\n");
    printf("----------------------------------------------------------\n");

    /* 4. verify each active daemon */
    for (i = 0; i < SHM_MAX_SUB; i++) {
        if (INFO(i).date_flag == 9)
            continue;
        if (INFO(i).process_count == 0 && INFO(i).process_id[0] == 0)
            continue;

        verify_daemon(db, env_id, i);

        /* only verify sub-groups if Shm_Mem is attached */
        if (Shm_Mem[i].Daemon != NULL) {
            if (DAEMON(i).f_count > 0)
                verify_file(db, env_id, i);
            if (DAEMON(i).d_count > 0)
                verify_dshm(db, env_id, i);
            if (DAEMON(i).t1_count > 0)
                verify_tcp1(db, env_id, i);
            if (DAEMON(i).t2_count > 0)
                verify_tcp2(db, env_id, i);
            if (DAEMON(i).u_count > 0)
                verify_udpip(db, env_id, i);
        }
    }

    /* 5. close DB */
    sqlite3_close(db);

    /* 6. summary */
    printf("\n==========================================================\n");
    printf("  Verification Summary\n");
    printf("==========================================================\n");
    printf("  Total fields : %d\n", g_total);
    printf("  PASS         : %d\n", g_pass);
    printf("  FAIL         : %d\n", g_fail);
    printf("  SKIP(no DB) : %d\n", g_skip);
    printf("----------------------------------------------------------\n");

    if (g_fail == 0)
        printf("  RESULT: PASS - DB matches SHM\n");
    else
        printf("  RESULT: FAIL - %d mismatches found\n", g_fail);

    printf("==========================================================\n");

    return (g_fail > 0) ? 1 : 0;

}   /* End of main () */

/*************************************************************************
    End of Program (cfg_verify.c)
*************************************************************************/
