/*------------------------------------------------------------------------
#   fx_sise_inject.c — pf_7400_ur(FX 시세수신) 테스트용 UDP CO_B6FX 주입기
#
#   dev(NO_AGXPI)에서 pf_7400_ur 의 Conv_Dispatch 가 raw CO_B6FX passthrough 이므로
#   이 도구가 CO_B6FX 구조체 그대로를 UDP 로 보내면 pf_ 가 Set_Sise → FX_Sise/Shm_FX
#   에 기록한다. (운영은 agxpi FIX 디코드; 여기선 시세 wire 계약을 CO_B6FX raw 로 대체)
#
#   usage: fx_sise_inject [count=3] [ip=127.0.0.1] [port=18400]
------------------------------------------------------------------------*/
#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>
#include    <unistd.h>
#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <arpa/inet.h>
#include    "fx.h"

int     main(int argc, char *argv[])
{
    int                 fd, n;
    int                 cnt  = (argc > 1) ? atoi(argv[1]) : 3;
    const char         *ip   = (argc > 2) ? argv[2] : "127.0.0.1";
    int                 port = (argc > 3) ? atoi(argv[3]) : 18400;
    struct sockaddr_in  addr;
    struct { char sym[8]; double bid, off; } tab[] = {
        { "EURUSD", 1.085000, 1.085100 },
        { "USDJPY", 149.200000, 149.220000 },
        { "GBPUSD", 1.271000, 1.271200 },
    };

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) { perror("socket"); return 1; }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    for (n = 0; n < cnt; n++) {
        CO_B6FX b;
        int     t = n % 3;

        memset(&b, 0, sizeof(b));
        b.excode[0] = 'J';                          /* JPM */
        memcpy(b.symb, tab[t].sym, 7);
        b.bidprc   = tab[t].bid;
        b.offerprc = tab[t].off;
        memcpy(b.bid_quote_id, "BIDQ00000001", 12);
        memcpy(b.ask_quote_id, "ASKQ00000001", 12);

        if (sendto(fd, &b, sizeof(b), 0,
                (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("sendto");
            close(fd);
            return 1;
        }
        printf("[fx_inject] sent #%d sym[%.7s] bid[%f] off[%f] len[%d]\n",
                n + 1, b.symb, b.bidprc, b.offerprc, (int)sizeof(b));
        usleep(150000);
    }
    close(fd);
    return 0;
}
