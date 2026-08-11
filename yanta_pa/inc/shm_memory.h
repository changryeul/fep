#ifndef     __SHM_MEMORY_H
#define     __SHM_MEMORY_H
/*------------------------------------------------------------------------
#   Module  : shared memory management
#   File    : shm_memory.h
#   copywrite : 박상훈
------------------------------------------------------------------------*/
#include    "krx_mk.h"
#include    "fx.h"
#include    "key_code.h"
#include    "strategy.h"

/*------------------------------------------------------------------------
  변수사용 시작위치 및 계좌사용건수
------------------------------------------------------------------------*/
#define     ACC_NO_CNT          20                      /* 사용계좌수    */
#define     FX_ACC_NO_CNT        1                      /* FX 사용계좌수    */
#define     ACC_NO_BASE         31                      /* 계좌기준수    */
#define     FILE_NO_BASE        30                      /* 파일기준수    */

/* ---------------------------------------------------------------- */
/* 주문번호 영역 (채번용 주문번호 영역 From ~ To)					*/
/* ---------------------------------------------------------------- */
/* 전략 전체 채번 시작주문번호 */
/* 1. 권장은 2억개를 사용하는 것이다. 
   2. 1억개는 메인서버 또는 1번서버에서 사용하고 
      나머지 1억은 백업서버 또는 2번서버에서 사용하는 것으로 한다. (설계시 서버사용 갯수 중요)
   3. 1개의 서버(1억개)는 40개의 전략Process에 할당한다.
   4. 40개로 할당된 주문번호는 1억/40 => 2,500,000(2백5십만)이 아니고 100만개씩 사용한다. 
   5. Client는 MK01_ORDER_NO_BASE+(MK01_ORDER_NO_GAP*MAX_AUTO_PROC)+MK01_ORDER_NO_GAP 부터 MK01_ORDER_NO_GAP를 준다.
      즉, 여기서는 200,000,000 + 40,000,000 + 1,000,000 = 241,000,000 ~ 242,000,000까지 사용
*/ 

/* ************************************************************************************************************ */
/* 여기서는 전체 사용영역을 메인서버기준 2억부터 3억까지 사용하는 것으로 define했으니 고객과 협의하여 변경한다. */ 
/* 채권은 3억부터 무한, 파생은 6400001부터 5만개만	*/
/* 300,000,001 ~ , 6,400,001 ~ 6,450,000			*/
/* ************************************************************************************************************ */
#define     MK01_ORDER_NO_BASE        300000000        /* 채권 주문번호 채번 사용구간 시작번호	 300,000,000 */
#define     MK02_ORDER_NO_BASE          6400001        /* 파생 주문번호 채번 사용구간 시작번호	 200,000,000 */

#define     MK01_ORDER_NO_BACK_GAP    100000000        /* 채권 주문번호 채번 사용구간 갭_2번서버 100,000,000 */
#define     MK02_ORDER_NO_BACK_GAP        40000        /* 파생 주문번호 채번 사용구간 갭_2번서버 100,000,000 */

#define     MK01_ORDER_NO_GAP           1000000        /* 채권 주문번호 채번 사용구간별 갭 		 1,000,000	*/
#define     MK02_ORDER_NO_GAP              1000        /* 파생 주문번호 채번 사용구간별 갭 		 	 1,000	*/


/* 자동주문 Process 수  */
#define     MAX_AUTO_PROC     40                        /* 자동주문 Process 수   */

/* 미체결 처리영역 Max건_시장별, 메모리에서 사용할 시장별 Arry */
#define     MAX_MICHE         10000                     /* Poll 밀린것 */

/* System Risk 관리, 고객협의내용 */
/* 1회 주문수량한도, 1회주문금액한도 설정여부에 따라 사용여부 달라짐(호가정합성체크와 유사) */
/* 선물금액은 75억, 옵션금액은 3억 */
/* 선물은 500,000으로 나누고, 옵션은 100,000으로 나눈값 */
#if 0
#define     MAX_POLL          1000                      /* Poll 밀린것 	*/
#define     MAX_FU_SU           50                      /* 선물수량기준 */
#define     MAX_OP_SU         1000                      /* 옵션수량기준 */
#define     MAX_FU_GUM       15000                      /* 선물주문금액 15,000,000 (1000원절삭) */
#define     MAX_OP_GUM        3000                      /* 옵션주문금액  3,000,000 (1000원절삭)	*/
#endif

#if 0
/* ****************************** */
/* 시장관리 Arry를 위한 값 DeFine */
/* 2025 Modifying to market word */
/* 파생시장구분 Start, 사용은 금융상품선물만 사용 (6)  */
/* 전체시장을 사용하는것이면 통합으로 필요할 수 있으나 2개만 사용하기 때문에 List Up만 해둔것임 */
#define     DEV_MK_CNT          17                      /* Market Cnt   */

#define     DEV_MK_COMMON        0                      /* 시장공통     */
#define     DEV_MK_JF            1                      /* 지수선물     */
#define     DEV_MK_K150F         2                      /* Kosdaq150 Futures    */
#define     DEV_MK_JO            3                      /* 지수옵션     */
#define     DEV_MK_SF            4                      /* 주식선물     */
#define     DEV_MK_SO            5                      /* 주식옵션     */
/* 사용 */
#define     DEV_MK_FIF           6                      /* 금융상품선물(국채,금리,통화) */
														/* Financial Instruments Futures */
														/* 여기서는 요상품만 거래 */
/* 사용 */
#define     DEV_MK_FIO           7                      /* 상품옵션(휴면) */
#define     DEV_MK_VIF           8                      /* 변동성지수선물, 섹터지수선물 */
#define     DEV_MK_KGIF          9                      /* 변동성지수선물, 코스닥글로벌지수선물 */
#define     DEV_MK_GCF          10                      /* 일반상품선물 (금,돈육(휴면)) */
#define     DEV_MK_MF           11                      /* 미니코스피 선물 */
#define     DEV_MK_MO           12                      /* 미니코스피 옵션 */
#define     DEV_MK_K300         13                      /* KRX300       */
#define     DEV_MK_ES50         14                      /* EURO STOXX 50선물    */
#define     DEV_MK_K150O        15                      /* Kosdaq150 Options    */
#define     DEV_MK_K200WO       16                      /* 코스피200 위클리 Options */
/* 파생시장구분, End, 사용은 금융상품선물만 사용 (6)  */

/* 채권시장구분, Start, 사용은 KTS만 사용 (3) */
#define     NOTE_MK_CNT         5                       /* Market Cnt   */

#define     NOTE_MK_COMMON      0                       /* 채권공통     */
#define     NOTE_MK_BND         1                       /* 채권			*/
#define     NOTE_MK_SMB         2                       /* 소액채권     */
/* 사용 */
#define     NOTE_MK_KTS         3                       /* 국채			*/
/* 사용 */
#define     NOTE_MK_RPO         4                       /* 레포			*/
/* 채권시장구분, End , 사용은 KTS만 사용 (3) */
#endif

/* Key_Search에서 사용 */
#define     NOTE_MK_KTS         0						/* 국채			*/
#define     DEV_MK_FIF          1						/* 금융상품선물(국채,금리,통화) */
#define     DEV_MK_FIF_N        2						/* 금융상품선물(국채,금리,통화) 야간	*/

/* 한도시장구분 */
/* 실 사용 시장만 적용함 2025 */
#define     RISK_MK_CNT         2                      /* Risk Market Cnt  */
#define     RISK_MK_FF          0                      /* Risk Market 		*/
#define     RISK_MK_FF_N        1                      /* Risk Market 		*/
/*
	0 : 채권(3) 시장, KTS국채
	1 : 파생(6) 시장, 금융상품선물(국채,금리,통화)
*/
/* 고객협의 통해 실제 얼마나 있는지 확인 필요, Data 검증필요 */
/* 시장별 MAX 종목 설정 값 */
//#define     SHM_MAX_NOTE	         10000              /* 3. 국채KTS							*/
#define     SHM_MAX_NOTE	          1000              /* 3. 국채KTS							*/
#define     SHM_MAX_RDS01	        100000              /* 3. 국채KTS							*/
#define     SHM_MAX_DEV_FIF           1000              /* 6. 금융상품선물(국채,금리,통화)		*/
                                                        /* Financial Instruments Futures		*/
                                                        /* 여기서는 요상품만 거래				*/
#define     SHM_MAX_DEV_FIF_N         1000              /* 6. 금융상품선물(국채,금리,통화) 야간		*/
                                                        /* Financial Instruments Futures		*/
                                                        /* 여기서는 요상품만 거래				*/
#define     SHM_MAX_FX				    30              /* 6. 금융상품							*/


#define     SHM_MAX_PREMATCH    RISK_MK_CNT             /* 미체결용, 실사용시장 3개만 사용, MK_CNT사용하지 않고 RISK_MK_CNT로 사용, 소스포함   */
#define     SHM_MAX_STRRG       ACC_NO_CNT              /* 전략용, 위 Define 20개				*/

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
	u_char	if_ip_addr[4];			/* interface ip address				*/
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
    int         if_meg_seq[10];		/* FEP직결 체결수신, ME그룹 10개Seq	*/
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

	// 2025 Add, 일괄송신용
	int			tr_seq;
	char		curr_tr[11];
	// 2025 Add, 일괄송신용

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
    계좌정보, 2019년이후 계좌정보로 사용하지 않고 Risk에서 처리
*************************************************************************/

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
}   SHM_MEMORY; /* Shared Memory    */


/* ********************************************************************* */
/* 시장별 공유메모리 설정 */
/* ********************************************************************* */

/* 공통시세 구축 A0+A3등, 호가정합성 체크를 위해 */
/* 시장은 금융파생(6)과 KTS(국채)에 필요한 항목만 List Up해서 시세수신시 처리한다 */
typedef struct                      
{                       
    int             auto_use;                   /* 자동전략기동시 해당종목 +- 설정, 설정된 경우에만 전략에 전달한다. */
    int             dont_trade;                 /* 매매불가종목정보(아침에 받고, 실시간으로 수정된거 받기   */
    /* **************************************************************************** */
    /* A0, Master Data */
    char            m_item_cd[12];                  /* Item Full Code               */
/* 채권은 상/하한가 A0에 없음 */
    double          m_h_lmt;                        /* Hi Limit                     */
    double          m_l_lmt;                        /* Low Limit                    */
/* 채권은 상/하한가 A0에 없음 */
    char            m_item_stat[1];                 /* Item Status , 0:Normal, !0:Cannot*/
													/* 채권은 거래정지여부 항목으로 사용하자 (trade_susp_yn), Y/N */
	/* 미사용 항목 2025 */
//    char            item_stat[1];                   /* 종목장운영정보의 종목거래가능 여부, Item Status 0:Normal, !0:Cannot  */
													/* 종목장운영정보는 모든 시장이 공통이다.  */
    double          m_stdard_price;                 /* Standard Price               */
                                                    /* 기준가사용, 조정기준가는 파생에 있는데 기준가로 사용 */
// 채권은 미사용, 파생만 사용
    int             m_cds_gbn_2;                    /* 지정가 호가조건코드 */
    int             m_cds_gbn_I;                    /* 조건부지정가 호가조건코드 */
    int             m_cds_gbn_T;                    /* 시장가 호가조건코드(1대신 파생용) */
    int             m_cds_gbn_W;                    /* 최유리지정가 호가조건코드(X대신 파생용) */
// 채권은 미사용, 파생만 사용

    double          m_max_qty;                      /* 상한수량(채권없음)  */
    char            m_clearance_gbn[1];             /* Y:정리매매, N:일반(채권없음)     */
    char            m_start_price_gbn[1];           /* Y:시가기준가종목, N:일반(채권없음)  */
    // Add, 20200602, 채권은 없음
    int             m_multiplier;                   /* 승수, 현물은 1로 처리    */
	double			m_striking_price;				/* 행사가, 파생만사용,현물1	17(9V8)중 11자리 소수점2자리까지만 사용 */
    char            m_closeday[8];                  /* 거래종료일                */
    double          basic_asset_price;              /* 기초자산 전일종가        */
    /* **************************************************************************** */
	/* 시세영역																		*/                                              
    /* **************************************************************************** */
    double          crprc;                      /* Matching Price       */                      
    double          sell_1_price;               /* Sell 1 Price         */                      
    double          buy_1_price;                /* Buy 1 Price          */                      
    double          realtime_hprc;              /* Runnig Time Hi Limit , run_tm_h_lmt, 파생만,채권X  */
    double          realtime_lprc;              /* Runnig Time Low Limit, run_tm_l_lmt, 파생만,채권X  */
    /* **************************************************************************** */
}   STANDARD_SISE_FORMAT;                       

/* 공통시세 구축 A0+A3등, 호가정합성 체크를 위해 */
/* 시장은 FX 필요한 항목만 List Up해서 시세수신시 처리한다 */
typedef struct                      
{                       
    int             auto_use;                   /* 자동전략기동시 해당종목 +- 설정, 설정된 경우에만 전략에 전달한다. */
    int             dont_trade;                 /* 매매불가종목정보(아침에 받고, 실시간으로 수정된거 받기   */
    /* **************************************************************************** */
    /* A0, Master Data */
	char			m_exch[1];					/* 거래소 code					*/
    char            m_item_cd[7];				/* symbol						*/
	char			complete_f[1];						/* ask와 bid가 따로 들어오는 경우 2개를 같이 처리하기 위해 */
#if	0
    double          m_h_lmt;                        /* Hi Limit                     */
    double          m_l_lmt;                        /* Low Limit                    */
    char            m_item_stat[1];                 /* Item Status , 0:Normal, !0:Cannot*/
													/* 채권은 거래정지여부 항목으로 사용하자 (trade_susp_yn), Y/N */
	/* 미사용 항목 2025 */
//    char            item_stat[1];                   /* 종목장운영정보의 종목거래가능 여부, Item Status 0:Normal, !0:Cannot  */
													/* 종목장운영정보는 모든 시장이 공통이다.  */
    double          m_stdard_price;                 /* Standard Price               */
                                                    /* 기준가사용, 조정기준가는 파생에 있는데 기준가로 사용 */
// 채권은 미사용, 파생만 사용
    int             m_cds_gbn_2;                    /* 지정가 호가조건코드 */
    int             m_cds_gbn_I;                    /* 조건부지정가 호가조건코드 */
    int             m_cds_gbn_T;                    /* 시장가 호가조건코드(1대신 파생용) */
    int             m_cds_gbn_W;                    /* 최유리지정가 호가조건코드(X대신 파생용) */
// 채권은 미사용, 파생만 사용

    double          m_max_qty;                      /* 상한수량(채권없음)  */
    char            m_clearance_gbn[1];             /* Y:정리매매, N:일반(채권없음)     */
    char            m_start_price_gbn[1];           /* Y:시가기준가종목, N:일반(채권없음)  */
    // Add, 20200602, 채권은 없음
    int             m_multiplier;                   /* 승수, 현물은 1로 처리    */
	double			m_striking_price;				/* 행사가, 파생만사용,현물1	17(9V8)중 11자리 소수점2자리까지만 사용 */
    char            m_closeday[8];                  /* 거래종료일                */
    double          basic_asset_price;              /* 기초자산 전일종가        */
#endif
    /* **************************************************************************** */
	/* 시세영역																		*/
    /* **************************************************************************** */
#if	0
    double          crprc;                      /* Matching Price       */                      
#endif
    double          sell_1_price;               /* Sell 1 Price         */                      
    double          buy_1_price;                /* Buy 1 Price          */                      
	char            bid_quote_id[30];               /* Quote ID - 신한용    */
	char            ask_quote_id[30];               /* Quote ID - 신한용    */
#if	0
    double          realtime_hprc;              /* Runnig Time Hi Limit , run_tm_h_lmt, 파생만,채권X  */
    double          realtime_lprc;              /* Runnig Time Low Limit, run_tm_l_lmt, 파생만,채권X  */
#endif
    /* **************************************************************************** */
}   FX_SISE_FORMAT;                       

/* **************************************************************** */
/*  채권 
	2025추가 메모리사용 영역 정의									*/
/* **************************************************************** */
typedef struct
{
    int             total_item_cnt;         /* 종목수, seq 0에만 사용		 */

    int             check_cnt;              /* 보유여부 확인(평가금액용)	 */
                                            /* 해당종목을 보유한 관리계좌가 있는지, 평가금액 확인 때문에 */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */

    int             CURR_Arry_Key;          /* 최근 시세내역 30개의 Key		 */
    int             Befor_CURR_Arry_Key;    /* CURR_Arry_Key 직전값기억		 */

    int             HogaLastGbn;            /* 0:체결(A3/G7), 1:호가(B6)	 */
 
    CO_M401K		M4;					    /* 장운영정보, seq는 0만사용	 */
    CO_A701K		A7;					    /* 장운영TS, 사용여부는 불투명	 */

    CO_A001_RDS02	A0;                     /* 채권 종목배치(RDS)			 */
    CO_A301K		A3;                     /* 채권 체결_A3					 */
    CO_G701K		G7;                     /* 채권 체결_G7					 */
    CO_B601K		B6;                     /* 채권 호가_B6					 */

	CO_G701K		CURR_Arry[30];          /* 최근 시세내역 30개 기억       */

}   SHM_NOTE;		/* 채권시세 Shared Memory */

/*************************************************************************
        파생(금융상품) 시세
*************************************************************************/
typedef  struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다.  */

    int             check_cnt;              /* 보유여부 확인(평가금액용)     */
                                            /* 해당종목을 보유한 관리계좌가 있는지, 평가금액 확인 때문에 */
#if	1
    char			auto_use[MAX_AUTO_PROC];/* 자동전략기동시 해당종목 전략 index에 1 설정, 설정된 경우에만 전략에 전달한다. */
#else
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정 */
#endif
	int             sise_delay;             /* 거래소시간과 비교해서 1초이상 차이발생 시 1로 설정*//* 2026/02/12 add */
	time_t          sise_delay_tm;          /* sise_delay flag 를 설정한 time                    *//* 2026/02/12 add */

    int             CURR_Arry_Key;          /* 최근 시세내역 30개의 Key     */
                                            /* 시세내역 30개 기억에만 사용  */
    int             Befor_CURR_Arry_Key;    /* CURR_Arry_Key 이전값 기억    */

    int             HogaLastGbn;            /* 0:G7, 1:B6                   */

    CO_M401F		M4;             	    /* 장운영정보, seq는 0만사용    */
    CO_A701A		A7;					    /* 장운영TS, 사용여부는 불투명  */

    CO_A001F		A0;                     /* 기본                         */
    CO_B601F		B6;                     /* 호가                         */
    CO_A301F		A3;                     /* 체결                         */
    CO_G701F		G7;                     /* 체결 + 호가                  */
/* 미사용, 고객이 사용한다면 다른 TR도 추가 가능
    CO_V101F		V1;
    CO_Q201F		Q2;
*/
	CO_G701F		CURR_Arry[30];          /* 최근 시세내역 30개 기억      */

}   SHM_FIN_FUT;	/* 금융상품파생 Shared Memory	*/

/*************************************************************************
        FX 시세
*************************************************************************/
typedef  struct
{
    int             total_item_cnt;         /* 종목수, arry 0에만 보관한다. */

    int             check_cnt;              /* 보유여부 확인(평가금액용)    */
                                            /* 해당종목을 보유한 관리계좌가 있는지, 평가금액 확인 때문에 */
#if	1
    char			auto_use[MAX_AUTO_PROC];/* 자동전략기동시 해당종목 전략 index에 1 설정, 설정된 경우에만 전략에 전달한다. */
#else
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */
#endif
	int             sise_delay;             /* 거래소시간과 비교해서 1초이상 차이발생 시 1로 설정*//* 2026/02/12 add */
	time_t          sise_delay_tm;          /* sise_delay flag 를 설정한 time                    *//* 2026/02/12 add */

    int             CURR_Arry_Key;          /* 최근 시세내역 30개의 Key     */
                                            /* 시세내역 30개 기억에만 사용  */
    int             Befor_CURR_Arry_Key;    /* CURR_Arry_Key 이전값 기억    */
#if	0
    int             HogaLastGbn;            /* 0:G7, 1:B6                   */
#endif

#if	0
    CO_M401F		M4;             	    /* 장운영정보, seq는 0만사용    */
    CO_A701A		A7;					    /* 장운영TS, 사용여부는 불투명  */

    CO_A001F		A0;                     /* 기본                         */
#endif
    CO_B6FX			B6;                     /* 호가                         */
#if	0
    CO_A301F		A3;                     /* 체결                         */
    CO_G701F		G7;                     /* 체결 + 호가                  */
#endif
/* 미사용, 고객이 사용한다면 다른 TR도 추가 가능
    CO_V101F		V1;
    CO_Q201F		Q2;
*/
	CO_B6FX			CURR_Arry[30];          /* 최근 시세내역 30개 기억      */

}   SHM_FX;	/* FX Shared Memory	*/

/*************************************************************************
    MiChe Data Struct
*************************************************************************/
typedef struct {
    int     Jan_Cnt;                        /* 주문잔량(절대값)		*/
											/* 채권LP는 매도수량+매수수량 */
    int     Mk_gbn;                         /* 시장구분             */
                                            /* 0(채권), 1(금융파생)	*/
    int     Item_Seq;                       /* 종목코드 Master Seq  */
    int     Acc_Seq;                        /* 계좌번호 Seq         */
    int     Str_No;                         /* 전략번호 Seq         */

/* 20220204 */
	double	Order_Price_dv;					/* 8. 주문가격(채권일반,파생용)	*/

	char    AccountNo[12];                  /* 1.계좌번호           */
	char    Item_Cd[12];                    /* 2. 종목코드          */
	char    OrderNo[10];                    /* 3. 주문번호          */
	char    OriginalOrderNo[10];            /* 4. 원주문번호        */
	char    OrderFlag[1];                   /* 5. 정정취소구분      */
                                            /* (0:신규,1:정정,2:취소)   */
	char    TradeFlag[1];                   /* 6. 매도매수구분      */
											/* 1:매도, 2:매수, 3:매도매수	*/

	char    Order_Cnt[8];                   /* 7. 주문수량          */
//    char    Order_Price[9];               /* 8. 주문가격(현물용),미사용	*/
/* 채권LP용 */
	char    Order_Cnt_Lp01[8];				/* 7. 매도주문수량(채권LP)	*/
	char    Order_Price_Lp01[9];			/* 8. 매도주문가격(현물용)	*/
	char    Order_Cnt_Lp02[8];				/* 7. 매수주문수량(채권LP)	*/
	char    Order_Price_Lp02[9];			/* 8. 매수주문가격(채권LP)	*/
/* 채권LP용 */

	char    OrderType[1];                   /* 9. 주문유형          */
                                            /* 'B', '최유리지정가'  */
                                            /* 'C', '조건부지정가'  */
                                            /* 'L', '지정가'        */
                                            /* 'M', '시장가'        */
                                            /* '0' ,원주문이 있을때 */
	char    JumunFlag[1];                   /* 10.호가조건코드		*/
                                            /* F:FOK, I:IOC, X:없음 */
	char    MembershipItem[60];             /* 11.회원사처리항목   */
											/* 채권은 60, 금융파생은 20 */
	char    Tmp[60];						/* 12.고객사헤더등 용도	*/
}   MICHE;

/*************************************************************************
    FX MiChe Data Struct
*************************************************************************/
typedef struct {
    int     Jan_Cnt;                        /* 주문잔량(절대값)		*/
											/* 채권LP는 매도수량+매수수량 */
    int     Mk_gbn;                         /* 시장구분             */
                                            /* 0(FX)				*/
    int     Item_Seq;                       /* 종목코드 Master Seq  */
    int     Acc_Seq;                        /* 계좌번호 Seq         */
    int     Str_No;                         /* 전략번호 Seq         */

	char    Account[30];                    /* 1.계좌번호  AccountNo         */
	char    Symbol[7];						/* 2. 종목코드 Item_Cd         */
	char    ClOrdID[24];                    /* 3. 주문번호 OrderNo         */
	char    OrigClOrdID[24];                /* 4. 원주문번호 OriginalOrderNo       */
	char    OrdStatus[1];                   /* 5. 정정취소구분 OrderFlag     */
											// 주문상태         : '0'-NEW
											//                  : '1'-Partially filled
											//                  : '2'-Filled
											//                  : '4'-Canceled
											//                  : '8'-Rejected"
	char    Side[1];                        /* 6. 매도매수구분 TradeFlag    */
											/* '1'-BUY, '2'-SELL			*/
	char    OrderQty[30];                   /* 7. 주문수량 Order_Cnt         */
    char    Price[30];                      /* 8. 주문가격	Order_Price		*/
	char    OrdType[1];                     /* 9. 주문유형 OrderType         */
											// Always 2('1'-시장가, '2'-지정가, '3'-예약주문")
	char    TimeInForce[1];                 /* 10.호가조건코드 JumunFlag		*/
											// 체결조건         : '0'-For Day
											//                  : '1'-For Good Till Cancel
											//                  : '3'-For Immediate Or Cancel(IOC)
											//                  : '4'-For Fill or Kill(FOK)
											//                  : '6'-For Good Till Date(GTD)
                                            /* F:FOK, I:IOC, X:없음 */
} FX_MICHE;

/* 종목별 (평가손익/수량) 손익/한도 관리 */
typedef struct
{
    long    item_getcnt;                    /* 계좌별 보유종목수량(부호있음), 현선물공통 */
                                            /* 최초 원장에서 받고 매매시 업데이트한다. */
    double  item_get_avg_price;             /* 종목별 평균매입단가, 체결시		*/
//	long	item_getmoney;                  /* 장부금액(수량*평단), 현물만(파생은 item_get_avg_price사용) */
                                            /* 최초 원장에서 받고 업데이트안한다. */
/* 매수만 체크하는 로직은 메리츠만 했음, 현물만 */
//    long    item_tot_sugum;               /* 당일 매수누적금액, 현물만사용    */
//    long    item_cha_sugum;               /* 당일 매수차감금액, 현물만사용    */

#if 0
    /* 전략에서 현물용 */
    long    item_borrow_cnt;                /* 차입수량                         */
    long    item_totsu_che;                 /* 매수누적체결, 전략에서 사용      */
    long    item_totdo_che;                 /* 매도누적체결, 전략에서 사용      */
#endif

	/* 20220204 */
    double  item_su_miche_gum;				/* 매수미체결 합산금액, , 신규:주문시 증가, 정정:회원처리호가시 조정	*/
    double  item_do_miche_gum;				/* 매도미체결 합산금액, , 신규:주문시 증가, 정정:회원처리호가시 조정	*/
    int     item_su_michecnt;				/* 매수미체결 수량    , , 신규:주문시 증가, 정정:회원처리호가시 조정	*/
    int     item_do_michecnt;				/* 매도미체결 수량,     , 신규:주문시 증가, 정정:회원처리호가시 조정	*/

	/* 20220204 */

#if 0
/* 20211209 */
    int     item_jucnt;                     /* 주문나간 수량(절대값), 수량한도,미체결수량 */

    double  item_get_avg_price;             /* 종목별 평균매입단가               */
    double  item_ver_prft;                  /* 종목별 평가손익(부호있음)       */
    double  item_real_prft;                 /* 종목별 평가손익(부호있음)       */
#endif
}   PROFIT;

/* ******************************************************************** */
/* 전략처리를 위한 SHM                                                 */
/* ******************************************************************** */
/* TR구분  500100/500110/500120 / 500200/500210/500220 / 500300/500310/500320 / 500410 */
/* 전략기동   기동요청  500100
              성공응답  500110
              오류응답  500120

   자동주문조건설정요청 500200 (한번에 끝나는 설정이면 O(One), 여러건일경우 S(Start)/N(Next)/E(End)의 순서로)
              성공응답  500210
              오류응답  500220

   전략종료   종료요청  500300
              성공응답  500310
              오류응답  500320

   강제종료   강제종료  500410
*/

/* 처리절차
   500100이후에는 ApType(기동Process번호 ex)50101 를 Key로 처리한다) Client는 반드시 관리하고 있어야 컨트롤이 가능하다
   1. Client는 500100(자동주문기동요청)을 보내면 pa_9001_mp에 Write한다.
      pa_9001_mp는 기동할 Process의 번호를 선택하여 ApType을 설정하고 Process를 기동시켜주고
      pa_8101_ts에 ApType을 응답(성공시500110, 오류시500120)으로 보내준다.
   2. 응답을 수신받은 Client는 500200(자동주문조건요청)을 전송한다.
      이때 오류가 발생하면 전략Process는 pa_8101_ts에 500220(자동기동오류)를 보내고, 성고하면 500210(자동기동성공)를 보낸다.
   2-1. 500200(자동주문조건설정요청)일 여러건 보낼경우 S(Start)/N(Next)/E(End)의 순으로 보내고 한번만 보낼때는 O(One)으로 보낸다
   3. Client는 자동종료 요청시에는 500300(자동종료요청)으로 보낸다. 성공시 500310(자동종료성공) or 500320(자동종료오류)로 보내며
      오류는 기동이 아닌데 종료요청시 반환된다.(모니터링시 발견되면 확인해봐야함)
   4. 한도에 걸리면 강제종료가 발생한다.
      주문송신에서 한도오류(한도초과등)발생시 500410(자동강제종료)를 pa_9001_mp로 보내고 해당 전략Process를 종료하고 Client로 송신한다.
*/

/* clinet와는 총 500byte를 주고받고 이중 20byte(480 byte는 Data)는 헤더이다          */
/* 샘플전략은 지수선물의 체결이 매수체결이면 지정한 call옵션을 매수하고 */
/*                              매도체결이면 지정한  put옵션을 매도한다 */

typedef struct
{
    int                 auto_run_flag[ACC_NO_CNT];  /* 0:not run, 1:run */
    SHM_SAMPLE01        SamPle_St01[ACC_NO_CNT];
    LP_Strrg            Lp_St[ACC_NO_CNT];
    Basket_t            basket[ACC_NO_CNT];
    ArbtSet_t           arbtset[ACC_NO_CNT];        // 차익거래
}   STRRG;

/* 시세 수신시(평가손익) 손익 && 한도 관련 메모리 처리  */
/* 계좌별 손익한도, 수량한도는 ACCNO에서 처리함         */
typedef struct
{
    int         MeChe_Cnt[RISK_MK_CNT][ACC_NO_CNT];              /* 시장별/계좌별 미체결수량        */
    MICHE       F_MiChe[RISK_MK_CNT][ACC_NO_CNT][MAX_MICHE];     /* 미체결 내역조회, MAX_MICHE(10000)	*/
    int         FX_MeChe_Cnt[1][FX_ACC_NO_CNT];					/* 시장별/계좌별 미체결수량        */
    FX_MICHE    FX_MiChe[1][FX_ACC_NO_CNT][MAX_MICHE];			/* 미체결 내역조회, MAX_MICHE(10000)	*/
                                                				/* 계좌단위가 아닌 시스템 전체 대상   */
}   MK_PREMATCH;        /* 종목별 한도/수량/평가손익 관리 Memory */
// SHM_DB[RISK_MK_CNT]   =>  MK_PREMATCH[RISK_MK_CNT]

/* 20211028 */
/* Accout Master */
typedef struct
{
    char    acc_no[12];                     /* Account No, KEY					*/
                                            /* KRX주문전문의 계좌와 동일해야함	*/
    char    mk_gbn[1];                      /* 0:채권  1:금융파생				*/
}   MST_ACCNO_MASTER;

/* 고객사의 내부 처리 방식에 따라서 추후 변경될 수 있음, 해당처리는 M사 기준임 */
typedef struct
{
#if 0
/* 20211209 */
    double  order_money;                    /* 1회 주문금액					*/
    char    qty_chk_gbn[1];                 /* 1:설정수량, 2:상장주식수*설정비율 */  
    double  qty_chk;                        /* 수량 또는 상장주식수*비율	*/
    char    lmt_chk_gbn[1];                 /* 1:Tick, 2:현재가*설정비율	*/    
    double  lmt_chk;                        /* Tick 또는 현재가*설정비율	*/  
#endif
    char    qty_gbn[1];                     /* 1회주문 계약수 체크여부		*/
    long    qty;                            /* 1회주문 계약수				*/
    char    money_gbn[1];                   /* 1회주문 금액 체크여부		*/
    double  money;                          /* 1회주문 금액					*/
    char    tick_gbn[1];                    /* 1회주문 Tick 체크여부		*/
    long    tick;                           /* 1회주문 Tick(8자리)			*/
}   CHK_RISK_ONE;

typedef struct
{
#if 0
/* 20211209 */
    double  order_money;                    // 주문총액
    double  qty_chk;                        /* 수량, 잔고 + 미체결주문(매도/매수) */
#endif
    char    do_cnt_gbn[1];                  /* 매도 계약수 체크여부			*/
    long    do_cnt;                         /* 매도 계약수					*/
    char    do_money_gbn[1];                /* 매도 금액 체크여부			*/
    double  do_money;                       /* 매도 금액					*/
    char    su_cnt_gbn[1];                  /* 매수 계약수 체크여부			*/
    long    su_cnt;                         /* 매수 계약수					*/
    char    su_money_gbn[1];                /* 매수 금액 체크여부			*/
    double  su_money;                       /* 매수 금액					*/
}   CHK_RISK_TOT;

/* 20211028 */

/* 전략Process별 사용할 주문번호를 Define해 둔다. pa_9001_mp가 기동시 초기화 한다.	*/
/* int는 +- 21억까지만 표현이 되어서 long으로 선언하였음 							*/
typedef struct
{
    long    START_JMNO;             /* 시작 주문번호, Process 별 주문번호 사용 대역(Range)  */
    long    USED_JMNO;              /* 사용중인 주문번호, 주문발주시 +1해서 처리함            */
    long    END_JMNO;               /* 끝  주문번호, Process 별 주문번호 사용 대역(Range)  */
}   ORDER_NO;

/* 202506, 내용 추가 
   RDS의 TRDESP01901 (가격단위규칙) 파일을 활용해서 체크를 해야한다. 
   채권은 3가지만 있어서 파일까지는 필요가 없어보이나 , 금융상품파생(06F)은 어떤 값이 있는지 알 수 없음
   TRDESP01901를 수신해서 처리하는게 정석임.
*/
/* 시장 공유메모리로 빼서 처리한다. 9001mp에서 초기화시 읽어서 처리 */
#if 0
typedef struct {
    int     hoga_depth;     /* arry 0에서만 사용 */
    double  band_price;
    double  band_unit;
    double  band_sum;
}   HO_CHE;
#endif

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
	int     system_down;						/* 비상용으로 값이 0이외 주문송신을 하지 않음 */
												/* Arry 0이면 모든 시스템, 1이면 채권중단, 2면 금융파생중단 */
    char    business_day[8];
	int		diff_calday;
    char    open_market_info[RISK_MK_CNT][2];    /* G1등 시세기준 3개정보, 보드ID */
    char    sub_market_info[RISK_MK_CNT][3];     /* AA1등 모드이벤트 */

#if 0
2025
    /* ******************************************** */
    /* 계좌로 주문번호를 나눌때 사용, 여기서는 전략Process단위로 나눔, 미사용
	   주문번호대역(서버로 나누어야함)
        - 주식 : 1530000000 ~ 1559999999 (3천만건)
        - 파생 : 1580000000 ~ 1609999999 (3천만건)  */
    /* ******************************************** */
    int     js_order_no_band;   /* 현물주문번호 */
    int     dv_order_no_band;   /* 파생주문번호 */

/* 미사용 */
    double  mk_hoga_dan_d[RISK_MK_CNT];          /* 시장별 1호가단위 ex) 0.01 */
    double  mk_hoga_val_d[RISK_MK_CNT];          /* 시장별 1호가 가격        */
#endif

/* 20211209 */
    double  cd_rate;                        /* CD금리, 전략에서 사용    */
    double  indv_rate[RISK_MK_CNT];              /* 위탁증거금율, KRX300만 사용   */
                                            /* 01 : 코스피200
                                               04 : 코스피200변동성지수
                                               05 : 미니코스피
                                               06 : 코스닥150
                                               07 : 유로스톡스
                                               08 : KRX300
                                               ......                   */
    BATCH_CNT       Batch_Cnt[7];           /* 배치 수신 및 처리건수 체크(아침체크) , 계좌정보 송/수신 건수처리(6000mp)   */
											/* 7인 이유는 내려받는 파일이 총 7개여서임. 이것도 고객사에 따라서 달라짐		*/
    AUTO_STAT       Auto_Stat[MAX_AUTO_PROC];   /* 자동전략 상태      */
/* 20211209 */
//  double  each_price[RISK_MK_CNT][SHM_MAX_STOCK];      /* 시장별 종목별 현재가, 장전은 기준가 */
    /* ************************************************************************ */
    /* 종목별 손익/한도 관리                                                 */
    //PROFIT            ProFit[RISK_MK_CNT][ACC_NO_CNT]; /*  0:kospi,    1:kosdaq, 
    //PROFIT          ProFit[RISK_MK_CNT][SHM_MAX_NOTE][ACC_NO_CNT];
    PROFIT          ProFit[RISK_MK_CNT][SHM_MAX_DEV_FIF][ACC_NO_CNT];
												/* 시장별 가장 큰 수량으로, 0:채권_10000, 1:금융상품선물_1000	*/
                                                /* 1000을 나눈값으로 보관       */
    /* ************************************************************************ */
/* 20211024 */
    int     Max_Seq[RISK_MK_CNT];
//    STANDARD_SISE_FORMAT            S_Sise[RISK_MK_CNT][SHM_MAX_NOTE];
    STANDARD_SISE_FORMAT            S_Sise[RISK_MK_CNT][SHM_MAX_DEV_FIF];
    FX_SISE_FORMAT					FX_Sise[1][SHM_MAX_FX];

/* 20211028 */
    MST_ACCNO_MASTER                Mst_Acc[ACC_NO_CNT];
    CHK_RISK_ONE                    O_M_Fund[RISK_MK_CNT][ACC_NO_CNT];  // 1회한도(설정값),  Onetime Set Management
    CHK_RISK_TOT                    T_M_Fund[RISK_MK_CNT][ACC_NO_CNT];  // 누적한도(설정값), Total   Set Management
    CHK_RISK_TOT                    T_S_Fund[RISK_MK_CNT][ACC_NO_CNT];  // 누적한도 사용값,  Total   Use(sayoung) 
/* 20211028 */
    ORDER_NO    Order_No[2][MAX_AUTO_PROC];         /* 자동주문 프로세스(pa_50101mp) 갯수 */
                                                    /* 0:채권, 1:파생   */
/* 202506 제외시킴, 현물용으로 처리했었음
	JS_Tick_Chk, Tick_Chk   : pa_1100_ts.c 에서 사용, 미사용으로 변경필요
	Hoga_Change				: pa_5020_mp.c 에서 사용, 미사용으로 변경필요
	---------------------------------------------------------------------
    HO_CHE      Ho_Chk[RISK_MK_CNT+2][7];
*/
}   RISK;       /* 종목별 한도/수량/평가손익 관리 Memory */

/* ************************************************************************************ */
/* 종목코드를 키로 하여 qsort와 bsearch를 하여 보다 빠르게 arry의 위치를 찾기 위한 처리 */
/* ************************************************************************************ */
typedef struct
{
    KS_NOTE_EXPCODE		N_Key[SHM_MAX_NOTE];			//  0.채권
    KS_EXPCODE      	D_Key[SHM_MAX_DEV_FIF];			//  1.금융상품파생
    KS_EXPCODE      	D_N_Key[SHM_MAX_DEV_FIF_N];		//  2.금융상품파생야간
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

#define     SHM_MAX_SUB     26                      /* A ~ Z    */
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
        (7)(8)  - 01:KTS채권, 02:금융상품선물
------------------------------------------------------------------------*/
#define     NOTE_SHM_KEY    0x21000001L             /* KTS 채권		*/
#define     RDS01_SHM_KEY	0x21000002L             /* RDS01		*/
#define     FF_SHM_KEY      0x21000003L             /* 금융상품선물	*/
#define     FF_N_SHM_KEY    0x21000004L				/* 금융상품선물	야간	*/
#define     FX_SHM_KEY      0x21000005L             /* 금융상품		*/

#define     B_SHM_KEY       0x21000011L             /* BackOffice   */
#define     STRRG_SHM_KEY   0x21000012L             /* 전략SHM        */
#define     MK_PM_SHM_KEY   0x21000013L             /* 미체결처리용   */
#define     RISK_SHM_KEY    0x21000014L             /* 한도관리SHM  */

#define     ITEM_SHM_KEY    0x21000030L             /* Key Sort용    */

#define     INFO(i)         SHM_All_Daemon_Info[i]
#define     DAEMON(i)       Shm_Mem[i].Daemon[0]
#define     PROC(i,j)       Shm_Mem[i].Proc[j]
#define     FILEM(i,j)      Shm_Mem[i].File[j]
#define     DSHM(i,j)       Shm_Mem[i].DShm[j]
#define     TCP1(i,j)       Shm_Mem[i].Tcp1[j]
#define     TCP2(i,j)       Shm_Mem[i].Tcp2[j]
#define     UDPIP(i,j)      Shm_Mem[i].Udpip[j]
#define     SISETR(i,j)     Shm_Mem[i].Sisetr[j]

#if defined ISAM_INCL
#define     CISAM(i,j)      Shm_Mem[i].Cisam[j]
#endif

#define     IF_SEQ(i,j)             PROC(i,j).if_seq
#define     IF_MEG_SEQ(i,j,k)		PROC(i,j).if_meg_seq[k]
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
#define     UDP_IF_IP1(i,j)		UDPIP(i,PROC(i,j).l.u-1).if_ip_addr[0]
#define     UDP_IF_IP2(i,j)		UDPIP(i,PROC(i,j).l.u-1).if_ip_addr[1]
#define     UDP_IF_IP3(i,j)		UDPIP(i,PROC(i,j).l.u-1).if_ip_addr[2]
#define     UDP_IF_IP4(i,j)		UDPIP(i,PROC(i,j).l.u-1).if_ip_addr[3]
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

/* 2025 */
int             SISE_Note_Shmid;
SHM_NOTE		*Shm_Note;				/* 1.KTS 국채					*/
int             RDS_01_Shmid;
CO_A001_RDS01	*Shm_Rds01;				/* 1.채권종목정보(너무많아서 뺏다) */
int             SISE_FF_Shmid;
SHM_FIN_FUT     *Shm_FinFut;			/* 2.금융상품선물               */
int             SISE_FF_N_Shmid;
SHM_FIN_FUT     *Shm_FinFut_N;			/* 3.금융상품선물야간           */
int             SISE_FX_Shmid;
SHM_FX			*Shm_FX;				/* 2.FX							*/

int             SISE_Mk_Prematch_Shmid;
MK_PREMATCH     *Shm_Mk_PreMatch;       /* 3.시장별 미결제정보         */
int             SISE_Strrg_Shmid;
STRRG           *Shm_Strrg;             /* 4.계좌별 전략영역          */
int             SISE_Risk_Shmid;
RISK            *Shm_Risk;              /* 5.한도관리                  */
int             SISE_Item_Shmid;
SHM_KEY_ARRY    *Shm_Item;              /* 6.종목코드 qsort/besearch   */
/* 2025 */

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

#if 0
extern  int             SISE_F_Shmid;
extern  SHM_FUTURES     *Shm_Futures;           /* 1.지수선물시세               */
#endif
/* 2025 */
extern  int             SISE_Note_Shmid;
extern  SHM_NOTE		*Shm_Note;				/* 1.KTS 국채					*/
extern	int             RDS_01_Shmid;
extern	CO_A001_RDS01	*Shm_Rds01;				/* 1.채권종목정보(너무많아서 뺏다) */
extern  int             SISE_FF_Shmid;
extern  SHM_FIN_FUT     *Shm_FinFut;			/* 2.금융상품선물               */
extern  int             SISE_FF_N_Shmid;
extern  SHM_FIN_FUT     *Shm_FinFut_N;			/* 3.금융상품선물야간           */
extern	int             SISE_FX_Shmid;
extern	SHM_FX			*Shm_FX;				/* 4.FX							*/

extern  int             SISE_Mk_Prematch_Shmid;
extern  MK_PREMATCH     *Shm_Mk_PreMatch;       /* 3.시장별 미결제정보         */
extern  int             SISE_Strrg_Shmid;
extern  STRRG           *Shm_Strrg;             /* 4.계좌별 전략영역          */
extern  int             SISE_Risk_Shmid;
extern  RISK            *Shm_Risk;              /* 5.한도관리                  */
extern  int             SISE_Item_Shmid;
extern  SHM_KEY_ARRY    *Shm_Item;              /* 6.종목코드 qsort/besearch   */
/* 2025 */

extern int              P_Semid;
extern int              DP_Semid;

#endif

/*************************************************************************
    End of Program (shm_memory.h)
*************************************************************************/
#endif
