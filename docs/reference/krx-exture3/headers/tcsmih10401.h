#ifndef _KRX_TCSMIH10401_H
#define _KRX_TCSMIH10401_H

/* TCSMIH10401 유지증거금구간별이론가격(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 560 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    5 상품완료여부 (String 1) */
    char Market_Identification[3];              /*    6 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*    7 증권그룹ID (String 2) */
    char Underlying_Asset_Code[2];              /*    8 기초자산코드 (String 2) */
    char Member_Number[5];                      /*    9 회원번호 ★변경 (String 5) */
    char Issue_Code[12];                        /*   10 종목코드 (String 12) */
    char Field[2];                              /*   11 종목변동성구분 (String 2) */
    char Field_2[20];                           /*   12 옵션확정델타 (Float 20) */
    char Field_3[16];                           /*   13 Down 옵션조정이론가 (Float 16) */
    char Field_4[16];                           /*   14 Up 옵션조정이론가 (Float 16) */
    char Maintenance_Margin_Range_Number[16];   /*   15 유지증거금구간수치 (Float 16) */
    char Maintenance_Marign_Range_Theory_Price[16];/*   16 유지증거금구간이론가격
(또는 구간 델타) (Float 16) */
    char Filler_Value[418];                     /*   17 필러값 (String 418) */
} TCSMIH10401_DATA;

#endif  /* _KRX_TCSMIH10401_H */
