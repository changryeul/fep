#ifndef _KRX_TSVIIR00203_H
#define _KRX_TSVIIR00203_H

/* TSVIIR00203 신용거래 잔고내역 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 284 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Second_Data_Request_Date[8];           /*    5 2차자료요청일자 (String 8) */
    char Base_Date[8];                          /*    6 기준일자 (String 8) */
    char Member_Number[5];                      /*    7 회원번호 (String 5) */
    char Branch_Number[5];                      /*    8 지점번호 (String 5) */
    char Account_Number[12];                    /*    9 계좌번호 (String 12) */
    char Loan_Date[8];                          /*   10 대출일자 (String 8) */
    char Expiration_Date[8];                    /*   11 만기일자 (String 8) */
    char Investigation_Information_Credit_Type_Code[1];/*   12 심리정보신용구분코드 (String 1) */
    char Trading_Issue_Code[12];                /*   13 거래종목코드 (String 12) */
    char Issue_Name[80];                        /*   14 종목명 (String 80) */
    char Loan_Unitprc[18];                      /*   15 대출단가 (Long 18) */
    char Credit_Quantity[15];                   /*   16 신용수량 (Long 15) */
    char Credit_Amount[22];                     /*   17 신용금액 (Float 22) */
    char Transaction_Redemption_Quantity[15];   /*   18 신용거래상환수량 (Long 15) */
    char Margin_Transaction_Redemption_Amount[22];/*   19 신용거래상환금액 (Float 22) */
    char Investor_Representative_Account_Number[12];/*   20 투자자대표계좌번호 (String 12) */
} TSVIIR00203_DATA;

#endif  /* _KRX_TSVIIR00203_H */
