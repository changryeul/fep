#ifndef _KRX_TSVIIR00312_H
#define _KRX_TSVIIR00312_H

/* TSVIIR00312 파생심리 고객계좌부 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1023 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Time[8];                               /*    6 시각 (String 8) */
    char Request_Date[8];                       /*    7 요청일자 (String 8) */
    char Submit_Date[8];                        /*    8 제출일자 (String 8) */
    char Investigation_Object_Start_Date[8];    /*    9 심리대상개시일자 (String 8) */
    char Investigation_Object_End_Date[8];      /*   10 심리대상종료일자 (String 8) */
    char Branch_Number[5];                      /*   11 지점번호 (String 5) */
    char Account_Number[12];                    /*   12 계좌번호 (String 12) */
    char Account_Name[80];                      /*   13 계좌명 (String 80) */
    char Stock_Link_Account_Number[12];         /*   14 주식연계계좌번호 (String 12) */
    char Resident_Registration_Number[20];      /*   15 주민등록번호 (String 20) */
    char Trading_Date[8];                       /*   16 거래일자 (String 8) */
    char Book_Trading_Type_Code[5];             /*   17 원장거래구분코드 (String 5) */
    char Transaction_Number[9];                 /*   18 거래번호 (Long 9) */
    char Customer_Account_Trading_Type_Code[1]; /*   19 고객계좌거래구분코드 (String 1) */
    char Trading_Issue_Code[12];                /*   20 거래종목코드 (String 12) */
    char Trading_Issue_Name[80];                /*   21 거래종목명 (String 80) */
    char Trading_Volumn[15];                    /*   22 체결수량 (Long 15) */
    char Trading_Unit_Price[18];                /*   23 거래단가 (Float 18) */
    char Trading_Value[18];                     /*   24 거래대금 (Float 18) */
    char Fee[22];                               /*   25 수수료 (Float 22) */
    char Tax[22];                               /*   26 제세금 (Float 22) */
    char Deposit_Previous_Day_Balance[22];      /*   27 예수금거래잔액 (String 22) */
    char Deposit_Today_Balance[22];             /*   28 예수금거래후잔액 (String 22) */
    char Credit_Interest_Shortselling_Securities_Utilization_Fee[22];/*   29 신용이자및대주이용료 (Float 22) */
    char Credit_Redemption_Payoff[22];          /*   30 신용상환차금 (Float 22) */
    char Credit_Amount[22];                     /*   31 신용금액 (Float 22) */
    char Normal_Cancellation_Type_Code[1];      /*   32 정상취소구분코드 (String 1) */
    char Margin_Transaction_Type_Code[2];       /*   33 신용거래구분코드 (String 2) */
    char Cash_Receivable_Amount_Occurrence_Amount[22];/*   34 현금미수금발생금액 (Float 22) */
    char Receivable_Amount_Previous_Day_Balance[22];/*   35 미수금거래전잔액 (Float 22) */
    char Receivable_Amount_Today_Balance[22];   /*   36 미수금거래후잔액 (Float 22) */
    char Receivable_Repayment_Type_Code[1];     /*   37 미수변제구분코드 (String 1) */
    char Process_Branch_Name[80];               /*   38 처리지점명 (String 80) */
    char Credit_Loan_Date[8];                   /*   39 신용대출일자 (String 8) */
    char Miscellaneous_Check_Deposit_Value[22]; /*   40 기타수표입금액 (Float 22) */
    char Settlement_Amount[22];                 /*   41 정산금액 (Float 22) */
    char Alternative_Counterpart_Account_Number[16];/*   42 대체상대계좌번호 (String 16) */
    char Cash_Receivable_Tender[22];            /*   43 현금미수변제금 (Float 22) */
    char Cash_Receivable_Overdue_Penalty[22];   /*   44 현금미수연체료 (Float 22) */
    char Credit_Interest_Unpayment_Tender[22];  /*   45 신용이자미납변제금 (Float 22) */
    char Credit_Interest_Unpayment_Overdue_Penalty[22];/*   46 신용이자미납연체료 (Float 22) */
    char Miscellaneous_Loan_Money_Tender[22];   /*   47 기타대여금변제금 (Float 22) */
    char Miscellaneous_Loan_Money_Overdue_Penalty[22];/*   48 기타대여금연체료 (Float 22) */
    char Check_Amount[22];                      /*   49 수표금액 (Float 22) */
    char Trading_Time[6];                       /*   50 거래시각 (String 6) */
    char Trading_Date_2[8];                     /*   51 매매체결일 (String 8) */
    char Trading_Type_Detail[100];              /*   52 거래구분상세내용 (String 100) */
    char Trading_Type_Detail_Code[10];          /*   53 거래구분상세코드 (String 10) */
} TSVIIR00312_DATA;

#endif  /* _KRX_TSVIIR00312_H */
