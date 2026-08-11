/*------------------------------------------------------------------------
#   Module  : send a message on a socket (TCP/IP)
#   File    : tcpip_send.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*************************************************************************
    Function        : . send a message on a socket
    Parameters IN   : . p_sfd   : file descriptor associated with the socket
                      . p_buf   : data buffer
                      . p_len   : data size
    Parameters OUT  : .
    Return Code     : . int (number of bytes sent: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Sendn(int p_sfd, char *p_buf, int p_len)
/*----------------------------------------------------------------------*/
{
    int     nleft, nsend, pt, cnt;

    cnt = nleft = p_len;
    pt = 0;

    while (nleft > 0) {
        nsend = send(p_sfd, p_buf+pt, nleft, 0);

        if (nsend <= 0)
            return (nsend);

        nleft -= nsend;
        pt += nsend;
        cnt --;

        if (cnt < 0)
            return (0);
    }

    return (p_len - nleft);
}   /* End of Sendn ()  */

/*************************************************************************
    End of Program (tcpip_send.c)
*************************************************************************/
