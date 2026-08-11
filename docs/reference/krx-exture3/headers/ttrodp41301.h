#ifndef _KRX_TTRODP41301_H
#define _KRX_TTRODP41301_H

/* TTRODP41301 채권일반처리호가 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 291 (interface-list.csv '길이(헤더제외)'와 대조) */
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
    char Account_Number[12];                    /*   13 계좌번호 (String 12) */
    char Order_Quantity[10];                    /*   14 호가수량 (Long 10) */
    char Order_Price[11];                       /*   15 호가가격 (Float 11) */
    char Order_Yld[13];                         /*   16 호가수익률 (Float 13) */
    char Order_Type_Code[1];                    /*   17 호가유형코드 (String 1) */
    char Order_Condition_Code[1];               /*   18 호가조건코드 (String 1) */
    char Ask_Type_Code[2];                      /*   19 매도유형코드 (String 2) */
    char Trust_Principal_Type_Code[2];          /*   20 위탁자기구분코드 (String 2) */
    char Trust_Company_Number[5];               /*   21 위탁사번호 (String 5) */
    char Account_Type_Code[2];                  /*   22 계좌구분코드 (String 2) */
    char Country_Code[3];                       /*   23 국가코드 (String 3) */
    char Investor_Type_Code[4];                 /*   24 투자자구분코드 (String 4) */
    char Filler[6];                             /*   25 필러값 (String 6) */
    char Foreign_Investor_Type_Code[2];         /*   26 외국인투자자구분코드 (String 2) */
    char Small_Bnd_Mktcls_Partc_Yn[1];          /*   27 소액채권장종료매매참여여부 (String 1) */
    char Non_Tax_Cd[1];                         /*   28 비과세여부 (String 1) */
    char Ord_Media_Tp_Cd[1];                    /*   29 주문매체구분코드 (String 1) */
    char Order_Identification_Information[12];  /*   30 주문자식별정보 (String 12) */
    char Mac_Addr[12];                          /*   31 MAC주소 (String 12) */
    char Order_Date[8];                         /*   32 호가일자 (String 8) */
    char Member_Firm_Order_Time[9];             /*   33 회원사주문시각 (String 9) */
    char Member_Use_Area[60];                   /*   34 회원사용영역 (String 60) */
    char Member_Firm_Order_Time_2[9];           /*   35 호가접수시각 (String 9) */
    char Trader_No[5];                          /*   36 거래원번호 (String 5) */
    char Real_Modify_Or_Cancel_Order_Quantity[10];/*   37 실정정취소호가수량 (Long 10) */
    char Automatic_Cancellation_Process_Type_Code[1];/*   38 자동취소처리구분코드 (String 1) */
    char Ord_Rej_Rsn_Cd[4];                     /*   39 호가거부사유코드 (String 4) */
    char Mm_Ord_Tp_No[11];                      /*   40 시장조성자호가구분번호 (Long 11) */
} TTRODP41301_DATA;

#endif  /* _KRX_TTRODP41301_H */
