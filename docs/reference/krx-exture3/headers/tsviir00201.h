#ifndef _KRX_TSVIIR00201_H
#define _KRX_TSVIIR00201_H

/* TSVIIR00201 세부심리자료응답 계좌기본정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1399 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Second_Data_Request_Date[8];           /*    5 2차자료요청일자 (String 8) */
    char Base_Date[8];                          /*    6 기준일자 (String 8) */
    char Member_Number[5];                      /*    7 회원번호 (String 5) */
    char Branch_Number[5];                      /*    8 지점번호 (String 5) */
    char Account_Number[12];                    /*    9 계좌번호 (String 12) */
    char Account_Name[80];                      /*   10 계좌명 (String 80) */
    char Resident_Registration_Number[20];      /*   11 주민등록번호 (String 20) */
    char Investor_Type_Code[4];                 /*   12 투자자구분코드 (String 4) */
    char Foreign_Investor_Type_Code[2];         /*   13 외국인투자자구분코드 (String 2) */
    char Foreigner_Unique_Number[20];           /*   14 외국인고유번호 (String 20) */
    char Foreigner_Investment_Type_Code[1];     /*   15 외국인투자구분코드 (String 1) */
    char Account_Manager_Number[10];            /*   16 계좌관리자번호 (String 10) */
    char Account_Opening_Date[8];               /*   17 계좌개설일자 (String 8) */
    char Account_Status_Code[1];                /*   18 계좌상태코드 (String 1) */
    char Closing_Date[8];                       /*   19 폐쇄일자 (String 8) */
    char Closing_Revival_Date[8];               /*   20 폐쇄부활일자 (String 8) */
    char Last_Trading_Date[8];                  /*   21 최종거래일자 (String 8) */
    char Trading_Tax_Type_Code[1];              /*   22 거래과세구분코드 (String 1) */
    char Dividend_Tax_Type_Code[1];             /*   23 배당과세구분코드 (String 1) */
    char Withdrawal_Delivery_Of_Goods_From_A_Warehouse_Stop_Type_Code[1];/*   24 출금출고정지구분코드 (String 1) */
    char Real_Name_Confirmation_Yes_Or_No[1];   /*   25 실명확인여부 (String 1) */
    char Real_Name_Confirmation_Date[8];        /*   26 실명확인일자 (String 8) */
    char Total_Deposit_Sign[1];                 /*   27 총예수금부호 (String 1) */
    char Total_Deposit[22];                     /*   28 총예수금 (Float 22) */
    char Credit_Account_Type_Code[1];           /*   29 신용계좌구분코드 (String 1) */
    char Credit_Opening_Date[8];                /*   30 신용개설일자 (String 8) */
    char Credit_Termination_Date[8];            /*   31 신용해지일자 (String 8) */
    char Futures_Link_Account_Number[12];       /*   32 선물연계계좌번호 (String 12) */
    char Margin_Collection_Levy_Yes_Or_No[1];   /*   33 증거금징수여부 (String 1) */
    char Card_Issue_Type_Code[1];               /*   34 카드발급구분코드 (String 1) */
    char Card_Issue_Count[16];                  /*   35 카드발급횟수 (Long 16) */
    char Bank_Name[80];                         /*   36 은행명 (String 80) */
    char Bank_Account_Number[20];               /*   37 은행계좌번호 (String 20) */
    char Take_Control_Date[8];                  /*   38 수관일자 (String 8) */
    char Take_Control_Account_Number[12];       /*   39 수관계좌번호 (String 12) */
    char Take_Control_Branch_Name[80];          /*   40 수관지점명 (String 80) */
    char Transfer_Of_Control_Date[8];           /*   41 이관일자 (String 8) */
    char Transfer_Of_Control_Account_Number[12];/*   42 이관계좌번호 (String 12) */
    char Transfer_Of_Control_Branch_Name[80];   /*   43 이관지점명 (String 80) */
    char Company_Name[80];                      /*   44 직장명 (String 80) */
    char Home_Telephone_Number_Number[20];      /*   45 자택전화번호 (String 20) */
    char Cellular_Phone_Number[20];             /*   46 휴대폰번호 (String 20) */
    char Company_Telephone_Number_Number[20];   /*   47 직장전화번호 (String 20) */
    char Home_Zip_Code[6];                      /*   48 자택우편번호 (String 6) */
    char Home_Address[200];                     /*   49 자택주소 (String 200) */
    char Book_Notice_Location_Type_Code[1];     /*   50 원장통보지구분코드 (String 1) */
    char Balance_Notice_Location_Type_Code[1];  /*   51 잔고통보지구분코드 (String 1) */
    char Domestic_And_Foreign_National_Type_Code[1];/*   52 내외국인구분코드 (String 1) */
    char Passport_Number[13];                   /*   53 여권번호 (String 13) */
    char Not_Redeemed_Amount[22];               /*   54 미상환금액 (Float 22) */
    char Credit_Interest_Unpayment_Amount[22];  /*   55 신용이자미납금액 (Float 22) */
    char Not_Redeemed_Loan_Amount[22];          /*   56 미상환융자금액 (Float 22) */
    char Miscellaneous_Loan_Amount[22];         /*   57 기타대여금액 (Float 22) */
    char Miscellaneous_Check_Amount[22];        /*   58 기타수표금액 (Float 22) */
    char Margin_Transaction_Establishment_Requirement_Amount[22];/*   59 신용거래설정보증금 (Float 22) */
    char Substitute_Amount_Aggregation_Amount[22];/*   60 대용금합계금액 (Float 22) */
    char Return_Count[16];                      /*   61 반송횟수 (Long 16) */
    char Investor_Representative_Account_Number[12];/*   62 투자자대표계좌번호 (String 12) */
    char Real_Account_Number[12];               /*   63 실제계좌번호 (String 12) */
    char Email[80];                             /*   64 이메일 (String 80) */
    char Asumdtec_Account_Bank_Name[20];        /*   65 가상계좌은행명 (String 20) */
    char Asumdtec_Account_Number[20];           /*   66 가상계좌번호 (String 20) */
    char Company_Address[100];                  /*   67 직장주소 (String 100) */
} TSVIIR00201_DATA;

#endif  /* _KRX_TSVIIR00201_H */
