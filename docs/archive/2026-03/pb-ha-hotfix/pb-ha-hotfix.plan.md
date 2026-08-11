# pb-ha-hotfix Planning Document

> **Summary**: PB-HA heartbeat 수신 불가 긴급 버그 수정 (5건)
>
> **Project**: FEP (Front-End Processor)
> **Version**: -
> **Author**: Claude Code
> **Date**: 2026-03-10
> **Status**: Draft

---

## 1. Overview

### 1.1 Purpose

pb_7100_ur.c의 HA(이중화) heartbeat 수신 실패 버그를 수정한다.
실 환경에서 Secondary(REAL2)가 Primary(REAL1)의 heartbeat를 수신하지 못하는 문제가 발견되었으며,
코드 심층 분석을 통해 확정 버그 2건 + 잠재 버그 3건을 식별했다.

### 1.2 Background

- PB-HA 이중화 기능은 2026-03-07에 완료 (Match Rate 100%)
- 실 서버 테스트에서 "서버간 UDP 전달은 되지만 Secondary에서 heartbeat 수신 불가" 증상 발견
- 코드 레벨 분석 결과, SO_RCVBUF 미설정으로 인한 패킷 드롭 → 에러 복구 루프 진입 → HB 처리 skip 경로 확인

### 1.3 Related Documents

- PB-HA 완료 보고서: `docs/04-report/features/PB-HA.report.md`
- PB-HA 아카이브: `docs/archive/2026-03/`
- 대상 소스: `st01/src/PB/pb_7100_ur.c`
- PA 동일 버그: `st01/src/PA/pa_7100_ur.c` (SO_RCVBUF 미설정 동일)

---

## 2. Scope

### 2.1 In Scope

- [x] BUG-01: `SO_RCVBUF` 미설정 (SO_REUSEADDR로 잘못 설정) 수정
- [x] BUG-02: HB `recvfrom` 에러 무시 + 크기 범위 gap 수정
- [x] BUG-03: HB 포트 프로세스별 분리 (환경변수+컴파일 타임)
- [x] BUG-04: `Device_Open()` Connect→Connect2(2초 타임아웃) 변경
- [x] BUG-05: `Device_Close()` sleep(3) 제거
- [x] BUG-06: TCP 실패 시 `continue` 제거 (Recv_Data 스킵 방지) — **핵심 버그**
- [x] BUG-07: `HB_TIMEOUT` 3초→5초 변경 + `TCP_RETRY_INTVL` 5초 도입
- [x] FR-07: PA 모듈 동일 SO_RCVBUF 버그 수정 (pa_7100_ur.c)

### 2.2 Out of Scope

- HA 아키텍처 변경 (heartbeat 프로토콜 자체 변경)
- HB_PACKET 구조체 `#pragma pack(1)` 적용 (동일 플랫폼 확인 후 결정)
- 3-way HA (현재 2-way Active/Standby 유지)

---

## 3. Requirements

### 3.1 Functional Requirements

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-01 | Socket_Connect에서 SO_RCVBUF를 1.2MB로 정확히 설정 | **Critical** | Done |
| FR-02 | HB recvfrom 에러 시 로그 출력 및 루프 탈출 | **Critical** | Done |
| FR-03 | HB 수신 크기 조건 `rt==2` → `rt>=2`로 확장 (gap 해소) | High | Done |
| FR-04 | pb_7102_ur=50000, pb_7103_ur=50001 포트 분리 (env+compile) | High | Done |
| FR-05 | Device_Open: Connect→Connect2(2초 타임아웃) | **Critical** | Done |
| FR-06 | Device_Close: sleep(3) 제거 | High | Done |
| FR-07 | pa_7100_ur.c 동일 SO_RCVBUF 버그 수정 | Medium | Done |
| **FR-08** | **TCP 실패 시 continue 제거 (Recv_Data 스킵 방지)** | **Critical** | **Done** |
| FR-09 | TCP_RETRY_INTVL(5초) 재연결 간격 도입 | High | Done |
| FR-10 | HB_TIMEOUT 3초→5초 변경 (Connect2 여유 확보) | High | Done |
| FR-11 | HA Init/Send/Recv 진단 로그 추가 | High | Done |

### 3.2 Non-Functional Requirements

| Category | Criteria | Measurement Method |
|----------|----------|-------------------|
| 호환성 | 기존 HB_PACKET 프로토콜 하위 호환 유지 | rt>=2 조건으로 구/신 버전 모두 수용 |
| 안정성 | 24시간 연속 운용 시 HB 누락 0건 | Primary/Secondary 양쪽 로그 확인 |
| 빌드 | mk.sh pb 정상 빌드 | Linux/HP-UX 모두 |

---

## 4. Success Criteria

### 4.1 Definition of Done

- [x] 5개 버그 수정 완료 (코드 반영)
- [ ] mk.sh pb 빌드 성공
- [ ] Primary 로그에 "HA HB sent" 출력 확인
- [ ] Secondary 로그에 "HA HB recv ok" 출력 확인
- [ ] 24시간 운용 후 오판 절체 0건

### 4.2 Quality Criteria

- [ ] SO_RCVBUF 실제 적용 확인 (`cat /proc/net/udp` 또는 `ss -ulnm`)
- [ ] 포트 50000/50001 분리 확인 (`netstat -ulnp`)
- [ ] HB 수신/전송 로그 주기적 출력 확인

---

## 5. Risks and Mitigation

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| sleep(3) 제거 시 TCP 재연결 폭주 | Medium | Low | select 1초 타임아웃이 자연 간격 역할 |
| HB 로그 과다 출력 (1초마다) | Low | High | 운영 안정화 후 로그 레벨 조정 또는 N초 간격 출력 |
| HB 포트 분리 시 pkg_env.sh 수정 필요 | Medium | Medium | _HA_HB_PORT 환경변수 우선, 미설정 시 컴파일 기본값 사용 |
| PA 모듈 미수정 시 동일 증상 재현 | High | Medium | FR-07로 pa_7100_ur.c 동시 수정 |

---

## 6. Architecture Considerations

### 6.1 Project Level Selection

| Level | Characteristics | Selected |
|-------|-----------------|:--------:|
| **Enterprise** | Multi-process daemon, IPC, HA이중화 | **O** |

### 6.2 Key Architectural Decisions

| Decision | Options | Selected | Rationale |
|----------|---------|----------|-----------|
| HB 포트 분리 방식 | 환경변수 / 컴파일 매크로 | 컴파일 매크로 (`#if B7102`) | 환경변수 하위호환 유지 + 기본값 자동 분리 |
| sleep(3) 대체 | 제거 / usleep(500ms) | 완전 제거 | select 1초가 자연 간격 역할 |
| 로그 레벨 | USR_OK / UDP_WARN | 혼합 | 정상=USR_OK, 에러=UDP_WARN |

### 6.3 수정 파일 목록

```
st01/src/PB/pb_7100_ur.c    ← 5개 버그 수정 (완료)
st01/src/PA/pa_7100_ur.c    ← SO_RCVBUF 버그 동일 수정 (FR-07, 미완)
st01/env/pkg_env.sh         ← 수정 불필요 (_HA_HB_PORT 기존 설정 유지)
```

---

## 7. Convention Prerequisites

### 7.1 Existing Project Conventions

- [x] `CLAUDE.md` has coding conventions section
- [x] Pure C89/ANSI C
- [x] KRX_MSG_COMMON 구조체 사용 규칙

### 7.2 Conventions to Define/Verify

| Category | Current State | To Define | Priority |
|----------|---------------|-----------|:--------:|
| **HA 로그 prefix** | 미정의 | "HA HB" prefix 통일 | High |
| **setsockopt 순서** | 혼재 | SO_REUSEADDR → SO_RCVBUF 순서 표준화 | Medium |

### 7.3 Environment Variables

| Variable | Purpose | Scope | Status |
|----------|---------|-------|:------:|
| `_HA_PEER_IP` | 상대 서버 IP | Server | 설정 완료 |
| `_HA_HB_PORT` | HB UDP 포트 | Server | 설정 완료 (50000) |
| `_FEP_DIV` | 환경 구분 (REAL1/REAL2/TEST) | Server | 설정 완료 |

---

## 8. Bug Detail

### BUG-01: SO_RCVBUF 미설정 (Critical)

**원인**: `setsockopt(Sockfd, SOL_SOCKET, SO_REUSEADDR, ...)` — `SO_RCVBUF`여야 할 옵션이 `SO_REUSEADDR`로 잘못 지정됨.
결과적으로 UDP 수신 버퍼가 커널 기본값(~212KB)으로 유지되어, 시세 폭주 시 패킷 드롭 → `select()` 에러 → 에러 복구 경로에서 `Recv_Data()` skip → HB 미처리.

**수정**: `SO_REUSEADDR`와 `SO_RCVBUF`를 별도 호출로 분리.

### BUG-02: recvfrom 에러 무시 (Critical)

**원인**: HB 수신 while 루프에서 `recvfrom()` 리턴값 -1(에러) 처리 누락. `rt`가 3~7 범위일 때 `rt >= HB_PKT_LEN(8)` 도 `rt == 2` 도 아니라 HB가 무시됨.

**수정**: 에러 체크 추가 + `rt == 2` → `rt >= 2`로 범위 확장.

### BUG-03: HB 포트 충돌

**원인**: pb_7102_ur, pb_7103_ur 모두 동일 소스에서 빌드되어 HB_PORT_DEFAULT=50000 공유. 동시 실행 시 bind 충돌 또는 패킷 독점.

**수정**: `#if defined B7102` / `B7103`으로 포트 분리 (50000/50001).

### BUG-04: Device_Open sleep(3)

**원인**: TCP 연결 실패 시 `sleep(3)` 동안 메인루프 블록 → HB 전송/수신 모두 차단. HB_TIMEOUT=3초와 동일하여 오판 절체 유발.

**수정**: `sleep(3)` 제거. select 1초 타임아웃이 재연결 간격 역할.

### BUG-05: 진단 로그 부재

**원인**: HB 송수신 성공/실패 로그 없어 실 환경 진단 불가.

**수정**: HA Init, Send, Recv 전 구간에 상세 로그 추가.

---

## 9. Next Steps

1. [x] ~~pb_7100_ur.c 5개 버그 수정~~
2. [ ] pa_7100_ur.c SO_RCVBUF 동일 수정 (FR-07)
3. [ ] mk.sh pb 빌드 확인
4. [ ] 실 서버 배포 및 HB 로그 확인
5. [ ] Design 문서 작성 (`/pdca design pb-ha-hotfix`)

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-03-10 | Initial draft — 5개 버그 식별 및 수정 완료 | Claude Code |
