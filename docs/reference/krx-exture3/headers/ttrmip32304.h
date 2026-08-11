#ifndef _KRX_TTRMIP32304_H
#define _KRX_TTRMIP32304_H

/* TTRMIP32304 결제적용기준환율 공개 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 53 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Opn_Info_Tp_Cd[3];                     /*    4 공개정보구분코드 (String 3) */
    char Tsc_Prod_Grp_Id[3];                    /*    5 장운영상품그룹ID (String 3) */
    char Opn_Tm[9];                             /*    6 공개시각 (String 9) */
    char Appl_Exchrt[13];                       /*    7 적용환율 (float 13) */
    char Curr_Tp_Cd[1];                         /*    8 통화구분코드 (string 1) */
} TTRMIP32304_DATA;

#endif  /* _KRX_TTRMIP32304_H */
