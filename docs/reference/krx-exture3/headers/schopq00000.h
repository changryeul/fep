#ifndef _KRX_SCHOPQ00000_H
#define _KRX_SCHOPQ00000_H

/* SCHOPQ00000 업무개시 요청 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 26 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Field[4];                              /*    1 거부사유코드 (String 4) */
    char Field_2[11];                           /*    2 TR-Code (String 11) */
    char Field_3[11];                           /*    3 일련번호 (Long 11) */
} SCHOPQ00000_DATA;

#endif  /* _KRX_SCHOPQ00000_H */
