#ifndef _KRX_TCSMIH26901_H
#define _KRX_TCSMIH26901_H

/* TCSMIH26901 옵션델타통보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msgseq[11];                            /*    1 메세지일련번호 (Long 11) */
    char Trcd[11];                              /*    2 트랜잭션코드 (String 11) */
    char Trnsmdd[8];                            /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    5 시각대완료여부 (String 1) */
    char Finalyn[1];                            /*    6 확정여부 (String 1) */
    char Mbrno[5];                              /*    7 회원번호 ★변경 (String 5) */
    char Calctm[9];                             /*    8 산출시각 (string 9) */
    char Isucd[12];                             /*    9 종목코드 (string 12) */
    char Delta[20];                             /*   10 델타 (Float 20) */
    char Filrval[1121];                         /*   11 필러값 (String 1121) */
} TCSMIH26901_DATA;

#endif  /* _KRX_TCSMIH26901_H */
