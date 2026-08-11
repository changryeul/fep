#ifndef _KRX_TTRMOP41302_H
#define _KRX_TTRMOP41302_H

/* TTRMOP41302 채권조성처리호가 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* (변형: 레이아웃 공유 그룹 TTRMOP41301) */
/* DATA 길이 합계 = 295 (interface-list.csv '길이(헤더제외)'와 대조) */
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
    char Ask_Bid_Type_Code[1];                  /*   11 매도매수구분코드 (String 1) */
    char Modify_Or_Cancel_Type_Code[1];         /*   12 정정취소구분코드 (String 1) */
    char Ord_Kind_Cd[1];                        /*   13 채권호가종류코드 (String 1) */
    char Account_Number[12];                    /*   14 계좌번호 (String 12) */
    char Ask_Ord_Qty[10];                       /*   15 매도호가수량 (Long 10) */
    char Ask_Ord_Prc[11];                       /*   16 매도호가가격 (Float 11) */
    char Ask_Ord_Yld[13];                       /*   17 매도호가수익률 (Float 13) */
    char Bid_Ord_Qty[10];                       /*   18 매수호가수량 (Long 10) */
    char Bid_Ord_Prc[11];                       /*   19 매수호가가격 (Float 11) */
    char Bid_Ord_Yld[13];                       /*   20 매수호가수익률 (Float 13) */
    char Order_Type_Code[1];                    /*   21 호가유형코드 (String 1) */
    char Order_Condition_Code[1];               /*   22 호가조건코드 (String 1) */
    char Investor_Type_Code[4];                 /*   23 투자자구분코드 (String 4) */
    char Efct_Stop_Reopn_Tp_Cd[1];              /*   24 효력정지재개구분코드 (String 1) */
    char Ord_Media_Tp_Cd[1];                    /*   25 주문매체구분코드 (String 1) */
    char Order_Identification_Information[12];  /*   26 주문자식별정보 (String 12) */
    char Mac_Addr[12];                          /*   27 MAC주소 (String 12) */
    char Order_Date[8];                         /*   28 호가일자 (String 8) */
    char Member_Firm_Order_Time[9];             /*   29 회원사주문시각 (String 9) */
    char Member_Use_Area[60];                   /*   30 회원사용영역 (String 60) */
    char Ord_Acpt_Tm[9];                        /*   31 호가접수시각 (String 9) */
    char Trader_No[5];                          /*   32 거래원번호 (String 5) */
    char Automatic_Cancellation_Process_Type_Code[1];/*   33 자동취소처리구분코드 (String 1) */
    char Ord_Rej_Rsn_Cd[4];                     /*   34 호가거부사유코드 (String 4) */
    char Mm_Ord_Tp_No[11];                      /*   35 시장조성자호가구분번호 (Long 11) */
    char Account_Type_Code[2];                  /*   36 계좌구분코드 (String 2) */
} TTRMOP41302_DATA;

#endif  /* _KRX_TTRMOP41302_H */
