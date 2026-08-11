#define     _GLOBAL
/*------------------------------------------------------------------------
 * #    Author  : Park SH
 * #    Module  : Protocol For Client, client send
 * #    File    : pa_8100_ts.c
 * #    Commant : Send To Client
 * ------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
 *  Header Files
 *------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "cli_interface.h"

#define     MAX_CNT         1

#define     DATA_SIZE       2048
#include    "buf_struct.h"

/*------------------------------------------------------------------------
 *  Constants and Structures
 *------------------------------------------------------------------------*/
#define     DEVICE_TIME     3 * 1000                        /*  3 sec   */
#define     DATA_TIME       (30-5) * 1000       /* Heart Beat  30 sec(30-5) */
#define     FOREVER_TIME    60 * 1000                       /* 60 sec   */

#define     FIFO_EVENT      0
#define     SOCKET_EVENT    1
#define     DATA_EVENT      2

/*------------------------------------------------------------------------
 *  Global Variables
 *------------------------------------------------------------------------*/
int     Sockfd, Newfd;
int		SendLen, MsgLen, ReTrCode;
int     PollCnt, FirstSeq, TimeOut, Pk, PortNo, SendFlag;
char    ApType[10];
char    RecvPkt[CLI_BUFF_MAX_LEN], SendPkt[CLI_BUFF_MAX_LEN];
char    DeviceSendFlag, LogOnFlag, OpenFlag;

FILE_BUFF_FORMAT        R_Fmt[MAX_CNT], W_Fmt[MAX_CNT];		// Read Buff

CLI_FORMAT    			*S_Pkt = (CLI_FORMAT *)SendPkt;	// Send Data Buff
CLI_HEAD                *R_Pkt = (CLI_HEAD *)RecvPkt;		// ResponsCode Data Buff

struct pollfd           Poll[3];

/*------------------------------------------------------------------------
 *  Function Prototypes
 *-----------------------------------------------------------------------*/
void    PA_8100_TS (void);
void    Init_Parameters (void);
void    Fifo_Event_Rtn (void);
void    Socket_Event_Rtn (void);
void    Data_Event_Rtn (void);
void    Device_Write (void);
void    Device_Open (void);
void    Device_Close (void);
int     Device_Read (void);
void    Time_Out_Rtn (void);
void	Make_Send_Msg (int);
void	Set_Socket_Linger (void);

/*----------------------------------------------------------------------*/
int     main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    Init_Proc (argc, argv);
    PA_8100_TS ();
    Exit_Process ();
}   /* End of main ()   */

/*----------------------------------------------------------------------*/
void    PA_8100_TS (void)
/*----------------------------------------------------------------------*/
{
    int     rt, i;

    Init_Parameters ();

#if defined HOLIDAY_APPLY
    char    t_time[12], dt[20];
    time_t  t = time(NULL);
    struct  tm  tm, *tp;

    while (1)
    {
        Get_Time (t_time);

        memset (dt, 0, sizeof (dt));
        sprintf (dt, "%.4s-%.2s-%.2s %.2s:%.2s:%.2s", DAEMON(D_K).date,
            DAEMON(D_K).date+4, DAEMON(D_K).date+6, t_time, t_time+2, t_time+4);
        strptime (dt, "%Y-%m-%d %H:%M:%S", &tm);
        //t = mktime (&tm);
        tp = localtime (&t);

        if (tp->tm_wday == 0 || tp->tm_wday == 6)       /* Sun, Sat */
        {
            Log (USR_OK, "it's weekend. sleeping...[%d]", tp->tm_wday);
            PROC(D_K,P_K).start_status = JOB_END;
            PROC(D_K,P_K).process_status = 9;
            sleep (60);
        }
        else
        {
                break;
/*
            Log (USR_OK, "it's weekday. ok...[%d]", tp->tm_wday);
			if (memcmp (Shm_Risk[0].business_day, DAEMON(D_K).date, 8) == 0)
                break;
            else
            {
                Log (USR_OK, "system Day와 business Day가 다르다...[%8.8s][%8.8s]",
                    DAEMON(D_K).date, Shm_Risk[0].business_day);
                sleep (600);
                continue;
            }
*/
        }
    }
#endif

    while (START_S != END)
    {
        Stat_Save ();

        if (TCP2_NET_STA(0) == END || TCP2_NET_STA(0) == JOB_STOP)
        {                                               /* 종료/중  */
            if (LogOnFlag == ON && TCP2_NET_STA(0) == END)
            {
                Device_Close ();
            }
        }
        else                                            /* 정상주문시간 */
        {
            if (OpenFlag == OFF)
            {
                Device_Open ();
				TCP2_NET_STA(0) = ON;
		
				Log (USR_OK, "acceptiong ...");
				Newfd = accept(Sockfd, (struct sockaddr *)NULL, NULL);
				if (Newfd < 0)
				{
                    if (SYS_NO == ECONNABORTED)
                    {
                        usleep (100000);
                        continue;
                    }

                    Log (TCP_ERROR, "accept[%d] {%d:%s}", Newfd, SYS_NO, SYS_STR);
                    return;
                }
                Log (USR_OK, "accepted");

                Set_Socket_Linger ();

                TCP2_LINE_ST = OpenFlag = ON;

                Poll[1].fd = Newfd;
                Poll[1].events = POLLIN;

                PollCnt = 2;
                TimeOut = FOREVER_TIME;
            }
            else
            {
				if (LogOnFlag == ON)
                {
                    if (DeviceSendFlag == ON)
                    {
                        PollCnt = 2;
                        TimeOut = FOREVER_TIME;
                    }
                    else
                    {
                        PollCnt = 3;
                        TimeOut = DATA_TIME;

#if defined A8101
                        if (WRITE_CNT > R_CNT(0,0))
#elif defined A8102
                        if (WRITE_CNT > R_CNT(0,1))
#endif
                        {
                            Data_Event_Rtn ();
                            continue;
                        }
                    }
                }
                else
                {
                    PollCnt = 2;
                    TimeOut = FOREVER_TIME;
                }
            }
        }

        rt = poll (Poll, PollCnt, TimeOut);
        if (rt < 0)
        {
            if (SYS_NO == EINTR)
                Log (SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
            else
                Log (SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);

            continue;
        }
        else if (rt == 0)
        {
            Time_Out_Rtn ();
            continue;
        }

        for (i = 0; i < PollCnt; i ++)
        {
            if (Poll[i].revents & POLLHUP)
            {
                if (i == SOCKET_EVENT)
                {
                    Log (TCP_ERROR, "socket disconnected[%#06x]", Poll[i].revents);
                    return;
                }

                Log (SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
                continue;
            }
        }

        for (i = 0; i < PollCnt; i ++)
        {
            if (Poll[i].revents & POLLIN)
            {
                Poll[i].revents = 0;
                break;
            }
        }

        switch (i)
        {
            case    FIFO_EVENT:
                Fifo_Event_Rtn ();
                break;
            case    SOCKET_EVENT:
                Socket_Event_Rtn ();
                break;
            case    DATA_EVENT:
                Data_Event_Rtn ();
                break;
            default:
                Log (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                Exit_Process ();
                break;
        }
    }

    //Device_Close ();

    return;
}   /* End of PA_8100_TS ()    */

/*************************************************************************
 *  Function        : . Init_Parameters
 *  Parameters IN   : .
 *  Parameters OUT  : .
 *  Return Code     : . void
 *  Comment         : . All Program Parameters Init
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Init_Parameters (void)
/*----------------------------------------------------------------------*/
{
    DeviceSendFlag = OFF;
    LogOnFlag = OFF;
    OpenFlag = OFF;

	L_K = 0;

	PortNo = TCP2_PORT_NO;
    Log (TCP_OK, "Server Side port[%d]", PortNo);

    TCP2_PROC_ST = ON;
    TCP2_LINE_ST = OFF;

    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;
    Poll[2].fd = INPUT_FD;
    Poll[2].events = POLLIN;

    return;
}   /* End of Init_Parameters ()    */

/*************************************************************************
 *  Function        : . Fifo_Event_Rtn
 *  Parameters IN   : .
 *  Parameters OUT  : .
 *  Return Code     : . void
 *  Comment         : . 업무 통제 FIFO SIGNAL GET & No Action
**************************************************************************/
/*----------------------------------------------------------------------*/
void    Fifo_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
    char    tmp[2];

    read (START_FD, tmp, 1);

    return;
}   /* End of Fifo_Event_Rtn () */

/*************************************************************************
 *  Function        : . Socket_Event_Rtn
 *  Parameters IN   : .
 *  Parameters OUT  : .
 *  Return Code     : . void
 *  Comment         : . CLI Data Recv & Response Action
**************************************************************************/
/*----------------------------------------------------------------------*/
void    Socket_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
    int         rval, r_seq, rt;
    char        m_time[24], Chg_Data[4096];

    rval = Device_Read ();

    if (rval < 0)
        return;

    DeviceSendFlag = OFF;

    if (LogOnFlag == ON)
    {
        if (memcmp (R_Pkt->MsgType, "DAOK", 4) == 0)
        {
            if (memcmp (R_Pkt->ResponsCode, "0000", 4) != 0)
            {
                Log (USR_ERROR, "DAOK Response is NotOk [%50.50s]", R_Pkt);
                Exit_Process ();                      // ReJect Data Write
            }

#if defined A8101
            Add_Count(PS_R_1, 1);
#elif defined A8102
            Add_Count(PS_R_2, 1);
#endif
            INT_SEQ ++;
        }
        else
        if (memcmp (R_Pkt->MsgType, "POOK", 4) == 0)
        {
            if (memcmp (R_Pkt->ResponsCode, "0000", 4) != 0)
            {
                Log (USR_ERROR, "POOK Response is NotOk [%4.4s]", R_Pkt->ResponsCode);
                Exit_Process ();
            }
        }
        else
        {
            Log (USR_ERROR, "MsgType Not Defined Code [%s]", R_Pkt);
            Exit_Process ();
        }
    }
    else        /* LOGIN(LINK) */
    {
        if (memcmp (R_Pkt->MsgType, "LINK", 4) != 0)
        {
            Log (USR_ERROR, "LOGON(LINK) error, check plese Recv Data[%s]", R_Pkt);
            Exit_Process ();
        }

        /* InterFace Seq Check && Processing */
        r_seq = AtoIf (R_Pkt->SeqNo, sizeof (R_Pkt->SeqNo));
		if (r_seq > WRITE_CNT || r_seq < 0 || r_seq > INT_SEQ+2)
        {
            TCP2_LINE_ST = OpenFlag = OFF;
            Log (USR_ERROR, "TR_LIOK recv:invalid HUB SEQ <%d> > INT_SEQ <%d> WRITE_CNT<%d>",
                                                                r_seq, INT_SEQ, WRITE_CNT);
            Exit_Process ();
        }
        else
        if (r_seq != INT_SEQ && r_seq <= WRITE_CNT)
        {
            Log (USR_WARN, "TR_LIOK recv:check HUB SEQ <%d> < <%d>", r_seq, INT_SEQ);
/* 20211218 */
/* LIOK로 현재 Seq를 준다. 그리고 Client는 그걸 받아서 Seq에 넣고 사용한다. */
/*
#if defined A8101
            R_CNT(0,0) = INT_SEQ = r_seq;
#elif defined A8102
            R_CNT(0,1) = INT_SEQ = r_seq;
#endif
*/
/* 20211218 */
        }

		/* Device Send */
		Make_Send_Msg (RP_LINK);
		Device_Write ();
		LogOnFlag = ON;
    }

    return;
}   /* End of Socket_Event_Rtn ()   */

/*************************************************************************
 *  Function        : . Data_Event_Rtn
 *  Parameters IN   : .
 *  Parameters OUT  : .
 *  Return Code     : . int : result
 *  Comment         : . SHM Data Processing
*************************************************************************/
/*----------------------------------------------------------------------*/
void		Data_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
    int     rt, r_cnt, i, j;
    char    tmp[128];

	SendFlag = 0;
    Make_Send_Msg (TR_DATA);

	if (SendFlag == 0)
		Make_Send_Msg (TR_POLL);

    /* Device Send */
    Device_Write ();
    Set_TR_Time ();
    DeviceSendFlag = ON;

    while (1)
    {
        rt = read (INPUT_FD, tmp, sizeof(tmp));

        if (rt == 0)
            break;
        else if (rt == -1)
        {
            if (SYS_NO == EINTR)
                continue;
            if (SYS_NO != 11)
                Log (FIF_ERROR, "Poll:cannot read FIFO {%d:%s}", SYS_NO, SYS_STR);
            break;
        }
    }

    return;
}   /* End of Data_Event_Rtn () */

/*************************************************************************
 *  Function        : .  Device_Open
 *  Parameters IN   : .
 *  Parameters OUT  : .
 *  Return Code     : . void
 *  Comment         : . Svm Line Status Set & TCPIP Poll fd set & LOGON
                        Server Version
**************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Open (void)
/*----------------------------------------------------------------------*/
{
    int         rt;
	const	int	on = 1;

    sleep (5);
    /* Creat Socekt */
	Sockfd = Socket ();
    if (Sockfd < 0)
    {
        Log (TCP_ERROR, "socket[%d] {%d:%s}", Sockfd, SYS_NO, SYS_STR);
        TCP2_NET_STA(0) = OFF;
        Exit_Process ();
    }
    Log (USR_OK, "socket created:Sockfd[%d]", Sockfd);

    /* Set Socket Option*/
    rt = setsockopt (Sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof (on));
    if (rt < 0)
    {
        Log (TCP_ERROR, "setsockopt[%d] {%d:%s}", rt, SYS_NO, SYS_STR);
        shutdown (Sockfd, SHUT_RDWR);
        close (Sockfd);
        TCP2_NET_STA(0) = OFF;
        Exit_Process ();
    }
    Log (USR_OK, "setsockopt[%d]", Sockfd);

    /* Bind */
    rt = Bind (Sockfd, PortNo);
    if (rt < 0)
    {
        Log (TCP_ERROR, "bind[%d][%d] {%d:%s}", rt, PortNo, SYS_NO, SYS_STR);
        close (Sockfd);
        TCP2_NET_STA(0) = OFF;
        Exit_Process ();
    }
    Log (USR_OK, "bind[%d]", Sockfd);

    /* Listen */
    Listen (Sockfd);
    Log (USR_OK, "listen[%d]", Sockfd);

    Poll[1].fd = Sockfd;
    Poll[1].events = POLLIN;

    return;
}   /* End of Device_Open ()    */

/*************************************************************************
 *  Function        : .  Device_Close
 *  Parameters IN   : .
 *  Parameters OUT  : .
 *  Return Code     : . void
 *  Comment         : . Svm Line Status Set
 ************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Close (void)
/*----------------------------------------------------------------------*/
{
    close (Sockfd);
    close (Newfd);
    Log (TCP_OK, "TCP device close");

    LogOnFlag = OFF;
    TCP2_LINE_ST = OpenFlag = OFF;

    return;
}   /* End of Device_Close ()   */

/*************************************************************************
 *  Function        : .  Device_Read
 *  Parameters IN   : .
 *  Parameters OUT  : .
 *  Return Code     : . int (0:success, -1:failure)
 *  Comment         : . Tcpip Data Recv
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Device_Read (void)
/*----------------------------------------------------------------------*/
{
    int     rt;
    char    m_time[24];

    memset (RecvPkt, 0, sizeof (RecvPkt));

    rt = Select_Receive_Cli (Newfd, RecvPkt);

    if (rt <= 0 || rt > 4096)
    {
        Log (TCP_ERROR, "TCP RD Worng Lenth[%s] (%d)<%d>", RecvPkt, rt, INT_SEQ);
        Device_Close ();
        return (NOTOK);
    }

    Log (TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen (RecvPkt), INT_SEQ);

    return (OK);
}   /* End of Device_Read ()    */

/*************************************************************************
 *  Function        : .  Device_Write
 *  Parameters IN   : .
 *  Parameters OUT  : .
 *  Return Code     : . void
 *  Comment         : . Tcpip Data Send
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Write (void)
/*----------------------------------------------------------------------*/
{
    int     rt;

    rt = Select_Send (Newfd, SendPkt, strlen (SendPkt));

    if (rt != OK)
    {
        Log (TCP_ERROR, "TCP data send fail");
        Device_Close ();
    }

    Log (TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen (SendPkt), INT_SEQ);

    return;
}   /* End of Device_Write ()   */

/*************************************************************************
 *  Function        : .  Time_Out_Rtn
 *  Parameters IN   : .
 *  Parameters OUT  : .
 *  Return Code     : . void
 *  Comment         : . Timeout Control
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Time_Out_Rtn (void)
/*----------------------------------------------------------------------*/
{
    int     rt;
    char    in_num[10];

    switch (PollCnt)
    {
        case    2:
            if (OpenFlag == ON)
            {
                Log (TCP_ERROR, "no Response from Client. check status <%d>", INT_SEQ);
                Device_Close ();
            }
            else
            {
                Log (USR_OK, "connect timeout <%d>", INT_SEQ);
            }

            break;
        case    3:
            Make_Send_Msg (TR_POLL);
            Device_Write ();
            DeviceSendFlag = ON;
            break;
        default:
            break;
    }

    return;
}   /* End of Time_Out_Rtn ()   */

/*************************************************************************
    Function        : . Make_Send_Msg
    Parameters IN   : . TR_TYPE
    Parameters OUT  : .
    Return Code     : .
    Comment         : . Send data header making
*************************************************************************/
/*----------------------------------------------------------------------*/
void     Make_Send_Msg (int tr_code)
/*----------------------------------------------------------------------*/
{
    int     r_cnt, data_length;
    int     arry_cnt, arry_len, arry_dat;
    char    d_time[16];

    memset (SendPkt,       0,   sizeof (SendPkt));
    memset (SendPkt,    0x20,       CLI_HEAD_LEN);  // 50

    Get_DateTime (d_time);

    /* Header 3. ResponsCode    */
    memcpy (S_Pkt->Head.ResponsCode,"0000", sizeof(S_Pkt->Head.ResponsCode));
    /* Header 4. Date           */
    memcpy (S_Pkt->Head.TradeDate,  d_time, sizeof(S_Pkt->Head.TradeDate));

    switch (tr_code)
    {
        case    TR_DATA:
            /* Header 2. MsgType */
            memcpy (S_Pkt->Head.MsgType, "DATA", sizeof(S_Pkt->Head.MsgType));
            /* Header 5. SeqNo */
            ItoAf (INT_SEQ+1, S_Pkt->Head.SeqNo, sizeof(S_Pkt->Head.SeqNo));

            memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);

#if defined A8101
            r_cnt = F_R (PS_R_1, (void *)R_Fmt, MAX_CNT);
#elif defined A8102
            r_cnt = F_R (PS_R_2, (void *)R_Fmt, MAX_CNT);
#endif
            if (r_cnt < 0 || r_cnt > MAX_CNT)
            {
                Log (SAM_FATAL, "F_R(PS_R_1) R cnt Err [%s] r_cnt[%d]", IDN(D_K,P_K,0), r_cnt);
                Exit_Process ();
            }
            else if (r_cnt == 0)
            {
                return;
            }
			else
			{
#if defined A8101
				if (memcmp (&R_Fmt[0].Data[19], "A", 1) != 0	&&		// A:All, T:2번, C:1번
					memcmp (&R_Fmt[0].Data[19], "C", 1) != 0	&&
					memcmp (R_Fmt[0].Data, "100", 3) != 0)
				{
					if (memcmp (&R_Fmt[0].Data[19], "T", 1) != 0)
						Log (USR_ERROR, "매체구분 오류 [%50.50s][%c]", R_Fmt[0].Data, &R_Fmt[0].Data[19]);
					Add_Count (PS_R_1, 1);
					INT_SEQ ++;
					return;
				}
#elif defined A8102
				if (memcmp (&R_Fmt[0].Data[19], "A", 1) != 0	&&		// A:All, T:2번, C:1번
					memcmp (&R_Fmt[0].Data[19], "T", 1) != 0	&&
					memcmp (R_Fmt[0].Data, "100", 3) != 0)
				{
					if (memcmp (&R_Fmt[0].Data[19], "C", 1) != 0)
						Log (USR_ERROR, "매체구분 오류 [%50.50s][%c]", R_Fmt[0].Data, &R_Fmt[0].Data[19]);
					Add_Count (PS_R_2, 1);
					INT_SEQ ++;
					return;
				}
#endif
			}
			SendFlag = 1;

			data_length = DATA_SIZE;
            /* 1. Header Length(50-4-data_Length) */
            ItoAf (data_length+46, S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));

            /* Copy a Data */
            memcpy (S_Pkt->Data, R_Fmt[0].Data, data_length);

            break;

        case    RP_LINK:
            /* 1. Packet Lengh(50-4 = 46) */
            ItoAf (CLI_HEAD_LEN - sizeof(S_Pkt->Head.Length),
                                        S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));
            /* Header 2. MsgType */
            memcpy (S_Pkt->Head.MsgType, "LIOK", sizeof(S_Pkt->Head.MsgType));
            /* Header 5. SeqNo */
            ItoAf (INT_SEQ, S_Pkt->Head.SeqNo, sizeof(S_Pkt->Head.SeqNo));

            break;

        case    TR_POLL:
            /* 1. Packet Lengh(50-4 = 46) */
            ItoAf (CLI_HEAD_LEN - sizeof(S_Pkt->Head.Length),
                                        S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));
            /* Header 2. MsgType */
            memcpy (S_Pkt->Head.MsgType, "POLL", sizeof(S_Pkt->Head.MsgType));
            /* Header 5. SeqNo */
            ItoAf (INT_SEQ, S_Pkt->Head.SeqNo, sizeof(S_Pkt->Head.SeqNo));

            break;

        default:
            break;
    }

    return;
}   /* End of Make_Send_Msg ()  */

/*************************************************************************
    Function        : . Set_Socket_Linger
    Parameters IN   : . 
    Parameters OUT  : .
    Return Code     : .
    Comment         : . set linger option on socket
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Set_Socket_Linger (void)
/*----------------------------------------------------------------------*/
{
    int     rt;
    struct linger   ling;

    /* close () returns after discarding any unsent data */
    ling.l_onoff = 1;
    ling.l_linger = 0;

    rt = setsockopt (Newfd, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling));
    if (rt < 0)
        Log (TCP_ERROR, "setsockopt SO_LINGER {%d:%s}", SYS_NO, SYS_STR);

    return;
}

/*************************************************************************
 *  End of Program (pa_8100_ts.c)
 **************************************************************************/

