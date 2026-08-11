#ifndef _KRX_TRDESP01301_H
#define _KRX_TRDESP01301_H

/* TRDESP01301 현물종목정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 2702 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Market_Identification[3];              /*    3 시장ID (String 3) */
    char Transmit_Date[8];                      /*    4 전송일자 (String 8) */
    char Business_Date[8];                      /*    5 영업일자 (String 8) */
    char Securities_Group_Identification[2];    /*    6 증권그룹ID (String 2) */
    char Issue_Code[12];                        /*    7 종목코드 (String 12) */
    char Market_Korean_Name[80];                /*    8 시장한글명 (String 80) */
    char Market_English_Name[80];               /*    9 시장영문명 (String 80) */
    char Securities_Group_Korean_Name[80];      /*   10 증권그룹한글명 (String 80) */
    char Securities_Group_English_Name[80];     /*   11 증권그룹영문명 (String 80) */
    char Issue_Shorten_Code[9];                 /*   12 종목단축코드 (String 9) */
    char Issue_Korean_Name[80];                 /*   13 종목한글명 (String 80) */
    char Issue_Korean_Abbreviation[40];         /*   14 종목한글약명 (String 40) */
    char Issue_English_Name[80];                /*   15 종목영문명 (String 80) */
    char Issue_English_Abbreviation[40];        /*   16 종목영문약명 (String 40) */
    char Listing_Date[8];                       /*   17 상장일자 (String 8) */
    char Delisting_Date[8];                     /*   18 상장폐지일자 (String 8) */
    char Market_Operation_Group_Code[3];        /*   19 장운영상품그룹ID (String 3) */
    char Me_Grp_No[2];                          /*   20 ME그룹번호 (string 2) */
    char Small_And_Medium_Sized_Company_Yes_Or_No[1];/*   21 중소기업여부 (String 1) */
    char Standard_Industry_Code[6];             /*   22 표준산업코드 (String 6) */
    char Governance_Excellence_Yes_Or_No[1];    /*   23 지배구조우량여부 (String 1) */
    char Upper_Limit_Price[11];                 /*   24 상한가 (String 11) */
    char Lower_Limit_Price[11];                 /*   25 하한가 (String 11) */
    char Base_Price[11];                        /*   26 기준가격 (String 11) */
    char Liquid_Provider_Order_Possibility_Yes_Or_No[1];/*   27 LP주문가능여부 (String 1) */
    char Country_Code[3];                       /*   28 국가코드 (String 3) */
    char Listed_Company_Issue_Code[12];         /*   29 상장사종목코드 (String 12) */
    char English_Symbol[40];                    /*   30 영문심볼 (String 40) */
    char Preferred_Stock_Type_Code[1];          /*   31 우선주구분코드 (String 1) */
    char Capital[22];                           /*   32 자본금 (Float 22) */
    char Capital_Correction_Amount[22];         /*   33 자본금보정금액 (Float 22) */
    char Listing_Agency_Company_Maket_Participant_Float[5];/*   34 상장주선사시장참가자번호 (String 5) */
    char Re_Listing_Yes_Or_No[1];               /*   35 재상장여부 (String 1) */
    char People_S_Stock_Yes_Or_No[1];           /*   36 국민주여부 등 (String 1) */
    char Opening_Price_Base_Price_Issue_Yes_Or_No[1];/*   37 시가기준가격종목여부 (String 1) */
    char Base_Price_Change_Issue_Yes_Or_No[1];  /*   38 기준가격변경종목여부 (String 1) */
    char Par_Value_Change_Type_Code[2];         /*   39 액면가변경구분코드 (String 2) */
    char Re_Valuation_Issue_Reason_Code[2];     /*   40 재평가종목사유코드 (String 2) */
    char Off_Type_Code[2];                      /*   41 락구분코드 (String 2) */
    char Capital_Increase_Type_Code[2];         /*   42 증자구분코드 (String 2) */
    char Stock_Assignment_Type_Code[5];         /*   43 주식배정구분코드 (Long 5) */
    char Par_Value[11];                         /*   44 액면가 (String 11) */
    char Stock_Split_Consolidation_Ratio[20];   /*   45 액면분할병합비율 (Float 20) */
    char Listing_Float_Of_Shares[16];           /*   46 상장주식수 (Long 16) */
    char Issue_Price[11];                       /*   47 발행가격 (String 11) */
    char Re_Tg_Cond_Cd[1];                      /*   48 임의종료발동조건코드 (String 1) */
    char Credit_Order_Possibility_Yes_Or_No[1]; /*   49 신용주문가능여부 (String 1) */
    char Reits_Kind_Code[1];                    /*   50 리츠종류코드 (String 1) */
    char Aim_Stock_Issue_Code[12];              /*   51 목적주권종목코드 (String 12) */
    char Exercise_Period_Start_Date[8];         /*   52 행사기간개시일자 (String 8) */
    char Exercise_Period_End_Date[8];           /*   53 행사기간종료일자 (String 8) */
    char Issue_Maket_Participant_Float[5];      /*   54 발행시장참가자번호 (String 5) */
    char Elw_Subsription_Warrant_Exercise_Price[13];/*   55 ELW신주인수권증권행사가격 (Float 13) */
    char Equity_Linked_Warrant_Right_Type_Code[1];/*   56 ELW권리유형코드 (String 1) */
    char Equity_Linked_Warrant_Exercise_Type_Code[1];/*   57 ELW권리행사유형코드 (String 1) */
    char Last_Settlement_Method_Code[1];        /*   58 최종결제방법코드 (String 1) */
    char Last_Trading_Date[8];                  /*   59 최종거래일자 (String 8) */
    char Payment_Date[8];                       /*   60 지급일자 (String 8) */
    char Equity_Linked_Warrant_Underlying_Asset_Base_Price[13];/*   61 ELW기초자산기초가격 (Float 13) */
    char Equity_Linked_Warrant_Exercise_Contents[200];/*   62 ELW권리행사내용
->ELW만기권리행사내용 (String 200) */
    char Equity_Linked_Warrant_Conversion_Ratio[13];/*   63 ELW전환비율 (Float 13) */
    char Equity_Linked_Warrant_Price_Up_Participation_Rate[10];/*   64 ELW가격상승참가율 (Float 10) */
    char Equity_Linked_Warrant_Compensation_Rate[10];/*   65 ELW보상률
->ELW최소지급비율 (Float 10) */
    char Equity_Linked_Warrant_Final_Payment_Value[22];/*   66 ELW확정지급액 (Float 22) */
    char Equity_Linked_Warrant_Payment_Agent_Name[80];/*   67 ELW지급대리인명 (String 80) */
    char Equity_Linked_Warrant_Expiration_Valuation_Price_Method[200];/*   68 ELW만기평가가격방식 (String 200) */
    char Equity_Linked_Warrant_Exotic_Option_Type_Code[1];/*   69 ELW이색옵션구분코드 (String 1) */
    char Investment_Organization_Type_Code[1];  /*   70 투자기구구분코드 (String 1) */
    char Existence_Start_Date[8];               /*   71 존립개시일자 (String 8) */
    char Existence_End_Date[8];                 /*   72 존립종료일자 (String 8) */
    char Market_Warning_Risk_Notice_Yes_Or_No[1];/*   73 시장경보위험예고여부 (String 1) */
    char Market_Warning_Type_Code[2];           /*   74 시장경보구분코드 (String 2) */
    char Limit_Price_Order_Condition_Type_Code[5];/*   75 지정가호가취소조건코드 (Long 5) */
    char Market_Price_Order_Condition_Type_Code[5];/*   76 시장가호가취소조건코드 (Long 5) */
    char Conditional_Limit_Price_Order_Condition_Type_Code[5];/*   77 조건부지정가취소호가조건코드 (Long 5) */
    char Best_Limit_Price_Order_Condition_Type_Code[5];/*   78 최유리지정가취소호가조건코드 (Long 5) */
    char Limit_Price_Of_First_Best_Order_Order_Condition_Type_Code[5];/*   79 최우선지정가취소호가조건코드 (Long 5) */
    char Currency_International_Standardization_Organization_Code[3];/*   80 통화ISO코드 (String 3) */
    char Valuation_Price[11];                   /*   81 평가가격 (String 11) */
    char Lowest_Order_Price[11];                /*   82 최저호가가격 (String 11) */
    char Highest_Order_Price[11];               /*   83 최고호가가격 (String 11) */
    char Opening_Price_Single_Price_Call_Auction_Treasury_Stock_Bid_Upper_Limit_Price[11];/*   84 시가단일가자사주매수상한가 (String 11) */
    char Opening_Price_Single_Price_Call_Auction_Treasury_Stock_Bid_Lower_Limit_Price[11];/*   85 시가단일가자사주매수하한가 (String 11) */
    char Opening_Price_Single_Price_Call_Auction_Treasury_Stock_Ask_Upper_Limit_Price[11];/*   86 시가단일가자사주매도상한가 (String 11) */
    char Opening_Price_Single_Price_Call_Auction_Treasury_Stock_Ask_Lower_Limit_Price[11];/*   87 시가단일가자사주매도하한가 (String 11) */
    char Pre_Off_Hours_Session_Block_Trade_Treasury_Stock_Upper_Limit_Price[11];/*   88 장개시전시간외대량매매자사주상한가 (String 11) */
    char Pre_Off_Hours_Session_Block_Trade_Treasury_Stock_Lower_Limit_Price[11];/*   89 장개시전시간외대량매매자사주하한가 (String 11) */
    char Substitute_Price[11];                  /*   90 대용가격 (String 11) */
    char Lp_Holding_Quantity[15];               /*   91 LP보유수량 (Long 15) */
    char Trading_Halt_Yes_Or_No[1];             /*   92 거래정지여부 (String 1) */
    char Unfaithful_Disclosure_Yes_Or_No[1];    /*   93 불성실공시여부 (String 1) */
    char Administrative_Issue_Yes_Or_No[1];     /*   94 관리종목여부 (String 1) */
    char Arrangement_Trade_Yes_Or_No[1];        /*   95 정리매매여부 (String 1) */
    char Backdoor_Listing_Yes_Or_No[1];         /*   96 우회상장여부 (String 1) */
    char Substitute_Securities_Effect_Stop_Yes_Or_No[1];/*   97 대용증권효력정지여부 (String 1) */
    char Regular_Session_Trading_Quantity_Unit[6];/*   98 정규시장매매수량단위 (Long 6) */
    char Off_Hours_Session_Trading_Quantity_Unit[6];/*   99 시간외매매수량단위 (Long 6) */
    char Issue_Maket_Participant_Korean_Name[80];/*  100 발행시장참가자한글명 (String 80) */
    char Issue_Maket_Participant_English_Name[80];/*  101 발행시장참가자영문명 (String 80) */
    char Closing_Price[11];                     /*  102 종가 (String 11) */
    char Closing_Price_Type_Code[1];            /*  103 종가구분코드 (String 1) */
    char Base_Price_Calculate_Representative_Issue_Code[12];/*  104 기준가격산출대표종목코드 (String 12) */
    char Issue_Market_Participant_Issue_Code[12];/*  105 발행시장참가자종목코드 (String 12) */
    char Shortsell_Possible_Yes_Or_No[1];       /*  106 공매도가능여부 (String 1) */
    char I_F_Price_Sign_Use_Yes_Or_No[1];       /*  107 I/F가격부호사용여부 (String 1) */
    char I_F_Integer_Valid_Digit[3];            /*  108 I/F가격정수유효자리수 (Long 3) */
    char I_F_Fraction_Valid_Digit[3];           /*  109 I/F가격소수유효자리수 (Long 3) */
    char Tracking_Earning_Rate_Multiplier[13];  /*  110 추적수익률배수 (Float 13) */
    char Exchange_Traded_Fund_Reference_Regs_Yn[1];/*  111 Regulation S여부 (String 1) */
    char Special_Purpose_Acquisition_Company[1];/*  112 SPAC여부 (String 1) */
    char Tax_Type_Code[1];                      /*  113 과세유형코드 (String 1) */
    char Substitute_Price_Haircut_Ratio[13];    /*  114 대용가격사정비율 (Float 13) */
    char Equity_Linked_Warrant_Early_End_Occurrence_Base_Price[13];/*  115 ELW조기종료발생기준가격 (Float 13) */
    char Equity_Linked_Warrant_Early_End_Exercise_Contents[200];/*  116 ELW조기종료권리행사내용 (String 200) */
    char Equity_Linked_Warrant_Early_End_Valuation_Price_Method[300];/*  117 ELW조기종료평가가격방식 (String 300) */
    char Section_Type_Code[1];                  /*  118 소속부구분코드 (String 1) */
    char Investment_Caution_Remind_Issue_Yn[1]; /*  119 투자주의환기종목여부 (String 1) */
    char Short_Term_Overheat_Issue_Type_Code[1];/*  120 단기과열종목구분코드 (String 1) */
    char Etf_Replication_Method[1];             /*  121 ETF복제방법구분코드 ★변경 (String 1) */
    char Mult_Acntcls_Mmdd_Contn[100];          /*  122 다중결산월일내용 (String 100) */
    char Rght_Delist_Dd[8];                     /*  123 신주인수권증서상장폐지일자 (String 8) */
    char Uly_Mkt_Calnd_Id[10];                  /*  124 기초자산시장달력ID (String 10) */
    char Distr_Tp_Cd[2];                        /*  125 분배금형태코드 (String 2) */
    char Exp_Dd[8];                             /*  126 만기일자 (String 8) */
    char Exp_Redmpt_Prc_Decsn_Strt_Dd[8];       /*  127 만기상환가격결정시작일자 (String 8) */
    char Exp_Redmpt_Prc_Decsn_End_Dd[8];        /*  128 만기상환가격결정종료일자 (String 8) */
    char Prc_Lmt_Rt[13];                        /*  129 가격제한율 (Float 13) */
    char Etp_Prod_Tp_Cd[1];                     /*  130 ETP상품구분코드 (string 1) */
    char Idx_Calc_Inst_Cd[2];                   /*  131 지수산출기관코드 (string 2) */
    char Idx_Mkt_Clss_Id[6];                    /*  132 지수시장분류ID (string 6) */
    char Idx_Seq[3];                            /*  133 지수일련번호 (string 3) */
    char Trace_Idx_Lvrg_Invrs_Tp_Cd[2];         /*  134 추적지수레버리지인버스구분코드 (string 2) */
    char Ref_Idx_Lvrg_Invrs_Tp_Cd[2];           /*  135 참고지수레버리지인버스구분코드 (string 2) */
    char Idx_Asst_Clss_Id1[6];                  /*  136 지수자산분류ID1 (string 6) */
    char Idx_Asst_Clss_Id2[6];                  /*  137 지수자산분류ID2 (string 6) */
    char Mm_Posbl_Yn[1];                        /*  138 시장조성가능여부 (string 1) */
    char Lwliqu_Yn[1];                          /*  139 저유동성여부 (string 1) */
    char Loss_Lmt_Etn_Earng_Struct_Cd[2];       /*  140 손실제한ETN수익구조코드 (string 2) */
    char Etn_Max_Redmpt_Prc[11];                /*  141 ETN최대상환가격 (float 11) */
    char Etn_Min_Redmpt_Prc[11];                /*  142 ETN최소상환가격 (float 11) */
    char Etn_Eary_Redmpt_Posbl_Yn[1];           /*  143 ETN조기상환가능여부 (string 1) */
    char Etn_Eary_Redmpt_Cycle_Cd[2];           /*  144 ETN조기상환주기코드 (string 2) */
    char Valu_Prc_Calc_Inst_Cd1[2];             /*  145 평가가격산출기관코드1 (string 2) */
    char Valu_Prc_Calc_Inst_Cd2[2];             /*  146 평가가격산출기관코드2 (string 2) */
    char If_Uplmt_Qty[16];                      /*  148 I/F상한수량 (Long 16) */
    char Invstwarn_Isu_Yn[1];                   /*  149 투자유의종목여부 (string 1) */
    char List_Shrs_Lack_Isu_Yn[1];              /*  150 상장주식수부족종목여부 (string 1) */
    char Spac_Laps_Merge_Yn[1];                 /*  151 SPAC소멸합병여부 (string 1) */
    char Seg_Tp_Cd[1];                          /*  152 세그먼트구분코드 (String 1) */
    char Prc_Chg_Tp_Cd[1];                      /*  153 가격변경유형코드 (String 1) */
    char List_Applnt_Isur_Cd[5];                /*  154 상장신청인발행기관코드 (String 5) */
    char List_Applnt_Isu_Cd[12];                /*  155 상장신청인종목코드 (String 12) */
    char Trst_Inst_Isur_Cd[5];                  /*  156 신탁기관발행기관코드 (String 5) */
    char Isu_Idx[7];                            /*  157 종목인덱스 (Long 7) */
    char Prc_Unit_Rule_Id[10];                  /*  158 가격단위규칙ID (String 10) */
    char Stoplmtprc_Ord_Cancl_Cond_Cd[5];       /*  159 스톱지정가호가취소조건코드 (Long 5) */
    char Midprc_Ord_Cancl_Cond_Cd[5];           /*  160 중간가호가취소조건코드 (Long 5) */
} TRDESP01301_DATA;

#endif  /* _KRX_TRDESP01301_H */
