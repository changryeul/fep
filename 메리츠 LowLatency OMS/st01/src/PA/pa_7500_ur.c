#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 현물 시세수신 (UDP)
#	File	: pa_7500_ur.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include    "pa_struct.h"

/* 기본정보, 유가증권/코스닥(800), ELW(910) 기본 */
#if defined	A7501
#define     DATA_SIZE       910		/* 유가증권(800), ELW(910) 기본 */
#elif defined A7601
#define     DATA_SIZE       800		/* 코스닥(800) 기본 */
/* 유가증권 */
#elif defined A7511||A7512
#define     DATA_SIZE       160		/* A3011/A4011/M4011	*/
#elif defined A7521||A7522
#define     DATA_SIZE       800		/* B6011/B7011/B8011	*/
/* 코스닥 */
#elif defined A7611||A7612
#define     DATA_SIZE       160		/* A3012/A4012/M4012	*/
#elif defined A7621||A7622
#define     DATA_SIZE       560		/* B6012/B8012	*/
/* ELW */
#elif defined A7711||A7712
#define     DATA_SIZE       160		/* A3021	*/
#elif defined A7721||A7722
#define     DATA_SIZE       800		/* B6021	*/
/* JISU */
#elif defined A7001
#define     DATA_SIZE       50		/* D0011...	*/
#endif

#include    "buf_struct.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		SVR_PORT_NO		UDP_PORT(D_K,P_K,0)
#define		READ_BUF_SIZE	2048

/* 시세 Seq 위치 Size */
#define		S_H_SIZE		17						/* TR(5) + Code(12)	*/

KS_EXPCODE  Key;

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int		Sockfd, FIFO_fd, tot_cq;
int		Idx, Che_Gbn;
struct	ip_mreq     mreq;
char	TrCode[8], ApType[10], Order_St[512];
struct sockaddr_in	SvrAddr, ClntAddr;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_7500_UR (void);
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
	PA_7500_UR ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_7500_UR (void)
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

		Che_Gbn = Idx = 0;
		Set_Sise (r_buf);
		
		/* 체결관련시 평가손익 && 자동전략 처리위한 전달 */
		if (Che_Gbn == 1)
		{
#if defined A7511||A7512
			if (Shm_Risk[0].S_Sise[5][Idx].auto_use > 0)		/* 기동중인 자동이 해당종목을 설정했을때 전달 */
				Write_Read_Fifo (1);
#elif defined A7611||A7612
			if (Shm_Risk[0].S_Sise[6][Idx].auto_use > 0)		/* 기동중인 자동이 해당종목을 설정했을때 전달 */
				Write_Read_Fifo (1);
#elif defined A7711||A7712
			if (Shm_Risk[0].S_Sise[5][Idx].auto_use > 0)		/* 기동중인 자동이 해당종목을 설정했을때 전달 */
				Write_Read_Fifo (1);
#elif defined A7001
			//if (Shm_Risk[0].S_Sise[0][Idx].auto_use > 0)		/* 기동중인 자동이 해당종목을 설정했을때 전달 */
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
//				Set_TR_Time ();
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
		if ((memcmp(r_buf, "A0", 2) == 0)   ||
			(memcmp(r_buf, "A1", 2) == 0)   ||
			(memcmp(r_buf, "S1", 2) == 0)   ||
            (Che_Gbn > 0    &&
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

			if (memcmp(r_buf, "S1", 2) == 0) 
				rt = F_W (TS_W2_1, (void *)&W_Fmt, 1);
			else
				rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);

			if (rt != 1)
			{
				SLog (SAM_FATAL, "file write fail[%s] rt[%d]", OFN(D_K,P_K,Che_Gbn-1), rt);
					close (Sockfd);
					return;
			}

			Add_Count_DD (TrCode);
		}

//		Set_TR_Time ();
//		INT_SEQ ++;
	}

	return;
}	/* End of PA_7500_UR ()	*/

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

#if defined A7001
	Shm_Jisu[0].total_item_cnt = 0;
	memset (Shm_Item[0].J_Key, 0, sizeof (KS_EXPCODE) * SHM_MAX_JISU);
#endif
	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
	LtoU (ApType, strlen (ApType));

/* ************************************************************************ */
/* 7111/7211/7311/7411 : 전략기동중인 시장+종목을 수신했을때 전략에 전달용	*/
/* ************************************************************************ */
#if defined A7511||A7512
	sprintf (fifo_name, "%s/PA/pa_7511_ur1", _FEP_FIFO);
#elif defined A7611||A7612
	sprintf (fifo_name, "%s/PA/pa_7611_ur1", _FEP_FIFO);
#elif defined A7711||A7712
	sprintf (fifo_name, "%s/PA/pa_7711_ur1", _FEP_FIFO);
#elif defined A7001
	sprintf (fifo_name, "%s/PA/pa_7001_ur1", _FEP_FIFO);
#endif

#if defined A7511||A7512||A7611||A7612||A7711||A7712||A7001
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

	Log(USR_OK, "Ip_Addr -> [%s]", ip_addr);

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
	int			s_k, rt;

/* *********************************** */
/* 시장별, TR별 종목 일련번호 갖고오기 */
/* *********************************** */
#if defined A7501
	if ((memcmp (p_buf, "A0011", 5) != 0)	&&
		(memcmp (p_buf, "A1011", 5) != 0)	&&
		(memcmp (p_buf, "A1041", 5) != 0))
	{
		Che_Gbn = -1;
		return;
	}
#elif defined A7601
	if (memcmp (p_buf, "A0012", 5) != 0)
	{
		Che_Gbn = -1;
		return;
	}
/* ******** */
/* 체결처리 */
/* ******** */
#elif defined A7511||A7512
	/* 수신시세 => Memory 시세에 Set */
	if (memcmp (p_buf, "A3011", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Stock[0].A3.seq_no));
		Shm_Stock[Idx].HogaLastGbn = 0;
		memcpy (Shm_Stock[Idx].A3.tr_gbn, p_buf, sizeof (STOCK_A3011));

		/* 공용 현재가 */
		Shm_Risk[0].S_Sise[5][Idx].crprc =
			(double)AtoIf(Shm_Stock[Idx].A3.crprc, sizeof(Shm_Stock[0].A3.crprc));
		/* 공용 매도1호가가격 */
		Shm_Risk[0].S_Sise[5][Idx].sell_1_price =
			(double)AtoIf(Shm_Stock[Idx].A3.sell_1_price, sizeof(Shm_Stock[0].A3.sell_1_price));
		/* 공용 매수1호가가격 */
		Shm_Risk[0].S_Sise[5][Idx].buy_1_price =
			(double)AtoIf(Shm_Stock[Idx].A3.buy_1_price, sizeof(Shm_Stock[0].A3.buy_1_price));
	}
	else if (memcmp (p_buf, "A4011", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Stock[0].A3.seq_no));
		memcpy (Shm_Stock[Idx].A4.tr_gbn, p_buf, sizeof (STOCK_A4011));
	}
	else if (memcmp (p_buf, "R8011", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Stock[0].A3.seq_no));
		memcpy (Shm_Stock[0].R8.tr_gbn, p_buf, sizeof (R8011));
	}
	else if (memcmp (p_buf, "M4011", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Stock[0].A3.seq_no));
		memcpy (Shm_Stock[0].M4.tr_gbn, p_buf, sizeof (M4011));
	}
	/* ETF BV,BW,L5 && ETN S1, S3 */
	else if ( memcmp (&p_buf[5], "999999999999", 12) != 0	&&
			  memcmp (&p_buf[5], "000000000000", 12) != 0	&&
			((memcmp (p_buf, "BV011", 5) == 0) ||
		 	 (memcmp (p_buf, "BW011", 5) == 0) ||
		 	 (memcmp (p_buf, "L5011", 5) == 0) ||
		 	 (memcmp (p_buf, "S1011", 5) == 0) ||
		 	 (memcmp (p_buf, "S3011", 5) == 0))	)
	{
		/* index search */
		memset (&Key, 0, sizeof (Key));
		sprintf (Key.expcode, "%-12.12s", &p_buf[5]);
		rt = 0;
		rt = Key_Search (MK_KOSPI, KEY_EXPCODE, (char*)&Key);
		if (rt > 0)
		{
			Idx = rt;
			if (memcmp (p_buf, "BV011", 5) == 0) 
				memcpy (Shm_Stock[Idx].BV.tr_gbn, p_buf, sizeof (BV011));
			else
			if (memcmp (p_buf, "BW011", 5) == 0) 
				memcpy (Shm_Stock[Idx].BW.tr_gbn, p_buf, sizeof (BW011));
			else
			if (memcmp (p_buf, "L5011", 5) == 0) 
				memcpy (Shm_Stock[Idx].L5.tr_gbn, p_buf, sizeof (L5011));
			else
			if (memcmp (p_buf, "S1011", 5) == 0) 
			{
				memcpy (Shm_Stock[Idx].S1.tr_gbn, p_buf, sizeof (STOCK_S1011));
				Che_Gbn = 2;
			}
			else
			if (memcmp (p_buf, "S3011", 5) == 0)
				memcpy (Shm_Stock[Idx].S3.tr_gbn, p_buf, sizeof (STOCK_S3011));
		}
		else
		{
			SLog (USR_ERROR, "%5.5s Key Not Find [%12.12s]", p_buf, &p_buf[5]);
			Che_Gbn = -1;
		}
	}
	else
	{
		Che_Gbn = -1;
	}
#elif defined A7611||A7612
	/* 수신시세 => Memory 시세에 Set */
	if (memcmp (p_buf, "A3012", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Kosdaq[0].A3.seq_no));
		Shm_Kosdaq[Idx].HogaLastGbn = 0;
		memcpy (Shm_Kosdaq[Idx].A3.tr_gbn, p_buf, sizeof (STOCK_A3011));

		/* 공용 현재가 */
		Shm_Risk[0].S_Sise[6][Idx].crprc =
			(double)AtoIf(Shm_Kosdaq[Idx].A3.crprc, sizeof(Shm_Kosdaq[0].A3.crprc));
		/* 공용 매도1호가가격 */
		Shm_Risk[0].S_Sise[6][Idx].sell_1_price =
			(double)AtoIf(Shm_Kosdaq[Idx].A3.sell_1_price, sizeof(Shm_Kosdaq[0].A3.sell_1_price));
		/* 공용 매수1호가가격 */
		Shm_Risk[0].S_Sise[6][Idx].buy_1_price =
			(double)AtoIf(Shm_Kosdaq[Idx].A3.buy_1_price, sizeof(Shm_Kosdaq[0].A3.buy_1_price));
	}
	else if (memcmp (p_buf, "A4012", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Kosdaq[0].A3.seq_no));
		memcpy (Shm_Kosdaq[Idx].A4.tr_gbn, p_buf, sizeof (STOCK_A4011));
	}
	else if (memcmp (p_buf, "M4012", 5) == 0)
	{
		memcpy (Shm_Kosdaq[0].M4.tr_gbn, p_buf, sizeof (M4012));
	}
	else
	{
		Che_Gbn = -1;
	}
#elif defined A7711||A7712
/* ELW주문처리는 STOCK영역에서 처리한다. ELW영역은 기초자료 참조용으로만 사용 */
	/* 수신시세 => Memory 시세에 Set */
	if (memcmp (p_buf, "A3021", 5) == 0)
	{
		Che_Gbn = 1;
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Stock[0].A3.seq_no));
		Shm_Stock[Idx].HogaLastGbn = 0;
		memcpy (Shm_Stock[Idx].A3.tr_gbn, p_buf, sizeof (STOCK_A3011));

		/* 공용 현재가 */
		Shm_Risk[0].S_Sise[5][Idx].crprc =
			(double)AtoIf(Shm_Stock[Idx].A3.crprc, sizeof(Shm_Stock[0].A3.crprc));
		/* 공용 매도1호가가격 */
		Shm_Risk[0].S_Sise[5][Idx].sell_1_price =
			(double)AtoIf(Shm_Stock[Idx].A3.sell_1_price, sizeof(Shm_Stock[0].A3.sell_1_price));
		/* 공용 매수1호가가격 */
		Shm_Risk[0].S_Sise[5][Idx].buy_1_price =
			(double)AtoIf(Shm_Stock[Idx].A3.buy_1_price, sizeof(Shm_Stock[0].A3.buy_1_price));
	}
	else
	{
		Che_Gbn = -1;
	}
/* **** */
/* 호가 */
/* **** */
#elif defined A7521||A7522
	/* KOSPI 수신시세 => Memory 시세에 Set */
	if (memcmp (p_buf, "B6011", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Stock[0].B6.seq_no));
		Shm_Stock[Idx].HogaLastGbn = 1;
		memcpy (Shm_Stock[Idx].B6.tr_gbn, p_buf, sizeof (STOCK_B6011));

		/* 공용 매도1호가가격 */
		Shm_Risk[0].S_Sise[5][Idx].sell_1_price =
			(double)AtoIf(Shm_Stock[Idx].B6.hoga[0].sell_price, sizeof(Shm_Stock[0].B6.hoga[0].sell_price));
		/* 공용 매수1호가가격 */
		Shm_Risk[0].S_Sise[5][Idx].buy_1_price =
			(double)AtoIf(Shm_Stock[Idx].B6.hoga[0].buy_price, sizeof(Shm_Stock[0].B6.hoga[0].buy_price));
	}
	else if (memcmp (p_buf, "B7011", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Stock[0].B7.seq_no));
		Shm_Stock[Idx].HogaLastGbn = 1;
		memcpy (Shm_Stock[Idx].B7.tr_gbn, p_buf, sizeof (STOCK_B7011));

		/* 공용 매도1호가가격 */
		Shm_Risk[0].S_Sise[5][Idx].sell_1_price =
			(double)AtoIf(Shm_Stock[Idx].B7.hoga[0].sell_price, sizeof(Shm_Stock[0].B7.hoga[0].sell_price));
		/* 공용 매수1호가가격 */
		Shm_Risk[0].S_Sise[5][Idx].buy_1_price =
			(double)AtoIf(Shm_Stock[Idx].B7.hoga[0].buy_price, sizeof(Shm_Stock[0].B7.hoga[0].buy_price));
	}
	else if (memcmp (p_buf, "B8011", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Stock[0].B8.seq_no));
		memcpy (Shm_Stock[Idx].B8.tr_gbn, p_buf, sizeof (STOCK_B8011));
	}
	else
	{
		Che_Gbn = -1;
	}
#elif defined A7621||A7622
	/* KOSDAQ 수신시세 => Memory 시세에 Set */
	if (memcmp (p_buf, "B6012", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Kosdaq[0].B6.seq_no));
		Shm_Kosdaq[Idx].HogaLastGbn = 1;
		memcpy (Shm_Kosdaq[Idx].B6.tr_gbn, p_buf, sizeof (STOCK_B6011));

		/* 공용 매도1호가가격 */
		Shm_Risk[0].S_Sise[6][Idx].sell_1_price =
			(double)AtoIf(Shm_Kosdaq[Idx].B6.hoga[0].sell_price, sizeof(Shm_Kosdaq[0].B6.hoga[0].sell_price));
		/* 공용 매수1호가가격 */
		Shm_Risk[0].S_Sise[6][Idx].buy_1_price =
			(double)AtoIf(Shm_Kosdaq[Idx].B6.hoga[0].buy_price, sizeof(Shm_Kosdaq[0].B6.hoga[0].buy_price));
	}
	else if (memcmp (p_buf, "B8012", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Kosdaq[0].B8.seq_no));
		memcpy (Shm_Kosdaq[Idx].B8.tr_gbn, p_buf, sizeof (STOCK_B8011));
	}
	else
	{
		Che_Gbn = -1;
	}
#elif defined A7721||A7722
	/* ELW 수신시세 => Memory 시세에 Set */
	if (memcmp (p_buf, "B7021", 5) == 0)
	{
		Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_Stock[0].B7.seq_no));
		Shm_Stock[Idx].HogaLastGbn = 1;
		memcpy (Shm_Stock[Idx].B7.tr_gbn, p_buf, sizeof (STOCK_B7021));

		/* 공용 매도1호가가격 */
		Shm_Risk[0].S_Sise[5][Idx].sell_1_price =
			(double)AtoIf(Shm_Stock[Idx].B7.hoga[0].sell_price, sizeof(Shm_Stock[0].B7.hoga[0].sell_price));
		/* 공용 매수1호가가격 */
		Shm_Risk[0].S_Sise[5][Idx].buy_1_price =
			(double)AtoIf(Shm_Stock[Idx].B7.hoga[0].buy_price, sizeof(Shm_Stock[0].B7.hoga[0].buy_price));
	}
	else
	{
		Che_Gbn = -1;
	}
#elif defined A7001
	if ((memcmp (p_buf, "D0011", 5) == 0)	||
		(memcmp (p_buf, "D1011", 5) == 0)	||
		(memcmp (p_buf, "D2011", 5) == 0)	||
		(memcmp (p_buf, "D3011", 5) == 0)	||
		(memcmp (p_buf, "T9012", 5) == 0)	||
		(memcmp (p_buf, "AE164", 5) == 0)	||
		(memcmp (p_buf, "AA011", 5) == 0)	)
	{
		/* index search */
		memset (&Key, 0, sizeof (Key));
		sprintf (Key.expcode, "%-8.8s", p_buf);
		rt = 0;
		rt = Key_Search (MK_JISU, KEY_EXPCODE, (char *)&Key);
		if (rt >= 0)
			Idx = rt;
		else
		{
			Idx = Shm_Jisu[0].total_item_cnt;

			Shm_Item[0].J_Key[Idx].idx = Idx;
			memset (Shm_Item[0].J_Key[Idx].expcode, 0, 12);
			memcpy (Shm_Item[0].J_Key[Idx].expcode, p_buf, 8);
			Shm_Jisu[0].total_item_cnt++;
			qsort (Shm_Item[0].J_Key, Shm_Jisu[0].total_item_cnt, sizeof (KS_EXPCODE), CmpExpcode);
		}
		memcpy (Shm_Jisu[Idx].Data.tr_gbn, p_buf, sizeof (STOCK_JISU));

		if (memcmp (p_buf, "AA011", 5) == 0)
			Che_Gbn = 1;
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
	End of program (pa_7500_ur.c)
*************************************************************************/ 

