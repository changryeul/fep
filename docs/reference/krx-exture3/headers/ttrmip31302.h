#ifndef _KRX_TTRMIP31302_H
#define _KRX_TTRMIP31302_H

/* TTRMIP31302 기준가정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 71 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Board_Id[2];                           /*    4 보드ID (String 2) */
    char Issue_Code[12];                        /*    5 종목코드 (String 12) */
    char Base_Price[11];                        /*    6 기준가격 (Float 11) */
    char Base_Price_Upper_Limit_Price[11];      /*    7 기준가격상한가 (Float 11) */
    char Base_Price_Lower_Limit_Price[11];      /*    8 기준가격하한가 (Float 11) */
} TTRMIP31302_DATA;

#endif  /* _KRX_TTRMIP31302_H */
