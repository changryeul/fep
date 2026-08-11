#ifndef _KRX_TRDESP03801_H
#define _KRX_TRDESP03801_H

/* TRDESP03801 ETN 수익구조내용 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 2053 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (string 11) */
    char Mkt_Id[3];                             /*    3 시장ID (string 3) */
    char Trnsm_Dd[8];                           /*    4 전송일자 (string 8) */
    char Bz_Dd[8];                              /*    5 영업일자 (string 8) */
    char Isu_Cd[12];                            /*    6 종목코드 (string 12) */
    char Etn_Earng_Struct_Contn[2000];          /*    7 ETN수익구조내용 (string 2000) */
} TRDESP03801_DATA;

#endif  /* _KRX_TRDESP03801_H */
