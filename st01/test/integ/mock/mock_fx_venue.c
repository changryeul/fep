/*------------------------------------------------------------------------
#   mock_fx_venue.c — 가상 FX 거래원(Virtual FX Venue) 엔진  [VX-4a]
#
#   SMB_ST(fx.h, FIX-flat 고정 1024B)를 TCP 로 주고받는 가상 거래원.
#   FEP(또는 프로브)가 보낸 신규주문(MsgType 'D')을 받아 회원사가 부여하는
#   OrdID/ExecID 를 채워 체결통지(MsgType '8')를 돌려준다. ClOrdID 를 echo 해
#   주문↔체결을 상관(correlation)시킨다. 상품/거래원/체결규칙은 cfg/vexch.ini
#   (order_proto=fx_smb, fx_excode, fx_port, fill_rule)에서 읽는다.
#
#   빌드: cc -I$ST01/inc -I$VEXCH -o mock_fx_venue mock_fx_venue.c \
#           ../../vexch/vexch_catalog.c
#   실행: VX_CATALOG=cfg/vexch.ini ./mock_fx_venue [port(기본 19100)]
#------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "fx.h"                 /* SMB_ST */
#include "vexch_catalog.h"

#define SMB_SZ  ((int)sizeof(SMB_ST))

static FILE *g_logfp = NULL;
static int   g_next_ordno  = 700001;   /* 거래원 부여 주문번호(OrdID)   */
static int   g_next_execid = 900001;   /* 체결ID(ExecID)                */

static void fxlog(const char *m)
{
    if (g_logfp) { fprintf(g_logfp, "%s\n", m); fflush(g_logfp); }
    printf("%s\n", m); fflush(stdout);
}
static void fxlogf(const char *fmt, ...)
{
    char b[512]; va_list ap; va_start(ap, fmt);
    vsnprintf(b, sizeof(b), fmt, ap); va_end(ap); fxlog(b);
}

/* SMB_ST char 필드(널 미종료)를 로그용 널종료 문자열로 복사(뒤쪽 공백/0 제거) */
static void field(char *dst, const char *src, int n)
{
    int i;
    memcpy(dst, src, n); dst[n] = 0;
    for (i = n - 1; i >= 0 && (dst[i] == ' ' || dst[i] == 0); i--) dst[i] = 0;
}

/* 고정폭 필드에 값 세팅(0 패딩) */
static void setf(char *dst, int n, const char *src)
{ memset(dst, 0, n); strncpy(dst, src, n); }

/* 체결통지(SMB_ST, MsgType '8') 전송 — 주문을 echo 해 ClOrdID/Symbol/Side 상관 */
static void send_exec(int fd, const SMB_ST *ord, char exectype, char ordstatus,
                      const char *cumqty, const char *lastqty,
                      const char *leavesqty, const char *lastpx)
{
    SMB_ST e; char num[40];
    memcpy(&e, ord, SMB_SZ);              /* echo: ClOrdID/Symbol/Side/OrderQty/Price 유지 */
    e.smb_MsgType[0]  = '8';              /* 주문확인 및 체결 */
    e.smb_ExecType[0] = exectype;
    e.smb_OrdStatus[0]= ordstatus;
    snprintf(num, sizeof(num), "%d", g_next_ordno++);  setf(e.smb_OrdID,  30, num);
    snprintf(num, sizeof(num), "%d", g_next_execid++); setf(e.smb_ExecID, 30, num);
    if (cumqty)    setf(e.smb_CumQty,    30, cumqty);
    if (lastqty)   setf(e.smb_LastQty,   30, lastqty);
    if (leavesqty) setf(e.smb_LeavesQty, 30, leavesqty);
    if (lastpx)    setf(e.smb_LastPx,    30, lastpx);
    if (write(fd, &e, SMB_SZ) != SMB_SZ)
        fxlogf("FX: send_exec write 실패 errno=%d", errno);
}

/* 신규주문 처리: New ack → fill_rule 에 따라 체결/부분/거부 */
static void handle_order(int fd, const SMB_ST *o, const VX_PRODUCT *pr)
{
    char cl[25], qty[31], px[31], sym[8];
    const char *fr = pr ? pr->fill_rule : "full";
    field(cl,  o->smb_ClOrdID, 24);
    field(qty, o->smb_OrderQty, 30);
    field(px,  o->smb_Price,   30);
    field(sym, o->smb_Symbol,   7);

    fxlogf("FX: REQ recv MsgType=%c ClOrdID=%s Symbol=%s Side=%c OrderQty=%s Price=%s excode=%c fill=%s",
           o->smb_MsgType[0], cl, sym, o->smb_Side[0], qty, px,
           (pr && pr->fx_excode[0]) ? pr->fx_excode[0] : '?', fr);

    if (o->smb_MsgType[0] == 'F') {                         /* 취소 요청 */
        char oc[25]; field(oc, o->smb_OrigClOrdID, 24);
        send_exec(fd, o, '4', '4', "0", "0", "0", NULL);    /* ExecType/OrdStatus 4=Canceled */
        fxlogf("FX: CANCELED ClOrdID=%s OrigClOrdID=%s", cl, oc);
        return;
    }
    if (o->smb_MsgType[0] != 'D') {
        fxlogf("FX: 미지원 MsgType=%c (D 신규 / F 취소만 처리) → 무시", o->smb_MsgType[0]);
        return;
    }

    if (!strcmp(fr, "reject")) {
        SMB_ST e; memcpy(&e, o, SMB_SZ);
        setf(e.smb_Text, 140, "VX reject (fill_rule=reject)");
        send_exec(fd, &e, '8', '8', "0", "0", qty, NULL);   /* ExecType/OrdStatus 8=Rejected */
        fxlogf("FX: REJECTED ClOrdID=%s", cl);
        return;
    }

    /* 공통: New ack (ExecType '0', OrdStatus '0') */
    send_exec(fd, o, '0', '0', "0", "0", qty, NULL);
    fxlogf("FX: ACK(New) ClOrdID=%s", cl);

    if (!strcmp(fr, "ack"))
        return;                                             /* 응답만 */

    if (!strcmp(fr, "partial")) {
        long q = atol(qty); char half[31];
        snprintf(half, sizeof(half), "%ld", q / 2);
        send_exec(fd, o, '1', '1', half, half, half, px);   /* Partially filled */
        fxlogf("FX: PARTIAL fill %s/%s ClOrdID=%s", half, qty, cl);
        return;
    }

    /* full (기본): 전량 체결 (ExecType 'F', OrdStatus '2') */
    send_exec(fd, o, 'F', '2', qty, qty, "0", px);
    fxlogf("FX: FILLED %s @ %s ClOrdID=%s", qty, px, cl);
}

int main(int argc, char **argv)
{
    int port = (argc > 1) ? atoi(argv[1]) : 19100;
    const char *cat = getenv("VX_CATALOG");
    char logpath[128];
    int  listen_fd, i, on = 1;
    struct sockaddr_in sa;
    VX_PRODUCT *pr = NULL;
    int  clients[FD_SETSIZE];
    static char cbuf[FD_SETSIZE][8192];
    int  clen[FD_SETSIZE];

    if (!cat) cat = "../../cfg/vexch.ini";
    snprintf(logpath, sizeof(logpath), "/tmp/mock_fx_%d.log", port);
    g_logfp = fopen(logpath, "w");

    vx_load_catalog(cat, fxlog);
    /* 이 포트에 해당하는 fx_smb 상품 선택(정확 포트 우선, 없으면 첫 fx_smb) */
    for (i = 0; i < vx_cat_cnt; i++) {
        if (!vx_cat[i].enabled || strcmp(vx_cat[i].order_proto, "fx_smb")) continue;
        if (vx_cat[i].fx_port == port) { pr = &vx_cat[i]; break; }
        if (!pr) pr = &vx_cat[i];
    }
    if (!pr) fxlogf("FX: cfg 에 order_proto=fx_smb 상품 없음 → fill_rule=full 기본 사용");
    else     fxlogf("FX: product=%s excode=%c fill=%s port=%d",
                    pr->name, pr->fx_excode[0] ? pr->fx_excode[0] : '?', pr->fill_rule, pr->fx_port);

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET; sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons((unsigned short)port);
    if (bind(listen_fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        fxlogf("FX: bind 실패 port=%d errno=%d", port, errno); return (1); }
    listen(listen_fd, 16);
    fxlogf("FX venue engine LISTEN port=%d proto=fx_smb SMB_ST=%d bytes", port, SMB_SZ);

    for (i = 0; i < FD_SETSIZE; i++) { clients[i] = -1; clen[i] = 0; }

    for (;;) {
        fd_set rf; int maxfd = listen_fd, rt;
        FD_ZERO(&rf); FD_SET(listen_fd, &rf);
        for (i = 0; i < FD_SETSIZE; i++)
            if (clients[i] >= 0) { FD_SET(clients[i], &rf); if (clients[i] > maxfd) maxfd = clients[i]; }
        if (select(maxfd + 1, &rf, NULL, NULL, NULL) < 0) { if (errno == EINTR) continue; break; }

        if (FD_ISSET(listen_fd, &rf)) {
            int nf = accept(listen_fd, NULL, NULL);
            if (nf >= 0 && nf < FD_SETSIZE) {
                for (i = 0; i < FD_SETSIZE && clients[i] >= 0; i++) ;
                if (i < FD_SETSIZE) { clients[i] = nf; clen[i] = 0; fxlogf("FX: client 접속 fd=%d", nf); }
                else close(nf);
            } else if (nf >= 0) close(nf);
        }
        for (i = 0; i < FD_SETSIZE; i++) {
            if (clients[i] < 0 || !FD_ISSET(clients[i], &rf)) continue;
            rt = recv(clients[i], cbuf[i] + clen[i], sizeof(cbuf[i]) - clen[i], 0);
            if (rt <= 0) { fxlogf("FX: client 종료 fd=%d", clients[i]); close(clients[i]); clients[i] = -1; clen[i] = 0; continue; }
            clen[i] += rt;
            while (clen[i] >= SMB_SZ) {                       /* 고정 프레임 단위 처리 */
                handle_order(clients[i], (SMB_ST *)cbuf[i], pr);
                memmove(cbuf[i], cbuf[i] + SMB_SZ, clen[i] - SMB_SZ);
                clen[i] -= SMB_SZ;
            }
        }
    }
    return (0);
}
