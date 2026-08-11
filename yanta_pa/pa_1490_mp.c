#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 체결처리
#	File	: pa_1490_mp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#if defined A1491 || A1492
#define		MK_GBN		0	/* 0:채권 */
#define		DATA_SIZE	400
#elif defined A2491 || A2492 || A3491 || A3492
#define		MK_GBN		1	/* 1:파생 */
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
int			R_Cnt, R_Max;
char		ApType[10];
FILE_BUFF_FORMAT	W_Fmt, R_Fmt[READ_MAX];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
int		Init_Parameters (void);
void	Write_Data (int);
void	PA_1490_MP (void);
void	Analyze_Data (void);
int		Che_Rtn(char *, double, int, int, int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_1490_MP ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_1490_MP (void)
/*----------------------------------------------------------------------*/
{
	int		rt, read_flag;

	rt = Init_Parameters ( );

	while (START_S != JOB_END)
	{
		Stat_Save ();

		while (START_S != JOB_END)
		{
			Stat_Save ();
			memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * READ_MAX);

			R_Cnt = F_R (PS_R_2, (void *)R_Fmt, 1);

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
			Add_Count (PS_R_2, 1);
		}

		rt = Poll_File (DATA_TIME);

		if (rt == 1)
			Log (USR_OK, "poll timeout <%d>", OFW(D_K,P_K,0,0));
		else if (rt == -1)
			continue;
	}
}	/* End of PA_1490_MP ()	*/

/*************************************************************************
    Function        : . Init_Parameters
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int
    Comment         : . All Program Parameters Init
*************************************************************************/
/*----------------------------------------------------------------------*/
int    Init_Parameters ()
/*----------------------------------------------------------------------*/
{
	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU (ApType, strlen (ApType));

	return	(OK);
}

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
	int		i, rt, che_cnt, retry_cnt, lp_gbn;
	int		mk_gbn, item_seq, acc_seq, mem_area_loc;
	double	od_price;
	char	tr_code[12];
#if defined A2491 || A2492 || A3491 || A3492
    YSK_SETTLE_FMT	*s_fmt = (YSK_SETTLE_FMT *)R_Fmt[0].Data;
#endif

	/* 처리할 데이터의 포맷이 다양해서 구분하여 처리한다.	*/
	/* 채권은 DATA전문만있고 파생은 20(헤더)있음			*/
	memset (tr_code, 0, sizeof (tr_code));

	/* ******************************************************** */
	/* 주문응답/체결의 도치현상처리방안							*/
	/* -------------------------------------------------------- */
	/* 체결Data에 해당하는 주문번호가 없으면 3초 Sleep후 재처리 */
	/* 3회(3초*3회)후 ERROR LOG && SKIP							*/
	/* ******************************************************** */

	retry_cnt = 0;
	/* 회원체결결과 */
#if defined A1491 || A1492
	memcpy (tr_code, &R_Fmt[0].Data[11], 11);
	mem_area_loc = 0;

	/* 채권은 체결전문이 동일하다. LP인지 일반인지 구분안됨. 계좌로 구분하자 */
	if (memcmp (tr_code, "TTRTDP42301", 11) == 0)			// 317
#elif defined A2491 || A2492 || A3491 || A3492
	memcpy (tr_code, s_fmt->SettleData.TrCode, 11);
	mem_area_loc = 10;

	if (memcmp (tr_code, "TTRTDP21301", 11) == 0)			// 체결결과 
#endif
	{
#if defined A1491 || A1492
		KRX_NOTE_SETTLE_DATA *dat =
			(KRX_NOTE_SETTLE_DATA *)R_Fmt[0].Data;
#elif defined A2491 || A2492 || A3491 || A3492
		KRX_SETTLE_DATA *dat =
			(KRX_SETTLE_DATA *)&s_fmt->SettleData;

#endif

		/* 내가낸 주문만 처리한다 */
		if ((memcmp (dat->MembershipItem+mem_area_loc+30, "A", 1) != 0) &&
			(memcmp (dat->MembershipItem+mem_area_loc+30, "C", 1) != 0) &&
			(memcmp (dat->MembershipItem+mem_area_loc+30, "T", 1) != 0) )
		{
			Log (USR_OK, "01.타매체 수신 [%20.20s] [%s]", dat->MembershipItem+mem_area_loc+30, dat);
			return;
		}

		mk_gbn      =   AtoIf (dat->MembershipItem+mem_area_loc+35, 1) - 1;  // 시장구분
        item_seq    =   AtoIf (dat->MembershipItem+mem_area_loc+37, 5);  // 종목 Seq
        acc_seq     =   AtoIf (dat->MembershipItem+mem_area_loc+42, 2);  // 계좌 seq

#if	0
		if (memcmp (dat->MembershipItem+mem_area_loc+35, "02", 2) == 0)
			lp_gbn = 1;
		else
			lp_gbn = 0;
#else
		lp_gbn = 0;
#endif
		/* ******************************************************** */
        /* MembershipItem => 30 byte부터 사용가능, 					*/
        /* 파생(IMECO는 20바이트만 준다.)                           */
        /*     따라서 스프레드는 사용못한다.                        */
		/* 스프래드 미사용 											*/
#if 0
  		/* 스프레드근월물 : 4부터 5자리 추가						*/
  		/* 스프레드원월물 : 16부터 5자리 추가						*/
#endif
		/* 아래 숫자는 Arry값으로 치환되어 사용한다.				*/
        /* ******************************************************** */
		/* 4,5,6,7,8 : 스프레드주문일경우만 사용, 근월물의 종목SEQ를 넣는다 */
		/* 16,17,18,19,20 : 
  					스프레드주문일경우만 사용, 원월물의 종목SEQ를 넣는다.) ex) "00032" */
        /* 30 : A(서버자동주문), C(메리츠Client주문) T(윈웨이매체)	*/
		/*    : 매체구분 (A:Auto or All, C:메리츠매체, T:윈웨이매체 */
		/*      A는 자동주문/주문응답,체결등 주문관련된건(TR100XXX) 양쪽매체에 보낸다.*/
		/*      Data Header 50 Byte중 20번째 1자리와 같이 사용한다(조회등) */
        /* 31,32,33,34 : 전략에서 사용								*/
		/* 35, 36 : 시장구분은 35 1자리만으로 체크                  */
        /*  (운용상품분류코드)  (시장index) (운영상품index) (시장구분명)*/
        /*			01                   0           1      채권일반
					02                   0           2      채권LP
					11                   1           1      금융파생_국채선물
					12                   1           2      금융파생_통화선물
					13                   1           3      금융파생_금리선물
					21                   2           1      금융파생_국채선물(야)
					22                   2           2      금융파생_통화선물(야)
					23                   2           3      금융파생_금리선물(야)
        */
        /* 37,38,39,40,41 : A0 seq번호 ex) 236 => (00236)           */
        /* 42, 43 : 계좌번호 seq                                    */
		/*          LP용계좌와 일반계좌를 구분해서 사용해야함.		*/
		/*			예, seq구분을 0~9(일반), 10~19(LP계좌)			*/
		/*			    #define     ACC_NO_CNT	20	//사용계좌수	*/
        /* 44     : 시장구분이 05(유가증권) 이면서 ETF/ETN/ELW면 '1' */
        /*          아니거나 다른시장이면 '0'                       */
        /* 45     : 스프레드종목이면 1, 아니면 0					*/
        /* 46     : 주식선물 && 주식옵션에서 유가증권종목이면 '1', 코스닥종목이면 '2' */
        /*          나머지시장이면 '0' : 사용안함                              */
        /* 46,47,48,49 : ApType 4자리 (50101중 0101만) Set          */
        /* ******************************************************** */
	
		/* 체결된 주문번호 찾기 */
		for (i = 0; i < MAX_MICHE; i++)
        {
			/* 해당 주문번호를 찾고 주문잔량은 감소시켜준다 */
			if (memcmp (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo, dat->OrderNo, 10) == 0)
			{
				Log (USR_OK, "OK Che Search [%10.10s] i[%d] acc[%d]", dat->OrderNo, i, acc_seq);

				/* 주문잔량 조정 */
				che_cnt = AtoIf(&dat->Trading_Volumn[1], sizeof (dat->Trading_Volumn)-1);
				if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt - che_cnt < 0)
				{
					Log (USR_ERROR, "체결 주문수량 부적합 주문번호 [%10.10s] 주문잔량[%d] 체결수량[%d]",
						dat->OrderNo, Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt, che_cnt);
					return;
				}
				Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt =
					Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt - che_cnt;

				if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt <= 0)
					Shm_Mk_PreMatch[0].MeChe_Cnt[mk_gbn][acc_seq]--;

				od_price = 0.0;
#if defined A1491 || A1492
				if (lp_gbn)												// 채권LP, 주문가격이 다르다. 매도수 주문이 있기 때문에 채권용 가격은 다르다.
				{
					if (memcmp (dat->TradeFlag, "2", 1) == 0)		// 채권매수, 매도수도 체결 매수매도는 구분해서준다.
						od_price = AtoDf (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_Lp02, 9);
					else													// 채권매도, 매도수도 체결 매수매도는 구분해서준다.
						od_price = AtoDf (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_Lp01, 9);
				}
				else													// 채권일반
				{
					od_price = Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_dv;
				}
#else
				od_price = Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_dv;
#endif
				break;
			}

			/* 회원처리호가(주문번호)가 아직 안들어왔으면 3초 쉬고 다시한다(3회) */
			/* 그래도 안들어오면 2번(재처리)로 뺀다								 */
			if (i >= MAX_MICHE)
			{
				if (retry_cnt > 3)
				{
					retry_cnt = 0;
#if defined A1491 || A2491 || A3491
					Log (USR_ERROR, "체결주문번호 찾지못했음 재처리로 Write, 주문응답 미처리 JN[%10.10s]",
						dat->OrderNo);

					// 1. W_Fmt 초기화
					memset(&W_Fmt, 0, sizeof(FILE_BUFF_FORMAT));

					// 2. R_Fmt[0]을 W_Fmt로 복사
					memcpy(&W_Fmt, &R_Fmt[0], sizeof(FILE_BUFF_FORMAT));
    				W_Fmt.LineFeed[0] = '\n';

					/* 1491,2492 Write 처리 */
					rt = F_W(TS_W2_1, (void *)&W_Fmt, 1);
					if (rt != 1)
					{
						Log (SAM_FATAL, "file write fail [%s]", OFN(D_K,P_K,1));
					}
#endif
					return;
				}

				i = -1;
				retry_cnt ++;
				Log (USR_OK, "체결주문번호 찾지못했음 retry[%d]회 주문응답 미처리 JN[%10.10s]",
						retry_cnt, dat->OrderNo);
#if defined A1491 || A2491 || A3491		// 1번째
				sleep (3);
#else									// 2번째 재처리
				sleep (60);
#endif
			}
		}	/* End of for */
				
		/* 한도처리 (손익/종목별주문송신수량 등) */
		rt = 0;
		if (memcmp (dat->MembershipItem+mem_area_loc+45, "1", 1) == 0)	// 스프레드 여부 체크
		{
#if defined A1491 || A2491 || A3491		// 1번째
			rt = Che_Rtn (R_Fmt[0].Data, od_price, 
#else									// 2번째 재처리
			rt = Che_Rtn (R_Fmt[0].Data, od_price, 
#endif
										 mk_gbn,
										 AtoIf (dat->MembershipItem+mem_area_loc+16, 5),		// 원월물의 종목SEQ
										 AtoIf (dat->MembershipItem+mem_area_loc+42, 2));
		}
		else
		{
#if defined A1491 || A2491 || A3491		// 1번째
			rt = Che_Rtn (R_Fmt[0].Data, od_price, 
#else									// 2번째 재처리
			rt = Che_Rtn (R_Fmt[0].Data, od_price, 
#endif
										 mk_gbn, item_seq, acc_seq);
		}

#if 0
20211201
		/* 한도초과처리 (비정상종료 + Client송신) */
		if (rt > 0)
		{
			Write_Data (rt-1);
		}
#endif
	}	/* End of 회원체결결과 */

	return;
}	/* End of Analyze_Data ()	*/

/*************************************************************************
    Function        : . Che_Rtn
    Parameters IN   : . p_buf   : received data
					  . od_price : 주문가격(현물0 미사용, 파생은 가격)
					  . mk_gbn	: 시장구분
						item_seq: 종목seq
						acc_seq : 계좌seq
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . set Acc data SHM (손익/수수료/평가손익)
*************************************************************************/
/*----------------------------------------------------------------------*/
int   Che_Rtn (char *p_buf, double od_price, int mk_gbn, int item_seq, int acc_seq)
/*----------------------------------------------------------------------*/
{
	int		i, rt, item_seqn, mem_area_loc;
	int		trad_flag, b_getcnt, n_getcnt;
	double	b_avg_d, che_price_d;
/* 20220204 */
	int		trad_flagn, b_getcntn;	// 원월물 정보등등
	double	avg_tmp_d;				// avg_tmp_d:체결종목의 평균단가계산용
	double	avg_tmp_dn, b_avg_dn, che_price_dn;	//*_dn:스프레드원월물의 계산용
/* 20220204 */
	char	item_code[13], r_time[20];

#if defined A1491 || A1492
	KRX_NOTE_SETTLE_DATA	*dat = (KRX_NOTE_SETTLE_DATA *)&p_buf;
	mem_area_loc = 0;
#elif defined A2491 || A2492 || A3491 || A3492
	KRX_SETTLE_DATA			*dat = (KRX_SETTLE_DATA *)&p_buf[YSK_DATA_HEAD_LEN];
	mem_area_loc = 10;
#endif

	memset (item_code,  0, sizeof (item_code));

	/* *************************** */
	/* 전시장 공통용 계산변수 빼기 */
	/* *************************** */
	/* 체결수량 */
	n_getcnt	= AtoIf (&dat->Trading_Volumn[1], sizeof (dat->Trading_Volumn) - 1);
/* 20220204 */
	/* 체결단가 (채권,파생은 모두 .00(소수점2자리), 현물은 자연수) */
	if (memcmp (dat->MembershipItem+mem_area_loc+45, "1", 1) != 0)	// 스프레드 여부 체크
	{
		/* 체결부호 */
		if (memcmp (dat->TradeFlag, "1", 1) == 0)
			trad_flag  = -1;
		else
			trad_flag  =  1;

		/* 체결가격 */
		che_price_d = (double)AtoDf (&dat->Trading_price[1], sizeof (dat->Trading_price) - 1);
	}
#if defined A2491 || A2492 || A3491 || A3492
	else
	{
		/* 체결부호 */
		if (memcmp (dat->TradeFlag, "1", 1) == 0)
		{
			trad_flag   =  1;	// 근월물 부호(매매부호와 반대)
			trad_flagn  = -1;	// 원월물 부호(매매부호와 동일)
		}
		else
		{
			trad_flag   = -1;	// 근월물 부호(매매부호와 반대)
			trad_flagn  =  1;	// 원월물 부호(매매부호와 동일)
		}

		che_price_d  = (double)AtoDf(&dat->Nearby_Trading_Price[1], sizeof (dat->Nearby_Trading_Price) - 1);	// 근월물체결가격
		che_price_dn = (double)AtoDf(&dat->Future_Trading_Price[1], sizeof (dat->Future_Trading_Price) - 1);	// 원월물체결가격
		/* 원월물 종목seq */
		item_seqn = AtoIf (dat->MembershipItem+mem_area_loc+16, 5);
		/* 원월물 기존잔고 */
		b_getcntn	= Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_getcnt;
		/* 원월물 기존평균단가 */
		b_avg_dn	= Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_get_avg_price;
	}
#endif

	/* 기존잔고 */
	b_getcnt	= Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt;
	/* 기존평균단가 */
	b_avg_d		= Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_get_avg_price;
/* 20220204 */
	/* *************************** */
	
	if (mk_gbn == 0)			/* 금융파생 */
	{
		memcpy (item_code,	Shm_FinFut[item_seq].A0.item_code,	sizeof (Shm_FinFut[0].A0.item_code)); 
	}
	else if (mk_gbn == 1)		/* 금융파생(야) */
	{
		memcpy (item_code,	Shm_FinFut_N[item_seq].A0.item_code,	sizeof (Shm_FinFut_N[0].A0.item_code)); 
	}
	else
	{
		Log (USR_ERROR, "시장구분 오류 mk_gbn[%d]", mk_gbn);
		return (NOTOK);
	}

	/* 스프레드는 종목코드로 체크할수 없다 그래서 스킵 */
	if (memcmp (dat->MembershipItem+mem_area_loc+45, "1", 1) != 0)	// 스프레드 여부 체크
	{
		if (memcmp (dat->ItemCode, item_code, strlen (item_code)) != 0)
		{
			Log (USR_ERROR, "Data 종목 미매칭 수신종목[%d][%12.12s] A0[%12.12s] 시장구분[%d]",
				item_seq, dat->ItemCode, item_code, mk_gbn);
			return (NOTOK);
		}
	}

	/* ******************************************************************** */
	/* 체결이면 체결만큼 종목잔고는 변경해준다.								*/
	/* -------------------------------------------------------------------- */
	/* 체결된 수량만큼 수정(미체결수량&&금액 감소) 그리고					*/
	/* - PROFIT(잔고/매도매수주문수량) <= 종목별(근월풀 포함)				*/
	/* ******************************************************************** */
	if (b_getcnt == 0 || (trad_flag * n_getcnt) * b_getcnt > 0)	// 같은방향
	{
		/* 잔고 */
		Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt =
			(trad_flag * n_getcnt) + b_getcnt;

		/* 평균단가 계산 */
		avg_tmp_d = ((b_avg_d * abs(b_getcnt)) + ((double)n_getcnt * che_price_d)) /
					(abs(b_getcnt) + n_getcnt);
	}
	else
	{	
		if (n_getcnt > abs(b_getcnt)) 	// 기존 -10, 신규 15 => 5
		{
			/* 잔고 */
			Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt =
				trad_flag * (n_getcnt - abs(b_getcnt));

			/* 평균단가 계산, 방향바뀜 */
			avg_tmp_d = che_price_d;
		}
		else
		{
			/* 잔고 */
			if (b_getcnt > 0)	// 매수잔고 && 매도체결
			{
				Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt =
					b_getcnt - n_getcnt;
			}
			else				// 매도잔고 && 매수체결
			{
				Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt =
					b_getcnt + n_getcnt;
			}
			/* 평균단가 계산, 정산 */
  			if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt == 0)
				avg_tmp_d = 0;
			else
				avg_tmp_d = b_avg_d;
		}
	}

/* 20220204 */
	/* 미체결에서 체결수량&&금액 빼줌 												*/
	/*																				*/
	/* 체결된 수량 && 금액만큼 미체결 수량 && 금액 빼주기, 0보다 작으면 0으로 처리	*/
	/* 스프레드체결시 근월물과 원월물에 각각 체결 수량 만큼 잔고를 잡아준다, 
	   단 미체결수량 && 금액의 가감은 하지 않는다. 
	   스프레드는 주문시 미체결을 잡지 않기 때문이다. */
	if (trad_flag == 1)	// 매수
	{
		Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt -= n_getcnt;
		/* 금액은 "원주문금액(미체결내역) * 체결수량 * 승수" 만큼 뺀다 */
		if (memcmp (dat->MembershipItem+mem_area_loc+45, "1", 1) != 0)	// 스프레드 여부 체크
			Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum -= (n_getcnt * od_price) * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier;

		if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt = 0)
			Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum = 0;
		else if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt < 0)
			Log (USR_ERROR, "한도계산 이상함, 체크바람 매수미체결수량 음수나옴");
	}
	else				// 매도
	{
		Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt -= n_getcnt;
		/* 금액은 "원주문금액(미체결내역) * 체결수량 * 승수" 만큼 뺀다 */
		if (memcmp (dat->MembershipItem+mem_area_loc+45, "1", 1) != 0)	// 스프레드 여부 체크
			Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum -= (n_getcnt * od_price) * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier;

		if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt <= 0)
			Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum = 0;
		else if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt < 0)
			Log (USR_ERROR, "한도계산 이상함, 체크바람 매도미체결수량 음수나옴");
	}

	/* 평균단가 set */
	Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_get_avg_price = avg_tmp_d;


	/* 원월물 체결처리 */
	if (memcmp (dat->MembershipItem+mem_area_loc+45, "1", 1) == 0)	// 스프레드 여부 체크
	{
		/* ************************************************************ */
		/* 스프레드시 원월물 처리를 추가로 해줘야한다.	20220204		*/
		/* ************************************************************ */
		if (b_getcntn == 0 || (trad_flagn * n_getcnt) * b_getcntn > 0)	// 같은방향
		{
			/* 잔고 */
			Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_getcnt =
				(trad_flagn * n_getcnt) + b_getcntn;

			/* 평균단가 계산 */
			avg_tmp_dn = ((b_avg_dn * abs(b_getcntn)) + ((double)n_getcnt * che_price_dn)) /
						(abs(b_getcntn) + n_getcnt);
		}
		else
		{	
			if (n_getcnt > abs(b_getcntn)) 	// 기존 -10, 신규 15 => 5
			{
				/* 잔고 */
				Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_getcnt =
					trad_flagn * (n_getcnt - abs(b_getcntn));

				/* 평균단가 계산, 방향바뀜 */
				avg_tmp_dn = che_price_dn;
			}
			else
			{
				/* 잔고 */
				if (b_getcntn > 0)	// 매수잔고 && 매도체결
				{
					Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_getcnt =
						b_getcntn - n_getcnt;
				}
				else				// 매도잔고 && 매수체결
				{
					Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_getcnt =
						b_getcntn + n_getcnt;
				}
				/* 평균단가 계산, 정산 */
				if (Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_getcnt == 0)
					avg_tmp_dn = 0;
				else
					avg_tmp_dn = b_avg_dn;
			}
		}

		/* 미체결에서 체결수량&&금액 빼줌 												*/
		/*																				*/
		/* 체결된 수량 && 금액만큼 미체결 수량 && 금액 빼주기, 0보다 작으면 0으로 처리	*/
		/* 스프레드체결시 근월물과 원월물에 각각 체결 수량 만큼 잔고를 잡아준다, 단 미체결수량 && 금액의 가감은 하지 않는다. 스프레드는 주문시 미체결을 잡지 않기 때문이다. */
		/* 현물은 매도수량만 처리한다(공매도금지 때문)									*/
		if (trad_flag == 1)	// 매수
		{
			Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_su_michecnt -= n_getcnt;

			if (Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_su_michecnt = 0)
				Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_su_miche_gum = 0;
			else if (Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_su_michecnt < 0)
				Log (USR_ERROR, "한도계산 이상함, 체크바람 매수미체결수량 음수나옴");
		}
		else				// 매도
		{
			Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_do_michecnt -= n_getcnt;

			if (Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_do_michecnt <= 0)
				Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_do_miche_gum = 0;
			else if (Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_do_michecnt < 0)
				Log (USR_ERROR, "한도계산 이상함, 체크바람 매도미체결수량 음수나옴");
		}

		/* 평균단가 set */
		Shm_Risk[0].ProFit[mk_gbn][item_seqn][acc_seq].item_get_avg_price = avg_tmp_dn;
	}

	return (OK);
}	/* End of Che_Rtn */

/**************************************************************************
    Function        : . Write_Data
    Parameters IN   : . acc_seq : 강제종료할 계좌번호 Seq
    Parameters OUT  : .
    Return Code     : . int
    Comment         : . write data to file
                    :   종료시 : Client 후처리로 처리,
                                 pa_9001_mp에 Auto 종료처리.
**************************************************************************/
/*-----------------------------------------------------------------------*/
void    Write_Data (int acc_seq)
/*-----------------------------------------------------------------------*/
{
#if 0
    int     rt, i, tot_che;
    char    m_time[24], str[10];
    char    cdata[30], fw_buf[1024];

    memset (m_time, 0, sizeof (m_time));
    memset (cdata, ' ', sizeof (cdata));
    memset (fw_buf, ' ', sizeof (fw_buf));

	Get_MicroTime (m_time);

    ItoAf (OFW_CNT(0,0) + 1, W_Fmt.If_Seq, sizeof (W_Fmt.If_Seq));
    memcpy (W_Fmt.ApType, ApType, sizeof (W_Fmt.ApType));
    memcpy (W_Fmt.ResponseCode, RES_NORMAL, strlen (RES_NORMAL));
    memcpy (W_Fmt.RecvTime1, m_time, sizeof (W_Fmt.RecvTime1));
    memcpy (W_Fmt.RecvTime2, &m_time[sizeof(W_Fmt.RecvTime1)],
        sizeof (W_Fmt.RecvTime2));

	memset (W_Fmt.DataHeader, 0x20, DATA_SIZE+20);
	sprintf (cdata, "%02d%12.12sACCNO_RISKOVER_STOP ", acc_seq, ACCNO(D_K,acc_seq).acc_no);
	memcpy (W_Fmt.Data, cdata, strlen(cdata));
	W_Fmt.LineFeed[0] = '\n';

	/* 9001 전략 강제종료 처리 */
	rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);
	if (rt != 1)
    {
        Log (SAM_FATAL, "file write fail [%s]", OFN(D_K,P_K,0));
    }
#endif
}

/*************************************************************************
	End of Program (pa_1490_mp.c)
*************************************************************************/

