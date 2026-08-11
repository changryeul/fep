#ifndef _KRX_TSVIIR00318_H
#define _KRX_TSVIIR00318_H

/* TSVIIR00318 파생심리 주문매체정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 350 (interface-list.csv '길이(헤더제외)'와 대조) */
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
    char Order_Date[8];                         /*   16 주문일자 (String 8) */
    char Order_Number[10];                      /*   17 주문번호 (Long 10) */
    char Issue_Code[12];                        /*   18 종목코드 (String 12) */
    char Issue_Name[40];                        /*   19 종목명 (String 40) */
    char Ask_Bid_Type[1];                       /*   20 매도수구분 (String 1) */
    char Modified_Cancel_Type[1];               /*   21 정정/취소구분 (String 1) */
    char Original_Order_Number[10];             /*   22 원주문번호 (Long 10) */
    char Quantity[15];                          /*   23 수량 (Long 15) */
    char Price[18];                             /*   24 가격 (Float 18) */
    char Order_Time[6];                         /*   25 주문시각 (String 6) */
    char Order_Media[1];                        /*   26 주문매체 (String 1) */
    char Ip_Address[12];                        /*   27 IP주소 (String 12) */
    char Field[8];                              /*   28 예약주문접수일자 (String 8) */
    char Order_Type[1];                         /*   29 주문유형 (String 1) */
} TSVIIR00318_DATA;

#endif  /* _KRX_TSVIIR00318_H */
