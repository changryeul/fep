#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: create and load shared memory
#	File	: pz_memory.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"daemon.h"

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void    Check_Argument (int argc, char *argv[]);

/*************************************************************************
	Function		: . main
	Parameters IN	: . argc	: number of arguments (1 or 2)
					  . argv[0]	: execution name
					  . argv[1]	: sub system name (a ~ z)
	Parameters OUT	: .
	Return Code		: . 0 (OK)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	_System_Name[0] = argv[0][0];
	LtoU (_System_Name, 1);
	sprintf (_Exe_Name, "%s", argv[0]);
	sprintf (_SubSystem_Name, "%-2.2s", argv[0]);
	sprintf (_Process_Name, "%s", argv[0]);
	D_K = P_K = -1;

	/* check environment values	*/
	Check_Environment ();

	/* check arguments	*/
	Check_Argument (argc, argv);

	/* main routine	*/
	Main_Process ();

	exit (OK);
} 	/* End of main ()	*/

/*************************************************************************
	Function		: . check arguments
	Parameters IN	: . argc	: number of arguments (1 or 2)
					  . argv[0]	: execution name
					  . argv[1]	: sub system name (a ~ z)
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
		puts ("==========================================================");
		printf ("ERROR: invalid arguments number [%d]\n", argc);
		printf ("USAGE: %s <sub system name (a ~ z)>\n", argv[0]);
		printf ("  e.g. 1) %s\n", argv[0]);
		printf ("       2) %s a\n", argv[0]);
		puts ("==========================================================");
		exit (FAIL);
	}

	/* check sub name	*/
	if (_SubSystem_Name[1] != 'z')
	{
		Log (USR_FATAL, "invalid sub name[%s]", _SubSystem_Name);
		exit (FAIL);
	}

	if (argc == 2)
		D_K = argv[1][0] - 'a';

	return;
}	/* End of Check_Argument ()	*/

/*************************************************************************
	End of Program (pz_memory.c)
*************************************************************************/
