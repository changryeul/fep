/*------------------------------------------------------------------------
#   Module  :
#   File    : hoga_check.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"
#include    "fep_interface.h"

/*************************************************************************
    Function        : . Tick_Chk
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int (0:success, 1:timeout, -1:failure)
*************************************************************************/
/*
    채권은 3가지의 가격단위가 존재한다. 여기서 체크할 것은
    1. 호가정합성 체크 (-1:불량, 0이상이면 정상이면서 2번의 결과값 리턴)
    2. 현재가와 주문낼 호가가격이 몇틱인지
*/
/*----------------------------------------------------------------------*/
int     Note_Tick_Chk(double curr_price, double order_price, int item_seq)
/*----------------------------------------------------------------------*/
{
    int     tick_diff;
    double  prc_result;
    double  fmod_result;

    /* 채권 가격단위규칙은 3개중 한개이며 A0종목정보에 값을 갖고있다.
       - A1   (  1원/POINT) :   1원/POINT 미만 단위의 가격으로 호가 제출 시 거부처리  예) 00009999.50
       - A0.5 (0.5원/POINT) : 0.5원/POINT 미만 단위의 가격으로 호가 제출 시 거부처리  예) 00009999.40
       - A0.1 (0.1원/POINT) : 0.1원/POINT 미만 단위의 가격으로 호가 제출 시 거부처리  예) 00009999.05
    */

    /* 1. 호가적합성, 호가구간 정합성 체크 */
    if (memcmp(Shm_Note[item_seq].A0.prc_unit_rule_id, "A1",   2) == 0)
        prc_result = 1.0;
    else
        if (memcmp(Shm_Note[item_seq].A0.prc_unit_rule_id, "A0.5", 4) == 0)
        prc_result = 0.5;
    else
        if (memcmp(Shm_Note[item_seq].A0.prc_unit_rule_id, "A0.1", 4) == 0)
        prc_result = 0.1;
    else {
        Log(USR_ERROR, "채권가격 호가적합성 가격구간 오류 item_seq[%d][%12.12s]", item_seq, Shm_Note[item_seq].A0.item_code);
        return (-1);
    }

    /* 2. 호가적합성, 주문가격호가단위 체크, 채권가격은 소수점 2자리. */
    fmod_result = fmod(order_price, prc_result);
    if (fmod_result > 0.00) {
        Log(USR_ERROR, "채권가격 호가적합성 가격단위 오류 나머지[%lf] item_seq[%d][%12.12s]", fmod_result, item_seq, Shm_Note[item_seq].A0.item_code);
        return (-1);
    }

    /* 3. 현재가와 호가가격의 Tick 체크(몇 Tick) */
    if (prc_result < 0) {
        Log(USR_ERROR, "채권가격 호가적합성 오류 item_seq[%d][%12.12s]", item_seq, Shm_Note[item_seq].A0.item_code);
        return (-1);
    }

    tick_diff = (int)round(fabs(curr_price - order_price) / prc_result);
    return tick_diff;

}   /* End of Tick_Chk ()   */

/*************************************************************************
    Function        : . Tick_Chk
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int (0:success, 1:timeout, -1:failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Tick_Chk(int market_gbn, double curr_price, double order_price)
/*----------------------------------------------------------------------*/
{
}   /* End of Tick_Chk ()   */

/*************************************************************************
    Function        : . JS_Tick_Chk
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int (0:success, 1:timeout, -1:failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     JS_Tick_Chk(int market_gbn, double standard_price, double curr_price, double order_price)
/*----------------------------------------------------------------------*/
{
}   /* End of JS_Tick_Chk ()    */

/*----------------------------------------------------------------------*/
double      Hoga_Change(int market_gbn, int item_seq, int mk_group, int chg_hoga, double order_price)
/*----------------------------------------------------------------------*/
{
}   /* End of Hoga_Change */

/*************************************************************************
    End of Program (hoga_check.c)
*************************************************************************/
