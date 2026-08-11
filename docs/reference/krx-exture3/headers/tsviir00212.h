#ifndef _KRX_TSVIIR00212_H
#define _KRX_TSVIIR00212_H

/* TSVIIR00212 거래시각별 매매내역 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 442 (interface-list.csv '길이(헤더제외)'와 대조) */
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
    char Transaction_Time[9];                   /*   11 거래시각 (String 9) */
    char Customer_Account_Trading_Type_Code[1]; /*   12 고객계좌거래구분코드 (String 1) */
    char Book_Trading_Type_Code[5];             /*   13 원장거래구분코드 (String 5) */
    char Account_Name[80];                      /*   14 계좌명 (String 80) */
    char Resident_Registration_Number[20];      /*   15 주민등록번호 (String 20) */
    char Trading_Issue_Code[12];                /*   16 거래종목코드 (String 12) */
    char Trading_Issue_Name[80];                /*   17 거래종목명 (String 80) */
    char Trading_Volumn[10];                    /*   18 체결수량 (Long 10) */
    char Trading_Unit_Price[13];                /*   19 거래단가 (Long 13) */
    char Trading_Value[22];                     /*   20 거래대금 (Float 22) */
    char Fee[22];                               /*   21 수수료 (Float 22) */
    char Special_Tax_For_Rural_Development[22]; /*   22 농특세 (Float 22) */
    char Transaction_Tax[22];                   /*   23 거래세 (Float 22) */
    char Income_Tax[22];                        /*   24 소득세 (Float 22) */
    char Inhabitant_Tax[22];                    /*   25 주민세 (Float 22) */
} TSVIIR00212_DATA;

#endif  /* _KRX_TSVIIR00212_H */
