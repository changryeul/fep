#ifndef _KRX_TCSMIH26801_H
#define _KRX_TCSMIH26801_H

/* TCSMIH26801 접속세션이용료(주식,파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Procs_Utilfee_Tp_Cd[1];                /*    5 회원프로세스이용료구분코드 (String 1) */
    char Mkt_Id[3];                             /*    6 시장ID ★변경 (String 3) */
    char Mbr_No[5];                             /*    7 회원번호 (String 5) */
    char Nclr_Mbr_No_1[5];                      /*    8 거래전문회원번호 (String 5) */
    char Setl_Dd[8];                            /*    9 결제일자 (String 8) */
    char Procs_Utilfee[22];                     /*   10 프로세스이용료 (Float 22) */
    char Bas_Procs_Utilfee[22];                 /*   11 기본프로세스이용료 (Float 22) */
    char Add_Procs_Utilfee[22];                 /*   12 추가프로세스이용료 (Float 22) */
    char Add_Collct_Procs_Utilfee[22];          /*   13 추가징수분 (Float 22) */
    char Refnd_Procs_Utilfee[22];               /*   14 환급분 (Float 22) */
    char Filr_Val[1037];                        /*   15 필러값 (String 1037) */
} TCSMIH26801_DATA;

#endif  /* _KRX_TCSMIH26801_H */
