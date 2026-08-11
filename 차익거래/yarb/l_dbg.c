
/* include files */
#include <pthread.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include "def.h"

int   l_mtxflg = 0;
int   l_loglvl = 0xff;
FILE *l_logfd  = NULL;
char  l_dbgfile[255];
pthread_mutex_t l_mtx_dbg=PTHREAD_MUTEX_INITIALIZER;

char *l_tmisec(char *t_buff, int t_len)
{
	struct timeval tt;
	char tbuf[40];

	memset(t_buff, 0x00, t_len);

	gettimeofday(&tt, NULL);
	sprintf(tbuf, ":%06ld", tt.tv_usec);
	strftime(t_buff, 32, "%Y%m%d-%T", localtime(&tt.tv_sec));
	strcat(t_buff, tbuf);
	return(t_buff);
}
	
void l_dbg_close() 
{
	if (l_logfd) fclose(l_logfd);
	l_logfd = NULL;
}

void __l_dbg(const char *func, int line, int loglvl, const char *fmt,...)
{
	va_list ap;
	int		ret;
	char 	time_buf[40];
	struct	stat fstat;

	if (!l_mtxflg) {
		l_mtxflg = 1;
	}

	pthread_mutex_lock(&l_mtx_dbg);
	ret = stat(l_dbgfile, &fstat);
	if (ret < 0) {
		if (l_logfd) fclose(l_logfd);
		l_logfd = NULL;
	}

	if (!l_logfd) 
		l_logfd = fopen(l_dbgfile, "a+");
	if (!l_logfd) {
		pthread_mutex_unlock(&l_mtx_dbg);
		return;
	}

	if (loglvl > l_loglvl) {
		pthread_mutex_unlock(&l_mtx_dbg);
		return;
	}

	fprintf(l_logfd, "[%s][%-17s:%03d][%ld] ", l_tmisec(time_buf, 32), func, line, syscall(SYS_gettid));
	va_start(ap, fmt);
	vfprintf(l_logfd, fmt, ap);
	fprintf(l_logfd, "\n");
	fflush(l_logfd);
	va_end(ap);

	pthread_mutex_unlock(&l_mtx_dbg);
	return;
}

void l_hex(char *title, char *msgbuf, int msglen)
{
	int  iptr = 0;
	int  len = 0, i, ret;
	char time_buf[128];
	
	char ch;
	char *bptr, *bp;
	struct stat fstat;

	pthread_mutex_lock(&l_mtx_dbg);
	ret = stat(l_dbgfile, &fstat);
	if (ret < 0) {
		if (l_logfd) fclose(l_logfd);
		l_logfd = NULL;
	}

	if (!l_logfd) l_logfd = fopen(l_dbgfile, "a+");
	if (!l_logfd) {
		pthread_mutex_unlock(&l_mtx_dbg);
		return;
	}

	if (l_loglvl != 0xff) {
		pthread_mutex_unlock(&l_mtx_dbg);
		return;
	}

	fprintf(l_logfd, " >>> hexdump(%s): time[%s] len[%d]\n",
			title, l_tmisec(time_buf, 32), msglen);
	fprintf(l_logfd, "--------------------------------------------------------\n");

	bptr = bp = msgbuf;

	for (; msglen > 0; msglen -= len) {
		len = L_MIN(msglen, DBGLINESZ);
		fprintf(l_logfd, "0x%04X: ", iptr);
		iptr += DBGLINESZ;

		for (i=0; i < len; i++) { 
			if (i == 7) {
				fprintf(l_logfd, "%02x ", *bptr++&0xff);
				fprintf(l_logfd, "- ");
			} else {
				fprintf(l_logfd, "%02x ", *bptr++&0xff);
			}
		}

		if (len < 7) fprintf(l_logfd, "  ");

		for (i = len; i < DBGLINESZ; i++)
			fprintf(l_logfd, "  ");

		fprintf(l_logfd, "    ");

		for (i = 0; i < len; i++) {
			ch = *bp++&0xff;
			fprintf(l_logfd, "%c", ((ch < CHR_SPACE || ch > CHR_TILDE) ? CHR_DOT : ch));
		}

		fprintf(l_logfd, "\n");
	}

	fprintf(l_logfd, "--------------------------------------------------------\n");
	fflush(l_logfd);
	pthread_mutex_unlock(&l_mtx_dbg);
	return;
}

void l_loglvl_set(int log_level)
{

	if (log_level == 0)
	{
		l_loglvl = L_ERR;
	}
	else if (log_level == 1)
	{
		l_loglvl = L_WAR;
	}
	else if (log_level == 2)
	{
		l_loglvl = L_INF;
	}
	else if (log_level == 3)
	{
		l_loglvl = L_DBG;
	}
	return;
}



