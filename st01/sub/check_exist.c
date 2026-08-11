/*------------------------------------------------------------------------
#   Module  : check if a module runs or not
#   File    : check_exist.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

#if defined __hpux
#include    <sys/syscall.h>
#include    <sys/pstat.h>
#elif defined sun || defined __linux
#include    <stdlib.h>
#include    <dirent.h>
#include    <limits.h>
#include    <sys/syscall.h>
#include    <sys/procfs.h>
#elif defined _AIX
#include    <procinfo.h>
#endif

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#if defined __hpux
#define     BURST   ((size_t)500)
#elif defined _AIX
#define     BURST   200
#endif

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
#if defined sun || defined __linux
int     Exist_Flag_chk;
#endif

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    Check_Exist(void);
#if defined __hpux
int     Check_Exist_HPUX(void);
#elif defined sun
int     Check_Exist_SUN(void);
int     Make_Proc_List(char *);
#elif defined _AIX
int     Check_Exist_AIX(void);
#elif defined __linux
int     Check_Exist_LINUX(void);
int     Make_Proc_List_Linux(char *);
#endif

/*************************************************************************
    Function        : . check if a module runs or not
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Check_Exist(void)
/*----------------------------------------------------------------------*/
{
#if defined __hpux
    if (Check_Exist_HPUX() == OK)
#elif defined sun
        if (Check_Exist_SUN() == OK)
#elif defined _AIX
        if (Check_Exist_AIX() == OK)
#elif defined __linux
        if (Check_Exist_LINUX() == OK)
#endif
        {
        Log(PRO_ERROR, "process already running[%s]", _Exe_Name);
        exit(FAIL);
    }

    return;
}   /* End of Check_Exist ()    */

#if defined __hpux
/*************************************************************************
    Function        : . check process - hpux
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int (0: run, -1: not run)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Check_Exist_HPUX(void)
/*----------------------------------------------------------------------*/
{
    int                 i, cnt, proc_cnt, idx, uid, run_flag;
    struct pst_status   pst[BURST];

    proc_cnt = idx = run_flag = 0;
    uid = getuid();

    while ((cnt = pstat_getproc(pst, sizeof (pst[0]), BURST, idx)) > 0) {
        for (i = 0; i < cnt; i ++) {
            if (uid != pst[i].pst_uid)
                continue;

            if (memcmp(_Exe_Name, pst[i].pst_cmd, strlen(_Exe_Name)) == 0)
                proc_cnt ++;

            if (proc_cnt > 1) {
                run_flag = 1;
                break;
            }
        }

        if (run_flag == 1)
            break;

        idx = pst[cnt-1].pst_idx + 1;
    }

    if (run_flag == 1)
        return (OK);

    return (NOTOK);
}   /* End of Check_Exist_HPUX ()   */
#endif

#if defined sun
/*************************************************************************
    Function        : . check process - sun
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int (0: run, -1: not run)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Check_Exist_SUN(void)
/*----------------------------------------------------------------------*/
{
    char            *directory="/proc/";
    DIR             *dp;
    struct dirent   *dirp;
    struct stat     statbuf;

    Exist_Flag_chk = 0;

    if (lstat(directory, &statbuf) < 0) {
        Log(SYS_ERROR, "Check_Exist_SUN:lstat {%d:%s}", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    if (S_ISDIR(statbuf.st_mode) == 0) {
        Log(SYS_ERROR, "Check_Exist_SUN:S_ISDIR {%d:%s}", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    if ((dp = opendir(directory)) == NULL) {
        Log(SYS_ERROR, "Check_Exist_SUN:opendir {%d:%s}", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    while ((dirp = readdir(dp)) != NULL) {
        if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
            continue;
        if (Make_Proc_List(dirp->d_name) == OK) {
            closedir(dp);
            return (OK);
        }
    }

    closedir(dp);

    return (NOTOK);
}   /* End of Check_Exist_SUN ()    */

/*************************************************************************
    Function        : . make process list
    Parameters IN   : . pid : process pid
    Parameters OUT  : .
    Return Code     : . int (0: run, -1: not run)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Make_Proc_List(char *pid)
/*----------------------------------------------------------------------*/
{
    int         fd;
    char        proc_path[PATH_MAX];
    prpsinfo_t  prpinfo;

    sprintf(proc_path, "/proc/%s", pid);
    if ((fd = open(proc_path, O_RDONLY)) == -1)
        return (NOTOK);

    if (ioctl(fd, PIOCPSINFO, &prpinfo) == -1) {
        close(fd);
        return (NOTOK);
    }

    if (getuid() != prpinfo.pr_uid) {
        close(fd);
        return (NOTOK);
    }

    if (memcmp(prpinfo.pr_psargs, _Exe_Name, strlen(_Exe_Name)) == 0) {
        close(fd);
        Exist_Flag_chk ++;
        return (Exist_Flag_chk > 1 ? OK : NOTOK);
    }

    close(fd);
    return (NOTOK);
}   /* End of Make_Proc_List () */
#endif

#if defined _AIX
/*************************************************************************
    Function        : . check process - aix
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int (0: run, -1: not run)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Check_Exist_AIX(void)
/*----------------------------------------------------------------------*/
{
    int                 i, cnt, proc_cnt, idx, uid, run_flag;
    pid_t               pid, next_pid;
    struct procsinfo    pi[BURST];

    proc_cnt = idx = run_flag = 0;
    uid = getuid();

    memset(pi, 0, sizeof (pi));
    next_pid = 0;

    while (1) {
        pid = next_pid;

        cnt = getprocs(pi, sizeof (pi[0]), NULL, 0, &pid, BURST);
        if (cnt <= 0)
            break;

        for (i = 0; i < cnt; i ++) {
            if (uid != pi[i].pi_uid)
                continue;

            if (memcmp(_Exe_Name, pi[i].pi_comm, strlen(_Exe_Name)) == 0)
                proc_cnt ++;

            if (proc_cnt > 1) {
                run_flag = 1;
                break;
            }
        }

        if (run_flag == 1)
            break;

        next_pid = pid;
    }

    if (run_flag == 1)
        return (OK);

    return (NOTOK);
}   /* End of Check_Exist_AIX ()    */
#endif

#if defined __linux
/*************************************************************************
    Function        : . check process - linux
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int (0: run, -1: not run)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Check_Exist_LINUX(void)
/*----------------------------------------------------------------------*/
{
    char        *directory="/proc/";
    DIR     *dp;
    struct dirent   *dirp;
    struct stat statbuf;

    Exist_Flag_chk = 0;

    if (lstat(directory, &statbuf) < 0) {
        Log(SYS_ERROR, "Check_Exist_SUN:lstat {%d:%s}", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    if (S_ISDIR(statbuf.st_mode) == 0) {
        Log(SYS_ERROR, "Check_Exist_SUN:S_ISDIR {%d:%s}", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    if ((dp = opendir(directory)) == NULL) {
        Log(SYS_ERROR, "Check_Exist_SUN:opendir {%d:%s}", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    while ((dirp = readdir(dp)) != NULL) {
        /* ascii 57(9) over, type is directory(4), ., .. skip */
        if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")
                || dirp->d_type != 4 || dirp->d_name[0] > 57)
        continue;
        if (Make_Proc_List_Linux(dirp->d_name) == OK) {
            closedir(dp);
            return (OK);
        }
    }

    closedir(dp);

    return (NOTOK);
}   /* End of Check_Exist_LINUX ()  */

/*************************************************************************
    Function        : . make process list
    Parameters IN   : . pid : process pid
    Parameters OUT  : .
    Return Code     : . int (0: run, -1: not run)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Make_Proc_List_Linux(char *pid)
/*----------------------------------------------------------------------*/
{
    FILE*           fp;
    char            buff[128], path[128];

    sprintf(path, "/proc/%s/status", pid);
    fp = fopen(path, "rt");

    if (fp) {
        if (fgets(buff, 128, fp) == NULL) {
            Log(SYS_ERROR, "fgets error return {%d:%s}", SYS_NO, SYS_STR);
        }
        fclose(fp);

        /* "process name" comparison "status file value" */
        if (strstr(buff, _Exe_Name)) {
            Exist_Flag_chk ++;
            return (Exist_Flag_chk > 1 ? OK : NOTOK);
        }
    }

    return (NOTOK);
}   /* End of Make_Proc_List_Linux ()   */
#endif

/*************************************************************************
    End of Program (check_exist.c)
*************************************************************************/
