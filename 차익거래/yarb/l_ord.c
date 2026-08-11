
#include "def.h"

// ------------------------------------------------------------
//  선물 주문/체결 응답처리
// ------------------------------------------------------------
int l_arb_fut_exec(int gb, int side, long ord_id, long fill_qty, double fill_px)
{
    int rc =0;
    FillEvent fill;
	int wait_ms    = 0;

	// 선물 주문 Flow by.kong 
	// case 1. 양방향 주문(시장가) : 선물, 현물중 어느것이 먼저 체결될지 모른다.
	// 선물 체결이 들어왔다면, 선물과 현물 세트 수량이 일치하는지 확인한다. 일치하면 해당 세트는 완료 된다.
	// 거부가 되었으면, 거부 케이스를 확인한다. 시장가/FAS 주문이므로 Auto Cancel은 발생하지 않을것. 거래소 거부이므로 전략 종료
	// case 2. 선물우선주문(시장가/지정가) : 선물 체결 후 현물 주문을 전송한다 -> 선물 지정가주문은 체결안될수도 있음
	// 이 경우에는 해당 세트를 무효화(완료) 처리한 후 다음 세트로 진행한다. -> 선물 응답에서 진행
	// 선물 체결이 들어왔다면, 현물 주문을 낸다.

    memset(&fill, 0x00, sizeof(FillEvent));

	// 현세트에 대한 주문임을 확인한다. 
	if (G_ORDNO->fut_ordno != ord_id)
		return 0;

	if (gb == 1) // 체결일때 
	{
		G_SET->fut_reject_stat  = 1; // 체결응답 
		G_SET->fut_fill_stat = 1; // 체결응답 

		fill.ord_id = ord_id;
		if (side == 1) // 1-매도 (선물**방향주의)
			fill.side = -1;
		else           // 2-매수 (선물**방향주의)
			fill.side = 1;
		fill.fill_px  = fill_px;
		fill.fill_qty = fill_qty;
		
		l_dbg(L_ERR, "[%s] 선물 체결 발생 ord_id=%ld side=%d qty=%d px=%.2f", __FUNCTION__, ord_id, side, fill_qty, fill_px);
		rc = l_arb_fut_fill(&fill);
		if (rc !=0)
		{
			l_dbg(L_ERR, "fut 체결 l_arb_fut_fill.. continue. rc=%d",rc);
			return rc;
		}
		
		if (G_SET->spot_reject_stat == -1) // 현물이 거부응답 받은 상태라면..
		{
			if (G_SET->real_retry_cnt < G_STRAT->retry_cnt)
			{
				// 선물주문이 체결된 상태이므로 현물주문 retry
				if (POS_FUT < 0)
					fill.side = 1;  //선물매도주문이 나간 상태이므로 현물매수주문
				else
					fill.side = -1; //선물매수주문이 나간 상태이므로 현물매도주문

			   // 시간체크 추가
			    int now = get_hhmmss_now();
			    if (now >= G_STRAT->hard_cut_time)
				{
				    fill.fill_qty  = abs(G_STRAT->pos_spot);
					l_dbg(L_INF, "[SPOT_RETRY] 현물 주문 하드컷 수량[%d]",fill.fill_qty);
			    }
				else
					fill.fill_qty  = G_STRAT->base_lot_spot;
				
				G_SET->spot_reject_stat = 0; // 미응답상태로 clear..
				wait_ms = G_STRAT->retry_interval_ms;
				if (wait_ms > 0)
				{
					l_dbg(L_ERR, "[SPOT_RETRY] retry_cnt = %d wait=%d ms before re-order", G_STRAT->retry_cnt, wait_ms);
					usleep(wait_ms * 1000);
					l_dbg(L_ERR, "[SPOT_RETRY] wake..");
				}
				rc = l_arb_spot_ord(&fill);
				if (rc != 0)
				{
					l_dbg(L_ERR, "[SPOT_RETRY] 현물 재주문 l_arb_spot_ord error.. rc=%d",rc);
					return rc ;
				}
				G_SET->real_retry_cnt ++; // 재시도 횟수 증가 - 세트메모리에 관리. 실제 사용 카운트
				return 0;
			}
			else
			{
				l_dbg(L_ERR, "[SPOT_RETRY] 현물 재주문 시도 초과 (max=%d). ARB 전략 중단!!", G_STRAT->retry_cnt);
				if (arb_stop_msg_send(E5103_MSG) < 0) // 5103 재시도 횟수 초과 
					l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
				return ARB_STOP_CD;
			}
		}
	}
	else if (gb == -1)  // 거부일때 (거래소 거부 Case 로 판단.) 
	{
		G_SET->fut_reject_stat  = -1; // 거부응답 
		G_SET->fut_fill_stat = 0; // 미체결

		if (POS_SPOT != 0 && POS_FUT == 0)
		{	
			l_dbg(L_ERR, "(선물[fut=%ld])주문 거부상태.. 거부사유 확인 요망 ARB 전략 중단!!", G_ORDNO->fut_ordno);
			if (arb_stop_msg_send(E5201_MSG) < 0) // 5201 선물주문거부 
				l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
			return ARB_STOP_CD;
		}
		else
		{
			 // 전략 중지시키고 alarm 
			 if (G_SET->spot_reject_stat == -1) // 현물도 거부상태일때..
			 {
			 	 l_dbg(L_ERR, "현물[%ld]/선물[%ld]거부 상태 Set 무효화.. ARB 전략 중단!!", G_ORDNO->spot_ordno, G_ORDNO->fut_ordno);
				 if (arb_stop_msg_send(E5203_MSG) < 0) // 5203 현물선물주문거부 
					l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
			 	 return ARB_STOP_CD;
			 }
			 else if (G_SET->spot_reject_stat == 0) // 현물미응답 상태이나.. Auto Cancle Case가 아니므로 중단.
			 {
				l_dbg(L_ERR, "현물[%ld]미응답/선물[%ld]거부 상태.. 선물거부사유 확인 ARB 전략 중단!!", G_ORDNO->spot_ordno, G_ORDNO->fut_ordno);
				if (arb_stop_msg_send(E5201_MSG) < 0) // 5201 선물주문거부 
					l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
				return ARB_STOP_CD;
			 }
		}
	}
	else if (gb == -2) // KRX Auto Cancel 일경우 - 선물우선(지정가FOK) 주문일때 발생
	{
		// 지정가 FOK 주문일때 선물 취소된 경우에는 해당 세트를 무효화하고 다음세트로 진행한다.
		G_SET->fut_reject_stat  = -2; // Auto Cancel
		G_SET->fut_fill_stat = 0; // 미체결
		
		//  case 1.선물우선이라 현물주문이 안나간 경우 - 다음 세트로 넘겨서 진행
		if (G_SET->spot_reject_stat == 0) 
		{
			 if (G_STRAT->exec_mode == EX_FUT_FIRST)
			 {
				G_SET->valid = 0;
				l_dbg(L_ERR, "[Cancel-Fut] 선물[fut=%ld])주문 Auto Cancel! 현물미완료(%d) 완료..다음 Set 진입", G_ORDNO->fut_ordno, POS_SPOT);
				l_dbg(L_ERR, "[Cancel-Fut] Sleep... [%d]", G_STRAT->order_interval_ms * 1000);
				usleep(G_STRAT->order_interval_ms * 1000);
				l_dbg(L_ERR, "[Cancel-Fut] Wake...");
				// 새 세트 증가
				l_set_id();
				return 0;
			 }
			 else
			 {   // 발생하지 않을 케이스(양방향 : 선물 -시장가(FAS 이므로 발생X))
				 l_dbg(L_ERR, "현물[spot=%ld]미응답/선물[fut=%ld]취소 상태 skip..", G_ORDNO->spot_ordno, G_ORDNO->fut_ordno);
			 	 return 0;
  		   	 }
		}
		else
		{
			l_dbg(L_ERR, "Not supposed to Error.. Arb process Stop..");
			if (arb_stop_msg_send(E9999_MSG) < 0) // system error 
				l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
			return ARB_STOP_CD; 
		}
	}
	else
	{
		l_dbg(L_ERR, "Code Error.. gb [%d].. Arb process Stop..", gb);
		if (arb_stop_msg_send(E9999_MSG) < 0) // system error 
			l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
		return ARB_STOP_CD; 
	}

	return 0;
}

// ------------------------------------------------------------
//  현물 주문/체결 응답 처리
// ------------------------------------------------------------
int l_arb_spot_exec(int gb, int side, long ord_id, long fill_qty, double fill_px)
{
    int rc =0;
	int wait_ms    = 0;
    
	FillEvent fill;

	// 현물 체결시 주문 Flow by.kong 
	// case 1. 양방향 주문(시장가) : 선물, 현물중 어느것이 먼저 체결될지 모른다.
	// 현물 체결이 들어왔다면, 선물과 현물 세트 수량이 일치하는지 확인한다. 일치하면 해당 세트는 완료 된다.
	// 거부가 되었으면 retry 횟수만큼 retry 간격을 두고 재주문한다.
	// 단, 다시 거부응답을 받고나서 진행한다. 응답을 받지 않고 진행하면 의도치 않게 현물 포지션이 확대될 수 있다.
	// case 2. 현물우선주문(지정가) : 현물 체결 후 선물 주문을 전송한다. 현물 거부시에 다음세트로 진행한다. 

    memset(&fill, 0x00, sizeof(FillEvent));

	// 현세트에 대한 주문임을 확인한다. 맞으면 fill ++
	if (G_ORDNO->spot_ordno != ord_id) 
		return 0;
		
	if (gb == 1) // 체결일때 
	{
		G_SET->spot_reject_stat  = 1; // 체결응답 
		G_SET->spot_fill_stat = 1; // 체결

		fill.ord_id = ord_id;
		if (side == 1) // 1-매수 (**현물방향주의)
			fill.side = +1;
		else           // 2-매도 (**현물방향주의)
			fill.side = -1;
	
		fill.fill_px  = fill_px;
		fill.fill_qty = fill_qty;
		
		l_dbg(L_ERR, "현물 체결 발생 [ord_id=%ld side=%d qty=%d px=%.5f]", ord_id, side, fill_qty, fill_px);
		rc = l_arb_spot_fill(&fill);
		if( rc !=0)
		{
			l_dbg(L_ERR, "현물 체결 l_arb_spot_fill.. continue. rc=%d",rc);
			return rc ;
		}

	}
	else if (gb == -1)  // 거부일때 
	{
		G_SET->spot_reject_stat  = -1; // 거부응답 

		l_dbg(L_ERR, "현물 거부 발생 [ord_id=%ld]", ord_id);
		if (POS_FUT != 0 && POS_SPOT == 0)
		{	
			if (G_SET->real_retry_cnt < G_STRAT->retry_cnt)
			{
				// 선물주문이 체결된 상태이므로 현물주문 retry
				if (POS_FUT < 0)
					fill.side = 1;  //선물매도주문이 나간 상태이므로 현물매수주문
				else
					fill.side = -1; //선물매수주문이 나간 상태이므로 현물매도주문

				fill.fill_qty  = G_STRAT->base_lot_spot;
				
				G_SET->spot_reject_stat = 0; // 미응답상태로 clear..
				wait_ms = G_STRAT->retry_interval_ms;
				if (wait_ms > 0)
				{
					l_dbg(L_ERR, "[SPOT_RETRY] retry_cnt = %d wait=%d ms before re-order", G_STRAT->retry_cnt, wait_ms);
					usleep(wait_ms * 1000);
					l_dbg(L_ERR, "[SPOT_RETRY] wake..");
				}
				rc = l_arb_spot_ord(&fill);
				if (rc != 0)
				{
					l_dbg(L_ERR, "[SPOT_RETRY] 현물 재주문 l_arb_spot_ord error.. rc=%d",rc);
					return rc ;
				}
				G_SET->real_retry_cnt++; // 재시도 횟수 증가 - 세트메모리에 관리. 실제 사용 카운트
				return 0;
			}
			else
			{
				l_dbg(L_ERR, "[SPOT_RETRY] 현물 재주문 시도 횟수초과 (max=%d). ARB 전략 중단!!", G_STRAT->retry_cnt);
				if (arb_stop_msg_send(E5103_MSG) < 0) // 5103 현물재시도횟수 초과 
					l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
				return ARB_STOP_CD;
			}
		}
		else
		{
			 // 현물우선주문 거부시 다음세트로 진행한다.
             if (G_STRAT->exec_mode == EX_SPOT_FIRST && POS_FUT == 0 && POS_SPOT == 0)
			 {
			    G_SET->valid = 0;
                l_dbg(L_ERR, "[Cancel-Spot] 현물우선(spot=%ld])ord reject! Next Set start..", G_ORDNO->spot_ordno);
                l_dbg(L_ERR, "[Cancel-Spot] Sleep... [%d]", G_STRAT->order_interval_ms * 1000);
                usleep(G_STRAT->order_interval_ms * 1000);
                l_dbg(L_ERR, "[Cancel-Spot] Wake...");
                // 새 세트 증가
                l_set_id();
                return 0;
			 }

 			 if (G_SET->fut_reject_stat == -1) // 선물도 거부상태
			 {
			 	l_dbg(L_ERR, "현물[spot=%ld]거부/선물[fut=%ld] 거부상태. ARB 전략 중단!!", G_ORDNO->spot_ordno, G_ORDNO->fut_ordno);
				if (arb_stop_msg_send(E5203_MSG) < 0) // 5203 현물선물주문거부  
					l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
			 	 return ARB_STOP_CD;
			 }
			 else if (G_SET->fut_reject_stat == 0) // 선물미응답 상태
			 {
			 	l_dbg(L_ERR, "현물[spot=%ld]거부/선물[fut=%ld] 주문 미응답 상태", G_ORDNO->spot_ordno, G_ORDNO->fut_ordno);
				return 0; // 일단 나간다..
			 }
 			 else if (G_SET->fut_reject_stat == -2) // 선물 Auto Cancel
			 									    // 발생불가한 케이스 - 선물우선주문에서만 Auto Cancel 발생하고, 
													// Auto Cancel 시 현물주문을 내지 않고 다음 세트로 넘어가기 때문에 발생하지 않을 케이스다.)
			 {
			 	l_dbg(L_ERR, "현물[spot=%ld]거부/선물[fut=%ld] Auto Cancel 상태. ARB 전략 중단!!", G_ORDNO->spot_ordno, G_ORDNO->fut_ordno);
				if (arb_stop_msg_send(E5203_MSG) < 0) // 5203 현물선물주문거부  
					l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
			 	return ARB_STOP_CD;
			 }
		}
	}
	else
	{
	 	l_dbg(L_ERR, "Code Error.. gb [%d].. Arb process Stop..", gb);
		if (arb_stop_msg_send(E9999_MSG) < 0) // system error 
			l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
		return ARB_STOP_CD; 
	}

	return 0;
}

// spot 주문 
int l_arb_spot_ord(FillEvent *fill)
{
    int     rc = 0;
    // 대응 현물 수량 설정
    int     spot_side = fill->side;
    int     spot_qty  = fill->fill_qty;
	double  spot_px = 0;

    l_dbg(L_INF, "[FILL] 선물 체결[fill:%d] 완료! 대응 현물 주문 side[%d], spot_qty[%d]", POS_FUT, spot_side, spot_qty);
    // 시세 다시 읽기
	rc = l_arb_set_hoga();
	if (rc != 0)
	{
    	l_dbg(L_ERR, "l_arb_set_hoga.. 호가이상..");
		return rc;
	}
	
    if (spot_side > 0)
	 	spot_px = G_SNAP->px_spot_ask; // 방향:매수 - 매도호가
	else
		spot_px = G_SNAP->px_spot_bid; // 방향:매도 - 매수호가

	
	int ord_type = ORD_TYPE_L; // 현물은 지정가 밖에 없음

    // 대응주문 : 현물 진입주문 전송
	l_add_wqlist_spot(ord_type, spot_qty, spot_side, G_STRAT->tif_type_spot, spot_px);
    l_dbg(L_INF, "[현물 주문] 전송 완료 (선물 FILL 대응)");
	spot_stat = 1;
	
	return 0;
}

// fut 주문 
int l_arb_fut_ord(FillEvent *fill)
{
    int     rc = 0;
    // 대응 현물 수량 설정
    int     fut_side = fill->side;
    int     fut_qty  = fill->fill_qty;
	double  fut_px   = 0;

    l_dbg(L_INF, "[FILL] 현물 체결[fill:%d] 완료! 대응 선물 주문 side[%d], spot_qty[%d]", POS_FUT, fut_side, fut_qty);
    // 시세 다시 읽기
	rc = l_arb_set_hoga();
	if (rc != 0)
	{
    	l_dbg(L_ERR, "l_arb_set_hoga.. 호가이상..");
		return rc;
	}
	
    if (fut_side > 0)
	 	fut_px = G_SNAP->px_fut_ask + G_STRAT->tick_offset ; // 방향:매수 - 매도호가
	else
		fut_px = G_SNAP->px_fut_bid - G_STRAT->tick_offset ; // 방향:매도 - 매수호가

	int ord_type = G_STRAT->ord_type_fut; 

    // 대응주문 : 선물 진입주문 전송
	l_add_wqlist_fut(ord_type, fut_qty, fut_side, G_STRAT->tif_type_fut, fut_px);
    l_dbg(L_INF, "[선물 주문] 전송 완료 (현물 FILL 대응)");
	fut_stat = 1;
	
	return 0;
}

int l_arb_spot_fill(FillEvent *fill) 
{
    int rc = 0;

    int strat_idx = -1;
    int set_idx   = -1;
	int log_idx   = -1;

    if (!fill) {
        l_dbg(L_ERR, "[FILL] spot event is NULL");
        if (arb_stop_msg_send(E9999_MSG) < 0) // system error 
			l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
        return ARB_STOP_CD;
    }
    
	l_dbg(L_INF, "[SPOT-FILL] oid=%ld qty=%d px=%.5f", fill->ord_id, fill->fill_qty, fill->fill_px);        
    SetOrdLog *log  = &G_SET->ordlog;

	// 주문 로그 확인 :log_idx 는 주문로그 엔트리 인덱스임. 
    log_idx = setord_log_find_index_by_id(log, fill->ord_id);
    if (log_idx < 0)
    {
        l_dbg(L_ERR, "[FILL] 주문로그를 찾지 못함....심각한 에러임. 주문번호[%ld]", fill->ord_id);
        if (arb_stop_msg_send(E9999_MSG) < 0) // system error 
			l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
        return ARB_STOP_CD;
    }
    SetOrdLogEnt *ent = &log->ent[log_idx];

    double avg_px = update_avg_price(ent->filled_px, ent->filled_qty, fill->fill_px, fill->fill_qty); 
    // 로그에서 해당 주문 ID를 찾아 FILL 정보 갱신
    rc = setord_log_update_fill_by_id(log, fill->ord_id, fill->fill_qty, avg_px); // 체결가격은 평균가로 처리가 필요함. by lcr
    if (rc != 0)
    {
        l_dbg(L_ERR, "[FILL] ord_id %ld not found in strat[%d] set[%d]", fill->ord_id, strat_idx, set_idx);
        if (arb_stop_msg_send(E9999_MSG) < 0) // system error 
			l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
        return ARB_STOP_CD;
    }

    // 세트수량 / 전체 보유수량 갱신
    if (fill->side > 0)
    {
        G_STRAT->pos_spot += fill->fill_qty;
        POS_SPOT          += fill->fill_qty;
    }
    else
    {
        G_STRAT->pos_spot -= fill->fill_qty;
        POS_SPOT          -= fill->fill_qty;
    }
	spot_stat = 2;

	//-----------------------------------------------------------------------------------
	// 현물우선 체결 주문의 경우 현물 체결과 동시에 선물 주문을 발송한다
	// 이 선물 주문의 체결은 선물체결 event에서 체결 여부를 확인 해야 함
	//-----------------------------------------------------------------------------------
    if ((G_STRAT->exec_mode == EX_SPOT_FIRST)     && 
		(abs(POS_SPOT) == G_STRAT->base_lot_spot) && 
		(POS_FUT  == 0)                           &&
		(fut_stat == 0))
	{
		 if (POS_SPOT < 0)
         	fill->side = 1;
		 else
		 	fill->side = -1;
	
		 fill->fill_qty  = G_STRAT->base_lot_fut;

         // 현물 주문 발송
		 l_dbg(L_INF, "[Fill-Spt(현물우선)] 현물체결 완료. 선물 대응주문 Start.. ");
         rc = l_arb_fut_ord(fill);
		 if (rc != 0)
		 {
		 	l_dbg(L_ERR, "[Fill-Fpt(현물우선)] 선물주문 오류 !! rc=%d", rc);
		 	return rc;
		 }
    }

    if ((G_STRAT->base_lot_spot == abs(POS_SPOT)) &&
		(G_STRAT->base_lot_fut  == abs(POS_FUT )))
	{
	    G_SET->valid = 0;
		l_dbg(L_DBG, "[Fill-Spot] Current SetIDX[%d] Current valid=[%d] 해당세트 완료", g_set_idx, G_SET->valid);
		l_dbg(L_DBG, "[Fill-Spot] SetIDX[%d] finish! Sleep[%d].. TotalPOS S[%d] F[%d]", g_set_idx, G_STRAT->order_interval_ms * 1000, G_STRAT->pos_spot, G_STRAT->pos_fut);

		usleep(G_STRAT->order_interval_ms * 1000);

		// 새 세트 증가
		l_set_id();
	}
    return 0;
}

int l_arb_fut_fill(FillEvent *fill) 
{
    int rc = 0;
    int strat_idx = -1;
    int set_idx   = -1;
	int log_idx   = -1;

    if (!fill) 
    {
        l_dbg(L_ERR, "[FILL] event is NULL");
        if (arb_stop_msg_send(E9999_MSG) < 0) // system error l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
		{
			l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
			return ARB_STOP_CD;
		}
		return ARB_STOP_CD;
    }

    SetOrdLog *log  = &G_SET->ordlog;
	// 주문 로그 확인 :log_idx 는 주문로그 엔트리 인덱스임. 
    log_idx = setord_log_find_index_by_id(log, fill->ord_id);
    if (log_idx < 0)
    {
        l_dbg(L_ERR, "[FILL] 주문로그를 찾지 못함....심각한 에러임. 주문번호[%d]", fill->ord_id);
        if (arb_stop_msg_send(E9999_MSG) < 0) // system error 
		{
			l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
			return ARB_STOP_CD;
		}
		return ARB_STOP_CD;
    }
    SetOrdLogEnt *ent  = &log->ent[log_idx];

    double avg_px = update_avg_price(ent->filled_px, ent->filled_qty, fill->fill_px, fill->fill_qty);
    // 로그에서 해당 주문 ID를 찾아 FILL 정보 갱신
    rc = setord_log_update_fill_by_id(log, fill->ord_id, fill->fill_qty, avg_px); // 체결가격은 평균가로 처리가 필요함
    if (rc != 0)
    {
        l_dbg(L_ERR, "[FILL] ord_id %llu not found in strat[%d] set[%d]", fill->ord_id, strat_idx, set_idx);
        if (arb_stop_msg_send(E9999_MSG) < 0) // system error 
		{
			l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
			return ARB_STOP_CD;
		}
		return ARB_STOP_CD;
    }

    // FILL 처리 로그
    l_dbg(L_INF, "[FILL-Fut] ord_id=%llu fill_qty=%ld px=%.2f", fill->ord_id, fill->fill_qty, fill->fill_px);

    // 세트수량 / 전체 보유수량 갱신
    if (fill->side > 0)
	{
         G_STRAT->pos_fut += fill->fill_qty; 
         POS_FUT          += fill->fill_qty; 
    }
    else
	{
         G_STRAT->pos_fut -= fill->fill_qty; 
         POS_FUT          -= fill->fill_qty; 
    }
	fut_stat = 2;

	//-----------------------------------------------------------------------------------
	// 선물먼저 체결 주문의 경우 선물 체결과 동시에 현물 주문을 발송한다
	// 이 현물 주문의 체결은 현물 체결 event에서 체결 여부를 확인 해야 함.
	//-----------------------------------------------------------------------------------
	/*
    if (G_STRAT->exec_mode == EX_FUT_FIRST          && 
		abs(POS_FUT)       == G_STRAT->base_lot_fut && // 선물체결 수량이 주문수량과 같아 졌을때
		POS_SPOT == 0)
	*/

    l_dbg(L_INF, "chk >> spot_stat[%d], POS_SPOT[%d] exec_mode[%d] POS_FUT[%d]", spot_stat, POS_SPOT, G_STRAT->exec_mode, abs(POS_FUT));

    if ((G_STRAT->exec_mode == EX_FUT_FIRST)    && 
		(abs(POS_FUT) == G_STRAT->base_lot_fut) && 
		(POS_SPOT  == 0)                        &&
		(spot_stat == 0))
	{
		 if (POS_FUT < 0)
         	fill->side = 1;
		 else
		 	fill->side = -1;
	
		 fill->fill_qty  = G_STRAT->base_lot_spot;

         // 현물 주문 발송
		 l_dbg(L_INF, "[Fill-Fut(선물우선)] 선물체결 완료. 현물 대응주문 Start.. ");
         rc = l_arb_spot_ord(fill);
		 if (rc != 0)
		 {
		 	l_dbg(L_ERR, "[Fill-Fut(선물우선)] 현물주문 오류 !! rc=%d", rc);
		 	return rc;
		 }
    }
    if ((G_STRAT->base_lot_spot == abs(POS_SPOT)) &&
		(G_STRAT->base_lot_fut  == abs(POS_FUT )))
	{
	    G_SET->valid = 0;
		l_dbg(L_DBG, "[Fill-Fut] Current SetIDX[%d] Current valid=[%d] 해당세트 완료", g_set_idx, G_SET->valid);

		l_dbg(L_ERR, "[Fill-Fut] SetIDX[%d] finish! Sleep[%d].. TotalPOS S[%d] F[%d]",
											g_set_idx, G_STRAT->order_interval_ms * 1000, G_STRAT->pos_spot, G_STRAT->pos_fut);
		usleep(G_STRAT->order_interval_ms * 1000);
        
		// 새 세트로 증가 
		l_set_id();
	}

	return 0;
}
