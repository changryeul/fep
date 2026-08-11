#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 주문체결처리
#	File	: pa_4400_mp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#define		DATA_SIZE	1024
#include	"buf_struct.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		DATA_TIME	60 * 1000
#define		READ_MAX	1

/*------------------------------------------------------------------------
	Global Variables
_Cnt;-------------------------------------------------------------------*/
int			R_Cnt;
FILE_BUFF_FORMAT	R_Fmt[READ_MAX];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_4400_MP (void);
void	Analyze_Data (void);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_4400_MP ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_4400_MP (void)
/*----------------------------------------------------------------------*/
{
	int     rt; 
	while (START_S != JOB_END)
	{
		Stat_Save ();

		while (START_S != JOB_END)
		{
			Stat_Save ();
			memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * READ_MAX);

			R_Cnt = F_R (PS_R_1, (void *)R_Fmt, READ_MAX);

			if (R_Cnt < 0)
			{
				Log (SAM_FATAL, "cannot read File[%s,%d:%s]",
					IFN(D_K,P_K,2), SYS_NO, SYS_STR);
				sleep (1);
				Exit_Process ();
			}
			else if (R_Cnt == 0)
				break;

			Log (USR_OK, "RD [%s:%d][%d]",
				IFN(D_K,P_K,0), R_Cnt, IFR(D_K,P_K,0,0));
			Analyze_Data ();
		}

		rt = Poll_File (DATA_TIME);

		if (rt == 1)
			Log (USR_OK, "poll timeout <%d>", OFW(D_K,P_K,0,0));
		else if (rt == -1)
			continue;
	}
}	/* End of PA_4400_MP ()	*/

/*************************************************************************
    Function        : . Analyze_Data
    Parameters IN   : . p_buf   : received data
					  . for_d	: data count
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . set Acc data SHM (손익/수수료/평가손익)
*************************************************************************/
/*----------------------------------------------------------------------*/
void   Analyze_Data (void)
/*----------------------------------------------------------------------*/
{
    int     i, rt, target;
	char    W_Fmt[8192];
	char    W_DFmt[8192];
	target = -1;

	for (i = 0; i < R_Cnt; i++)
	{

		/* 이곳에는 체결만 들어온다 */
		/* DATA FORMAT : 체결전문	*/
		/* Write는 체결전문만 넣는다.	*/
		SMB_ST	*dat = (SMB_ST *)&R_Fmt[i].Data[0];

		memset (W_Fmt, 0x20, sizeof (W_Fmt));
		memcpy (&W_Fmt, &R_Fmt[i], sizeof(BUFF_RW_HEAD));
		memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)],
			&R_Fmt[i].Data[0], sizeof (SMB_ST));
		W_Fmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

		if (memcmp (&dat->smb_ClOrdID[18], "0000", 4) != 0 && memcmp (&dat->smb_ClOrdID[18], "    ", 4) != 0 )
		{
			target = (AtoIf (&dat->smb_ClOrdID[18], 2) - 1) * 10;
			target = AtoIf (&dat->smb_ClOrdID[20], 2) + target;
			/* 전략에 전달 */
            /* 체결 */
			/* FX:20 + DATA전문 */
            memset (W_DFmt, 0x20, sizeof (W_DFmt));
		    memcpy (W_DFmt, &R_Fmt[i], sizeof (BUFF_RW_HEAD));          // 70
		    memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)],     "100140",   6);     // 응답(100120), 체결(100140)
			
	    	memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)+6],   "04",       2);     // 시장구분(04:FX)	
		
            memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)+20], R_Fmt[i].Data, DATA_SIZE);
		    W_DFmt[sizeof(BUFF_RW_HEAD)+ODS(D_K,P_K,0)] = '\n';

			rt = DSHM_W (target*10, (void *)&W_DFmt, 1);
			if (rt != 1)
			{
				Log (SAM_FATAL, "Dshm write fail[%s:%d]", ODN(D_K,P_K,target-1), rt);
				Exit_Process ();
			}
            Log (USR_OK, "DSHM Write OK [%450.450s][%d] target[%d]", W_DFmt, strlen(W_DFmt), target);
		}

		Add_Count (PS_R_1, 1);
	}	/* End of for */

}

/*************************************************************************
	End of Program (pa_4400_mp.c)
*************************************************************************/

