#ifndef _KRX_TTRBOP12344_H
#define _KRX_TTRBOP12344_H

/* TTRBOP12344 장개시전 협의거래주문정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 111 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Prod_Id[11];                           /*    4 상품ID (String 11) */
    char Trd_Rpt_Id[9];                         /*    5 거래리포트ID (String 9) */
    char Isu_Cd[12];                            /*    6 종목코드 (String 12) */
    char Ord_Qty[10];                           /*    7 호가수량 (Long 10) */
    char Ord_Prc[11];                           /*    8 호가가격 (Float 11) */
    char Trd_Dd[8];                             /*    9 거래일자 (String 8) */
    char Ask_Bid_Tp_Cd[1];                      /*   10 매도매수구분코드 (String 1) */
    char Mbr_No[5];                             /*   11 회원번호 (String 5) */
    char Acnt_Pw[9];                            /*   12 계좌비밀번호 (String 9) */
    char Lst_Procs_Seq[11];                     /*   13 최종처리일련번호 (Long 11) */
} TTRBOP12344_DATA;

#endif  /* _KRX_TTRBOP12344_H */
