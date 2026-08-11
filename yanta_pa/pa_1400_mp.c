#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 주문체결처리
#	File	: pa_1400_mp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#if defined A1401
#define		DATA_SIZE	400
#elif defined A2401 || A3401
#define		DATA_SIZE	450
#endif
#include	"buf_struct.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		DATA_TIME	60 * 1000
#define		READ_MAX	1

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int			R_Cnt;
FILE_BUFF_FORMAT	R_Fmt[READ_MAX];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_1400_MP (void);
void	Analyze_Data (void);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_1400_MP ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_1400_MP (void)
/*----------------------------------------------------------------------*/
{
	int		rt, read_flag;

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
}	/* End of PA_1400_MP ()	*/

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
	int		i, j, rt, w_flag, target, mem_area_loc;
	char	W_Fmt[8192];
	char	W_DFmt[8192];
	char	tr_code[12];
	YSK_SETTLE_FMT	*s_fmt;

	target = -1;

	for (i = 0; i < R_Cnt; i++)
	{
		w_flag = 0;
		memset (tr_code, 0, sizeof (tr_code));
#if defined A1401
		memcpy (tr_code, &R_Fmt[i].Data[11], 11);
#elif defined A2401 || A3401
		s_fmt = (YSK_SETTLE_FMT *)R_Fmt[i].Data;
		memcpy (tr_code, &s_fmt->SettleData.TrCode, 11);
#endif

#if defined A1401
		/* 회원체결결과, 이곳에는 회원체결만 들어온다 */
		/* DATA FORMAT : 82 + 체결전문	*/
		/* Write는 체결전문만 넣는다.	*/
		if (memcmp (tr_code, "TTRTDP42301", 11) == 0)
		{
			KRX_NOTE_SETTLE_DATA		*dat	=
				(KRX_NOTE_SETTLE_DATA *)&R_Fmt[i].Data[0];

			mem_area_loc = 0;

			// 내가낸 주문은 CLIENT에 전달 */
			if ((memcmp (dat->MembershipItem+mem_area_loc+30, "A", 1) != 0) &&
				(memcmp (dat->MembershipItem+mem_area_loc+30, "C", 1) != 0) &&	
				(memcmp (dat->MembershipItem+mem_area_loc+30, "T", 1) != 0)	)
			{
//				Log (USR_ERROR, "Other Media Recv!! Check Plz [%3.3s] [%s]", dat->MembershipItem+mem_area_loc+30, dat->DataSeq);
			}
			else
			{
				memset (W_Fmt, 0x20, sizeof (W_Fmt));
				memcpy (&W_Fmt, &R_Fmt[i], sizeof(BUFF_RW_HEAD));
				memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)],
					&R_Fmt[i].Data[0], sizeof (KRX_NOTE_SETTLE_DATA));
				W_Fmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

				if (memcmp (dat->MembershipItem+mem_area_loc+46, "0000", 4) > 0)
				{
					target = (AtoIf (dat->MembershipItem+mem_area_loc+46, 2) - 1) * 10;
					target = AtoIf (dat->MembershipItem+mem_area_loc+48, 2) + target;

					/* 전략에 전달 */
					if (target > 0 && target <= 40)
					{
						/* 체결 */
						/* 채권 : 20 + DATA전문 */
						/* 파생 : 20 + 20+DATA전문 */
						memset (W_DFmt, 0x20, sizeof (W_DFmt));
						memcpy (W_DFmt, &R_Fmt[i], sizeof (BUFF_RW_HEAD));          // 70
						memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)],     "100140",   6);     // 응답(100120), 체결(100140)
						memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)+6],   "00",       2);     // 시장구분(00:채권, 01:파생)
						memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)+20], R_Fmt[i].Data, DATA_SIZE);
						W_DFmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

						rt = DSHM_W (target*10, (void *)&W_DFmt, 1);
						if (rt != 1)
						{
							Log (SAM_FATAL, "Dshm write fail[%s:%d]", ODN(D_K,P_K,target-1), rt);
							Exit_Process ();
						}
						Log (USR_OK, "DSHM Write OK [%s][%d] target[%d]", W_DFmt, strlen(W_DFmt), target);
					}
				}
				
				/* Client로 Data 전달 */
				rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);
				if (rt != 1)
				{
					Log (SAM_FATAL, "file write fail[%s:%d]", OFN(D_K,P_K,0), rt);
					Exit_Process ();
				}
			}
		}
#elif defined A2401 || A3401
		/* 회원체결결과, 이곳에는 회원체결만 들어온다 */
		/* DATA FORMAT : 20 + 체결전문	*/
		/* Write는 체결전문만 넣는다.	*/
		if (memcmp (tr_code, "TTRTDP21301", 11) == 0)		// TTRTDP42301(체결결과)
		{
			KRX_SETTLE_DATA *dat = (KRX_SETTLE_DATA *)&s_fmt->SettleData;

			mem_area_loc = 10;

			// 내가낸 주문은 CLIENT에 전달 */
			if ((memcmp (dat->MembershipItem+mem_area_loc+30, "A", 1) != 0) &&
				(memcmp (dat->MembershipItem+mem_area_loc+30, "C", 1) != 0) &&	
				(memcmp (dat->MembershipItem+mem_area_loc+30, "T", 1) != 0)	)
			{
//				Log (USR_ERROR, "Other Media Recv!! Check Plz [%1.1s] [%*.*s]", dat->MembershipItem+mem_area_loc+30, sizeof(KRX_SETTLE_DATA), sizeof(KRX_SETTLE_DATA), dat);
			}
			else
			{
				memset (W_Fmt, 0x20, sizeof (W_Fmt));
				memcpy (&W_Fmt, &R_Fmt[i], sizeof(BUFF_RW_HEAD));
				memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)], R_Fmt[i].Data, YSK_SETTLE_FMT_LEN);
				W_Fmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

				if (memcmp (dat->MembershipItem+mem_area_loc+46, "0000", 4) > 0)
				{
					target = (AtoIf (dat->MembershipItem+mem_area_loc+46, 2) - 1) * 10;
					target = AtoIf (dat->MembershipItem+mem_area_loc+48, 2) + target;
					/* 전략에 전달 */
					if (target > 0 && target <= MAX_AUTO_PROC)
					{




						memset (W_DFmt, 0x20, sizeof (W_DFmt));
						memcpy (W_DFmt, &R_Fmt[i], sizeof (BUFF_RW_HEAD));          // 70
						memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)],     "100140",   6);     // 응답(100120), 체결(100140)
//						시장구분(00:채권, 01:파생)
#if defined A2401
						memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)+6],   "01",       2);
#elif defined A3401
						memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)+6],   "01",       2);
#endif
						memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)+20], R_Fmt[i].Data, DATA_SIZE);
						W_DFmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

						rt = DSHM_W (target*10, (void *)&W_DFmt, 1);
						if (rt != 1)
						{
							Log (SAM_FATAL, "Dshm write fail[%s:%d]", ODN(D_K,P_K,target-1), rt);
							Exit_Process ();
						}
						Log (USR_OK, "DSHM Write OK [%200.200s][%d] target[%d]", W_DFmt, strlen(W_DFmt), target);
					}
				}
				
				/* Client로 Data 전달 */
				rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);
				if (rt != 1)
				{
					Log (SAM_FATAL, "file write fail[%s:%d]", OFN(D_K,P_K,0), rt);
					Exit_Process ();
				}
			}
		}
#endif
		else
			Log (USR_ERROR, "Else Case TR Recv Check Plz [%s] [%s]",
				tr_code, &R_Fmt[i].Data[0]);

		Add_Count (PS_R_1, 1);
	}	/* End of for */

}

/*************************************************************************
	End of Program (pa_1400_mp.c)
*************************************************************************/
