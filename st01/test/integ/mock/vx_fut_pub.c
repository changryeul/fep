/*------------------------------------------------------------------------
#   vx_fut_pub.c — 가상 선물 거래소 연속 시세 발행기  [VX-5]
#
#   KRX 파생 체결 원 TR(A306F/A301F, 173B ASCII)을 멀티캐스트로 연속 발행.
#   같은 시세를 두 소비자에게 팬아웃(그룹/TR코드만 다름, 값 동일):
#     - wtg : A306F → 227.10.20.10:60641 → mci-price-krx(화면 SHM) + mci-edge-krx(ws)
#     - FEP : A301F → 239.1.1.1:17001    → pc_7100_ur(차익거래)
#   A306F/A301F 는 CO_A301F(A301F~A316F) 173B 동일 계열. 숫자=소수점 ASCII(atof).
#   레이아웃(DecodeA306F): TR@0(5) seqno@5(8) board@13(2) sess@15(2) code@17(12)
#     time@35(12) cprc@47(9) cvol@56(9) nprc@65 fprc@74 oprc@83 hprc@92 lprc@101
#     pprc@110(9) tvol@119(12) tamt@131(22) ftcd@153(1) uldp@154(9)  = 173B
#
#   빌드: cc -o vx_fut_pub vx_fut_pub.c
#   실행(env): VX_FUT_WTG_GRP/PORT, VX_FUT_FEP_GRP/PORT, VX_FUT_CODES, VX_FUT_IFACE
#     인자: [interval_ms(기본1000)] [duration_sec(0=무한)]
------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TRLEN   173
#define MSTLEN  1318
#define MAXCODE 16

/* 고정폭 필드에 우측정렬 실수(소수점 ASCII) 기록 */
static void putf(char *b, int off, int w, double v)
{ char t[64]; snprintf(t, sizeof(t), "%*.2f", w, v); memcpy(b + off, t, w); }
static void puti(char *b, int off, int w, long v)
{ char t[64]; snprintf(t, sizeof(t), "%*ld", w, v); memcpy(b + off, t, w); }
static void puts_l(char *b, int off, int w, const char *s)  /* 좌측정렬 문자 */
{ int n = strlen(s); if (n > w) n = w; memcpy(b + off, s, n); }

/* 파생 체결 원 TR 1건 구성(173B, 나머지는 공백) */
static void build_tr(char *b, const char *trcode, long seq, const char *code,
                     const char *hms, double cprc, long cvol, double oprc,
                     double hprc, double lprc, double pprc, long tvol,
                     double tamt, double uldp)
{
    memset(b, ' ', TRLEN);
    memcpy(b, trcode, 5);              /* @0  TR "A306F"/"A301F" */
    puti(b, 5, 8, seq);               /* @5  seqno */
    memcpy(b + 13, "G1", 2);          /* @13 board_id */
    memcpy(b + 15, "  ", 2);          /* @15 session_id */
    puts_l(b, 17, 12, code);          /* @17 종목코드 */
    puts_l(b, 35, 12, hms);           /* @35 time HHMMSSmmm */
    putf(b, 47, 9, cprc);             /* @47 체결가 */
    puti(b, 56, 9, cvol);             /* @56 체결수량 */
    putf(b, 65, 9, cprc);             /* @65 근월물(=cprc 대용) */
    putf(b, 74, 9, cprc);             /* @74 원월물 */
    putf(b, 83, 9, oprc);             /* @83 시가 */
    putf(b, 92, 9, hprc);             /* @92 고가 */
    putf(b, 101, 9, lprc);            /* @101 저가 */
    putf(b, 110, 9, pprc);            /* @110 직전가 */
    puti(b, 119, 12, tvol);           /* @119 누적거래량 */
    putf(b, 131, 22, tamt);           /* @131 누적거래대금 */
    memcpy(b + 153, "2", 1);          /* @153 매도매수구분 */
    putf(b, 154, 9, uldp);            /* @154 상한 */
    /* @163~172 (하한 등) 공백 유지 */
}

/* 파생 종목 마스터 A006F(1318B) — DecodeA006F 오프셋 기준(종목/기준가/상하한/전일종가) */
static void build_master(char *b, long seq, const char *code, const char *name,
                         double bprc, double upl, double dnl, double yprc, const char *exdt)
{
    memset(b, ' ', MSTLEN);
    memcpy(b, "A006F", 5);            /* @0   TR */
    puti(b, 5, 8, seq);               /* @5   seqno */
    puts_l(b, 27, 12, code);          /* @27  표준종목코드 */
    memcpy(b + 45, "F", 1);           /* @45  focd 선물 */
    puts_l(b, 57, 9, code);           /* @57  iscd 단축코드 */
    puts_l(b, 66, 80, name);          /* @66  klnm 종목명 */
    puts_l(b, 146, 40, name);         /* @146 ksnm 약명 */
    puts_l(b, 309, 8, "20250102");    /* @309 ltdt 상장일 */
    putf(b, 331, 11, upl);            /* @331 upl1 상한 */
    putf(b, 364, 11, dnl);            /* @364 lpl1 하한 */
    putf(b, 397, 11, bprc);           /* @397 bprc 기준가 */
    puts_l(b, 457, 8, exdt);          /* @457 exdt 만기 */
    putf(b, 484, 22, 1.0);            /* @484 unit 거래단위 */
    putf(b, 506, 22, 250000.0);       /* @506 mult 거래승수 */
    putf(b, 748, 11, yprc);           /* @748 yprc 전일종가 */
}

int main(int argc, char **argv)
{
    int    interval_ms = (argc > 1) ? atoi(argv[1]) : 1000;
    int    duration_s  = (argc > 2) ? atoi(argv[2]) : 0;
    const char *wgrp = getenv("VX_FUT_WTG_GRP"); const char *wprt = getenv("VX_FUT_WTG_PORT");
    const char *fgrp = getenv("VX_FUT_FEP_GRP"); const char *fprt = getenv("VX_FUT_FEP_PORT");
    const char *codes_env = getenv("VX_FUT_CODES");
    const char *iface = getenv("VX_FUT_IFACE");
    char   codes[MAXCODE][16]; double px[MAXCODE], op[MAXCODE], hi[MAXCODE], lo[MAXCODE], pp[MAXCODE], base[MAXCODE];
    long   tv[MAXCODE]; int ncode = 0;
    struct sockaddr_in wtg, fep;
    int    s, i, on = 1; unsigned char ttl = 4, loop = 1;
    long   seq = 1, tick = 0;
    time_t start = time(NULL);

    if (!wgrp) wgrp = "227.10.20.10"; if (!wprt) wprt = "60641";
    if (!fgrp) fgrp = "239.1.1.1";    if (!fprt) fprt = "17001";
    if (!codes_env) codes_env = "101V6000,105V3000";

    { char tmp[256], *t; strncpy(tmp, codes_env, sizeof(tmp)-1); tmp[sizeof(tmp)-1]=0;
      for (t = strtok(tmp, ","); t && ncode < MAXCODE; t = strtok(NULL, ",")) {
        strncpy(codes[ncode], t, 15); codes[ncode][15]=0;
        px[ncode] = 260.0 + ncode*55.0;             /* 시작가(종목별 상이) */
        base[ncode]=px[ncode];                      /* 기준가(전일종가) 고정 */
        op[ncode]=px[ncode]; hi[ncode]=px[ncode]; lo[ncode]=px[ncode]; pp[ncode]=px[ncode];
        tv[ncode]=0; ncode++;
      } }

    s = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    if (iface && *iface) {
        struct in_addr ia; ia.s_addr = inet_addr(iface);
        if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &ia, sizeof(ia)) < 0)
            fprintf(stderr, "IP_MULTICAST_IF(%s) 실패 {%d}\n", iface, errno);
    }
    memset(&wtg, 0, sizeof(wtg)); wtg.sin_family = AF_INET;
    wtg.sin_addr.s_addr = inet_addr(wgrp); wtg.sin_port = htons((unsigned short)atoi(wprt));
    memset(&fep, 0, sizeof(fep)); fep.sin_family = AF_INET;
    fep.sin_addr.s_addr = inet_addr(fgrp); fep.sin_port = htons((unsigned short)atoi(fprt));

    printf("vx_fut_pub: wtg %s:%s(A306F) + FEP %s:%s(A301F), codes=%d, %dms, dur=%ds\n",
           wgrp, wprt, fgrp, fprt, ncode, interval_ms, duration_s); fflush(stdout);

    srand((unsigned)start);
    for (;;) {
        struct tm *lt; time_t now = time(NULL); char hms[16];
        if (duration_s > 0 && (now - start) >= duration_s) break;
        lt = localtime(&now);
        snprintf(hms, sizeof(hms), "%02d%02d%02d%03ld", lt->tm_hour, lt->tm_min, lt->tm_sec, (tick%1000));
        if (tick == 0 || tick % 60 == 0) {          /* 마스터(A006F): 기동시 + 주기 재발행 → wtg 화면 종목/기준가/등락 */
            for (i = 0; i < ncode; i++) {
                char mb[MSTLEN];
                build_master(mb, seq++, codes[i], "KOSPI200 FUT",
                             base[i], base[i]*1.08, base[i]*0.92, base[i], "20260312");
                sendto(s, mb, MSTLEN, 0, (struct sockaddr*)&wtg, sizeof(wtg));
            }
        }
        for (i = 0; i < ncode; i++) {
            char b[TRLEN]; double step = ((rand()%7)-3) * 0.05;   /* ±0.15 워크(0.05틱) */
            pp[i] = px[i]; px[i] += step; if (px[i] < 1) px[i] = 1;
            if (px[i] > hi[i]) hi[i] = px[i]; if (px[i] < lo[i]) lo[i] = px[i];
            tv[i] += 1 + (rand()%5);
            build_tr(b, "A306F", seq, codes[i], hms, px[i], 1+(rand()%5),
                     op[i], hi[i], lo[i], pp[i], tv[i], px[i]*tv[i]*250000.0, px[i]*1.1);
            sendto(s, b, TRLEN, 0, (struct sockaddr*)&wtg, sizeof(wtg));   /* → wtg */
            memcpy(b, "A301F", 5);                                        /* 같은 값, TR만 교체 */
            sendto(s, b, TRLEN, 0, (struct sockaddr*)&fep, sizeof(fep));   /* → FEP */
            seq++;
        }
        tick++;
        if (tick % 10 == 0) { printf("vx_fut_pub: tick=%ld seq=%ld last[%s]=%.2f\n", tick, seq, codes[0], px[0]); fflush(stdout); }
        usleep(interval_ms * 1000);
    }
    printf("vx_fut_pub: done ticks=%ld\n", tick); fflush(stdout);
    close(s);
    return (0);
}
