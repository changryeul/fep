/*------------------------------------------------------------------------
#   Module  : accept a connection on a socket (TCP/IP)
#   File    : tcpip_accept.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*************************************************************************
    Function        : . accept a connection on a socket
    Parameters IN   : . p_sfd   : socket file descriptor
    Parameters OUT  : . p_ip    : client IP address
                      . p_port  : client port
    Return Code     : . int (a descriptor for the accepted socket: success,
                        -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Accept(int p_sfd, char *p_ip, u_short *p_port)
/*----------------------------------------------------------------------*/
{
    char                hostip[INET_ADDRSTRLEN];
    int                 rt;
#if defined __hpux || defined _AIX || defined __linux
    socklen_t           addrlen;
#else
    size_t              addrlen;
#endif
    struct sockaddr_in  address;
    struct hostent      *hp;

#if defined __linux
    addrlen = sizeof (address);
#else
    addrlen = sizeof (struct sockaddr);
#endif
    memset((char *)&address, 0, sizeof (address));

    rt = accept(p_sfd, (struct sockaddr *)&address, &addrlen);

    if (rt < 0)
        return (rt);

    hp = gethostbyaddr((char *)&address.sin_addr,
            sizeof (struct in_addr), address.sin_family);
    inet_ntop(AF_INET, &address.sin_addr, hostip, sizeof (hostip));
    *p_port = ntohs (address.sin_port);
    sprintf(p_ip, "%s", hostip);
    if (hp != NULL)
        Log(TCP_OK, "request from %s(%s:%u)", hp->h_name, hostip, *p_port);
    else
        Log(TCP_OK, "request from(%s:%u)", hostip, *p_port);

    {
        int flag = 1;
        struct timeval snd_timeout;

        if (setsockopt(rt, IPPROTO_TCP, TCP_NODELAY,
                (char *)&flag, sizeof (flag)) < 0)
        Log(TCP_WARN, "Accept:TCP_NODELAY fail {%d:%s}",
                SYS_NO, SYS_STR);

        snd_timeout.tv_sec = 0;
        snd_timeout.tv_usec = 200000;
        if (setsockopt(rt, SOL_SOCKET, SO_SNDTIMEO,
                (char *)&snd_timeout, sizeof (snd_timeout)) < 0)
        Log(TCP_WARN, "Accept:SO_SNDTIMEO fail {%d:%s}",
                SYS_NO, SYS_STR);
    }

    return (rt);
}   /* End of Accept () */

/*************************************************************************
    End of Program (tcpip_accept.c)
*************************************************************************/
