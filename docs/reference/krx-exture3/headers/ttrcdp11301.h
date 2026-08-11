#ifndef _KRX_TTRCDP11301_H
#define _KRX_TTRCDP11301_H

/* TTRCDP11301 Cancel On Disconnect(COD) 처리 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 38 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 ★신규 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 ★신규 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 ★신규 (String 2) */
    char Cod_Occr_Rsn_Cd[2];                    /*    4 COD발생사유코드 ★신규 (String 2) */
    char Tg_Micr_Tm[12];                        /*    5 발동시각 ★신규 (String 12) */
} TTRCDP11301_DATA;

#endif  /* _KRX_TTRCDP11301_H */
