#define     _GLOBAL
/*------------------------------------------------------------------------
 * #    Author  : Park SH
 * #    Module  : Protocol For Inner
 * #    File    : pa_8200_tr.c
 * #    Commant : Send To Inner(Noa)
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
#define     DATA_TIME       (30+5) * 1000       /* Heart Beat  30 sec(30+5) */
#define     FOREVER_TIME    60 * 1000                       /* 60 sec   */

#define     FIFO_EVENT      0
#define     SOCKET_EVENT    1

/*------------------------------------------------------------------------
 *  Global Variables
 *------------------------------------------------------------------------*/
int     Sockfd, Newfd;
int     PollCnt, FirstSeq, TimeOut, PortNo;
char    ApType[10];
char    RecvPkt[TCP_BUFF_MAX_LEN], SendPkt[TCP_BUFF_MAX_LEN];
char    DeviceSendFlag, LogOnFlag, OpenFlag;

FILE_BUFF_FORMAT    W_Fmt[MAX_CNT];         // Write Buff

CLI_HEAD     		*S_Pkt = (CLI_HEAD *)SendPkt;    // Response Data Buff
CLI_FORMAT          *R_Pkt = (CLI_FORMAT *)RecvPkt;        // Recv Data Buff

struct pollfd       Poll[2];

/*------------------------------------------------------------------------
 *  Function Prototypes
 *-----------------------------------------------------------------------*/
void    PA_8200_TR (void);
void    Init_Parameters (void);
void    Fifo_Event_Rtn (void);
void    Socket_Event_Rtn (void);
void    Device_Write (void);
void    Device_Open (void);
void    Device_Close (void);
int     Device_Read (void);
void    Time_Out_Rtn (void);
int     Make_Send_Msg (int);
void    Write_Data (void);
void	Set_Socket_Linger (void);

/*----------------------------------------------------------------------*/
int     main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    Init_Proc (argc, argv);
    PA_8200_TR ();
    Exit_Process ();
}   /* End of main ()   */

/*----------------------------------------------------------------------*/
void    PA_8200_TR (void)
/*----------------------------------------------------------------------*/
{
    int     rt, i;
    int     size = 1000000;

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

                Log (USR_OK, "accepting ...");
                Newfd = accept(Sockfd, (struct sockaddr*)NULL, NULL);
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
                    PollCnt = 2;
                    TimeOut = DATA_TIME;
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
            default:
                Log (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                Exit_Process ();
                break;
        }
    }

    //Device_Close ();

    return;
}   /* End of PA_8200_TR ()    */

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

	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU (ApType, strlen (ApType));

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
 *  Comment         : . TCP Data Recv & Response Action
**************************************************************************/
/*----------------------------------------------------------------------*/
void    Socket_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
    int         rval, rt, cnt, i;
    char        m_time[24], Chg_Data[4096];

    rval = Device_Read ();

    if (rval < 0)
        return;

    DeviceSendFlag = OFF;

    if (LogOnFlag == ON)
    {
        if (memcmp (R_Pkt->Head.MsgType, "DATA", 4) == 0)
        {
            if (memcmp (R_Pkt->Head.ResponsCode, "0000", 4) != 0)
            {
                Log (USR_ERROR, "DATA Response is NotOk [%32.32s]", R_Pkt);
                Exit_Process ();
            }

            rt = Make_Send_Msg (RP_DATA);
            Device_Write ();

            if (!rt)
            {
                Write_Data ();                      // Data Write
                INT_SEQ ++;
            }
        }
        else
        if (memcmp (R_Pkt->Head.MsgType, "POLL", 4) == 0)
        {
            if (memcmp (R_Pkt->Head.ResponsCode, "0000", 4) != 0)
            {
                Log (USR_ERROR, "POLL Response is NotOk [%4.4s]", R_Pkt->Head.ResponsCode);
                Exit_Process ();
            }

            rt = Make_Send_Msg (RP_POLL);
            Device_Write ();
        }
        else
        {
            Log (USR_ERROR, "MsgType Not Defined Code [%32.32s]", R_Pkt);
            Exit_Process ();
        }
    }
    else        /* LOGIN(LINK) */
    {
        if (memcmp (R_Pkt->Head.MsgType, "LINK", 4) != 0)
        {
            Log (USR_ERROR, "LOGON(LINK) error, check plese Recv Data[%32.32s]", R_Pkt);
            Exit_Process ();
        }

		/* Device Send */
        Make_Send_Msg (RP_LINK);
        Device_Write ();

        LogOnFlag = ON;
    }

    return;
}   /* End of Socket_Event_Rtn ()   */

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
    /* socket/setsockopt/bind/listen/accept */
    int         rt;
    const   int on = 1;

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

//  TCP2_LINE_ST = OpenFlag = ON;

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
        Log (TCP_ERROR, "TCP RD Worng Length[%s] (%d)<%d>", RecvPkt, rt, INT_SEQ);
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
	Log (USR_OK, "TimeOut");
#if 0
    int     rt;
    char    in_num[10];

    switch (PollCnt)
    {
        case    2:
            Log (TCP_ERROR, "no DATA && Response from NOA. check status <%d>", INT_SEQ);
			Device_Close ();

            break;
        default:
            break;
    }
#endif

    return;
}   /* End of Time_Out_Rtn ()   */

/*************************************************************************
    Function        : . Make_Send_Msg
    Parameters IN   : . TR_TYPE
    Parameters OUT  : . result (0:OK, 1:NotOK)
    Return Code     : .
    Comment         : . Send data header making
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Make_Send_Msg (int tr_code)
/*----------------------------------------------------------------------*/
{
    int     rt = 0;
    int     r_cnt;
    int     arry_cnt, arry_len, arry_dat;
    char    d_time[16];

    memset (SendPkt,       0,       sizeof (SendPkt));
    memset (SendPkt,    0x20,   	CLI_HEAD_LEN);  // 50

	Get_DateTime (d_time);

	/* 1. Packet Length (50-4=46) */
	ItoAf (CLI_HEAD_LEN - sizeof (S_Pkt->Length),
							S_Pkt->Length, sizeof (S_Pkt->Length));
	/* Header 3. ResponsCode	*/
	memcpy (S_Pkt->ResponsCode, "0000", sizeof (S_Pkt->ResponsCode));
	/* Header 4. Date			*/
	memcpy (S_Pkt->TradeDate,	d_time,	sizeof (S_Pkt->TradeDate));

    switch (tr_code)
    {
        case    RP_DATA:
			/* 2. MsgType */
			memcpy (S_Pkt->MsgType,	"DAOK",	sizeof (S_Pkt->MsgType));

			/* 5. SeqNo */
			ItoAf (INT_SEQ + 1, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));

            break;

        case    RP_LINK:
			/* 2-1. MsgType */
			memcpy (S_Pkt->MsgType, "LIOK", sizeof (S_Pkt->MsgType));

			/* 5. SeqNo */
			ItoAf (INT_SEQ, S_Pkt->SeqNo,	sizeof (S_Pkt->SeqNo));

            break;

        case    RP_POLL:
			/* 2-1. MsgType */
			memcpy (S_Pkt->MsgType, "POOK", sizeof (S_Pkt->MsgType));

			/* 5. SeqNo */
			ItoAf (INT_SEQ, S_Pkt->SeqNo,	sizeof (S_Pkt->SeqNo));

            break;

        default:
            break;
    }

    return (rt);
}   /* End of Make_Send_Msg ()  */

/*************************************************************************
    Function        : . Write_Data
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . ReJect from HUB, write for Noa
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Write_Data (void)
/*----------------------------------------------------------------------*/
{
    int     rt, cnt, i, target;
    char    m_time[24];

    memset (W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));
    memset (m_time, 0, sizeof (m_time));

    Get_MicroTime (m_time);

    /* 1. Seq is Write at F_W Funtion   */
    /* 2. If_Seq        */
    ItoAf (INT_SEQ+1,               W_Fmt[0].If_Seq,    sizeof (W_Fmt[0].If_Seq));
    /* 3. ApType        */
    memcpy (W_Fmt[0].ApType,        ApType,             sizeof (W_Fmt[0].ApType));
    /* 4. ResponseCode  */
    memcpy (W_Fmt[0].ResponseCode,  RES_NORMAL,         strlen (RES_NORMAL));
    /* 5. RecvTime1     */
    memcpy (W_Fmt[0].RecvTime1,     m_time,             sizeof (W_Fmt[0].RecvTime1));
    /* 6. RecvTime2     */
    memcpy (W_Fmt[0].RecvTime2,     m_time+10,          sizeof (W_Fmt[0].RecvTime2));
    /* 7. DataHeader(Data Size) */
    ItoAf (strlen(R_Pkt->Data),		W_Fmt[0].DataHeader,    4);

	if ((memcmp (R_Pkt->Data, "700", 3) == 0)	||
		(memcmp (R_Pkt->Data, "900", 3) == 0))
	{
		/* Copy Data        */
		memcpy (W_Fmt[0].Data,	R_Pkt->Data,	strlen(R_Pkt->Data));
		W_Fmt[0].LineFeed[0] = '\n';

	    rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);			// pa_6001_mp
	}
	else if (memcmp (R_Pkt->Data, "500", 3) == 0)
	{
		/* Copy Data        */
		memcpy (W_Fmt[0].Data,	R_Pkt->Data,	strlen(R_Pkt->Data));
		W_Fmt[0].LineFeed[0] = '\n';

		if (memcmp (R_Pkt->Data, "500100", 6) == 0)		// 기동요청
			rt = F_W (TS_W2_1, (void *)&W_Fmt, 1);		// pa_9001_mp
		else
		{
			SEARCH_HEADER	*Header = (SEARCH_HEADER *)&R_Pkt->Data;
			target = (AtoIf (&Header->ApType_Cd[1], 2) - 1) * 10;
			target = AtoIf (&Header->ApType_Cd[3], 2) + 3 + target;
			rt = DSHM_W (target*10, (void *)&W_Fmt, 1);		// 자동Process
		}
	}
	else if (memcmp (R_Pkt->Data, "100", 3) == 0)		// 100100 주문
	{
		/* Copy Data        */
		memcpy (W_Fmt[0].Data,	&R_Pkt->Data[SEARCH_HEADER_LEN],	strlen(R_Pkt->Data)-SEARCH_HEADER_LEN);
		W_Fmt[0].LineFeed[0] = '\n';

		KRX_JUMUN_DATA  *dat = (KRX_JUMUN_DATA *)R_Pkt->Data;
		if ((memcmp (dat->MembershipItem+35, "05", 2) == 0)	||
			(memcmp (dat->MembershipItem+35, "07", 2) == 0)	)
		{
			rt = DSHM_W (TS_W1_1, (void *)&W_Fmt, 1);	// 유가증권, 7(ELW/ETF/ETN)
		}
		else
		if (memcmp (dat->MembershipItem+35, "05", 2) == 0)
		{
			rt = DSHM_W (TS_W2_1, (void *)&W_Fmt, 1);	// 코스닥
		}
		else
			rt = DSHM_W (TS_W3_1, (void *)&W_Fmt, 1);	// 파생
	}
	else
    {
        Log (SAM_FATAL, "Recv TR_Code No Data [%50.50s]", R_Pkt->Data);
	}

    if (rt != 1)
    {
        Log (SAM_FATAL, "file write [%3.3s]", R_Pkt->Data);
        Exit_Process ();
    }

    Log (USR_OK, "file write[%3.3s] rt[%d]", R_Pkt->Data, rt);
    return;

}   /* End of Write_Data () */

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

    rt = setsockopt (Sockfd, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling));
    if (rt < 0)
        Log (TCP_ERROR, "setsockopt SO_LINGER {%d:%s}", SYS_NO, SYS_STR);

    return;
}

/*************************************************************************
 *  End of Program (pa_8200_tr.c)
 **************************************************************************/

