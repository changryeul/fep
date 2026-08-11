#ifndef _KRX_TRDESP50301_H
#define _KRX_TRDESP50301_H

/* TRDESP50301 채권 발행기관코드 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 283 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Bz_Dd[8];                              /*    4 영업일자 (String 8) */
    char Isur_Cd[5];                            /*    5 발행기관코드 (string 5) */
    char Com_Nm[80];                            /*    6 회사명 (string 80) */
    char Com_Abbrv[40];                         /*    7 회사약명 (String 40) */
    char Com_Eng_Nm[80];                        /*    8 회사영문명 (string 80) */
    char Com_Eng_Abbrv[40];                     /*    9 회사영문약명 (String 40) */
} TRDESP50301_DATA;

#endif  /* _KRX_TRDESP50301_H */
