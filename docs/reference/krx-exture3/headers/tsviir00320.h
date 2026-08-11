#ifndef _KRX_TSVIIR00320_H
#define _KRX_TSVIIR00320_H

/* TSVIIR00320 파생심리 예탁금부문 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 288 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Time[8];                               /*    6 시각 (String 8) */
    char Request_Date[8];                       /*    7 요청일자 (String 8) */
    char Submit_Date[8];                        /*    8 제출일자 (String 8) */
    char Investigation_Object_Start_Date[8];    /*    9 심리대상개시일자 (String 8) */
    char Investigation_Object_End_Date[8];      /*   10 심리대상종료일자 (String 8) */
    char Branch_Number[5];                      /*   11 지점번호 (String 5) */
    char Account_Number[12];                    /*   12 계좌번호 (String 12) */
    char Account_Name[80];                      /*   13 계좌명 (String 80) */
    char Stock_Link_Account_Number[12];         /*   14 주식연계계좌번호 (String 12) */
    char Resident_Registration_Number[20];      /*   15 주민등록번호 (String 20) */
    char Trading_Date[8];                       /*   16 거래일자 (String 8) */
    char Book_Trading_Type_Code[5];             /*   17 원장거래구분코드 (String 5) */
    char Customer_Deposit_Cash_Sign[1];         /*   18 고객예탁현금부호 (String 1) */
    char Customer_Deposit_Cash[22];             /*   19 고객예탁현금 (Float 22) */
    char Customer_Deposit_Substitute_Amount[22];/*   20 고객예탁대용금액 (Float 22) */
    char Trust_Margin_Total_Amount_Sign[1];     /*   21 위탁증거금총액부호 (String 1) */
    char Trust_Margin_Total_Amount[22];         /*   22 위탁증거금총액 (Float 22) */
} TSVIIR00320_DATA;

#endif  /* _KRX_TSVIIR00320_H */
