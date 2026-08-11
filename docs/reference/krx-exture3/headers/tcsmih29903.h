#ifndef _KRX_TCSMIH29903_H
#define _KRX_TCSMIH29903_H

/* TCSMIH29903 업무마감#2 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Spot_Futures_Type_Code[1];             /*    4 현선물구분코드 (String 1) */
    char Emsg_Complt_Yn[1];                     /*    5 전문완료여부 (String 1) */
    char Filler_Value[1168];                    /*    6 필러값 (String 1168) */
} TCSMIH29903_DATA;

#endif  /* _KRX_TCSMIH29903_H */
