# final-perf-optimize Planning Document

> **Summary**: §4.9 성능 개선 우선순위 잔여 3개 항목 일괄 처리. 순위4(시세 수신 경로 최적화), 순위6(이미 완료), 순위10(sleep→poll 대체).
>
> **Project**: FEP (Front-End Processor)
> **Author**: Claude
> **Date**: 2026-02-28
> **Status**: Draft

---

## 1. Overview

### 1.1 Purpose

FEP_Architecture_Analysis.md §4.9 성능 개선 우선순위 표의 미완료 항목 3개를 일괄 처리하여, 성능 최적화 시리즈를 완결한다.

### 1.2 항목별 현황

| 순위 | 항목 | 상태 | 비고 |
|------|------|------|------|
| 4 | 시세 수신 경로 memset/strlen/로그 제거 | **미착수** | pb_7100_ur.c |
| 6 | F_R_Proc strlen 제거 | **이미 완료** | tmp_off 추적 방식으로 대체됨 |
| 10 | sleep(1) → poll 동적 timeout | **미착수** | pb_1100_ts.c |

**순위 6**: `file_rw.c:129` `F_R_Proc()`에서 `strlen(tmp)` 호출이 이미 `tmp_off` 오프셋 추적으로 대체되어 있음. 추가 변경 불필요.

### 1.3 Related Documents

- Architecture: `docs/FEP_Architecture_Analysis.md` §4.4, §3.6, §4.9
- 선행: tcp-nodelay(#41), stat-save-optimize(#42), file-rw-optimize(#43), atoif-optimize(#45), poll-traversal(#46), select-send-optimize(#47)

---

## 2. Scope

### 2.1 In Scope

**A. 시세 수신 경로 최적화 (pb_7100_ur.c)**
- [ ] FR-01: `memset(rbuf, 0, sizeof(rbuf))` 제거 → `rbuf[rt] = '\0'` 수신 후 null 종료
- [ ] FR-02: `strlen(rbuf)` → `rt` (Recv_Data 리턴값 사용)
- [ ] FR-03: `Log(USR_OK, "RD[%s]...")` 매 건 로그 제거 (TCP SD 로그가 동일 데이터 기록)
- [ ] FR-04: `sprintf(TrCode, "%-2.2s", rbuf)` → `memcpy` + null 종료
- [ ] FR-05: LK 응답 경로 `memset` 제거, `strlen` → 상수 15

**B. sleep(1) → poll 대체 (pb_1100_ts.c)**
- [ ] FR-06: `sleep(1)` → `poll(Poll, 1, 1000)` + FIFO 이벤트 처리

### 2.2 Out of Scope

- `Log(TCP_OK, "TCP SD[%s]...")` — 시퀀스 추적용, 제거 대상 아님
- Recv_Data() 내부 select() — UDP 수신 대기용, 필수
- pb_1100_ts.c 내 다른 sleep(3) — 에러/종료 경로용, 의도적 지연
- F_R_Proc strlen — 이미 완료

---

## 3. Requirements

### 3.1 Functional Requirements

| ID | Requirement | Priority | File |
|----|-------------|----------|------|
| FR-01 | 수신 전 memset(2048) 제거, 수신 후 rbuf[rt]='\0' | High | pb_7100_ur.c |
| FR-02 | strlen(rbuf) → rt (recvfrom 리턴값) | High | pb_7100_ur.c |
| FR-03 | USR_OK "RD" 로그 제거 (TCP SD 로그로 대체) | Medium | pb_7100_ur.c |
| FR-04 | sprintf TrCode → memcpy(TrCode, rbuf, 2) + '\0' | Medium | pb_7100_ur.c |
| FR-05 | LK 응답 memset 제거, strlen→15 상수 | Medium | pb_7100_ur.c |
| FR-06 | sleep(1) → poll(Poll,1,1000) + Fifo_Event_Rtn() | High | pb_1100_ts.c |

### 3.2 Non-Functional Requirements

| Category | Criteria |
|----------|----------|
| 성능 | 시세 처리량 20-30% 향상 (memset/strlen/Log I/O 제거) |
| 호환성 | HP-UX, SunOS, AIX, Linux 전부 빌드/동작 |
| 안전성 | Select_Send 전송 길이 동일, FIFO 이벤트 처리 유지 |
| C89 준수 | ANSI C89 문법 준수 |

---

## 4. Technical Design

### 4.1 시세 수신 경로 (pb_7100_ur.c)

**현재 문제**:
- `memset(rbuf, 0, 2048)`: 매 패킷마다 2KB 초기화 — recvfrom이 덮어쓸 영역
- `strlen(rbuf)`: 바이너리 데이터에 strlen — 위험하고 느림. rt로 대체 가능
- `Log(USR_OK, "RD...")`: 초당 수천 건 로그 I/O — TCP SD 로그와 중복
- `sprintf(TrCode, "%-2.2s", rbuf)`: 2바이트 복사에 printf 계열 사용

**Recv_Data() 분석**: `recvfrom()` 리턴값 = 수신 바이트 수 → `rt`로 정확한 길이 확보

### 4.2 sleep(1) → poll (pb_1100_ts.c)

**현재 문제**: 장종료 후 `sleep(1)` — FIFO 이벤트(운영자 명령) 1초간 무시

**개선**: `poll(Poll, 1, 1000)` — FIFO 감시 + 1초 대기. 이벤트 도착 시 즉시 처리.

`Poll[0]`은 `Init_Parameters()`에서 `START_FD`(FIFO)로 초기화됨 (line 329).

---

## 5. 파일 목록 및 변경량

| 파일 | 변경 | 줄 수 |
|------|------|-------|
| `src/PB/pb_7100_ur.c` | memset/strlen/Log/sprintf 제거/대체 | ~-8 |
| `src/PB/pb_1100_ts.c` | sleep(1) → poll + Fifo_Event_Rtn | ~+3 |
| **합계** | **2개 파일** | **~-5줄 순** |

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-28 | Initial draft — 2 files, 6 FRs | Claude |
