#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: TCP/IP 업무접속 Service
#	File	: pw_2000_tr.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		TCP_TIME_OUT	10
#define		PORT_NO(pos)	TCP1_SPORT(D_K,P_K,pos)
#define		S_CT(pos)		TCP1_S_CT(D_K,P_K,pos)
#define		S_ST(pos)		TCP1_S_ST(D_K,P_K,pos)
#define		CLEAN_SHUT(pos)	S_ST(pos)=0,S_CT(pos)--;

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
u_short		ClntPort;
int			Pos, Sockfd, Newfd, Dk;
char		ProcName[20], ClntIp[20];
char		RecvPkt[TCP_MESSAGE_MAX_LEN], SendPkt[TCP_MESSAGE_MAX_LEN];
key_t		Base_Key = BASE_SHM_KEY;
TCP_HEAD	*S_Pkt = (TCP_HEAD *)SendPkt;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PW_2000_TR (void);
void	Init_Parameters (void);
void	Device_Open (void);
void	Child_Process (void);
int		Check_Header (void);
int		Send_Packet (void);
void 	Register_Parent_Signal (void);
void 	Catch_Parent_Signal (int);
void 	Register_Child_Signal (void);
void 	Catch_Child_Signal (int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PW_2000_TR ();

	TCP1_NET_STA = OFF;
	close (Sockfd);
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PW_2000_TR (void)
/*----------------------------------------------------------------------*/
{
	int		pid, rt;

	Init_Parameters ();
	Log (USR_OK, "PORT_NO=[%d]", PORT_NO(Pos));

	Register_Parent_Signal ();
	Device_Open ();

	TCP1_NET_STA = ON;

	while (1)
	{
		Log (USR_OK, "accepting ...");

		memset (ClntIp, 0, sizeof (ClntIp));

		Newfd = Accept (Sockfd, ClntIp, &ClntPort);

		if (Newfd < 0)
		{
			if (SYS_NO == ECONNABORTED || SYS_NO == EINTR)
			{
				usleep (100000);
				continue;
			}

			Log (TCP_ERROR, "accept[%d] {%d:%s}", Newfd, SYS_NO, SYS_STR);
			CLEAN_SHUT(Pos);
			return;
		}

		Log (USR_OK, "accept[%d,%d]", Sockfd, Newfd);

		pid = fork ();

		if (pid == 0)
		{
			Register_Child_Signal ();
			close (Sockfd);
			Child_Process ();
			close (Newfd);
			exit (OK);
		}

		if (pid < 0)
		{
			Log (SYS_ERROR, "fork failure {%d:%s}", SYS_NO, SYS_STR);
			CLEAN_SHUT(Pos);
			usleep (100000);
			return;
		}

		close (Newfd);
	}

	return;
}	/* End of PW_2000_TR ()	*/

/*************************************************************************
	Function		: . Init_Parameters
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . initialize parameters
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Init_Parameters (void)
/*----------------------------------------------------------------------*/
{
	SYS_NO = 0;
	signal (SIGCHLD, SIG_IGN);
	memset (ProcName, 0, sizeof (ProcName));

	if (TIME_OUT == 0)
		TIME_OUT = TCP_TIME_OUT;

	Pos = AtoIf (_Exe_Name+6, 1) - 1;
	S_ST(Pos) = ON;

	if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
		Base_Key += 0x01000000L;

	return;
}	/* End of Init_Parameters ()	*/

/*************************************************************************
	Function		: . Device_Open
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . open device (create socket, bind and listen)
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Device_Open (void)
/*----------------------------------------------------------------------*/
{
	int			rt, bind_cnt, sleep_secs;
	const int	on = 1;

	bind_cnt = 1;
	sleep_secs = 60;

	do {
		Sockfd = Socket ();

		if (Sockfd < 0)
		{
			Log (TCP_ERROR, "socket[%d] {%d:%s}", Sockfd, SYS_NO, SYS_STR);
			TCP1_NET_STA = OFF;
			CLEAN_SHUT(Pos);
			Exit_Process ();
		}

		rt = setsockopt (Sockfd, SOL_SOCKET, SO_REUSEADDR,
			(char *)&on, sizeof (on));

		if (rt < 0)
		{
			Log (TCP_ERROR, "setsockopt[%d] {%d:%s}", rt, SYS_NO, SYS_STR);
			TCP1_NET_STA = OFF;
			shutdown (Sockfd, SHUT_RDWR);
			CLEAN_SHUT(Pos);
			Exit_Process ();
		}

		if (bind_cnt % 2 == 0)
			sleep_secs -= 5;

		rt = Bind (Sockfd, PORT_NO(Pos));

		if (rt < 0)
		{
			Log (TCP_ERROR, "bind[%d][%d] {%d:%s}",
				rt, PORT_NO(Pos), SYS_NO, SYS_STR);
			close (Sockfd);
			sleep (sleep_secs);
			bind_cnt ++;
		}

		if (bind_cnt > 30)
		{
			close (Sockfd);
			TCP1_NET_STA = OFF;
			CLEAN_SHUT(Pos);
			Exit_Process ();
		}
	} while (rt < 0);

	Log (USR_OK, "bind[%d]", Sockfd);

	Listen (Sockfd);
	Log (USR_OK, "listen[%d]", Sockfd);

	return;
}	/* Device_Open ()	*/

/*************************************************************************
	Function		: . Child_Process
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . child process routine
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Child_Process (void)
/*----------------------------------------------------------------------*/
{
	int		rt;
	char	path[256], name[20], sfd[8], port[4];

	rt = Select_Receive (Newfd, RecvPkt);

	if (rt <= 0)
	{
		S_CT(Pos) --;
		return;
	}

	Log (USR_OK, "TCP RD [%s](%d)", RecvPkt, strlen (RecvPkt));

	rt = Check_Header ();

	SHM_Detach ((char *)SHM_All_Daemon_Info);
	SHM_Detach ((char *)SHM_Mem[Dk]);

	if (rt == NOTOK)
	{
		sleep (5);

		rt = Send_Packet ();

		if (rt == OK)
			Log (USR_OK, "TCP SD [%s](%d)", SendPkt, strlen (SendPkt));

		S_CT(Pos) --;
		return;
	}

	sprintf (path, "%s/%s", _FEP_BIN, ProcName);
	sprintf (name, "%s", ProcName);
	sprintf (sfd, "%d", Newfd);
	sprintf (port, "%c.%d", _Exe_Name[3], Pos + 1);

	rt = execl (path, name, sfd, port, (char *)NULL);

	if (rt < 0)
	{
		Log (SYS_ERROR, "execl[%s,%s,%s] {%d:%s}",
			name, sfd, port, SYS_NO, SYS_STR);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T3, strlen (ERR_HEAD_T3));

		rt = Send_Packet ();

		if (rt == OK)
			Log (USR_OK, "TCP SD [%s](%d)", SendPkt, strlen (SendPkt));

		S_CT(Pos) --;
		return;
	}

	return;
}	/* End of Child_Process ()	*/

/*************************************************************************
	Function		: . Check_Header
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure)
	Comment			: . check validity of the received header
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Check_Header (void)
/*----------------------------------------------------------------------*/
{
	int			rt, val, pk, tmp_ss, d_s_ss, d_e_ss, p_s_ss, p_e_ss;
	key_t		shm_key;
	pid_t		p_id;
	char		aptype[10], key[12];
	time_t		sys_time;
	struct tm	*date;

	memset (SendPkt, 0, sizeof (SendPkt));
	memcpy (SendPkt, RecvPkt, TCP_HEAD_LEN);

	memset (aptype, 0, sizeof (aptype));
	memcpy (aptype, S_Pkt->ApType, sizeof (S_Pkt->ApType));
	UtoL (aptype, strlen (aptype));

	memset (ProcName, 0, sizeof (ProcName));
	sprintf (ProcName, "%-2.2s_%-4.4s_%-2.2s", aptype, aptype+2, aptype+6);

	Dk = S_Pkt->ApType[1] - 'A';

	rt = -1;
	sprintf (key, "0x00%02d0000", Dk + 1);
	shm_key = Base_Key + strtol (key, NULL, 16);

	rt = SHM_Creat (shm_key, 0);

	if (rt == -1)
	{
		Log (USR_ERROR, "%2.2s 부문 미사용 [%-8.8s]",
			S_Pkt->ApType, S_Pkt->ApType);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T3, strlen (ERR_HEAD_T3));
		return (NOTOK);
	}

	Sub_SHM ();
	Mem_SHM (1, Dk);

	if (DAEMON(Dk).process_status == 0)
	{
		Log (USR_ERROR, "%2.2s 부문 미사용 [%-8.8s:%d]", S_Pkt->ApType,
			S_Pkt->ApType, DAEMON(Dk).process_status);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T3, strlen (ERR_HEAD_T3));
		return (NOTOK);
	}
	else if (DAEMON(Dk).process_status == 2)
	{
		Log (USR_ERROR, "%2.2s 부문 업무종료 상태 [%-8.8s:%d]", S_Pkt->ApType,
			S_Pkt->ApType, DAEMON(Dk).process_status);
		memcpy (S_Pkt->ResponseCode, ERR_TIME_END, strlen (ERR_TIME_END));
		return (NOTOK);
	}
	else if (DAEMON(Dk).process_status == 3)
	{
		Log (USR_WARN, "%2.2s 부문 업무중지 상태 [%-8.8s:%d]", S_Pkt->ApType,
			S_Pkt->ApType, DAEMON(Dk).process_status);
		memcpy (S_Pkt->ResponseCode, ERR_TIME_STOP, strlen (ERR_TIME_STOP));
		return (NOTOK);
	}

	for (pk = 0; pk < DAEMON(Dk).p_count; pk ++)
	{
		if (memcmp (PROC(Dk,pk).process_id, ProcName, 10) == 0)
			break;

		if (pk == DAEMON(Dk).p_count - 1)
		{
			Log (USR_ERROR, "unregistered process [%s]", ProcName);
			memcpy (S_Pkt->ResponseCode, ERR_HEAD_T3, strlen (ERR_HEAD_T3));
			return (NOTOK);
		}
	}

	p_id = Check_Proc (ProcName);
	if (p_id != 0)
	{
		Log (USR_ERROR, "%s already running", ProcName);
		memcpy (S_Pkt->ResponseCode, ERR_PROC_RUN, strlen (ERR_PROC_RUN));
		return (NOTOK);
	}

	/* check the process operating time	*/
	time (&sys_time);
	date = localtime (&sys_time);
	tmp_ss = (date->tm_hour * 60 * 60) + (date->tm_min * 60) + date->tm_sec;

	p_s_ss = AtoIf (PROC(Dk,pk).start_time, 2) * 60 * 60 +
		AtoIf (PROC(Dk,pk).start_time+2, 2) * 60;
	p_e_ss = AtoIf (PROC(Dk,pk).end_time, 2) * 60 * 60 +
		AtoIf (PROC(Dk,pk).end_time+2, 2) * 60;

	d_s_ss = AtoIf (DAEMON(Dk).start_time, 2) * 60 * 60 +
		AtoIf (DAEMON(Dk).start_time+2, 2) * 60;
	d_e_ss = AtoIf (DAEMON(Dk).end_time, 2) * 60 * 60 +
		AtoIf (DAEMON(Dk).end_time+2, 2) * 60;

	if ((p_s_ss > d_s_ss && (tmp_ss >= d_s_ss && tmp_ss < p_s_ss)) ||
		(p_s_ss < d_s_ss && ((tmp_ss >= d_s_ss && tmp_ss < 24 * 60 * 60) ||
		(tmp_ss >= 0 && tmp_ss < p_s_ss))))
	{
		Log (USR_WARN, "process 기동시각 전 [%-8.8s:%.4s]",
			S_Pkt->ApType, PROC(Dk,pk).start_time);
		memcpy (S_Pkt->ResponseCode, ERR_TIME_BEFORE, strlen (ERR_TIME_BEFORE));
		return (NOTOK);
	}

	if ((p_e_ss > d_e_ss && ((tmp_ss > p_e_ss && tmp_ss < 24 * 60 * 60) ||
		(tmp_ss >= 0 && tmp_ss < d_e_ss))) ||
		(p_e_ss < d_e_ss && (tmp_ss > p_e_ss && tmp_ss <= d_e_ss)))
	{
		Log (USR_WARN, "process 종료시각 후 [%-8.8s:%.4s]",
			S_Pkt->ApType, PROC(Dk,pk).end_time);
		memcpy (S_Pkt->ResponseCode, ERR_TIME_END, strlen (ERR_TIME_END));
		return (NOTOK);
	}

	if (PROC(Dk,pk).type != TY_TRS1)
	{
		Log (USR_ERROR, "invalid process requested [%s]", ProcName);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T3, strlen (ERR_HEAD_T3));
		return (NOTOK);
	}

	/* Stx (0x02)	*/
	if (S_Pkt->Stx[0] != STX)
	{
		Log (USR_ERROR, "header (Stx)[%#04x]", S_Pkt->Stx[0]);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T1, strlen (ERR_HEAD_T1));
		return (NOTOK);
	}

	/* Length (packet 총 길이)	*/
	val = AtoIf (S_Pkt->Length, sizeof (S_Pkt->Length));
	if (val != strlen (RecvPkt) || strlen (RecvPkt) < TCP_HEAD_LEN)
	{
		Log (USR_ERROR, "header (Length)[%d:%d]", val, strlen (RecvPkt));
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T2, strlen (ERR_HEAD_T2));
		return (NOTOK);
	}

	/* ResponseCode (응답코드)	*/
	if (AtoIf (S_Pkt->ResponseCode, sizeof (S_Pkt->ResponseCode)) != 0)
	{
		Log (USR_ERROR, "header (ResponseCode)[%-4.4s:%d]",
			S_Pkt->ResponseCode, 0);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T6, strlen (ERR_HEAD_T6));
		return (NOTOK);
	}

	/* MsgType (운영코드)	*/
	if (memcmp (S_Pkt->MsgType, TCP_STRT_CD, sizeof (S_Pkt->MsgType)) != 0)
	{
		Log (USR_ERROR, "header (MsgType)[%-4.4s:%s]",
			S_Pkt->MsgType, TCP_STRT_CD);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T7, strlen (ERR_HEAD_T7));
		return (NOTOK);
	}

	memcpy (PROC(Dk,pk).ip, ClntIp, strlen (ClntIp));
	PROC(Dk,pk).port = ClntPort;

	return (OK);
}	/* End of Check_Header ()	*/

/*************************************************************************
	Function		: . Send_Packet
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure)
	Comment			: . send packet
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Send_Packet (void)
/*----------------------------------------------------------------------*/
{
	int		rt;
	char	t_time[12];

	S_Pkt->Stx[0] = STX;
	ItoAf (TCP_HEAD_LEN, S_Pkt->Length, sizeof (S_Pkt->Length));
	memset (t_time, 0, sizeof (t_time));
	Get_Time (t_time);
	memcpy (S_Pkt->Time, t_time, sizeof (S_Pkt->Time));
	memcpy (S_Pkt->MsgType, TCP_STOK_CD, sizeof (S_Pkt->MsgType));
	ItoAf (0, S_Pkt->DataCnt, sizeof (S_Pkt->DataCnt));

	rt = Select_Send (Newfd, SendPkt, TCP_HEAD_LEN);

	return (rt);
}	/* End of Send_Packet ()	*/

/*************************************************************************
	Function		: . Register_Parent_Signal
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . register parent signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Register_Parent_Signal (void)
/*----------------------------------------------------------------------*/
{
	struct sigaction	act;

	sigemptyset (&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = Catch_Parent_Signal;

	if (sigaction (SIGPIPE, &act, NULL) < 0)
	{
		Log (SYS_ERROR, "sigaction (SIGPIPE) {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	if (sigaction (SIGTERM, &act, NULL) < 0)
	{
		Log (SYS_ERROR, "sigaction (SIGTERM) {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	return;
}	/* End of Register_Parent_Signal ()	*/

/*************************************************************************
	Function		: . Catch_Parent_Signal
	Parameters IN	: . signo : signal number
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . catch parent signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Catch_Parent_Signal (int signo)
/*----------------------------------------------------------------------*/
{
	Log (PRO_WARN, "signal (%d) occurred to parent[%5d]", signo, getpid ());
	S_ST(Pos) = OFF;
	TCP1_NET_STA = OFF;
	close (Sockfd);
	close (Newfd);
	Exit_Process ();
}	/* End of Catch_Parent_Signal ()	*/

/*************************************************************************
	Function		: . Register_Child_Signal
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . register child signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Register_Child_Signal (void)
/*----------------------------------------------------------------------*/
{
	struct sigaction	act;

	sigemptyset (&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = Catch_Child_Signal;

	if (sigaction (SIGPIPE, &act, NULL) < 0)
	{
		Log (SYS_ERROR, "sigaction (SIGPIPE) {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	if (sigaction (SIGTERM, &act, NULL) < 0)
	{
		Log (SYS_ERROR, "sigaction (SIGTERM) {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	return;
}	/* End of Register_Child_Signal ()	*/

/*************************************************************************
	Function		: . Catch_Child_Signal
	Parameters IN	: . signo : signal number
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . catch child signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Catch_Child_Signal (int signo)
/*----------------------------------------------------------------------*/
{
	Log (PRO_WARN, "signal (%d) occurred to child[%5d]", signo, getpid ());
	close (Newfd);
	exit (FAIL);
}	/* End of Catch_Child_Signal ()	*/

/*************************************************************************
	End of Program (pw_2000_tr.c)
*************************************************************************/
