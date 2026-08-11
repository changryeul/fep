#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

Config g_cfg;

int load_config(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    char line[512];
    char section[64] = "";

    while (fgets(line, sizeof(line), fp)) 
    {
        if (line[0] == '#' || line[0] == '\n') continue;

        // Remove leading/trailing whitespace
        char *start = line;
        while (*start == ' ' || *start == '\t') ++start;

        if (*start == '\0' || *start == '#') continue;

        // SECTION: [NAME]
        if (*start == '[') {
            sscanf(start, "[%63[^]]", section);
            continue;
        }

        // key=value parsing
        char *key = strtok(start, "=\r\n");
        char *val = strtok(NULL, "\r\n");

        if (!key || !val) continue;

        // Trim leading spaces in val
        while (*val == ' ' || *val == '\t') ++val;

        if (strcmp(section, "OMS") == 0) {
            if (strcmp(key, "log_level") == 0) {
                g_cfg.oms_log_level = atoi(val);
            } else if (strcmp(key, "log_file") == 0) {
                strncpy(g_cfg.oms_log_file, val, sizeof(g_cfg.oms_log_file) - 1);
            } else if (strcmp(key, "cli_port") == 0) {
                g_cfg.oms_cli_port = atoi(val);
            } else if (strcmp(key, "cmd_port") == 0) {
                g_cfg.oms_cmd_port = atoi(val);
            } else if (strcmp(key, "rpl_port") == 0) {
                g_cfg.oms_rpl_port = atoi(val);
            } else if (strcmp(key, "file_name") == 0) {
                strncpy(g_cfg.oms_file_name, val, sizeof(g_cfg.oms_file_name) - 1);
            } else if (strcmp(key, "msg_key") == 0) {
			    g_cfg.oms_msg_key = (key_t)strtol(val, NULL, 0);
            }
        }
        else if (strcmp(section, "RDS_DC") == 0) {
            if (strcmp(key, "log_level") == 0) {
                g_cfg.rds_dc_log_level = atoi(val);
            } else if (strcmp(key, "log_file") == 0) {
                strncpy(g_cfg.rds_dc_log_file, val, sizeof(g_cfg.rds_dc_log_file) - 1);
            } else if (strcmp(key, "cli_port") == 0) {
                g_cfg.rds_dc_cli_port = atoi(val);
            } else if (strcmp(key, "port") == 0) {
                g_cfg.rds_dc_port = atoi(val);
            } else if (strcmp(key, "file_name") == 0) {
                strncpy(g_cfg.rds_dc_file_name, val, sizeof(g_cfg.rds_dc_file_name) - 1);
            } else if (strcmp(key, "msg_key") == 0) {
			    g_cfg.rds_dc_msg_key = (key_t)strtol(val, NULL, 0);
            }
        }
        else if (strcmp(section, "RDS_IS") == 0) {
            if (strcmp(key, "log_level") == 0) {
                g_cfg.rds_is_log_level = atoi(val);
            } else if (strcmp(key, "log_file") == 0) {
                strncpy(g_cfg.rds_is_log_file, val, sizeof(g_cfg.rds_is_log_file) - 1);
            } else if (strcmp(key, "cli_port") == 0) {
                g_cfg.rds_is_cli_port = atoi(val);
            } else if (strcmp(key, "port") == 0) {
                g_cfg.rds_is_port = atoi(val);
            } else if (strcmp(key, "file_name") == 0) {
                strncpy(g_cfg.rds_is_file_name, val, sizeof(g_cfg.rds_is_file_name) - 1);
            } else if (strcmp(key, "msg_key") == 0) {
			    g_cfg.rds_is_msg_key = (key_t)strtol(val, NULL, 0);
            }
        }
        else if (strcmp(section, "BATCH") == 0) {
            if (strcmp(key, "log_level") == 0) {
                g_cfg.bat_log_level = atoi(val);
            } else if (strcmp(key, "log_file") == 0) {
                strncpy(g_cfg.bat_log_file, val, sizeof(g_cfg.bat_log_file) - 1);
            } else if (strcmp(key, "cli_port") == 0) {
                g_cfg.bat_cli_port = atoi(val);
            } else if (strcmp(key, "cmd_port") == 0) {
                g_cfg.bat_cmd_port = atoi(val);
            } else if (strcmp(key, "rpl_port") == 0) {
                g_cfg.bat_rpl_port = atoi(val);
            } else if (strcmp(key, "file_name") == 0) {
                strncpy(g_cfg.bat_file_name, val, sizeof(g_cfg.bat_file_name) - 1);
            }
        }
        else if (strcmp(section, "RDS_R1") == 0) { // 채권종목정보 -- TRDESP50101
            if (strcmp(key, "log_level") == 0) {
                g_cfg.rds_r1_log_level = atoi(val);
            } else if (strcmp(key, "log_file") == 0) {
                strncpy(g_cfg.rds_r1_log_file, val, sizeof(g_cfg.rds_r1_log_file) - 1);
            } else if (strcmp(key, "cli_port") == 0) {
                g_cfg.rds_r1_cli_port = atoi(val);
            } else if (strcmp(key, "port") == 0) {
                g_cfg.rds_r1_port = atoi(val);
            } else if (strcmp(key, "file_name") == 0) {
                strncpy(g_cfg.rds_r1_file_name, val, sizeof(g_cfg.rds_r1_file_name) - 1);
            }
        }
        else if (strcmp(section, "RDS_R2") == 0) { // KTS종목정보 -- TRDESP50102
            if (strcmp(key, "log_level") == 0) {
                g_cfg.rds_r2_log_level = atoi(val);
            } else if (strcmp(key, "log_file") == 0) {
                strncpy(g_cfg.rds_r2_log_file, val, sizeof(g_cfg.rds_r2_log_file) - 1);
            } else if (strcmp(key, "cli_port") == 0) {
                g_cfg.rds_r2_cli_port = atoi(val);
            } else if (strcmp(key, "port") == 0) {
                g_cfg.rds_r2_port = atoi(val);
            } else if (strcmp(key, "file_name") == 0) {
                strncpy(g_cfg.rds_r2_file_name, val, sizeof(g_cfg.rds_r2_file_name) - 1);
            }
        }
        else if (strcmp(section, "RDS_R3") == 0) { //  지점 정보 일괄수신
            if (strcmp(key, "log_level") == 0) {
                g_cfg.rds_r3_log_level = atoi(val);
            } else if (strcmp(key, "log_file") == 0) {
                strncpy(g_cfg.rds_r3_log_file, val, sizeof(g_cfg.rds_r3_log_file) - 1);
            } else if (strcmp(key, "cli_port") == 0) {
                g_cfg.rds_r3_cli_port = atoi(val);
            } else if (strcmp(key, "port") == 0) {
                g_cfg.rds_r3_port = atoi(val);
            } else if (strcmp(key, "file_name") == 0) {
                strncpy(g_cfg.rds_r3_file_name, val, sizeof(g_cfg.rds_r3_file_name) - 1);
            } else if (strcmp(key, "msg_key") == 0) {
			    g_cfg.rds_r3_msg_key = (key_t)strtol(val, NULL, 0);
            }
        }
    }

    fclose(fp);
    return 0;
}
