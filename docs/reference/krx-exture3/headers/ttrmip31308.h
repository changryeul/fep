#ifndef _KRX_TTRMIP31308_H
#define _KRX_TTRMIP31308_H

/* TTRMIP31308 실시간가격제한 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 70 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Board_Id[2];                           /*    4 보드ID (String 2) */
    char Issue_Code[12];                        /*    5 종목코드 (String 12) */
    char Me_Procs_Tm[9];                        /*    6 ME처리시각 (String 9) */
    char Dynmc_Prc_Lmt_Estb_Cd[1];              /*    7 실시간가격제한설정코드 (String 1) */
    char Dynmc_Uplmtprc[11];                    /*    8 실시간상한가 (Float 11) */
    char Dynmc_Lwlmtprc[11];                    /*    9 실시간하한가 (Float 11) */
} TTRMIP31308_DATA;

#endif  /* _KRX_TTRMIP31308_H */
