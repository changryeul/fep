# tcp-nodelay Planning Document

> **Summary**: TCP 소켓에 TCP_NODELAY 설정으로 Nagle 알고리즘 비활성화, 주문 전송 지연 제거
>
> **Project**: FEP (Front-End Processor)
> **Date**: 2026-02-27
> **Status**: Draft
> **Reference**: FEP_Architecture_Analysis.md §4.1 (성능 개선 우선순위 1위)

---

## 1. Overview

### 1.1 Purpose

FEP의 모든 TCP 소켓에 Nagle 알고리즘이 활성 상태(기본값). 소규모 주문 패킷(~300B)이 커널 버퍼에서 병합 대기하며 **수~수십 ms 지연** 발생 가능. `TCP_NODELAY` 옵션으로 Nagle을 비활성화하여 즉시 전송.

### 1.2 Background

**현재 상태**: TCP_NODELAY가 코드베이스 전체에서 사용되지 않음 (0건).

**현재 설정된 소켓 옵션**:
- `SO_REUSEADDR` — 서버 소켓 6곳 (PA 4, PB 2)
- `SO_LINGER(1,0)` — 클라이언트 연결 3곳 (`fep_common.c`, `pa_2700_tr.c`, `pa_8200_tr.c`)
- UDP: `SO_SNDBUF`, `SO_RCVBUF`, `SO_BROADCAST`, `IP_ADD_MEMBERSHIP`

**TCP 소켓 생성 경로**:
- `sub/tcpip_sock.c:Socket()` — `socket(AF_INET, SOCK_STREAM, 0)` 반환
- `sub/tcpip_connect.c:Connect()` — 클라이언트 연결 (KRX, IMECO 등)
- `sub/tcpip_accept.c:Accept()` — 서버 연결 수락 (클라이언트 접속)

**Socket() 호출 위치** (활성 소스, JC_OLD 제외):

| # | Process | File | Type | Target |
|---|---------|------|------|--------|
| 1 | pb_1800_ts | `src/PB/pb_1800_ts.c:525` | 클라이언트 | KRX 채권 주문 |
| 2 | pb_7100_ts | `src/PB/pb_7100_ts.c:379` | 클라이언트 | KRX 시세 요청 |
| 3 | pb_7100_ur | `src/PB/pb_7100_ur.c:97` | 클라이언트 | TCP 보조 연결 |
| 4 | pb_7200_tr | `src/PB/pb_7200_tr.c:304` | 클라이언트 | KRX 시세 수신 |
| 5 | pb_8100_ts | `src/PB/pb_8100_ts.c:467` | 서버 | 클라이언트 주문 접수 |
| 6 | pb_8200_tr | `src/PB/pb_8200_tr.c:363` | 서버 | 클라이언트 응답 송신 |
| 7 | pa_1600_tr | `src/PA/pa_1600_tr.c:274` | 클라이언트 | 체결 수신 |
| 8 | pa_2100_ts | `src/PA/pa_2100_ts.c:226` | 클라이언트 | 주문 전송 |
| 9 | pa_2200_tr | `src/PA/pa_2200_tr.c:224` | 클라이언트 | 응답 수신 |
| 10 | pa_2700_tr | `src/PA/pa_2700_tr.c:207` | 클라이언트 | IMECO 체결 |
| 11 | pa_3100_ts | `src/PA/pa_3100_ts.c:433` | 클라이언트 | 조회 전송 |
| 12 | pa_7000_tr | `src/PA/pa_7000_tr.c:428` | 서버 | 시세 수신 |
| 13 | pa_7100_ts | `src/PA/pa_7100_ts.c:312` | 클라이언트 | 시세 요청 |
| 14 | pa_8100_ts | `src/PA/pa_8100_ts.c:475` | 서버 | 클라이언트 접수 |
| 15 | pa_8200_tr | `src/PA/pa_8200_tr.c:384` | 서버 | 클라이언트 응답 |

**Nagle 알고리즘의 영향**:
- 주문 전송(ts) 프로세스: 소규모 패킷(~300B)을 전송 → Nagle이 ACK 대기 동안 버퍼링 → **최대 40~200ms 지연**
- 시세 수신(tr/ur) 프로세스: 수신 측은 Nagle 영향 적음 (데이터 수신은 지연 없음), 응답 전송 시에만 영향
- 서버 소켓(8100/8200): Accept 후 클라이언트 통신 시 양방향 모두 Nagle 영향

### 1.3 Related Documents

- FEP_Architecture_Analysis.md §4.1 "TCP_NODELAY 미설정"
- FEP_Architecture_Analysis.md §4.9 성능 개선 우선순위 1위

---

## 2. Scope

### 2.1 In Scope

- [ ] `sub/tcpip_connect.c`의 `Connect()` 함수에 TCP_NODELAY 설정 추가
- [ ] `sub/tcpip_accept.c`의 `Accept()` 함수에 TCP_NODELAY 설정 추가
- [ ] `inc/fep_sub.h`에 `<netinet/tcp.h>` include 추가
- [ ] `Connect2()` 함수에도 동일 적용 (미사용이지만 정합성)

### 2.2 Out of Scope

- 호출자 코드 변경 (15개 Socket() 호출 위치는 그대로)
- UDP 소켓 (TCP_NODELAY는 TCP 전용)
- SO_KEEPALIVE 등 추가 소켓 옵션 (별도 기능)
- 기존 SO_REUSEADDR, SO_LINGER 설정 변경
- Socket() 함수 자체 변경 (socket 생성 시점에는 connected socket이 아님)

---

## 3. Requirements

### 3.1 Functional Requirements

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-01 | `Connect()` 성공 후 연결된 소켓에 TCP_NODELAY 설정 | High | Pending |
| FR-02 | `Accept()` 성공 후 수락된 소켓에 TCP_NODELAY 설정 | High | Pending |
| FR-03 | `<netinet/tcp.h>` include를 `fep_sub.h`에 추가 | Medium | Pending |
| FR-04 | `Connect2()` 성공 후에도 TCP_NODELAY 설정 (정합성) | Low | Pending |
| FR-05 | setsockopt 실패 시 Log 경고만 출력, 연결은 계속 진행 | Medium | Pending |

### 3.2 Non-Functional Requirements

| Category | Criteria | Measurement Method |
|----------|----------|-------------------|
| Performance | 소규모 패킷 전송 지연 제거 (Nagle 비활성화) | tcpdump으로 패킷 전송 시점 확인 |
| Compatibility | 기존 호출자 15곳 코드 변경 없음 | 코드 diff 확인 |
| Safety | setsockopt 실패 시 연결 중단하지 않음 | 에러 경로 확인 |
| Portability | HP-UX, SunOS, AIX, Linux 모두 지원 | TCP_NODELAY는 POSIX 표준 |

---

## 4. Success Criteria

### 4.1 Definition of Done

- [ ] Connect() 내부에 TCP_NODELAY setsockopt 추가
- [ ] Accept() 내부에 TCP_NODELAY setsockopt 추가
- [ ] Connect2() 내부에 TCP_NODELAY setsockopt 추가
- [ ] fep_sub.h에 netinet/tcp.h include 추가
- [ ] setsockopt 실패 시 Log 경고 + 정상 진행
- [ ] 변경 파일: 3개 (tcpip_connect.c, tcpip_accept.c, fep_sub.h)
- [ ] 기존 호출자 코드 변경 0건

### 4.2 Quality Criteria

- [ ] Gap Analysis 일치율 90% 이상
- [ ] 빌드 성공 (mk.sh sub)

---

## 5. Technical Design Preview

### 5.1 접근 방식: 공유 함수 내부에 설정

**핵심 아이디어**: `Connect()`와 `Accept()` 내부에서 연결 성공 직후 `setsockopt(TCP_NODELAY)`를 호출. 모든 TCP 연결이 자동으로 Nagle 비활성화.

```c
/* Connect() 내부 — connect 성공 후 */
rt = connect(p_sfd, ...);
if (rt == 0) {
    int flag = 1;
    if (setsockopt(p_sfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag)) < 0)
        Log(TCP_WARN, "TCP_NODELAY fail {%d:%s}", SYS_NO, SYS_STR);
}
return (rt);
```

```c
/* Accept() 내부 — accept 성공 후 */
rt = accept(p_sfd, ...);
if (rt >= 0) {
    int flag = 1;
    if (setsockopt(rt, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag)) < 0)
        Log(TCP_WARN, "TCP_NODELAY fail {%d:%s}", SYS_NO, SYS_STR);
}
```

### 5.2 왜 Socket()이 아닌 Connect()/Accept()에서 설정하는가

- `Socket()`은 아직 연결되지 않은 소켓을 반환. TCP_NODELAY는 연결된 소켓에 설정하는 것이 의미적으로 정확
- `Connect()` 성공 시점이 "이 소켓으로 데이터를 보낼 준비 완료" 시점
- `Accept()`가 반환하는 fd가 실제 통신용 소켓 (listening 소켓이 아님)
- 참고: 기술적으로는 connect 전에도 TCP_NODELAY 설정 가능하지만, Connect/Accept 내부가 더 명확

### 5.3 Connect2() 처리

`Connect2()`는 현재 `src/`에서 호출되지 않지만 (grep 결과 0건), `tcpip_connect.c`에 정의되어 있으므로 정합성을 위해 동일하게 적용.

---

## 6. Risks and Mitigation

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| TCP_NODELAY로 작은 패킷이 더 많이 전송되어 네트워크 오버헤드 증가 | 낮음 | 낮음 | FEP 패킷은 이미 의미 단위(주문 1건)로 전송. 추가 분할 없음 |
| setsockopt 실패 | 없음 | 매우 낮음 | 경고 로그 + 정상 연결 진행. TCP_NODELAY는 모든 POSIX 플랫폼 지원 |
| netinet/tcp.h 포함 시 기존 심볼 충돌 | 없음 | 매우 낮음 | 표준 POSIX 헤더. 기존 netinet/in.h과 충돌 없음 |

---

## 7. Impact Analysis

### 7.1 변경 파일

| File | Change | Risk |
|------|--------|------|
| `inc/fep_sub.h` | `#include <netinet/tcp.h>` 추가 (1줄) | 낮음 |
| `sub/tcpip_connect.c` | Connect(), Connect2() 내부에 setsockopt 추가 | 낮음 |
| `sub/tcpip_accept.c` | Accept() 내부에 setsockopt 추가 | 낮음 |

### 7.2 영향 범위

- **호출자 변경 없음**: 15개 Socket() 호출 위치, 모든 Connect/Accept 호출자 수정 불필요
- **빌드 영향**: `libfepP.a` 재빌드 → 전체 바이너리 재링크 (`mk.sh all`)
- **런타임 영향**: 모든 TCP 연결에서 Nagle 비활성화

### 7.3 지연 제거 예상치

| 프로세스 유형 | 현재 지연 | 개선 후 | 효과 |
|-------------|----------|---------|------|
| 주문 전송 (ts) | 0~200ms (Nagle) | 즉시 전송 | 주문 응답 시간 개선 |
| 응답 수신 (tr) | 수신은 영향 없음, ACK 전송 시 지연 가능 | 즉시 ACK | 안정성 개선 |
| 시세 전송 (ts) | 0~40ms | 즉시 전송 | 시세 지연 제거 |
| 클라이언트 통신 (8x00) | 0~200ms | 즉시 전송 | 클라이언트 응답 개선 |

---

## 8. Next Steps

1. [ ] Design 문서 작성 (`tcp-nodelay.design.md`)
2. [ ] 구현 (fep_sub.h, tcpip_connect.c, tcpip_accept.c)
3. [ ] Gap Analysis
4. [ ] Completion Report

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-27 | Initial draft | Claude |
