#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: Q주문응답 송신 (UDP)
#	File	: pa_5200_qs.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include    "pa_struct.h"

#if	defined A5201 ||A5202 || A5401 ||A5402				// 채권 회원처리호가(5201), 체결(5401)
#define     DATA_SIZE   400
#elif	defined A5211 || A5212 || A5411 || A5412		// 파생 회원처리호가(5211), 체결(5411)
#define     DATA_SIZE   450
#endif
#include    "buf_struct.h"

#define     FIFO_EVENT      0
#define     SOCKET_EVENT    1
#define     FILE_EVENT      2

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define     DATA_TIME   60 * 1000
#define		READ_MAX	1

#define		READ_BUF_SIZE	1024
#define		QUEUE_MAX_BYTES	10485760

/* For Queue */
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

#define     WR_CNT          W_CNT(0,1)
#define     RD_CNT          R_CNT(0,1)
#define     IN_NAME         IFN(D_K,P_K,0)

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int					FIFO_fd, FIFO_fd1;
char				TrCode[8], ApType[10];
struct				ip_mreq		mreq;
struct sockaddr_in	SvrAddr, ClntAddr;


FILE_BUFF_FORMAT    R_Fmt[READ_MAX];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_5200_QS (void);
int		ReceiveQueue();
int		QueueClear(int);
int		GetQid(size_t);
int		CheckQueue(int);
int     MakeQueue(key_t);
int     ReceiveQueue(int, long, char *);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_5200_QS ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_5200_QS (void)
/*----------------------------------------------------------------------*/
{
	int		i, rt;
	int		msqid, R_Cnt;
	char	fifo_name[100];
	char	m_time[24], key[12];
	size_t	buf_length;
	key_t	new_key, base_key;

	Msgbuf	sbuf;

	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", 
							_Exe_Name, _Exe_Name+3, _Exe_Name+8);
	LtoU (ApType, strlen (ApType));


/*
	// FIFO_fd 설정 
	memset (fifo_name, 0, sizeof(fifo_name));
	sprintf (fifo_name, "%s/PA/%s%d", _FEP_FIFO, Ofn, ODN(D_K,P_K,0));
	FIFO_fd = open (fifo_name, O_RDWR|O_NDELAY);
	if (FIFO_fd < 0)
	{
		Log (FIF_FATAL, "cannot open FIFO[%s][%d] {%d:%s}", fifo_name, FIFO_fd, SYS_NO, SYS_STR);
		Exit_Process ();
	}

	// FIFO_fd1 설정 
	memset (fifo_name, 0, sizeof(fifo_name));
	sprintf (fifo_name, "%s/PA/%s%d", _FEP_FIFO, Ofn, ODN(D_K,P_K,1));
	FIFO_fd1 = open (fifo_name, O_RDWR|O_NDELAY);
	if (FIFO_fd1 < 0)
	{
		Log (FIF_FATAL, "cannot open FIFO1[%s][%d] {%d:%s}", fifo_name, FIFO_fd1, SYS_NO, SYS_STR);
		Exit_Process ();
	}
*/

	// Get msg q ID
#if		defined A5201	// 채권 응답송신
	new_key = 0x22000002;
#elif	defined A5211	// 파생 응답송신
	new_key = 0x22000012;
#elif	defined A5212	// 파생(야) 응답송신
	new_key = 0x22000022;
#elif	defined A5401	// 채권 체결송신
	new_key = 0x22000004;
#elif	defined A5411	// 파생 체결송신
	new_key = 0x22000014;
#elif	defined A5412	// 파생(야) 체결송신
	new_key = 0x22000024;
#endif
#if	0
	if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
		new_key += 0x01000000L;
#endif

    msqid = MakeQueue(new_key);
    if (msqid < 0)
    {
        Log(USR_ERROR, "msgget msqid[%d]\n", msqid);
		Exit_Process ();
    }
    else
        Log (USR_OK, "msgget: succeeded: msqid = %d new_key [%#x]\n", msqid, new_key);

	/*
	rt = QueueClear(msqid);
    if (rt <= 0)
    {
        Log (USR_OK, "msg q Clear Error = %d rt : [%d]\n", msqid, rt);
		Exit_Process ();
    }
    else
        Log (USR_OK, "msg q Clear OK = %d rt : [%d]\n", msqid, rt);
    */

	sbuf.mtype = 1;

	while (START_S != JOB_END)
	{
		Stat_Save ();

		while (START_S != JOB_END)
		{
//sleep (5);
			memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * READ_MAX);
			R_Cnt = F_R (PS_R_3, (void *)R_Fmt, READ_MAX);
			if (R_Cnt < 0)
			{
				Log (SAM_FATAL, "cannot read File[%s,%d:%s]",
					IFN(D_K,P_K,0), SYS_NO, SYS_STR);
				sleep (1);
				Exit_Process ();
			}
			else if (R_Cnt == 0)
				break;

			Log (USR_OK, "RD [%s:%d][%d]", IFN(D_K,P_K,0), R_Cnt, IFR(D_K,P_K,0,2));

			/* ************************************************************************ */
			/* 주문응답 : 11+4 + 주문전문(오류) or 회원처리호가(확인/거부/자동취소)		*/
			/* 체결     : 체결전문만													*/
			/* 접수거부(Client) : 15(REJC) + 주문전문									*/ 
			/* ************************************************************************ */
			/* Q SEnd */
			memset (sbuf.mtext, 0, sizeof (sbuf.mtext));
			memcpy (sbuf.mtext, R_Fmt[0].Data, strlen (R_Fmt[0].Data));

			buf_length = strlen(sbuf.mtext)+1;
			//rt = msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT);
			rt = msgsnd(msqid, &sbuf, buf_length, 0);
			if (rt < 0)
			{
				Log (USR_ERROR, "ERROR %d, %d, %s, %d", msqid, sbuf.mtype, sbuf.mtext, strlen(sbuf.mtext));
				Exit_Process ();
			}
			Log (USR_OK, "Q rt[%d] SD[%s][%d]", rt, R_Fmt[0].Data, strlen (R_Fmt[0].Data));
			Add_Count (PS_R_3, 1);
		}

		rt = Poll_File (DATA_TIME);

        if (rt == 1)
        {
            Log (USR_OK, "poll timeout <%d>", OFW(D_K,P_K,0,0));
        }
        else if (rt == -1)
            continue;
	}

	return;
}	/* End of PA_5200_QS ()	*/


/*************************************************************************
	End of program (pa_5200_qs.c)
*************************************************************************/ 
