#ifndef _KRX_TSVIIR00101_H
#define _KRX_TSVIIR00101_H

/* TSVIIR00101 계좌응답정보(특이위탁자 계좌정보) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 849 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Request_Date[8];                       /*    5 요청일자 (String 8) */
    char Issue_Code[12];                        /*    6 종목코드 (String 12) */
    char Member_Number[5];                      /*    7 회원번호 (String 5) */
    char Branch_Number[5];                      /*    8 지점번호 (String 5) */
    char Account_Number[12];                    /*    9 계좌번호 (String 12) */
    char Resident_Registration_Number[20];      /*   10 주민등록번호 (String 20) */
    char Account_Name[80];                      /*   11 계좌명 (String 80) */
    char Home_Zip_Code[6];                      /*   12 자택우편번호 (String 6) */
    char Home_Address[200];                     /*   13 자택주소 (String 200) */
    char Home_Telephone_Number_Number[20];      /*   14 자택전화번호 (String 20) */
    char Company_Zip_Code[6];                   /*   15 직장우편번호 (String 6) */
    char Company_Address[200];                  /*   16 직장주소 (String 200) */
    char Company_Telephone_Number_Number[20];   /*   17 직장전화번호 (String 20) */
    char Cellular_Phone_Number[20];             /*   18 휴대폰번호 (String 20) */
    char Company_Name[80];                      /*   19 직장명 (String 80) */
    char Email[100];                            /*   20 이메일 (String 100) */
    char Account_Opening_Date[8];               /*   21 계좌개설일자 (String 8) */
    char Credit_Cash_Type_Code[1];              /*   22 신용현금구분코드 (String 1) */
    char Employee_Securities_Saving_Yes_Or_No[1];/*   23 근로자증권저축여부 (String 1) */
    char Investor_Representative_Account_Number[12];/*   24 투자자대표계좌번호 (String 12) */
} TSVIIR00101_DATA;

#endif  /* _KRX_TSVIIR00101_H */
