#ifndef _KRX_TCHDTR40001_H
#define _KRX_TCHDTR40001_H

/* TCHDTR40001 일반,소액채권 채권신고매매 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 152 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Bnd_Decl_Trd_Tp_Cd[3];                 /*    3 채권신고매매구분코드 (String 3) */
    char Trd_Dd[8];                             /*    4 매매일자 (String 8) */
    char Mbr_No[5];                             /*    5 회원번호 (String 5) */
    char Brn_No[5];                             /*    6 지점번호 (String 5) */
    char Acnt_No[12];                           /*    7 계좌번호 (String 12) */
    char Isu_Cd[12];                            /*    8 종목코드 (String 12) */
    char Trd_Qty[15];                           /*    9 매매수량 (Long 15) */
    char Yd[13];                                /*   10 수익률 (Float 13) */
    char Prc[11];                               /*   11 가격 (Float 11) */
    char Askbid_Tp_Cd[1];                       /*   12 매도매수구분코드 (String 1) */
    char Trd_Tm[9];                             /*   13 거래시간 (String 9) */
    char Trst_Princ_Tp_Cd[2];                   /*   14 위탁자기구분코드 (String 2) */
    char Forninvst_Tp_Cd[2];                    /*   15 외국인투자자구분코드 (String 2) */
    char Ask_Cntr_Cd[3];                        /*   16 국가코드 (String 3) */
    char Ask_Invst_Tp_Cd[4];                    /*   17 매도투자자구분코드 (String 4) */
    char Decl_Trd_Seq[11];                      /*   18 신고매매일련번호 (Long 11) */
    char Decl_Trd_Orgn_Seq[11];                 /*   19 신고매매원일련번호 (Long 11) */
    char Ask_Agnc_Contrt_Tr_Yn[1];              /*   20 매도대행계약거래여부 (String 1) */
    char Decl_Rej_Rsn_Cd[2];                    /*   21 신고거부사유코드 (String 2) */
} TCHDTR40001_DATA;

#endif  /* _KRX_TCHDTR40001_H */
