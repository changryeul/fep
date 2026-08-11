# Design: PB-HA Heartbeat 수신 불가 긴급 버그 수정

## Feature ID
pb-ha-hotfix

## Plan 참조
`docs/01-plan/features/pb-ha-hotfix.plan.md`

---

## 1. 문제 분석

### 1.1 현상

실 서버(podm11/podm12) 운영 중 발견된 증상:

| 상황 | Primary (REAL1) | Secondary (REAL2) |
|------|----------------|-------------------|
| 시세 있을 때 | 시세 수신 + TCP 전달 | **시세 수신 + TCP 전달 (비정상: 양쪽 ACTIVE)** |
| 시세 없을 때 | HB 전송 중 | **서버에 HB 도착하나 프로그램이 수신 못함** |

### 1.2 근본 원인 분석 (Root Cause Chain)

```
[시작] 시스템 기동 또는 야간 운용

① Primary: Device_Open() → Connect() 블로킹 (~75초)
   ↳ TCP 서버 미응답 시 connect() 시스템콜이 OS 기본 타임아웃까지 차단
   ↳ 메인루프 전체 정지 → HB 전송 간격 ~75초로 늘어남

② Secondary: HB_TIMEOUT(3초) 초과 → FAILOVER (ha_active=ON)
   ↳ 정상적 절체 — Primary가 죽은 것으로 판단

③ Secondary(ACTIVE): Device_Open() → Connect 실패 → SockTcpfd=-1
   ↳ if (SockTcpfd < 0) continue;  ← ★ 핵심 버그 ★
   ↳ Recv_Data() 스킵 → heartbeat recvfrom() 호출 안 됨
   ↳ HB가 소켓에 쌓이지만 프로그램이 읽지 않음

④ Primary: Connect 성공 (TCP 서버 가동) → HB 정상 전송 재개
   ↳ 그러나 Secondary는 ③에서 HB를 못 읽으므로 Failback 불가
   ↳ 양쪽 모두 ACTIVE → 시세 이중 전달

⑤ 야간: TCP 서버 미가동 → ③ 반복 → 영구 고착
```

### 1.3 버그 목록

| ID | 분류 | 내용 | 심각도 | 영향 |
|----|------|------|--------|------|
| BUG-01 | Socket | SO_RCVBUF 미설정 (SO_REUSEADDR로 잘못 지정) | High | 시세 폭주 시 UDP 버퍼 오버플로우 |
| BUG-02 | HB수신 | recvfrom 에러 무시 + 크기 조건 gap (rt==2) | Medium | HB 패킷 누락 가능 |
| BUG-03 | 포트 | pb_7102/7103 HB 포트 동일 + 환경변수 덮어쓰기 | Medium | 포트 충돌 시 한쪽 HB 수신 불가 |
| BUG-04 | 블로킹 | Device_Open: Connect() 타임아웃 없음 (~75초) | **Critical** | 메인루프 블로킹 → HB 전송 지연 |
| BUG-05 | 블로킹 | Device_Close: sleep(3) | High | 메인루프 3초 차단 → HB 전송/수신 차단 |
| **BUG-06** | **메인루프** | **TCP실패 시 `continue`가 Recv_Data 스킵** | **Critical** | **HB 수신 완전 차단 → Failback 불가 → 양쪽 ACTIVE 고착** |
| BUG-07 | 타이밍 | HB_TIMEOUT(3초)이 Connect2(2초)+select(1초)와 동일 | High | 경계선 오판 절체 가능 |

---

## 2. 설계

### 2.1 수정 대상 파일

```
st01/src/PB/pb_7100_ur.c    ← 7개 버그 수정 (완료)
st01/src/PA/pa_7100_ur.c    ← SO_RCVBUF 동일 수정 (완료)
st01/test/ha_test.c         ← 테스트 프로그램 동기화 (완료)
st01/env/pkg_env.sh         ← 수정 불필요 (_HA_HB_PORT 기존 유지)
```

### 2.2 메인루프 흐름 변경 (BUG-06 핵심 수정)

#### 기존 코드 (버그)

```c
while (START_S != JOB_END) {
    HA_Heartbeat_Send();          /* or HA_Check_Failover() */

    if (ha_active == ON) {
        if (SockTcpfd < 0) Device_Open();
        if (SockTcpfd < 0) continue;   /* ← Recv_Data 스킵! */
    } else {
        if (SockTcpfd >= 0) Device_Close_HA();
    }

    rt = Recv_Data(rbuf);              /* HB 수신은 여기서 처리 */
    ...
    if (ha_active == OFF) continue;
    /* TCP 전송 */
    Select_Send(SockTcpfd, rbuf, ...);
}
```

#### 수정 코드

```c
while (START_S != JOB_END) {
    HA_Heartbeat_Send();          /* or HA_Check_Failover() */

    if (ha_active == ON) {
        if (SockTcpfd < 0) {
            time_t now = time(NULL);
            if (now - tcp_last_try >= TCP_RETRY_INTVL) {  /* 5초 간격 재시도 */
                Device_Open();          /* Connect2(2초 타임아웃) */
                tcp_last_try = now;
            }
        }
        /* continue 제거: Recv_Data 항상 호출 보장 */
    } else {
        if (SockTcpfd >= 0) Device_Close_HA();
    }

    rt = Recv_Data(rbuf);              /* HB + 시세 항상 수신 */
    ...
    if (ha_active == OFF || SockTcpfd < 0) continue;  /* TCP 전송만 건너뜀 */
    /* TCP 전송 */
    Select_Send(SockTcpfd, rbuf, ...);
}
```

**변경점 요약:**
1. `continue` 제거 → `Recv_Data()` 항상 호출 (HB 수신 보장)
2. `TCP_RETRY_INTVL(5초)` 도입 → 매 루프 Connect 블로킹 방지
3. TCP 전송 조건에 `SockTcpfd < 0` 체크 추가

### 2.3 Device_Open 수정 (BUG-04)

```c
/* 기존: Connect() — 타임아웃 없음, ~75초 블로킹 */
rt = Connect(SockTcpfd, IpAddr, TCP2_PORT_NO);

/* 수정: Connect2() — alarm 기반 2초 타임아웃 */
rt = Connect2(SockTcpfd, IpAddr, TCP2_PORT_NO, 2);
```

`Connect2()`는 `sub/tcpip_connect.c`에 이미 구현되어 있음 (alarm 기반).

### 2.4 Device_Close 수정 (BUG-05)

```c
/* 기존 */
void Device_Close(void) {
    close(SockTcpfd);
    SockTcpfd = -1;
    sleep(3);          /* ← 메인루프 3초 차단 */
    ...
}

/* 수정: sleep(3) 제거 */
void Device_Close(void) {
    close(SockTcpfd);
    SockTcpfd = -1;
    /* sleep 제거 — select 1초가 재연결 간격 역할 */
    ...
}
```

### 2.5 Socket_Connect SO_RCVBUF 수정 (BUG-01)

```c
/* 기존: SO_REUSEADDR에 SO_RCVBUF 값을 설정 (잘못됨) */
val = UDP_SOCK_RCVBUF_SIZE;
setsockopt(Sockfd, SOL_SOCKET, SO_REUSEADDR, &val, len);

/* 수정: 분리 호출 */
val = 1;
setsockopt(Sockfd, SOL_SOCKET, SO_REUSEADDR, &val, len);  /* 포트 재사용 */

val = UDP_SOCK_RCVBUF_SIZE;  /* 1,228,800 bytes */
setsockopt(Sockfd, SOL_SOCKET, SO_RCVBUF, &val, len);     /* 수신 버퍼 */
```

### 2.6 HB 포트 프로세스별 분리 (BUG-03)

```c
/* 컴파일 타임 기본값 */
#if defined B7102
#define HB_PORT_DEFAULT 50000   /* pb_7102_ur */
#elif defined B7103
#define HB_PORT_DEFAULT 50001   /* pb_7103_ur */
#endif

/* 환경변수 처리: base port + 프로세스 offset */
char *port_str = getenv("_HA_HB_PORT");
if (port_str != NULL) {
    int base_port = atoi(port_str);
#if defined B7102
    hb_port = base_port;        /* +0 */
#elif defined B7103
    hb_port = base_port + 1;    /* +1 */
#endif
}
```

`_HA_HB_PORT=50000` 설정 시: pb_7102_ur→50000, pb_7103_ur→50001 자동 분리.

### 2.7 HB recvfrom 에러 처리 (BUG-02)

```c
/* 기존: 에러 무시, rt==2 고정 비교 */
/* 수정: 에러 체크 + rt>=2 범위 확장 */
rt = recvfrom(hb_sockfd, hb_buf, sizeof(hb_buf), 0, ...);
if (rt < 0) {
    Log(UDP_WARN, "HA HB recvfrom fail(%d:%s)", SYS_NO, SYS_STR);
    break;
}
if (rt >= (int)HB_PKT_LEN && memcmp(hb_buf, "HB", 2) == 0) {
    /* 정규 HB 패킷 */
} else if (rt >= 2 && memcmp(hb_buf, "HB", 2) == 0) {
    /* 호환 HB (구 버전/크기 불일치) */
}
```

### 2.8 HB 타이밍 설정 변경 (BUG-07)

| 항목 | 기존 | 수정 | 이유 |
|------|------|------|------|
| HB_SEND_INTVL | 1초 | 1초 | 유지 |
| HB_TIMEOUT | 3초 | **5초** | Connect2(2초) 블로킹 여유 확보 |
| HB_STABLE_COUNT | 3회 | 3회 | 유지 |
| TCP_RETRY_INTVL | - | **5초** | 신규: TCP 재연결 간격 |
| Connect2 timeout | - | **2초** | 신규: 연결 타임아웃 |

절체 소요 시간: 최대 5초 (시세 시스템에서 충분히 빠름).

---

## 3. 메인루프 타이밍 분석

### 3.1 야간 (TCP 서버 미가동)

```
                           Connect2
루프 1: HB전송 → (스킵) → Recv_Data(1초) → ~1초
루프 2: HB전송 → (스킵) → Recv_Data(1초) → ~1초
루프 3: HB전송 → (스킵) → Recv_Data(1초) → ~1초
루프 4: HB전송 → (스킵) → Recv_Data(1초) → ~1초
루프 5: HB전송 → Connect2(2초) → Recv_Data(1초) → ~3초  ← 5초 경과, 재시도
루프 6: HB전송 → (스킵) → Recv_Data(1초) → ~1초
...

HB 전송 간격: 대부분 ~1초, 5초에 1번 ~3초
HB_TIMEOUT(5초) 이내: 최소 3~4회 HB 전송 보장 → 오판 절체 없음
```

### 3.2 주간 (TCP 서버 가동)

```
루프 N: HB전송 → (TCP 연결됨, 스킵) → Recv_Data(1초) → ~1초
        ↳ 시세 수신 시 즉시 리턴 → 수십ms/회

HB 전송 간격: ~1초 (정상)
```

### 3.3 절체/복귀 시나리오

```
[정상 운영]
  Primary: HB 1초 간격 전송
  Secondary: STANDBY, HB 수신 + 시세 수신 (TCP 전송 안 함)

[Primary 장애]
  T+0  마지막 HB
  T+5  Secondary FAILOVER → ACTIVE, TCP 전송 시작

[Primary 복구]
  T+0  HB 전송 재개
  T+3  Secondary: HB 3회 연속 수신 → FAILBACK → STANDBY

[야간 TCP 미가동]
  Primary: HB 정상 전송 (TCP_RETRY_INTVL=5초 간격으로만 Connect 시도)
  Secondary: STANDBY 유지 (HB 정상 수신)
  → 양쪽 ACTIVE 고착 현상 없음
```

---

## 4. PA 모듈 동일 수정

`pa_7100_ur.c`의 `Socket_Connect()` 함수에 동일한 SO_RCVBUF 버그 존재.

```c
/* 기존 (pa_7100_ur.c 라인 293) */
val = UDP_SOCK_RCVBUF_SIZE;
setsockopt(Sockfd, SOL_SOCKET, SO_REUSEADDR, &val, len);

/* 수정: SO_REUSEADDR + SO_RCVBUF 분리 */
val = 1;
setsockopt(Sockfd, SOL_SOCKET, SO_REUSEADDR, &val, len);
val = UDP_SOCK_RCVBUF_SIZE;
setsockopt(Sockfd, SOL_SOCKET, SO_RCVBUF, &val, len);
```

PA 모듈은 HA 기능이 없으므로 BUG-04~07은 해당 없음. BUG-01만 수정.

---

## 5. 테스트 설계

### 5.1 ha_test.c 테스트 모드 (8종)

| 모드 | 명령어 | 검증 내용 |
|------|--------|-----------|
| primary | `./ha_test primary` | HB 전송 동작 확인 |
| secondary | `./ha_test secondary` | HB 수신 + 절체/복귀 판단 |
| auto | `./ha_test auto` | Failover→Failback 자동 시나리오 |
| oscillation | `./ha_test oscillation` | FR-08 진동 재현 (sleep(3) 버그) |
| fixed | `./ha_test fixed` | FR-08 수정 검증 (진동 없음) |
| nodata | `./ha_test nodata` | 시세 기반 절체 (DATA FAILOVER) |
| **tcpfail-bug** | `./ha_test tcpfail-bug` | **BUG-06 재현: TCP실패→HB스킵→Failback불가** |
| **tcpfail-fix** | `./ha_test tcpfail-fix` | **BUG-06 수정: TCP실패→HB수신→Failback성공** |

### 5.2 테스트 판정 기준

| 테스트 | PASS 조건 |
|--------|-----------|
| auto | Phase2에서 FAILOVER, Phase3에서 FAILBACK 각 1회 |
| tcpfail-bug | Phase3에서 FAILBACK 미발생 (BUG 재현) |
| tcpfail-fix | Phase3에서 FAILBACK 발생 (수정 검증) |
| fixed | FAILOVER→FAILBACK 1회, 이후 진동 없음 |
| nodata | Phase2에서 DATA FAILOVER, Phase3에서 FAILBACK |

### 5.3 서버 테스트 계획

#### Stage 1: macOS 로컬 (완료)

```sh
cd st01/test
gcc -o ha_test ha_test.c -Wall
./ha_test auto          # PASS
./ha_test tcpfail-bug   # PASS (BUG 재현)
./ha_test tcpfail-fix   # PASS (Failback 성공)
```

#### Stage 2: 서버 단독 (mk.sh 빌드)

```sh
mk.sh pb ur             # pb_7102_ur, pb_7103_ur 빌드
mk.sh pa ur             # pa_7102_ur 등 빌드

# 로그 확인
tail -f st03/LOG/PB/pb_7102_ur.log | grep "HA "
```

확인 항목:
- [ ] `HA init OK: role=1 active=1 hb_fd=N hb_port=50000`
- [ ] `HA HB sent (rt=8, cnt=N, to=192.168.151.12:50000)`
- [ ] `setsockopt(SO_RCVBUF)` 로그 (실패 시 WARN)

#### Stage 3: 2대 서버 연동

```sh
# podm11 (Primary)
tail -f st03/LOG/PB/pb_7102_ur.log | grep "HA "
# → "HA HB sent" 1초 간격 확인

# podm12 (Secondary)
tail -f st03/LOG/PB/pb_7102_ur.log | grep "HA "
# → "HA HB recv ok" 1초 간격 확인
```

절체 테스트:
```sh
# podm11에서 pb_7102_ur kill
kill $(pgrep pb_7102_ur)

# podm12 로그: 5초 후 "FAILOVER" 확인
# podm11에서 재기동 후: podm12 로그 "FAILBACK" 확인
```

---

## 6. 수정 요약표

| BUG | 파일 | 수정 내용 | 검증 방법 |
|-----|------|-----------|-----------|
| BUG-01 | pb_7100_ur.c | SO_REUSEADDR/SO_RCVBUF 분리 | `ss -ulnm`으로 rcvbuf 확인 |
| BUG-02 | pb_7100_ur.c | recvfrom 에러체크 + rt>=2 | ha_test auto |
| BUG-03 | pb_7100_ur.c | HB 포트 env+compile 분리 | `netstat -ulnp` 포트 확인 |
| BUG-04 | pb_7100_ur.c | Connect→Connect2(2초) | 야간 HB 로그 간격 확인 |
| BUG-05 | pb_7100_ur.c | Device_Close sleep(3) 제거 | ha_test fixed |
| **BUG-06** | **pb_7100_ur.c** | **continue 제거 + TCP_RETRY_INTVL** | **ha_test tcpfail-fix** |
| BUG-07 | pb_7100_ur.c | HB_TIMEOUT 3→5초 | ha_test auto (5초 후 절체) |
| FR-07 | pa_7100_ur.c | SO_RCVBUF 동일 수정 | mk.sh pa 빌드 확인 |

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-03-11 | Initial design — 7 bugs + 1 FR, 메인루프 흐름 재설계 | Claude Code |
