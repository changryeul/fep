#ifndef _KRX_TCSMIP46501_H
#define _KRX_TCSMIP46501_H

/* TCSMIP46501 시장조성자증권거래세면제헤지거래신고 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 176 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sesuence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Member_Number[5];                      /*    4 회원번호 (String 5) */
    char Trading_Date[8];                       /*    5 거래일자 (String 8) */
    char Derivation_Account_Number[12];         /*    6 파생계좌번호 (String 12) */
    char Link_Spot_Account_Number[12];          /*    7 연계현물계좌번호 (String 12) */
    char Issue_Code[12];                        /*    8 종목코드 (String 12) */
    char That_Day_Trading_Limit_Stock_Quantity[15];/*    9 당일거래한도주식수량 (Long 15) */
    char Delta_Change_Limit_Stock_Quantity[15]; /*   10 델타변화한도주식수량 (Long 15) */
    char Last_Settlement_Limit_Stock_Quantity[15];/*   11 최종결제한도주식수량 (Long 15) */
    char That_Day_Ask_Quantity[15];             /*   12 당일매도수량 (Long 15) */
    char Securities_Transaction_Tax_Exemption_Quantity[15];/*   13 증권거래세면제수량 (Long 15) */
    char Securities_Transaction_Tax_Exemption_Amount[22];/*   14 증권거래세면제금액 (Float 22) */
} TCSMIP46501_DATA;

#endif  /* _KRX_TCSMIP46501_H */
