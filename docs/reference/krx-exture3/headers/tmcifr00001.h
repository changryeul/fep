#ifndef _KRX_TMCIFR00001_H
#define _KRX_TMCIFR00001_H

/* TMCIFR00001 신용정보(공통) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1092 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Trading_Date[8];                       /*    3 거래일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Issue_Code[12];                        /*    6 종목코드 (String 12) */
    char New_Principal_Loan_Number_Of_Shares[16];/*    7 신규자기융자주식수 (Long 16) */
    char New_Principal_Loan_Amount[22];         /*    8 신규자기융자금액 (Float 22) */
    char New_Principal_Stockloan_Number_Of_Shares[16];/*    9 신규자기대주주식수 (Long 16) */
    char New_Principal_Stockloan_Amount[22];    /*   10 신규자기대주금액 (Float 22) */
    char New_Flow_Loan_Number_Of_Shares[16];    /*   11 신규유통융자주식수 (Long 16) */
    char New_Flow_Loan_Amount[22];              /*   12 신규유통융자금액 (Float 22) */
    char New_Flow_Stockloan_Number_Of_Shares[16];/*   13 신규유통대주주식수 (Long 16) */
    char New_Flow_Stockloan_Amount[22];         /*   14 신규유통대주금액 (Float 22) */
    char Principal_Ask_Redemption_Number_Of_Shares[16];/*   15 자기융자매도상환주식수 (Long 16) */
    char Principal_Ask_Redemption_Amount[22];   /*   16 자기융자매도상환금액 (Float 22) */
    char Principal_Loan_Cash_Redemption_Number_Of_Shares[16];/*   17 자기융자현금상환주식수 (Long 16) */
    char Principal_Loan_Cash_Redemption_Amount[22];/*   18 자기융자현금상환금액 (Float 22) */
    char Flow_Loan_Ask_Redemption_Number_Of_Shares[16];/*   19 유통융자매도상환주식수 (Long 16) */
    char Flow_Loan_Ask_Redemption_Amount[22];   /*   20 유통융자매도상환금액 (Float 22) */
    char Flow_Loan_Cash_Redemption_Number_Of_Shares[16];/*   21 유통융자현금상환주식수 (Long 16) */
    char Flow_Loan_Cash_Redemption_Amount[22];  /*   22 유통융자현금상환금액 (Float 22) */
    char Principal_Stockloan_Bid_Redemption_Number_Of_Shares[16];/*   23 자기대주매수상환주식수 (Long 16) */
    char Principal_Stockloan_Bid_Spot_Redemption_Amount[22];/*   24 자기대주매수현금상환금액 (Float 22) */
    char Principal_Stockloan_Spot_Redemption_Number_Of_Shares[16];/*   25 자기대주현물상환주식수 (LONG 16) */
    char Principal_Stockloan_Spot_Redemption_Amount[22];/*   26 자기대주현물상환금액 (Float 22) */
    char Flow_Loan_Bid_Redemption_Number_Of_Shares[16];/*   27 유통대주매수상환주식수 (Long 16) */
    char Flow_Loan_Bid_Redemption_Amount[22];   /*   28 유통대주매수상환금액 (Float 22) */
    char Flow_Loan_Spot_Redemption_Number_Of_Shares[16];/*   29 유통대주현물상환주식수 (Long 16) */
    char Flow_Loan_Spot_Redemption_Amount[22];  /*   30 유통대주현물상환금액 (Float 22) */
    char Previous_Day_Total_Principal_Loan_Number_Of_Shares[16];/*   31 전일총자기융자주식수 (Long 16) */
    char Previous_Day_Total_Principal_Loan_Amount[22];/*   32 전일총자기융자금액 (Float 22) */
    char Previous_Day_Total_Principal_Stockloan_Number_Of_Shares[16];/*   33 전일총자기대주주식수 (Long 16) */
    char Previous_Day_Total_Principal_Stockloan_Amount[22];/*   34 전일총자기대주금액 (Float 22) */
    char Previous_Day_Total_Flow_Loan_Number_Of_Shares[16];/*   35 전일총유통융자주식수 (Long 16) */
    char Previous_Day_Total_Flow_Loan_Amount[22];/*   36 전일총유통융자금액 (Float 22) */
    char Previous_Day_Total_Flow_Stockloan_Number_Of_Shares[16];/*   37 전일총유통대주주식수 (Long 16) */
    char Previous_Day_Total_Flow_Stockloan_Amount[22];/*   38 전일총유통대주금액 (Float 22) */
    char That_Day_Total_Principal_Loan_Number_Of_Shares[16];/*   39 당일총자기융자주식수 (Long 16) */
    char That_Day_Total_Principal_Loan_Amount[22];/*   40 당일총자기융자금액 (Float 22) */
    char That_Day_Total_Principal_Stockloan_Number_Of_Shares[16];/*   41 당일총자기대주주식수 (Long 16) */
    char That_Day_Total_Principal_Stockloan_Amount[22];/*   42 당일총자기대주금액 (Float 22) */
    char That_Day_Total_Flow_Loan_Number_Of_Shares[16];/*   43 당일총유통융자주식수 (Long 16) */
    char That_Day_Total_Flow_Loan_Amount[22];   /*   44 당일총유통융자금액 (Float 22) */
    char That_Day_Total_Flow_Stockloan_Number_Of_Shares[16];/*   45 당일총유통대주주식수 (Long 16) */
    char That_Day_Total_Flow_Stockloan_Amount[22];/*   46 당일총유통대주금액 (Float 22) */
    char Cfd_New_Bid_Number_Of_Shares[16];      /*   47 CFD 신규매수주식수 (Long 16) */
    char Cfd_New_Bid_Amount[22];                /*   48 CFD 신규매수금액 (Float 22) */
    char Cfd_New_Ask_Number_Of_Shares[16];      /*   49 CFD 신규매도주식수 (Long 16) */
    char Cfd_New_Ask_Amount[22];                /*   50 CFD 신규매도금액 (Float 22) */
    char Cfd_Clearing_Settlement_Bid_Number_Of_Shares[16];/*   51 CFD 청산매수주식수 (Long 16) */
    char Cfd_Clearing_Settlement_Bid_Amount[22];/*   52 CFD 청산매수금액 (Float 22) */
    char Cfd_Clearing_Settlement_Ask_Number_Of_Shares_Amount[16];/*   53 CFD 청산매도주식수 (Long 16) */
    char Cfd_Clearing_Settlement_Bid_Amount_2[22];/*   54 CFD 청산매도금액 (Float 22) */
    char Cfd_Bid_Balance_Number_Of_Shares[16];  /*   55 CFD 매수잔고주식수 (Long 16) */
    char Cfd_Bid_Balance_Amount[22];            /*   56 CFD 매수잔고금액 (Float 22) */
    char Cfd_Ask_Balance_Number_Of_Shares[16];  /*   57 CFD 매도잔고주식수 (Long 16) */
    char Cfd_Ask_Balance_Amount[22];            /*   58 CFD 매도잔고금액 (Float 22) */
    char Cfd_Count_Of_Account[16];              /*   59 CFD 계좌수 (Long 16) */
    char Cfd_Covering_Number_Of_Shares[16];     /*   60 CFD 반대매매주식수 (Long 16) */
    char Cfd_Covering_Amount[22];               /*   61 CFD 반대매매금액 (Float 22) */
} TMCIFR00001_DATA;

#endif  /* _KRX_TMCIFR00001_H */
