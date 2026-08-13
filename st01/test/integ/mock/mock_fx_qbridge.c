/*------------------------------------------------------------------------
#   mock_fx_qbridge.c — 가상 win FEP 브리지(SysV 메시지큐)  [VX-4f]
#
#   win 의 mon/fep 모듈을 대역: FEP(pf_1100_ts msgq 모드)가 주문큐(ord_queue)에
#   msgsnd 한 SMB_ST 신규('D')/취소('F')를 msgrcv 하고, 회원사 OrdID/ExecID 를
#   채운 체결통지('8')를 체결큐(exe_queue)에 거래원 mtype 으로 msgsnd 한다.
#   실 연동과 동일한 전송수단(SysV msgq)·큐키·SMB_ST 포맷을 사용 —
#   라이브 win 은 건드리지 않는 안전 검증용.
#
#   빌드: cc -I$ST01/inc -o mock_fx_qbridge mock_fx_qbridge.c
#   설정(env): VX_FX_QBRIDGE="excode:ord_key:exe_key:exe_mtype:fill_rule[,...]"
#     예) "S:0x90001110:0x90002110:100:full,E:0x90001330:0x90002110:300:ack"
------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "fx.h"

#define SMB_SZ  ((int)sizeof(SMB_ST))
#define MAXVN   8

typedef struct { long mtype; SMB_ST body; } QMSG;

typedef struct {
    char excode;
    long ord_key, exe_key, exe_mtype;
    char fill[12];
    int  ord_qid, exe_qid;
} VN;

static VN   vn[MAXVN];
static int  vncnt = 0;
static int  g_ordno = 700001, g_execid = 900001;
static FILE *g_lf = NULL;

static void qlog(const char *fmt, ...)
{
    char b[512]; va_list ap; va_start(ap, fmt);
    vsnprintf(b, sizeof(b), fmt, ap); va_end(ap);
    if (g_lf) { fprintf(g_lf, "%s\n", b); fflush(g_lf); }
    printf("%s\n", b); fflush(stdout);
}
static void field(char *dst, const char *src, int n)
{ int i; memcpy(dst, src, n); dst[n]=0;
  for (i=n-1; i>=0 && (dst[i]==' '||dst[i]==0); i--) dst[i]=0; }
static void setf(char *dst, int n, const char *src)
{ memset(dst, 0, n); strncpy(dst, src, n); }

/* 체결통지(SMB_ST '8')를 체결큐에 msgsnd — 주문 echo 로 ClOrdID/OrigClOrdID 상관 */
static void send_exec(VN *v, const SMB_ST *ord, char exectype, char ordstatus,
                      const char *cum, const char *last, const char *leaves, const char *lpx)
{
    QMSG m; char num[40];
    m.mtype = v->exe_mtype;
    memcpy(&m.body, ord, SMB_SZ);
    m.body.smb_MsgType[0]  = '8';
    m.body.smb_ExecType[0] = exectype;
    m.body.smb_OrdStatus[0]= ordstatus;
    snprintf(num, sizeof(num), "%d", g_ordno++);  setf(m.body.smb_OrdID,  30, num);
    snprintf(num, sizeof(num), "%d", g_execid++); setf(m.body.smb_ExecID, 30, num);
    if (cum)    setf(m.body.smb_CumQty,    30, cum);
    if (last)   setf(m.body.smb_LastQty,   30, last);
    if (leaves) setf(m.body.smb_LeavesQty, 30, leaves);
    if (lpx)    setf(m.body.smb_LastPx,    30, lpx);
    if (msgsnd(v->exe_qid, &m, SMB_SZ, 0) < 0)
        qlog("QB: msgsnd exec fail venue=%c {%d:%s}", v->excode, errno, strerror(errno));
}

static void handle(VN *v, const SMB_ST *o)
{
    char cl[25], oc[25], qty[31], px[31];
    field(cl, o->smb_ClOrdID, 24);
    field(oc, o->smb_OrigClOrdID, 24);
    field(qty, o->smb_OrderQty, 30);
    field(px,  o->smb_Price, 30);
    qlog("QB: REQ recv venue=%c MsgType=%c ClOrdID=%s OrderQty=%s Price=%s fill=%s",
         v->excode, o->smb_MsgType[0], cl, qty, px, v->fill);

    if (o->smb_MsgType[0] == 'F') {                     /* 취소 */
        send_exec(v, o, '4', '4', "0", "0", "0", NULL);
        qlog("QB: CANCELED venue=%c ClOrdID=%s OrigClOrdID=%s", v->excode, cl, oc);
        return;
    }
    if (o->smb_MsgType[0] != 'D') {
        qlog("QB: 미지원 MsgType=%c → 무시", o->smb_MsgType[0]); return;
    }
    if (!strcmp(v->fill, "reject")) {
        send_exec(v, o, '8', '8', "0", "0", qty, NULL);
        qlog("QB: REJECTED venue=%c ClOrdID=%s", v->excode, cl); return;
    }
    send_exec(v, o, '0', '0', "0", "0", qty, NULL);     /* New ack */
    qlog("QB: ACK(New) venue=%c ClOrdID=%s", v->excode, cl);
    if (!strcmp(v->fill, "ack")) return;
    if (!strcmp(v->fill, "partial")) {
        long q = atol(qty); char half[31];
        snprintf(half, sizeof(half), "%ld", q/2);
        send_exec(v, o, '1', '1', half, half, half, px);
        qlog("QB: PARTIAL venue=%c %s/%s ClOrdID=%s", v->excode, half, qty, cl); return;
    }
    send_exec(v, o, 'F', '2', qty, qty, "0", px);       /* full */
    qlog("QB: FILLED venue=%c %s @ %s ClOrdID=%s", v->excode, qty, px, cl);
}

int main(void)
{
    const char *cfg = getenv("VX_FX_QBRIDGE");
    char tmp[512], *save1, *tok;
    int i;

    g_lf = fopen("/tmp/mock_fxq.log", "w");
    if (!cfg || !*cfg) { qlog("QB: VX_FX_QBRIDGE 없음"); return (1); }
    strncpy(tmp, cfg, sizeof(tmp)-1); tmp[sizeof(tmp)-1]=0;
    for (tok = strtok_r(tmp, ",", &save1); tok && vncnt < MAXVN; tok = strtok_r(NULL, ",", &save1)) {
        char *s2, *e = strtok_r(tok, ":", &s2), *ok = strtok_r(NULL,":",&s2),
             *xk = strtok_r(NULL,":",&s2), *mt = strtok_r(NULL,":",&s2), *fr = strtok_r(NULL,":",&s2);
        if (e && ok && xk && mt && fr) {
            vn[vncnt].excode = e[0];
            vn[vncnt].ord_key = strtol(ok, NULL, 16);
            vn[vncnt].exe_key = strtol(xk, NULL, 16);
            vn[vncnt].exe_mtype = atol(mt);
            setf(vn[vncnt].fill, 12, fr); vn[vncnt].fill[11]=0;
            vncnt++;
        }
    }
    for (i = 0; i < vncnt; i++) {
        vn[i].ord_qid = msgget((key_t)vn[i].ord_key, 0666 | IPC_CREAT);
        vn[i].exe_qid = msgget((key_t)vn[i].exe_key, 0666 | IPC_CREAT);
        if (vn[i].ord_qid < 0 || vn[i].exe_qid < 0) {
            qlog("QB: msgget fail venue=%c {%d:%s}", vn[i].excode, errno, strerror(errno)); return (1); }
        qlog("QB: venue=%c ord_qid=%d(0x%lx) exe_qid=%d(0x%lx) mtype=%ld fill=%s",
             vn[i].excode, vn[i].ord_qid, vn[i].ord_key, vn[i].exe_qid, vn[i].exe_key, vn[i].exe_mtype, vn[i].fill);
    }
    qlog("QB: FX q-bridge ready venues=%d SMB_ST=%d", vncnt, SMB_SZ);

    for (;;) {                                          /* 주문큐 폴링 */
        QMSG m;
        int got = 0;
        for (i = 0; i < vncnt; i++)
            while (msgrcv(vn[i].ord_qid, &m, SMB_SZ, 0, IPC_NOWAIT) > 0)
            { handle(&vn[i], &m.body); got = 1; }
        if (!got) usleep(50000);                        /* 50ms */
    }
    return (0);
}
