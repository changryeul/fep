#ifndef _KRX_TSVIIR00211_H
#define _KRX_TSVIIR00211_H

/* TSVIIR00211 계좌관리자기본정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 402 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Request_Date[8];                       /*    5 요청일자 (String 8) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Branch_Number[5];                      /*    7 지점번호 (String 5) */
    char Account_Number[12];                    /*    8 계좌번호 (String 12) */
    char Account_Manager_Number[10];            /*    9 계좌관리자번호 (String 10) */
    char Administrative_Period_Start_Date[8];   /*   10 관리기간시작일자 (String 8) */
    char Administrative_Period_End_Date[8];     /*   11 관리기간종료일자 (String 8) */
    char Account_Manager_Type_Code[1];          /*   12 계좌관리자구분코드 ★변경 (String 1) */
    char Name[80];                              /*   13 성명 (String 80) */
    char Resident_Registration_Number[20];      /*   14 주민등록번호 (String 20) */
    char Address[200];                          /*   15 주소 (String 200) */
    char Investor_Representative_Account_Number[12];/*   16 투자자대표계좌번호 (String 12) */
} TSVIIR00211_DATA;

#endif  /* _KRX_TSVIIR00211_H */
