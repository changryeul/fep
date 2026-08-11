#ifndef _KRX_TCSMIH27201_H
#define _KRX_TCSMIH27201_H

/* TCSMIH27201 Buy-in실행정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Tracsaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Buyin_Type_Complt_Yn[1];               /*    5 Buy-in구분완료여부 (String 1) */
    char Buyin_Type_Code[1];                    /*    6 Buy-in구분코드 (String 1) */
    char Trading_Date[8];                       /*    7 거래일자 (String 8) */
    char Settlement_Date[8];                    /*    8 결제일자 (String 8) */
    char Member_Number[5];                      /*    9 회원번호 (String 5) */
    char Market_Identification[3];              /*   10 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*   11 증권그룹ID (String 2) */
    char Issue_Code[12];                        /*   12 종목코드 (String 12) */
    char Trust_Principal_Type_Code[2];          /*   13 위탁자기구분코드 (String 2) */
    char Buyin_Total_Quantity[15];              /*   14 Buy-in 실행수량 (Long 15) */
    char Buyin_Bid_Order_Quantity[15];          /*   15 Buy-in 매수 주문수량 (Long 15) */
    char Filler_Value[1097];                    /*   16 FILLER (String 1097) */
} TCSMIH27201_DATA;

#endif  /* _KRX_TCSMIH27201_H */
