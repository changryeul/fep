#ifndef _KRX_TSVIIR00313_H
#define _KRX_TSVIIR00313_H

/* TSVIIR00313 파생심리 수표입출금내역 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 761 (interface-list.csv '길이(헤더제외)'와 대조) */
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
    char Trading_Date[8];                       /*   16 거래일자 (String 8) */
    char Book_Trading_Type_Code[5];             /*   17 원장거래구분코드 (String 5) */
    char Transaction_Number[9];                 /*   18 거래번호 (Long 9) */
    char Sequence_Number[3];                    /*   19 일련번호 (Long 3) */
    char Money_In_Out_Type_Code[1];             /*   20 입출금구분코드 (String 1) */
    char Check_Amount[22];                      /*   21 수표금액 (Float 22) */
    char Issue_Date[8];                         /*   22 발행일자 (String 8) */
    char Issue_Bank_Name[80];                   /*   23 발행은행명 (String 80) */
    char Issue_Store_Name[80];                  /*   24 발행점포명 (String 80) */
    char Face_Value_Amount[22];                 /*   25 권면금액 (Float 22) */
    char Issue_Count[16];                       /*   26 발행매수 (Long 16) */
    char Check_Number[200];                     /*   27 수표번호 (String 200) */
    char Field[100];                            /*   28 비고 (String 100) */
} TSVIIR00313_DATA;

#endif  /* _KRX_TSVIIR00313_H */
