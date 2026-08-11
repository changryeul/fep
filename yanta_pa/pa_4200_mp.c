#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 주문접수처리
#	File	: pa_4200_mp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#define		DATA_SIZE	1024
#include	"buf_struct.h"
#include	"fx.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		DATA_TIME			60 * 1000
#define		READ_MAX			1
#define     ADD_HEADER_SIZE		0

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int			R_Cnt;
char		ApType[10];
FILE_BUFF_FORMAT	R_Fmt[READ_MAX];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_4200_MP (void);
void	Analyze_Data (void);
int		Check_rtn(char *, int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_4200_MP ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_4200_MP (void)
/*----------------------------------------------------------------------*/
{
	int		rt, read_flag;

	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
	LtoU (ApType, strlen (ApType));

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
					IFN(D_K,P_K,0), SYS_NO, SYS_STR);
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
}	/* End of PA_4200_MP ()	*/

/*************************************************************************
	Function		: . Analyze_Data
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . analyze and divide data
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Analyze_Data (void)
/*----------------------------------------------------------------------*/
{
	int		i, rt;

	for (i = 0; i < R_Cnt; i ++)
	{
		rt = Check_rtn(R_Fmt[i].Data, i);
		Add_Count (PS_R_1, 1);
	}

	return;
}	/* End of Analyze_Data ()	*/

/*************************************************************************
    Function        : . Check_rtn
    Parameters IN   : . p_buf   : received data
					  . for_d	: data count
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . set Acc data SHM (손익/수수료/평가손익)
*************************************************************************/
/*----------------------------------------------------------------------*/
int   Check_rtn (char *p_buf, int for_d)
/*----------------------------------------------------------------------*/
{
	SMB_ST	*dat = (SMB_ST *)p_buf;
	int		rt;
	int		w_flag, target;
	char	member_area[60];
	char	W_Fmt[8192];
	char    W_DFmt[8192];

	w_flag = 0;
	target = -1;

	/* **************************************************************** */
	/* FX는 원장에서 받는다.	 										*/
	/* 응답으로는 신규, 취소, 거부, 주문확인, 체결이 올수 있다.			*/
	/* DATA FORMAT : 1024 SMB_ST										*/
	/* **************************************************************** */
	memset (member_area, 0, sizeof(member_area));
	memcpy (member_area, &dat->smb_ClOrdID[18], 4);

	if (memcmp (&member_area, "0000", 4) != 0 && memcmp (&member_area, "    ", 4) != 0 )
	{
		w_flag = 1;
		target = (AtoIf (&member_area[0], 2) - 1) * 10;
		target =  AtoIf (&member_area[2], 2) + target;
	}

	/* Data 전달 */
	if (w_flag > 0)
	{
		memset (W_Fmt, 0x20, sizeof (W_Fmt));
		memcpy (&W_Fmt, &R_Fmt[for_d], sizeof (FILE_BUFF_FORMAT)-1);
		W_Fmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

		/* 전략에 전달 */
		if (target > 0 && target <= 40)
		{
            /* 전략에는 20+DATA로 내린다 */
			/* 회원처리호가 + 주문거부 */
			/* FX : 20 + DATA전문 */
            memset (W_DFmt, 0x20, sizeof (W_DFmt));
		    memcpy (W_DFmt, &R_Fmt[for_d], sizeof (BUFF_RW_HEAD));          // 70
		    memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)],		"100120",	6);     // 응답(100120), 체결(100140)
			memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)+6],   "04",       2);     // 시장구분(04:FX)

            memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)+20], R_Fmt[for_d].Data, DATA_SIZE);
			W_DFmt[sizeof(BUFF_RW_HEAD)+ODS(D_K,P_K,0)] = '\n';

			rt = DSHM_W (target*10, (void *)&W_DFmt, 1);
			if (rt != 1)
			{
				Log (SAM_FATAL, "Dshm write fail[%s:%d]", ODN(D_K,P_K,target-1), rt);
				Exit_Process ();
			}
			Log (USR_OK, "DSHM Write OK [%s][%d] target[%d]", W_DFmt, strlen(W_DFmt), target);
		}
	}

	return (OK);
}

/*************************************************************************
	End of Program (pa_4200_mp.c)
*************************************************************************/

