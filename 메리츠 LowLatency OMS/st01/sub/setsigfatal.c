/*------------------------------------------------------------------------
#	Module	: set fatal signal handlers
#	File	: setsigfatal.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"
#include	"fep_interface.h"

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
extern void		End_Routine (int);
extern void		Exit_Process (void);

/*************************************************************************
	Function		: . set fatal signal handlers
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Setsigfatal (void)
/*----------------------------------------------------------------------*/
{
	signal (SIGINT, End_Routine);
	signal (SIGKILL, End_Routine);
	signal (SIGQUIT, End_Routine);
	signal (SIGILL, End_Routine);
	signal (SIGTERM, End_Routine);
	signal (SIGBUS, End_Routine);
	signal (SIGSEGV, End_Routine);
	signal (SIGHUP, End_Routine);

	return;
}	/* End of Setsigfatal ()	*/

/*************************************************************************
	Function		: . process end routine
	Parameters IN	: . p_signo	: signal number
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	End_Routine (int p_signo)
/*----------------------------------------------------------------------*/
{
	switch (p_signo) 
	{
		case	SIGINT:
			Log (SYS_FATAL, "signal caught [%d:SIGINT]", p_signo);
			break;
		case	SIGKILL:
			Log (SYS_WARN, "signal caught [%d:SIGKILL]", p_signo);
			break;
		case	SIGILL:
			Log (SYS_FATAL, "signal caught [%d:SIGILL]", p_signo);
			break;
		case	SIGQUIT:
			Log (SYS_FATAL, "signal caught [%d:SIGQUIT]", p_signo);
			break;
		case	SIGTERM:
			Log (SYS_WARN, "signal caught [%d:SIGTERM]", p_signo);
			break;
		case	SIGBUS:
			Log (SYS_FATAL, "signal caught [%d:SIGBUS]", p_signo);
			break;
		case	SIGSEGV:
			Log (SYS_FATAL, "signal caught [%d:SIGSEGV]", p_signo);
			break;
		case	SIGHUP:
			Log (SYS_FATAL, "signal caught [%d:SIGHUP]", p_signo);
			break;
		default:
			Log (SYS_WARN, "signal caught [%d:XXXXXXX]", p_signo);
			break;
	}

	Exit_Process ();

	exit (FAIL);
}	/* End of End_Routine ()	*/

/*************************************************************************
	Function		: . initialize process status and exit process
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Exit_Process (void)
/*----------------------------------------------------------------------*/
{
	char	bumun[4];

	if (D_K != -1) 
	{
		if (DD_K != -1)				/* sub daemons (e.g. pa_daemon_mp)	*/
			INFO(DD_K).process_no = 0;
		
		if (P_K != -1) 
		{
			if (PROC(D_K,P_K).type == TY_TRS2)					/* TCP2	*/
			{
				if (PROC(D_K,P_K).l.t2.l[1] != 0 && PROC(D_K,P_K).backup == 1)
				{									/* primary + backup	*/
					TCP2_LSTAT(D_K,P_K,0) = 0;
					TCP2_PSTAT(D_K,P_K,0) = 0;

					TCP2_LSTAT(D_K,P_K,1) = 0;
					TCP2_PSTAT(D_K,P_K,1) = 0;
					PROC(D_K,P_K).process_no = 0;
				}
				else
				{										/* primary only	*/
					TCP2_LSTAT(D_K,P_K,0) = 0;
					TCP2_PSTAT(D_K,P_K,0) = 0;
					PROC(D_K,P_K).process_no = 0;
				}
			}
			else
				PROC(D_K,P_K).process_no = 0;

			if (PROC(D_K,P_K).process_status == 1 &&		/* 1: run	*/
				PROC(D_K,P_K).start_status != 2) 			/* 2: end	*/
				write (DTART_FD, "1", 1);				/* daemon FIFO	*/
		}
	}

	sprintf (bumun, "%s", _SubSystem_Name);
	LtoU (bumun, 2);

	if (DD_K == -1 && bumun[1] != 'W' && bumun[1] != 'X' &&
		bumun[1] != 'Y' && bumun[1] != 'Z')
	{
		if (PROC(D_K,P_K).process_status == 4)
			PROC(D_K,P_K).process_status = 2;
		else if (PROC(D_K,P_K).process_status == 8)			/* TR1, TS1	*/
			PROC(D_K,P_K).process_status = 9;
		else
			Stat_Save ();
	}

	Log (PRO_OK, "process STOP");

	exit (FAIL);
}	/* End of Exit_Process ()	*/

/*************************************************************************
	End of Program (setsigfatal.c)
*************************************************************************/
