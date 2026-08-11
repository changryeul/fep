#ifndef		__STRATEGY_H
#define		__STRATEGY_H
/*------------------------------------------------------------------------
#	Module	: common header files
#	File	: strategy.h
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"
#include	"fep_interface.h"
#include    "strategy01.h"
#include    "strategy03.h"

typedef struct
{
    char        next_flag[1];                       /* S/N/E or O */
                                                    /* S시에는 위에부분만, N/E시에는 Condition/Edit_Condition 부분을 채워서 보낸다. */
    char        strategy_no[2];                     /* 전략번호             */
    char        st_accno[12];						/* 현물주문낼 계좌번호, 대상아니면 space(stock)    */
	char		st_accno_seq[2];
    char        dv_accno[12];						/* 파생주문낼 계좌번호, 대상아니면 space(derivatives)    */
	char		dv_accno_seq[2];

    char        tot_item_cnt[2];                    /* 총 종목수량      */
    char        use_market_cnt[2];                  /* 사용할 시장수, PIPE에서 사용할 수로 사용		*/
	char		market_gbn[2][11];					/* MK_CNT는 11									*/
    char        tot_order_cnt[2];                   /* 주문낼 횟수, 0:조건맞으면 계속, 1:한번만..	*/
    struct
    {
        char    arry_no[3];							/* 일련번호, 종목의 순번 0번부터+1..    */
        char    market_gbn[2];                      /* 시장구분         */
                                                    /* 1(지수선물), 2(지수옵션)     */
                                                    /* 3(주식선물), 4(주식옵션)     */
                                                    /* 5(유가증권/ELW/ETF/ETN)      */
                                                    /* 6(코스닥), 8(KRX300),        */
                                                    /* 9(Kosdaq150F) 10(Kosdaq150O) */
        char    item_code[12];                      /* 종목코드         */
		char    item_seq[5];                        /* 종목일련번호, A0에 있는 일련번호 */
        char    target_gbn[1];                      /* 0:조건이 되는 종목
                                                       1:타겟이 되는 종목   */

        char    order_cnt[8];                       /* 1회 주문수량                 */
        char    order_type[1];                      /* 주문타입, 1:지정가, 2:시장가 */
        char    order_price_gbn[1];                 /* 주문가격구분, 0:매수>시 매수1호가, 1:매수시 상대호가매도1호가 */
                                                    /*               0:매도>시 매도1호가, 1:매도시 상대호가매수1호가 */

        char    order_price_condition[1];            /* 주문가격조건, 0:매수>시 "주문가격구분"+0, 1:매수시 "주문가격구분"+1 */
                                                    /*               0:매도>시 "주문가격구분"+0, 1:매도시 "주문가격구분"-1 */
    }   Condition;
    struct
    {
    }   Edit_Condition;                             /* 정정/취소 조건, 여기>선 없는걸로 하겠음  */
    char        tmp[387];                           /* 480 - 93 = 37 */
}   IN_SAMPLE01;        /* Client로부터 받을 샘플전략 전문, 송/수신전문은 char이 기본임 */

typedef struct
{
    int         auto_run;                           /* 자동기동여부, 0 or 1 */
/*  int         jm_stat;                               주문송신후 응답/체결에 반응해야 하는 주문은 주문 상태관리가 추가되어야 한다. */

	int			use_market_cnt;						/* 사용할 총 시장의 수				*/
	int         tot_item_cnt;                       /* 총 종목수량, Set할 Arry수로 사용 */
    int         tot_order_cnt;                      /* 주문낼 횟수, 0:조건맞으면 계속, 1:한번만.. */
    char        st_accno[12];						/* 현물주문낼 계좌번호, 대상아니면 space    */
    char        dv_accno[12];						/* 파생주문낼 계좌번호, 대상아니면 space    */
    struct
    {
        int     market_gbn;                         /* 시장구분             */
        char    item_code[12];                      /* 종목코드             */
        int     item_seq;                           /* 종목일련번호, A0에 있는 일련번호 */
        int     target_gbn;                         /* 0:조건이 되는 종목
                                                       1:타겟이 되는 종목   */
        int     order_cnt;                          /* 1회 주문수량                 */
        int     order_type;                         /* 주문타입, 1:지정가, 2:시장가 */
        int     order_price_gbn;                    /* 주문가격구분, 0:매수>시 매수1호가, 1:매수시 상대호가매도1호가 */
                                                    /*               0:매도>시 매도1호가, 1:매도시 상대호가매수1호가 */
        int     order_price_condition;               /* 주문가격조건, 0:매수>시 "주문가격구분"+0, 1:매수시 "주문가격구분"+1 */
                                                    /*               0:매도>시 "주문가격구분"+0, 1:매도시 "주문가격구분"-1 */

	}   Condition[300];          /* 주문조건을 설정시 종목별로 설정할려면 이곳에, 공통으로 설정하려면 위에,
                                    샘플처럼 조건이 명확하면 로직에만 넣고 >이곳에 않넣어도 된다.(결정하기 나름)    */
}   SHM_SAMPLE01;       /* Client로부터 받을 샘플전략 전문, 송/수신전문은 char이 기본임 */


/*************************************************************************
	End of Program (strategy.h)
*************************************************************************/
#endif
