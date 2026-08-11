#ifndef _KRX_TSVIIR00210_H
#define _KRX_TSVIIR00210_H

/* TSVIIR00210 유가증권입출고내역(실물) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 698 (interface-list.csv '길이(헤더제외)'와 대조) */
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
    char Transaction_Number[9];                 /*   10 거래번호 (String 9) */
    char Sequence_Number[11];                   /*   11 일련번호 (Long 11) */
    char Account_Name[80];                      /*   12 계좌명 (String 80) */
    char Securities_In_Out_Type_Code[1];        /*   13 입출고구분코드 (String 1) */
    char Trading_Issue_Code[12];                /*   14 거래종목코드 (String 12) */
    char Trading_Issue_Name[80];                /*   15 거래종목명 (String 80) */
    char Securities_Securities_In_Out_Quantity[15];/*   16 유가증권입출고수량 (Long 15) */
    char Securities_Securities_In_Out_Round[6]; /*   17 유가증권입출고회차 (Long 6) */
    char Unit_Of_Bond_Type_Code[1];             /*   18 권종구분코드 (String 1) */
    char Stock_Certificate_Number[200];         /*   19 주권번호 (String 200) */
    char Investor_Representative_Account_Number[12];/*   20 투자자대표계좌번호 (String 12) */
    char Investigation_Information_Miscellaneous_Contents[200];/*   21 심리정보기타내용 (String 200) */
} TSVIIR00210_DATA;

#endif  /* _KRX_TSVIIR00210_H */
