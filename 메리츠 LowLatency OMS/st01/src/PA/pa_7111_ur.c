#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 파생 시세수신 (UDP)
#	File	: pa_7111_ur.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include    "pa_struct.h"
#include	"fep_fepp.h"

#if defined A7191||A7291||A7391||A7491||A7891||A7991||A7992||A6591||A6691
#define     DATA_SIZE       1300
#elif defined A7192||A7193||A7292||A7293
#define     DATA_SIZE       300
#endif

#include    "buf_struct.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		SVR_PORT_NO		UDP_PORT(D_K,P_K,0)
#define		READ_BUF_SIZE	2048

/* 시세 Seq 위치 Size */
#define		S_H_SIZE		17						/* TR(5) + Code(12)	*/
/* 기본마스터 Seq 위치 Size */
#define		M_H_SIZE		30	/* TR(5) + JCNT(5) + DATE(8) + Code(12)	*/

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int		Sockfd, FIFO_fd, tot_cq;
int		Idx, Che_Gbn, w_gbn;
struct	ip_mreq     mreq;
char	TrCode[8], ApType[10], Order_St[512];
struct sockaddr_in	SvrAddr, ClntAddr;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_7111_UR (void);
int     Init_Parameters (void);
int		Socket_Connect (void);
int		Recv_Data (char *);
int		Add_Count_UR (void);
int		Add_Count_DD (char *);
void	Set_Sise (char *);
int		Write_Read_Fifo (int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_7111_UR ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_7111_UR (void)
/*----------------------------------------------------------------------*/
{
	char	m_time[24];
	char	Wdt[30], W2_Fmt[512];
	char 	r_buf[READ_BUF_SIZE];
	int		tr_gbn, rt, len, skip_rt;

	BUFF_RW_HEAD		f_head;
	FILE_BUFF_FORMAT    W_Fmt;

	rt = Init_Parameters ( );
	if (rt == NOTOK)
		return;

	rt = Socket_Connect ();
	if (rt == NOTOK)
		return;

	SLog (USR_OK, "socket connected:port[%d]", SVR_PORT_NO);

	while (START_S != JOB_END)
	{
/* 20211022
		Stat_Save ();
*/
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

		Che_Gbn = Idx = w_gbn = 0;
		Set_Sise (r_buf);
		
		/* 체결관련시 평가손익 && 자동전략 처리위한 전달 */
		if (Che_Gbn > 0)
		{
#if defined A7192||A7193
			if (Shm_Risk[0].S_Sise[1][Idx].auto_use > 0)			/* 기동중인 자동이 해당종목을 설정했을때 전달 */
				Write_Read_Fifo (1);
#elif defined A7292||A7293
			if (Shm_Risk[0].S_Sise[2][Idx].auto_use > 0)			/* 기동중인 자동이 해당종목을 설정했을때 전달 */
				Write_Read_Fifo (1);
#elif defined A7391
			if (Shm_Risk[0].S_Sise[3][Idx].auto_use > 0)			/* 기동중인 자동이 해당종목을 설정했을때 전달 */
				Write_Read_Fifo (1);
#elif defined A7491
			if (Shm_Risk[0].S_Sise[4][Idx].auto_use > 0)			/* 기동중인 자동이 해당종목을 설정했을때 전달 */
				Write_Read_Fifo (1);
#elif defined A7891
			if (Shm_Risk[0].S_Sise[8][Idx].auto_use > 0)			/* 기동중인 자동이 해당종목을 설정했을때 전달 */
				Write_Read_Fifo (1);
#elif defined A7991
			if (Shm_Risk[0].S_Sise[9][Idx].auto_use > 0)			/* 기동중인 자동이 해당종목을 설정했을때 전달 */
				Write_Read_Fifo (1);
#elif defined A7992
			if (Shm_Risk[0].S_Sise[10][Idx].auto_use > 0)			/* 기동중인 자동이 해당종목을 설정했을때 전달 */
				Write_Read_Fifo (1);
#elif defined A6591
			if (Shm_Risk[0].S_Sise[11][Idx].auto_use > 0)			/* 기동중인 자동이 해당종목을 설정했을때 전달 */
				Write_Read_Fifo (1);
#elif defined A6691
			if (Shm_Risk[0].S_Sise[12][Idx].auto_use > 0)			/* 기동중인 자동이 해당종목을 설정했을때 전달 */
				Write_Read_Fifo (1);
#endif
		}
		else if (Che_Gbn < 0)
		{
#if 0
			/* polling data */
			if (memcmp (r_buf, "I2", 2) == 0)
			{
				SLog (USR_OK, "I2 [%.5s]", r_buf);
			}
			else
				SLog (USR_OK, "Other [%.5s]", r_buf);
#endif
			continue;
		}

/* 지금은 필요 없을 듯
	    len = Add_Count_UR ();
*/

		/* 운영장비에서는 File Write를 하지않고 개발기나 테스트장비에서 File Write한다.		*/
		/* 해당 파일은 비상시 또는 장종류후 전략시뮬레이션을 위한 통자료로만 사용을 한다.	*/
		if ((memcmp(r_buf, "A0", 2) == 0)	||
			(Che_Gbn > 0	&& 
			 (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)) )
		{
			/* File Write */
			memset (&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));

			/* write to DD file	*/
			ItoAf (O_W_CNT1 + 1, W_Fmt.If_Seq, sizeof (W_Fmt.If_Seq));
			memcpy (W_Fmt.ApType, ApType, sizeof (W_Fmt.ApType));
			memcpy (W_Fmt.ResponseCode, RES_NORMAL, strlen (RES_NORMAL));
			memcpy (W_Fmt.RecvTime1, m_time, sizeof (W_Fmt.RecvTime1));
			memcpy (W_Fmt.RecvTime2, &m_time[sizeof(W_Fmt.RecvTime1)],
											sizeof (W_Fmt.RecvTime2));

			memset (W_Fmt.DataHeader, 0x20, 20+DATA_SIZE);
			memcpy (W_Fmt.Data, r_buf, strlen(r_buf));
			W_Fmt.LineFeed[0] = '\n';

			if (w_gbn == 0)
				rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);
			else
				rt = F_W (TS_W2_1, (void *)&W_Fmt, 1);
			if (rt != 1)
			{
				SLog (SAM_FATAL, "file write fail[%s] rt[%d]", OFN(D_K,P_K,w_gbn), rt);
					close (Sockfd);
					return;
			}

			Add_Count_DD (TrCode);
		}

//		Set_TR_Time ();
//		INT_SEQ ++;
	}

	return;
}	/* End of PA_7111_UR ()	*/

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
/* 7111/7211/7311/7411 : 전략기동중인 시장+종목을 수신했을때 전략에 전달용	*/
/* ************************************************************************ */
#if defined A7192||A7193
	sprintf (fifo_name, "%s/PA/pa_7111_ur1", _FEP_FIFO);
#elif defined A7292||A7293
	sprintf (fifo_name, "%s/PA/pa_7211_ur1", _FEP_FIFO);
#elif defined A7391
	sprintf (fifo_name, "%s/PA/pa_7311_ur1", _FEP_FIFO);
#elif defined A7491
	sprintf (fifo_name, "%s/PA/pa_7411_ur1", _FEP_FIFO);
#elif defined A7891
	sprintf (fifo_name, "%s/PA/pa_7811_ur1", _FEP_FIFO);
#elif defined A7991
	sprintf (fifo_name, "%s/PA/pa_7911_ur1", _FEP_FIFO);
#elif defined A7992
	sprintf (fifo_name, "%s/PA/pa_7912_ur1", _FEP_FIFO);
#elif defined A6591
	sprintf (fifo_name, "%s/PA/pa_6511_ur1", _FEP_FIFO);
#elif defined A6691
	sprintf (fifo_name, "%s/PA/pa_6611_ur1", _FEP_FIFO);
#endif

#if defined A7192||A7193 || A7292||A7293 || A7391 || A7491 || A7891 || A7991 || A7992 || A6591||A6691
	/* 전략전달용 FIFO */
	FIFO_fd = open (fifo_name, O_RDWR|O_NDELAY);
	if (FIFO_fd < 0) 
		SLog (FIF_FATAL, "cannot open FIFO[%s][%d][%d:%s]",
			fifo_name, FIFO_fd, SYS_NO, SYS_STR);
#endif

	/* 누적체결수량 */
	tot_cq = 0;

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
/* 202108
    SvrAddr.sin_addr.s_addr    = inet_addr (ip_addr);
*/
    inet_pton(AF_INET, ip_addr, &SvrAddr.sin_addr.s_addr);
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
#if 0
	val = 524288;												/* 512K	*/
#endif
	val = 1228800;												/* 1228800K	*/
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
		rt = recvfrom (Sockfd, p_str, READ_BUF_SIZE, 0,
			(struct sockaddr *)&ClntAddr, (socklen_t *)&len);
#else
		rt = recvfrom (Sockfd, p_str, READ_BUF_SIZE, NULL,
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
	int			s_k, sign;

/* *********************************** */
/* 시장별, TR별 종목 일련번호 갖고오기 */
/* *********************************** */
#if defined A7191
	if (memcmp (p_buf, "A0014", 5) != 0)
	{
		Che_Gbn = -1;
		return;
	}
#elif defined A7291
	if (memcmp (p_buf, "A0034", 5) != 0)
	{
		Che_Gbn = -1;
		return;
	}
#elif defined A7192||A7193
	/* 수신시세 => Memory 시세에 Set */
	if (memcmp (p_buf, "A3014", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Futures[0].A3.seq_no));
		memcpy (Shm_Futures[Idx].A3.tr_gbn, p_buf, sizeof (SIF_A3014));

		/* 공용 현재가 */
		if (memcmp (Shm_Futures[Idx].A3.crprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[1][Idx].crprc =
			sign *
			(double)AtoIf(Shm_Futures[Idx].A3.crprc, sizeof(Shm_Futures[0].A3.crprc))/100;
		/* 공용 실시간상한가가격 */
		if (memcmp (Shm_Futures[Idx].A3.realtime_hprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[1][Idx].realtime_hprc =
			sign *
			(double)AtoIf(Shm_Futures[Idx].A3.realtime_hprc, sizeof(Shm_Futures[0].A3.realtime_hprc))/100;
		/* 공용 실시간하한가가격 */
		if (memcmp (Shm_Futures[Idx].A3.realtime_lprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[1][Idx].realtime_lprc =
			sign *
			(double)AtoIf(Shm_Futures[Idx].A3.realtime_lprc, sizeof(Shm_Futures[0].A3.realtime_lprc))/100;
	}
	else if (memcmp (p_buf, "G7014", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Futures[0].A3.seq_no));
		Shm_Futures[Idx].HogaLastGbn = 0;
		memcpy (Shm_Futures[Idx].G7.tr_gbn, p_buf, sizeof (SIF_G7014));

		/* 공용 현재가 */
		if (memcmp (Shm_Futures[Idx].G7.crprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[1][Idx].crprc =
			sign *
			(double)AtoIf(Shm_Futures[Idx].G7.crprc, sizeof(Shm_Futures[0].G7.crprc))/100;
		/* 공용 실시간상한가가격 */
		if (memcmp (Shm_Futures[Idx].G7.realtime_hprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[1][Idx].realtime_hprc =
			sign *
			(double)AtoIf(Shm_Futures[Idx].G7.realtime_hprc, sizeof(Shm_Futures[0].G7.realtime_hprc))/100;
		/* 공용 실시간하한가가격 */
		if (memcmp (Shm_Futures[Idx].G7.realtime_lprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[1][Idx].realtime_lprc =
			sign *
			(double)AtoIf(Shm_Futures[Idx].G7.realtime_lprc, sizeof(Shm_Futures[0].G7.realtime_lprc))/100;
		/* 공용 매도1호가가격 */
		if (memcmp (Shm_Futures[Idx].G7.sell_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[1][Idx].sell_1_price =
			sign *
			(double)AtoIf(Shm_Futures[Idx].G7.sell_1_price, sizeof(Shm_Futures[0].G7.sell_1_price))/100;
		/* 공용 매수1호가가격 */
		if (memcmp (Shm_Futures[Idx].G7.buy_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[1][Idx].buy_1_price =
			sign *
			(double)AtoIf(Shm_Futures[Idx].G7.buy_1_price, sizeof(Shm_Futures[0].G7.buy_1_price))/100;
	}	
	else if (memcmp (p_buf, "B6014", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Futures[0].A3.seq_no));
		Shm_Futures[Idx].HogaLastGbn = 1;
		memcpy (Shm_Futures[Idx].B6.tr_gbn, p_buf, sizeof (SIF_B6014));

		/* 공용 매도1호가가격 */
		if (memcmp (Shm_Futures[Idx].B6.sell_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[1][Idx].sell_1_price =
			sign *
			(double)AtoIf(Shm_Futures[Idx].B6.sell_1_price, sizeof(Shm_Futures[0].B6.sell_1_price))/100;
		/* 공용 매수1호가가격 */
		if (memcmp (Shm_Futures[Idx].B6.buy_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[1][Idx].buy_1_price =
			sign *
			(double)AtoIf(Shm_Futures[Idx].B6.buy_1_price, sizeof(Shm_Futures[0].B6.buy_1_price))/100;
	}
	else if (memcmp (p_buf, "M4014", 5) == 0)
	{
		memcpy (Shm_Futures[0].M4.tr_gbn, p_buf, sizeof (M4014));
	}
	else if (memcmp (p_buf, "V1014", 5) == 0)
	{
		memcpy (Shm_Futures[0].V1.tr_gbn, p_buf, sizeof (V1014));
	}
	else if (memcmp (p_buf, "Q2014", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Futures[0].Q2.seq_no));
		memcpy (Shm_Futures[Idx].Q2.tr_gbn, p_buf, sizeof (Q2014));
	}
	else
	{
		Che_Gbn = -1;
	}
#elif defined A7292||A7293
	/* 수신시세 => Memory 시세에 Set */
	if (memcmp (p_buf, "A3034", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Options[0].A3.seq_no));
		memcpy (Shm_Options[Idx].A3.tr_gbn, p_buf, sizeof (SIO_A3034));

		/* 공용 현재가 */
		Shm_Risk[0].S_Sise[2][Idx].crprc =
			(double)AtoIf(Shm_Options[Idx].A3.crprc, sizeof(Shm_Options[0].A3.crprc))/100;
		/* 공용 실시간상한가가격 */
		Shm_Risk[0].S_Sise[2][Idx].realtime_hprc =
			(double)AtoIf(Shm_Options[Idx].A3.realtime_hprc, sizeof(Shm_Options[0].A3.realtime_hprc))/100;
		/* 공용 실시간하한가가격 */
		Shm_Risk[0].S_Sise[2][Idx].realtime_lprc =
			(double)AtoIf(Shm_Options[Idx].A3.realtime_lprc, sizeof(Shm_Options[0].A3.realtime_lprc))/100;
	}
	else if (memcmp (p_buf, "G7034", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Options[0].A3.seq_no));
		Shm_Options[Idx].HogaLastGbn = 0;
		memcpy (Shm_Options[Idx].G7.tr_gbn, p_buf, sizeof (SIO_G7034));

		/* 공용 현재가 */
		Shm_Risk[0].S_Sise[2][Idx].crprc =
			(double)AtoIf(Shm_Options[Idx].G7.crprc, sizeof(Shm_Options[0].G7.crprc))/100;
		/* 공용 실시간상한가가격 */
		Shm_Risk[0].S_Sise[2][Idx].realtime_hprc = 
			(double)AtoIf(Shm_Options[Idx].G7.realtime_hprc, sizeof(Shm_Options[0].G7.realtime_hprc))/100;
		/* 공용 실시간하한가가격 */
		Shm_Risk[0].S_Sise[2][Idx].realtime_lprc =
			(double)AtoIf(Shm_Options[Idx].G7.realtime_lprc, sizeof(Shm_Options[0].G7.realtime_lprc))/100;
		/* 공용 매도1호가가격 */
		Shm_Risk[0].S_Sise[2][Idx].sell_1_price =
			(double)AtoIf(Shm_Options[Idx].G7.sell_1_price, sizeof(Shm_Options[0].G7.sell_1_price))/100;
		/* 공용 매수1호가가격 */
		Shm_Risk[0].S_Sise[2][Idx].buy_1_price =
			(double)AtoIf(Shm_Options[Idx].G7.buy_1_price, sizeof(Shm_Options[0].G7.buy_1_price))/100;
	}	
	else if (memcmp (p_buf, "B6034", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Options[0].A3.seq_no));
		Shm_Options[Idx].HogaLastGbn = 1;
		memcpy (Shm_Options[Idx].B6.tr_gbn, p_buf, sizeof (SIO_B6034));

		/* 공용 매도1호가가격 */
		Shm_Risk[0].S_Sise[2][Idx].sell_1_price =
			(double)AtoIf(Shm_Options[Idx].B6.sell_1_price, sizeof(Shm_Options[0].B6.sell_1_price))/100;
		/* 공용 매수1호가가격 */
		Shm_Risk[0].S_Sise[2][Idx].buy_1_price =
			(double)AtoIf(Shm_Options[Idx].B6.buy_1_price, sizeof(Shm_Options[0].B6.buy_1_price))/100;
	}
	else if (memcmp (p_buf, "M4034", 5) == 0)
	{
		memcpy (Shm_Options[0].M4.tr_gbn, p_buf, sizeof (M4034));
	}
	else if (memcmp (p_buf, "V1034", 5) == 0)
	{
		memcpy (Shm_Options[0].V1.tr_gbn, p_buf, sizeof (V1034));
	}
	else if (memcmp (p_buf, "Q2034", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Options[0].Q2.seq_no));
		memcpy (Shm_Options[Idx].Q2.tr_gbn, p_buf, sizeof (Q2034));
	}
	else
	{
		Che_Gbn = -1;
	}
#elif defined A7391		// 개별선물 
	w_gbn = 1;
	/* 수신시세 => Memory 시세에 Set */
	if (memcmp (p_buf, "A3015", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_SFutures[0].A3.seq_no));
		memcpy (Shm_SFutures[Idx].A3.tr_gbn, p_buf, sizeof (SSF_A3015));

		/* 공용 현재가 */
		if (memcmp (Shm_SFutures[Idx].A3.crprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[3][Idx].crprc =
			sign *
			(double)AtoIf(Shm_SFutures[Idx].A3.crprc, sizeof(Shm_SFutures[0].A3.crprc));
		/* 공용 실시간상한가가격 */
		if (memcmp (Shm_SFutures[Idx].A3.realtime_hprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[3][Idx].realtime_hprc =
			sign *
			(double)AtoIf(Shm_SFutures[Idx].A3.realtime_hprc, sizeof(Shm_SFutures[0].A3.realtime_hprc));
		/* 공용 실시간하한가가격 */
		if (memcmp (Shm_SFutures[Idx].A3.realtime_lprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[3][Idx].realtime_lprc =
			sign *
			(double)AtoIf(Shm_SFutures[Idx].A3.realtime_lprc, sizeof(Shm_SFutures[0].A3.realtime_lprc));
	}
	else if (memcmp (p_buf, "G7015", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_SFutures[0].A3.seq_no));
		Shm_SFutures[Idx].HogaLastGbn = 0;
		memcpy (Shm_SFutures[Idx].G7.tr_gbn, p_buf, sizeof (SSF_G7015));

		/* 공용 현재가 */
		if (memcmp (Shm_SFutures[Idx].G7.crprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[3][Idx].crprc =
			sign *
			(double)AtoIf(Shm_SFutures[Idx].G7.crprc, sizeof(Shm_SFutures[0].G7.crprc));
		/* 공용 실시간상한가가격 */
		if (memcmp (Shm_SFutures[Idx].G7.realtime_hprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[3][Idx].realtime_hprc =
			sign *
			(double)AtoIf(Shm_SFutures[Idx].G7.realtime_hprc, sizeof(Shm_SFutures[0].G7.realtime_hprc));
		/* 공용 실시간하한가가격 */
		if (memcmp (Shm_SFutures[Idx].G7.realtime_lprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[3][Idx].realtime_lprc =
			sign *
			(double)AtoIf(Shm_SFutures[Idx].G7.realtime_lprc, sizeof(Shm_SFutures[0].G7.realtime_lprc));
		/* 공용 매도1호가가격 */
		if (memcmp (Shm_SFutures[Idx].G7.ask_hoga[0].ask_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[3][Idx].sell_1_price =
			sign *
			(double)AtoIf(Shm_SFutures[Idx].G7.ask_hoga[0].ask, sizeof(Shm_SFutures[0].G7.ask_hoga[0].ask));
		/* 공용 매수1호가가격 */
		if (memcmp (Shm_SFutures[Idx].G7.bid_hoga[0].bid_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[3][Idx].buy_1_price =
			sign *
			(double)AtoIf(Shm_SFutures[Idx].G7.bid_hoga[0].bid, sizeof(Shm_SFutures[0].G7.bid_hoga[0].bid));
	}	
	else if (memcmp (p_buf, "B6015", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_SFutures[0].A3.seq_no));
		Shm_SFutures[Idx].HogaLastGbn = 1;
		memcpy (Shm_SFutures[Idx].B6.tr_gbn, p_buf, sizeof (SSF_B6015));

		/* 공용 매도1호가가격 */
		if (memcmp (Shm_SFutures[Idx].B6.ask_hoga[0].ask_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[3][Idx].sell_1_price =
			sign *
			(double)AtoIf(Shm_SFutures[Idx].B6.ask_hoga[0].ask, sizeof(Shm_SFutures[0].B6.ask_hoga[0].ask));
		/* 공용 매수1호가가격 */
		if (memcmp (Shm_SFutures[Idx].B6.bid_hoga[0].bid_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[3][Idx].buy_1_price =
			sign *
			(double)AtoIf(Shm_SFutures[Idx].B6.bid_hoga[0].bid, sizeof(Shm_SFutures[0].B6.bid_hoga[0].bid));
	}
	else if (memcmp (p_buf, "M4015", 5) == 0)
	{
		memcpy (Shm_SFutures[0].M4.tr_gbn, p_buf, sizeof (M4025));	// 015,025공통사용
	}
	else if (memcmp (p_buf, "V1015", 5) == 0)
	{
		memcpy (Shm_SFutures[0].V1.tr_gbn, p_buf, sizeof (V1015));	// 015,025공통사용
	}
	else if (memcmp (p_buf, "Q2015", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_SFutures[0].Q2.seq_no));
		memcpy (Shm_SFutures[Idx].Q2.tr_gbn, p_buf, sizeof (Q2015));
	}
	else if (memcmp (p_buf, "A0015", 5) == 0)
	{
		w_gbn = 0;
		return;
	}
	else
	{
		Che_Gbn = -1;
	}
#elif defined A7491
	w_gbn = 1;
	/* 수신시세 => Memory 시세에 Set */
	if (memcmp (p_buf, "A3025", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_SOptions[0].A3.seq_no));
		memcpy (Shm_SOptions[Idx].A3.tr_gbn, p_buf, sizeof (SSO_A3025));

		/* 공용 현재가 */
		Shm_Risk[0].S_Sise[4][Idx].crprc =
			(double)AtoIf(Shm_SOptions[Idx].A3.crprc, sizeof(Shm_SOptions[0].A3.crprc));
		/* 공용 실시간상한가가격 */
		Shm_Risk[0].S_Sise[4][Idx].realtime_hprc =
			(double)AtoIf(Shm_SOptions[Idx].A3.realtime_hprc, sizeof(Shm_SOptions[0].A3.realtime_hprc));
		/* 공용 실시간하한가가격 */
		Shm_Risk[0].S_Sise[4][Idx].realtime_lprc =
			(double)AtoIf(Shm_SOptions[Idx].A3.realtime_lprc, sizeof(Shm_SOptions[0].A3.realtime_lprc));
	}
	else if (memcmp (p_buf, "G7025", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_SOptions[0].A3.seq_no));
		Shm_SOptions[Idx].HogaLastGbn = 0;
		memcpy (Shm_SOptions[Idx].G7.tr_gbn, p_buf, sizeof (SSO_G7025));

		/* 공용 현재가 */
		Shm_Risk[0].S_Sise[4][Idx].crprc =
			(double)AtoIf(Shm_SOptions[Idx].G7.crprc, sizeof(Shm_SOptions[0].G7.crprc));
		/* 공용 실시간상한가가격 */
		Shm_Risk[0].S_Sise[4][Idx].realtime_hprc = 
			(double)AtoIf(Shm_SOptions[Idx].G7.realtime_hprc, sizeof(Shm_SOptions[0].G7.realtime_hprc));
		/* 공용 실시간하한가가격 */
		Shm_Risk[0].S_Sise[4][Idx].realtime_lprc =
			(double)AtoIf(Shm_SOptions[Idx].G7.realtime_lprc, sizeof(Shm_SOptions[0].G7.realtime_lprc));
		/* 공용 매도1호가가격 */
		Shm_Risk[0].S_Sise[4][Idx].sell_1_price =
			(double)AtoIf(Shm_SOptions[Idx].G7.ask_hoga[0].ask, sizeof(Shm_SOptions[0].G7.ask_hoga[0].ask));
		/* 공용 매수1호가가격 */
		Shm_Risk[0].S_Sise[4][Idx].buy_1_price =
			(double)AtoIf(Shm_SOptions[Idx].G7.bid_hoga[0].bid, sizeof(Shm_SOptions[0].G7.bid_hoga[0].bid));
	}	
	else if (memcmp (p_buf, "B6025", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_SOptions[0].A3.seq_no));
		Shm_SOptions[Idx].HogaLastGbn = 1;
		memcpy (Shm_SOptions[Idx].B6.tr_gbn, p_buf, sizeof (SSO_B6025));

		/* 공용 매도1호가가격 */
		Shm_Risk[0].S_Sise[4][Idx].sell_1_price =
			(double)AtoIf(Shm_SOptions[Idx].B6.ask_hoga[0].ask, sizeof(Shm_SOptions[0].B6.ask_hoga[0].ask));
		/* 공용 매수1호가가격 */
		Shm_Risk[0].S_Sise[4][Idx].buy_1_price =
			(double)AtoIf(Shm_SOptions[Idx].B6.bid_hoga[0].bid, sizeof(Shm_SOptions[0].B6.bid_hoga[0].bid));
	}
	else if (memcmp (p_buf, "M4025", 5) == 0)
	{
		memcpy (Shm_SOptions[0].M4.tr_gbn, p_buf, sizeof (M4025));
	}
	else if (memcmp (p_buf, "V1025", 5) == 0)
	{
		memcpy (Shm_SOptions[0].V1.tr_gbn, p_buf, sizeof (V1025));
	}
	else if (memcmp (p_buf, "A0025", 5) == 0)
	{
		w_gbn = 0;
		return;
	}
	else
	{
		Che_Gbn = -1;
	}
#elif defined A7891
	w_gbn = 1;
	/* 수신시세 => Memory 시세에 Set */
	if (memcmp (p_buf, "A3164", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_K300[0].A3.seq_no));
		memcpy (Shm_K300[Idx].A3.tr_gbn, p_buf, sizeof (A3164));

		/* 공용 현재가 */
		if (memcmp (Shm_K300[Idx].A3.crprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[8][Idx].crprc =
			sign *
			(double)AtoIf(Shm_K300[Idx].A3.crprc, sizeof(Shm_K300[0].A3.crprc))/100;
		/* 공용 실시간상한가가격 */
		if (memcmp (Shm_K300[Idx].A3.realtime_hprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[8][Idx].realtime_hprc =
			sign *
			(double)AtoIf(Shm_K300[Idx].A3.realtime_hprc, sizeof(Shm_K300[0].A3.realtime_hprc))/100;
		/* 공용 실시간하한가가격 */
		if (memcmp (Shm_K300[Idx].A3.realtime_lprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[8][Idx].realtime_lprc =
			sign *
			(double)AtoIf(Shm_K300[Idx].A3.realtime_lprc, sizeof(Shm_K300[0].A3.realtime_lprc))/100;
	}
	else if (memcmp (p_buf, "G7164", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_K300[0].A3.seq_no));
		Shm_K300[Idx].HogaLastGbn = 0;
		memcpy (Shm_K300[Idx].G7.tr_gbn, p_buf, sizeof (G7164));

		/* 공용 현재가 */
		if (memcmp (Shm_K300[Idx].G7.crprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[8][Idx].crprc =
			sign *
			(double)AtoIf(Shm_K300[Idx].G7.crprc, sizeof(Shm_K300[0].G7.crprc))/100;
		/* 공용 실시간상한가가격 */
		if (memcmp (Shm_K300[Idx].G7.realtime_hprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[8][Idx].realtime_hprc =
			sign *
			(double)AtoIf(Shm_K300[Idx].G7.realtime_hprc, sizeof(Shm_K300[0].G7.realtime_hprc))/100;
		/* 공용 실시간하한가가격 */
		if (memcmp (Shm_K300[Idx].G7.realtime_lprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[8][Idx].realtime_lprc =
			sign *
			(double)AtoIf(Shm_K300[Idx].G7.realtime_lprc, sizeof(Shm_K300[0].G7.realtime_lprc))/100;
		/* 공용 매도1호가가격 */
		if (memcmp (Shm_K300[Idx].G7.sell_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[8][Idx].sell_1_price =
			sign *
			(double)AtoIf(Shm_K300[Idx].G7.sell_1_price, sizeof(Shm_K300[0].G7.sell_1_price))/100;
		/* 공용 매수1호가가격 */
		if (memcmp (Shm_K300[Idx].G7.buy_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[8][Idx].buy_1_price =
			sign *
			(double)AtoIf(Shm_K300[Idx].G7.buy_1_price, sizeof(Shm_K300[0].G7.buy_1_price))/100;
	}	
	else if (memcmp (p_buf, "B6164", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_K300[0].A3.seq_no));
		Shm_K300[Idx].HogaLastGbn = 1;
		memcpy (Shm_K300[Idx].B6.tr_gbn, p_buf, sizeof (B6164));

		/* 공용 매도1호가가격 */
		if (memcmp (Shm_K300[Idx].B6.sell_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[8][Idx].sell_1_price =
			sign *
			(double)AtoIf(Shm_K300[Idx].B6.sell_1_price, sizeof(Shm_K300[0].B6.sell_1_price))/100;
		/* 공용 매수1호가가격 */
		if (memcmp (Shm_K300[Idx].B6.buy_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[8][Idx].buy_1_price =
			sign *
			(double)AtoIf(Shm_K300[Idx].B6.buy_1_price, sizeof(Shm_K300[0].B6.buy_1_price))/100;
	}
	else if (memcmp (p_buf, "M4164", 5) == 0)
	{
		memcpy (Shm_K300[0].M4.tr_gbn, p_buf, sizeof (M4164));
	}
	else if (memcmp (p_buf, "V1164", 5) == 0)
	{
		memcpy (Shm_K300[0].V1.tr_gbn, p_buf, sizeof (V1164));
	}
	else if (memcmp (p_buf, "A0164", 5) == 0)
	{
		w_gbn = 0;
		return;
	}
	else
	{
		Che_Gbn = -1;
	}
#elif defined A7991
	w_gbn = 1;
	/* 수신시세 => Memory 시세에 Set */
	if (memcmp (p_buf, "A3024", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_K150F[0].A3.seq_no));
		memcpy (Shm_K150F[Idx].A3.tr_gbn, p_buf, sizeof (A3024));

		/* 공용 현재가 */
		if (memcmp (Shm_K150F[Idx].A3.crprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[9][Idx].crprc =
			sign *
			(double)AtoIf(Shm_K150F[Idx].A3.crprc, sizeof(Shm_K150F[0].A3.crprc))/100;
		/* 공용 실시간상한가가격 */
		if (memcmp (Shm_K150F[Idx].A3.realtime_hprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[9][Idx].realtime_hprc =
			sign *
			(double)AtoIf(Shm_K150F[Idx].A3.realtime_hprc, sizeof(Shm_K150F[0].A3.realtime_hprc))/100;
		/* 공용 실시간하한가가격 */
		if (memcmp (Shm_K150F[Idx].A3.realtime_lprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[9][Idx].realtime_lprc =
			sign *
			(double)AtoIf(Shm_K150F[Idx].A3.realtime_lprc, sizeof(Shm_K150F[0].A3.realtime_lprc))/100;
	}
	else if (memcmp (p_buf, "G7024", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_K150F[0].A3.seq_no));
		Shm_K150F[Idx].HogaLastGbn = 0;
		memcpy (Shm_K150F[Idx].G7.tr_gbn, p_buf, sizeof (G7024));

		/* 공용 현재가 */
		if (memcmp (Shm_K150F[Idx].G7.crprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[9][Idx].crprc =
			sign *
			(double)AtoIf(Shm_K150F[Idx].G7.crprc, sizeof(Shm_K150F[0].G7.crprc))/100;
		/* 공용 실시간상한가가격 */
		if (memcmp (Shm_K150F[Idx].G7.realtime_hprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[9][Idx].realtime_hprc =
			sign *
			(double)AtoIf(Shm_K150F[Idx].G7.realtime_hprc, sizeof(Shm_K150F[0].G7.realtime_hprc))/100;
		/* 공용 실시간하한가가격 */
		if (memcmp (Shm_K150F[Idx].G7.realtime_lprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[9][Idx].realtime_lprc =
			sign *
			(double)AtoIf(Shm_K150F[Idx].G7.realtime_lprc, sizeof(Shm_K150F[0].G7.realtime_lprc))/100;
		/* 공용 매도1호가가격 */
		if (memcmp (Shm_K150F[Idx].G7.sell_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[9][Idx].sell_1_price =
			sign *
			(double)AtoIf(Shm_K150F[Idx].G7.sell_1_price, sizeof(Shm_K150F[0].G7.sell_1_price))/100;
		/* 공용 매수1호가가격 */
		if (memcmp (Shm_K150F[Idx].G7.buy_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[9][Idx].buy_1_price =
			sign *
			(double)AtoIf(Shm_K150F[Idx].G7.buy_1_price, sizeof(Shm_K150F[0].G7.buy_1_price))/100;
	}	
	else if (memcmp (p_buf, "B6024", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_K150F[0].A3.seq_no));
		Shm_K150F[Idx].HogaLastGbn = 1;
		memcpy (Shm_K150F[Idx].B6.tr_gbn, p_buf, sizeof (B6024));

		/* 공용 매도1호가가격 */
		if (memcmp (Shm_K150F[Idx].B6.sell_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[9][Idx].sell_1_price =
			sign *
			(double)AtoIf(Shm_K150F[Idx].B6.sell_1_price, sizeof(Shm_K150F[0].B6.sell_1_price))/100;
		/* 공용 매수1호가가격 */
		if (memcmp (Shm_K150F[Idx].B6.buy_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[9][Idx].buy_1_price =
			sign *
			(double)AtoIf(Shm_K150F[Idx].B6.buy_1_price, sizeof(Shm_K150F[0].B6.buy_1_price))/100;
	}
	else if (memcmp (p_buf, "M4024", 5) == 0)
	{
		memcpy (Shm_K150F[0].M4.tr_gbn, p_buf, sizeof (M4024));
	}
	else if (memcmp (p_buf, "V1024", 5) == 0)
	{
		memcpy (Shm_K150F[0].V1.tr_gbn, p_buf, sizeof (V1024));
	}
	else if (memcmp (p_buf, "A0024", 5) == 0)
	{
		w_gbn = 0;
		return;
	}
	else
	{
		Che_Gbn = -1;
	}
#elif defined A7992
	w_gbn = 1;
	/* 수신시세 => Memory 시세에 Set */
	if (memcmp (p_buf, "A3174", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_K150O[0].A3.seq_no));
		memcpy (Shm_K150O[Idx].A3.tr_gbn, p_buf, sizeof (A3174));

		/* 공용 현재가 */
		Shm_Risk[0].S_Sise[10][Idx].crprc =
			(double)AtoIf(Shm_K150O[Idx].A3.crprc, sizeof(Shm_K150O[0].A3.crprc))/100;
		/* 공용 실시간상한가가격 */
		Shm_Risk[0].S_Sise[10][Idx].realtime_hprc =
			(double)AtoIf(Shm_K150O[Idx].A3.realtime_hprc, sizeof(Shm_K150O[0].A3.realtime_hprc))/100;
		/* 공용 실시간하한가가격 */
		Shm_Risk[0].S_Sise[10][Idx].realtime_lprc =
			(double)AtoIf(Shm_K150O[Idx].A3.realtime_lprc, sizeof(Shm_K150O[0].A3.realtime_lprc))/100;
	}
	else if (memcmp (p_buf, "G7174", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_K150O[0].A3.seq_no));
		Shm_K150O[Idx].HogaLastGbn = 0;
		memcpy (Shm_K150O[Idx].G7.tr_gbn, p_buf, sizeof (G7174));

		/* 공용 현재가 */
		Shm_Risk[0].S_Sise[10][Idx].crprc =
			(double)AtoIf(Shm_K150O[Idx].G7.crprc, sizeof(Shm_K150O[0].G7.crprc))/100;
		/* 공용 실시간상한가가격 */
		Shm_Risk[0].S_Sise[10][Idx].realtime_hprc = 
			(double)AtoIf(Shm_K150O[Idx].G7.realtime_hprc, sizeof(Shm_K150O[0].G7.realtime_hprc))/100;
		/* 공용 실시간하한가가격 */
		Shm_Risk[0].S_Sise[10][Idx].realtime_lprc =
			(double)AtoIf(Shm_K150O[Idx].G7.realtime_lprc, sizeof(Shm_K150O[0].G7.realtime_lprc))/100;
		/* 공용 매도1호가가격 */
		Shm_Risk[0].S_Sise[10][Idx].sell_1_price =
			(double)AtoIf(Shm_K150O[Idx].G7.sell_1_price, sizeof(Shm_K150O[0].G7.sell_1_price))/100;
		/* 공용 매수1호가가격 */
		Shm_Risk[0].S_Sise[10][Idx].buy_1_price =
			(double)AtoIf(Shm_K150O[Idx].G7.buy_1_price, sizeof(Shm_K150O[0].G7.buy_1_price))/100;
	}	
	else if (memcmp (p_buf, "B6174", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_K150O[0].A3.seq_no));
		Shm_K150O[Idx].HogaLastGbn = 1;
		memcpy (Shm_K150O[Idx].B6.tr_gbn, p_buf, sizeof (B6174));

		/* 공용 매도1호가가격 */
		Shm_Risk[0].S_Sise[10][Idx].sell_1_price =
			(double)AtoIf(Shm_K150O[Idx].B6.sell_1_price, sizeof(Shm_K150O[0].B6.sell_1_price))/100;
		/* 공용 매수1호가가격 */
		Shm_Risk[0].S_Sise[10][Idx].buy_1_price =
			(double)AtoIf(Shm_K150O[Idx].B6.buy_1_price, sizeof(Shm_K150O[0].B6.buy_1_price))/100;
	}
	else if (memcmp (p_buf, "M4174", 5) == 0)
	{
		memcpy (Shm_K150O[0].M4.tr_gbn, p_buf, sizeof (M4174));
	}
	else if (memcmp (p_buf, "V1174", 5) == 0)
	{
		memcpy (Shm_K150O[0].V1.tr_gbn, p_buf, sizeof (V1174));
	}
	else if (memcmp (p_buf, "A0174", 5) == 0)
	{
		w_gbn = 0;
		return;
	}
	else
	{
		Che_Gbn = -1;
	}
#elif defined A6591
	w_gbn = 1;
	/* 수신시세 => Memory 시세에 Set */
	if (memcmp (p_buf, "A3124", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_MF[0].A3.seq_no));
		memcpy (Shm_MF[Idx].A3.tr_gbn, p_buf, sizeof (SIF_A3014));

		/* 공용 현재가 */
		if (memcmp (Shm_MF[Idx].A3.crprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[11][Idx].crprc =
			sign *
			(double)AtoIf(Shm_MF[Idx].A3.crprc, sizeof(Shm_MF[0].A3.crprc))/100;
		/* 공용 실시간상한가가격 */
		if (memcmp (Shm_MF[Idx].A3.realtime_hprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[11][Idx].realtime_hprc =
			sign *
			(double)AtoIf(Shm_MF[Idx].A3.realtime_hprc, sizeof(Shm_MF[0].A3.realtime_hprc))/100;
		/* 공용 실시간하한가가격 */
		if (memcmp (Shm_MF[Idx].A3.realtime_lprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[11][Idx].realtime_lprc =
			sign *
			(double)AtoIf(Shm_MF[Idx].A3.realtime_lprc, sizeof(Shm_MF[0].A3.realtime_lprc))/100;
	}
	else if (memcmp (p_buf, "G7124", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_MF[0].A3.seq_no));
		Shm_MF[Idx].HogaLastGbn = 0;
		memcpy (Shm_MF[Idx].G7.tr_gbn, p_buf, sizeof (SIF_G7014));

		/* 공용 현재가 */
		if (memcmp (Shm_MF[Idx].G7.crprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[11][Idx].crprc =
			sign *
			(double)AtoIf(Shm_MF[Idx].G7.crprc, sizeof(Shm_MF[0].G7.crprc))/100;
		/* 공용 실시간상한가가격 */
		if (memcmp (Shm_MF[Idx].G7.realtime_hprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[11][Idx].realtime_hprc =
			sign *
			(double)AtoIf(Shm_MF[Idx].G7.realtime_hprc, sizeof(Shm_MF[0].G7.realtime_hprc))/100;
		/* 공용 실시간하한가가격 */
		if (memcmp (Shm_MF[Idx].G7.realtime_lprc_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[11][Idx].realtime_lprc =
			sign *
			(double)AtoIf(Shm_MF[Idx].G7.realtime_lprc, sizeof(Shm_MF[0].G7.realtime_lprc))/100;
		/* 공용 매도1호가가격 */
		if (memcmp (Shm_MF[Idx].G7.sell_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[11][Idx].sell_1_price =
			sign *
			(double)AtoIf(Shm_MF[Idx].G7.sell_1_price, sizeof(Shm_MF[0].G7.sell_1_price))/100;
		/* 공용 매수1호가가격 */
		if (memcmp (Shm_MF[Idx].G7.buy_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[11][Idx].buy_1_price =
			sign *
			(double)AtoIf(Shm_MF[Idx].G7.buy_1_price, sizeof(Shm_MF[0].G7.buy_1_price))/100;
	}	
	else if (memcmp (p_buf, "B6124", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_MF[0].A3.seq_no));
		Shm_MF[Idx].HogaLastGbn = 1;
		memcpy (Shm_MF[Idx].B6.tr_gbn, p_buf, sizeof (SIF_B6014));

		/* 공용 매도1호가가격 */
		if (memcmp (Shm_MF[Idx].B6.sell_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[11][Idx].sell_1_price =
			sign *
			(double)AtoIf(Shm_MF[Idx].B6.sell_1_price, sizeof(Shm_MF[0].B6.sell_1_price))/100;
		/* 공용 매수1호가가격 */
		if (memcmp (Shm_MF[Idx].B6.buy_1_sign, "-", 1) == 0)
			sign = -1;
		else
			sign = 1;
		Shm_Risk[0].S_Sise[11][Idx].buy_1_price =
			sign *
			(double)AtoIf(Shm_MF[Idx].B6.buy_1_price, sizeof(Shm_MF[0].B6.buy_1_price))/100;
	}
	else if (memcmp (p_buf, "M4124", 5) == 0)
	{
		memcpy (Shm_MF[0].M4.tr_gbn, p_buf, sizeof (M4014));
	}
	else if (memcmp (p_buf, "V1124", 5) == 0)
	{
		memcpy (Shm_MF[0].V1.tr_gbn, p_buf, sizeof (V1014));
	}
	else if (memcmp (p_buf, "Q2124", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_MF[0].Q2.seq_no));
		memcpy (Shm_MF[Idx].Q2.tr_gbn, p_buf, sizeof (Q2014));
	}
	else if (memcmp (p_buf, "A0124", 5) == 0)
	{
		w_gbn = 0;
		return;
	}
	else
	{
		Che_Gbn = -1;
	}
#elif defined A6691
	w_gbn = 1;
	/* 수신시세 => Memory 시세에 Set */
	if (memcmp (p_buf, "A3134", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_MO[0].A3.seq_no));
		memcpy (Shm_MO[Idx].A3.tr_gbn, p_buf, sizeof (SIO_A3034));

		/* 공용 현재가 */
		Shm_Risk[0].S_Sise[12][Idx].crprc =
			(double)AtoIf(Shm_MO[Idx].A3.crprc, sizeof(Shm_MO[0].A3.crprc))/100;
		/* 공용 실시간상한가가격 */
		Shm_Risk[0].S_Sise[12][Idx].realtime_hprc =
			(double)AtoIf(Shm_MO[Idx].A3.realtime_hprc, sizeof(Shm_MO[0].A3.realtime_hprc))/100;
		/* 공용 실시간하한가가격 */
		Shm_Risk[0].S_Sise[12][Idx].realtime_lprc =
			(double)AtoIf(Shm_MO[Idx].A3.realtime_lprc, sizeof(Shm_MO[0].A3.realtime_lprc))/100;
	}
	else if (memcmp (p_buf, "G7134", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_MO[0].A3.seq_no));
		Shm_MO[Idx].HogaLastGbn = 0;
		memcpy (Shm_MO[Idx].G7.tr_gbn, p_buf, sizeof (SIO_G7034));

		/* 공용 현재가 */
		Shm_Risk[0].S_Sise[12][Idx].crprc =
			(double)AtoIf(Shm_MO[Idx].G7.crprc, sizeof(Shm_MO[0].G7.crprc))/100;
		/* 공용 실시간상한가가격 */
		Shm_Risk[0].S_Sise[12][Idx].realtime_hprc = 
			(double)AtoIf(Shm_MO[Idx].G7.realtime_hprc, sizeof(Shm_MO[0].G7.realtime_hprc))/100;
		/* 공용 실시간하한가가격 */
		Shm_Risk[0].S_Sise[12][Idx].realtime_lprc =
			(double)AtoIf(Shm_MO[Idx].G7.realtime_lprc, sizeof(Shm_MO[0].G7.realtime_lprc))/100;
		/* 공용 매도1호가가격 */
		Shm_Risk[0].S_Sise[12][Idx].sell_1_price =
			(double)AtoIf(Shm_MO[Idx].G7.sell_1_price, sizeof(Shm_MO[0].G7.sell_1_price))/100;
		/* 공용 매수1호가가격 */
		Shm_Risk[0].S_Sise[12][Idx].buy_1_price =
			(double)AtoIf(Shm_MO[Idx].G7.buy_1_price, sizeof(Shm_MO[0].G7.buy_1_price))/100;
	}	
	else if (memcmp (p_buf, "B6134", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_MO[0].A3.seq_no));
		Shm_MO[Idx].HogaLastGbn = 1;
		memcpy (Shm_MO[Idx].B6.tr_gbn, p_buf, sizeof (SIO_B6034));

		/* 공용 매도1호가가격 */
		Shm_Risk[0].S_Sise[12][Idx].sell_1_price =
			(double)AtoIf(Shm_MO[Idx].B6.sell_1_price, sizeof(Shm_MO[0].B6.sell_1_price))/100;
		/* 공용 매수1호가가격 */
		Shm_Risk[0].S_Sise[12][Idx].buy_1_price =
			(double)AtoIf(Shm_MO[Idx].B6.buy_1_price, sizeof(Shm_MO[0].B6.buy_1_price))/100;
	}
	else if (memcmp (p_buf, "M4134", 5) == 0)
	{
		memcpy (Shm_MO[0].M4.tr_gbn, p_buf, sizeof (M4034));
	}
	else if (memcmp (p_buf, "V1134", 5) == 0)
	{
		memcpy (Shm_MO[0].V1.tr_gbn, p_buf, sizeof (V1034));
	}
	else if (memcmp (p_buf, "Q2134", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_MO[0].Q2.seq_no));
		memcpy (Shm_MO[Idx].Q2.tr_gbn, p_buf, sizeof (Q2034));
	}
	else if (memcmp (p_buf, "A0134", 5) == 0)
	{
		w_gbn = 0;
		return;
	}
	else
	{
		Che_Gbn = -1;
	}
#endif

	return;
}	/* End of Set_Sise ()	*/

/*************************************************************************
	Function		: . Write_Read_Fifo
	Parameters IN	: . 1 : A3/G7체결, 2 : B6호가
	Parameters OUT	: . 
	Comment			: . write to FIFO
*************************************************************************/
int		Write_Read_Fifo (int c0h0)
{
	int		rt;
	char	tmp[128];

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
}	/* End of Write_Read_Fifo ()	*/

/*************************************************************************
	End of program (pa_7111_ur.c)
*************************************************************************/ 

