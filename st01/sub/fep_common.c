/*------------------------------------------------------------------------
#   Module  : Common event-loop functions for PA/PB processes
#   File    : fep_common.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "fep_common.h"
#include    "krx_trcode.h"

extern int  ErrCd;          /* int for KRX processes, char[] for IMECO */
extern int  Make_Send_Msg(int);  /* int for KRX, void for client/sise processes */

/*************************************************************************
    Function        : . Get_Msec
    Parameters IN   : .
    Parameters OUT  : . msec : elapsed time in seconds (double)
    Return Code     : . void
    Comment         : . Get current time in seconds with microsecond precision
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Get_Msec(double *msec)
/*----------------------------------------------------------------------*/
{
    struct timeval  tv;

    gettimeofday(&tv, NULL);
    *msec = tv.tv_sec + tv.tv_usec * 1e-6;

    return;
}   /* End of Get_Msec ()   */

/*************************************************************************
    Function        : . Line_Change
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . change lines (main <-> backup)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Line_Change(void)
/*----------------------------------------------------------------------*/
{
    TCP2_LINE_GU = TCP2_LINE_GU + 1;
    S_K = TCP2_LINE_GU % 2;
    TCP2_LINE_GU = S_K;
    sprintf(IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
            TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    Log(TCP_OK, "line changed to %s(%s:%d)",
            S_K == 0 ? "main" : "backup", IpAddr, TCP2_PORT_NO);

    return;
}   /* End of Line_Change ()    */

/*************************************************************************
    Function        : . Device_Close_Base
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . TCP device close (base logic, no encryption cleanup)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Close_Base(void)
/*----------------------------------------------------------------------*/
{
    close(Sockfd);
    Log(TCP_OK, "TCP device close");

    LogOnFlag = OFF;
    TCP2_LINE_ST = OpenFlag = OFF;

    return;
}   /* End of Device_Close_Base ()  */

/*************************************************************************
    Function        : . Err_Msg
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . KRX error message handler
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Err_Msg(void)
/*----------------------------------------------------------------------*/
{
    char    t_time[12];

    memset(t_time, 0, sizeof (t_time));

    switch (ErrCd) {
        case    0:  /* 정상   */
            break;
        case    1:  /* 사용자검증(ID,PASSWORD)오류 */
            Log(USR_ERROR, "사용자검증(ID,PASSWORD)오류[%d] <%d:%d:%d>",
                    ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            sleep(2);
            break;
        case    2:  /* 세션메시지전문수순오류 */
            Log(USR_ERROR, "세션메시지전문수순오류[%d] <%d:%d:%d>",
                    ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            sleep(2);
            break;
        case    3:  /* 헤더회원번호오류 */
            Log(USR_ERROR, "헤더회원번호오류[%d] <%d:%d:%d>",
                    ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            sleep(2);
            break;
        case    4:  /* 데이터일련번호오류 */
            Log(USR_ERROR, "데이터일련번호오류[%d] <%d:%d:%d>",
                    ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            sleep(2);
            break;
        case    5:  /* 메시지길이오류 */
            Log(USR_ERROR, "메시지길이오류[%d] <%d:%d:%d>",
                    ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            sleep(2);
            break;
        case    6:  /* 업무기마감오류 */
            Log(USR_ERROR, "업무기마감오류[%d] <%d:%d:%d>",
                    ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            sleep(2);
            break;
        case    9:  /* 시스템오류 */
            Log(USR_ERROR, "시스템오류[%d] <%d:%d:%d>",
                    ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            sleep(2);
            break;
        case  101:  /* 호가접수개시전 */
            Log(USR_ERROR, "호가접수개시전[%d] <%d:%d:%d>",
                    ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            FirstSeq --;
            memset(t_time, 0, sizeof (t_time));
            Get_Time(t_time);

            if (memcmp(t_time, NoTime, 4) < 0)     /* hhmm */
                sleep(10);
            sleep(2);
            break;
        case  102:  /* 매매거래시간종료후 */
            Log(USR_ERROR, "매매거래시간종료후[%d] <%d:%d:%d>",
                    ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            FirstSeq --;
            sleep(2);
            break;
        case  103:  /* 호가접수일시중지 */
            Log(USR_ERROR, "호가접수일시중지[%d] <%d:%d:%d>",
                    ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            FirstSeq --;
            sleep(2);
            break;
        default:
            Log(USR_ERROR,
                    "RP_DATA recv:unknown 거부사유코드[%d] <%d:%d:%d>",
                    ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            sleep(2);
            break;
    }

    if (ErrCd == 102)   /* 매매거래시간종료후 */ {
        ErrCd = 0;
        Log_Out();
    }
} /* End of Err_Msg ()  */

/*************************************************************************
    Function        : . Log_Out_Base
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int (OK:success, NOTOK:failure)
    Comment         : . KRX LOGOUT request/response (common skeleton)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Log_Out_Base(void)
/*----------------------------------------------------------------------*/
{
    int     rt, rval;

    rt = Make_Send_Msg(TR_LOOU);
    memset(DataBuff, 0, KRX_DATA_BUFF_SIZE);
    memcpy(DataBuff, FmtPtr, SendLen);
    Device_Write();
    Log(USR_OK, "send LOGOUT request");

    rval = Device_Read();
    if (rval < 0) {
        Log(USR_ERROR, "LOGOUT response recv error");
        close(Sockfd);
        return (NOTOK);
    }

    if (IS_RESP_OK(DataBuff)) {
        Log(USR_OK, "LOGOUT success");

        Device_Close();
        TCP2_NET_STA(S_K) = END;
        return (OK);
    }
    else {
        ErrCd = AtoIf(&DataBuff[KRX_HEAD_LEN], KRX_ERRCODE_LEN);
        Err_Msg();
        return (NOTOK);
    }
}   /* End of Log_Out_Base ()   */

/*************************************************************************
    Function        : . Device_Open_Logon
    Parameters IN   : . has_encrypt  : 1=PB (Handshake before LOGON), 0=PA
                    : . immediate_open : 1=set OpenFlag/NET_STA immediately (7800)
    Parameters OUT  : .
    Return Code     : . int (OK:success, NOTOK:failure)
    Comment         : . KRX LOGON request (Socket/Connect/[Handshake]/LOGON)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Device_Open_Logon(int has_encrypt, int immediate_open)
/*----------------------------------------------------------------------*/
{
    int     rt, rval;

    Sockfd = Socket();

    if (Sockfd < 0) {
        Log(TCP_ERROR, "socket fail:Sockfd[%d] {%d:%s}",
                Sockfd, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    Log(USR_OK, "socket created:Sockfd[%d]", Sockfd);
    Log(USR_OK, "connecting to %s:%d", IpAddr, TCP2_PORT_NO);

    rt = Connect(Sockfd, IpAddr, TCP2_PORT_NO);

    if (rt < 0) {
        Log(TCP_ERROR, "connect fail {%d:%s}", SYS_NO, SYS_STR);
        close(Sockfd);
        if (has_encrypt)
            sleep(5);
        return (NOTOK);
    }

    if (has_encrypt) {
        if (Handshake() < 0) {
            close(Sockfd);
            return (NOTOK);
        }
    }

    rt = Make_Send_Msg(TR_LOON);
    memset(DataBuff, 0, KRX_DATA_BUFF_SIZE);
    memcpy(DataBuff, FmtPtr, SendLen);
    Device_Write();
    Log(USR_OK, "send LOGON request");

    rval = Device_Read();
    if (rval < 0 ||
            memcmp(((KRX_HEADER *)DataBuff)->MsgType, "SCHLIR00000", 11) != 0) {
        Log(USR_ERROR, "LOGON response recv error");
        close(Sockfd);
        return (NOTOK);
    }

    if (IS_RESP_OK(DataBuff)) {
        Log(USR_OK, "LOGON success");
    }
    else {
        ErrCd = AtoIf(&DataBuff[KRX_HEAD_LEN], KRX_ERRCODE_LEN);
        Err_Msg();

        close(Sockfd);
        return (NOTOK);
    }

    LogOnFlag = ON;

    if (immediate_open) {
        TCP2_NET_STA(S_K) = ON;
        TCP2_LINE_ST = OpenFlag = ON;
        ConnectRetryCnt = 0;
    }

    Poll[1].fd = Sockfd;
    Poll[1].events = POLLIN;
    Log(TCP_OK, "TCP Connect & LOGON OK");

    return (OK);
}   /* End of Device_Open_Logon ()  */

/*************************************************************************
    Function        : . Time_Out_Disconnect
    Parameters IN   : . source : connection source label ("KRX" or "FOT")
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . Timeout disconnect/reconnect pattern
                    : . Checks TCP2_NET_STA, closes device, triggers
                    : . Line_Change after 3 consecutive retries
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Time_Out_Disconnect(const char *source)
/*----------------------------------------------------------------------*/
{
    if (TCP2_NET_STA(S_K) == OFF || TCP2_NET_STA(S_K) == ON) {
        TCP2_LINE_ST = END;
        Log(TCP_ERROR, "no data from %s. check status <%d>", source, INT_SEQ);
        Device_Close();
        ConnectRetryCnt ++;

        if (ConnectRetryCnt == 3) {
            Line_Change();
            ConnectRetryCnt = 0;
        }
    }

    return;
}   /* End of Time_Out_Disconnect ()    */

/*************************************************************************
    End of Program (fep_common.c)
*************************************************************************/
