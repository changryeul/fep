#define _GLOBAL
/*------------------------------------------------------------------------
#	Module	:	시세 송신 (UDP)
#	File	:	pa_7030_us.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

#define     DATA_SIZE       790
#include    "buf_struct.h"
/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     ITEM_CODE       INT_SEQ

/*-------------------------------------------------------------------------
   Program Define Constants
-------------------------------------------------------------------------*/
#define     MAX_BUF_SIZE    83000
#define     DATA_TIME       60 * 1000

/*-------------------------------------------------------------------------
   Program Global Variable
-------------------------------------------------------------------------*/
int     Sockfd[4][4], id = -1;
struct	sockaddr_in Svr_Addr[4][4], Clnt_Addr[4][4];

/*-------------------------------------------------------------------------
   Function Declarations.
-------------------------------------------------------------------------*/
void	PA_7030_US (void);
void	Microsec_Sleep (int);

/*-----------------------------------------------------------------------*/
int     main (int argc, char *argv[])
/*-----------------------------------------------------------------------*/
{
    Init_Proc (argc, argv);

    PA_7030_US ();

    Exit_Process();
}   /* end of main() */

/*-----------------------------------------------------------------------*/
void    PA_7030_US (void)
/*-----------------------------------------------------------------------*/
{
	int			i, rt, add_len, R_Cnt, Sise_Gbn, sd_size, len, sleep_speed;
	int			start_flag, sltime; 
	double		recv_sec, befo_sec;
	char		tmp[4], bumun[4], send_data[1000];

	FILE_BUFF_FORMAT    R_Fmt[1];

	bzero ((unsigned char *)Svr_Addr,  sizeof (struct sockaddr_in) * 3);
	bzero ((unsigned char *)Clnt_Addr, sizeof (struct sockaddr_in) * 3);

	for (i = 0; i < 4; i++)
	{
		Sockfd[i][0] = -1;
		Sockfd[i][1] = -1;
		Sockfd[i][2] = -1;
		Sockfd[i][3] = -1;
	}

   	rt = Socket_Connect ();
	if (rt == NOTOK)
		return;

	start_flag = 0;

	if (DELAY_TIME == 0)
		sleep_speed = 1;
	else
		sleep_speed = DELAY_TIME;

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

			/* 4가지 종류의 데이터를 송신해야하기 때문에 데이터 종류에 따라 처리(G7014, B6014, A3014, B7021, A3021) */
			/* G7(1), B6(2), A3(3), B7(4), A3(5) */
            if (memcmp (R_Fmt[0].Data, "G7014", 5) == 0)
            {
				Sise_Gbn = 1;
				sd_size = sizeof(SIF_G7);
			}
            else if (memcmp (R_Fmt[0].Data, "B6014", 5) == 0)
            {
				Sise_Gbn = 1;
				sd_size = sizeof(SIF_B6);
			}
            else if (memcmp (R_Fmt[0].Data, "A3014", 5) == 0)
            {
				Sise_Gbn = 1;
				sd_size = sizeof(SIF_A3);
			}
            else if (memcmp (R_Fmt[0].Data, "B7021", 5) == 0)
            {
				Sise_Gbn = 2;
				sd_size = sizeof(STOCK_B7);
			}
            else if (memcmp (R_Fmt[0].Data, "A3021", 5) == 0)
            {
				Sise_Gbn = 3;
				sd_size = sizeof(STOCK_A3);
			}

/* 1초 차이 넘는 시세는 없다 치고   */
			recv_sec = AtoIf (R_Fmt[0].RecvTime2,   2) * 60 * 60
                     + AtoIf (R_Fmt[0].RecvTime2+2, 2) * 60
                     + AtoIf (R_Fmt[0].RecvTime2+4, 2)
                     + AtoIf (R_Fmt[0].RecvTime2+6, 6) / 1000000.0;

            if (start_flag == 0)
            {
                start_flag = 1;
				befo_sec = recv_sec;
            }

            READ_CNT++;

			if (befo_sec < recv_sec)
			{
            	sltime = (recv_sec - befo_sec) * 1000000.0;
#if (1)
/* Test Log */
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
	
				befo_sec = recv_sec;
			}

/*
            Set_Sise (R_Fmt[0].Data);
*/


            for (i = 0; i < 4; i++)
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
/* Test Log */
            Log (USR_OK, "UDP S [%d][%.10s](%d)",
                    Sise_Gbn - 1,  R_Fmt[0].Data, sd_size);
#endif
            }


		}

#if (0)
		Log (USR_OK, "UDP S [%.10s](%d)", send_data, strlen (send_data));
#endif
	}
}

/*----------------------------------------------------------------------*/
int     Socket_Connect (void)
/*----------------------------------------------------------------------*/
{
	int     rt, i, j, bufflen, oplen;
	int		in1, in2, in3, in4;
	char    Svr_IP[3];

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 3; j++)
		{
    	    in1 = in2 = in3 = in4 = 0;
	        memset (Svr_IP, 0x00, sizeof(Svr_IP));
	        in1 = AtoIf(ACCNO(D_K,i).ip_addr, 3);
	        in2 = AtoIf(&ACCNO(D_K,i).ip_addr[3], 3);
	        in3 = AtoIf(&ACCNO(D_K,i).ip_addr[6], 3);
	        in4 = AtoIf(&ACCNO(D_K,i).ip_addr[9], 3);

	        sprintf(Svr_IP, "%d.%d.%d.%d", in1, in2, in3, in4);

            Sockfd[i][j] = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (Sockfd[i][j] < 0)
            {
                Log (UDP_FATAL, "Socket Open Error[%d:%s]", SYS_NO, SYS_STR);
                return 0;
            }
            Svr_Addr[i][j].sin_family         = AF_INET;
           	Svr_Addr[i][j].sin_addr.s_addr    = inet_addr (Svr_IP);

            if (j == 0)
                Svr_Addr[i][j].sin_port           = htons (15572);
            else if (j == 1)
                Svr_Addr[i][j].sin_port           = htons (19621);
            else if (j == 2)
                Svr_Addr[i][j].sin_port           = htons (19616);

            Log (UDP_OK, "socket %d created [%s]", i, Svr_IP);

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
                Log (UDP_FATAL, "Call¿E¼C Socket Bind Error[%d:%s]", SYS_NO, SYS_STR);
                return (NOTOK);
            }
		}
	}

	return 1;
}

/*************************************************************************
	Function        : . Microsec_Sleep
	Parameters IN   : . msec    : sleep time (in microsec)
	Parameters OUT  : .
	Return Code     : . void
	Comment         : . sleep for microsec
*************************************************************************/
void    Microsec_Sleep (int msec)
{
	int             rt;
	struct timespec ts;

	ts.tv_sec = 0;
	ts.tv_nsec = msec * 1000;

	rt = nanosleep (&ts, NULL);

	if (rt == -1)
		Log (SYS_ERROR, "nanosleep fail {%d:%s}", SYS_NO, SYS_STR);

	return;
}   /* End of Microsec_Sleep () */

/**************************************************************************
    end of program(pa_7030_us.c)
**************************************************************************/
