#ifndef _KRX_TRDESP50102_H
#define _KRX_TRDESP50102_H

/* TRDESP50102 KTS종목정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 350 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Bz_Dd[8];                              /*    4 영업일자 (string 8) */
    char Market_Identification[3];              /*    5 시장ID (String 3) */
    char Board_Id[2];                           /*    6 보드ID (String 2) */
    char Isu_Cd[12];                            /*    7 종목코드 (string 12) */
    char Clsprc[11];                            /*    8 종가 (float 11) */
    char Clsprc_Yield[13];                      /*    9 종가수익률 (float 13) */
    char Bas_Prc[11];                           /*   10 기준가격,기준가액 (float 11) */
    char Field[1];                              /*   11 국채종목구분코드 (string 1) */
    char Field_2[3];                            /*   12 국채종류코드 (string 3) */
    char Halt_Yn[1];                            /*   13 거래정지여부 (string 1) */
    char Halt_Rsn_Cd[3];                        /*   14 거래정지사유코드 (string 3) */
    char Issue_Korean_Name[80];                 /*   15 종목한글명 (string 80) */
    char Issue_Korean_Abbreviation[40];         /*   16 종목한글약명 (string 40) */
    char Issue_English_Name[80];                /*   17 종목영문명 (string 80) */
    char Issue_English_Abbreviation[40];        /*   18 종목영문약명 (string 40) */
    char Infllnk_Yn[1];                         /*   19 물가연동여부 (string 1) */
    char Strip_Mm_Yn[1];                        /*   20 스트립시장조성여부 (string 1) */
    char Prc_Unit_Rule_Id[10];                  /*   21 가격단위규칙ID (String 10) */
} TRDESP50102_DATA;

#endif  /* _KRX_TRDESP50102_H */
