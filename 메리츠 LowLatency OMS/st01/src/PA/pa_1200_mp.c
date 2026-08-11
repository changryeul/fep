#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 주문접수처리
#	File	: pa_1200_mp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#define		DATA_SIZE	500
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
char		ApType[10];
FILE_BUFF_FORMAT	R_Fmt[READ_MAX];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_1200_MP (void);
void	Analyze_Data (void);
int		Check_rtn(char *, int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_1200_MP ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_1200_MP (void)
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
}	/* End of PA_1200_MP ()	*/

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
	int		i, j, s_k, rt, order_no, loop;
	double	STime, RTime;
	char	turn_time[20], m_time[24], t_time[12], W_Fmt[8192];
	char	tr_code[12];
	int		w_flag, target;
	BUFF_RW_HEAD	f_head;

	w_flag = 0;
	target = -1;

Log (USR_OK, "Check_rtn [%13.13s][%s]", &p_buf[SEARCH_HEADER_LEN+11], p_buf);
	/* 응답으로는 FEP거부/KRX거부/KRX확인(신규,정정,취소)이 올수 있다. */
	/* 이중 거부는 거부헤더가 있고 다른건 헤더가 없다 */
	if (memcmp (&p_buf[SEARCH_HEADER_LEN+11], "MERITZREJEC", 11) == 0)	// 내부주문거부(80 Byte 데이터헤더에 거부)
	{
		TCP_RJ_HEADER	*reject_head	=	(TCP_RJ_HEADER	*)&p_buf[SEARCH_HEADER_LEN];
		KRX_JUMUN_DATA	*dat			=	(KRX_JUMUN_DATA *)&p_buf[SEARCH_HEADER_LEN+TCP_REJE_HEAD_LEN];

		/* 내가 낸 주문은 Client에 전달 */
		if ((memcmp (dat->MembershipItem+30, "A", 1) == 0)	||
			(memcmp (dat->MembershipItem+30, "C", 1) == 0)	||
			(memcmp (dat->MembershipItem+30, "T", 1) == 0))
		{
			w_flag = 1;
			Log (USR_ERROR, "Inner Reject MUST Check [%s]", dat->DataSeq);
		}

		if (memcmp (dat->MembershipItem+47, "0000", 4) > 0)
		{
			target = (AtoIf (dat->MembershipItem+47, 2) - 1) * 10;
			target = AtoIf (dat->MembershipItem+49, 2) + target;
		}
Log (USR_OK, "OK01 target[%d]", target);
	}
	else
	{
Log (USR_OK, "OK02");
		memset (tr_code, 0, sizeof (tr_code));
		memcpy (tr_code, &p_buf[SEARCH_HEADER_LEN+11], 11);

		if ((memcmp (tr_code, "TTRODP11301", 11) == 0) ||		// TTRODP11301(정상), TTRODP11321(거부), TTRODP11303(자동취소)
			(memcmp (tr_code, "TTRODP11321", 11) == 0) ||
			(memcmp (tr_code, "TTRODP11303", 11) == 0)	)
		{
			KRX_SETTLE_RESP_DATA	*dat	=	(KRX_SETTLE_RESP_DATA *)&p_buf[SEARCH_HEADER_LEN];

			// 내가낸 주문은 CLIENT에 전달 */
			if ((memcmp (dat->MembershipItem+30, "A", 1) == 0)	||
				(memcmp (dat->MembershipItem+30, "C", 1) == 0)	||
				(memcmp (dat->MembershipItem+30, "T", 1) == 0)	)
			{
				w_flag = 2;
			}

			if (memcmp (dat->MembershipItem+47, "0000", 4) > 0)
			{
				target = (AtoIf (dat->MembershipItem+47, 2) - 1) * 10;
				target = AtoIf (dat->MembershipItem+49, 2) + target;
			}
Log (USR_OK, "OK03 target[%d]", target);
		}
		else
			Log (USR_ERROR, "Else Case Tr Recv Check Plz [%s] [%s]", tr_code, &p_buf[0]);
	}

	/* Data 전달 */
	if (w_flag > 0)
	{
		//memset (W_Fmt, 0, sizeof (W_Fmt));
		//memcpy (&W_Fmt, &R_Fmt[for_d], sizeof (FILE_BUFF_FORMAT));
		memset (W_Fmt, 0x20, sizeof (W_Fmt));
		memcpy (&W_Fmt, &R_Fmt[for_d], sizeof (FILE_BUFF_FORMAT)-1);
		W_Fmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

		/* 전략에 전달 */
		if (target > 0)
		{
			rt = DSHM_W (target*10, (void *)&W_Fmt, 1);
			if (rt != 1)
			{
				Log (SAM_FATAL, "Dshm write fail[%s:%d]", ODN(D_K,P_K,target-1), rt);
				Exit_Process ();
			}
			Log (USR_OK, "Write OK [%s][%d] target[%d]", W_Fmt, strlen(W_Fmt), target);
		}

		/* Client에 전달 */
		rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);
		if (rt != 1)
		{
			Log (SAM_FATAL, "file write fail[%s:%d]", OFN(D_K,P_K,0), rt);
			Exit_Process ();
		}
	}
	else
Log (USR_OK, "OK05");

	return (OK);
}

/*************************************************************************
	End of Program (pa_1200_mp.c)
*************************************************************************/

