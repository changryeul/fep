#ifndef _KRX_TCSMIH32101_H
#define _KRX_TCSMIH32101_H

/* TCSMIH32101 증권시장착오거래구제접수결과송신 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sesuence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Member_Number[5];                      /*    4 회원번호 (String 5) */
    char Erroneous_Trading_Relief_Application_Identification[10];/*    5 착오거래구제신청ID (String 10) */
    char Account_Number[12];                    /*    6 계좌번호 (String 12) */
    char Market_Identification[3];              /*    7 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*    8 증권그룹ID (String 2) */
    char Trading_Start_Time[9];                 /*    9 체결시작시각 (String 9) */
    char Trading_End_Time[9];                   /*   10 체결종료시각 (String 9) */
    char Person_In_Charge_Name[80];             /*   11 담당자명 (String 80) */
    char Person_In_Charge_Telephone_Number[40]; /*   12 담당자전화번호 (String 40) */
    char Manager_Name[80];                      /*   13 책임자명 (String 80) */
    char Manager_Telephone_Number[20];          /*   14 책임자전화번호 (String 20) */
    char Application_Media_Type_Code[1];        /*   15 신청매체구분코드 (String 1) */
    char Erroneous_Trading_Relief_Application_Rejected_Reason_Code[4];/*   16 착오거래구제신청거부사유코드 (String 4) */
    char Erroneous_Trading_Relief_Application_Time[9];/*   17 착오거래구제신청시각 (String 9) */
    char Erroneous_Trading_Relief_Accept_Number[10];/*   18 착오거래구제접수번호 (Long 10) */
    char Erroneous_Trading_Relief_Accept_Time[9];/*   19 착오거래구제접수시각 (String 9) */
    char Filler[867];                           /*   20 필러 (String 867) */
} TCSMIH32101_DATA;

#endif  /* _KRX_TCSMIH32101_H */
