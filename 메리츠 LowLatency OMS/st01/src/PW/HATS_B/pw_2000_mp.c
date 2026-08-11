#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: SHM log write manager
#	File	: pw_2000_mp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		DATA_TIME	(2 * 1000)

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PW_2000_MP (void);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PW_2000_MP ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PW_2000_MP (void)
/*----------------------------------------------------------------------*/
{
	int				i, rt, data_cnt, poll_cnt;
	long			offset;
	char			w_buf[2048], poll_flag, tmp[128];
	struct pollfd	Poll;

	poll_cnt = 0;
	poll_flag = OFF;
	Poll.fd = ShmLogFifoFd;
	Poll.events = POLLIN;

	while (START_S != JOB_END)
	{
		data_cnt = SLOGW(D_K) - SLOGR(D_K);

		if (poll_flag == ON && data_cnt == 0)
		{
			while (1)
			{
				rt = read (Poll.fd, tmp, sizeof (tmp));

				if (rt == 0)
				{
					poll_flag = OFF;
					break;
				}
				else if (rt == -1)
				{
					if (SYS_NO == EINTR)
						continue;

					Log (FIF_ERROR, "[%s_slog] cannot read FIFO {%d:%s}",
						DAEMON(D_K).start_FIFO_name, SYS_NO, SYS_STR);
					break;
				}
			}
		}

		if (data_cnt > 0)
		{
			for (i = 0; i < data_cnt; i ++)
			{
				memset (w_buf, 0, sizeof (w_buf));
				offset = ((SLOGR(D_K) + i) % SHM_LOG_MAX) * SHM_LOG_SIZE;
				memcpy (w_buf, &ShmLogPtr[offset], SHM_LOG_SIZE);

				if (w_buf[0] != NULL)
					Write_SLog (w_buf);

				poll_cnt = 0;
			}

			SLOGR(D_K) += data_cnt;
		}
		else
		{
			poll_cnt ++;

			rt = poll (&Poll, 1, DATA_TIME);

			if (rt > 0)
			{
				if (Poll.revents & POLLIN)
				{
					Poll.revents = 0;
					poll_flag = ON;
					continue;
				}

				if (Poll.revents & POLLHUP)
				{
					Log (SYS_FATAL, "[%s_slog] poll hangup",
						DAEMON(D_K).start_FIFO_name);
					continue;
				}
			}
			else if (rt == 0)
			{
				if (poll_cnt == 30)
				{
					Log (USR_OK, "[%s_slog] poll timeout <%d>",
						DAEMON(D_K).start_FIFO_name, SLOGR(D_K));
					poll_cnt = 0;
				}

				continue;
			}
			else
			{
				if (SYS_NO == EINTR)
					continue;

				Log (SYS_ERROR, "[%s_slog] poll fail {%d:%s}",
					DAEMON(D_K).start_FIFO_name, SYS_NO, SYS_STR);
				continue;
			}
		}
	}

	return;
}	/* End of PW_2000_MP ()	*/

/*************************************************************************
	End of Program (pw_2000_mp.c)
*************************************************************************/
