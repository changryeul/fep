#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "comsvc.h"

int l_syslog(char *pname, const char *format, ...);
int l_getpid(char *pname,char *args1,char *args2);
int l_today(YMD *t_date);
int l_ntoi(char *n_str, int n_len);
int l_ltrim(char *instr);

int l_syslog(char *pname, const char *format, ...)
{
	time_t	today;
	struct	tm  tm, tt;

	char	log_path[128], mode[4];
	FILE	*fp;
	va_list	vl;
	struct	stat	st;
	int	rc;

	today = time(0);
	localtime_r(&today, &tm);
	memcpy(&tt, &tm, sizeof(struct tm));
	
	sprintf(log_path, "%s/%s-%d.log",
		LOG_DIR, pname, tm.tm_wday);
	rc = stat(log_path, &st);
	if (rc < 0)
		strcpy(mode, "a");
	else
	{
		localtime_r(&st.st_ctime, &tm);
		if (tm.tm_mon != tt.tm_mon || tm.tm_mday != tt.tm_mday)
			strcpy(mode, "w");
		else
			strcpy(mode, "a");
	}

	fp = fopen(log_path, mode);
	if (fp == NULL)
		return (-1);
	fprintf(fp, "[%02d/%02d %02d:%02d:%02d] ",
		tt.tm_mon+1, tt.tm_mday,
		tt.tm_hour, tt.tm_min, tt.tm_sec);

	va_start(vl, format);
	vfprintf(fp, format, vl);
	fprintf(fp, "\n");
	va_end(vl);
	fclose(fp);
	return (0);
}

/*******************************************************************************
 * NAME	: l_getpid(pname, args1, args2)
 * DESC	: 실행중인 PROCESS의 ID를 GET.
 * NOTE	: 실행중인 PROCESS ID가 CALL한 PROCESS는 제외.
 * IN	:
 * OUT	:
 * RTN	:
 * VER	: 1.00 2009-12 Winway	initial	version.
 * AUTH.:
 ******************************************************************************/
int l_getpid(char *pname, char *args1, char *args2)
{
	FILE	*chkf;
	char	*logf;
	char	command[128];
	char	line_b[128];
	char	ign_b[64], pid_b[16];
	char	arg_p[8][64];
	char	*nam_b;
	int	arg_n, cmp_i;
	int	pid, nii, ii;
	int	retc;

	sprintf(command, "Mx%d", getpid());
	logf = tempnam(P_tmpdir, command);
	if (logf == NULL)
		return (-1);
	sprintf(command, "/bin/ps -ef | /bin/grep %s | /bin/grep -v grep > %s 2>&1", pname, logf);
	retc = system(command);

	pid  = -1;
	chkf = fopen(logf, "r");
	if (chkf == NULL)
		return (pid);
	while ((fgets(line_b, sizeof(line_b), chkf)) == line_b)
	{
		pid_b[0] = '\0';
		for (ii = 0; ii < 8; ii++)
			arg_p[ii][0] = '\0';

		sscanf(line_b, "%s %s %s %s %s %s %s %s %s %s %s %s",
				ign_b, pid_b, ign_b, ign_b,
				arg_p[0], arg_p[1], arg_p[2], arg_p[3],
				arg_p[4], arg_p[5], arg_p[6], arg_p[7]);

		for (ii = 0; ii < 8; ii++)
		{
			if (strlen(arg_p[ii]))
				arg_n = ii+1;
		}

		cmp_i = arg_n - 1;
		if (args2 != NULL && strlen(args2))
		{
			if (strcmp(arg_p[cmp_i], args2) != 0)
				continue;
			cmp_i--;
		}

		if (args1 != NULL && strlen(args1))
		{
			if (strcmp(arg_p[cmp_i], args1) != 0)
				continue;
			cmp_i--;
		}

		nam_b = arg_p[cmp_i];
#ifdef	SHORT_PATH
		for (ii = 0; ii < strlen(arg_p[cmp_i]); ii++)
		{
			if (arg_p[cmp_i][ii] == '/')
				nam_b = &arg_p[cmp_i][ii+1];
		}
#endif
		if (strcmp(nam_b, pname) != 0)
			continue;

		pid = atoi(pid_b);

		/* check to see if pid value is same as the calling process's
                   pid.  If is then continue */
		if (pid == getpid())
		{
			pid = -1;
			continue;
		}
		break;
	}
	fclose(chkf);

	unlink(logf);
	(void)free(logf);

	return (pid);
}

int l_today(YMD *t_date)
{
    time_t  t_clock;
    struct  tm *tm;

    t_clock = time(0);
    tm = localtime(&t_clock);
    t_date->yy = tm->tm_year + 1900;
    t_date->mm = tm->tm_mon + 1;
    t_date->dd = tm->tm_mday;
    return(0);
}

int l_ntoi(char *n_str, int n_len)
{
    char    edit_b[128];
    int edit_l;
    int ii;

    memcpy(edit_b, n_str, n_len);
    edit_b[n_len] = '\0';
    for (ii = 0, edit_l = 0; ii < n_len; ii++)
    {
        if (edit_b[ii] == ',')
            continue;
        edit_b[edit_l++] = edit_b[ii];
    }
    edit_b[edit_l] = '\0';
    return (atoi(edit_b));
}

int l_ltrim(instr)
char    *instr;
{
	int len;
	int ii, jj;

	len = strlen(instr);
	for (ii = 0; ii < len; ii++)
	{
		if (instr[ii] != ' ')
			break;
	}

	for (jj = ii; jj < len; jj++)
		instr[jj-ii] = instr[jj];
	instr[jj-ii] = 0x00;
	return(jj-ii);
}
