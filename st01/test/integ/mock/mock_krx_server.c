/*------------------------------------------------------------------------
#   Module  : Mock KRX TCP Server
#   File    : mock_krx_server.c
#   Purpose : Standalone TCP server that emulates KRX protocol exchange.
#             Runs in foreground, handles one connection, then exits.
#
#   Usage   : mock_krx_server [port]
#             Default port: 19999
#
#   Protocol flow:
#     accept → recv LOGON(SCHLIQ) → send LOGON_RESP(SCHLIR)
#            → recv LINK(SCHOPQ)  → send LINK_RESP(SCHOPR)
#            → recv DATA(TCHODR*) → send DATA_RESP
#            → recv POLL(SCHHEQ)  → send POLL_RESP(SCHHER)
#            → recv LOGOFF(SCHLOQ) → close
------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include "../lib/krx_protocol.h"
#include "mock_krx_server.h"
#include "../../vexch/vexch_catalog.h"   /* VX-3: 공유 카탈로그 */

static int g_listen_fd = -1;
static int g_client_fd = -1;
static FILE *g_logfp   = NULL;

/*------------------------------------------------------------------------
    파생 등록용 payload 빌더 (pc_→SEAM→po_ 통합 E2E 전용)

    payload = 전문 DATA부에서 BODY_COMMON(24: DataSeq/TrCode/Megrp) 제외분.
    po_ 구조체 오프셋 X(>=24) 는 payload[X-24] 에 대응(BODY_COMMON 선행).
------------------------------------------------------------------------*/
static void build_ttrodp11301_payload(char *p)   /* 회원처리호가 정상, 294B */
{
    memset(p, ' ', 294);
    memcpy(p + 12,  "0000000002", 10);   /* Order_Identification @36     */
    memcpy(p + 32,  "KR4101SC0009", 12); /* Issue_Code_Symbol @56        */
    p[44] = '2';                         /* Ask_Bid_Type_Code @68 매수   */
    p[45] = '1';                         /* Modify_Or_Cancel @69 신규    */
    memcpy(p + 46,  "200000000002", 12); /* Account_Number @70           */
    memcpy(p + 58,  "0000000100", 10);   /* Order_Quantity @82 =100      */
    memcpy(p + 68,  "00000010000", 11);  /* Order_Price @92              */
    p[79] = '2';                         /* Order_Type_Code @103         */
    p[80] = '0';                         /* Order_Condition_Code @104    */
    /* Member_Use_Area @202 → payload[178] (매체30/시장35/종목37/계좌42) */
    p[178 + 30] = 'C';
    p[178 + 35] = '1';
    memcpy(p + 178 + 37, "00001", 5);
    memcpy(p + 178 + 42, "01", 2);
}

static void build_ttrtdp21301_payload(char *p)    /* 회원체결결과, 209B */
{
    memset(p, ' ', 209);
    memcpy(p + 12,  "0000000002", 10);   /* Order_Identification @36     */
    memcpy(p + 32,  "KR4101SC0009", 12); /* Issue_Code @56               */
    memcpy(p + 55,  "00000010000", 11);  /* Trading_Price @79            */
    memcpy(p + 66,  "0000000100", 10);   /* Trading_Volumn @90 =100      */
    p[117] = '2';                        /* Ask_Bid_Type_Code @141 매수  */
    memcpy(p + 118, "200000000002", 12); /* Account_Number @142          */
    /* Member_Use_Area @172 → payload[148] (시장35/종목37/계좌42) */
    p[148 + 35] = '1';
    memcpy(p + 148 + 37, "00001", 5);
    memcpy(p + 148 + 42, "01", 2);
}

static void build_ttrodp41301_payload(char *p)    /* 채권 회원처리호가, 294B (payload=전문-24) */
{
    memset(p, ' ', 294);
    memcpy(p + 15,  "0000000001", 10);   /* OrderNo @39                  */
    memcpy(p + 35,  "KR6000000001", 12); /* Issue_Code @59 (채권)        */
    memcpy(p + 61,  "0000000100", 10);   /* Order_Quantity @85 =100      */
    /* 회원사용영역(echo 대상, payload[178]) */
    p[178 + 30] = 'C'; p[178 + 35] = '1';
    memcpy(p + 178 + 37, "00001", 5);
    memcpy(p + 178 + 42, "01", 2);
}
static void build_ttrtdp42301_payload(char *p)    /* 채권 회원체결결과, 233B */
{
    memset(p, ' ', 233);
    memcpy(p + 15,  "0000000001", 10);   /* OrderNo @39                  */
    memcpy(p + 82,  "0000000100", 10);   /* Trading_Volumn @106 =100     */
    p[148 + 35] = '1';
    memcpy(p + 148 + 37, "00001", 5);
    memcpy(p + 148 + 42, "01", 2);
}

/*------------------------------------------------------------------------
    Logging
------------------------------------------------------------------------*/
static void mock_log(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    if (g_logfp) {
        vfprintf(g_logfp, fmt, ap);
        fprintf(g_logfp, "\n");
        fflush(g_logfp);
    }

    va_end(ap);

    /* Also print to stdout */
    va_start(ap, fmt);
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
}

/*------------------------------------------------------------------------
    Reliable send/recv (handle partial I/O)
------------------------------------------------------------------------*/
static int mock_sendn(int fd, const char *buf, int len)
{
    int sent = 0, rt;
    while (sent < len) {
        rt = (int)send(fd, buf + sent, len - sent, 0);
        if (rt <= 0) return -1;
        sent += rt;
    }
    return sent;
}

static int mock_recvn(int fd, char *buf, int len)
{
    int rcvd = 0, rt;
    while (rcvd < len) {
        rt = (int)recv(fd, buf + rcvd, len - rcvd, 0);
        if (rt <= 0) return (rt == 0) ? 0 : -1;
        rcvd += rt;
    }
    return rcvd;
}

/*------------------------------------------------------------------------
    Signal handler for clean shutdown
------------------------------------------------------------------------*/
static void signal_handler(int sig)
{
    mock_log("MOCK: Signal %d received, shutting down", sig);
    if (g_client_fd >= 0) close(g_client_fd);
    if (g_listen_fd >= 0) close(g_listen_fd);
    if (g_logfp) fclose(g_logfp);
    exit(0);
}

/*------------------------------------------------------------------------
    Receive one KRX message (header first, then body if any)
    Returns total message length, 0 for disconnect, -1 for error.
------------------------------------------------------------------------*/
static int mock_recv_krx_msg(int fd, char *buf, int bufsize)
{
    int rt, body_len, total;

    /* Receive header (82 bytes) */
    rt = mock_recvn(fd, buf, KRX_HDR_LEN);
    if (rt <= 0) return rt;

    /* Parse BodyLength field */
    body_len = krx_get_body_length(buf);
    if (body_len < 0 || body_len > bufsize - KRX_HDR_LEN) {
        mock_log("MOCK: Invalid BodyLength=%d", body_len);
        return -1;
    }

    total = KRX_HDR_LEN + body_len;

    /* Receive body if any */
    if (body_len > 0) {
        rt = mock_recvn(fd, buf + KRX_HDR_LEN, body_len);
        if (rt <= 0) return rt;
    }

    return total;
}

/*------------------------------------------------------------------------
    Get MsgType from buffer (11 chars at offset 14)
------------------------------------------------------------------------*/
static void get_msg_type(const char *buf, char *msg_type)
{
    memcpy(msg_type, buf + KRX_OFF_MSGTYPE, 11);
    msg_type[11] = '\0';
}

/*------------------------------------------------------------------------
    Handle one client connection
------------------------------------------------------------------------*/
/*==== VX-3: 공유 카탈로그(vexch_catalog) — 상품 추가 = cfg/vexch.ini 블록 1개 ====*/
static void vx_log(const char *m) { mock_log("%s", m); }

/*==== VX-1b-full: order book (주문↔체결 상관) ====
   주문(TCHODR) 수신 → 회원사용영역(60B) + OrderNo 채번 → book.
   응답/체결 push 시 booked 주문의 회원영역+OrderNo 를 payload 에 echo
   → 전략이 자기 주문을 회원처리항목으로 매칭. (echo 없으면 template 기본)  */
#define VX_ORD_MEMBER_OFF   202   /* 주문 전문 회원사용영역(60B) 오프셋(테스트 규약) */
#define VX_RESP_MEMBER_OFF  178   /* 응답/체결 payload 회원영역 오프셋 (@202 - BODY_COMMON 24) */
#define VX_RESP_ORDNO_OFF    12   /* payload Order_Identification (@36 - 24) */
typedef struct { char member[60]; int product_idx, orderno, filled; } VX_ORDER;
static VX_ORDER g_book[64];
static int      g_book_cnt = 0, g_next_ordno = 60000;

static int vx_book_order(int product_idx, const char *body, int body_len)
{
    VX_ORDER *o;
    if (g_book_cnt >= 64) return (-1);
    o = &g_book[g_book_cnt++];
    memset(o, 0, sizeof(*o));
    o->product_idx = product_idx;
    o->orderno = ++g_next_ordno;
    if (body_len >= VX_ORD_MEMBER_OFF + 60)
        memcpy(o->member, body + VX_ORD_MEMBER_OFF, 60);
    return o->orderno;
}
static VX_ORDER *vx_find_booked(int product_idx)   /* 상품의 미체결 booked 주문 */
{
    int i;
    for (i = 0; i < g_book_cnt; i++)
        if (!g_book[i].filled && g_book[i].product_idx == product_idx) return &g_book[i];
    return NULL;
}

/* payload 디스패치: 알려진 TR은 실 필드채움, 기타는 blank 템플릿(정상 envelope) */
static int vx_build_payload(const char *tr, char *p, int size)
{
    memset(p, ' ', size);
    if (!strncmp(tr, "TTRODP11301", 11)) { build_ttrodp11301_payload(p); return 294; }
    if (!strncmp(tr, "TTRTDP21301", 11)) { build_ttrtdp21301_payload(p); return 209; }
    if (!strncmp(tr, "TTRODP41301", 11)) { build_ttrodp41301_payload(p); return 294; }
    if (!strncmp(tr, "TTRTDP42301", 11)) { build_ttrtdp42301_payload(p); return 233; }
    return size;   /* 기타 상품: order book 상관(VX-1b)에서 실 필드채움 */
}
/* 카탈로그 구동 push: enabled 상품마다 응답(+fill_rule 체결) 발행 */
/* booked 주문의 OrderNo + 회원사용영역을 payload 에 echo (correlation) */
static void vx_echo_order(char *pl, int plen, VX_ORDER *bo)
{
    char no[12];
    if (!bo) return;
    snprintf(no, sizeof(no), "%010d", bo->orderno);
    if (plen >= VX_RESP_ORDNO_OFF + 10)  memcpy(pl + VX_RESP_ORDNO_OFF, no, 10);
    if (plen >= VX_RESP_MEMBER_OFF + 60) memcpy(pl + VX_RESP_MEMBER_OFF, bo->member, 60);
}
static int g_push_seq = 0;   /* push 세션 전역 단조 MsgSeqNum (SCHOPQ서 리셋) */
static void vx_push_catalog(int fd, char *resp, int respcap, int only_ci)
{
    char pl[2048];
    int  ci, plen, push_len, ms;

    for (ci = 0; ci < vx_cat_cnt; ci++) {
        VX_PRODUCT *pr = &vx_cat[ci];
        VX_ORDER   *bo;
        if (only_ci >= 0 && ci != only_ci) continue;   /* 주문 왕복: 해당 상품만 */
        if (!pr->enabled) continue;
        if (!pr->resp_tr[0]) continue;   /* 시세-only 상품(FX 등)은 TCP 주문/체결 push 제외 */

        bo = vx_find_booked(ci);          /* 이 상품의 booked 주문(있으면 correlation) */

        plen = vx_build_payload(pr->resp_tr, pl, pr->resp_size);
        vx_echo_order(pl, plen, bo);
        ms = ++g_push_seq;
        push_len = krx_build_data_push(resp, respcap, KRX_DEFAULT_SENDER,
                                       pr->wrapper, pr->resp_tr, ms, ms, pl, plen);
        if (push_len > 0) { mock_sendn(fd, resp, push_len);
            mock_log("VX: push RESP %s (%d B) seq=%d [%s]%s", pr->resp_tr, push_len, ms, pr->name,
                     bo ? " CORRELATED(OrderNo+member echo)" : ""); }
        usleep(300000);

        if (!strcmp(pr->fill_rule, "ack")) {
            /* 응답만(체결 없음) */
        }
        else if (!strcmp(pr->fill_rule, "reject")) {
            mock_log("VX: %s REJECTED (fill_rule=reject, 체결 없음) [%s]", pr->order_tr, pr->name);
            if (bo) bo->filled = 1;       /* 거부도 book 소진 */
        }
        else {   /* full | partial : 체결 push */
            plen = vx_build_payload(pr->exec_tr, pl, pr->exec_size);
            vx_echo_order(pl, plen, bo);
            if (!strcmp(pr->fill_rule, "partial")) {
                /* 부분체결: Trading_Volumn 절반(exec_tr별 offset) */
                int qoff = !strncmp(pr->exec_tr, "TTRTDP21301", 11) ? 66 :
                           !strncmp(pr->exec_tr, "TTRTDP42301", 11) ? 82 : -1;
                if (qoff >= 0 && plen >= qoff + 10) memcpy(pl + qoff, "0000000050", 10);
            }
            ms = ++g_push_seq;
            push_len = krx_build_data_push(resp, respcap, KRX_DEFAULT_SENDER,
                                           pr->wrapper, pr->exec_tr, ms, ms, pl, plen);
            if (push_len > 0) { mock_sendn(fd, resp, push_len);
                mock_log("VX: push EXEC %s (%d B) seq=%d [%s]%s%s", pr->exec_tr, push_len, ms, pr->name,
                         bo ? " CORRELATED" : "",
                         !strcmp(pr->fill_rule, "partial") ? " PARTIAL(qty=50)" : ""); }
            if (bo) bo->filled = 1;       /* 체결 완료 → book 소진 */
        }
    }
}

/*========================================================================
    VX-1b-b: multi-connection 거래소 (select 루프)
    실 FEP 이중소켓 — 주문 송신소켓(pb_1101_ts)과 체결 수신소켓(pb_1201_tr)을
    동시 처리하고 global order book 을 공유해 주문↔체결 correlation 을 성립.
    (기존 단일 handle_client → vx_serve select 루프로 승격)
========================================================================*/
static int g_push_fd = -1;   /* recv/push 세션 fd (SCHOPQ10000 발신 소켓 = pc_1201_tr) */
static int vx_route_msg(int fd, char *buf, int rt)
{
    char resp[MOCK_BUF_SIZE];
    char msg_type[12];
    int  resp_len;

    get_msg_type(buf, msg_type);
    mock_log("VX: fd=%d recv %dB MsgType=[%s]", fd, rt, msg_type);

    /* Route by MsgType */
        if (strcmp(msg_type, KRX_LOGON_REQ) == 0) {
            /* LOGON request → LOGON response */
            resp_len = krx_build_logon_resp(resp, sizeof(resp), KRX_DEFAULT_SENDER, 1);
            if (resp_len > 0) {
                mock_sendn(fd, resp, resp_len);
                mock_log("MOCK: Sent LOGON_RESP (%d bytes)", resp_len);
            }
        }
        else if (strcmp(msg_type, KRX_LINK_REQ) == 0) {
            /* LINK request → LINK response
               (E2E: pb_1101_ts는 Body 첫 4바이트 ResponseCode="0000"을
                검사하므로 헤더 뒤에 정상코드 Body를 덧붙인다) */
            resp_len = krx_build_link_resp(resp, sizeof(resp), KRX_DEFAULT_SENDER);
            if (resp_len > 0) {
                char bl[16];
                memcpy(resp + resp_len, "0000", 4);
                resp_len += 4;
                sprintf(bl, "%06d", 4);
                memcpy(resp + KRX_OFF_BODYLEN, bl, 6);   /* BodyLength 갱신 */
                /* pb_1101_ts는 FirstSeq(MsgSeqNum) > INT_SEQ 이면 오류 처리
                   - 신규 세션이므로 0으로 명시 */
                sprintf(bl, "%011d", 0);
                memcpy(resp + KRX_OFF_MSGSEQNUM, bl, 11);
                mock_sendn(fd, resp, resp_len);
                mock_log("MOCK: Sent LINK_RESP (%d bytes)", resp_len);
            }
        }
        else if (strcmp(msg_type, "SCHOPQ10000") == 0) {
            /* 수신측(체결/응답 회선) 개시 요청 → SCHOPR10000 응답 후 회원처리호가 push.
               pc_1200_tr LINK 검증: INT_SEQ != FirstSeq 면 Exit. 업무개시(SCHOPQ10000)
               송신으로 INT_SEQ=1 이므로 SCHOPR10000 MsgSeqNum=1 이어야 통과. Body="0000". */
            char bl[16];
            resp_len = krx_build_header(resp, sizeof(resp),
                                        "SCHOPR10000", 4, KRX_DEFAULT_SENDER, 0);
            memcpy(resp + resp_len, "0000", 4);
            resp_len += 4;
            sprintf(bl, "%011d", 0);
            memcpy(resp + KRX_OFF_MSGSEQNUM, bl, 11);
            mock_sendn(fd, resp, resp_len);
            mock_log("MOCK: Sent LINK_RESP(SCHOPR10000) (%d bytes)", resp_len);
            g_push_fd = fd; g_push_seq = 0; /* 이 소켓 = 응답/체결 수신(push) 세션, seq 리셋 */

            /* VX-1/run_pc_rx: SCHOPQ 접속 시 catalog auto-push(하위호환).
               VX-6 주문왕복 테스트는 VX_NO_SCHOPQ_PUSH=1 로 억제(주문 트리거 push만). */
            if (getenv("VX_NO_SCHOPQ_PUSH") == NULL)
                vx_push_catalog(fd, resp, sizeof(resp), -1);
        }
        else if (strcmp(msg_type, KRX_POLL_REQ) == 0) {
            /* POLL request → POLL response */
            resp_len = krx_build_poll_resp(resp, sizeof(resp), KRX_DEFAULT_SENDER);
            if (resp_len > 0) {
                mock_sendn(fd, resp, resp_len);
                mock_log("MOCK: Sent POLL_RESP (%d bytes)", resp_len);
            }
        }
        else if (strcmp(msg_type, KRX_LOGOFF_REQ) == 0) {
            /* LOGOFF — close connection */
            mock_log("VX: fd=%d LOGOFF", fd);
            return (-1);
        }
        else if (memcmp(msg_type, "TCHODR", 6) == 0) {
            /* Order/Data message: 실 KRX는 주문송신 소켓(pb_1101_ts)으로
               응답을 echo하지 않는다 — 응답/체결은 별도 수신 프로세스
               (pb_1201_tr)의 별도 접속으로 온다. echo 금지(I-9).
               VX-1b: 카탈로그로 주문 상품 식별(order-aware). */
            /* 실 KRX 주문: Header MsgType 은 generic 래퍼(TCHODR00000), 구체 TR 은
               Body Transaction_Code(@KRX_HDR_LEN+11, 11B). vx_probe(헤더 TR)·실
               pc_1100_ts(바디 TR) 모두 지원 — 둘 다 order_tr 과 대조. */
            int  ci, matched = -1;
            char body_tr[12];
            memcpy(body_tr, buf + KRX_HDR_LEN + 11, 11); body_tr[11] = '\0';
            for (ci = 0; ci < vx_cat_cnt; ci++) {
                int L; if (!vx_cat[ci].order_tr[0]) continue; L = strlen(vx_cat[ci].order_tr);
                if (!strncmp(msg_type, vx_cat[ci].order_tr, L) ||
                    !strncmp(body_tr, vx_cat[ci].order_tr, L)) { matched = ci; break; }
            }
            if (matched >= 0) {
                int ono = vx_book_order(matched, buf + KRX_HDR_LEN, rt - KRX_HDR_LEN);
                mock_log("VX: ORDER recv hdr=%s body_tr=%s [%s] booked OrderNo=%d",
                         msg_type, body_tr, vx_cat[matched].name, ono);
                /* VX-6 왕복: 주문 수신 즉시 응답/체결을 수신(push)세션으로 교차 전송(booked→correlation) */
                if (g_push_fd >= 0 && g_push_fd != fd) {
                    vx_push_catalog(g_push_fd, resp, sizeof(resp), matched);
                    mock_log("VX: 주문→체결 왕복: [%s] 응답/체결을 수신소켓(fd=%d)으로 push", vx_cat[matched].name, g_push_fd);
                }
            }
            else
                mock_log("VX: ORDER recv %s (미등록 상품 → cfg/vexch.ini 에 블록 추가 필요)",
                         msg_type);
        }
        else {
            mock_log("MOCK: Unknown MsgType [%s], ignored (no echo)", msg_type);
        }
    return (0);   /* 계속 서비스 */
}

/*------------------------------------------------------------------------
    vx_serve — multi-connection select 루프 (listen + N clients, 공유 book)
------------------------------------------------------------------------*/
static int vx_serve(int *lfds, int nl)
{
    char    buf[MOCK_BUF_SIZE];
    int     clients[FD_SETSIZE], nclients = 0;
    int     i, maxfd, cfd, rt;
    fd_set  rset;

    mock_log("VX: multi-connection exchange up (listen ports=%d)", nl);
    while (1) {
        FD_ZERO(&rset); maxfd = -1;
        for (i = 0; i < nl; i++) { FD_SET(lfds[i], &rset); if (lfds[i] > maxfd) maxfd = lfds[i]; }
        for (i = 0; i < nclients; i++) { FD_SET(clients[i], &rset); if (clients[i] > maxfd) maxfd = clients[i]; }
        if (select(maxfd + 1, &rset, NULL, NULL, NULL) < 0) { if (errno == EINTR) continue; break; }
        for (i = 0; i < nl; i++) if (FD_ISSET(lfds[i], &rset)) {
            cfd = accept(lfds[i], NULL, NULL);
            if (cfd >= 0 && nclients < FD_SETSIZE) { clients[nclients++] = cfd;
                mock_log("VX: client connected fd=%d (active=%d)", cfd, nclients); }
        }
        for (i = 0; i < nclients; ) {
            int fd = clients[i];
            if (!FD_ISSET(fd, &rset)) { i++; continue; }
            rt = mock_recv_krx_msg(fd, buf, sizeof(buf));
            if (rt <= 0 || vx_route_msg(fd, buf, rt) < 0) {
                mock_log("VX: client fd=%d closed", fd);
                close(fd); clients[i] = clients[--nclients]; continue;
            }
            i++;
        }
    }
    return (MOCK_OK);
}

/*------------------------------------------------------------------------
    Main
------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int port = MOCK_DEFAULT_PORT;
    struct sockaddr_in  svr_addr, cli_addr;
    socklen_t           cli_len;
    int                 opt = 1;
    char                log_path[256];

    if (argc > 1)
        port = atoi(argv[1]);

    /* Open log file */
    sprintf(log_path, "/tmp/mock_krx_%d.log", port);
    g_logfp = fopen(log_path, "w");

    /* Signal handlers */
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);

    /* VX-1: 상품 카탈로그 로드 (env VX_CATALOG, 기본 cfg/vexch.ini) */
    {
        const char *cat = getenv("VX_CATALOG");
        if (vx_load_catalog(cat ? cat : "../../cfg/vexch.ini", vx_log) <= 0)
            mock_log("VX: catalog empty → fallback DERIV hardcode 불가, push 없음");
    }

    /* Create listen socket(s) — 다중 포트(주문 37221 + 수신 57221 등)를 단일
       프로세스가 listen → 이중소켓 왕복(g_push_fd/book 공유)이 한 mock 에서 성립.
       argv[1..] = 포트들. VX-6b: `mock_krx_server 37221 57221`. */
    {
        int  pi, np = 0, nl = 0, lfds[8], ports[8];
        if (argc > 1) { for (pi = 1; pi < argc && np < 8; pi++) ports[np++] = atoi(argv[pi]); }
        else ports[np++] = MOCK_DEFAULT_PORT;
        for (pi = 0; pi < np; pi++) {
            int lf = socket(AF_INET, SOCK_STREAM, 0);
            if (lf < 0) { perror("socket"); return 1; }
            setsockopt(lf, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#ifdef SO_REUSEPORT
            setsockopt(lf, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif
            memset(&svr_addr, 0, sizeof(svr_addr));
            svr_addr.sin_family      = AF_INET;
            svr_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            svr_addr.sin_port        = htons(ports[pi]);
            if (bind(lf, (struct sockaddr *)&svr_addr, sizeof(svr_addr)) < 0) { perror("bind"); close(lf); return 1; }
            if (listen(lf, 16) < 0) { perror("listen"); close(lf); return 1; }
            lfds[nl++] = lf;
            mock_log("MOCK: Listening on 127.0.0.1:%d (log=%s)", ports[pi], log_path);
        }
        g_listen_fd = lfds[0];
        fflush(stdout);
        (void)cli_addr; (void)cli_len;
        vx_serve(lfds, nl);
        for (pi = 0; pi < nl; pi++) close(lfds[pi]);
    }
    /* (unreached below kept for structure) */
    if (0) close(g_listen_fd);
    if (g_logfp) fclose(g_logfp);

    mock_log("MOCK: Server exiting");
    return 0;
}
