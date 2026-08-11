#ifndef SIMPLE_LOG_H
#define SIMPLE_LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define LOG_MAX_SINKS 8
#define LOG_USE_COLOR 1

static inline int get_today(void)
{
    time_t t = time(NULL);

    struct tm *tm_info=localtime(&t);

    int yyyymmdd= (tm_info->tm_year+1900) * 10000  +
                  (tm_info->tm_mon+1) * 100 + tm_info->tm_mday;
    return yyyymmdd;
}

typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN  = 1,
    LOG_LEVEL_INFO  = 2,
    LOG_LEVEL_DEBUG = 3
} log_level_t;

// ===== extern 전역 (한 번만 정의)
extern FILE *g_log_sinks[LOG_MAX_SINKS];
extern int   g_log_sink_cnt;
extern int   g_log_level;

// ===== 함수 프로토타입
void  log_init_default(void);
int   log_add_file(const char *path, int line_buffered);
void  log_set_level(int level);
void  log_flush_all(void);
int   log_open_daily( char *base_dir, char *basename);
void  log_write(int level, const char *file, int line, const char *fmt, ...);

// ===== 매크로
#define LOG_ERR(fmt, ...)   log_write(LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  log_write(LOG_LEVEL_WARN,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  log_write(LOG_LEVEL_INFO,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) log_write(LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
