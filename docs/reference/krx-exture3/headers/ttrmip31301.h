#ifndef _KRX_TTRMIP31301_H
#define _KRX_TTRMIP31301_H

/* TTRMIP31301 공개장운영 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 91 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Tsc_Prod_Grp_Id[3];                    /*    4 장운영상품그룹ID (String 3) */
    char Board_Id[2];                           /*    5 보드ID (String 2) */
    char Brd_Evt_Id[3];                         /*    6 보드이벤트ID ★변경 (String 3) */
    char Brd_Evt_Strt_Tm[9];                    /*    7 보드이벤트시작시각 (String 9) */
    char Brd_Evt_Applgrp_Cd[5];                 /*    8 보드이벤트적용군코드 (Long 5) */
    char Sess_Strt_End_Cd[2];                   /*    9 세션개시종료코드 (String 2) */
    char Sess_Id[2];                            /*   10 세션ID (String 2) */
    char Isu_Cd[12];                            /*   11 종목코드 (String 12) */
    char Class_Id[11];                          /*   12 상품ID (String 11) */
    char Halt_Rsn_Cd[3];                        /*   13 거래정지사유코드 (String 3) */
    char Halt_Occr_Tp_Cd[1];                    /*   14 거래정지발생유형코드 (String 1) */
    char Appl_Step[2];                          /*   15 적용단계 (Long 2) */
    char Bas_Isu_Ple_Occr_Cd[1];                /*   16 기준종목가격확대발생코드 (String 1) */
    char Ple_Schdl_Tm[9];                       /*   17 가격확대예정시각 (String 9) */
    char Evt_Obj_Me_Grp_No[2];                  /*   18 이벤트대상ME그룹번호 (String 2) */
} TTRMIP31301_DATA;

#endif  /* _KRX_TTRMIP31301_H */
