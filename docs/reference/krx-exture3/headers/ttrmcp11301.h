#ifndef _KRX_TTRMCP11301_H
#define _KRX_TTRMCP11301_H

/* TTRMCP11301 일괄호가취소(MOC) 처리 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 192 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 ★신규 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 ★신규 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 ★신규 (String 2) */
    char Member_Number[5];                      /*    4 회원번호 ★신규 (String 5) */
    char Issue_Code[12];                        /*    5 종목코드 ★신규 (String 12) */
    char Prod_Id[11];                           /*    6 상품ID ★신규 (String 11) */
    char Rght_Tp_Cd[1];                         /*    7 권리유형코드 ★신규 (String 1) */
    char Trdr_Id[6];                            /*    8 거래자ID ★신규 (String 6) */
    char Askbid_Tp_Cd[1];                       /*    9 매도매수구분코드 ★신규 (String 1) */
    char Acnt_No[12];                           /*   10 계좌번호 ★신규 (String 12) */
    char Ord_Grp_No[2];                         /*   11 호가그룹번호 ★신규 (String 2) */
    char Ord_Media_Tp_Cd[1];                    /*   12 주문매체구분코드 ★신규 (String 1) */
    char Order_Identification_Information[12];  /*   13 주문자식별정보 (String 12) */
    char Mac_Addr[12];                          /*   14 MAC주소 ★신규 (String 12) */
    char Order_Date[8];                         /*   15 신고일자 ★신규 (String 8) */
    char Mbr_Ord_Tm[9];                         /*   16 신고시각 ★신규 (String 9) */
    char Member_Use_Area[60];                   /*   17 회원사용영역 ★신규 (String 60) */
    char Ord_Rej_Rsn_Cd[4];                     /*   18 호가거부사유코드 ★신규 (String 4) */
    char Tg_Micr_Tm[12];                        /*   19 발동시각 ★신규 (String 12) */
} TTRMCP11301_DATA;

#endif  /* _KRX_TTRMCP11301_H */
