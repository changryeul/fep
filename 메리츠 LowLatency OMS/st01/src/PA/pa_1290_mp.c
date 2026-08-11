#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 주문접수처리(미체결내역관리)
#	File	: pa_1290_mp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#if defined A1291
#define		MK_GBN		0	/* 0:현물 */
#elif defined A2291
#define		MK_GBN		1	/* 1:파생 */
#endif

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
FILE_BUFF_FORMAT	R_Fmt[READ_MAX];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_1290_MP (void);
void	Analyze_Data (void);
int		Make_MiChe(char *, int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_1290_MP ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_1290_MP (void)
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
				IFN(D_K,P_K,0), R_Cnt, IFR(D_K,P_K,1,0));
			Analyze_Data ();
		}

		rt = Poll_File (DATA_TIME);

		if (rt == 1)
			Log (USR_OK, "poll timeout <%d>", OFW(D_K,P_K,0,0));
		else if (rt == -1)
			continue;
	}
}	/* End of PA_1290_MP ()	*/

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
	int		i, j, rt, order_no, loop, mng_flag = 0;
	int		ord_cnt, real_ord_cnt, err_flag;
	int		mk_gbn, item_seq, acc_seq, trad_flag;
	char	tr_code[12];
	long	org_gum, org_cnt, edt_gum, edt_cnt;
	double	org_price_dv, edt_price_dv, edt_cnt_dv;
	BUFF_RW_HEAD	f_head;

	err_flag = 0;
	/* 처리할 데이터의 포맷이 다양해서 구분하여 처리한다. */
	memset (tr_code, 0, sizeof (tr_code));
	memcpy (tr_code, &p_buf[SEARCH_HEADER_LEN+11], 11);

	/* **************************************************************** */
	/* 미체결내역 처리(미니원장)										*/
	/* **************************************************************** */
	/* 내가낸 주문만 처리한다.											*/
	/* **************************************************************** */
	/*   TTRODP11301(정산)만 처리한다.									*/
	/* **************************************************************** */
	/* 20211108 처리로직 변경, 주문송신시 미체결수량 가감 선행한다.		*/
	/* 1(신규) +, 2(정정) 가감없음, 3(취소) -							*/
	/* TTRODP11301(확인)시 미체결등록은 동일하고 미체결수량 가감만 변경	*/
	/* 					1(신규)/2(정정) : 가감없음, 3(취소) : -			*/
	/* TTRODP11303(자동취소)	또는 									*/
	/* TTRODP11321(거부)시 미체결등록은 동일하고 미체결수량 가감만 변경	*/
	/* 					1(신규) -,2(정정)/2(취소) 가감없음				*/
	/* **************************************************************** */
	if (memcmp (tr_code, "TTRODP11301", 11) == 0)	// TTRODP11301(확인)
	{
		KRX_SETTLE_RESP_DATA	*dat	=
			(KRX_SETTLE_RESP_DATA *)&p_buf[SEARCH_HEADER_LEN];

		mk_gbn		=	AtoIf (dat->MembershipItem+35, 2);	// 시장구분
		item_seq	=	AtoIf (dat->MembershipItem+37, 5);	// 종목 Seq
		acc_seq		=	AtoIf (dat->MembershipItem+42, 2);	// 계좌 seq

		if (memcmp (dat->TradeFlag, "1", 1)	== 0)
			trad_flag	=	-1;
		else
			trad_flag	=	 1;
		/* ******************************************************** */
        /* MembershipItem => 30 byte부터 사용가능                   */
        /* ******************************************************** */
		/*  4, 5, 6, 7, 8 : 스프레드 근월물 종목Seq					*/
		/* 16,17,18,19,20 : 스프레드 원월물 종목Seq					*/
        /* 30 : A(서버자동주문), C(메리츠Client주문) T(윈웨이매체)  */
        /*    : 매체구분 (A:Auto or All, C:메리츠매체, T:윈웨이매체 */
        /*      A는 자동주문/주문응답,체결등 주문관련된건(TR100XXX) 양쪽매체에 보낸다.*/
        /*      Data Header 50 Byte중 20번째 1자리와 같이 사용한다(조회등) */
        /* 31,32,33,34 : 전략에서 사용                              */
        /* 35, 36 : 시장구분                                        */
        /*          1(지수선물) 2(지수옵션) 3(주식선물) 4(주식옵션) */
        /*          5(유가증권/ELW/ETF/ETN) 6(코스닥)               */
        /*          8(KRX300선물) 9(Kosdaq150선물)                  */
        /* 37,38,39,40,41 : A0 seq번호 ex) 236 => (00236)           */
        /* 42, 43 : 계좌번호 seq                                    */
        /* 44     : 시장구분이 05(유가증권) 이면서 ETF/ETN/ELW면 '1' */
        /*          아니거나 다른시장이면 '0'                       */
        /* 45     : 스프레드종목이면 1, 아니면 0                    */
        /* 46     : 주식선물 && 주식옵션에서 유가증권종목이면 '1', 코스닥종목이면 '2' */
        /*          나머지시장이면 '0'                              */
        /* 47,48,49,50 : ApType 4자리 (50101중 0101만) Set          */
        /* 51, 52 : 리스크시장seq(운용상품분류말고 index로 주세요)  */
        /*  (운용상품분류코드)  (index) (시장구분명)                */
        /*          11              0   코스피200선물
                    12              1   코스피200콜옵션
                    13              2   코스피200풋옵션
                    14              3   주식선물
                    15              4   주식콜옵션
                    16              5   주식풋옵션
                    20              6   코스닥150선물
                    44              7   코스닥150콜옵션
                    45              8   코스닥150풋옵션
                    46              9   KRX300선물
                    51              10  현물주식
                    34              11  미니코스피200선물
                    35              12  미니코스피200콜옵션
                    36              13  미니코스피200풋옵션         */
        /* ******************************************************** */

		/* 내가낸 주문만 처리한다 */
		if ((memcmp (dat->MembershipItem+30, "A", 1) != 0)	&&
			(memcmp (dat->MembershipItem+30, "C", 1) != 0)	&&
			(memcmp (dat->MembershipItem+30, "T", 1) != 0)	)
		{
			Log (USR_ERROR, "타매체 수신 [%30.30s] [%s]", dat->MembershipItem+30, dat->Seq);
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
		/*				응답(1291/2291)												*/
		/* ------------------------------------------------------------------------ */
		/* 신규		정상 : 신규추가													*/
		/* 			거부 : SKIP														*/
		/* ------------------------------------------------------------------------ */
		/* 정정		정상 : 신규추가													*/
		/*				   원주문번호찾아서 주문잔량 감소(실제처리건수로)			*/
		/*			거부 : SKIP														*/
		/* 취소		정상 : 원주문번호찾아서 주문잔량 감소(실제처리건수로)   		*/
		/* ************************************************************************ */
		/* 미체결 처리																*/
		/* - 신규/정정   : 등록처리한다. 이후 정정은 원주문번호 찾아서 수량을 조정	*/
		/* - 취소        : 원주문번호 찾아서 수량을 조정							*/
		/* ------------------------------------------------------------------------ */
		/* 신규 : 등록																*/
		/* 정정 : 등록/조정															*/
		/* 취소 :      조정															*/	
		/* ************************************************************************ */

		if ((memcmp (dat->New_Modify_Cancel_gbn, "1", 1) == 0)	||
			(memcmp (dat->New_Modify_Cancel_gbn, "2", 1) == 0)	)
		{
			for (i = 0; i < MAX_MICHE; i++)
			{
				/* 신규로 등록 */
				if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt <= 0)
				{
					Shm_Mk_PreMatch[0].MeChe_Cnt[mk_gbn][acc_seq]++;

					/* int1. 주문잔량 */
					if (memcmp (dat->New_Modify_Cancel_gbn, "1", 1) == 0)
						Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt = AtoIf (dat->OrderQuantity, 10);
					else
						Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt = AtoIf (dat->Real_Modify_Cancel_Cnt, 	10);
					/* int2. 시장구분 */
					Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Mk_gbn	= AtoIf (dat->MembershipItem+35, 2);
					/* int3. 종목일련번호 */
					Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Item_Seq	= AtoIf (dat->MembershipItem+37, 5);
					/* int4. 계좌일련번호 */
					Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Acc_Seq	= AtoIf (dat->MembershipItem+42, 2);
					/* int5. 전략번호 */
					//Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Str_No	= AtoIf (dat->MembershipItem+33, 2);

					/* 1. 계좌번호 */
					memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].AccountNo,		dat->AccountNo, 12);
					/* 2. 종목코드 */
					memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Item_Cd,			dat->ItemCode,	12);
					/* 3. 주문번호 */
					memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo,			dat->OrderNo,	10);
					/* 4. 원주문번호 */
					memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OriginalOrderNo,	dat->OriginalOrderNo,		10);
					/* 5. 정정취소구분 */
					memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderFlag,		dat->New_Modify_Cancel_gbn,	1);
					/* 6. 매도매수구분 */
					memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].TradeFlag,		dat->TradeFlag,	1);
					/* 7. 주문수량 (&) */
					if (memcmp (dat->New_Modify_Cancel_gbn, "1", 1) == 0)
						memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt,	&dat->OrderQuantity[2],				8);
					else
						memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt,	&dat->Real_Modify_Cancel_Cnt[2],	8);
					/* 8. 주문가격 (&) */
#if defined A1291
					if ((memcmp (dat->Order_Type, "1", 1) == 0)    ||  // 시장가
						(memcmp (dat->Order_Type, "I", 1) == 0)    ||  // 조건부 지정가
						(memcmp (dat->Order_Type, "X", 1) == 0)    ||  // 최유리 지정가
						(memcmp (dat->Order_Type, "Y", 1) == 0))       // 최우선 지정가
					{
						if (memcmp (dat->TradeFlag, "1", 1) == 0)
							if (mk_gbn == 5)
								memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price, Shm_Stock[item_seq].A0.low_limit_price, 9);
							else
								memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price, Shm_Kosdaq[item_seq].A0.low_limit_price, 9);
						else
							if (mk_gbn == 5)
								memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price, Shm_Stock[item_seq].A0.high_limit_price, 9);
							else
								memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price, Shm_Kosdaq[item_seq].A0.high_limit_price, 9);
					}
					else
						memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price,	&dat->Price[2],	9);

					/* 한도체크용 정정주문 금액한도감소용 사전처리 */
					if (memcmp (dat->New_Modify_Cancel_gbn, "2", 1) == 0)
					{
						edt_gum = AtoLf (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price, 9);
						edt_cnt = AtoLf (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt, 8);
/*
						edt_gum = AtoLf (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt, 8)
								* AtoLf (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price, 9);
*/
					}
#elif defined A2291
					if (memcmp (dat->MembershipItem+45, "1", 1) != 0)	// !스프레드
					{
						/* 스프레드가 아니면 미체결 수량과 금액을 감소시켜줘야한다. 금액감소를 위한 사전작업 */
						org_price_dv = edt_price_dv = edt_cnt_dv = 0;
// 주문수량*거래승수*금액
						if ((memcmp (dat->Order_Type, "T", 1) == 0)    ||	// 시장가(1대신 T)
							(memcmp (dat->Order_Type, "W", 1) == 0))		// 최유리 지정가(X대신 W)
						{
							Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_dv = Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_h_lmt;
						}
						else
							Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_dv = AtoDf (&dat->Price[2], 9);

						if (memcmp (dat->New_Modify_Cancel_gbn, "2", 1) == 0)
						{
							edt_price_dv = Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_dv;
							edt_cnt_dv   = AtoDf (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt, 8);
						}
					}
					else			// 스프레드, 미체결 수량만처리하고 금액은 처리하지 않는다.
					{
					}
#endif
					/* 9. 호가유형코드 */
					memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderType,		dat->Order_Type,1);
					/* 10. 호가조건 (0:일반, 3:IOC, 4:FOK) */
					memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].JumunFlag,		dat->Order_Condition,	1);
					/* 11. 회원사처리항목 */
					memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].MembershipItem,	dat->MembershipItem,	60);
					
					Log (USR_OK, "MiChe 등록 Mem [%d][%12.12s] acc_seq[%d] JCNT[%10.10s] JN[%10.10s] OJN[%10.10s] MK[%d]",
						Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt,
						Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].AccountNo, acc_seq,
						Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt,
						dat->OrderNo, dat->OriginalOrderNo, MK_GBN);

					break;
				}	/* End of 신규등록 */

				if (i > 9999)
				{
					Log (USR_ERROR, "미체결공간 10,000모두사요 10초후 재시도 시장[%d]", MK_GBN);
					sleep (10);
					i = -1;
				}
			}	/* End of for MAX_MICHE */
		}	/* Endof 신규 or 정정 */

		/* 원주문번호 찾아서 수량 조정해줘야 한다 */
		if ((memcmp (dat->New_Modify_Cancel_gbn, "2", 1) == 0)	||
			(memcmp (dat->New_Modify_Cancel_gbn, "3", 1) == 0)	)
		{
			for (i = 0; i < MAX_MICHE; i++)
			{
				/* 원주문번호 찾기 */
				if (memcmp (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo, dat->OriginalOrderNo, 10) == 0)
				{
					if (memcmp (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].AccountNo, dat->AccountNo, 12) != 0)
					{
						Log (USR_ERROR, "주문주체와 원주문주체의 계좌가 다르다. 확인요망 OJN[%10.10s]",
							dat->OriginalOrderNo);
						return (NOTOK);
					}

					// 실제처리된 수량
					real_ord_cnt = AtoIf (dat->Real_Modify_Cancel_Cnt, sizeof (dat->Real_Modify_Cancel_Cnt));
#if defined A1291
					/* 한도체크용 정정주문한도 */
					if (memcmp (dat->New_Modify_Cancel_gbn, "2", 1) == 0)
					{
						org_gum = AtoLf (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price, 9);
/* 						
						org_gum = AtoLf (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt, 8)
								* AtoLf (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price, 9);
*/
					}
					else if (memcmp (dat->New_Modify_Cancel_gbn, "3", 1) == 0)
					{
						/* 취소시 원주문금액으로 계산 */
						org_gum = AtoLf (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price, 9);
						org_cnt = AtoLf (&dat->Real_Modify_Cancel_Cnt[2], 8);
					}
#elif defined A2291
					/* 정정,취소된 원주문번호의 원래 주문가격, 수량 보관 */
					org_price_dv = Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_dv;
#endif
					if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt - real_ord_cnt < 0)
					{
						Log (USR_ERROR, "확인 수량 부적합 [%d][%d] 주문번호 [%10.10s]",
							Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt, real_ord_cnt, dat->OriginalOrderNo);
						return (NOTOK);
					}

					/* 주문잔량 조정 */
					Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt =
						Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt - real_ord_cnt;

					if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt <= 0)
						Shm_Mk_PreMatch[0].MeChe_Cnt[mk_gbn][acc_seq]--;

					Log (USR_OK, "MiChe 조정 Mem [%d][%12.12s] acc_seq[%d] JN[%10.10s] O_JN[%10.10s] MK[%d]",
						Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt,
						Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].AccountNo, acc_seq,
						dat->OrderNo, dat->OriginalOrderNo, MK_GBN);

					break;
				}	/* End of 원주문번호 찾기 */

				if (i > 9999)
				{
					Log (USR_ERROR, "원주문번호 못찾았음. MK[%d] JN[%10.10s] O_JN[%10.10s] MK[%d]",
						MK_GBN, dat->OrderNo, dat->OriginalOrderNo, MK_GBN);
					return (NOTOK);
				}
			}	/* End of for MAX_MICHE */

#if defined A1291
			/* **************************************************** */
			/* 한도처리 로직(매수만)								*/
			/* ---------------------------------------------------- */
			/* 정정확인 : 정정확인 전문 수신시 , 					*/
			/* 원주문증거금 < 정정증거금 이면 증가 정정시에만(org)	*/
			/* 원주가격 < 정정가격이면 정정수량*차액 만큼(수정본)	*/
			/* 당일 매수주문금액에 더함.(감소정정시에는 반영없음)	*/
			/* ---------------------------------------------------- */
			/* 신규거부/취소확인 : 신규거부/취소확인전문 수신시,	*/
			/* 당일차감 매수 금액 증가(매수만)						*/
			/* **************************************************** */
			if (memcmp (dat->TradeFlag, "2", 1) == 0)		// 매수
			{
				if (memcmp (dat->New_Modify_Cancel_gbn, "2", 1) == 0)
				{
					if (edt_gum > org_gum)
					{
						/* 매수주문금액 누적(차액*수량) */
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_tot_sugum += ((edt_gum-org_gum) * real_ord_cnt);
					}
				}
				else if (memcmp (dat->New_Modify_Cancel_gbn, "3", 1) == 0)
				{
					/* 매수당일차감금액 누적 */
					Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_cha_sugum += (org_gum * real_ord_cnt);
				}
			}
			else											// 매도
			{
				/* 매도시 정정/취소의 수량만큼 감소처리 한다. */
				Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt -= real_ord_cnt;
			}
#elif defined A2291
			if (memcmp (dat->MembershipItem+45, "1", 1) != 0)	// !스프레드, 정정시 미체결 수량X, 차액만 수정
																//            취소시 미체결 수량 && 금액 수정
			{
				if (memcmp (dat->TradeFlag, "2", 1) == 0)			// 매수
				{
					if (memcmp (dat->New_Modify_Cancel_gbn, "2", 1) == 0)		// 정정, 일부정정없다(차액만큼 뺀다)
					{
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum -= real_ord_cnt * (org_price_dv - edt_price_dv) * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier;
					}
					else														// 취소, 일부취소없다.(원주문금액만큼 뺀다)
					{
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt -= real_ord_cnt;
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum -= real_ord_cnt * org_price_dv * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier;
					}
				}
				else												// 매도
				{
					if (memcmp (dat->New_Modify_Cancel_gbn, "2", 1) == 0)		// 정정, 일부정정없다(차액만큼 뺀다)
					{
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum -= real_ord_cnt * (org_price_dv - edt_price_dv) * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier;
					}
					else														// 취소, 일부취소없다.(원주문금액만큼 뺀다)
					{
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt -= real_ord_cnt;
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum -= real_ord_cnt * org_price_dv * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier;
					}
				}
			}
			else												// 스프레드, 미체결 수량만(근/원월물 각각)
			{
				if (memcmp (dat->New_Modify_Cancel_gbn, "2", 1) == 0)	// 취소, 일부취소없다(취소된만큼 뺀다)
				{
					if (memcmp (dat->TradeFlag, "2", 1) == 0)				// 매수
					{
						// 근월물
						Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+4,  5)][acc_seq].item_su_michecnt -= real_ord_cnt;
						// 원월물
						Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+16, 5)][acc_seq].item_su_michecnt -= real_ord_cnt;
					}
					else													// 매도
					{
						// 근월물
						Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+4,  5)][acc_seq].item_do_michecnt -= real_ord_cnt;
						// 원월물
						Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+16, 5)][acc_seq].item_do_michecnt -= real_ord_cnt;
					}
				}
			}
#endif
		}	/* End of 정정 or 취소 */
				
	}	/* End of TTRODP11301 */
	else
	if ((memcmp (tr_code, "TTRODP11321", 11) == 0)	||	// TTRODP11301(확인), TTRODP11321(거부), TTRODP11303(자동취소)
		(memcmp (tr_code, "TTRODP11303", 11) == 0))
	{
		KRX_SETTLE_RESP_DATA	*dat	=
			(KRX_SETTLE_RESP_DATA *)&p_buf[SEARCH_HEADER_LEN];

		mk_gbn		=	AtoIf (dat->MembershipItem+35, 2);	// 시장구분
		item_seq	=	AtoIf (dat->MembershipItem+37, 5);	// 종목 Seq
		acc_seq		=	AtoIf (dat->MembershipItem+42, 2);	// 계좌 seq

#if defined A1291
		/* ******************************************** */
		/* 한도처리 로직 								*/
		/* 신규 거부나면 당일차감 매수 금액 증가		*/
		/* ******************************************** */
		if (memcmp (dat->TradeFlag, "2", 1) == 0)		// 매수
		{
			if (memcmp (dat->New_Modify_Cancel_gbn, "1", 1) == 0)	// 신규
			{
				if ((memcmp (dat->Order_Type, "1", 1) == 0)    ||  // 시장가
					(memcmp (dat->Order_Type, "I", 1) == 0)    ||  // 조건부 지정가
					(memcmp (dat->Order_Type, "X", 1) == 0)    ||  // 최유리 지정가
					(memcmp (dat->Order_Type, "Y", 1) == 0))       // 최우선 지정가
				{
					if (memcmp (dat->TradeFlag, "1", 1) == 0)
						if (mk_gbn == 5)
							edt_gum = AtoLf (Shm_Stock[item_seq].A0.low_limit_price, 9);
						else
							edt_gum = AtoLf (Shm_Kosdaq[item_seq].A0.low_limit_price, 9);
					else
						if (mk_gbn == 5)
							edt_gum = AtoLf (Shm_Stock[item_seq].A0.high_limit_price, 9);
						else
							edt_gum = AtoLf (Shm_Kosdaq[item_seq].A0.high_limit_price, 9);
				}
				else
					edt_gum = AtoLf (dat->Price, 11);

				edt_cnt = AtoLf (dat->OrderQuantity, 10);

				/* 매수당일차감금액 누적 */
				Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_cha_sugum += (edt_gum * edt_cnt);
			}
		}
		else											// 매도
		{
			if (memcmp (dat->New_Modify_Cancel_gbn, "1", 1) == 0)	// 신규
			{
				/* 매도시 거부된 주문수량만 감소처리 한다. */
				Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt -= AtoIf (&dat->OrderQuantity[2], 8);
			}
		}
#elif defined A2291
		if (memcmp (dat->MembershipItem+45, "1", 1) != 0)	// !스프레드, 미체결 수량 && 금액 감소
		{
			if (memcmp (dat->New_Modify_Cancel_gbn, "1", 1) == 0)	// 신규
			{
				/* 주문가격 */
				edt_price_dv = 0;
				if ((memcmp (dat->Order_Type, "T", 1) == 0)    ||	// 시장가(1대신 T)
					(memcmp (dat->Order_Type, "W", 1) == 0))		// 최유리 지정가(X대신 W)
				{
					edt_price_dv = Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_h_lmt;
				}
				else
					edt_price_dv = AtoDf (&dat->Price[2], 9);

				/* 거부된 주문수량 && 금액 감소처리 한다 */
				if (memcmp (dat->TradeFlag, "2", 1) == 0)			// 매수
				{
					Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt  -= AtoIf (&dat->OrderQuantity[2], 8);
					Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum -= AtoIf (&dat->OrderQuantity[2], 8) * edt_price_dv * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier;
				}
				else												// 매도
				{
					Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt  -= AtoIf (&dat->OrderQuantity[2], 8);
					Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum -= AtoIf (&dat->OrderQuantity[2], 8) * edt_price_dv * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier;
				}
			}
		}
		else												// 스프레드
		{
			if (memcmp (dat->New_Modify_Cancel_gbn, "1", 1) == 0)	// 신규
			{
				/* 스프레드는 거부된 주문수량만 감소처리 한다. */
				if (memcmp (dat->TradeFlag, "2", 1) == 0)			// 매수
				{
					Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt  -= AtoIf (&dat->OrderQuantity[2], 8);
				}
				else
				{
					Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt  -= AtoIf (&dat->OrderQuantity[2], 8);
				}
			}
		}
#endif
	}	/* End of TTRODP11321 */

	return (OK);
}	/* End of MakeMiChe */

/*************************************************************************
	End of Program (pa_1290_mp.c)
*************************************************************************/

