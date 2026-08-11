#include "context.h"
#include <stdarg.h>
#include <sys/wait.h>
#include "context.h"
#include "stream.h"

//#define	LOG_WITH_WDAY	0
#define	MAX_CSVB	32


//
// CSV's format data to field 
//
int csvform(const char *buff, char csvb[MAX_CSVB][128])
{
	int	csvn = 0, flag = 0;
	int	slen = strlen(buff);
	int	csvl = 0, spcc = 0;
	int	ii;

	for (ii = 0; ii < MAX_CSVB; ii++)
		csvb[ii][0] = '\0';
	for (ii = 0; ii < slen; ii++)
	{
		switch (flag)
		{
		case 0:
			switch (buff[ii])
			{
			case '"':	// "??,???....
				flag = 1;
				break;
			case ',':
				csvb[csvn][csvl] = '\0';
				csvn++;
				csvl = 0;
				spcc = 0;
				break;
			case ' ':
			case '\t':
				if (csvl > 0)
					spcc++;
				break;
			case '\n':
			case '\r':
				break;
			default:
				if (spcc)
					csvb[csvn][csvl++] = ' ';
				csvb[csvn][csvl++] = buff[ii];
				spcc = 0;
				break;
			}
			break;
		default:
			switch (buff[ii])
			{
			case '"':
				flag = 0;
				break;
			case ' ':
			case '\t':
				if (csvl > 0)
					spcc++;
				break;
			default:
				if (spcc)
					csvb[csvn][csvl++] = ' ';
				csvb[csvn][csvl++] = buff[ii];
				spcc = 0;
				break;
			}
			break;
		}
	}
	if (csvl > 0)
	{
		csvb[csvn][csvl++] = '\0';
		csvn++;
	}
	return(csvn);
}
static	struct {
	int	mark;
	char	symb[16];
	char	exnm[2][8];
	int	(*init)(MARKET *);
	char	desc[60];
} symb2exnm[] = { 
//	{ 14, 	"01",	{ "014", "034" }, 	mds004_init, "K200 지수선물"	},
	{  0,	"",	{ "",	""     },	NULL,	  ""			}
};


static void sigchld(int signo)
{
	pid_t	pid;
	int	status;

	while (1)
	{
		if ((pid = waitpid(-1, &status, WNOHANG)) <= 0)
			break;
	}
}

//
// mds_symb()
// Return realtime symbol
//
void mds_symb4push(MARKET *market, char *real_symb, const char *symb, int delayed) 
{
	if (delayed)
		sprintf(real_symb, "%s@", symb);
	else
		sprintf(real_symb, "%s", symb);
}


//
// mds_exec()
// Eecute comands
//
int mds_exec(MARKET *market, const char *command, int wait)
{
	static	char *pargv[10];
	static	char pargs[12][128];
	char	path[128];
	int	status;
	pid_t	pid, cpid;
	int	ii, jj;

	for (ii = 0; ii < 12; ii++)
		pargs[ii][0] = '\0';

	sscanf(command, "%s %s %s %s %s %s %s %s %s", pargs[0], pargs[1], pargs[2],
		     	pargs[3], pargs[4], pargs[5], pargs[6], pargs[7], pargs[8]);

	for (ii = 0, jj = 0; ii <= 8; ii++)
	{
		if (strlen(pargs[ii]) <= 0)
			break;
		pargv[jj++] = pargs[ii];
	}
	pargv[jj] = '\0';
	if (pargs[0][0] == '/')
		sprintf(path, "%s", pargs[0]);
	else
		sprintf(path, "%s/%s", BIN_DIR, pargs[0]);
	pargv[0] = path;

	signal(SIGCHLD, SIG_DFL);
	switch ((pid = fork()))
	{
	case -1: // fork() error
		return(-1);
	case  0:// child : exec command
		execv(path, pargv);
		mds_log(market, LOG_ERROR, "Cannot execute command '%s'.", command);
		exit(-1);
	default:
		break;
	}
	if (!wait)
	{
		signal(SIGCHLD, sigchld);
		return(0);
	}

	signal(SIGCHLD, SIG_DFL);
	while (1)
	{
		if ((cpid = waitpid(pid, &status, 0)) == pid)
			break;
	}
	if (WIFEXITED(status))
		mds_log(market, LOG_MUST, "Command '%s' has been issued. return code = %d", command, WEXITSTATUS(status));
	else
		mds_log(market, LOG_MUST, "Command '%s' was terminated by signal(some error)", command);
	return(WEXITSTATUS(status));
}

void mds_lock(MARKET *market)
{
	MDCTX	*ctx = market->ctx;

	pthread_mutex_lock(&ctx->lock);
}

void mds_unlock(MARKET *market)
{
	MDCTX	*ctx = market->ctx;

	pthread_mutex_unlock(&ctx->lock);
}

const MDACCT *mds_getacct(MARKET *market)
{
	MDCTX	*ctx  = market->ctx;
	MDINFO	*info = ctx->info;

	return(&info->acct);
}

//
// msg2fld()
// Convert FIX message format to structured format
//
int mds_msg2fld(FIXFLD *fixfld, int howmany, char *msgb, int msgl, int seperator)
{
	char	*lptr, *cptr;
	int	many;

	for (lptr = msgb, many = 0; lptr < &msgb[msgl] && many < howmany-1; )
	{
		if ((cptr = strchr(lptr, '=')) == NULL)
			break;
		fixfld[many].tagn = atoi(lptr);
		lptr = cptr + 1;
		if ((cptr = strchr(lptr, '@')) == NULL)
			break;
		*cptr = '\0'; cptr++;
		fixfld[many].vptr = lptr;
		fixfld[many].dval = atof(lptr);
		fixfld[many].ival = fixfld[many].dval;
		many++;
		lptr = cptr;
	}
	fixfld[many].tagn = 0;
	return(many);
}

FIXFLD *mds_getfld(FIXFLD *fixfld, int tag_number)
{
	int	ii;

	for (ii = 0; fixfld[ii].tagn > 0; ii++)
	{
		if (fixfld[ii].tagn == tag_number)
			return(&fixfld[ii]);
	}
	return(NULL);
}

void qpricef(char *buf, const char *form, double price, double base)
{
	if (price == 0. || base == 0.) 	buf[0] = ' ';
	else if (price > base) 		buf[0] = '+';
	else if (price < base)		buf[0] = '-';
	else				buf[0] = ' ';
	sprintf(&buf[1], form, price);
}

void changef(char *buf, const char *form, int sign, double diff)
{
	buf[0] = sign | '0';
	sprintf(&buf[1], form, diff);
}

void udratef(char *buf, double rate)
{
	if (rate > 0.)		sprintf(buf, "+%.2f", rate);
	else if (rate < 0)	sprintf(buf, "%.2f", rate);
	else			sprintf(buf, "0.00");
}


void str2s(char *ts, int tl, char *fs, int fl)
{
	int	ii, go = 0;

	memset(ts, 0, tl);
	for (ii = fl-1; ii >= 0; ii--)
	{
		if (fs[ii] == ' ' && !go)
			continue;
		go = 1;
		ts[ii] = fs[ii];
	}
} 

int str2i(char *s, int l)
{
	char	t[200];

	memcpy(t, s, l);
	t[l] = '\0';
	return(atoi(t));
}

int64_t str2l(char *s, int l)
{
	char	t[200];

	memcpy(t, s, l);
	t[l] = '\0';
	return(atol(t));
}
/*
float str2f(char *s, int l)
{
	char	t[200];
	float	f;

	sprintf(t, "%.*s0", l, s);
	f = atof(t);
	return(f);
}
*/

float str2f(char *s, int l)
{
	char	t[200];
	double	d;
	char	*lptr;

	sprintf(t, "%.*s", l, s);

	if ((lptr = strchr(t, '.')) != NULL)
		sprintf(t, "%.*s0", l, s);
	else
		sprintf(t, "%.*s", l, s);
	d = atof(t);
	return(d);
}



double str2d(char *s, int l)
{
	char	t[200];
	double	d;
	char	*lptr;

	sprintf(t, "%.*s", l, s);

	if ((lptr = strchr(t, '.')) != NULL)
		sprintf(t, "%.*s0", l, s);
	else
		sprintf(t, "%.*s", l, s);
	d = atof(t);
	return(d);
}

double str2p(char *s, int l, int denominator, int with_sign)
{
	int	sign = 1;
	double	d;

	if (with_sign)
	{
		if (s[0] == '-')
			sign = -1;
		d = str2d(&s[1], l-1);
	}
	else
		d = str2d(s, l);
	d *= sign;
	d /= denominator;
	return(d);
}


//
// mds_log()
// Print formating string for debugging
//
void mds_log(MARKET *market, int level, const char *format, ...)
{
	FILE	*logF;
	time_t	clock;
	struct	tm tm;
#ifdef	LOG_WITH_WDAY
	struct	tm tx;
	struct	stat lstat;
#endif
	char	logmsg[5*1024], logpath[128], lstr[40], mode[8];
	va_list	vl;

	if (strlen(market->procname) <= 0)
		return;
	if (level > market->llog)
		return;
	clock = time(0);
	clock += market->e2lt;
	localtime_r(&clock, &tm);

	sprintf(mode, "a");
#ifdef	LOG_WITH_WDAY
	if (strlen(market->exnm) > 0)
		sprintf(logpath, "%s/%s_%s-%d.log", LOG_DIR, market->procname, market->exnm, tm.tm_wday);
	else
		sprintf(logpath, "%s/%s-%d.log", LOG_DIR, market->procname, tm.tm_wday);
	if (stat(logpath, &lstat) == 0)
	{
		clock = lstat.st_mtime + market->e2lt;
		localtime_r(&clock, &tx);
		if (tx.tm_yday != tm.tm_yday)
			strcpy(mode, "w");
	}
#else
	if (strlen(market->exnm) > 0)
		sprintf(logpath, "%s/%s_%s-%04d%02d%02d.log", LOG_DIR, market->procname, market->exnm, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);
	else
		sprintf(logpath, "%s/%s-%04d%02d%02d.log", LOG_DIR, market->procname, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);
#endif
	switch (level)
	{
	case LOG_MUST:	   strcpy(lstr, "***");	break;
	case LOG_ERROR:    strcpy(lstr, "ERR");	break;
	case LOG_WARNING:  strcpy(lstr, "WRN");	break;
	case LOG_PROGRESS: strcpy(lstr, "PRO");	break;
	case LOG_DEBUG:	   strcpy(lstr, "DBG");	break;
	default:	   strcpy(lstr, "   ");	break;
	}
	sprintf(logmsg, "%02d/%02d %02d:%02d:%02d %s %s ", 
		tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, lstr, market->procname);
	va_start(vl, format);
	vsprintf(&logmsg[strlen(logmsg)], format, vl);
	va_end(vl);


	if ((logF = fopen(logpath, mode)) != NULL)
	{

		fprintf(logF, "%s\n", logmsg);
		fclose(logF);
	}
#ifdef	DEBUG
	if (isatty(1) == 1)
		printf("%s\n", logmsg);
#endif
}

//
// mds_printf()
// Print formating string for debugging
//
void mds_printf(const char *file, unsigned int line, const char *func, const char *format, ...)
{
	char	logmsg[256];
	va_list	vl;

	if (!isatty(1))
		return;

	sprintf(logmsg, "%s #%d %s() ", file, line, func);

	va_start(vl, format);
	vsprintf(&logmsg[strlen(logmsg)], format, vl);
	va_end(vl);

	printf("%s\n", logmsg);
}

//
// mds_sleep()
//
void mds_sleep(int microseconds)
{
	struct	timeval timeval;

	timeval.tv_sec = 0;
	timeval.tv_usec = microseconds;
	select(0, NULL, NULL, NULL, &timeval);
}

#ifdef	SunOS
#define	_STRUCTURED_PROC	1
#include <sys/procfs.h>
#endif

//
// mds_procname()
// Get process name by process id
//
void mds_procname(char *procname)
{
#ifdef	SunOS
	char	 procinfo[128];
	psinfo_t psinfo;
	pid_t	pid;
	char	*base;
	int	fd, rl;

	pid = getpid();
	procname[0] = '\0';
	sprintf(procinfo, "/proc/%d/psinfo", pid);

	fd = open(procinfo, O_RDONLY);
	if (fd < 0)
		return;
	rl = read(fd, &psinfo, sizeof(psinfo_t));
	close(fd);
	if (rl >= sizeof(psinfo_t))
	{
		base = basename(psinfo.pr_fname);
		strcpy(procname, base);
	}
#endif
#ifdef	LINUX
	char	 procinfo[128];
	char	cmdline[256];
	FILE	*pFile;
	char	*base;
	pid_t	pid;

	pid = getpid();
	procname[0] = '\0';
	sprintf(procinfo, "/proc/%d/cmdline", pid);
	pFile = fopen(procinfo, "r");
	if (pFile == NULL)
		return;
	cmdline[0] = '\0';
	if (fgets(cmdline, sizeof(cmdline), pFile) == cmdline)
	{
		base = basename(cmdline);
		strcpy(procname, base);
	}
	fclose(pFile);
#endif
}


#include "mdtick.h"
static int ishms(uint32_t hms) 
{ 
    int h = HOUR(hms); 
    int m = MINUTE(hms); 
    int s = SECOND(hms); 
    if (h < 0 || h >= 24) return 0; 
    if (m < 0 || m >= 60) return 0; 
    if (s < 0 || s >= 60) return 0; 
    return  1; 
}
//
// mds_tickbegin()
//
//uint32_t mds_tickbegin(MARKET *market, char *symb, uint32_t xymd)
//{
//    int     isrc, mode;
//    int     xhms;
//    int     ntick;
//    MDTICK  tick;
//    MDCTX   *ctx = market->ctx;
// //   ISAMF   *isamf;
//
//
//    isamf = is_open(market, TICK, xymd, ISINPUT);
//    if (isamf == NULL)
//        return -1;
//
//    memset(&tick, 0x00, sizeof(tick));
//    strcpy(tick.symb, symb);
//    tick.xymd = xymd;
//    tick.xhms = 1;
//    tick.seqn = INT_MAX;
//
//    mode = ISLAST;
//    do {
//        isrc = is_read(isamf, (char *)&tick, mode);
//        if (isrc != 0)
//            return -1;
//
//        xhms = tick.xhms;
//        mode = ISPREV;
//    } while(!ishms(xhms));
//
//    return xhms;
//}
//
//
//// 
//// mds_tickend()
////
//int mds_tickend(MARKET *market, char *symb, uint32_t xymd)
//{
//    int     isrc, mode;
//    int     xhms;
//    int     ntick;
//    MDTICK  tick;
//    MDCTX   *ctx = market->ctx;
//    ISAMF   *isamf;
//
//    isamf = is_open(market, TICK, xymd, ISINPUT);
//    if (isamf == NULL)
//        return -1;
//
//    memset(&tick, 0x00, sizeof(tick));
//    strcpy(tick.symb, symb);
//    tick.xymd = xymd;
//    tick.xhms = 1;
//    tick.seqn = INT_MAX;
//
//    mode = ISFIRST;
//    do {
//        isrc = is_read(isamf, (char *)&tick, mode);
//        if (isrc != 0)
//            return -1;
//
//        xhms = tick.xhms;
//        mode = ISNEXT;
//    } while(!ishms(xhms));
//
//    is_close(isamf);
//
//    return xhms;
//}

uint32_t mds_gettymd(MARKET *market)
{
	MDINFO *info = market->info;
	return info->tymd;
}

static char	TZ[40];
/*******************************************************************************
 * NAME	: mds_tmplog()
 * DESC	: 임시진행상황 디버깅 LOG 처리(개별 일자별)
 ******************************************************************************/
int mds_tmplog(MARKET *market, char *pname, const char *format, ...)
{
	time_t	clock;
	struct	tm  *tm, tx;
	char	log_path[128];
	FILE	*fp;
	va_list	vl;
	char	*tzenv;

	clock = time(0);

	/* 2017.03.29 */
	if ((tzenv = getenv("TZ")) != NULL)
		sprintf(TZ, "TZ=%s", tzenv);
	putenv(TZ_KST); tzset();
	/*------------*/

	localtime_r(&clock, &tx);
	tm = &tx;

	sprintf(log_path, "%s/%s.%02d%02d",
			TMP_DIR, pname, tm->tm_mon+1, tm->tm_mday);

	fp = fopen(log_path, "a");
	if (fp == NULL)
		return (-1);
	fprintf(fp, "[%02d/%02d %02d:%02d:%02d] ",
		tm->tm_mon+1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	va_start(vl, format);
	vfprintf(fp, format, vl);
	fprintf(fp, "\n");
	va_end(vl);
	fclose(fp);

	/* 2017.03.29 */
	putenv(TZ); tzset();
	/*------------*/
	return (0);
}
