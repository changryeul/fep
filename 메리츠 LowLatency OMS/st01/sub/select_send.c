/*------------------------------------------------------------------------
#	Module	: select and send a packet
#	File	: select_send.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*************************************************************************
	Function		: . select and send a packet
	Prameters IN	: . p_sfd		: socket file descriptor
					  . p_send		: data buffer
					  . p_length	: data size
	Parameters OUT	: .
	Return Code		: . int (0: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Select_Send (int p_sfd, char *p_send, int p_length)
/*----------------------------------------------------------------------*/
{
	int				rt;
	fd_set			write_set;
	struct timeval	timeout;

	FD_ZERO (&write_set);
	FD_SET (p_sfd, &write_set);
	timeout.tv_sec = 0;
	timeout.tv_usec = 200000;

	rt = select (p_sfd+1, NULL, &write_set, NULL, &timeout);
	if(rt < 0)
	{
		Log (TCP_ERROR, "Select_Send:select failure[%d] {%d:%s}",
			rt, SYS_NO, SYS_STR);
		return (NOTOK);
	}

	if (!FD_ISSET (p_sfd, &write_set))
	{
		Log (TCP_ERROR, "Select_Send:FD_ISSET {%d:%s}", SYS_NO, SYS_STR);
		return (NOTOK);
	}

	rt = Sendn (p_sfd, p_send, p_length);
	if (rt <= 0)
	{
		Log (TCP_ERROR, "Select_Send:send failure[%d] {%d:%s}",
			rt, SYS_NO, SYS_STR);
		return (NOTOK);
	}

	return (OK);
}	/* End of Select_Send ()	*/

/*************************************************************************
	End of Program (select_send.c)
*************************************************************************/
