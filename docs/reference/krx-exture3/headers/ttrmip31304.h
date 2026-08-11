#ifndef _KRX_TTRMIP31304_H
#define _KRX_TTRMIP31304_H

/* TTRMIP31304 종목마감 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 105 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Board_Id[2];                           /*    4 보드ID (String 2) */
    char Issue_Code[12];                        /*    5 종목코드 (String 12) */
    char Issue_Close_Closing_Price[11];         /*    6 종목마감종가 (Float 11) */
    char Issue_Close_Price_Type_Code[1];        /*    7 종목마감가격구분코드 (String 1) */
    char Issue_Close_Off_Hours_Session_Single_Price_Call_Auction_Upper_Limit_Price[11];/*    8 종목마감시간외단일가상한가 (Float 11) */
    char Issue_Close_Off_Hours_Session_Single_Price_Call_Auction_Lower_Limit_Price[11];/*    9 종목마감시간외단일가하한가 (Float 11) */
    char Issue_Close_Buy_In_Base_Price[11];     /*   10 종목마감매입인도기준가격 (Float 11) */
    char Issue_Close_Buy_In_Upper_Limit_Price[11];/*   11 종목마감매입인도상한가 (Float 11) */
    char Issue_Close_Buy_In_Lower_Limit_Price[11];/*   12 종목마감매입인도하한가 (Float 11) */
} TTRMIP31304_DATA;

#endif  /* _KRX_TTRMIP31304_H */
