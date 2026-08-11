#ifndef _KRX_TSVIIR00317_H
#define _KRX_TSVIIR00317_H

/* TSVIIR00317 파생심리 계좌관리자기본정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 367 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Time[8];                               /*    6 시각 (String 8) */
    char Request_Date[8];                       /*    7 요청일자 (String 8) */
    char Submit_Date[8];                        /*    8 제출일자 (String 8) */
    char Investigation_Object_Start_Date[8];    /*    9 심리대상개시일자 (String 8) */
    char Investigation_Object_End_Date[8];      /*   10 심리대상종료일자 (String 8) */
    char Branch_Number[5];                      /*   11 지점번호 (String 5) */
    char Account_Number[12];                    /*   12 계좌번호 (String 12) */
    char Account_Name[80];                      /*   13 계좌명 (String 80) */
    char Stock_Link_Account_Number[12];         /*   14 주식연계계좌번호 (String 12) */
    char Resident_Registration_Number[20];      /*   15 주민등록번호 (String 20) */
    char Account_Manager_Number[10];            /*   16 계좌관리자번호 (String 10) */
    char Administrative_Period_Start_Date[8];   /*   17 관리기간시작일자 (String 8) */
    char Administrative_Period_End_Date[8];     /*   18 관리기간종료일자 (String 8) */
    char Account_Manager_Type_Code[1];          /*   19 계좌관리자구분코드 (String 1) */
    char Name[20];                              /*   20 성명 (String 20) */
    char Resident_Registration_Number_2[13];    /*   21 주민등록번호 (String 13) */
    char Address[100];                          /*   22 주소 (String 100) */
} TSVIIR00317_DATA;

#endif  /* _KRX_TSVIIR00317_H */
