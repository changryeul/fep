#ifndef _KRX_TCSMIR32201_H
#define _KRX_TCSMIR32201_H

/* TCSMIR32201 증권시장착오거래구제가격변경신청수신및응답 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 337 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sesuence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Member_Number[5];                      /*    4 회원번호 (String 5) */
    char Erroneous_Trading_Relief_Price_Change_Application_Identification[10];/*    5 착오거래구제가격변경신청ID (String 10) */
    char Account_Number[12];                    /*    6 계좌번호 (String 12) */
    char Market_Identification[3];              /*    7 시장ID (String 3) */
    char Board_Identification[2];               /*    8 보드ID (String 2) */
    char Issue_Code[12];                        /*    9 종목코드 (String 12) */
    char Ask_And_Bid_Type_Code[1];              /*   10 매도매수구분코드 (String 1) */
    char Trading_Number[11];                    /*   11 체결번호 (Long 11) */
    char Erroneous_Trading_Relief_Accept_Date[8];/*   12 착오거래구제접수일자 (String 8) */
    char Erroneous_Trading_Relief_Accept_Number[10];/*   13 착오거래구제접수번호 (Long 10) */
    char Person_In_Charge_Name[80];             /*   14 담당자명 (String 80) */
    char Person_In_Charge_Telephone_Number[40]; /*   15 담당자전화번호 (String 40) */
    char Manager_Name[80];                      /*   16 책임자명 (String 80) */
    char Manager_Telephone_Number[20];          /*   17 책임자전화번호 (String 20) */
    char Erroneous_Trading_Relief_Price_Change_Application_Rejected_Reason_Code[4];/*   18 착오거래구제가격변경신청거부사유코드 (String 4) */
    char Erroneous_Trading_Relief_Price_Change_Application_Time[9];/*   19 착오거래구제가격변경신청시각 (String 9) */
} TCSMIR32201_DATA;

#endif  /* _KRX_TCSMIR32201_H */
