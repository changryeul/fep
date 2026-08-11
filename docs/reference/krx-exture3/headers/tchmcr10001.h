#ifndef _KRX_TCHMCR10001_H
#define _KRX_TCHMCR10001_H

/* TCHMCR10001 일괄호가취소(MOC) 입력 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 181 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 ★신규 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 ★신규 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 ★신규 (String 2) */
    char Market_Identification[3];              /*    4 시장ID ★신규 (String 3) */
    char Board_Id[2];                           /*    5 보드ID ★신규 (String 2) */
    char Mbr_No[5];                             /*    6 회원번호 ★신규 (String 5) */
    char Issue_Code[12];                        /*    7 종목코드 ★신규 (String 12) */
    char Prod_Id[11];                           /*    8 상품ID ★신규 (String 11) */
    char Rght_Tp_Cd[1];                         /*    9 권리유형코드 ★신규 (String 1) */
    char Trdr_Id[6];                            /*   10 거래자ID ★신규 (String 6) */
    char Askbid_Tp_Cd[1];                       /*   11 매도매수구분코드 ★신규 (String 1) */
    char Acnt_No[12];                           /*   12 계좌번호 ★신규 (String 12) */
    char Ord_Grp_No[2];                         /*   13 호가그룹번호 ★신규 (String 2) */
    char Ord_Media_Tp_Cd[1];                    /*   14 주문매체구분코드 ★신규 (String 1) */
    char Order_Identification_Information[12];  /*   15 주문자식별정보 ★신규 (String 12) */
    char Mac_Addr[12];                          /*   16 MAC주소 ★신규 (String 12) */
    char Decl_Dd[8];                            /*   17 신고일자 ★신규 (String 8) */
    char Decl_Tm[9];                            /*   18 신고시각 ★신규 (String 9) */
    char Member_Use_Area[60];                   /*   19 회원사용영역 ★신규 (String 60) */
} TCHMCR10001_DATA;

#endif  /* _KRX_TCHMCR10001_H */
