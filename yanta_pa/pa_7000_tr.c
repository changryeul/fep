#define     _GLOBAL
/*------------------------------------------------------------------------
 * #    Author  : Park SH
 * #    Module  : KRX시세수신 TCP용(KRX,IMECO)
 * #    File    : pa_7000_tr.c
 * #    Commant : 대외수신 & 내부수신
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
#define     FOREVER_TIME    60 * 1000                       /* 60 sec   */

#define     SOCKET_EVENT    0

/*------------------------------------------------------------------------
 *  Global Variables
 *------------------------------------------------------------------------*/
int     Sockfd, Newfd;
int     PollCnt, FirstSeq, TimeOut, PortNo;
char    ApType[10];
char    RecvPkt[TCP_BUFF_MAX_LEN], SendPkt[TCP_BUFF_MAX_LEN];
char	OpenFlag;

FILE_BUFF_FORMAT    W_Fmt;         // Write Buff

struct pollfd       Poll[1];

/*------------------------------------------------------------------------
 *  Function Prototypes
 *-----------------------------------------------------------------------*/
void    PA_7000_TR (void);
void    Init_Parameters (void);
void    Fifo_Event_Rtn (void);
void    Socket_Event_Rtn (void);
void    Device_Write (void);
void    Device_Open (void);
void    Device_Close (void);
int     Device_Read (void);
void    Time_Out_Rtn (void);
void	Set_Socket_Linger (void);

/*----------------------------------------------------------------------*/
int     main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    Init_Proc (argc, argv);
    PA_7000_TR ();
    Exit_Process ();
}   /* End of main ()   */

/*----------------------------------------------------------------------*/
void    PA_7000_TR (void)
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
			Device_Close ();
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

                Poll[0].fd = Newfd;
                Poll[0].events = POLLIN;

                PollCnt = 1;
                TimeOut = FOREVER_TIME;

            }
            else
            {
				PollCnt = 1;
				TimeOut = FOREVER_TIME;
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
}   /* End of PA_7000_TR ()    */

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
	OpenFlag = OFF;

    L_K = 0;

	PortNo = TCP2_PORT_NO;
    Log (TCP_OK, "Server Side port[%d]", PortNo);

    TCP2_PROC_ST = ON;
    TCP2_LINE_ST = OFF;

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
    int         rval, rt, w_gbn = 0;
    char        m_time[24], Chg_Data[4096];

    rval = Device_Read ();

    if (rval < 0)
        return;

#if defined A7701 || A7702		// 금융파생 시세통수신 TCP
/*
#	A001F(10301/11301) : 1318, 종목정보
#	A301F : 173, 파생체결
#	G701F : 431, 파생체결G7 (우선호가  5단계) 04F & 05F(제외)
#	B601F : 324, 파생우선호가 (우선호가  5단계) 04F & 05F(제외)
#	A701A : 68,  장운영정보(모든 A7공통.. All)

#	M401A : 83,  장운영스캐쥴
#	R101F : 359, 파생 장운영TS + 우선호가 (우선호가  5단계) 단일가 세션, 04F, 05F 제외
*/
	if (memcmp (RecvPkt, "A006F", 5) == 0)	
		w_gbn = 1;
	else if ((memcmp (RecvPkt, "A306F", 5) == 0)	||
			 (memcmp (RecvPkt, "G706F", 5) == 0)	||
			 (memcmp (RecvPkt, "B606F", 5) == 0)	||
			 (memcmp (RecvPkt, "A706F", 5) == 0))
		w_gbn = 2;
	else if (memcmp (RecvPkt, "M406F", 5) == 0)
		w_gbn = 3;
	else
	{
		w_gbn = 0;
		Log (USR_OK, "Else(LK) Recv [%s][%d]", RecvPkt, strlen(RecvPkt));
	}
#elif defined A7622				// 채권 수신시세 전달 To.TCP
/* 기본정보 채권KTS 시세 */
/*	7622
#    A701K : 68  장운영TS
#    M401K : 83, 장운영스캐쥴
#	7623
#    B601K : 462 일반채권, 국고채권 우선호가
#    A301K : 223 채권 체결
#    G701K : 643 일반채권, 국고채권 체결 + 우선호가
#    A601K : 57  채권 종목마감
#    I2000 : 10  Polling, Log만 File은 처리안함
*/
	if ( (memcmp (RecvPkt, "A301K", 5) == 0)	||
		 (memcmp (RecvPkt, "G701K", 5) == 0)	||
		 (memcmp (RecvPkt, "B601K", 5) == 0)	||
		 (memcmp (RecvPkt, "A601K", 5) == 0))
		w_gbn = 1;
	else
	{
		w_gbn = 0;
		Log (USR_OK, "Else(LK) Recv [%s][%d]", RecvPkt, strlen(RecvPkt));
	}
#elif defined A7623				// 채권 수신시세 전달 To.TCP
/* 기본정보 채권KTS 시세 */
/*	7622
#    A701K : 68  장운영TS
#    M401K : 83, 장운영스캐쥴
#	7623
#    B601K : 462 일반채권, 국고채권 우선호가
#    A301K : 223 채권 체결
#    G701K : 643 일반채권, 국고채권 체결 + 우선호가
#    A601K : 57  채권 종목마감
#    I2000 : 10  Polling, Log만 File은 처리안함
*/
	if ( (memcmp (RecvPkt, "A701K", 5) == 0)	||
		 (memcmp (RecvPkt, "M401K", 5) == 0))
		w_gbn = 1;
	else
	{
		w_gbn = 0;
		Log (USR_OK, "Else(LK) Recv [%s][%d]", RecvPkt, strlen(RecvPkt));
	}
#endif

	if (w_gbn)
	{
		Get_MicroTime (m_time);

		/* File Write */
		memset (&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));

		/* write to DD file */
		if (w_gbn == 1)
			ItoAf (O_W_CNT1 + 1, W_Fmt.If_Seq,  sizeof (W_Fmt.If_Seq));
		else if (w_gbn == 2)
			ItoAf (O_W_CNT2 + 1, W_Fmt.If_Seq,  sizeof (W_Fmt.If_Seq));
		else if (w_gbn == 3)
			ItoAf (O_W_CNT3 + 1, W_Fmt.If_Seq,  sizeof (W_Fmt.If_Seq));
		memcpy (W_Fmt.ApType, ApType,       sizeof (W_Fmt.ApType));
		memcpy (W_Fmt.ResponseCode, RES_NORMAL, strlen (RES_NORMAL));
		memcpy (W_Fmt.RecvTime1, m_time,    sizeof (W_Fmt.RecvTime1));
		memcpy (W_Fmt.RecvTime2, &m_time[sizeof(W_Fmt.RecvTime1)],
											sizeof (W_Fmt.RecvTime2));

		memset (W_Fmt.DataHeader, 0x20, 20+DATA_SIZE);
		memcpy (W_Fmt.Data, RecvPkt, strlen(RecvPkt));
		W_Fmt.Data[OFS(D_K,P_K,w_gbn-1)] = '\n';
		W_Fmt.LineFeed[0] = '\n';

		rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);
		if (rt != 1)
		{
			SLog (SAM_FATAL, "file write fail[%s] rt[%d]", OFN(D_K,P_K,w_gbn-1), rt);
				close (Sockfd);
				return;
		}
	}

	INT_SEQ ++;

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

    Poll[0].fd = Sockfd;
    Poll[0].events = POLLIN;

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

	OpenFlag = OFF;

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

    rt = Sise_Select_Receive (Newfd, RecvPkt);

	if (rt == 0) {
        // 아직 패킷이 안 끝났으므로 한 번 더 읽어본다
        rt = Sise_Select_Receive(Newfd, RecvPkt);
    }

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
 *  End of Program (pa_7000_tr.c)
 **************************************************************************/

