#ifndef _KRX_TTSSDP10004_H
#define _KRX_TTSSDP10004_H

/* TTSSDP10004 회원별약정(프리마켓) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Market_Identification[3];              /*    3 시장ID (String 3) */
    char Transmit_Date[8];                      /*    4 전송일자 (String 8) */
    char Trading_Date[8];                       /*    5 거래일자 (String 8) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Ask_Trading_Volumn[12];                /*    7 누적매도체결수량 (Long 12) */
    char Bid_Trading_Volumn[12];                /*    8 누적매수체결수량 (Long 12) */
    char Ask_Trading_Value[22];                 /*    9 누적매도거래대금 (Float 22) */
    char Bid_Trading_Value[22];                 /*   10 누적매수거래대금 (Float 22) */
    char Filler[1086];                          /*   11 필러값 (String 1086) */
} TTSSDP10004_DATA;

#endif  /* _KRX_TTSSDP10004_H */
