#ifndef _KRX_TTRMIP32303_H
#define _KRX_TTRMIP32303_H

/* TTRMIP32303 회원 제재/해제 공개 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 63 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Opn_Info_Tp_Cd[3];                     /*    4 공개정보구분코드 (String 3) */
    char Isu_Cd[12];                            /*    5 종목코드 (String 12) */
    char Opn_Tm[9];                             /*    6 공개시각 (String 9) */
    char Mbr_No[5];                             /*    7 회원번호 (String 5) */
    char Brn_No[5];                             /*    8 지점번호 (String 5) */
    char Mbr_Trd_Scope_Cd[5];                   /*    9 회원거래범위코드 (Long 5) */
} TTRMIP32303_DATA;

#endif  /* _KRX_TTRMIP32303_H */
