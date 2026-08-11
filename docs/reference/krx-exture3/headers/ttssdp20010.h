#ifndef _KRX_TTSSDP20010_H
#define _KRX_TTSSDP20010_H

/* TTSSDP20010 착오추정호가적출 회원사송신 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Dtec_Dd[8];                            /*    3 적출일자 (String 8) */
    char Ord_Id[10];                            /*    4 주문ID,호가ID (String 10) */
    char Ord_Acpt_No[11];                       /*    5 호가접수번호 (Long 11) */
    char Dtec_Tm[9];                            /*    6 적출시각,적출시점 (String 9) */
    char Mbr_No[5];                             /*    7 회원번호 (String 5) */
    char Brn_No[5];                             /*    8 지점번호 (String 5) */
    char Prod_Id[11];                           /*    9 상품ID (String 11) */
    char Isu_Cd[12];                            /*   10 종목코드 (String 12) */
    char Acnt_No[12];                           /*   11 계좌번호 (String 12) */
    char Trst_Princ_Tp_Cd[2];                   /*   12 위탁자기구분코드 (String 2) */
    char Askbid_Tp_Cd[1];                       /*   13 매도매수구분코드 (String 1) */
    char Modcancl_Tp_Cd[1];                     /*   14 정정취소구분코드 (String 1) */
    char Ord_Tp_Cd[1];                          /*   15 호가유형코드,주문유형코드 (String 1) */
    char Ord_Prc[11];                           /*   16 호가가격,주문가격 (Float 11) */
    char Ord_Qty[10];                           /*   17 호가수량,주문수량 (Long 10) */
    char Bas_Prc[11];                           /*   18 기준가격,기준가액 (Float 11) */
    char Errtrd_Deem_Loss_Amt[22];              /*   19 착오거래예상손실금액 (Float 22) */
    char Filler[1036];                          /*   20 필러값 (String 1036) */
} TTSSDP20010_DATA;

#endif  /* _KRX_TTSSDP20010_H */
