#ifndef _KRX_TCSMIH25201_H
#define _KRX_TCSMIH25201_H

/* TCSMIH25201 착오거래구제처리결과송신 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sesuence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Erroneous_Trading_Relief_Accept_Date[8];/*    4 착오거래구제접수일자 (String 8) */
    char Erroneous_Trading_Relief_Accept_Number[10];/*    5 착오거래구제접수번호 (Long 10) */
    char Erroneous_Trading_Relief_Send_Type_Code[1];/*    6 착오거래구제송신구분코드 (String 1) */
    char Member_Number[5];                      /*    7 회원번호 (String 5) */
    char Erroneous_Trading_Relief_Process_Result_Code[2];/*    8 착오거래구제처리결과코드 (String 2) */
    char Filler_Value[1144];                    /*    9 필러값 (String 1144) */
} TCSMIH25201_DATA;

#endif  /* _KRX_TCSMIH25201_H */
