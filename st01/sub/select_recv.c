/*------------------------------------------------------------------------
#   Module  : select and receive a packet
#   File    : select_recv.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#define SZ_FEEDDATA_MAX 8192

/*************************************************************************
    Function        : . select and receive a packet
    Prameters IN    : . p_sfd       : socket file descriptor
                      . p_recv      : data buffer
    Parameters OUT  : .
    Return Code     : . int (0:success, -1:failure, >0:packet length)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Select_Receive(int p_sfd, char *p_recv)
/*----------------------------------------------------------------------*/
{
    int             rt, pkt_len;
    fd_set          read_set;
    struct timeval  timeout;

    SYS_NO = 0;
    FD_ZERO(&read_set);
    FD_SET(p_sfd, &read_set);

    timeout.tv_sec = TIME_OUT;
    timeout.tv_usec = 0;

    rt = select(p_sfd+1, &read_set, NULL, NULL, &timeout);

    if (rt < 0) {
        Log(TCP_ERROR, "Select_Receive:select fail[%d] {%d:%s}",
                rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    if (!FD_ISSET(p_sfd, &read_set)) {
        Log(TCP_OK, "Select_Receive:select timeout");
        return (OK);
    }

    rt = Recvn(p_sfd, p_recv, 5);

    if (rt < 0) {
        if (SYS_NO == ECONNRESET)
            Log(TCP_WARN, "Select_Receive:socket reset");
        else
            Log(TCP_ERROR, "Select_Receive:length:receive fail[%d] {%d:%s}",
                    rt, SYS_NO, SYS_STR);

        return (NOTOK);
    }
    else if (rt == 0) {
        Log(TCP_WARN, "Select_Receive:socket disconnected[%d]", rt);
        return (NOTOK);
    }

    pkt_len = AtoIf(p_recv+1, 4);

    if (pkt_len < TCP_HEAD_LEN - 5 || pkt_len > TCP_BUFF_MAX_LEN - 5) {
        Log(USR_ERROR, "Select_Receive:invalid Length[%d]", pkt_len);
        return (NOTOK);
    }

    rt = Recvn(p_sfd, p_recv+5, pkt_len);

    if (rt < 0) {
        Log(TCP_ERROR, "Select_Receive:data:receive fail[%d] {%d:%s}",
                rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    return (pkt_len+5);
}   /* End of Select_Receive () */

/*************************************************************************
    Function        : . select and receive a packet
    Prameters IN    : . p_sfd       : socket file descriptor
                      . p_recv      : data buffer
                      . p_len       : size of length field
    Parameters OUT  : .
    Return Code     : . int (0:success, -1:failure, >0:packet length)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Select_Receive2(int p_sfd, char *p_recv, int p_len)
/*----------------------------------------------------------------------*/
{
    int             rt, pkt_len;
    fd_set          read_set;
    struct timeval  timeout;

    SYS_NO = 0;
    FD_ZERO(&read_set);
    FD_SET(p_sfd, &read_set);

    timeout.tv_sec = TIME_OUT;
    timeout.tv_usec = 0;

    rt = select(p_sfd+1, &read_set, NULL, NULL, &timeout);

    if (rt < 0) {
        Log(TCP_ERROR, "Select_Receive2:select fail[%d] {%d:%s}",
                rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    if (!FD_ISSET(p_sfd, &read_set)) {
        Log(TCP_WARN, "Select_Receive2:select timeout");
        return (OK);
    }

    rt = Recvn(p_sfd, p_recv, p_len);

    if (rt < 0) {
        if (SYS_NO == ECONNRESET)
            Log(TCP_WARN, "Select_Receive2:socket reset");
        else
            Log(TCP_ERROR, "Select_Receive2:length:receive fail[%d] {%d:%s}",
                    rt, SYS_NO, SYS_STR);

        return (NOTOK);
    }
    else if (rt == 0) {
        Log(TCP_WARN, "Select_Receive2:socket disconnected[%d]", rt);
        return (NOTOK);
    }

    pkt_len = AtoIf(p_recv, p_len);

    if (pkt_len <= p_len || pkt_len > TCP_BUFF_MAX_LEN) {
        Log(USR_ERROR, "Select_Receive2:invalid Length[%d]", pkt_len);
        return (NOTOK);
    }

    rt = Recvn(p_sfd, &p_recv[p_len], pkt_len - p_len);

    if (rt < 0) {
        Log(TCP_ERROR, "Select_Receive2:data:receive fail[%d] {%d:%s}",
                rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    return (pkt_len);
}   /* End of Select_Receive2 ()    */

/*************************************************************************
    Function        : . select and receive a packet
    Prameters IN    : . p_sfd       : socket file descriptor
                      . p_recv      : data buffer
                      . p_len       : size of length field
    Parameters OUT  : .
    Return Code     : . int (0:success, -1:failure, >0:packet length)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Select_Receive_Krx(int p_sfd, char *p_recv, int p_len)
/*----------------------------------------------------------------------*/
{
    int             rt, pkt_len;
    fd_set          read_set;
    struct timeval  timeout;

    SYS_NO = 0;
    FD_ZERO(&read_set);
    FD_SET(p_sfd, &read_set);

    timeout.tv_sec = TIME_OUT;
    timeout.tv_usec = 0;

    rt = select(p_sfd+1, &read_set, NULL, NULL, &timeout);

    if (rt < 0) {
        Log(TCP_ERROR, "Select_Receive_Krx:select fail[%d] {%d:%s}",
                rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    if (!FD_ISSET(p_sfd, &read_set)) {
        Log(TCP_WARN, "Select_Receive_Krx:select timeout");
        return (OK);
    }

    rt = Recvn(p_sfd, p_recv, p_len);

    if (rt < 0) {
        if (SYS_NO == ECONNRESET)
            Log(TCP_WARN, "Select_Receive_Krx:socket reset");
        else
            Log(TCP_ERROR, "Select_Receive_Krx:length:receive fail[%d] {%d:%s}",
                    rt, SYS_NO, SYS_STR);

        return (NOTOK);
    }
    else if (rt == 0) {
        Log(TCP_WARN, "Select_Receive_Krx:socket disconnected[%d]", rt);
        return (NOTOK);
    }

    pkt_len = AtoIf(&p_recv[8], 6);

    if (pkt_len > 0) {
        if (pkt_len > KRX_DATA_BUFF_SIZE - p_len) {
            Log(USR_ERROR, "Select_Receive_Krx:invalid Length[%d]", pkt_len);
            return (NOTOK);
        }

        rt = Recvn(p_sfd, &p_recv[p_len], pkt_len);

        if (rt < 0) {
            Log(TCP_ERROR, "Select_Receive_Krx:data:receive fail[%d] {%d:%s}",
                    rt, SYS_NO, SYS_STR);
            return (NOTOK);
        }
    }

    return (pkt_len+p_len);
}   /* End of Select_Receive_Krx () */

/*************************************************************************
    Function        : . select and receive a packet
    Prameters IN    : . p_sfd       : socket file descriptor
                      . p_recv      : data buffer
    Parameters OUT  : .
    Return Code     : . int (0:success, -1:failure, >0:packet length)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Select_Receive_Cli(int p_sfd, char *p_recv)
/*----------------------------------------------------------------------*/
{
    int             rt, pkt_len, rev_len;
    fd_set          read_set;
    struct timeval  timeout;

    SYS_NO = 0;
    FD_ZERO(&read_set);
    FD_SET(p_sfd, &read_set);

    timeout.tv_sec = TIME_OUT;
    timeout.tv_usec = 0;

    rt = select(p_sfd+1, &read_set, NULL, NULL, &timeout);

    if (rt < 0) {
        Log(TCP_ERROR, "Select_Receive:select fail[%d] {%d:%s}",
                rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    if (!FD_ISSET(p_sfd, &read_set)) {
        Log(TCP_OK, "Select_Receive:select timeout");
        return (OK);
    }

    rt = Recvn(p_sfd, p_recv, 4);
    if (rt < 0) {
        if (SYS_NO == ECONNRESET)
            Log(TCP_WARN, "Select_Receive:socket reset");
        else
            Log(TCP_ERROR, "Select_Receive:length:receive fail[%d] {%d:%s}",
                    rt, SYS_NO, SYS_STR);

        return (NOTOK);
    }
    else if (rt == 0) {
        Log(TCP_WARN, "Select_Receive:socket disconnected[%d]", rt);
        return (NOTOK);
    }

    pkt_len = AtoIf(p_recv, 4);
    if (pkt_len < CLI_HEAD_LEN - 4 || pkt_len > CLI_BUFF_MAX_LEN - 4) {
        Log(USR_ERROR, "Select_Receive_Cli:invalid Length[%d]", pkt_len);
        return (NOTOK);
    }

    rt = Recvn(p_sfd, p_recv+4, pkt_len);

    if (rt < 0) {
        Log(TCP_ERROR, "Select_Receive:data:receive fail[%d] {%d:%s}",
                rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    return (pkt_len);
}   /* End of Select_Receive_Cli () */

/*************************************************************************
    Function        : . select and receive a packet
    Prameters IN    : . p_sfd       : socket file descriptor
                      . p_recv      : data buffer
    Parameters OUT  : .
    Return Code     : . int (0:success, -1:failure, >0:packet length)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Select_Receive_Imeco(int p_sfd, char *p_recv)
/*----------------------------------------------------------------------*/
{
    int             rt, pkt_len, rev_len;
    fd_set          read_set;
    struct timeval  timeout;

    SYS_NO = 0;
    FD_ZERO(&read_set);
    FD_SET(p_sfd, &read_set);

    timeout.tv_sec = TIME_OUT;
    timeout.tv_usec = 0;

    rt = select(p_sfd+1, &read_set, NULL, NULL, &timeout);

    if (rt < 0) {
        Log(TCP_ERROR, "Select_Receive:select fail[%d] {%d:%s}",
                rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    if (!FD_ISSET(p_sfd, &read_set)) {
        Log(TCP_OK, "Select_Receive:select timeout");
        return (OK);
    }

    rt = Recvn(p_sfd, p_recv, 4);
    if (rt < 0) {
        if (SYS_NO == ECONNRESET)
            Log(TCP_WARN, "Select_Receive:socket reset");
        else
            Log(TCP_ERROR, "Select_Receive:length:receive fail[%d] {%d:%s}",
                    rt, SYS_NO, SYS_STR);

        return (NOTOK);
    }
    else if (rt == 0) {
        Log(TCP_WARN, "Select_Receive:socket disconnected[%d]", rt);
        return (NOTOK);
    }

    pkt_len = AtoIf(p_recv, 4) + 16;
    if (pkt_len < 20 - 4 || pkt_len > TCP_BUFF_MAX_LEN - 4) {
        Log(USR_ERROR, "Select_Receive_Imeco:invalid Length[%d]", pkt_len);
        return (NOTOK);
    }

    rt = Recvn(p_sfd, p_recv+4, pkt_len);

    if (rt < 0) {
        Log(TCP_ERROR, "Select_Receive:data:receive fail[%d] {%d:%s}",
                rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    return (pkt_len);
}   /* End of Select_Receive_Imeco () */

/*************************************************************************
    Function        : . select and receive a packet
    Prameters IN    : . p_sfd       : socket file descriptor
                      . p_recv      : data buffer
    Parameters OUT  : .
    Return Code     : . int (0:success, -1:failure, >0:packet length)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Select_Receive_Imeco_Sise(int p_sfd, char *p_recv)
/*----------------------------------------------------------------------*/
{
    int             rt, pkt_len, rev_len;
    fd_set          read_set;
    struct timeval  timeout;

    SYS_NO = 0;
    FD_ZERO(&read_set);
    FD_SET(p_sfd, &read_set);

    timeout.tv_sec = TIME_OUT;
    timeout.tv_usec = 0;

    rt = select(p_sfd+1, &read_set, NULL, NULL, &timeout);

    if (rt < 0) {
        Log(TCP_ERROR, "Select_Receive:select fail[%d] {%d:%s}",
                rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    if (!FD_ISSET(p_sfd, &read_set)) {
        Log(TCP_OK, "Select_Receive:select timeout");
        return (OK);
    }

    rt = Recvn(p_sfd, p_recv, 10);
    if (rt < 0) {
        if (SYS_NO == ECONNRESET)
            Log(TCP_WARN, "Select_Receive:socket reset");
        else
            Log(TCP_ERROR, "Select_Receive:length:receive fail[%d] {%d:%s}",
                    rt, SYS_NO, SYS_STR);

        return (NOTOK);
    }
    else if (rt == 0) {
        Log(TCP_WARN, "Select_Receive:socket disconnected[%d]", rt);
        return (NOTOK);
    }

    pkt_len = AtoIf(p_recv, 10) + 40;
    if (pkt_len < 50 - 10 || pkt_len > TCP_BUFF_MAX_LEN - 10) {
        Log(USR_ERROR, "Select_Receive_Imeco_Sise:invalid Length[%d]", pkt_len);
        return (NOTOK);
    }

    rt = Recvn(p_sfd, p_recv+10, pkt_len);

    if (rt < 0) {
        Log(TCP_ERROR, "Select_Receive:data:receive fail[%d] {%d:%s}",
                rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    return (pkt_len);
}   /* End of Select_Receive_Imeco_Sise () */

/*************************************************************************
    Function        : . select and receive a packet
    Prameters IN    : . p_sfd       : socket file descriptor
                      . p_recv      : data buffer
    Parameters OUT  : .
    Return Code     : . int (0:success, -1:failure, >0:packet length)
*************************************************************************/
/*----------------------------------------------------------------------*/
int Sise_Select_Receive(int p_sfd, char *p_recv)
/*----------------------------------------------------------------------*/
{
    int             rt;
    fd_set          read_set;
    struct timeval  timeout;

    static char     sockbuff[SZ_FEEDDATA_MAX];  /* cumulative buffer */
    static int      sockpos = 0;                /* accumulated length */
    int             ipos = 0;

    SYS_NO = 0;
    FD_ZERO(&read_set);
    FD_SET(p_sfd, &read_set);

    timeout.tv_sec  = TIME_OUT;
    timeout.tv_usec = 0;

    rt = select(p_sfd+1, &read_set, NULL, NULL, &timeout);

    if (rt < 0) {
        Log(TCP_ERROR, "Select_Receive:select fail[%d] {%d:%s}", rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    if (!FD_ISSET(p_sfd, &read_set)) {
        Log(TCP_OK, "Select_Receive:select timeout");
        return (OK);
    }

    /* accumulate received data */
    rt = recv(p_sfd, sockbuff + sockpos, SZ_FEEDDATA_MAX - sockpos, 0);
    if (rt <= 0) {
        if (rt == 0)
            Log(TCP_WARN, "Select_Receive:socket disconnected");
        else
            Log(TCP_ERROR, "Select_Receive:recv fail[%d] {%d:%s}", rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }
    sockpos += rt;

    /* search for packet delimiter (0xFF 0x0D 0x0A) */
    ipos = 0;
    while (ipos < sockpos) {
        p_recv[ipos] = sockbuff[ipos];

        if (ipos >= 2 &&
                sockbuff[ipos-2] == (char)0xFF &&
                sockbuff[ipos-1] == (char)0x0D &&
                sockbuff[ipos]   == (char)0x0A) {

            ipos++;  /* packet length = ipos */

            /* shift remaining data forward */
            memmove(sockbuff, sockbuff + ipos, sockpos - ipos);
            sockpos -= ipos;

            return ipos;  /* return 1 packet length */
        }
        ipos++;
    }

    return 0;  /* packet not yet complete */
}

/*************************************************************************
    End of Program (select_recv.c)
*************************************************************************/
