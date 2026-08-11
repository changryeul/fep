/*------------------------------------------------------------------------
#   Module  : bind a name to a socket (TCP/IP)
#   File    : tcpip_bind.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*************************************************************************
    Function        : . bind a name to a socket
    Parameters IN   : . p_sfd       : file descriptor of the socket to be bound
                      . p_port_no   : port number
    Parameters OUT  : .
    Return Code     : . int (0: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Bind(int p_sfd, int p_port_no)
/*----------------------------------------------------------------------*/
{
    int                 rt;
    struct sockaddr_in  address;

    memset((char *)&address, 0, sizeof (address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(p_port_no);

    rt = bind(p_sfd, (struct sockaddr *)&address, sizeof (struct sockaddr));

    return (rt);
}   /* End of Bind ()   */

/*************************************************************************
    End of Program (tcpip_bind.c)
*************************************************************************/
