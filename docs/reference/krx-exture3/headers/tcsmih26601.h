#ifndef _KRX_TCSMIH26601_H
#define _KRX_TCSMIH26601_H

/* TCSMIH26601 회원별결제예정내역(주식시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Electronic_Message_Completion_Yes_Or_No[1];/*    4 전문완료여부 (String 1) */
    char Market_Identification[3];              /*    5 시장ID (String 3) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Non_Clrearing_Member_Number[5];        /*    7 거래전문회원번호 (String 5) */
    char Settlement_Type[1];                    /*    8 결제구분 (String 1) */
    char Trading_Date[8];                       /*    9 거래일자 (String 8) */
    char Settlement_Date[8];                    /*   10 결제일자 (String 8) */
    char Liquid_Provider_Yes_Or_No[1];          /*   11 LP여부 (String 1) */
    char Trust_Ask_Trading_Value[22];           /*   12 위탁매도거래대금 (Float 22) */
    char Trust_Bid_Trading_Value[22];           /*   13 위탁매수거래대금 (Float 22) */
    char Principal_Ask_Trading_Value[22];       /*   14 자기매도거래대금 (Float 22) */
    char Principal_Bid_Trading_Value[22];       /*   15 자기매수거래대금 (Float 22) */
    char Securities_In_Out_Type_Code[1];        /*   16 입출금구분코드 (String 1) */
    char Settlement_Amount[22];                 /*   17 결제금액 (Float 22) */
    char Filler_Value[1027];                    /*   18 필러값 (String 1027) */
} TCSMIH26601_DATA;

#endif  /* _KRX_TCSMIH26601_H */
