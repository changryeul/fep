/*------------------------------------------------------------------------
#   Module  : .ini to SQLite DB migration tool
#   File    : ini2db.c
#   Description : Parses FEP .ini config files and populates
#                 fep_config.db for DB-based config loading.
#                 Creates schema + imports all config groups.
#
#   Usage   : ini2db -e ENV_ID [-c CFG_DIR] [-d DB_PATH] [--dry-run]
#
#   Build   : cc -o ini2db ini2db.c -lsqlite3
------------------------------------------------------------------------*/

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <ctype.h>
#include    <sqlite3.h>

/*------------------------------------------------------------------------
    Constants
------------------------------------------------------------------------*/
#define     MAX_LINE    1024
#define     MAX_KEY     100
#define     MAX_VAL     512
#define     MAX_GROUPS  10

/*------------------------------------------------------------------------
    Schema SQL
------------------------------------------------------------------------*/
static const char *SCHEMA_SQL =
"PRAGMA journal_mode=WAL;\n"
"PRAGMA foreign_keys=ON;\n"
"\n"
"CREATE TABLE IF NOT EXISTS fep_environment(\n"
"    env_id      TEXT PRIMARY KEY,\n"
"    hostname    TEXT NOT NULL,\n"
"    description TEXT,\n"
"    is_active   INTEGER DEFAULT 1,\n"
"    created_at  TEXT DEFAULT(datetime('now','localtime'))\n"
");\n"
"\n"
"CREATE TABLE IF NOT EXISTS fep_config_group(\n"
"    group_id    TEXT PRIMARY KEY,\n"
"    ini_file    TEXT NOT NULL,\n"
"    description TEXT,\n"
"    load_order  INTEGER NOT NULL\n"
");\n"
"\n"
"CREATE TABLE IF NOT EXISTS fep_config(\n"
"    config_id   INTEGER PRIMARY KEY AUTOINCREMENT,\n"
"    env_id      TEXT NOT NULL,\n"
"    group_id    TEXT NOT NULL,\n"
"    section     TEXT NOT NULL,\n"
"    key_name    TEXT NOT NULL,\n"
"    key_value   TEXT,\n"
"    data_type   TEXT DEFAULT 'STRING',\n"
"    sort_order  INTEGER DEFAULT 0,\n"
"    is_encrypted INTEGER DEFAULT 0,\n"
"    description TEXT,\n"
"    updated_at  TEXT DEFAULT(datetime('now','localtime')),\n"
"    updated_by  TEXT DEFAULT 'system',\n"
"    UNIQUE(env_id, group_id, section, key_name)\n"
");\n"
"\n"
"CREATE INDEX IF NOT EXISTS idx_config_env_group\n"
"    ON fep_config(env_id, group_id);\n"
"CREATE INDEX IF NOT EXISTS idx_config_section\n"
"    ON fep_config(section);\n"
"\n"
"CREATE TABLE IF NOT EXISTS fep_config_history(\n"
"    history_id    INTEGER PRIMARY KEY AUTOINCREMENT,\n"
"    config_id     INTEGER NOT NULL,\n"
"    env_id        TEXT NOT NULL,\n"
"    group_id      TEXT NOT NULL,\n"
"    section       TEXT NOT NULL,\n"
"    key_name      TEXT NOT NULL,\n"
"    old_value     TEXT,\n"
"    new_value     TEXT,\n"
"    changed_at    TEXT DEFAULT(datetime('now','localtime')),\n"
"    changed_by    TEXT DEFAULT 'system',\n"
"    change_reason TEXT\n"
");\n"
"\n"
"CREATE INDEX IF NOT EXISTS idx_history_config\n"
"    ON fep_config_history(config_id);\n"
"CREATE INDEX IF NOT EXISTS idx_history_time\n"
"    ON fep_config_history(changed_at);\n"
"\n"
"CREATE TRIGGER IF NOT EXISTS trg_config_update\n"
"AFTER UPDATE OF key_value ON fep_config\n"
"WHEN OLD.key_value != NEW.key_value\n"
"BEGIN\n"
"    INSERT INTO fep_config_history\n"
"        (config_id, env_id, group_id, section, key_name,\n"
"         old_value, new_value, changed_by)\n"
"    VALUES\n"
"        (NEW.config_id, NEW.env_id, NEW.group_id, NEW.section,\n"
"         NEW.key_name, OLD.key_value, NEW.key_value, NEW.updated_by);\n"
"END;\n"
"\n"
"CREATE TABLE IF NOT EXISTS fep_ip_whitelist(\n"
"    ip_id       INTEGER PRIMARY KEY AUTOINCREMENT,\n"
"    env_id      TEXT NOT NULL,\n"
"    ip_address  TEXT NOT NULL,\n"
"    ip_type     TEXT NOT NULL CHECK(ip_type IN('CLIENT', 'ADMIN')),\n"
"    description TEXT,\n"
"    is_active   INTEGER DEFAULT 1,\n"
"    updated_at  TEXT DEFAULT(datetime('now','localtime'))\n"
");\n"
"\n"
"CREATE INDEX IF NOT EXISTS idx_whitelist_env\n"
"    ON fep_ip_whitelist(env_id, ip_type);\n"
"\n"
"CREATE TABLE IF NOT EXISTS fep_db_meta(\n"
"    key_name    TEXT PRIMARY KEY,\n"
"    key_value   TEXT,\n"
"    updated_at  TEXT DEFAULT(datetime('now','localtime'))\n"
");\n";

/*------------------------------------------------------------------------
    Seed data SQL
------------------------------------------------------------------------*/
static const char *SEED_SQL =
"INSERT OR IGNORE INTO fep_environment VALUES\n"
"    ('TEST',  'at05',   'Test Server',  1, datetime('now','localtime'));\n"
"INSERT OR IGNORE INTO fep_environment VALUES\n"
"    ('REAL1', 'podm11', 'Production 1', 1, datetime('now','localtime'));\n"
"INSERT OR IGNORE INTO fep_environment VALUES\n"
"    ('REAL2', 'podm12', 'Production 2', 1, datetime('now','localtime'));\n"
"\n"
"INSERT OR IGNORE INTO fep_config_group VALUES\n"
"    ('daemon',  'daemon.ini',  'Daemon process',    1);\n"
"INSERT OR IGNORE INTO fep_config_group VALUES\n"
"    ('file',    'file.ini',    'SAM files',         2);\n"
"INSERT OR IGNORE INTO fep_config_group VALUES\n"
"    ('dshm',    'dshm.ini',    'Data SHM',          3);\n"
"INSERT OR IGNORE INTO fep_config_group VALUES\n"
"    ('tcp1',    'tcp1.ini',    'TCP port daemon',   4);\n"
"INSERT OR IGNORE INTO fep_config_group VALUES\n"
"    ('tcp2',    'tcp2.ini',    'TCP direct connect', 5);\n"
"INSERT OR IGNORE INTO fep_config_group VALUES\n"
"    ('udpip',   'udpip.ini',   'UDP multicast',     6);\n"
"INSERT OR IGNORE INTO fep_config_group VALUES\n"
"    ('sisetr',  'sisetr.ini',  'Sise TR codes',     7);\n"
"INSERT OR IGNORE INTO fep_config_group VALUES\n"
"    ('proc',    'proc.ini',    'Process config',    8);\n"
"\n"
"INSERT OR REPLACE INTO fep_db_meta VALUES\n"
"    ('schema_version', '1.0', datetime('now','localtime'));\n"
"INSERT OR REPLACE INTO fep_db_meta VALUES\n"
"    ('created_at', datetime('now','localtime'),\n"
"     datetime('now','localtime'));\n";

/*------------------------------------------------------------------------
    Group definitions
------------------------------------------------------------------------*/
typedef struct {
    const char  *group_id;
    const char  *ini_file;
    const char  *end_marker;    /* e.g. "Daemon_End", "Tcp2_End" */
    int         is_daemon;      /* 1 for daemon.ini (special parsing) */
} GROUP_DEF;

static GROUP_DEF g_groups[] = {
    { "daemon",  "daemon.ini",  "Daemon_End",  1 },
    { "file",    "file.ini",    "File_End",    0 },
    { "dshm",    "dshm.ini",    "Dshm_End",    0 },
    { "tcp1",    "tcp1.ini",    "Tcp1_End",    0 },
    { "tcp2",    "tcp2.ini",    "Tcp2_End",    0 },
    { "udpip",   "udpip.ini",   "Udpip_End",   0 },
    { "sisetr",  "sisetr.ini",  "Sise_End",    0 },
    { "proc",    "proc.ini",    "Proc_End",    0 },
    { NULL, NULL, NULL, 0 }
};

/*------------------------------------------------------------------------
    Global state
------------------------------------------------------------------------*/
static sqlite3      *g_db = NULL;
static int          g_dry_run = 0;
static int          g_total_keys = 0;
static int          g_total_ips = 0;

/*------------------------------------------------------------------------
    Utility: uppercase string in-place
------------------------------------------------------------------------*/
static void str_upper(char *s) {
    while (*s) {
        *s = toupper ((unsigned char) *s);
        s++;
    }
}

/*------------------------------------------------------------------------
    Utility: trim trailing whitespace/newline
------------------------------------------------------------------------*/
static void str_trim(char *s) {
    int len = strlen(s);

    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' ||
            s[len-1] == ' ' || s[len-1] == '\t')) {
        s[--len] = '\0';
    }
}

/*------------------------------------------------------------------------
    Insert a config key into fep_config
------------------------------------------------------------------------*/
static int insert_config(const char *env_id, const char *group_id,
        const char *section, const char *key_name, const char *key_value,
        int sort_order) {
    sqlite3_stmt    *stmt = NULL;
    int             rc;
    static const char *sql =
    "INSERT OR REPLACE INTO fep_config "
    "(env_id, group_id, section, key_name, key_value, sort_order) "
    "VALUES(?, ?, ?, ?, ?, ?)";

    if (g_dry_run) {
        printf("  [%s] %s.%s.%s = %s(order=%d)\n",
                env_id, group_id, section, key_name, key_value, sort_order);
        g_total_keys++;
        return 0;
    }

    rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, group_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, section, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, key_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, key_value, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, sort_order);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
        return -1;

    g_total_keys++;
    return 0;
}

/*------------------------------------------------------------------------
    Insert an IP into fep_ip_whitelist
------------------------------------------------------------------------*/
static int insert_ip(const char *env_id, const char *ip_addr,
        const char *ip_type) {
    sqlite3_stmt    *stmt = NULL;
    int             rc;
    static const char *sql =
    "INSERT OR IGNORE INTO fep_ip_whitelist "
    "(env_id, ip_address, ip_type) VALUES(?, ?, ?)";

    if (g_dry_run) {
        printf("  [IP:%s] %s = %s\n", ip_type, ip_addr, env_id);
        g_total_ips++;
        return 0;
    }

    rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, ip_addr, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, ip_type, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
        return -1;

    g_total_ips++;
    return 0;
}

/*------------------------------------------------------------------------
    Parse daemon.ini (special: DAEMON_{X}_ prefix stripping)
    section = daemon letter (A, B, ...)
    key_name = field name without DAEMON_X_ prefix (uppercase)
------------------------------------------------------------------------*/
static int parse_daemon_ini(const char *path, const char *env_id) {
    FILE    *fp;
    char    buf[MAX_LINE], key_up[MAX_KEY], val[MAX_VAL];
    char    *eq;
    char    daemon_letter;
    int     sort_order = 0, count = 0;
    int     in_block = 0;
    char    section[4];

    fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "WARNING: cannot open %s\n", path);
        return 0;
    }

    printf("  Parsing daemon.ini ...\n");

    while (fgets(buf, sizeof (buf), fp) != NULL) {
        str_trim(buf);

        /* skip comments and blanks */
        if (buf[0] == '#' || buf[0] == '\0' ||
                buf[0] == '\t' || buf[0] == ' ')
        continue;

        /* DAEMON_CONF_START / DAEMON_CONF_END */
        if (strcmp(buf, "DAEMON_CONF_START") == 0) {
            in_block = 1;
            continue;
        }
        if (strcmp(buf, "DAEMON_CONF_END") == 0)
            break;

        /* Daemon_End marker */
        if (strcmp(buf, "Daemon_End") == 0) {
            sort_order = 0;
            continue;
        }

        if (!in_block)
            continue;

        /* parse key=value */
        eq = strchr(buf, '=');
        if (eq == NULL)
            continue;

        /* extract key (uppercase) */
        memset(key_up, 0, sizeof (key_up));
        memcpy(key_up, buf, eq - buf);
        str_upper(key_up);

        /* extract value (after '=', keep original case) */
        memset(val, 0, sizeof (val));
        strncpy(val, eq + 1, sizeof (val) - 1);

        /* key format: DAEMON_{X}_{FIELD} → extract X and FIELD */
        if (strncmp(key_up, "DAEMON_", 7) != 0)
            continue;

        daemon_letter = key_up[7];
        if (daemon_letter < 'A' || daemon_letter > 'Z')
            continue;

        if (key_up[8] != '_')
            continue;

        /* section = daemon letter, key_name = field after DAEMON_X_ */
        section[0] = daemon_letter;
        section[1] = '\0';

        insert_config(env_id, "daemon", section,
                &key_up[9], val, sort_order);

        sort_order++;
        count++;
    }

    fclose(fp);
    printf("    -> %d keys imported\n", count);
    return count;
}

/*------------------------------------------------------------------------
    Parse generic .ini file (proc, tcp2, tcp1, udpip, file, dshm, sisetr)
    Uses XX_CONF_START to detect daemon letter.
    Keys are stored uppercase as-is.
------------------------------------------------------------------------*/
static int parse_generic_ini(const char *path, const char *env_id,
        const char *group_id, const char *end_marker) {
    FILE    *fp;
    char    buf[MAX_LINE], key_up[MAX_KEY], val[MAX_VAL];
    char    *eq, *sp;
    char    section[4];
    int     sort_order = 0, count = 0;
    int     in_section = 0;
    char    daemon_letter = '\0';

    fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "WARNING: cannot open %s\n", path);
        return 0;
    }

    printf("  Parsing %s ...\n", group_id);

    section[0] = '\0';

    while (fgets(buf, sizeof (buf), fp) != NULL) {
        str_trim(buf);

        /* skip comments and blanks */
        if (buf[0] == '#' || buf[0] == '\0' ||
                buf[0] == '\t' || buf[0] == ' ')
        continue;

        /* check XX_CONF_START (e.g. PB_CONF_START) */
        if (strstr(buf, "_CONF_START") != NULL) {
            /* format: {sys}{daemon}_CONF_START → daemon = buf[1] */
            if (strlen(buf) >= 2 && buf[2] == '_') {
                daemon_letter = toupper((unsigned char) buf[1]);
                section[0] = daemon_letter;
                section[1] = '\0';
                in_section = 1;
                sort_order = 0;
            }
            continue;
        }

        /* check XX_CONF_END */
        if (strstr(buf, "_CONF_END") != NULL) {
            in_section = 0;
            continue;
        }

        /* check end marker (e.g. Tcp2_End, Proc_End) */
        if (strcmp(buf, end_marker) == 0) {
            /* reset sort_order for next item within same daemon */
            continue;
        }

        if (!in_section || daemon_letter == '\0')
            continue;

        /* parse key=value */
        eq = strchr(buf, '=');
        if (eq == NULL)
            continue;

        /* extract key (uppercase) */
        memset(key_up, 0, sizeof (key_up));
        memcpy(key_up, buf, eq - buf);
        str_upper(key_up);

        /* extract value: take everything after '='
           but stop at first space/tab (trim trailing comment) */
        memset(val, 0, sizeof (val));
        sp = eq + 1;

        /* for COMMENT fields, take entire rest of line */
        if (strstr(key_up, "COMMENT") != NULL) {
            strncpy(val, sp, sizeof (val) - 1);
        }
        else {
            /* take value up to first space/tab */
            {
                int i = 0;
                while (*sp && *sp != ' ' && *sp != '\t' &&
                        i < (int)sizeof (val) - 1) {
                    val[i++] = *sp++;
                }
                val[i] = '\0';
            }
        }

        insert_config(env_id, group_id, section,
                key_up, val, sort_order);

        sort_order++;
        count++;
    }

    fclose(fp);
    printf("    -> %d keys imported\n", count);
    return count;
}

/*------------------------------------------------------------------------
    Parse IP whitelist file (client.ini / pc.ini)
    Each non-comment, non-blank line is an IP address.
------------------------------------------------------------------------*/
static int parse_ip_file(const char *path, const char *env_id,
        const char *ip_type) {
    FILE    *fp;
    char    buf[MAX_LINE];
    int     count = 0;

    fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "WARNING: cannot open %s\n", path);
        return 0;
    }

    printf("  Parsing %s IPs ...\n", ip_type);

    while (fgets(buf, sizeof (buf), fp) != NULL) {
        str_trim(buf);

        /* skip comments, blanks, header lines */
        if (buf[0] == '#' || buf[0] == '\0' ||
                buf[0] == '\t' || buf[0] == ' ')
        continue;

        /* basic IP format check: starts with digit */
        if (!isdigit((unsigned char) buf[0]))
            continue;

        insert_ip(env_id, buf, ip_type);
        count++;
    }

    fclose(fp);
    printf("    -> %d IPs imported\n", count);
    return count;
}

/*------------------------------------------------------------------------
    Initialize DB schema
------------------------------------------------------------------------*/
static int init_schema(void) {
    char    *errmsg = NULL;
    int     rc;

    rc = sqlite3_exec(g_db, SCHEMA_SQL, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "ERROR: schema creation failed: %s\n", errmsg);
        sqlite3_free(errmsg);
        return -1;
    }

    rc = sqlite3_exec(g_db, SEED_SQL, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "ERROR: seed data failed: %s\n", errmsg);
        sqlite3_free(errmsg);
        return -1;
    }

    return 0;
}

/*------------------------------------------------------------------------
    Print usage
------------------------------------------------------------------------*/
static void usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s -e ENV_ID [-c CFG_DIR] [-d DB_PATH] [--dry-run]\n"
            "\n"
            "  -e ENV_ID    Environment ID(TEST, REAL1, REAL2)\n"
            "  -c CFG_DIR   Config directory(default: $_FEP_CFG or ./cfg)\n"
            "  -d DB_PATH   SQLite DB path(default: CFG_DIR/fep_config.db)\n"
            "  --dry-run    Parse only, no DB writes\n"
            "\n"
            "Examples:\n"
            "  %s -e TEST\n"
            "  %s -e TEST -c /home/fep/st01/cfg\n"
            "  %s -e REAL1 -d /home/fep/st01/cfg/fep_config.db\n"
            "  %s -e TEST --dry-run\n",
            prog, prog, prog, prog, prog);
}

/*------------------------------------------------------------------------
    Main
------------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    char    env_id[20];
    char    cfg_dir[256];
    char    db_path[256];
    char    ini_path[512];
    char    *env_val;
    int     i, rc;
    int     has_env = 0, has_cfg = 0, has_db = 0;

    memset(env_id, 0, sizeof (env_id));
    memset(cfg_dir, 0, sizeof (cfg_dir));
    memset(db_path, 0, sizeof (db_path));

    /*-- parse arguments --*/
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            strncpy(env_id, argv[++i], sizeof (env_id) - 1);
            str_upper(env_id);
            has_env = 1;
        }
        else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            strncpy(cfg_dir, argv[++i], sizeof (cfg_dir) - 1);
            has_cfg = 1;
        }
        else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            strncpy(db_path, argv[++i], sizeof (db_path) - 1);
            has_db = 1;
        }
        else if (strcmp(argv[i], "--dry-run") == 0) {
            g_dry_run = 1;
        }
        else {
            usage(argv[0]);
            return 1;
        }
    }

    if (!has_env) {
        fprintf(stderr, "ERROR: -e ENV_ID is required\n\n");
        usage(argv[0]);
        return 1;
    }

    /*-- determine config directory --*/
    if (!has_cfg) {
        env_val = getenv("_FEP_CFG");
        if (env_val != NULL)
            strncpy(cfg_dir, env_val, sizeof (cfg_dir) - 1);
        else
            strncpy(cfg_dir, "./cfg", sizeof (cfg_dir) - 1);
    }

    /*-- determine DB path --*/
    if (!has_db)
        snprintf(db_path, sizeof (db_path),
                "%s/fep_config.db", cfg_dir);

    printf("================================================\n");
    printf("  ini2db - FEP Config Migration Tool\n");
    printf("================================================\n");
    printf("  Environment : %s\n", env_id);
    printf("  Config Dir  : %s\n", cfg_dir);
    printf("  DB Path     : %s\n", db_path);
    printf("  Dry Run     : %s\n", g_dry_run ? "YES" : "NO");
    printf("================================================\n\n");

    /*-- open/create DB --*/
    if (!g_dry_run) {
        rc = sqlite3_open_v2(db_path, &g_db,
                SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "ERROR: cannot open DB [%s]: %s\n",
                    db_path, sqlite3_errmsg(g_db));
            return 1;
        }

        /*-- initialize schema --*/
        if (init_schema() != 0) {
            sqlite3_close(g_db);
            return 1;
        }

        /*-- begin transaction for performance --*/
        sqlite3_exec(g_db, "BEGIN TRANSACTION", NULL, NULL, NULL);
    }

    /*-- delete existing data for this environment --*/
    if (!g_dry_run) {
        char    del_sql[256];

        snprintf(del_sql, sizeof (del_sql),
                "DELETE FROM fep_config WHERE env_id='%s'", env_id);
        sqlite3_exec(g_db, del_sql, NULL, NULL, NULL);

        snprintf(del_sql, sizeof (del_sql),
                "DELETE FROM fep_ip_whitelist WHERE env_id='%s'", env_id);
        sqlite3_exec(g_db, del_sql, NULL, NULL, NULL);
    }

    /*-- parse each config group --*/
    for (i = 0; g_groups[i].group_id != NULL; i++) {
        snprintf(ini_path, sizeof (ini_path),
                "%s/%s", cfg_dir, g_groups[i].ini_file);

        if (g_groups[i].is_daemon)
            parse_daemon_ini(ini_path, env_id);
        else
            parse_generic_ini(ini_path, env_id,
                    g_groups[i].group_id, g_groups[i].end_marker);
    }

    /*-- parse IP whitelist files --*/
    snprintf(ini_path, sizeof (ini_path), "%s/client.ini", cfg_dir);
    parse_ip_file(ini_path, env_id, "CLIENT");

    snprintf(ini_path, sizeof (ini_path), "%s/pc.ini", cfg_dir);
    parse_ip_file(ini_path, env_id, "ADMIN");

    /*-- commit transaction --*/
    if (!g_dry_run) {
        sqlite3_exec(g_db, "COMMIT", NULL, NULL, NULL);

        /* update meta */
        {
            char    meta_sql[256];

            snprintf(meta_sql, sizeof (meta_sql),
                    "INSERT OR REPLACE INTO fep_db_meta VALUES "
                    "('last_imported_at', datetime('now','localtime'), "
                    "datetime('now','localtime'))");
            sqlite3_exec(g_db, meta_sql, NULL, NULL, NULL);

            snprintf(meta_sql, sizeof (meta_sql),
                    "INSERT OR REPLACE INTO fep_db_meta VALUES "
                    "('last_imported_env', '%s', "
                    "datetime('now','localtime'))", env_id);
            sqlite3_exec(g_db, meta_sql, NULL, NULL, NULL);
        }

        sqlite3_close(g_db);
    }

    /*-- summary --*/
    printf("\n================================================\n");
    printf("  Migration Complete\n");
    printf("================================================\n");
    printf("  Config keys : %d\n", g_total_keys);
    printf("  IP entries  : %d\n", g_total_ips);
    printf("  Total       : %d\n", g_total_keys + g_total_ips);
    printf("================================================\n");

    return 0;
}

/*************************************************************************
    End of Program (ini2db.c)
*************************************************************************/
