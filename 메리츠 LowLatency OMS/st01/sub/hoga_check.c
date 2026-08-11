/*------------------------------------------------------------------------
#	Module	: 
#	File	: hoga_check.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"
#include	"fep_interface.h"

/*************************************************************************
	Function		: . Tick_Chk
	Parameters IN	: . 
	Parameters OUT	: .
	Return Code		: . int (0:success, 1:timeout, -1:failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Tick_Chk (int market_gbn, double curr_price, double order_price)
/*----------------------------------------------------------------------*/
{
	int     i, for_cnt;
    int     order_price_tick = 0, curr_price_tick = 0;

	for_cnt = Shm_Risk[0].Ho_Chk[market_gbn][0].hoga_depth - 1;

	/* 주문가격 Tick 구하기 */
	for (i = 0; i <= for_cnt; i++)
	{
		if ((order_price / Shm_Risk[0].Ho_Chk[market_gbn][i].band_price >= 1)   ||
			(order_price / Shm_Risk[0].Ho_Chk[market_gbn][i].band_price <= -1))
		{
			/* 호가적합성, 소수점 2자리까지 있어서 1000으로 곱하고 계산한다. */
			if (fmod(order_price*1000, Shm_Risk[0].Ho_Chk[market_gbn][i].band_unit*1000))
			{
#if 0
Log는 테스트후 막는다.
				Log (USR_OK, "market_gbn[%d] i[%d] order_price[%lf] Ho_Chkband_unit[%lf] / [%lf] band_price[%lf]", market_gbn, i, order_price, Shm_Risk[0].Ho_Chk[market_gbn][i].band_unit, order_price / Shm_Risk[0].Ho_Chk[market_gbn][i].band_price, Shm_Risk[0].Ho_Chk[market_gbn][i].band_price);
				Log (USR_OK, "CK [%lf] ",fmod(order_price*1000, Shm_Risk[0].Ho_Chk[market_gbn][i].band_unit*1000));
				Log (USR_OK, "CK2 [%lf] [%lf]",(order_price*1000), (Shm_Risk[0].Ho_Chk[market_gbn][i].band_unit*1000));
#endif
				return  (-2);
			}

			/* 주문가격의 Tick */
			order_price_tick = (int)((order_price - Shm_Risk[0].Ho_Chk[market_gbn][i].band_price) / Shm_Risk[0].Ho_Chk[market_gbn][i].band_unit);
			order_price_tick += (int)Shm_Risk[0].Ho_Chk[market_gbn][i].band_sum;
//Log (USR_OK, "order_price[%lf] tick[%d]", order_price, order_price_tick);
			break;
		}

		if (i >= for_cnt)
		{
			order_price_tick = -1;
		}
	}

	/* 현재가 Tick 구하기 */
	for (i = 0; i <= for_cnt; i++)
	{
		if ((curr_price / Shm_Risk[0].Ho_Chk[market_gbn][i].band_price >= 1)	||
			(curr_price / Shm_Risk[0].Ho_Chk[market_gbn][i].band_price <= -1))
		{
			/* 주문가격의 Tick */
			curr_price_tick = (int)((curr_price - Shm_Risk[0].Ho_Chk[market_gbn][i].band_price) / Shm_Risk[0].Ho_Chk[market_gbn][i].band_unit);
			curr_price_tick += (int)Shm_Risk[0].Ho_Chk[market_gbn][i].band_sum;
//Log (USR_OK, "curr_price[%lf] tick[%d]", curr_price, curr_price_tick);
			break;
		}


		if (i >= for_cnt)
		{
			curr_price_tick = -1; 
		}
	}

	return (abs(curr_price_tick - order_price_tick));	
}	/* End of Tick_Chk ()	*/

/*************************************************************************
	Function		: . JS_Tick_Chk
	Parameters IN	: . 
	Parameters OUT	: .
	Return Code		: . int (0:success, 1:timeout, -1:failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		JS_Tick_Chk (int market_gbn, double standard_price, double curr_price, double order_price)
/*----------------------------------------------------------------------*/
{
	int     i, for_cnt;
    int     order_price_tick = 0, curr_price_tick = 0;

	if (market_gbn != 0 && market_gbn != 3)
	{
		Log (USR_ERROR, "JS_Tick_Chk() market_gbn -1 ERROR [%d]", market_gbn);
		return  (-1);
	}

	for_cnt = Shm_Risk[0].Ho_Chk[market_gbn][0].hoga_depth - 1;

	/* 주문가격 Tick 구하기 */
	for (i = 0; i <= for_cnt; i++)
	{
		if ((standard_price / Shm_Risk[0].Ho_Chk[market_gbn][i].band_price >= 1)   ||
			(standard_price / Shm_Risk[0].Ho_Chk[market_gbn][i].band_price <= -1))
		{
			/* 호가적합성, 소수점 2자리까지 있어서 1000으로 곱하고 계산한다. */
			if (fmod(order_price*1000, Shm_Risk[0].Ho_Chk[market_gbn][i].band_unit*1000))
			{
				Log (USR_ERROR, "JS_Tick_Chk() 호가정합성 -2 ERROR");
				return  (-2);
			}

			return ((abs(order_price - curr_price)*1000) / (Shm_Risk[0].Ho_Chk[market_gbn][i].band_unit*1000));
		}
	}

	Log (USR_ERROR, "JS_Tick_Chk() 기준가호가 -3 ERROR");
	return (-3);
}	/* End of JS_Tick_Chk ()	*/

/*----------------------------------------------------------------------*/
double		Hoga_Change (int market_gbn, int item_seq, int mk_group, int chg_hoga, double order_price)
/*----------------------------------------------------------------------*/
{
	int     i, for_cnt, try;
	double	cal_price;
	
	if (chg_hoga == 0)
		return (order_price);

	if (market_gbn == 3)
	{
		if (mk_group == 1)
			market_gbn = 0;			/* 주식선물 코스닥이면 시장구분을 0으로 변경 */
	}

	for_cnt = Shm_Risk[0].Ho_Chk[market_gbn][0].hoga_depth - 1;

	/* 주문가격 Tick 반영하기(매수) */
	i = try = 0;
	cal_price = order_price;
	
	while (1)
	{
		if (try >= abs(chg_hoga))
			break;

		if (chg_hoga > 0)
		{
			if (i == 0 && cal_price >= Shm_Risk[0].Ho_Chk[market_gbn][i].band_price)
			{
				cal_price = cal_price + Shm_Risk[0].Ho_Chk[market_gbn][i].band_unit;
				try ++;
				continue;
			}
			else if (i < for_cnt)
			{
				if (cal_price <  Shm_Risk[0].Ho_Chk[market_gbn][i].band_price	&&
						 cal_price >= Shm_Risk[0].Ho_Chk[market_gbn][i+1].band_price)
				{
					cal_price = cal_price + Shm_Risk[0].Ho_Chk[market_gbn][i+1].band_price;
					try ++;
					continue;
				}
			}
			else
			{
				if (i >= for_cnt)
				{
					Log (USR_ERROR, "Error01");
					break;
				}
				i++;
			}
		}
		else
		{
			if (i == 0 && cal_price >  Shm_Risk[0].Ho_Chk[market_gbn][i].band_price)
			{
				cal_price = cal_price - Shm_Risk[0].Ho_Chk[market_gbn][i].band_unit;
				try ++;
				continue;
			}
			else if (i < for_cnt)
			{
				if (cal_price <= Shm_Risk[0].Ho_Chk[market_gbn][i].band_price	&&
						 cal_price >  Shm_Risk[0].Ho_Chk[market_gbn][i+1].band_price)
				{
					cal_price = cal_price - Shm_Risk[0].Ho_Chk[market_gbn][i+1].band_price;
					try ++;
					continue;
				}
			}
			else
			{
				if (i >= for_cnt)
				{
					Log (USR_ERROR, "Error02");
					break;
				}
				i++;
			}
		}
	}

	return (cal_price);

}	/* End of Hoga_Change */

/*************************************************************************
	End of Program (hoga_check.c)
*************************************************************************/
