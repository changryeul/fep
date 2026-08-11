# sizeof-memset-bugfix Completion Report

> **Feature**: sizeof-memset-bugfix
> **날짜**: 2026-02-22
> **상태**: Completed
> **일치 Rate**: 100% (7/7)
> **Iterations**: 0

---

## 1. 요약

Fixed a critical `sizeof()` pointer arithmetic bug in `pb_1200_tr.c:520` (`DecryptBody()` function). The expression `sizeof(DataBuff-82)` performed pointer arithmetic inside `sizeof()`, yielding 8 bytes (pointer size on 64-bit) instead of the intended 4014 bytes (4096 - 82). This caused only 8 bytes to be zeroed before decrypted KRX bond response data was copied in, leaving up to 4006 bytes of stale data from previous messages.

Also replaced a hardcoded `82` with the named constant `KRX_HEAD_LEN` on line 522 for consistency.

---

## 2. PDCA Cycle

| 단계 | 상태 | Notes |
|-------|:------:|-------|
| Plan | Done | 2 FR items, 1 file identified. Codebase scan confirmed only instance. |
| 설계 | Done | Before/after specs for both lines, 5 verification criteria defined. |
| Do | Done | 2 line changes applied in single batch. |
| Check | Done | 100% match rate (2/2 FR + 5/5 verification). 0 iterations needed. |
| Report | Done | This document. |

---

## 3. 변경사항

| FR | 파일 | Line | Before | After |
|----|------|:----:|--------|-------|
| FR-01 | `pb_1200_tr.c` | 520 | `sizeof(DataBuff-82)` | `sizeof(DataBuff) - KRX_HEAD_LEN` |
| FR-02 | `pb_1200_tr.c` | 522 | `82 + dec_len` | `KRX_HEAD_LEN + dec_len` |

**Impact**: FR-01 is a real bug fix (memset now clears 4014 bytes instead of 8). FR-02 is a consistency improvement (named constant instead of magic number).

---

## 4. Metrics

| Metric | Value |
|--------|-------|
| Files changed | 1 |
| Lines changed | 2 |
| FR items | 2 |
| Match rate | 100% |
| Iterations | 0 |
| Bug severity | Critical (data corruption / information leakage) |

---

## 5. Cumulative 프로젝트 Stats

| # | Feature | Lines | Files | 일치 | Iter |
|:-:|---------|------:|------:|:-----:|:----:|
| 1 | fifo-event-rtn-adoption | -195 | 8 | 100% | 0 |
| 2 | poll-buffer-overflow-fix | -2 | 1 | 100% | 0 |
| 3 | poll-oob-write-fix | -12 | 4 | 100% | 0 |
| 4 | socket-linger-extraction | -357 | 15 | 100% | 0 |
| 5 | dead-code-cleanup | -1,945 | 36 | 98% | 0 |
| 6 | pa-pb-8100-cleanup | -64 | 2 | 100% | 0 |
| 7 | commented-code-cleanup-round2 | -213 | 14 | 100% | 0 |
| 8 | sub-commented-code-final | -39 | 3 | 100% | 0 |
| 9 | src-commented-code-cleanup | -489 | 35 | 100% | 1 |
| 10 | if1-block-cleanup | -36 | 11 | 100% | 0 |
| **11** | **sizeof-memset-bugfix** | **2** | **1** | **100%** | **0** |
| | **Cumulative** | **~-3,350** | | **avg 100%** | **1 total** |

Note: This is the first feature in the series that is a **bug fix** rather than dead code/cleanup. Net line count is +0 (2 lines modified, no additions or deletions).

---

## 6. 교훈

- `sizeof(array - N)` is a subtle C trap: the array decays to a pointer, making the subtraction pointer arithmetic, and `sizeof` returns the pointer size (8) not the intended buffer calculation.
- Codebase-wide scan confirmed this was the only instance of this pattern.
- Using named constants (`KRX_HEAD_LEN`) consistently prevents future bugs if header size changes.
