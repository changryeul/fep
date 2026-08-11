#ifndef _KRX_TSVIIR00213_H
#define _KRX_TSVIIR00213_H

/* TSVIIR00213 주문매체정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Second_Data_Request_Date[8];           /*    5 2차자료요청일자 (String 8) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Branch_Number[5];                      /*    7 지점번호 (String 5) */
    char Account_Number[12];                    /*    8 계좌번호 (String 12) */
    char Order_Date[8];                         /*    9 주문일자 (String 8) */
    char Order_Number[10];                      /*   10 주문번호 (Long 10) */
    char Issue_Code[12];                        /*   11 종목코드 (String 12) */
    char Issue_Name[40];                        /*   12 종목명 (String 40) */
    char Askbid_Type[1];                        /*   13 매도수구분 (String 1) */
    char Modify_Or_Calcel_Type[1];              /*   14 정정/취소 구분 (String 1) */
    char Original_Order_Number[10];             /*   15 원주문번호 (Long 10) */
    char Order_Quantity[10];                    /*   16 호가수량 (Long 10) */
    char Order_Price[18];                       /*   17 호가가격 (Long 18) */
    char Order_Time[6];                         /*   18 주문시각 (String 6) */
    char Ord_Media_Tp_Cd[1];                    /*   19 주문매체구분코드 (String 1) */
    char Ip_Address[12];                        /*   20 IP주소 (String 12) */
    char Reserv_Ord_Acpt_Tdd[8];                /*   21 예약주문접수일자 (String 8) */
} TSVIIR00213_DATA;

#endif  /* _KRX_TSVIIR00213_H */
