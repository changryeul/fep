#ifndef _KRX_TCSMIH28401_H
#define _KRX_TCSMIH28401_H

/* TCSMIH28401 채권시장거래수수료 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Trading_Member_Number[5];              /*    6 거래회원번호 (String 5) */
    char Market_Type[1];                        /*    7 시장구분 (String 1) */
    char Trading_Date[8];                       /*    8 매매일 (String 8) */
    char Settlement_Date[8];                    /*    9 결제일 (String 8) */
    char Fee_Payment_Date[8];                   /*   10 수수료납부일 (String 8) */
    char Trading_Fee[22];                       /*   11 거래체결KRX거래수수료 (Float 22) */
    char Clearing_Settlement_Fee[22];           /*   12 거래체결KRX청산결제수수료 (Float 22) */
    char Fee[22];                               /*   13 수수료 (Float 22) */
    char Filler_Value[1068];                    /*   14 필러값 (String 1068) */
} TCSMIH28401_DATA;

#endif  /* _KRX_TCSMIH28401_H */
