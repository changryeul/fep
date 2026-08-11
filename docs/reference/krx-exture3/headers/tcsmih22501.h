#ifndef _KRX_TCSMIH22501_H
#define _KRX_TCSMIH22501_H

/* TCSMIH22501 주식선물미결제수량조정후변경내역 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    5 상품완료여부 (String 1) */
    char Market_Identification[3];              /*    6 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*    7 증권그룹ID (String 2) */
    char Underlying_Asset_Code[2];              /*    8 기초자산코드 (String 2) */
    char Member_Number[5];                      /*    9 회원번호 (String 5) */
    char Non_Clrearing_Member_Number[5];        /*   10 거래전문회원번호 (String 5) */
    char Start_Date[8];                         /*   11 적용일자 (String 8) */
    char Adjustment_Reason_Code[2];             /*   12 조정사유코드 (String 2) */
    char Account_Number[12];                    /*   13 계좌번호 (String 12) */
    char Issue_Code[12];                        /*   14 종목코드 (String 12) */
    char Contrtsz_Adj_Factr_Numrtr[6];          /*   15 미결제약정수량조정계수 (Long 6) */
    char Before_Adjustment_Ask_Open_Interest_Quantity[10];/*   16 조정전매도미결제수량 (Long 10) */
    char Before_Adjustment_Bid_Open_Interest_Quantity[10];/*   17 조정전매수미결제수량 (Long 10) */
    char After_Adjustment_Ask_Open_Interest_Quantity[10];/*   18 조정후매도미결제수량 (Long 10) */
    char After_Adjustment_Bid_Open_Interest_Quantity[10];/*   19 조정후매수미결제수량 (Long 10) */
    char Filler_Value[1071];                    /*   20 필러값 (String 1071) */
} TCSMIH22501_DATA;

#endif  /* _KRX_TCSMIH22501_H */
