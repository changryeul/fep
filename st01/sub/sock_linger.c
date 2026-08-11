/*------------------------------------------------------------------------
#   Module  : Socket linger utility
#   File    : sock_linger.c
#   Note    : Separated from fep_common.c to avoid multiple-definition
#             conflicts with processes that define their own
#             Device_Read/Device_Write (e.g., pb_8100_ts, pa_8100_ts).
------------------------------------------------------------------------*/

#include    "fep_fepp.h"

/*************************************************************************
    Function        : . Set_Socket_Linger
    Parameters IN   : . fd : socket file descriptor
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . set linger option on socket (immediate RST on close)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Set_Socket_Linger(int fd)
/*----------------------------------------------------------------------*/
{
    int             rt;
    struct linger   ling;

    /* close () returns after discarding any unsent data */
    ling.l_onoff = 1;
    ling.l_linger = 0;

    rt = setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof (ling));
    if (rt < 0)
        Log(TCP_ERROR, "setsockopt SO_LINGER {%d:%s}", SYS_NO, SYS_STR);

    return;
}   /* End of Set_Socket_Linger ()  */

/*************************************************************************
    End of Program (sock_linger.c)
*************************************************************************/
