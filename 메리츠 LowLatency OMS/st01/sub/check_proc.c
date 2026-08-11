/*------------------------------------------------------------------------
#	Module  : Check exist running process
#	File  	: check_proc.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"

#if defined __hpux
#include	<sys/syscall.h>
#include	<sys/pstat.h>
#elif defined sun  || __linux
#include	<stdlib.h>
#include	<dirent.h>
#include	<limits.h>
#include	<sys/syscall.h>
#include	<sys/procfs.h>
#elif defined _AIX
#include	<procinfo.h>
#endif

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#if defined __hpux
#define		BURST	((size_t)500)
#elif defined _AIX
#define		BURST	200
#endif

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
#if defined sun || __linux
int		Exist_Flag;
#endif

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
#if defined __hpux
pid_t	Check_Proc_HPUX (char *);
#elif defined sun
pid_t	Check_Proc_SUN (char *);
int		Make_List (char *, char *);
#elif defined _AIX
pid_t	Check_Proc_AIX (char *);
#elif defined __linux
pid_t   Check_Proc_LINUX (char *);
int     Make_List_Linux (char *, char *);
#endif

/*************************************************************************
	Function		: . check if a process runs or not
	Parameters IN	: . pname	: process name
	Parameters OUT	: .
	Return Code		: . 0:process not run, !=0:process pid
*************************************************************************/
/*----------------------------------------------------------------------*/
pid_t	Check_Proc (char *pname)
/*----------------------------------------------------------------------*/
{
	pid_t	rt;

#if defined __hpux
	rt = Check_Proc_HPUX (pname);
#elif defined sun
	rt = Check_Proc_SUN (pname);
#elif defined _AIX
	rt = Check_Proc_AIX (pname);
#elif defined __linux
    rt = Check_Proc_LINUX (pname);
#endif

	return (rt);
}	/* End of Check_Proc ()	*/

#if defined __hpux
/*************************************************************************
	Function		: . check process - hpux
	Parameters IN	: . proc_name	: process name
	Parameters OUT	: .
	Return Code		: . 0:process not run, !=0:process pid
*************************************************************************/
/*----------------------------------------------------------------------*/
pid_t	Check_Proc_HPUX (char *proc_name)
/*----------------------------------------------------------------------*/
{
	int					i, cnt, idx, uid;
	struct pst_status	pst[BURST];

	idx = 0;
	uid = getuid ();

	while ((cnt = pstat_getproc (pst, sizeof (pst[0]), BURST, idx)) > 0)
	{
		for (i = 0; i < cnt; i ++)
		{
			if (uid != pst[i].pst_uid)
				continue;

			if (memcmp (proc_name, pst[i].pst_cmd, strlen (proc_name)) == 0)
				return (pst[i].pst_pid);
		}
		idx = pst[cnt-1].pst_idx + 1;
	}

	return (0);
}	/* End of Check_Proc_HPUX ()	*/
#endif

#if defined sun
/*************************************************************************
	Function		: . check process - sun
	Parameters IN	: . proc_name	: process name
	Parameters OUT	: .
	Return Code		: . 0:process not run, !=0:process pid
*************************************************************************/
/*----------------------------------------------------------------------*/
pid_t	Check_Proc_SUN (char *proc_name)
/*----------------------------------------------------------------------*/
{
	pid_t			rt;
	char			*directory="/proc/";	
	DIR				*dp;
	struct dirent	*dirp;
	struct stat		statbuf;

	Exist_Flag = 0;

	if (lstat (directory, &statbuf) < 0)
	{
		Log (SYS_ERROR, "Check_Proc_SUN:lstat {%d:%s}", SYS_NO, SYS_STR);
		return (0);
	}

	if (S_ISDIR (statbuf.st_mode) == 0)
	{
		Log (SYS_ERROR, "Check_Proc_SUN:S_ISDIR {%d:%s}", SYS_NO, SYS_STR);
		return (0);
	}

	if ((dp = opendir (directory)) == NULL)
	{
		Log (SYS_ERROR, "Check_Proc_SUN:opendir {%d:%s}", SYS_NO, SYS_STR);
		return (0);
	}

	while ((dirp = readdir (dp)) != NULL) 
	{
		if (!strcmp (dirp->d_name, ".") || !strcmp (dirp->d_name, ".."))
			continue;
		if (Make_List (dirp->d_name, proc_name) == OK)
		{
			closedir (dp);
			rt = atoi (dirp->d_name);
			return (rt);
		}
	}
	closedir (dp);

	return (0);
}	/* End of Check_Proc_SUN ()	*/

/*************************************************************************
	Function		: . make process list
	Parameters IN	: . pid			: process pid
					  . proc_name	: process name
	Parameters OUT	: .
	Return Code		: . int (0: run, -1: not run)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Make_List (char *pid, char *proc_name)
/*----------------------------------------------------------------------*/
{
	int 		fd;
	char		proc_path[PATH_MAX];
	prpsinfo_t	prpinfo;
  
	sprintf (proc_path, "/proc/%s", pid);
	if ((fd = open (proc_path, O_RDONLY)) == -1)
		return (NOTOK);

	if (ioctl (fd, PIOCPSINFO, &prpinfo) == -1)
	{
		close (fd);
		return (NOTOK);
	}

	if (getuid () != prpinfo.pr_uid)
	{
		close (fd);
		return (NOTOK);
	}

	if (memcmp (prpinfo.pr_psargs, proc_name, strlen (proc_name)) == 0)
	{
		close (fd);
		Exist_Flag ++;
		return (Exist_Flag == 1 ? OK : NOTOK);
	}

	close (fd);
	return (NOTOK);
}	/* End of Make_List ()	*/
#endif

#if defined _AIX
/*************************************************************************
	Function		: . check process - aix
	Parameters IN	: . proc_name	: process name
	Parameters OUT	: .
	Return Code		: . 0:process not run, !=0:process pid
*************************************************************************/
/*----------------------------------------------------------------------*/
pid_t	Check_Proc_AIX (char *proc_name)
/*----------------------------------------------------------------------*/
{
	int					i, cnt, idx, uid;
	pid_t				pid, next_pid;
	struct procsinfo	pi[BURST];

	idx = 0;
	uid = getuid ();

	memset (pi, 0, sizeof (pi));
	next_pid = 0;

	while (1)
	{
		pid = next_pid;

		cnt = getprocs (pi, sizeof (pi[0]), NULL, 0, &pid, BURST);
		if (cnt <= 0)
			break;

		for (i = 0; i < cnt; i ++)
		{
			if (uid != pi[i].pi_uid)
				continue;

			if (memcmp (proc_name, pi[i].pi_comm, strlen (proc_name)) == 0)
				return (pi[i].pi_pid);
		}
		next_pid = pid;
	}

	return (0);
}	/* End of Check_Proc_AIX ()	*/
#endif

#if defined __linux
/*************************************************************************
        Function                : . check process - linux
        Parameters IN   : .
        Parameters OUT  : .
        Return Code             : . int (0: run, -1: not run)
*************************************************************************/
/*----------------------------------------------------------------------*/
pid_t             Check_Proc_LINUX (char *proc_name)
/*----------------------------------------------------------------------*/
{
    pid_t           rt;
    char            *directory="/proc/";
    DIR             *dp;
    struct dirent   *dirp;
    struct stat     statbuf;

    Exist_Flag = 0;

    if (lstat (directory, &statbuf) < 0)
    {
        Log (SYS_ERROR, "Check_Exist_LINUX:lstat {%d:%s}", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    if (S_ISDIR (statbuf.st_mode) == 0)
    {
        Log (SYS_ERROR, "Check_Exist_LINUX:S_ISDIR {%d:%s}", SYS_NO, SYS_STR);
        return (NOTOK);
    }

	if ((dp = opendir (directory)) == NULL)
    {
        Log (SYS_ERROR, "Check_Exist_LINUX:opendir {%d:%s}", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    while ((dirp = readdir (dp)) != NULL)
    {
        /* ascii 57(9) over, type is directory(4), ., .. skip */
        if (!strcmp (dirp->d_name, ".") || !strcmp (dirp->d_name, "..")
            || dirp->d_type != 4 || dirp->d_name[0] > 57)
                continue;
        if (Make_List_Linux (dirp->d_name, proc_name) == OK)
        {
            closedir (dp);
            rt = atoi (dirp->d_name);
            return (rt);
        }
    }

    closedir (dp);

    return (OK);
}       /* End of Check_Exist_LINUX ()  */

/*************************************************************************
        Function                : . make process list
        Parameters IN   : . pid : process pid
        Parameters OUT  : .
        Return Code             : . int (0: run, -1: not run)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Make_List_Linux (char *pid, char *proc_name)
/*----------------------------------------------------------------------*/
{
    FILE*           fp;
    char            buff[128], path[128];

    sprintf (path, "/proc/%s/status", pid);
    fp = fopen (path, "rt");

    if (fp)
    {
        fgets (buff, 128, fp);
        fclose(fp);

        /* "process name" comparison "status file value" */
        if (strstr (buff, proc_name))
        {
            Exist_Flag ++;
            return (Exist_Flag == 1 ? OK : NOTOK);
        }
    }
    else
    {
        Log (SYS_ERROR, "fopen err Check_Exist_LINUX:fopen {%d:%s} check_proc.c[%d] pid[%s][%s]", SYS_NO, SYS_STR, __LINE__, pid, proc_name);
    }

    return (NOTOK);
}       /* End of Make_Proc_List_Sub_p ()       */
#endif

/*************************************************************************
	End of Program (check_proc.c)
*************************************************************************/
