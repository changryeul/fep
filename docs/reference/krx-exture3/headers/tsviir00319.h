#ifndef _KRX_TSVIIR00319_H
#define _KRX_TSVIIR00319_H

/* TSVIIR00319 파생심리 선물옵션 미결제약정현황(잔고내역) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 348 (interface-list.csv '길이(헤더제외)'와 대조) */
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
    char Field[12];                             /*   17 종목코드 (String 12) */
    char Field_2[80];                           /*   18 종목명 (String 80) */
    char Field_3[1];                            /*   19 미결제약정수량부호 (String 1) */
    char Field_4[10];                           /*   20 미결제약정수량 (Long 10) */
    char Field_5[15];                           /*   21 당일매수수량 (Long 15) */
    char Field_6[15];                           /*   22 당일매도수량 (Long 15) */
} TSVIIR00319_DATA;

#endif  /* _KRX_TSVIIR00319_H */
