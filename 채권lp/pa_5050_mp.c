#define		_GLOBAL
/*--------------------------------------------------------
#	Module	: Server º±π∞æ˜∆Ω call∏≈ºˆ, ¥ŸøÓ∆Ω put∏≈ºˆ
#	File	: pa_5050_mp.c
--------------------------------------------------------*/
/*------------------------------------------------------*/
/*------------------------------------------------------*/

/*--------------------------------------------------------
	Header Files
--------------------------------------------------------*/
#include	"fep_fepp.h"
#include 	"pa_struct.h"
#include	"seam_queue.h"		/* SEAM Ï£ºÎ¨∏ÌÅê(Ï†ÑÎûµ‚ÜíÏÜ°Ïã†Î∂Ä, P2) */

/* ø‹∫Œ º“Ω∫ */
#ifndef _OMS_SOURCE_
#define		_OMS_SOURCE_	1
#include   "log_conv.h"
#include   "mem.h"
#include   "sem.h"
#include   "mcp.h"
#include   "etc.h"
#include   "blp.h"
#include   "blp.c"
#include   "blp_sise.c"
#include   "blp_order.c"
#include   "blp_convert.c"
#include   "blp_print.c"
#endif /* _OMS_SOURCE_ */

#define		DATA_SIZE	2048
#define		ORDER_DATA_SIZE	400
#include 	"buf_struct.h"
#include	"shm_memory.h"

/*--------------------------------------------------------
	FOR BLP
--------------------------------------------------------*/
BLP 		*Blp;
int			Continue = 1;

/*--------------------------------------------------------
	Constants and Structures
--------------------------------------------------------*/
#define		FIFO_EVENT 	0
#define		FILE_EVENT	1

#define		TCP_TIME_OUT 30
#define		DATA_TIME	60 * 1000
#define		READ_MAX	1

#ifdef  SAM_USE
#define  WR_CNT   W_CNT(0,0)
#define  RD_CNT   R_CNT(0,0)
#else
#define  WR_CNT   IDW_CNT(0,0)
#define  RD_CNT   IDR_CNT(0,0)
#endif

/*--------------------------------------------------------
	Global Variables
--------------------------------------------------------*/
char    Order_Dat[261] = {"00000000001TCHODR10001G10001002999                    KR70059300031100000000182400000001230000007050020000000000000    0011030     00            000041010000011720172121276805CAE61CDC202110261317010280101                                                        0"};

int					Pk_mp[2];
char				ApType[10], Cli_handler[5];
char				W_DFmt[1024];
char				Order_St[1024];
FILE_BUFF_FORMAT	W_Fmt, R_Fmt[READ_MAX];
struct pollfd		Poll[10];
int		InPollCnt, PollCnt;

int		OD_SEQ;
int		Set_Flag;

/*--------------------------------------------------------
	Function Prototypes
--------------------------------------------------------*/
void	PA_5050_MP(void);
int		Init_Parameters(void);
void	Fifo_Event_Rtn(void);
void	Check_Proc_Status(int);
int		Od_Write_Data(int, int, int, int, int);

int		Auto_Logic(int);
void    File_Event_Rtn (void);
void	Set_In_Param(void);			// 500200(±‚µøSetting);
void	Set_In_Stop(int, int);		// 500300(¡æ∑·ø‰√ª ºˆΩ≈), 500410(∞≠¡¶¡æ∑· º€Ω≈)
void	Set_Response(void);			// 100120(¿¿¥‰»Æ¿Œ/∞≈∫Œ), 100140(√º∞·/»Æ¿Œ/∞≈∫Œ)

BLP		*Blp;

        /* »£∞°¿Ø«¸ƒ⁄µÂ, ø©±‚º≠¥¬ ¡ˆ¡§∞°∑Œ √≥∏Æ«ﬂ¥Ÿ. ªÛ/«œ«—∞°∑Œ ¡÷πÆ≥æ∂ßµµ ¡ˆ¡§∞°∑Œ ªÛ«œ«—∞° ∞°∞›¿∏∑Œ ≥ª∏È µ»¥Ÿ. ≥™∏”¡ˆ¥¬ ¿¸∑´ø° µ˚∂Û 
            ªÁøÎ«œ∏È µ»¥Ÿ. */
        /* »£∞°¡∂∞«ƒ⁄µÂ, ø©±‚º≠¥¬ ¥ÎªÛ¿Ã æ∆¥œ¥Ÿ. Default(0)ªÁøÎ, ºˆ¡§« ø‰æ¯¿Ω √ ±‚∞™¿Ã 0¿”  */
        /* Ω√¿Â¡∂º∫¿⁄»£∞°±∏∫–π¯»£, ø©±‚º≠¥¬ Default(0) ªÁøÎ, LP¥¬ 1:LP»£∞°∑Œ Set « ø‰       */
        /* ±‚≈∏µÓµÓ ¿¸∑´ π◊ ∞Ë¡¬ªÛ≈¬ ¡÷πÆ¡∂∞«µÓø° µ˚∂Û ≥™∏”¡ˆ ∞™µÈµµ ∏¬∞‘ √≥∏Æ«œ∏È µ ...    */

        /* ******************************************************************** */
        /* 261Byte¿« KRX¡÷πÆ ∆˜∏À¿ª ∏∏µÈ∞Ì ∏∂¡ˆ∏∑¿∏∑Œ »∏ø¯ªÁ√≥∏Æ«◊∏Ò ∞™ º≥¡§ */
        /* »∏ø¯ªÁ√≥∏Æ«◊∏Ò 60πŸ¿Ã∆Æ¡ﬂ æ’ 30πŸ¿Ã∆Æ¥¬ ø¯¿Âø°º≠ ø‰√ª«— ∞™¿ª ≥÷æÓæﬂ «œ∞Ì, µ⁄ 30πŸ¿Ã∆Æ¥¬ æ∆∑°¿« ¡∂∞«ø° ∏¬∞‘ Setting«ÿæﬂ«‘ (Clientµµ µø¿œ) */
        /* 30           : A(º≠πˆ¿⁄µø¡÷πÆ), C(Clientø°º≠ ≥Ω ¡÷πÆ)                */
        /* 31, 32       : ST(Strategy)                                          */
        /* 33, 34       : ¿¸∑´π¯»£ 01 ~ 99                                      */
        /* 35, 36       : Ω√¿Â±∏∫–                                              */
        /*                1(¡ˆºˆº±π∞) 2(¡ˆºˆø…º«) 3(¡÷Ωƒº±π∞) 4(¡÷Ωƒø…º«)
                          5(¿Ø∞°¡ı±«/ELW/ETF/ETN) 6(ƒ⁄Ω∫¥⁄)
                          8(KRX300) 9(Kosdaq150 Futures) 10(Kosdaq150 Options)  */
        /* 37,38,39,40,41 : A0¿« Seqπ¯»£ Ex) 236 => (00236)                     */
        /* 42, 43       : ∞Ë¡¬π¯»£ Seq                                          */
        /* 44           : 0 (Default, ¿Ø∞°¡ı±«∞˙ ¡÷Ωƒº±π∞∏∏ º±≈√ ≥™∏”¡¯ 0)
                        : ¿Ø∞°¡ı±«(5)¿œ ∞ÊøÏ => 0(Normal), 1(ELW), 2(ETN), 3(ETF)
                        : ¡÷Ωƒº±π∞(3)¿œ ∞ÊøÏ => «ÿ¥Á¡æ∏Ò¿Ã ƒ⁄Ω∫««∏È 0, ƒ⁄Ω∫¥⁄¿Ã∏È 1 */
        /* 45           : Ω∫«¡∑πµÂø©∫Œ, 0:normal, 1:Ω∫«¡∑πµÂ                    */
        /* 46,47,48,49  : Process Nick Name
                          pa_50101mp => 0101, pa_50203mp => 0203                */
        /* ******************************************************************** */
        /* ******************************************************** */
        /* MembershipItem => 30 byte∫Œ≈Õ ªÁøÎ∞°¥…                   */
        /* ∆ƒª˝(IMECO¥¬ 20πŸ¿Ã∆Æ∏∏ ¡ÿ¥Ÿ.)                           */
        /*     µ˚∂Ûº≠ Ω∫«¡∑πµÂ¥¬ ªÁøÎ∏¯«—¥Ÿ.                        */
        /* ******************************************************** */
        /*  4, 5, 6, 7, 8 : Ω∫«¡∑πµÂ ±Ÿø˘π∞ ¡æ∏ÒSeq(πÃªÁøÎ)         */
        /* 16,17,18,19,20 : Ω∫«¡∑πµÂ ø¯ø˘π∞ ¡æ∏ÒSeq(πÃªÁøÎ)         */
        /* 30 : A(º≠πˆ¿⁄µø¡÷πÆ), C(∏ﬁ∏Æ√˜Client¡÷πÆ) T(¿©ø˛¿Ã∏≈√º)  */
        /*    : ∏≈√º±∏∫– (A:Auto or All, C:∏ﬁ∏Æ√˜∏≈√º, T:¿©ø˛¿Ã∏≈√º */
        /*      A¥¬ ¿⁄µø¡÷πÆ/¡÷πÆ¿¿¥‰,√º∞·µÓ ¡÷πÆ∞¸∑√µ»∞«(TR100XXX) æÁ¬ ∏≈√ºø° ∫∏≥Ω¥Ÿ.*/
        /*      Data Header 50 Byte¡ﬂ 20π¯¬∞ 1¿⁄∏ÆøÕ ∞∞¿Ã ªÁøÎ«—¥Ÿ(¡∂»∏µÓ) */
        /* 31,32,33,34 : ¿¸∑´ø°º≠ ªÁøÎ                              */
        /* 35, 36 : Ω√¿Â±∏∫–¿∫ 35 1¿⁄∏Æ∏∏¿∏∑Œ √º≈©                  */
        /*  (øÓøÎªÛ«∞∫–∑˘ƒ⁄µÂ)  (Ω√¿Âindex) (øÓøµªÛ«∞index) (Ω√¿Â±∏∫–∏Ì)*/
        /*          01                   0           1      √§±«¿œπ›
                    02                   0           2      √§±«LP
                    11                   1           1      ±›¿∂∆ƒª˝_±π√§º±π∞
                    12                   1           2      ±›¿∂∆ƒª˝_≈Î»≠º±π∞
                    13                   1           3      ±›¿∂∆ƒª˝_±›∏Æº±π∞
         */
        /* 37,38,39,40,41 : A0 seqπ¯»£ ex) 236 => (00236)           */
        /* 42, 43 : ∞Ë¡¬π¯»£ seq                                    */
        /* 44     : πÃªÁøÎ                                          */
        /* 45     : Ω∫«¡∑πµÂ¡æ∏Ò¿Ã∏È 1, æ∆¥œ∏È 0                    */
        /* 46     : ¡÷Ωƒº±π∞ && ¡÷Ωƒø…º«ø°º≠ ¿Ø∞°¡ı±«¡æ∏Ò¿Ã∏È '1', ƒ⁄Ω∫¥⁄¡æ∏Ò¿Ã∏È '2' */
        /*          ≥™∏”¡ˆΩ√¿Â¿Ã∏È '0'                              */
        /* 47,48,49,50 : ApType 4¿⁄∏Æ (50101¡ﬂ 0101∏∏) Set          */
        /* ******************************************************** */


/*--------------------------------------------------------*/
int		main(int argc, char *argv[])
/*--------------------------------------------------------*/
{
	Init_Proc(argc, argv);
	SEAM_ORD_Init();		/* Î∂ÄÎ¨∏Í∞Ñ SEAM Ï£ºÎ¨∏ÌÅê ÌôïÎ≥¥(Ï†ÑÎûµ‚ÜíÏãúÏû• ÏÜ°Ïã†Î∂Ä) */
	PA_5050_MP();
	Exit_Process();
}	/* End of main */

/*--------------------------------------------------------*/
void		PA_5050_MP(void)
/*--------------------------------------------------------*/
{
	int		i, j, rt, bi_y, rc, ju_edt;
	int		as_p, o_price, o_ticks01, o_ticks03;
	int		call_jan, call_ticks, put_jan, put_ticks, fu_jan, jm_su;
	int		call_price, put_price, fu_price, for_cnt, op_gbn;
	int		f_rp_c, f_rp_p, ju_gbn, o_rp;
	char	t_time[12];

	char	sise[ 8192];
	int		rtn;
	
	Set_Flag = 0;
	Log( USR_OK, "start ..... sleep 5");
	sleep(5);			// Client∑Œ∫Œ≈Õ Parameter∏¶ πﬁ¿ª Ω√∞£¿ª ¡ÿ¥Ÿ.
	rt = Init_Parameters();
	
	if (rt != OK)
	{
		Exit_Process();
		sleep(1);
	}

	/*
	Log( USR_OK, "[%s:%d] Call Set_In_Param ... ", __FUNCTION__, __LINE__);
	Set_In_Param();
	*/

	while (START_S < JOB_END)
	{
		Stat_Save();

		while (START_S < JOB_END)
		{
/* **************************************************************************************************** */
/* ¿⁄µø∑Œ¡˜ Ω√¿€ 2021.11.02																				*/
/* **************************************************************************************************** */
			// Log( USR_OK, "START_S=[%d] JOB_END=[%d]", START_S, JOB_END);
			// Log( USR_OK, "WR_CNT=[%d] RD_CNT=[%d]", WR_CNT, RD_CNT);
			if (WR_CNT > RD_CNT)
			{
				File_Event_Rtn();		// ¿¿¥‰/√º∞· √≥∏Æ
				if (Set_Flag > 0)
					PollCnt = InPollCnt + 2;
				else
					PollCnt = 2;
				continue;
			}
			else
				break;
		}
		
		/*
		PollCnt = InPollCnt +2;
		Log( USR_OK, "poll ... PollCnt=[%d] TIME_OUT=[%d]", PollCnt, TIME_OUT * 1000);
		*/
		rt = poll(Poll, PollCnt, TIME_OUT * 100);
		// Log( USR_OK, "poll ... rt=[%d]", rt);
		if (rt > 0)
		{
			for (i = 0; i < PollCnt; i++)
			{	
				if (Poll[i].revents & POLLIN)
				{
					Poll[i].revents = 0;
					break;
				}
				
				if (Poll[i].revents & POLLHUP)
				{
					Log( USR_OK, "poll hangup[%d,%d]", i, PollCnt);
					continue;
				}
		
			}
		}
		else
		{
			if (rt == 0)
			{
				// Log( USR_OK, "poll timeout (no response)");
				Blp_Process( Blp, NULL);
				continue;
			}
			else
			{
				if (SYS_NO == EINTR)
					Log(USR_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
				else
					Log(USR_OK, "poll failure {%d:%s}", SYS_NO, SYS_STR);
				
				continue;
			}
		}
		
		switch (i)
		{
			case FIFO_EVENT:
				Fifo_Event_Rtn();
				break;
			case FILE_EVENT:			//File Event, ¿¿¥‰/√º∞·√≥∏Æ, º≥¡§/¡æ∑·ºˆΩ≈µÓ
				File_Event_Rtn();
				if (Set_Flag > 0)
					PollCnt = InPollCnt + 2;
				else
					PollCnt = 2;
				
				while(START_S < JOB_END)
				{
					if (WR_CNT > RD_CNT)
					{
						File_Event_Rtn();
						if (Set_Flag > 0)
							PollCnt = InPollCnt + 2;
						else
							PollCnt = 2;
						continue;
					}
					else
						break;
				}
				break;
				
			case 2:			// √ππ¯¬∞ Ω√¿ÂΩ≈»£(ex, √§±«) - øÏº± test∑Œ √§±« Ω√ºº 
				Blp_SiseRecv( Blp, sise, sizeof( sise));
				Log( USR_OK, "sise received data=[%s]", sise);
				rtn = Blp_Process( Blp, sise);
				if( rtn <= 0) continue;
				break;

			case 3:			// µŒπ¯¬∞ Ω√¿ÂΩ≈»£(ex, ±›¿∂∆ƒª˝)
				rt = Auto_Logic(i-2);
				if (rt)
				{
					rt = Od_Write_Data(1, 1, 1, 2, 1);		// mk_gbn : Ω√¿Â±∏∫– (1~10),
															// nmc_gbn : 1:Ω≈±‘, 2:¡§¡§, 3:√Îº“,  
															// p_flag : write falg 1:√§±«, 2:∆ƒª˝Ω√¿Â¡÷πÆ
															// mm_gbn : 1(∏≈µµ), 2(∏≈ºˆ)
															// arry : ¡÷πÆ≥æ ¥ÎªÛ¿« arry(ºˆΩ≈πﬁ¿∫ ¡æ∏Ò¡ﬂ ∏Óπ¯¬∞)
				}
				else
					return;

				break;
				
			default:
				Log(USR_OK, "event case error[%d,%d]", i, PollCnt);
				return;
				break;
		}
	}
	
	Check_Proc_Status(0);
	Check_Proc_Status(1);
	
}	/* End of PA_5050_MP () */

/***********************************************************
	Function : . File_Event_Rtn
	Parameters IN : .
	Parameters OUT : .
	Return Code : . void
	Comment : . read file and send data
***********************************************************/
/*--------------------------------------------------------*/
void	File_Event_Rtn (void)
/*--------------------------------------------------------*/
{
	int rt, R_Cnt;
	char tmp[128];
	
	Log( USR_OK, "File_Event_Rtn start ..." );
//	sleep( 1);

Log (USR_OK, "TTT01");
	R_Cnt = DSHM_R (PS_R_1, (void *)R_Fmt, 1);
Log (USR_OK, "TTT02");
	Log( USR_OK, "    TS_R1_1 R_Fmt=[%s]", R_Fmt );
	Log( USR_OK, "    TS_R1_1 R_Cnt=[%d]", R_Cnt );
	if (R_Cnt < 0)
	{
		Log(SAM_FATAL, "cannot read file[%s,%d:%s] W[%d]R[%d]",
			IFN(D_K,P_K,0), SYS_NO, SYS_STR, WRITE_CNT, RD_CNT);
		sleep(1);
		Exit_Process();
	}
	else if (R_Cnt == 0)
		return;
	
	Log( USR_OK, "    Data=[%.50s]", ( char *)R_Fmt[0].Data);

	if (memcmp(R_Fmt[0].Data, "500100", 6) == 0)				// ¿⁄µø±‚µø Setting
		Set_In_Param();
	else if (memcmp(R_Fmt[0].Data, "500200", 6) == 0)				// ∆Û¡ˆ
		Set_In_Param();
	else if (memcmp(R_Fmt[0].Data, "500300", 6) == 0)		// ¿⁄µø¡æ∑·ø‰√ª
	{
		Blp_StopProcess( Blp, NULL);
		if (Set_Flag == 0)
			Set_In_Stop(0,0);		// ∞®º“æ¯¿Ã ¡æ∑·
		else
			Set_In_Stop(0,1);		// ∞®º“Ω√≈∞∞Ì ¡æ∑·
		Exit_Process();
	}
	else if ((memcmp(R_Fmt[0].Data, "100120", 6) == 0) ||		// ¿¿¥‰(100120), √º∞·(100140)
			 (memcmp(R_Fmt[0].Data, "100140", 6) == 0))
	{
		rt = Blp_ProcessExecute( Blp, &R_Fmt[ 0]);
		if( rt < 0)
		{
			Log( USR_OK, "Blp_ProcessExecute error. rt=[%d]", rt);
			Exit_Process();
		}
		Set_Response();
	}
	else
	{
		Log(USR_ERROR, "Other Tr Code Check Plz [%20.20s]", R_Fmt[0].Data);
	}
	
	Dshm_Add_Count(TS_W1_1, 1);
	
	while(1)
	{
		rt = read(INPUT_FD, tmp, sizeof(tmp));
#if defined __linux
		if (rt == 0 || errno == EAGAIN)
#else
		if (rt == 0)
#endif
			break;
	}
	
	return;
}	/* File_Event_Rtn() */

/*--------------------------------------------------------*/
int Auto_Logic(int fd_gbn)
/*--------------------------------------------------------*/
{
	int		rt = 0;
	int		curr_key, be_curr_key;
	/* **************************************************************************** */
	/* √§±«Ω√¿Â																		*/
	/* 1. ¡∂∞«¿∫ √º∞·(A3/G7)ø°∏∏ π›¿¿«—¥Ÿ.											*/
	/* 2. ª˘«√¿¸∑´¿∫ √§±«¿« √º∞·ºˆ∑Æ¿Ã 50∞Ëæ‡ ¿ÃªÛ && √º∞·∆Ω¿Ã ªÛΩ¬ => ∏≈ºˆ			*/
	/* 				   √º∞·ºˆ∑Æ¿Ã 50∞Ëæ‡ ¿ÃªÛ && √º∞·∆Ω¿Ã «œ∂Ù => ∏≈µµ				*/
	/* 3. ¡÷πÆ¿∫ 1»∏∏∏ √≥∏Æ«œ∞Ì ¡æ∑·«—¥Ÿ. 											*/
	/* 4. ¿¸∑´º≥¡§Ω√ 3∞≥¿« ¡æ∏Ò¿ª πﬁ¥¬µ• √ππ¯¬∞¥¬ ¡∂∞«¿Ã µ«¥¬ ¡æ∏Ò¿Ã∞Ì				*/
	/*						 µŒπ¯¬∞¥¬ ¡÷πÆ¥ÎªÛ¿Ã µ«¥¬ ¡æ∏Ò¿Ã¥Ÿ						*/
	/* 5. ∂««— »≠∏È(¿¸∑´±‚µø)ø°º≠µµ 3¡æ∏Ò¿∫ º¯º≠¥Î∑Œ πﬁ±‚∑Œ «œø¥¥Ÿ.						*/
	/* **************************************************************************** */
	Log( USR_OK, "Auto_Logic start ..." );
	
	if (AtoIf(&Shm_Note[0].G7.bond_accum_exec_vol[7], 8) <= 0)	// ¿Â∞≥Ω√¿¸ø°¥¬ ¡÷πÆ√≥∏Æ æ»«‘.
		return (0);
	
	/* º±π∞¿« √º∞·ºˆ∑Æ(50¿ÃªÛ)∞˙ UpTick or DownTic √º≈© */
	if (Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].HogaLastGbn == 0)	// 0:A3 or G7, 1:B6
	{
		// «ˆ¿Á√º∞·∞°
		curr_key	= Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry_Key;
		// ¡˜¿¸√º∞·∞°
		be_curr_key	= Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].Befor_CURR_Arry_Key;

		/* √º∞·ºˆ∑Æ √º≈© */
		if (memcmp(&Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry[curr_key].volume[2], "00000050", 8) > 0)
		{
			/* Up or Down √º≈© */
			if (memcmp(Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry[curr_key].crprc,
					   Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry[be_curr_key].crprc, 10) > 0)	//UP
			{
				rt = 1;			// √ππ¯¬∞¡æ∏Ò ∏≈ºˆ
			}
			else
			if (memcmp(Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry[curr_key].crprc,
					   Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry[be_curr_key].crprc, 10) < 0)	//Down
			{
				rt = 2;			// µŒπ¯¬∞¡æ∏Ò ∏≈ºˆ
			}
		}
	}
	
	return (rt);
	
}	/* End of Auto_Logic() */

/***********************************************************
	Function 		: . Check_Proc_Status
	Parameters IN 	: .
	Parameters OUT 	: .
	Return Code 	: . void
	Comment 		: . ¡÷πÆπ¯»£ √§π¯
***********************************************************/
/*--------------------------------------------------------*/
int	GetOrderNo( char *buf, int sz)
{
	char	rec[ 64];

	Log( USR_OK, "GetOrderNo start ...");

	Log( USR_OK, "    OD_SEQ                                    = [%d]", OD_SEQ);
	Log( USR_OK, "    Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO = [%ld]", Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO);

	/* ¡÷πÆπ¯»£ √§π¯ */
	Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO++;        // ∆ƒª˝¿∫ ....Order_No[1]....

	if (Shm_Risk[0].Order_No[0][OD_SEQ].START_JMNO > Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO  ||
		Shm_Risk[0].Order_No[0][OD_SEQ].END_JMNO  <= Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO)
	{
		Log (USR_ERROR, "¡÷πÆπ¯»£ ªÁøÎ øµø™ FULL!!! jm_no[%ld]", Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO);
		// for TEST return (-1);
	}

	Log( USR_OK, "¡÷πÆπ¯»£ √§π¯ ... no=[%ld]", Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO);

	sprintf( rec, "%0*ld", sz, Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO);
	memcpy( buf, rec, sz);
	return sz;
}


/***********************************************************
	Function 		: . Check_Proc_Status
	Parameters IN 	: .
	Parameters OUT 	: .
	Return Code 	: . void
	Comment 		: . check validity of Send_Proc status
***********************************************************/
/*--------------------------------------------------------*/
void	Check_Proc_Status(int mk)
/*--------------------------------------------------------*/	  
{
	int rt;
	
#if 0
	if (TCP1_NSTAT(D_K,Pk_mp[mk]) != ON)
	{
		Log(USR_ERROR, "check process status[%.10s]", OFN(D_K,P_K,mk));
		
		sleep(1);
	}
#endif
	
	return;
}	/* End of Check_Proc_Status() */

/***********************************************************
	Function 		: . Init_Parameters
	Parameters IN 	: .
	Parameters OUT 	: .
	Return Code 	: . int
	Comment 		: . All Program Parameters Init
***********************************************************/
/*--------------------------------------------------------*/
int		Init_Parameters()
/*--------------------------------------------------------*/
{
	int 	i, rt;
	
	SEARCH_HEADER		*hd;
	KRX_JUMUN_DATA		*KrxOrder;
	
	Log( USR_OK, "Init_Parameters ... ");

	Blp = Blp_Open( BLP_KEY, 1);
	if( Blp == NULL)
	{
		Log( USR_OK, "Blp_Open error. key=[0x%08x]", 0xbb001001);
		Exit_Process();
	}
	Log( USR_OK, "    Blp_Open success.");


	Poll[0].fd = START_FD;		// of Daemon
	Poll[0].events = POLLIN;
	Poll[1].fd = INPUT_FD; 		// of SelfFile
	Poll[1].events = POLLIN;

	Log( USR_OK, "    START_FD =[%d]", START_FD);
	Log( USR_OK, "    DTART_FD =[%d]", DTART_FD);
	Log( USR_OK, "    INPUT_FD =[%d]", INPUT_FD);
	Log( USR_OK, "    INPUT_FD2=[%d]", INPUT_FD2);
	
	if (TIME_OUT == 0)
		TIME_OUT = TCP_TIME_OUT;
	
	memset(ApType, 0, sizeof(ApType));
	sprintf(ApType, "%-2.2s%-5.5s", _Exe_Name, _Exe_Name+3);
	LtoU(ApType, strlen(ApType));
	Log( USR_OK, "    ApType=[%s]", ApType);
	
	/* Process ¡ı∞°µ…∂ß∏∂¥Ÿ ºˆ¡§∞À¡ı¿ª «ÿæﬂµ . */
	OD_SEQ = ((AtoIf(&ApType[3], 2) -1) * 10) + AtoIf(&ApType[5], 2) -1;
	Log( USR_OK, "    OD_SEQ=[%d]", OD_SEQ);
	
	Log( USR_OK, "    IDN(D_K,P_K,0)=[%s]", IDN(D_K,P_K,0));
	Log( USR_OK, "    IDN(D_K,P_K,1)=[%s]", IDN(D_K,P_K,1));
	Log( USR_OK, "    FFN(D_K,P_K,0)=[%s]", FFN(D_K,P_K,0));
	Log( USR_OK, "    FFN(D_K,P_K,1)=[%s]", FFN(D_K,P_K,1));
	Log( USR_OK, "    OFN(D_K,P_K,0)=[%s]", OFN(D_K,P_K,0));
	Log( USR_OK, "    OFN(D_K,P_K,1)=[%s]", OFN(D_K,P_K,1));
	Log( USR_OK, "    OFN(D_K,P_K,2)=[%s]", OFN(D_K,P_K,2));
	Log( USR_OK, "    ODN(D_K,P_K,0)=[%s]", ODN(D_K,P_K,0));
	Log( USR_OK, "    ODN(D_K,P_K,1)=[%s]", ODN(D_K,P_K,1));
	Log( USR_OK, "    ODN(D_K,P_K,2)=[%s]", ODN(D_K,P_K,2));
	Log( USR_OK, "    ODN(D_K,P_K,3)=[%s]", ODN(D_K,P_K,3));


	Log( USR_OK, "    DAEMON(D_K).p_count     =[%d]", DAEMON(D_K).p_count);
	Log( USR_OK, "    DAEMON(D_K).process_no  =[%d]", DAEMON(D_K).process_no);
	Log( USR_OK, "    DAEMON(D_K).process_id  =[%s]", DAEMON(D_K).process_id);
	Log( USR_OK, "    DAEMON(D_K).start_FIFO  =[%s]", DAEMON(D_K).start_FIFO_name);
	Log( USR_OK, "    DAEMON(D_K).exit_FIFO   =[%s]", DAEMON(D_K).exit_FIFO_name);
	Log( USR_OK, "    DAEMON(D_K).daemon_FIFO =[%s]", DAEMON(D_K).daemon_FIFO_name);
	Log( USR_OK, "    DAEMON(D_K).tcp1_count  =[%d]", DAEMON(D_K).tcp1_count);
	Log( USR_OK, "    DAEMON(D_K).tcp2_count  =[%d]", DAEMON(D_K).tcp2_count);

	/* SendProcess »∏º± Set */
#if 0
	for(i = 0; i < 3; i++)
	{
		for(Pk_mp[i] = 0; Pk_mp[i] < DAEMON(D_K).p_count; Pk_mp[i] ++)
		{
			Log( USR_OK, "    Pk_mp[%d] PROC(D_K,Pk_mp[%d].process_id=[%s]", Pk_mp[i], Pk_mp[i], PROC(D_K,Pk_mp[i]).process_id);
			if (memcmp(PROC(D_K,Pk_mp[i]).process_id, ODN(D_K,P_K,i), 10) == 0)
				break;
			
			if (Pk_mp[i] == DAEMON(D_K).p_count - 1)
			{
				Log(USR_FATAL, "unregistered process[%s]", ODN(D_K,P_K,i));
				TCP1_NET_STA = OFF;
				Exit_Process();
			}
		}
		Log( USR_OK, "    ODN=[%.10s]", PROC(D_K,Pk_mp[i]).process_id);
	}
#endif
	Log( USR_OK, "Init_Parameters ... END");
	
}	/* End of Init_Parameters() */

/***********************************************************
	Function 		: . Set_In_Param
	Parameters IN 	: .
	Parameters OUT 	: .
	Return Code 	: . int
	Comment 		: . 
***********************************************************/
/*--------------------------------------------------------*/
void	Set_In_Param(void)
/*--------------------------------------------------------*/
{
	int		i, rt, arry, mk_gbn, item_seq;
	int 	return_flag = 0;
	char 	fifo_name[256];
	char	err_no[10], head_size[10];

	int		sock;
	
// ø‰±‚º≠∫Œ≈Õ ƒ⁄µ˘, 202111031
	IN_SAMPLE01			*dat;
	unsigned char		*ptr = ( char *)dat;
	
	Log( USR_OK, "Set_In_Param ... ");

	memset (W_DFmt, 0,		sizeof (W_DFmt));
	memset (W_DFmt, 0x20,	70+400);
	memcpy (&W_DFmt, &R_Fmt, sizeof (BUFF_RW_HEAD)+20);

	dat = (IN_SAMPLE01 *)&R_Fmt[0].Data[sizeof(SEARCH_HEADER)];
	Log( USR_OK, "    dat->next_flag=[%s]", dat->next_flag);
	Log( USR_OK, "    dat=[%s]", ( char *)dat);

	Blp_SetArg( Blp, ( char *)dat, ApType);

	sock = Blp_SiseOpen( Blp);
	Log( USR_OK, "    multicast sock=[%d] InPollCnt=[%d]", sock, InPollCnt);
	Set_Flag++;
	Poll[InPollCnt+2].fd = sock;
	Poll[InPollCnt+2].events = POLLIN;
	InPollCnt++;
	/*
	Poll[i+2].fd = sock;
	Poll[i+2].events = POLLIN;
	if (Poll[i+2].fd < 0)
	*/

	Log( USR_OK, "    R_Fmt=[%s]", ( char *)&R_Fmt[ 0]);

#if 0
	for( i = 0; i < sizeof( IN_SAMPLE01); i++)
	{
		Log( USR_OK, "    %3d=[%c][%02x]", i, ptr[ i], ( char)ptr[ i]);
	}





	/* ¿¸∑´¿¸¥ﬁ¡ﬂ ∞¯≈Î(¿≠∫Œ∫–)¿ª Set«œ∞Ì Ω√¿Â FD∞™¿ª º≥¡§«—¥Ÿ */
	if ((memcmp(dat->next_flag, "O", 1) == 0) ||		// 1∞≥ Struct
		(memcmp(dat->next_flag, "S", 1) == 0))		// Struct Ω√¿€
	{
		Set_Flag = 0;
		/* ªÁøÎµ… √— ¡æ∏Ò ºˆ∑Æ */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].tot_item_cnt	= AtoIf(dat->tot_item_cnt, 2);
		/* ªÁøÎµ… √— Ω√¿Â ºˆ */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].use_market_cnt	= AtoIf(dat->use_market_cnt, 2);
		/* 1»∏ ¡÷πÆΩ√ ¡÷πÆ ºˆ∑Æ */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].tot_order_cnt	= AtoIf(dat->tot_order_cnt, 2);
		/* ¡÷πÆ≥æ «ˆπ∞ ∞Ë¡¬π¯»£ */
		memcpy(Shm_Strrg[0].SamPle_St01[OD_SEQ].st_accno, dat->st_accno, sizeof(dat->st_accno));
		/* ¡÷πÆ≥æ ∆ƒª˝ ∞‘¡¬π¯»£ */
		memcpy(Shm_Strrg[0].SamPle_St01[OD_SEQ].dv_accno, dat->dv_accno, sizeof(dat->dv_accno));
		
		/* Ω√¿Â∫∞ fd∞™ º≥¡§ */
		InPollCnt = AtoIf(dat->use_market_cnt, 2);
		InPollCnt += 2;
		Log( USR_OK, "    InPollCnt=[%d]", InPollCnt);
		
// ∞¯≈Î
		for(i = 0; i < InPollCnt; i++)
		{
			memset(fifo_name, 0, sizeof(fifo_name));
			if (memcmp(dat->market_gbn[i], "10", 2) == 0)
				sprintf(fifo_name, "%s/PA/pa_7912_ur1", _FEP_FIFO);
			else
				sprintf(fifo_name, "%s/PA/pa_7%1.1s11_ur1", _FEP_FIFO, dat->market_gbn[i]+1);
			Log( USR_OK, "    fifo_name=[%s]", fifo_name);
			
			Poll[i+2].fd = open(fifo_name, O_RDWR|O_NDELAY);
			Poll[i+2].events = POLLIN;
			if (Poll[i+2].fd < 0)
			{
				Log(FIF_FATAL, "    cannot open i{%d] FIFO[%s][%d][%d:%s]",
					i, fifo_name, Poll[i+2].fd, SYS_NO, SYS_STR);
				return_flag = -1;
			}
		}
		Log(USR_OK, "    Poll Cnt = [%d]", InPollCnt);
		
		if (memcmp(dat->next_flag, "O", 1) == 0)
			Set_Flag = 1;
		else
		{
			if (return_flag == 0)
				return;
		}
	}
	else
	if ((memcmp(dat->next_flag, "N", 1) == 0) ||		// Struct ∞Ëº”
		(memcmp(dat->next_flag, "E", 1) == 0))		// Struct ¡æ∑·
	{
		/* Arry π¯»£, º¯π¯ */
		arry = AtoIf(dat->Condition.arry_no, 3);
		/* Ω√¿Â±∏∫– */
		mk_gbn = AtoIf(dat->Condition.market_gbn, 2);
		Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].market_gbn = mk_gbn;
		/* ¡æ∏Òseq */
		item_seq = AtoIf(dat->Condition.item_seq, 5);
		Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq = item_seq;
		/* ¡æ∏Òƒ⁄µÂ ¡§«’º∫ √º≈© */
		if (memcmp(Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_item_cd,
				  dat->Condition.item_code, 12) != 0)
		{
			return_flag = -2;
		}
		memcpy(Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_code,
			   dat->Condition.item_code, 12);
		/*≈∏ƒœ¿Œ¡ˆ ¬¸¡∂¿Œ¡ˆ ø©∫Œ */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].target_gbn = 
			AtoIf(dat->Condition.target_gbn, 1);
		/* 1»∏ ¡÷πÆºˆ∑Æ */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_cnt = 
			AtoIf(dat->Condition.order_cnt, 8);
		/* ¡÷πÆ≈∏¿‘ */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_type = 
			AtoIf(dat->Condition.order_type, 1);
		/* ¡÷πÆ∞°∞›±∏∫– */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_gbn = 
			AtoIf(dat->Condition.order_price_gbn, 1);
		/* ¡÷πÆ∞°∞›¡∂∞« */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_condition = 
			AtoIf(dat->Condition.order_price_condition, 1);
			
		if (memcmp(dat->next_flag, "E", 1) == 0)
			Set_Flag = 1;
		else
		{
			if (return_flag == 0)
				return;
		}
	}
	else	// ¿¸∑´ æ¯¿Ω --- CDC
	{
		Log( USR_OK, "    ¿¸∑´ æ¯¿Ω");
		Set_Flag = 0;

		Log( USR_OK, "    InPollCnt=[%d]", InPollCnt);
		Log( USR_OK, "    PollCnt=[%d]", PollCnt);

		/* 1 - √§±«KTS_Ω√ºººˆΩ≈ : pa_7102_ur */
		/* 2 - √§±«KTS_M4ºˆΩ≈   : pa_7103_ur */
		InPollCnt = 2;

		for( i = 0; i < 2; i++)
		{
			if( i == 0)	sprintf( fifo_name, "%s/PA/pa_7102_dd1", _FEP_FIFO);
			else
			if( i == 1)	sprintf( fifo_name, "%s/PA/pa_7103_dd1", _FEP_FIFO);
			Log( USR_OK, "    fifo_name=[%s]", fifo_name);
			Poll[InPollCnt +2].fd = open(fifo_name, O_RDWR|O_NDELAY);
			Poll[InPollCnt +2].events = POLLIN;
			if (Poll[InPollCnt +2].fd < 0)
			{
				Log(FIF_FATAL, "    cannot open i{%d] FIFO[%s][%d][%d:%s]",
					i, fifo_name, Poll[2].fd, SYS_NO, SYS_STR);
				return_flag = -1;
			}
		}

		Log(USR_OK, "    InPollCnt = [%d]", InPollCnt);
	}
	
	Log( USR_OK, "    return_flag=[%d]", return_flag);
	/* ********************************************************* */
	/* ¡§ªÛø©∫Œ ¿¿¥‰ √≥∏Æ */
	memset(W_DFmt, 0x20, sizeof(W_DFmt));
	memcpy(&W_DFmt, &R_Fmt, sizeof(BUFF_RW_HEAD) + 20);
	/* ±‚µø Set ø¿∑˘ */
	if (return_flag < 0)
	{
		// ¡æ∑·»ƒ Clientø° Error º€Ω≈
		memcpy(&W_DFmt[sizeof(BUFF_RW_HEAD)], "500220", 6);		/* SEARCH_HEADER(20), Tr Code */
	}
	else
	{
		// Clientø° ¡§ªÛ±‚µø (50021) º€Ω≈
		memcpy(&W_DFmt[sizeof(BUFF_RW_HEAD)], "500210", 6);		/* SEARCH_HEADER(20), Tr Code */
		Log( USR_OK, "    Clientø° ¡§ªÛ±‚µø (50021) º€Ω≈");
	}
	memset(err_no, 0, sizeof(err_no));
	sprintf(err_no, "%04d", -1*return_flag);
	memcpy(&W_DFmt[sizeof(BUFF_RW_HEAD)+10], err_no, 4);	// SEARCH_HEADER(20), ErrCode(0000:Normal) */
	W_DFmt[sizeof(BUFF_RW_HEAD) + DATA_SIZE] = '\n';
	
	rt = F_W(TS_W1_1, (void *)&W_DFmt, 1);
	if (rt != 1)
	{
		Log(SAM_FATAL, "    file write fail[%s]", OFN(D_K,P_K,1));
		Exit_Process();
	}
	
	Log(USR_OK, "    file write[%s:%d:%d]",
		OFN(D_K,P_K,OD_SEQ+2), OFW(D_K,P_K,OD_SEQ+2,0), rt);
	/* ********************************************************** */
	
	/* Ω√¿Â∫∞ Ω√ºº¿« auto_use∏¶ ¡ı∞°Ω√ƒ—¡ÿ¥Ÿ. Ω√ººΩ≈»£∏¶ πﬁ±‚¿ß«ÿº≠ */
	Log( USR_OK, "    Set_Flag=[%d] return_flag=[%d]", Set_Flag, return_flag);
	if (Set_Flag == 1 && return_flag == 0) 	/* Client¿« Set√≥∏Æ∞° ¡§ªÛ¿˚¿∏∑Œ ≥°≥µ¿∏∏È */
	{
		// Log( USR_OK, "    Shm_Strrg[0].SamPle_St01[OD_SEQ].tot_item_cnt=[%d]", Shm_Strrg[0].SamPle_St01[OD_SEQ].tot_item_cnt);
		for(i = 0; i < Shm_Strrg[0].SamPle_St01[OD_SEQ].tot_item_cnt; i++)
		{
			mk_gbn 	 = Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].market_gbn;
			item_seq = Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq;
			Shm_Risk[0].S_Sise[mk_gbn][item_seq].auto_use++;
			Log( USR_OK, "    Shm_Risk[0].S_Sise[mk_gbn][item_seq].auto_use=[%d]", Shm_Risk[0].S_Sise[mk_gbn][item_seq].auto_use);
		}
		
		/* 9001¿Ã Set «—¥Ÿ.
		Shm_Strrg[0].auto_run_flag[OD_SEQ] = 1;
		*/
	}
	else if (return_flag < 0)
	{
		Set_In_Stop(1, 0); 		//∞®º“æ¯¿Ã ¡æ∑·
	}
#endif
	
	Log( USR_OK, "Set_In_Param ... END");
	return;
}	/* End of Set_In_Param */


/***********************************************************
	Function 		: . Set_In_Stop
	Parameters IN 	: . tr_gbn   => 0:500310(¡§ªÛ¡æ∑·), 1:500410(∞≠¡¶¡æ∑·)
					: . min_flag => 0:∞®º“æ¯¿Ã, 1:∞®º“«œ∞Ì
	Parameters OUT 	: .
	Return Code 	: . int
	Comment 		: . 
***********************************************************/
/*--------------------------------------------------------*/
void	Set_In_Stop (int tr_gbn, int min_flag)
/*--------------------------------------------------------*/
{
	int		i, rt, mk_gbn, item_seq;
	char	err_no[10], fifo_name[128];

	/* ************************************************** */
	/* ¡§ªÛø©∫Œ ¿¿¥‰ √≥∏Æ */
	memset (W_DFmt, 0,		sizeof (W_DFmt));
	memset (W_DFmt, 0x20,	70+400);
	memcpy (&W_DFmt, &R_Fmt, sizeof (BUFF_RW_HEAD)+20);
	if (tr_gbn == 0)
		memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)], "500310", 6);		/* SEARCH_HEADER(20), TrCode (500310:¡§ªÛ¡æ∑·)	*/
	else
		memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)], "500410", 6);		/* SEARCH_HEADER(20), TrCode (500410:∞≠¡¶¡æ∑·)	*/
	memset (err_no, 0, sizeof (err_no));
	sprintf (err_no, "%04d", 0);
	memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)+10], err_no, 4);			/* SEARCH_HEADER(20), ErrCode(0000:Normal)		*/
	W_DFmt[sizeof(BUFF_RW_HEAD) + DATA_SIZE] = '\n';

	rt = F_W (TS_W1_1, (void *)&W_DFmt, 1);
	if (rt != 1)
	{
		Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,1));
		Exit_Process ();
	}

	Log (USR_OK, "file write[%s:%d:%d]",
		OFN(D_K,P_K,OD_SEQ+2), OFW(D_K,P_K,OD_SEQ+2,0), rt);
	/* ************************************************** */

	/* Ω√¿Â∫∞ Ω√ºº¿« auto_use∏¶ ¡ı∞° Ω√ƒ—¡ÿ¥Ÿ. Ω√ººΩ≈»£∏¶ πﬁ±‚¿ß«ÿº≠ */
	if (min_flag == 1)
	{
		for (i = 0; i < Shm_Strrg[0].SamPle_St01[OD_SEQ].tot_item_cnt; i++)
		{
			mk_gbn	 = Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[i].market_gbn;
			item_seq = Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[i].item_seq;

			if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].auto_use < 0)
				Shm_Risk[0].S_Sise[mk_gbn][item_seq].auto_use = 0;
			else
				Shm_Risk[0].S_Sise[mk_gbn][item_seq].auto_use--;
		}
	}

	/* ¿¸∑´¿Ã ¡æ∑·√≥∏Æ∏¶ ¿ß«— Set¿ª «—¥Ÿ */
	Shm_Strrg[0].auto_run_flag[OD_SEQ] = 0;
	PROC(D_K,P_K).start_status = JOB_END;
	PROC(D_K,P_K).process_status = 9;

	/* Daemonø°∞‘ ¡Ô∞¢¿˚¿Œ ¡æ∑·ø‰√ª¿ª ¿ß«ÿ signal ¡÷±‚ */
	sprintf (fifo_name, "%s/PA/%s", _FEP_FIFO, INFO(D_K).daemon_FIFO_name);
	DFIFD(D_K) = open (fifo_name, O_RDWR | O_NDELAY);

	if (DFIFD(D_K) == -1)
	{
		Log (FIF_FATAL, "cannot open daemon FIFO[%s] {%d:%s}",
			fifo_name, SYS_NO, SYS_STR);
		exit (FAIL);
	}

	write (DFIFD(D_K), "1", 1);
}	/* End of Set_In_Stop () */

/***********************************************************
	Function 		: . Set_Response
	Parameters IN 	: . 
	Parameters OUT 	: .
	Return Code 	: . int
	Comment 		: . 
***********************************************************/
/*--------------------------------------------------------*/
void	Set_Response(void)
/*--------------------------------------------------------*/
{
	/* ø©±‚ø°º≠ ¿¿¥‰/√º∞·¿ª πﬁ∞Ì ¡∂∞«ø° ∏¬∞‘ «œΩ√∏È µÀ¥œ¥Ÿ. */

	return;
}	/* End of Set_Response () */

/***********************************************************
	Function 		: . Od_Write_Data
	Parameters IN 	: . mk_gbn : Ω√¿Â±∏∫– (1~10)	1:√§±«, 2:±›¿∂∆ƒª˝
					: . nmc_gbn : 1:Ω≈±‘, 2:¡§¡§, 3:√Îº“
					: . p_flag : write falg
                        1 : ¿œπ›
                        2 : LP
					: . mm_gbn : 1:∏≈µµ, 2:∏≈ºˆ
					: . arry : ¡÷πÆ≥æ ¥ÎªÛ¿« arry(ºˆΩ≈πﬁ¿∫ ¡æ∏Ò¡ﬂ ∏Óπ¯¬∞)
	Parameters OUT 	: .
	Return Code 	: . int
	Comment 		: . 
***********************************************************/
/*--------------------------------------------------------*/
int		Od_Write_DataBond( int option, char *data, int sz)
{
	int		rt, o_gbn = 0;
	char	m_time[24], str[10];
	char	cdata[30];
	char	tmp[128];
	char	W_DFmt[1024];

	double	cal_price;

	Log( USR_OK, "Od_Write_DataBond start ...");

	/* buffer initial */
	memset (m_time, 0, sizeof (m_time));
	memset (&W_Fmt, ' ', sizeof( FILE_BUFF_FORMAT));

	/* header set */
	Log( USR_OK, "OFW_CNT=[%d]", OFW_CNT( 0,0));
	Log( USR_OK, "data_sz=[%d]", sizeof(KRX_JUMUN_DATA));
	Log( USR_OK, "form   =[%d]", sizeof(FILE_BUFF_FORMAT));
	
	Get_MicroTime (m_time);
	// ItoAf (OFW_CNT(p_flag-1,0) + 1, W_Fmt.If_Seq, sizeof (W_Fmt.If_Seq));
	ItoAf( OFW_CNT(0,0) + 1, W_Fmt.If_Seq, sizeof (W_Fmt.If_Seq));
	sprintf( cdata, "%-8.8s", ApType);
	memcpy( W_Fmt.ApType, cdata, sizeof (W_Fmt.ApType));
	memcpy( W_Fmt.ResponseCode, RES_NORMAL, strlen (RES_NORMAL));
	memcpy( W_Fmt.RecvTime1, m_time, sizeof (W_Fmt.RecvTime1));
	memcpy( W_Fmt.RecvTime2, &m_time[sizeof(W_Fmt.RecvTime1)], sizeof (W_Fmt.RecvTime2));

#if 0
/* data header (20 bytes)   */
typedef struct {
    char    Length[4];                  /* Data (20)     */
    char    DataSeq[8];                 /* Data                 */
    char    ResponseCode[4];            /*                      */
    char    LineFlag[3];                /*  ()  */
    char    Filler[1];                  /*                          */
}   FILE_DATA_HEAD;
#endif

	sprintf( cdata, "%04d%08d00000000", HEAD_SIZE + ORDER_DATA_SIZE, OFW_CNT(0,0) +1);
	memcpy( W_Fmt.DataHeader, cdata, sizeof( FILE_DATA_HEAD));

	memset (W_DFmt, 0x20,	sizeof(W_DFmt));
	memcpy (W_DFmt,	&W_Fmt,	sizeof(BUFF_RW_HEAD));
	/* ¡÷πÆ data copy */
	memcpy (&W_DFmt[sizeof(BUFF_RW_HEAD)], data, 400);
	W_DFmt[400+sizeof(BUFF_RW_HEAD)] = '\n';
#if 0
	/* ¡÷πÆ data copy */
	memcpy (&Order_St[sizeof (BUFF_RW_HEAD)], data, sizeof(KRX_JUMUN_DATA));
	Log( USR_OK, "    data=[%s]", data);

	/* ¡÷πÆº€Ω≈ √≥∏Æ */
	memcpy (&W_Fmt, Order_St, sizeof (BUFF_RW_HEAD) + ODS(D_K,P_K,0));
	W_Fmt.LineFeed[0] = '\n';
#endif

	if( option == 0)					// √§±«¿œπ›
	{
		Log( USR_OK, "    √§±«¿œπ›");
		o_gbn = 0;
		// rt = DSHM_W (TS_W1_1, (void *)W_DFmt, 1);
	}
	else									 
	if( option == 1)					// √§±«LP
	{
		Log( USR_OK, "    √§±«LP");
		// TEST pa_1101_ts¿Ã ¿Ã √§±« ¡÷πÆ º€Ω≈ ¿Ã¥œ±Ó 0¿∏∑Œ ≥™∞°æﬂ «œ¥¬∞Õ æ∆¥—¡ˆ o_gbn = 1;
		o_gbn = 0;
		rt = DSHM_W (TS_W1_1, (void *)W_DFmt, 1);
	}
	else											// ±›¿∂∆ƒª˝
	{
		Log( USR_OK, "    ±›¿∂∆ƒª˝ Data[%s]", W_DFmt);
		o_gbn = 2;
		// rt = DSHM_W (TS_W1_1, (void *)W_DFmt, 1);
	}

	FILE_BUFF_FORMAT_Print( ( FILE_BUFF_FORMAT *)W_DFmt);
	Log( USR_OK, "W_DFmt=[%d:%s]", sizeof(W_DFmt), W_DFmt);


	if (rt != 1)
	{
		Log (SAM_FATAL, "shm write fail [%s]", ODN(D_K,P_K,o_gbn));
		return (NOTOK);
	}

	Log (SAM_OK, "file write [%s:%d:%d]", ODN(D_K,P_K,o_gbn), ODW(D_K,P_K,0,o_gbn), rt);

#if 0
	/* Clientø° º€Ω≈ */
	rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);
	if (rt != 1)
	{
		Log (SAM_FATAL, "file write fail [%s]", ODN(D_K,P_K,0));
		return (NOTOK);
	}
		
	Log (USR_OK, "fail write [%s:%d:%d]", OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), rt);
#endif
	return (OK);
}





/***********************************************************
	Function 		: . Od_Write_Data
	Parameters IN 	: . mk_gbn : Ω√¿Â±∏∫– (1~10)	1:√§±«, 2:±›¿∂∆ƒª˝
					: . nmc_gbn : 1:Ω≈±‘, 2:¡§¡§, 3:√Îº“
					: . p_flag : write falg
                        1 : ¿œπ›
                        2 : LP
					: . mm_gbn : 1:∏≈µµ, 2:∏≈ºˆ
					: . arry : ¡÷πÆ≥æ ¥ÎªÛ¿« arry(ºˆΩ≈πﬁ¿∫ ¡æ∏Ò¡ﬂ ∏Óπ¯¬∞)
	Parameters OUT 	: .
	Return Code 	: . int
	Comment 		: . 
***********************************************************/
/*--------------------------------------------------------*/
int		Od_Write_Data(int mk_gbn, int nmc_gbn, int p_flag, int mm_gbn, int arry)
/*--------------------------------------------------------*/
{
	int		rt, o_gbn = 0;
	char	m_time[24], str[10];
	char	cdata[30];
	char	tmp[128];

	double	cal_price;

	Log( USR_OK, "Od_Write_Data start ...");

	memset (m_time, 0, sizeof (m_time));
	memset (&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));

	/* KRX ¡÷πÆ∆˜∏À √ ±‚∞™¿ª ∫πªÁ«ÿµ–¥Ÿ */
	memcpy (&Order_St[sizeof (BUFF_RW_HEAD)], Order_Dat, sizeof(KRX_JUMUN_DATA));

	/* KRX¡÷πÆ∆˜∏À ±‚∫ª¿∫ ≥÷æ˙∞Ì « ø‰«— «◊∏Ò¿ª Update «ÿ¡ÿ¥Ÿ */
	if (mk_gbn == 1)			// √§±«¿œπ›¡÷πÆ
	{
		/* ************************************************************************ */
		/* ∫Øºˆ∞™ ≥÷±‚ */

		/* ¡÷πÆπ¯»£ √§π¯ */
		Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO++;		// ∆ƒª˝¿∫ ....Order_No[1]....
		if (Shm_Risk[0].Order_No[0][OD_SEQ].START_JMNO > Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO	||
			Shm_Risk[0].Order_No[0][OD_SEQ].END_JMNO  <= Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO)
		{
			Log (USR_ERROR, "¡÷πÆπ¯»£ ªÁøÎ øµø™ FULL!!! jm_no[%ld]", Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO);
			// TEST return (-1);
		}

		/* TR Code, 11 ø©±‚º≠¥¬ ¿¸∑´ªÛ Ω≈±‘¡÷πÆ∏∏ ≥÷±‚ ∂ßπÆø° ºˆ¡§«œ¡ˆ æ ¿Ω
		if (nmc_gbn == 1)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+11], "TCHODR10001", 11);
		else if (nmc_gbn == 2)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+11], "TCHODR10002", 11);
		else if (nmc_gbn == 3)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+11], "TCHODR10003", 11);	*/

		/* ∫∏µÂIDµµ ≥÷æÓæﬂ «œ¥¬µ• G1¿Ãø‹¿« ∞ÊøÏø° ∞≈∑°«œ∏È æ»µ«¥œ±Ó Default∑Œ G1æ¥¥Ÿ */
	
		/* ¡÷πÆID, 34∫Œ≈Õ */
		memset (tmp, 0, sizeof (tmp));
		sprintf (tmp, "%010ld", Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO);
		memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+34], tmp, 10);

		/* Ω≈±‘∂Û ø¯¡÷πÆπ¯»£¥¬ ≥÷¡ˆ æ ¥¬¥Ÿ. 44∫Œ≈Õ
 		   ¡§¡§,√Îº“¥¬ ≥ª∞°≥¬¥¯ ¡÷πÆπ¯»£∏¶ ∫∏∞¸«œ∞Ì ¿÷¥Ÿ∞° ¡∂∞«
		   ∫∏∞¸¿∫ ¿¿¥‰/√º∞· ºˆΩ≈Ω√ «œ¥¯∞° ¡÷πÆ≥æΩ√ «œ¥¯∞°.... ∞·¡§
		   ≥ª∞°≥Ω ¡÷πÆø° ¥Î«— πÃ√º∞·∞¸∏Æ∏¶ «ÿæﬂ «—¥Ÿ¥¬ ¿«πÃ¿Ã¥Ÿ. ∞¯¿Ø∏ﬁ∏∏Æ Struct∏¶ ∏∏µÈæÓº≠ «œ∏È µ»¥Ÿ. 
		if (nmc_gbn == 2 || nmc_gbn == 3)		// ¡§¡§, √Îº“
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+44], xxxxxxxxxx, 10);	*/

		/* ¡æ∏Òƒ⁄µÂ, 54∫Œ≈Õ */
#if 0
		memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+54],
				Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].m_item_cd, 12);
#else
		memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+54], "KR103502GF62", 12);
#endif

		/* ∏≈µµ∏≈ºˆ ±∏∫–, 66 */
		if (mk_gbn == 1)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+66], "1", 1);
		else if (mk_gbn == 2)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+66], "2", 2);
		else
		{
			Log (USR_ERROR, "∏≈µµ∏≈ºˆ±∏∫– Error!!! mm_gbn[%d]", mm_gbn);
			return (-2);
		}
			
		/* ¡§¡§/√Îº“±∏∫–, 67 */
		if (nmc_gbn == 1)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+67], "1", 1);
		else if (nmc_gbn == 2)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+67], "2", 1);
		else if (nmc_gbn == 3)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+67], "3", 1);

		/* ∞Ë¡¬π¯»£(«ˆπ∞), 68 */
		memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+68], Shm_Strrg[0].SamPle_St01[OD_SEQ].st_accno, 12);

		/* »£∞°ºˆ∑Æ, 80 */
		memset (tmp, 0, sizeof (tmp));
		sprintf (tmp, "%010d", Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_cnt);
		memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+80], tmp, 10);

		/* »£∞°∞°∞› , 90 */
		memset (tmp, 0, sizeof (tmp));
		if (mm_gbn == 1)		// ∏≈µµ 
		{
			if (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_type == 1)	// 1:¡ˆ¡§∞°
			{
				if (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_gbn == 0)		// 0:∏≈µµ1»£∞°(¿⁄±‚»£∞°), 1:∏≈ºˆ1»£∞°(ªÛ¥Î»£∞°)
				{
					cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].sell_1_price;
				}
				else
				{
					cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].buy_1_price;
				}

/*
				cal_price = Hoga_Change (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].market_gbn,
										 Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq,
										 0,			// ≥™∏”¡¯ 0(Default), ¡÷Ωƒº±π∞∏∏ ªÁøÎ, 0:ƒ⁄Ω∫««, 1:ƒ⁄Ω∫¥⁄
										 Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_condition,
										 cal_price);
*/
				cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].m_l_lmt;
			}
			else	// 2.Ω√¿Â∞°, A0¿« «œ«—∞°∞›¿∏∑Œ ¡÷πÆ
			{
				cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].m_l_lmt;
			}
		}
		else		// ∏≈ºˆ
		{
			if (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_type == 1)		//1: ¡ˆ¡§∞°
			{
				if (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_gbn == 0)	// 0:∏≈ºˆ1»£∞°(¿⁄±‚»£∞°), 1:∏≈µµ1»£∞°(ªÛ¥Î»£∞°)
				{
					cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].buy_1_price;
				}
				else
				{
					cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].sell_1_price;
				}
					
/*
				cal_price = Hoga_Change (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].market_gbn,
										 Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq,
										 0,			// ≥™∏”¡¯ 0(Default), ¡÷Ωƒº±π∞∏∏ ªÁøÎ, 0:ƒ⁄Ω∫««, 1:ƒ⁄Ω∫¥⁄
										 Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_condition,
										 cal_price);
*/
				cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].m_h_lmt;
			}
			else	// 2.Ω√¿Â∞°, A0¿« ªÛ«—∞°∞›¿∏∑Œ ¡÷πÆ
			{
				cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].m_h_lmt;
			}
		}

		sprintf (tmp, "%011ld", (long)cal_price);
		memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+90], tmp, 11);
			
		/* »£∞°¿Ø«¸ƒ⁄µÂ, ø©±‚º≠¥¬ ¡ˆ¡§∞°∑Œ √≥∏Æ«ﬂ¥Ÿ. ªÛ/«œ«—∞°∑Œ ¡÷πÆ≥æ∂ßµµ ¡ˆ¡§∞°∑Œ ªÛ«œ«—∞° ∞°∞›¿∏∑Œ ≥ª∏È µ»¥Ÿ. ≥™∏”¡ˆ¥¬ ¿¸∑´ø° µ˚∂Û ªÁøÎ«œ∏È µ»¥Ÿ. */
		/* »£∞°¡∂∞«ƒ⁄µÂ, ø©±‚º≠¥¬ ¥ÎªÛ¿Ã æ∆¥œ¥Ÿ. Default(0)ªÁøÎ, ºˆ¡§« ø‰æ¯¿Ω √ ±‚∞™¿Ã 0¿”	*/
		/* Ω√¿Â¡∂º∫¿⁄»£∞°±∏∫–π¯»£, ø©±‚º≠¥¬ Default(0) ªÁøÎ, LP¥¬ 1:LP»£∞°∑Œ Set « ø‰ 		*/
		/* ±‚≈∏µÓµÓ ¿¸∑´ π◊ ∞Ë¡¬ªÛ≈¬ ¡÷πÆ¡∂∞«µÓø° µ˚∂Û ≥™∏”¡ˆ ∞™µÈµµ ∏¬∞‘ √≥∏Æ«œ∏È µ ...	*/

		/* ******************************************************************** */
		/* 261Byte¿« KRX¡÷πÆ ∆˜∏À¿ª ∏∏µÈ∞Ì ∏∂¡ˆ∏∑¿∏∑Œ »∏ø¯ªÁ√≥∏Æ«◊∏Ò ∞™ º≥¡§ */
		/* »∏ø¯ªÁ√≥∏Æ«◊∏Ò 60πŸ¿Ã∆Æ¡ﬂ æ’ 30πŸ¿Ã∆Æ¥¬ ø¯¿Âø°º≠ ø‰√ª«— ∞™¿ª ≥÷æÓæﬂ «œ∞Ì, µ⁄ 30πŸ¿Ã∆Æ¥¬ æ∆∑°¿« ¡∂∞«ø° ∏¬∞‘ Setting«ÿæﬂ«‘ (Clientµµ µø¿œ)	*/
		/* 30			: A(º≠πˆ¿⁄µø¡÷πÆ), C(Clientø°º≠ ≥Ω ¡÷πÆ)				*/
		/* 31, 32		: ST(Strategy)											*/
		/* 33, 34		: ¿¸∑´π¯»£ 01 ~ 99										*/
		/* 35, 36		: Ω√¿Â±∏∫–												*/
		/* 				  1(¡ˆºˆº±π∞) 2(¡ˆºˆø…º«) 3(¡÷Ωƒº±π∞) 4(¡÷Ωƒø…º«)	
						  5(¿Ø∞°¡ı±«/ELW/ETF/ETN) 6(ƒ⁄Ω∫¥⁄)
						  8(KRX300) 9(Kosdaq150 Futures) 10(Kosdaq150 Options)	*/
		/* 37,38,39,40,41 : A0¿« Seqπ¯»£ Ex) 236 => (00236)						*/
		/* 42, 43		: ∞Ë¡¬π¯»£ Seq											*/
		/* 44           : 0 (Default, ¿Ø∞°¡ı±«∞˙ ¡÷Ωƒº±π∞∏∏ º±≈√ ≥™∏”¡¯ 0)					
						: ¿Ø∞°¡ı±«(5)¿œ ∞ÊøÏ => 0(Normal), 1(ELW), 2(ETN), 3(ETF)
						: ¡÷Ωƒº±π∞(3)¿œ ∞ÊøÏ => «ÿ¥Á¡æ∏Ò¿Ã ƒ⁄Ω∫««∏È 0, ƒ⁄Ω∫¥⁄¿Ã∏È 1	*/
		/* 45           : Ω∫«¡∑πµÂø©∫Œ, 0:normal, 1:Ω∫«¡∑πµÂ					*/
		/* 46,47,48,49	: Process Nick Name
						  pa_50101mp => 0101, pa_50503mp => 0203				*/
		/* ******************************************************************** */
 
	}
	else		// ∆ƒª˝Ω√¿Â 
	{
		/* ∆ƒª˝Ω√¿Âø° ∏¬∞‘ KRX¿¸πÆ √§øÏ±‚(¿ß «ˆπ∞ ¬¸¡∂)	*/
	}

	/* *************************************************************** */
	/* ¡÷πÆº€Ω≈ √≥∏Æ */
	memcpy (&W_Fmt, Order_St, sizeof (BUFF_RW_HEAD) + ODS(D_K,P_K,0));
	W_Fmt.LineFeed[0] = '\n';

	if (mk_gbn == 1 && p_flag == 1)					// √§±«¿œπ›
	{
		Log( USR_OK, "    √§±«¿œπ›");
		o_gbn = 0;
		rt = SEAM_ORD_W (SEAM_ORDQ_BOND, (void *)&W_Fmt, SEAM_ORD_WIRE_RECSZ);
	}
	else									 
	if (mk_gbn == 1 && p_flag == 2)					// √§±«LP
	{
		Log( USR_OK, "    √§±«LP");
		// TEST pa_1101_ts¿Ã ¿Ã √§±« ¡÷πÆ º€Ω≈ ¿Ã¥œ±Ó ¿œ∑Œ ≥™∞°æﬂ «œ¥¬∞Õ æ∆¥—¡ˆ o_gbn = 1;
		o_gbn = 0;
		rt = SEAM_ORD_W (SEAM_ORDQ_BOND, (void *)&W_Fmt, SEAM_ORD_WIRE_RECSZ);
	}
	else											// ±›¿∂∆ƒª˝
	{
		Log( USR_OK, "    ±›¿∂∆ƒª˝");
		o_gbn = 2;
		rt = SEAM_ORD_W (SEAM_ORDQ_DERIV, (void *)&W_Fmt, SEAM_ORD_WIRE_RECSZ);
	}

	if (rt != 1)
	{
		Log (SAM_FATAL, "shm write fail [%s]", ODN(D_K,P_K,o_gbn));
		return (NOTOK);
	}

	Log (SAM_OK, "file write [%s:%d:%d]", ODN(D_K,P_K,o_gbn), ODW(D_K,P_K,0,o_gbn), rt);

#if 0
	/* Clientø° º€Ω≈ */
	rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);
	if (rt != 1)
	{
		Log (SAM_FATAL, "file write fail [%s]", ODN(D_K,P_K,0));
		return (NOTOK);
	}
		
	Log (USR_OK, "fail write [%s:%d:%d]", OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), rt);
#endif
	return (OK);
}	/* End of Write_Data() */

/*************************************************************************
    Function  : . Fifo_Event_Rtn
    Parameters IN : .
    Parameters OUT : .
    Return Code  : . void
    Comment   : . get the management FIFO signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Fifo_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
    char tmp[2];

    read (START_FD, tmp, 1);

    return;
}   /* End of Fifo_Event_Rtn () */

/*************************************************************************
    End of Program (pa_5050_mp.c)
*************************************************************************/

