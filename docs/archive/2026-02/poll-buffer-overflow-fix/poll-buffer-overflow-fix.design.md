# poll-buffer-overflow-fix 설계 문서

> **요약**: pb_7200_tr.c의 중요 버퍼 오버플로우 수정 — out-of-bounds Poll[1] 할당 제거
>
> **프로젝트**: FEP (Front-End Processor)
> **작성자**: Claude Code
> **날짜**: 2026-02-22
> **상태**: Draft
> **유형**: BUG FIX (Critical)
> **계획 문서**: [poll-buffer-overflow-fix.plan.md](../../01-plan/features/poll-buffer-overflow-fix.plan.md)

---

## 1. 개요

### 1.1 설계 목표

1. pb_7200_tr.c에서 heap/BSS 버퍼 오버플로우를 야기하는 dead code 2줄 제거
2. 동작 변화 없음 (제거된 코드는 읽히지 않는 메모리에 쓰기만 함)

### 1.2 설계 원칙

- **최소 변화** — 문제의 2줄만 제거
- **실제 코드 인용** — 정확한 BEFORE/AFTER with 줄 번호

---

## 2. 상세 구현

### 2.1 FR-01: out-of-bounds Poll[1] 접근 제거

**파일**: `st01/src/PB/pb_7200_tr.c`
**함수**: `Init_Parameters()` (줄 197)

**BEFORE** (줄 211-218):
```c
    TCP2_PROC_ST = ON;
    TCP2_LINE_ST = OFF;

    Poll[1].fd = INPUT_FD;
    Poll[1].events = POLLIN;

    /* 2025 R=W (Skip), 시세시에는 지나간 데이터 스킵하기위해서, 하지만 일반데이터는 아님 */
    //R_CNT (0,1) = W_CNT (0,0);
```

**AFTER** (줄 211-215):
```c
    TCP2_PROC_ST = ON;
    TCP2_LINE_ST = OFF;

    /* 2025 R=W (Skip), 시세시에는 지나간 데이터 스킵하기위해서, 하지만 일반데이터는 아님 */
    //R_CNT (0,1) = W_CNT (0,0);
```

**변경**: 줄 214-215 제거. Delta: **-2 lines**.

**안전성**: `Poll`은 `struct pollfd Poll[1]` (줄 41)로 선언되고, `PollCnt = 1` (줄 128). 줄 131의 `poll()` 호출은 `Poll[0]`만 검사. `Poll[1]` write는 `Poll[2]`를 가진 `pb_7100_ts.c`에서 복사된 dead code.

---

## 3. 검증 체크리스트

- [ ] 줄 214-215 (`Poll[1].fd`와 `Poll[1].events`) pb_7200_tr.c에서 제거
- [ ] `struct pollfd Poll[1]` 선언 줄 41 변경 없음
- [ ] `PollCnt = 1` 줄 128 변경 없음
- [ ] 파일의 다른 변경 없음

---

## 4. 영향 요약

| 파일 | Delta |
|------|-------|
| `src/PB/pb_7200_tr.c` | **-2** |

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-22 | Initial draft with actual code citations | Claude Code |
