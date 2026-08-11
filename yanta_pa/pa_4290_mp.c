#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 주문접수처리(미체결내역관리)
#	File	: pa_4290_mp.c
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
void	PA_4290_MP (void);
void	Analyze_Data (void);
int		Make_MiChe(char *, int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_4290_MP ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_4290_MP (void)
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

			R_Cnt = F_R (PS_R_2, (void *)R_Fmt, READ_MAX);

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
				IFN(D_K,P_K,0), R_Cnt, IFR(D_K,P_K,0,1));
			Analyze_Data ();
		}

		rt = Poll_File (DATA_TIME);

		if (rt == 1)
			Log (USR_OK, "poll timeout <%d>", OFW(D_K,P_K,0,0));
		else if (rt == -1)
			continue;
	}
}	/* End of PA_4290_MP ()	*/

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
		rt = Make_MiChe(R_Fmt[i].Data, i);

		Add_Count (PS_R_2, 1);
	}

	return;
}	/* End of Analyze_Data ()	*/

/*************************************************************************
    Function        : . Make_MiChe
    Parameters IN   : . p_buf   : received data
					  . for_d	: data count
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . set Acc data SHM (손익/수수료/평가손익)
*************************************************************************/
/*----------------------------------------------------------------------*/
int   Make_MiChe (char *p_buf, int for_d)
/*----------------------------------------------------------------------*/
{
    int     i;
    int     real_ord_cnt, m_size;
    int     mk_gbn, item_seq, acc_seq;
	SMB_ST  *dat = (SMB_ST *)p_buf;

	/* FX 원달러 금액 소수점 2자리(double) */
	long	org_cnt, edt_cnt;
	double	org_gum, edt_gum;

	BUFF_RW_HEAD	f_head;

	/* ************************************************************************	*/
	/* 미체결내역 처리(미니원장)												*/
	/* ************************************************************************	*/
	/* 내가낸 주문만 처리한다.													*/
	/* ************************************************************************	*/
	/*   TTRODP11301(정산)만 처리한다.											*/
	/* ************************************************************************	*/
	/* - 한도는 신규주문시점에 잡는다.											*/
	/*          정정/취소는 회원처리호가 수신시점(여기서) 가감한다.				*/ 
	/* - 미체결관리는 아래 로직으로 처리한다.									*/
	/*   미체결등록은 회원처리호가 정상접수시에 한다.							*/
	/*   그래야 미접수분에 대한 정정/취소를 못하게 할 수 있다.					*/
	/*   미체결은 Main이 수량이다. 매수수량+매도수량.							*/
	/*            정정은 금액변경이 필수이고, 시스템트레이딩에서는				*/
	/*			  일부정정 하지않는다.(손으로 입력해야해서)						*/
	/* 20211108 처리로직 변경, 주문송신시 미체결수량 가감 선행한다.				*/
	/*          선행을 해야 중복주문이 안나간다.								*/
	/* ************************************************************************	*/
	/* 회원처리호가에 따른 미체결 처리방안(미체결은 "주문번호+계좌"=Key)		*/
	/* 1(신규) +(추가), 2(정정) 추가없음_증거금만변경, 3(취소) -(삭제및감소)	*/
	/* TTRODP11301(확인)시 미체결등록은 동일하고 미체결수량 가감만 변경			*/
	/* 					1(신규)/2(정정) : 수량은 가감없음, 3(취소) : -			*/
	/*					정정&매수&정정금액증가 : 처리수량만큼 증거금 증가		*/
	/*					정정&매수&정정금액감소 : 처리수량만큼 증거금 감소		*/ // 2025로직추가
	/* TTRODP11321(거부)시 미체결등록은 동일하나 미체결수량 가감만 처리			*/
	/* 					1(신규) -,2(정정)/2(취소) 가감없음						*/
	/* TTRODP11303(자동취소)시 (취소의 자동취소는 없다)							*/
	/*                  자동취소 수량만큼 감소 & 증거금 감소					*/
	/* ************************************************************************	*/
	/* 확인 처리 */
	if ( memcmp (dat->smb_MsgType, "8", 1) == 0 && memcmp (dat->smb_OrdStatus, "0", 1) == 0 )
	{

#if	1	/* HONG */
		mk_gbn		=	0;												// 시장구분
		item_seq	=	0;												// 종목 Seq(USDKRW)
		acc_seq		=	0;												// 계좌 seq : 계좌가 1개
#else	/* HONG */
		mk_gbn		=	AtoIf (dat->MembershipItem+imeco_gbn+35, 1);	// 시장구분
		item_seq	=	AtoIf (dat->MembershipItem+imeco_gbn+37, 5);	// 종목 Seq
		acc_seq		=	AtoIf (dat->MembershipItem+imeco_gbn+42, 2);	// 계좌 seq
#endif	/* HONG */

		/* 내가낸 주문만 처리한다 */
		if ( memcmp (&dat->smb_ClOrdID[18], "0000", 4) == 0 && memcmp (&dat->smb_ClOrdID[18], "    ", 4) == 0 )
		{
			Log (USR_ERROR, "타매체 수신 [%s]", dat);
			return (OK);
		}

		/* ******************************************************************************** */
		/* 강제종료 처리																	*/
		// 협의필요
		/* ******************************************************************************** */
		/* 정정/취소 주문이고																*/
		/* 원주문번호는 서버주문 && 주문번호는 CLIENT주문시 원주문번호의 자동주문 강제종료	*/
		/* ******************************************************************************** */
/* 
전략구축시 처리 또는 미처리 (CLIENT개입시 강제종료)
		for (i = 0; i < MAX_MICHE; i++)
		{
		}
*/

		/* ************************************************************************ */
		/*				응답(3291)의 미체결내역 관리								*/
		/* ------------------------------------------------------------------------ */
		/* 신규		정상 : 신규추가													*/
		/* 			거부 : SKIP (미체결SKIP, 한도감소)								*/
		/*          자취 : 감소	(미체결/한도 감소) 									*/
		/* ------------------------------------------------------------------------ */
		/* 정정		정상 : 정정주문 신규추가										*/
		/*				   원주문번호찾아서 주문잔량 감소(실제처리건수로)			*/
		/*				   증거금도 변경(증가시,감소시 고려해서, 한도포함)			*/
		/*			거부 : SKIP (미체결SKIP, 한도가감)								*/
		/*                 금액증가시 감소, 금액감소시 증가							*/
		/* 취소		정상 : 원주문번호찾아서 주문잔량 감소(실제처리건수로)   		*/
		/*                 한도포함													*/
		/* ************************************************************************ */
		/* 미체결 처리																*/
		/* - 신규/정정   : 등록처리한다. 이후 정정은 원주문번호 찾아서 수량을 조정	*/
		/* - 취소        : 원주문번호 찾아서 수량을 조정							*/
		/* ------------------------------------------------------------------------ */
		/* 신규 : 등록																*/
		/* 정정 : 등록/조정															*/
		/* 취소 :      조정															*/	
		/* ************************************************************************ */
		if ((memcmp (dat->smb_MsgType, "8", 1) == 0) && 
		    (memcmp (dat->smb_OrdStatus, "0", 1) == 0))
		{
			for (i = 0; i < MAX_MICHE; i++)
			{
				/* 신규로 등록 */
				if (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt <= 0)
				{
					Shm_Mk_PreMatch[0].FX_MeChe_Cnt[mk_gbn][acc_seq]++;

					/* int1. 주문잔량 */
 	         	    Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt = AtoIf (dat->smb_OrderQty, 30);
					/* int2. 시장구분 */
					Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Mk_gbn	= mk_gbn;
                    Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Item_Seq =  item_seq;   // 종목 Seq(USDKRW)
		            Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Acc_Seq  =  acc_seq;    // 계좌 seq
					/* 1. 계좌번호 */
					memcpy (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Account,		dat->smb_Account, 30);
					/* 2. 종목코드 */
					 memcpy (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Symbol,		dat->smb_Symbol,	7);
					 /* 3. 주문번호 */
					 memcpy (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].ClOrdID,		dat->smb_ClOrdID, 24);
					 
					 /* 4. 원주문번호 */
					memcpy (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].OrigClOrdID,	dat->smb_OrigClOrdID,		24);
					
					/* 5. 정정취소구분 */
					memcpy (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].OrdStatus,		dat->smb_OrdStatus,	1);
					/* 6. 매도매수구분 */
					memcpy (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Side,			dat->smb_Side,	1);
					/* 7. 주문수량 (&) 취소만 있음 */
					memcpy (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].OrderQty,		dat->smb_OrderQty,	30);
					
					/* 8. 주문가격 (&), 가격없는 주문유형 */
                    memcpy (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Price, 			dat->smb_Price,		30);
					edt_gum	= AtoLf (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Price, 30);
					edt_cnt	= AtoLf (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].OrderQty, 30);
					/* 9. 호가유형코드 */
					memcpy (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].OrdType,		dat->smb_OrdType, 1);
					/* 10. 호가조건  (0:일반, 3:IOC, 4:FOK) */
					memcpy (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].TimeInForce,	dat->smb_TimeInForce,	1);
					/* 11. 회원사처리항목 */
#if 0    /* HONG ---FX헤더에 매핑정보없음 설계자 확인 필요함 */
					memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].MembershipItem,	dat->MembershipItem,	m_size);
#endif   /* HONG */					
					Log (USR_OK, "MiChe 등록 Mem [%d][%30.30s] Acc_Seq[%d] JCNT[%30.30s] JN[%24.24s] OJN[%24.24s] MK[%d]",
						Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt,
						Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Account, acc_seq,
						Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].OrderQty,
						dat->smb_OrdID, dat->smb_OrigClOrdID, mk_gbn);

					break;
				}	/* End of 신규등록 */

				if (i >= MAX_MICHE)
				{
					Log (USR_ERROR, "미체결공간 10,000모두사요 10초후 재시도 시장[%d]", mk_gbn);
					sleep (10);
					i = -1;
				}
			}	/* End of for MAX_MICHE */
		}	/* Endof 신규 or 정정 */

		/* 원주문번호 찾아서 수량 조정해줘야 한다. 정정 없음 */
		if ((memcmp (dat->smb_MsgType, "8", 1) == 0) && 
		    (memcmp (dat->smb_OrdStatus, "0", 4) == 0))
		{
			for (i = 0; i < MAX_MICHE; i++)
			{
				/* 원주문번호 찾기 */
				if (memcmp (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].ClOrdID, dat->smb_OrigClOrdID, 24) == 0)
				{
					if (memcmp (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Account, dat->smb_Account, 30) != 0)
					{
						 Log (USR_ERROR, "주문주체와 원주문주체의 계좌가 다르다. 확인요망 OJN[%24.24s]",
							  dat->smb_OrigClOrdID);
						return (NOTOK);
					}																
					// 실제처리된 수량
					real_ord_cnt = AtoIf (dat->smb_LastQty, sizeof (dat->smb_LastQty));
					 /* 정정,취소e 원주문번호의 원래 주문가격, 수량 보관 */
					org_gum	= AtoLf (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Price, 30);
					org_cnt	= AtoLf (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].OrderQty, 30);

					if (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt - real_ord_cnt < 0)
					{
						Log (USR_ERROR, "확인 수량 부적합 [%d][%d] 주문번호 [%24.24s]",
						Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt, real_ord_cnt, dat->smb_OrigClOrdID);
						return (NOTOK);
					}

					/* 원주문 주문잔량 조정 */
					Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt =
						Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt - real_ord_cnt;

					if (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt <= 0)
						Shm_Mk_PreMatch[0].FX_MeChe_Cnt[mk_gbn][acc_seq]--;

					Log (USR_OK, "MiChe 조정 Mem [%d][%30.30s] acc_seq[%d] JN[%24.24s] O_JN[%24.24s] MK[%d]",
						Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt,
						Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Account, acc_seq,
						dat->smb_OrdID, dat->smb_OrigClOrdID, mk_gbn);
					break;
				}	/* End of 원주문번호 찾기 */

				if (i >= MAX_MICHE)
				{
					Log (USR_ERROR, "원주문번호 못찾았음. JN[%24.24s] O_JN[%24.24s] MK[%d]",
						dat->smb_OrdID, dat->smb_OrigClOrdID, mk_gbn);
					return (NOTOK);
				}
			}	/* End of for MAX_MICHE */

			/* ************************************************************************ */
			/* 기존에는 현물만 사용														*/
			/* 한도처리 로직(공매도는 대상제외, 공매도는 시스템영역아님 )				*/
			/* ------------------------------------------------------------------------ */
			/* < 신규확인 > Skip														*/
			/* < 정정확인 >  															*/
			/* 원주가격 < 정정가격이면 정정수량*차액 만큼 당일 매수주문금액에 더함.		*/
			/* 원주가격 > 정정가격이면 정정수량*차액 만큼 당일 매수주문금액에 뺌.		*/
			/* ------------------------------------------------------------------------ */
			/* 신규거부/취소확인 : 신규거부/취소확인전문 수신시,						*/
			/* 당일차감 매수 금액 증가(매수만)											*/
			/* ************************************************************************ */
		}	/* End of 정정 or 취소 */
	}	/* End of 접수확인 (TTRODP11301/TTRODP41301) */

/* 회원처리호가 거부/자동취소 처리 */
	else
    if ( memcmp (dat->smb_MsgType, "8", 1) == 0
	 && (memcmp (dat->smb_OrdStatus, "4", 1) == 0 || memcmp (dat->smb_OrdStatus, "8", 1) == 0))
	{
#if 0   /* HONG */
		 mk_gbn      =   AtoIf (dat->MembershipItem+imeco_gbn+35, 1);    // 시장구분
		 item_seq    =   AtoIf (dat->MembershipItem+imeco_gbn+37, 5);    // 종목 Seq
		 acc_seq     =   AtoIf (dat->MembershipItem+imeco_gbn+42, 2);    // 계좌 seq
#endif  /* HONG */

		/* ******************************************** */
		/* 한도처리 로직 								*/
		/* 신규 거부나면 당일차감 매수 금액 증가		*/
		/* ******************************************** */

		/* 20220323 자동취소시 미체결정리 추가 */
	 	if (memcmp (dat->smb_OrdStatus, "4", 1) == 0)
		{
			/* 주문번호 찾기 */
			for (i = 0; i < MAX_MICHE; i++)
			{
				if (memcmp (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].ClOrdID, dat->smb_ClOrdID, 24) == 0)
				{
					if (memcmp (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Account, dat->smb_Account, 30) != 0)
                    {
                        Log (USR_ERROR, "주문주체와 원주문주체의 계좌가 다르다. 확인요망 JN[%24.24s]", dat->smb_ClOrdID);
                        return (NOTOK);
                    }

                    /* 실제처리된 수량 처리 */
					Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt -=
						AtoIf (dat->smb_LastQty, sizeof (dat->smb_LastQty));

                    if (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt <= 0)
                        Shm_Mk_PreMatch[0].FX_MeChe_Cnt[mk_gbn][acc_seq]--;

					break;
				}	/* End of 주문번호 찾기 */

				if (i >= MAX_MICHE)
                {
                    Log (USR_ERROR, "주문번호 못찾았음. MK[%d] JN[%24.24s] acc_seq[%d]",
                        mk_gbn, dat->smb_ClOrdID, acc_seq);
                    return (NOTOK);
                }
			}	/* End of For */
		}
		/* 20220323 자동취소시 미체결정리 추가 */
	}   /* End of TTRODP41302, TTRODP41303 */

	return (OK);
}	/* End of MakeMiChe */

/*************************************************************************
	End of Program (pa_4290_mp.c)
*************************************************************************/

