#ifndef _KRX_TRDESP50501_H
#define _KRX_TRDESP50501_H

/* TRDESP50501 소액채권 매매종류정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 72 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Bz_Dd[8];                              /*    4 영업일자 (String 8) */
    char Small_Trd_Kind_Cd[3];                  /*    5 소액매매종류코드 (string 3) */
    char Small_Trd_Kind_Nm[20];                 /*    6 소액매매종류명 (string 20) */
    char Small_Trd_Kind_Abbrv[10];              /*    7 소액매매종류약명 (string 10) */
    char Kind_Ord_Posbl_Yn[1];                  /*    8 종류호가가능여부 (string 1) */
} TRDESP50501_DATA;

#endif  /* _KRX_TRDESP50501_H */
