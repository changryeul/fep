/**
 * =============================================================================
 *  @시스템  명 : FEP
 *  @프로그램ID : bok_fx.h
 *  @프로그램명 : 한국은행 F/X외환전송망 Data 송신
 *  @개　　　요 : 한국은행 F/X외환전송망 Data 송신
 * =============================================================================
 *  << 프로그램 변경 정보 >>
 * ------------------------
 *  @변 경 일자 : 변경자 : 변     경     내     역
 * -----------------------------------------------------------------------------
 *  @2011-06-16 : 박병근 : 최초 작성
 *  @2014-10-20 : 김도형 : 비밀번호 변경 요구/비밀번호 변경 응답 전문 추가
 *                         동작상태 확인 요구/동작상태 확인 응답 전문 추가 
 *  @2014-11-03 : 김도형 : FX0074 추가(FX0073 이용)
 *  @2015-01-19 : 김도형 : WORK_ALL을 WORK_FX0074와 동일한 값으로 변경
 *          transfile dhkim normal'로 전송할 때 'FX0074'만 전송되도록 변경
 *
 * =============================================================================
**/
#ifndef __BOK_FORIGN_EXCHANGE_HDR__
#define __BOK_FORIGN_EXCHANGE_HDR__

/*==========================================================================*/
/*================== Common.h ==============================================*/
/*==========================================================================
2005/03/03 FX0073_DB_FORM에서 장내외구분 이 주석처리되어 있던 것을 풀어줌. by 김도형.
2007/06/21 전송규약 변경에 따른 수정. by 김도형.
2009/10/05 전송규약 변경에 따른 수정. by 김도형.
================== Macro Define ==========================================*/

/* DB Select Type */
#define SELECT_FIRST        1
#define SELECT_NEXT         2

/* File Open Type */
#define FILE_CREATE         1
#define FILE_OPEN           2
#define FILE_OPEN_CREATE    3

/* WORK Type (업무 개시/종료) */
#define WORK_OPEN           1
#define WORK_CLOSE          2
#define WORK_TERMINATION    3

/* Data Size */
#define 8           8
#define MAX_SEQ           200

/* Work Code */
#define WORK_FX0067     0x01
#define WORK_FX0068     0x02
#define WORK_FX0069     0x04
#define WORK_FX0070     0x08
#define WORK_FX0071     0x10
#define WORK_FX0072     0x20
#define WORK_FX0073     0x40
/* #define WORK_ALL        0x7F */
#define WORK_ALL        0x80
#define WORK_FX0074     0x80

/* Result Code */
#define RESULT_ERROR        0
#define RESULT_OK           1

/* 전송 Type */
#define DATA_NORMAL     '1'
#define DATA_DELETE     '2'
#define DATA_UPDATE     '4'

/* Sql Code */
#define SQL_NOMORE_DATA     1403

/* 참가기관/전송기관 */
#define COMPANY_CODE        "7103"

/*================== Data Structure ========================================*/


/*=========
 * DB Data Format
 */

struct _FX0067_DB_FORM {
    int     CheGyeolNo;                     /* 체결번호         */
    char    CheGyeolDate[9];      /* 체결일자         */
    double  GeumAek;                        /* 금액             */
    double  SeonMulHwanYul;                 /* 선물 환율        */
    double  HyeonMulHwanYul;                /* 현물 환율        */
    char    FixingDate[9];        /* Fixing Date      */
    char    ValueDate[9];         /* Value Date       */
};
typedef struct _FX0067_DB_FORM FX0067_DB_FORM;

struct _FX0068_DB_FORM {
    int     CheGyeolNo;                     /* 체결번호         */
    char    CheGyeolDate[9];      /* 체결일자         */
    double  GeumAek;                        /* 금액             */
    char    NearValueDate[9];     /* Near value date  */
    char    FarValueDate[9];      /* Far value date   */
    int     GiGan;                          /* 기간(일수)       */
    double  MaJin;                          /* 마진(전)         */
};
typedef struct _FX0068_DB_FORM FX0068_DB_FORM;

struct _FX0069_DB_FORM {
    char    Good_Fg[2];                   /* 상품구분         */
    int     CheGyeolNo;                     /* 체결번호         */
    char    CheGyeolDate[9];      /* 체결일자         */
    double  GeumAek;                        /* 금액             */
    char    SiJakDate[9];         /* 시작일           */
    char    JongRyoDate[9];       /* 종료일           */
    int     GiGan;                          /* 기간(일수)       */
    double  GoJeongGeumLi;                  /* 고정금리         */
    double  ByeondongGeumLi;                /* 변동금리         */
    char    MaeIpEopCheCode[5];           /* 매입 업체 코드   */
    char    MaeDoEopCheCode[5];           /* 매도 업체 코드   */
};
typedef struct _FX0069_DB_FORM FX0069_DB_FORM;

struct _FX0070_DB_FORM {
    int     CheGyeolNo;                     /* 체결번호         */
    char    CheGyeolDate[9];      /* 체결일자         */
    char    Type[2];                      /* Type             */
    char    PutCall[2];                   /* Put/Call         */
    double  GeumAek;                        /* 금액             */
    double  HaengSaGaGyeok;                 /* 행사가격         */
    double  OptionGaGyeokWon;               /* 옵션가격(원화)   */
    double  OptionGaGyeokUS;                /* 옵션가격(달러화) */
    char    HaengSaManGiDate[9];  /* 행사만기일       */
    char    MaeIpEopCheCode[5];           /* 매입 업체 코드   */
    char    MaeDoEopCheCode[5];           /* 매도 업체 코드   */
};
typedef struct _FX0070_DB_FORM FX0070_DB_FORM;

struct _FX0071_DB_FORM {
    int     CheGyeolNo;                     /* 체결번호         */
    char    CheGyeolDate[9];      /* 체결일자         */
    double  GeumAek;                        /* 금액             */
    double  HwanYul;                        /* 환율             */
    char    FixingDate[9];        /* Fixing Date      */
    char    ValueDate[9];         /* Value Date       */
    char    MaeIpEopCheCode[5];           /* 매입 업체 코드   */
    char    MaeDoEopCheCode[5];           /* 매도 업체 코드   */
};
typedef struct _FX0071_DB_FORM FX0071_DB_FORM;

struct _FX0072_DB_FORM {
    int     CheGyeolNo;                     /* 체결번호         */
    char    CheGyeolDate[9];      /* 체결일자         */
    char    Term[3];                      /* Term             */
    char    PutCall[5];                   /* Put/Call         */
    double  GeumAek;                        /* 금액             */
    double  OptionGaGyeokWon;               /* 옵션가격(원화)   */
};
typedef struct _FX0072_DB_FORM FX0072_DB_FORM;

struct _FX0073_DB_FORM {
    int     CheGyeolNo;                     /* 체결번호         */
    char    CheGyeolDate[9];      /* 체결일자         */
    char    GiJunTongHwaCode[4];          /* 기준통화 코드    */
    char    SangDaeTongHwaCode[4];        /* 상대통화 코드    */
    char    SangPumCode[2];               /* 상품 코드        */
    char    GiilCode[3];                  /* 기일 코드        */
    char    CheGyeolTime[7];              /* 체결 시각        */
    char    MaeIpEopCheCode[5];           /* 매입 업체 코드   */
    char    MaeDoEopCheCode[5];           /* 매도 업체 코드   */
    double  MaeMaeYul;                      /* 매매율           */
    double  CheGyeolGeumAek;                /* 체결 금액        */
    double  HyeonMulHwanYul;                /* 현물 환율        */
    char    GyeolJeDate[9];       /* 결제 일자        */
    char    ManGiDate[9];         /* 만기 일자        */
    char    JangNaeOeFlag[2];             /* 장내/장외 구분   */
    char    NearValueDate[9];     /* Near value date  */
    char    FarValueDate[9];      /* Far value date   */
};
typedef struct _FX0073_DB_FORM FX0073_DB_FORM;

struct _FX0074_DB_FORM {
    char    SangPumCode[3];               /* 상품 코드                         */
    int     CheGyeolNo;                     /* 체결번호                          */
    char    MaeIpEopCheCode[5];           /* 매입 업체 코드                    */
    char    MaeDoEopCheCode[5];           /* 매도 업체 코드                    */
    double  GeumAek;                        /* 금액(단위:USD)                    */
    char    GiJunTongHwaCode[4];          /* 기준통화 코드                     */
    char    SangDaeTongHwaCode[4];        /* 상대통화 코드                     */
    char    CheGyeolTime[7];              /* 체결 시각                         */
    char    JangNaeOeFlag[2];             /* 장내/장외 구분                    */
    char    GyeolJeDate[9];       /* 결제일                            */
    char    SiJakDate[9];         /* 시작일                            */
    char    JongRyoDate[9];       /* 종료일                            */
    char    HaengSaManGiDate[9];  /* 행사만기일                        */
    char    GiilCode[3];                  /* 기일 코드                         */
    double  GeoRaeHwanYul;                  /* 거래 환율                         */
    double  NearRate;                       /* 근일물 거래환율                   */
    double  FarRate;                        /* 원일물 거래환율                   */
    double  MarginWon;                      /* 시장평균환율 대비 가감율(단위:원) */
    double  GoJeongGeumLi;                  /* 고정금리                          */
    char    ByeondongGeumLiCode[3];       /* 중개사 거래 변동금리 코드         */
    double  ByeondongGeumLi;                /* 변동금리                          */
    char    PutCall[2];                   /* Put/Call                          */
    char    Type[2];                      /* Type                              */
    double  OptionGaGyeokUS;                /* 옵션가격(달러화)                  */
};
typedef struct _FX0074_DB_FORM FX0074_DB_FORM;

/*========= File Data Format */

typedef struct  {
    char    CheGyeolNo[5];                  /* 체결번호         */
    char    CheGyeolDate[8];        /* 체결일자         */
    char    GeumAek[16];                    /* 금액             */
    char    SeonMulHwanYul[12];             /* 선물 환율        */
    char    HyeonMulHwanYul[12];            /* 현물 환율        */
    char    FixingDate[8];          /* Fixing Date      */
    char    ValueDate[8];           /* Value Date       */
} FX0067_DATA_FORM;

typedef struct  {
    char    CheGyeolNo[5];                  /* 체결번호         */
    char    CheGyeolDate[8];        /* 체결일자         */
    char    GeumAek[16];                    /* 금액             */
    char    NearValueDate[8];       /* Near value date  */
    char    FarValueDate[8];        /* Far value date   */
    char    GiGan[4];                       /* 기간(일수)       */
    char    MaJin[16];                      /* 마진(전)         */
} FX0068_DATA_FORM;

typedef struct  {
    char    Good_Fg[1];                     /* 상품구분         */
    char    CheGyeolNo[5];                  /* 체결번호         */
    char    CheGyeolDate[8];        /* 체결일자         */
    char    GeumAek[16];                    /* 금액             */
    char    SiJakDate[8];           /* 시작일           */
    char    JongRyoDate[8];         /* 종료일           */
    char    GiGan[4];                       /* 기간(일수)       */
    char    GoJeongGeumLi[12];              /* 고정금리         */
    char    ByeondongGeumLi[12];            /* 변동금리         */
    char    MaeIpEopCheCode[4];             /* 매입 업체 코드   */
    char    MaeDoEopCheCode[4];             /* 매도 업체 코드   */
} FX0069_DATA_FORM;

typedef struct  {
    char    CheGyeolNo[5];                  /* 체결번호         */
    char    CheGyeolDate[8];        /* 체결일자         */
    char    Type[1];                        /* Type             */
    char    PutCall[1];                     /* Put/Call         */
    char    GeumAek[16];                    /* 금액             */
    char    HaengSaGaGyeok[16];             /* 행사가격         */
    char    OptionGaGyeokWon[16];           /* 옵션가격(원화)   */
    char    OptionGaGyeokUS[16];            /* 옵션가격(달러화) */
    char    HaengSaManGiDate[8];    /* 행사만기일       */
    char    MaeIpEopCheCode[4];             /* 매입 업체 코드   */
    char    MaeDoEopCheCode[4];             /* 매도 업체 코드   */
} FX0070_DATA_FORM;

typedef struct  {
    char    CheGyeolNo[5];                  /* 체결번호         */
    char    CheGyeolDate[8];        /* 체결일자         */
    char    GeumAek[16];                    /* 금액             */
    char    HwanYul[12];                    /* 환율             */
    char    FixingDate[8];          /* Fixing Date      */
    char    ValueDate[8];           /* Value Date       */
    char    MaeIpEopCheCode[4];             /* 매입 업체 코드   */
    char    MaeDoEopCheCode[4];             /* 매도 업체 코드   */
} FX0071_DATA_FORM;

typedef struct  {
    char    CheGyeolNo[5];                  /* 체결번호         */
    char    CheGyeolDate[8];        /* 체결일자         */
    char    Term[2];                        /* Term             */
    char    PutCall[1];                     /* Put/Call         */
    char    GeumAek[16];                    /* 금액             */
    char    OptionGaGyeokWon[16];           /* 옵션가격(원화)   */
} FX0072_DATA_FORM;

typedef struct  {
    char    CheGyeolNo[5];                  /* 체결번호         */
    char    CheGyeolDate[8];        /* 체결일자         */
    char    GiJunTongHwaCode[3];            /* 기준통화 코드    */
    char    SangDaeTongHwaCode[3];          /* 상대통화 코드    */
    char    SangPumCode[1];                 /* 상품 코드        */
    char    GiilCode[2];                    /* 기일 코드        */
    char    CheGyeolTime[6];                /* 체결 시각        */
    char    MaeIpEopCheCode[4];             /* 매입 업체 코드   */
    char    MaeDoEopCheCode[4];             /* 매도 업체 코드   */
    char    MaeMaeYul[12];                  /* 매매율           */
    char    CheGyeolGeumAek[16];            /* 체결 금액        */
    char    HyeonMulHwanYul[12];            /* 현물 환율        */
    char    GyeolJeDate[8];         /* 결제 일자        */
    char    ManGiDate[8];           /* 만기 일자        */
    char    JangNaeOeFlag[1];               /* 장내/장외 구분   */
    char    NearValueDate[8];       /* Near value date  */
    char    FarValueDate[8];        /* Far value date   */
} FX0073_DATA_FORM;

typedef struct  {
    char    SangPumCode[2];                 /* 상품 코드                         */
    char    CheGyeolNo[5];                  /* 체결번호                          */
    char    MaeIpEopCheCode[4];             /* 매입 업체 코드                    */
    char    MaeDoEopCheCode[4];             /* 매도 업체 코드                    */
    char    GeumAek[16];                    /* 금액                              */
    char    GiJunTongHwaCode[3];            /* 기준통화 코드                     */
    char    SangDaeTongHwaCode[3];          /* 상대통화 코드                     */
    char    CheGyeolTime[6];                /* 체결 시각                         */
    char    JangNaeOeFlag[1];               /* 장내/장외 구분                    */
    char    GyeolJeDate[8];         /* 결제일                            */
    char    SiJakDate[8];           /* 시작일                            */
    char    JongRyoDate[8];         /* 종료일                            */
    char    HaengSaManGiDate[8];    /* 행사만기일                        */
    char    GiilCode[2];                    /* 기일 코드                         */
    char    GeoRaeHwanYul[12];              /* 거래 환율                         */
    char    NearRate[12];                   /* 근일물 거래환율                   */
    char    FarRate[12];                    /* 원일물 거래환율                   */
    char    MarginWon[12];                  /* 시장평균환율 대비 가감율(단위:원) */
    char    GoJeongGeumLi[12];              /* 고정금리                          */
    char    ByeondongGeumLiCode[2];         /* 중개사 거래 변동금리 코드         */
    char    ByeondongGeumLi[12];            /* 변동금리                          */
    char    PutCall[1];                     /* Put/Call                          */
    char    Type[1];                        /* Type                              */
    char    OptionGaGyeokUS[16];            /* 옵션가격(달러화)                  */
} FX0074_DATA_FORM;

typedef struct  {
    char    BoGoGiGwanCode[4];              /* 보고기관코드     */
    char    ChwiGeupJeomPoCode[4];          /* 취급점포코드     */
    char    JeonSongDate[8];                /* 전송일자(오늘자) */
    char    JeonSongCount[2];               /* 전송횟수         */
    char    JeonSongType;                   /* 전송구분         */
    char    CheoRiNo[6];                    /* 처리번호         */
    char    FileCode[6];
    char    BoGoJeomPoGroup;                /* 보고점포그룹구분 */
    char    JakSeongGiJunDate[8];           /* 작성기준일자     */
    char    Report;
} FX1000_DATA_FORM;

/*========= File Format ============================================*/

typedef struct  {
    char    BoGoGiGwanCode[4];              /* 보고기관코드     */
    char    ChwiGeupJeomPoCode[4];          /* 취급점포코드     */
    char    ChwiGeupJeomPoPost[6];          /* 취급점포소재지   */
    char    BoGoJeomPoGroup;                /* 보고점포그룹구분 */
    char    JeonSongDate[8];                /* 전송일자(오늘자) */
    char    JakSeongGiJunDate[8];           /* 작성기준일자     */
    char    JeonSongCount[2];               /* 전송횟수         */
    char    JeonSongType;                   /* 전송구분         */
    char    CheoRiNo[6];                    /* 처리번호         */
} FILE_HEADER_FORM;

typedef struct  {
    FILE_HEADER_FORM    Head;           /* 화일 헤더        */
    FX0067_DATA_FORM    Data;           /* 데이터 버퍼      */
} FX0067_FILE_FORM;

typedef struct  {
    FILE_HEADER_FORM    Head;           /* 화일 헤더        */
    FX0068_DATA_FORM    Data;           /* 데이터 버퍼      */
} FX0068_FILE_FORM;

typedef struct  {
    FILE_HEADER_FORM    Head;           /* 화일 헤더        */
    FX0069_DATA_FORM    Data;           /* 데이터 버퍼      */
} FX0069_FILE_FORM;

typedef struct  {
    FILE_HEADER_FORM    Head;           /* 화일 헤더        */
    FX0070_DATA_FORM    Data;           /* 데이터 버퍼      */
} FX0070_FILE_FORM;

typedef struct  {
    FILE_HEADER_FORM    Head;           /* 화일 헤더        */
    FX0071_DATA_FORM    Data;           /* 데이터 버퍼      */
} FX0071_FILE_FORM;

typedef struct  {
    FILE_HEADER_FORM    Head;           /* 화일 헤더        */
    FX0072_DATA_FORM    Data;           /* 데이터 버퍼      */
} FX0072_FILE_FORM;

typedef struct  {
    FILE_HEADER_FORM    Head;           /* 화일 헤더        */
    FX0073_DATA_FORM    Data;           /* 데이터 버퍼      */
} FX0073_FILE_FORM;

typedef struct  {
    FILE_HEADER_FORM    Head;           /* 화일 헤더        */
    FX0074_DATA_FORM    Data;           /* 데이터 버퍼      */
} FX0074_FILE_FORM;

typedef struct  {
    FX1000_DATA_FORM    Data;           /* 데이터 버퍼      */
} FX1000_FILE_FORM;


/*============== 전송 Format ======================================*/

/* 공통 정보 */
typedef struct  {
    char    TelgLength[4];                  /* 전문길이정보           */
    char    Transaction[9];                 /* 'FOREXFTP2'            */
    char    SystemName[3];                  /* 'FXB'                  */
    char    CompanyCode[4];                 /* '7103'                 */
    char    MessageCode[8];
    char    Filler[2];                      /* '  '(space)            */
    char    ContinueYN[1];
    char    ResponseCode[3];                /* '000'                  */
} NET_COMMON_FORM;

/* 업무 개시 요구 및 응답 정보 0600/0010, 0610/0010 */
typedef struct  {
    char    CurrentDateTime[14];
    char    SendID[20];                     /* '7103FTP0000000000001' */
    char    SendPW[16];                     /* '7103FTP@'             */
    char    ConfirmGankyuk[4];
    char    SendMode[1];                    /* 'B'                    */
    char    CompressFg[1];                  /* '0'(사용안함)          */
    char    InheritYN[1];                   /* 이어받기 사용여부      */
} NET_WORK_OPEN_FORM;

/* 업무 종료 요구 및 응답 정보 0600/0040, 0610/0040 */
typedef struct  {
    char    CloseDateTime[14];
} NET_WORK_CLOSE_FORM;

/* 파일 송신 요구 정보 0300/0010 */
typedef struct  {
    char    SendReqDate[8];
    char    FileCode[6];
    char    DataType[2];
} NET_FILE_SEND_REQ_FORM;

/* 파일 송신 응답 정보 0310/0010 */
typedef struct  {
    char    SendReqDate[8];
    char    Result[1];
    char    DataCnt[3];
} NET_FILE_SEND_RES_FORM;

/* 파일 시작 요구 정보 0300/0020 */
typedef struct  {
    char    FileCode[6];
    char    DataType[2];
    char    RecordLength[4];
    char    CompressFg[1];
    char    TotalLength[10];
} NET_FILE_BEGIN_REQ_FORM;

/* 파일 시작 응답 정보 0310/0020 */
typedef struct  {
    char    FileCode[6];
    char    DataType[2];
    char    RecordLength[4];
    char    CompressFg[1];
    char    TotalLength[10];
    char    InheritYN[1];
    char    RecvLength[10];
} NET_FILE_BEGIN_RES_FORM;

/* 파일 데이터 송신 정보 0300/0030 */
typedef struct  {
    char    Sequence[7];
    char    SentLength[10];
    char    DataLength[4];
} NET_FILE_DATA_SEND_FORM;

/* 파일 데이터 정보 0300/0030 */
typedef struct  {
    char    FileCode[6];
    char    DataType[2];
    char    SequenceNo[7];
} NET_FILE_DATA_FORM;

/* 파일 확인 요구 정보 0300/0040 */
typedef struct  {
    char    LastSequence[7];
    char    TotSentLength[10];
} NET_LOST_REQ_FORM;

/* 파일 확인 응답 정보 0310/0040 */
typedef struct  {
    char    Result[2];
    char    LastSequenceNo[7];
    char    LastRecvLength[10];
} NET_LOST_RES_FORM;

/* 파일 종료 요구 정보 0300/0050 */
typedef struct  {
    char    LastSequence[7];
    char    TotSentLength[10];
    char    CloseDateTime[14];
} NET_FILE_CLOSE_REQ_FORM;

/* 파일 종료 응답 정보 0310/0050 */
typedef struct  {
    char    Result[2];
    char    LastSequenceNo[7];
    char    LastRecvLength[10];
    char    CloseDateTime[14];
    char    ServerControlNo[33];
} NET_FILE_CLOSE_RES_FORM;

/* 2014.10.20 추가 시작. 김도형 */ 
/* 비밀번호 변경 요구 정보 0800/0020 */
typedef struct  {
    char    SendID[20];                     /* '7103FTP0000000000001' */
    char    SendPW_OLD[16];                 /* '7103FTP@'             */
    char    SendPW_NEW[16];                 /* '7103FTP@'             */
} NET_PW_CHG_REQ_FORM;

/* 비밀번호 변경 응답 정보 0810/0020 */
typedef struct  {
    char    Result[2];
} NET_PW_CHG_RES_FORM;

/* 동작상태 확인요구 정보 0800/0010 */
typedef struct  {
    char    CurrentDateTime[14];
} NET_STATUS_REQ_FORM;

/* 동작상태 확인응답 정보 0810/0010 */
typedef struct  {
    char    CurrentDateTime[14];
} NET_STATUS_RES_FORM;
/* 2014.10.20 추가 종료. 김도형 */ 

/* 파일 업무 데이타 */

typedef struct  {
    NET_FILE_DATA_FORM      Info;
    FX0067_FILE_FORM    Data;
} NET_DATA_FX0067;

typedef struct  {
    NET_FILE_DATA_FORM      Info;
    FX0068_FILE_FORM    Data;
} NET_DATA_FX0068;

typedef struct  {
    NET_FILE_DATA_FORM      Info;
    FX0069_FILE_FORM    Data;
} NET_DATA_FX0069;

typedef struct  {
    NET_FILE_DATA_FORM      Info;
    FX0070_FILE_FORM    Data;
} NET_DATA_FX0070;

typedef struct  {
    NET_FILE_DATA_FORM      Info;
    FX0071_FILE_FORM    Data;
} NET_DATA_FX0071;

typedef struct  {
    NET_FILE_DATA_FORM      Info;
    FX0072_FILE_FORM    Data;
} NET_DATA_FX0072;

typedef struct  {
    NET_FILE_DATA_FORM      Info;
    FX0073_FILE_FORM    Data;
} NET_DATA_FX0073;

typedef struct  {
    NET_FILE_DATA_FORM      Info;
    FX0074_FILE_FORM    Data;
} NET_DATA_FX0074;

typedef struct  {
    NET_FILE_DATA_FORM      Info;
    FX1000_FILE_FORM        Data;
} NET_DATA_FX1000;

/* 전송 할 수 있는 MAX Record 수 */

#define MAX_DATA_CNT_FX0067  ( 4045 / sizeof(NET_DATA_FX0067) )
#define MAX_DATA_CNT_FX0068  ( 4045 / sizeof(NET_DATA_FX0068) )
#define MAX_DATA_CNT_FX0069  ( 4045 / sizeof(NET_DATA_FX0069) )
#define MAX_DATA_CNT_FX0070  ( 4045 / sizeof(NET_DATA_FX0070) )
#define MAX_DATA_CNT_FX0071  ( 4045 / sizeof(NET_DATA_FX0071) )
#define MAX_DATA_CNT_FX0072  ( 4045 / sizeof(NET_DATA_FX0072) )
#define MAX_DATA_CNT_FX0073  ( 4045 / sizeof(NET_DATA_FX0073) )
#define MAX_DATA_CNT_FX0074  ( 4045 / sizeof(NET_DATA_FX0074) )



/* 전송 패킷(Packet) 정보 */

typedef struct  {
    NET_COMMON_FORM         Common;
    NET_WORK_OPEN_FORM     Info;
} PACKET_WORK_OPEN;

typedef struct  {
    NET_COMMON_FORM         Common;
    NET_WORK_CLOSE_FORM     Info;
} PACKET_WORK_CLOSE;

typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_SEND_REQ_FORM    Info;
} PACKET_FILE_SEND_REQUEST;

typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_SEND_RES_FORM    Info;
} PACKET_FILE_SEND_RESPONSE;

typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_BEGIN_REQ_FORM    Info;
} PACKET_FILE_BEGIN_REQUEST;

typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_BEGIN_RES_FORM    Info;
} PACKET_FILE_BEGIN_RESPONSE;

typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_CLOSE_REQ_FORM   Info;
} PACKET_FILE_CLOSE_REQ;

typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_CLOSE_RES_FORM   Info;
} PACKET_FILE_CLOSE_RES;

typedef struct  {
    NET_COMMON_FORM     Common;
    NET_LOST_REQ_FORM     Info;
} PACKET_LOST_REQUEST;


typedef struct  {
    NET_COMMON_FORM     Common;
    NET_LOST_RES_FORM     Info;
} PACKET_LOST_RESPONSE;

/* 2014.10.20 추가 시작. 김도형 */ 
typedef struct  {
    NET_COMMON_FORM         Common;
    NET_PW_CHG_REQ_FORM       Info;
} PACKET_PW_CHG_REQUEST;

typedef struct  {
    NET_COMMON_FORM         Common;
    NET_PW_CHG_RES_FORM       Info;
} PACKET_PW_CHG_RESPONSE;

typedef struct  {
    NET_COMMON_FORM         Common;
    NET_STATUS_REQ_FORM       Info;
} PACKET_STATUS_REQUEST;

typedef struct  {
    NET_COMMON_FORM         Common;
    NET_STATUS_RES_FORM       Info;
} PACKET_STATUS_RESPONSE;
/* 2014.10.20 추가 종료. 김도형 */ 

typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_DATA_SEND_FORM DataSend;
    NET_DATA_FX0067         Data[MAX_DATA_CNT_FX0067];
} PACKET_FILE_FX0067;


typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_DATA_SEND_FORM DataSend;
    NET_DATA_FX0068         Data[MAX_DATA_CNT_FX0068];
} PACKET_FILE_FX0068;


typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_DATA_SEND_FORM DataSend;
    NET_DATA_FX0069         Data[MAX_DATA_CNT_FX0069];
} PACKET_FILE_FX0069;


typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_DATA_SEND_FORM DataSend;
    NET_DATA_FX0070         Data[MAX_DATA_CNT_FX0070];
} PACKET_FILE_FX0070;


typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_DATA_SEND_FORM DataSend;
    NET_DATA_FX0071         Data[MAX_DATA_CNT_FX0071];
} PACKET_FILE_FX0071;


typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_DATA_SEND_FORM DataSend;
    NET_DATA_FX0072         Data[MAX_DATA_CNT_FX0072];
} PACKET_FILE_FX0072;


typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_DATA_SEND_FORM DataSend;
    NET_DATA_FX0073         Data[MAX_DATA_CNT_FX0073];
} PACKET_FILE_FX0073;

typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_DATA_SEND_FORM DataSend;
    NET_DATA_FX0074         Data[MAX_DATA_CNT_FX0074];
} PACKET_FILE_FX0074;

typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_DATA_SEND_FORM DataSend;
    NET_DATA_FX1000         Data;
} PACKET_FILE_FX1000;

typedef struct  {
    NET_COMMON_FORM         Common;
    NET_FILE_DATA_SEND_FORM DataSend;
    char                    Data[4045];
} PACKET_MESSAGE_FORM;

typedef struct  {
    char    Result[8];
    char    FileCode[9];
    char    Message[100];
} RESULT_FORM;

typedef struct  {
    int     fx0067;
    int     fx0068;
    int     fx0069;
    int     fx0070;
    int     fx0071;
    int     fx0072;
    int     fx0073;
    int     fx0074;
} RECORD_COUNT_FORM;

/*===================== Function Prototypes ================================*/

/*===== Top Modules */

#endif //__BOK_FORIGN_EXCHANGE_HDR__
