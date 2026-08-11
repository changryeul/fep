/*------------------------------------------------------------------------
#   Module  : INISAFE-Net encryption functions for PB processes
#   File    : fep_encrypt.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "fep_common.h"
#include    "fep_encrypt.h"

#ifdef NO_INISAFE
/*========================================================================
    NO_INISAFE mode: stub implementations (no encryption)
========================================================================*/

/*----------------------------------------------------------------------*/
void    Free_All(void *ctxf)
/*----------------------------------------------------------------------*/
{
    (void)ctxf;
    /* No-op: INISAFE disabled */
}   /* End of Free_All () stub */

/*----------------------------------------------------------------------*/
int     Handshake(void)
/*----------------------------------------------------------------------*/
{
    Log(USR_OK, "INISAFE disabled (NO_INISAFE) - Handshake skipped");
    return (OK);
}   /* End of Handshake () stub */

#else
/*========================================================================
    INISAFE enabled: real implementations
========================================================================*/

/*************************************************************************
    Function        : . Free_All
    Parameters IN   : . ctxf : non-zero to also free EnCtx
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . Free all INISAFE handshake buffers
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Free_All(void *ctxf)
/*----------------------------------------------------------------------*/
{
    if (cinitout) {
        INL_Free_Buf(cinitout);
        cinitout = NULL;
    }
    if (cupdateout) {
        INL_Free_Buf(cupdateout);
        cupdateout = NULL;
    }
    if (cfinalout) {
        INL_Free_Buf(cfinalout);
        cfinalout = NULL;
    }
    if (sinitout) {
        INL_Free_Buf(sinitout);
        sinitout = NULL;
    }
    if (supdateout) {
        INL_Free_Buf(supdateout);
        supdateout = NULL;
    }

    if (ctxf && EnCtx) {
        INL_Free_Ctx(EnCtx);
        EnCtx = NULL;
    }
}   /* End of Free_All ()   */

/*************************************************************************
    Function        : . Handshake
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int (OK:success, NOTOK:failure)
    Comment         : . INISAFE-Net 3-phase handshake (Init/Update/Final)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Handshake(void)
/*----------------------------------------------------------------------*/
{
    int     result;
    int     hl;

    result = INL_Initialize(CLIENT_CTX, KRX_INITECH_CONF_PATH, NULL);
    if (result != 0) {
        Log(TCP_ERROR, "INL_Initialize Failed. [%d:%s]",
                result, INL_Error_String(result));
        Free_All((void *)(long)1);
        return (NOTOK);
    }

    result = INL_New_Ctx(CLIENT_CTX, &EnCtx);
    if (result != 0) {
        Log(TCP_ERROR, "INL_CtxNew Failed. [%d:%s]",
                result, INL_Error_String(result));
        Free_All((void *)(long)1);
        return (NOTOK);
    }

    /* InitHandShake */
    result = INL_Handshake_Init(EnCtx, NULL, 0, &cinitout, &cinitoutl);
    if (result != 0) {
        Log(TCP_ERROR, "Client Init HandShake Failed. [%d:%s]",
                result, INL_Error_String(result));
        Free_All((void *)(long)1);
        return (NOTOK);
    }

    result = Make_Send_Msg(TR_HSI);
    memset(DataBuff, 0, sizeof (DataBuff));
    memcpy(DataBuff, FmtPtr, SendLen);
    Device_Write();
    Log(USR_OK, "send Handshake init request");

    result = Device_Read();
    if (result < 0) {
        Log(TCP_ERROR, "No Handshake init response from Server[%d]", result);
        Free_All((void *)(long)1);
        return (NOTOK);
    }

    hl = RecvLen - KRX_HEAD_LEN - KRX_ERRCODE_LEN;
    if (memcmp(((KRX_HEADER *)DataBuff)->MsgType, "SCHLIQ00102", 11) != 0 ||
            memcmp(&DataBuff[KRX_HEAD_LEN], RESP_SUCCESS, KRX_ERRCODE_LEN) != 0) {
        Log(TCP_ERROR, "Handshake init response error[%.11s:%.4s]",
                ((KRX_HEADER *)DataBuff)->MsgType, &DataBuff[KRX_HEAD_LEN]);
        Free_All((void *)(long)1);
        return (NOTOK);
    }

    sinitout = (unsigned char *)malloc(hl + 1);
    if (!sinitout) {
        Log(TCP_ERROR, "sinitout malloc fail size = [%d]", hl);
        Free_All((void *)(long)1);
        return (NOTOK);
    }

    memset(sinitout, '\0', hl + 1);
    memcpy(sinitout, &DataBuff[KRX_HEAD_LEN + KRX_ERRCODE_LEN], hl);

    /* UpdateHandShake */
    result = INL_Handshake_Update(EnCtx, sinitout, hl, &cupdateout, &cupdateoutl);
    if (result != 0) {
        Log(TCP_ERROR, "Client Update HandShake Failed. [%d:%s]",
                result, INL_Error_String(result));
        Free_All((void *)(long)1);
        return (NOTOK);
    }

    result = Make_Send_Msg(TR_HSU);
    memset(DataBuff, 0, sizeof (DataBuff));
    memcpy(DataBuff, FmtPtr, SendLen);
    Device_Write();
    Log(USR_OK, "send Handshake update request");

    result = Device_Read();
    if (result < 0) {
        Log(TCP_ERROR, "No Handshake update  response from Server[%d]", result);
        Free_All((void *)(long)1);
        return (NOTOK);
    }

    hl = RecvLen - KRX_HEAD_LEN - KRX_ERRCODE_LEN;
    if (memcmp(((KRX_HEADER *)DataBuff)->MsgType, "SCHLIQ00104", 11) != 0 ||
            memcmp(&DataBuff[KRX_HEAD_LEN], RESP_SUCCESS, KRX_ERRCODE_LEN) != 0) {
        Log(TCP_ERROR, "Handshake update response error[%.11s:%.4s]",
                ((KRX_HEADER *)DataBuff)->MsgType, &DataBuff[KRX_HEAD_LEN]);
        Free_All((void *)(long)1);
        return (NOTOK);
    }

    supdateout = (unsigned char *)malloc(hl + 1);
    if (!supdateout) {
        Log(TCP_ERROR, "supdateout malloc fail size = [%d]", hl);
        Free_All((void *)(long)1);
        return (NOTOK);
    }

    memset(supdateout, '\0', hl + 1);
    memcpy(supdateout, &DataBuff[KRX_HEAD_LEN + KRX_ERRCODE_LEN], hl);

    /* FinalHandShake */
    result = INL_Handshake_Final(EnCtx, supdateout, hl, &cfinalout, &cfinaloutl);
    if (result != 0) {
        Log(TCP_ERROR, "Client Final HandShake Failed. [%d:%s]",
                result, INL_Error_String(result));
        Free_All((void *)(long)1);
        return (NOTOK);
    }

    result = Make_Send_Msg(TR_HSF);
    memset(DataBuff, 0, sizeof (DataBuff));
    memcpy(DataBuff, FmtPtr, SendLen);
    Device_Write();
    Log(USR_OK, "send Handshake final request");

    Free_All(0);

    return (OK);
}   /* End of Handshake ()  */

#endif /* NO_INISAFE */

/*************************************************************************
    End of Program (fep_encrypt.c)
*************************************************************************/
