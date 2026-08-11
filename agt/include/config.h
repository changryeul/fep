#ifndef CONFIG_H
#define CONFIG_H

#include <sys/types.h>
#include <sys/ipc.h>

typedef struct {

    // OMS
    int  oms_log_level;
    char oms_log_file[256];
    int  oms_cli_port;
    int  oms_cmd_port;
    int  oms_rpl_port;
    char oms_file_name[256];
    key_t oms_msg_key;	

    // RDS_DR
    int  rds_dc_log_level;
    char rds_dc_log_file[256];
    int  rds_dc_cli_port;
    int  rds_dc_port;
    char rds_dc_file_name[256];
    key_t rds_dc_msg_key;	
    // RDS_IS
    int  rds_is_log_level;
    char rds_is_log_file[256];
    int  rds_is_cli_port;
    int  rds_is_port;
    char rds_is_file_name[256];
    key_t rds_is_msg_key;	

    // RD_R1 채권 종목정보 
    int  rds_r1_log_level;
    char rds_r1_log_file[256];
    int  rds_r1_cli_port;
    int  rds_r1_port;
    char rds_r1_file_name[256];

    // RD_R2 KTS 종목정보 
    int  rds_r2_log_level;
    char rds_r2_log_file[256];
    int  rds_r2_cli_port;
    int  rds_r2_port;
    char rds_r2_file_name[256];

    // RD_R3  지점정보 일괄수신
    int  rds_r3_log_level;
    char rds_r3_log_file[256];
    int  rds_r3_cli_port;
    int  rds_r3_port;
    char rds_r3_file_name[256];
    key_t rds_r3_msg_key;	

    // BATCH
    int  bat_log_level;
    char bat_log_file[256];
    int  bat_cli_port;
    int  bat_cmd_port;
    int  bat_rpl_port;
    char bat_file_name[256];
} Config;

int load_config(const char *path);

#endif // CONFIG_H
