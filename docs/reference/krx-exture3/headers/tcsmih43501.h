#ifndef _KRX_TCSMIH43501_H
#define _KRX_TCSMIH43501_H

/* TCSMIH43501 장중추가증거금_위탁자기별종목별포지션/VWAP(증권시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 193 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Round_Number[4];                       /*    5 회차번호 (Long 4) */
    char Round_Time[9];                         /*    6 회차시각 (String 9) */
    char Calculate_Base_Time[9];                /*    7 데이터산출기준시각 (String 9) */
    char Member_Number[5];                      /*    8 회원번호 (String 5) */
    char Non_Clrearing_Member_Number[5];        /*    9 거래전문회원번호 (String 5) */
    char Trust_Principal_Intergration_Type_Code[2];/*   10 위탁자기통합구분코드 (String 2) */
    char Market_Identification[3];              /*   11 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*   12 증권그룹ID (String 2) */
    char Issue_Code[12];                        /*   13 종목코드 (String 12) */
    char Ask_Trading_Volumn[12];                /*   12 매도체결수량 (Long 12) */
    char Bid_Trading_Volumn[12];                /*   13 매수체결수량 (Long 12) */
    char Trading_Volumn_Weighted_Average_Price[10];/*   14 거래량가중평균가격 (Long 10) */
    char Filler_Value[77];                      /*   15 필러값 (String 77) */
} TCSMIH43501_DATA;

#endif  /* _KRX_TCSMIH43501_H */
