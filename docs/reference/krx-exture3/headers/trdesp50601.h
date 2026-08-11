#ifndef _KRX_TRDESP50601_H
#define _KRX_TRDESP50601_H

/* TRDESP50601 소액채권 전담회원정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 43 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Bz_Dd[8];                              /*    4 영업일자 (String 8) */
    char Mbr_No[5];                             /*    5 회원번호 (string 5) */
} TRDESP50601_DATA;

#endif  /* _KRX_TRDESP50601_H */
