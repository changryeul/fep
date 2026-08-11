#ifndef _KRX_TTRRTP42311_H
#define _KRX_TTRRTP42311_H

/* TTRRTP42311 채권Repo-체결 (DropCopy) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 335 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Mkt_Id[3];                             /*    4 시장ID (String 3) */
    char Brd_Id[2];                             /*    5 보드ID (String 2) */
    char Mbr_No[5];                             /*    6 회원번호 (String 5) */
    char Brn_No[5];                             /*    7 지점번호 (String 5) */
    char Ord_Id[10];                            /*    8 주문ID (String 10) */
    char Orgn_Ord_Id[10];                       /*    9 원주문ID (String 10) */
    char Isu_Cd[12];                            /*   10 종목코드 (String 12) */
    char Repo_Clss_Cd[2];                       /*   11 REPO분류코드 (String 2) */
    char Repo_Pd[4];                            /*   12 REPO기간 (Long 4) */
    char Ord_Qty[10];                           /*   13 호가수량 (Long 10) */
    char Ord_Prc[11];                           /*   14 호가가격 (Float 11) */
    char Trd_No[11];                            /*   15 체결번호 (Long 11) */
    char Contrt_No[24];                         /*   16 계약번호 (String 24) */
    char Trd_Prc[11];                           /*   17 체결가격 (Float 11) */
    char Trdvol[10];                            /*   18 체결수량 (Long 10) */
    char Sess_Id[2];                            /*   19 세션ID (String 2) */
    char Trdval[22];                            /*   20 거래대금 (Float 22) */
    char Trd_Dd[8];                             /*   21 체결일자 (String 8) */
    char Trd_Tm[9];                             /*   22 체결시각 (String 9) */
    char Askbid_Tp_Cd[1];                       /*   23 매도매수구분코드 (String 1) */
    char Acnt_No[12];                           /*   24 계좌번호 (String 12) */
    char Trstcom_No[5];                         /*   25 위탁사번호 (String 5) */
    char Acnt_Tp_Cd[2];                         /*   26 계좌구분코드 (String 2) */
    char Clr_Mbr_No[5];                         /*   27 결제회원번호 (String 5) */
    char Fnd_Cd[12];                            /*   28 펀드코드 (String 12) */
    char Invst_Tp_Cd[4];                        /*   29 투자자구분코드 (String 4) */
    char Trdr_No[5];                            /*   30 거래원번호 (String 5) */
    char Trst_Princ_Tp_Cd[2];                   /*   31 위탁자기구분코드 (String 2) */
    char Repur_Dd[8];                           /*   32 환매일자 (String 8) */
    char Repur_Amt[22];                         /*   33 환매금액 (Float 22) */
    char Member_Use_Area[60];                   /*   34 회원사용영역 (String 60) */
    char Lst_Askbid_Tp_Cd[1];                   /*   35 최종매도매수구분코드 (String 1) */
    char Nego_Partc_Tp_Cd[1];                   /*   36 협상참여구분코드 (String 1) */
} TTRRTP42311_DATA;

#endif  /* _KRX_TTRRTP42311_H */
