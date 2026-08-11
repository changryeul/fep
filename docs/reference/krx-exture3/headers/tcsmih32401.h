#ifndef _KRX_TCSMIH32401_H
#define _KRX_TCSMIH32401_H

/* TCSMIH32401 증권시장착오거래구제처리결과송신 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sesuence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Errtrd_Rlf_Acpt_Dd[8];                 /*    4 착오거래구제접수일자 (String 8) */
    char Erroneous_Trading_Relief_Accept_Number[10];/*    5 착오거래구제접수번호 (Long 10) */
    char Erroneous_Trading_Relief_Send_Type_Code[1];/*    6 착오거래구제송신구분코드 (String 1) */
    char Erroneous_Trading_Relief_Process_Result_Code[2];/*    7 착오거래구제처리결과코드 (String 2) */
    char Member_Number[5];                      /*    8 회원번호 (String 5) */
    char Account_Number[12];                    /*    9 계좌번호 (String 12) */
    char Market_Identification[3];              /*   10 시장ID (String 3) */
    char Board_Id[2];                           /*   11 보드ID (String 2) */
    char Issue_Code[12];                        /*   12 종목코드 (String 12) */
    char Ask_Bid_Type_Code[1];                  /*   13 매도매수구분코드 (String 1) */
    char Trading_Number[11];                    /*   14 체결번호 (Long 11) */
    char Trading_Price[11];                     /*   15 체결가격 (Float 11) */
    char Trading_Volumn[10];                    /*   16 체결수량 (Long 10) */
    char Trading_Time[9];                       /*   17 체결시각 (String 9) */
    char Erroneous_Trading_Relief_Price[11];    /*   18 착오거래구제가격 (Float 11) */
    char Member_Base_Trading_Number[11];        /*   19 회원기준체결번호 (Long 11) */
    char Filler[1051];                          /*   20 필러 (String 1051) */
} TCSMIH32401_DATA;

#endif  /* _KRX_TCSMIH32401_H */
