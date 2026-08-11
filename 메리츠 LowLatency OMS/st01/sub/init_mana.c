/*------------------------------------------------------------------------
#	Module	: initialize global variables and attach to daemon SHM (INFO)
#				- used by daemons or utilities
#	File	: init_mana.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"

/*----------------------------------------------------------------------*/
void	Init_Mana (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	int		i;
	char	fifo_name[100], bumun[4];

	if (argv[0][1] < 'a' || argv[0][1] > 'z')
	{
		printf ("ERROR:invalid sub system name[%c]\n", argv[0][1]);
		exit (FAIL);
	}

	_System_Name[0] = argv[0][0];						/* system name	*/
	LtoU (_System_Name, 1);

	sprintf (_Exe_Name, "%s", argv[0]);				/* execution name	*/
	sprintf (_Process_Name, "%s", argv[0]);
	sprintf (_SubSystem_Name, "%-2.2s", argv[0]);			/*sub name	*/
	D_K = _SubSystem_Name[1] - 'a';						/* daemon key	*/
	P_K = -1;											/* process key	*/

	/* check environment values	*/
	Check_Environment ();

	/* check if this module run or not	*/
	if (_Exe_Name[1] != 'x')
	{
		Log (PRO_OK, "process START");
		Check_Exist ();
	}

	/* set fatal signal handlers	*/
	Setsigfatal ();

	/* attach daemon SHM (INFO)	*/
	Sub_SHM ();

	if (memcmp (_Exe_Name+3, "daemon_mp", 9) != 0 &&
		memcmp (_Exe_Name+3, "procchk", 7) != 0)
		/* attach all sub SHM	*/
		Mem_SHM (0, -1);

	for (i = 0; i < SHM_MAX_SUB; i++)
	{
		if (memcmp (INFO(i).process_id, _Exe_Name, strlen (_Exe_Name)) == 0) 
			DD_K = i;
	}
	
	sprintf (bumun, "%s", _SubSystem_Name);
	LtoU (bumun, 2);

	sprintf (fifo_name,
		"%s/%s/%s", _FEP_FIFO, bumun, INFO(D_K).start_FIFO_name);
	SFIFD(D_K) = open (fifo_name, O_RDWR | O_NDELAY);
	if (SFIFD(D_K) == -1)
	{
		Log (FIF_FATAL, "Init_Mana:cannot open start FIFO[%s] {%d:%s}",
			fifo_name, SYS_NO, SYS_STR);
		Exit_Process ();
	}

	sprintf (fifo_name, "%s/%s/%s", _FEP_FIFO, bumun, INFO(D_K).exit_FIFO_name);
	EFIFD(D_K) = open (fifo_name, O_RDWR | O_NDELAY);
	if (EFIFD(D_K) == -1)
	{
		Log (FIF_FATAL, "Init_Mana:cannot open daemon exit FIFO[%s] {%d:%s}",
			fifo_name, SYS_NO, SYS_STR);
		Exit_Process ();
	}

	sprintf (fifo_name, "%s/%s/%s",
		_FEP_FIFO, bumun, INFO(D_K).daemon_FIFO_name);
	DFIFD(D_K) = open (fifo_name, O_RDWR | O_NDELAY);
	if (DFIFD(D_K) == -1)
	{
		Log (FIF_FATAL, "Init_Mana:cannot open daemon FIFO[%s] {%d:%s}",
			fifo_name, SYS_NO, SYS_STR);
		Exit_Process ();
	}

	if (_Exe_Name[1] != 'x')
		Log (PRO_OK, "SFIFD[%d] EFIFD[%d] DFIFD[%d]",
			SFIFD(D_K), EFIFD(D_K), DFIFD(D_K));

	return;
}	/* End of Init_Mana ()	*/

/*************************************************************************
	End of Program (init_mana.c)
*************************************************************************/
