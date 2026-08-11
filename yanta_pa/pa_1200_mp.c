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

#if defined A1201
#define		DATA_SIZE	400
#elif defined A2201 || A3201
#define		DATA_SIZE	450
#endif
#include	"buf_struct.h"

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
	int		i, j, s_k, rt, order_no, loop, mem_area_loc;
	double	STime, RTime;
	char	turn_time[20], m_time[24], t_time[12];
	char	W_Fmt[8192];
	char	W_DFmt[8192];
	char	tr_code[12], member_area[60];
	int		w_flag, target;
	BUFF_RW_HEAD	f_head;
#if defined A2201 || A3201		// 파생
	YSK_JUMUN_FMT	*j_fmt = (YSK_JUMUN_FMT *)R_Fmt[0].Data;
#endif

	w_flag = 0;
	target = -1;

Log (USR_OK, "R_Data[%s]", R_Fmt[0].Data);

	memset (member_area, 0, sizeof(member_area));

	/* 20250901 
		qr에서 수신받아서 ps_1101_ts의 기동이 안되어 있으면 1201mp에 write한다.
		이때 헤더에는 "REJC"으로 사유코드를 넣는다. 이는 테스트를 위함이다.
		계속사용할수도 있다(의사결정필요).
		pa_1101_ts의 세션으로 처리하면 문제가 된다. 그래서 기동여부로 체크한다.
	*/
	/* SKIP로직 */
	/* 읽는 데이터 포멧 : 0 + 주문전문 */
#if defined A1201		// 채권
	if (memcmp (R_Fmt[0].Data, "REJC", 4) == 0)		// CLIENT에게 주문거부 송신
#elif defined A2201 || A3201		// 파생
	if (memcmp (j_fmt->Header.sRpCode, "REJC", sizeof(j_fmt->Header.sRpCode)) == 0)	// CLIENT에게 주문거부 송신
#endif
	{
#if defined A1201		// 채권
		Log (USR_OK, "Reject SKIP Write OK ResponseCode[%4.4s]", R_Fmt[0].Data);
#elif defined A2201 || A3201		// 파생
		Log (USR_OK, "Reject SKIP Write OK ResponseCode[%4.4s]", j_fmt->Header.sRpCode);
#endif
		return (OK);
	}
	/* SKIP로직 */

#if defined A1201		// 채권
	mem_area_loc = 0;
#elif defined A2201 || A3201		// 파생
	mem_area_loc = 10;
#endif
	/* **************************************************************** */
	/* 파생은 IMECO와 주고받는다. DATA FORMAT이 다르다.					*/
	/* DATA FORMAT : 20 + 주문 또는 회원처리호가						*/
	/* 				 단, 체결은 20빼고 넣는다.							*/
	/* **************************************************************** */
	/* 채권은 KRX와 주고받는다. 										*/
	/* 응답으로는 KRX거부/KRX회원처리호가(신규,정정,취소)이 올수 있다.	*/
	/* DATA FORMAT : 4+11 + 0 + 주문 또는 회원처리호가					*/
	/*               주문은 주문거부임.	시장에서는 회원처리호가만 		*/
	/*               0는 거부는 있을것이고 회원처리호가나 체결은 별도의 공유메모리에서 관리하는걸 뽑아와야 */ // 202509신규
	/* 회원사용영역을 찾아야 한다 */
	/* **************************************************************** */
#if defined A1201
	// REJE : 호가정합성,RISK,착오매매에 의한 주문중단
	// 0000이외 : 시장에서 준 오류처리
	if ((memcmp (&p_buf[      ADD_HEADER_SIZE+ 11], "TCHODR4000", 10) == 0)	||	/* 채권일반호가(254) */
		(memcmp (&p_buf[4+11 +ADD_HEADER_SIZE+ 11], "TCHODR4000", 10) == 0))	/* 채권일반호가(254) */
	{
		KRX_NOTE_JUMUN_DATA *dat		= (KRX_NOTE_JUMUN_DATA *)&p_buf[4+11 +ADD_HEADER_SIZE];	
		memcpy (member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
Log (USR_OK, "R_DAT KRX_NOTE_JUMUN_DATA 01");
	}
	else
	if ((memcmp (&p_buf[      ADD_HEADER_SIZE+ 11], "TCHMOR4000", 10) == 0)	||	/* 채권LP일반호가(255) */
		(memcmp (&p_buf[4+11 +ADD_HEADER_SIZE+ 11], "TCHMOR4000", 10) == 0))	/* 채권LP일반호가(255) */
	{
		KRX_LP_NOTE_JUMUN_DATA *dat		= (KRX_LP_NOTE_JUMUN_DATA *)&p_buf[4+11 +ADD_HEADER_SIZE];	
		memcpy (member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
Log (USR_OK, "R_DAT KRX_LP_NOTE_JUMUN_DATA 02");
	}
	else
	if ((memcmp (&p_buf[      ADD_HEADER_SIZE+ 11], "TCHKOR1000", 10) == 0)	||	/* Kill Switch , KTS만 가능 (147) */
		(memcmp (&p_buf[4+11 +ADD_HEADER_SIZE+ 11], "TCHKOR1000", 10) == 0))	/* Kill Switch , KTS만 가능 (147) */
	{
		KRX_NOTE_KILLSWITCH_DATA *dat	= (KRX_NOTE_KILLSWITCH_DATA *)&p_buf[4+11 +ADD_HEADER_SIZE];	
		memcpy (member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
Log (USR_OK, "R_DAT KRX_NOTE_KILLSWITCH_DATA 03");
	}
	else
	if ((memcmp (&p_buf[      ADD_HEADER_SIZE+ 11], "TTRODP4130", 10) == 0)	||	/* 채권일반회원처리호가(291) */
		(memcmp (&p_buf[4+11 +ADD_HEADER_SIZE+ 11], "TTRODP4130", 10) == 0))	/* 채권일반회원처리호가(291) */
	{
		KRX_NOTE_SETTLE_RESP_DATA *dat	= (KRX_NOTE_SETTLE_RESP_DATA *)&p_buf[4+11 +ADD_HEADER_SIZE];	
		memcpy (member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
Log (USR_OK, "R_DAT KRX_NOTE_SETTLE_RESP_DATA 04");
	}
	else
	if ((memcmp (&p_buf[      ADD_HEADER_SIZE+ 11], "TTRMOP4130", 10) == 0) ||	/* 채권LP일반회원처리호가(295) */
		(memcmp (&p_buf[4+11 +ADD_HEADER_SIZE+ 11], "TTRMOP4130", 10) == 0))	/* 채권LP일반회원처리호가(295) */
	{
		KRX_LP_NOTE_SETTLE_RESP_DATA *dat		= (KRX_LP_NOTE_SETTLE_RESP_DATA *)&p_buf[4+11 +ADD_HEADER_SIZE];	
		memcpy (member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
Log (USR_OK, "R_DAT KRX_LP_NOTE_SETTLE_RESP_DATA 05");
	}
	else
	if ((memcmp (&p_buf[      ADD_HEADER_SIZE+ 11], "TTRKOP1130", 10) == 0) ||   /* KillSwitch, KTS만 가능 (161) */
		(memcmp (&p_buf[4+11 +ADD_HEADER_SIZE+ 11], "TTRKOP1130", 10) == 0))     /* KillSwitch, KTS만 가능 (161) */
	{
		KRX_NOTE_KILLSWITCH_DATA *dat	= (KRX_NOTE_KILLSWITCH_DATA *)&p_buf[4+11 +ADD_HEADER_SIZE];	
		memcpy (member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
Log (USR_OK, "R_DAT KRX_NOTE_KILLSWITCH_DATA 06");
	}
#elif defined A2201 || A3201
	if (memcmp (j_fmt->Header.sRpCode, "0000", sizeof(j_fmt->Header.sRpCode)) > 0)		// 주문거부(주문전문)
	{
		if (memcmp (j_fmt->JumunData.Transaction_Code, "TCHODR1000", 10) == 0)
		{
			KRX_JUMUN_DATA *dat	= (KRX_JUMUN_DATA *)&j_fmt->JumunData;	
			memcpy (member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
		}
		else
		{
			KRX_SETTLE_RESP_DATA *dat	= (KRX_SETTLE_RESP_DATA *)&j_fmt->JumunData;	
			memcpy (member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
		}
	}
	else												// 회원처리호가(확인/거부/자동취소)
	{
//		KRX_SETTLE_DATA *dat	= (KRX_SETTLE_DATA *)&j_fmt->JumunData;	
		KRX_SETTLE_RESP_DATA *dat	= (KRX_SETTLE_RESP_DATA *)&j_fmt->JumunData;	
		memcpy (member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
	}
#endif

	/* 내가 낸 주문은 Client에 전달 */
	if ((memcmp (&member_area[30+mem_area_loc], "A", 1) == 0)  ||
		(memcmp (&member_area[30+mem_area_loc], "C", 1) == 0)  ||
		(memcmp (&member_area[30+mem_area_loc], "T", 1) == 0))
	{
		w_flag = 1;
	}

Log (USR_OK, "member_area[%s] mem_area_loc=[%d]", member_area, mem_area_loc);
	if (memcmp (&member_area[46+mem_area_loc], "0000", 4) > 0)
	{
Log (USR_OK, "area0=[%.60s]", member_area);
		target = (AtoIf (&member_area[46+mem_area_loc], 2) - 1) * 10;
Log (USR_OK, "area1=[%.2s]", &member_area[46+mem_area_loc]);
		target =  AtoIf (&member_area[48+mem_area_loc], 2) + target;
Log (USR_OK, "area2=[%.2s]", &member_area[48+mem_area_loc]);
	}

Log (USR_OK, "target[%d]", target);

	/* Data 전달 */
	if (w_flag > 0)
	{
		memset (W_Fmt, 0x20, sizeof (W_Fmt));
		memcpy (W_Fmt, &R_Fmt[for_d], sizeof (BUFF_RW_HEAD));		// 70
		/* 회원처리호가 */ 
		memcpy (&W_Fmt[sizeof (BUFF_RW_HEAD)], R_Fmt[for_d].Data, DATA_SIZE);
		W_Fmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

		/* 전략에 전달 */
		if (target > 0 && target <= 40)
		{
			/* 전략에는 20+DATA로 내린다 */
			/* 회원처리호가 + 주문거부 */ 
			/* 채권 : 20 + 15+DATA전문 */
			/* 파생 : 20 + 20+DATA전문 */
			memset (W_DFmt, 0x20, sizeof (W_DFmt));
			memcpy (W_DFmt, &R_Fmt[for_d], sizeof (BUFF_RW_HEAD));			// 70
			memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)],		"100120",	6);		// 응답(100120), 체결(100140)
#if defined A1201
			memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)+6],	"00", 		2);		// 시장구분(00:채권, 01:파생)
#elif defined A2201 || A3201
			memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)+6],	"01", 		2);		// 시장구분(00:채권, 01:파생)
#endif
			memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)+20], R_Fmt[for_d].Data, DATA_SIZE);
			W_DFmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

			rt = DSHM_W (target*10, (void *)&W_DFmt, 1);
			if (rt != 1)
			{
				Log (SAM_FATAL, "Dshm write fail[%s:%d]", ODN(D_K,P_K,target-1), rt);
				Exit_Process ();
			}
			Log (USR_OK, "DSHM Write OK [%s][%d] target[%d]", W_DFmt, strlen(W_DFmt), target);
		}

		/* Client에 전달, To pa_8101_ts */
		rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);
		if (rt != 1)
		{
			Log (SAM_FATAL, "file write fail[%s:%d]", OFN(D_K,P_K,0), rt);
			Exit_Process ();
		}
	}

	return (OK);
}

/*************************************************************************
	End of Program (pa_1200_mp.c)
*************************************************************************/

