#include <stdio.h>

typedef struct _xchg
{
    int     exid            ;   /* exchange id */
    char    exnm        [ 8];   /* short name */
    char    excode      [ 1];   /*  S MBS/ K MBS/E MBS/ C MBS/ B est/ Z CUST */
    char    TZ          [60];   /* time zone */
    int     maxcnt          ;   /* maximum no of symbol for shared memory */
    int     arbitrage       ;   /* 재정환율계산여부 */
    int     sendclient_flag ;   /* 클라이어언트 시세 전용 여부 */
    int     db_pnum         ;   /* number of db update threads */
        char    recv_name    [16];   /* port id (=product name) */
        char    recv_ipad    [20];   /* ip address */
        int     recv_port        ;   /* port number */
        int     apsnd_cust        ;   /* CUST 전송여부 */
        int     apsnd_cast        ;   /* AP 전송여부 */
        int     apsnd_best        ;   /* BEST 전송여부 */
        char    apsnd_neta    [20];   /* local address */
        char    apsnd_ipad    [20];   /* multicasting IP-address */
        int     apsnd_port        ;
    char    quenm      [128];   /* 시세 수신 큐이름 (exchange.cfg) */
    char    dirp       [128];   /* data file path */
    int     llog            ;   /* log level */
    char    logf       [128];   /* log path */
} XCHG;


typedef struct _mdarch
{
    XCHG    xchg            ;   /* exchange information */
    int     sizefold        ;
    time_t  rtim            ;   /* receive time stamp */
    int     rsum            ;   /* daily total */
    int     tymd            ;   /* 영업일 */
    int     mrec            ;   /* max record */
    int     nrec            ;   /* current record numbers of whereis */
} MDARCH;                       /* shared memory pointer */

typedef struct _kswitch_t
{
    char userid     [16];       /* 관리자ID */
    char usdkrw_exnm[8];        /* USD/KRW 브로커 */
    int all_flag;               /* 전체 O:OFF 1:ON */
    int run_flag;               /* 개별 0:OFF 1:ON */
    double value;               /* USD/KRW:Available bid/offer Spread, Other: 시초가대비 등락율 */
} kswitch_t;



typedef struct _candle
{
        double open             ;
        double open_tm          ;
        double high             ;
        double high_tm          ;
        double lowp             ;
        double lowp_tm          ;
        double clos             ;
        double clos_tm          ;
        double base             ;
        double last_valid       ;
}	CANDLE;

typedef struct _MDCANDLE
{
    char    exnm            [ 8];
    char    symb            [ 8];
    int     kymd                ;
    int     khms                ;
    int     tymd                ;
    int     seqn                ;
    char    tenor           [ 8];
	CANDLE	bid;
	CANDLE	ask;
	CANDLE	mid;
    char    tickkey         [32];   /* exnm(4) + symb(6) + 날짜(8) + 시간(9) */
    int     tenor_idx           ;
} MDCANDLE;


typedef struct _mdside_t
{
    char     excode     [ 1];           /* BID 원천 : 'S'MBS/'K'MBS/E'MBS/'C'MBS */
    double   open           ;
    double   open_tm        ;
    double   high           ;
    double   high_tm        ;
    double   lowp           ;
    double   lowp_tm        ;
    double   last           ;
    double   last_tm        ;
    double   lastvol        ;
    double   last_valid     ;           /* last 중'0'이 아닌값 */
    double   base           ;           /* 전일종가 */
    double   best           ;
    double   bestvol        ;
    int      sign           ;
    double   diff           ;
    double   rate           ;
    int      dirf           ;
} mdside_t;

typedef struct _mdquot_t
{
    double  swap_bid        ;
    double  swap_ask        ;
    double  swap_bid_man    ;           /* 수기입력 */
    double  swap_ask_man    ;           /* 수기입력 */
    mdside_t bid            ;           /* */
    mdside_t ask            ;           /* */
    mdside_t mid            ;           /* */
    int      tick_seqn      ;
    MDCANDLE mdintr         ;
    char     tickkey[32]    ;
} mdquot_t;

typedef struct _mdfold {
    char     symb       [8];
    int      seqn           ;           /* MDFOLD OFFSET */
    int tymd           ;           /* current trading day */
    int kymd           ;           /* last update date */
    int khms           ;           /* last update time */
    int      zdiv           ;           /* 직원 소수점자리수 */
    int      custzdiv       ;           /* 고객 소수점자리수 */
    int      swapzdiv       ;           /* 스왑 소수점자리수 */
    int      mrktzdiv       ;           /* 마켓 소수점자리수 */
    int      feed           ;           /* data feeder id number */
    int      trdf           ;           /* tradable flag */
    int      open_hms       ;           /* 장시작시간(hhmmss); */

    mdquot_t mdquot[10]     ;           /* SPT, TOD, TOM, W01, M01, M02, M03, M06, Y01 */

    int      crossrate      ;           /* 크로스레이트 여부 */
    char     fillid[32]     ;
    int      fillseq        ;           /* 체결일련번호 */
    double   fillprc        ;           /* 체결가격 */
    double   fillqty        ;           /* 체결수량 */
    kswitch_t ks            ;           /* 킬스위치 */
    int      close_hms      ;           /* 장종료시간 */
    time_t   kswitch_time_usd;          /* USD/KRW 킬스위치 작동 시간 */
    time_t   kswitch_time_etc;          /* 기타통화 킬스위치 작동시간 */
    int      best_flag;
    char     filler[256-32]    ;
} MDFOLD;

