#ifndef     __STRATEGY03_H
#define     __STRATEGY03_H
/*------------------------------------------------------------------------
#   Module  : common header files
#   File    : strategy03.h
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/

#define ORDERNUM    5

#if(0)
typedef struct
{
    char    sEmpNo[8];
    char    sRunStop[1];
    char    sCode[12];
    char    sFund[12];
    char    sSBalance[8];
    char    sBBalance[8];
    char    sSTdayQty[8];      /* 누적매도체결수량 */
    char    sBTdayQty[8];      /* 누적매수체결수량 */
    char    sSellTick[8];
    char    sSellQty[8];
    char    sSellCnt[8];
    char    sBuyTick[8];
    char    sBuyQty[8];
    char    sBuyCnt[8];
    char    sRetryOrdRate[8];
    char    sStopTm[4];
    char    sBaseTp[1];    /* 기준이론가 1.IIV 2.선물 3.수동입력 */
    struct
    {
        char    sBump[8];           /* 수동입력 */
        char    sLeadCode[12];      /* 근월물 */
        char    sLeadSettle1[10];   /* 근월물정산가 */
        char    sLeadSettle2[10];   /* 근월물정산가 */
        char    sLeadRepo[10];
        char    sLeadWgt[10];       /* 근월물선물비중 */
        char    sNextCode[12];      /* 차근월물 */
        char    sNextSettle1[10];   /* 차근월물정산가 */
        char    sNextSettle2[10];   /* 차근월물정산가 */
        char    sNextRepo[10];      /* 차근월물정산가 */
    } Base;
    struct
    {
        char     sIndexDistor[8];   /* baseiiv 거래소IIV차이 */
        char     sHogaDistor[8];    /* 현재나가는호가 거래소IIV차이 */
        char     sHogaSpread[8];    /* 호가 스프레드 */
        char     sSafeQty[8];
    } Stop;
} LP_Set;
#endif

typedef struct
{
    char    sEmpNo[8];
    char    sRunStop[1];    /* 1.실행 2.정지 3.kill */
    char    sCode[12];      /* 종목명 */
    char    sFund[12];      /* 펀드번호 */
    int     iSBalance;      /* 매도잔고 */
    int     iBBalance;      /* 매출수량 */
    int     iSTdayQty;      /* 누적매도체결수량 */
    int     iBTdayQty;      /* 누적매수체결수량 */
    int     iSellTick;      /* 매도틱 */
    int     iSellQty;       /* 매도수량 */
    int     iSellCnt;       /* 매도건수 */
    int     iBuyTick;       /* 매수틱 */
    int     iBuyQty;        /* 매수수량 */
    int     iBuyCnt;        /* 매수건수 */
    double  dRetryOrdRate;  /* 재주문율(%) */
    char    sStopTm[4];     /* 주문가능종료시간 */
    int     iBaseTp;    /* 기준이론가 1.IIV 2.선물 3.수동입력 */
    struct
    {
        int     iBump;      /* 수동입력 */
        char    sLeadCode[12];  /* 근월물 */
        double  dLeadSettle1;  /* 근월물 */
        double  dLeadSettle2;  /* 근월물 */
        double  dLeadRepo;       /* 근월물 */
        double  dLeadWgt;   /* 근월물선물비중 */
        char    sNextCode[12];  /* 차근월물 */
        double  dNextSettle1;  /* 차근월물 */
        double  dNextSettle2;  /* 차근월물 */
        double  dNextRepo;       /* 근월물 */
    } Base;
    struct
    {
        double  dIndexDistor;     /* baseiiv 거래소IIV차이 */
        double  dHogaDistor;      /* 현재나가는호가 거래소IIV차이 */
        double  dHogaSpread;
        int     iSafeQty;
    } Stop;
    struct
    {
        int     iOrdNo;
        int     iOrgOrdNo;
        int     iOrdPrice;
        int     iModPrice;
        int     iOrdQty;
        int     iOrdStat;
    } SELL[ORDERNUM];
    struct
    {
        int     iOrdNo;
        int     iOrgOrdNo;
        int     iOrdPrice;
        int     iModPrice;
        int     iOrdQty;
        int     iOrdStat;
    } BUY[ORDERNUM];
    int     iBasePrice;
} LP_Strrg;

/*************************************************************************
    End of Program (strategy03.h)
*************************************************************************/
#endif
