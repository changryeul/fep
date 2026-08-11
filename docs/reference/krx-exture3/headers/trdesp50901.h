#ifndef _KRX_TRDESP50901_H
#define _KRX_TRDESP50901_H

/* TRDESP50901 소매채권 분류코드정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 81 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Bz_Dd[8];                              /*    4 영업일자 (string 8) */
    char Retail_Bnd_Clss_Cd[2];                 /*    5 소매채권분류코드 (string 2) */
    char Retail_Bnd_Clss_Nm[20];                /*    6 소매채권분류명 (string 20) */
    char Retail_Bnd_Clss_Eng_Nm[20];            /*    7 소매채권분류영문명 (string 20) */
    char Retail_Bnd_Mm_Ord_Posbl_Yn[1];         /*    8 소매채권조성호가가능여부 (string 1) */
} TRDESP50901_DATA;

#endif  /* _KRX_TRDESP50901_H */
