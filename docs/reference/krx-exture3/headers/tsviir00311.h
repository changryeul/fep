#ifndef _KRX_TSVIIR00311_H
#define _KRX_TSVIIR00311_H

/* TSVIIR00311 파생심리 계좌기본정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1541 (interface-list.csv '길이(헤더제외)'와 대조) */
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
    char Country_Code[3];                       /*   16 국가코드 (String 3) */
    char Investor_Type_Code[4];                 /*   17 투자자구분코드 (String 4) */
    char Foreign_Investor_Type_Code[2];         /*   18 외국인투자자구분코드 (String 2) */
    char Foreign_Unique_Number[20];             /*   19 외국인고유번호 (String 20) */
    char Foreign_Investor_Type[1];              /*   20 외국인투자구분 (String 1) */
    char Trust_Principal_Type_Code[2];          /*   21 위탁자기구분코드 (String 2) */
    char Account_Manager_Number[10];            /*   22 계좌관리자번호 (String 10) */
    char Account_Opening_Date[8];               /*   23 계좌개설일자 (String 8) */
    char Account_Status_Code[1];                /*   24 계좌상태코드 (String 1) */
    char Account_Closing_Date[8];               /*   25 계좌폐쇄일자 (String 8) */
    char Closing_Revival_Date[8];               /*   26 폐쇄부활일자 (String 8) */
    char Last_Trading_Date[8];                  /*   27 최종거래일자 (String 8) */
    char Trading_Tax_Type_Code[1];              /*   28 거래과세구분코드 (String 1) */
    char Dividend_Tax_Type_Code[1];             /*   29 배당과세구분코드 (String 1) */
    char Withdrawal_Delivery_Stop_Type_Code[1]; /*   30 출금출고정지구분코드 (String 1) */
    char Real_Name_Confirmation_Yn[1];          /*   31 실명확인여부 (String 1) */
    char Real_Name_Confirmation_Date[8];        /*   32 실명확인일자 (String 8) */
    char Total_Deposit[22];                     /*   33 총예수금 (Float 22) */
    char Credit_Account_Type_Code[1];           /*   34 신용계좌구분코드 (String 1) */
    char Credit_Opening_Date[8];                /*   35 신용개설일자 (String 8) */
    char Credit_Closing_Date[8];                /*   36 신용해지일자 (String 8) */
    char Margin_Collection_Yn[1];               /*   37 증거금징수여부 (String 1) */
    char Card_Issue_Type_Code[1];               /*   38 카드발급구분 (String 1) */
    char Card_Issue_Count[6];                   /*   39 카드발급횟수 (String 6) */
    char Bank_Name[80];                         /*   40 은행명 (String 80) */
    char Bank_Account_Number[20];               /*   41 은행계좌번호 (String 20) */
    char Take_Control_Date[8];                  /*   42 수관일자 (String 8) */
    char Take_Control_Account_Number[12];       /*   43 수관계좌번호 (String 12) */
    char Take_Control_Branch_Name[80];          /*   44 수관지점명 (String 80) */
    char Transfer_Date[8];                      /*   45 이관일자 (String 8) */
    char Transfer_Of_Control_Date[12];          /*   46 이관계좌번호 (String 12) */
    char Transfer_Of_Control_Account_Number[80];/*   47 이관지점명 (String 80) */
    char Transfer_Of_Control_Branch_Name[80];   /*   48 직장명 (String 80) */
    char Home_Telephone_Number_Number[20];      /*   49 자택전화번호 (String 20) */
    char Cellular_Phone_Number[20];             /*   50 휴대폰번호 (String 20) */
    char Company_Telephone_Number_Number[20];   /*   51 직장전화번호 (String 20) */
    char Home_Zip_Code[6];                      /*   52 자택우편번호 (String 6) */
    char Home_Address[200];                     /*   53 자택주소 (String 200) */
    char Book_Notice_Location_Type_Code[1];     /*   54 원장통보지구분코드 (String 1) */
    char Balance_Notice_Location_Type_Code[1];  /*   55 잔고통보지구분코드 (String 1) */
    char Domestic_And_Foreign_National_Type_Code[1];/*   56 내외국인구분코드 (String 1) */
    char Passport_Number[13];                   /*   57 여권번호 (String 13) */
    char Not_Redeemed_Amount[22];               /*   58 미상환금액 (Float 22) */
    char Credit_Interest_Unpayment_Amount[22];  /*   59 신용이자미납금액 (Float 22) */
    char Not_Redeemed_Loan_Amount[22];          /*   60 미상환융자금액 (Float 22) */
    char Miscellaneous_Loan_Amount[22];         /*   61 기타대여금액 (Float 22) */
    char Miscellaneous_Check_Amount[22];        /*   62 기타수표금액 (Float 22) */
    char Margin_Transaction_Establishment_Requirement_Amount[22];/*   63 신용거래설정보증금 (Float 22) */
    char Substitute_Amount_Aggregation_Amount[22];/*   64 대용금합계금액 (Float 22) */
    char Return_Count[16];                      /*   65 반송회수 (Long 16) */
    char Original_Account_Number[12];           /*   66 실제계좌번호 (String 12) */
    char Email[100];                            /*   67 이메일 (String 100) */
    char Assumption_Account_Bank_Name[20];      /*   68 가상계좌은행명 (String 20) */
    char Assumption_Account_Number[20];         /*   69 가상계좌번호 (String 20) */
    char Company_Address[200];                  /*   70 직장주소 (String 200) */
    char Wireless_Terminal_Opening_Date[8];     /*   71 무선단말개설일자 (String 8) */
    char Home_Trading_System_Opening_Date[8];   /*   72 HTS개설일자 (String 8) */
} TSVIIR00311_DATA;

#endif  /* _KRX_TSVIIR00311_H */
