#ifndef _KRX_TRDESP03601_H
#define _KRX_TRDESP03601_H

/* TRDESP03601 ETP지수구성 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 224 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (string 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (string 8) */
    char Idx_Calc_Inst_Cd[2];                   /*    4 지수산출기관코드 (string 2) */
    char Idx_Mkt_Clss_Id[6];                    /*    5 지수시장분류ID (string 6) */
    char Idx_Seq[3];                            /*    6 지수일련번호 (string 3) */
    char Idx_Lvrg_Invrs_Tp_Cd[2];               /*    7 지수레버리지인버스구분코드 (string 2) */
    char Idx_Nm[80];                            /*    8 지수명 (string 80) */
    char Idx_Eng_Nm[80];                        /*    9 지수영문명 (string 80) */
    char Idx_Id[12];                            /*   10 지수ID (string 12) */
    char Filler[9];                             /*   11 필러값 (String 9) */
} TRDESP03601_DATA;

#endif  /* _KRX_TRDESP03601_H */
