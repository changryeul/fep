/*------------------------------------------------------------------------
#   vx_sise_pub.c — 가상거래소 시세 발행기 (VX-2, 공유 카탈로그 구동)
#
#   cfg/vexch.ini(공유 vexch_catalog) 의 sise_kind!=none 상품에 시세 UDP 발행.
#     fx  : CO_B6FX (excode/symb/bidprc/offerprc 틱) → sise_ip:sise_port
#           (pf_7400_ur Conv_Dispatch(NO_AGXPI passthrough) → Set_Sise → FX_Sise)
#     krx : TrCode prefix(A301F 등) + body(sise_size, template) → UDP  (VX-2b)
#
#   usage: vx_sise_pub [ticks=3]     (env VX_CATALOG=cfg/vexch.ini)
------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "fx.h"                             /* CO_B6FX */
#include "../../vexch/vexch_catalog.h"      /* 공유 카탈로그 */

static int is_multicast(const char *ip)   /* 224.0.0.0 ~ 239.255.255.255 */
{
    int o = atoi(ip);
    return (o >= 224 && o <= 239);
}
static int udp_open(const char *ip, int port, struct sockaddr_in *a)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(a, 0, sizeof(*a));
    a->sin_family = AF_INET; a->sin_port = htons(port);
    inet_pton(AF_INET, ip, &a->sin_addr);
    if (fd >= 0 && is_multicast(ip)) {           /* 멀티캐스트 송신: TTL + loopback(단일호스트 수신) */
        unsigned char ttl = 2, loop = 1;
        setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL,  &ttl,  sizeof(ttl));
        setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    }
    return fd;
}

int main(int argc, char *argv[])
{
    int ticks = (argc > 1) ? atoi(argv[1]) : 3;
    const char *cat = getenv("VX_CATALOG"); int i, k;

    vx_load_catalog(cat ? cat : "../../cfg/vexch.ini", NULL);

    for (i = 0; i < vx_cat_cnt; i++) {
        VX_PRODUCT *p = &vx_cat[i];
        struct sockaddr_in a; int fd;
        if (!p->enabled || !strcmp(p->sise_kind, "none") || p->sise_kind[0] == 0) continue;
        fd = udp_open(p->sise_ip, p->sise_port, &a);
        if (fd < 0) { printf("  [%s] socket fail\n", p->name); continue; }

        if (!strcmp(p->sise_kind, "fx")) {
            for (k = 0; k < ticks; k++) {
                CO_B6FX b; memset(&b, 0, sizeof(b));
                b.excode[0] = p->sise_excode[0] ? p->sise_excode[0] : 'J';
                memcpy(b.symb, p->sise_symbol, strnlen(p->sise_symbol, 7));
                b.bidprc   = 1300.0 + k * 0.5;
                b.offerprc = 1300.1 + k * 0.5;
                memcpy(b.bid_quote_id, "VXBID0000001", 12);
                memcpy(b.ask_quote_id, "VXASK0000001", 12);
                sendto(fd, &b, sizeof(b), 0, (struct sockaddr *)&a, sizeof(a));
                printf("  [%s] fx CO_B6FX #%d %.7s bid=%.2f off=%.2f -> %s:%d (%dB)\n",
                       p->name, k+1, b.symb, b.bidprc, b.offerprc, p->sise_ip, p->sise_port, (int)sizeof(b));
                usleep(150000);
            }
        } else if (!strcmp(p->sise_kind, "krx")) {
            char pkt[2048]; int sz = (p->sise_size > 0 && p->sise_size < 2048) ? p->sise_size : 173;
            for (k = 0; k < ticks; k++) {
                char pr[10], vol[10];
                memset(pkt, ' ', sz);
                memcpy(pkt, p->sise_tr, strnlen(p->sise_tr, 5));   /* tr_gbn @0 (A301F) */
                /* CO_A301F 핵심필드: item_code@17(12), crprc@47(9), volume@56(9) */
                if (!strncmp(p->sise_tr, "A301F", 5)) {
                    memcpy(pkt + 17, "KR4101SC0009", 12);
                    snprintf(pr,  sizeof(pr),  "%09d", 30000 + k * 25);   /* 체결가 */
                    snprintf(vol, sizeof(vol), "%09d", 10 + k);           /* 거래량 */
                    memcpy(pkt + 47, pr, 9);
                    memcpy(pkt + 56, vol, 9);
                }
                sendto(fd, pkt, sz, 0, (struct sockaddr *)&a, sizeof(a));
                printf("  [%s] krx %.5s #%d item=KR4101SC0009 crprc=%d vol=%d -> %s:%d (%dB)\n",
                       p->name, p->sise_tr, k+1, 30000 + k*25, 10 + k, p->sise_ip, p->sise_port, sz);
                usleep(150000);
            }
        }
        close(fd);
    }
    return 0;
}
