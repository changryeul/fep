#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 전략기동/종료
#	File	: pa_9000_mp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#define		DATA_SIZE	2048
#include	"buf_struct.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		DATA_TIME	60 * 1000
#define		READ_MAX	1
#define		KRX_PROCNT	4
#define     PLAY_TIME	"0739"
#define     AUTO_PLAY_TIME	"081000"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int					R_Cnt, RR;
char				ApType[10];
FILE_BUFF_FORMAT	W_Fmt, R_Fmt[READ_MAX];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_9000_MP (void);
void    Init_Parameters (void);
void	Analyze_Data (void);
void	Start_Client (char *);
void	Write_Auto_Data (int, char *, int);
void	Stop_Client (char *, int);
void	Auto_Stop_Client (char *);
void	Write_Data (int);
void	Write_Auto_Fifo (int);
void	Stop_All_Client (char *, int);
void	Acc_Search (void);
void	Acc_Update (char *);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_9000_MP ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_9000_MP (void)
/*----------------------------------------------------------------------*/
{
	int		rt, DelayFlag;
	char	m_time[24];

	DelayFlag = OFF;
	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
	LtoU (ApType, strlen (ApType));

	while (START_S != JOB_END)
	{
		Stat_Save ();

		while (START_S != JOB_END)
		{
			Stat_Save ();

			if (DelayFlag == OFF)
			{
				memset (m_time, 0, sizeof(m_time));
				Get_Time (m_time);

				/* Acc정보 초기화 작업용 Data 요구처리 */
				if (memcmp (m_time, "0900", 4) < 0)		// 9시이후에는 초기화못하게
					Init_Parameters ();
Log (USR_OK, "Init_P OK");
				DelayFlag = ON;
			}

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
			Log (USR_OK, "poll timeout <%d>", INT_SEQ);
		else if (rt == -1)
			continue;
	}
}	/* End of PA_9000_MP ()	*/

/*************************************************************************
	Function        : . Init_Parameters
	Parameters IN   : .
	Parameters OUT  : .
	Return Code     : . void
	Comment         : . All Program Parameters Init
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Init_Parameters (void)
/*----------------------------------------------------------------------*/
{
	int     i, j, k;

	RR = 0;
	/* ******************** */
	/* 기초자료 초기화 작업 */
	/* -------------------- */
	/* 시세초기화는 기본dd에서 한다.										*/
	/* 단, 시세에서 주문관련된건 여기서 한다.(auto_use)			*/
	/* 여기서는 shm_memory의 MK_PREMATCH(미체결), RISK(리스크) 처리를 한다. */
	/* ******************************************************************** */

	/* 시장별 거래관련 초기화 */
	/* 내부거래불가종목 초기화 */
	for (i = 0; i < SHM_MAX_JISU; i++)		// 지수정보
	{
		Shm_Risk[0].S_Sise[0][i].auto_use = 0;
		Shm_Risk[0].S_Sise[0][i].dont_trade = 0;
	}
	for (i = 0; i < SHM_MAX_FUTURES; i++)	// 지수선물
	{
		Shm_Risk[0].S_Sise[1][i].auto_use = 0;
		Shm_Risk[0].S_Sise[1][i].dont_trade = 0;
	}
	for (i = 0; i < SHM_MAX_OPTIONS; i++)	// 지수옵션
	{
		Shm_Risk[0].S_Sise[2][i].auto_use = 0;
		Shm_Risk[0].S_Sise[2][i].dont_trade = 0;
	}
	for (i = 0; i < SHM_MAX_S_FUTURES; i++)	// 주식선물
	{
		Shm_Risk[0].S_Sise[3][i].auto_use = 0;
		Shm_Risk[0].S_Sise[3][i].dont_trade = 0;
	}
	for (i = 0; i < SHM_MAX_S_OPTIONS; i++)	// 주식옵션
	{
		Shm_Risk[0].S_Sise[4][i].auto_use = 0;
		Shm_Risk[0].S_Sise[4][i].dont_trade = 0;
	}
	for (i = 0; i < SHM_MAX_STOCK; i++)		// 유가증권(ELW/ETF/ETN)
	{
		Shm_Risk[0].S_Sise[5][i].auto_use = 0;
		Shm_Risk[0].S_Sise[5][i].dont_trade = 0;
	}
	for (i = 0; i < SHM_MAX_KOSDAQ; i++)		// 코스닥
	{
		Shm_Risk[0].S_Sise[6][i].auto_use = 0;
		Shm_Risk[0].S_Sise[6][i].dont_trade = 0;
	}
	for (i = 0; i < SHM_MAX_K300; i++)		// K300
	{
		Shm_Risk[0].S_Sise[8][i].auto_use = 0;
		Shm_Risk[0].S_Sise[8][i].dont_trade = 0;
	}
	for (i = 0; i < SHM_MAX_K150F; i++)		// K150F
	{
		Shm_Risk[0].S_Sise[9][i].auto_use = 0;
		Shm_Risk[0].S_Sise[9][i].dont_trade = 0;
	}
	for (i = 0; i < SHM_MAX_K150O; i++)		// K150O
	{
		Shm_Risk[0].S_Sise[10][i].auto_use = 0;
		Shm_Risk[0].S_Sise[10][i].dont_trade = 0;
	}
	for (i = 0; i < SHM_MAX_MF; i++)		// 미니선물
	{
		Shm_Risk[0].S_Sise[11][i].auto_use = 0;
		Shm_Risk[0].S_Sise[11][i].dont_trade = 0;
	}
	for (i = 0; i < SHM_MAX_MO; i++)		// 미니옵션
	{
		Shm_Risk[0].S_Sise[12][i].auto_use = 0;
		Shm_Risk[0].S_Sise[12][i].dont_trade = 0;
	}

	/* 해외시장 초기화추가 */
	memset (Shm_CME[0].A0.type, 0, (sizeof(OC_MASTER)+sizeof(OC_Q)+sizeof(OC_B)+sizeof(OC_S)) * SHM_MAX_CME);
	memset (Shm_CMX[0].A0.type, 0, (sizeof(OC_MASTER)+sizeof(OC_Q)+sizeof(OC_B)+sizeof(OC_S)) * SHM_MAX_CMX);
	memset (Shm_SGX[0].A0.type, 0, (sizeof(OC_MASTER)+sizeof(OC_Q)+sizeof(OC_B)+sizeof(OC_S)) * SHM_MAX_SGX);
	memset (Shm_ERX[0].A0.type, 0, (sizeof(OC_MASTER)+sizeof(OC_Q)+sizeof(OC_B)+sizeof(OC_S)) * SHM_MAX_ERX);
	memset (Shm_HKE[0].A0.type, 0, (sizeof(OC_MASTER)+sizeof(OC_Q)+sizeof(OC_B)+sizeof(OC_S)) * SHM_MAX_HKE);

	for (i = 0; i < SHM_MAX_CME; i++)		// CME
		Shm_CME[i].auto_use = 0;
	for (i = 0; i < SHM_MAX_SGX; i++)		// SGX
		Shm_SGX[i].auto_use = 0;
	for (i = 0; i < SHM_MAX_ERX; i++)		// ERX
		Shm_ERX[i].auto_use = 0;
	for (i = 0; i < SHM_MAX_HKE; i++)		// HKE
		Shm_HKE[i].auto_use = 0;
	/* 시장별 거래관련 초기화 */

	/* 미체결 내역 초기화 */ 
	for (i = 0; i < MK_CNT; i++)		
	{
		for (j = 0; j < ACC_NO_CNT; j ++)		
		{
			Shm_Mk_PreMatch[0].MeChe_Cnt[i][j] = 0;

			for (k = 0; k < MAX_MICHE; k ++)		
				Shm_Mk_PreMatch[0].F_MiChe[i][j][k].Jan_Cnt = 0;
		}
	}
	
	/* ************* */
	/* 리스크 초기화 */
	memset (&Shm_Risk[0].ProFit[0][0][0].item_getcnt,  0,
								sizeof (PROFIT) * MK_CNT * SHM_MAX_STOCK * ACC_NO_CNT);
	// 1.지수선물
	Shm_Risk[0].mk_hoga_dan_d[1] = 0.05;
	Shm_Risk[0].mk_hoga_val_d[1] = 12500.0;
	// 2.지수옵션
	Shm_Risk[0].mk_hoga_dan_d[2] = 0.01;
	Shm_Risk[0].mk_hoga_val_d[2] = 1000.0;
	// 3.주식선물
	Shm_Risk[0].mk_hoga_dan_d[3] = 1.0;
	Shm_Risk[0].mk_hoga_val_d[3] = 10.0;
	// 4.주식옵션
	Shm_Risk[0].mk_hoga_dan_d[4] = 1.0;
	Shm_Risk[0].mk_hoga_val_d[4] = 10.0;
	// 5.유가증권
	Shm_Risk[0].mk_hoga_dan_d[5] = 1.0;
	Shm_Risk[0].mk_hoga_val_d[5] = 1.0;
	// 6.코스닥
	Shm_Risk[0].mk_hoga_dan_d[6] = 1.0;
	Shm_Risk[0].mk_hoga_val_d[6] = 1.0;
	// 7.ELW/ETF/ETN
	Shm_Risk[0].mk_hoga_dan_d[7] = 5.0;
	Shm_Risk[0].mk_hoga_val_d[7] = 5.0;
	// 8.KRX300선물
	Shm_Risk[0].mk_hoga_dan_d[8] = 0.2;
	Shm_Risk[0].mk_hoga_val_d[8] = 10000.0;
	// 9.kosdaq150선물
	Shm_Risk[0].mk_hoga_dan_d[9] = 0.1;
	Shm_Risk[0].mk_hoga_val_d[9] = 1000.0;
	// 10.kosdaq150옵션
	Shm_Risk[0].mk_hoga_dan_d[10] = 0.1;
	Shm_Risk[0].mk_hoga_val_d[10] = 1000.0;

	/* **********/
	/* 20211102 */
	/* 1. KOSPI200 Futures Call Price Set */
	Shm_Risk[0].Ho_Chk[1][0].hoga_depth = 1;

    Shm_Risk[0].Ho_Chk[1][0].band_price = 0.05;

    Shm_Risk[0].Ho_Chk[1][0].band_unit  = 0.05;

    Shm_Risk[0].Ho_Chk[1][0].band_sum   =    0;

	/* 2. KOSPI200 Options Call Price Set */
    Shm_Risk[0].Ho_Chk[2][0].hoga_depth = 2;

    Shm_Risk[0].Ho_Chk[2][0].band_price = 10.0;
    Shm_Risk[0].Ho_Chk[2][1].band_price = 0.01;

    Shm_Risk[0].Ho_Chk[2][0].band_unit  = 0.05;
    Shm_Risk[0].Ho_Chk[2][1].band_unit  = 0.01;

    Shm_Risk[0].Ho_Chk[2][0].band_sum   = 1000;
    Shm_Risk[0].Ho_Chk[2][1].band_sum   =    0;

	/* 2. SF_Ksp Call Price Set */
	/* 주식선물에 기초자산이 코스닥인지 코스피인지에 따라서 다르다. 해서 코스피면 정상적으로 3, 코스닥이면 0으로 한다.(0은 지수라 비어있다) */
    Shm_Risk[0].Ho_Chk[3][0].hoga_depth = 5;

    Shm_Risk[0].Ho_Chk[3][0].band_price = 500000;
    Shm_Risk[0].Ho_Chk[3][1].band_price = 100000;
    Shm_Risk[0].Ho_Chk[3][2].band_price =  50000;
    Shm_Risk[0].Ho_Chk[3][3].band_price =  10000;
    Shm_Risk[0].Ho_Chk[3][4].band_price =     1;

    Shm_Risk[0].Ho_Chk[3][0].band_unit  = 1000;
    Shm_Risk[0].Ho_Chk[3][1].band_unit  =  500;
    Shm_Risk[0].Ho_Chk[3][2].band_unit  =  100;
    Shm_Risk[0].Ho_Chk[3][3].band_unit  =   50;
    Shm_Risk[0].Ho_Chk[3][4].band_unit  =   10;

    Shm_Risk[0].Ho_Chk[3][0].band_sum   = 3100;
    Shm_Risk[0].Ho_Chk[3][1].band_sum   = 2300;
    Shm_Risk[0].Ho_Chk[3][2].band_sum   = 1800;
    Shm_Risk[0].Ho_Chk[3][3].band_sum   = 1000;
    Shm_Risk[0].Ho_Chk[3][4].band_sum   =    0;

    /* 0. SF_Ksd Call Price Set */
	/* 주식선물에 기초자산이 코스닥인지 코스피인지에 따라서 다르다. 해서 코스피면 정상적으로 3, 코스닥이면 0으로 한다.(0은 지수라 비어있다) */
    Shm_Risk[0].Ho_Chk[0][0].hoga_depth = 5;

    Shm_Risk[0].Ho_Chk[0][0].band_price = 50000;
    Shm_Risk[0].Ho_Chk[0][1].band_price = 10000;
    Shm_Risk[0].Ho_Chk[0][2].band_price =  5000;
    Shm_Risk[0].Ho_Chk[0][3].band_price =  1000;
    Shm_Risk[0].Ho_Chk[0][4].band_price =     1;

    Shm_Risk[0].Ho_Chk[0][0].band_unit  = 100;
    Shm_Risk[0].Ho_Chk[0][1].band_unit  =  50;
    Shm_Risk[0].Ho_Chk[0][2].band_unit  =  10;
    Shm_Risk[0].Ho_Chk[0][3].band_unit  =   5;
    Shm_Risk[0].Ho_Chk[0][4].band_unit  =   1;

    Shm_Risk[0].Ho_Chk[0][0].band_sum   = 3100;
    Shm_Risk[0].Ho_Chk[0][1].band_sum   = 2300;
    Shm_Risk[0].Ho_Chk[0][2].band_sum   = 1800;
    Shm_Risk[0].Ho_Chk[0][3].band_sum   = 1000;
    Shm_Risk[0].Ho_Chk[0][4].band_sum   =    0;

    /* 4. SO_Ksp Call Price Set */
    Shm_Risk[0].Ho_Chk[4][0].hoga_depth = 5;

    Shm_Risk[0].Ho_Chk[4][0].band_price = 10000;
    Shm_Risk[0].Ho_Chk[4][1].band_price =  5000;
    Shm_Risk[0].Ho_Chk[4][2].band_price =  2000;
    Shm_Risk[0].Ho_Chk[4][3].band_price =  1000;
    Shm_Risk[0].Ho_Chk[4][4].band_price =     1;

    Shm_Risk[0].Ho_Chk[4][0].band_unit  = 200;
    Shm_Risk[0].Ho_Chk[4][1].band_unit  = 100;
    Shm_Risk[0].Ho_Chk[4][2].band_unit  =  50;
    Shm_Risk[0].Ho_Chk[4][3].band_unit  =  20;
    Shm_Risk[0].Ho_Chk[4][4].band_unit  =  10;

    Shm_Risk[0].Ho_Chk[4][0].band_sum   = 260;
    Shm_Risk[0].Ho_Chk[4][1].band_sum   = 210;
    Shm_Risk[0].Ho_Chk[4][2].band_sum   = 150;
    Shm_Risk[0].Ho_Chk[4][3].band_sum   = 100;
    Shm_Risk[0].Ho_Chk[4][4].band_sum   =   0;

	/* 5. Ksp Call Price Set */
    Shm_Risk[0].Ho_Chk[5][0].hoga_depth = 7;

    Shm_Risk[0].Ho_Chk[5][0].band_price = 500000;
    Shm_Risk[0].Ho_Chk[5][1].band_price = 100000;
    Shm_Risk[0].Ho_Chk[5][2].band_price =  50000;
    Shm_Risk[0].Ho_Chk[5][3].band_price =  10000;
    Shm_Risk[0].Ho_Chk[5][4].band_price =   5000;
    Shm_Risk[0].Ho_Chk[5][5].band_price =   1000;
    Shm_Risk[0].Ho_Chk[5][6].band_price =      1;

    Shm_Risk[0].Ho_Chk[5][0].band_unit  = 1000;
    Shm_Risk[0].Ho_Chk[5][1].band_unit  =  500;
    Shm_Risk[0].Ho_Chk[5][2].band_unit  =  100;
    Shm_Risk[0].Ho_Chk[5][3].band_unit  =   50;
    Shm_Risk[0].Ho_Chk[5][4].band_unit  =   10;
    Shm_Risk[0].Ho_Chk[5][5].band_unit  =    5;
    Shm_Risk[0].Ho_Chk[5][6].band_unit  =    1;

    Shm_Risk[0].Ho_Chk[5][0].band_sum   = 4400;
    Shm_Risk[0].Ho_Chk[5][1].band_sum   = 3600;
    Shm_Risk[0].Ho_Chk[5][2].band_sum   = 3100;
    Shm_Risk[0].Ho_Chk[5][3].band_sum   = 2300;
    Shm_Risk[0].Ho_Chk[5][4].band_sum   = 1800;
    Shm_Risk[0].Ho_Chk[5][5].band_sum   = 1000;
    Shm_Risk[0].Ho_Chk[5][6].band_sum   =    0;

    /* 1.Ksd Call Price Set */
    Shm_Risk[0].Ho_Chk[6][0].hoga_depth = 5;

    Shm_Risk[0].Ho_Chk[6][0].band_price = 50000;
    Shm_Risk[0].Ho_Chk[6][1].band_price = 10000;
    Shm_Risk[0].Ho_Chk[6][2].band_price =  5000;
    Shm_Risk[0].Ho_Chk[6][3].band_price =  1000;
    Shm_Risk[0].Ho_Chk[6][4].band_price =     1;

    Shm_Risk[0].Ho_Chk[6][0].band_unit  = 100;
    Shm_Risk[0].Ho_Chk[6][1].band_unit  =  50;
    Shm_Risk[0].Ho_Chk[6][2].band_unit  =  10;
    Shm_Risk[0].Ho_Chk[6][3].band_unit  =   5;
	Shm_Risk[0].Ho_Chk[1][4].band_unit  =   1;

    Shm_Risk[0].Ho_Chk[6][0].band_sum   = 3100;
    Shm_Risk[0].Ho_Chk[6][1].band_sum   = 2300;
    Shm_Risk[0].Ho_Chk[6][2].band_sum   = 1800;
    Shm_Risk[0].Ho_Chk[6][3].band_sum   = 1000;
    Shm_Risk[0].Ho_Chk[6][4].band_sum   =    0;
	
	/* 7. Ksp ETF/ETN/ELW Call Price Set */
    Shm_Risk[0].Ho_Chk[7][0].hoga_depth = 1;

    Shm_Risk[0].Ho_Chk[7][0].band_price = 5;

    Shm_Risk[0].Ho_Chk[7][0].band_unit  = 5;

    Shm_Risk[0].Ho_Chk[7][0].band_sum   = 0;

	/* 8. KRX300 Call Price Set */
    Shm_Risk[0].Ho_Chk[8][0].hoga_depth = 1;

    Shm_Risk[0].Ho_Chk[8][0].band_price =  0.2;

    Shm_Risk[0].Ho_Chk[8][0].band_unit  =  0.2;

    Shm_Risk[0].Ho_Chk[8][0].band_sum   =    0;

    /* 9 KSD150 Futures Call Price Set */
    Shm_Risk[0].Ho_Chk[9][0].hoga_depth = 1;

    Shm_Risk[0].Ho_Chk[9][0].band_price =  0.1;

    Shm_Risk[0].Ho_Chk[9][0].band_unit  =  0.1;

    Shm_Risk[0].Ho_Chk[9][0].band_sum   =    0;

	/* 10. KSD150 Options Call Price Set */
    Shm_Risk[0].Ho_Chk[10][0].hoga_depth = 2;

    Shm_Risk[0].Ho_Chk[10][0].band_price = 50.0;
    Shm_Risk[0].Ho_Chk[10][1].band_price = 0.1;

    Shm_Risk[0].Ho_Chk[10][0].band_unit  = 0.5;
    Shm_Risk[0].Ho_Chk[10][1].band_unit  = 0.1;

    Shm_Risk[0].Ho_Chk[10][0].band_sum   = 500;
    Shm_Risk[0].Ho_Chk[10][1].band_sum   =   0;

	/* 11. 미니선물 Call Price Set */
	Shm_Risk[0].Ho_Chk[11][0].hoga_depth = 1;

    Shm_Risk[0].Ho_Chk[11][0].band_price = 0.02;

    Shm_Risk[0].Ho_Chk[11][0].band_unit  = 0.02;

    Shm_Risk[0].Ho_Chk[11][0].band_sum   =    0;

	/* 12. 미니옵션 Call Price Set */
    Shm_Risk[0].Ho_Chk[12][0].hoga_depth = 3;

    Shm_Risk[0].Ho_Chk[12][0].band_price = 10.0;
    Shm_Risk[0].Ho_Chk[12][1].band_price = 3.00;
    Shm_Risk[0].Ho_Chk[12][2].band_price = 0.01;

    Shm_Risk[0].Ho_Chk[12][0].band_unit  = 0.05;
    Shm_Risk[0].Ho_Chk[12][1].band_unit  = 0.02;
    Shm_Risk[0].Ho_Chk[12][2].band_unit  = 0.01;

    Shm_Risk[0].Ho_Chk[12][0].band_sum   =  650;
    Shm_Risk[0].Ho_Chk[12][1].band_sum   =  300;
    Shm_Risk[0].Ho_Chk[12][2].band_sum   =    0;

	/* 13. Ksp_SP Call Price Set */
    Shm_Risk[0].Ho_Chk[13][0].hoga_depth = 1;
    Shm_Risk[0].Ho_Chk[13][0].band_price = 0.0;
    Shm_Risk[0].Ho_Chk[13][0].band_unit  = 0.0;
    Shm_Risk[0].Ho_Chk[13][0].band_sum   =   0;

	/* 14. Ksd_SP Call Price Set */
    Shm_Risk[0].Ho_Chk[14][0].hoga_depth = 1;
    Shm_Risk[0].Ho_Chk[14][0].band_price = 0.0;
    Shm_Risk[0].Ho_Chk[14][0].band_unit  = 0.0;
    Shm_Risk[0].Ho_Chk[14][0].band_sum   =   0;

	/* 20211102 */
	/* ******** */

	/* 주문번호 초기화 */
	int		dual_atv_no;
	for (i = 0; i < MAX_AUTO_PROC; i++)
	{
		if ((memcmp ((char *)getenv ("_FEP_DIV"), "REAL1", 5) == 0)	||
			(memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0))
			dual_atv_no = 0;
		else
			dual_atv_no = 10000000;

		/* 현물 */
		Shm_Risk[0].Order_No[0][i].START_JMNO	= 1530100000 + (100000*i) + dual_atv_no;
		Shm_Risk[0].Order_No[0][i].USED_JMNO	= 1530100000 + (100000*i) + dual_atv_no;
		Shm_Risk[0].Order_No[0][i].END_JMNO	= 1530199999 + (100000*i) + dual_atv_no;
		/* 파생 */
		Shm_Risk[0].Order_No[1][i].START_JMNO	= 1580100000 + (100000*i) + dual_atv_no;
		Shm_Risk[0].Order_No[1][i].USED_JMNO	= 1580100000 + (100000*i) + dual_atv_no;
		Shm_Risk[0].Order_No[1][i].END_JMNO	= 1580199999 + (100000*i) + dual_atv_no;
	}

	/* *************** */
	/* 계좌정보 초기화 */
	// ACCNO는 여기서 미사용
	for (i = 0; i < ACC_NO_CNT; i++)
	{
		ACCNO(D_K,i).acc_risk_max			= 0;
		ACCNO(D_K,i).acc_f_order_maxcnt		= 0;
		ACCNO(D_K,i).acc_f_get_maxcnt		= 0;
		ACCNO(D_K,i).acc_so_order_maxcnt	= 0;
		ACCNO(D_K,i).acc_so_get_maxcnt		= 0;
		ACCNO(D_K,i).over_acc_ver_prft		= 0;
		ACCNO(D_K,i).over_sf_order_currcnt	= 0;
		ACCNO(D_K,i).over_o_order_currcnt	= 0;
		ACCNO(D_K,i).acc_real_prft			= 0;
		ACCNO(D_K,i).acc_fee				= 0;
		ACCNO(D_K,i).acc_ver_prft			= 0;
		ACCNO(D_K,i).risk_flag				= 0;
		ACCNO(D_K,i).auto_run				= 0;
		memset (ACCNO(D_K,i).auto_set, 0, sizeof (ACCNO(0,0).auto_set));
	}

	/* 받아오는 메모리의 계좌정보 초기화 */
	for (i = 0; i < MK_CNT; i++)
	{
		for (j = 0; j < ACC_NO_CNT; j ++)
		{
			/* 1회 한도 설정값 */
			memcpy (Shm_Risk[0].O_M_Fund[i][j].qty_gbn,		" ", 1);
			memcpy (Shm_Risk[0].O_M_Fund[i][j].money_gbn,	" ", 1);
			memcpy (Shm_Risk[0].O_M_Fund[i][j].tick_gbn,	" ", 1);
			Shm_Risk[0].O_M_Fund[i][j].qty = 0;
			Shm_Risk[0].O_M_Fund[i][j].money = 0;
			Shm_Risk[0].O_M_Fund[i][j].tick = 0;

			/* 누적 한도 설정값 */
			memcpy (Shm_Risk[0].T_M_Fund[i][j].do_cnt_gbn,	" ", 1);
			memcpy (Shm_Risk[0].T_M_Fund[i][j].do_money_gbn," ", 1);
			memcpy (Shm_Risk[0].T_M_Fund[i][j].su_cnt_gbn,	" ", 1);
			memcpy (Shm_Risk[0].T_M_Fund[i][j].su_money_gbn," ", 1);
			Shm_Risk[0].T_M_Fund[i][j].do_cnt	= 0;
			Shm_Risk[0].T_M_Fund[i][j].do_money	= 0;
			Shm_Risk[0].T_M_Fund[i][j].su_cnt	= 0;
			Shm_Risk[0].T_M_Fund[i][j].su_money	= 0;

			/* 누적 한도 사용값 */
			memcpy (Shm_Risk[0].T_S_Fund[i][j].do_cnt_gbn,	" ", 1);
			memcpy (Shm_Risk[0].T_S_Fund[i][j].do_money_gbn," ", 1);
			memcpy (Shm_Risk[0].T_S_Fund[i][j].su_cnt_gbn,	" ", 1);
			memcpy (Shm_Risk[0].T_S_Fund[i][j].su_money_gbn," ", 1);
			Shm_Risk[0].T_S_Fund[i][j].do_cnt	= 0;
			Shm_Risk[0].T_S_Fund[i][j].do_money	= 0;
			Shm_Risk[0].T_S_Fund[i][j].su_cnt	= 0;
			Shm_Risk[0].T_S_Fund[i][j].su_money	= 0;
		}
	}

	/* 잔고정보 초기화 */
	for (i = 0; i < MK_CNT; i++)
	{
		for (j = 0; j < SHM_MAX_STOCK; j ++)
		{
			for (k = 0; k < ACC_NO_CNT; k ++)
			{
				Shm_Risk[0].ProFit[i][j][k].item_getcnt		= 0;
				Shm_Risk[0].ProFit[i][j][k].item_getmoney	= 0;
				Shm_Risk[0].ProFit[i][j][k].item_tot_sugum	= 0;
				Shm_Risk[0].ProFit[i][j][k].item_cha_sugum	= 0;
				Shm_Risk[0].ProFit[i][j][k].item_borrow_cnt = 0;
				Shm_Risk[0].ProFit[i][j][k].item_totsu_che	= 0;
				Shm_Risk[0].ProFit[i][j][k].item_totdo_che	= 0;

				Shm_Risk[0].ProFit[i][j][k].item_su_michecnt	= 0;
				Shm_Risk[0].ProFit[i][j][k].item_su_miche_gum	= 0;
				Shm_Risk[0].ProFit[i][j][k].item_do_michecnt	= 0;
				Shm_Risk[0].ProFit[i][j][k].item_do_miche_gum	= 0;
				Shm_Risk[0].ProFit[i][j][k].item_get_avg_price	= 0;
			}	
		}	
	}	

	/* 배치정보 수신,처리건수 초기화 */
	for (i = 0; i < 7; i++)
	{
		Shm_Risk[0].Batch_Cnt[i].W_cnt = 0;
		Shm_Risk[0].Batch_Cnt[i].R_cnt = 0;
	}

	return;
}	/* End of Init_Parameters ()    */

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
	int				i, rt;
	char			m_time[24];
	SEARCH_HEADER	*i_hd;

	memset (m_time, 0, sizeof (m_time));
	memset (&W_Fmt, ' ', sizeof (FILE_BUFF_FORMAT));

	for (i = 0; i < R_Cnt; i++)
	{
		i_hd = (SEARCH_HEADER *)R_Fmt[i].Data;

		if (memcmp (i_hd->TrCode, "500100", 6) == 0)
		{	/* 개별 Client전략 기동	*/
			if ((memcmp (i_hd->ApType_Cd, "5010", 4) >= 0)	&&
				(memcmp (i_hd->ApType_Cd, "5999", 4) <= 0))
			{
				Start_Client (R_Fmt[i].Data);
			}
			else
			{	
				Log (USR_ERROR, "ApType_Cd error Input[%50.50s]", i_hd->ApType_Cd);
				memset (&W_Fmt, ' ', sizeof (FILE_BUFF_FORMAT));
				memcpy (W_Fmt.Data, &R_Fmt[i].Data,	sizeof (SEARCH_HEADER));
				memcpy (W_Fmt.Data, "500120",	6);
				Write_Data (-1);
			}
			
		}
#if 0
		else if ((memcmp (i_hd->TrCode, "500120", 6) == 0)	||
				 (memcmp (i_hd->TrCode, "500210", 6) == 0)	||
				 (memcmp (i_hd->TrCode, "500220", 6) == 0)	)
		{	/* 개별 Client전략 종료	*/
			memset (Cli_handler, ' ', sizeof (Cli_handler));
			memcpy (Cli_handler, &R_Fmt[i].DataHeader[16], 4);
			memcpy (Tcp_Data_Head.LineFlag, &R_Fmt[i].DataHeader[16], 4);
			Stop_Client (R_Fmt[i].Data, AtoIf(i_hd->TrCode, 6));
		}
		else if (memcmp (i_hd->TrCode, "500310", 6) == 0)
		{	/* 특정계좌의 전체 Client전략 종료(세션이상등)	*/
			Stop_All_Client (R_Fmt[i].Data, 1);
		}
		else if (memcmp (i_hd->TrCode, "500320", 6) == 0)
		{	/* 특정계좌의 전체 Client전략 종료(한도체크등)	*/
			Stop_All_Client (R_Fmt[i].Data, 2);
		}
		else if (memcmp (i_hd->TrCode, "500330", 6) == 0)
		{	/* Auto 개별 API 종료 통보 */
			Auto_Stop_Client (R_Fmt[i].Data);
		}
		else if (memcmp (i_hd->TrCode, "500999", 6) == 0)
		{	/* Account정보 초기화 */
			Acc_Update (R_Fmt[i].Data);
		}
#endif
		else										/* TR code error	*/
		{	
			Log (USR_ERROR, "TR code error Input[%50.50s]", i_hd->TrCode);
			memset (&W_Fmt, ' ', sizeof (FILE_BUFF_FORMAT));
			memcpy (W_Fmt.Data, &R_Fmt[i].Data,	sizeof (SEARCH_HEADER));
			memcpy (W_Fmt.Data, "500120",	6);
			Write_Data (-1);
		}

		Add_Count (PS_R_1, 1);
		INT_SEQ ++;
		Set_TR_Time ();
	}

	return;
}	/* End of Analyze_Data ()	*/

/*************************************************************************
	Function        : . Start_Client
	Parameters IN   : . p_buf	: input data
	Parameters OUT  : .
	Return Code     : . void
	Comment         : . start a client process
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Start_Client (char *p_buf)
/*----------------------------------------------------------------------*/
{
	int						pk, w_flag, i, rt, flag, bin_type, m;
	int						auto_f_tcnt, auto_f_scnt, auto_m_tcnt, auto_m_scnt;
	char					pname[12], cmd[256], m_time[20], chk_proc[20];
	char					st_name[20], tg_name[10];
	int						search_seq, get_proc;
	SEARCH_HEADER			*i_hd;

	memset (m_time, 0, sizeof(m_time));
	Get_Time (m_time);

	memset (&W_Fmt, ' ', sizeof (FILE_BUFF_FORMAT));
	memcpy (W_Fmt.Data,	p_buf, DATA_SIZE);

	i_hd = (SEARCH_HEADER *)p_buf;
	memset (st_name, 0, sizeof (st_name));
	sprintf (st_name, "pa_%4.4s_mp", i_hd->ApType_Cd);

	/* Process Name Search */
	get_proc = 0;
	search_seq = RR;
	for (i = 0; i < MAX_AUTO_PROC; i++)
	{
		if (search_seq >= MAX_AUTO_PROC)
			search_seq = 1;
		else
			search_seq++;

		memset (pname, 0, sizeof (pname));
		if (search_seq%10 == 0)
			sprintf (pname, "%2.2s_50%01d%02dmp", _SubSystem_Name, search_seq/10, 10);
		else
			sprintf (pname, "%2.2s_50%01d%02dmp", _SubSystem_Name, search_seq/10+1, search_seq%10);

		for (pk = 0; pk < DAEMON(D_K).p_count; pk ++)
		{
			if (memcmp (PROC(D_K,pk).process_id, pname, 10) == 0)
			{
				if (PROC(D_K,pk).process_status == 1)
					continue;
				else 
				{
					get_proc = 1;
					/* Edit of Input File Count */
					IDR(D_K,pk,0,0) = IDW(D_K,pk,0,0);
					break;
				}
			}

			if (pk == DAEMON(D_K).p_count - 1)
			{
				Log (USR_ERROR, "unregistered process 01[%s]", pname);
				memcpy (W_Fmt.Data, "500120",	6);
				Write_Data (-1);
				return;
			}
		}
	
		if (get_proc == 0 && i >= MAX_AUTO_PROC-1)
		{
			Log (USR_ERROR, "Fail Auto Run, process Full Run input[%50.50s]", i_hd->TrCode);
			memcpy (W_Fmt.Data, "500120",	6);
			Write_Data (-1);
			return;
		}
	

		if (get_proc > 0)
		{
			RR = search_seq;
			break;
		}
	}

Log (USR_OK, "cp [%s/%s %s/%s]", (char *)getenv("_P_BIN"), st_name, (char *)getenv("_P_BIN"), pname);

	sprintf (cmd, "cp %s/%s %s/%s", (char *)getenv("_P_BIN"), st_name, (char *)getenv("_P_BIN"), pname);

	rt = system (cmd);

	if (rt < 0 && SYS_NO != 10)
	{
		Log (SYS_FATAL, "system call fail[%s] {%d:%s}",
			cmd, SYS_NO, SYS_STR);
		Exit_Process ();
	}

	PROC(D_K,pk).start_status = JOB_INIT;
	PROC(D_K,pk).process_status = 1;
	write (DTART_FD, "1", 1);

	memcpy (W_Fmt.Data, "500110",		6);

	memset (tg_name, 0, sizeof (tg_name));
	sprintf (tg_name, "%5.5s", &pname[3]);
	memcpy (&W_Fmt.Data[14], tg_name,	5);
	Write_Data (1);

	return;
}	/* End of Start_Client ()	*/

/**************************************************************************
	Function		: . Write_Data
	Parameters IN	: . p_flag	: write flag
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . write data to file
**************************************************************************/
/*-----------------------------------------------------------------------*/
void	Write_Data (int p_flag)
/*-----------------------------------------------------------------------*/
{
	int		rt;

	W_Fmt.LineFeed[0] = '\n';

	rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);

	if (rt != 1)
	{
		Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,0));
		Exit_Process ();
	}

	Log (USR_OK, "file write[%s:%d:%d]", 
		OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), rt);

	return;
}	/* End of Write_Data ()	*/

/*************************************************************************
	End of Program (pa_9000_mp.c)
*************************************************************************/

