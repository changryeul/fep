/*------------------------------------------------------------------------
#   vx_fx_probe.c — 가상 FX 거래원 프로브(스모크 클라이언트)  [VX-4a]
#
#   mock_fx_venue 에 SMB_ST 신규주문(MsgType 'D')을 보내고, 돌아오는
#   체결통지(MsgType '8')를 드레인해 ClOrdID echo(상관)·ExecType 을 검증.
#
#   빌드: cc -I$ST01/inc -I$VEXCH -o vx_fx_probe vx_fx_probe.c
#   실행: ./vx_fx_probe [port(기본 19100)] [ClOrdID(기본 FXPROBE00000001)]
#   출력(마지막): "VXFX RESULT exec=<N> correlated=<0/1>"
#------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "fx.h"                 /* SMB_ST */

#define SMB_SZ  ((int)sizeof(SMB_ST))

static void setf(char *dst, int n, const char *src)
{ memset(dst, 0, n); strncpy(dst, src, n); }
static void field(char *dst, const char *src, int n)
{ int i; memcpy(dst, src, n); dst[n] = 0;
  for (i = n - 1; i >= 0 && (dst[i] == ' ' || dst[i] == 0); i--) dst[i] = 0; }

int main(int argc, char **argv)
{
    int port = (argc > 1) ? atoi(argv[1]) : 19100;
    const char *clord = (argc > 2) ? argv[2] : "FXPROBE00000001";
    int fd, exec_cnt = 0, correlated = 0;
    SMB_ST o;
    struct sockaddr_in sa;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");
    sa.sin_port = htons((unsigned short)port);
    if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        printf("VXFX probe: connect 실패 port=%d\n", port);
        printf("VXFX RESULT exec=0 correlated=0\n"); return (1);
    }

    /* SMB_ST 신규주문 구성 (MsgType 'D') */
    memset(&o, 0, SMB_SZ);
    o.smb_MsgType[0] = 'D';                       /* 신규 */
    setf(o.smb_SenderCompID, 50, "FEPTEST");
    setf(o.smb_TargetCompID, 50, "SMB");
    setf(o.smb_ClOrdID, 24, clord);               /* 주문번호(상관 키) */
    setf(o.smb_Account,  30, "FXACCT0001");
    setf(o.smb_Currency,  3, "USD");
    setf(o.smb_Symbol,    7, "USD/KRW");
    o.smb_Side[0]    = '1';                        /* BUY */
    o.smb_OrdType[0] = '2';                        /* 지정가 */
    setf(o.smb_OrderQty, 30, "1000000");
    setf(o.smb_Price,    30, "1385.50");
    o.smb_TimeInForce[0] = '3';                    /* IOC */
    setf(o.smb_TransactTime, 30, "20260811-05:30:00.000");

    if (write(fd, &o, SMB_SZ) != SMB_SZ) {
        printf("VXFX probe: write 실패\n");
        printf("VXFX RESULT exec=0 correlated=0\n"); return (1);
    }
    printf("VXFX probe: ORDER sent ClOrdID=%s Side=1 Qty=1000000 Px=1385.50\n", clord);

    /* 체결통지 드레인 (최대 2초) */
    {
        char buf[8192]; int blen = 0;
        for (;;) {
            fd_set rf; struct timeval tv; int rt;
            FD_ZERO(&rf); FD_SET(fd, &rf); tv.tv_sec = 2; tv.tv_usec = 0;
            rt = select(fd + 1, &rf, NULL, NULL, &tv);
            if (rt <= 0) break;                    /* 타임아웃/에러 → 종료 */
            rt = recv(fd, buf + blen, sizeof(buf) - blen, 0);
            if (rt <= 0) break;
            blen += rt;
            while (blen >= SMB_SZ) {
                SMB_ST *e = (SMB_ST *)buf;
                char cl[25], oid[31], eid[31], cum[31], lpx[31];
                field(cl,  e->smb_ClOrdID, 24);
                field(oid, e->smb_OrdID,   30);
                field(eid, e->smb_ExecID,  30);
                field(cum, e->smb_CumQty,  30);
                field(lpx, e->smb_LastPx,  30);
                exec_cnt++;
                if (!strcmp(cl, clord)) correlated = 1;   /* ClOrdID echo 확인 */
                printf("VXFX recv[%d] MsgType=%c ExecType=%c OrdStatus=%c ClOrdID=%s OrdID=%s ExecID=%s CumQty=%s LastPx=%s\n",
                       exec_cnt, e->smb_MsgType[0], e->smb_ExecType[0], e->smb_OrdStatus[0],
                       cl, oid, eid, cum, lpx);
                memmove(buf, buf + SMB_SZ, blen - SMB_SZ);
                blen -= SMB_SZ;
            }
        }
    }
    close(fd);
    printf("VXFX RESULT exec=%d correlated=%d\n", exec_cnt, correlated);
    return (0);
}
