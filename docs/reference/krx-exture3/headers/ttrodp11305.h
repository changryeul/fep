#ifndef _KRX_TTRODP11305_H
#define _KRX_TTRODP11305_H

/* TTRODP11305 회원처리호가 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* (변형: 레이아웃 공유 그룹 TTRODP11301) */
/* DATA 길이 합계 = 318 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Board_Id[2];                           /*    4 보드ID (String 2) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Branch_Number[5];                      /*    6 지점번호 (String 5) */
    char Order_Identification[10];              /*    7 주문ID (String 10) */
    char Original_Order_Identification[10];     /*    8 원주문ID (String 10) */
    char Issue_Code_Symbol[12];                 /*    9 종목코드 (String 12) */
    char Ask_Bid_Type_Code[1];                  /*   10 매도매수구분코드 (String 1) */
    char Modify_Or_Cancel_Type_Code[1];         /*   11 정정취소구분코드 (String 1) */
    char Account_Number[12];                    /*   12 계좌번호 (String 12) */
    char Order_Quantity[10];                    /*   13 호가수량 (Long 10) */
    char Order_Price[11];                       /*   14 호가가격 (Float 11) */
    char Order_Type_Code[1];                    /*   15 호가유형코드 (String 1) */
    char Order_Condition_Code[1];               /*   16 호가조건코드 (String 1) */
    char Min_Trdvol[10];                        /*   17 최소체결수량 ★신규 (Long 10) */
    char Mm_Ord_Tp_Cd[1];                       /*   18 시장조성자호가구분코드 ★변경 (String 1) */
    char Treasury_Stock_Statement_Identification[5];/*   19 자사주신고서ID (String 5) */
    char Treasury_Stock_Trading_Method_Code[1]; /*   20 자사주매매방법코드 (String 1) */
    char Ask_Type_Code[2];                      /*   21 매도유형코드 (String 2) */
    char Credit_Type_Code[2];                   /*   22 신용구분코드 (String 2) */
    char Trust_Principal_Type_Code[2];          /*   23 위탁자기구분코드 (String 2) */
    char Trust_Company_Number[5];               /*   24 위탁사번호 (String 5) */
    char Program_Trading_Type_Code[2];          /*   25 PT구분코드 (String 2) */
    char Substitute_Stock_Certificate_Account_Number[12];/*   26 대용주권계좌번호 (String 12) */
    char Account_Type_Code[2];                  /*   27 계좌구분코드 (String 2) */
    char Account_Margin_Type_Code[2];           /*   28 계좌증거금유형코드 (String 2) */
    char Country_Code[3];                       /*   29 국가코드 (String 3) */
    char Investor_Type_Code[4];                 /*   30 투자자구분코드 (String 4) */
    char Foreign_Investor_Type_Code[2];         /*   31 외국인투자자구분코드 (String 2) */
    char Ord_Media_Tp_Cd[1];                    /*   32 주문매체구분코드 (String 1) */
    char Order_Identification_Information[12];  /*   33 주문자식별정보 (String 12) */
    char Mac_Addr[12];                          /*   34 MAC주소 (String 12) */
    char Order_Date[8];                         /*   35 호가일자 (String 8) */
    char Member_Firm_Order_Time[9];             /*   36 회원사주문시각 (String 9) */
    char Member_Use_Area[60];                   /*   37 회원사용영역 (String 60) */
    char Ord_Acpt_Tm[9];                        /*   38 호가접수시각 (String 9) */
    char Real_Modify_Or_Cancel_Order_Quantity[10];/*   39 실정정취소호가수량 (Long 10) */
    char Automatic_Cancellation_Process_Type_Code[1];/*   40 자동취소처리구분코드 ★변경 (String 1) */
    char Ord_Rej_Rsn_Cd[4];                     /*   41 호가거부사유코드 (String 4) */
    char Algo_Stgy_Tp_Cd[1];                    /*   42 알고리즘전략구분코드 ★신규 (String 1) */
    char Trdr_Id[6];                            /*   43 거래자ID ★신규 (String 6) */
    char Ord_Grp_No[2];                         /*   44 호가그룹번호 ★신규 (String 2) */
    char Smp_Cd[1];                             /*   45 자전거래방지코드 ★신규 (String 1) */
    char Ord_Cond_Prc[11];                      /*   46 호가조건가격 (Float 11) */
    char Trd_Mkt_Choic_Tp_Cd[1];                /*   47 거래시장선택구분코드 (String 1) */
    char Srtsell_Id[10];                        /*   48 공매도ID (String 10) */
} TTRODP11305_DATA;

#endif  /* _KRX_TTRODP11305_H */
