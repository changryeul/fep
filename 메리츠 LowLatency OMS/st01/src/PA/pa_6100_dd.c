#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 시세 수신 (UDP)
#	File	: pa_6100_dd.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

/* Master */
#if defined	A6101||A6301||A6401||A6501
#define		DATA_SIZE	400
/* 정산 */
#elif defined	A6100||A6300||A6400||A6500
#define		DATA_SIZE	100
/* Sise */
#elif defined	A6102||A6103||A6104||A6104||A6105||A6106||A6107 || A6202||A6302||A6402||A6502
#define		DATA_SIZE	450
#endif
#include    "buf_struct.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
#define		DATA_TIME       60 * 1000

KS_LONGCODE	Key;
char		ApType[10];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_6100_DD (void);
void    Set_Sise (char *);
void	Microsec_Sleep (int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_6100_DD ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_6100_DD (void)
/*----------------------------------------------------------------------*/
{
	int				idx, rt, len, R_Cnt, i;
	char 			m_time[24], path[100], f_buf[100];
	FILE_BUFF_FORMAT    R_Fmt[1];

/* Master */
#if defined A6101
    Shm_CME[0].total_item_cnt = 0;

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_CME; i++)
	{
		memset (Shm_CME[i].A0.type, 	0, sizeof (OC_MASTER));
		memset (Shm_CME[i].Q.type, 		0, sizeof (OC_Q));
		memset (Shm_CME[i].B.type, 		0, sizeof (OC_B));
		memset (Shm_CME[i].S.type, 		0, sizeof (OC_S));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].CME_Key,		0,	sizeof (KS_LONGCODE) * SHM_MAX_CME);
#elif defined A6301
    Shm_SGX[0].total_item_cnt = 0;

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_SGX; i++)
	{
		memset (Shm_SGX[i].A0.type, 	0, sizeof (OC_MASTER));
		memset (Shm_SGX[i].Q.type, 		0, sizeof (OC_Q));
		memset (Shm_SGX[i].B.type, 		0, sizeof (OC_B));
		memset (Shm_SGX[i].S.type, 		0, sizeof (OC_S));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].SGX_Key,		0,	sizeof (KS_LONGCODE) * SHM_MAX_SGX);
#elif defined A6401
    Shm_ERX[0].total_item_cnt = 0;

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_ERX; i++)
	{
		memset (Shm_ERX[i].A0.type, 	0, sizeof (OC_MASTER));
		memset (Shm_ERX[i].Q.type, 		0, sizeof (OC_Q));
		memset (Shm_ERX[i].B.type, 		0, sizeof (OC_B));
		memset (Shm_ERX[i].S.type, 		0, sizeof (OC_S));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].ERX_Key,		0,	sizeof (KS_LONGCODE) * SHM_MAX_ERX);
#elif defined A6501
    Shm_HKE[0].total_item_cnt = 0;

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_HKE; i++)
	{
		memset (Shm_HKE[i].A0.type, 	0, sizeof (OC_MASTER));
		memset (Shm_HKE[i].Q.type, 		0, sizeof (OC_Q));
		memset (Shm_HKE[i].B.type, 		0, sizeof (OC_B));
		memset (Shm_HKE[i].S.type, 		0, sizeof (OC_S));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].HKE_Key,		0,	sizeof (KS_LONGCODE) * SHM_MAX_HKE);
#endif

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

		rt = Poll_File (60*1000);

		if (rt == 1)
			SLog (USR_OK, "poll timeout <%d>", INT_SEQ);
		else if (rt == -1)
			continue;
	}

	return;
}	/* End of PA_6100_DD ()	*/

/*************************************************************************
    Function        : . Set_Sise
    Parameters IN   : . p_buf   : received data
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . set sise data SHM (현재가)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Set_Sise (char *p_buf)
/*----------------------------------------------------------------------*/
{
    int         idx, rt, ii;
	char		item_cd[20];
    char        TrCode[10];

/* *********************************** */
/* 시장별, TR별 종목 일련번호 갖고오기 */
/* *********************************** */
#if defined A6101
	if (memcmp (&p_buf[20], "FCME", 4) == 0)
	{
		/* index search */
		memset (&Key, 0, sizeof (Key));
        sprintf (Key.longcode, "%-20.20s", &p_buf[28]);
        rt = 0;
        rt = Key_Search (MK_CME, KEY_LONGCODE, (char *)&Key);
        if (rt >= 0)
            idx = rt;
        else
        {
            idx = Shm_CME[0].total_item_cnt;

            Shm_Item[0].CME_Key[idx].idx = idx;
            memset (Shm_Item[0].CME_Key[idx].longcode, 0, 20);
            memcpy (Shm_Item[0].CME_Key[idx].longcode, &p_buf[28], 20);
            Shm_CME[0].total_item_cnt++;
            qsort (Shm_Item[0].CME_Key, Shm_CME[0].total_item_cnt, sizeof (KS_LONGCODE), CmpLongcode); 
		}
        memcpy (Shm_CME[idx].A0.type, p_buf, sizeof (OC_MASTER));
	}
#elif defined A6301
	if (memcmp (&p_buf[20], "FSGX", 4) == 0)
	{
		/* index search */
		memset (&Key, 0, sizeof (Key));
        sprintf (Key.longcode, "%-20.20s", &p_buf[28]);
        rt = 0;
        rt = Key_Search (MK_SGX, KEY_LONGCODE, (char *)&Key);
        if (rt >= 0)
            idx = rt;
        else
        {
            idx = Shm_SGX[0].total_item_cnt;

            Shm_Item[0].SGX_Key[idx].idx = idx;
            memset (Shm_Item[0].SGX_Key[idx].longcode, 0, 20);
            memcpy (Shm_Item[0].SGX_Key[idx].longcode, &p_buf[28], 20);
            Shm_SGX[0].total_item_cnt++;
            qsort (Shm_Item[0].SGX_Key, Shm_SGX[0].total_item_cnt, sizeof (KS_LONGCODE), CmpLongcode); 
		}
        memcpy (Shm_SGX[idx].A0.type, p_buf, sizeof (OC_MASTER));
	}
#elif defined A6401
	if (memcmp (&p_buf[20], "FERX", 4) == 0)		// EUX=>ERX
	{
		/* index search */
		memset (&Key, 0, sizeof (Key));
        sprintf (Key.longcode, "%-20.20s", &p_buf[28]);
        rt = 0;
        rt = Key_Search (MK_ERX, KEY_LONGCODE, (char *)&Key);
        if (rt >= 0)
            idx = rt;
        else
        {
            idx = Shm_ERX[0].total_item_cnt;

            Shm_Item[0].ERX_Key[idx].idx = idx;
            memset (Shm_Item[0].ERX_Key[idx].longcode, 0, 20);
            memcpy (Shm_Item[0].ERX_Key[idx].longcode, &p_buf[28], 20);
            Shm_ERX[0].total_item_cnt++;
            qsort (Shm_Item[0].ERX_Key, Shm_ERX[0].total_item_cnt, sizeof (KS_LONGCODE), CmpLongcode); 
		}
        memcpy (Shm_ERX[idx].A0.type, p_buf, sizeof (OC_MASTER));
	}
#elif defined A6501
	if (memcmp (&p_buf[20], "FHKE", 4) == 0)		// HKG => HKE
	{
		/* index search */
		memset (&Key, 0, sizeof (Key));
        sprintf (Key.longcode, "%-20.20s", &p_buf[28]);
        rt = 0;
        rt = Key_Search (MK_HKE, KEY_LONGCODE, (char *)&Key);
        if (rt >= 0)
            idx = rt;
        else
        {
            idx = Shm_HKE[0].total_item_cnt;

            Shm_Item[0].HKE_Key[idx].idx = idx;
            memset (Shm_Item[0].HKE_Key[idx].longcode, 0, 20);
            memcpy (Shm_Item[0].HKE_Key[idx].longcode, &p_buf[28], 20);
            Shm_HKE[0].total_item_cnt++;
            qsort (Shm_Item[0].HKE_Key, Shm_HKE[0].total_item_cnt, sizeof (KS_LONGCODE), CmpLongcode); 
		}
        memcpy (Shm_HKE[idx].A0.type, p_buf, sizeof (OC_MASTER));
	}
#elif defined A6100		// 정산
	if (memcmp (&p_buf[20], "FCME", 4) == 0)
	{
		/* index search */
		memset (&Key, 0, sizeof (Key));
        sprintf (Key.longcode, "%-20.20s", &p_buf[28]);
        rt = 0;
        rt = Key_Search (MK_CME, KEY_LONGCODE, (char *)&Key);
        if (rt >= 0)
		{
            idx = rt;
			memcpy (Shm_CME[idx].S.type, p_buf, sizeof (OC_S));
		}
        else
        {
			Log (USR_ERROR, "%1.1s %4.4s Key Not Found [%20.20s]", p_buf, &p_buf[20], &p_buf[28]);
		}
	}
#elif defined A6300		// 정산
	if (memcmp (&p_buf[20], "FSGX", 4) == 0)
	{
		/* index search */
		memset (&Key, 0, sizeof (Key));
        sprintf (Key.longcode, "%-20.20s", &p_buf[28]);
        rt = 0;
        rt = Key_Search (MK_SGX, KEY_LONGCODE, (char *)&Key);
        if (rt >= 0)
		{
            idx = rt;
			memcpy (Shm_SGX[idx].S.type, p_buf, sizeof (OC_S));
		}
        else
        {
			Log (USR_ERROR, "%1.1s %4.4s Key Not Found [%20.20s]", p_buf, &p_buf[20], &p_buf[28]);
		}
	}
#elif defined A6300		// 정산
	if (memcmp (&p_buf[20], "FERX", 4) == 0)		// EUX => ERX
	{
		/* index search */
		memset (&Key, 0, sizeof (Key));
        sprintf (Key.longcode, "%-20.20s", &p_buf[28]);
        rt = 0;
        rt = Key_Search (MK_ERX, KEY_LONGCODE, (char *)&Key);
        if (rt >= 0)
		{
            idx = rt;
			memcpy (Shm_ERX[idx].S.type, p_buf, sizeof (OC_S));
		}
        else
        {
			Log (USR_ERROR, "%1.1s %4.4s Key Not Found [%20.20s]", p_buf, &p_buf[20], &p_buf[28]);
		}
	}
#elif defined A6300		// 정산
	if (memcmp (&p_buf[20], "FHKE", 4) == 0)		// HKG => HKE
	{
		/* index search */
		memset (&Key, 0, sizeof (Key));
        sprintf (Key.longcode, "%-20.20s", &p_buf[28]);
        rt = 0;
        rt = Key_Search (MK_HKE, KEY_LONGCODE, (char *)&Key);
        if (rt >= 0)
		{
            idx = rt;
			memcpy (Shm_HKE[idx].S.type, p_buf, sizeof (OC_S));
		}
        else
        {
			Log (USR_ERROR, "%1.1s %4.4s Key Not Found [%20.20s]", p_buf, &p_buf[20], &p_buf[28]);
		}
	}
#endif

    return;
}   /* End of Set_Sise ()   */

/*************************************************************************
	Function		: . Microsec_Sleep
	Parameters IN	: . msec	: sleep time (in microsec)
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . sleep for microsec
*************************************************************************/
void	Microsec_Sleep (int msec)
{
	int				rt;
	struct timespec	ts;

	ts.tv_sec = 0;
	ts.tv_nsec = msec * 1000;

	rt = nanosleep (&ts, NULL);

	if (rt == -1)
		Log (SYS_ERROR, "nanosleep fail {%d:%s}", SYS_NO, SYS_STR);

	return;
}	/* End of Microsec_Sleep ()	*/

/*************************************************************************
	End of program (pa_6100_dd.c)
*************************************************************************/ 
