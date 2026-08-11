#ifndef _KRX_TSVIIR00209_H
#define _KRX_TSVIIR00209_H

/* TSVIIR00209 유가증권입출고내역(대체) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 583 (interface-list.csv '길이(헤더제외)'와 대조) */
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
    char Securities_In_Out_Type_Code[1];        /*   12 입출고구분코드 (String 1) */
    char Source_Type_Code[1];                   /*   13 출처구분코드 (String 1) */
    char Counterpart_Member_Number[5];          /*   14 상대회원번호 (String 5) */
    char Counterpart_Branch_Number[5];          /*   15 상대지점번호 (String 5) */
    char Counterpart_Account_Number[12];        /*   16 상대계좌번호 (String 12) */
    char Counterpart_Account_Name[80];          /*   17 상대계좌명 (String 80) */
    char Trading_Issue_Code[12];                /*   18 거래종목코드 (String 12) */
    char Trading_Issue_Name[80];                /*   19 거래종목명 (String 80) */
    char Securities_Securities_In_Out_Quantity[15];/*   20 유가증권입출고수량 (Long 15) */
    char Investor_Representative_Account_Number[12];/*   21 투자자대표계좌번호 (String 12) */
    char Investigation_Information_Miscellaneous_Contents[200];/*   22 심리정보기타내용 (String 200) */
} TSVIIR00209_DATA;

#endif  /* _KRX_TSVIIR00209_H */
