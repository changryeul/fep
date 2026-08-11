#ifndef _KRX_TRDESP03401_H
#define _KRX_TRDESP03401_H

/* TRDESP03401 보드별거래종목 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 60 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence[11];                  /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Market_Identification[3];              /*    3 시장ID (String 3) */
    char Transmit_Date[8];                      /*    4 전송일자 (String 8) */
    char Business_Date[8];                      /*    5 영업일자 (String 8) */
    char Securities_Group_Identification[2];    /*    6 증권그룹ID (String 2) */
    char Underlying_Asset_Identification[3];    /*    7 기초자산ID (String 3) */
    char Issue_Code[12];                        /*    8 종목코드 (String 12) */
    char Board_Id[2];                           /*    9 보드ID (String 2) */
} TRDESP03401_DATA;

#endif  /* _KRX_TRDESP03401_H */
