/*------------------------------------------------------------------------
#   Module  : make it a daemon
#   File    : make_daemon.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*************************************************************************
    Function        : . make it a daemon
    Parameters IN   :
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Make_Daemon(void)
/*----------------------------------------------------------------------*/
{
    int     childpid;

    usleep(200000);

    /* run in background    */
    if ((childpid = fork()) < 0) {
        Log(SYS_FATAL, "Make_Daemon:fork failure[%d] {%d:%s}",
                childpid, SYS_NO, SYS_STR);
        exit(FAIL);
    }
    else if (childpid > 0)
        exit(OK);

    /* become a process group and session group leader;
        has no controlling terminal */
    setsid();

    /* never regain a controlling terminal  */
    signal(SIGHUP, SIG_IGN);
    if ((childpid = fork()) < 0) {
        Log(SYS_FATAL,
                "Make_Daemon:2nd fork failure {%d:%s}", SYS_NO, SYS_STR);
        exit(FAIL);
    }
    else if (childpid > 0)
        exit(OK);

    /* ensure that our process doesn't keep any directory in use    */
    if (chdir("/") == -1)       /* 0:OK, -1:error */ {
        Log(SAM_FATAL, "chdir 실패: {%d:%s}", SYS_NO, SYS_STR);
    }

    /* have complete control over the permissions of anything we write  */
    umask(0);

    return;
}   /* End of Make_Daemon ()    */

/*************************************************************************
    End of Program (make_daemon.c)
*************************************************************************/
