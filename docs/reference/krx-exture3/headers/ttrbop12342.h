#ifndef _KRX_TTRBOP12342_H
#define _KRX_TTRBOP12342_H

/* TTRBOP12342 수탁거부미제출회원사통보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 40 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Prod_Id[11];                           /*    4 상품ID (String 11) */
    char Mbr_No[5];                             /*    5 회원번호 (String 5) */
} TTRBOP12342_DATA;

#endif  /* _KRX_TTRBOP12342_H */
