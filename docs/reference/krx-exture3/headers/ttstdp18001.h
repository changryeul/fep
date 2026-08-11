#ifndef _KRX_TTSTDP18001_H
#define _KRX_TTSTDP18001_H

/* TTSTDP18001 채권-회원WIT확정결과 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 318 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Board_Id[2];                           /*    5 보드ID (String 2) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Branch_Number[5];                      /*    7 지점번호 (String 5) */
    char Order_Identification[10];              /*    8 주문ID (String 10) */
    char Original_Order_Identification[10];     /*    9 원주문ID (String 10) */
    char Issue_Code[12];                        /*   10 종목코드 (String 12) */
    char Trading_Number[11];                    /*   11 체결번호 (Long 11) */
    char Trading_Yld[13];                       /*   12 체결수익률 (Float 13) */
    char Trading_Price[11];                     /*   13 체결가격 (Float 11) */
    char Trading_Volumn[10];                    /*   14 체결수량 (Long 10) */
    char Sess_Id[2];                            /*   15 세션ID (String 2) */
    char Trading_Date[8];                       /*   16 체결일자 (String 8) */
    char Trading_Time[9];                       /*   17 체결시각 (String 9) */
    char Ask_Bid_Type_Code[1];                  /*   18 매도매수구분코드 (String 1) */
    char Ask_Type_Code[2];                      /*   19 매도유형코드 (String 2) */
    char Account_Number[12];                    /*   20 계좌번호 (String 12) */
    char Ord_Kind_Cd[1];                        /*   21 채권호가종류코드 (String 1) */
    char Ord_Qty[10];                           /*   22 호가수량 (Long 10) */
    char Ord_Prc[11];                           /*   23 호가가격 (Float 11) */
    char Yld[13];                               /*   24 주문수익률 (Float 13) */
    char Trst_Princ_Tp_Cd[2];                   /*   25 위탁자기구분코드 (String 2) */
    char Trust_Company_Number[5];               /*   26 위탁사번호 (String 5) */
    char Account_Type_Code[2];                  /*   27 계좌구분코드 (String 2) */
    char Ask_Invst_Tp_Cd[4];                    /*   28 투자자구분코드 (String 4) */
    char Filler[6];                             /*   29 필러값 (String 6) */
    char Foreign_Investor_Type_Code[2];         /*   30 외국인투자자구분코드 (String 2) */
    char Ord_Media_Tp_Cd[1];                    /*   31 주문매체구분코드 (String 1) */
    char Order_Identification_Information[12];  /*   32 주문자식별정보 (String 12) */
    char Mac_Addr[12];                          /*   33 MAC주소 (String 12) */
    char Efct_Stop_Reopn_Tp_Cd[1];              /*   34 효력정지재개구분코드 (String 1) */
    char Member_Use_Area[60];                   /*   35 회원사용영역 (String 60) */
    char Trader_No[5];                          /*   36 거래원번호 (String 5) */
    char Setl_Dd[8];                            /*   37 결제일자 (String 8) */
    char Mm_Ord_Tp_No[11];                      /*   38 시장조성자호가구분번호 (Long 11) */
    char Lst_Askbid_Tp_Cd[1];                   /*   39 최종매도매수구분코드 (String 1) */
    char Cancl_Yn[1];                           /*   40 취소여부 (String 1) */
} TTSTDP18001_DATA;

#endif  /* _KRX_TTSTDP18001_H */
