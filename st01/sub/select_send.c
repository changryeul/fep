/*------------------------------------------------------------------------
#   Module  : select and send a packet
#   File    : select_send.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*************************************************************************
    Function        : . select and send a packet
    Prameters IN    : . p_sfd       : socket file descriptor
                      . p_send      : data buffer
                      . p_length    : data size
    Parameters OUT  : .
    Return Code     : . int (0: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Select_Send(int p_sfd, char *p_send, int p_length)
/*----------------------------------------------------------------------*/
{
    int     rt;

    rt = Sendn(p_sfd, p_send, p_length);
    if (rt <= 0) {
        Log(TCP_ERROR, "Select_Send:send failure[%d] {%d:%s}",
                rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    return (OK);
}   /* End of Select_Send ()    */

/*************************************************************************
    End of Program (select_send.c)
*************************************************************************/
