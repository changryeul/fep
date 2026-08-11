#ifndef _KRX_TMCIFR00004_H
#define _KRX_TMCIFR00004_H

/* TMCIFR00004 투자자 구분별 체결내역(공통) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 101 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Trading_Date[8];                       /*    3 거래일자 (String 8) */
    char Investor_Type_Code[4];                 /*    4 투자자구분코드 (String 4) */
    char Foreign_Investor_Type_Code[2];         /*    5 외국인투자자구분코드 (String 2) */
    char Ask_Bid_Type_Code[1];                  /*    6 매도매수구분코드 (String 1) */
    char Issue_Code[12];                        /*    7 종목코드 (String 12) */
    char Order_Quantity[10];                    /*    8 주문수량 (Long 10) */
    char Trading_Volumn[10];                    /*    9 체결수량 (Long 10) */
    char Traded_Value[22];                      /*   10 체결금액 (Float 22) */
    char Member_Number[5];                      /*   11 회원번호 (String 5) */
    char Branch_Number[5];                      /*   12 지점번호 (String 5) */
} TMCIFR00004_DATA;

#endif  /* _KRX_TMCIFR00004_H */
