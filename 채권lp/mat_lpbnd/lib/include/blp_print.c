/** ***************************************************************************
**  @file       blp.c
**  @date       2025/09/01
**  @author     cdc
**  @version    V0.0.20250901
**  @brif
**  채권 시장조성 라이브러리
**	blp.c			- 
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#define		DATA_SIZE	2048
#include "shm_memory.h"
#include "pa_struct.h"
#include "buf_struct.h"

#include "mem.h"
#include "sem.h"
#include "etc.h"
#ifndef _OMS_SOURCE_
#include "log.h"
#else
#include "log_conv.h"
#endif
#include "blp.h"

/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/
extern int		Continue;
extern SHM_NOTE	*Shm_Note;

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#include "shm_memory.h"


int BLP_ARG_IF_Print( BLP_ARG_IF* ptr)
{
    LogRaw( "%s", "----[ BLP_ARG_IF ]----------------------------------------------------------------------\n");
    LogRaw( "실행일자                      exe_ymd                8    0 = [%.8s]\n",   ptr->exe_ymd);
    LogRaw( "실행번호                      exe_no                 5    8 = [%.5s]\n",   ptr->exe_no);
    LogRaw( "처리상태구분코드              proc_stus_dstcd        1   13 = [%.1s]\n",   ptr->proc_stus_dstcd);
    LogRaw( "종목코드                      item_cd               12   14 = [%.12s]\n",  ptr->item_cd);
    LogRaw( "유동성공급종목구분코드        mm_item_dstcd          2   26 = [%.2s]\n",   ptr->mm_item_dstcd);
    LogRaw( "주문수량단위                  ord_qanty_unit         5   28 = [%.5s]\n",   ptr->ord_qanty_unit);
    LogRaw( "괴리율대상구분코드            dspratio_taget_dstcd   2   33 = [%.2s]\n",   ptr->dspratio_taget_dstcd);
    LogRaw( "KRX괴리율구분코드             dspratio_dstcd         1   35 = [%.1s]\n",   ptr->dspratio_dstcd);
    LogRaw( "괴리율                        dspratio              11   36 = [%.11s]\n",  ptr->dspratio);
    LogRaw( "틱단위                        tick_unit             11   47 = [%.11s]\n",  ptr->tick_unit);
    LogRaw( "거래원번호                    trdr_uno               5   58 = [%.5s]\n",   ptr->trdr_uno);
    LogRaw( "CMBS트레이더번호              trdr_no                4   63 = [%.4s]\n",   ptr->trdr_no);
    LogRaw( "민간평가수익률                prv_yild              11   67 = [%.11s]\n",  ptr->prv_yild);
    LogRaw( "민간평가가격                  prv_prc               11   78 = [%.11s]\n",  ptr->prv_prc);
    LogRaw( "종가                          clsng_prc             11   89 = [%.11s]\n",  ptr->clsng_prc);
    LogRaw( "종가수익률                    clsng_yild            11  100 = [%.11s]\n",  ptr->clsng_yild);
    LogRaw( "계좌번호                      account_no            12  111 = [%.12s]\n",  ptr->account_no);
    LogRaw( "시장 조성 순번 항상 3 1:오전  mk_stat                1  123 = [%.1s]\n",   ptr->mk_stat);
    LogRaw( "LP 조성 시작 시간 HHMMSS      start_1                6  124 = [%.6s]\n",   ptr->start_1);
    LogRaw( "LP 조성 종료 시간 HHMMSS      end_1                  6  130 = [%.6s]\n",   ptr->end_1);
    LogRaw( "LP 최소조성시간 (분)          lp_time_1              5  136 = [%.5s]\n",   ptr->lp_time_1);
    LogRaw( "체결이후 조성 대기 시간 (초)  exe_delay_1            5  141 = [%.5s]\n",   ptr->exe_delay_1);
    LogRaw( "반대매매 유지시간 (초)        rev_wait_1             5  146 = [%.5s]\n",   ptr->rev_wait_1);
    LogRaw( "시장조성 제출 한도 시간 - 제  submit_limit_1         5  151 = [%.5s]\n",   ptr->submit_limit_1);
    LogRaw( "스프래드가격 1,2,3            sped_prc_11           11  156 = [%.11s]\n",  ptr->sped_prc_11);
    LogRaw( "스프래드가격 1,2,3            sped_prc_12           11  167 = [%.11s]\n",  ptr->sped_prc_12);
    LogRaw( "스프래드가격 1,2,3            sped_prc_13           11  178 = [%.11s]\n",  ptr->sped_prc_13);
    LogRaw( "주문수량 1,2,3                ord_qanty_11          10  189 = [%.10s]\n",  ptr->ord_qanty_11);
    LogRaw( "주문수량 1,2,3                ord_qanty_12          10  199 = [%.10s]\n",  ptr->ord_qanty_12);
    LogRaw( "주문수량 1,2,3                ord_qanty_13          10  209 = [%.10s]\n",  ptr->ord_qanty_13);
    LogRaw( "LP 조성 시작 시간 HHMMSS      start_2                6  219 = [%.6s]\n",   ptr->start_2);
    LogRaw( "LP 조성 종료 시간 HHMMSS      end_2                  6  225 = [%.6s]\n",   ptr->end_2);
    LogRaw( "LP 최소조성시간 (분)          lp_time_2              5  231 = [%.5s]\n",   ptr->lp_time_2);
    LogRaw( "체결이후 조성 대기 시간 (초)  exe_delay_2            5  236 = [%.5s]\n",   ptr->exe_delay_2);
    LogRaw( "반대매매 유지시간 (초)        rev_wait_2             5  241 = [%.5s]\n",   ptr->rev_wait_2);
    LogRaw( "시장조성 제출 한도 시간 - 제  submit_limit_2         5  246 = [%.5s]\n",   ptr->submit_limit_2);
    LogRaw( "스프래드가격 1,2,3            sped_prc_21           11  251 = [%.11s]\n",  ptr->sped_prc_21);
    LogRaw( "스프래드가격 1,2,3            sped_prc_22           11  262 = [%.11s]\n",  ptr->sped_prc_22);
    LogRaw( "스프래드가격 1,2,3            sped_prc_23           11  273 = [%.11s]\n",  ptr->sped_prc_23);
    LogRaw( "주문수량 1,2,3                ord_qanty_21          10  284 = [%.10s]\n",  ptr->ord_qanty_21);
    LogRaw( "주문수량 1,2,3                ord_qanty_22          10  294 = [%.10s]\n",  ptr->ord_qanty_22);
    LogRaw( "주문수량 1,2,3                ord_qanty_23          10  304 = [%.10s]\n",  ptr->ord_qanty_23);
    LogRaw( "LP 조성 시작 시간 HHMMSS      start_3                6  314 = [%.6s]\n",   ptr->start_3);
    LogRaw( "LP 조성 종료 시간 HHMMSS      end_3                  6  320 = [%.6s]\n",   ptr->end_3);
    LogRaw( "LP 최소조성시간 (분)          lp_time_3              5  326 = [%.5s]\n",   ptr->lp_time_3);
    LogRaw( "체결이후 조성 대기 시간 (초)  exe_delay_3            5  331 = [%.5s]\n",   ptr->exe_delay_3);
    LogRaw( "반대매매 유지시간 (초)        rev_wait_3             5  336 = [%.5s]\n",   ptr->rev_wait_3);
    LogRaw( "시장조성 제출 한도 시간 - 제  submit_limit_3         5  341 = [%.5s]\n",   ptr->submit_limit_3);
    LogRaw( "스프래드가격 1,2,3            sped_prc_31           11  346 = [%.11s]\n",  ptr->sped_prc_31);
    LogRaw( "스프래드가격 1,2,3            sped_prc_32           11  357 = [%.11s]\n",  ptr->sped_prc_32);
    LogRaw( "스프래드가격 1,2,3            sped_prc_33           11  368 = [%.11s]\n",  ptr->sped_prc_33);
    LogRaw( "주문수량 1,2,3                ord_qanty_31          10  379 = [%.10s]\n",  ptr->ord_qanty_31);
    LogRaw( "주문수량 1,2,3                ord_qanty_32          10  389 = [%.10s]\n",  ptr->ord_qanty_32);
    LogRaw( "주문수량 1,2,3                ord_qanty_33          10  399 = [%.10s]\n",  ptr->ord_qanty_33);
    LogRaw( "%s", "----------------------------------------------------------------------[ BLP_ARG_IF ]----\n");

    return sizeof( BLP_ARG_IF);
}

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

int CO_G701K_Print( CO_G701K* ptr)
{
    LogRaw( "%s", "----[ CO_G701K ]------------------------------------------------------------------------\n");
    LogRaw( "TR CODE                       tr_gbn                 5    0 = [%.5s]\n",   ptr->tr_gbn);
    LogRaw( "정보분배일련번호              seq_no                 8    5 = [%.8s]\n",   ptr->seq_no);
    LogRaw( "보드ID                        board_id               2   13 = [%.2s]\n",   ptr->board_id);
    LogRaw( "세션ID                        session_id             2   15 = [%.2s]\n",   ptr->session_id);
    LogRaw( "종목코드                      item_code             12   17 = [%.12s]\n",  ptr->item_code);
    LogRaw( "매매처리시각                  trade_time            12   29 = [%.12s]\n",  ptr->trade_time);
    LogRaw( "체결가격                      crprc                 11   41 = [%.11s]\n",  ptr->crprc);
    LogRaw( "거래량                        volume                10   52 = [%.10s]\n",  ptr->volume);
    LogRaw( "거래일자                      trade_date             8   62 = [%.8s]\n",   ptr->trade_date);
    LogRaw( "거래대금                      trade_amt             22   70 = [%.22s]\n",  ptr->trade_amt);
    LogRaw( "체결수익률                    exec_yield            13   92 = [%.13s]\n",  ptr->exec_yield);
    LogRaw( "시가                          open_price            11  105 = [%.11s]\n",  ptr->open_price);
    LogRaw( "고가                          high_price            11  116 = [%.11s]\n",  ptr->high_price);
    LogRaw( "저가                          low_price             11  127 = [%.11s]\n",  ptr->low_price);
    LogRaw( "시가수익률                    open_yield            13  138 = [%.13s]\n",  ptr->open_yield);
    LogRaw( "고가수익률                    high_yield            13  151 = [%.13s]\n",  ptr->high_yield);
    LogRaw( "저가수익률                    low_yield             13  164 = [%.13s]\n",  ptr->low_yield);
    LogRaw( "채권누적체결수량              bond_accum_exec_vol   15  177 = [%.15s]\n",  ptr->bond_accum_exec_vol);
    LogRaw( "누적거래대금                  accum_trade_amt       22  192 = [%.22s]\n",  ptr->accum_trade_amt);
    LogRaw( "결제일자                      settl_date             8  214 = [%.8s]\n",   ptr->settl_date);
    LogRaw( "매도1단계우선호가가격         ask1_price            11  222 = [%.11s]\n",  ptr->ask1_price);
    LogRaw( "매수1단계우선호가가격         bid1_price            11  233 = [%.11s]\n",  ptr->bid1_price);
    LogRaw( "채권매도1단계우선호가잔량     bond_ask1_remain_vol  15  244 = [%.15s]\n",  ptr->bond_ask1_remain_vol);
    LogRaw( "채권매수1단계우선호가잔량     bond_bid1_remain_vol  15  259 = [%.15s]\n",  ptr->bond_bid1_remain_vol);
    LogRaw( "매도1단계우선호가수익률       ask1_yield            13  274 = [%.13s]\n",  ptr->ask1_yield);
    LogRaw( "매수1단계우선호가수익률       bid1_yield            13  287 = [%.13s]\n",  ptr->bid1_yield);
    LogRaw( "매도2단계우선호가가격         ask2_price            11  300 = [%.11s]\n",  ptr->ask2_price);
    LogRaw( "매수2단계우선호가가격         bid2_price            11  311 = [%.11s]\n",  ptr->bid2_price);
    LogRaw( "채권매도2단계우선호가잔량     bond_ask2_remain_vol  15  322 = [%.15s]\n",  ptr->bond_ask2_remain_vol);
    LogRaw( "채권매수2단계우선호가잔량     bond_bid2_remain_vol  15  337 = [%.15s]\n",  ptr->bond_bid2_remain_vol);
    LogRaw( "매도2단계우선호가수익률       ask2_yield            13  352 = [%.13s]\n",  ptr->ask2_yield);
    LogRaw( "매수2단계우선호가수익률       bid2_yield            13  365 = [%.13s]\n",  ptr->bid2_yield);
    LogRaw( "매도3단계우선호가가격         ask3_price            11  378 = [%.11s]\n",  ptr->ask3_price);
    LogRaw( "매수3단계우선호가가격         bid3_price            11  389 = [%.11s]\n",  ptr->bid3_price);
    LogRaw( "채권매도3단계우선호가잔량     bond_ask3_remain_vol  15  400 = [%.15s]\n",  ptr->bond_ask3_remain_vol);
    LogRaw( "채권매수3단계우선호가잔량     bond_bid3_remain_vol  15  415 = [%.15s]\n",  ptr->bond_bid3_remain_vol);
    LogRaw( "매도3단계우선호가수익률       ask3_yield            13  430 = [%.13s]\n",  ptr->ask3_yield);
    LogRaw( "매수3단계우선호가수익률       bid3_yield            13  443 = [%.13s]\n",  ptr->bid3_yield);
    LogRaw( "매도4단계우선호가가격         ask4_price            11  456 = [%.11s]\n",  ptr->ask4_price);
    LogRaw( "매수4단계우선호가가격         bid4_price            11  467 = [%.11s]\n",  ptr->bid4_price);
    LogRaw( "채권매도4단계우선호가잔량     bond_ask4_remain_vol  15  478 = [%.15s]\n",  ptr->bond_ask4_remain_vol);
    LogRaw( "채권매수4단계우선호가잔량     bond_bid4_remain_vol  15  493 = [%.15s]\n",  ptr->bond_bid4_remain_vol);
    LogRaw( "매도4단계우선호가수익률       ask4_yield            13  508 = [%.13s]\n",  ptr->ask4_yield);
    LogRaw( "매수4단계우선호가수익률       bid4_yield            13  521 = [%.13s]\n",  ptr->bid4_yield);
    LogRaw( "매도5단계우선호가가격         ask5_price            11  534 = [%.11s]\n",  ptr->ask5_price);
    LogRaw( "매수5단계우선호가가격         bid5_price            11  545 = [%.11s]\n",  ptr->bid5_price);
    LogRaw( "채권매도5단계우선호가잔량     bond_ask5_remain_vol  15  556 = [%.15s]\n",  ptr->bond_ask5_remain_vol);
    LogRaw( "채권매수5단계우선호가잔량     bond_bid5_remain_vol  15  571 = [%.15s]\n",  ptr->bond_bid5_remain_vol);
    LogRaw( "매도5단계우선호가수익률       ask5_yield            13  586 = [%.13s]\n",  ptr->ask5_yield);
    LogRaw( "매수5단계우선호가수익률       bid5_yield            13  599 = [%.13s]\n",  ptr->bid5_yield);
    LogRaw( "채권매도호가총잔량            bond_total_ask_vol    15  612 = [%.15s]\n",  ptr->bond_total_ask_vol);
    LogRaw( "채권매수호가총잔량            bond_total_bid_vol    15  627 = [%.15s]\n",  ptr->bond_total_bid_vol);
    LogRaw( "정보분배메세지종료키워드      msg_end_key            1  642 = [%.1s]\n",   ptr->msg_end_key);
    LogRaw( "%s", "------------------------------------------------------------------------[ CO_G701K ]----\n");

    return sizeof( CO_G701K);
}

int BLP_ARG_Print( BLP_ARG* ptr)
{
	int			i;
	BLP_TIME	*mk_time;

    LogRaw( "%s", "----[ BLP_ARG ]-------------------------------------------------------------------------\n");
    LogRaw( "실행일자                      exe_ymd           9    0 = [%.9s]\n",   ptr->exe_ymd);
    LogRaw( "실행번호                      exe_no            4    9 = [%d]\n",     ptr->exe_no);
    LogRaw( "처리상태구분코드              proc_stus_dstcd   2   13 = [%.2s]\n",   ptr->proc_stus_dstcd);
#if 0
    LogRaw( "시작일시                      start_yms        15   15 = [%.15s]\n",  ptr->start_yms);
    LogRaw( "종료일시                      end_yms          15   30 = [%.15s]\n",  ptr->end_yms);
    LogRaw( "유동성공급시작일시            lp_start_yms     15   45 = [%.15s]\n",  ptr->lp_start_yms);
    LogRaw( "유동성공급종료일시            lp_end_yms       15   60 = [%.15s]\n",  ptr->lp_end_yms);
#endif
    LogRaw( "종목코드                      item_cd          13   75 = [%.13s]\n",  ptr->item_cd);
    LogRaw( "유동성공급종목구분코드        mm_item_dstcd     3   88 = [%.3s]\n",   ptr->mm_item_dstcd);
    LogRaw( "주문수량단위                  ord_qanty_unit    4  139 = [%d]\n",     ptr->ord_qanty_unit);
    LogRaw( "괴리율대상구분코드            dspratio_taget_dstcd   3  143 = [%.3s]\n",      ptr->dspratio_taget_dstcd);
    LogRaw( "KRX괴리율구분코드             dspratio_dstcd    2  146 = [%.2s]\n",   ptr->dspratio_dstcd);
    LogRaw( "괴리율                        dspratio          8  148 = [%f]\n",     ptr->dspratio);
    LogRaw( "틱단위                        tick_unit         8  156 = [%f]\n",     ptr->tick_unit);
    LogRaw( "거래원번호                    trdr_uno          6  164 = [%.6s]\n",   ptr->trdr_uno);
    LogRaw( "CMBS트레이더번호              trdr_no           5  170 = [%.5s]\n",   ptr->trdr_no);
    LogRaw( "민간평가수익률                prv_yild          8  175 = [%f]\n",     ptr->prv_yild);
    LogRaw( "민간평가가격                  prv_prc           8  183 = [%f]\n",     ptr->prv_prc);
    LogRaw( "종가                          clsng_prc         8  191 = [%f]\n",     ptr->clsng_prc);
    LogRaw( "종가수익률                    clsng_yild        8  199 = [%f]\n",     ptr->clsng_yild);
    LogRaw( "계좌번호                      account_no       12  207 = [%.12s]\n",  ptr->account_no);
    LogRaw( "시장                          mk_stat          12  207 = [%d]\n",     ptr->mk_stat);
	for( i = 0; i < BLP_MAX_MK_STAT; i++)
	{
		mk_time = &ptr->mk_time[ i];
    	LogRaw( "mk_time [%d]                                                 \n",     i);
    	LogRaw( "    LP 조성 시작 시간         start             8   91 = [%s]\n",     TtoS( mk_time->start));
    	LogRaw( "    LP 조성 종료 시간         end               8   91 = [%s]\n",     TtoS( mk_time->end));
    	LogRaw( "    체결이후 조성 대기 시간   exe_delay         8   91 = [%d]\n",     mk_time->exe_delay);
    	LogRaw( "    반대매매 유지시간         rev_wait          8   91 = [%d]\n",     mk_time->rev_wait);
    	LogRaw( "    시장조성 제출 한도 시간   submit_limit      8   91 = [%d]\n",     mk_time->submit_limit);
    	LogRaw( "    매매 정리 여부            end_stat          8   91 = [%d]\n",     mk_time->end_stat);
    	LogRaw( "    스프래드1가격             sped_prc[0]       8   91 = [%f]\n",     mk_time->sped_prc[ 0]);
    	LogRaw( "    스프래드2가격             sped_prc[1]       8   99 = [%f]\n",     mk_time->sped_prc[ 1]);
    	LogRaw( "    스프래드3가격             sped_prc[2]       8  107 = [%f]\n",     mk_time->sped_prc[ 2]);
    	LogRaw( "    주문1수량                 ord_qanty[0]      8  115 = [%f]\n",     mk_time->ord_qanty[ 0]);
    	LogRaw( "    주문2수량                 ord_qanty[1]      8  123 = [%f]\n",     mk_time->ord_qanty[ 1]);
    	LogRaw( "    주문3수량                 ord_qanty[2]      8  131 = [%f]\n",     mk_time->ord_qanty[ 2]);
	}
    LogRaw( "%s", "-------------------------------------------------------------------------[ BLP_ARG ]----\n");

    return sizeof( BLP_ARG);
}

int BLP_SISE_Print( BLP_SISE* ptr)
{
	int		i, j;

    LogRaw( "%s", "----[ BLP_SISE ]------------------------------------------------------------------------\n");
    LogRaw( "                              tv                     8    0 = [%s:%06d]\n",  TtoS( ptr->tv.tv_sec), ptr->tv.tv_usec);
    LogRaw( "                              tv_old                 8   16 = [%s:%06d]\n",  TtoS( ptr->tv_old.tv_sec), ptr->tv_old.tv_usec);
    LogRaw( "호가 처리 건수                ho_cnt                 4   32 = [%d]\n",     ptr->ho_cnt);
    LogRaw( "체결 처리 건수                exe_cnt                4   36 = [%d]\n",     ptr->exe_cnt);
    LogRaw( "체결 가격                     price                  8   40 = [%f]\n",     ptr->price);
    LogRaw( "체결 수량                     volume                 8   48 = [%f]\n",     ptr->volume);

	for( i = 0; i < 2; i++)
	{
	for( j = 0; j < BLP_SISE_HOGA; j++)
	{
    LogRaw( "매도 [ %3d][ %3d]   ask                              8   56 = [%f]\n",i, j, ptr->ask[ i][ j]);
    LogRaw( "매수 [ %3d][ %3d]   bid                              8   64 = [%f]\n",i, j, ptr->bid[ i][ j]);
	}
	}
    LogRaw( "%s", "------------------------------------------------------------------------[ BLP_SISE ]----\n");

    return sizeof( BLP_SISE);
}

int KRX_NOTE_JUMUN_DATA_Print( KRX_NOTE_JUMUN_DATA* ptr)
{
    LogRaw( "%s", "----[ KRX_NOTE_JUMUN_DATA ]-------------------------------------------------------------\n");
    LogRaw( "메세지일련번호                DataSeq               11    0 = [%.11s]\n",  ptr->DataSeq);
    LogRaw( "트랜잭션코드                  Transaction_Code      11   11 = [%.11s]\n",  ptr->Transaction_Code);
    LogRaw( "ME그룹번호                    Megrp_no               2   22 = [%.2s]\n",   ptr->Megrp_no);
    LogRaw( "시장ID                        Undly_Asset_Mkt_Id     3   24 = [%.3s]\n",   ptr->Undly_Asset_Mkt_Id);
    LogRaw( "보드ID                        Board_id               2   27 = [%.2s]\n",   ptr->Board_id);
    LogRaw( "회원번호                      MembershipNo           5   29 = [%.5s]\n",   ptr->MembershipNo);
    LogRaw( "지점번호                      BranchNo               5   34 = [%.5s]\n",   ptr->BranchNo);
    LogRaw( "주문ID                        OrderNo               10   39 = [%.10s]\n",  ptr->OrderNo);
    LogRaw( "원주문ID                      OriginalOrderNo       10   49 = [%.10s]\n",  ptr->OriginalOrderNo);
    LogRaw( "종목코드                      ItemCode              12   59 = [%.12s]\n",  ptr->ItemCode);
    LogRaw( "매도매수구분코드              TradeFlag              1   71 = [%.1s]\n",   ptr->TradeFlag);
    LogRaw( "정정취소구분코드              New_Modify_Cancel_gbn  1   72 = [%.1s]\n",   ptr->New_Modify_Cancel_gbn);
    LogRaw( "계좌번호                      AccountNo             12   73 = [%.12s]\n",  ptr->AccountNo);
    LogRaw( "호가수량                      OrderQuantity         10   85 = [%.10s]\n",  ptr->OrderQuantity);
    LogRaw( "호가가격                      Price                 11   95 = [%.11s]\n",  ptr->Price);
    LogRaw( "호가유형코드                  Order_Type             1  106 = [%.1s]\n",   ptr->Order_Type);
    LogRaw( "호가조건코드                  Order_Condition        1  107 = [%.1s]\n",   ptr->Order_Condition);
    LogRaw( "매도유형코드                  Ask_Type               2  108 = [%.2s]\n",   ptr->Ask_Type);
    LogRaw( "위탁자기구분코드              Trust_Principal_Type   2  110 = [%.2s]\n",   ptr->Trust_Principal_Type);
    LogRaw( "위탁사번호                    Trust_Company_No       5  112 = [%.5s]\n",   ptr->Trust_Company_No);
    LogRaw( "계좌구분코드                  Account_Type           2  117 = [%.2s]\n",   ptr->Account_Type);
    LogRaw( "국가코드                      Country_Code           3  119 = [%.3s]\n",   ptr->Country_Code);
    LogRaw( "투자자구분코드                Investor_Type          4  122 = [%.4s]\n",   ptr->Investor_Type);
    LogRaw( "필러값                        Filler                 6  126 = [%.6s]\n",   ptr->Filler);
    LogRaw( "외국인투자자구분코드          Foreign_Investor_Type  2  132 = [%.2s]\n",   ptr->Foreign_Investor_Type);
    LogRaw( "소액채권장종료매매참여여부    Sm_Bond_Part_Yn        1  134 = [%.1s]\n",   ptr->Sm_Bond_Part_Yn);
    LogRaw( "비과세여부                    Tax_Exempt_Yn          1  135 = [%.1s]\n",   ptr->Tax_Exempt_Yn);
    LogRaw( "주문매체구분코드              Order_Mesia_Type       1  136 = [%.1s]\n",   ptr->Order_Mesia_Type);
    LogRaw( "주문자식별정보                Order_Identi          12  137 = [%.12s]\n",  ptr->Order_Identi);
    LogRaw( "MAC주소                       Mac_Addr              12  149 = [%.12s]\n",  ptr->Mac_Addr);
    LogRaw( "호가일자                      Order_Date             8  161 = [%.8s]\n",   ptr->Order_Date);
    LogRaw( "회원사주문시각                Member_Send_Time       9  169 = [%.9s]\n",   ptr->Member_Send_Time);
    LogRaw( "회원사용영역                  MembershipItem        60  178 = [%.60s]\n",  ptr->MembershipItem);
    LogRaw( "거래원번호                    Trdr_Id                5  238 = [%.5s]\n",   ptr->Trdr_Id);
    LogRaw( "시장조성자호가구분번호        Mm_Order_Type_No      11  243 = [%.11s]\n",  ptr->Mm_Order_Type_No);
    LogRaw( "%s", "-------------------------------------------------------------[ KRX_NOTE_JUMUN_DATA ]----\n");

    return sizeof( KRX_NOTE_JUMUN_DATA);
}

int BLP_MEMBER_AREA_Print( BLP_MEMBER_AREA* ptr)
{
    LogRaw( "%s", "----[ BLP_MEMBER_AREA ]-----------------------------------------------------------------\n");
    LogRaw( "원장 사용 영역                reserved              30    0 = [%.30s]\n",  ptr->reserved);
    LogRaw( "자동주문 여부 A=자동주문 C=C  auto_yn                1   30 = [%.1s]\n",   ptr->auto_yn);
    LogRaw( "전략 여부 전략=ST             strategy               2   31 = [%.2s]\n",   ptr->strategy);
    LogRaw( "전략번호                      stg_no                 2   33 = [%.2s]\n",   ptr->stg_no);
    LogRaw( "시장구분 01=채권일반 02=채권  market                 2   35 = [%.2s]\n",   ptr->market);
    LogRaw( "A0 순번                       index                  5   37 = [%.5s]\n",   ptr->index);
    LogRaw( "계좌번호 Seq                  acc_seq                2   42 = [%.2s]\n",   ptr->acc_seq);
    LogRaw( "유가증권 주식선물 구분 채권=  stock                  1   44 = [%.1s]\n",   ptr->stock);
    LogRaw( "스프레드여부 0:normal, 1:스? spread                 1   45 = [%.1s]\n",   ptr->spread);
    LogRaw( "Process Nick Name pa_50203mp  proc_nm                4   46 = [%.4s]\n",   ptr->proc_nm);
    LogRaw( "%s", "-----------------------------------------------------------------[ BLP_MEMBER_AREA ]----\n");

    return sizeof( BLP_MEMBER_AREA);
}


int KRX_LP_NOTE_JUMUN_DATA_Print( KRX_LP_NOTE_JUMUN_DATA* ptr)
{
    LogRaw( "%s", "----[ KRX_LP_NOTE_JUMUN_DATA ]----------------------------------------------------------\n");
    LogRaw( "메세지일련번호                DataSeq               11    0 = [%.11s]\n",  ptr->DataSeq);
    LogRaw( "트랜잭션코드                  Transaction_Code      11   11 = [%.11s]\n",  ptr->Transaction_Code);
    LogRaw( "ME그룹번호                    Megrp_no               2   22 = [%.2s]\n",   ptr->Megrp_no);
    LogRaw( "시장ID                        Trd_Mkt_Choic_Tp_Cd    3   24 = [%.3s]\n",   ptr->Trd_Mkt_Choic_Tp_Cd);
    LogRaw( "보드ID                        Board_id               2   27 = [%.2s]\n",   ptr->Board_id);
    LogRaw( "회원번호                      MembershipNo           5   29 = [%.5s]\n",   ptr->MembershipNo);
    LogRaw( "지점번호                      BranchNo               5   34 = [%.5s]\n",   ptr->BranchNo);
    LogRaw( "주문ID                        OrderNo               10   39 = [%.10s]\n",  ptr->OrderNo);
    LogRaw( "원주문ID                      OriginalOrderNo       10   49 = [%.10s]\n",  ptr->OriginalOrderNo);
    LogRaw( "종목코드                      ItemCode              12   59 = [%.12s]\n",  ptr->ItemCode);
    LogRaw( "매도매수구분코드              TradeFlag              1   71 = [%.1s]\n",   ptr->TradeFlag);
    LogRaw( "정정취소구분코드              New_Modify_Cancel_gbn  1   72 = [%.1s]\n",   ptr->New_Modify_Cancel_gbn);
    LogRaw( "채권호가종류코드              Bond_Offer_Type_Cd     1   73 = [%.1s]\n",   ptr->Bond_Offer_Type_Cd);
    LogRaw( "계좌번호                      AccountNo             12   74 = [%.12s]\n",  ptr->AccountNo);
    LogRaw( "매도호가수량                  Ask_Offer_Qty         10   86 = [%.10s]\n",  ptr->Ask_Offer_Qty);
    LogRaw( "매도호가가격                  Ask_Offer_Prc         11   96 = [%.11s]\n",  ptr->Ask_Offer_Prc);
    LogRaw( "매수호가수량                  Bid_Offer_Qty         10  107 = [%.10s]\n",  ptr->Bid_Offer_Qty);
    LogRaw( "매수호가가격                  Bid_Offer_Prc         11  117 = [%.11s]\n",  ptr->Bid_Offer_Prc);
    LogRaw( "호가유형코드                  Order_Type             1  128 = [%.1s]\n",   ptr->Order_Type);
    LogRaw( "호가조건코드                  Order_Condition        1  129 = [%.1s]\n",   ptr->Order_Condition);
    LogRaw( "투자자구분코드                Investor_Type          4  130 = [%.4s]\n",   ptr->Investor_Type);
    LogRaw( "효력정지재개구분코드      Effect_Suspend_Resume_Cd   1  134 = [%.1s]\n",   ptr->Effect_Suspend_Resume_Cd);
    LogRaw( "주문매체구분코드              Order_Mesia_Type       1  135 = [%.1s]\n",   ptr->Order_Mesia_Type);
    LogRaw( "주문자식별정보                Order_Identi          12  136 = [%.12s]\n",  ptr->Order_Identi);
    LogRaw( "MAC주소                       Mac_Addr              12  148 = [%.12s]\n",  ptr->Mac_Addr);
    LogRaw( "호가일자                      Order_Date             8  160 = [%.8s]\n",   ptr->Order_Date);
    LogRaw( "회원사주문시각                Member_Send_Time       9  168 = [%.9s]\n",   ptr->Member_Send_Time);
    LogRaw( "회원사용영역                  MembershipItem        60  177 = [%.60s]\n",  ptr->MembershipItem);
	BLP_MEMBER_AREA_Print( ( BLP_MEMBER_AREA *)ptr->MembershipItem);
    LogRaw( "거래원번호                    Trdr_Id                5  237 = [%.5s]\n",   ptr->Trdr_Id);
    LogRaw( "시장조성자호가구분번호        Mm_Order_Type_No      11  242 = [%.11s]\n",  ptr->Mm_Order_Type_No);
    LogRaw( "계좌구분코드                  Account_Type           2  253 = [%.2s]\n",   ptr->Account_Type);
    LogRaw( "%s", "----------------------------------------------------------[ KRX_LP_NOTE_JUMUN_DATA ]----\n");

    return sizeof( KRX_LP_NOTE_JUMUN_DATA);
}

int FILE_BUFF_FORMAT_Print( FILE_BUFF_FORMAT* ptr)
{
    LogRaw( "%s", "----[ FILE_BUFF_FORMAT ]----------------------------------------------------------------\n");
    LogRaw( "일련번호                      Seq                    8    0 = [%.8s]\n",   ptr->Seq);
    LogRaw( "통신일련번호                  If_Seq                 8    8 = [%.8s]\n",   ptr->If_Seq);
    LogRaw( "업무구분식별자                ApType                 8   16 = [%.8s]\n",   ptr->ApType);
    LogRaw( "응답코드                      ResponseCode           4   24 = [%.4s]\n",   ptr->ResponseCode);
    LogRaw( "system 시각 (nnnnnnnnnn (sec  RecvTime1             10   28 = [%.10s]\n",  ptr->RecvTime1);
    LogRaw( "system 시각 (HHMMSSmmmmmm)    RecvTime2             12   38 = [%.12s]\n",  ptr->RecvTime2);
    LogRaw( "data header                   DataHeader            20   50 = [%.20s]\n",  ptr->DataHeader);
    LogRaw( "data                          Data                 400   70 = [%.400s]\n",         ptr->Data);
    LogRaw( "line feed (0x0a)              LineFeed               1  470 = [%.1s]\n",   ptr->LineFeed);
    LogRaw( "%s", "----------------------------------------------------------------[ FILE_BUFF_FORMAT ]----\n");

    return sizeof( FILE_BUFF_FORMAT);
}

int KRX_LP_NOTE_SETTLE_RESP_DATA_Print( KRX_LP_NOTE_SETTLE_RESP_DATA* ptr)
{
    LogRaw( "%s", "----[ KRX_LP_NOTE_SETTLE_RESP_DATA ]----------------------------------------------------\n");
    LogRaw( "메세지일련번호                DataSeq               11    0 = [%.11s]\n",  ptr->DataSeq);
    LogRaw( "트랜잭션코드                  Transaction_Code      11   11 = [%.11s]\n",  ptr->Transaction_Code);
    LogRaw( "ME그룹번호                    Megrp_no               2   22 = [%.2s]\n",   ptr->Megrp_no);
    LogRaw( "시장ID                        Trd_Mkt_Choic_Tp_Cd    3   24 = [%.3s]\n",   ptr->Trd_Mkt_Choic_Tp_Cd);
    LogRaw( "보드ID                        Board_id               2   27 = [%.2s]\n",   ptr->Board_id);
    LogRaw( "회원번호                      MembershipNo           5   29 = [%.5s]\n",   ptr->MembershipNo);
    LogRaw( "지점번호                      BranchNo               5   34 = [%.5s]\n",   ptr->BranchNo);
    LogRaw( "주문ID                        OrderNo               10   39 = [%.10s]\n",  ptr->OrderNo);
    LogRaw( "원주문ID                      OriginalOrderNo       10   49 = [%.10s]\n",  ptr->OriginalOrderNo);
    LogRaw( "종목코드                      ItemCode              12   59 = [%.12s]\n",  ptr->ItemCode);
    LogRaw( "매도매수구분코드              TradeFlag              1   71 = [%.1s]\n",   ptr->TradeFlag);
    LogRaw( "정정취소구분코드              New_Modify_Cancel_gbn   1   72 = [%.1s]\n",  ptr->New_Modify_Cancel_gbn);
    LogRaw( "채권호가종류코드              Bond_Offer_Type_Cd     1   73 = [%.1s]\n",   ptr->Bond_Offer_Type_Cd);
    LogRaw( "계좌번호                      AccountNo             12   74 = [%.12s]\n",  ptr->AccountNo);
    LogRaw( "매도호가수량                  Ask_Offer_Qty         10   86 = [%.10s]\n",  ptr->Ask_Offer_Qty);
    LogRaw( "매도호가가격                  Ask_Offer_Prc         11   96 = [%.11s]\n",  ptr->Ask_Offer_Prc);
    LogRaw( "매도호가수익률                Ask_Offer_Yield       13  107 = [%.13s]\n",  ptr->Ask_Offer_Yield);
    LogRaw( "매수호가수량                  Bid_Offer_Qty         10  120 = [%.10s]\n",  ptr->Bid_Offer_Qty);
    LogRaw( "매수호가가격                  Bid_Offer_Prc         11  130 = [%.11s]\n",  ptr->Bid_Offer_Prc);
    LogRaw( "매수호가수익률                Bid_Offer_Yield       13  141 = [%.13s]\n",  ptr->Bid_Offer_Yield);
    LogRaw( "호가유형코드                  Order_Type             1  154 = [%.1s]\n",   ptr->Order_Type);
    LogRaw( "호가조건코드                  Order_Condition        1  155 = [%.1s]\n",   ptr->Order_Condition);
    LogRaw( "투자자구분코드                Investor_Type          4  156 = [%.4s]\n",   ptr->Investor_Type);
    LogRaw( "효력정지재개구분코드          Effect_Suspend_Resume_Cd   1  160 = [%.1s]\n",       ptr->Effect_Suspend_Resume_Cd);
    LogRaw( "주문매체구분코드              Order_Mesia_Type       1  161 = [%.1s]\n",   ptr->Order_Mesia_Type);
    LogRaw( "주문자식별정보                Order_Identi          12  162 = [%.12s]\n",  ptr->Order_Identi);
    LogRaw( "MAC주소                       Mac_Addr              12  174 = [%.12s]\n",  ptr->Mac_Addr);
    LogRaw( "호가일자                      Order_Date             8  186 = [%.8s]\n",   ptr->Order_Date);
    LogRaw( "회원사주문시각                Member_Send_Time       9  194 = [%.9s]\n",   ptr->Member_Send_Time);
    LogRaw( "회원사용영역                  MembershipItem        60  203 = [%.60s]\n",  ptr->MembershipItem);
    LogRaw( "호가접수시각                  Order_Accept_Time      9  263 = [%.9s]\n",   ptr->Order_Accept_Time);
    LogRaw( "거래원번호                    Trdr_Id                5  272 = [%.5s]\n",   ptr->Trdr_Id);
    LogRaw( "자동취소처리구분코드          Auto_Cancel_Process_Type   1  277 = [%.1s]\n",       ptr->Auto_Cancel_Process_Type);
    LogRaw( "호가거부사유코드              Order_Rejected_Reason   4  278 = [%.4s]\n",  ptr->Order_Rejected_Reason);
    LogRaw( "시장조성자호가구분번호        Mm_Order_Type_No      11  282 = [%.11s]\n",  ptr->Mm_Order_Type_No);
    LogRaw( "계좌구분코드                  Account_Type           2  293 = [%.2s]\n",   ptr->Account_Type);
    LogRaw( "%s", "----------------------------------------------------[ KRX_LP_NOTE_SETTLE_RESP_DATA ]----\n");

    return sizeof( KRX_LP_NOTE_SETTLE_RESP_DATA);
}

int KRX_NOTE_SETTLE_DATA_Print( KRX_NOTE_SETTLE_DATA* ptr)
{
    LogRaw( "%s", "----[ KRX_NOTE_SETTLE_DATA ]------------------------------------------------------------\n");
    LogRaw( "메세지일련번호                DataSeq               11    0 = [%.11s]\n",  ptr->DataSeq);
    LogRaw( "트랜잭션코드                  TrCode                11   11 = [%.11s]\n",  ptr->TrCode);
    LogRaw( "ME그룹번호                    Megrp_no               2   22 = [%.2s]\n",   ptr->Megrp_no);
    LogRaw( "시장ID                        Undly_Asset_Mkt_Id     3   24 = [%.3s]\n",   ptr->Undly_Asset_Mkt_Id);
    LogRaw( "보드ID                        Board_id               2   27 = [%.2s]\n",   ptr->Board_id);
    LogRaw( "회원번호                      MembershipNo           5   29 = [%.5s]\n",   ptr->MembershipNo);
    LogRaw( "지점번호                      BranchNo               5   34 = [%.5s]\n",   ptr->BranchNo);
    LogRaw( "주문ID                        OrderNo               10   39 = [%.10s]\n",  ptr->OrderNo);
    LogRaw( "원주문ID                      OriginalOrderNo       10   49 = [%.10s]\n",  ptr->OriginalOrderNo);
    LogRaw( "종목코드                      ItemCode              12   59 = [%.12s]\n",  ptr->ItemCode);
    LogRaw( "체결번호                      Trading_No            11   71 = [%.11s]\n",  ptr->Trading_No);
    LogRaw( "체결수익률                    Trading_Yield         13   82 = [%.13s]\n",  ptr->Trading_Yield);
    LogRaw( "체결가격                      Trading_price         11   95 = [%.11s]\n",  ptr->Trading_price);
    LogRaw( "체결수량                      Trading_Volumn        10  106 = [%.10s]\n",  ptr->Trading_Volumn);
    LogRaw( "세션ID                        Session_Id             2  116 = [%.2s]\n",   ptr->Session_Id);
    LogRaw( "체결일자                      Trading_Date           8  118 = [%.8s]\n",   ptr->Trading_Date);
    LogRaw( "체결시각                      Trading_Time           9  126 = [%.9s]\n",   ptr->Trading_Time);
    LogRaw( "매도매수구분코드              TradeFlag              1  135 = [%.1s]\n",   ptr->TradeFlag);
    LogRaw( "매도유형코드                  Ask_Type               2  136 = [%.2s]\n",   ptr->Ask_Type);
    LogRaw( "계좌번호                      AccountNo             12  138 = [%.12s]\n",  ptr->AccountNo);
    LogRaw( "채권호가종류코드              Bond_Order_Type        1  150 = [%.1s]\n",   ptr->Bond_Order_Type);
    LogRaw( "호가수량                      OrderQuantity         10  151 = [%.10s]\n",  ptr->OrderQuantity);
    LogRaw( "호가가격                      Price                 11  161 = [%.11s]\n",  ptr->Price);
    LogRaw( "주문수익률                    Order_Yield           13  172 = [%.13s]\n",  ptr->Order_Yield);
    LogRaw( "위탁자기구분코드              Trust_Principal_Type   2  185 = [%.2s]\n",   ptr->Trust_Principal_Type);
    LogRaw( "위탁사번호                    Trust_Company_No       5  187 = [%.5s]\n",   ptr->Trust_Company_No);
    LogRaw( "계좌구분코드                  Account_Type           2  192 = [%.2s]\n",   ptr->Account_Type);
    LogRaw( "투자자구분코드                Investor_Type          4  194 = [%.4s]\n",   ptr->Investor_Type);
    LogRaw( "필러값                        Filler                 6  198 = [%.6s]\n",   ptr->Filler);
    LogRaw( "외국인투자자구분코드          Foreign_Investor_Type   2  204 = [%.2s]\n",  ptr->Foreign_Investor_Type);
    LogRaw( "주문매체구분코드              Order_Mesia_Type       1  206 = [%.1s]\n",   ptr->Order_Mesia_Type);
    LogRaw( "주문자식별정보                Order_Identi          12  207 = [%.12s]\n",  ptr->Order_Identi);
    LogRaw( "MAC주소                       Mac_Addr              12  219 = [%.12s]\n",  ptr->Mac_Addr);
    LogRaw( "효력정지재개구분코드          Effect_Suspend_Resume   1  231 = [%.1s]\n",  ptr->Effect_Suspend_Resume);
    LogRaw( "회원사용영역                  MembershipItem        60  232 = [%.60s]\n",  ptr->MembershipItem);
    LogRaw( "거래원번호                    Trdr_Id                5  292 = [%.5s]\n",   ptr->Trdr_Id);
    LogRaw( "결제일자                      Settle_Date            8  297 = [%.8s]\n",   ptr->Settle_Date);
    LogRaw( "시장조성자호가구분번호        Mm_Order_Type_No      11  305 = [%.11s]\n",  ptr->Mm_Order_Type_No);
    LogRaw( "최종매도매수구분코드          Last_Ask_Bid_Type      1  316 = [%.1s]\n",   ptr->Last_Ask_Bid_Type);
    LogRaw( "%s", "------------------------------------------------------------[ KRX_NOTE_SETTLE_DATA ]----\n");

    return sizeof( KRX_NOTE_SETTLE_DATA);
}

int KRX_NOTE_SETTLE_RESP_DATA_Print( KRX_NOTE_SETTLE_RESP_DATA* ptr)
{
    LogRaw( "%s", "----[ KRX_NOTE_SETTLE_RESP_DATA ]-------------------------------------------------------\n");
    LogRaw( "메세지일련번호                DataSeq               11    0 = [%.11s]\n",  ptr->DataSeq);
    LogRaw( "트랜잭션코드                  TrCode                11   11 = [%.11s]\n",  ptr->TrCode);
    LogRaw( "ME그룹번호                    Megrp_no               2   22 = [%.2s]\n",   ptr->Megrp_no);
    LogRaw( "시장ID                        Undly_Asset_Mkt_Id     3   24 = [%.3s]\n",   ptr->Undly_Asset_Mkt_Id);
    LogRaw( "보드ID                        Board_id               2   27 = [%.2s]\n",   ptr->Board_id);
    LogRaw( "회원번호                      MembershipNo           5   29 = [%.5s]\n",   ptr->MembershipNo);
    LogRaw( "지점번호                      BranchNo               5   34 = [%.5s]\n",   ptr->BranchNo);
    LogRaw( "주문ID                        OrderNo               10   39 = [%.10s]\n",  ptr->OrderNo);
    LogRaw( "원주문ID                      OriginalOrderNo       10   49 = [%.10s]\n",  ptr->OriginalOrderNo);
    LogRaw( "종목코드                      ItemCode              12   59 = [%.12s]\n",  ptr->ItemCode);
    LogRaw( "매도매수구분코드              TradeFlag              1   71 = [%.1s]\n",   ptr->TradeFlag);
    LogRaw( "정정취소구분코드              New_Modify_Cancel_gbn   1   72 = [%.1s]\n",  ptr->New_Modify_Cancel_gbn);
    LogRaw( "계좌번호                      AccountNo             12   73 = [%.12s]\n",  ptr->AccountNo);
    LogRaw( "호가수량                      OrderQuantity         10   85 = [%.10s]\n",  ptr->OrderQuantity);
    LogRaw( "호가가격                      Price                 11   95 = [%.11s]\n",  ptr->Price);
    LogRaw( "호가수익률                    Order_Yield           13  106 = [%.13s]\n",  ptr->Order_Yield);
    LogRaw( "호가유형코드                  Order_Type             1  119 = [%.1s]\n",   ptr->Order_Type);
    LogRaw( "호가조건코드                  Order_Condition        1  120 = [%.1s]\n",   ptr->Order_Condition);
    LogRaw( "매도유형코드                  Ask_Type               2  121 = [%.2s]\n",   ptr->Ask_Type);
    LogRaw( "위탁자기구분코드              Trust_Principal_Type   2  123 = [%.2s]\n",   ptr->Trust_Principal_Type);
    LogRaw( "위탁사번호                    Trust_Company_No       5  125 = [%.5s]\n",   ptr->Trust_Company_No);
    LogRaw( "계좌구분코드                  Account_Type           2  130 = [%.2s]\n",   ptr->Account_Type);
    LogRaw( "국가코드                      Country_Code           3  132 = [%.3s]\n",   ptr->Country_Code);
    LogRaw( "투자자구분코드                Investor_Type          4  135 = [%.4s]\n",   ptr->Investor_Type);
    LogRaw( "필러값                        Filler                 6  139 = [%.6s]\n",   ptr->Filler);
    LogRaw( "외국인투자자구분코드          Foreign_Investor_Type   2  145 = [%.2s]\n",  ptr->Foreign_Investor_Type);
    LogRaw( "소액채권장종료매매참여여부    Sm_Bond_Part_Yn        1  147 = [%.1s]\n",   ptr->Sm_Bond_Part_Yn);
    LogRaw( "비과세여부                    Tax_Exempt_Yn          1  148 = [%.1s]\n",   ptr->Tax_Exempt_Yn);
    LogRaw( "주문매체구분코드              Order_Mesia_Type       1  149 = [%.1s]\n",   ptr->Order_Mesia_Type);
    LogRaw( "주문자식별정보                Order_Identi          12  150 = [%.12s]\n",  ptr->Order_Identi);
    LogRaw( "MAC주소                       Mac_Addr              12  162 = [%.12s]\n",  ptr->Mac_Addr);
    LogRaw( "호가일자                      Order_Date             8  174 = [%.8s]\n",   ptr->Order_Date);
    LogRaw( "회원사주문시각                Member_Send_Time       9  182 = [%.9s]\n",   ptr->Member_Send_Time);
    LogRaw( "회원사용영역                  MembershipItem        60  191 = [%.60s]\n",  ptr->MembershipItem);
    LogRaw( "호가접수시각                  Order_Accept_Time      9  251 = [%.9s]\n",   ptr->Order_Accept_Time);
    LogRaw( "거래원번호                    Trdr_Id                5  260 = [%.5s]\n",   ptr->Trdr_Id);
    LogRaw( "실정정취소호가수량            Real_Modify_Cancel_Cnt  10  265 = [%.10s]\n",        ptr->Real_Modify_Cancel_Cnt);
    LogRaw( "자동취소처리구분코드          Auto_Cancel_Process_Type   1  275 = [%.1s]\n",       ptr->Auto_Cancel_Process_Type);
    LogRaw( "호가거부사유코드              Order_Rejected_Reason   4  276 = [%.4s]\n",  ptr->Order_Rejected_Reason);
    LogRaw( "시장조성자호가구분번호        Mm_Order_Type_No      11  280 = [%.11s]\n",  ptr->Mm_Order_Type_No);
    LogRaw( "%s", "-------------------------------------------------------[ KRX_NOTE_SETTLE_RESP_DATA ]----\n");

    return sizeof( KRX_NOTE_SETTLE_RESP_DATA);
}









