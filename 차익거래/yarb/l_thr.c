#define _GNU_SOURCE
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "def.h"
#include "arb.h"

typedef struct wqdata {
	int    ord_typ;
	int    ord_qty;
	int    ord_side;
	int    tif_type;
	double px;
	struct wqdata *next;
} WQDATA;

typedef struct {
	pthread_mutex_t mtx;
	pthread_cond_t  cnd;
	WQDATA         *wq;
} WQLIST;

struct mabuf
{
    char host      [1];
    char trd_type  [1];
    char fepcd     [1];
    char paccno    [4];
    char ordicnd   [1];
    char traderid  [2];
    char auto_type [1];
    char auto_seq  [2];
    char hts_id    [8];
    char stgy_cd   [1];
    char ordno     [7];
    char fill_1    [4];
    char fepused   [7];
    char omsuse   [20];
} mabuf;

extern int extflg;
WQLIST wqlist_fut;
WQLIST wqlist_spot;
extern int thr_usr2flg;
pthread_mutex_t l_mtx_ord=PTHREAD_MUTEX_INITIALIZER;
extern int spot_sem_id;
extern int fut_sem_id;

void l_set_fut_exit();
void l_set_spot_exit();

// 선물 주문 전송 - KRX
void
wq_work_fut(WQDATA *wq)
{
    int   rc=0, len=0, ts_hhmmss, hdr_len=YSKMSG_HDR_SZ, body_len=YSKMSG_ORD_SZ;
    long  oid=0;
	//char  m_time[24];
    //char  cdata[30];  
    char  ordbuf[1024];
    char  qty_buf[64]={0}, prc_buf[64]={0}, oid_buf[128]={0} ;
    char  tif_type_buf[2]={0}, ord_type_buf[2]={0};
    char  area[64]={0}, len_tmp[4+1], ord_dt[8+1];

	YSKMSG_HDR  hdr;
	YSKMSG_ORD  body;
	YSK_STRGMSG fut_st;
	MsgBuf      msg_buf;

	memset(&hdr, 0x20, hdr_len);
	memset(&body, 0x20, body_len);

	Sem_Lock(fut_sem_id);
	if (G_OID_FUT->fut_ordno == 0) {
	    G_OID_FUT->fut_ordno = g_cfg.futoid;
	    G_ORDNO->fut_ordno = g_cfg.futoid;
	} else {
		if (G_OID_FUT->fut_ordno >= g_cfg.futoidlast) {
			l_dbg(L_ERR, "[FUT][주문번호 초과]curr=[%ld] last=[%ld]", 
				G_OID_FUT->fut_ordno, g_cfg.futoidlast);
			Sem_Unlock(fut_sem_id);
			
			if (arb_stop_msg_send(E5004_MSG) < 0) // 주문번호 대역 초과 
				l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
			//extflg = 1;
			l_set_fut_exit();
			return;
		}
		G_OID_FUT->fut_ordno++;
	}
	oid = G_OID_FUT->fut_ordno; 
	G_ORDNO->fut_ordno = oid;                  // 내부주문번호 
	Sem_Unlock(fut_sem_id);
	
	l_dbg(L_DBG, "[FUT][주문번호 채번] ordno[%ld]", oid);

	// Data Header
	memset(len_tmp, 0X00, sizeof(len_tmp));
    len =  (sizeof(YSKMSG_ORD) + sizeof(YSKMSG_HDR)) - sizeof(hdr.sLength);
    snprintf(len_tmp, sizeof(len_tmp), "%04d", len);
    memcpy(hdr.sLength, len_tmp, sizeof(hdr.sLength));
   
	memset(ord_dt, 0x00, sizeof(ord_dt));
    get_todate(ord_dt, sizeof(ord_dt));
	memcpy(hdr.sDate      , ord_dt ,  sizeof(hdr.sDate      ));
    memcpy(hdr.sSndTp     , "72"   ,  sizeof(hdr.sSndTp     ));  // 72  : 파생
    if (G_STRAT->day_ngt_cd[0] == '1')
		memcpy(hdr.sMediaType , "612"  ,  sizeof(hdr.sMediaType ));  // 102 : 정규장102  - 야간장 612 
	else
		memcpy(hdr.sMediaType , "102"  ,  sizeof(hdr.sMediaType )); 
    memcpy(hdr.sSystemType, "00"   ,  sizeof(hdr.sSystemType));  // 시스템구분( 00:정상 01:비상 02:백업)
    memcpy(hdr.sOrderType , "01"   ,  sizeof(hdr.sOrderType ));  // 01 신규 02 정정 03 취소
	
	//---------------------------------------------------------------------
    //  1 KRX 메세지일련번호 - Space
    //---------------------------------------------------------------------

    //---------------------------------------------------------------------
    //  2 KRX Transaction Code - 신규(TCHODR10001) 정정(TCHODR10002) 취소(TCHODR10003)
    //---------------------------------------------------------------------
     memcpy(body.Transaction_Code, "TCHODR10001", sizeof(body.Transaction_Code));
	
	//---------------------------------------------------------------------
    //  3 KRX ME group Code  -- ADD 202512  회원사는 '00'
    //---------------------------------------------------------------------
    memcpy(body.Megrp_no, "00", sizeof(body.Megrp_no));

    //---------------------------------------------------------------------
    //  4 KRX Board ID - G1 유가/코스닥/파생/코넥스/
    //---------------------------------------------------------------------
    memcpy(body.Board_Id, "G1", sizeof(body.Board_Id));

    //---------------------------------------------------------------------
    //  5 KRX Member Number  -- ADD 202512 거래소가 부여한 회원번호
    //---------------------------------------------------------------------
    memcpy(body.MembershipNo, "00024", sizeof(body.MembershipNo));   // 00024 자기거래(상품)지점 

    //---------------------------------------------------------------------
    //  6 KRX Branch Num...  -- ADD 202512
    //---------------------------------------------------------------------
    memcpy(body.BranchNo, "00999", sizeof(body.BranchNo));           // 09999 자기거래(상품)지점

    //---------------------------------------------------------------------
    //  7 KRX Order ID - @
    //---------------------------------------------------------------------
	snprintf(oid_buf, sizeof(oid_buf), "%010ld", oid);
    memcpy(body.OrderNo, oid_buf, sizeof(body.OrderNo));

	//---------------------------------------------------------------------
    //  8 KRX Original Order ID. Amend/Cancel KRX Original ID, New Order Space Set
    //---------------------------------------------------------------------

    //---------------------------------------------------------------------
    //  9 KRX Issue Code.  KRX ISIN Code
    //---------------------------------------------------------------------
    memcpy(body.ItemCode, G_STRAT->fut_sym, sizeof(body.ItemCode));

    //---------------------------------------------------------------------
    // 10 KRX Side.  1.Sell 2.Buy  -- 정정 취소 는 원호가 정합성 대상
    //---------------------------------------------------------------------
    if (wq->ord_side == SIDE_BUY)
        body.TradeFlag[0] = '2';
    else {
        body.TradeFlag[0] = '1';
	}

	//---------------------------------------------------------------------
    // 11 KRX New or Amend or Cancel.  1.New  2.Amend  3.Cancel  //////////
    //---------------------------------------------------------------------
    body.New_Modify_Cancel_gbn[0] = '1'; 

    //---------------------------------------------------------------------
    // 12 KRX Account Number
    // 원호가 정합성 체크 대상
    //---------------------------------------------------------------------
    memcpy(body.AccountNo, G_STRAT->fcm_acno_fut, sizeof(body.AccountNo));

    //---------------------------------------------------------------------
    // 13 KRX Order Quantity
    //---------------------------------------------------------------------
    // 주문 수량
    snprintf(qty_buf, sizeof(qty_buf), "%010d", wq->ord_qty);
	//memset(temp, 0x00, sizeof(temp));
    //sprintf(temp, "%0*lld", sizeof(body.Order_Quantity), pford->l_ordr_qanty);
    memcpy(body.Order_Quantity, qty_buf, sizeof(body.Order_Quantity));

    //---------------------------------------------------------------------
    // 14 KRX  Order Price
    //---------------------------------------------------------------------
    //memset(temp, 0x00, sizeof(temp));
    //sprintf(temp, "%0*.2f", sizeof(body.Order_Price), pford->d_ordr_prc);
	if (wq->ord_typ == ORD_TYPE_L) 
		snprintf(prc_buf, sizeof(prc_buf), "%011.2f", wq->px); // 지정가일때만 가격셋팅
	else
		snprintf(prc_buf, sizeof(prc_buf), "%011.2f", 0.0); // 시장가
    memcpy(body.Order_Price, prc_buf, sizeof(body.Order_Price));

    //---------------------------------------------------------------------
    // 15 KRX Order Type
    // T.시장가 (T.가격제한시장가)
    // 2.지정가
    // I.조건부지정가 (I.조건부지정가)- 장중 미체결주문이 장종료 동시호가에 시장가로 자동 전환되는 호가유형. (I.조건부지정가)
    // W.최유리지정가 (W.가격제한 최유리지정가)  - 가격 입력 불가. 상대 최우선 매수/도 호가 자동셋팅(KRX)
    //---------------------------------------------------------------------
	if (wq->ord_typ == ORD_TYPE_L)
		ord_type_buf[0] = '2';
	else
		ord_type_buf[0] = 'T';

	body.Order_Type[0] = ord_type_buf[0];

    //---------------------------------------------------------------------
    // 16 KRX 호가( 체결) 조건구분코드(0.FAS 3.FAK(IOC) 4.FOK)
    //---------------------------------------------------------------------
	if (wq->tif_type == OT_FOK) // 지정가(FOK)
		tif_type_buf[0] = '4';
	else // 시장가(FAS), 지정가(FAS) 
		tif_type_buf[0] = '0'; // FAS

    body.Order_Condition[0] = tif_type_buf[0];

    //---------------------------------------------------------------------
    // 17 KRX Minimum Trade Volume Count   -- ADD 202512
    // 회원사는 0
    //---------------------------------------------------------------------
    memcpy(body.Min_Che_Cnt, "0000000000", sizeof(body.Min_Che_Cnt));

    //---------------------------------------------------------------------
    // 18 KRX Manage Market Order Type   -- ADD 202512
    // 정정호가인 경우 원호가와 동일한 값이 입력되어야 함(원호가 정합성 체크
    // 취소호가는 0으로 입력
    //---------------------------------------------------------------------
    memcpy(body.Mm_Order_Type_No, "0", sizeof(body.Mm_Order_Type_No));   

    //---------------------------------------------------------------------
    // 19 KRX etc 자사주신고서    -- ADD 202512
    // 파생상품은 해당없음(0    )  사용
    //---------------------------------------------------------------------
    memcpy(body.Treasury_Stock_Id, "0    ", sizeof(body.Treasury_Stock_Id));

	// 20 KRX  자사주매매방법코드   -- ADD 202512
    // 파생상품은 0 해당없음 사용
    //---------------------------------------------------------------------
     memcpy(body.Treasury_Stock_Method, "0", sizeof(body.Treasury_Stock_Method));

    //---------------------------------------------------------------------
    // 21 KRX Credit Type Code   -- ADD 202512
    // 파생상품은 00 해당없음 사용
    //---------------------------------------------------------------------
     memcpy(body.Ask_Type, "00", sizeof(body.Ask_Type));

    //---------------------------------------------------------------------
    // 22 KRX Credit Type Code   -- ADD 202512
    // 파생상품은 10 보통 사용
    //---------------------------------------------------------------------
    memcpy(body.Credit_Type, "10", sizeof(body.Credit_Type));

    //---------------------------------------------------------------------
    // 23 KRX 위탁 자기 구분 Code   -- ADD 202512
    //  파생) 호가입력회원의 자기거래: 31  -- 원호가 정합성 체크 대상--
    //---------------------------------------------------------------------
    memcpy(body.Trust_Principal_Type, "31", sizeof(body.Trust_Principal_Type));

	//---------------------------------------------------------------------
    // 24 KRX 위탁사번호(5)  -- ADD 202512
    //    위탁사가 없는 경우(위탁자기구분코드 11, 12, 31 이면) SPACE로
    //---------------------------------------------------------------------
    memcpy(body.Trust_Company_No, "     ", sizeof(body.Trust_Company_No));   

    //---------------------------------------------------------------------
    // 25 Program Trading Type Code  - 2025.08.27 (확정 - 무조건 '00')
    // 파생상품
    // 00 일반
    // 10 차익거래(주식 및 주가지수)
    // 20 헤지거래(주식 및 주가지수)
    //---------------------------------------------------------------------
    memcpy(body.Program_Trading_Type, "00", sizeof(body.Program_Trading_Type)); 

    //---------------------------------------------------------------------
    // 26 KRX 대용주뭔계좌번호  -- ADD 202512
    // * 현물은 SPACE로 입력
    // * 정정, 취소호가는 SPACE 입력
    // * 정정의 경우 원호가의 정보와 동일한 것으로 인정함
    //---------------------------------------------------------------------
    memcpy(body.Substitute_AccNo, "            ", sizeof(body.Substitute_AccNo));  

    //---------------------------------------------------------------------
    // 27 KRX Account Type -- ADD 202512
    // 파생 자기일반계좌: 41
    //---------------------------------------------------------------------
    memcpy(body.Account_Type, "41", sizeof(body.Account_Type));          

    //---------------------------------------------------------------------
    // 28 KRX Account Margin Type -- ADD 202512
    // 사후증거금일반 : 11
    //---------------------------------------------------------------------
    memcpy(body.Account_Margin_Type, "11", sizeof(body.Account_Margin_Type));  

	//---------------------------------------------------------------------
    // 29 KRX Country Code (3) -- ADD 202512
    //---------------------------------------------------------------------
    memcpy(body.Country_Code, "410", sizeof(body.Country_Code));          

    //---------------------------------------------------------------------
    // 30 KRX Investeor Code (4) -- ADD 202512
    // 금융투자회사: 1000
    //---------------------------------------------------------------------
    memcpy(body.Investor_Type, "1000", sizeof(body.Investor_Type));          

    //---------------------------------------------------------------------
    // 31 KRX Foreign Investeor Code (2) -- ADD 202512
    //  파생 - 외국인 아: 00
    //---------------------------------------------------------------------
    memcpy(body.Foreign_Investor_Type, "00", sizeof(body.Foreign_Investor_Type)); 

    //---------------------------------------------------------------------
    // 32 KRX Order Media Type -- ADD 202512
    // 기타 :9
    //---------------------------------------------------------------------
    memcpy(body.Order_Media_Type, "9", sizeof(body.Order_Media_Type));             

    //---------------------------------------------------------------------
    // 33 KRX Order Identification Info. IP Information ★  추후 셋팅 ip -kong
    //---------------------------------------------------------------------
    //int a, b, c, d;
    //sscanf(pford->prvt_ip, "%d.%d.%d.%d", &a, &b, &c, &d);                      
    //memset(temp, 0x00, sizeof(temp));
    //sprintf(temp, "%03d%03d%03d%03d", a, b, c, d);
    //memcpy(body.Order_Identi, temp, sizeof(body.Order_Identi));

    //---------------------------------------------------------------------
    // 34  KRX Mac Addr(12) -- ADD 202512
    //---------------------------------------------------------------------
    memcpy(body.Mac_Addr, "            ", sizeof(body.Mac_Addr));                

	//---------------------------------------------------------------------
    // 35 KRX Order Date -- ADD 202512 / 36 MemberSendTime skip-already space set
    //     -- 파생야간주문은 RDS 파생종목정보 영업일자 입력-- <<야간>>
    //---------------------------------------------------------------------
#if 0 //20260423 화면에서 야간장시간체크한다고 함 주석처리
    if (G_STRAT->day_ngt_cd[0] == 1)
		memcpy(body.Order_Date, ord_dt, sizeof(body.Order_Date));
	else
#endif
	memcpy(body.Order_Date, G_STRAT->bsns_dt_fut, sizeof(body.Order_Date));
	
    //---------------------------------------------------------------------
    // 36 KRX MemberSend Time (FEP or)  -- ADD 202512 -- Space
    //---------------------------------------------------------------------

    //---------------------------------------------------------------------
    // 37 KRX Member Use Area (60) -- ADD 202512
    //---------------------------------------------------------------------
    // 회원처리 항목
	/* ************************************************************************** */
    /* 30 : A(서버자동주문), C(메리츠Client주문) T(윈웨이매체)                    */
    /*    : 매체구분 (A:Auto or All, C:메리츠매체, T:윈웨이매체                   */
    /*      A는 자동주문/주문응답,체결등 주문관련된건(TR100XXX) 양쪽매체에 보낸다.*/
    /*      Data Header 50 Byte중 20번째 1자리와 같이 사용한다(조회등)            */
    /* 31,32,33,34 : 전략에서 사용                                                */
    /* 35, 36 : 시장구분은 35 1자리만으로 체크                                    */
    /*  (운용상품분류코드)  (시장index) (운영상품index) (시장구분명)*/
    /*          01                   0           1      채권일반
                02                   0           2      채권LP
                11                   1           1      금융파생_국채선물
                12                   1           2      금융파생_통화선물
                13                   1           3      금융파생_금리선물
                21                   2           1      금융파생_국채선물(야)
                22                   2           2      금융파생_통화선물(야)
                23                   2           3      금융파생_금리선물(야)
    */
    /* 37,38,39,40,41 : A0 seq번호 ex) 236 => (00236)                             */
    /* 42, 43 : 계좌번호 seq                                                      */
    /* 44     : 미사용                                                            */
    /* 45     : 스프레드종목이면 1, 아니면 0                                      */
    /* 46     : 주식선물 && 주식옵션에서 유가증권종목이면 '1', 코스닥종목이면 '2' */
    /*          나머지시장이면 '0' 사용안함                                       */
    /* 46,47,48,49 : ApType 4자리 (50101중 0101만) Set                            */
    /* ****************************************************************************/
    memset(&mabuf, 0X20, sizeof(mabuf));
    memcpy(mabuf.host,      "Y",    sizeof(mabuf.host));              // X: FX 일반주문  Y: FX 자동주문
    memcpy(mabuf.trd_type,  "0",    sizeof(mabuf.trd_type));          // 0: 거래유형(IAS사용)
    
	if (G_STRAT->day_ngt_cd[0] == '1')
		memcpy(mabuf.stgy_cd,   "L",    sizeof(mabuf.stgy_cd));       // 0: 일반주문     L: 야간시장 주문 (CME)
   	else
		memcpy(mabuf.stgy_cd,   "0",    sizeof(mabuf.stgy_cd));       // 0: 일반주문     L: 야간시장 주문 (CME)
 
	//memcpy(mabuf.fepused,   "       ",    sizeof(mabuf.stgy_cd));   // Space(7) : fepused (7) already set
	memset(area, 0x00, sizeof(area)); 
	sprintf(area, "A    12%05d00 0%.4s", G_STRAT->fut_idx, G_STRAT->aptype + 3);
    memcpy(mabuf.omsuse,   area,    sizeof(mabuf.omsuse));      
	l_dbg(L_ERR, "area[%s]", area);
    
	memcpy(body.MembershipItem,  mabuf.host, sizeof(body.MembershipItem));
	l_dbg(L_ERR, "회원사영역[%.*s]", sizeof(body.MembershipItem), body.MembershipItem);

    //---------------------------------------------------------------------
    // 38 KRX 알고리즘 전략구분코드 - 2025.08.27 (확정 - 2.알고리즘호가)
    // 1.일반호가  2.알고리즘호가  3.고속 알고리즘호가
    // * 정정 및 취소는 SPACE로 입력
    //---------------------------------------------------------------------
    memcpy(body.Algo_Stgy_Type, "2", sizeof(body.Algo_Stgy_Type));  // 2 : 알고리즘호가

    //---------------------------------------------------------------------
    // 39 KRX TRDR ID (6) -- ADD 202512
    // Space : 해당사항 없음
    //---------------------------------------------------------------------

	//---------------------------------------------------------------------
    // 40 KRX Order Group Number (2) -- ADD 202512
    // Space : 해당사항 없음
    //---------------------------------------------------------------------

    //---------------------------------------------------------------------
    // 41 KRX 자전거래방지코드
    // 해당없음 : 0
    memcpy(body.Smp_Cd, "0", sizeof(body.Smp_Cd)); // 해당없음 : 0

    //---------------------------------------------------------------------
    // 42 KRX etc2 : 호가조건가격-- ADD 202512
    // 유가,코스닥 이외는 0 입력
    //---------------------------------------------------------------------
    memcpy(body.Ord_Cond_Prc, "00000000.00", sizeof(body.Ord_Cond_Prc));

    //---------------------------------------------------------------------
    // 43 KRX 거래시장 선택구분코드 -- ADD 202512
    // [ATS 미참여사, ETP, 코넥스, 파생] : 1
    //---------------------------------------------------------------------
    memcpy(body.Trd_Mkt_Choic_Tp_Cd, "1", sizeof(body.Trd_Mkt_Choic_Tp_Cd));

    //---------------------------------------------------------------------
    // 44 KRX etc3 : 파생은 공매도 허용대상 아님SPACE
    //---------------------------------------------------------------------
    // Srt_Sell_Id(10) space
    //---------------------------------------------------------------------

	l_dbg(L_ERR, "[FUTORD] side[%.1s] ordtype[%.1s] qty[%.10s] px[%.11s] tif[%.1s]", body.TradeFlag, body.Order_Type, body.Order_Quantity, body.Order_Price, body.Order_Condition);
   
	int fut_qid = 0; 
	if (G_STRAT->day_ngt_cd[0] == '0')
	{
		fut_qid        = g_cfg.krx_day_order_id; 
		msg_buf.mtype  = g_cfg.krx_order_day_msg_mtype;
	}
	else
	{ 
		fut_qid        = g_cfg.krx_ngt_order_id; 
		msg_buf.mtype  = g_cfg.krx_order_ngt_msg_mtype;
	}

    //l_dbg(L_ERR, "fut_qid[%d]", fut_qid);
  
    memcpy(ordbuf, &hdr, hdr_len); 
    memcpy(ordbuf+hdr_len, &body, body_len); 
    memset(&msg_buf, 0x20, sizeof(MsgBuf));       
    memcpy(msg_buf.mtext, ordbuf, hdr_len+body_len); 

	rc = l_que_snd(fut_qid, &msg_buf, hdr_len+body_len, 0);
	if (rc < 0)	
	{
        l_dbg(L_ERR, " **선물 주문 오류 **oid [%ld], len[%d]", oid, hdr_len+body_len);
		if (arb_stop_msg_send(E9999_MSG) < 0) // System error 
		    l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
		//extflg = 1;
		l_set_fut_exit();
		return;
    }

    l_dbg(L_ERR, "선물[%ld]   주문완료 S[b:%.5f a:%.5f] F[b:%.2f a:%.2f qb:%d qa:%d]", 
		oid, G_SNAP->px_spot_bid, G_SNAP->px_spot_ask, G_SNAP->px_fut_bid, G_SNAP->px_fut_ask, G_SNAP->qty_fut_bid, G_SNAP->qty_fut_ask);

	// 로그 전송 추가
    memset(&msg_buf, 0x20, sizeof(MsgBuf));       
    memset(&fut_st , 0x20, sizeof(YSK_STRGMSG));  
   
	memcpy(&fut_st.ysk_rcv, &body, body_len); // 주문전송시 조립한 패킷 assign

	ts_hhmmss = get_hhmmss_now();
    OrderLogMsg msg = {
        .strat_idx  = g_strat_idx,
        .set_idx    = g_set_idx,
        .set_id     = G_SET->set_id,
        .ord_id     = oid,
        .leg        = SO_LEG_FUT,
        .side       = wq->ord_side,
        .qty        = wq->ord_qty,
        .px         = wq->px,
        .ts_hhmmss  = ts_hhmmss 
    };

    // 주문 후 처리 패킷 구성
	orderlogq_msg_build(&msg, 2, NULL, &fut_st); // 2-선물후처리패킷조립

    msg_buf.mtype = g_cfg.orderlogq_fut_msg_mtype;
    memcpy(msg_buf.mtext, &fut_st, sizeof(YSK_STRGMSG));
	len = sizeof(YSK_STRGMSG);

    // 실제 전송 로직
	rc = l_que_snd(g_cfg.fut_ordlog_id, &msg_buf, len, 0);
    if (rc != 0)
    {
        l_dbg(L_ERR,  "[선물주문후처리전달] orderlogq_push failed. queue full!! rc=%d", rc);
		if (arb_stop_msg_send(E9999_MSG) < 0) // system error 
	    	l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");

		//extflg = 1;
		l_set_fut_exit();
		return;
    }
        
	// 주문 로그 기록 : 엔트리(진입)로 기록
	pthread_mutex_lock(&l_mtx_ord);
    setord_log_append(&G_SET->ordlog, oid, SO_LEG_FUT, wq->ord_side, wq->ord_qty, wq->px, ts_hhmmss); 
    G_SET->ordlog.n += 1;
	pthread_mutex_unlock(&l_mtx_ord);

    return;
}

// 현물주문 전송
void
wq_work_spot(WQDATA *wq)
{
    int     rc  = 0, ts_hhmmss, body_len=sizeof(SMB_ORD);
    long    oid = 0;
    char    clordid[24+1];
	char	ord_qty[30+1], ord_price[30+1], ord_time[30+1];
	char    ord_dt[8+1], side_buf[1+1], ord_type_buf[1+1], tif_type_buf[1+1];
	char 	tmp[64+1];
	int     qty_mult = 0;
	int     len =0;
	SMB_ORD body;
	MsgBuf  msgbuf;	
	SMB_STRGMSG spot_st;

	memset(&body,   0x20, sizeof(SMB_ORD));
    memset(&msgbuf, 0x00, sizeof(MsgBuf));
    
	long spotoid = 0;
	long spotoidlast = 0;
    if (G_STRAT->exch_spot[0] == 'N') {
        spotoid     = g_cfg.nhspotoid;
        spotoidlast = g_cfg.nhspotoidlast;
    } else if (G_STRAT->exch_spot[0] == 'J') {
        spotoid     = g_cfg.jpmspotoid;
        spotoidlast = g_cfg.jpmspotoidlast;
    } else if (G_STRAT->exch_spot[0] == 'S') {
        spotoid     = g_cfg.shspotoid;
        spotoidlast = g_cfg.shspotoidlast;
	}	else {
		if (arb_stop_msg_send(E9999_MSG) < 0) 
		   	l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
		//extflg = 1;
		l_set_spot_exit();
		return;
	}

	Sem_Lock(spot_sem_id);
	if (G_OID_SPOT->spot_ordno == 0) {
	    G_OID_SPOT->spot_ordno = spotoid;
	    G_ORDNO->spot_ordno   = spotoid;
	} else {
		if (G_OID_SPOT->spot_ordno >= spotoidlast){
			l_dbg(L_ERR, "[SPOT][주문번호 초과]curr=[%ld] last=[%ld]", 
				G_OID_SPOT->spot_ordno, spotoidlast);

			Sem_Unlock(spot_sem_id);
			if (arb_stop_msg_send(E5004_MSG) < 0) // 주문번호 대역 초과 
				l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
			//extflg = 1;
			l_set_spot_exit();
			return;
       }
	   G_OID_SPOT->spot_ordno++;
	}
	oid = G_OID_SPOT->spot_ordno;
	G_ORDNO->spot_ordno = oid;
	Sem_Unlock(spot_sem_id);

    l_dbg(L_DBG, "[SPOT][주문번호 채번] ordno[%ld]", oid);

    // spot 주문 포맷 구성
    memcpy(body.smb_MsgType, "D", strlen("D"));                             // 메세지유형
	set_str_lpad_space(body.smb_SenderCompID ,50 , G_STRAT->scid);          // 송신회사ID (JORD.s_trsmt_taget_id)
	set_str_lpad_space(body.smb_TargetCompID ,50 , G_STRAT->tgid);          // 수신회사ID (JORD.s_recv_taget_id)
	set_str_lpad_space(body.smb_Account      ,30 , G_STRAT->fcm_acno_spot); // 계좌
 
	// 회원처리항목1 :  일자(8) + (채번한 내부주문번호 + 3000000000 (10) + 전략 APType
   	memset(ord_dt, 0x00, sizeof(ord_dt)); 
	get_todate(ord_dt, sizeof(ord_dt));

	memset(clordid, 0x00, sizeof(clordid));

	sprintf(clordid, "%.8s%010ld%.4s", ord_dt, oid, G_STRAT->aptype+3); // 2025/11/22 check 
	set_str_lpad_space(body.smb_ClOrdID, 24, clordid); // 회원처리항목1 

	// 통화코드 
	memset(tmp, 0x00, sizeof(tmp));
	sprintf(tmp,"%.3s", G_STRAT->spot_sym);
	set_str_lpad_space(body.smb_Currency, 3, tmp); 

	// 주문수량(1M 단위 입력, 백만 곱하여 전송) 
	memset(ord_qty, 0x00, sizeof(ord_qty));
	qty_mult = wq->ord_qty * 1000000;
	sprintf(ord_qty, "%d", qty_mult);                                  
    set_str_lpad_space(body.smb_OrderQty, 30, ord_qty); 

	// 주문가격 -- USD : .2f , JPY/CNH : .5f
	memset(ord_price , 0x00, sizeof(ord_price));

	if (memcmp(G_STRAT->spot_sym, "USDKRW", 6) == 0 || memcmp(G_STRAT->spot_sym, "USDKRO", 6) == 0)
		sprintf(ord_price, "%.2f", wq->px);    
	else if (memcmp(G_STRAT->spot_sym,"JPYKRO", 6) == 0)
	{
		double div_px = wq->px / 100;
		sprintf(ord_price, "%.5f", div_px);   
	}
	else if (memcmp(G_STRAT->spot_sym,"CNHKRO", 6) == 0)
		sprintf(ord_price, "%.5f", wq->px);   
 
    set_str_lpad_space(body.smb_Price, 30, ord_price); 

	// 현물매매구분('1'-매수 '2'-매도)
    if (wq->ord_side == SIDE_BUY)
        side_buf[0] = '1';
    else
        side_buf[0] = '2';
    body.smb_Side[0]  = side_buf[0];   

	// 통화코드
	memset(tmp, 0x00, sizeof(tmp));
	sprintf(tmp,"%.3s/%.3s", G_STRAT->spot_sym, G_STRAT->spot_sym + 3);
	memcpy(body.smb_Symbol, tmp, 7); 

    // 현물주문유형 - SMB : 무조건 지정가('2')
    if (wq->ord_typ == ORD_TYPE_L)
        ord_type_buf[0] = '2';
    else
        ord_type_buf[0] = '2';
    body.smb_OrdType[0]= ord_type_buf[0]; 

    // 현물체결조건 구분 -  NH/SH : '3'-IOC,  JPM :'4'-FOK 
    if (wq->tif_type == OT_FAK)
        tif_type_buf[0] = '3'; // IOC 
    else {
        tif_type_buf[0] = '4'; // FOK
	}

	body.smb_TimeInForce[0] = tif_type_buf[0];

	// 주문일시 포맷
	struct timeval tv;
	struct tm *tm_info;
	
	gettimeofday(&tv, NULL);
	tm_info = gmtime(&tv.tv_sec); // SMB - gmtime 사용 / localtime x

	memset(ord_time, 0x00, sizeof(ord_time));	
	snprintf(ord_time, sizeof(ord_time), "%04d%02d%02d-%02d:%02d:%02d.%03ld",
			tm_info->tm_year + 1900,
			tm_info->tm_mon  + 1,
			tm_info->tm_mday,
			tm_info->tm_hour,
			tm_info->tm_min,
			tm_info->tm_sec,
			tv.tv_usec / 1000);

	set_str_lpad_space(body.smb_TransactTime, 30, ord_time);     // 주문일시 : '20180425-12:05:14.256'  

	if (G_STRAT->strategy_name[0] == 'D') {
		memcpy(body.smb_ValueDate, G_STRAT->expire_dt, 8);
		memcpy(body.smb_SettType, "ND", strlen("ND"));
	}
	else {
		memcpy(body.smb_SettType, "SP", strlen("SP"));
	}

	// 20260519 신한 Quote ID
	if (G_STRAT->exch_spot[0] == 'S')
	{
		memcpy(body.smb_ValueDate, G_STRAT->expire_dt, 8);
		if (side_buf[0] == '1')
			memcpy(body.smb_AskQuoteId, G_SNAP->quote_id_ask, 30);
		else
			memcpy(body.smb_BidQuoteId, G_SNAP->quote_id_bid, 30);
	}
//	memcpy(body.smb_BidQuoteId, G_SNAP->quote_id_bid, 30);
//	memcpy(body.smb_AskQuoteId, G_SNAP->quote_id_ask, 30);

	body.smb_EOF[0] = 0x00;
 
	l_dbg(L_ERR, "[SPOTORD] side[%.1s] ordtype[%.1s] qty[%.15s] px[%.7s] tif[%.1s] sett[%.2s]",  
		body.smb_Side, body.smb_OrdType, body.smb_OrderQty, body.smb_Price, body.smb_TimeInForce, body.smb_SettType);

    memcpy(msgbuf.mtext, &body, body_len); 
 	
	int spot_qid = 0;

	if (G_STRAT->exch_spot[0] == 'N') {
        msgbuf.mtype = g_cfg.nh_order_msg_mtype;
		spot_qid = g_cfg.nh_spot_order_id; 
	}
	else if (G_STRAT->exch_spot[0] == 'J') { 
        msgbuf.mtype = g_cfg.jpm_order_msg_mtype;
		spot_qid = g_cfg.jpm_spot_order_id;
	}
	else if (G_STRAT->exch_spot[0] == 'S') { 
        msgbuf.mtype = g_cfg.sh_order_msg_mtype;
		spot_qid = g_cfg.sh_spot_order_id;
	}
	else
	{
        l_dbg(L_ERR, " **현물주문 오류 **oid [%ld], body_len[%d]", oid, body_len);
		if (arb_stop_msg_send(E9999_MSG) < 0) // System error 
		    l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
		//extflg = 1;
		l_set_spot_exit();
		return;
	}
		 
	rc = l_que_snd(spot_qid, &msgbuf, body_len, 0);
	if (rc < 0)	
	{
        l_dbg(L_ERR, " **현물주문 오류 **oid [%ld], body_len[%d]", oid, body_len);
		if (arb_stop_msg_send(E9999_MSG) < 0) // System error 
		    l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");

		//extflg = 1;
		l_set_spot_exit();
		return;
    }

    l_dbg(L_ERR, "현물[%ld] 주문완료 S[b:%.5f a:%.5f] F[b:%.2f a:%.2f qb:%d qa:%d]", 
		oid, G_SNAP->px_spot_bid, G_SNAP->px_spot_ask, G_SNAP->px_fut_bid, G_SNAP->px_fut_ask, G_SNAP->qty_fut_bid, G_SNAP->qty_fut_ask);

    memset(&msgbuf,  0x20, sizeof(MsgBuf));       // space로 처리
    memset(&spot_st, 0x20, sizeof(SMB_STRGMSG));  // space로 처리
   
	memcpy(&spot_st.smb_rcv, &body, body_len);    // 주문전송시 조립한 패킷 assign

	ts_hhmmss = get_hhmmss_now(); 

    OrderLogMsg msg = {
        .strat_idx  = g_strat_idx,
        .set_idx    = g_set_idx,
        .set_id     = G_SET->set_id,
        .ord_id     = oid,
        .leg        = SO_LEG_SPOT,
        .side       = wq->ord_side,
        .qty        = wq->ord_qty,
        .px         = wq->px,
        .ts_hhmmss  = ts_hhmmss
    };

    // 주문 후 처리 패킷 구성
	orderlogq_msg_build(&msg, 1, &spot_st, NULL); //1-현물후처리패킷조립

	int ordlog_id = 0;
	if (G_STRAT->exch_spot[0] == 'J') {
 	   msgbuf.mtype = g_cfg.orderlogq_jpm_msg_mtype;
	   ordlog_id = g_cfg.jpm_ordlog_id;
	}
	else if (G_STRAT->exch_spot[0] == 'S') {
 	   msgbuf.mtype = g_cfg.orderlogq_sh_msg_mtype;
	   ordlog_id = g_cfg.sh_ordlog_id;
	} 
	else {
 	   msgbuf.mtype = g_cfg.orderlogq_spot_msg_mtype;
	   ordlog_id = g_cfg.spot_ordlog_id;
	}
    memcpy(msgbuf.mtext, &spot_st, sizeof(SMB_STRGMSG));
	len = sizeof(SMB_STRGMSG);

    // 실제 전송 로직
    rc = l_que_snd(ordlog_id, &msgbuf, len, 0);
    if (rc !=0)
    {
        l_dbg(L_ERR, "[현물주문후처리전달] orderlogq_push failed. queue full!! rc=%d", rc);
		if (arb_stop_msg_send(E9999_MSG) < 0) // System Error
			l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
        //extflg = 1;
		l_set_spot_exit();
        return;
    }
     
	// 주문 로그 기록 : 엔트리(진입)로 기록
	pthread_mutex_lock(&l_mtx_ord);
    setord_log_append(&G_SET->ordlog, oid, SO_LEG_SPOT, wq->ord_side, wq->ord_qty, wq->px, ts_hhmmss); 
    G_SET->ordlog.n += 1;
	pthread_mutex_unlock(&l_mtx_ord);

    return;
}

void
l_add_wqlist_fut(int ord_typ, int ord_qty, int side, int tif_type, double px)
{
    WQDATA *wq=NULL;
    wq = malloc(sizeof(WQDATA)+1);
    if (wq == NULL) {
        l_dbg(L_ERR, "[%-15s][%4d] system malloc failed!!! errno[%d]", __FUNCTION__, __LINE__, errno);
        //extflg = 1;
		if (arb_stop_msg_send(E9999_MSG) < 0) // System Error
			l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
		l_set_fut_exit();
        return;
    }

	wq->ord_typ  = ord_typ;
	wq->ord_qty  = ord_qty;
	wq->ord_side = side;
	wq->tif_type = tif_type;
	wq->px       = px;
    wq->next     = NULL;

    pthread_mutex_lock(&wqlist_fut.mtx);
    if (wqlist_fut.wq == NULL) {
        wqlist_fut.wq = wq;

    } else {
        WQDATA *jp;
        for (jp=wqlist_fut.wq; jp->next != NULL; jp=jp->next) ;
        jp->next = wq;
    }
    pthread_cond_signal(&wqlist_fut.cnd);
    pthread_mutex_unlock(&wqlist_fut.mtx);
}

void
l_add_wqlist_spot(int ord_typ, int ord_qty, int side, int tif_type, double px)
{
    WQDATA *wq=NULL;
    wq = malloc(sizeof(WQDATA)+1);
    if (wq == NULL) {
        l_dbg(L_ERR, "[%-15s][%4d] system malloc failed!!! errno[%d]", __FUNCTION__, __LINE__, errno);
        //extflg = 1;
		if (arb_stop_msg_send(E9999_MSG) < 0) // System Error
			l_dbg(L_ERR, "Alarm Msg Send Error.. Arb Stop..");
		l_set_spot_exit();
        return;
    }

	wq->ord_typ  = ord_typ;
	wq->ord_qty  = ord_qty;
	wq->ord_side = side;
	wq->tif_type = tif_type;
	wq->px       = px;
    wq->next     = NULL;

    pthread_mutex_lock(&wqlist_spot.mtx);
    if (wqlist_spot.wq == NULL) {
        wqlist_spot.wq = wq;

    } else {
        WQDATA *jp;
        for (jp=wqlist_spot.wq; jp->next != NULL; jp=jp->next) ;
        jp->next = wq;
    }
    pthread_cond_signal(&wqlist_spot.cnd);
    pthread_mutex_unlock(&wqlist_spot.mtx);
}

void
l_fut_proc()
{
    WQDATA *wq;

    pthread_mutex_lock(&wqlist_fut.mtx);
    while (!extflg && wqlist_fut.wq == NULL) {
        pthread_cond_wait(&wqlist_fut.cnd, &wqlist_fut.mtx);
    }

    if (extflg) return;

    wq = wqlist_fut.wq;
    wqlist_fut.wq = wq->next;
    pthread_mutex_unlock(&wqlist_fut.mtx);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    wq_work_fut(wq);

    free(wq);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

}

void
l_spot_proc()
{
    WQDATA *wq;

    pthread_mutex_lock(&wqlist_spot.mtx);
    while (!extflg && wqlist_spot.wq == NULL) {
        pthread_cond_wait(&wqlist_spot.cnd, &wqlist_spot.mtx);
    }

    if (extflg) return;

    wq = wqlist_spot.wq;
    wqlist_spot.wq = wq->next;
    pthread_mutex_unlock(&wqlist_spot.mtx);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    wq_work_spot(wq);

    free(wq);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

}

void *l_fut_dat_proc()
{
    pthread_setname_np(pthread_self(), __FUNCTION__);
    while(!extflg) {
        if (thr_usr2flg || extflg) break;
        l_fut_proc();
    }
    pthread_exit(NULL);
    return(NULL);
}

void *l_spot_dat_proc()
{
    pthread_setname_np(pthread_self(), __FUNCTION__);
    while(!extflg) {
        if (thr_usr2flg || extflg) break;
        l_spot_proc();
    }
    pthread_exit(NULL);
    return(NULL);
}

void l_set_fut_exit()
{
	extflg = 1;
	pthread_mutex_lock(&wqlist_fut.mtx);
	pthread_cond_broadcast(&wqlist_fut.cnd); // 모든 대기 스레드에게 종료신호
	pthread_mutex_unlock(&wqlist_fut.mtx);
}

void l_set_spot_exit()
{
	extflg = 1;
	pthread_mutex_lock(&wqlist_spot.mtx);
	pthread_cond_broadcast(&wqlist_spot.cnd); // 모든 대기 스레드에게 종료신호
	pthread_mutex_unlock(&wqlist_spot.mtx);
}
