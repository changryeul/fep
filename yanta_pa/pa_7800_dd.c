#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: RDS시세수신 DD
#	File	: pa_7800_dd.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#if defined A7801
#define     DATA_SIZE       1200
#elif defined A7802
#define     DATA_SIZE       350
#elif defined A7803
#define     DATA_SIZE       100
#endif

#include    "buf_struct.h"
/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define            DATA_TIME       60 * 1000
/* 시세 Seq 위치 Size */
#define		S_H_SIZE		17						/* TR(5) + Code(12) */
/* 기본마스터 Seq 위치 Size */
#define		M_H_SIZE		30	/* TR(5) + JCNT(5) + DATE(8) + Code(12)	*/
KS_EXPCODE	Key;

int			Che_Gbn;
char		ApType[10];
char		d_time[16];
FILE_BUFF_FORMAT		R_Fmt[1];
BUFF_RW_HEAD        	f_head;

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_7800_DD (void);
void	Set_Sise (char *);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_7800_DD ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_7800_DD (void)
/*----------------------------------------------------------------------*/
{
	int				i, rt, len, R_Cnt;
	char 			m_time[24];
	char			fifo_name[100], bumun[4];
	char			W2_Fmt[2048];

	/* get date and time (yyyymmddhhmmss) */
	Get_DateTime (d_time);

	/* ******************************************************* */
	/* RDS파일 읽어서 A0파일대상 Write 하기 (ex. 7801 => 7181) */
	/* ******************************************************* */
	while (START_S != JOB_END)
	{
		Stat_Save ();

		while (START_S != JOB_END)
		{
			Stat_Save ();
			memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT));

			R_Cnt = F_R(PS_R_1, (void *)R_Fmt, 1);
			if (R_Cnt < 0)
        	{
            	Log (SAM_FATAL, "cannot read file[%s,%d:%s]",
                	IFN(D_K,P_K,0), SYS_NO, SYS_STR);
            	sleep (1);
            	Exit_Process ();
        	}
        	else if (R_Cnt == 0)
            	break;

			Set_Sise (R_Fmt[0].Data);

			Set_TR_Time ();
			INT_SEQ ++;

			Add_Count(PS_R_1, 1);
		}

        rt = Poll_File (DATA_TIME);

        if (rt == 1)
            Log (USR_OK, "poll timeout <%d>", INT_SEQ);
        else if (rt == -1)
            continue;
	}

	return;
}	/* End of PA_7800_DD ()	*/

/*************************************************************************
	Function		: . Set_Sise
	Parameters IN	: . p_buf	: received data
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . set sise data SHM (현재가)
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Set_Sise (char *p_buf)
/*----------------------------------------------------------------------*/
{
	int			ii, s_k, idx, rt;
	char		W_Fmt[2048];

#if defined A7801
	/* RDS 채권종목정보 읽어서 대상이면 7181에 Write */
	CO_A001_RDS01 *dat = (CO_A001_RDS01 *)p_buf;

	if ((memcmp (dat->tr_code, 			"TRDESP50101", 11) == 0)	&&		// 채권종목정보
		(memcmp (dat->biz_date, 		d_time, 		8) == 0)	&&		// 영업일자
		(memcmp (dat->bond_Ipo_gbn_cd,	"Y", 			1) == 0)	&&		// 상장여부
		(memcmp (dat->bond_type_cd,		"GB", 			2) == 0)	)		// 채권유형코드(GB:국채)
#elif defined A7802
	/* RDS KTS종목정보 읽어서 대상이면 7181에 Write */
	CO_A001_RDS02 *dat = (CO_A001_RDS02 *)p_buf;

	if ((memcmp (dat->tr_code, 			"TRDESP50102", 11) == 0)	&&		// KTS종목정보
		(memcmp (dat->biz_date, 		d_time, 		8) == 0)	&&		// 영업일자
		(memcmp (dat->board_id,			"G1", 			2) == 0)	&&		// 장운영코드(보드ID), G1:정규장, KTS는 정규장만
		(memcmp (dat->trade_susp_yn,	"Y", 			1) == 0)	)		// 상장여부
#elif defined A7802
	if (1)
#endif
	{
		memset (W_Fmt, 0x20, sizeof (W_Fmt));
		memcpy (&W_Fmt, &R_Fmt[0], sizeof (FILE_BUFF_FORMAT)-1);
		W_Fmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

		rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);
		if (rt != 1)
		{
			Log (SAM_FATAL, "file write fail[%s:%d]", OFN(D_K,P_K,0), rt);
			Exit_Process ();
		}

			
	}

	return;
}	/* End of Set_Sise ()	*/

/*************************************************************************
	End of program (pa_7800_dd.c)
*************************************************************************/ 
