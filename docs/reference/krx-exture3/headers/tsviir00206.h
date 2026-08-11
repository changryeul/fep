#ifndef _KRX_TSVIIR00206_H
#define _KRX_TSVIIR00206_H

/* TSVIIR00206 이체입출금내역 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 655 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Second_Data_Request_Date[8];           /*    5 2차자료요청일자 (String 8) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Branch_Number[5];                      /*    7 지점번호 (String 5) */
    char Account_Number[12];                    /*    8 계좌번호 (String 12) */
    char Trading_Date[8];                       /*    9 거래일자 (String 8) */
    char Transaction_Number[9];                 /*   10 거래번호 (Long 9) */
    char Account_Name[80];                      /*   11 계좌명 (String 80) */
    char Money_In_Out_Type_Code[1];             /*   12 입출금구분코드 ★변경 (String 1) */
    char Counterpart_Bank_Name[80];             /*   13 상대은행명 (String 80) */
    char Counterpart_Bank_Store_Name[80];       /*   14 상대은행점포명 (String 80) */
    char Counterpart_Bank_Account_Number[20];   /*   15 상대은행계좌번호 (String 20) */
    char Counterpart_Account_Name[80];          /*   16 상대계좌명 (String 80) */
    char Transfer_In_Out_Amount[22];            /*   17 이체입출금액 (Float 22) */
    char Investor_Representative_Account_Number[12];/*   18 투자자대표계좌번호 (String 12) */
    char Investigation_Information_Miscellaneous_Contents[200];/*   19 심리정보기타내용 (String 200) */
} TSVIIR00206_DATA;

#endif  /* _KRX_TSVIIR00206_H */
