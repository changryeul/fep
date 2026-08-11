#ifndef _KRX_TCSMIR31601_H
#define _KRX_TCSMIR31601_H

/* TCSMIR31601 옴니버스계좌미결제배분신고및접수통보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 193 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Tracsaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Member_Number[5];                      /*    4 회원번호 (String 5) */
    char Open_Interest_Allocation_Declaration_Id[10];/*    5 미결제배분신고ID (String 10) */
    char Open_Interest_Allocation_Declaration_Original_Id[10];/*    6 미결제배분원신고ID (String 10) */
    char Declaration_Input_Type_Code[1];        /*    7 신고입력구분코드 (String 1) */
    char Allocation_End_Client_Account_Number[12];/*    8 배분최종투자자계좌번호 (String 12) */
    char Issue_Code[12];                        /*    9 종목코드 (String 12) */
    char Allocation_Open_Interest_Base_Date[8]; /*   10 배분대상미결제약정기준일자 (String 8) */
    char Ask_Open_Interest_Quantity[10];        /*   11 매도미결제약정수량 (Long 10) */
    char Bid_Open_Interest_Quantity[10];        /*   12 매수미결제약정수량 (Long 10) */
    char Sub_End_Client_Account_Number[12];     /*   13 하위최종투자자계좌번호 (String 12) */
    char Member_Use_Area[60];                   /*   14 회원사용영역 (String 60) */
    char Declaration_Time[9];                   /*   15 신고시각 (String 9) */
    char Open_Interest_Allocation_Declaration_Rejected_Reason_Code[4];/*   16 미결제배분신고거부사유코드 (String 4) */
} TCSMIR31601_DATA;

#endif  /* _KRX_TCSMIR31601_H */
