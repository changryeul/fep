#ifndef _KRX_TCSMIR30701_H
#define _KRX_TCSMIR30701_H

/* TCSMIR30701 할인계좌헤지,차익거래신고및접수통보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 130 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Mkt_Id[3];                             /*    4 시장ID ★변경 (String 3) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Discount_Account_Number[12];           /*    6 사후증거금할인계좌번호 (String 12) */
    char Declaration_Time[8];                   /*    7 신고시각 (String 8) */
    char Rejected_Code[2];                      /*    8 거부코드 (String 2) */
    char Trading_Date[8];                       /*    9 거래일자 (String 8) */
    char Declaration_Input_Type_Code[1];        /*   10 신고입력구분코드 (String 1) */
    char Declaration_Type_Type_Code[1];         /*   11 신고유형구분코드 (String 1) */
    char Member_User_Area[60];                  /*   12 회원사용영역 (String 60) */
} TCSMIR30701_DATA;

#endif  /* _KRX_TCSMIR30701_H */
