#ifndef _KRX_TTRMIP31306_H
#define _KRX_TTRMIP31306_H

/* TTRMIP31306 배분정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 49 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Board_Id[2];                           /*    4 보드ID (String 2) */
    char Isu_Cd[12];                            /*    5 종목코드 (String 12) */
    char Alloc_Appl_Tp_Cd[1];                   /*    6 배분적용구분코드 (String 1) */
    char Alloc_Procs_Tp_Cd[1];                  /*    7 배분처리구분코드 (String 1) */
    char Alloc_Releas_Tm[9];                    /*    8 배분해제시각 (String 9) */
} TTRMIP31306_DATA;

#endif  /* _KRX_TTRMIP31306_H */
