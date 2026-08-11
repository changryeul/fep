#ifndef _KRX_SCHOPQ10000_H
#define _KRX_SCHOPQ10000_H

/* SCHOPQ10000 업무개시 요청(체결,장운영,DropCopy 전용) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 116 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Field[4];                              /*    1 거부사유코드 (String 4) */
    char Field_2[2];                            /*    2 ME그룹수 (Long 2) */
    char Field_3[11];                           /*    3 ME그룹#01 일련번호 (Long 11) */
    char Field_4[11];                           /*    4 ME그룹#02 일련번호 (Long 11) */
    char Field_5[11];                           /*    5 ME그룹#03 일련번호 (Long 11) */
    char Field_6[11];                           /*    6 ME그룹#04 일련번호 (Long 11) */
    char Field_7[11];                           /*    7 ME그룹#05 일련번호 (Long 11) */
    char Field_8[11];                           /*    8 ME그룹#06 일련번호 (Long 11) */
    char Field_9[11];                           /*    9 ME그룹#07 일련번호 (Long 11) */
    char Field_10[11];                          /*   10 ME그룹#08 일련번호 (Long 11) */
    char Field_11[11];                          /*   11 ME그룹#09 일련번호 (Long 11) */
    char Field_12[11];                          /*   12 ME그룹#10 일련번호 (Long 11) */
} SCHOPQ10000_DATA;

#endif  /* _KRX_SCHOPQ10000_H */
