#if !defined(_LARGE_FILES)
#define     _LARGE_FILES
#endif
/*------------------------------------------------------------------------
#   Module  : config loader abstraction layer
#   File    : config_loader.c
#   Description : Provides Config_Load_All() as the unified entry point
#                 for all FEP configuration loading.
#                 Selects DB or INI source based on _FEP_DB_MODE.
#                 AUTO mode (default): DB first, .ini fallback on failure.
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "daemon.h"
#include    "config_db.h"
#include    "config_loader.h"

/*------------------------------------------------------------------------
    Static Variables
------------------------------------------------------------------------*/
static CFG_LOAD_MODE    g_load_mode = CFG_MODE_AUTO;
static CFG_LOAD_RESULT  g_last_result;
static int              g_initialized = 0;

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
CFG_LOAD_RESULT     Config_Load_All(int flag);
CFG_LOAD_RESULT     Config_Reload_Group(const char *group_id, int flag);
CFG_LOAD_MODE       Config_Get_Mode(void);
CFG_LOAD_RESULT     Config_Get_Last_Result(void);

static void         cfg_loader_detect_mode(void);
static int          cfg_loader_load_db(int flag);
static void         cfg_loader_load_ini(int flag);
static int          cfg_loader_load_group_db(const char *group_id, int flag);
static void         cfg_loader_load_group_ini(const char *group_id, int flag);

/*************************************************************************
    Function        : . detect load mode from _FEP_DB_MODE env var
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void (sets g_load_mode)
*************************************************************************/
/*----------------------------------------------------------------------*/
static void     cfg_loader_detect_mode(void)
/*----------------------------------------------------------------------*/
{
    char    *mode_env;

    mode_env = getenv("_FEP_DB_MODE");

    if (mode_env == NULL || mode_env[0] == '\0') {
        g_load_mode = CFG_MODE_AUTO;
    }
    else if (strcmp(mode_env, "INI") == 0) {
        g_load_mode = CFG_MODE_INI;
    }
    else if (strcmp(mode_env, "DB") == 0) {
        g_load_mode = CFG_MODE_DB;
    }
    else {
        g_load_mode = CFG_MODE_AUTO;
        Log(USR_WARN, "Config_Load: unknown _FEP_DB_MODE [%s], using AUTO",
                mode_env);
    }

    g_initialized = 1;
    return;

}   /* End of cfg_loader_detect_mode () */

/*************************************************************************
    Function        : . load all config from DB
    Parameters IN   : . flag
    Parameters OUT  : .
    Return Code     : . 0 (all success) or non-zero (partial/full failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
static int      cfg_loader_load_db(int flag)
/*----------------------------------------------------------------------*/
{
    int     rc = 0;
    int     err_cnt = 0;

    rc = cfg_db_load_file(flag);
    if (rc != CFG_DB_OK) {
        Log(ORA_WARN, "Config_Load: cfg_db_load_file failed(rc=%d)", rc);
        err_cnt++;
    }

    rc = cfg_db_load_dshm(flag);
    if (rc != CFG_DB_OK) {
        Log(ORA_WARN, "Config_Load: cfg_db_load_dshm failed(rc=%d)", rc);
        err_cnt++;
    }

    rc = cfg_db_load_tcp1(flag);
    if (rc != CFG_DB_OK) {
        Log(ORA_WARN, "Config_Load: cfg_db_load_tcp1 failed(rc=%d)", rc);
        err_cnt++;
    }

    rc = cfg_db_load_tcp2(flag);
    if (rc != CFG_DB_OK) {
        Log(ORA_WARN, "Config_Load: cfg_db_load_tcp2 failed(rc=%d)", rc);
        err_cnt++;
    }

    rc = cfg_db_load_udpip(flag);
    if (rc != CFG_DB_OK) {
        Log(ORA_WARN, "Config_Load: cfg_db_load_udpip failed(rc=%d)", rc);
        err_cnt++;
    }

    rc = cfg_db_load_proc(flag);
    if (rc != CFG_DB_OK) {
        Log(ORA_WARN, "Config_Load: cfg_db_load_proc failed(rc=%d)", rc);
        err_cnt++;
    }

    /* SiseTr and Accno are currently disabled (since 202201) */

    return err_cnt;

}   /* End of cfg_loader_load_db () */

/*************************************************************************
    Function        : . load all config from .ini files (existing logic)
    Parameters IN   : . flag
    Parameters OUT  : .
    Return Code     : . void
    Note            : Calls existing *_Config_Read() functions directly
*************************************************************************/
/*----------------------------------------------------------------------*/
static void     cfg_loader_load_ini(int flag)
/*----------------------------------------------------------------------*/
{
    Log(USR_OK, "cfg_loader_load_ini: START flag=%d", flag);

    File_Config_Read(flag);
    Log(USR_OK, "cfg_loader_load_ini: File_Config_Read done");

    Dshm_Config_Read(flag);
    Log(USR_OK, "cfg_loader_load_ini: Dshm_Config_Read done");

#if defined ISAM_INCL
    Cisam_Config_Read(flag);
    Log(USR_OK, "cfg_loader_load_ini: Cisam_Config_Read done");
#endif

    Tcp1_Config_Read(flag);
    Log(USR_OK, "cfg_loader_load_ini: Tcp1_Config_Read done");

    Tcp2_Config_Read(flag);
    Log(USR_OK, "cfg_loader_load_ini: Tcp2_Config_Read done");

    Udpip_Config_Read(flag);
    Log(USR_OK, "cfg_loader_load_ini: Udpip_Config_Read done");

    Log(USR_OK, "cfg_loader_load_ini: calling Proc_Config_Read ...");
    Proc_Config_Read(flag);
    Log(USR_OK, "cfg_loader_load_ini: Proc_Config_Read done");

    /* SiseTr_Config_Read and Accno_Config_Read disabled since 202201 */

    return;

}   /* End of cfg_loader_load_ini () */

/*************************************************************************
    Function        : . load specific group from DB
    Parameters IN   : . group_id, flag
    Parameters OUT  : .
    Return Code     : . 0 (success) or error code
*************************************************************************/
/*----------------------------------------------------------------------*/
static int      cfg_loader_load_group_db(const char *group_id, int flag)
/*----------------------------------------------------------------------*/
{
    if (strcmp(group_id, "daemon") == 0)
        return cfg_db_load_daemon(flag);
    else if (strcmp(group_id, "file") == 0)
        return cfg_db_load_file(flag);
    else if (strcmp(group_id, "dshm") == 0)
        return cfg_db_load_dshm(flag);
    else if (strcmp(group_id, "tcp1") == 0)
        return cfg_db_load_tcp1(flag);
    else if (strcmp(group_id, "tcp2") == 0)
        return cfg_db_load_tcp2(flag);
    else if (strcmp(group_id, "udpip") == 0)
        return cfg_db_load_udpip(flag);
    else if (strcmp(group_id, "sisetr") == 0)
        return cfg_db_load_sisetr(flag);
    else if (strcmp(group_id, "proc") == 0)
        return cfg_db_load_proc(flag);
    else {
        Log(ORA_WARN, "cfg_loader_load_group_db: unknown group [%s]",
                group_id);
        return CFG_DB_ERR_NODATA;
    }

}   /* End of cfg_loader_load_group_db () */

/*************************************************************************
    Function        : . load specific group from .ini (existing logic)
    Parameters IN   : . group_id, flag
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
static void     cfg_loader_load_group_ini(const char *group_id, int flag)
/*----------------------------------------------------------------------*/
{
    if (strcmp(group_id, "daemon") == 0)
        Daemon_Config_Read(flag);
    else if (strcmp(group_id, "file") == 0)
        File_Config_Read(flag);
    else if (strcmp(group_id, "dshm") == 0)
        Dshm_Config_Read(flag);
    else if (strcmp(group_id, "tcp1") == 0)
        Tcp1_Config_Read(flag);
    else if (strcmp(group_id, "tcp2") == 0)
        Tcp2_Config_Read(flag);
    else if (strcmp(group_id, "udpip") == 0)
        Udpip_Config_Read(flag);
    else if (strcmp(group_id, "sisetr") == 0)
        Log(USR_WARN, "SiseTr_Config_Read disabled since 202201, use DB mode");
    else if (strcmp(group_id, "proc") == 0)
        Proc_Config_Read(flag);
    else
        Log(USR_WARN, "cfg_loader_load_group_ini: unknown group [%s]",
                group_id);

    return;

}   /* End of cfg_loader_load_group_ini () */

/*************************************************************************
    Function        : . load all configuration (main entry point)
    Parameters IN   : . flag (same as *_Config_Read flag)
                        0: full init (all daemons)
                        1: current daemon only
    Parameters OUT  : .
    Return Code     : . CFG_LOAD_RESULT
    Note            : This replaces direct *_Config_Read() calls
                      in Shm_Conf_Process().
                      Daemon_Config_Read() is NOT included here
                      (it is called separately in Main_Process_Memory).
*************************************************************************/
/*----------------------------------------------------------------------*/
CFG_LOAD_RESULT     Config_Load_All(int flag)
/*----------------------------------------------------------------------*/
{
    CFG_LOAD_RESULT     result;
    char                db_path[256];
    int                 db_err;

    memset(&result, 0, sizeof(result));

    /* 1. detect load mode */
    if (!g_initialized)
        cfg_loader_detect_mode();

    Log(USR_OK, "Config_Load_All: mode=%s flag=%d",
            (g_load_mode == CFG_MODE_DB)  ? "DB" :
            (g_load_mode == CFG_MODE_INI) ? "INI" : "AUTO",
            flag);

    /* 2. INI mode: use existing logic directly */
    if (g_load_mode == CFG_MODE_INI) {
        cfg_loader_load_ini(flag);
        result.source = CFG_SRC_INI;
        result.ini_ok = 1;
        goto done;
    }

    /* 3. determine DB path */
    {
        char    *db_env;

        db_env = getenv("_FEP_DB");
        if (db_env != NULL && db_env[0] != '\0') {
            strncpy(db_path, db_env, sizeof(db_path) - 1);
            db_path[sizeof(db_path) - 1] = '\0';
        }
        else {
            snprintf(db_path, sizeof(db_path), "%s/fep_config.db",
                    _FEP_CFG);
        }
    }

    /* 4. try DB load */
    if (cfg_db_open(db_path) == CFG_DB_OK) {
        db_err = cfg_loader_load_db(flag);

        if (db_err == 0) {
            /* DB load success */
            result.source = CFG_SRC_DB;
            result.db_ok = 1;

            {
                const char  *env;

                env = cfg_db_detect_env();
                result.total_keys = cfg_db_get_key_count(env, "file")
                + cfg_db_get_key_count(env, "dshm")
                + cfg_db_get_key_count(env, "tcp1")
                + cfg_db_get_key_count(env, "tcp2")
                + cfg_db_get_key_count(env, "udpip")
                + cfg_db_get_key_count(env, "proc");
            }

            Log(USR_OK, "Config_Load_All: loaded from DB [%s] keys=%d",
                    db_path, result.total_keys);

            /* sync DB -> .ini (keep fallback files updated) */
            cfg_db_sync_all_ini();
        }
        else {
            /* DB load partial/full failure */
            cfg_db_close();
            Log(USR_WARN, "Config_Load_All: DB load failed(%d errors), "
                    "fallback to INI", db_err);
            goto fallback_ini;
        }

        cfg_db_close();
    }
    /* 5. DB open failed */
    else if (g_load_mode == CFG_MODE_AUTO) {
        fallback_ini:
        Log(USR_WARN, "Config_Load_All: using INI fallback");
        cfg_loader_load_ini(flag);
        result.source = CFG_SRC_INI;
        result.ini_ok = 1;
    }
    /* 6. DB-only mode and DB failed -> fatal error */
    else {
        Log(SAM_FATAL, "Config_Load_All: DB open failed and no fallback "
                "[%s]", db_path);
        sleep(3);
        exit(FAIL);
    }

    done:
    /* record load result */
    Get_DateTime(result.loaded_at);
    strncpy(result.env_id, cfg_db_detect_env(), sizeof(result.env_id) - 1);
    result.env_id[sizeof(result.env_id) - 1] = '\0';

    Log(USR_OK, "Config_Load_All: done source=%s env=%s keys=%d",
            (result.source == CFG_SRC_DB)  ? "DB" :
            (result.source == CFG_SRC_INI) ? "INI" : "NONE",
            result.env_id, result.total_keys);

    /* save last result */
    memcpy(&g_last_result, &result, sizeof(result));

    return result;

}   /* End of Config_Load_All () */

/*************************************************************************
    Function        : . reload specific config group
    Parameters IN   : . group_id ("daemon", "proc", "tcp1", ...)
                        flag
    Parameters OUT  : .
    Return Code     : . CFG_LOAD_RESULT
    Note            : Called from px_cfgload for runtime config reload
*************************************************************************/
/*----------------------------------------------------------------------*/
CFG_LOAD_RESULT     Config_Reload_Group(const char *group_id, int flag)
/*----------------------------------------------------------------------*/
{
    CFG_LOAD_RESULT     result;
    char                db_path[256];
    int                 rc;

    memset(&result, 0, sizeof(result));

    if (!g_initialized)
        cfg_loader_detect_mode();

    Log(USR_OK, "Config_Reload_Group: group=%s flag=%d mode=%s",
            group_id, flag,
            (g_load_mode == CFG_MODE_DB)  ? "DB" :
            (g_load_mode == CFG_MODE_INI) ? "INI" : "AUTO");

    /* INI mode: use existing logic directly */
    if (g_load_mode == CFG_MODE_INI) {
        cfg_loader_load_group_ini(group_id, flag);
        result.source = CFG_SRC_INI;
        result.ini_ok = 1;
        goto done;
    }

    /* try DB load */
    {
        char    *db_env;

        db_env = getenv("_FEP_DB");
        if (db_env != NULL && db_env[0] != '\0') {
            strncpy(db_path, db_env, sizeof(db_path) - 1);
            db_path[sizeof(db_path) - 1] = '\0';
        }
        else {
            snprintf(db_path, sizeof(db_path), "%s/fep_config.db",
                    _FEP_CFG);
        }
    }

    if (cfg_db_open(db_path) == CFG_DB_OK) {
        rc = cfg_loader_load_group_db(group_id, flag);

        if (rc == CFG_DB_OK) {
            result.source = CFG_SRC_DB;
            result.db_ok = 1;

            {
                const char  *env;
                env = cfg_db_detect_env();
                result.total_keys = cfg_db_get_key_count(env, group_id);
            }

            Log(USR_OK, "Config_Reload_Group: [%s] loaded from DB",
                    group_id);
        }
        else if (g_load_mode == CFG_MODE_AUTO) {
            Log(USR_WARN, "Config_Reload_Group: [%s] DB load failed, "
                    "fallback to INI", group_id);
            cfg_loader_load_group_ini(group_id, flag);
            result.source = CFG_SRC_INI;
            result.ini_ok = 1;
        }
        else {
            Log(SAM_FATAL, "Config_Reload_Group: [%s] DB load failed, "
                    "no fallback", group_id);
        }

        cfg_db_close();
    }
    else if (g_load_mode == CFG_MODE_AUTO) {
        cfg_loader_load_group_ini(group_id, flag);
        result.source = CFG_SRC_INI;
        result.ini_ok = 1;
    }
    else {
        Log(SAM_FATAL, "Config_Reload_Group: DB open failed, no fallback");
    }

    done:
    Get_DateTime(result.loaded_at);
    strncpy(result.env_id, cfg_db_detect_env(), sizeof(result.env_id) - 1);
    result.env_id[sizeof(result.env_id) - 1] = '\0';

    memcpy(&g_last_result, &result, sizeof(result));

    return result;

}   /* End of Config_Reload_Group () */

/*************************************************************************
    Function        : . get current load mode
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . CFG_LOAD_MODE
*************************************************************************/
/*----------------------------------------------------------------------*/
CFG_LOAD_MODE       Config_Get_Mode(void)
/*----------------------------------------------------------------------*/
{
    if (!g_initialized)
        cfg_loader_detect_mode();

    return g_load_mode;

}   /* End of Config_Get_Mode () */

/*************************************************************************
    Function        : . get last load result
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . CFG_LOAD_RESULT
*************************************************************************/
/*----------------------------------------------------------------------*/
CFG_LOAD_RESULT     Config_Get_Last_Result(void)
/*----------------------------------------------------------------------*/
{
    return g_last_result;

}   /* End of Config_Get_Last_Result () */

/*************************************************************************
    End of Program (config_loader.c)
*************************************************************************/
