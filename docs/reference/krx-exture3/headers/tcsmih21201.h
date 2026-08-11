#ifndef _KRX_TCSMIH21201_H
#define _KRX_TCSMIH21201_H

/* TCSMIH21201 통화상품회원별인수도내역(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    5 상품완료여부 (String 1) */
    char Market_Identification[3];              /*    6 시장ID (String 3) */
    char Underlying_Asset_Code[2];              /*    7 기초자산코드 (String 2) */
    char Member_Number[5];                      /*    8 회원번호 (String 5) */
    char Non_Clrearing_Member_Number[5];        /*    9 거래전문회원번호 (String 5) */
    char Settlement_Date[8];                    /*   10 결제일자 (String 8) */
    char Futures_And_Option_Upper_Level_Class_Code[1];/*   11 선물옵션대분류코드 (String 1) */
    char Underwriting_And_Delivering_Type_Code[1];/*   12 인수도유형코드 (String 1) */
    char Settlement_Quantity[10];               /*   13 결제수량 (Long 10) */
    char Underwriting_And_Delivering_Quantity[17];/*   14 인수도수량 (Long 17) */
    char Underwriting_And_Delivering_Amount[23];/*   15 인수도금액 (Float 23) */
    char Underlying_Asset_Deduction_Underwriting_And_Delivering_Quantity[17];/*   16 기초자산차감인수도수량 (Long 17) */
    char Underwriting_And_Delivering_Settlement_Payoff[23];/*   17 인수도결제차금 (Float 23) */
    char Filler_Value[1053];                    /*   18 필러값 (String 1053) */
} TCSMIH21201_DATA;

#endif  /* _KRX_TCSMIH21201_H */
