#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 해외 시세수신 (UDP)
#	File	: pa_6100_ur.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include    "pa_struct.h"
#include	"fep_fepp.h"

/* Master */
#if defined A6101||A6301||A6401||A6501
#define     DATA_SIZE       400
/* Sise */
#elif defined A6102||A6103||A6104||A6105||A6106||A6107 || A6202||A6302||A6402||A6502
#define     DATA_SIZE       450
#endif

#include    "buf_struct.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		SVR_PORT_NO		UDP_PORT(D_K,P_K,0)
#define		READ_BUF_SIZE	2048

KS_LONGCODE  Key;

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int		Sockfd, FIFO_fd, tot_cq;
int		Idx, W_gbn, Risk_Chk, Che_Gbn;
struct	ip_mreq     mreq;
char	TrCode[8], ApType[10], Order_St[512];
struct sockaddr_in	SvrAddr, ClntAddr;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_6100_UR (void);
int     Init_Parameters (void);
int		Socket_Connect (void);
int		Recv_Data (char *);
int		Add_Count_UR (void);
int		Add_Count_DD (char *);
void	Set_Sise (char *);
int     Write_Read_Fifo (int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_6100_UR ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_6100_UR (void)
/*----------------------------------------------------------------------*/
{
	char	m_time[24];
	char	Wdt[30], W2_Fmt[512];
	char 	r_buf[READ_BUF_SIZE];
	int		tr_gbn, rt, len, skip_rt;

	BUFF_RW_HEAD		f_head;
	FILE_BUFF_FORMAT    W_Fmt;

	SLog (USR_OK, "OK01");
	rt = Init_Parameters ( );
	if (rt == NOTOK)
		return;

#ifdef  HOLIDAY_CHECK
    while (1)
    {
        char     t_time[12], dt[20];
        time_t   t = time(NULL);
        struct  tm tm, *tp;

        Get_Time    (t_time);

        memset  (dt, 0, sizeof (dt));
        sprintf (dt, "%.4s-%.2s-%.2s %.2s:%.2s:%.2s", DAEMON(D_K).date,
            DAEMON(D_K).date+4, DAEMON(D_K).date+6, t_time, t_time+2, t_time+4);
        strptime    (dt, "%Y-%m-%d %H:%M:%s", &tm);
        //t   = mktime (&tm);
        tp  = localtime (&t);

        if  (tp->tm_wday == 0 || tp->tm_wday == 6) /* sun, sat */
        {
            SLog (USR_OK, "it's weekend. sleeping...[%d]", tp->tm_wday);
            sleep (60);
            continue;
        }
        else
            break;
    }
#endif

	rt = Socket_Connect ();
	if (rt == NOTOK)
		return;

	SLog (USR_OK, "socket connected:port[%d]", SVR_PORT_NO);

	while (START_S != JOB_END)
	{
		Stat_Save ();
		memset (r_buf, 0, sizeof (r_buf));

		rt = Recv_Data (r_buf);

		if (rt == 0)
		{
			SLog (USR_OK, "poll timeout");
			continue;
		}
		else if (rt < 0)
		{
			SLog (UDP_ERROR, "receive fail {%d:%s}", SYS_NO, SYS_STR);
			close (Sockfd);

			rt = Socket_Connect ();

			if (rt == NOTOK)
				return;

			continue;
		}

		memset (TrCode, 0, sizeof (TrCode));			/* TRCODE 저장	*/
		sprintf (TrCode, "%-5.5s", r_buf);

		Che_Gbn = W_gbn = Risk_Chk = Idx = 0;
		Set_Sise (r_buf);

		/* 체결관련시 평가손익 && 자동전략 처리위한 전달 */
        if (Che_Gbn > 0 && W_gbn == 1)
        {
#if defined A6102||A6103||A6104||A6105||A6106||A6107 || A6202
            if (Shm_CME[Idx].auto_use > 0)            /* 기동중인 자동이 해당종목을 설정했을때 전달 */
                Write_Read_Fifo (1);
#elif defined A6302
            if (Shm_SGX[Idx].auto_use > 0)            /* 기동중인 자동이 해당종목을 설정했을때 전달 */
                Write_Read_Fifo (1);
#elif defined A6402
            if (Shm_ERX[Idx].auto_use > 0)            /* 기동중인 자동이 해당종목을 설정했을때 전달 */
                Write_Read_Fifo (1);
#elif defined A6502
            if (Shm_HKE[Idx].auto_use > 0)            /* 기동중인 자동이 해당종목을 설정했을때 전달 */
                Write_Read_Fifo (1);
#endif
		}
		
		/* 체결관련시 평가손익 && 자동전략 처리위한 전달 */
		if (W_gbn <= 0)
			continue;

/* 지금은 필요 없을 듯
	    len = Add_Count_UR ();
*/

		/* 운영장비에서는 File Write를 하지않고 개발기나 테스트장비에서 File Write한다.		*/
		/* 해당 파일은 비상시 또는 장종류후 전략시뮬레이션을 위한 통자료로만 사용을 한다.	*/
		/* File Write */
		memset (&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));

		/* write to DD file	*/
		ItoAf (OFW_CNT(0,0) + 1, W_Fmt.If_Seq, sizeof (W_Fmt.If_Seq));
		memcpy (W_Fmt.ApType, ApType, sizeof (W_Fmt.ApType));
		memcpy (W_Fmt.ResponseCode, RES_NORMAL, strlen (RES_NORMAL));
		memcpy (W_Fmt.RecvTime1, m_time, sizeof (W_Fmt.RecvTime1));
		memcpy (W_Fmt.RecvTime2, &m_time[sizeof(W_Fmt.RecvTime1)],
										sizeof (W_Fmt.RecvTime2));

		if (W_gbn == 1)
		{
			memcpy (W_Fmt.Data, r_buf, strlen(r_buf));
			W_Fmt.LineFeed[0] = '\n';
			rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);
		}
		else if (W_gbn == 2)
		{
			memset (W_Fmt.Data,	0x20,	OFS(D_K,P_K,1));
			memcpy (W_Fmt.Data, r_buf, strlen(r_buf));
			W_Fmt.Data[OFS(D_K,P_K,1)] = '\n';
			rt = F_W (TS_W2_1, (void *)&W_Fmt, 1);
		}

		if (rt != 1)
		{
			SLog (SAM_FATAL, "file write fail[%s] rt[%d]", OFN(D_K,P_K,W_gbn-1), rt);
			close (Sockfd);
			return;
		}

		Add_Count_DD (TrCode);

//		Set_TR_Time ();
//		INT_SEQ ++;
	}

	return;
}	/* End of PA_6100_UR ()	*/

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
	int         i, j, rt, flag;
	char        item_code[20], memberitem[10], tmp[128];
	char        d_time[16], head_size[10], cli_orsnd[21];
	char		fifo_name[100], bumun[4], m_time[24];

	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
	LtoU (ApType, strlen (ApType));

/* ************************************************************************ */
/* 전략기동중인 시장+종목을 수신했을때 전략에 전달용						*/
/* ************************************************************************ */
#if defined A6102||A6103||A6104||A6105||A6106||A6107 || A6202
    sprintf (fifo_name, "%s/PA/pa_6102_ur1", _FEP_FIFO);
#elif defined A6302
    sprintf (fifo_name, "%s/PA/pa_6302_ur1", _FEP_FIFO);
#elif defined A6402
    sprintf (fifo_name, "%s/PA/pa_6402_ur1", _FEP_FIFO);
#elif defined A6502
    sprintf (fifo_name, "%s/PA/pa_6502_ur1", _FEP_FIFO);
#endif

#if defined A6102||A6103||A6104||A6105||A6106||A6107 || A6202 ||A6302||A6402||A6502
    /* 전략전달용 FIFO */
    FIFO_fd = open (fifo_name, O_RDWR|O_NDELAY);
    if (FIFO_fd < 0)
        SLog (FIF_FATAL, "cannot open FIFO[%s][%d][%d:%s]",
            fifo_name, FIFO_fd, SYS_NO, SYS_STR);
#endif

	return (OK);
}   /* End of Init_Parameters ()    */

/*************************************************************************
	Function		: . Socket_Connect
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure)
	Comment			: . connect to a socket
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Socket_Connect (void)
/*----------------------------------------------------------------------*/
{
	int 	rt, val, len;

	char	ip_addr[16];

	sprintf(ip_addr, "%d.%d.%d.%d", UDP_IP1(D_K,P_K,0), UDP_IP2(D_K,P_K,0),
									UDP_IP3(D_K,P_K,0), UDP_IP4(D_K,P_K,0));

	SLog(USR_OK, "Ip_Addr -> [%s]", ip_addr);

	bzero ((unsigned char *)&SvrAddr, sizeof (SvrAddr));
	SvrAddr.sin_family         = AF_INET;
    SvrAddr.sin_addr.s_addr    = inet_addr (ip_addr);
/* 202201
    inet_pton(AF_INET, ip_addr, &SvrAddr.sin_addr.s_addr);
*/
	SvrAddr.sin_port           = htons (SVR_PORT_NO);

	bzero ((unsigned char *)&mreq, sizeof (mreq));
	mreq.imr_multiaddr          = SvrAddr.sin_addr;
	mreq.imr_interface.s_addr   = htonl(INADDR_ANY);

	Sockfd = socket (AF_INET, SOCK_DGRAM, 0);

	if (Sockfd < 0)
	{
		SLog (UDP_FATAL, "socket open fail {%d:%s}", SYS_NO, SYS_STR);
		return (NOTOK);
	}

	/* 2011.03 시세수신 증속에 따른 버퍼량 증가 */
	/* val = 262144; 256K, 524288(512K)	*/
	val = 524288;												/* 512K	*/
#if 0
	val = 1228800;												/* 1228800K	*/
#endif
	len = sizeof (val);

	//rt = setsockopt (Sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&val, len);
	rt = setsockopt (Sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, len);
	if (rt == -1)
	{
		SLog (UDP_WARN, "setsockopt (SO_RCVBUF) fail {%d:%s}", SYS_NO, SYS_STR);
		return (NOTOK);
	}

	if (bind (Sockfd, (struct sockaddr *)&SvrAddr, sizeof (SvrAddr)) < 0)
	{
		SLog (UDP_FATAL, "bind fail {%d:%s}", SYS_NO, SYS_STR);
		return (NOTOK);
	}

	rt = setsockopt(Sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
	if (rt == -1)
	{
		SLog (UDP_WARN, "setsockopt (MULTICAST[%d]) fail {%d:%s}", mreq, SYS_NO, SYS_STR);
		return (NOTOK);
	}

	return (OK);
}	/* End of Socket_Connect ()	*/

/*************************************************************************
	Function		: . Recv_Data
	Parameters IN	: .
	Parameters OUT	: . p_str	: receive buffer
	Return Code		: . int (0:timeout, -1:failure, >0:number of bytes received)
	Comment			: . receive data
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Recv_Data (char *p_str)
/*----------------------------------------------------------------------*/
{
	int				rt, len;
#if defined __linux
	fd_set	read_set;
#else
	struct fd_set	read_set;
#endif
	struct timeval	timeout;

	len = sizeof (ClntAddr);

	FD_ZERO (&read_set);
	FD_SET (Sockfd, &read_set);
	timeout.tv_sec = 70;
	timeout.tv_usec = 0;

	rt = select (Sockfd+1, &read_set, NULL, NULL, &timeout);

	if (rt < 0)
	{
	SLog (SYS_FATAL, "select fail {%d:%s}", SYS_NO, SYS_STR);
		return (NOTOK);
	}

	if (FD_ISSET (Sockfd, &read_set))
	{
#if defined __linux
		rt = (int)recvfrom (Sockfd, p_str, READ_BUF_SIZE, 0,
			(struct sockaddr *)&ClntAddr, (socklen_t *)&len);
#else
		rt = (int)recvfrom (Sockfd, p_str, READ_BUF_SIZE, NULL,
			(struct sockaddr *)&ClntAddr, (socklen_t *)&len);
#endif
		return (rt);
	}

	return (OK);
}	/* End of Recv_Data ()	*/

/*************************************************************************
	Function		: . Add_Count_UR
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (-1:failure, 0:not used, >0:data length)
	Comment			: . add receive count
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Add_Count_UR (void)
/*----------------------------------------------------------------------*/
{
	int         rt;
	register    i;

	rt = NOTOK;
	T_K = NOTOK;

	for (i = 0; i < DAEMON(D_K).sisetr_count; i ++)
	{
		if (SISETR(D_K,i).tr[0] == '\0')
			break;

		if (memcmp (SISETR(D_K,i).tr, TrCode, 5) == 0)
		{
			if (SISETR(D_K,i).queue == 0)					/* not used	*/
			{
				rt = 0;
				break;
			}
			else
			{
				SISETR(D_K,i).ur_count ++;

				T_K = i;
				rt = SISETR(D_K,T_K).length;
				break;
			}
		}
	}

	return (rt);
}	/* End of Add_Count_UR ()	*/

/*************************************************************************
	Function		: . Add_Count_DD
	Parameters IN	: . tr	: TR code
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . add divide count
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Add_Count_DD (char *p_tr)
/*----------------------------------------------------------------------*/
{
    int         rt = NOTOK;
    register    i;

    for (i = 0; i < DAEMON(D_K).sisetr_count; i ++)
    {
        if (SISETR(D_K,i).tr[0] == '\0')
            break;

        if (memcmp (SISETR(D_K,i).tr, p_tr, strlen (p_tr)) == 0)
        {
            SISETR(D_K,i).dd_count ++;

            rt = SISETR(D_K,i).length;
            break;
        }
    }

    return (rt);
}	/* End of Add_Count_DD ()	*/

/*************************************************************************
	Function		: . Set_Sise
	Parameters IN	: . p_buf	: received data
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . set sise data SHM (기본/호가/체결)
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Set_Sise (char *p_buf)
/*----------------------------------------------------------------------*/
{
	int			s_k, rt;

/* *********************************** */
/* 시장별, TR별 종목 일련번호 갖고오기 */
/* *********************************** */
#if defined A6101
	if ((memcmp (p_buf, "M", 1) == 0)	&&
		(memcmp (&p_buf[20], "FCME", 4) == 0))
	{
		W_gbn = 1;
	}
	else
		Log (USR_OK, "Other Tr[%s]", p_buf);
#elif defined A6301
	if ((memcmp (p_buf, "M", 1) == 0)	&&
		(memcmp (&p_buf[20], "FSGX", 4) == 0))
	{
		W_gbn = 1;
	}
	else
		Log (USR_OK, "Other Tr[%s]", p_buf);
#elif defined A6401
	if ((memcmp (p_buf, "M", 1) == 0)	&&
		(memcmp (&p_buf[20], "FERX", 4) == 0))		// 20211202 EUX=>ERX
	{
		W_gbn = 1;
	}
	else
		Log (USR_OK, "Other Tr[%s]", p_buf);
#elif defined A6501
	if ((memcmp (p_buf, "M", 1) == 0)	&&
		(memcmp (&p_buf[20], "FHKE", 4) == 0))		// 20211202 HKG=>HKE
	{
		W_gbn = 1;
	}
	else
		Log (USR_OK, "Other Tr[%s]", p_buf);
#elif defined A6102||A6103||A6104||A6105||A6106||A6107 || A6202
	if ((memcmp (p_buf, "Q", 1) == 0)	||
		(memcmp (p_buf, "B", 1) == 0)	||
		(memcmp (p_buf, "S", 1) == 0))
	{
		/* 전문의 송신일시로 체크, 8시이전의 값은 Skip(마스터이후만 유효) */
		if ((memcmp (p_buf, "Q", 1) == 0 && memcmp (&p_buf[197+8], "0800", 4) >= 0)	||
			(memcmp (p_buf, "B", 1) == 0 && memcmp (&p_buf[70+8], "0800", 4) >= 0)	||
			(memcmp (p_buf, "S", 1) == 0 && memcmp (&p_buf[76+8], "0800", 4) >= 0))
		{
			/* index search */
			memset (&Key, 0, sizeof (Key));
			sprintf (Key.longcode, "%-20.20s", &p_buf[28]);

			rt = 0;
			rt = Key_Search (MK_CME, KEY_LONGCODE, (char*)&Key);
			if (rt >= 0)
			{
				Idx = rt;
				W_gbn = 1;
				if (memcmp (p_buf, "Q", 1) == 0)
				{
					Che_Gbn = W_gbn = 1;
					memcpy (Shm_CME[Idx].Q.type, p_buf, sizeof (OC_Q));
				}
				else if (memcmp (p_buf, "B", 1) == 0)
				{
					W_gbn = 1;
					memcpy (Shm_CME[Idx].B.type, p_buf, sizeof (OC_B));
				}
				else if (memcmp (p_buf, "S", 1) == 0)
				{
					W_gbn = 2;
				}
			}
			else
				Log (USR_ERROR, "%1.1s %4.4s Key Not Found [%20.20s]", p_buf, &p_buf[20], &p_buf[28]);
		}
		else
		{
			if (memcmp (p_buf, "S", 1) == 0 && memcmp (&p_buf[76+8], "0800", 4) < 0)
				W_gbn = 2;
		}
	}
	else
		Log (USR_OK, "Other Tr[%s]", p_buf);
#elif defined A6302
	if ((memcmp (p_buf, "Q", 1) == 0)	||
		(memcmp (p_buf, "B", 1) == 0)	||
		(memcmp (p_buf, "S", 1) == 0))
	{
		/* 전문의 송신일시로 체크, 8시이전의 값은 Skip(마스터이후만 유효) */
		if ((memcmp (p_buf, "Q", 1) == 0 && memcmp (&p_buf[197+8], "0800", 4) >= 0)	||
			(memcmp (p_buf, "B", 1) == 0 && memcmp (&p_buf[70+8], "0800", 4) >= 0)	||
			(memcmp (p_buf, "S", 1) == 0 && memcmp (&p_buf[76+8], "0800", 4) >= 0))
		{
			/* index search */
			memset (&Key, 0, sizeof (Key));
			sprintf (Key.longcode, "%-20.20s", &p_buf[28]);

			rt = 0;
			rt = Key_Search (MK_SGX, KEY_LONGCODE, (char*)&Key);
			if (rt >= 0)
			{
				Idx = rt;
				W_gbn = 1;
				if (memcmp (p_buf, "Q", 1) == 0)
				{
					Che_Gbn = W_gbn = 1;
					memcpy (Shm_SGX[Idx].Q.type, p_buf, sizeof (OC_Q));
				}
				else if (memcmp (p_buf, "B", 1) == 0)
				{
					W_gbn = 1;
					memcpy (Shm_SGX[Idx].B.type, p_buf, sizeof (OC_B));
				}
				else if (memcmp (p_buf, "S", 1) == 0)
				{
					W_gbn = 2;
				}
			}
			else
				Log (USR_ERROR, "%1.1s %4.4s Key Not Found [%20.20s]", p_buf, &p_buf[20], &p_buf[28]);
		}
		else
		{
			if (memcmp (p_buf, "S", 1) == 0 && memcmp (&p_buf[76+8], "0800", 4) < 0)
				W_gbn = 2;
		}
	}
#elif defined A6402
	if ((memcmp (p_buf, "Q", 1) == 0)	||
		(memcmp (p_buf, "B", 1) == 0)	||
		(memcmp (p_buf, "S", 1) == 0))
	{
		/* 전문의 송신일시로 체크, 8시이전의 값은 Skip(마스터이후만 유효) */
		if ((memcmp (p_buf, "Q", 1) == 0 && memcmp (&p_buf[197+8], "0800", 4) >= 0)	||
			(memcmp (p_buf, "B", 1) == 0 && memcmp (&p_buf[70+8], "0800", 4) >= 0)	||
			(memcmp (p_buf, "S", 1) == 0 && memcmp (&p_buf[76+8], "0800", 4) >= 0))
		{

			/* index search */
			memset (&Key, 0, sizeof (Key));
			sprintf (Key.longcode, "%-20.20s", &p_buf[28]);

			rt = 0;
			rt = Key_Search (MK_ERX, KEY_LONGCODE, (char*)&Key);
			if (rt >= 0)
			{
				Idx = rt;
				if (memcmp (p_buf, "Q", 1) == 0)
				{
					Che_Gbn = W_gbn = 1;
					memcpy (Shm_ERX[Idx].Q.type, p_buf, sizeof (OC_Q));
				}
				else if (memcmp (p_buf, "B", 1) == 0)
				{
					W_gbn = 1;
					memcpy (Shm_ERX[Idx].B.type, p_buf, sizeof (OC_B));
				}
				else if (memcmp (p_buf, "S", 1) == 0)
				{
					W_gbn = 2;
				}
			}
			else
				Log (USR_ERROR, "%1.1s %4.4s Key Not Found [%20.20s]", p_buf, &p_buf[20], &p_buf[28]);
		}
		else
		{
			if (memcmp (p_buf, "S", 1) == 0 && memcmp (&p_buf[76+8], "0800", 4) < 0)
				W_gbn = 2;
		}
	}
#elif defined A6502
	if ((memcmp (p_buf, "Q", 1) == 0)	||
		(memcmp (p_buf, "B", 1) == 0)	||
		(memcmp (p_buf, "S", 1) == 0))
	{
		/* 전문의 송신일시로 체크, 8시이전의 값은 Skip(마스터이후만 유효) */
		if ((memcmp (p_buf, "Q", 1) == 0 && memcmp (&p_buf[197+8], "0800", 4) >= 0)	||
			(memcmp (p_buf, "B", 1) == 0 && memcmp (&p_buf[70+8], "0800", 4) >= 0)	||
			(memcmp (p_buf, "S", 1) == 0 && memcmp (&p_buf[76+8], "0800", 4) >= 0))
		{
			/* index search */
			memset (&Key, 0, sizeof (Key));
			sprintf (Key.longcode, "%-20.20s", &p_buf[28]);

			rt = 0;
			rt = Key_Search (MK_HKE, KEY_LONGCODE, (char*)&Key);
			if (rt >= 0)
			{
				Idx = rt;
				if (memcmp (p_buf, "Q", 1) == 0)
				{
					Che_Gbn = W_gbn = 1;
					memcpy (Shm_HKE[Idx].Q.type, p_buf, sizeof (OC_Q));
				}
				else if (memcmp (p_buf, "B", 1) == 0)
				{
					W_gbn = 1;
					memcpy (Shm_HKE[Idx].B.type, p_buf, sizeof (OC_B));
				}
				else if (memcmp (p_buf, "S", 1) == 0)
				{
					W_gbn = 2;
				}
			}
			else
				Log (USR_ERROR, "%1.1s %4.4s Key Not Found [%20.20s]", p_buf, &p_buf[20], &p_buf[28]);
		}
		else
		{
			if (memcmp (p_buf, "S", 1) == 0 && memcmp (&p_buf[76+8], "0800", 4) < 0)
				W_gbn = 2;
		}
	}
#endif

	return;
}	/* End of Set_Sise ()	*/

/*************************************************************************
    Function        : . Write_Read_Fifo
    Parameters IN   : . 
    Parameters OUT  : .
    Comment         : . write to FIFO
*************************************************************************/
int     Write_Read_Fifo (int c0h0)
{
    int     rt;
    char    tmp[128];

    rt = write (FIFO_fd, "1", 1);

    if (rt < 0)
    SLog (FIF_FATAL, "cannot write FIFO[%d][%d:%s]",
        FIFO_fd, SYS_NO, SYS_STR);

    while (1)
    {
        rt = read (FIFO_fd, tmp, sizeof(tmp));
#if defined __linux
        if (rt == 0 || errno == EAGAIN)
#else
        if (rt == 0)
#endif
            break;
    }

    return;
}   /* End of Write_Read_Fifo ()    */

/*************************************************************************
	End of program (pa_6100_ur.c)
*************************************************************************/ 

