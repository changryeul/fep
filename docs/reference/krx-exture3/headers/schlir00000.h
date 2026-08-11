#ifndef _KRX_SCHLIR00000_H
#define _KRX_SCHLIR00000_H

/* SCHLIR00000 로그온 응답 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 5 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Field[4];                              /*    1 거부사유코드 (String 4) */
    char Field_2[1];                            /*    2 암호화 적용여부 (String 1) */
} SCHLIR00000_DATA;

#endif  /* _KRX_SCHLIR00000_H */
