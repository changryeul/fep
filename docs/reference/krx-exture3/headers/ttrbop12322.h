#ifndef _KRX_TTRBOP12322_H
#define _KRX_TTRBOP12322_H

/* TTRBOP12322 회원대량처리호가 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* (변형: 레이아웃 공유 그룹 TTRBOP12301) */
/* DATA 길이 합계 = 320 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Board_Id[2];                           /*    4 보드ID (String 2) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Branch_Number[5];                      /*    6 지점번호 (String 5) */
    char Order_Identification[10];              /*    7 주문ID (String 10) */
    char Original_Order_Identification[10];     /*    8 원주문ID (String 10) */
    char Issue_Code[12];                        /*    9 종목코드 (String 12) */
    char Ask_Bid_Type_Code[1];                  /*   10 매도매수구분코드 (String 1) */
    char Modify_Or_Cancel_Type_Code[1];         /*   11 정정취소구분코드 (String 1) */
    char Account_Number[12];                    /*   12 계좌번호 (String 12) */
    char Order_Quantity[10];                    /*   13 호가수량 (Long 10) */
    char Order_Price[11];                       /*   14 호가가격 (Float 11) */
    char Treasury_Stock_Statement_Identification[5];/*   15 자사주신고서ID (String 5) */
    char Treasury_Stock_Trading_Method_Code[1]; /*   16 자사주매매방법코드 (String 1) */
    char Ask_Type_Code[2];                      /*   17 매도유형코드 (String 2) */
    char Credit_Type_Code[2];                   /*   18 신용구분코드 (String 2) */
    char Trust_Principal_Type_Code[2];          /*   19 위탁자기구분코드 (String 2) */
    char Trust_Company_Number[5];               /*   20 위탁사번호 (String 5) */
    char Account_Type_Code[2];                  /*   21 계좌구분코드 (String 2) */
    char Account_Margin_Type_Code[2];           /*   22 계좌증거금유형코드 (String 2) */
    char Country_Code[3];                       /*   23 국가코드 (String 3) */
    char Investor_Type_Code[4];                 /*   24 투자자구분코드 (String 4) */
    char Foreign_Investor_Type_Code[2];         /*   25 외국인투자자구분코드 (String 2) */
    char Ord_Media_Tp_Cd[1];                    /*   26 주문매체구분코드 (String 1) */
    char Order_Identification_Information[12];  /*   27 주문자식별정보 (String 12) */
    char Mac_Addr[12];                          /*   28 MAC주소 (String 12) */
    char Negotiation_Number[6];                 /*   29 협상번호 (Long 6) */
    char Block_Trade_Negotiation_Detail_Number[6];/*   30 협상상세번호 (Long 6) */
    char Negotiator_Identification[10];         /*   31 협상자ID (String 10) */
    char Counterpart_Member_Number[5];          /*   32 상대회원번호 (String 5) */
    char Counterpart_Account_Number[12];        /*   33 상대계좌번호 (String 12) */
    char Negotiation_Completion_Time[9];        /*   34 협의완료시각 (String 9) */
    char Order_Date[8];                         /*   35 호가일자 (String 8) */
    char Member_Firm_Order_Time[9];             /*   36 회원사주문시각 (String 9) */
    char Member_Use_Area[60];                   /*   37 회원사용영역 (String 60) */
    char Ord_Acpt_Tm[9];                        /*   38 호가접수시각 (String 9) */
    char Ord_Rej_Rsn_Cd[4];                     /*   39 호가거부사유코드 (String 4) */
    char Spot_Prc[11];                          /*   40 현물가격 (Float 11) */
    char Pt_Tp_Cd[2];                           /*   41 PT구분코드 (String 2) */
    char Trd_Mkt_Choic_Tp_Cd[1];                /*   42 거래시장선택구분코드 (String 1) */
    char Srtsell_Id[10];                        /*   43 공매도ID (String 10) */
} TTRBOP12322_DATA;

#endif  /* _KRX_TTRBOP12322_H */
