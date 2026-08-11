#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: Q 수신 (주문등)
#	File	: pa_5000_qr.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include    "pa_struct.h"


#if		defined A5001
#define     DATA_SIZE       400		// 채권주문
#else
#define     DATA_SIZE       450		// 금융파생
#endif
#include    "buf_struct.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define     DATA_TIME   60 * 1000

#define		READ_BUF_SIZE	1024
#define		QUEUE_MAX_BYTES	10485760

/* 55바이트 업무헤더 추가시 */
#if		defined A5001
#define     ADD_HEADER_SIZE	0		// 55=>0, 안쓰기로
#elif	defined A5011 || A5012			// 금융파생 주문수신
#define     ADD_HEADER_SIZE	YSK_DATA_HEAD_LEN
#endif

#if			defined A5001	// 채권     주문수신
#define     CHK_PROC        "pa_1101_ts"
#elif		defined A5011	// 금융파생 주문수신
#define     CHK_PROC        "pa_2101_ts"
#define     CHK_PROC1       "pa_2102_ts"
#elif		defined A5012	// 금융파생(야) 주문수신
#define     CHK_PROC        "pa_3101_ts"
#define     CHK_PROC1       "pa_3102_ts"
#endif

/* For Queue 
#define		MAXSIZE         4096
#define PERM                0x1B6
#define QUEUE_READY         -1
#define QUEUE_WAIT          -2
#define QUEUE_FULL          -3
#define QUEUE_NOT_EXIST     -4
#define QUEUE_TIME_OUT      -5
#define QUEUE_SEND_ERROR    -6
#define TRUE            	1
#define FALSE           	0
*/

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int					FIFO_fd, FIFO_fd1, Pk, Pk0, Pk1;
char				TrCode[8], ApType[10];
struct				ip_mreq		mreq;
struct sockaddr_in	SvrAddr, ClntAddr;


FILE_BUFF_FORMAT    W_Fmt[1];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_5000_QR (void);
void    Init_Parameters (void);
int		QueueClear(int);
int		GetQid(size_t);
int		CheckQueue(int);
int		MakeQueue(key_t);
int		ReceiveQueue(int, long, char *);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_5000_QR ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_5000_QR (void)
/*----------------------------------------------------------------------*/
{
	int		Chk_G1;
	int		i, rt, w_flag;
	int		msqid;
	char	fifo_name[100];
	char	m_time[24], key[12];
	char    u_time[24], tms_time[6], deci_time[6];
	char	FW_Fmt[8192];
	int     tms, deci, st_time, st_micro_time;
	key_t	new_key, base_key;

	Init_Parameters ();

	//Msgbuf	r_buf;
	char	r_buf[1024];

	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", 
							_Exe_Name, _Exe_Name+3, _Exe_Name+8);
	LtoU (ApType, strlen (ApType));


	// Get msg q ID
#if		defined A5001	// 채권 주문수신
	new_key = 0x22000001;
#elif	defined A5011	// 금융파생 주문수신
	new_key = 0x22000011;
#elif	defined A5012	// 금융파생(야) 주문수신
	new_key = 0x22000021;
#endif
#if	0
	if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
        new_key += 0x01000000L;
#endif

	Chk_G1 = 1;

    msqid = MakeQueue (new_key);
    if (msqid < 0)
    {
        Log(USR_ERROR, "msgget msqid[%d]", msqid);
		Exit_Process ();
    }
    else
        Log (USR_OK, "msgget: succeeded: msqid = %d new_key [%#x]", msqid, new_key);

	while (START_S != JOB_END)
	{
		memset (r_buf, 0, sizeof (r_buf));

		rt = ReceiveQueue(msqid, 0L, r_buf);
		if (rt < 0)
		{
			Log (USR_ERROR, "receive fail {%d:%s} rt[%d]", SYS_NO, SYS_STR, rt);
			Exit_Process ();
		}

		/* Data 수신은 "55Byte + 주문전문"				*/
		/*      송신도 "55Byte + 주문전문" 동일 Format	*/
		//Log (USR_OK, "Q Recv [%s][%d]", r_buf, strlen(r_buf));
		if (1)
		{
			/* **************************************************** */
			/* File Write											*/
			/* **************************************************** */
			memset (m_time, 0, sizeof (m_time));
			Get_MicroTime (m_time);

			/* File Write */
			memset (W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));

			/* write to DD file	*/
			ItoAf (INT_SEQ + 1, W_Fmt[0].If_Seq, sizeof (W_Fmt[0].If_Seq));
			memcpy (W_Fmt[0].ApType, ApType, sizeof (W_Fmt[0].ApType));
			memcpy (W_Fmt[0].ResponseCode, RES_NORMAL, strlen (RES_NORMAL));
			memcpy (W_Fmt[0].RecvTime1, m_time, sizeof (W_Fmt[0].RecvTime1));
			memcpy (W_Fmt[0].RecvTime2, &m_time[sizeof(W_Fmt[0].RecvTime1)],
											sizeof (W_Fmt[0].RecvTime2));

//			memset (W_Fmt[0].DataHeader, 0x20, sizeof(W_Fmt[0].DataHeader));
			/* 202509 송신Process가 기동되지 않으면 "REJC"으로 거부처리 한다. */
			if (TCP2_NSTAT(D_K,Pk0,S_K) == 1)	// 0:OFF, 1:ON, 2:END
			{
				w_flag = 0;		// 정상 
				if (Pk == 0 && TCP2_NSTAT(D_K,Pk1,S_K) == 1)
					Pk = 1;
				else
					Pk = 0;
			}
			else if (TCP2_NSTAT(D_K,Pk1,S_K) == 1)
			{
				w_flag = 0;		// 정상 
				if (Pk == 1 && TCP2_NSTAT(D_K,Pk0,S_K) == 1)
					Pk = 0;
				else
					Pk = 1;
			}
			else
				w_flag = 1;		// 거부(REJC)

#if		defined A5001	// 채권 주문수신
			if (w_flag)					// 거부(REJC)
			{
				/*
					일반 채권은 254, LP채권은 255 주문전문만 받음.
					KRX Header 82는 Space + 수신한 주문전문
				*/
				memcpy (W_Fmt[0].Data, "REJC00000000000", 15);											// 거부코드 세팅

				if ( memcmp (&r_buf[ADD_HEADER_SIZE+11], "TCHODR4000", 10) == 0 )
					memcpy (&W_Fmt[0].Data[15], r_buf, ADD_HEADER_SIZE + sizeof (KRX_NOTE_JUMUN_DATA));		// 채권일반, 254;
				else if ( memcmp (&r_buf[ADD_HEADER_SIZE+11], "TCHMOR4000", 10) == 0 )
					memcpy (&W_Fmt[0].Data[15], r_buf, ADD_HEADER_SIZE + sizeof (KRX_LP_NOTE_JUMUN_DATA));	// 채권LP, 255;
				else if ( memcmp (&r_buf[ADD_HEADER_SIZE+11], "TCHKOR10001", 11) == 0 )
					memcpy (&W_Fmt[0].Data[15], r_buf, ADD_HEADER_SIZE + 147);								// Kill Switch , KTS만 가능 (147);
				else
				{
					Log (USR_ERROR, "REJC Other TR Recv [%s] size[%d]", r_buf, strlen(r_buf));
					break;
				}
			}
			else
			{
				/*
					일반 채권은 254, LP채권은 255 주문전문만 받음.
					KRX Header 82는 Space + 수신한 주문전문
				*/
				if ( memcmp (&r_buf[ADD_HEADER_SIZE+11], "TCHODR4000", 10) == 0 )
					memcpy (W_Fmt[0].Data, r_buf, ADD_HEADER_SIZE + sizeof (KRX_NOTE_JUMUN_DATA));		// 채권일반, 254;
				else if ( memcmp (&r_buf[ADD_HEADER_SIZE+11], "TCHMOR4000", 10) == 0 )
					memcpy (W_Fmt[0].Data, r_buf, ADD_HEADER_SIZE + sizeof (KRX_LP_NOTE_JUMUN_DATA));	// 채권LP, 255;
				else if ( memcmp (&r_buf[ADD_HEADER_SIZE+11], "TCHKOR10001", 11) == 0 )
					memcpy (W_Fmt[0].Data, r_buf, ADD_HEADER_SIZE + 147);								// Kill Switch , KTS만 가능 (147);
				else
				{
					Log (USR_ERROR, "Other TR Recv [%s] size[%d]", r_buf, strlen(r_buf));
					break;
				}
			}
#elif	defined A5011 || A5012	// 파생 주문수신
			/*
				주문전문만 받음.
				YSK_DATA_HEAD (100) + 주문전문(294)
			*/
			if (memcmp (&r_buf[ADD_HEADER_SIZE+11], "TCHODR1000", 10) == 0 )
				memcpy (W_Fmt[0].Data, r_buf, ADD_HEADER_SIZE + sizeof (KRX_JUMUN_DATA));
			else
			{
				Log (USR_ERROR, "REJC Other TR Recv [%s] size[%d]", r_buf, strlen(r_buf));
				break;
			}

			if (w_flag)					// 거부(REJC)
			{
				YSK_DATA_HEAD	*H_Fmt = (YSK_DATA_HEAD *)W_Fmt[0].Data;
				memcpy (H_Fmt->sRpCode,			"REJC",					4);							// 거부코드 세팅
			}
#endif
//			W_Fmt[0].Data[ODS(D_K,P_K,0)] = '\n';		// ODS나 OFS 모두 사이즈가 동일하다.

			if (w_flag)					// 거부(REJC)
			{
				W_Fmt[0].Data[OFS(D_K,P_K,0)] = '\n';
				rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);
				if (rt < 0)
				{
					Log (USR_OK, "REJC Write Error W_Fmt[%s][%d]", W_Fmt[0].Data, strlen(W_Fmt[0].Data));
					Exit_Process ();
				}
				Log (USR_OK, "F_W OK [%s]", W_Fmt);
			}
			else						// 정상
			{
				W_Fmt[0].Data[ODS(D_K,P_K,0)] = '\n';
				if (Pk == 0)
					rt = DSHM_W (TS_W1_1, (void *)&W_Fmt, 1);
				else
					rt = DSHM_W (TS_W2_1, (void *)&W_Fmt, 1);
				if (rt < 0)
				{
					Log (USR_OK, "SHM Write Error W_Fmt[%s][%d]", W_Fmt[0].Data, strlen(W_Fmt[0].Data));
					Log (DSH_FATAL, "DSHM write[%s]", ODN(D_K,P_K,1));
					Exit_Process ();
				}
				Log (USR_OK, "DSHM_W[%d] OK [%s]", Pk + 1, W_Fmt);
			}

			Log (USR_OK, "Recv [%s] size[%d]", r_buf, strlen(r_buf));

			Set_TR_Time ();
			INT_SEQ ++;
		}
		else
		{
			Log (USR_ERROR, "Recv [%s] size[%d]", r_buf, strlen(r_buf));
			Exit_Process ();
		}
	}

	return;
}	/* End of PA_5000_QR ()	*/

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
	Pk = 0;
	Pk0 = Pk1 = 0;

	/* 주문Process01 process key 구함 */
    for (Pk0 = 0; Pk0 < DAEMON(D_K).p_count; Pk0 ++)
    {
        if (memcmp (PROC(D_K,Pk0).process_id, CHK_PROC, 10) == 0)
            break;

        if (Pk0 == DAEMON(D_K).p_count - 1)
        {
            Log (USR_FATAL, "unregistered process[%s]", CHK_PROC);
            Exit_Process ();
        }
    }

    for (Pk1 = 0; Pk1 < DAEMON(D_K).p_count; Pk1 ++)
    {
        if (memcmp (PROC(D_K,Pk1).process_id, CHK_PROC1, 10) == 0)
            break;

        if (Pk1 == DAEMON(D_K).p_count - 1)
        {
            Log (USR_FATAL, "unregistered process[%s]", CHK_PROC1);
            Exit_Process ();
        }
    }
}

/*************************************************************************
	End of program (pa_5000_qr.c)
*************************************************************************/ 
