/*------------------------------------------------------------------------
#   vx_probe.c — 가상거래소(vexch) 수신측 스모크 클라이언트
#   SCHOPQ10000(수신개시)를 보내 카탈로그 구동 push(응답/체결)를 수신,
#   포함된 TrCode 를 스캔 출력한다. (krx_protocol.c 재사용)
#   usage: vx_probe [port=19998]
------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../lib/krx_protocol.h"

int main(int argc, char *argv[])
{
    int  fd, n, port = (argc > 1) ? atoi(argv[1]) : 19998;
    /* mode: (기본)both=한 소켓서 주문+링크 / send=주문만 / recv=링크+수신 (이중소켓 검증) */
    const char *mode = (argc > 2) ? argv[2] : "both";
    int  do_send = strcmp(mode, "recv") != 0;   /* both/send → 주문송신 */
    int  do_recv = strcmp(mode, "send") != 0;   /* both/recv → 링크+수신 */
    struct sockaddr_in a;
    char buf[512], r[16384];
    const char *trs[] = { "TTRODP11301", "TTRTDP21301", "TTRODP41301", "TTRTDP42301" };
    int  i, total = 0;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET; a.sin_port = htons(port);
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(fd, (struct sockaddr *)&a, sizeof(a)) < 0) { perror("connect"); return 1; }

    if (do_send) {
        /* VX-1b-full: 주문 1건 전송(회원사용영역 @202 마커) → 상관(echo) 검증 */
        n = krx_build_header(buf, sizeof(buf), "TCHODR10001", 294, "MOCKPROBE00", 0);
        memset(buf + n, ' ', 294);
        memcpy(buf + n + 202, "VXCLORD0000042", 14);   /* 회원사용영역 오프셋 202 마커 */
        n += 294;
        send(fd, buf, n, 0);
        printf("vx_probe: sent TCHODR10001 order (%d B, member marker VXCLORD0000042)\n", n);
        usleep(300000);
    }
    if (!do_recv) { usleep(300000); close(fd); printf("vx_probe: send-only done\n"); return 0; }

    n = krx_build_header(buf, sizeof(buf), "SCHOPQ10000", 0, "MOCKPROBE00", 0);
    send(fd, buf, n, 0);
    printf("vx_probe: sent SCHOPQ10000 (%d B), draining pushes...\n", n);

    usleep(1500000);   /* 카탈로그 push 대기(상품별 usleep 300ms) */
    while ((n = recv(fd, r + total, sizeof(r) - total - 1, MSG_DONTWAIT)) > 0) {
        total += n; if (total >= (int)sizeof(r) - 1) break; usleep(200000);
    }
    r[total > 0 ? total : 0] = 0;
    printf("vx_probe: received %d bytes total\n", total);

    for (i = 0; i < 4; i++) {
        int cnt = 0, off;
        for (off = 0; off + 11 <= total; off++)
            if (memcmp(r + off, trs[i], 11) == 0) cnt++;
        printf("  %s : %d\n", trs[i], cnt);
    }
    /* VX-1b-full: 주문 회원영역 마커가 응답/체결에 echo 됐는지(=correlation) */
    { int off, mk = 0;
      for (off = 0; off + 14 <= total; off++)
          if (memcmp(r + off, "VXCLORD0000042", 14) == 0) mk++;
      printf("  CORRELATION member echo [VXCLORD0000042] : %d (>=1 이면 상관성공)\n", mk); }
    close(fd);
    return 0;
}
