#ifndef _KRX_TCHEDP99000_H
#define _KRX_TCHEDP99000_H

/* TCHEDP99000 회원사 체결 인터페이스 종료 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 153 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Field[2];                              /*    4 ME그룹수 (Long 2) */
    char Field_2[11];                           /*    5 ME그룹#01 일련번호 (Long 11) */
    char Field_3[11];                           /*    6 ME그룹#02 일련번호 (Long 11) */
    char Field_4[11];                           /*    7 ME그룹#03 일련번호 (Long 11) */
    char Field_5[11];                           /*    8 ME그룹#04 일련번호 (Long 11) */
    char Field_6[11];                           /*    9 ME그룹#05 일련번호 (Long 11) */
    char Field_7[11];                           /*   10 ME그룹#06 일련번호 (Long 11) */
    char Field_8[11];                           /*   11 ME그룹#07 일련번호 (Long 11) */
    char Field_9[11];                           /*   12 ME그룹#08 일련번호 (Long 11) */
    char Field_10[11];                          /*   13 ME그룹#09 일련번호 (Long 11) */
    char Field_11[11];                          /*   14 ME그룹#10 일련번호 (Long 11) */
    char Transaction_Date[8];                   /*   15 전송일자 (String 8) */
    char Transaction_Time[9];                   /*   16 전송시각 (String 9) */
} TCHEDP99000_DATA;

#endif  /* _KRX_TCHEDP99000_H */
