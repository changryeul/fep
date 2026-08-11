
#include "arb.h"
#include "msgq.h"
#include "def.h"

int fut_stat=0, spot_stat=0;

int l_arb_ord_proc()
{
    int    rc  = 0;
	int    gb  = 0;
	//int    now = get_hhmmss_now();
	time_t now = time(NULL);

	// 시세 스냅샷
	rc = l_arb_set_hoga();
	if (rc != 0)
	{
        //l_dbg(L_DBG, "호가이상.. rc=%d", rc );
        return rc; // 호가이상케이스.. 일단은 대기하면서 정상호가 수신을 기다린다. 
	}

    if (now >= G_STRAT->hard_cut_time_t && G_STRAT->hard_cut_enabled == 1 ) {
		gb = TRADE_PHASE_HARD_CUT;
        l_dbg(L_DBG, "[하드컷(최종청산)주문단계] now[%ld] >= hardcut[%ld]", now, G_STRAT->hard_cut_time_t);

    } else if (now >= G_STRAT->close_t2_t && G_STRAT->close_spread_enabled_t2 == 1 ) {
		gb = TRADE_PHASE_EXIT_2;
        l_dbg(L_DBG, "[2차청산주문단계] now[%ld] >= 2차청산[%ld]", now, G_STRAT->close_t2_t);

    } else if (now >= G_STRAT->close_t1_t && G_STRAT->close_spread_enabled_t1 == 1 ) {
		gb = TRADE_PHASE_EXIT_1; 
        l_dbg(L_DBG, "[1차청산주문단계] now[%ld] >= 1차청산[%ld]", now, G_STRAT->close_t1_t);

    } else if (now >= G_STRAT->entry_start_time) {
		gb = TRADE_PHASE_ENTRY;
        l_dbg(L_DBG, "[진입주문단계] now[%ld] >= 진입주문[%ld]", now, G_STRAT->ent_tm_t);
	} 
	else
	{
        l_dbg(L_DBG, "진입시간이 아닙니다. now[%d] < 진입주문[%ld]", now, G_STRAT->ent_tm_t);
		return -1;
	}

	if (fut_stat == 3 && spot_stat == 3) {
		spot_stat = 0;
		fut_stat  = 0;
	}

	// arb_dir_mode => 차익방향구분코드 - 1.매수, 2.매도, 3.양방향 
	if (gb == TRADE_PHASE_ENTRY) {

//		l_dbg(L_DBG, "check>> TRADE_PHASE_ENTRY(BUY): ARB_BOTH>EX_SIMUL> calc_bp[%.5f][%.5f] qty[%d]-[%d]", 
//			G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread), G_STRAT->entry_buy_spread_bp, G_SNAP->qty_fut_bid, G_STRAT->min_lot_fut);
//	    l_dbg(L_DBG, "check>> TRADE_PHASE_ENTRY(SEL): ARB_BOTH>EX_SIMUL> calc_bp[%.5f][%.5f] qty[%d]-[%d]", 
//			(G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask, G_STRAT->entry_sell_spread_bp, G_SNAP->qty_fut_ask, G_STRAT->min_lot_fut);


		/* 진입-현물 매수 (매수차익) */
		if ((G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread) >= G_STRAT->entry_buy_spread_bp) &&
			(abs(G_STRAT->pos_spot) < G_STRAT->max_lot_abs_spot) &&
			(G_SNAP->qty_fut_bid >= G_STRAT->min_lot_fut) && 
			(G_STRAT->arb_dir_mode != ARB_SELL) &&
			(fut_stat == 0 && spot_stat == 0)) {

			if (G_STRAT->exec_mode == EX_SIMUL) {
				l_dbg(L_ERR, "TRADE_PHASE_ENTRY(BUY): ARB_BOTH>EX_SIMUL> calc_bp[%.5f][%.5f] qty[%d]-[%d]", 
					G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread), G_STRAT->entry_buy_spread_bp, G_SNAP->qty_fut_bid, G_STRAT->min_lot_fut);
				fut_stat  = 1;
				spot_stat = 1;
				l_add_wqlist_fut (G_STRAT->ord_type_fut,  G_STRAT->base_lot_fut , -1, G_STRAT->tif_type_fut,  G_SNAP->px_fut_bid - G_STRAT->tick_offset);
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot,  1, G_STRAT->tif_type_spot, G_SNAP->px_spot_ask);
			} else
			if (G_STRAT->exec_mode == EX_FUT_FIRST) {
				l_dbg(L_ERR, "TRADE_PHASE_ENTRY(BUY): ARB_BOTH>EX_FUT_FIRST> calc_bp[%.5f][%.5f] qty[%d]-[%d]", 
					G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread), G_STRAT->entry_buy_spread_bp, G_SNAP->qty_fut_bid, G_STRAT->min_lot_fut);
				fut_stat = 1;
				l_add_wqlist_fut(G_STRAT->ord_type_fut, G_STRAT->base_lot_fut, -1, G_STRAT->tif_type_fut, G_SNAP->px_fut_bid - G_STRAT->tick_offset);
			}
			if (G_STRAT->exec_mode == EX_SPOT_FIRST) {
				l_dbg(L_ERR, "TRADE_PHASE_ENTRY(BUY): ARB_BOTH>EX_SPOT_FIRST> calc_bp[%.5f][%.5f] qty[%d]-[%d]", 
					G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread), G_STRAT->entry_buy_spread_bp, G_SNAP->qty_fut_bid, G_STRAT->min_lot_fut);
				spot_stat = 1;
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot,  1, G_STRAT->tif_type_spot, G_SNAP->px_spot_ask);
			}
		}
		
		/* 진입-현물 매도 (매도차익)*/
		if (((G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask >= G_STRAT->entry_sell_spread_bp) &&
			(abs(G_STRAT->pos_spot) < G_STRAT->max_lot_abs_spot) &&
			(G_SNAP->qty_fut_ask >= G_STRAT->min_lot_fut) && 
			(G_STRAT->arb_dir_mode != ARB_BUY) &&
			(fut_stat == 0 && spot_stat == 0)) {
			if (G_STRAT->exec_mode == EX_SIMUL) {
				l_dbg(L_ERR, "TRADE_PHASE_ENTRY(SEL): ARB_BOTH>EX_SIMUL> calc_bp[%.5f]-[%.5f] qty[%d]-[%d]", 
					(G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask, G_STRAT->entry_sell_spread_bp, G_SNAP->qty_fut_ask, G_STRAT->min_lot_fut);
				fut_stat  = 1;
				spot_stat = 1;
				l_add_wqlist_fut (G_STRAT->ord_type_fut,  G_STRAT->base_lot_fut,   1, G_STRAT->tif_type_fut,  G_SNAP->px_fut_ask + G_STRAT->tick_offset );
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot, -1, G_STRAT->tif_type_spot, G_SNAP->px_spot_bid);
			} else
			if (G_STRAT->exec_mode == EX_FUT_FIRST) {
				l_dbg(L_ERR, "TRADE_PHASE_ENTRY(SEL): ARB_BOTH>EX_FUT_FIRST> calc_bp[%.5f][%.5f] qty[%d]-[%d]", 
					(G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask, G_STRAT->entry_sell_spread_bp, G_SNAP->qty_fut_ask, G_STRAT->min_lot_fut);
				fut_stat = 1;
				l_add_wqlist_fut(G_STRAT->ord_type_fut, G_STRAT->base_lot_fut, 1, G_STRAT->tif_type_fut, G_SNAP->px_fut_ask + G_STRAT->tick_offset);
			}
			if (G_STRAT->exec_mode == EX_SPOT_FIRST) {
				l_dbg(L_ERR, "TRADE_PHASE_ENTRY(SEL): ARB_BOTH>EX_SPOT_FIRST> calc_bp[%.5f][%.5f] qty[%d]-[%d]", 
					(G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask, G_STRAT->entry_sell_spread_bp, G_SNAP->qty_fut_ask, G_STRAT->min_lot_fut);
				spot_stat = 1;
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot, -1, G_STRAT->tif_type_spot, G_SNAP->px_spot_bid);
			}
		}

		/* 청산-현물 매수 (매수차익) */
		if ((G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread) >= G_STRAT->exit_spread_bp) &&
			(G_STRAT->pos_spot < 0) &&
			(G_SNAP->qty_fut_bid >= G_STRAT->min_lot_fut) && 
			(fut_stat == 0 && spot_stat == 0)) {

			if (G_STRAT->exec_mode == EX_SIMUL) {
				l_dbg(L_ERR, "TRADE_PHASE_ENTRY_EXIT(BUY): pos! both---> calc_bp[%.5f][%.5f] qty[%d]-[%d]", 
					G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread), G_STRAT->exit_spread_bp, G_SNAP->qty_fut_bid, G_STRAT->min_lot_fut);
				fut_stat  = 1;
				spot_stat = 1;
				l_add_wqlist_fut (G_STRAT->ord_type_fut,  G_STRAT->base_lot_fut,  -1, G_STRAT->tif_type_fut,  G_SNAP->px_fut_bid - G_STRAT->tick_offset);
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot,  1, G_STRAT->tif_type_spot, G_SNAP->px_spot_ask);

			} else 
			if (G_STRAT->exec_mode == EX_FUT_FIRST) {
        		l_dbg(L_ERR, "TRADE_PHASE_ENTRY_EXIT(BUY): pos! fut fst ---> calc_bp[%.5f][%.5f] qty[%d]-[%d]", 
					G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread), G_STRAT->exit_spread_bp, G_SNAP->qty_fut_bid, G_STRAT->min_lot_fut);
				fut_stat = 1;
				l_add_wqlist_fut(G_STRAT->ord_type_fut, G_STRAT->base_lot_fut, -1, G_STRAT->tif_type_fut, G_SNAP->px_fut_bid - G_STRAT->tick_offset);
			}
			if (G_STRAT->exec_mode == EX_SPOT_FIRST) {
        		l_dbg(L_ERR, "TRADE_PHASE_ENTRY_EXIT(BUY): pos! spot fst ---> calc_bp[%.5f][%.5f] qty[%d]-[%d]", 
					G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread), G_STRAT->exit_spread_bp, G_SNAP->qty_fut_bid, G_STRAT->min_lot_fut);
				spot_stat = 1;
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot,  1, G_STRAT->tif_type_spot, G_SNAP->px_spot_ask);
			}
		}

		/* 청산-현물 매수 (매수차익) */
		if (((G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask >= G_STRAT->exit_spread_bp) &&
			(G_STRAT->pos_spot > 0) &&
			(G_SNAP->qty_fut_ask >= G_STRAT->min_lot_fut) && 
			(fut_stat == 0 && spot_stat == 0)) {

			if (G_STRAT->exec_mode == EX_SIMUL) {
        		l_dbg(L_ERR, "TRADE_PHASE_ENTRY_EXIT(SEL): pos! both---> calc_bp[%.5f][%.5f] qty[%d]-[%d]", 
					(G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask, G_STRAT->exit_spread_bp, G_SNAP->qty_fut_ask, G_STRAT->min_lot_fut); 
				fut_stat  = 1;
				spot_stat = 1;
				l_add_wqlist_fut (G_STRAT->ord_type_fut,  G_STRAT->base_lot_fut,   1, G_STRAT->tif_type_fut,  G_SNAP->px_fut_ask + G_STRAT->tick_offset);
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot, -1, G_STRAT->tif_type_spot, G_SNAP->px_spot_bid);

			} else 
			if (G_STRAT->exec_mode == EX_FUT_FIRST) {
        		l_dbg(L_ERR, "TRADE_PHASE_ENTRY_EXIT(SEL): pos! fut fst ---> calc_bp[%.5f][%.5f] qty[%d]-[%d]", 
					(G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask, G_STRAT->exit_spread_bp, G_SNAP->qty_fut_ask, G_STRAT->min_lot_fut); 
				fut_stat = 1;
				l_add_wqlist_fut(G_STRAT->ord_type_fut, G_STRAT->base_lot_fut, 1, G_STRAT->tif_type_fut, G_SNAP->px_fut_ask + G_STRAT->tick_offset);
			}
			if (G_STRAT->exec_mode == EX_SPOT_FIRST) {
        		l_dbg(L_ERR, "TRADE_PHASE_ENTRY_EXIT(SEL): pos! spot fst ---> calc_bp[%.5f][%.5f] qty[%d]-[%d]", 
					(G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask, G_STRAT->exit_spread_bp, G_SNAP->qty_fut_ask, G_STRAT->min_lot_fut); 
				spot_stat = 1;
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot, -1, G_STRAT->tif_type_spot, G_SNAP->px_spot_bid);
			}
		}

	} else
	if (gb == TRADE_PHASE_EXIT_1) {
		if (G_STRAT->pos_spot == 0)	{
			l_dbg(L_ERR, "TRADE_PHASE_EXIT_1 : No positions to close pos_spot[%d] pos_fut[%d]", G_STRAT->pos_spot, G_STRAT->pos_fut);
			if (arb_stop_msg_send(E5101_MSG) < 0) // 5101 청산 대상 포지션 없음 l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
				return ARB_STOP_CD;
			return(ARB_STOP_CD);
		}

		if ((G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread) >= G_STRAT->close_spread_bp_t1) &&
			(G_STRAT->pos_spot < 0) &&
			(G_SNAP->qty_fut_bid >= G_STRAT->min_lot_fut) && 
			(fut_stat == 0 && spot_stat == 0)) {

			if (G_STRAT->exec_mode == EX_SIMUL) {
        		l_dbg(L_ERR, "TRADE_PHASE_EXIT_1(BUY): pos! both---> calc_bp[%.5f][%.5f] qty[%d]-[%d]",
					 G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread), G_STRAT->close_spread_bp_t1, G_SNAP->qty_fut_bid, G_STRAT->min_lot_fut);
				fut_stat  = 1;
				spot_stat = 1;
				l_add_wqlist_fut (G_STRAT->ord_type_fut,  G_STRAT->base_lot_fut,  -1, G_STRAT->tif_type_fut,  G_SNAP->px_fut_bid - G_STRAT->tick_offset);
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot,  1, G_STRAT->tif_type_spot, G_SNAP->px_spot_ask);
			} else 
			if (G_STRAT->exec_mode == EX_FUT_FIRST) {
        		l_dbg(L_ERR, "TRADE_PHASE_EXIT_1(BUY): pos! fut fst ---> calc_bp[%.5f][%.5f] qty[%d]-[%d]",
					G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread), G_STRAT->close_spread_bp_t1, G_SNAP->qty_fut_bid, G_STRAT->min_lot_fut);
				fut_stat = 1;
				l_add_wqlist_fut(G_STRAT->ord_type_fut, G_STRAT->base_lot_fut, -1, G_STRAT->tif_type_fut, G_SNAP->px_fut_bid - G_STRAT->tick_offset);
			} else 
			if (G_STRAT->exec_mode == EX_SPOT_FIRST) {
        		l_dbg(L_ERR, "TRADE_PHASE_EXIT_1(BUY): pos! spt fst ---> calc_bp[%.5f][%.5f] qty[%d]-[%d]",
					G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread), G_STRAT->close_spread_bp_t1, G_SNAP->qty_fut_bid, G_STRAT->min_lot_fut);
				spot_stat = 1;
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot,  1, G_STRAT->tif_type_spot, G_SNAP->px_spot_ask);
			}
		}

		if (((G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask >= G_STRAT->close_spread_bp_t1) &&
			(G_STRAT->pos_spot > 0) &&
			(G_SNAP->qty_fut_ask >= G_STRAT->min_lot_fut) && 
			(fut_stat == 0 && spot_stat == 0)) {

			if (G_STRAT->exec_mode == EX_SIMUL) {
        		l_dbg(L_ERR, "TRADE_PHASE_EXIT_1(SEL): pos! both ---> calc_bp[%.5f][%.5f] qty[%d]-[%d]",
					(G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask, G_STRAT->close_spread_bp_t1, G_SNAP->qty_fut_ask, G_STRAT->min_lot_fut);
				fut_stat  = 1;
				spot_stat = 1;
				l_add_wqlist_fut (G_STRAT->ord_type_fut,  G_STRAT->base_lot_fut,   1, G_STRAT->tif_type_fut,  G_SNAP->px_fut_ask + G_STRAT->tick_offset);
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot, -1, G_STRAT->tif_type_spot, G_SNAP->px_spot_bid);

			} else 
			if (G_STRAT->exec_mode == EX_FUT_FIRST) {
        		l_dbg(L_ERR, "TRADE_PHASE_EXIT_1(SEL): pos! fut fst ---> calc_bp[%.5f][%.5f] qty[%d]-[%d]",
					(G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask, G_STRAT->close_spread_bp_t1, G_SNAP->qty_fut_ask, G_STRAT->min_lot_fut);
				fut_stat = 1;
				l_add_wqlist_fut(G_STRAT->ord_type_fut, G_STRAT->base_lot_fut, 1, G_STRAT->tif_type_fut, G_SNAP->px_fut_ask + G_STRAT->tick_offset);
			} else 
			if (G_STRAT->exec_mode == EX_FUT_FIRST) {
        		l_dbg(L_ERR, "TRADE_PHASE_EXIT_1(SEL): pos! spt fst ---> calc_bp[%.5f][%.5f] qty[%d]-[%d]",
					(G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask, G_STRAT->close_spread_bp_t1, G_SNAP->qty_fut_ask, G_STRAT->min_lot_fut);
				spot_stat = 1;
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot, -1, G_STRAT->tif_type_spot, G_SNAP->px_spot_bid);
			}

		}

	} else
	if (gb == TRADE_PHASE_EXIT_2) {
		if (G_STRAT->pos_spot == 0) {
			l_dbg(L_ERR, "TRADE_PHASE_EXIT_2 : No positions to close pos_spot[%d] pos_fut[%d]", G_STRAT->pos_spot, G_STRAT->pos_fut);
			if (arb_stop_msg_send(E5101_MSG) < 0) // 5101 청산 대상 포지션 없음 l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
				return ARB_STOP_CD;
			return(ARB_STOP_CD);
		}
		
		if ((G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread) >= G_STRAT->close_spread_bp_t2) &&
			(G_STRAT->pos_spot < 0) &&
			(G_SNAP->qty_fut_bid >= G_STRAT->min_lot_fut) && 
			(fut_stat == 0 && spot_stat == 0)) {

			if (G_STRAT->exec_mode == EX_SIMUL) {
        		l_dbg(L_ERR, "TRADE_PHASE_EXIT_2(BUY): pos! both---> calc_bp[%.5f][%.5f] qty[%d]-[%d]",
					G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread), G_STRAT->close_spread_bp_t2, G_SNAP->qty_fut_bid, G_STRAT->min_lot_fut);
				fut_stat  = 1;
				spot_stat = 1;
				l_add_wqlist_fut (G_STRAT->ord_type_fut,  G_STRAT->base_lot_fut,  -1, G_STRAT->tif_type_fut,  G_SNAP->px_fut_bid - G_STRAT->tick_offset);
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot,  1, G_STRAT->tif_type_spot, G_SNAP->px_spot_ask);

			} else 
			if (G_STRAT->exec_mode == EX_FUT_FIRST) {
        		l_dbg(L_ERR, "TRADE_PHASE_EXIT_2(BUY): pos! fut fst ---> calc_bp[%.5f][%.5f] qty[%d]-[%d]",
					G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread), G_STRAT->close_spread_bp_t2, G_SNAP->qty_fut_bid, G_STRAT->min_lot_fut);
				fut_stat = 1;
				l_add_wqlist_fut(G_STRAT->ord_type_fut, G_STRAT->base_lot_fut, -1, G_STRAT->tif_type_fut, G_SNAP->px_fut_bid - G_STRAT->tick_offset);
			} else 
			if (G_STRAT->exec_mode == EX_SPOT_FIRST) {
        		l_dbg(L_ERR, "TRADE_PHASE_EXIT_2(BUY): pos! spt fst ---> calc_bp[%.5f][%.5f] qty[%d]-[%d]",
					G_SNAP->px_fut_bid - (G_SNAP->px_spot_ask + G_STRAT->basis_spread), G_STRAT->close_spread_bp_t2, G_SNAP->qty_fut_bid, G_STRAT->min_lot_fut);
				spot_stat = 1;
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot,  1, G_STRAT->tif_type_spot, G_SNAP->px_spot_ask);
			}
		}

		if (((G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask >= G_STRAT->close_spread_bp_t1) &&
			(G_STRAT->pos_spot > 0) &&
			(G_SNAP->qty_fut_ask >= G_STRAT->min_lot_fut) && 
			(fut_stat == 0 && spot_stat == 0)) {

			if (G_STRAT->exec_mode == EX_SIMUL) {
        		l_dbg(L_ERR, "TRADE_PHASE_EXIT_2(SEL): pos! both ---> calc_bp[%.5f][%.5f] qty[%d]-[%d]",
					(G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask, G_STRAT->close_spread_bp_t2, G_SNAP->qty_fut_ask, G_STRAT->min_lot_fut);
				fut_stat  = 1;
				spot_stat = 1;
				l_add_wqlist_fut (G_STRAT->ord_type_fut,  G_STRAT->base_lot_fut,   1, G_STRAT->tif_type_fut,  G_SNAP->px_fut_ask + G_STRAT->tick_offset);
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot, -1, G_STRAT->tif_type_spot, G_SNAP->px_spot_bid);
			} else 
			if (G_STRAT->exec_mode == EX_FUT_FIRST) {
        		l_dbg(L_ERR, "TRADE_PHASE_EXIT_2(SEL): pos! fut fst ---> calc_bp[%.5f][%.5f] qty[%d]-[%d]",
					(G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask, G_STRAT->close_spread_bp_t2, G_SNAP->qty_fut_ask, G_STRAT->min_lot_fut);
				fut_stat = 1;
				l_add_wqlist_fut(G_STRAT->ord_type_fut, G_STRAT->base_lot_fut, 1, G_STRAT->tif_type_fut, G_SNAP->px_fut_ask + G_STRAT->tick_offset);
			} else 
			if (G_STRAT->exec_mode == EX_SPOT_FIRST) {
        		l_dbg(L_ERR, "TRADE_PHASE_EXIT_2(SEL): pos! spt fst ---> calc_bp[%.5f][%.5f] qty[%d]-[%d]",
					(G_SNAP->px_spot_bid + G_STRAT->basis_spread) - G_SNAP->px_fut_ask, G_STRAT->close_spread_bp_t2, G_SNAP->qty_fut_ask, G_STRAT->min_lot_fut);
				spot_stat = 1;
				l_add_wqlist_spot(G_STRAT->ord_type_spot, G_STRAT->base_lot_spot, -1, G_STRAT->tif_type_spot, G_SNAP->px_spot_bid);
			}
		}
	} else
	if (gb == TRADE_PHASE_HARD_CUT) { 
		
		if (G_STRAT->pos_spot == 0) {
			l_dbg(L_ERR, "TRADE_PHASE_HARD_CUT : No positions to hard_cut pos_spot[%d] pos_fut[%d]", G_STRAT->pos_spot, G_STRAT->pos_fut);
			if (arb_stop_msg_send(E5101_MSG) < 0) // 5101 청산 대상 포지션 없음 l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
				return ARB_STOP_CD;
			return ARB_STOP_CD;
		}
	
		if (G_STRAT->pos_spot > 0) {
            l_dbg(L_ERR, "TRADE_PHASE_HARD_CUT: pos! spot ask, fut bid--->");
			fut_stat = 1;
			spot_stat = 1;
            l_add_wqlist_fut (0/* market */,          abs(G_STRAT->pos_fut ),   1, G_STRAT->tif_type_fut , G_SNAP->px_fut_ask);
            l_add_wqlist_spot(G_STRAT->ord_type_spot, abs(G_STRAT->pos_spot),  -1, G_STRAT->tif_type_spot, G_SNAP->px_spot_bid);
        }

        if (G_STRAT->pos_spot < 0) {
            l_dbg(L_ERR, "TRADE_PHASE_HARD_CUT: pos! spot bid, fut ask--->");
			fut_stat = 1;
			spot_stat = 1;
            l_add_wqlist_fut (0/* market */,          abs(G_STRAT->pos_fut ),  -1, G_STRAT->tif_type_fut,  G_SNAP->px_fut_bid);
            l_add_wqlist_spot(G_STRAT->ord_type_spot, abs(G_STRAT->pos_spot),   1, G_STRAT->tif_type_spot, G_SNAP->px_spot_ask);
        }
	}

	return 0;
}

int l_arb_proc()
{
    int rc, mode, ii;
	//int now = get_hhmmss_now();
	time_t now = time(NULL);

    if (g_initialized != 1)
    {
		l_dbg(L_ERR, "** 전략 초기화 실패로 전략 재기동 필요함**");
		if (arb_stop_msg_send(E9999_MSG) < 0) // 9999 system error l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
        	l_dbg(L_ERR, "Alarm send error..");
		return ARB_STOP_CD;
	}

	if (now >= G_STRAT->end_tm_t) // 전략자동종료 로직 추가 
	{
        l_dbg(L_ERR, "[전략자동종료] now[%ld] >= end_time[%ld]", now, G_STRAT->end_tm_t);
		if (arb_stop_msg_send(E5002_MSG) < 0) // 현선물차익거래 전략 자동 종료되었습니다. 
        	l_dbg(L_ERR, "Alarm send error..");
		return ARB_STOP_CD;
	}

	// 새 세트번호로 진입
	if (G_SET_CNT == 0) // 최초진입시 셋팅 
    {
        G_SET_CNT     = 1;
        G_SET->set_id = 1;
		G_SET->valid  = 1; 
    	l_dbg(L_INF, "[SETID] 세트번호 채번 g_set_idx[%d] set_id[%d] fut_stat[%d] spot_stat[%d]", g_set_idx, G_SET->set_id, fut_stat, spot_stat);
    }

	if ((abs(POS_FUT)  == G_STRAT->base_lot_fut) &&  
		(abs(POS_SPOT) == G_STRAT->base_lot_spot)) // 세트잔고와 기본수량이 같으면 세트 주문이 정상적으로 나간것이니 다음 주문 가능
	{
		mode = 1;
	} 
	else if (POS_FUT == 0 && POS_SPOT == 0)        // 둘다 0 이므로 신규 진입 가능
	{
		mode = 1;

	} else	
		mode = 2;


	if (mode == 1)
    {
		if (G_SET->valid != 1)
		{
			l_dbg(L_INF, "[%d] Set가 끝나지 않아 신규 진입 대기 valid[%d]", g_set_idx, G_SET->valid);
			return -1;
		}

		/* 해당 주문로그를 확인해서 해당주문set이 완료되지 않은 주문을 확인 */
		SetOrdLog *log = &G_SET->ordlog;
		for (ii=0; ii < log->n; ii++)
		{
			 SetOrdLogEnt *oent = &log->ent[ii]; 
			 l_dbg(L_INF, "leg=(%d) ord_id=(%ld) ordtype=(%d), side=(%d), qty=(%d),fill_qty(%d) time=(%d)", 
				oent->leg, oent->ord_id, oent->ordtype, oent->side, oent->qty, oent->filled_qty, oent->ts_hhmmss);
			 if (oent->valid == 1)
			 {
				 //l_dbg(L_INF, "[%d]Set 완료되지 않은 주문 존재해서 진입 Skip ...", g_set_idx);
				 l_dbg(L_INF, "leg=(%d) ord_id=(%ld) ordtype=(%d), side=(%d), qty=(%d),fill_qty(%d) time=(%d)", 
					oent->leg, oent->ord_id, oent->ordtype, oent->side, oent->qty, oent->filled_qty, oent->ts_hhmmss);
				 return -1;
			 }
		}

        rc = l_arb_ord_proc();
		if (rc != 0)
		{
            //l_dbg(L_INF,  "진입 주문 대기 ....rc = (%d) ", rc );
			return rc;
		}
	}
	else // 한쪽만 포지션이 있는 상태.. 실시간으로 체결 응답이 추가적으로 들어올 수 있으므로 대기한다. 
	{
	 	l_dbg(L_INF, "[%d]set position 불균형 상태로 다음 세트 진입 대기 fut[%d]spot[%d]", g_set_idx, POS_FUT, POS_SPOT); 
		return -1;
	}

    return 0;

}
