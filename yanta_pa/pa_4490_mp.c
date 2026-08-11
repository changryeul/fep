#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 체결처리
#	File	: pa_4490_mp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
--ii_struct.h----------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#define		MK_GBN		0	/* 0 */
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
int			R_Cnt, R_Max;
char		ApType[10];
FILE_BUFF_FORMAT	W_Fmt, R_Fmt[READ_MAX];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
int		Init_Parameters (void);
void	Write_Data (int);
void	PA_4490_MP (void);
void	Analyze_Data (void);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_4490_MP ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_4490_MP (void)
/*----------------------------------------------------------------------*/
{
    int     rt; 

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
}	/* End of PA_4490_MP ()	*/

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
    int     i, rt, che_cnt, retry_cnt;
	int     mk_gbn, item_seq, acc_seq;
	double  od_price;
    char    tr_code[12];

	/* 처리할 데이터의 포맷이 다양해서 구분하여 처리한다. */
	/* 체결은 KRX헤더와 응답(4+11) 모두 필요없다. 체결전문만 있다 */
	memset (tr_code, 0, sizeof (tr_code));

	/* ******************************************************** */
	/* 주문응답/체결의 도치현상처리방안							*/
	/* -------------------------------------------------------- */
	/* 체결Data에 해당하는 주문번호가 없으면 3초 Sleep후 재처리 */
	/* 3회(3초*3회)후 ERROR LOG && SKIP							*/
	/* ******************************************************** */

	retry_cnt = 0;
	/* 회원체결결과 */

	SMB_ST *dat = (SMB_ST *)R_Fmt[0].Data;

	if (memcmp (dat->smb_MsgType, "8", 1) == 0
	 && (memcmp (dat->smb_OrdStatus, "1", 1) == 0
	  || memcmp (dat->smb_OrdStatus, "2", 1) == 0)
	 && (memcmp (dat->smb_ExecType, "1", 1) == 0
	  || memcmp (dat->smb_ExecType, "2", 1) == 0
	  || memcmp (dat->smb_ExecType, "F", 1) == 0))
	{

#if 1    /* HONG */
        mk_gbn      =   MK_GBN;                                         // 시장구분(FX)
		item_seq    =   0;                                              // 종목 Seq(USDKRW)
		acc_seq     =   0;												// 계좌 seq
#else    /* HONG */		
		mk_gbn      =   AtoIf (dat->MembershipItem+imeco_gbn+35, 1);  // 시장구분
        item_seq    =   AtoIf (dat->MembershipItem+imeco_gbn+37, 5);  // 종목 Seq
        acc_seq     =   AtoIf (dat->MembershipItem+imeco_gbn+42, 2);  // 계좌 seq
#endif   /* HONG */

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
        /*          01                   0           1      채권일반
                    02                   0           2      채권LP
                    11                   1           1      금융파생_국채선물
                    12                   1           2      금융파생_통화선물
                    13                   1           3      금융파생_금리선물
        */
        /* 37,38,39,40,41 : A0 seq번호 ex) 236 => (00236)           */
        /* 42, 43 : 계좌번호 seq                                    */
		/*          LP용계좌와 일반계좌를 구분해서 사용해야함.		*/
		/*			예, seq구분을 0~9(일반), 10~19(LP계좌)			*/
		/*			    #define     ACC_NO_CNT	20	//사용계좌수	*/
        /* 44     : 시장구분이 05(유가증권) 이면서 ETF/ETN/ELW면 '1' */
        /*          아니거나 다른시장이면 '0'                       */
        /* 45     : 스프레드종목이면 1, 아니면 0                    */
        /* 46     : 주식선물 && 주식옵션에서 유가증권종목이면 '1', 코스닥종목이면 '2' */
        /*          나머지시장이면 '0'                              */
        /* 47,48,49,50 : ApType 4자리 (50101중 0101만) Set          */
        /* ******************************************************** */
	
		/* 체결된 주문번호 찾기 */
		for (i = 0; i < MAX_MICHE; i++)
        {
			/* 해당 주문번호를 찾고 주문잔량은 감소시켜준다 */
			if (memcmp (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].ClOrdID, dat->smb_ClOrdID, 24) == 0)
			{
				Log (USR_OK, "OK Che Search [%24.24s] i[%d] acc[%d]", dat->smb_ClOrdID, i, acc_seq);

				/* 주문잔량 조정 */
				che_cnt = AtoIf(dat->smb_LastQty, sizeof (dat->smb_LastQty));
				if (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt - che_cnt < 0)
				{
					Log (USR_ERROR, "체결 주문수량 부적합 주문번호 [%24.24s] 주문잔량[%d] 체결수량[%d]",
						dat->smb_OrdID, Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt, che_cnt);
					return;
				}
				Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt =
					Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt - che_cnt;

				if (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Jan_Cnt <= 0)
					Shm_Mk_PreMatch[0].FX_MeChe_Cnt[mk_gbn][acc_seq]--;

				od_price = 0.0;
				od_price = AtoDf (Shm_Mk_PreMatch[0].FX_MiChe[mk_gbn][acc_seq][i].Price, 30);
				break;
			}

			/* 회원처리호가(주문번호)가 아직 안들어왔으면 3초 쉬고 다시한다(3회) */
			/* 그래도 안들어오면 2번(재처리)로 뺀다								 */
			if (i >= MAX_MICHE)
			{
				if (retry_cnt > 3)
				{
					retry_cnt = 0;

#if defined A4491
                    Log (USR_ERROR, "체결주문번호 찾지못했음 재처리로 Write, 주문응답 미처리 JN[%24.24s]",
				        dat->smb_ClOrdID);

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
                Log (USR_OK, "체결주문번호 찾지못했음 retry[%d]회 주문응답 미처리 JN[%24.24s]",
				       retry_cnt, dat->smb_ClOrdID);

#if defined A4491				// 1번째
				sleep (3);
#else							// 2번째 재처리
				sleep (60);
#endif
			}
		}	/* End of for */
				

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
	End of Program (pa_4490_mp.c)
*************************************************************************/

