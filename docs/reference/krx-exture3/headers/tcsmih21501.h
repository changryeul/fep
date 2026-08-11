#ifndef _KRX_TCSMIH21501_H
#define _KRX_TCSMIH21501_H

/* TCSMIH21501 거래증거금및결제대금변동통보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Mkt_Id[3];                             /*    4 시장ID ★변경 (String 3) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Margin_Settlement_Type_Code[1];        /*    6 거래증거금결제금액구분코드 (String 1) */
    char Trust_Principal_Integration_Type_Code[2];/*    7 위탁자기통합구분코드 (String 2) */
    char Before_Change_Amount[22];              /*    8 변경전금액 (Float 22) */
    char After_Change_Amount[22];               /*    9 변경후금액 (Float 22) */
    char Change_Reason[2];                      /*   10 금액변경사유코드 (String 2) */
    char Filler_Value[1113];                    /*   11 필러값 (String 1113) */
} TCSMIH21501_DATA;

#endif  /* _KRX_TCSMIH21501_H */
