#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 시세 수신 (UDP)
#	File	: pa_9009_mp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#define     DATA_SIZE       20
#include    "buf_struct.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
#define		DATA_TIME       60 * 1000
#define		READ_MAX		1

int					R_Cnt;
char				ApType[10];
FILE_BUFF_FORMAT	W_Fmt, R_Fmt[READ_MAX];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_9009_MP (void);
void    Analyze_Data (void);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_9009_MP ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_9009_MP (void)
/*----------------------------------------------------------------------*/
{
	int				rt;
	char 			m_time[24];

	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
	LtoU (ApType, strlen (ApType));

	while (START_S != JOB_END)
	{
		Stat_Save ();

		while (START_S != JOB_END)
		{
			Stat_Save ();
			memset (R_Fmt, 0, sizeof (R_Fmt));

			R_Cnt = F_R (PS_R_1, (void *)R_Fmt, READ_MAX);
			if (R_Cnt < 0)
			{
				Log (SAM_FATAL, "cannot read File[%s,%d:%s]",
					IFN (D_K,P_K,0), SYS_NO, SYS_STR);
				sleep (1);
				Exit_Process ();
			}
			else if (R_Cnt == 0)
				break;

			Log (USR_OK, "RD [%s:%d][%d]",
				IFN (D_K,P_K,0), R_Cnt, IFR (D_K,P_K,0,0));
			Analyze_Data ();
		}

		rt = Poll_File (DATA_TIME);

        if (rt == 1)
            Log (USR_OK, "poll timeout <%d>", INT_SEQ);
        else if (rt == -1)
            continue;
	}

}	/* End of PA_9009_MP ()	*/

/*************************************************************************
    Function        : . Analyze_Data
    Parameters IN   : . 
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . analyze and divide data
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Analyze_Data (void)
/*----------------------------------------------------------------------*/
{
	/* DATA포맷, 시장구분(2) + SeqNo(5) + 종목코드(12) + SPACE(1) 총 20 */
	char	Data[100];
	char	item_code[13];
	int		mk_gbn, item_seq, i;
	double	mk_price_d, avg_tmp_d, ver_tmp_d, b_ver_d;

	memset (Data,		0,	sizeof (Data));
	memset (item_code,	0,	sizeof (item_code));

	memcpy (Data, R_Fmt[0].Data,	DATA_SIZE);

	/* 시장구분 */
	mk_gbn		=	AtoIf (Data, 2);
	/* 종목seq */
	item_seq	=	AtoIf (&Data[2], 5);
	/* 현재가 */
	mk_price_d	=	Shm_Risk[0].S_Sise[mk_gbn][item_seq].crprc;

	for (i = 0; i < ACC_NO_CNT; i++)
	{
		/* 계좌별 잔고가 존재하면 평가손익 재계산 */
		if (Shm_Risk[0].ProFit[mk_gbn][item_seq][i].item_getcnt != 0)
		{
			/* 기존평균가 */
			b_ver_d		=	Shm_Risk[0].ProFit[mk_gbn][item_seq][i].item_ver_prft;

			/* 평균손익 계산 */
			if (Shm_Risk[0].ProFit[mk_gbn][item_seq][i].item_getcnt > 0)
			{
				ver_tmp_d	=	(mk_price_d - avg_tmp_d) 
								/ Shm_Risk[0].mk_hoga_dan_d[mk_gbn] 
								* Shm_Risk[0].mk_hoga_val_d[mk_gbn]
								* abs(Shm_Risk[0].ProFit[mk_gbn][item_seq][i].item_getcnt);
			}
			else
			{
				ver_tmp_d	=	(avg_tmp_d - mk_price_d) 
								/ Shm_Risk[0].mk_hoga_dan_d[mk_gbn] 
								* Shm_Risk[0].mk_hoga_val_d[mk_gbn]
								* abs(Shm_Risk[0].ProFit[mk_gbn][item_seq][i].item_getcnt);
			}

			/* 평가손익 */
			Shm_Risk[0].ProFit[mk_gbn][item_seq][i].item_ver_prft = ver_tmp_d;	// 종목의 평가손익
			ACCNO(D_K,i).acc_ver_prft = 
				ACCNO(D_K,i).acc_ver_prft + (ver_tmp_d - b_ver_d);				// 계좌의 평가손익
		}
	}
	
    return;
}   /* End of Set_Sise ()   */

/*************************************************************************
	End of program (pa_9009_mp.c)
*************************************************************************/ 
