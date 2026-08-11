#ifndef _KRX_TTRTDP21303_H
#define _KRX_TTRTDP21303_H

/* TTRTDP21303 회원체결결과 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* (변형: 레이아웃 공유 그룹 TTRTDP21301) */
/* DATA 길이 합계 = 233 (interface-list.csv '길이(헤더제외)'와 대조) */
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
    char Trading_Number[11];                    /*   10 체결번호 (Long 11) */
    char Trading_Price[11];                     /*   11 체결가격 (Float 11) */
    char Trading_Volumn[10];                    /*   12 체결수량 (Long 10) */
    char Sess_Id[2];                            /*   13 세션ID (String 2) */
    char Trading_Date[8];                       /*   14 체결일자 (String 8) */
    char Trading_Time[9];                       /*   15 체결시각 (String 9) */
    char The_Nearby_Month_Trading_Price[11];    /*   16 근월물체결가격 (Float 11) */
    char The_Future_Month_Trading_Price[11];    /*   17 원월물체결가격 (Float 11) */
    char Ask_Bid_Type_Code[1];                  /*   18 매도매수구분코드 (String 1) */
    char Account_Number[12];                    /*   19 계좌번호 (String 12) */
    char Mm_Ord_Tp_Cd[1];                       /*   20 시장조성자호가구분코드 ★변경 (String 1) */
    char Trust_Company_Number[5];               /*   21 위탁사번호 (String 5) */
    char Substitute_Stock_Certificate_Account_Number[12];/*   22 대용주권계좌번호 (String 12) */
    char Member_Use_Area[60];                   /*   23 회원사용영역 (String 60) */
    char Last_Ask_Bid_Type_Code[1];             /*   24 최종매도매수구분코드 (String 1) */
} TTRTDP21303_DATA;

#endif  /* _KRX_TTRTDP21303_H */
