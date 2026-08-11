#define _GLOBAL
/*------------------------------------------------------------------------
#	Module	:	시세 송신 (UDP)
#	File	:	pa_7010_us.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     SVR_PORT_NO     UDP_PORT(D_K,P_K,0)
#define     ITEM_CODE       INT_SEQ

/*-------------------------------------------------------------------------
   Program Define Constants
-------------------------------------------------------------------------*/
#define     MAX_BUF_SIZE    83000
#define     DATA_TIME       60 * 1000

/*-------------------------------------------------------------------------
   Program Global Variable
-------------------------------------------------------------------------*/
int     Sockfd[ACC_NO_CNT], id = -1;
struct  sockaddr_in Svr_Addr[ACC_NO_CNT], Clnt_Addr[ACC_NO_CNT];

/*-------------------------------------------------------------------------
   Function Declarations.
-------------------------------------------------------------------------*/
void    PA_7010_US (void);

/*-----------------------------------------------------------------------*/
int     main (int argc, char *argv[])
/*-----------------------------------------------------------------------*/
{
    Init_Proc (argc, argv);

    PA_7010_US ();

    Exit_Process();
}   /* end of main() */

/*-----------------------------------------------------------------------*/
void    PA_7010_US (void)
/*-----------------------------------------------------------------------*/
{
	int			i, rt, add_len, FIFO_fd;
	char		sub[4], tmp[4], bumun[4], fifo_name[256], send_data[1000];

	bzero ((unsigned char *)Svr_Addr,
		sizeof (struct sockaddr_in) * ACC_NO_CNT);
	bzero ((unsigned char *)Clnt_Addr,
		sizeof (struct sockaddr_in) * ACC_NO_CNT);

	Sockfd[0] = -1;

    sprintf (sub, "%2.2s", _Exe_Name);
    LtoU (sub, 2);
    sprintf (fifo_name, "%s/%s/%s", _FEP_FIFO, sub, FFN(D_K,P_K,0));

    FIFO_fd = open (fifo_name, O_RDWR|O_NDELAY);

    if (FIFO_fd < 0)
        Log (FIF_FATAL, "cannot open FIFO[%s] FIFO_fd[%d] {%d:%s}",
            fifo_name, FIFO_fd, SYS_NO, SYS_STR);

   	rt = Socket_Connect ();
	if (rt == NOTOK)
		return;

	Log (USR_OK, "socket connected:port[%d]", SVR_PORT_NO);

	while (START_S != JOB_END)
	{
		Stat_Save ();

		rt = Poll_File (DATA_TIME);

		memset(send_data, 0x00, sizeof(send_data));

		if (rt == 1)
		{
			Log (USR_OK, "poll timeout <%d>", INT_SEQ);
			continue;
		}
		else if (rt == -1)
			continue;
		else
		{
			while (1)
			{
				rt = read (FIFO_fd, tmp, 1);
				if (rt == 0)
					break;
			}

#if defined A7001											/* 지수선물시세 */
			if (memcmp (Shm_Futures[0].Futures_CURR_Arry[Shm_Futures[0].CURR_Arry_Key].tot_con_qty,
						Shm_Futures[0].Futures_CURR_Arry[Shm_Futures[0].Befor_CURR_Arry_Key].tot_con_qty,
						sizeof(Shm_Futures[0].Futures_CURR.tot_con_qty)) == 0)
			{
				memcpy(send_data, Shm_Futures[0].Futures_B6.tr_gbn, sizeof (SIF_B6));
			}
			else
			{
				memcpy(send_data, Shm_Futures[0].Futures_CURR.tr_gbn, sizeof (SIF_G7));
			}

#elif defined A7002 || A7003								/* 지수옵션시세 */
			if (memcmp (Shm_Options[ITEM_CODE].Options_CURR_Arry[Shm_Options[ITEM_CODE].CURR_Arry_Key].tot_con_qty,
						Shm_Options[ITEM_CODE].Options_CURR_Arry[Shm_Options[ITEM_CODE].Befor_CURR_Arry_Key].tot_con_qty,
				 sizeof(Shm_Options[ITEM_CODE].Options_CURR.tot_con_qty)) == 0)
			{
				memcpy(send_data, Shm_Options[0].Options_B6.tr_gbn, sizeof (SIO_B6));
			}
			else
			{
				memcpy(send_data, Shm_Options[0].Options_CURR.tr_gbn, sizeof (SIO_G7));
			}
#endif
		}

		add_len = sizeof (Svr_Addr[0]);

		for ( i = 0; i < ACC_NO_CNT; i++)
		{

#if (1)
			rt = sendto (Sockfd[i], send_data, strlen(send_data),
				0, (struct sockaddr *)&Svr_Addr[i], add_len);
			if (rt < 0)
			{
				Log (UDP_FATAL, "Data Send Error [%d:%s]", SYS_NO, SYS_STR);
				break;
			}
#endif
		}

#if (1)
		Log (USR_OK, "UDP S [%.10s](%d)", send_data, strlen (send_data));
#endif
	}
}

/*----------------------------------------------------------------------*/
int     Socket_Connect (void)
/*----------------------------------------------------------------------*/
{
	int     rt, i, bufflen, oplen;
	int		in1, in2, in3, in4;
	char    Svr_IP[20];

	for (i = 0; i < 4; i++)
	{
		in1 = in2 = in3 = in4 = 0;
		memset (Svr_IP, 0x00, sizeof(Svr_IP));
		in1 = AtoIf(ACCNO(D_K,i).ip_addr, 3);
		in2 = AtoIf(&ACCNO(D_K,i).ip_addr[3], 3);
		in3 = AtoIf(&ACCNO(D_K,i).ip_addr[6], 3);
		in4 = AtoIf(&ACCNO(D_K,i).ip_addr[9], 3);

		sprintf(Svr_IP, "%d.%d.%d.%d", in1, in2, in3, in4);

		Sockfd[i] = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (Sockfd[i] < 0)
		{
			Log (UDP_FATAL, "Socket Open Error[%d:%s]", SYS_NO, SYS_STR);
			return 0;
		}
		Svr_Addr[i].sin_family         = AF_INET;
		Svr_Addr[i].sin_addr.s_addr    = inet_addr (Svr_IP);
		Svr_Addr[i].sin_port           = htons (SVR_PORT_NO);

		Log (UDP_OK, "socket %d created [%s:%s:%d]", i, ACCNO(D_K,i).ip_addr,
                Svr_IP, SVR_PORT_NO);

		Clnt_Addr[i].sin_family       = AF_INET;
		Clnt_Addr[i].sin_addr.s_addr  = htonl (INADDR_ANY);
		Clnt_Addr[i].sin_port         = htons (0);

		bufflen = 1024 * 64;                                        /* 64K  */
		oplen   = sizeof(bufflen);
		setsockopt (Sockfd[i],SOL_SOCKET,SO_SNDBUF, (void *)&bufflen, oplen);

#if (1)
		bufflen = 1;
		oplen   = sizeof(bufflen);
		setsockopt (Sockfd[i],SOL_SOCKET,SO_BROADCAST, (void *)&bufflen, oplen);
#endif
		rt = bind (Sockfd[i], (struct sockaddr *)&Clnt_Addr[i],
                sizeof (Clnt_Addr[i]));
		if (rt < 0)
		{
            Log (UDP_FATAL, "Socket Bind Error[%d:%s]", SYS_NO, SYS_STR);
            return (NOTOK);
		}
	}

	return 1;
}

/**************************************************************************
    end of program(pa_7010_us.c)
**************************************************************************/
