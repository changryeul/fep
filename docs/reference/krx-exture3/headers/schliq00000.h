#ifndef _KRX_SCHLIQ00000_H
#define _KRX_SCHLIQ00000_H

/* SCHLIQ00000 로그온 요청 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 41 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Field[10];                             /*    1 I/F프로세스정보 (String 10) */
    char Field_2[30];                           /*    2 I/F프로세스고유번호 (String 30) */
    char Field_3[1];                            /*    3 암호화 적용여부 (String 1) */
} SCHLIQ00000_DATA;

#endif  /* _KRX_SCHLIQ00000_H */
