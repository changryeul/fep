# Design: PA-HA 시세수신 이중화

## Feature ID
PA

## Plan 참조
`docs/01-plan/features/PA.plan.md`

---

## 1. 설계 개요

pa_7100_ur.c에 PB-HA와 동일한 heartbeat 기반 Active/Standby 이중화를 구현한다.

**핵심 차이**: PB는 TCP 전달을 HA로 제어하지만, PA는 TCP가 없으므로 **SHM 기록 + FIFO 전달**을 HA로 제어한다. UDP 수신은 양쪽 모두 항상 수행한다.

---

## 2. 수정 대상 파일

```
st01/src/PA/pa_7100_ur.c    ← HA 이중화 구현 (전체)
st01/env/pkg_env.sh         ← _PA_HA_PEER_IP, _PA_HA_HB_PORT 추가 (선택)
```

---

## 3. 추가할 코드 설계

### 3.1 헤더 및 상수 (FR-01)

`#include "buf_struct.h"` 뒤에 추가:

```c
#include    <net/if.h>
#include    <ifaddrs.h>

/*------------------------------------------------------------------------
 *  HA(이중화) 관련 상수
 *------------------------------------------------------------------------*/
#if defined(A7102)
#define     HB_PORT_DEFAULT 50010   /* pa_7102_ur */
#elif defined(A7103)
#define     HB_PORT_DEFAULT 50011   /* pa_7103_ur */
#elif defined(A7201)
#define     HB_PORT_DEFAULT 50012   /* pa_7201_ur */
#elif defined(A7202)
#define     HB_PORT_DEFAULT 50013   /* pa_7202_ur */
#elif defined(A7203)
#define     HB_PORT_DEFAULT 50014   /* pa_7203_ur */
#else
#define     HB_PORT_DEFAULT 50010
#endif

#define     HB_SEND_INTVL   1       /* HB 전송 간격 (초) */
#define     HB_TIMEOUT      5       /* 절체 판단 타임아웃 (초) */
#define     HB_STABLE_COUNT 3       /* Failback 연속 수신 횟수 */
#define     HB_DATA_TIMEOUT 5       /* 시세 끊김 연속 횟수 → 절체 */

typedef struct {
    char    hb_mark[2];     /* "HB" */
    int     recv_cnt;       /* 직전 HB 이후 시세 수신 건수 */
} HB_PACKET;

#define     HB_PKT_LEN     sizeof(HB_PACKET)

#define     HA_PRIMARY      1
#define     HA_SECONDARY    2
#define     HA_STANDALONE   0
```

### 3.2 전역 변수 (FR-01)

기존 전역 변수 선언부 뒤에 추가:

```c
/*------------------------------------------------------------------------
 *  HA(이중화) 전역 변수
 *------------------------------------------------------------------------*/
int     ha_role      = HA_STANDALONE;
int     ha_active    = ON;              /* 기본: 단독 운영 = 항상 활성 */
int     hb_port      = HB_PORT_DEFAULT;
int     hb_sockfd    = -1;
time_t  hb_last_recv = 0;
time_t  hb_last_send = 0;
struct sockaddr_in hb_peer_addr;
char    ha_peer_ip[20];
int     hb_stable_cnt   = 0;
int     my_recv_cnt     = 0;
int     pri_recv_cnt    = 0;
int     pri_no_data_cnt = 0;
```

### 3.3 함수 프로토타입

기존 프로토타입 뒤에 추가:

```c
/* HA(이중화) 함수 */
int     HA_Init(void);
void    HA_Heartbeat_Send(void);
void    HA_Check_Failover(void);
```

---

## 4. HA 함수 설계

### 4.1 HA_Init() (FR-02, FR-06)

PB의 HA_Init()과 동일 구조. 차이점:

| PB | PA |
|----|----|
| `_HA_PEER_IP` | `_HA_PEER_IP` (동일 환경변수 재사용) |
| `_HA_HB_PORT` | `_PA_HA_HB_PORT` (PA 전용, 미설정 시 컴파일 기본값) |
| `B7102`/`B7103` offset | `A7102`~`A7203` offset (0~4) |
| Primary: `ha_active=ON` → TCP 전송 | Primary: `ha_active=ON` → SHM/FIFO 기록 |

```c
int HA_Init(void) {
    int rt, val;
    char *peer_ip;

    /* 1. _FEP_DIV로 역할 결정 */
    if (memcmp(_FEP_DIV, "REAL1", 5) == 0) {
        ha_role   = HA_PRIMARY;
        ha_active = ON;
        SLog(USR_OK, "HA role: PRIMARY (always active)");
    }
    else if (memcmp(_FEP_DIV, "REAL2", 5) == 0) {
        ha_role   = HA_SECONDARY;
        ha_active = OFF;
        SLog(USR_OK, "HA role: SECONDARY (standby)");
    }
    else {
        ha_role   = HA_STANDALONE;
        ha_active = ON;
        SLog(USR_OK, "HA role: STANDALONE (no HA)");
        return (OK);
    }

    /* 2. peer IP */
    peer_ip = (char *)getenv("_HA_PEER_IP");
    if (peer_ip == NULL || peer_ip[0] == '\0') {
        SLog(UDP_ERROR, "HA _HA_PEER_IP not set");
        return (NOTOK);
    }
    strncpy(ha_peer_ip, peer_ip, sizeof(ha_peer_ip) - 1);
    ha_peer_ip[sizeof(ha_peer_ip) - 1] = '\0';

    /* 3. HB 포트 (PA 전용 환경변수) */
    {
        char *port_str = (char *)getenv("_PA_HA_HB_PORT");
        if (port_str != NULL && port_str[0] != '\0') {
            int base_port = atoi(port_str);
            if (base_port > 0 && base_port <= 65535) {
#if defined(A7102)
                hb_port = base_port;        /* +0 */
#elif defined(A7103)
                hb_port = base_port + 1;    /* +1 */
#elif defined(A7201)
                hb_port = base_port + 2;    /* +2 */
#elif defined(A7202)
                hb_port = base_port + 3;    /* +3 */
#elif defined(A7203)
                hb_port = base_port + 4;    /* +4 */
#else
                hb_port = base_port;
#endif
            }
        }
    }

    /* 4. UDP 소켓 생성 */
    hb_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (hb_sockfd < 0) {
        SLog(UDP_ERROR, "HA HB socket fail (%d:%s)", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    val = 1;
    setsockopt(hb_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(val));

    /* 5. peer 주소 설정 */
    memset(&hb_peer_addr, 0, sizeof(hb_peer_addr));
    hb_peer_addr.sin_family = AF_INET;
    hb_peer_addr.sin_port   = htons(hb_port);

    if (ha_role == HA_PRIMARY) {
        inet_pton(AF_INET, ha_peer_ip, &hb_peer_addr.sin_addr);
    }
    else {
        /* Secondary: bind for receiving */
        struct sockaddr_in bind_addr;
        memset(&bind_addr, 0, sizeof(bind_addr));
        bind_addr.sin_family      = AF_INET;
        bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        bind_addr.sin_port        = htons(hb_port);

        if (bind(hb_sockfd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
            SLog(UDP_ERROR, "HA HB bind fail (%d:%s)", SYS_NO, SYS_STR);
            close(hb_sockfd);
            hb_sockfd = -1;
            return (NOTOK);
        }
        hb_last_recv = time(NULL);
    }

    SLog(USR_OK, "HA init OK: role=%d active=%d hb_fd=%d hb_port=%d peer=%s",
        ha_role, ha_active, hb_sockfd, hb_port, ha_peer_ip);

    return (OK);
}
```

### 4.2 HA_Heartbeat_Send() (FR-03)

PB와 **동일**. TCP 관련 코드 없음.

```c
void HA_Heartbeat_Send(void) {
    time_t now;
    int    rt;
    HB_PACKET pkt;

    now = time(NULL);
    if ((now - hb_last_send) < HB_SEND_INTVL)
        return;

    memcpy(pkt.hb_mark, "HB", 2);
    pkt.recv_cnt = my_recv_cnt;
    my_recv_cnt = 0;

    rt = sendto(hb_sockfd, (char *)&pkt, HB_PKT_LEN, 0,
                (struct sockaddr *)&hb_peer_addr, sizeof(hb_peer_addr));

    if (rt < 0)
        SLog(UDP_WARN, "HA HB send fail (%d:%s)", SYS_NO, SYS_STR);
    else
        SLog(USR_OK, "HA HB sent (rt=%d, cnt=%d, to=%s:%d)",
            rt, pkt.recv_cnt, ha_peer_ip, hb_port);

    hb_last_send = now;
}
```

### 4.3 HA_Check_Failover() (FR-04)

PB와 **동일** 로직. `ha_active`가 SHM/FIFO를 제어 (PB에서는 TCP를 제어).

```c
void HA_Check_Failover(void) {
    time_t now;
    int    elapsed, my_data;

    now     = time(NULL);
    elapsed = (int)(now - hb_last_recv);
    my_data = my_recv_cnt;
    my_recv_cnt = 0;

    if (ha_active == OFF) {
        /* FAILOVER 1: HB timeout */
        if (elapsed >= HB_TIMEOUT) {
            ha_active = ON;
            hb_stable_cnt = 0;
            pri_no_data_cnt = 0;
            SLog(USR_OK, "HA FAILOVER: HB timeout (%dsec)", elapsed);
            return;
        }
        /* FAILOVER 2: Primary 시세 끊김 */
        if (pri_recv_cnt == 0 && my_data > 0) {
            pri_no_data_cnt++;
            if (pri_no_data_cnt >= HB_DATA_TIMEOUT) {
                ha_active = ON;
                hb_stable_cnt = 0;
                pri_no_data_cnt = 0;
                SLog(USR_OK, "HA FAILOVER: Primary no data (%d consecutive)",
                    HB_DATA_TIMEOUT);
            }
        } else {
            pri_no_data_cnt = 0;
        }
    }
    else {
        /* FAILBACK */
        if (elapsed >= HB_TIMEOUT) {
            hb_stable_cnt = 0;
        }
        else if (hb_stable_cnt >= HB_STABLE_COUNT) {
            if (pri_recv_cnt > 0 || my_data == 0) {
                ha_active = OFF;
                hb_stable_cnt = 0;
                pri_no_data_cnt = 0;
                SLog(USR_OK, "HA FAILBACK: Primary stable (%d HB, pri=%d)",
                    HB_STABLE_COUNT, pri_recv_cnt);
            } else {
                hb_stable_cnt = 0;
                SLog(USR_OK, "HA FAILBACK blocked: Primary no data (pri=%d, my=%d)",
                    pri_recv_cnt, my_data);
            }
        }
    }
}
```

---

## 5. 메인루프 변경 설계 (FR-05)

### 5.1 Recv_Data 변경

현재 PA의 `Recv_Data()`는 Sockfd만 select 감시 (70초 타임아웃).
PB와 동일하게 **hb_sockfd 추가 감시 + 타임아웃 1초**로 변경.

```c
int Recv_Data(char *p_str) {
    int rt, len, maxfd;
    fd_set read_set;
    struct timeval timeout;

    len = sizeof(ClntAddr);
    FD_ZERO(&read_set);
    FD_SET(Sockfd, &read_set);
    maxfd = Sockfd;

    /* HB 소켓 추가 감시 (Secondary만) */
    if (hb_sockfd >= 0 && ha_role == HA_SECONDARY) {
        FD_SET(hb_sockfd, &read_set);
        if (hb_sockfd > maxfd)
            maxfd = hb_sockfd;
    }

    /* 타임아웃 1초 (기존 70초에서 변경) */
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    rt = select(maxfd + 1, &read_set, NULL, NULL, &timeout);
    if (rt < 0) {
        SLog(SYS_FATAL, "select fail {%d:%s}", SYS_NO, SYS_STR);
        return (NOTOK);
    }
    if (rt == 0) return 0;  /* 1초 타임아웃 */

    /* HB 수신 처리 (Secondary) */
    if (hb_sockfd >= 0 && ha_role == HA_SECONDARY &&
        FD_ISSET(hb_sockfd, &read_set)) {
        char hb_buf[32];
        struct sockaddr_in from_addr;
        socklen_t from_len;

        while (1) {
            fd_set hb_set;
            struct timeval tv_zero;

            from_len = sizeof(from_addr);
            rt = recvfrom(hb_sockfd, hb_buf, sizeof(hb_buf), 0,
                          (struct sockaddr *)&from_addr, &from_len);
            if (rt < 0) {
                SLog(UDP_WARN, "HA HB recvfrom fail(%d:%s)", SYS_NO, SYS_STR);
                break;
            }
            if (rt >= (int)HB_PKT_LEN && memcmp(hb_buf, "HB", 2) == 0) {
                HB_PACKET *p = (HB_PACKET *)hb_buf;
                pri_recv_cnt = p->recv_cnt;
                hb_last_recv = time(NULL);
                hb_stable_cnt++;
                SLog(USR_OK, "HA HB recv ok (pri_cnt=%d, stable=%d)",
                    pri_recv_cnt, hb_stable_cnt);
            }
            else if (rt >= 2 && memcmp(hb_buf, "HB", 2) == 0) {
                pri_recv_cnt = -1;
                hb_last_recv = time(NULL);
                hb_stable_cnt++;
            }

            /* 추가 패킷 확인 */
            FD_ZERO(&hb_set);
            FD_SET(hb_sockfd, &hb_set);
            tv_zero.tv_sec = 0;
            tv_zero.tv_usec = 0;
            if (select(hb_sockfd + 1, &hb_set, NULL, NULL, &tv_zero) <= 0)
                break;
        }
    }

    /* 시세 데이터 수신 */
    if (FD_ISSET(Sockfd, &read_set)) {
#if defined __linux
        rt = recvfrom(Sockfd, p_str, READ_BUF_SIZE, 0,
                (struct sockaddr *)&ClntAddr, (socklen_t *)&len);
#else
        rt = recvfrom(Sockfd, p_str, READ_BUF_SIZE, NULL,
                (struct sockaddr *)&ClntAddr, (socklen_t *)&len);
#endif
        return (rt);
    }

    return (0);  /* HB만 도착, 시세 없음 */
}
```

### 5.2 메인루프 PA_7100_UR() 변경

```c
void PA_7100_UR(void) {
    /* ... 기존 변수 선언 동일 ... */

    rt = Init_Parameters();
    if (rt == NOTOK) return;

    rt = Socket_Connect();
    if (rt == NOTOK) return;

    SLog(USR_OK, "socket connected:port[%d]", SVR_PORT_NO);

    /* ★ HA 초기화 추가 */
    rt = HA_Init();
    if (rt == NOTOK)
        SLog(USR_OK, "HA init failed, running as STANDALONE");

    while (START_S != JOB_END) {

        /* ★ HA heartbeat 처리 */
        if (ha_role == HA_PRIMARY)
            HA_Heartbeat_Send();
        else if (ha_role == HA_SECONDARY)
            HA_Check_Failover();

        memset(r_buf, 0, sizeof(r_buf));

        rt = Recv_Data(r_buf);

        if (rt == 0) {
            continue;           /* 1초 타임아웃 → 루프 재진입 (HB 처리) */
        }
        else if (rt < 0) {
            SLog(UDP_ERROR, "receive fail {%d:%s}", SYS_NO, SYS_STR);
            close(Sockfd);
            rt = Socket_Connect();
            if (rt == NOTOK) return;
            continue;
        }

        my_recv_cnt++;  /* ★ 시세 수신 건수 (HB 전송 시 리셋) */

        /* ★ Standby → SHM 기록 + FIFO 전달 skip */
        if (ha_active == OFF) {
            continue;
        }

        /* === 이하 기존 코드 동일 (TrCode 추출, Set_Sise, Write_Read_Fifo, F_W) === */
        memset(TrCode, 0, sizeof(TrCode));
        sprintf(TrCode, "%-5.5s", r_buf);

        Che_Gbn = Idx = 0;
        Set_Sise(r_buf);

        if (Che_Gbn > 0) {
#if defined(A7102)
            if (Shm_Risk[0].S_Sise[0][Idx].auto_use > 0)
                Write_Read_Fifo(1);
#elif defined(A7202)
            if (Shm_Risk[0].S_Sise[1][Idx].auto_use > 0)
                Write_Read_Fifo(1);
#endif
        }
        else if (Che_Gbn < 0) {
            continue;
        }

        /* FIFO 기록 → DD 프로세스 전달 */
        memset(&W_Fmt, 0x20, sizeof(FILE_BUFF_FORMAT));
        /* ... 기존 W_Fmt 필드 설정 동일 ... */

        rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);
        if (rt != 1) {
            SLog(SAM_FATAL, "file write fail[%s] rt[%d]", OFN(D_K,P_K,0), rt);
            close(Sockfd);
            return;
        }

        Add_Count_DD(TrCode);
    }

    /* ★ 종료 시 HB 소켓 닫기 */
    if (hb_sockfd >= 0) {
        close(hb_sockfd);
        hb_sockfd = -1;
    }

    return;
}
```

### 5.3 메인루프 변경 요약

| 위치 | 변경 | 이유 |
|------|------|------|
| Socket_Connect 후 | `HA_Init()` 호출 추가 | HA 초기화 |
| while 루프 시작 | `HA_Heartbeat_Send()`/`HA_Check_Failover()` 추가 | HB 처리 |
| Recv_Data 후, Set_Sise 전 | `my_recv_cnt++` + `if (ha_active == OFF) continue;` | Standby 시 SHM/FIFO skip |
| 루프 종료 후 | `close(hb_sockfd)` | HB 소켓 정리 |
| `Recv_Data()` | select 타임아웃 70초→1초, hb_sockfd 추가 감시 | HB 수신 통합 |

---

## 6. PB-HA 대비 제거 항목

PA는 TCP가 없으므로 다음 PB 코드는 **적용하지 않음**:

| PB 코드 | PA 미적용 이유 |
|---------|---------------|
| `Device_Open()` / `Device_Close()` / `Device_Close_HA()` | TCP 없음 |
| `SockTcpfd`, `TCP2_PORT_NO` | TCP 없음 |
| `tcp_last_try`, `TCP_RETRY_INTVL` | TCP 재연결 없음 |
| `Connect2()` 사용 | TCP 없음 |
| `if (ha_active == OFF \|\| SockTcpfd < 0) continue;` | TCP 전송 조건 불필요 |
| `Select_Send()`, `Device_Write()` | TCP 전송 없음 |

대신 PA에서는:
```c
if (ha_active == OFF) continue;  /* SHM 기록 + FIFO 전달 skip */
```

---

## 7. 타이밍 분석

### 7.1 야간 (시세 없음)

```
루프 1: HB전송 → Recv_Data(1초 timeout) → continue  ~1초
루프 2: HB전송 → Recv_Data(1초 timeout) → continue  ~1초
...
→ HB 1초 간격 정상 전송, TCP 블로킹 이슈 없음 (PA는 TCP 없음)
```

### 7.2 주간 (시세 수신 중)

```
루프 N: HB전송 → Recv_Data(즉시 리턴) → Set_Sise → F_W  ~수ms
→ 시세 처리량에 영향 없음
```

### 7.3 절체/복귀 시나리오

```
[정상]
  Primary: SHM 기록 + FIFO + HB 전송
  Secondary: UDP 수신만 (SHM 미기록, FIFO 미전달)

[Primary 장애] (T+5초)
  Secondary: FAILOVER → SHM 기록 + FIFO 전달 시작

[Primary 복구] (T+3초)
  Secondary: HB 3회 연속 → FAILBACK → SHM/FIFO 중단
```

---

## 8. 환경변수

### 8.1 pkg_env.sh 추가 항목

```sh
# PA HA 설정 (PB와 별도 포트 대역)
export _PA_HA_HB_PORT=50010     # PA HB base port (PB: 50000)
# _HA_PEER_IP는 PB와 공유 (이미 설정됨)
```

### 8.2 포트 매핑 전체표

| 프로세스 | Port | 환경변수 |
|----------|------|----------|
| pb_7102_ur | 50000 | _HA_HB_PORT=50000 |
| pb_7103_ur | 50001 | _HA_HB_PORT=50000 |
| pa_7102_ur | 50010 | _PA_HA_HB_PORT=50010 |
| pa_7103_ur | 50011 | _PA_HA_HB_PORT=50010 |
| pa_7201_ur | 50012 | _PA_HA_HB_PORT=50010 |
| pa_7202_ur | 50013 | _PA_HA_HB_PORT=50010 |
| pa_7203_ur | 50014 | _PA_HA_HB_PORT=50010 |

---

## 9. 빌드

```sh
mk.sh pa ur     # 5종 바이너리 빌드:
                # pa_7102_ur (-DA7102)
                # pa_7103_ur (-DA7103)
                # pa_7201_ur (-DA7201)
                # pa_7202_ur (-DA7202)
                # pa_7203_ur (-DA7203)
```

---

## 10. 검증 항목

| 항목 | 방법 |
|------|------|
| HA 초기화 | 로그 "HA init OK" 확인 |
| HB 전송 | Primary 로그 "HA HB sent" 1초 간격 |
| HB 수신 | Secondary 로그 "HA HB recv ok" |
| Standby 동작 | Secondary에서 SHM 미기록 확인 (DD 카운트 0) |
| FAILOVER | Primary kill → 5초 후 Secondary "HA FAILOVER" |
| FAILBACK | Primary 재기동 → 3초 후 "HA FAILBACK" |
| 포트 분리 | `netstat -ulnp` 으로 50010~50014 확인 |

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-03-11 | Initial design — PB-HA 복제 + PA 맞춤 설계 (TCP 제거, SHM/FIFO 제어) | Claude Code |
