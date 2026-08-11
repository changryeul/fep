#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: sub daemon
#	File	: pz_daemon.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"sub_daemon.h"

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	Check_Argument (int argc, char *argv[]);

/*************************************************************************
	Function		: . super daemon main
	Parameters IN	: . argc	: number of arguments (1)
					  . argv	: execution name
	Parameters OUT	: .
	Return Code		: . 0 (OK)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	/* initialize global variables and attach to daemon SHM (INFO)	*/
	Init_Mana (argc, argv);

	/* check arguments	*/
	Check_Argument (argc, argv);

	/* make it a daemon - create a new process and set process group ID	*/
	Make_Daemon ();

	/* get the process ID and set pid of daemon SHM	*/
	INFO(D_K).process_no = getpid ();

	/* set fatal signal handlers	*/
	Setsigfatal ();

	/* main routine	*/
	Main_Process ();

	exit (OK);
} 	/* End of main ()	*/

/*************************************************************************
	Function		: . check arguments
	Parameters IN	: . argc	: number of arguments (1)
					  . argv	: execution name
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Check_Argument (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	/* check the number of arguments	*/
	if (argc > 2)
	{
		Log (USR_FATAL, "invalid argument count[%d]", argc);
		exit (FAIL);
	}

	/* check the sub system name	*/
	if (argv[0][1] < 'a' || argv[0][1] > 'w')
	{
		Log (USR_FATAL, "invalid sub system name[%c]", argv[0][1]);
		exit (FAIL);
	}

	return;
}	/* End of Check_Argument ()	*/

/*************************************************************************
	End of Program (pz_daemon.c)
*************************************************************************/
