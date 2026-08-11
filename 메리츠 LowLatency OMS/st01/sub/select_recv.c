/*------------------------------------------------------------------------
#	Module	: select and receive a packet
#	File	: select_recv.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*************************************************************************
	Function		: . select and receive a packet
	Prameters IN	: . p_sfd		: socket file descriptor
					  . p_recv		: data buffer
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure, >0:packet length)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Select_Receive (int p_sfd, char *p_recv)
/*----------------------------------------------------------------------*/
{
	int 			rt, pkt_len;
	fd_set			read_set;
	struct timeval	timeout;

	SYS_NO = 0;
	FD_ZERO (&read_set);
	FD_SET (p_sfd, &read_set);

	timeout.tv_sec = TIME_OUT;
	timeout.tv_usec	= 0;

	rt = select (p_sfd+1, &read_set, NULL, NULL, &timeout);

	if (rt < 0)
	{
		Log (TCP_ERROR, "Select_Receive:select fail[%d] {%d:%s}",
			rt, SYS_NO, SYS_STR);
		return (NOTOK);
	}

	if (!FD_ISSET (p_sfd, &read_set))
	{
		Log (TCP_OK, "Select_Receive:select timeout");
		return (OK);
	}

	rt = Recvn (p_sfd, p_recv, 5);

	if (rt < 0)
	{
		if (SYS_NO == ECONNRESET)
			Log (TCP_WARN, "Select_Receive:socket reset");
		else
			Log (TCP_ERROR, "Select_Receive:length:receive fail[%d] {%d:%s}",
				rt, SYS_NO, SYS_STR);

		return (NOTOK);
	}
	else if (rt == 0)
	{
		Log (TCP_WARN, "Select_Receive:socket disconnected[%d]", rt);
		return (NOTOK);
	}

	pkt_len = AtoIf (p_recv+1, 4);

	if (pkt_len < TCP_HEAD_LEN-5)
		Log (USR_ERROR, "Select_Receive:invalid Length[%d]", pkt_len);

//	rt = Recvn (p_sfd, p_recv+5, pkt_len - 5);
	rt = Recvn (p_sfd, p_recv+5, pkt_len);

	if (rt < 0)
	{
		Log (TCP_ERROR, "Select_Receive:data:receive fail[%d] {%d:%s}",
			rt, SYS_NO, SYS_STR);
		return (NOTOK);
	}

	return (pkt_len+5);
}	/* End of Select_Receive ()	*/

/*************************************************************************
	Function		: . select and receive a packet
	Prameters IN	: . p_sfd		: socket file descriptor
					  . p_recv		: data buffer
					  . p_len		: size of length field
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure, >0:packet length)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Select_Receive2 (int p_sfd, char *p_recv, int p_len)
/*----------------------------------------------------------------------*/
{
	int 			rt, pkt_len;
	fd_set			read_set;
	struct timeval	timeout;

	SYS_NO = 0;
	FD_ZERO (&read_set);
	FD_SET (p_sfd, &read_set);

	timeout.tv_sec = TIME_OUT;
	timeout.tv_usec	= 0;

	rt = select (p_sfd+1, &read_set, NULL, NULL, &timeout);

	if (rt < 0)
	{
		Log (TCP_ERROR, "Select_Receive2:select fail[%d] {%d:%s}",
			rt, SYS_NO, SYS_STR);
		return (NOTOK);
	}

	if (!FD_ISSET (p_sfd, &read_set))
	{
		Log (TCP_WARN, "Select_Receive2:select timeout");
		return (OK);
	}

	rt = Recvn (p_sfd, p_recv, p_len);

	if (rt < 0)
	{
		if (SYS_NO == ECONNRESET)
			Log (TCP_WARN, "Select_Receive2:socket reset");
		else
			Log (TCP_ERROR, "Select_Receive2:length:receive fail[%d] {%d:%s}",
				rt, SYS_NO, SYS_STR);

		return (NOTOK);
	}
	else if (rt == 0)
	{
		Log (TCP_WARN, "Select_Receive2:socket disconnected[%d]", rt);
		return (NOTOK);
	}

	pkt_len = AtoIf (p_recv, p_len);

	rt = Recvn (p_sfd, &p_recv[p_len], pkt_len - p_len);

	if (rt < 0)
	{
		Log (TCP_ERROR, "Select_Receive2:data:receive fail[%d] {%d:%s}",
			rt, SYS_NO, SYS_STR);
		return (NOTOK);
	}

	return (pkt_len);
}	/* End of Select_Receive2 ()	*/

/*************************************************************************
    Function        : . select and receive a packet
    Prameters IN    : . p_sfd       : socket file descriptor
                      . p_recv      : data buffer
                      . p_len       : size of length field
    Parameters OUT  : .
    Return Code     : . int (0:success, -1:failure, >0:packet length)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Select_Receive_Krx (int p_sfd, char *p_recv, int p_len)
/*----------------------------------------------------------------------*/
{
    int             rt, pkt_len;
    fd_set          read_set;
    struct timeval  timeout;

    SYS_NO = 0;
    FD_ZERO (&read_set);
    FD_SET (p_sfd, &read_set);

    timeout.tv_sec = TIME_OUT;
    timeout.tv_usec = 0;

    rt = select (p_sfd+1, &read_set, NULL, NULL, &timeout);

    if (rt < 0)
    {
        Log (TCP_ERROR, "Select_Receive_Krx:select fail[%d] {%d:%s}",
            rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    if (!FD_ISSET (p_sfd, &read_set))
    {
        Log (TCP_WARN, "Select_Receive_Krx:select timeout");
        return (OK);
    }

    rt = Recvn (p_sfd, p_recv, p_len);

    if (rt < 0)
    {
        if (SYS_NO == ECONNRESET)
            Log (TCP_WARN, "Select_Receive_Krx:socket reset");
        else
            Log (TCP_ERROR, "Select_Receive_Krx:length:receive fail[%d] {%d:%s}",
                rt, SYS_NO, SYS_STR);

        return (NOTOK);
    }
    else if (rt == 0)
    {
        Log (TCP_WARN, "Select_Receive_Krx:socket disconnected[%d]", rt);
        return (NOTOK);
    }

    pkt_len = AtoIf (&p_recv[8], 6);

    if (pkt_len > 0)
    {
        rt = Recvn (p_sfd, &p_recv[p_len], pkt_len);

        if (rt < 0)
        {
            Log (TCP_ERROR, "Select_Receive_Krx:data:receive fail[%d] {%d:%s}",
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
int     Select_Receive_Cli (int p_sfd, char *p_recv)
/*----------------------------------------------------------------------*/
{
    int             rt, pkt_len, rev_len;
    fd_set          read_set;
    struct timeval  timeout;

    SYS_NO = 0;
    FD_ZERO (&read_set);
    FD_SET (p_sfd, &read_set);

    timeout.tv_sec = TIME_OUT;
    timeout.tv_usec = 0;

    rt = select (p_sfd+1, &read_set, NULL, NULL, &timeout);

    if (rt < 0)
    {
        Log (TCP_ERROR, "Select_Receive:select fail[%d] {%d:%s}",
            rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    if (!FD_ISSET (p_sfd, &read_set))
    {
        Log (TCP_OK, "Select_Receive:select timeout");
        return (OK);
    }

	rt = Recvn (p_sfd, p_recv, 4);
    if (rt < 0)
    {
        if (SYS_NO == ECONNRESET)
            Log (TCP_WARN, "Select_Receive:socket reset");
        else
            Log (TCP_ERROR, "Select_Receive:length:receive fail[%d] {%d:%s}",
                rt, SYS_NO, SYS_STR);

        return (NOTOK);
    }
    else if (rt == 0)
    {
        Log (TCP_WARN, "Select_Receive:socket disconnected[%d]", rt);
        return (NOTOK);
    }

	pkt_len = AtoIf (p_recv, 4);
    if (pkt_len < CLI_HEAD_LEN - 4)
        Log (USR_ERROR, "Select_Receive:invalid Length[%d]", pkt_len);

    //rt = Recvn (p_sfd, p_recv+4, pkt_len - 4);
    rt = Recvn (p_sfd, p_recv+4, pkt_len);

    if (rt < 0)
    {
        Log (TCP_ERROR, "Select_Receive:data:receive fail[%d] {%d:%s}",
            rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    return (pkt_len);
}   /* End of Select_Receive_Cli () */

/*************************************************************************
	End of Program (select_recv.c)
*************************************************************************/
