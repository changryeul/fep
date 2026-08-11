/*------------------------------------------------------------------------
#   Module  : receive a message from a socket (TCP/IP)
#   File    : tcpip_recv.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*************************************************************************
    Function        : . receive a message from a socket
    Parameters IN   : . p_sfd   : file descriptor associated with the socket
                      . p_len   : data size
    Parameters OUT  : . p_buf   : data buffer
    Return Code     : . int (number of bytes received: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Recvn(int p_sfd, char *p_buf, int p_len)
/*----------------------------------------------------------------------*/
{
    int     nleft, nrecv;

    nleft = p_len;

    while (nleft > 0) {
        nrecv = recv(p_sfd, p_buf, nleft, 0);
        if (nrecv < 0)
            return (nrecv);
        else if (nrecv == 0)
            break;

        nleft -= nrecv;
        p_buf += nrecv;
    }

    return (p_len - nleft);
}   /* End of Recvn ()  */

/*************************************************************************
    End of Program (tcpip_recv.c)
*************************************************************************/
