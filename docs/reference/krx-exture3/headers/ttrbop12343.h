#ifndef _KRX_TTRBOP12343_H
#define _KRX_TTRBOP12343_H

/* TTRBOP12343 장개시전 협의거래수탁거부신고통보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 96 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Board_Id[2];                           /*    4 보드ID (String 2) */
    char Prod_Id[11];                           /*    5 상품ID (String 11) */
    char Mbr_No[5];                             /*    6 회원번호 (String 5) */
    char Isu_Cd[12];                            /*    7 종목코드 (String 12) */
    char Askbid_Tp_Cd[1];                       /*    8 매도매수구분코드 (String 1) */
    char Modcancl_Tp_Cd[1];                     /*    9 정정취소구분코드 (String 1) */
    char Ord_Dd[8];                             /*   10 호가일자 (String 8) */
    char Trd_Rpt_Id[9];                         /*   11 거래리포트ID (String 9) */
    char Acnt_Pw[9];                            /*   12 계좌비밀번호 (String 9) */
    char Real_Modcancl_Ord_Qty[10];             /*   13 실정정취소호가수량 (Long 10) */
    char Ord_Rej_Rsn_Cd[4];                     /*   14 호가거부사유코드 (String 4) */
} TTRBOP12343_DATA;

#endif  /* _KRX_TTRBOP12343_H */
