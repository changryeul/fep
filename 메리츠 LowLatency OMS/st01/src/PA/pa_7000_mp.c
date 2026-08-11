#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 시세Real & Udp 송신
#			  모든 시세처리함 (지수선물/Call옵션/Put옵션)
#	File	: pa_7000_mp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#define     DATA_SIZE       300
#include    "buf_struct.h"
/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/

#define     S_H_SIZE        17                      /* TR(5) + Code(12) */
/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int			Sise_Gbn, sleep_speed;
int			Sockfd[4][4], id = -1, Max_IP_Addr;
char		Svr_IP[20*4];
struct		sockaddr_in Svr_Addr[4][4], Clnt_Addr[4][4];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_7000_MP (int, char **);
void	Set_Curr (char *);
void	Set_Sise (char *);
void	Microsec_Sleep (int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_7000_MP (argc, argv);
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_7000_MP (int argc, char **argv)
/*----------------------------------------------------------------------*/
{
	int		i, idx, rt, len, R_Cnt, flag, start_flag, sltime, sd_size;
	int		FIFO_fd, FIFO_fd1;
	char    fifo_name[100], fifo_name1[100], tmp[128], bumun[4];
	char 	r_buf[1024];
	double	recv_sec, befo_sec;
	FILE_BUFF_FORMAT    R_Fmt[1];

	/* open FIFO    */
	sprintf (bumun, "%s", _SubSystem_Name);
	LtoU (bumun, 2);

	if ( (memcmp ((char *)getenv ("host_name"), "ap54", 4) == 0) ||
		 (memcmp ((char *)getenv ("host_name"), "ap64", 4) == 0) )
	{
		sprintf (fifo_name, "%s/PA/pa_7102_ur1", _FEP_FIFO);
		sprintf (fifo_name1, "%s/PA/pa_7112_ur1", _FEP_FIFO);
	}
	else if (memcmp ((char *)getenv ("host_name"), "at05", 4) == 0)
	{
		sprintf (fifo_name, "%s/PA/pa_7102_ur1", _FEP_FIFO);
		sprintf (fifo_name1, "%s/PB/pb_7102_ur1", _FEP_FIFO);
	}	
	else									/* ap67, at05, at07 */
	{
		sprintf (fifo_name, "%s/PB/pb_7102_ur1", _FEP_FIFO);
		sprintf (fifo_name1, "%s/PB/pb_7112_ur1", _FEP_FIFO);
	}	

	/* A3,G7(C0) FIFO */
	FIFO_fd = open (fifo_name, O_RDWR|O_NDELAY);

	if (FIFO_fd < 0)
		Log (FIF_FATAL, "cannot open FIFO[%d][%d:%s][%s]",
			FIFO_fd, SYS_NO, SYS_STR, fifo_name);

	/* B6(H0) FIFO */
	FIFO_fd1 = open (fifo_name1, O_RDWR|O_NDELAY);

	if (FIFO_fd1 < 0)
		Log (FIF_FATAL, "cannot open FIFO1[%d][%d:%s][%s]",
			FIFO_fd1, SYS_NO, SYS_STR, fifo_name1);

	if (argc > 2)      /* TEST */
	{
						sleep_speed = AtoIf(argv[1], strlen(argv[1]));
/* 3명까지는 유니캐스트 처리방식 가능 */
		if (argc > 5)   Max_IP_Addr = 1;
		else            Max_IP_Addr = argc - 2;
	}
	else if (argc == 1)
	{
						Max_IP_Addr = 1;
						sleep_speed = 1;
	}
	else if (argc == 2)
	{
						Max_IP_Addr = 1;
						sleep_speed = AtoIf(argv[1], strlen(argv[1]));
	}

	if (sleep_speed == 0)
		sleep_speed = 1;

	for (i=0; i < Max_IP_Addr; i++)
	{
		Sockfd[i][0] = -1;
		Sockfd[i][1] = -1;
		Sockfd[i][2] = -1;
		/* 최고 3개IP 까지 처리가능 */
		bzero ((unsigned char *)Clnt_Addr, sizeof (struct sockaddr_in) * 16);
		bzero ((unsigned char *)Svr_Addr, sizeof (struct sockaddr_in) * 16);

		if (argc > 2)
			sprintf (&Svr_IP[20*i], "%s", argv[2+i]);
		else
			sprintf (Svr_IP, "172.21.101.255");
	}

	rt = Socket_Connect ();

	start_flag = 0;

	while (START_S != JOB_END)
	{
		Stat_Save ();

		while (WRITE_CNT > READ_CNT)
		{
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

			/* 3개의 Udp Port로 쏴야하기 때문에 어떤 시장이의 Data인지 확인 */
			/* 선물(1), Call(2), Put(3)	*/
			if (memcmp (&R_Fmt[0].Data[2], "01", 2) == 0)
			{
				Sise_Gbn = 1;
				if (memcmp (R_Fmt[0].Data, "B6", 2) == 0)
					sd_size = sizeof(SIF_B6);
				else if (memcmp (R_Fmt[0].Data, "G7", 2) == 0)
					sd_size = sizeof(SIF_G7);
				else if (memcmp (R_Fmt[0].Data, "A3", 2) == 0)
					sd_size = sizeof(SIF_A3);

#if defined A7001

				if ( (sd_size != sizeof(SIF_B6)) &&
					 (memcmp (&R_Fmt[0].Data[S_H_SIZE], "01", 2) == 0)	)
				{
					Set_Curr (R_Fmt[0].Data);

					/* 자동주문에 시세인지 */
					rt = write (FIFO_fd, "1", 1);
					if (rt < 0)
					Log (FIF_FATAL, "cannot write FIFO[%d][%d:%s]",
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

/* Test */
					/* 자동주문에 시세인지 */
					rt = write (FIFO_fd1, "1", 1);
					if (rt < 0)
					Log (FIF_FATAL, "cannot write FIFO1[%d][%d:%s]",
						FIFO_fd1, SYS_NO, SYS_STR);

					while (1)
					{
						rt = read (FIFO_fd1, tmp, sizeof(tmp));
#if defined __linux
						if (rt == 0 || errno == EAGAIN)
#else
						if (rt == 0)
#endif
							break;
					}
				}
/*
				else if ( (sd_size == sizeof(SIF_B6)) &&
					 (memcmp (&R_Fmt[0].Data[S_H_SIZE], "01", 2) == 0)	)
				{
					Set_Curr (R_Fmt[0].Data);
*/

					/* 자동주문에 시세인지 */
/*
					rt = write (FIFO_fd1, "1", 1);
					if (rt < 0)
					Log (FIF_FATAL, "cannot write FIFO1[%d][%d:%s]",
						FIFO_fd1, SYS_NO, SYS_STR);

					while (1)
					{
#if defined __linux
						if (rt == 0 || errno == EAGAIN)
#else
						if (rt == 0)
#endif
						if (rt == 0)
							break;
					}
				}
*/
#endif
			}
			else if ( (memcmp (&R_Fmt[0].Data[2], "03", 2) == 0) &&
					  (memcmp (&R_Fmt[0].Data[2+6], "2", 1) == 0) )
			{
				Sise_Gbn = 2;
				if (memcmp (R_Fmt[0].Data, "B6", 2) == 0)
					sd_size = sizeof(SIO_B6);
				else if (memcmp (R_Fmt[0].Data, "G7", 2) == 0)
					sd_size = sizeof(SIO_G7);
				else if (memcmp (R_Fmt[0].Data, "A3", 2) == 0)
					sd_size = sizeof(SIO_A3);
/* 자동주문 로직 검증시 필요하나 시세 확인용으로만 개발 */
#if defined A7001
			Set_Curr (R_Fmt[0].Data);
#endif
			}
			else if ( (memcmp (&R_Fmt[0].Data[2], "03", 2) == 0) &&
					  (memcmp (&R_Fmt[0].Data[2+6], "3", 1) == 0) )
			{
				Sise_Gbn = 3;
				if (memcmp (R_Fmt[0].Data, "B6", 2) == 0)
					sd_size = sizeof(SIO_B6);
				else if (memcmp (R_Fmt[0].Data, "G7", 2) == 0)
					sd_size = sizeof(SIO_G7);
				else if (memcmp (R_Fmt[0].Data, "A3", 2) == 0)
					sd_size = sizeof(SIO_A3);
/* 자동주문 로직 검증시 필요하나 시세 확인용으로만 개발 */
#if defined A7001
			Set_Curr (R_Fmt[0].Data);
#endif
			}
			else
			{
				READ_CNT++;
				continue;
			}

/* 1초 차이 넘는 시세는 없다 치고	*/
			if (start_flag == 0)
			{
				start_flag = 1;
				recv_sec = AtoIf (R_Fmt[0].RecvTime2,   2) * 60 * 60
					 	 + AtoIf (R_Fmt[0].RecvTime2+2, 2) * 60
					 	 + AtoIf (R_Fmt[0].RecvTime2+4, 2)
					 	 + AtoIf (R_Fmt[0].RecvTime2+6, 6) / 1000000.0;
			}
			befo_sec = recv_sec;

			recv_sec = AtoIf (R_Fmt[0].RecvTime2,   2) * 60 * 60
					 + AtoIf (R_Fmt[0].RecvTime2+2, 2) * 60
					 + AtoIf (R_Fmt[0].RecvTime2+4, 2)
					 + AtoIf (R_Fmt[0].RecvTime2+6, 6) / 1000000.0;

			READ_CNT++;
			
			sltime = (recv_sec - befo_sec) * 1000000.0;
#if (0)
/* Test Log	*/
Log (USR_OK, "sleep [%d] sleep_speed [%d]", sltime, sleep_speed);
#endif

			/* 100 Microsec 미만은 sleep 안하고 처리함 */
			if ( 1.0 <= (recv_sec - befo_sec) )
			{
				sltime = (recv_sec - befo_sec) / 1;
				if (sleep_speed < 0)
					sleep (sltime * sleep_speed);
				else
					sleep (sltime / sleep_speed);
			}
			else if (10000 <= sltime)
			{
				if (sleep_speed < 0)
					usleep ((sltime - 100) * sleep_speed);
				else
					usleep ((sltime - 100) / sleep_speed);
			}
			else if (100 <= sltime)
			{
				if (sleep_speed < 0)
					Microsec_Sleep ((sltime - 100) * sleep_speed);
				else
					Microsec_Sleep ((sltime - 100) / sleep_speed);
			}

			Set_Sise (R_Fmt[0].Data);

			for (i=0; i<Max_IP_Addr; i++)
			{

				len = sizeof (Svr_Addr[i][Sise_Gbn - 1]);
				rt = sendto (Sockfd[i][Sise_Gbn - 1], R_Fmt[0].Data, sd_size,
						0, (struct sockaddr *)&Svr_Addr[i][Sise_Gbn - 1], len);
				if (rt < 0)
				{
					Log (UDP_FATAL, "Data Send Error [%d:%s] Sockfd [%d]", SYS_NO, SYS_STR, Sockfd[i][Sise_Gbn - 1]);
					break;
				}

#if (0)
/* Test Log	*/
			Log (USR_OK, "UDP S [%d][%.10s](%d)",
					Sise_Gbn - 1,  R_Fmt[0].Data, strlen (R_Fmt[0].Data));
#endif
    		}
		}

		sleep(10);
	}

	return;
}	/* End of PA_7000_MP ()	*/

/*----------------------------------------------------------------------*/
int     Socket_Connect (void)
/*----------------------------------------------------------------------*/
{
	int     rt, i=0, j=0, bufflen, oplen;

	bufflen = 1024 * 64;										/* 64k	*/
	oplen   = sizeof(bufflen);

	for (i = 0; i < Max_IP_Addr; i++)
	{
		for (j = 0; j < 3; j++)				/* 0:선물, 1:Call, 2:Put	*/
		{
			Sockfd[i][j] = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (Sockfd[i][j] < 0)
			{
				Log (UDP_FATAL, "Socket Open Error[%d:%s]", SYS_NO, SYS_STR);
				return 0;
			}
			Svr_Addr[i][j].sin_family         = AF_INET;
/*
            Svr_Addr[i][j].sin_addr.s_addr    = inet_addr (&Svr_IP[20*i]);
*/
            inet_pton(AF_INET, &Svr_IP[20*i], &Svr_Addr[i][j].sin_addr.s_addr);
#if defined A7001 || A7002 || A7003
			Svr_Addr[i][j].sin_port           = htons (60631+j);
#endif
#if defined A7004
			if (j == 0)
				Svr_Addr[i][j].sin_port           = htons (5572);
			else if (j == 1)
				Svr_Addr[i][j].sin_port           = htons (5515);
			else if (j == 2)
				Svr_Addr[i][j].sin_port           = htons (5516);
#endif

			Log (UDP_OK, "socket %d created [%s:%d]", i, &Svr_IP[20*i], 60631+j);

			Clnt_Addr[i][j].sin_family       = AF_INET;
			Clnt_Addr[i][j].sin_addr.s_addr  = htonl (INADDR_ANY);
			Clnt_Addr[i][j].sin_port         = htons (0);

			setsockopt (Sockfd[i][j],SOL_SOCKET,SO_SNDBUF, (void *)&bufflen, oplen);
			setsockopt (Sockfd[i][j],SOL_SOCKET,SO_RCVBUF, (void *)&bufflen, oplen);

#if (1)
			setsockopt (Sockfd[i][j],SOL_SOCKET,SO_BROADCAST, (void *)&bufflen, oplen);
#endif
			rt = bind (Sockfd[i][j], (struct sockaddr *)&Clnt_Addr[i][j],
					sizeof (Clnt_Addr[i][j]));
			if (rt < 0)
			{
				Log (UDP_FATAL, "Call옵션 Socket Bind Error[%d:%s]", SYS_NO, SYS_STR);
				return (NOTOK);
			}
		}
	}

	return 1;
}

/*************************************************************************
	Function		: . Set_Curr
	Parameters IN	: . p_buf	: received data
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . set sise data SHM (현재가)
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Set_Curr (char *p_buf)
/*----------------------------------------------------------------------*/
{
	int			i;

	if (Sise_Gbn == 1)
	{
		/* Seq  */
		i = AtoIf (&p_buf[S_H_SIZE], 2);
		i --;

		Shm_Futures[i].Befor_CURR_Arry_Key = Shm_Futures[i].CURR_Arry_Key;

		if ( (Shm_Futures[i].CURR_Arry_Key <   0) ||
			 (Shm_Futures[i].CURR_Arry_Key >= 29)   )
			Shm_Futures[i].CURR_Arry_Key = 0;
		else
			Shm_Futures[i].CURR_Arry_Key ++;

		/* Read Data => Curr	*/
		if (memcmp (p_buf, "G7014", 5) == 0)				/* 호가 + 체결	*/
		{
			/* 선물 체결 Arry Key Settin    */
			Shm_Futures[i].Befor_CHE_Arry_Key = Shm_Futures[i].CHE_Arry_Key;

			if ( (Shm_Futures[i].CHE_Arry_Key <   0) ||
				(Shm_Futures[i].CHE_Arry_Key >= 999)   )
			{
				Shm_Futures[i].CHE_Cnt = Shm_Futures[i].CHE_Cnt + 1;
				Shm_Futures[i].CHE_Arry_Key = 0;
			}
			else
				Shm_Futures[i].CHE_Arry_Key ++;

			/* CHE => CHE_Arry(호가접수시 8자리는 뺀다) */
			memcpy (Shm_Futures[i].Futures_CHE_Arry[Shm_Futures[i].CHE_Arry_Key].tr_gbn,
				p_buf, sizeof (SIF_G7) - 8);

			memcpy (Shm_Futures[i].Futures_CURR.tr_gbn, p_buf, sizeof (SIF_G7) - 8);
		}
		else if (memcmp (p_buf, "A3014", 5) == 0)                   /* 체결 */
		{

			/* 선물 체결 Arry Key Settin    */
			Shm_Futures[i].Befor_CHE_Arry_Key = Shm_Futures[i].CHE_Arry_Key;

			if ( (Shm_Futures[i].CHE_Arry_Key <   0) ||
				(Shm_Futures[i].CHE_Arry_Key >= 999)   )
			{
				Shm_Futures[i].CHE_Cnt = Shm_Futures[i].CHE_Cnt + 1;
				Shm_Futures[i].CHE_Arry_Key = 0;
			}
			else
				Shm_Futures[i].CHE_Arry_Key ++;

			/* CHE => CHE_Arry(직전의 G7으로 초기화 작업후 A3를 UpDate한다)  */
			memcpy (Shm_Futures[i].Futures_CHE_Arry[Shm_Futures[i].CHE_Arry_Key].tr_gbn,
				Shm_Futures[i].Futures_G7.tr_gbn, sizeof (SIF_G7) - 8);
			memcpy (Shm_Futures[i].Futures_CHE_Arry[Shm_Futures[i].CHE_Arry_Key].tr_gbn,
				p_buf, sizeof (SIF_A3));

			memcpy (Shm_Futures[i].Futures_CURR.tr_gbn, p_buf, sizeof (SIF_A3));
		}
		else if (memcmp (p_buf, "B6014", 5) == 0)                   /* 호가 */
		{
			memcpy (Shm_Futures[i].Futures_CURR.tr_gbn,
				p_buf, sizeof (Shm_Futures[0].Futures_CURR.tr_gbn));
			memcpy (Shm_Futures[i].Futures_CURR.market_state_gubun,
				&p_buf[S_H_SIZE+2], sizeof (SIF_B6) - S_H_SIZE + 2);
		}
		else
		{
			SLog (USR_OK, "SISE SKIP [%10.10s]", p_buf);
			return;
		}

		/* Curr => Curr_Arry    */
		memcpy (Shm_Futures[i].Futures_CURR_Arry[Shm_Futures[i].CURR_Arry_Key].tr_gbn,
			Shm_Futures[i].Futures_CURR.tr_gbn, sizeof (SIF_G7));
	}
	else
	{
		/* Seq  */
		i = AtoIf (&p_buf[S_H_SIZE], 3);
		i --;

		/* C0를 CURR Arry로 Setting Start	*/
       	Shm_Options[i].Befor_CURR_Arry_Key = Shm_Options[i].CURR_Arry_Key;

       	if ( (Shm_Options[i].CURR_Arry_Key <   0) ||
           	 (Shm_Options[i].CURR_Arry_Key >= 29)   )
           	Shm_Options[i].CURR_Arry_Key = 0;
       	else
           	Shm_Options[i].CURR_Arry_Key ++;

		/* Recv Data => Curr    */
		if (memcmp (p_buf, "G7034", 5) == 0)                /* 호가 + 체결  */
		{
			/* 옵션 체결 Arry Key Settin    */
			Shm_Options[i].Befor_CHE_Arry_Key = Shm_Options[i].CHE_Arry_Key;

			if ( (Shm_Options[i].CHE_Arry_Key <   0) ||
				(Shm_Options[i].CHE_Arry_Key >= 999)   )
			{
				Shm_Options[i].CHE_Cnt = Shm_Options[i].CHE_Cnt + 1;
				Shm_Options[i].CHE_Arry_Key = 0;
			}
			else
				Shm_Options[i].CHE_Arry_Key ++;

			/* CHE => CHE_Arry  */
			memcpy (Shm_Options[i].Options_CHE_Arry[Shm_Options[i].CHE_Arry_Key].tr_gbn,
				p_buf, sizeof (SIO_G7) - 8);

			memcpy (Shm_Options[i].Options_CURR.tr_gbn, p_buf, sizeof (SIO_G7) - 8);
		}
		else if (memcmp (p_buf, "A3034", 5) == 0)                   /* 체결 */
		{
			/* 옵션 체결 Arry Key Settin    */
			Shm_Options[i].Befor_CHE_Arry_Key = Shm_Options[i].CHE_Arry_Key;

			if ( (Shm_Options[i].CHE_Arry_Key <   0) ||
				(Shm_Options[i].CHE_Arry_Key >= 999)   )
			{
				Shm_Options[i].CHE_Cnt = Shm_Options[i].CHE_Cnt + 1;
				Shm_Options[i].CHE_Arry_Key = 0;
			}
			else
				Shm_Options[i].CHE_Arry_Key ++;

			/* CHE => CHE_Arry(직전의 G7으로 초기화 작업후 A3를 UpDate한다)  */
			memcpy (Shm_Options[i].Options_CHE_Arry[Shm_Options[i].CHE_Arry_Key].tr_gbn,
				Shm_Options[i].Options_G7.tr_gbn, sizeof (SIO_G7) - 8);
			memcpy (Shm_Options[i].Options_CHE_Arry[Shm_Options[i].CHE_Arry_Key].tr_gbn,
				p_buf, sizeof (SIO_A3));

			memcpy (Shm_Options[i].Options_CURR.tr_gbn, p_buf, sizeof (SIO_A3));
		}
		else if (memcmp (p_buf, "B6034", 5) == 0)                   /* 호가 */
		{
			memcpy (Shm_Options[i].Options_CURR.market_state_gubun,
				&p_buf[S_H_SIZE+3], sizeof (SIO_B6) - S_H_SIZE + 3);
		}
		else
		{
			SLog (USR_OK, "SISE SKIP [%10.10s]", p_buf);
			return;
		}

		/* Curr => Curr_Arry    */
		memcpy (&Shm_Options[i].Options_CURR_Arry[Shm_Options[i].CURR_Arry_Key],
			Shm_Options[i].Options_CURR.tr_gbn, sizeof (SIO_G7));
	}

	return;
}	/* End of Set_Curr ()	*/

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
	int			i;

	if (Sise_Gbn == 1)								/* 지수선물시세 */
	{

		i = AtoIf (&p_buf[S_H_SIZE], 2);
		i --;

		if (memcmp (p_buf, "B6014", 5) == 0)                        /* 호가 */
		{
			memcpy (Shm_Futures[i].Futures_B6.tr_gbn, p_buf, sizeof (SIF_B6));
		}
		else if (memcmp (p_buf, "A3014", 5) == 0)                   /* 체결 */
		{
			memcpy (Shm_Futures[i].Futures_A3.tr_gbn, p_buf, sizeof (SIF_A3));
		}
		else if (memcmp (p_buf, "G7014", 5) == 0)           /* 체결 + 호가 */
		{
			memcpy (Shm_Futures[i].Futures_G7.tr_gbn, p_buf, sizeof (SIF_G7) - 8);
		}

	}
	else												/* 지수옵션시세 */
	{

		i = AtoIf (&p_buf[S_H_SIZE], 3);
		i --;

		if (memcmp (p_buf, "B6034", 5) == 0)                        /* 호가 */
		{
			memcpy (&Shm_Options[i].Options_B6, p_buf, sizeof (SIO_B6));
		}
		else if (memcmp (p_buf, "A3034", 5) == 0)                   /* 체결 */
		{
			memcpy (&Shm_Options[i].Options_A3, p_buf, sizeof (SIO_A3));
		}
		else if (memcmp (p_buf, "G7034", 5) == 0)           /* 체결 + 호가 */
		{
			memcpy (&Shm_Options[i].Options_G7, p_buf, sizeof (SIO_G7) - 8);
		}

	}

	return;
}	/* End of Set_Sise ()	*/


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
	End of program (pa_7000_mp.c)
*************************************************************************/ 

