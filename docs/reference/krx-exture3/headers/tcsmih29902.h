#ifndef _KRX_TCSMIH29902_H
#define _KRX_TCSMIH29902_H

/* TCSMIH29902 최종마감 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Filler_Value[1164];                    /*    6 필러값 (String 1164) */
} TCSMIH29902_DATA;

#endif  /* _KRX_TCSMIH29902_H */
