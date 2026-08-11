#include <stdio.h>
#include "log.h"
#include "blp.h"

int _CO_B601K_Print( _CO_B601K* ptr)
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

    return sizeof( _CO_B601K);
}

int _CO_B601K_PrintFile( _CO_B601K* ptr, FILE *fp)
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

    return sizeof( _CO_B601K);
}

