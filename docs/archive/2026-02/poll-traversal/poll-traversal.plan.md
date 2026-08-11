# poll-traversal Planning Document

> **Summary**: poll 이벤트 이중 순회를 단일 순회로 통합하여 동시 이벤트 지연 제거
>
> **Project**: FEP (Front-End Processor)
> **Author**: Claude
> **Date**: 2026-02-28
> **Status**: Draft
> **Reference**: FEP_Architecture_Analysis.md §4.8

---

## 1. Overview

### 1.1 Purpose

현재 poll 이벤트 처리가 2단계 순회(1차: POLLHUP 체크, 2차: POLLIN 체크)로 구현되어 있어:
1. **O(2n) 순회** — PollCnt 크기만큼 2회 반복
2. **첫 번째 POLLIN만 처리** — 2차 루프에서 `break`로 첫 이벤트만 반환, 동시 이벤트는 다음 cycle까지 지연

단일 순회로 통합하여 O(n) + 모든 이벤트 즉시 처리를 달성한다.

### 1.2 Background

- §4.8 "poll 이벤트 이중 순회" (난이도: 낮음)
- FIFO와 소켓에 동시 이벤트 발생 시 한 쪽이 다음 poll cycle까지 지연
- 공유 함수 `detect_poll_event()`와 인라인 이중 루프가 혼재

### 1.3 Related Documents

- `docs/FEP_Architecture_Analysis.md` §4.8 (lines 669-703)
- `st01/sub/poll_event.c` — 공유 함수
- Previous: `atoif-optimize` (§4.7), `file-rw-optimize` (§4.3)

---

## 2. Scope

### 2.1 In Scope

- [x] `sub/poll_event.c:detect_poll_event()` 이중 루프 → 단일 루프 통합
- [x] 인라인 이중 루프 파일 11개 → 단일 루프 + 전체 이벤트 처리
- [x] 동작 등가성 보장 (POLLHUP 우선 처리 유지)

### 2.2 Out of Scope

- 단일 fd 파일 (pw_1000_mp, pw_2000_mp, pz_daemon_proc, pz_fepp) — for 루프 없음
- 역순 단일 루프 파일 (pa_2100_ts, pa_5020_mp, pa_5010_mp) — 이미 단일 루프
- detect_poll_event() 호출자의 다중 이벤트 처리 전환 — 반환값 호환성 유지

---

## 3. Requirements

### 3.1 Functional Requirements

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-01 | `sub/poll_event.c:detect_poll_event()` 이중 루프를 단일 루프로 통합. 모든 POLLHUP를 체크하면서 첫 POLLIN 인덱스를 반환하는 기존 반환 의미론 유지 | High | Pending |
| FR-02 | 인라인 이중 루프 11개소를 단일 루프로 통합. switch를 루프 내부로 이동하여 모든 POLLIN 이벤트를 한 cycle에 처리 | High | Pending |
| FR-03 | 동작 등가성: 각 디스크립터에서 POLLHUP을 POLLIN보다 먼저 체크. 소켓 POLLHUP 시 즉시 return/disconnect 유지 | High | Pending |
| FR-04 | C89 코딩 스타일 준수 (탭 들여쓰기, 기존 주석 스타일) | Medium | Pending |

### 3.2 대상 파일 목록

**FR-01: 공유 함수** (1개 파일, 7개 호출자 자동 적용)

| 파일 | 변경 | 호출자 |
|------|------|--------|
| `sub/poll_event.c` | 이중 루프 → 단일 루프 | pa_1100_ts, pa_3100_ts, pa_7100_ts, pa_7800_tr, pa_8100_ts, pb_1100_ts, pb_8200_tr |

**FR-02: 인라인 이중 루프** (11개소)

| # | 파일 | POLLHUP 루프 | POLLIN 루프 |
|---|------|-------------|-------------|
| 1 | `src/PA/pa_1200_tr.c` | ~256 | ~273 |
| 2 | `src/PA/pa_7000_tr.c` | ~210 | ~226 |
| 3 | `src/PA/pa_8200_tr.c` | ~224 | ~240 |
| 4 | `src/PA/pa_3100_ts.c` | ~705 | ~714 |
| 5 | `src/PB/pb_1200_tr.c` | ~260 | ~277 |
| 6 | `src/PB/pb_1800_ts.c` | ~259 | ~275 |
| 7 | `src/PB/pb_7100_ts.c` | ~216 | ~232 |
| 8 | `src/PB/pb_7200_tr.c` | ~166 | ~182 |
| 9 | `src/PB/pb_7800_tr.c` | ~167 | ~183 |
| 10 | `src/PB/pb_8100_ts.c` | ~264 | ~280 |
| 11 | `src/PW/pw_4000_ts.c` | ~236 | ~252 |

### 3.3 Non-Functional Requirements

| Category | Criteria | Measurement |
|----------|----------|-------------|
| Performance | 동시 이벤트 시 1 poll cycle 절약 | 코드 검사 |
| Compatibility | 기존 호출자 변경 없음 (FR-01) | grep 검증 |
| Safety | POLLHUP 우선순위 디스크립터 단위 유지 | 코드 리뷰 |

---

## 4. Success Criteria

### 4.1 Definition of Done

- [ ] detect_poll_event() 단일 루프로 통합
- [ ] 인라인 11개소 단일 루프로 통합
- [ ] for 루프 내 POLLHUP → POLLIN 순서 유지
- [ ] 빌드 성공 (mk.sh sub && mk.sh src)

### 4.2 Quality Criteria

- [ ] 이중 루프 패턴 0건 (grep 검증)
- [ ] 기존 switch case 로직 보존
- [ ] 7개 detect_poll_event 호출자 수정 없음

---

## 5. Risks and Mitigation

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| 인라인 파일별 switch case/변수명 차이 | Low | High | 각 파일 개별 확인 후 변환 |
| POLLHUP+POLLIN 동시 발생 시 처리 순서 변경 | Medium | Low | 디스크립터 단위 POLLHUP 우선 유지 |
| pa_3100_ts.c 이중 소유 (detect_poll_event + inline) | Low | Low | 메인 루프(detect_poll_event)는 유지, 보조 루프만 변환 |

---

## 6. 변환 패턴

### 6.1 detect_poll_event() (FR-01)

**Before** (이중 루프):
```c
/* Check for hangup events */
for (i = 0; i < poll_cnt; i++) {
    if (poll_arr[i].revents & POLLHUP) {
        if (i == socket_event_idx) return -1;
        Log(SYS_ERROR, ...);
    }
}
/* Find first POLLIN event */
for (i = 0; i < poll_cnt; i++) {
    if (poll_arr[i].revents & POLLIN) {
        poll_arr[i].revents = 0;
        return i;
    }
}
return -2;
```

**After** (단일 루프):
```c
int first_pollin = -2;
for (i = 0; i < poll_cnt; i++) {
    if (poll_arr[i].revents & POLLHUP) {
        if (i == socket_event_idx) return -1;
        Log(SYS_ERROR, ...);
    }
    if ((poll_arr[i].revents & POLLIN) && first_pollin == -2) {
        poll_arr[i].revents = 0;
        first_pollin = i;
    }
}
return first_pollin;
```

**핵심**: 모든 POLLHUP를 체크하면서 첫 POLLIN 인덱스 기록. 반환값 의미론 동일 (-1, -2, >=0).

### 6.2 인라인 이중 루프 (FR-02)

**Before** (이중 루프 + switch 외부):
```c
for (i = 0; i < PollCnt; i++) {
    if (Poll[i].revents & POLLHUP) {
        if (i == SOCKET_EVENT) { Log(...); return; }
        Log(SYS_ERROR, ...);
    }
}
for (i = 0; i < PollCnt; i++) {
    if (Poll[i].revents & POLLIN) {
        Poll[i].revents = 0;
        break;
    }
}
switch (i) {
    case FIFO_EVENT:   Fifo_Event_Rtn();   break;
    case SOCKET_EVENT: Socket_Event_Rtn();  break;
    case DATA_EVENT:   Data_Event_Rtn();    break;
    default: Log(USR_ERROR, ...); Exit_Process(); break;
}
```

**After** (단일 루프 + switch 내부):
```c
for (i = 0; i < PollCnt; i++) {
    if (Poll[i].revents & POLLHUP) {
        if (i == SOCKET_EVENT) { Log(...); return; }
        Log(SYS_ERROR, ...);
    }
    if (Poll[i].revents & POLLIN) {
        Poll[i].revents = 0;
        switch (i) {
            case FIFO_EVENT:   Fifo_Event_Rtn();   break;
            case SOCKET_EVENT: Socket_Event_Rtn();  break;
            case DATA_EVENT:   Data_Event_Rtn();    break;
            default: Log(USR_ERROR, ...); Exit_Process(); break;
        }
    }
}
```

**핵심**: switch를 루프 내부로 이동. break는 switch의 break (루프 아님). 모든 POLLIN 이벤트 한 cycle에 처리.

---

## 7. Next Steps

1. [ ] Design 문서 작성 (`poll-traversal.design.md`)
2. [ ] 12개 파일 정확한 before/after 코드
3. [ ] 구현 및 Gap 분석

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-28 | Initial draft | Claude |
