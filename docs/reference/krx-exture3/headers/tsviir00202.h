#ifndef _KRX_TSVIIR00202_H
#define _KRX_TSVIIR00202_H

/* TSVIIR00202 유가증권 잔고내역 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 206 (interface-list.csv '길이(헤더제외)'와 대조) */
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
    char Issue_Code[12];                        /*   10 종목코드 (String 12) */
    char Issue_Name[80];                        /*   11 종목명 (String 80) */
    char Receivable_Type_Code[1];               /*   12 미수구분코드 (String 1) */
    char Spot_Quantity[15];                     /*   13 현물수량 (Long 15) */
    char Futures_Quantity[15];                  /*   14 선물수량 (Long 15) */
    char Investor_Representative_Account_Number[12];/*   15 투자자대표계좌번호 (String 12) */
} TSVIIR00202_DATA;

#endif  /* _KRX_TSVIIR00202_H */
