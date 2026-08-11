/*------------------------------------------------------------------------
#   Module  : select - test file descriptors (TCP/IP)
#   File    : tcpip_select.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*************************************************************************
    Function        : . select - test file descriptors
    Parameters IN   : . p_sfd       : file descriptor associated with the socket
                      . p_time_s    : timeout (in seconds)
                      . p_time_ms   : timeout (in microseconds)
    Parameters OUT  : .
    Return Code     : . int (0: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Select(int p_sfd, int p_time_s, int p_time_ms)
/*----------------------------------------------------------------------*/
{
    int             rt;
    fd_set          readfds, writefds, exceptfds;
    struct timeval  timeout;

    timeout.tv_sec = p_time_s;
    timeout.tv_usec = p_time_ms;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);

    FD_SET(p_sfd, &readfds);
    FD_SET(p_sfd, &writefds);
    FD_SET(p_sfd, &exceptfds);

    rt = select(p_sfd+1, (fd_set *)&readfds, (fd_set *)&writefds,
            (fd_set *)&exceptfds, &timeout);

    if (rt == -1)
        return (-1);

    return (0);
}   /* End of Select () */

/*************************************************************************
    End of Program (tcpip_select.c)
*************************************************************************/
