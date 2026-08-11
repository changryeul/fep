/*------------------------------------------------------------------------
#   Module  : file/directory creation helpers with error checking
#   File    : file_util.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*************************************************************************
    Function        : . Create FIFO (named pipe) with error checking
    Description     : Uses mkfifo() (POSIX) instead of deprecated mknod().
                      Tolerates EEXIST (daemon restart scenario).
                      Always ensures 0777 mode.
    Returns         : 0 on success/EEXIST, -1 on error (logged)
*************************************************************************/
int Create_FIFO(const char *path) {
    if (mkfifo(path, 0777) == -1) {
        if (errno != EEXIST) {
            Log(SYS_FATAL, "mkfifo failed [%s] errno=%d:%s",
                    path, errno, strerror(errno));
            return -1;
        }
    }
    chmod(path, 0777);
    return 0;
}

/*************************************************************************
    Function        : . Create directory with error checking
    Description     : Tolerates EEXIST (daemon restart scenario).
                      Always ensures 0777 mode.
    Returns         : 0 on success/EEXIST, -1 on error (logged)
*************************************************************************/
int Create_Dir(const char *path) {
    if (mkdir(path, 0777) == -1) {
        if (errno != EEXIST) {
            Log(SYS_FATAL, "mkdir failed [%s] errno=%d:%s",
                    path, errno, strerror(errno));
            return -1;
        }
    }
    chmod(path, 0777);
    return 0;
}
