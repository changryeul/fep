#ifndef _KRX_TRDESP00501_H
#define _KRX_TRDESP00501_H

/* TRDESP00501 옵션기준가 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 113 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence[11];                  /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Market_Identification[3];              /*    3 시장ID (String 3) */
    char Transmit_Date[8];                      /*    4 전송일자 (String 8) */
    char Business_Date[8];                      /*    5 영업일자 (String 8) */
    char Securities_Group_Identification[2];    /*    6 증권그룹ID (String 2) */
    char Uly_Id[3];                             /*    7 기초자산ID (String 3) */
    char Underlying_Asset_Code[2];              /*    8 기초자산코드 (String 2) */
    char Issue_Code[12];                        /*    9 종목코드 (String 12) */
    char Exercise_Price[18];                    /*   10 행사가격 (Float 18) */
    char Settlement_Multiplier[22];             /*   11 거래승수 (Float 22) */
    char Base_Price[11];                        /*   12 기준가격 (String 11) */
    char Base_Price_Type_Code[2];               /*   13 기준가격구분코드 (String 2) */
} TRDESP00501_DATA;

#endif  /* _KRX_TRDESP00501_H */
