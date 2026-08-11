#ifndef _KRX_TCSMIP46101_H
#define _KRX_TCSMIP46101_H

/* TCSMIP46101 당일거래분유통결제정보(주식시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Trading_Date[8];                       /*    5 거래일자 (String 8) */
    char Settlement_Date[8];                    /*    6 결제일자 (String 8) */
    char Member_Number[5];                      /*    7 회원번호 (String 5) */
    char Issue_Code[12];                        /*    8 종목코드 (String 12) */
    char Loan_Redemption_Security_Receive[15];  /*    9 융자상환증권수령수량 (Long 15) */
    char Loan_New_Security_Payment[15];         /*   10 융자신규증권납부수량 (Long 15) */
    char Loan_Redemption_Amount_Payment[22];    /*   11 융자상환납부금액 (Float 22) */
    char Loan_New_Amount_Receive[22];           /*   12 융자신규수령금액 (Float 22) */
    char Stockloan_New_Security_Receive[15];    /*   13 대주신규증권수령수량 (Long 15) */
    char Stockloan_Redemption_Security_Payment[15];/*   14 대주상환증권납부수량 (Long 15) */
    char Stockloan_New_Amount_Payment[22];      /*   15 대주신규납부금액 (Float 22) */
    char Stockloan_Redemption_Amount_Receive[22];/*   16 대주상환수령금액 (Float 22) */
    char Filler_Value[986];                     /*   17 필러값 (String 986) */
} TCSMIP46101_DATA;

#endif  /* _KRX_TCSMIP46101_H */
