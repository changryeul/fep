#include "shm_memory.h"
#include "blp.h"
#include "log.h"

int SHM_NOTE_Print( SHM_NOTE* ptr)
{
    LogRaw( "%s", "----[ SHM_NOTE ]------------------------------------------------------------------------\n");
    LogRaw( "종목수, seq 0에만 사용        total_item_cnt         4    0 = [%d]\n",     ptr->total_item_cnt);
    LogRaw( "보유여부 확인(평가금액용)     check_cnt              4    4 = [%d]\n",     ptr->check_cnt);
    LogRaw( "자동전략기동시 해당종목 +- ? auto_use               4    8 = [%d]\n",     ptr->auto_use);
    LogRaw( "최근 시세내역 30개의 Key      CURR_Arry_Key          4   12 = [%d]\n",     ptr->CURR_Arry_Key);
    LogRaw( "CURR_Arry_Key 직전값기억      Befor_CURR_Arry_Key    4   16 = [%d]\n",     ptr->Befor_CURR_Arry_Key);
    LogRaw( "0:체결(A3/G7), 1:호가(B6)     HogaLastGbn            4   20 = [%d]\n",     ptr->HogaLastGbn);
#if 0
    CO_M401K_Print( ptr->M4);
    CO_A701K_Print( ptr->A7);
    CO_A001_RDS02_Print( ptr->A0);
    CO_A301K_Print( ptr->A3);
    CO_G701K_Print( ptr->G7);
    CO_B601K_Print( ptr->B6);
    CO_G701K_Print( ptr->CURR_Arry);
#endif
    LogRaw( "%s", "------------------------------------------------------------------------[ SHM_NOTE ]----\n");

    return sizeof( SHM_NOTE);
}


int CO_B601K_Print( CO_B601K* ptr)
{
    LogRaw( "%s", "----[ _CO_B601K ]-----------------------------------------------------------------------\n");
    LogRaw( "TR CODE                       tr_gbn                 5    0 = [%.5s]\n",   ptr->tr_gbn);
    LogRaw( "정보분배일련번호              seq_no                 8    5 = [%.8s]\n",   ptr->seq_no);
    LogRaw( "보드ID                        board_id               2   13 = [%.2s]\n",   ptr->board_id);
    LogRaw( "세션ID                        session_id             2   15 = [%.2s]\n",   ptr->session_id);
    LogRaw( "종목코드                      item_code             12   17 = [%.12s]\n",  ptr->item_code);
    LogRaw( "매매처리시각                  trade_time            12   29 = [%.12s]\n",  ptr->trade_time);
    LogRaw( "매도1단계우선호가가격         ask1_price            11   41 = [%.11s]\n",  ptr->ask1_price);
    LogRaw( "매수1단계우선호가가격         bid1_price            11   52 = [%.11s]\n",  ptr->bid1_price);
    LogRaw( "채권매도1단계우선호가잔량     bond_ask1_remain_vol  15   63 = [%.15s]\n",  ptr->bond_ask1_remain_vol);
    LogRaw( "채권매수1단계우선호가잔량     bond_bid1_remain_vol  15   78 = [%.15s]\n",  ptr->bond_bid1_remain_vol);
    LogRaw( "매도1단계우선호가수익률       ask1_yield            13   93 = [%.13s]\n",  ptr->ask1_yield);
    LogRaw( "매수1단계우선호가수익률       bid1_yield            13  106 = [%.13s]\n",  ptr->bid1_yield);
    LogRaw( "매도2단계우선호가가격         ask2_price            11  119 = [%.11s]\n",  ptr->ask2_price);
    LogRaw( "매수2단계우선호가가격         bid2_price            11  130 = [%.11s]\n",  ptr->bid2_price);
    LogRaw( "채권매도2단계우선호가잔량     bond_ask2_remain_vol  15  141 = [%.15s]\n",  ptr->bond_ask2_remain_vol);
    LogRaw( "채권매수2단계우선호가잔량     bond_bid2_remain_vol  15  156 = [%.15s]\n",  ptr->bond_bid2_remain_vol);
    LogRaw( "매도2단계우선호가수익률       ask2_yield            13  171 = [%.13s]\n",  ptr->ask2_yield);
    LogRaw( "매수2단계우선호가수익률       bid2_yield            13  184 = [%.13s]\n",  ptr->bid2_yield);
    LogRaw( "매도3단계우선호가가격         ask3_price            11  197 = [%.11s]\n",  ptr->ask3_price);
    LogRaw( "매수3단계우선호가가격         bid3_price            11  208 = [%.11s]\n",  ptr->bid3_price);
    LogRaw( "채권매도3단계우선호가잔량     bond_ask3_remain_vol  15  219 = [%.15s]\n",  ptr->bond_ask3_remain_vol);
    LogRaw( "채권매수3단계우선호가잔량     bond_bid3_remain_vol  15  234 = [%.15s]\n",  ptr->bond_bid3_remain_vol);
    LogRaw( "매도3단계우선호가수익률       ask3_yield            13  249 = [%.13s]\n",  ptr->ask3_yield);
    LogRaw( "매수3단계우선호가수익률       bid3_yield            13  262 = [%.13s]\n",  ptr->bid3_yield);
    LogRaw( "매도4단계우선호가가격         ask4_price            11  275 = [%.11s]\n",  ptr->ask4_price);
    LogRaw( "매수4단계우선호가가격         bid4_price            11  286 = [%.11s]\n",  ptr->bid4_price);
    LogRaw( "채권매도4단계우선호가잔량     bond_ask4_remain_vol  15  297 = [%.15s]\n",  ptr->bond_ask4_remain_vol);
    LogRaw( "채권매수4단계우선호가잔량     bond_bid4_remain_vol  15  312 = [%.15s]\n",  ptr->bond_bid4_remain_vol);
    LogRaw( "매도4단계우선호가수익률       ask4_yield            13  327 = [%.13s]\n",  ptr->ask4_yield);
    LogRaw( "매수4단계우선호가수익률       bid4_yield            13  340 = [%.13s]\n",  ptr->bid4_yield);
    LogRaw( "매도5단계우선호가가격         ask5_price            11  353 = [%.11s]\n",  ptr->ask5_price);
    LogRaw( "매수5단계우선호가가격         bid5_price            11  364 = [%.11s]\n",  ptr->bid5_price);
    LogRaw( "채권매도5단계우선호가잔량     bond_ask5_remain_vol  15  375 = [%.15s]\n",  ptr->bond_ask5_remain_vol);
    LogRaw( "채권매수5단계우선호가잔량     bond_bid5_remain_vol  15  390 = [%.15s]\n",  ptr->bond_bid5_remain_vol);
    LogRaw( "매도5단계우선호가수익률       ask5_yield            13  405 = [%.13s]\n",  ptr->ask5_yield);
    LogRaw( "매수5단계우선호가수익률       bid5_yield            13  418 = [%.13s]\n",  ptr->bid5_yield);
    LogRaw( "채권매도호가총잔량            bond_total_ask_vol    15  431 = [%.15s]\n",  ptr->bond_total_ask_vol);
    LogRaw( "채권매수호가총잔량            bond_total_bid_vol    15  446 = [%.15s]\n",  ptr->bond_total_bid_vol);
    LogRaw( "정보분배메세지종료키워드      msg_end_key            1  461 = [%.1s]\n",   ptr->msg_end_key);
    LogRaw( "%s", "-----------------------------------------------------------------------[ _CO_B601K ]----\n");

    return sizeof( CO_B601K);
}

int CO_B601K_PrintFile( CO_B601K* ptr, FILE *fp)
{
    fprintf( fp, "%s", "----[ _CO_B601K ]-----------------------------------------------------------------------\n");
    fprintf( fp, "TR CODE                       tr_gbn                 5    0 = [%.5s]\n",   ptr->tr_gbn);
    fprintf( fp, "정보분배일련번호              seq_no                 8    5 = [%.8s]\n",   ptr->seq_no);
    fprintf( fp, "보드ID                        board_id               2   13 = [%.2s]\n",   ptr->board_id);
    fprintf( fp, "세션ID                        session_id             2   15 = [%.2s]\n",   ptr->session_id);
    fprintf( fp, "종목코드                      item_code             12   17 = [%.12s]\n",  ptr->item_code);
    fprintf( fp, "매매처리시각                  trade_time            12   29 = [%.12s]\n",  ptr->trade_time);
    fprintf( fp, "매도1단계우선호가가격         ask1_price            11   41 = [%.11s]\n",  ptr->ask1_price);
    fprintf( fp, "매수1단계우선호가가격         bid1_price            11   52 = [%.11s]\n",  ptr->bid1_price);
    fprintf( fp, "채권매도1단계우선호가잔량     bond_ask1_remain_vol  15   63 = [%.15s]\n",  ptr->bond_ask1_remain_vol);
    fprintf( fp, "채권매수1단계우선호가잔량     bond_bid1_remain_vol  15   78 = [%.15s]\n",  ptr->bond_bid1_remain_vol);
    fprintf( fp, "매도1단계우선호가수익률       ask1_yield            13   93 = [%.13s]\n",  ptr->ask1_yield);
    fprintf( fp, "매수1단계우선호가수익률       bid1_yield            13  106 = [%.13s]\n",  ptr->bid1_yield);
    fprintf( fp, "매도2단계우선호가가격         ask2_price            11  119 = [%.11s]\n",  ptr->ask2_price);
    fprintf( fp, "매수2단계우선호가가격         bid2_price            11  130 = [%.11s]\n",  ptr->bid2_price);
    fprintf( fp, "채권매도2단계우선호가잔량     bond_ask2_remain_vol  15  141 = [%.15s]\n",  ptr->bond_ask2_remain_vol);
    fprintf( fp, "채권매수2단계우선호가잔량     bond_bid2_remain_vol  15  156 = [%.15s]\n",  ptr->bond_bid2_remain_vol);
    fprintf( fp, "매도2단계우선호가수익률       ask2_yield            13  171 = [%.13s]\n",  ptr->ask2_yield);
    fprintf( fp, "매수2단계우선호가수익률       bid2_yield            13  184 = [%.13s]\n",  ptr->bid2_yield);
    fprintf( fp, "매도3단계우선호가가격         ask3_price            11  197 = [%.11s]\n",  ptr->ask3_price);
    fprintf( fp, "매수3단계우선호가가격         bid3_price            11  208 = [%.11s]\n",  ptr->bid3_price);
    fprintf( fp, "채권매도3단계우선호가잔량     bond_ask3_remain_vol  15  219 = [%.15s]\n",  ptr->bond_ask3_remain_vol);
    fprintf( fp, "채권매수3단계우선호가잔량     bond_bid3_remain_vol  15  234 = [%.15s]\n",  ptr->bond_bid3_remain_vol);
    fprintf( fp, "매도3단계우선호가수익률       ask3_yield            13  249 = [%.13s]\n",  ptr->ask3_yield);
    fprintf( fp, "매수3단계우선호가수익률       bid3_yield            13  262 = [%.13s]\n",  ptr->bid3_yield);
    fprintf( fp, "매도4단계우선호가가격         ask4_price            11  275 = [%.11s]\n",  ptr->ask4_price);
    fprintf( fp, "매수4단계우선호가가격         bid4_price            11  286 = [%.11s]\n",  ptr->bid4_price);
    fprintf( fp, "채권매도4단계우선호가잔량     bond_ask4_remain_vol  15  297 = [%.15s]\n",  ptr->bond_ask4_remain_vol);
    fprintf( fp, "채권매수4단계우선호가잔량     bond_bid4_remain_vol  15  312 = [%.15s]\n",  ptr->bond_bid4_remain_vol);
    fprintf( fp, "매도4단계우선호가수익률       ask4_yield            13  327 = [%.13s]\n",  ptr->ask4_yield);
    fprintf( fp, "매수4단계우선호가수익률       bid4_yield            13  340 = [%.13s]\n",  ptr->bid4_yield);
    fprintf( fp, "매도5단계우선호가가격         ask5_price            11  353 = [%.11s]\n",  ptr->ask5_price);
    fprintf( fp, "매수5단계우선호가가격         bid5_price            11  364 = [%.11s]\n",  ptr->bid5_price);
    fprintf( fp, "채권매도5단계우선호가잔량     bond_ask5_remain_vol  15  375 = [%.15s]\n",  ptr->bond_ask5_remain_vol);
    fprintf( fp, "채권매수5단계우선호가잔량     bond_bid5_remain_vol  15  390 = [%.15s]\n",  ptr->bond_bid5_remain_vol);
    fprintf( fp, "매도5단계우선호가수익률       ask5_yield            13  405 = [%.13s]\n",  ptr->ask5_yield);
    fprintf( fp, "매수5단계우선호가수익률       bid5_yield            13  418 = [%.13s]\n",  ptr->bid5_yield);
    fprintf( fp, "채권매도호가총잔량            bond_total_ask_vol    15  431 = [%.15s]\n",  ptr->bond_total_ask_vol);
    fprintf( fp, "채권매수호가총잔량            bond_total_bid_vol    15  446 = [%.15s]\n",  ptr->bond_total_bid_vol);
    fprintf( fp, "정보분배메세지종료키워드      msg_end_key            1  461 = [%.1s]\n",   ptr->msg_end_key);
    fprintf( fp, "%s", "-----------------------------------------------------------------------[ _CO_B601K ]----\n");

    return sizeof( CO_B601K);
}

int BLP_ARG_Print( BLP_ARG* ptr)
{
    LogRaw( "%s", "----[ BLP_ARG ]-------------------------------------------------------------------------\n");
    LogRaw( "1  실행일자                   sb31_exe_ymd           9    0 = [%.9s]\n",   ptr->sb31_exe_ymd);
    LogRaw( "2  실행번호                   sb31_exe_no            4    9 = [%d]\n",     ptr->sb31_exe_no);
    LogRaw( "3  처리상태구분코드           sb31_proc_stus_dstcd   2   13 = [%.2s]\n",   ptr->sb31_proc_stus_dstcd);
    LogRaw( "4  시작일시                   sb31_start_yms        15   15 = [%.15s]\n",  ptr->sb31_start_yms);
    LogRaw( "5  종료일시                   sb31_end_yms          15   30 = [%.15s]\n",  ptr->sb31_end_yms);
    LogRaw( "6  유동성공급시작일시         sb31_lp_start_yms     15   45 = [%.15s]\n",  ptr->sb31_lp_start_yms);
    LogRaw( "7  유동성공급종료일시         sb31_lp_end_yms       15   60 = [%.15s]\n",  ptr->sb31_lp_end_yms);
    LogRaw( "8  종목코드                   sb31_item_cd          13   75 = [%.13s]\n",  ptr->sb31_item_cd);
    LogRaw( "9  유동성공급종목구분코드     sb31_mm_item_dstcd     3   88 = [%.3s]\n",   ptr->sb31_mm_item_dstcd);
    LogRaw( "10 스프래드1가격              sb31_sped_prc[0]       8   91 = [%f]\n",     ptr->sb31_sped_prc[ 0]);
    LogRaw( "11 스프래드2가격              sb31_sped_prc[1]       8   99 = [%f]\n",     ptr->sb31_sped_prc[ 1]);
    LogRaw( "12 스프래드3가격              sb31_sped_prc[2]       8  107 = [%f]\n",     ptr->sb31_sped_prc[ 2]);
    LogRaw( "13 주문1수량                  sb31_ord_qanty[0]      8  115 = [%f]\n",     ptr->sb31_ord_qanty[ 0]);
    LogRaw( "14 주문2수량                  sb31_ord_qanty[1]      8  123 = [%f]\n",     ptr->sb31_ord_qanty[ 1]);
    LogRaw( "15 주문3수량                  sb31_ord_qanty[2]      8  131 = [%f]\n",     ptr->sb31_ord_qanty[ 2]);
    LogRaw( "16 주문수량단위               sb31_ord_qanty_unit    4  139 = [%d]\n",     ptr->sb31_ord_qanty_unit);
    LogRaw( "17 괴리율대상구분코드         sb31_dspratio_taget_dstcd   3  143 = [%.3s]\n",      ptr->sb31_dspratio_taget_dstcd);
    LogRaw( "18 KRX괴리율구분코드          sb31_dspratio_dstcd    2  146 = [%.2s]\n",   ptr->sb31_dspratio_dstcd);
    LogRaw( "19 괴리율                     sb31_dspratio          8  148 = [%f]\n",     ptr->sb31_dspratio);
    LogRaw( "20 틱단위                     sb31_tick_unit         8  156 = [%f]\n",     ptr->sb31_tick_unit);
    LogRaw( "21 거래원번호                 sb31_trdr_uno          6  164 = [%.6s]\n",   ptr->sb31_trdr_uno);
    LogRaw( "22 CMBS트레이더번호           sb31_trdr_no           5  170 = [%.5s]\n",   ptr->sb31_trdr_no);
    LogRaw( "23 민간평가수익률             sb31_prv_yild          8  175 = [%f]\n",     ptr->sb31_prv_yild);
    LogRaw( "24 민간평가가격               sb31_prv_prc           8  183 = [%f]\n",     ptr->sb31_prv_prc);
    LogRaw( "25 종가                       sb31_clsng_prc         8  191 = [%f]\n",     ptr->sb31_clsng_prc);
    LogRaw( "26 종가수익률                 sb31_clsng_yild        8  199 = [%f]\n",     ptr->sb31_clsng_yild);
    LogRaw( "27 계좌번호                   sb31_account_no       12  207 = [%.12s]\n",  ptr->sb31_account_no);
    LogRaw( "%s", "-------------------------------------------------------------------------[ BLP_ARG ]----\n");

    return sizeof( BLP_ARG);
}



