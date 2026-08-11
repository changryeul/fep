#ifndef     __SHM_MEMORY_H
#define     __SHM_MEMORY_H
/*------------------------------------------------------------------------
#   Module  : shared memory management
#   File    : shm_memory.h
#   copywrite : 박상훈
------------------------------------------------------------------------*/
#include    "krx_mk.h"
#include    "key_code.h"
#include    "strategy.h"

/*------------------------------------------------------------------------
  시장별 종목 건수
------------------------------------------------------------------------*/
/* 파생 */
#define     ACC_NO_CNT          20                      /* 사용계좌수    */
#define     ACC_NO_BASE         31                      /* 계좌기준수    */
#define     FILE_NO_BASE        30                      /* 파일기준수    */

/* 주문번호 영역, 메리츠 */
/* 현물 3천만건, 파생 3천만건 */
/* 프로세스(pa_50101mp..)당 10만건, 마지막 천만건은 Client에 할당    */
/* 현물 : 1530000000 ~ 1549999999, Client : 1550000000 ~ 1559999999 */
/* 파생 : 1580000000 ~ 1599999999, Client : 1600000000 ~ 1609999999 */
/* ---------------------------------------------------------------- */
/* pa_50101mp : 현물 => 1530100000 ~ 1530199999, 파생 => 1580100000 ~ 1580199999 */
/* pa_50102mp : 현물 => 1530200000 ~ 1530299999, 파생 => 1580200000 ~ 1580299999 */
/* pa_50103mp : 현물 => 1530300000 ~ 1530399999, 파생 => 1580300000 ~ 1580399999 */
/* pa_50104mp : 현물 => 1530400000 ~ 1530499999, 파생 => 1580400000 ~ 1580499999 */
/* pa_50105mp : 현물 => 1530500000 ~ 1530599999, 파생 => 1580500000 ~ 1580599999 */
/* pa_50106mp : 현물 => 1530600000 ~ 1530699999, 파생 => 1580600000 ~ 1580699999 */
/* pa_50107mp : 현물 => 1530700000 ~ 1530799999, 파생 => 1580700000 ~ 1580799999 */
/* pa_50108mp : 현물 => 1530800000 ~ 1530899999, 파생 => 1580800000 ~ 1580899999 */
/* pa_50109mp : 현물 => 1530900000 ~ 1530999999, 파생 => 1580900000 ~ 1580999999 */
/* pa_50110mp : 현물 => 1531000000 ~ 1531099999, 파생 => 1581000000 ~ 1581099999 */
/* pa_50201mp : 현물 => 1531100000 ~ 1531199999, 파생 => 1581100000 ~ 1581199999 */
/* pa_50202mp : 현물 => 1531200000 ~ 1531299999, 파생 => 1581200000 ~ 1581299999 */
/* pa_50203mp : 현물 => 1531300000 ~ 1531399999, 파생 => 1581300000 ~ 1581399999 */
/* pa_50204mp : 현물 => 1531400000 ~ 1531499999, 파생 => 1581400000 ~ 1581499999 */
/* pa_50205mp : 현물 => 1531500000 ~ 1531599999, 파생 => 1581500000 ~ 1581599999 */
/* pa_50206mp : 현물 => 1531600000 ~ 1531699999, 파생 => 1581600000 ~ 1581699999 */
/* pa_50207mp : 현물 => 1531700000 ~ 1531799999, 파생 => 1581700000 ~ 1581799999 */
/* pa_50208mp : 현물 => 1531800000 ~ 1531899999, 파생 => 1581800000 ~ 1581899999 */
/* pa_50209mp : 현물 => 1531900000 ~ 1531999999, 파생 => 1581900000 ~ 1581999999 */
/* pa_50210mp : 현물 => 1532000000 ~ 1532099999, 파생 => 1582000000 ~ 1582099999 */
/* ---------------------------------------------------------------- */
#if 0
#define     JS_START_NO_01    1530000000                /* 현물주문번호 START(1번서버)   && TEST서버 */
#define     JS_END_NO_01      1544999999                /* 현물주문번호 LAST(1번서버)    && TEST서버   */
#define     JS_START_NO_02    1545000000                /* 현물주문번호 START(2번서버)   */
#define     JS_END_NO_02      1559999999                /* 현물주문번호 LAST(2번서버)    */

#define     DV_START_NO_01    1580000000                /* 파생주문번호 START(1번서버)   && TEST서버   */
#define     DV_END_NO_01      1594999999                /* 파생주문번호 LAST(1번서버)    && TEST서버   */
#define     DV_START_NO_02    1595000000                /* 파생주문번호 START(2번서버)   */
#define     DV_END_NO_02      1609999999                /* 파생주문번호 LAST(2번서버)    */
#endif

/* 자동주문 Process 수  */
#define     MAX_AUTO_PROC     40                        /* 자동주문 Process 수   */

/* 미체결 처리영역 */
#define     MAX_MICHE         10000                     /* Poll 밀린것 */

/* System Risk 관리 */
/* 선물금액은 75억, 옵션금액은 3억 */
/* 선물은 500,000으로 나누고, 옵션은 100,000으로 나눈값 */
#define     MAX_POLL          1000                      /* Poll 밀린것 */
#define     MAX_FU_SU           50                      /* 선물수량기준 */
#define     MAX_OP_SU         1000                      /* 옵션수량기준 */
#define     MAX_FU_GUM       15000                      /* 선물주문금액   */
#define     MAX_OP_GUM        3000                      /* 옵션주문금액   */

/* 자동로직에서 사용되는 주문포맷 형태 */
#define     JU_SUB_MAX          20                      /* 주문포맷Max  */

/* 시장관리 Arry를 위한 값 DeFine */
#define     MK_CNT               14                     /* Market Cnt   */

#define     MK_JISU              0                      /* 지수           */
#define     MK_JF                1                      /* 지수선물     */
#define     MK_JO                2                      /* 지수옵션     */
#define     MK_SF                3                      /* 주식선물     */
#define     MK_SO                4                      /* 주식옵션     */
#define     MK_KOSPI             5                      /* 유가증권     */
#define     MK_KOSDAQ            6                      /* 코스닥      */
#define     MK_ELW               7                      /* ELW          */
#if 0
#define     MK_ETN              17                      /* ETN          */
#endif
#define     MK_K300              8                      /* KRX300       */
#define     MK_K150F             9                      /* Kosdaq150 Futures    */
#define     MK_K150O            10                      /* Kosdaq150 Options    */
#define     MK_MF               11                      /* 미니코스피 선물 */
#define     MK_MO               12                      /* 미니코스피 옵션 */

#define     MK_CME              31                      /* CME          */
#define     MK_CMX              32                      /* CMX          */
#define     MK_SGX              33                      /* SGX          */
#define     MK_ERX              34                      /* ERX          */
#define     MK_HKE              35                      /* HKE          */

/* 한도시장구분 */
#define     RISK_MK_CNT         14                      /* Risk Market Cnt  */
/*  11( 0) : 지수선물(1)
    12( 1) : 지수콜옵션(2)
    13( 2) : 지수풋옵션(2)
    14( 3) : 주식선물(3)
    15( 4) : 주식콜옵션(4)
    16( 5) : 주식풋옵션(4)
    20( 6) : 코스닥150선물(9)
    44( 7) : 코스닥150콜옵션(10)
    45( 8) : 코스닥150풋옵션(10)
    46( 9) : KRX300(8)
    51(10) : 현물주식(5,6,7)
    34(11) : 미니코스피200선물(11)
    35(12) : 미니코스피200콜옵션(12)
    36(13) : 미니코스피200풋옵션(12)
*/

/*------------------------------------------------------------------------
    시장별 종목 Arry MAX 건수
------------------------------------------------------------------------*/
#define     SHM_MAX_FUTURES     100                     /* 지수선물시세   13*/
#define     SHM_MAX_OPTIONS     2000                    /* 지수옵션시세   1200*/
#define     SHM_MAX_S_FUTURES   4000                    /* 주식선물시세   2721*/
#define     SHM_MAX_S_OPTIONS   10000                   /* 주식옵션시세   7202*/
#define     SHM_MAX_STOCK       15000                   /* 유가증권시세   */
#define     SHM_MAX_KOSDAQ       5000                   /* 코스닥종목시세  3269*/
#if 0
ELW
#define     SHM_MAX_ELW         10000                   /* ELW 종목시세 */
#define     SHM_MAX_ETF          2000                   /* ETF 종목시세 */
#endif
#define     SHM_MAX_ETN          2000                   /* ETN 종목시세 */
#define     SHM_MAX_K300         100                    /* KRX300 종목시세  */
#define     SHM_MAX_K150F        100                    /* KOSDAQ150 종목시세   */
#define     SHM_MAX_K150O        500                    /* KOSDAQ150 종목시세   */
#define     SHM_MAX_MF           100                    /* 미니선물시세   */
#define     SHM_MAX_MO           2000                   /* 미니옵션시세   */
#define     SHM_MAX_JISU         500                    /* 지수시세 tr(2)+업종코드(3)가 키    */
                                                        /* D0:kospi지수 001~027*/
                                                        /* D1:kospi지수예상지수 001~027*/
                                                        /* D2:kospi200지수 029*/
                                                        /* D3:kospi200예상지수 029*/
                                                        /* E4:kosdaq지수 181~184*/
                                                        /* E5:kosdaq예상지수 181~184*/

#define     SHM_MAX_KEY          1                      /* 종목코드 bsearch용Key*/
#define     SHM_MAX_BACKOFFICE  10                      /* BackOffice시세         */
#define     SHM_MAX_PREMATCH    MK_CNT                  /* 미체결용 MK_CNT(9)   */
#define     SHM_MAX_STRRG       ACC_NO_CNT              /* 전략용              */

#define     SHM_MAX_CME         10000                   /* CME MAX  */
#define     SHM_MAX_CMX         10000                   /* CMX MAX  */
#define     SHM_MAX_SGX         10000                   /* SGX MAX  */
#define     SHM_MAX_ERX         10000                   /* ERX MAX  */
#define     SHM_MAX_HKE         10000                   /* HKE MAX  */

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
typedef struct {
    u_short port_no;                /* master port number               */
    u_short service_port_no[10];    /* service port number              */
    int     service_count[10];      /* service count                    */
    u_char  ip_addr[4];             /* ip address                       */
    u_char  port_status;            /* port status (0:off 1:on)         */
    u_char  service_status[10];     /* service status (0:off 1:on 9:N/A)*/
    char    tcp_info[40];           /* tcp1 information                 */
}   TCP1_INFO;  /* TCP/IP 1 (TCP1)  */

typedef struct {
    short   dup_id;                 /* line distinction (to avoid dup)  */
    u_short port_no;                /* port number                      */
    u_char  ip_addr[4];             /* ip address                       */
    u_char  proc_status;            /* process status (0:init/off 1:on) */
    u_char  line_status;            /* line status (0:off 1:on)         */
    u_char  network_status; /* network status (0:init/off 1:on 2:end)   */
    char    tcp_info[40];           /* tcp2 information                 */
}   TCP2_INFO;  /* TCP/IP 2 (TCP2)  */

typedef struct {
    short   dup_id;                 /* line distinction (to avoid dup)  */
    u_short port[20];               /* server port no                   */
    u_char  ip_addr[20][4];         /* server ip address                */
    u_char  port_status;            /* port status (0:off 1:on)         */
    u_char  service_status;         /* 0:off 1:on                       */
    char    info[40];               /* infomation                       */
}   UDPIP_INFO; /* UDP/IP (UDPIP)   */

typedef struct {
    char    file_name[20];          /* file name                        */
    int     w_cnt[2];               /* write count                      */
    int     r_cnt[9];               /* read count                       */
    u_char  fifo_count;             /* number of fifo in use (1 ~ 9)    */
    short   record_size;            /* record size                      */
    char    file_info[40];          /* file information                 */
}   FILE_INFO;  /* File (FILEM) */

typedef struct {
    char    data_name[12];          /* data SHM base name               */
    char    key_info[12];   /* base key(4)+','+section(1)+','+format(1) */
    u_char  fifo_count;             /* number of fifo in use (1 ~ 9)    */
    short   data_size;              /* data size                        */
    int     max_rec;                /* max record                       */
    int     offset;                 /* offset                           */
    int     w_cnt[2];               /* write count                      */
    int     r_cnt[9];               /* read count                       */
    int     sm_r_cnt;               /* sync manager read count          */
    char    info[40];               /* information                      */
}   DSHM_INFO;  /* data SHM (DSHM)  */

#if defined ISAM_INCL
typedef struct {
    char    file_name[20];          /* file name                        */
    short   key_size;               /* key size                         */
    short   record_size;            /* record size                      */
    char    file_info[40];          /* file information                 */
}   CISAM_INFO; /* C-ISAM (CISAM)   */
#endif

typedef struct {
    char    tr[6];
    short   length;
    short   queue;
    int     count;
    int     dd_count;
    int     as_count;
    int     mp_count;
    int     ms_count;
    int     us_count;
    int     ur_count;
    int     fo_count;
    char    tr_info[40];            /* sise tr infomation               */
}   SISETR_INFO;    /* sise TR (SISETR) */

typedef union {
    struct {
        u_char  line_gubun; /* line type (1,3...:primary 2,4...:backup) */
        u_char  port_type;      /* port type (0:master 1~9:service)     */
        u_char  network_status; /* network status(0:init/off 1:on 2:end)*/
    }       t1;                     /* tcp1                             */
    struct {
        u_char  line_gubun;     /* line type (0:primary 1:backup)       */
        u_char  connect_status; /* connect status (0:off 1:connected)   */
        u_short l[2];           /* line                                 */
    }       t2;                     /* tcp2                             */
    short   u;                  /* udpip port index                     */
}   LINE_INFO;  /* Line */

typedef struct {
    int         process_no;         /* process number (pid)             */
    char        process_path[100];  /* process path                     */
    char        process_id[20];     /* process id (name)                */
    u_char      process_status; /*process status(1:run 2:stop 9:not run)*/
    char        process_info[40];   /* process information              */
    int         in_FIFO_fd[3];      /* fd of in-data FIFO               */
    u_char      start_status;/*start status(0:init 1:start 2:end 3:stop)*/
    short       timeout;            /* timeout                          */
    short       delay;              /* PS delay time (millisec)         */
    char        start_time[6];      /* process start time (hhmm)        */
    char        end_time[6];        /* process end time (hhmm)          */
    u_char      backup;             /* backup (0:no 1:yes)              */
    u_char      dr_flag;            /* flag of dr system change         */
                    /* 0:init 1:make dr data 2:make end 3:end dr change */
    char        process_type[4];    /* process type (PS, TR2, DD, ...)  */
    int         if_seq;             /* interface sequence number        */
    int         counter_seq;        /* counter sequence (for DR use)    */
    u_char      type;       /* type (1:mp 2:dd 3:x.25 4:tcp1 5:tcp2)    */
    LINE_INFO   l;                  /* line information                 */
    short       in_f[3];            /* input file position              */
    short       out_f[99];          /* output file position             */
    short       in_d[3];            /* input data SHM position          */
    short       out_d[99];          /* output data SHM position         */
#if defined ISAM_INCL
    short       in_c[3];            /* input c-isam file position       */
    short       out_c[3];           /* output c-isam file position      */
#endif
    char        fifo_f[3][20];      /* fifo file name                   */
    char        date[10];           /* business date (yyyymmdd)         */
    char        tr_s_tm[6];         /* first transaction time (hhmmss)  */
    char        tr_e_tm[6];         /* last transaction time (hhmmss)   */
    char        error_cd[4];        /* last error code                  */
    char        error_tm[6];        /* last error time (hhmmss)         */
    char        ip[20];             /* client IP (TCP1)                 */
    char        last_tr[11];        /* Last Send,Recv TR Code Copy      */
    char        logon_id[10];       /* Logon ID                         */
    char        logon_pw[30];       /* Logon Password                   */
    short       session_stat;       /* Session, En/Decrypt Routine Status   */
                                    /* 0 : Nomal                        */
                                    /* 2 : Session Key X, En/Decrypt:X  */
    u_short     port;               /* client port (TCP1)               */
    u_char      data_flag;          /* use SHM data buffer (0:no 1:yes) */
    u_char      data_cnt;           /* number of SHM data buffer        */
    char        *data;              /* SHM data                         */
}   PROCESS_INFO;   /* Process (PROC)   */

typedef struct {
    int     process_no;             /* process number (pid)             */
    char    process_id[20];         /* process id (name)                */
    char    start_time[6];          /* process start time (hhmm)        */
    char    end_time[6];            /* process end time (hhmm)          */
    u_char  system_status;          /* system status (0:off 1:on)       */
    u_char  process_status;         /* running status                   */
                                    /* (0:not run 1:run 2:end 3:stop)   */
    u_char  check_status;           /* check status (1:SHM re-created)  */
    char    start_FIFO_name[20];    /* start FIFO (e.g. ja_FIFO)        */
    char    exit_FIFO_name[20];     /* exit FIFO (e.g. ja_FIFO_exit)    */
    char    daemon_FIFO_name[20];   /* daemon FIFO (e.g. ja_FIFO_ctrl)  */
    short   process_count;          /* process count                    */
    short   file_count;             /* file count                       */
    short   dshm_count;             /* data SHM count                   */
#if defined ISAM_INCL
    short   cisam_count;            /* cisam count                      */
#endif
    short   tcp1_count;             /* tcp1 count                       */
    short   tcp2_count;             /* tcp2 count                       */
    short   udpip_count;            /* udpip count                      */
    short   sisetr_count;           /* sisetr count                     */
    short   accno_count;            /* account count                    */
    short   data_count; /* number of processes to use SHM data buffer   */
    char    date[10];               /* work date (yyyymmdd)             */
    u_char  date_flag;              /* work date flag (아래 참조)       */
    u_char  compact_days;           /* compact days                     */
    u_char  shm_log;                /* 1:use SHM (delayed) log          */
}   ALL_DAEMON_INFO;    /* All Daemon (INFO)    */

/*----------------------------------------------------------------------*/
/* date_flag (1:D~D 2:D~D+1 3:D+1~D+1 4:D-1~D 5:D-1~D-1)                */
/*                                                                      */
/* D = work date                                                        */
/*   00:00              00:00               00:00               00:00   */
/*     |     < D - 1 >    |       < D >       |     < D + 1 >     |     */
/* ----+------------------+-------------------+-------------------+---- */
/*                          |_______1_______|                           */
/*                |_______4_______|   |_______2_______|                 */
/*       |_______5_______|                      |_______3_______|       */
/*----------------------------------------------------------------------*/
typedef struct {
    int     process_no;             /* process number (pid)             */
    char    process_path[100];      /* process path                     */
    char    process_id[20];         /* process id                       */
    char    start_time[6];          /* process start time (hhmm)        */
    char    end_time[6];            /* process end time (hhmm)          */
    u_char  system_status;          /* system mode (0:off 1:on)         */
    u_char  process_status;         /* running status                   */
                                    /* (0:not run 1:run 2:end 3:stop)   */
    char    start_FIFO_name[20];    /* start FIFO name                  */
    char    exit_FIFO_name[20];     /* exit FIFO name                   */
    char    daemon_FIFO_name[20];   /* daemon FIFO name                 */
    short   process_count;          /* process count                    */
    short   p_count;                /* number of processes              */
    size_t  Shmsize;                /* process SHM size                 */
    short   file_count;             /* file count                       */
    short   f_count;                /* number of files                  */
    size_t  FShmsize;               /* file SHM size                    */
    short   dshm_count;             /* data SHM count                   */
    short   d_count;                /* number of data SHM               */
    size_t  DShmsize;               /* data SHM size                    */
#if defined ISAM_INCL
    short   cisam_count;            /* cisam count                      */
    short   c_count;                /* number of files                  */
    size_t  CShmsize;               /* cisam file SHM size              */
#endif
    short   tcp1_count;             /* tcp1 count                       */
    short   t1_count;               /* number of tcp1                   */
    size_t  T1Shmsize;              /* tcp1 SHM size                    */
    short   tcp2_count;             /* tcp2 count                       */
    short   t2_count;               /* number of tcp2                   */
    size_t  T2Shmsize;              /* tcp2 SHM size                    */
    short   udpip_count;            /* udpip count                      */
    short   u_count;                /* number of udpip                  */
    size_t  UShmsize;               /* udpip SHM size                   */
    short   sisetr_count;           /* sisetr count                     */
    short   s_count;                /* number of sisetr                 */
    size_t  SShmsize;               /* sisetr SHM size                  */
    short   accno_count;            /* account count                    */
    short   a_count;                /* number of account                */
    size_t  AShmsize;               /* account SHM size                 */
    char    process_info[40];       /* daemon information               */
    short   data_count; /* number of processes to use SHM data buffer   */
    char    date[10];               /* work date (yyyymmdd)             */
    u_char  date_flag;              /* work date flag (위 참조)            */
    u_char  compact_days;           /* compact days                     */
    u_char  shm_log;                /* 1:use SHM (delayed) log          */
    int     w_cnt;                  /* log write count                  */
    int     r_cnt;                  /* log read count                   */
}   SUB_DAEMON_INFO;    /* Sub Daemon (DAEMON)  */

/*************************************************************************
    계좌정보
*************************************************************************/
typedef struct
{
    char    info[40];               /* information, ACCNO의 Comment      */
    char    aptype_code[2];         /* ApType의 대표 값 (2)             */
    char    acc_no[12];             /* 계좌번호 (9)                     */
    char    login_pwd[8];           /* Login 비밀번호                   */
    char    athrty[3];  /* 사용자권한(D:Dealer M:Manager)+ApTypeCode(2)  */
    char    ip_addr[12];            /* 자동주문송신시사용                */
    char    auto_man_cnt[22];       /* 자동주문권한                       */
    char    auto_use_cnt[22];       /* 자동주문사용현황(cfg불필요)     */

    /* ******************************************** */
    /* 주문번호대역(서버로 나누어야함)
        - 주식 : 1530000000 ~ 1559999999 (3천만건)
        - 파생 : 1580000000 ~ 1609999999 (3천만건)   */
    /* ******************************************** */
    int     js_order_no_band;       /* 자동주문용현물주문번호          */
    int     dv_order_no_band;       /* 자동주문용파생주문번호          */
#if 0
    int     js_order_no_band_b;     /* 자동주문용지수선물번호Client통보  */
    int     acc_order_no;           /* 계좌주문번호최종값                */
                                    /* client에서 자동주문시 사용할 주문번호 최정번호 + 10 */
                                    /* Login시 OutPut값으로 활용      */
#endif

/* 202109 */
    /* 손실한도는 계좌단위 */
    /* 수량제한은 현물/선물/옵션 단위(단 상황에 따라서 시장별로 처리) */
    double  acc_risk_max;           /* 계좌별 손실 한도액               */

    /* 계좌는 현물, 파생 계좌가 구분된다. 따라서 현물통합, 파생통합으로 사용되게 된다        */
    /* 선물1계약과 옵션1계약의 금액차이가 크기 때문에 선물과 옵션의 max수량을 각각 정한다.    */
    /* 또한 현물은 옵션수량항목으로 사용한다                                             */
/* => 종목단위로 처리할 수 밖에 없다 (합산에 문제발생여지 있음), 주문수량 */
    /* 선물 수량 */
    int     acc_f_order_maxcnt;     /* 계좌별 주문최대수량(미체결주문+잔고(절대값)+신규주문)*/
    int     acc_f_get_maxcnt;       /* 계좌별 잔고최대수량(절대값)      */
#if 0
    int     acc_f_order_currcnt;    /* cfg미설정,계좌별 주문현재수량(미체결주문+잔고(절대값))     */
    int     acc_f_get_currcnt;      /* cfg미설정,계좌별 잔고현재수량(절대값)       */
#endif
    /* 현물 또는 옵션 수량 */
    int     acc_so_order_maxcnt;    /* 계좌별 주문최대수량(미체결주문+잔고(절대값)+신규주문)*/
    int     acc_so_get_maxcnt;      /* 계좌별 잔고최대수량(절대값)      */
#if 0
    int     acc_so_order_currcnt;   /* cfg미설정,계좌별 주문현재수량(미체결주문+잔고(절대값))     */
    int     acc_so_get_currcnt;     /* cfg미설정,계좌별 잔고현재수량(절대값)       */
#endif


/* 여기부터는 cfg load와 무관하게 관리용이다 */
    /* OverNight 잔고에 대한 값, 배치에서 처리하며 고정값이다 */
    double  over_acc_ver_prft;      /* OverNight 보유종목의 평가손익 */
    int     over_sf_order_currcnt;  /* 계좌별 주문현재수량(미체결주문+잔고(절대값))        */
    int     over_o_order_currcnt;   /* 계좌별 주문현재수량(미체결주문+잔고(절대값))        */

    /* 총손익 관련 */
    double  acc_real_prft;          /* 실질순익(부호있음)               */
    double  acc_fee;                /* 계좌수수료                        */
    double  acc_ver_prft;           /* 평가손익(부호있음)               */
                    /* Curr의 현재가-이전현재가의 계산된 값만 증감처리  */

    int     risk_flag;  /* 계좌별한도발생여부이력('0':초기값 '1':발생)  */

    /* 미체결 건수 관리 */
//  int     acc_miche_cnt;          /* 미체결내역 건수(계좌별)            */

    /* 전략기동여부등 전략관련 */
    int     auto_run;                   /* 0:종료, 1:기동               */
    char    auto_set[40];               /* 자동주문 설정값              */
                                        /* 0 : 1~6, 콜풋 구분           */
                                        /* 1 : 0~7, 8가지 경우의수      */

#if 0   // 202109
    int     acc_risk_max;           /* 계좌별 손실 한도액               */
    int     js_order_maxcnt;        /* 선물최대수량                       */
    int     jo_order_maxcnt;        /* 옵션최대수량                       */
/* MM 200809 */
    int     medo_man_cnt;           /* 매도수량 관리용 지정  수량      */
    int     medo_use_cnt;           /* 매도수량 사용자 사용 수량       */
    int     mesu_man_cnt;           /* 매수수량 관리용 지정  수량      */
    int     mesu_use_cnt;           /* 매수수량 사용자 사용 수량       */

    int     medo_man_money;         /* 매도 총 관리용 보유 금액(단위 백원)*/
    int     medo_use_money;         /* 매도 총 사용자 보유 금액(단위 백원)*/
    int     mesu_man_money;         /* 매수 총 관리용 보유 금액(단위 백원)*/
    int     mesu_use_money;         /* 매수 총 사용자 보유 금액(단위 백원)*/

    int     over_acc_ver_prft;      /* OverNight 보유종목의 평가손익 */
    int     over_medo_cnt;          /* OverNight 매도 Count               */
    int     over_mesu_cnt;          /* OverNight 매수 Count               */
    int     over_medo_money;        /* OverNight 매수 보유금액(단위 백원)*/
    int     over_mesu_money;        /* OverNight 매수 보유금액(단위 백원)*/
/* MM 200809 */

    int     acc_real_prft;          /* 실질순익(부호있음)               */
    int     acc_fee;                /* 계좌수수료                        */
    int     acc_ver_prft;           /* 평가손익(부호있음)               */
                    /* Curr의 현재가-이전현재가의 계산된 값만 증감처리  */

    int     risk_flag;  /* 계좌별한도발생여부이력('0':초기값 '1':발생)  */
/* MM 200809 */
    int     etc_risk_flag;          /* 기타 한도 Check 처리(초기값:'0')  */
                                    /* 1:매도수량, 2:매수수량           */
                                    /* 3:매도금액, 4:매수금액           */
/* MM 200809 */
/* 2010 4월 미체결내역관리 추가 */
    int     miche_cnt;              /* 미체결내역 건수(계좌별)            */
/* 2010 4월 미체결내역관리 추가 */
    char    filler[2];              /* 예비                               */
    char    info[40];               /* information, ACCNO의 Comment      */
#endif
}   ACCNO_INFO; /* 계좌정보 */

typedef struct {
    int             start_FIFO_fd;  /* file descriptor of start FIFO    */
    int             exit_FIFO_fd;   /* file descriptor of exit FIFO     */
    int             daemon_FIFO_fd; /* file descriptor of daemon FIFO   */
    SUB_DAEMON_INFO *Daemon;        /* address of daemon SHM            */
    PROCESS_INFO    *Proc;          /* address of process SHM           */
    FILE_INFO       *File;          /* address of file SHM              */
    DSHM_INFO       *DShm;          /* address of data SHM              */
#if defined ISAM_INCL
    CISAM_INFO      *Cisam;         /* address of cisam SHM             */
#endif
    TCP1_INFO       *Tcp1;          /* address of tcp1 SHM              */
    TCP2_INFO       *Tcp2;          /* address of tcp2 SHM              */
    UDPIP_INFO      *Udpip;         /* address of udpip SHM             */
    SISETR_INFO     *Sisetr;        /* address of sisetr SHM            */
    ACCNO_INFO      *Accno;         /* address of account SHM           */
}   SHM_MEMORY; /* Shared Memory    */


/* ********************************************************************* */
/* 시장별 공유메모리 설정 */
/* ********************************************************************* */

/* 20211020 */
/* 공통시세 구축 A0+A3등, 호가정합성 체크를 위해 */
typedef struct                      
{                       
    int             auto_use;                   /* 자동전략기동시 해당종목 +- 설정 */
    int             dont_trade;                 /* 매매불가종목정보(아침에 받고, 실시간으로 수정된거 받기   */
    /*         거래승수                 거래단위
    ksd150   9999999999999.99999999(21,13V8), 999999999.99999999(17,9V8)
    krx300   9999999999999.99999999(21,13V8), 999999999.99999999(17,9V8)
    개별주식 9999999999999.99999999(21,13V8), 999999999.99999999(17,9V8)
    */
    /* **************************************************************************** */
    /* A0, Master Data */
    char            m_item_cd[12];                  /* Item Full Code               */
    double          m_h_lmt;                        /* Hi Limit                     */
    double          m_l_lmt;                        /* Low Limit                    */
    char            m_item_stat[1];                 /* Item Status , 0:Normal, !0:Cannot*/
    char            item_stat[1];                   /* 종목장운영정보의 종목거래가능 여부, Item Status 0:Normal, !0:Cannot  */
    double          m_stdard_price;                 /* Standard Price               */
                                                    /* 장중종목정보                   */
    char            nb_shares_stock_1per[10];       /* 상장주식수의 1%, shares issued(stock cnt)  */
    // Add, 20200506
    int             m_cds_gbn_1;                    /* 시장가 호가조건코드(현물용) */
    int             m_cds_gbn_2;                    /* 지정가 호가조건코드 */
    int             m_cds_gbn_I;                    /* 조건부지정가 호가조건코드 */
    int             m_cds_gbn_X;                    /* 최유리지정가 호가조건코드(현물용) */
    int             m_cds_gbn_Y;                    /* 최우선지정가 호가조건코드 */
    int             m_cds_gbn_T;                    /* 시장가 호가조건코드(1대신 파생용) */
    int             m_cds_gbn_W;                    /* 최유리지정가 호가조건코드(X대신 파생용) */
    double          m_max_qty;                      /* 상한수량 */
    char            m_clearance_gbn[1];             /* Y:정리매매, N:일반     */
    char            m_start_price_gbn[1];           /* Y:시가기준가종목, N:일반  */
    double          m_top_price;                    /* 최고호가 가격          */
    double          m_lowest_price;                 /* 최저호가 가격          */
    // Add, 20200602
    int             m_multiplier;                   /* 승수, 현물은 1로 처리    */
	double			m_striking_price;				/* 행사가, 파생만사용,현물1	17(9V8)중 11자리 소수점2자리까지만 사용 */
//  float           indv_rate;                      /* 위탁 증거금율, 설정      */
    // Add, 20200609, 파생만
    char            m_closeday[8];                  /* 거래종료일                */
    int             sf_mk_gbn;                      /* 주식선물 마켓구분, 0:초기값, 5:유가증권, 6:코스닥 */
    double          basic_asset_price;              /* 기초자산 전일종가        */
    char            m_group_id[2];                  /* 증권그룹ID               */
    /* **************************************************************************** */
                                                
    /* **************************************************************************** */
    /* Market Recv Data */                      
#if 0
    char            crprc[11];                  /* Matching Price       */                      
    char            sell_1_price[11];           /* Sell 1 Price         */                      
    char            buy_1_price[11];            /* Buy 1 Price          */                      
    char            realtime_hprc[11];          /* Runnig Time Hi Limit */
    char            realtime_lprc[11];          /* Runnig Time Low Limit*/
#endif
    double          crprc;                      /* Matching Price       */                      
    double          sell_1_price;               /* Sell 1 Price         */                      
    double          buy_1_price;                /* Buy 1 Price          */                      
    double          realtime_hprc;              /* Runnig Time Hi Limit , run_tm_h_lmt  */
    double          realtime_lprc;              /* Runnig Time Low Limit, run_tm_l_lmt  */
    /* **************************************************************************** */
}   STANDARD_SISE_FORMAT;                       


/* 현물 */
typedef struct
{
    STOCK_A3011     a3;
    STOCK_B6011     b6;
}   STOCK_CURR;

typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */

    int             check_cnt;              /* 보유여부 확인(평가금액용)    */
                                            /* 해당종목을 보유한 관리계좌가 있는지, 평가금액 확인 때문에 */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */

    int             CURR_Arry_Key;          /* 최근 시세내역 30개의 Key */
    int             Befor_CURR_Arry_Key;    /* CURR_Arry_Key 직전값기억 */

    int             HogaLastGbn;            /* 0:A3, 1:B6               */
//  int             A1A0_seq;               /* elw A1011의 일련번호 Set, A1011수신시 처리 */
//  int             I7A0_seq;               /* elw I7011의 일련번호 Set, A1011수신시 처리 */

    M4011           M4;             /* 장운영정보, seq는 0만사용 */

    STOCK_A0011     A0;             /* 유가증권종목배치(Kospi용)*/
    STOCK_A3011     A3;             /* 유가증권체결               */
    STOCK_B6011     B6;             /* 유가증권호가               */
    STOCK_A4011     A4;             /* 기준가결정                */
    STOCK_B7011     B7;             /* B7011 ETF,ETN 호가     */
    STOCK_B8011     B8;             /* 장개시전 호가잔량(총만)    */
    R8011           R8;             /* 코스피종목상태정보        */

    /* ETW, 종목일련번호 Search 필요 */
    ELW_A1011       ELW_A1;         /* A1(ELW) 종목배치 */
    STOCK_A3011     ELW_A3;         /* A3021(ELW)체결         */
    STOCK_B7021     ELW_B7;         /* B7021(ELW)호가         */

    /* ETN, 종목일련번호 Search 필요 */
    STOCK_A1041     ETN_A1;         /* A1(ETN) 종목배치             */
    STOCK_S1011     S1;             /* S1(ETN) 사무수탁정보           */
    STOCK_S3011     S3;             /* S3(ETN) IIV(실시간지표가치) */

    /* ETF, 종목일련번호 Search 필요 */
    BV011           BV;             /* ETF NAV                  */
    BW011           BW;             /* ETF 예상NAV                */
    L5011           L5;             /* 해외지수 ETF NAV         */
//  STOCK_A3011     CURR_Arry[30];  /* CURR Data                */
}   SHM_STOCK;  /* 유가증권시세 Shared Memory */

typedef struct
{
    int             total_item_cnt;         /* 종목수, seq 0에만 사용. */

    int             check_cnt;              /* 보유여부 확인(평가금액용)*/
                                            /* 해당종목을 보유한 관리계좌가 있는지, 평가금액 확인 때문에 */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */

    int             CURR_Arry_Key;          /* 최근 시세내역 30개의 Key */
    int             Befor_CURR_Arry_Key;    /* CURR_Arry_Key 직전값기억 */

    int             HogaLastGbn;            /* 0:A3, 1:B6               */

    M4011           M4;             /* 장운영정보, seq는 0만사용 */

    STOCK_A0011     A0;                 /* 코스닥 종목배치         */
    STOCK_A3011     A3;                 /* 코스닥 체결               */
    STOCK_A4011     A4;                 /* 기준가 결정               */
    STOCK_B6011     B6;                 /* 코스닥 호가               */
    STOCK_B8011     B8;                 /* 장개시전 호가잔량(총만)    */

    STOCK_A3011     CURR_Arry[30];      /* CURR Data                */
}   SHM_KOSDAQ; /* 코스닥시세 Shared Memory */

#if 0
/* ELW */
typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */

    int             A0A1_seq;
    int             A0I7_seq;

    ELW_A1011       A1;                         /* A1(ELW) 종목배치 */
    ELW_I7011       I7;                         /* I7(ELW) LP정보 */
                                        /* A3021(체결) + B7011(호가)    */
}   SHM_ELW;    /* ELW시세 Shared Memory */

/* ETN */
typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */

    int             A0A1_seq;

                                        /* A3021(체결) + B7011(호가)    */
}   SHM_ETN;    /* ELW시세 Shared Memory */
#endif
/* KOSPI지수(D0)
 * KOSPI예상지수(D1)
 * KOSDAQ지수(E4)
 * KOSDAQ예상지수(E5)
 * KOSPI200지수(D2)
 * KOSPI200예상지수( D3)
 */
typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */

    STOCK_JISU      Data;                   /* 지수값      */
}   SHM_JISU;   /* 거래소 (장내) 시세 Shared Memory */

/*************************************************************************
        선물옵션 시세
*************************************************************************/
typedef  struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */

    int             check_cnt;              /* 보유여부 확인(평가금액용)    */
                                            /* 해당종목을 보유한 관리계좌가 있는지, 평가금액 확인 때문에 */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */

    int             CURR_Arry_Key;          /* 최근 시세내역 30개의 Key    */
                                            /* 시세내역 30개 기억에만 사용 */
    int             Befor_CURR_Arry_Key;    /* CURR_Arry_Key 이전값 기억 */

    int             HogaLastGbn;            /* 0:G7, 1:B6               */

    M4014           M4;             /* 장운영정보, seq는 0만사용 */

    SIF_A0014       A0;                 /* 기본                     */
    SIF_B6014       B6;                 /* 호가                     */
    SIF_A3014       A3;                 /* 체결                     */
    SIF_G7014       G7;                 /* 체결 + 호가              */
    V1014           V1;
    Q2014           Q2;

    SIF_G7014       CURR_Arry[30];      /* 최근 시세내역 30개 기억   */
}   SHM_FUTURES;    /* 지수선물 시세 Shared Memory, 014  */

typedef  struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */

    int             check_cnt;              /* 보유여부 확인(평가금액용)    */
                                            /* 해당종목을 보유한 관리계좌가 있는지, 평가금액 확인 때문에 */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */

    int             CURR_Arry_Key;          /* 최근 시세내역 30개의 Key    */
                                            /* 시세내역 30개 기억에만 사용 */
    int             Befor_CURR_Arry_Key;    /* CURR_Arry_Key 이전값 기억 */

    int             HogaLastGbn;            /* 0:G7, 1:B6               */

    M4014           M4;             /* 장운영정보, seq는 0만사용 */

    SIF_A0014       A0;                 /* 기본                         */
    SSF_B6015       B6;                 /* 호가                         */
    SSF_A3015       A3;                 /* 체결                         */
    SSF_G7015       G7;                 /* 체결 + 호가                  */
    V1015           V1;
    Q2015           Q2;

    SSF_G7015       CURR_Arry[30];      /* 최근 시세내역 30개 기억     */
}   SHM_STOCK_FUTURES;  /* 주식선물 시세 Shared Memory, 015  */

/*************************************************************************
        지수옵션 시세
*************************************************************************/
typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */

    int             check_cnt;              /* 보유여부 확인(평가금액용)    */
                                            /* 해당종목을 보유한 관리계좌가 있는지, 평가금액 확인 때문에 */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */

    int             CURR_Arry_Key;          /* 최근 시세내역 30개의 Key    */
                                            /* 시세내역 30개 기억에만 사용 */
    int             Befor_CURR_Arry_Key;    /* CURR_Arry_Key 이전값 기억 */

    int             HogaLastGbn;            /* 0:G7, 1:B6               */

    M4034           M4;             /* 장운영정보, seq는 0만사용 */

    SIF_A0014       A0;             /* 기본                         */
    SIO_B6034       B6;             /* 호가                         */
    SIO_A3034       A3;             /* 체결                         */
    SIO_G7034       G7;             /* 체결 + 호가                  */
    V1034           V1;
    Q2034           Q2;

    SIO_G7034       CURR_Arry[30];  /* 최근 시세내역 30개 기억     */
}   SHM_OPTIONS;    /* 지수옵션 시세 Shared Memory, 034  */

typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */

    int             check_cnt;              /* 보유여부 확인(평가금액용)    */
                                            /* 해당종목을 보유한 관리계좌가 있는지, 평가금액 확인 때문에 */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */

    int             CURR_Arry_Key;          /* 최근 시세내역 30개의 Key    */
                                            /* 시세내역 30개 기억에만 사용 */
    int             Befor_CURR_Arry_Key;    /* CURR_Arry_Key 이전값 기억 */

    int             HogaLastGbn;            /* 0:G7, 1:B6               */

    M4025           M4;             /* 장운영정보, seq는 0만사용 */

    SIF_A0014       A0;                     /* 기본                         */
    SSO_B6025       B6;                     /* 호가                         */
    SSO_A3025       A3;                     /* 체결                         */
    SSO_G7025       G7;                     /* 체결 + 호가                  */
    V1025           V1;

    SSO_G7025       CURR_Arry[30];          /* 최근 시세내역 30개 기억     */
}   SHM_STOCK_OPTIONS;  /* 주식옵션 시세 Shared Memory, 025  */

typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */

    int             check_cnt;              /* 보유여부 확인(평가금액용)    */
                                            /* 해당종목을 보유한 관리계좌가 있는지, 평가금액  인 때문에 */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */

    int             CURR_Arry_Key;          /* 최근 시세내역 30개의 Key    */
                                            /* 시세내역 30개 기억에만 사용 */
    int             Befor_CURR_Arry_Key;    /* CURR_Arry_Key 이전값 기억 */

    int             HogaLastGbn;            /* 0:G7, 1:B6               */

    M4164           M4;             /* 장운영정보, seq는 0만사용 */

    SIF_A0014       A0;                     /* 기본                         */
    B6164           B6;                     /* 호가                         */
    A3164           A3;                     /* 체결                         */
    G7164           G7;                     /* 체결 + 호가                  */
    V1164           V1;

//  G7164           CURR_Arry[30];          /* 최근 시세내역 30개 기억     */
}   SHM_K300;   /* KRX300 시세 Shared Memory, 164  */

typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */

    int             check_cnt;              /* 보유여부 확인(평가금액용)    */
                                            /* 해당종목을 보유한 관리계좌가 있는지, 평가금액 확인 때문에 */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */

    int             CURR_Arry_Key;          /* 최근 시세내역 30개의 Key    */
                                            /* 시세내역 30개 기억에만 사용 */
    int             Befor_CURR_Arry_Key;    /* CURR_Arry_Key 이전값 기억 */

    int             HogaLastGbn;            /* 0:G7, 1:B6               */

    M4024           M4;             /* 장운영정보, seq는 0만사용 */

    SIF_A0014       A0;                     /* 기본                         */
    B6024           B6;                     /* 호가                         */
    A3024           A3;                     /* 체결                         */
    G7024           G7;                     /* 체결 + 호가                  */
    V1024           V1;

//  SSO_G7024       CURR_Arry[30];          /* 최근 시세내역 30개 기억     */
}   SHM_K150F;  /* KRX150F 시세 Shared Memory, 024  */

typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */

    int             check_cnt;              /* 보유여부 확인(평가금액용)    */
                                            /* 해당종목을 보유한 관리계좌가 있는지, 평가금액 확인 때문에 */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */

    int             CURR_Arry_Key;          /* 최근 시세내역 30개의 Key    */
                                            /* 시세내역 30개 기억에만 사용 */
    int             Befor_CURR_Arry_Key;    /* CURR_Arry_Key 이전값 기억 */

    int             HogaLastGbn;            /* 0:G7, 1:B6               */

    M4174           M4;             /* 장운영정보, seq는 0만사용 */

    SIF_A0014       A0;                     /* 기본                         */
    B6174           B6;                     /* 호가                         */
    A3174           A3;                     /* 체결                         */
    G7174           G7;                     /* 체결 + 호가                  */
    V1174           V1;

//  SSO_G7174       CURR_Arry[30];          /* 최근 시세내역 30개 기억     */
}   SHM_K150O;  /* KRX150O 시세 Shared Memory, 174  */

typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */

    int             check_cnt;              /* 보유여부 확인(평가금액용)    */
                                            /* 해당종목을 보유한 관리계좌가 있는지, 평가금액 확인 때문에 */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */

    int             CURR_Arry_Key;          /* 최근 시세내역 30개의 Key    */
                                            /* 시세내역 30개 기억에만 사용 */
    int             Befor_CURR_Arry_Key;    /* CURR_Arry_Key 이전값 기억 */

    int             HogaLastGbn;            /* 0:G7, 1:B6               */

    M4014           M4;             /* 장운영정보, seq는 0만사용 */

    SIF_A0014       A0;                 /* 기본                     */
    SIF_B6014       B6;                 /* 호가                     */
    SIF_A3014       A3;                 /* 체결                     */
    SIF_G7014       G7;                 /* 체결 + 호가              */
    V1014           V1;
    Q2014           Q2;

}   SHM_MF;     /* 미니선물  */
typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */

    int             check_cnt;              /* 보유여부 확인(평가금액용)    */
                                            /* 해당종목을 보유한 관리계좌가 있는지, 평가금액 확인 때문에 */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */

    int             CURR_Arry_Key;          /* 최근 시세내역 30개의 Key    */
                                            /* 시세내역 30개 기억에만 사용 */
    int             Befor_CURR_Arry_Key;    /* CURR_Arry_Key 이전값 기억 */

    int             HogaLastGbn;            /* 0:G7, 1:B6               */

    M4034           M4;             /* 장운영정보, seq는 0만사용 */

    SIF_A0014       A0;             /* 기본                         */
    SIO_B6034       B6;             /* 호가                         */
    SIO_A3034       A3;             /* 체결                         */
    SIO_G7034       G7;             /* 체결 + 호가                  */
    V1034           V1;
    Q2034           Q2;

    SSO_G7025       CURR_Arry[30];          /* 최근 시세내역 30개 기억     */
}   SHM_MO;     /* 미니옵션  */

/*************************************************************************
    Over Sea SISE (CME/CMX/SGX/ERX/HKE)
*************************************************************************/
typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */
    OC_MASTER       A0;                     /* Master       */
    OC_Q            Q;                      /* 현재가      */
    OC_B            B;                      /* 호가           */
    OC_S            S;                      /* 정산가      */
}   SHM_CME;

typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */
    OC_MASTER       A0;                     /* Master       */
    OC_Q            Q;                      /* 현재가      */
    OC_B            B;                      /* 호가           */
    OC_S            S;                      /* 정산가      */
}   SHM_CMX;

typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */
    OC_MASTER       A0;                     /* Master       */
    OC_Q            Q;                      /* 현재가      */
    OC_B            B;                      /* 호가           */
    OC_S            S;                      /* 정산가      */
}   SHM_SGX;

typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */
    OC_MASTER       A0;                     /* Master       */
    OC_Q            Q;                      /* 현재가      */
    OC_B            B;                      /* 호가           */
    OC_S            S;                      /* 정산가      */
}   SHM_ERX;

typedef struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */
    OC_MASTER       A0;                     /* Master       */
    OC_Q            Q;                      /* 현재가      */
    OC_B            B;                      /* 호가           */
    OC_S            S;                      /* 정산가      */
}   SHM_HKE;

/*************************************************************************
    MiChe Data Struct
*************************************************************************/
typedef struct {
    int     Jan_Cnt;                            /* 주문잔량(절대값)        */
    int     Mk_gbn;                             /* 시장구분             */
                                                /* 1(지수선물), 2(지수옵션), 3(주식선물), 4(주식옵션)
                                                   5(유가증권/ELW/ETF/ETN), 6(코스닥), 8(KRX300), 9(Kosdaq150F) 10(Kosdaq150O)*/
    int     Item_Seq;                           /* 종목코드 Master Seq  */
    int     Acc_Seq;                            /* 계좌번호 Seq         */
    int     Str_No;                             /* 전략번호 Seq         */

/* 20220204 */
	double	Order_Price_dv;						/* 8. 주문가격(파생용)	*/

    char    AccountNo[12];                      /* 1.계좌번호           */
    char    Item_Cd[12];                        /* 2. 종목코드          */
    char    OrderNo[10];                        /* 3. 주문번호          */
    char    OriginalOrderNo[10];                /* 4. 원주문번호        */
    char    OrderFlag[1];                       /* 5. 정정취소구분      */
                                            /* (0:신규,1:정정,2:취소)   */
    char    TradeFlag[1];                       /* 6. 매도매수구분      */
    char    Order_Cnt[8];                       /* 7. 주문수량          */
    char    Order_Price[9];                     /* 8. 주문가격(현물용)	*/
    char    OrderType[1];                       /* 9. 주문유형          */
                                                /* 'B', '최유리지정가'  */
                                                /* 'C', '조건부지정가'  */
                                                /* 'L', '지정가'        */
                                                /* 'M', '시장가'        */
                                                /* '0' ,원주문이 있을때 */
    char    JumunFlag[1];                       /* 10.호가조건코드        */
                                                /* F:FOK, I:IOC, X:없음 */
    char    MembershipItem[60];                 /* 11.회원사처리항목   */
}   MICHE;

/* 종목별 (평가손익/수량) 손익/한도 관리 */
typedef struct
{
    long    item_getcnt;                        /* 계좌별 보유종목수량(부호있음), 현선물공통 */
                            /* 최초 원장에서 받고 매매시 업데이트한다. */
	long	item_getmoney;                      /* 장부금액(수량*평단), 현물만(파생은 item_get_avg_price사용) */
                            /* 최초 원장에서 받고 업데이트안한다. */
    long    item_tot_sugum;                     /* 당일 매수누적금액, 현물만사용    */
    long    item_cha_sugum;                     /* 당일 매수차감금액, 현물만사용    */

    /* 전략에서 사용용 */
    long    item_borrow_cnt;                    /* 차입수량                         */
    long    item_totsu_che;                     /* 매수누적체결, 전략에서 사용      */
    long    item_totdo_che;                     /* 매도누적체결, 전략에서 사용      */

	/* 20220204 */
    double  item_su_miche_gum;					/* 매수미체결 합산금액, 파생만사용	*/
    double  item_do_miche_gum;					/* 매도미체결 합산금액, 파생만사용	*/
    int     item_su_michecnt;					/* 매수미체결 수량    , 파생만사용	*/
    int     item_do_michecnt;					/* 매도미체결 수량, 현파모두사용 	*/
												/* 현물은 공매도 체크용으로 사용	*/
    double  item_get_avg_price;                 /* 종목별 평균매입단가, 파생만사용	*/
	/* 20220204 */

#if 0
/* 20211209 */
    int     item_jucnt;                         /* 주문나간 수량(절대값), 수량한도,미체결수량 */

    double  item_get_avg_price;                 /* 종목별 평균매입단가               */
    double  item_ver_prft;                      /* 종목별 평가손익(부호있음)       */
    double  item_real_prft;                     /* 종목별 평가손익(부호있음)       */
#endif
}   PROFIT;

/* ******************************************************************** */
/* 전략처리를 위한 SHM                                                 */
/* ******************************************************************** */
/* 처리절차 500100/500110/500120 / 500200/500210/500220 / 500300/500310/500320 / 500410 */
/*          500100이후에는 ApType(기동Process번호 ex)50101 를 Key로 처리한다) Client는 반드시 관리하고 있어야 컨트롤이 가능하다 */
/* 1. Client는 500100(자동주문기동요청)을 보내면 
      9001mp로 보내서 기동할 Process를 선택하여 ApType을 설정하고 Process를 기동시켜주고 8101ts에 ApType을 응답(성공시500110, 오류시500120)으로 보내준다. */
/* 2. 응답을 수신받은 Client는 500200(자동주문조건요청)을 전송한다. 이때 오류가 발생하면 전략Process는 8101ts에 500220(자동기동오류)/500210(자동기동성공)를 보낸다. */
/* 2-1. 500200(자동주문조건요청)일 여러건 보낼경우 S(Start)/N(Next)/E(End)의 순으로 보내고 한번만 보낼때는 O(One)으로 보낸다   */
/* 3. Client는 자동종료 요청시에는 500300(자동종료요청)으로 보낸다. 성공시 500310(자동종료성공) or 500320(자동종료오류)로 보내며 오류는 기동이 아닌데 요청시반환된다. */
/* 4. 한도에 걸리면 강제종료가 발생한다. 주문송신에서 한도오류시 500410(자동강제종료)를 9001mp로 보내고 해당Process를 종료하고 Client로 송신한다. */

/* clinet와는 총 500byte를 주고받고 이중 20바이트는 헤더이다          */
/* 샘플전략은 지수선물의 체결이 매수체결이면 지정한 call옵션을 매수하고 */
/*                              매도체결이면 지정한  put옵션을 매도한다 */

typedef struct
{
    int                 auto_run_flag[ACC_NO_CNT];  /* 0:not run, 1:run */
    SHM_SAMPLE01        SamPle_St01[ACC_NO_CNT];
    LP_Strrg            Lp_St[ACC_NO_CNT];
    Basket_t            basket[ACC_NO_CNT];
    ArbtSet_t           arbtset[ACC_NO_CNT];            // 차익거래
}   STRRG;

/* 시세 수신시(평가손익) 손익 && 한도 관련 메모리 처리  */
/* 계좌별 손익한도, 수량한도는 ACCNO에서 처리함          */
typedef struct
{
    int         MeChe_Cnt[MK_CNT][ACC_NO_CNT];              /* 시장별/계좌별 미체결수량        */
    MICHE       F_MiChe[MK_CNT][ACC_NO_CNT][MAX_MICHE];     /* 미체결 내역조회             */
                                                /* 계좌단위가 아닌 시스템 전체 대상   */
}   MK_PREMATCH;        /* 종목별 한도/수량/평가손익 관리 Memory */
// SHM_DB[MK_CNT]   =>  MK_PREMATCH[MK_CNT]

/* 20211028 */
/* Accout Master */
typedef struct
{
    char    acc_no[12];                     /* Account No, KEY              */
                                            /* KRX주문전문의 계좌와 동일해야함 */
    char    mk_gbn[1];                      /* 1:STOCK  2:Derivatives  */
}   MST_ACCNO_MASTER;

typedef struct
{
#if 0
/* 20211209 */
    double  order_money;                    /* 1회 주문금액              */
    char    qty_chk_gbn[1];                 /* 1:설정수량, 2:상장주식수*설정비율 */  
    double  qty_chk;                        /* 수량 또는 상장주식수*비율 */
    char    lmt_chk_gbn[1];                 /* 1:Tick, 2:현재가*설정비율 */    
    double  lmt_chk;                        /* Tick 또는 현재가*설정비율 */  
#endif
    char    qty_gbn[1];                     /* 1회주문 계약수 체크여부    */
    long    qty;                            /* 1회주문 계약수         */
    char    money_gbn[1];                   /* 1회주문 금액 체크여부 */
    double  money;                          /* 1회주문 금액              */
    char    tick_gbn[1];                    /* 1회주문 Tick 체크엽        */
    long    tick;                           /* 1회주문 Tick(8자리)       */
}   CHK_RISK_ONE;

typedef struct
{
#if 0
/* 20211209 */
    double  order_money;                    // 주문총액
    double  qty_chk;                        /* 수량, 잔고 + 미체결주문(매도/매수) */
#endif
    char    do_cnt_gbn[1];                  /* 매도 계약수 체크여부      */
    long    do_cnt;                         /* 매도 계약수               */
    char    do_money_gbn[1];                /* 매도 금액 체크여부       */
    double  do_money;                       /* 매도 금액                */
    char    su_cnt_gbn[1];                  /* 매수 계약수 체크여부      */
    long    su_cnt;                         /* 매수 계약수               */
    char    su_money_gbn[1];                /* 매수 금액 체크여부       */
    double  su_money;                       /* 매수 금액                */
}   CHK_RISK_TOT;

/* 20211028 */

typedef struct
{
    long    START_JMNO;             /* 시작 주문번호, Process 별 주문번호 사용 대역(Band)  */
    long    USED_JMNO;              /* 사용중인 주문번호, 주문발주시 +1해서 처리함            */
    long    END_JMNO;               /* 끝  주문번호, Process 별 주문번호 사용 대역(Band)  */
}   ORDER_NO;

/* 20211102 */
typedef struct {
    int     hoga_depth;     /* arry 0에서만 사용 */
    double  band_price;
    double  band_unit;
    double  band_sum;
}   HO_CHE;
/* 20211102 */

/* 20211209 */
typedef struct {
    int     W_cnt;                  /* 수신받은 건수  */
    int     R_cnt;                  /* 처리된 건수       */
}   BATCH_CNT;

/* 모두 전략이 Set하고 공통은 조회올라오면 읽어서 보내준다(기동된것만) */
typedef struct {
    char    ApType[5];              /* ApType(ex, 50101)    */
    char    Str_No[4];              /* 전략번호(ex, 5030)   */
    char    Item_Code[12];          /* 종목코드             */
    char    Run_Gbn[1];             /* 기동여부, 1:기동, 0:미기동    */
}   AUTO_STAT;
/* 20211209 */

/* 한도관리 SHM영역 */
typedef struct
{
    char    business_day[8];
	int		diff_calday;
    char    open_market_info[MK_CNT][2];    /* G1등 시세기준 3개정보, 보드ID */
    char    sub_market_info[MK_CNT][3];     /* AA1등 모드이벤트 */
    /* ******************************************** */
    /* 주문번호대역(서버로 나누어야함)
        - 주식 : 1530000000 ~ 1559999999 (3천만건)
        - 파생 : 1580000000 ~ 1609999999 (3천만건)  */
    /* ******************************************** */
    int     js_order_no_band;   /* 현물주문번호 */
    int     dv_order_no_band;   /* 파생주문번호 */

    double  mk_hoga_dan_d[MK_CNT];          /* 시장별 1호가단위 ex) 0.01 */
    double  mk_hoga_val_d[MK_CNT];          /* 시장별 1호가 가격        */

/* 20211209 */
    double  cd_rate;                        /* CD금리, 전략에서 사용    */
    double  indv_rate[MK_CNT];              /* 위탁증거금율, KRX300만 사용   */
                                            /* 01 : 코스피200
                                               04 : 코스피200변동성지수
                                               05 : 미니코스피
                                               06 : 코스닥150
                                               07 : 유로스톡스
                                               08 : KRX300
                                               ......                   */
    BATCH_CNT       Batch_Cnt[7];           /* 배치 수신 및 처리건수 체크(아침체크)    */
    AUTO_STAT       Auto_Stat[MAX_AUTO_PROC];   /* 자동전략 상태      */
/* 20211209 */
//  double  each_price[MK_CNT][SHM_MAX_STOCK];      /* 시장별 종목별 현재가, 장전은 기준가 */
    /* ************************************************************************ */
    /* 종목별 손익/한도 관리                                                 */
    //PROFIT            ProFit[MK_CNT][ACC_NO_CNT]; /*  0:kospi,    1:kosdaq, 
    PROFIT          ProFit[MK_CNT][SHM_MAX_STOCK][ACC_NO_CNT];      /* 가장큰 종목수량이 2만이다 */
                                                /*  0:kospi,    1:kosdaq, 
                                                    2:지수선물, 3:지수옵션, 
                                                    4:주식선물, 5:주식옵션      
                                                    8:KRX30, 9:Ko150F, 10:Ko150O*/
                                                /* 1000을 나눈값으로 보관       */
    /* ************************************************************************ */
/* 20211024 */
    int     Max_Seq[MK_CNT];
    STANDARD_SISE_FORMAT            S_Sise[MK_CNT][SHM_MAX_STOCK];

/* 20211028 */
    MST_ACCNO_MASTER                Mst_Acc[ACC_NO_CNT];
    /* 시장별로 하면 MK_CNT, 시장구분없이하면 MK_CNT 필요없다 */
    CHK_RISK_ONE                    O_M_Fund[RISK_MK_CNT][ACC_NO_CNT];  // 1회한도(설정값)
    CHK_RISK_TOT                    T_M_Fund[RISK_MK_CNT][ACC_NO_CNT];  // 누적한도(설정값)
    CHK_RISK_TOT                    T_S_Fund[RISK_MK_CNT][ACC_NO_CNT];  // 누적한도 사용값
/* 20211028 */
    ORDER_NO    Order_No[2][MAX_AUTO_PROC];         /* 자동주문 프로세스(pa_50101mp) 갯수 */
                                                    /* 0:현물, 1:파생   */
/* 20211102 */
    HO_CHE      Ho_Chk[MK_CNT+2][7];
/* 20211102 */
}   RISK;       /* 종목별 한도/수량/평가손익 관리 Memory */

/* ************************************************************************************ */
/* 종목코드를 키로 하여 qsort와 bsearch를 하여 보다 빠르게 arry의 위치를 찾기 위한 처리 */
/* ************************************************************************************ */
typedef struct
{
    KS_EXPCODE      F_Key[SHM_MAX_FUTURES];     //  1.지수선물
    KS_EXPCODE      O_Key[SHM_MAX_OPTIONS];     //  2.지수옵션
    KS_EXPCODE      SF_Key[SHM_MAX_S_FUTURES];  //  3.주식선물
    KS_EXPCODE      SO_Key[SHM_MAX_S_OPTIONS];  //  4.주식옵션
    KS_EXPCODE      S_Key[SHM_MAX_STOCK];       //  5.유가증권
    KS_EXPCODE      K_Key[SHM_MAX_KOSDAQ];      //  6.코스닥
    KS_EXPCODE      K300_Key[SHM_MAX_K300];     //  8.KRX300
    KS_EXPCODE      K150F_Key[SHM_MAX_K150F];   //  9.KOSDAQ150 Futures
    KS_EXPCODE      K150O_Key[SHM_MAX_K150O];   //  10.KOSDAQ150 Options
    KS_EXPCODE      MF_Key[SHM_MAX_MF];         //  11.MINI Futures
    KS_EXPCODE      MO_Key[SHM_MAX_MO];         //  12.MINI Options
    KS_EXPCODE      J_Key[SHM_MAX_JISU];        //  0.지수
    KS_LONGCODE     CME_Key[SHM_MAX_CME];       //  31.CME
    KS_LONGCODE     CMX_Key[SHM_MAX_CMX];       //  32.CMX
    KS_LONGCODE     SGX_Key[SHM_MAX_SGX];       //  33.SGX
    KS_LONGCODE     ERX_Key[SHM_MAX_ERX];       //  34.ERX
    KS_LONGCODE     HKE_Key[SHM_MAX_HKE];       //  35.HKE
}   SHM_KEY_ARRY;

/*------------------------------------------------------------------------
    *   queue   =
        0: 미사용   TR
        1: 거래소, 거래원정보, OTCBB,   뉴스
        2: KOSDAQ, 거래소호가
        3: 선물
        4: 옵션
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Defined Constants
------------------------------------------------------------------------*/
#define     JOB_INIT        0                       /* job initialized  */
#define     JOB_START       1                       /* job started      */
#define     JOB_END         2                       /* job ended        */
#define     JOB_STOP        3                       /* job stopped      */

#define     SHM_MAX_SUB     26                              /* A ~ Z    */
#define     MAX_DSHM_SEG    50      /* max number of data SHM segments  */


#define     FILE_BUF_LEN    (8*1024)
#define     SHM_DATA_SIZE   5120
#define     DATA_BUF_CNT    30

/*------------------------------------------------------------------------
    환경 SHM key: 0x(1)(2)(3)(4)(5)(6)(7)(8)
        (1)     - 1:FEP, 2:PK system
        (2)     - 1:Real, 2:Test
        (3)(4)  - 부문. e.g. 01:거래소, 02:선물, 03:지수옵션,04:KOSDAQ, ...
        (5)(6)  - 00:부문 SHM, 01~50:data SHM segment의 base key
                  51~99:index SHM segment의 base key
        (7)(8)  - 00:data SHM, 01~99:semaphore
------------------------------------------------------------------------*/
#define     BASE_SHM_KEY    0x21000000L                 /* base SHM key */

/*------------------------------------------------------------------------
    시세 SHM key: 0x(1)(2)(3)(4)(5)(6)(7)(8)
        (1)     - 2:PK system
        (2)     - 1:Real, 2:Test
        (3)~(6) - 0000
        (7)(8)  - 01:지수선물, 02:지수옵션, 03:유가증권, 04:지수
------------------------------------------------------------------------*/
#define     F_SHM_KEY       0x21000001L                 /* 지수선물     */
#define     O_SHM_KEY       0x21000002L                 /* 지수옵션     */
#define     SF_SHM_KEY      0x21000003L                 /* 주식선물   */
#define     SO_SHM_KEY      0x21000004L                 /* 주식옵션   */
#define     S_SHM_KEY       0x21000005L                 /* 유가증권     */
#define     K_SHM_KEY       0x21000006L                 /* 코스닥증권   */
#define     K300_SHM_KEY    0x21000008L                 /* KRX300       */
#define     K150F_SHM_KEY   0x21000009L                 /* KOSDAQ150    */
#define     K150O_SHM_KEY   0x21000010L                 /* KOSDAQ150    */
#define     MF_SHM_KEY      0x21000011L                 /* 미니선물    */
#define     MO_SHM_KEY      0x21000012L                 /* 미니옵션    */
#define     J_SHM_KEY       0x21000020L                 /* 현물지수     */

#define     B_SHM_KEY       0x21000021L                 /* BackOffice   */
#define     STRRG_SHM_KEY   0x21000022L                 /* 전략SHM        */
#define     MK_PM_SHM_KEY   0x21000023L                 /* 미체결처리용   */
#define     RISK_SHM_KEY    0x21000024L                 /* 한도관리SHM  */

#define     ITEM_SHM_KEY    0x21000030L                 /* Key Sort용    */

/* item code qsort 처리용 */
#define     CME_KEY         0x21000031L                 /* CME */
#define     CMX_KEY         0x21000032L                 /* CMX */
#define     SGX_KEY         0x21000033L                 /* SGX */
#define     ERX_KEY         0x21000034L                 /* ERX */
#define     HKE_KEY         0x21000035L                 /* HKE */

#define     INFO(i)         SHM_All_Daemon_Info[i]
#define     DAEMON(i)       Shm_Mem[i].Daemon[0]
#define     PROC(i,j)       Shm_Mem[i].Proc[j]
#define     FILEM(i,j)      Shm_Mem[i].File[j]
#define     DSHM(i,j)       Shm_Mem[i].DShm[j]
#define     TCP1(i,j)       Shm_Mem[i].Tcp1[j]
#define     TCP2(i,j)       Shm_Mem[i].Tcp2[j]
#define     UDPIP(i,j)      Shm_Mem[i].Udpip[j]
#define     SISETR(i,j)     Shm_Mem[i].Sisetr[j]
#define     ACCNO(i,j)      Shm_Mem[i].Accno[j]

#if defined ISAM_INCL
#define     CISAM(i,j)      Shm_Mem[i].Cisam[j]
#endif

#define     IF_SEQ(i,j)             PROC(i,j).if_seq
#define     START_STAT(i,j)         PROC(i,j).start_status
#define     IFIFD(i,j,k)            PROC(i,j).in_FIFO_fd[k]
#define     FFN(i,j,k)              PROC(i,j).fifo_f[k]
#define     TIME_VALUE(i,j)         PROC(i,j).timeout
#define     LAST_TR(i,j)            PROC(i,j).last_tr
#define     LOGON_ID(i,j)           PROC(i,j).logon_id
#define     LOGON_PW(i,j)           PROC(i,j).logon_pw
#define     TCP1_NSTAT(i,j)         PROC(i,j).l.t1.network_status
#define     TCP2_LINE_GUBUN(i,j)    PROC(i,j).l.t2.line_gubun
#define     TCP2_CSTAT(i,j)         PROC(i,j).l.t2.connect_status
#define     DATA_CNT(i,j)           PROC(i,j).data_cnt
#define     SESSION_STAT(i,j)       PROC(i,j).session_stat

#define TCP1_PORT(i,j)    TCP1(i,PROC(i,j).l.t1.line_gubun-1).port_no
#define TCP1_SPORT(i,j,k) TCP1(i,PROC(i,j).l.t1.line_gubun-1).service_port_no[k]
#define TCP1_IP(i,j,k)    TCP1(i,PROC(i,j).l.t1.line_gubun-1).ip_addr[k]
#define TCP1_P_ST(i,j)    TCP1(i,PROC(i,j).l.t1.line_gubun-1).port_status
#define TCP1_S_ST(i,j,k)  TCP1(i,PROC(i,j).l.t1.line_gubun-1).service_status[k]
#define TCP1_S_CT(i,j,k)  TCP1(i,PROC(i,j).l.t1.line_gubun-1).service_count[k]
#define TCP1_INFO(i,j)    TCP1(i,PROC(i,j).l.t1.line_gubun-1).tcp_info

#define     TCP2_ID(i,j,k)      TCP2(i,PROC(i,j).l.t2.l[k]-1).dup_id
#define     TCP2_PORT(i,j,k)    TCP2(i,PROC(i,j).l.t2.l[k]-1).port_no
#define     TCP2_IP1(i,j,k)     TCP2(i,PROC(i,j).l.t2.l[k]-1).ip_addr[0]
#define     TCP2_IP2(i,j,k)     TCP2(i,PROC(i,j).l.t2.l[k]-1).ip_addr[1]
#define     TCP2_IP3(i,j,k)     TCP2(i,PROC(i,j).l.t2.l[k]-1).ip_addr[2]
#define     TCP2_IP4(i,j,k)     TCP2(i,PROC(i,j).l.t2.l[k]-1).ip_addr[3]
#define     TCP2_PSTAT(i,j,k)   TCP2(i,PROC(i,j).l.t2.l[k]-1).proc_status
#define     TCP2_LSTAT(i,j,k)   TCP2(i,PROC(i,j).l.t2.l[k]-1).line_status
#define     TCP2_NSTAT(i,j,k)   TCP2(i,PROC(i,j).l.t2.l[k]-1).network_status
#define     TCP2_INFO(i,j,k)    TCP2(i,PROC(i,j).l.t2.l[k]-1).tcp_info

#define     UDP_ID(i,j)         UDPIP(i,PROC(i,j).l.u-1).dup_id
#define     UDP_PORT(i,j,k)     UDPIP(i,PROC(i,j).l.u-1).port[k]
#define     UDP_IP1(i,j,k)      UDPIP(i,PROC(i,j).l.u-1).ip_addr[k][0]
#define     UDP_IP2(i,j,k)      UDPIP(i,PROC(i,j).l.u-1).ip_addr[k][1]
#define     UDP_IP3(i,j,k)      UDPIP(i,PROC(i,j).l.u-1).ip_addr[k][2]
#define     UDP_IP4(i,j,k)      UDPIP(i,PROC(i,j).l.u-1).ip_addr[k][3]
#define     UDP_P_ST(i,j)       UDPIP(i,PROC(i,j).l.u-1).port_status
#define     UDP_S_ST(i,j)       UDPIP(i,PROC(i,j).l.u-1).service_status
#define     UDP_INFO(i,j)       UDPIP(i,PROC(i,j).l.u-1).info

/* input files  */
#define     IFN(i,j,k)          FILEM(i,PROC(i,j).in_f[k]-1).file_name
#define     IFW(i,j,k,l)        FILEM(i,PROC(i,j).in_f[k]-1).w_cnt[l]
#define     IFR(i,j,k,l)        FILEM(i,PROC(i,j).in_f[k]-1).r_cnt[l]
#define     IFS(i,j,k)          FILEM(i,PROC(i,j).in_f[k]-1).record_size
#define     IFI(i,j,k)          FILEM(i,PROC(i,j).in_f[k]-1).file_info
#define     IFC(i,j,k)          FILEM(i,PROC(i,j).in_f[k]-1).fifo_count

/* output files */
#define     OFN(i,j,k)          FILEM(i,PROC(i,j).out_f[k]-1).file_name
#define     OFW(i,j,k,l)        FILEM(i,PROC(i,j).out_f[k]-1).w_cnt[l]
#define     OFR(i,j,k,l)        FILEM(i,PROC(i,j).out_f[k]-1).r_cnt[l]
#define     OFS(i,j,k)          FILEM(i,PROC(i,j).out_f[k]-1).record_size
#define     OFI(i,j,k)          FILEM(i,PROC(i,j).out_f[k]-1).file_info
#define     OFC(i,j,k)          FILEM(i,PROC(i,j).out_f[k]-1).fifo_count

/* input data SHM   */
#define     IDN(i,j,k)          DSHM(i,PROC(i,j).in_d[k]-1).data_name
#define     IDK(i,j,k)          DSHM(i,PROC(i,j).in_d[k]-1).key_info
#define     IDC(i,j,k)          DSHM(i,PROC(i,j).in_d[k]-1).fifo_count
#define     IDS(i,j,k)          DSHM(i,PROC(i,j).in_d[k]-1).data_size
#define     IDM(i,j,k)          DSHM(i,PROC(i,j).in_d[k]-1).max_rec
#define     IDO(i,j,k)          DSHM(i,PROC(i,j).in_d[k]-1).offset
#define     IDW(i,j,k,l)        DSHM(i,PROC(i,j).in_d[k]-1).w_cnt[l]
#define     IDR(i,j,k,l)        DSHM(i,PROC(i,j).in_d[k]-1).r_cnt[l]
#define     IDI(i,j,k)          DSHM(i,PROC(i,j).in_d[k]-1).info

/* output data SHM  */
#define     ODN(i,j,k)          DSHM(i,PROC(i,j).out_d[k]-1).data_name
#define     ODK(i,j,k)          DSHM(i,PROC(i,j).out_d[k]-1).key_info
#define     ODC(i,j,k)          DSHM(i,PROC(i,j).out_d[k]-1).fifo_count
#define     ODS(i,j,k)          DSHM(i,PROC(i,j).out_d[k]-1).data_size
#define     ODM(i,j,k)          DSHM(i,PROC(i,j).out_d[k]-1).max_rec
#define     ODO(i,j,k)          DSHM(i,PROC(i,j).out_d[k]-1).offset
#define     ODW(i,j,k,l)        DSHM(i,PROC(i,j).out_d[k]-1).w_cnt[l]
#define     ODR(i,j,k,l)        DSHM(i,PROC(i,j).out_d[k]-1).r_cnt[l]
#define     ODI(i,j,k)          DSHM(i,PROC(i,j).out_d[k]-1).info

#if defined ISAM_INCL
/* input c-isam files   */
#define     ICN(i,j,k)          CISAM(i,PROC(i,j).in_c[k]-1).file_name
#define     ICK(i,j,k)          CISAM(i,PROC(i,j).in_c[k]-1).key_size
#define     ICS(i,j,k)          CISAM(i,PROC(i,j).in_c[k]-1).record_size
#define     ICI(i,j,k)          CISAM(i,PROC(i,j).in_c[k]-1).file_info

/* output c-isam files  */
#define     OCN(i,j,k)          CISAM(i,PROC(i,j).out_c[k]-1).file_name
#define     OCK(i,j,k)          CISAM(i,PROC(i,j).out_c[k]-1).key_size
#define     OCS(i,j,k)          CISAM(i,PROC(i,j).out_c[k]-1).record_size
#define     OCI(i,j,k)          CISAM(i,PROC(i,j).out_c[k]-1).file_info
#endif

#define     SFIFD(i)            Shm_Mem[i].start_FIFO_fd
#define     EFIFD(i)            Shm_Mem[i].exit_FIFO_fd
#define     DFIFD(i)            Shm_Mem[i].daemon_FIFO_fd
#define     SLOGW(i)            DAEMON(i).w_cnt
#define     SLOGR(i)            DAEMON(i).r_cnt

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
#ifdef  _GLOBAL

size_t          Shmsize;                /* shared memory size           */
char            *Shmptr;                /* shared memory pointer        */
char            *DShmPtr[MAX_DSHM_SEG]; /* data SHM pointer             */
int             D_K = -1;               /* daemon key                   */
int             DD_K = -1;              /* daemon key                   */
int             P_K = -1;               /* process key                  */
int             T_K = -1;               /* sisetr index key             */
int             S_K;                    /* line key                     */
int             L_K;                /* line key (after business hours)  */
char           *Data_Ptr[DATA_BUF_CNT]; /* address of data buffer       */
int             Data_I = 0;             /* data buffer count            */
int             Process_Count = SHM_MAX_SUB;/* maximum No of sub daemon */
int             SHM_Shmid;
int             SemId[99];
int             OD_FIFO_fd[99][9];      /* fd of out-data FIFO          */
ALL_DAEMON_INFO *SHM_All_Daemon_Info;   /* address of daemon SHM        */
SUB_DAEMON_INFO Info[SHM_MAX_SUB];      /* temporary daemon buffer      */
int             Mem_Shmid[SHM_MAX_SUB];
char            *SHM_Mem[SHM_MAX_SUB];  /* address of sub daemon SHM    */
SHM_MEMORY      Shm_Mem[SHM_MAX_SUB];   /* sub SHM                      */
char            *ShmLogPtr;             /* SHM log pointer              */
int             ShmLogSemId;            /* semaphore ID for SHM log lock*/
int             ShmLogFifoFd;           /* fd of SHM log FIFO           */

int             SISE_F_Shmid;
SHM_FUTURES     *Shm_Futures;           /* 1.지수선물시세             */
int             SISE_O_Shmid;
SHM_OPTIONS     *Shm_Options;           /* 2.지수옵션시세             */
int             SISE_SF_Shmid;
SHM_STOCK_FUTURES       
                *Shm_SFutures;          /* 3.주식선물시세             */
int             SISE_SO_Shmid;
SHM_STOCK_OPTIONS       
                *Shm_SOptions;          /* 4.주식옵션시세             */
int             SISE_S_Shmid;
SHM_STOCK       *Shm_Stock;             /* 5.유가증권시세             */
int             SISE_K_Shmid;
SHM_KOSDAQ      *Shm_Kosdaq;            /* 6.코스닥시세                  */
int             SISE_K300_Shmid;
SHM_K300        *Shm_K300;              /* 8.KRX300시세                   */
int             SISE_K150F_Shmid;
SHM_K150F       *Shm_K150F;             /* 9.KOSDAQ150 Futures 시세       */
int             SISE_K150O_Shmid;
SHM_K150O       *Shm_K150O;             /* 10.KOSDAQ150 Options 시세  */
int             SISE_MF_Shmid;
SHM_MF          *Shm_MF;                /* 11.미니선물                  */
int             SISE_MO_Shmid;
SHM_MO          *Shm_MO;                /* 12.미니옵션                  */
int             SISE_J_Shmid;
SHM_JISU        *Shm_Jisu;              /* 20.지수시세                  */

int             SISE_Mk_Prematch_Shmid;
MK_PREMATCH     *Shm_Mk_PreMatch;       /* 23.시장별 미결제정보         */
int             SISE_Strrg_Shmid;
STRRG           *Shm_Strrg;             /* 22.계좌별 전략영역          */
int             SISE_Risk_Shmid;
RISK            *Shm_Risk;              /* 24.한도관리                  */
int             SISE_Item_Shmid;
SHM_KEY_ARRY    *Shm_Item;              /* 30.종목코드 qsort/besearch   */

int             SISE_CME_Shmid;
SHM_CME         *Shm_CME;               /* 31.CME                       */
int             SISE_CMX_Shmid;
SHM_CMX         *Shm_CMX;               /* 32.CMX                       */
int             SISE_SGX_Shmid;
SHM_SGX         *Shm_SGX;               /* 33.SGX                       */
int             SISE_ERX_Shmid;
SHM_ERX         *Shm_ERX;               /* 34.ERX                       */
int             SISE_HKE_Shmid;
SHM_HKE         *Shm_HKE;               /* 35.HKE                       */


int             P_Semid = -1;
int             DP_Semid = -1;

#else

extern size_t           Shmsize;
extern char             *Shmptr;
extern char             *DShmPtr[MAX_DSHM_SEG];
extern int              D_K;
extern int              DD_K;
extern int              P_K;
extern int              T_K;
extern int              S_K;
extern int              L_K;
extern char             *Data_Ptr[DATA_BUF_CNT];
extern int              Data_I;
extern int              Process_Count;
extern int              SHM_Shmid;
extern int              SemId[99];
extern int              OD_FIFO_fd[99][9];
extern ALL_DAEMON_INFO  *SHM_All_Daemon_Info;
extern SUB_DAEMON_INFO  Info[SHM_MAX_SUB];
extern int              Mem_Shmid[SHM_MAX_SUB];
extern char             *SHM_Mem[SHM_MAX_SUB];
extern SHM_MEMORY       Shm_Mem[SHM_MAX_SUB];
extern char             *ShmLogPtr;
extern int              ShmLogSemId;
extern int              ShmLogFifoFd;

extern  int             SISE_F_Shmid;
extern  SHM_FUTURES     *Shm_Futures;           /* 1.지수선물시세               */
extern  int             SISE_O_Shmid;
extern  SHM_OPTIONS     *Shm_Options;           /* 2.지수옵션시세               */
extern  int             SISE_SF_Shmid;
extern  SHM_STOCK_FUTURES
                       *Shm_SFutures;          /* 3.주식선물시세               */
extern  int             SISE_SO_Shmid;
extern  SHM_STOCK_OPTIONS
                       *Shm_SOptions;          /* 4.주식옵션시세               */
extern  int             SISE_S_Shmid;
extern  SHM_STOCK       *Shm_Stock;             /* 5.유가증권시세               */
extern  int             SISE_K_Shmid;
extern  SHM_KOSDAQ      *Shm_Kosdaq;            /* 6.코스닥시세                 */
extern  int             SISE_K300_Shmid;
extern  SHM_K300        *Shm_K300;              /* 8.KRX300시세                   */
extern  int             SISE_K150F_Shmid;
extern  SHM_K150F       *Shm_K150F;             /* 9.KOSDAQ150 Futures 시세       */
extern  int             SISE_K150O_Shmid;
extern  SHM_K150O       *Shm_K150O;             /* 10.KOSDAQ150 Options 시세  */
extern  int             SISE_MF_Shmid;
extern  SHM_MF          *Shm_MF;                /* 11.미니선물                  */
extern  int             SISE_MO_Shmid;
extern  SHM_MO          *Shm_MO;                /* 12.미니옵션                  */
extern  int             SISE_J_Shmid;
extern  SHM_JISU        *Shm_Jisu;              /* 20.지수시세                  */

//extern int               SISE_DB_Shmid;
//extern SHM_DB            *Shm_Db;             /* DB사용                       */
extern  int             SISE_Mk_Prematch_Shmid;
extern  MK_PREMATCH     *Shm_Mk_PreMatch;       /* 23.시장별 미결제정보         */
extern  int             SISE_Strrg_Shmid;
extern  STRRG           *Shm_Strrg;             /* 22.계좌별 전략영역          */
extern  int             SISE_Risk_Shmid;
extern  RISK            *Shm_Risk;              /* 24.한도관리                  */
extern  int             SISE_Item_Shmid;
extern  SHM_KEY_ARRY    *Shm_Item;              /* 30.종목코드 qsort/besearch   */

extern  int             SISE_CME_Shmid;
extern  SHM_CME         *Shm_CME;               /* 31.CME                       */
extern  int             SISE_CMX_Shmid;
extern  SHM_CMX         *Shm_CMX;               /* 32.CMX                       */
extern  int             SISE_SGX_Shmid;
extern  SHM_SGX         *Shm_SGX;               /* 33.SGX                       */
extern  int             SISE_ERX_Shmid;
extern  SHM_ERX         *Shm_ERX;               /* 34.ERX                       */
extern  int             SISE_HKE_Shmid;
extern  SHM_HKE         *Shm_HKE;               /* 35.HKE                       */

extern int              P_Semid;
extern int              DP_Semid;

#endif

/*************************************************************************
    End of Program (shm_memory.h)
*************************************************************************/
#endif
