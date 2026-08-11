/*------------------------------------------------------------------------
#   Module  : Common Device_Read / Device_Write for KRX event-loop
#   File    : device_rw.c
#   Note    : Separated from fep_common.c to avoid multiple-definition
#             conflicts with processes that define their own
#             Device_Read/Device_Write (e.g., pb_8100_ts, pb_1800_ts).
------------------------------------------------------------------------*/

#include    "fep_fepp.h"
#include    "fep_common.h"

/*************************************************************************
    Function        : . Device_Read
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int (0:success, -1:failure)
    Comment         : . Tcpip Data Recv
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Device_Read(void)
/*----------------------------------------------------------------------*/
{
    int     rt;

    memset(DataBuff, 0, KRX_DATA_BUFF_SIZE);
    RecvLen = 0;

    rt = Select_Receive_Krx(Sockfd, DataBuff, KRX_HEAD_LEN);

    if (rt <= 0) {
        Device_Close();
        return (NOTOK);
    }

    RecvLen = rt;
    Log_Hot(TCP_OK, "TCP RD [%s](%d)<%d>", DataBuff, RecvLen, INT_SEQ);

    return (OK);
}   /* End of Device_Read ()    */

/*************************************************************************
    Function        : . Device_Write
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . Tcpip Data Send (SendLen must be set before call)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Write(void)
/*----------------------------------------------------------------------*/
{
    int     rt;

    rt = Select_Send(Sockfd, DataBuff, SendLen);

    if (rt != OK) {
        Log(TCP_ERROR, "TCP data send fail");
        Device_Close();
    }

    Log_Hot(TCP_OK, "TCP SD [%s](%d)<%d>", DataBuff, SendLen, INT_SEQ);
    Get_Msec(&SendMsec);

    return;
}   /* End of Device_Write ()   */

/*************************************************************************
    End of Program (device_rw.c)
*************************************************************************/
