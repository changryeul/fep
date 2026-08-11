# poll-oob-write-fix 설계 문서

> **요약**: 2개 파일의 Poll[2] out-of-bounds writes 수정 + 2개 파일의 주석 처리된 dead code 제거
>
> **프로젝트**: FEP (Front-End Processor)
> **작성자**: Claude Code
> **날짜**: 2026-02-22
> **상태**: Draft
> **유형**: BUG FIX (Critical) + Dead Code Cleanup
> **계획 문서**: [poll-oob-write-fix.plan.md](../../01-plan/features/poll-oob-write-fix.plan.md)

---

## 1. 개요

### 1.1 설계 목표

1. `pb_1200_tr.c`와 `pa_1200_tr.c`에서 2개의 active out-of-bounds `Poll[2]` writes 제거
2. `pb_7800_tr.c`와 `pa_7800_tr.c`에서 2개의 주석 처리된 `Poll[2]` dead code 블록 제거
3. 동작 변화 없음 (제거된 코드는 모두 dead — poll되지 않음, switch case 없음)

### 1.2 설계 원칙

- **최소 변화** — 문제/dead 줄만 제거
- **실제 코드 인용** — 정확한 BEFORE/AFTER with 줄 번호

---

## 2. 상세 구현

### 2.1 FR-01: pb_1200_tr.c에서 active Poll[2] OOB write 제거

**파일**: `st01/src/PB/pb_1200_tr.c`
**함수**: `Init_Parameters()` (줄 266)

**BEFORE** (줄 296-300):
```c
    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;
    Poll[2].fd = INPUT_FD;
    Poll[2].events = POLLIN;

```

**AFTER** (줄 296-298):
```c
    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;

```

**변경**: 줄 298-299 제거. Delta: **-2 lines**.

**안전성**: `Poll`은 `struct pollfd Poll[2]` (줄 120)로 선언, `PollCnt` max는 2 (줄 186, 193). `poll()`은 인덱스 0과 1만 검사. 줄 238의 switch는 `FIFO_EVENT=0`과 `SOCKET_EVENT=1`만 처리. `Poll[1]`은 `fep_common.c:348`의 `Device_Open_Logon()`에서 `Sockfd`로 설정.

---

### 2.2 FR-02: pa_1200_tr.c에서 active Poll[2] OOB write 제거

**파일**: `st01/src/PA/pa_1200_tr.c`
**함수**: `Init_Parameters()` (줄 245)

**BEFORE** (줄 272-276):
```c
    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;
    Poll[2].fd = INPUT_FD;
    Poll[2].events = POLLIN;

```

**AFTER** (줄 272-274):
```c
    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;

```

**변경**: 줄 274-275 제거. Delta: **-2 lines**.

**안전성**: FR-01과 동일한 이유. `struct pollfd Poll[2]` (줄 100), `PollCnt` max 2, switch는 인덱스 0-1만 처리.

---

### 2.3 FR-03: pb_7800_tr.c에서 주석 처리된 Poll[2] dead code 제거

**파일**: `st01/src/PB/pb_7800_tr.c`
**함수**: `Init_Parameters()` (줄 257)

**BEFORE** (줄 272-278):
```c
    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;
/*
    Poll[2].fd = INPUT_FD;			//	Input??????
    Poll[2].events = POLLIN;		//	Input??????
*/

```

**AFTER** (줄 272-274):
```c
    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;

```

**변경**: 줄 274-277 제거 (`/* ... */` 블록). Delta: **-4 lines**.

---

### 2.4 FR-04: pa_7800_tr.c에서 주석 처리된 Poll[2] dead code 제거

**파일**: `st01/src/PA/pa_7800_tr.c`
**함수**: `Init_Parameters()` (줄 228)

**BEFORE** (줄 242-248):
```c
    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;
/*
    Poll[2].fd = INPUT_FD;			//	Input있으면
    Poll[2].events = POLLIN;		//	Input있으면
*/

```

**AFTER** (줄 242-244):
```c
    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;

```

**변경**: 줄 244-247 제거 (`/* ... */` 블록). Delta: **-4 lines**.

---

## 3. 검증 체크리스트

- [ ] FR-01: `Poll[2].fd`와 `Poll[2].events` active 줄 `pb_1200_tr.c`에서 제거
- [ ] FR-02: `Poll[2].fd`와 `Poll[2].events` active 줄 `pa_1200_tr.c`에서 제거
- [ ] FR-03: `/* Poll[2] ... */` comment 블록 `pb_7800_tr.c`에서 제거
- [ ] FR-04: `/* Poll[2] ... */` comment 블록 `pa_7800_tr.c`에서 제거
- [ ] 모든 4개 파일: `struct pollfd Poll[2]` 선언 변경 없음
- [ ] 모든 4개 파일: `PollCnt` 할당 변경 없음
- [ ] 모든 4개 파일: 다른 변경 없음

---

## 4. 영향 요약

| 파일 | Delta |
|------|------:|
| `src/PB/pb_1200_tr.c` | **-2** |
| `src/PA/pa_1200_tr.c` | **-2** |
| `src/PB/pb_7800_tr.c` | **-4** |
| `src/PA/pa_7800_tr.c` | **-4** |
| **합계** | **-12** |

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-22 | Initial draft with actual code citations | Claude Code |
