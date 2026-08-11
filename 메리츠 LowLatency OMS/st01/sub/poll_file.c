/*------------------------------------------------------------------------
#	Module	: poll - input/output multiplexing
#	File	: poll_file.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"
#include	"fep_interface.h"

/*************************************************************************
	Function		: . poll
	Parameters IN	: . p_time	: timeout
	Parameters OUT	: .
	Return Code		: . int (0:success, 1:timeout, -1:failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Poll_File (int p_time)
/*----------------------------------------------------------------------*/
{
    int				rt;
	char			buf[10];
	struct pollfd	Poll;

	do {
		Poll.fd = INPUT_FD;
		Poll.events = POLLIN;

		rt = poll (&Poll, 1, p_time);
		if (rt == 0)
			return (1);
		else if (rt < 0)
		{
			if (SYS_NO == EINTR)
				continue;

			Log (SYS_ERROR, "Poll_File:poll failure {%d:%s}", SYS_NO, SYS_STR);
			return (-1);
		}
	} while (rt < 0);

	if (Poll.revents & POLLHUP)
	{
		Log (SYS_FATAL, "Poll_File:poll hangup");
		return (0);
	}

    if (Poll.revents & POLLIN)
		Poll.revents = 0;

	while (1)
	{
		rt = read (Poll.fd, buf, 1);
#if defined __linux
		if (rt == 0 || errno == EAGAIN)
#else
		if (rt == 0)
#endif
			break;
		else if (rt == -1)
		{
			if (SYS_NO == EINTR)
				continue;

			Log (FIF_ERROR,
				"Poll_File:cannot read FIFO {%d:%s}", SYS_NO, SYS_STR);
			return (rt);
		}
	}

    return (0);
}	/* End of Poll_File ()	*/

/*************************************************************************
	End of Program (poll_file.c)
*************************************************************************/
