#ifndef _KRX_TTSSDP90007_H
#define _KRX_TTSSDP90007_H

/* TTSSDP90007 협의(협의대량, EFP, FLEX)비상주문체결재송신 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* (변형: 레이아웃 공유 그룹 TTSSDP20007) */
/* DATA 길이 합계 = 209 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Board_Id[2];                           /*    4 보드ID (String 2) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Branch_Number[5];                      /*    6 지점번호 (String 5) */
    char Order_Identification[10];              /*    7 주문ID (String 10) */
    char Issue_Code[12];                        /*    8 종목코드 (String 12) */
    char Trading_Number[11];                    /*    9 체결번호 (Long 11) */
    char Trading_Price[11];                     /*   10 체결가격 (Float 11) */
    char Trading_Volumn[10];                    /*   11 체결수량 (Long 10) */
    char Sess_Id[2];                            /*   12 세션ID (String 2) */
    char Trading_Date[8];                       /*   13 체결일자 (String 8) */
    char Trading_Time[9];                       /*   14 체결시각 (String 9) */
    char Ask_Bid_Type_Code[1];                  /*   15 매도매수구분코드 (String 1) */
    char Account_Number[12];                    /*   16 계좌번호 (String 12) */
    char Negotiation_Number[6];                 /*   17 협상번호 (Long 6) */
    char Negotiation_Detail_Number[6];          /*   18 협상상세번호 (Long 6) */
    char Negotiator_Identification[10];         /*   19 협상자ID (String 10) */
    char Member_Use_Area[60];                   /*   20 회원사용영역 (String 60) */
    char Trust_Company_Number[5];               /*   21 위탁사번호 (String 5) */
} TTSSDP90007_DATA;

#endif  /* _KRX_TTSSDP90007_H */
