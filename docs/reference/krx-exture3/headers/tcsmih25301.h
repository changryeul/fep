#ifndef _KRX_TCSMIH25301_H
#define _KRX_TCSMIH25301_H

/* TCSMIH25301 착오거래구제접수송신 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sesuence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Member_Number[5];                      /*    4 회원번호 (String 5) */
    char Erroneous_Trading_Relief_Application_Identification[10];/*    5 착오거래구제신청ID (String 10) */
    char Account_Number[12];                    /*    6 계좌번호 (String 12) */
    char Product_Identification[11];            /*    7 상품ID (String 11) */
    char Trading_Start_Time[9];                 /*    8 체결시작시각 (String 9) */
    char Trading_End_Time[9];                   /*    9 체결종료시각 (String 9) */
    char Person_In_Charge_Name[80];             /*   10 담당자명 (String 80) */
    char Person_In_Charge_Telephone_Number[40]; /*   11 담당자전화번호 (String 40) */
    char Application_Media_Type_Code[1];        /*   12 신청매체구분코드 (String 1) */
    char Erroneous_Trading_Relief_Application_Rejected_Reason_Code[4];/*   13 착오거래구제신청거부사유코드 (String 4) */
    char Erroneous_Trading_Relief_Application_Time[9];/*   14 착오거래구제신청시각 (String 9) */
    char Erroneous_Trading_Relief_Accept_Time[9];/*   15 착오거래구제접수시각 (String 9) */
    char Erroneous_Trading_Relief_Accept_Number[10];/*   16 착오거래구제접수번호 (Long 10) */
    char Filler_Value[961];                     /*   17 필러값 (String 961) */
} TCSMIH25301_DATA;

#endif  /* _KRX_TCSMIH25301_H */
