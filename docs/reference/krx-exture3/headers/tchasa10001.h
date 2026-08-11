#ifndef _KRX_TCHASA10001_H
#define _KRX_TCHASA10001_H

/* TCHASA10001 알고리즘거래계좌신고 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* (변형: 레이아웃 공유 그룹 TCHATA10001) */
/* DATA 길이 합계 = 523 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Mkt_Id[3];                             /*    4 시장ID (String 3) */
    char Mbr_No[5];                             /*    5 회원번호 (String 5) */
    char Acnt_Decl_Tp_Cd[1];                    /*    6 계좌신고유형코드 (String 1) */
    char Acpt_Tm[9];                            /*    7 접수시각 (String 9) */
    char Rej_Cd[2];                             /*    8 거부코드 (String 2) */
    char Acnt_No[12];                           /*    9 신고계좌번호 (String 12) */
    char If_Procs_Info[10];                     /*   10 I/F프로세스정보 (String 10) */
    char Chrg_Nm[80];                           /*   11 담당자명 (String 80) */
    char Chrg_Jobtitl_Nm[100];                  /*   12 담당자직위명 (String 100) */
    char Chrg_Dep_Nm[100];                      /*   13 담당자부서명 (String 100) */
    char Chrg_Tel_No[40];                       /*   14 담당자전화번호 (String 40) */
    char Chrg_Cellphone_No[20];                 /*   15 담당자휴대폰번호 (String 20) */
    char Chrg_Email[100];                       /*   16 담당자이메일 (String 100) */
    char Trst_Princ_Tp_Cd[2];                   /*   17 위탁자기구분코드 (String 2) */
    char Cntr_Cd[3];                            /*   18 국가코드 (String 3) */
    char Invst_Tp_Cd[4];                        /*   19 투자자구분코드 (String 4) */
    char Forninvst_Tp_Cd[2];                    /*   20 외국인투자자구분코드 (String 2) */
} TCHASA10001_DATA;

#endif  /* _KRX_TCHASA10001_H */
