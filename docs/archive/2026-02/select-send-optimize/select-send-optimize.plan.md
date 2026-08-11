# select-send-optimize Planning Document

> **Summary**: Select_Send()에서 불필요한 select() syscall 제거 — SO_SNDTIMEO 소켓 타임아웃으로 대체. FEP_Architecture_Analysis.md §4.6 성능 개선 항목.
>
> **Project**: FEP (Front-End Processor)
> **Author**: Claude
> **Date**: 2026-02-28
> **Status**: Draft

---

## 1. Overview

### 1.1 Purpose

TCP 전송 함수 `Select_Send()`에서 매 호출마다 수행되는 불필요한 `select()` syscall을 제거하여, 전송당 수십μs의 성능 개선을 달성한다. 기존 200ms 타임아웃 보호는 `SO_SNDTIMEO` 소켓 옵션으로 대체한다.

### 1.2 Background

**아키텍처 분석 §4.6 원문**:
> 소켓 송신 버퍼가 충분하면 `select()`는 거의 항상 즉시 리턴. 불필요한 syscall. 200ms timeout도 장중 주문에 비해 너무 긺.

**현재 코드** (`sub/select_send.c`):
```c
int Select_Send(int p_sfd, char *p_send, int p_length) {
    /* 1. select() — 쓰기 가능 여부 확인 (200ms timeout) */
    rt = select(p_sfd+1, NULL, &write_set, NULL, &timeout);  /* ← 불필요 */
    if (rt < 0) { Log(...); return NOTOK; }
    if (!FD_ISSET(p_sfd, &write_set)) { Log(...); return NOTOK; }

    /* 2. Sendn() — 실제 전송 */
    rt = Sendn(p_sfd, p_send, p_length);
    if (rt <= 0) { Log(...); return NOTOK; }
    return OK;
}
```

**문제 분석**:
- `select()`가 99%+ 즉시 리턴 (송신 버퍼 거의 항상 여유)
- 매 전송마다 **2번의 syscall** (select + send) → **1번**으로 가능
- 200ms timeout이 유일한 안전장치이나, `SO_SNDTIMEO`로 동일 보호 가능

### 1.3 Related Documents

- Architecture: `docs/FEP_Architecture_Analysis.md` §4.6
- 선행 기능: tcp-nodelay (Feature #41, 아카이브 완료) — Connect/Accept에 TCP_NODELAY 추가 패턴 동일
- Sendn(): `sub/tcpip_send.c` — blocking send loop, 부분 전송 처리

---

## 2. Scope

### 2.1 In Scope

- [ ] FR-01: Connect()/Connect2()에 SO_SNDTIMEO 설정 (200ms)
- [ ] FR-02: Accept()에 SO_SNDTIMEO 설정 (accepted fd 대상)
- [ ] FR-03: Select_Send()에서 select() 제거, 직접 Sendn() 호출
- [ ] FR-04: SO_SNDTIMEO 실패 시 경고 로그 (TCP_WARN, 연결 중단 안 함)

### 2.2 Out of Scope

- `select_recv.c` — 수신 대기 select()는 필수 (데이터 도착 감지용)
- `tcpip_select.c` — 연결 상태 확인용 select()로 별도 목적
- Non-blocking 소켓 전환 — 전체 recv/send 동작에 영향, 위험도 높음
- SO_RCVTIMEO — 수신 측은 poll() 타임아웃으로 이미 관리

---

## 3. Requirements

### 3.1 Functional Requirements

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-01 | Connect() 성공 후 SO_SNDTIMEO 200ms 설정 (TCP_NODELAY 패턴 동일) | High | Pending |
| FR-02 | Connect2() 성공 후 SO_SNDTIMEO 200ms 설정 | High | Pending |
| FR-03 | Accept() 성공 후 accepted fd(rt)에 SO_SNDTIMEO 200ms 설정 | High | Pending |
| FR-04 | Select_Send()에서 select()/FD_ISSET 제거, Sendn() 직접 호출 | High | Pending |
| FR-05 | Sendn() 실패 시 ETIMEDOUT 구분 로그 (기존 에러 경로 유지) | Medium | Pending |

### 3.2 Non-Functional Requirements

| Category | Criteria | Measurement Method |
|----------|----------|-------------------|
| 성능 | 전송당 1 syscall 절감 (~수십μs) | strace 카운트 비교 |
| 호환성 | HP-UX, SunOS, AIX, Linux 전부 빌드/동작 | `mk.sh all` on each OS |
| 안전성 | 200ms 전송 타임아웃 유지 (기존 동작 보존) | SO_SNDTIMEO = 200ms |
| C89 준수 | ANSI C89 문법 준수 | 컴파일 경고 0건 |

---

## 4. Technical Design

### 4.1 SO_SNDTIMEO 접근법

**핵심 아이디어**: select()의 200ms 타임아웃 역할을 소켓 자체의 `SO_SNDTIMEO` 옵션으로 이전.

```c
/* SO_SNDTIMEO: 소켓 수준 전송 타임아웃 */
struct timeval snd_timeout;
snd_timeout.tv_sec = 0;
snd_timeout.tv_usec = 200000;  /* 200ms — 기존 Select_Send timeout과 동일 */
if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,
    (char *)&snd_timeout, sizeof(snd_timeout)) < 0)
    Log(TCP_WARN, "SO_SNDTIMEO fail {%d:%s}", SYS_NO, SYS_STR);
```

**동작 비교**:

| 시나리오 | 현재 (select + send) | 개선 (SO_SNDTIMEO + send) |
|---------|---------------------|--------------------------|
| 버퍼 여유 (99%+) | select 즉시 리턴 → send 성공 (2 syscall) | send 즉시 성공 (**1 syscall**) |
| 버퍼 풀 (드묾) | select 200ms 대기 → send 또는 NOTOK | send 200ms 대기 → ETIMEDOUT |
| 연결 끊김 | select 에러 또는 send 에러 | send 에러 (EPIPE/ECONNRESET) |

### 4.2 SO_SNDTIMEO 플랫폼 호환성

| OS | SO_SNDTIMEO | struct timeval | 비고 |
|----|:-----------:|:--------------:|------|
| Linux | O | O | 표준 POSIX |
| HP-UX | O | O | HP-UX 11.0+ |
| SunOS/Solaris | O | O | Solaris 2.6+ |
| AIX | O | O | AIX 4.3+ |

**모든 FEP 타겟 OS에서 지원됨.**

### 4.3 개선된 Select_Send

```c
/* Before: 2 syscalls */
int Select_Send(int p_sfd, char *p_send, int p_length) {
    fd_set write_set;
    struct timeval timeout;
    FD_ZERO(&write_set);
    FD_SET(p_sfd, &write_set);
    timeout.tv_sec = 0;
    timeout.tv_usec = 200000;
    rt = select(p_sfd+1, NULL, &write_set, NULL, &timeout);  /* syscall #1 */
    if (rt < 0) { Log(...); return NOTOK; }
    if (!FD_ISSET(p_sfd, &write_set)) { Log(...); return NOTOK; }
    rt = Sendn(p_sfd, p_send, p_length);                     /* syscall #2 */
    if (rt <= 0) { Log(...); return NOTOK; }
    return OK;
}

/* After: 1 syscall (SO_SNDTIMEO가 타임아웃 담당) */
int Select_Send(int p_sfd, char *p_send, int p_length) {
    int rt;
    rt = Sendn(p_sfd, p_send, p_length);
    if (rt <= 0) {
        Log(TCP_ERROR, "Select_Send:send failure[%d] {%d:%s}",
            rt, SYS_NO, SYS_STR);
        return (NOTOK);
    }
    return (OK);
}
```

### 4.4 Connect/Accept 패턴 (TCP_NODELAY와 동일)

```c
/* tcpip_connect.c — Connect() 내부, TCP_NODELAY 블록 직후 */
if (rt == 0)
{
    int flag = 1;
    if (setsockopt(p_sfd, IPPROTO_TCP, TCP_NODELAY, ...))  /* 기존 */
        Log(TCP_WARN, ...);

    /* ← 여기에 SO_SNDTIMEO 추가 */
    {
        struct timeval snd_timeout;
        snd_timeout.tv_sec = 0;
        snd_timeout.tv_usec = 200000;
        if (setsockopt(p_sfd, SOL_SOCKET, SO_SNDTIMEO,
            (char *)&snd_timeout, sizeof(snd_timeout)) < 0)
            Log(TCP_WARN, "Connect:SO_SNDTIMEO fail {%d:%s}",
                SYS_NO, SYS_STR);
    }
}
```

---

## 5. 파일 목록 및 변경량

| 파일 | 변경 | 줄 수 |
|------|------|-------|
| `sub/tcpip_connect.c` | Connect(), Connect2()에 SO_SNDTIMEO 추가 | +14 |
| `sub/tcpip_accept.c` | Accept() accepted fd에 SO_SNDTIMEO 추가 | +7 |
| `sub/select_send.c` | select()/FD_ISSET 제거, Sendn() 직접 호출 | -15, +5 |
| **합계** | **3개 파일** | **~+11줄 순** |

**호출자 변경: 0건** (Select_Send 시그니처/반환값 동일)

### 5.1 호출자 목록 (22개, 변경 없음)

| Module | Files | Call Sites |
|--------|-------|------------|
| PA (8) | pa_2100_ts, pa_2200_tr, pa_2700_tr, pa_1600_tr, pa_3100_ts, pa_7000_tr, pa_7100_ts, pa_8100_ts, pa_8200_tr | 9 |
| PB (6) | pb_1800_ts, pb_7100_ts, pb_7100_ur(x3), pb_7200_tr, pb_8100_ts, pb_8200_tr | 8 |
| PW (2) | pw_3010_tr, pw_3030_tr, pw_4000_ts | 3 |
| sub (1) | fep_common.c | 1 |
| **합계** | **17개 파일** | **21 호출** |

---

## 6. Success Criteria

### 6.1 Definition of Done

- [ ] SO_SNDTIMEO 3곳 설정 (Connect, Connect2, Accept)
- [ ] Select_Send에서 select() 제거
- [ ] 기존 에러 처리 경로 보존
- [ ] 호출자 0건 변경

### 6.2 Quality Criteria

- [ ] `mk.sh sub` 빌드 성공
- [ ] Select_Send 시그니처 동일 (`int Select_Send(int, char*, int)`)
- [ ] SO_SNDTIMEO 실패 시 Log(TCP_WARN)만, 연결 중단 없음

---

## 7. Risks and Mitigation

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| SO_SNDTIMEO와 Sendn() 부분전송 상호작용 | Medium | Low | Sendn()의 while(nleft>0) 루프가 부분 전송 처리. SO_SNDTIMEO는 개별 send()에만 적용 |
| ETIMEDOUT 에러 코드 차이 | Low | Low | Sendn()이 nsend<=0 리턴 → Select_Send이 NOTOK 리턴. 기존과 동일 에러 경로 |
| 특정 OS에서 SO_SNDTIMEO 동작 미묘한 차이 | Low | Very Low | TCP_NODELAY와 동일 패턴: 실패 시 WARN만, 기존 동작(무제한 blocking) 유지 |

---

## 8. 성능 영향 분석

### 8.1 직접 영향

| 항목 | 현재 | 개선 후 | 절감 |
|------|------|---------|------|
| 전송당 syscall | 2 (select + send) | 1 (send) | **50%** |
| 전송당 시간 (버퍼 여유) | ~2μs | ~1μs | ~1μs/건 |
| 시세 전송 (초당 수천건) | ~2ms/천건 | ~1ms/천건 | ~1ms/천건 |

### 8.2 간접 영향

- 주문 전송 지연(latency) 미미하게 감소 (이미 TCP_NODELAY 적용)
- 시세 분배 처리량 소폭 증가 (pb_7100_ur 3곳 호출)
- fd_set 초기화/FD_SET/FD_ISSET 연산 제거 (CPU 미세 절감)

### 8.3 호출 빈도별 효과

| 프로세스 유형 | 호출 빈도 | 절감 효과 |
|-------------|----------|----------|
| 시세 분배 (pb_7100_ur) | 초당 수백~수천 | 가장 큰 절감 |
| 주문 전송 (pa_*_ts, pb_*_ts) | 초당 수~수십 | 미미하지만 latency 개선 |
| 응답/체결 (pa_*_tr, pb_*_tr) | 이벤트 기반 | 거의 무시 가능 |

---

## 9. Next Steps

1. [ ] Design 문서 작성 (`/pdca design select-send-optimize`)
2. [ ] 구현 (3개 파일)
3. [ ] Gap 분석
4. [ ] 완료 보고서

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-28 | Initial draft — SO_SNDTIMEO 접근법. 3개 파일, 22 호출자 변경 없음 | Claude |
