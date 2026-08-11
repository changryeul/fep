/*------------------------------------------------------------------------
#   vx_sise_probe.c — 가상거래소 KRX 시세 UDP 수신 프로브 (VX-2b)
#   지정 UDP 포트를 bind 해 수신한 KRX 시세 전문의 TrCode(@0,5) / item_code(@17,12)
#   / crprc(@47,9) 를 출력한다. (pc_7100_ur 실 체인 전 self-verify)
#   usage: vx_sise_probe [port=17001] [count=3]
------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    int  port  = (argc > 1) ? atoi(argv[1]) : 17001;
    int  want  = (argc > 2) ? atoi(argv[2]) : 3;
    const char *grp = (argc > 3) ? argv[3] : NULL;   /* 멀티캐스트 그룹(선택) */
    int  fd, n, got = 0;
    struct sockaddr_in a;
    char buf[2048];

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    { int opt = 1; setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); }
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET; a.sin_port = htons(port);
    a.sin_addr.s_addr = grp ? htonl(INADDR_ANY) : htonl(INADDR_LOOPBACK);
    if (bind(fd, (struct sockaddr *)&a, sizeof(a)) < 0) { perror("bind"); return 1; }
    if (grp) {                                        /* 멀티캐스트 그룹 join */
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(grp);
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
            perror("IP_ADD_MEMBERSHIP");
        printf("vx_sise_probe: listening udp *:%d joined mcast %s\n", port, grp);
    } else
        printf("vx_sise_probe: listening udp 127.0.0.1:%d\n", port);

    while (got < want) {
        n = recv(fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        got++;
        { char tr[6], item[13], crprc[10];
          memcpy(tr, buf, 5); tr[5] = 0;
          memcpy(item, buf + 17, 12); item[12] = 0;
          memcpy(crprc, buf + 47, 9); crprc[9] = 0;
          printf("  recv #%d %dB TrCode=[%s] item=[%s] crprc=[%s]\n", got, n, tr, item, crprc); }
    }
    close(fd);
    printf("vx_sise_probe: received %d packets\n", got);
    return (got >= want) ? 0 : 2;
}
