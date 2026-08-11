#ifndef _KRX_TSVIIR00204_H
#define _KRX_TSVIIR00204_H

/* TSVIIR00204 고객계좌부 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1154 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Second_Data_Request_Date[8];           /*    5 2차자료요청일자 (String 8) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Branch_Number[5];                      /*    7 지점번호 (String 5) */
    char Account_Number[12];                    /*    8 계좌번호 (String 12) */
    char Trading_Date[8];                       /*    9 거래일자 (String 8) */
    char Transaction_Number[9];                 /*   10 거래번호 (Long 9) */
    char Customer_Account_Trading_Type_Code[1]; /*   11 고객계좌거래구분코드 (String 1) */
    char Book_Trading_Type_Code[5];             /*   12 원장거래구분코드 (String 5) */
    char Account_Name[80];                      /*   13 계좌명 (String 80) */
    char Resident_Registration_Number[20];      /*   14 주민등록번호 (String 20) */
    char Trading_Issue_Code[12];                /*   15 거래종목코드 (String 12) */
    char Trading_Issue_Name[80];                /*   16 거래종목명 (String 80) */
    char Trading_Volumn[15];                    /*   17 체결수량 ★변경 (Long 15) */
    char Trading_Unit_Price[18];                /*   18 거래단가 (Long 18) */
    char Trading_Value[22];                     /*   19 거래대금 (Float 22) */
    char Fee[22];                               /*   20 수수료 (Float 22) */
    char Special_Tax_For_Rural_Development[22]; /*   21 농특세 (Float 22) */
    char Deposit_Previous_Day_Balance_Sign[1];  /*   22 예수금전일잔액부호 (String 1) */
    char Deposit_Previous_Day_Balance[22];      /*   23 예수금전일잔액 (Float 22) */
    char Deposit_Today_Balance_Sign[1];         /*   24 예수금금일잔액부호 (String 1) */
    char Deposit_Today_Balance[22];             /*   25 예수금금일잔액 (Float 22) */
    char Securities_Previous_Day_Remaining_Quantity_Sign[1];/*   26 유가증권전일잔량부호 (String 1) */
    char Securities_Previous_Day_Remaining_Quantity[18];/*   27 유가증권전일잔량 (Long 18) */
    char Securities_Today_Remaining_Quantity_Sign[1];/*   28 유가증권금일잔량부호 (String 1) */
    char Securities_Today_Remaining_Quantity[18];/*   29 유가증권금일잔량 (Long 18) */
    char Transaction_Tax[22];                   /*   30 거래세 (Float 22) */
    char Credit_Interest_Shortselling_Securities_Utilization_Fee[22];/*   31 신용이자및대주이용료 (Float 22) */
    char Credit_Redemption_Payoff_Sign[1];      /*   32 신용상환차금부호 (String 1) */
    char Credit_Redemption_Payoff[22];          /*   33 신용상환차금 (Float 22) */
    char Credit_Amount[22];                     /*   34 신용금액 (Float 22) */
    char Normal_Cancellation_Type_Code[1];      /*   35 정상취소구분코드 (String 1) */
    char Income_Tax[22];                        /*   36 소득세 (Float 22) */
    char Inhabitant_Tax[22];                    /*   37 주민세 (Float 22) */
    char Dividend_Odd_Lot_Sales_Amount[22];     /*   38 배당단주매각금액 (Float 22) */
    char Margin_Transaction_Type_Code[2];       /*   39 신용거래구분코드 (String 2) */
    char Cash_Receivable_Amount_Occurrence_Amount[22];/*   40 현금미수금발생금액 (Float 22) */
    char Receivable_Amount_Previous_Day_Balance[22];/*   41 미수금전일잔액 (Float 22) */
    char Receivable_Amount_Today_Balance[22];   /*   42 미수금금일잔액 (Float 22) */
    char Securities_Receivable_Occurrence_Quantity[15];/*   43 유가증권미수발생수량 (Long 15) */
    char Receivable_Repayment_Type_Code[1];     /*   44 미수변제구분코드 (String 1) */
    char Process_Branch_Name[80];               /*   45 처리지점명 (String 80) */
    char Credit_Loan_Date[8];                   /*   46 신용대출일자 (String 8) */
    char Miscellaneous_Check_Deposit_Value[22]; /*   47 기타수표입금액 (Float 22) */
    char Settlement_Amount[22];                 /*   48 정산금액 (Float 22) */
    char Field[16];                             /*   49 대체상대은행계좌번호 (String 16) */
    char Securities_Receivable_Repayment_Quantity[15];/*   50 유가증권미수변제수량 (Long 15) */
    char Cash_Receivable_Tender[22];            /*   51 현금미수변제금 (Float 22) */
    char Cash_Receivable_Overdue_Penalty[22];   /*   52 현금미수연체료 (Float 22) */
    char Credit_Interest_Unpayment_Tender[22];  /*   53 신용이자미납변제금 (Float 22) */
    char Credit_Interest_Unpayment_Overdue_Penalty[22];/*   54 신용이자미납연체료 (Float 22) */
    char Miscellaneous_Loan_Money_Tender[22];   /*   55 기타대여금변제금 (Float 22) */
    char Miscellaneous_Loan_Money_Overdue_Penalty[22];/*   56 기타대여금연체료 (Float 22) */
    char Check_Amount[22];                      /*   57 수표금액 (Float 22) */
    char Investor_Representative_Account_Number[12];/*   58 투자자대표계좌번호 (String 12) */
    char Trading_Time[6];                       /*   59 거래시각 (String 6) */
    char Order_Date[8];                         /*   60 매매체결일 (String 8) */
    char Transaction_Type_Detail_Contents[100]; /*   61 거래구분상세내용 (String 100) */
    char Transaction_Type_Detail_Detail_Code[10];/*   62 거래구분상세코드 (String 10) */
} TSVIIR00204_DATA;

#endif  /* _KRX_TSVIIR00204_H */
