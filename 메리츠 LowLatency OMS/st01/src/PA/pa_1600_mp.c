#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 
#	File	: pa_1600_mp.c
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#define		DATA_SIZE	200
#include	"buf_struct.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		DATA_TIME	60 * 1000
#define		READ_MAX	1

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
/* ******************************************************************** */
/* 상품ID 매핑(파생상품)	*/
/* 유가증권 : STK
 * 코스닥 : KSQ
 * 파생 : */
/* ******************************************************************** */
int						R_Cnt;
FILE_BUFF_FORMAT		R_Fmt[READ_MAX], W_Fmt;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_1600_MP (void);
void	Analyze_Data (void);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_1600_MP ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_1600_MP (void)
/*----------------------------------------------------------------------*/
{
	int		rt, i;
	char	m_time[24];
	char    O_Buff[2048], msgnum[100];
	char    w_fmt[6000];

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
		{
			Log (USR_OK, "poll timeout <%d>", OFW(D_K,P_K,0,0));
		}
		else if (rt == -1)
			continue;
	}
}	/* End of PA_1600_MP ()	*/

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
	int     i, rt, mk_gbn;
	char	InBuff[DATA_SIZE];
	char    m_time[24];

	memset (InBuff, 0, sizeof(InBuff));

	memcpy (InBuff, R_Fmt[0].Data, strlen (R_Fmt[0].Data));
	Log (USR_OK, "Input Data [%s][%d]", InBuff, strlen (InBuff));

	if (memcmp (&InBuff[11], "TTRMIP31301", 11) == 0)			// 장운영정보
	{
#if		defined		A1601
		if (memcmp (&InBuff[24], "STK", 3) == 0)				// 유가증권
			mk_gbn = 5;
		else
		if (memcmp (&InBuff[24], "KSQ", 3) == 0)				// 코스닥
			mk_gbn = 6;
		else
		{
			Set_TR_Time ();
			INT_SEQ ++;
			Add_Count(PS_R_1, 1);

			return;
		}
#elif	defined		A2601		// 파생
		if ((memcmp (&InBuff[24], "FKI", 3) == 0)	||			// 1.지수선물
			(memcmp (&InBuff[24], "OKI", 3) == 0)	||			// 2.지수옵션
			(memcmp (&InBuff[24], "FST", 3) == 0)	||			// 3.개별주식선물(코스피종목)
			(memcmp (&InBuff[24], "FKQ", 3) == 0)   ||			// 0.개별주식선물(코스닥종목)
			(memcmp (&InBuff[24], "OST", 3) == 0)	||			// 4.개별주식옵션
			(memcmp (&InBuff[24], "FX3", 3) == 0)	||			// 8.KRX300선물
			(memcmp (&InBuff[24], "FSI", 3) == 0)	||			// 9.코스닥150선물
			(memcmp (&InBuff[24], "OQI", 3) == 0)	||			// 10.코스닥150옵션
			(memcmp (&InBuff[24], "FMK", 3) == 0)	||			// 11.미니선물
			(memcmp (&InBuff[24], "OMK", 3) == 0)	)			// 12.미니옵션
		{
			if (memcmp (&InBuff[24], "FKI", 3) == 0)
				mk_gbn = 1;
			else if (memcmp (&InBuff[24], "OKI", 3) == 0)
				mk_gbn = 2;
			else if ((memcmp (&InBuff[24], "FST", 3) == 0) ||
					 (memcmp (&InBuff[24], "FKQ", 3) == 0))
				mk_gbn = 3;
			else if (memcmp (&InBuff[24], "OST", 3) == 0)
				mk_gbn = 4;
			else if (memcmp (&InBuff[24], "FX3", 3) == 0)
				mk_gbn = 8;
			else if (memcmp (&InBuff[24], "FSI", 3) == 0)
				mk_gbn = 9;
			else if (memcmp (&InBuff[24], "OQI", 3) == 0)
				mk_gbn = 10;
			else if (memcmp (&InBuff[24], "FMK", 3) == 0)
				mk_gbn = 11;
			else if (memcmp (&InBuff[24], "OMK", 3) == 0)
				mk_gbn = 12;
		}
		else
		{
			Set_TR_Time ();
			INT_SEQ ++;
			Add_Count(PS_R_1, 1);

			return;
		}
#endif
		/* 20200624 수정 */
		/* 시장임시정지는 AL/AE8로 오고 재개시에는 G1의 단일가로 매매거래재개함. 임시정지만 처리함 */
		/* 사이드카는 G1이며 거래해도 됨 */
		if ((memcmp (&InBuff[27], "G1", 2) == 0)		||
			(memcmp (&InBuff[27], "G2", 2) == 0)		||
			((memcmp (&InBuff[27], "AL", 2) == 0)	&&
			 (memcmp (&InBuff[29], "AE8", 3) == 0))		||		// 시장임시정지
			((memcmp (&InBuff[27], "AL", 2) == 0)	&&
			 (memcmp (&InBuff[29], "AF8", 3) == 0)))			// 주식CB중단
		{
			/* 20200710 로직추가 */
			/* 보드ID인 G1은 모두 사용하지만 보드이벤트는 주식선물스프레드만 사용한다 */
			/* 따라서 장중에 다른 시장의 AA1(단일가)처리가 될수 있기때문에 			  */
			/* G1AAA1(단일가개시)처리는 FSTG1AA1 또는 FKQG1AA1만 처리한다.			  */
			if ((memcmp (&InBuff[29], "AA1", 3) == 0)	&&
				(memcmp (&InBuff[24], "FST", 3) != 0 && memcmp (&InBuff[24], "FKQ", 3) != 0))
			{
				Set_TR_Time ();
				INT_SEQ ++;
				Add_Count(PS_R_1, 1);

				return;
			}

			// 보드ID 
			memcpy (Shm_Risk[0].open_market_info[mk_gbn], &InBuff[27], 2);
			// 보드 이벤트 
			memcpy (Shm_Risk[0].sub_market_info[mk_gbn],  &InBuff[29], 3);

			Log (USR_OK, "Board ID [%2.2s][%2.2s] [%3.3s][%3.3s] mk_gbn[%d]", Shm_Risk[0].open_market_info[mk_gbn], &InBuff[27], Shm_Risk[0].sub_market_info[mk_gbn], &InBuff[29], mk_gbn);

			/* 장마감 처리 */
			if (((memcmp (&InBuff[27], "G1", 2) == 0)	&&
			 	 (memcmp (&InBuff[29], "AC2", 3) == 0))	||	// 유가증권, 코스닥, 파생 장마감.
			 	((memcmp (&InBuff[27], "G1", 2) == 0)	&&
			 	 (memcmp (&InBuff[29], "AB2", 3) == 0)))	// 유가증권, 코스닥, 파생 장마감.
			{
				// 보드ID 
				memcpy (Shm_Risk[0].open_market_info[mk_gbn], "GE", 2);
			}

		}
	}

	Set_TR_Time ();
	INT_SEQ ++;
	Add_Count(PS_R_1, 1);

	return;
}	/* End of Analyze_Data ()	*/

/*************************************************************************
	End of Program (pa_1600_mp.c)
*************************************************************************/

