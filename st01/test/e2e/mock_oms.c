/*------------------------------------------------------------------------
#   Module  : E2E 하니스 - Mock OMS (주문 주입 서버)
#   File    : mock_oms.c
#
#   pb_1301_tr(주문수신, TCP 클라이언트)이 접속해 오는 OMS(AP서버)를
#   시뮬레이션한다. listen → accept → N건 주문을 CLI framing
#   ([4바이트 ASCII 길이][전문 254B][LF])으로 push.
#
#   전문: 채권일반호가 신규주문 (KRX_NOTE_JUMUN_DATA 254B,
#         order_inject.c와 동일 레이아웃). OrderNo는 1부터 증가.
#
#   송신 직전 시각을 mock_oms.lat에 기록 (lat_report 입력 포맷,
#   proc명 "e2e" — pb_1101_ts.lat의 OUT을 proc명 치환해 합치면
#   주입→KRX송신 엔드투엔드 구간 측정 가능).
#
#   Usage: mock_oms <port> <count> [interval_ms]
------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#define ORDER_LEN   254         /* 채권일반호가 TCHODR40001 */
#define ORDER_LEN_D 294         /* 파생/현물 호가입력 TCHODR10001 (KRX_JUMUN_DATA) */
#define ORDER_LEN_MAX 294

static void right_align(char *dst, int len, const char *src)
{
    int slen = (int)strlen(src);
    memset(dst, ' ', len);
    if (slen >= len)
        memcpy(dst, src, len);
    else
        memcpy(dst + len - slen, src, slen);
}

/* 채권일반호가 신규주문 전문 254B 구성 (order_inject.c 레이아웃) */
static void build_order(char *buf, int seq, const char *today)
{
    char tmp[32];

    memset(buf, ' ', ORDER_LEN);

    sprintf(tmp, "%011d", seq);
    memcpy(buf + 0,   tmp, 11);                     /* DataSeq          */
    memcpy(buf + 11,  "TCHODR40001", 11);           /* Transaction_Code */
    memcpy(buf + 22,  "01", 2);                     /* Megrp_no         */
    memcpy(buf + 24,  "K  ", 3);                    /* Mkt_Id           */
    memcpy(buf + 27,  "01", 2);                     /* Board_id         */
    memcpy(buf + 29,  "99999", 5);                  /* MembershipNo     */
    memcpy(buf + 34,  "00001", 5);                  /* BranchNo         */
    sprintf(tmp, "%010d", seq);
    memcpy(buf + 39,  tmp, 10);                     /* OrderNo          */
    /* OriginalOrderNo(49,10) = 공백 (신규)                              */
    memcpy(buf + 59,  "KR7005930003", 12);          /* ItemCode         */
    buf[71] = '2';                                  /* TradeFlag 매수   */
    buf[72] = '1';                                  /* New(1)           */
    memcpy(buf + 73,  "100000000001", 12);          /* AccountNo        */
    right_align(buf + 85, 10, "100");               /* OrderQuantity    */
    right_align(buf + 95, 11, "98500");             /* Price            */
    buf[106] = '1';                                 /* Order_Type 지정가 */
    buf[107] = '0';                                 /* Order_Condition  */
    /* Ask_Type(108,2) 공백 */
    memcpy(buf + 110, "01", 2);                     /* Trust_Principal  */
    /* Trust_Company_No(112,5) 공백 */
    memcpy(buf + 117, "01", 2);                     /* Account_Type     */
    memcpy(buf + 119, "KR ", 3);                    /* Country_Code     */
    memcpy(buf + 122, "1000", 4);                   /* Investor_Type    */
    buf[135] = 'T';                                 /* Order_Mesia_Type (126+6+2+1=135) */
    /* Order_Identi(136,12), Mac_Addr(148,12) 공백                       */
    memcpy(buf + 160, today, 8);                    /* Order_Date       */
    /* Member_Send_Time(168,9), MembershipItem(177,60) 아래에서          */
    memcpy(buf + 177 + 35, "07", 2);                /* 시장구분=KTS      */
}

/* 파생/현물 호가입력 신규주문 전문 294B 구성 (KRX_JUMUN_DATA / TCHODR10001)
   채권(254)과 달리 Mkt_Id(3) 없음 → Board_Id가 offset 24. pc_1100_ts는 TrCode(@11)와
   크기(294)만 판정하고 본문은 pass-through하므로, 추적용 핵심 필드만 채우고 나머지 SPACE. */
static void build_deriv_order(char *buf, int seq, const char *today)
{
    char tmp[32];

    memset(buf, ' ', ORDER_LEN_D);

    sprintf(tmp, "%011d", seq);
    memcpy(buf + 0,   tmp, 11);                     /* DataSeq                    */
    memcpy(buf + 11,  "TCHODR10001", 11);           /* Transaction_Code (신규호가)*/
    memcpy(buf + 22,  "00", 2);                     /* Me_Grp_No (회원사 '00')    */
    memcpy(buf + 24,  "G1", 2);                     /* Board_Id                   */
    memcpy(buf + 26,  "99999", 5);                  /* Member_Number              */
    memcpy(buf + 31,  "00001", 5);                  /* Branch_Number              */
    sprintf(tmp, "%010d", seq);
    memcpy(buf + 36,  tmp, 10);                     /* Order_Identification       */
    /* Original_Order_Identification(46,10) = SPACE (신규)                        */
    memcpy(buf + 56,  "KR4101K30000", 12);          /* Issue_Code (파생 예시)     */
    buf[68] = '2';                                  /* Ask_Bid_Type 매수          */
    buf[69] = '1';                                  /* Modify_Or_Cancel 신규      */
    memcpy(buf + 70,  "100000000001", 12);          /* Account_Number             */
    right_align(buf + 82, 10, "1");                 /* Order_Quantity             */
    right_align(buf + 92, 11, "30000");             /* Order_Price                */
    buf[103] = '2';                                 /* Order_Type (파생 1불가→지정)*/
    buf[104] = '0';                                 /* Order_Condition 일반        */
    right_align(buf + 105, 10, "0");                /* Min_Trdvol (회원사 '0')     */
    buf[115] = '0';                                 /* Mm_Ord_Tp_Cd               */
}

static int send_all(int fd, const char *buf, int len)
{
    int sent = 0, rt;
    while (sent < len) {
        rt = (int)send(fd, buf + sent, len - sent, 0);
        if (rt <= 0)
            return -1;
        sent += rt;
    }
    return len;
}

int main(int argc, char *argv[])
{
    int     port, count, interval_ms = 0;
    int     lfd, cfd, k, on = 1;
    int     order_len, is_deriv = 0;
    const char *kind;
    struct  sockaddr_in addr;
    char    order[ORDER_LEN_MAX], hdr[8], today[16], t9[16];
    char    ordno[16];
    FILE    *lat;
    struct  timeval tv;
    struct  tm *tp;
    time_t  now;
    long long usec;

    if (argc < 3) {
        printf("Usage: mock_oms <port> <count> [interval_ms]\n");
        return 1;
    }

    port  = atoi(argv[1]);
    count = atoi(argv[2]);
    if (argc > 3) interval_ms = atoi(argv[3]);

    /* 주문 종류: 환경변수 MOCK_OMS_KIND=deriv → 파생 294B, 기본 bond 254B */
    kind = getenv("MOCK_OMS_KIND");
    if (kind != NULL && strcmp(kind, "deriv") == 0) {
        is_deriv  = 1;
        order_len = ORDER_LEN_D;
    }
    else {
        is_deriv  = 0;
        order_len = ORDER_LEN;
    }

    time(&now);
    tp = localtime(&now);
    sprintf(today, "%04d%02d%02d", tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday);

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((unsigned short)port);

    if (bind(lfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("[MOCK_OMS] bind fail :%d (%s)\n", port, strerror(errno));
        return 1;
    }
    listen(lfd, 1);
    printf("[MOCK_OMS] listening :%d, waiting for pb_1301_tr...\n", port);
    fflush(stdout);

    cfd = accept(lfd, NULL, NULL);
    if (cfd < 0) {
        printf("[MOCK_OMS] accept fail (%s)\n", strerror(errno));
        return 1;
    }
    setsockopt(cfd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    printf("[MOCK_OMS] pb_1301_tr connected. injecting %d order(s)...\n", count);
    fflush(stdout);

    lat = fopen("mock_oms.lat", "w");

    for (k = 1; k <= count; k++) {
        if (is_deriv)
            build_deriv_order(order, k, today);
        else
            build_order(order, k, today);

        /* 회원사주문시각 (bond 레이아웃 offset 168; deriv는 mock 추적에 불필요 → skip) */
        if (!is_deriv) {
            gettimeofday(&tv, NULL);
            tp = localtime(&tv.tv_sec);
            sprintf(t9, "%02d%02d%02d%03d", tp->tm_hour, tp->tm_min, tp->tm_sec,
                    (int)(tv.tv_usec / 1000));
            memcpy(order + 168, t9, 9);
        }

        /* framing: 4바이트 길이 = 전문 + LF(1) */
        sprintf(hdr, "%04d", order_len + 1);

        gettimeofday(&tv, NULL);
        usec = (long long)tv.tv_sec * 1000000LL + tv.tv_usec;
        sprintf(ordno, "%010d", k);
        if (lat != NULL)
            fprintf(lat, "%lld|e2e|IN|%s\n", usec, ordno);

        if (send_all(cfd, hdr, 4) < 0 ||
                send_all(cfd, order, order_len) < 0 ||
                send_all(cfd, "\n", 1) < 0) {
            printf("[MOCK_OMS] send fail at %d (%s)\n", k, strerror(errno));
            break;
        }

        if (interval_ms > 0 && k < count)
            usleep((useconds_t)interval_ms * 1000);
    }

    if (lat != NULL)
        fclose(lat);

    printf("[MOCK_OMS] done (%d sent). holding connection 5s...\n", k - 1);
    fflush(stdout);
    sleep(5);

    close(cfd);
    close(lfd);
    return 0;
}

/*************************************************************************
    End of Program (mock_oms.c)
*************************************************************************/
