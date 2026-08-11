#ifndef _KRX_TRDESP03501_H
#define _KRX_TRDESP03501_H

/* TRDESP03501 위탁/거래증거금율 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence[11];                  /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 ★신규 (String 1) */
    char Business_Date[8];                      /*    5 영업일자 (String 8) */
    char Parm_Id[10];                           /*    6 파라미터ID (string 10) */
    char Prod_Id[11];                           /*    7 상품ID (string 11) */
    char Mrgn_Rt_Val[19];                       /*    8 증거금율값 (Long 19) */
    char Mrgn_Rto_Decipnt_Info[10];             /*    9 증거금율소수점정보 (Long 10) */
    char Filr_Val[1111];                        /*   10 필러값 (String 1111) */
} TRDESP03501_DATA;

#endif  /* _KRX_TRDESP03501_H */
