#ifndef _KRX_TSVIIR00401_H
#define _KRX_TSVIIR00401_H

/* TSVIIR00401 실제투자자상세주문내역 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 535 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Request_Date[8];                       /*    5 요청일자 (String 8) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Branch_Number[5];                      /*    7 지점번호 (String 5) */
    char Account_Number[12];                    /*    8 계좌번호 (String 12) */
    char Request_Investor_Type_Code[1];         /*    9 요청투자자구분코드 (String 1) */
    char Exch_Id[2];                            /*   10 거래소ID (String 2) */
    char Trading_Issue_Code[12];                /*   11 거래종목코드 (String 12) */
    char Trading_Issue_Name[80];                /*   12 거래종목명 (String 80) */
    char Trading_Date[8];                       /*   13 거래일자 (String 8) */
    char Ask_Bid_Type_Code[1];                  /*   14 매도매수구분코드 (String 1) */
    char Board_Id[2];                           /*   15 보드ID (String 2) */
    char Order_Identification[10];              /*   16 주문ID (String 10) */
    char Order_Quantity[10];                    /*   17 호가수량 (Long 10) */
    char End_Client_Com_Nm[100];                /*   18 계산주체회사명 (String 100) */
    char End_Client_Member_Number[5];           /*   19 계산주체회원번호 (String 5) */
    char End_Client_Branch_Number[5];           /*   20 계산주체지점번호 (String 5) */
    char End_Client_Account_Number[50];         /*   21 계산주체계좌번호 (String 50) */
    char End_Client_Order_Identification[50];   /*   22 계산주체주문ID (String 50) */
    char End_Client_Order_Time[9];              /*   23 계산주체주문시간 (String 9) */
    char End_Client_Order_Quantity[10];         /*   24 계산주체주문수량 (Long 10) */
    char End_Client_Order_Price[11];            /*   25 계산주체주문가격 (Float 11) */
    char End_Client_Investor_Type_Code[4];      /*   26 계산주체투자자구분코드 (String 4) */
    char End_Client_Foreign_Investor_Type_Code[2];/*   27 계산주체외국인투자자구분코드 (String 2) */
    char Broker_1_Order_Identification[50];     /*   28 중개사1주문ID (String 50) */
    char Broker_2_Order_Identification[50];     /*   29 중개사2주문ID (String 50) */
} TSVIIR00401_DATA;

#endif  /* _KRX_TSVIIR00401_H */
