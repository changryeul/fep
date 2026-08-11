#ifndef _KRX_TRDESP51001_H
#define _KRX_TRDESP51001_H

/* TRDESP51001 소액인센티브시장조성종목 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 94 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Bz_Dd[8];                              /*    4 영업일자 (string 8) */
    char Isu_Cd[12];                            /*    5 종목코드 (string 12) */
    char Ord_Spd_Uplmt_Val[22];                 /*    6 호가스프레드상한값 (float 22) */
    char Ord_Spd_Lwlmt_Val[22];                 /*    7 호가스프레드하한값 (float 22) */
} TRDESP51001_DATA;

#endif  /* _KRX_TRDESP51001_H */
