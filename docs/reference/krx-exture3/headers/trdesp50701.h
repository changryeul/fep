#ifndef _KRX_TRDESP50701_H
#define _KRX_TRDESP50701_H

/* TRDESP50701 소액채권 신고시장수익률 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 54 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Dd[8];                                 /*    4 일자 (string 8) */
    char Small_Trd_Kind_Cd[3];                  /*    5 소액매매종류코드 (string 3) */
    char Decl_Mkt_Clsprc_Yd[13];                /*    6 신고시장종가수익률 (float 13) */
} TRDESP50701_DATA;

#endif  /* _KRX_TRDESP50701_H */
