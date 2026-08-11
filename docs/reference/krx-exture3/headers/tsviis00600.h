#ifndef _KRX_TSVIIS00600_H
#define _KRX_TSVIIS00600_H

/* TSVIIS00600 투자자식별정보SALT내용 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 102 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Salt_Information[64];                  /*    4 SALT정보 (String 64) */
    char Application_Date[8];                   /*    5 적용일자 (String 8) */
} TSVIIS00600_DATA;

#endif  /* _KRX_TSVIIS00600_H */
