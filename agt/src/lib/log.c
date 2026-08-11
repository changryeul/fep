#include "log.h"

// ---- 실제 전역 변수 정의 (모든 소스 공유)
FILE *g_log_sinks[LOG_MAX_SINKS] = {0};
int   g_log_sink_cnt = 0;
int   g_log_level = LOG_LEVEL_INFO;
//static char g_log_basepath[256] = {0};
static char g_current_date[16] = {0};
static FILE *g_log_fp = NULL;
static char g_logdate[16] = "";
static char g_base_dir[256]= {0};
static char g_basename[256]= {0};

// ---- 이하 실제 구현 (기존 코드 그대로)
static inline void _log_timestamp(char *buf, size_t bufsz) {
    struct timeval tv; gettimeofday(&tv, NULL);
    struct tm tm; localtime_r(&tv.tv_sec, &tm);
    int ms = (int)(tv.tv_usec / 1000);
    strftime(buf, bufsz, "%Y-%m-%d %H:%M:%S", &tm);
    size_t n = strlen(buf);
    snprintf(buf + n, bufsz - n, ".%03d", ms);
}

const char* _log_level_name(int lvl) {
    switch (lvl) {
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_WARN:  return "WARN";
        case LOG_LEVEL_INFO:  return "INFO";
        case LOG_LEVEL_DEBUG: return "DEBUG";
        default: return "LOG";
    }
}
static void _log_current_date( char *buf, size_t bufsz)
{
    time_t t = time(NULL);
	struct tm tm;
	localtime_r (&t, &tm);
	strftime(buf, bufsz,"%Y%m%d", &tm);
}

void log_set_level(int level) { g_log_level = level; }

void log_init_default(void) {
    g_log_sink_cnt = 0;
    g_log_sinks[g_log_sink_cnt++] = stderr;
}

void log_flush_all(void) {
    for (int i = 0; i < g_log_sink_cnt; i++) {
        if (g_log_sinks[i]) fflush(g_log_sinks[i]);
    }
}

int log_add_file(const char *path, int line_buffered)
{
    g_log_fp = fopen(path, "a");
    if (!g_log_fp) { perror("log_add_file fopen"); return -1; }

    // 중복 등록 방지
    for (int i = 0; i < g_log_sink_cnt; i++) {
        if (g_log_sinks[i] && fileno(g_log_sinks[i]) == fileno(g_log_fp)) {
            fclose(g_log_fp); return 0;
        }
    }

    if (line_buffered) setvbuf(g_log_fp, NULL, _IOLBF, 0);
    if (g_log_sink_cnt >= LOG_MAX_SINKS) {
        // fprintf(stderr, "log_add_file: sink limit reached (%d)\n", LOG_MAX_SINKS);
        fclose(g_log_fp);
        return -2;
    }

    g_log_sinks[g_log_sink_cnt++] = g_log_fp;
    // fprintf(stderr, "log_add_file: added '%s' (count=%d)\n", path, g_log_sink_cnt);
    return 0;
}

static void _open_logfile_if_needed(void)
{
    char today[16];
	_log_current_date(today, sizeof( today));

	if ( !g_log_fp || strcmp( today, g_logdate) != 0)
	{
	    if(g_log_fp) fclose(g_log_fp);

        char path[256];
	    snprintf(path, sizeof(path),"%s/%s_%s.log",g_base_dir,g_basename,today);

        g_log_fp = fopen( path, "a");
		if( !g_log_fp)
		{
		    g_log_fp = stderr;
        }
	    strncpy(g_logdate, today, sizeof(g_logdate));
	}
}

	
void log_write(int level, const char *file, int line, const char *fmt, ...) {
    if (level > g_log_level) return;

    _open_logfile_if_needed();

    if (g_log_sink_cnt == 0) log_init_default();
     

    char ts[32]; _log_timestamp(ts, sizeof(ts));

    const char *fname = strrchr(file, '/');
#ifdef _WIN32
    if (!fname) fname = strrchr(file, '\\');
#endif
    fname = fname ? fname + 1 : file;

    char prefix[256];
    int prefix_len = snprintf(prefix, sizeof(prefix),
                              "%s.%s.%05d: [%s] ",
                              ts, fname, line, _log_level_name(level));

    va_list ap;
    va_start(ap, fmt);
	char msg[4096];
	vsnprintf( msg, sizeof(msg), fmt, ap);
	va_end(ap);

    for (int i = 0; i < g_log_sink_cnt; i++) {
        FILE *out = g_log_sinks[i];
        if (!out) continue;
		fprintf(out, "%s%s\n", prefix,msg);
		fflush(out);
#if 0
        fwrite(prefix, 1, prefix_len, out);
        vfprintf(out, fmt, ap);
        fputc('\n', out);
        fflush(out);
#endif
    }
    va_end(ap);
}

#if 0
static void _log_ensure_dir( char *dir)
{
    struct stat st;
	if(stat(dir,&st) == -1)
	{
	    mkdir(dir, 0755);
    }
}
#endif 
int log_open_daily( char *base_dir, char *basename)
{
    char date[16];
	_log_current_date( date, sizeof(date));

    strncpy( g_base_dir, base_dir, sizeof(g_base_dir) -1); 
    strncpy( g_basename, basename, sizeof(g_basename) -1); 

	char path[512];
	snprintf(path, sizeof(path),"%s/%s_%s.log",g_base_dir,g_basename,date);

	if(strcmp(date,g_current_date) == 0 && g_log_sink_cnt > 1)
	{
	    return 0;
    }
	strcpy( g_current_date, date);

	for( int i =1 ; i< g_log_sink_cnt;i++)
	{
	    if(g_log_sinks[i])
		{
		    fclose(g_log_sinks[i]);
			g_log_sinks[i] = NULL;
        }
    }
	g_log_sink_cnt = 1;

	g_log_fp=fopen( path, "a");
	if(!g_log_fp)
	{
	    perror("log_open_daily fopen ");
		return -1;
    }
	chmod(path,0660);
	setvbuf(g_log_fp, NULL, _IOLBF, 0);
	g_log_sinks[g_log_sink_cnt++] = g_log_fp;

	//fprintf(stderr,"[log] opened daily log:%s\n", path);
//	strncpy(g_log_basepath,basename,sizeof(g_log_basepath) -1);
	return 0;
}
