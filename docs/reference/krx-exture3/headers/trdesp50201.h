#ifndef _KRX_TRDESP50201_H
#define _KRX_TRDESP50201_H

/* TRDESP50201 채권종목별 신용평가정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 62 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Bz_Dd[8];                              /*    4 영업일자 (String 8) */
    char Market_Identification[3];              /*    5 시장ID (String 3) */
    char Isu_Cd[12];                            /*    6 종목코드 (string 12) */
    char Credit_Valu_Inst_Cd[4];                /*    7 신용평가기관코드 (string 4) */
    char Byinst_Credit_Valu_Grd[4];             /*    8 기관별신용평가등급 (string 4) */
    char Sf_Valu_Yn[1];                         /*    9 SF평가여부 (string 1) */
} TRDESP50201_DATA;

#endif  /* _KRX_TRDESP50201_H */
