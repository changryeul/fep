#ifndef _KRX_TCSMIR30101_H
#define _KRX_TCSMIR30101_H

/* TCSMIR30101 옵션권리행사(거부)신고및접수통보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 157 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*    5 증권그룹ID (String 2) */
    char Underlying_Asset_Code[2];              /*    6 기초자산코드 (String 2) */
    char Member_Number[5];                      /*    7 회원번호 (String 5) */
    char Declaration_Time[8];                   /*    8 신고시각 (String 8) */
    char Rejected_Code[2];                      /*    9 거부코드 (String 2) */
    char Account_Number[12];                    /*   10 계좌번호 (String 12) */
    char Issue_Code[12];                        /*   11 종목코드 (String 12) */
    char Ask_Open_Interest_Quantity[10];        /*   12 매수미결제약정수량 (Long 10) */
    char Exercise_Declaration_Type_Code[1];     /*   13 권리행사신고유형코드 (String 1) */
    char Declaration_Quantity[10];              /*   14 권리행사신고수량 (Long 10) */
    char Member_User_Area[60];                  /*   15 회원사용영역 (String 60) */
} TCSMIR30101_DATA;

#endif  /* _KRX_TCSMIR30101_H */
