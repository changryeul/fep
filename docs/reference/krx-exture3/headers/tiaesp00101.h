#ifndef _KRX_TIAESP00101_H
#define _KRX_TIAESP00101_H

/* TIAESP00101 종목지수정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 61 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Business_Date[8];                      /*    4 영업일자 (String 8) */
    char Market_Identification[3];              /*    5 시장ID (String 3) */
    char Issue_Code[12];                        /*    6 종목코드 (String 12) */
    char Krx100_Yn[1];                          /*    7 KRX100여부 (String 1) */
    char Kospi_Yn[1];                           /*    8 KOSPI여부 (String 1) */
    char Kospi200_Yn[1];                        /*    9 KOSPI200여부 (String 1) */
    char Kospi100_Yn[1];                        /*   10 KOSPI100여부 (String 1) */
    char Kospi50_Yn[1];                         /*   11 KOSPI50여부 (String 1) */
    char Krx300_Yn[1];                          /*   12 KRX300여부 (String 1) */
    char Kosdaq_Yn[1];                          /*   13 KOSDAQ여부 (String 1) */
    char Kosdaq150_Yn[1];                       /*   14 KOSDAQ150여부 (String 1) */
} TIAESP00101_DATA;

#endif  /* _KRX_TIAESP00101_H */
