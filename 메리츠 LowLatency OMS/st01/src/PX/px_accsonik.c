#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: read configuration file and compare it with current shared memory
#	File	: px_accsonik.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include    "pa_struct.h"

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	int		i;
	char	path[100];

    Sub_SHM ();
    Mem_SHM (1, 0);
    Sise_SHM ();

	sprintf (path, "%s%s3/SEQ/TOTAL", (char *)getenv("_FEP_HOME"),
                        (char *)getenv("_FEP_SYSTEM"));

	printf("%s\n", PROC(0,0).date);
	printf("--------------------------------------------------------\n");
	printf(" ApType       매매손익   수수료   평가손익\n");

    for (i = 0; i < ACC_NO_CNT; i++)
    {
		printf("--------------------------------------------------------\n");

        /* 계좌정보 */
		printf("   %2.2s : ", ACCNO(0,i).aptype_code);

        /* 매매손익 */
		printf("%10d", ACCNO(0,i).acc_real_prft*10 - ACCNO(0,i).acc_fee +
						ACCNO(0,i).acc_ver_prft);

        /* 수수료   */
		printf("%10d", ACCNO(0,i).acc_fee);

        /* 평가손익 */
		printf("%10d\n", ACCNO(0,i).acc_ver_prft);

    }

	printf("--------------------------------------------------------\n");

}	/* End of main ()	*/

/*************************************************************************
	End of Program (px_accsonik.c)
*************************************************************************/
