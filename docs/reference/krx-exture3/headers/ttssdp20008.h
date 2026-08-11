#ifndef _KRX_TTSSDP20008_H
#define _KRX_TTSSDP20008_H

/* TTSSDP20008 알고리즘거래계좌신고대상 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 67 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Mkt_Id[3];                             /*    4 시장ID (String 3) */
    char Mbr_No[5];                             /*    5 회원번호 (String 5) */
    char Acnt_No[12];                           /*    6 계좌번호 (String 12) */
    char Tot_Ord_Cnt[16];                       /*    7 총호가건수 (Long 16) */
    char Foreign_Integ_Acnt_Incld_Yn[1];        /*    8 외국인통합계좌포함여부 (String 1) */
} TTSSDP20008_DATA;

#endif  /* _KRX_TTSSDP20008_H */
