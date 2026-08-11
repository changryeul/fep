#ifndef _KRX_TCHHOR10003_H
#define _KRX_TCHHOR10003_H

/* TCHHOR10003 장개시전 협의거래호가입력 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* (변형: 레이아웃 공유 그룹 TCHHOR10001) */
/* DATA 길이 합계 = 243 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 ★신규 (String 2) */
    char Board_Id[2];                           /*    4 보드ID (String 2) */
    char Mbr_No[5];                             /*    5 회원번호 (String 5) */
    char Brn_No[5];                             /*    6 지점번호 (String 5) */
    char Ord_Id[10];                            /*    7 주문ID (String 10) */
    char Orgn_Ord_Id[10];                       /*    8 원주문ID (String 10) */
    char Isu_Cd[12];                            /*    9 종목코드 (String 12) */
    char Ask_Bid_Tp_Cd[1];                      /*   10 매도매수구분코드 (String 1) */
    char Modcancl_Tp_Cd[1];                     /*   11 정정취소구분코드 (String 1) */
    char Acnt_No[12];                           /*   12 계좌번호 (String 12) */
    char Ord_Qty[10];                           /*   13 호가수량 (Long 10) */
    char Ord_Prc[11];                           /*   14 호가가격 (Float 11) */
    char Trst_Princ_Tp_Cd[2];                   /*   15 위탁자기구분코드 (String 2) */
    char Trust_Company_Number[5];               /*   16 위탁사번호 (String 5) */
    char Acnt_Tp_Cd[2];                         /*   17 계좌구분코드 (String 2) */
    char Acnt_Mrgn_Tp_Cd[2];                    /*   18 계좌증거금유형코드 (String 2) */
    char Cntr_Cd[3];                            /*   19 국가코드 (String 3) */
    char Invst_Tp_Cd[4];                        /*   20 투자자구분코드 (String 4) */
    char Forninvst_Tp_Cd[2];                    /*   21 외국인투자자구분코드 (String 2) */
    char Ord_Media_Tp_Cd[1];                    /*   22 주문매체구분코드 (String 1) */
    char Order_Identification_Information[12];  /*   23 주문자식별정보 (String 12) */
    char Mac_Addr[12];                          /*   24 MAC주소 (String 12) */
    char Ord_Dd[8];                             /*   25 호가일자 (String 8) */
    char Mbr_Ord_Tm[9];                         /*   26 회원사주문시각 (String 9) */
    char Mbr_Use_Area[60];                      /*   27 회원사용영역 (String 60) */
    char Trd_Rpt_Id[9];                         /*   28 거래리포트ID (String 9) */
    char Acnt_Pw[9];                            /*   29 계좌비밀번호 (String 9) */
} TCHHOR10003_DATA;

#endif  /* _KRX_TCHHOR10003_H */
