#ifndef _KRX_TCSMIH25401_H
#define _KRX_TCSMIH25401_H

/* TCSMIH25401 착오거래구제대상송신 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sesuence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Erroneous_Trading_Relief_Accept_Number[10];/*    4 착오거래구제접수번호 (Long 10) */
    char Erroneous_Trading_Relief_Send_Type_Code[1];/*    5 착오거래구제송신구분코드 (String 1) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Account_Number[12];                    /*    7 계좌번호 (String 12) */
    char Board_Id[2];                           /*    8 보드ID (String 2) */
    char Issue_Code[12];                        /*    9 종목코드 (String 12) */
    char Ask_Bid_Type_Code[1];                  /*   10 매도매수구분코드 (String 1) */
    char Trading_Number[11];                    /*   11 체결번호 (Long 11) */
    char Trading_Price[11];                     /*   12 체결가격 (Float 11) */
    char Trading_Volumn[10];                    /*   13 체결수량 (Long 10) */
    char Trading_Time[9];                       /*   14 체결시각 (String 9) */
    char Filler_Value[1086];                    /*   15 필러값 (String 1086) */
} TCSMIH25401_DATA;

#endif  /* _KRX_TCSMIH25401_H */
