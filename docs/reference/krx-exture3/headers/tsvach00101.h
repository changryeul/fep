#ifndef _KRX_TSVACH00101_H
#define _KRX_TSVACH00101_H

/* TSVACH00101 계좌번호변경 내역 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 99 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Member_Number[5];                      /*    3 회원번호 (String 5) */
    char Spot_Derivation_Type_Code[1];          /*    4 현물파생구분코드 (String 1) */
    char Sequence_Number[11];                   /*    5 일련번호 (Long 11) */
    char Before_Change_Member_Number[5];        /*    6 변경전 회원번호 (String 5) */
    char Before_Change_Branch_Number[5];        /*    7 변경전 지점번호 (String 5) */
    char Before_Change_Account_Number[12];      /*    8 변경전 계좌번호 (String 12) */
    char After_Change_Member_Number[5];         /*    9 변경후 회원번호 (String 5) */
    char After_Change_Branch_Number[5];         /*   10 변경후 지점번호 (String 5) */
    char After_Change_Account_Number[12];       /*   11 변경후 계좌번호 (String 12) */
    char Application_Date[8];                   /*   12 적용일자 (String 8) */
    char Input_Date[8];                         /*   13 입력일자 (String 8) */
} TSVACH00101_DATA;

#endif  /* _KRX_TSVACH00101_H */
