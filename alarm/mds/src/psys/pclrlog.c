/******************************************************************************/
/*  Components  : opsysrmv.c						      					  */
/*  Description	: Delete log and trace file (daily log file)		      	  */
/*  Rev. History: Ver	Date	Description				      				  */
/*		  		  ----	-------	----------------------------------------------*/
/*		  		  1.0	2006-08	Initial version				      			  */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/stat.h>
#include "comsvc.h"
#include "config.h"

#define	DAY_CLOCK	(24 * 60 * 60)
int	delete_dat_file();
int	delete_tmp_file();
int	delete_log_file(); 
int l_date_diff(YMD *, YMD *);

struct	proc_a {			/* procedure table		*/
	int	flow;			/* flow seq number		*/
	int	kday;			/* keeping days			*/
	int	(*proc)();		/* procedure			*/
	char	desc[64];		/* descriptions			*/
} proc_a[] = {
#if 0
	{  1,  90,  delete_dat_file,	"delete dat files"	},
#endif
	{  1,  10,  delete_tmp_file,	"delete tmp files"	},
	{  1,  15,  delete_log_file,	"delete log files"	},
	{  9,  -1,  NULL,		"EOT"			}
};

char	log_m[128];			/* log message buffer		*/
char	*myname;			/* process name			*/

/******************************************************************************/
/* NAME	: main()							      */
/* DESC	: MAIN PROCEDURE						      */
/******************************************************************************/
main(int argc, char *argv[])
{
	int	ii;
	int	retc;

	myname = basename(argv[0]);

	l_syslog(myname, "LOG delete start.");
	for (ii = 0; proc_a[ii].flow != 9; ii++)
	{
		if (!proc_a[ii].flow)
			continue;

		retc = (*proc_a[ii].proc)(proc_a[ii].kday);
		if (retc != 0)
			break;
	}

	switch (proc_a[ii].flow)
	{
	case 9 : l_syslog(myname, "LOG delete OK");
		 exit(0);
	default: l_syslog(myname, "LOG delete failed.(%d)", errno);
		 exit(1);
	}

	exit(0);
}

int backup_log_file(int date)
{
	DIR	*DIR;
	char	dirpath[128], *mmdd_b;
	char	filepath[32];

	sprintf(filepath, "%s/%08d", LOG_DIR, date);
	sprintf(dirpath, filepath);
	DIR = opendir(dirpath);
	if (DIR == NULL)
	{
		l_syslog(myname, "directory open error [%s] (%d)",
			dirpath, errno);
		return(-1);
	}
}

/******************************************************************************/
/* NAME	: delete_log_file()						      */
/* DESC	: delete the log files						      */
/******************************************************************************/
delete_log_file(int keep_day)
{
	DIR	*DIR;
	struct	dirent	*dirent;
	struct	stat	statbuf;
	char	dirpath[128], *mmdd_b;
	char	filepath[32];
	int		kday;

	time_t	tclock;
	int	retc, n_file;

	kday = get_keepday(myname, "log");
	if (kday <= 0)
		kday = keep_day;
	l_syslog(myname, "log-files delete start. keepday:%d ", kday);

	sprintf(filepath, "%s", LOG_DIR);
	sprintf(dirpath, filepath);
	DIR = opendir(dirpath);
	if (DIR == NULL)
	{
		l_syslog(myname, "directory open error [%s] (%d)",
			dirpath, errno);
		return(-1);
	}
	l_syslog(myname, "log-files delete        dirpath:%s ", dirpath);

	n_file = 0;
	tclock = time(0);

	while ((dirent = readdir(DIR)) != NULL)
	{
		if (dirent->d_name[0] == '.')
			continue;

		sprintf(dirpath, "%s/%s", LOG_DIR, dirent->d_name);
		if (stat(dirpath, &statbuf) == -1)
		{
			l_syslog(myname, "log-files ignore nostat dirpath:%s ", dirpath);
			continue;
		}
		if (!S_ISREG(statbuf.st_mode))
		{
			l_syslog(myname, "log-files ignore no reg dirpath:%s ", dirpath);
			continue;
		}

		/* file name format : XXXXXXXX.mmdd */
#if 0
		if (dirent->d_name[strlen(dirent->d_name)-5] != '.')
			continue;
		mmdd_b = &dirent->d_name[strlen(dirent->d_name)-4];
		if (atoi(mmdd_b) < 101 || atoi(mmdd_b) > 1231)
			continue;
#endif

		if (tclock - statbuf.st_mtime < (kday * DAY_CLOCK))
	//	if (tclock - statbuf.st_ctime < (kday * DAY_CLOCK))
	//		continue;
		{
			l_syslog(myname, "log-files ignore tmchck dirpath:%s ", dirpath);
		//	l_syslog(myname, "a[%ld] m[%ld] c[%ld]", statbuf.st_atime, statbuf.st_mtime, statbuf.st_ctime);
			continue;
		}
		l_syslog(myname, "%dth file:%s ", n_file, dirpath);

#ifdef	DEBUG
printf("[%s] [%s]\n", dirpath, mmdd_b);
#endif

		retc = unlink(dirpath);
		n_file++;
	}
	closedir(DIR);
	l_syslog(myname, "log-files delete ok. count=[%d]", n_file);

	return(0);
}

/******************************************************************************/
/* NAME	: delete_tmp_file()						      */
/* DESC	: delete the tmp files						      */
/******************************************************************************/
delete_tmp_file(int keep_day)
{
	DIR	*DIR;
	struct	dirent	*dirent;
	struct	stat	statbuf;
	char	dirpath[128], *mmdd_b;

	time_t	tclock;
	int	retc, n_file;
	int	kday;

	kday = get_keepday(myname, "tmp");
	if (kday <= 0)
		kday = keep_day;

	l_syslog(myname, "tmp-files delete start. keepday:%d ", kday);

	sprintf(dirpath, "%s", TMP_DIR);
	DIR = opendir(dirpath);
	if (DIR == NULL)
	{
		l_syslog(myname, "directory open error [%s] (%d)",
			dirpath, errno);
		return(-1);
	}
	l_syslog(myname, "tmp-files delete        dirpath:%s ", dirpath);

	n_file = 0;
	tclock = time(0);

	while ((dirent = readdir(DIR)) != NULL)
	{
		if (dirent->d_name[0] == '.')
			continue;

		sprintf(dirpath, "%s/%s", TMP_DIR, dirent->d_name);
		if (stat(dirpath, &statbuf) == -1)
		{
			l_syslog(myname, "tmp-files ignore nostat dirpath:%s ", dirpath);
			continue;
		}
		if (!S_ISREG(statbuf.st_mode))
		{
			l_syslog(myname, "tmp-files ignore no reg dirpath:%s ", dirpath);
			continue;
		}

		/* file name format : XXXXXXXX.mmdd */
		/*
		if (dirent->d_name[strlen(dirent->d_name)-5] != '.')
			continue;
		mmdd_b = &dirent->d_name[strlen(dirent->d_name)-4];
		if (atoi(mmdd_b) < 101 || atoi(mmdd_b) > 1231)
			continue;
		*/

		if (tclock - statbuf.st_mtime < (keep_day * DAY_CLOCK))
	//	if (tclock - statbuf.st_ctime < (keep_day * DAY_CLOCK))
	//		continue;
		{
			l_syslog(myname, "tmp-files ignore tmchck dirpath:%s ", dirpath);
		//	l_syslog(myname, "a[%ld] m[%ld] c[%ld]", statbuf.st_atime, statbuf.st_mtime, statbuf.st_ctime);
			continue;
		}
		l_syslog(myname, "%dth file:%s ", n_file, dirpath);

#ifdef	DEBUG
printf("[%s]\n", dirpath);
#endif

		retc = unlink(dirpath);
		n_file++;
	}
	closedir(DIR);
	l_syslog(myname, "tmp-files delete ok. count=[%d]", n_file);

	return(0);
}

/******************************************************************************/
/* NAME	: delete_dat_file()						      						  */
/* DESC	: delete the dat files						      					  */
/******************************************************************************/
delete_dat_file(int keep_day)
{
	DIR	*DIR;
	struct	dirent	*dirent;
	struct	stat	statbuf;
	char	dirpath[128], *mmdd_b;
	char	tmpb[32], *lptr;

	YMD		sday, eday;
	struct	tm ctm;
	time_t	tclock;
	int	retc, n_file;
	int	dval;

	l_syslog(myname, "dat-files delete start.");

	//sprintf(dirpath, "%s", DAT_DIR);
	sprintf(dirpath, "%s", TMP_DIR);
	DIR = opendir(dirpath);
	if (DIR == NULL)
	{
		l_syslog(myname, "directory open error [%s] (%d)",
			dirpath, errno);
		return(-1);
	}

	n_file = 0;
	tclock = time(0);
	localtime_r(&tclock, &ctm);
	sprintf(tmpb, "%04d%02d%02d",
			ctm.tm_year+1900, ctm.tm_mon+1, ctm.tm_mday);
	sday.yy = ctm.tm_year+1900;
	sday.mm = ctm.tm_mon+1;
	sday.dd = ctm.tm_mday;

	while ((dirent = readdir(DIR)) != NULL)
	{
		if (dirent->d_name[0] == '.')
			continue;

		//sprintf(dirpath, "%s/%s", DAT_DIR, dirent->d_name);
		sprintf(dirpath, "%s/%s", TMP_DIR, dirent->d_name);
		if (stat(dirpath, &statbuf) == -1)
			continue;
		if (!S_ISREG(statbuf.st_mode))
			continue;

		/* ISAM파일 삭제 							*/
		/* MDTICK : tick-file, MDINTR : intr-file 	*/
        if (strstr(dirent->d_name, "MDTICK") == NULL && 
			strstr(dirent->d_name, "MDINTR") == NULL )
			continue;
#if (0)
		if (tclock - statbuf.st_mtime < (keep_day * DAY_CLOCK))
			continue;
#else
		lptr = strchr(dirent->d_name, '_');
		if (lptr == NULL)
			continue;
		sprintf(tmpb, "%.8s", lptr+1);
		eday.yy = l_ntoi(&tmpb[0], 4);
		eday.mm = l_ntoi(&tmpb[4], 2);
		eday.dd = l_ntoi(&tmpb[6], 2);
		dval = l_date_diff(&sday, &eday);
		if (dval < keep_day)
			continue;
#endif

#ifdef DEBUG
printf("[%s] dval=%d\n", dirpath, dval);
#endif

		retc = unlink(dirpath);
		l_syslog(myname, "dirpath=%s dval=%d deleted.", dirpath, dval);
		n_file++;
	}
	closedir(DIR);
	l_syslog(myname, "dat-files delete ok. count=[%d]", n_file);

	return(0);
}

/******************************************************************************/
/* NAME	: delete_core()						      */
/* DESC	: delete the core file				      */
/******************************************************************************/
delete_core_file(int keep_day)
{
	char    filenm[32];
	char    command[128];
	char    *envs;

	envs = getenv("ZNET_HOME");
	sprintf(filenm, "%s/bin/core", envs);
	sprintf(command, "/usr/bin/rm -f %s > /dev/null 2>&1", filenm);
	system(command);

	return(0);
}

int l_date_diff(xday, yday)
YMD *xday, *yday;
{
    time_t  xtime, ytime;
    struct  tm  ctm;
    int ival;

    memset(&ctm, 0x00, sizeof(struct tm));
    ctm.tm_year = xday->yy - 1900;
    ctm.tm_mon  = xday->mm - 1;
    ctm.tm_mday = xday->dd;
    xtime = mktime(&ctm);

    memset(&ctm, 0x00, sizeof(struct tm));
    ctm.tm_year = yday->yy - 1900;
    ctm.tm_mon  = yday->mm - 1;
    ctm.tm_mday = yday->dd;
    ytime = mktime(&ctm);

    ival = (xtime - ytime) / DAY_CLOCK;
    if (ival < 0)
        ival *= -1;

    return(ival);
}

int get_keepday(const char *cfgname, char *name)
{
    int     ii, jj;
    int     nconf;
    char    xmlpath[256];
    char    argv[4][80];
	int		n_symb = 0;
	struct   xmltag xmltag[50];
	
    sprintf(xmlpath, "%s/%s.cfg", ETC_DIR, cfgname);
    if ((nconf = getxmlcfg(xmlpath, xmltag)) <= 0)
    {
        errno = ESRCH;
        return 0;
    }

    for (ii = 0; ii < nconf ; ii++)
    {
        if (strcmp(xmltag[ii].tags, "keep") != 0)
			continue;

		getargs(&xmltag[ii], name, argv[0]);
		if (strlen(argv[0]) > 0)
			return(atoi(argv[0]));
		else
			return(0);
    }

	return(0);
}
