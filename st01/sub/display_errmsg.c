/*------------------------------------------------------------------------
#   System  : HANWHA FEP
#   Module  : display KRX error message
#   File    : display_errmsg.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*************************************************************************
    Function        : . display KRX error message
    Parameters IN   : . p_code      : integer
                    : . p_firstseq  : integer
    Return Code     : .
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Display_ErrMsg(int p_code, int p_firstseq)
/*----------------------------------------------------------------------*/
{
    switch (p_code) {
        /* ���� */
        case    0:  /* ���� */
            break;
        case    1:
            Log(USR_ERROR, "����ڰ���(ID,PASSWORD)����[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;
        case    2:
            Log(USR_ERROR, "���Ǹ޽���������������[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;
        case    3:
            Log(USR_ERROR, "���ȸ����ȣ����[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;
        case    4:
            Log(USR_ERROR, "�������Ϸù�ȣ����[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;
        case    5:
            Log(USR_ERROR, "����Ÿ�Ǽ�����[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;
        case    6: /* Batch�ۼ��Ž� TR�� ����(999~) ������ ���� TR ��۽� */
            Log(USR_WARN, "�����⸶������[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;
        case    7:
            Log(USR_ERROR, "�¶��ΰ�������[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;
        case    8:
            Log(USR_ERROR, "���������޽���Ÿ�Կ���[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;
        case    9:
            Log(USR_ERROR, "����ó����[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;
        case    10:
            Log(USR_ERROR, "����޼������̿���[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;
        case    11:
            Log(USR_ERROR, "�Ϻ�ȣȭ���ÿ���[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;
        case    12:
            Log(USR_ERROR, "Message Body���� Null����Ÿ����[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;

            /* ���� */
        case    90:
            Log(USR_ERROR, "�ý��ۿ���[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;

            /* ���� */
        case  101:
            Log(USR_OK, "ȣ������������[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;
        case  102:
            Log(USR_WARN, "�ŸŰŷ��ð�������[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;
        case  103:
            Log(USR_ERROR, "ȣ�������Ͻ�����[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;
        default:
            Log(USR_ERROR,
                    "RP_DATA recv:unknown �źλ����ڵ�[%d] <%d:%d:%d>",
                    p_code, p_firstseq, INT_SEQ, LOAD_CNT);
            break;
    }

}   /* End of Display_ErrMsg () */

/*************************************************************************
    End of Program (display_errmsg.c)
*************************************************************************/
