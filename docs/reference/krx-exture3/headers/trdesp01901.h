#ifndef _KRX_TRDESP01901_H
#define _KRX_TRDESP01901_H

/* TRDESP01901 가격단위규칙 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 81 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence[11];                  /*    1 메세지일련번호 ★신규 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 ★신규 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 ★신규 (String 8) */
    char Business_Date[8];                      /*    4 영업일자 ★신규 (String 8) */
    char Prc_Unit_Rule_Id[10];                  /*    5 가격단위규칙ID ★신규 (String 10) */
    char Rng_Min_Prc[11];                       /*    6 구간최소가격 ★신규 (Float 11) */
    char Rng_Max_Prc[11];                       /*    7 구간최대가격 ★신규 (Float 11) */
    char Prc_Unit[11];                          /*    8 가격단위 ★신규 (Float 11) */
} TRDESP01901_DATA;

#endif  /* _KRX_TRDESP01901_H */
