#ifndef _KRX_TCSMIR33001_H
#define _KRX_TCSMIR33001_H

/* TCSMIR33001 옴니버스계좌신고및접수통보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 139 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Mbr_No[5];                             /*    4 회원번호 (String 5) */
    char Decl_Tm[8];                            /*    5 신고시각 (String 8) */
    char Omni_Acnt_Decl_Rej_Cd[2];              /*    6 옴니버스계좌신고 거부코드 (String 2) */
    char Omni_Acnt_Decl_Tp_Cd[2];               /*    7 옴니버스계좌 신고구분코드 (String 2) */
    char Decl_Acnt_No[12];                      /*    8 신고계좌번호 (String 12) */
    char Decl_Hgrk_Acnt_No[12];                 /*    9 최종투자자상위계좌번호 (String 12) */
    char Omni_Acnt_Algo_Yn[1];                  /*   10 옴니버스계좌 알고리즘여부 (String 1) */
    char Cntr_Cd[3];                            /*   11 국가코드 (String 3) */
    char Investor_Type_Code[4];                 /*   12 투자자구분코드 (String 4) */
    char Mbr_Use_Area[60];                      /*   13 회원사용영역 (String 60) */
} TCSMIR33001_DATA;

#endif  /* _KRX_TCSMIR33001_H */
