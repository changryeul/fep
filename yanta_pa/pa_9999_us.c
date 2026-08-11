#define _GLOBAL
/*------------------------------------------------------------------------
#	Module	:	계좌손익정보송신 (UDP)
#	File	:	pa_9999_us.c
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


/*-------------------------------------------------------------------------
   Program Define Constants
-------------------------------------------------------------------------*/
#define     MAX_BUF_SIZE    83000
#define     DATA_TIME       5 * 1000

/*-------------------------------------------------------------------------
   Program Global Variable
-------------------------------------------------------------------------*/
#if defined A9999
#define     Acc_Cnt		ACC_NO_CNT
#define     Acc_No		0
#elif defined A9998
#define     Acc_Cnt		ELW_ACC_NO_CNT
#define     Acc_No		ACC_NO_CNT
#endif
int     Sockfd[Acc_Cnt], id = -1;
struct  sockaddr_in Svr_Addr[Acc_Cnt], Clnt_Addr[Acc_Cnt];
UDP_OUT_DATA			Wdata[Acc_Cnt];
UDP_OUT_HEADER			Wheader;

/*-------------------------------------------------------------------------
   Function Declarations.
-------------------------------------------------------------------------*/
void    PA_9999_US (void);

/*-----------------------------------------------------------------------*/
int     main (int argc, char *argv[])
/*-----------------------------------------------------------------------*/
{
    Init_Proc (argc, argv);

    PA_9999_US ();

    Exit_Process();
}   /* end of main() */

/*-----------------------------------------------------------------------*/
void    PA_9999_US (void)
/*-----------------------------------------------------------------------*/
{
	int			i, rt, add_len;
	char		send_data[1000], packet_size[5], data_size[4], data_cnt[3];

	bzero ((unsigned char *)Svr_Addr,
		sizeof (struct sockaddr_in) * Acc_Cnt);
	bzero ((unsigned char *)Clnt_Addr,
		sizeof (struct sockaddr_in) * Acc_Cnt);

	Sockfd[0] = -1;

   	rt = Socket_Connect ();
	if (rt == NOTOK)
		return;

	Log (USR_OK, "socket connected:port[%d]", SVR_PORT_NO);

	while (START_S != JOB_END)
	{
		Stat_Save ();

		rt = Poll_File (DATA_TIME);

		memset(&Wdata, 0x00, sizeof(UDP_OUT_DATA) * Acc_Cnt);
		memset(&Wheader, 0x00, sizeof(Wheader));
		memset(packet_size, 0x00, sizeof(packet_size));
		memset(data_size, 0x00, sizeof(data_size));
		memset(send_data, 0x00, sizeof(send_data));
		memset(data_cnt, 0x00, sizeof(data_cnt));

		if (rt == 1)
		{
			memcpy(send_data, "0010P00000",	sizeof(UDP_OUT_HEADER));
			Log (USR_OK, "poll timeout <%d>", INT_SEQ);
		}
		else if (rt == -1)
			continue;
		else
		{
			sprintf(packet_size, "%04d", 
				sizeof(UDP_OUT_HEADER) + Acc_Cnt * sizeof(UDP_OUT_DATA));
			sprintf(data_size, "%03d",  sizeof(UDP_OUT_DATA));
			sprintf(data_cnt, "%02d",  Acc_Cnt);

			/* Length 4 byte */
			memcpy(Wheader.Length, packet_size,	sizeof(Wheader.Length));

			/* Data 구분값 1 'D' */
			memcpy(Wheader.Dgbn, "D",	sizeof(Wheader.Dgbn));

			/* Data Size 정보 */
			memcpy(Wheader.Dlength, data_size,	sizeof(Wheader.Dlength));

			/* Data Cnt 정보 */
			memcpy(Wheader.Data_Cnt, data_cnt,	sizeof(Wheader.Data_Cnt));

			for (i = 0; i < Acc_Cnt; i++)
			{
				/* 계좌정보 */
				memcpy(Wdata[i].Ap_Gbn, ACCNO(D_K,i+Acc_No).aptype_code,	
					sizeof(Wdata[i].Ap_Gbn));

				/* 매매손익	*/
				sprintf(Wdata[i].Acc_real_prft, "%10d", 
										ACCNO(D_K,i+Acc_No).acc_real_prft);

				/* 수수료	*/
				sprintf(Wdata[i].Acc_fee, "%10d", ACCNO(D_K,i+Acc_No).acc_fee);

				/* 평가손익	*/
				sprintf(Wdata[i].Acc_ver_prft, "%10d", 
					  ACCNO(D_K,i+Acc_No).acc_ver_prft
					- ACCNO(D_K,i+Acc_No).over_acc_ver_prft);

				/* 한도 구분 */
				sprintf(Wdata[i].Risk_Gbn, "%1d", ACCNO(D_K,i+Acc_No).risk_flag);

				/* MM 200809 */
				sprintf(Wdata[i].Etc_Risk_Gbn, "%1d", ACCNO(D_K,i+Acc_No).etc_risk_flag);
			}
			
			memcpy(send_data,	&Wheader,	sizeof(UDP_OUT_HEADER));
			memcpy(&send_data[sizeof(UDP_OUT_HEADER)],
				&Wdata,	sizeof(UDP_OUT_DATA) * Acc_Cnt);
		}

		usleep(PROC(D_K,P_K).timeout * 10000);

		add_len = sizeof (Svr_Addr[0]);

		for ( i = 0; i < Acc_Cnt; i++)
		{

			if (memcmp(ACCNO(D_K,i+Acc_No).ip_addr, "000", 3) == 0)
				continue;

			rt = sendto (Sockfd[i], send_data, strlen(send_data),
				0, (struct sockaddr *)&Svr_Addr[i], add_len);
			if (rt < 0)
			{
				Log (UDP_FATAL, "Data Send Error [%d:%s]", SYS_NO, SYS_STR);
				break;
			}
		}

		if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
			Log (USR_OK, "UDP S [%.10s](%d)", send_data, strlen (send_data));
	}
}

/*----------------------------------------------------------------------*/
int     Socket_Connect (void)
/*----------------------------------------------------------------------*/
{
	int     rt, i, bufflen, oplen;
	int		in1, in2, in3, in4;
	char    Svr_IP[20];

	for (i = 0; i < Acc_Cnt; i++)
	{
		if (memcmp(ACCNO(D_K,i+Acc_No).ip_addr, "000", 3) == 0)
			continue;

		in1 = in2 = in3 = in4 = 0;
		memset (Svr_IP, 0x00, sizeof(Svr_IP));
		in1 = AtoIf(ACCNO(D_K,i+Acc_No).ip_addr, 3);
		in2 = AtoIf(&ACCNO(D_K,i+Acc_No).ip_addr[3], 3);
		in3 = AtoIf(&ACCNO(D_K,i+Acc_No).ip_addr[6], 3);
		in4 = AtoIf(&ACCNO(D_K,i+Acc_No).ip_addr[9], 3);

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

		Log (UDP_OK, "socket %d created [%s:%s:%d]", i, ACCNO(D_K,i+Acc_No).ip_addr,
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
    end of program(pa_9999_us.c)
**************************************************************************/
