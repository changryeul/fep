#ifndef _KRX_TTRMIP31309_H
#define _KRX_TTRMIP31309_H

/* TTRMIP31309 가격제한폭확대발동 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 84 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Board_Id[2];                           /*    4 보드ID (String 2) */
    char Issue_Code[12];                        /*    5 종목코드 (String 12) */
    char Me_Procs_Tm[9];                        /*    6 ME처리시각 (String 9) */
    char Prc_Expn_Tm[9];                        /*    7 가격확대시각 (String 9) */
    char Ple_Uplmt_Step[3];                     /*    8 가격제한확대상한단계 (Long 3) */
    char Ple_Lwlmt_Step[3];                     /*    9 가격제한확대하한단계 (Long 3) */
    char Uplmtprc[11];                          /*   10 상한가 (Float 11) */
    char Lwlmtprc[11];                          /*   11 하한가 (Float 11) */
} TTRMIP31309_DATA;

#endif  /* _KRX_TTRMIP31309_H */
