#ifndef _KRX_TCSMIR31801_H
#define _KRX_TCSMIR31801_H

/* TCSMIR31801 타회원위탁계좌(사전증거금계좌)신고및접수통보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1037 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Mbr_No[5];                             /*    4 회원번호 (String 5) */
    char Decl_Tm[8];                            /*    5 신고시각 (String 8) */
    char Othrmbr_Trst_Acnt_Decl_Rej_Cd[2];      /*    6 타회원위탁계좌신고 거부코드 (String 2) */
    char Othrmbr_Trst_Acnt_Decl_Tp_Cd[1];       /*    7 타회원위탁계좌신고 구분코드 (String 1) */
    char Decl_Acnt_No[12];                      /*    8 신고계좌번호 (String 12) */
    char Cntr_Cd[3];                            /*    9 국가코드 (String 3) */
    char Forninvst_Tp_Cd[2];                    /*   10 외국인투자자구분코드 (String 2) */
    char Trstee_Mbr_Chrg_Nm[80];                /*   11 수탁회원 담당자 성명 (String 80) */
    char Trstee_Mbr_Chrg_Nm_Contactpnt[20];     /*   12 수탁회원 담당자 연락처 (String 20) */
    char Trster_Mbr_No[5];                      /*   13 위탁회원번호 (String 5) */
    char Trster_Mbr_Chrg_Nm[80];                /*   14 위탁회원 담당자 성명 (String 80) */
    char Trster_Mbr_Chrg_Nm_Contactpnt[20];     /*   15 위탁회원 담당자 연락처 (String 20) */
    char Bas_Dd[8];                             /*   16 기준일자 (String 8) */
    char Retrst_Yn[1];                          /*   17 재위탁여부 (String 1) */
    char Opnint_Qty_Lmt_Adm_Methd[350];         /*   18 미결제약정 한도관리 방식 (String 350) */
    char Trd_Prod[150];                         /*   19 거래상품 (String 150) */
    char Ord_Sutb_Insp_Methd[100];              /*   20 호가적합성 점검방식 (String 100) */
    char Othrmbr_Trst_Acnt_Decl_Rsn[100];       /*   21 타회원위탁계좌 신고사유 (String 100) */
    char Mbr_Use_Area2[60];                     /*   22 회원사용영역 (String 60) */
} TCSMIR31801_DATA;

#endif  /* _KRX_TCSMIR31801_H */
