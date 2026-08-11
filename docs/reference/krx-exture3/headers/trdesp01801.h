#ifndef _KRX_TRDESP01801_H
#define _KRX_TRDESP01801_H

/* TRDESP01801 해외지수휴장시간정보 송신 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 66 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence[11];                  /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Business_Date[8];                      /*    4 영업일자 (String 8) */
    char Uly_Mkt_Calnd_Id[10];                  /*    5 기초자산시장달력ID (String 10) */
    char Non_Trading_Start_Time[9];             /*    6 휴장시작시각 (String 9) */
    char Non_Trading_End_Time[9];               /*    7 휴장종료시각 (String 9) */
} TRDESP01801_DATA;

#endif  /* _KRX_TRDESP01801_H */
