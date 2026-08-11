#ifndef _KRX_TCHDTR40002_H
#define _KRX_TCHDTR40002_H

/* TCHDTR40002 소액채권 신고시장수익률 송수신 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 335 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Modcancl_Tp_Cd[1];                     /*    3 정정취소구분코드 (String 1) */
    char Trnsm_Dd[8];                           /*    4 전송일자 (String 8) */
    char Mbr_No[5];                             /*    5 회원번호 (String 5) */
    char If_Small_Trd_Kind_Cd_1[3];             /*    6 I/F소액매매종류코드_1 (String 3) */
    char If_Clsprc_Yd_1[13];                    /*    7 I/F종가수익률_1 (Float 13) */
    char If_Small_Trd_Kind_Cd_2[3];             /*    8 I/F소액매매종류코드_2 (String 3) */
    char If_Clsprc_Yd_2[13];                    /*    9 I/F종가수익률_2 (Float 13) */
    char If_Small_Trd_Kind_Cd_3[3];             /*   10 I/F소액매매종류코드_3 (String 3) */
    char If_Clsprc_Yd_3[13];                    /*   11 I/F종가수익률_3 (Float 13) */
    char If_Small_Trd_Kind_Cd_4[3];             /*   12 I/F소액매매종류코드_4 (String 3) */
    char If_Clsprc_Yd_4[13];                    /*   13 I/F종가수익률_4 (Float 13) */
    char If_Small_Trd_Kind_Cd_5[3];             /*   14 I/F소액매매종류코드_5 (String 3) */
    char If_Clsprc_Yd_5[13];                    /*   15 I/F종가수익률_5 (Float 13) */
    char If_Small_Trd_Kind_Cd_6[3];             /*   16 I/F소액매매종류코드_6 (String 3) */
    char If_Clsprc_Yd_6[13];                    /*   17 I/F종가수익률_6 (Float 13) */
    char If_Small_Trd_Kind_Cd_7[3];             /*   18 I/F소액매매종류코드_7 (String 3) */
    char If_Clsprc_Yd_7[13];                    /*   19 I/F종가수익률_7 (Float 13) */
    char If_Small_Trd_Kind_Cd_8[3];             /*   20 I/F소액매매종류코드_8 (String 3) */
    char If_Clsprc_Yd_8[13];                    /*   21 I/F종가수익률_8 (Float 13) */
    char If_Small_Trd_Kind_Cd_9[3];             /*   22 I/F소액매매종류코드_9 (String 3) */
    char If_Clsprc_Yd_9[13];                    /*   23 I/F종가수익률_9 (Float 13) */
    char If_Small_Trd_Kind_Cd_10[3];            /*   24 I/F소액매매종류코드_10 (String 3) */
    char If_Clsprc_Yd_10[13];                   /*   25 I/F종가수익률_10 (Float 13) */
    char If_Small_Trd_Kind_Cd_11[3];            /*   26 I/F소액매매종류코드_11 (String 3) */
    char If_Clsprc_Yd_11[13];                   /*   27 I/F종가수익률_11 (Float 13) */
    char If_Small_Trd_Kind_Cd_12[3];            /*   28 I/F소액매매종류코드_12 (String 3) */
    char If_Clsprc_Yd_12[13];                   /*   29 I/F종가수익률_12 (Float 13) */
    char If_Small_Trd_Kind_Cd_13[3];            /*   30 I/F소액매매종류코드_13 (String 3) */
    char If_Clsprc_Yd_13[13];                   /*   31 I/F종가수익률_13 (Float 13) */
    char If_Small_Trd_Kind_Cd_14[3];            /*   32 I/F소액매매종류코드_14 (String 3) */
    char If_Clsprc_Yd_14[13];                   /*   33 I/F종가수익률_14 (Float 13) */
    char If_Small_Trd_Kind_Cd_15[3];            /*   34 I/F소액매매종류코드_15 (String 3) */
    char If_Clsprc_Yd_15[13];                   /*   35 I/F종가수익률_15 (Float 13) */
    char If_Small_Trd_Kind_Cd_16[3];            /*   36 I/F소액매매종류코드_16 (String 3) */
    char If_Clsprc_Yd_16[13];                   /*   37 I/F종가수익률_16 (Float 13) */
    char Decl_Yd_Input_Rej_Rsn_Cd[3];           /*   38 신고수익률입력거부사유 (string 3) */
    char Decl_Yd_Opertr_Nm[20];                 /*   39 신고수익률입력자명 (String 20) */
    char Contactpnt[20];                        /*   40 연락처 (String 20) */
} TCHDTR40002_DATA;

#endif  /* _KRX_TCHDTR40002_H */
