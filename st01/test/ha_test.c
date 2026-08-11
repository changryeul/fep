/*------------------------------------------------------------------------
 *  HA(이중화) 로직 단독 테스트 프로그램
 *  File: ha_test.c
 *
 *  pb_7100_ur의 HA 로직(heartbeat 송수신, 절체/복귀)을
 *  SHM, INI, KRX 연결 없이 단독으로 검증한다.
 *
 *  사용법:
 *    gcc -o ha_test ha_test.c -Wall
 *
 *    [테스트 1] 정상 동작 — Primary/Secondary 동시 기동
 *      터미널1: ./ha_test primary
 *      터미널2: ./ha_test secondary
 *      => Secondary가 Standby 유지 확인
 *
 *    [테스트 2] Failover — Primary 종료
 *      터미널1: Ctrl+C (Primary 종료)
 *      => Secondary가 5초 후 Active 전환 확인
 *
 *    [테스트 3] Failback — Primary 재기동
 *      터미널1: ./ha_test primary (다시 시작)
 *      => Secondary가 heartbeat 수신 후 Standby 복귀 확인
 *
 *    [테스트 4] 자동 시나리오 (fork)
 *      ./ha_test auto
 *      => Primary/Secondary fork 후 자동으로 Failover/Failback 시연
 *
 *    [테스트 5] FR-08 진동 검증
 *      ./ha_test oscillation
 *      => Device_Close sleep(3) 진동 현상 재현
 *
 *  환경변수:
 *    HA_TEST_PORT  - heartbeat 포트 (기본: 55555)
 *    HA_TEST_IP    - 상대 IP (기본: 127.0.0.1)
 *------------------------------------------------------------------------*/

#define _XOPEN_SOURCE   700
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

/* MSG_DONTWAIT removed: value differs between Linux(0x40) and macOS(0x80),
   and _XOPEN_SOURCE may hide it. Use blocking recvfrom after select instead. */

/*------------------------------------------------------------------------
 *  상수 (pb_7100_ur.c와 동일)
 *------------------------------------------------------------------------*/
#define HB_PORT_DEFAULT 55555   /* 테스트용 포트 (운영 50000과 분리) */
#define HB_SEND_INTVL   1
#define HB_TIMEOUT      5
#define HB_STABLE_COUNT 3       /* Failback 조건: 연속 수신 횟수     */
#define HB_DATA_TIMEOUT 5       /* Primary 시세 끊김 연속 N회 → 절체 */

/* HB 패킷 구조 (pb_7100_ur.c와 동일) */
typedef struct {
    char    hb_mark[2];     /* "HB"                              */
    int     recv_cnt;       /* 직전 HB 이후 시세 수신 건수       */
} HB_PACKET;

#define HB_PKT_LEN      sizeof(HB_PACKET)

#define HA_PRIMARY      1
#define HA_SECONDARY    2
#define HA_STANDALONE   0

#define ON              1
#define OFF             0

/*------------------------------------------------------------------------
 *  전역 변수 (pb_7100_ur.c 구조와 동일)
 *------------------------------------------------------------------------*/
int     ha_role      = HA_STANDALONE;
int     ha_active    = ON;
int     hb_port      = HB_PORT_DEFAULT;
int     hb_sockfd    = -1;
time_t  hb_last_recv = 0;
time_t  hb_last_send = 0;
struct sockaddr_in hb_peer_addr;
char    ha_peer_ip[20] = "127.0.0.1";

int     hb_stable_cnt   = 0;
int     my_recv_cnt     = 0;    /* 자기 시세 수신 건수 (매 HB 리셋)  */
int     pri_recv_cnt    = 0;    /* Primary의 시세 수신 건수 (HB수신) */
int     pri_no_data_cnt = 0;    /* Primary 시세 없음 연속 횟수       */
int     sim_market_data = 1;    /* 시세 수신 시뮬레이션 (1=수신중)   */

volatile int running = 1;

/*------------------------------------------------------------------------
 *  유틸리티: 타임스탬프 출력
 *------------------------------------------------------------------------*/
static void ts_print(const char *role, const char *fmt, ...)
{
    struct timeval tv;
    struct tm *tm;
    va_list ap;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    printf("[%02d:%02d:%02d.%03d] [%-9s] ",
           tm->tm_hour, tm->tm_min, tm->tm_sec,
           (int)(tv.tv_usec / 1000), role);

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    printf("\n");
    fflush(stdout);
}

static const char *role_str(void)
{
    switch (ha_role) {
        case HA_PRIMARY:   return "PRIMARY";
        case HA_SECONDARY: return "SECONDARY";
        default:           return "STANDALONE";
    }
}

static const char *active_str(void)
{
    return ha_active == ON ? "ACTIVE(TCP전송)" : "STANDBY(대기)";
}

/*------------------------------------------------------------------------
 *  HA_Init: pb_7100_ur.c와 동일한 로직
 *------------------------------------------------------------------------*/
int HA_Init(void)
{
    char *env_ip, *env_port;
    struct sockaddr_in bind_addr;

    /* 환경변수에서 설정 읽기 */
    env_ip = getenv("HA_TEST_IP");
    if (env_ip != NULL && strlen(env_ip) > 0)
        strncpy(ha_peer_ip, env_ip, sizeof(ha_peer_ip) - 1);

    env_port = getenv("HA_TEST_PORT");
    if (env_port != NULL) {
        int p = atoi(env_port);
        if (p > 0 && p < 65536) hb_port = p;
    }

    ts_print(role_str(), "HA_Init: role=%s, peer=%s, port=%d",
             role_str(), ha_peer_ip, hb_port);

    /* Standalone이면 HA 불필요 */
    if (ha_role == HA_STANDALONE) {
        ha_active = ON;
        ts_print(role_str(), "STANDALONE mode — HA disabled, TCP always active");
        return 0;
    }

    /* heartbeat UDP 소켓 생성 */
    hb_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (hb_sockfd < 0) {
        ts_print(role_str(), "ERROR: heartbeat socket failed: %s", strerror(errno));
        ha_role = HA_STANDALONE;
        ha_active = ON;
        return -1;
    }

    /* SO_REUSEADDR + SO_REUSEPORT 설정 */
    {
        int optval = 1;
        setsockopt(hb_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
#ifdef SO_REUSEPORT
        setsockopt(hb_sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
#endif
    }

    if (ha_role == HA_PRIMARY) {
        /* Primary: 전송 대상 주소 설정 */
        ha_active = ON;
        memset(&hb_peer_addr, 0, sizeof(hb_peer_addr));
        hb_peer_addr.sin_family = AF_INET;
        hb_peer_addr.sin_port = htons(hb_port);
        hb_peer_addr.sin_addr.s_addr = inet_addr(ha_peer_ip);

        ts_print(role_str(), "HA_Init: PRIMARY — TCP active, heartbeat→%s:%d",
                 ha_peer_ip, hb_port);
    }
    else if (ha_role == HA_SECONDARY) {
        /* Secondary: 수신용 바인드 */
        ha_active = OFF;
        memset(&bind_addr, 0, sizeof(bind_addr));
        bind_addr.sin_family = AF_INET;
        bind_addr.sin_port = htons(hb_port);
        bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(hb_sockfd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
            ts_print(role_str(), "ERROR: heartbeat bind failed: %s", strerror(errno));
            close(hb_sockfd);
            hb_sockfd = -1;
            ha_role = HA_STANDALONE;
            ha_active = ON;
            return -1;
        }

        hb_last_recv = time(NULL);

        ts_print(role_str(), "HA_Init: SECONDARY — TCP standby, listening port %d",
                 hb_port);
    }

    return 0;
}

/*------------------------------------------------------------------------
 *  HA_Heartbeat_Send: Primary → Secondary heartbeat 전송
 *------------------------------------------------------------------------*/
void HA_Heartbeat_Send(void)
{
    time_t now = time(NULL);
    HB_PACKET pkt;

    if (ha_role != HA_PRIMARY || hb_sockfd < 0)
        return;

    if (now - hb_last_send >= HB_SEND_INTVL) {
        /* 시세 수신 시뮬레이션: sim_market_data가 ON이면 매초 5건씩 */
        if (sim_market_data)
            my_recv_cnt += 5;

        /* HB 패킷 구성: "HB" + 시세 수신 건수 */
        memcpy(pkt.hb_mark, "HB", 2);
        pkt.recv_cnt = my_recv_cnt;
        my_recv_cnt = 0;

        sendto(hb_sockfd, (char *)&pkt, HB_PKT_LEN, 0,
               (struct sockaddr *)&hb_peer_addr, sizeof(hb_peer_addr));
        hb_last_send = now;
        if (now % 10 == 0)
            ts_print(role_str(), "heartbeat sent (data=%d)", pkt.recv_cnt);
    }
}

/*------------------------------------------------------------------------
 *  HA_Check_Failover: Secondary의 절체/복귀 판단
 *------------------------------------------------------------------------*/
void HA_Check_Failover(void)
{
    time_t now;
    int    elapsed;
    int    my_data;     /* 직전 주기 시세 수신 건수 (스냅샷) */

    if (ha_role != HA_SECONDARY)
        return;

    now = time(NULL);
    elapsed = (int)(now - hb_last_recv);

    /* my_recv_cnt 스냅샷 후 리셋 — 누적 방지 (장 종료 시 false failover 방지) */
    my_data = my_recv_cnt;
    my_recv_cnt = 0;

    if (ha_active == OFF) {
        /* Failover 판단 1: HB 타임아웃 */
        if (elapsed >= HB_TIMEOUT) {
            ha_active = ON;
            hb_stable_cnt = 0;
            pri_no_data_cnt = 0;
            ts_print(role_str(),
                     "*** FAILOVER(HB) *** heartbeat %d초 미수신 → ACTIVE!",
                     elapsed);
            return;
        }

        /* Failover 판단 2: Primary 시세 끊김 */
        if (pri_recv_cnt == 0 && my_data > 0) {
            pri_no_data_cnt++;
            if (pri_no_data_cnt >= HB_DATA_TIMEOUT) {
                ha_active = ON;
                hb_stable_cnt = 0;
                pri_no_data_cnt = 0;
                ts_print(role_str(),
                         "*** FAILOVER(DATA) *** Primary 시세 없음 %d회 연속 "
                         "(my=%d) → ACTIVE!",
                         HB_DATA_TIMEOUT, my_data);
            }
        }
        else {
            pri_no_data_cnt = 0;
        }
    }
    else {
        /* Failback 판단 */
        if (elapsed >= HB_TIMEOUT) {
            hb_stable_cnt = 0;
        }
        else if (hb_stable_cnt >= HB_STABLE_COUNT) {
            if (pri_recv_cnt > 0 || my_data == 0) {
                ts_print(role_str(),
                         "*** FAILBACK *** Primary 안정 (HB %d회, data=%d) "
                         "→ STANDBY!",
                         HB_STABLE_COUNT, pri_recv_cnt);
                ha_active = OFF;
                hb_stable_cnt = 0;
                pri_no_data_cnt = 0;
            }
            else {
                hb_stable_cnt = 0;  /* 다시 3회 대기 (로그 반복 방지) */
                ts_print(role_str(),
                         "FAILBACK blocked: Primary alive but no data "
                         "(pri=%d, my=%d)",
                         pri_recv_cnt, my_data);
            }
        }
    }
}

/*------------------------------------------------------------------------
 *  Recv_Heartbeat: select()로 heartbeat 수신 (Recv_Data 간소화 버전)
 *
 *  실제 pb_7100_ur에서는 시세 소켓(Sockfd)과 hb_sockfd를 동시에
 *  select()로 감시한다. 여기서는 hb_sockfd만 감시.
 *------------------------------------------------------------------------*/
int Recv_Heartbeat(void)
{
    fd_set rfds;
    struct timeval tv;
    int ret;
    char buf[64];
    struct sockaddr_in from;
    socklen_t fromlen;

    if (hb_sockfd < 0)
        return 0;

    FD_ZERO(&rfds);
    FD_SET(hb_sockfd, &rfds);

    tv.tv_sec = 1;     /* 1초 타임아웃 (pb_7100_ur과 동일) */
    tv.tv_usec = 0;

    ret = select(hb_sockfd + 1, &rfds, NULL, NULL, &tv);

    if (ret > 0 && FD_ISSET(hb_sockfd, &rfds)) {
        /* heartbeat 수신 — select()가 ready를 보고했으므로 1회 읽기 */
        fromlen = sizeof(from);
        ret = recvfrom(hb_sockfd, buf, sizeof(buf), 0,
                       (struct sockaddr *)&from, &fromlen);
        if (ret < 0) {
            ts_print(role_str(), "HB recvfrom fail: %s", strerror(errno));
            return 0;
        }
        if (ret >= (int)HB_PKT_LEN && memcmp(buf, "HB", 2) == 0) {
            /* 확장 HB 패킷: 시세 수신 건수 추출 */
            HB_PACKET *p = (HB_PACKET *)buf;
            pri_recv_cnt = p->recv_cnt;
            hb_last_recv = time(NULL);
            hb_stable_cnt++;
            ts_print(role_str(), "HB recv ok (pri_cnt=%d, stable=%d, rt=%d)",
                     pri_recv_cnt, hb_stable_cnt, ret);
        }
        else if (ret >= 2 && memcmp(buf, "HB", 2) == 0) {
            /* 구 버전 또는 크기 불일치 HB 호환 */
            pri_recv_cnt = -1;
            hb_last_recv = time(NULL);
            hb_stable_cnt++;
            ts_print(role_str(), "HB recv ok (legacy, rt=%d, stable=%d)",
                     ret, hb_stable_cnt);
        }
        else {
            ts_print(role_str(), "HB unknown pkt (rt=%d, hdr=0x%02X%02X)",
                     ret, (unsigned char)buf[0], (unsigned char)buf[1]);
        }
        return 1;   /* heartbeat 수신됨 */
    }

    return 0;   /* 타임아웃 (수신 없음) */
}

/*------------------------------------------------------------------------
 *  시그널 핸들러
 *------------------------------------------------------------------------*/
void sig_handler(int sig)
{
    (void)sig;
    running = 0;
}

/*------------------------------------------------------------------------
 *  simulate_device_close: FR-08 진동 재현용 (기존 버그)
 *------------------------------------------------------------------------*/
void simulate_device_close(void)
{
    ts_print(role_str(), "Device_Close: TCP 종료 + sleep(3) 시작...");
    sleep(3);   /* <-- 이것이 FR-08 진동의 원인 */
    ts_print(role_str(), "Device_Close: sleep(3) 완료");
}

/*------------------------------------------------------------------------
 *  simulate_device_close_ha: FR-08 수정판 (sleep 없음)
 *------------------------------------------------------------------------*/
void simulate_device_close_ha(void)
{
    ts_print(role_str(), "Device_Close_HA: TCP 종료 (no sleep)");
}

/*------------------------------------------------------------------------
 *  run_primary: Primary 메인 루프
 *------------------------------------------------------------------------*/
void run_primary(void)
{
    ha_role = HA_PRIMARY;
    HA_Init();

    ts_print(role_str(), "=== Primary 시작 (Ctrl+C로 장애 시뮬레이션) ===");

    while (running) {
        HA_Heartbeat_Send();
        usleep(200000);     /* 200ms — 실제 Recv_Data의 select 1초보다 빠르게 */
    }

    if (hb_sockfd >= 0) { close(hb_sockfd); hb_sockfd = -1; }
    ts_print(role_str(), "=== Primary 종료 ===");
}

/*------------------------------------------------------------------------
 *  run_secondary: Secondary 메인 루프
 *------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 *  run_secondary: Secondary 메인 루프
 *  close_mode: 0=close없음(auto), 1=sleep포함(FR-08재현), 2=sleep없음(수정판)
 *------------------------------------------------------------------------*/
void run_secondary(int close_mode)
{
    int prev_active;

    ha_role = HA_SECONDARY;
    HA_Init();

    ts_print(role_str(), "=== Secondary 시작 (Standby 대기) ===");
    if (close_mode == 1)
        ts_print(role_str(), "[FR-08 재현] failback 시 Device_Close sleep(3) 포함");
    else if (close_mode == 2)
        ts_print(role_str(), "[FR-08 수정] failback 시 Device_Close_HA (no sleep)");

    {
        int loop_cnt = 0;
        time_t last_diag = 0;

        while (running) {
            prev_active = ha_active;

            HA_Check_Failover();
            Recv_Heartbeat();
            loop_cnt++;

            /* 시세 수신 시뮬레이션: sim_market_data가 ON이면 매 루프 1건 */
            if (sim_market_data)
                my_recv_cnt++;

            /* 5초마다 진단 출력 */
            {
                time_t now = time(NULL);
                if (now - last_diag >= 5) {
                    ts_print(role_str(),
                             "[DIAG] loop=%d active=%d elapsed=%ld "
                             "stable=%d pri_data=%d my_data=%d nodata=%d",
                             loop_cnt, ha_active,
                             (long)(now - hb_last_recv), hb_stable_cnt,
                             pri_recv_cnt, my_recv_cnt, pri_no_data_cnt);
                    last_diag = now;
                }
            }

            /* 상태 변경 감지 시 출력 */
            if (prev_active != ha_active) {
                ts_print(role_str(), ">>> 상태 변경: %s", active_str());

                /* Failback 시 Device_Close 시뮬레이션 */
                if (prev_active == ON && ha_active == OFF) {
                    if (close_mode == 1)
                        simulate_device_close();       /* 버그 재현: sleep(3) */
                    else if (close_mode == 2)
                        simulate_device_close_ha();    /* 수정판: no sleep */
                }
            }
        }
    }

    if (hb_sockfd >= 0) { close(hb_sockfd); hb_sockfd = -1; }
    ts_print(role_str(), "=== Secondary 종료 ===");
}

/*------------------------------------------------------------------------
 *  run_auto: fork로 Primary/Secondary 동시 실행 + 자동 시나리오
 *------------------------------------------------------------------------*/
void run_auto(void)
{
    pid_t primary_pid, secondary_pid;

    printf("============================================================\n");
    printf("  HA 자동 테스트 시나리오\n");
    printf("============================================================\n");
    printf("  [Phase 1]  0~ 8초: 정상 운영 (Primary=Active, Secondary=Standby)\n");
    printf("  [Phase 2]  8~14초: Failover (Primary 종료 → Secondary Active)\n");
    printf("  [Phase 3] 14~22초: Failback (Primary 재기동 → Secondary Standby)\n");
    printf("  [Phase 4] 22초~  : 정상 복귀 확인 후 종료\n");
    printf("============================================================\n\n");

    fflush(stdout);

    /* Secondary 먼저 시작 (수정판: Device_Close_HA 사용) */
    secondary_pid = fork();
    if (secondary_pid == 0) {
        /* 자식: Secondary */
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        run_secondary(2);
        exit(0);
    }

    usleep(500000);  /* Secondary bind 대기 */

    /* Phase 1: Primary 기동 */
    ts_print("SCENARIO", "[Phase 1] Primary 기동 — 정상 운영 8초");

    fflush(stdout);
    primary_pid = fork();
    if (primary_pid == 0) {
        /* 자식: Primary */
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        run_primary();
        exit(0);
    }

    sleep(8);

    /* Phase 2: Primary 종료 → Failover 유도 */
    ts_print("SCENARIO", "[Phase 2] Primary 종료 → Failover 대기 %d초...", HB_TIMEOUT);
    kill(primary_pid, SIGTERM);
    waitpid(primary_pid, NULL, 0);

    sleep(8);   /* HB_TIMEOUT(5초) + 여유 3초 */

    /* Phase 3: Primary 재기동 → Failback 유도 */
    ts_print("SCENARIO", "[Phase 3] Primary 재기동 → Failback 대기...");
    fflush(stdout);

    primary_pid = fork();
    if (primary_pid == 0) {
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        run_primary();
        exit(0);
    }

    sleep(8);

    /* Phase 4: 정리 */
    ts_print("SCENARIO", "[Phase 4] 테스트 완료 — 정리 중");
    kill(primary_pid, SIGTERM);
    kill(secondary_pid, SIGTERM);
    waitpid(primary_pid, NULL, 0);
    waitpid(secondary_pid, NULL, 0);

    printf("\n============================================================\n");
    printf("  테스트 완료\n");
    printf("============================================================\n");
}

/*------------------------------------------------------------------------
 *  run_oscillation: FR-08 진동 재현
 *------------------------------------------------------------------------*/
void run_oscillation(void)
{
    pid_t primary_pid, secondary_pid;

    printf("============================================================\n");
    printf("  FR-08 진동(oscillation) 재현 테스트\n");
    printf("============================================================\n");
    printf("  원인: failback 시 Device_Close()의 sleep(3)이\n");
    printf("        heartbeat 수신을 차단 → 재 failover 반복\n");
    printf("============================================================\n");
    printf("  [Phase 1]  0~ 5초: 정상 운영\n");
    printf("  [Phase 2]  5~12초: Primary 종료 → Failover\n");
    printf("  [Phase 3] 12~30초: Primary 재기동 → 진동 관찰\n");
    printf("============================================================\n\n");

    fflush(stdout);

    /* Secondary (FR-08 모드: Device_Close sleep 포함) */
    secondary_pid = fork();
    if (secondary_pid == 0) {
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        run_secondary(1);   /* use_device_close_sleep = 1 */
        exit(0);
    }

    usleep(500000);

    /* Phase 1: 정상 운영 */
    ts_print("SCENARIO", "[Phase 1] 정상 운영 5초");
    fflush(stdout);
    primary_pid = fork();
    if (primary_pid == 0) {
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        run_primary();
        exit(0);
    }

    sleep(5);

    /* Phase 2: Failover */
    ts_print("SCENARIO", "[Phase 2] Primary 종료 → Failover");
    kill(primary_pid, SIGTERM);
    waitpid(primary_pid, NULL, 0);

    sleep(8);   /* HB_TIMEOUT(5초) + 여유 3초 */

    /* Phase 3: Failback → 진동 관찰 */
    ts_print("SCENARIO", "[Phase 3] Primary 재기동 → 진동 관찰 (18초)");
    fflush(stdout);
    primary_pid = fork();
    if (primary_pid == 0) {
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        run_primary();
        exit(0);
    }

    sleep(18);

    /* 정리 */
    ts_print("SCENARIO", "테스트 완료 — 정리");
    kill(primary_pid, SIGTERM);
    kill(secondary_pid, SIGTERM);
    waitpid(primary_pid, NULL, 0);
    waitpid(secondary_pid, NULL, 0);

    printf("\n============================================================\n");
    printf("  FR-08 진동 테스트 완료\n");
    printf("  위 로그에서 FAILOVER/FAILBACK이 반복되면 진동 재현 성공\n");
    printf("============================================================\n");
}

/*------------------------------------------------------------------------
 *  usage
 *------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 *  run_fixed: FR-08 수정 검증 (진동 없음 확인)
 *------------------------------------------------------------------------*/
void run_fixed(void)
{
    pid_t primary_pid, secondary_pid;

    printf("============================================================\n");
    printf("  FR-08 수정 검증 — Device_Close_HA (sleep 없음)\n");
    printf("============================================================\n");
    printf("  oscillation과 동일한 시나리오이나\n");
    printf("  failback 시 Device_Close_HA (no sleep) 사용\n");
    printf("  => 진동 없이 안정적으로 Standby 복귀해야 함\n");
    printf("============================================================\n\n");

    fflush(stdout);

    /* Secondary (수정판: close_mode=2) */
    secondary_pid = fork();
    if (secondary_pid == 0) {
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        run_secondary(2);
        exit(0);
    }

    usleep(500000);

    /* Phase 1: 정상 운영 */
    ts_print("SCENARIO", "[Phase 1] 정상 운영 5초");
    fflush(stdout);
    primary_pid = fork();
    if (primary_pid == 0) {
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        run_primary();
        exit(0);
    }

    sleep(5);

    /* Phase 2: Failover */
    ts_print("SCENARIO", "[Phase 2] Primary 종료 → Failover");
    kill(primary_pid, SIGTERM);
    waitpid(primary_pid, NULL, 0);

    sleep(8);   /* HB_TIMEOUT(5초) + 여유 3초 */

    /* Phase 3: Failback → 안정 복귀 확인 */
    ts_print("SCENARIO", "[Phase 3] Primary 재기동 → 안정 Failback 확인 (10초)");
    fflush(stdout);
    primary_pid = fork();
    if (primary_pid == 0) {
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        run_primary();
        exit(0);
    }

    sleep(10);

    /* 정리 */
    ts_print("SCENARIO", "테스트 완료 — 정리");
    kill(primary_pid, SIGTERM);
    kill(secondary_pid, SIGTERM);
    waitpid(primary_pid, NULL, 0);
    waitpid(secondary_pid, NULL, 0);

    printf("\n============================================================\n");
    printf("  FR-08 수정 검증 완료\n");
    printf("  FAILOVER→FAILBACK이 1회만 발생하면 수정 성공\n");
    printf("============================================================\n");
}

/*------------------------------------------------------------------------
 *  run_nodata: Primary 시세 끊김 시나리오
 *  - Phase 1: 양쪽 정상 (5초)
 *  - Phase 2: Primary의 시세만 끊김 (10초) → DATA FAILOVER 발생해야 함
 *  - Phase 3: Primary 시세 복구 (10초) → FAILBACK 발생해야 함
 *------------------------------------------------------------------------*/
void run_nodata(void)
{
    pid_t primary_pid, secondary_pid;

    printf("============================================================\n");
    printf("  시세 기반 절체 테스트 — Primary 시세 끊김\n");
    printf("============================================================\n");
    printf("  [Phase 1]  0~ 5초: 양쪽 정상 (Primary/Secondary 시세 수신)\n");
    printf("  [Phase 2]  5~15초: Primary 시세만 끊김 → DATA FAILOVER\n");
    printf("  [Phase 3] 15~25초: Primary 시세 복구 → FAILBACK\n");
    printf("============================================================\n\n");

    fflush(stdout);

    /* Secondary (시세 항상 수신) */
    secondary_pid = fork();
    if (secondary_pid == 0) {
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        sim_market_data = 1;    /* Secondary는 항상 시세 수신 */
        run_secondary(2);
        exit(0);
    }

    usleep(500000);

    /* Phase 1: 양쪽 정상 */
    ts_print("SCENARIO", "[Phase 1] 양쪽 정상 운영 5초");
    fflush(stdout);
    primary_pid = fork();
    if (primary_pid == 0) {
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        sim_market_data = 1;    /* Primary도 시세 수신 */
        run_primary();
        exit(0);
    }

    sleep(5);

    /* Phase 2: Primary 시세 끊김 — Primary 재시작 (sim_market_data=0) */
    ts_print("SCENARIO", "[Phase 2] Primary 시세 끊김 → DATA FAILOVER 대기");
    kill(primary_pid, SIGTERM);
    waitpid(primary_pid, NULL, 0);

    fflush(stdout);
    primary_pid = fork();
    if (primary_pid == 0) {
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        sim_market_data = 0;    /* Primary 시세 끊김! */
        run_primary();
        exit(0);
    }

    sleep(10);

    /* Phase 3: Primary 시세 복구 */
    ts_print("SCENARIO", "[Phase 3] Primary 시세 복구 → FAILBACK 대기");
    kill(primary_pid, SIGTERM);
    waitpid(primary_pid, NULL, 0);

    fflush(stdout);
    primary_pid = fork();
    if (primary_pid == 0) {
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        sim_market_data = 1;    /* Primary 시세 복구! */
        run_primary();
        exit(0);
    }

    sleep(10);

    /* 정리 */
    ts_print("SCENARIO", "테스트 완료 — 정리");
    kill(primary_pid, SIGTERM);
    kill(secondary_pid, SIGTERM);
    waitpid(primary_pid, NULL, 0);
    waitpid(secondary_pid, NULL, 0);

    printf("\n============================================================\n");
    printf("  시세 기반 절체 테스트 완료\n");
    printf("  Phase 2에서 DATA FAILOVER, Phase 3에서 FAILBACK이면 성공\n");
    printf("============================================================\n");
}

/*------------------------------------------------------------------------
 *  run_secondary_tcpfail: BUG-06 재현/수정 검증
 *
 *  TCP 연결 실패 시 기존 코드의 `continue`가 Recv_Data를 스킵하여
 *  heartbeat를 영원히 못 읽는 현상을 재현한다.
 *
 *  bug_mode:
 *    1 = 기존 코드 (BUG 재현): ha_active=ON + TCP실패 → HB 수신 스킵
 *    0 = 수정 코드: ha_active=ON + TCP실패 → HB 수신 계속
 *------------------------------------------------------------------------*/
void run_secondary_tcpfail(int bug_mode)
{
    int prev_active;
    int sim_tcp_connected = 0;  /* TCP 서버 미가동 시뮬레이션 */

    ha_role = HA_SECONDARY;
    HA_Init();

    ts_print(role_str(), "=== Secondary 시작 (TCP서버 미가동 시뮬레이션) ===");
    ts_print(role_str(), "mode: %s",
             bug_mode ? "BUG 재현 (continue로 HB 스킵)"
                      : "수정 코드 (HB 항상 수신)");

    {
        int loop_cnt = 0;
        time_t last_diag = 0;

        while (running) {
            prev_active = ha_active;

            HA_Check_Failover();

            /*------------------------------------------------------------
             *  pb_7100_ur.c 메인루프의 TCP 관리 부분을 시뮬레이션:
             *
             *  [기존 코드 - BUG]
             *    if (ha_active == ON) {
             *        if (SockTcpfd < 0) Device_Open();  // 실패
             *        if (SockTcpfd < 0) continue;       // ← HB 스킵!
             *    }
             *
             *  [수정 코드]
             *    if (ha_active == ON) {
             *        if (SockTcpfd < 0) Device_Open();  // 실패
             *        // continue 없음 → Recv_Data 항상 호출
             *    }
             *------------------------------------------------------------*/
            if (ha_active == ON && !sim_tcp_connected) {
                /* TCP 연결 시도 실패 시뮬레이션 */
                if (loop_cnt % 50 == 0) /* 로그 과다 방지 */
                    ts_print(role_str(), "Device_Open fail (TCP서버 미가동)");

                if (bug_mode) {
                    /* BUG 재현: continue — Recv_Heartbeat 호출 안 함 */
                    loop_cnt++;
                    usleep(200000);
                    continue;
                }
                /* 수정 코드: continue 없이 Recv_Heartbeat로 진행 */
            }

            Recv_Heartbeat();
            loop_cnt++;

            if (sim_market_data)
                my_recv_cnt++;

            /* 5초마다 진단 출력 */
            {
                time_t now = time(NULL);
                if (now - last_diag >= 5) {
                    ts_print(role_str(),
                             "[DIAG] loop=%d active=%d tcp=%d elapsed=%ld "
                             "stable=%d pri_data=%d",
                             loop_cnt, ha_active, sim_tcp_connected,
                             (long)(now - hb_last_recv), hb_stable_cnt,
                             pri_recv_cnt);
                    last_diag = now;
                }
            }

            if (prev_active != ha_active) {
                ts_print(role_str(), ">>> 상태 변경: %s", active_str());
            }

            usleep(200000);
        }
    }

    if (hb_sockfd >= 0) { close(hb_sockfd); hb_sockfd = -1; }
    ts_print(role_str(), "=== Secondary 종료 ===");
}

/*------------------------------------------------------------------------
 *  run_tcpfail: BUG-06 재현/수정 비교 테스트
 *
 *  시나리오:
 *    Phase 1: 정상 운영 (Primary Active, Secondary Standby)
 *    Phase 2: Primary 종료 → Secondary FAILOVER (Active)
 *    Phase 3: Primary 재기동, 그러나 TCP 서버는 미가동
 *             → BUG: Secondary가 HB를 못 읽어 Failback 불가
 *             → FIX: Secondary가 HB를 읽어 Failback 성공
 *------------------------------------------------------------------------*/
void run_tcpfail(int bug_mode)
{
    pid_t primary_pid, secondary_pid;

    printf("============================================================\n");
    printf("  BUG-06: TCP 실패 시 HB 수신 차단 %s\n",
           bug_mode ? "[BUG 재현]" : "[수정 검증]");
    printf("============================================================\n");
    printf("  핵심: ha_active=ON + TCP실패 시 continue가\n");
    printf("        Recv_Data()를 스킵 → HB를 영원히 못 읽음\n");
    printf("============================================================\n");
    printf("  [Phase 1]  0~ 5초: 정상 운영\n");
    printf("  [Phase 2]  5~10초: Primary 종료 → FAILOVER\n");
    printf("  [Phase 3] 10~25초: Primary 재기동 + TCP 서버 미가동\n");
    if (bug_mode)
        printf("    → BUG: HB 수신 스킵 → Failback 안 됨 (ACTIVE 고착)\n");
    else
        printf("    → FIX: HB 수신 계속 → Failback 성공 (STANDBY 복귀)\n");
    printf("============================================================\n\n");
    fflush(stdout);

    /* Secondary */
    secondary_pid = fork();
    if (secondary_pid == 0) {
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        run_secondary_tcpfail(bug_mode);
        exit(0);
    }

    usleep(500000);

    /* Phase 1: 정상 운영 */
    ts_print("SCENARIO", "[Phase 1] 정상 운영 5초");
    fflush(stdout);
    primary_pid = fork();
    if (primary_pid == 0) {
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        run_primary();
        exit(0);
    }

    sleep(5);

    /* Phase 2: Primary 종료 → Failover */
    ts_print("SCENARIO", "[Phase 2] Primary 종료 → FAILOVER 대기");
    kill(primary_pid, SIGTERM);
    waitpid(primary_pid, NULL, 0);

    sleep(5);

    /* Phase 3: Primary 재기동 (HB 전송), TCP 서버는 미가동 */
    ts_print("SCENARIO", "[Phase 3] Primary 재기동 + TCP 서버 미가동");
    ts_print("SCENARIO", "  %s",
             bug_mode ? "→ BUG: HB 스킵 → Failback 불가 예상"
                      : "→ FIX: HB 수신 → Failback 예상");
    fflush(stdout);
    primary_pid = fork();
    if (primary_pid == 0) {
        setvbuf(stdout, NULL, _IOLBF, 0);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        run_primary();
        exit(0);
    }

    sleep(15);

    /* 정리 */
    ts_print("SCENARIO", "테스트 완료 — 정리");
    kill(primary_pid, SIGTERM);
    kill(secondary_pid, SIGTERM);
    waitpid(primary_pid, NULL, 0);
    waitpid(secondary_pid, NULL, 0);

    printf("\n============================================================\n");
    if (bug_mode) {
        printf("  BUG 재현 결과:\n");
        printf("  Phase 3에서 FAILBACK이 안 되면 BUG 재현 성공\n");
        printf("  (Secondary가 ACTIVE 고착 = 양쪽 TCP 전달 상태)\n");
    } else {
        printf("  수정 검증 결과:\n");
        printf("  Phase 3에서 FAILBACK이 발생하면 수정 성공\n");
    }
    printf("============================================================\n");
}

void usage(const char *prog)
{
    printf("============================================================\n");
    printf("  HA(이중화) 로직 테스트 — pb_7100_ur heartbeat 검증\n");
    printf("============================================================\n");
    printf("\n");
    printf("사용법:\n");
    printf("  %s primary       Primary 모드 (heartbeat 송신)\n", prog);
    printf("  %s secondary     Secondary 모드 (heartbeat 수신, 절체 판단)\n", prog);
    printf("  %s auto          자동 시나리오 (fork, Failover→Failback)\n", prog);
    printf("  %s oscillation   FR-08 진동 재현 (버그 확인용)\n", prog);
    printf("  %s fixed         FR-08 수정 검증 (진동 없음 확인)\n", prog);
    printf("  %s nodata        시세 기반 절체 (Primary 시세 끊김)\n", prog);
    printf("  %s tcpfail-bug   BUG-06 재현: TCP실패→HB스킵→Failback불가\n", prog);
    printf("  %s tcpfail-fix   BUG-06 수정: TCP실패→HB수신→Failback성공\n", prog);
    printf("\n");
    printf("환경변수:\n");
    printf("  HA_TEST_PORT=%d  heartbeat 포트\n", HB_PORT_DEFAULT);
    printf("  HA_TEST_IP=x.x.x.x     상대 서버 IP (기본 127.0.0.1)\n");
    printf("\n");
    printf("수동 테스트 (터미널 2개):\n");
    printf("  [터미널1] %s primary\n", prog);
    printf("  [터미널2] %s secondary\n", prog);
    printf("  터미널1에서 Ctrl+C → 5초 후 터미널2가 ACTIVE 전환\n");
    printf("  터미널1 다시 시작 → 터미널2가 STANDBY 복귀\n");
    printf("============================================================\n");
}

/*------------------------------------------------------------------------
 *  main
 *------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "primary") == 0) {
        run_primary();
    }
    else if (strcmp(argv[1], "secondary") == 0) {
        run_secondary(0);
    }
    else if (strcmp(argv[1], "auto") == 0) {
        run_auto();
    }
    else if (strcmp(argv[1], "oscillation") == 0) {
        run_oscillation();
    }
    else if (strcmp(argv[1], "fixed") == 0) {
        run_fixed();
    }
    else if (strcmp(argv[1], "nodata") == 0) {
        run_nodata();
    }
    else if (strcmp(argv[1], "tcpfail-bug") == 0) {
        run_tcpfail(1);     /* BUG 재현: continue로 HB 스킵 */
    }
    else if (strcmp(argv[1], "tcpfail-fix") == 0) {
        run_tcpfail(0);     /* 수정 검증: HB 항상 수신 */
    }
    else {
        usage(argv[0]);
        return 1;
    }

    return 0;
}
