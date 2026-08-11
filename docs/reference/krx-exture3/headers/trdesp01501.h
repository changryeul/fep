#ifndef _KRX_TRDESP01501_H
#define _KRX_TRDESP01501_H

/* TRDESP01501 통합회원지점정보 송신 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 420 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence[11];                  /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Business_Date[8];                      /*    4 영업일자 (String 8) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Member_Name[80];                       /*    6 회원명 (String 80) */
    char Number_Of_Branch[6];                   /*    7 지점수 (Long 6) */
    char Branch_Number[5];                      /*    8 지점번호 (String 5) */
    char Branch_Name[80];                       /*    9 지점명 (String 80) */
    char Zip_Code[6];                           /*   10 우편번호 (String 6) */
    char Branch_Address[200];                   /*   11 지점주소 (String 200) */
} TRDESP01501_DATA;

#endif  /* _KRX_TRDESP01501_H */
