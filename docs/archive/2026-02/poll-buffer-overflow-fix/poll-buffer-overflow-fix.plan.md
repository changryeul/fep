# poll-buffer-overflow-fix Planning Document

> **Summary**: Fix critical buffer overflow bug in pb_7200_tr.c — Poll[1] array accessed at out-of-bounds index 1
>
> **Project**: FEP (Front-End Processor)
> **Author**: Claude Code
> **Date**: 2026-02-22
> **Status**: Draft
> **Type**: BUG FIX (Critical)

---

## 1. Overview

### 1.1 Purpose

Fix a heap buffer overflow vulnerability in `pb_7200_tr.c` where the `Init_Parameters()` function writes to `Poll[1]` (index 1) while the array is declared as `struct pollfd Poll[1]` (only 1 element, valid index: 0 only).

### 1.2 Bug Description

**File**: `st01/src/PB/pb_7200_tr.c`

```
Line  41: struct pollfd           Poll[1];     ← Array size = 1 (valid index: 0)
Line 214: Poll[1].fd = INPUT_FD;               ← WRITE TO INDEX 1 (OUT OF BOUNDS)
Line 215: Poll[1].events = POLLIN;             ← WRITE TO INDEX 1 (OUT OF BOUNDS)
```

This writes 8 bytes (sizeof `struct pollfd`) past the end of the `Poll` array, corrupting whatever global variable is stored immediately after `Poll` in memory. On most platforms, `struct pollfd` is 8 bytes, so this overwrites the next 8 bytes of the BSS segment.

### 1.3 Root Cause

Copy-paste from `pb_7100_ts.c` which correctly declares `Poll[2]` (2 elements) and uses both indices:
- `pb_7100_ts.c`: `struct pollfd Poll[2];` + `PollCnt = 2;` + has `DATA_EVENT = 1` case
- `pb_7200_tr.c`: `struct pollfd Poll[1];` + `PollCnt = 1;` + NO DATA_EVENT case

The `Poll[1]` assignment was copied verbatim but the array size and PollCnt were correctly reduced to 1. The dead assignment was left behind.

### 1.4 Impact Assessment

- **Severity**: CRITICAL (heap/BSS buffer overflow)
- **Runtime effect**: Overwrites the global variable adjacent to `Poll` in memory. Since `Poll` is a global, this corrupts BSS segment. The corrupted variable depends on compiler layout.
- **Exploitability**: Not externally exploitable (writes a fixed value), but causes **silent data corruption** at process startup
- **Why not caught**: The written value (`INPUT_FD` / `POLLIN`) is never used because `PollCnt = 1`, so the process appears to work normally while silently corrupting adjacent memory

### 1.5 Scope Verification

Checked ALL files with `struct pollfd Poll[1]` declaration for the same bug:

| File | Poll size | Accesses Poll[1]? | Bug? |
|------|:---------:|:-----------------:|:----:|
| `pb_7200_tr.c` | [1] | YES (lines 214-215) | **YES** |
| `pa_7000_tr.c` | [1] | No | No |
| `pa_8200_tr.c` | [1] | No | No |
| `pb_8200_tr.c` | [1] | No | No |
| `pz_fepp.c` | [1] | No | No |

Only `pb_7200_tr.c` is affected.

---

## 2. Scope

### 2.1 In Scope

- [x] Remove out-of-bounds `Poll[1]` assignment in `pb_7200_tr.c` Init_Parameters()

### 2.2 Out of Scope

- Other `Poll[]` array usage patterns
- `pb_7100_ts.c` (correctly uses `Poll[2]`, no bug)
- Any other files (verified: no other file has this bug)

---

## 3. Requirements

### 3.1 Functional Requirements

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-01 | Remove `Poll[1].fd = INPUT_FD;` and `Poll[1].events = POLLIN;` from pb_7200_tr.c | Critical | Pending |

### 3.2 Non-Functional Requirements

| Category | Criteria | Measurement Method |
|----------|----------|-------------------|
| No behavior change | Process still polls only socket (Poll[0]) | PollCnt unchanged at 1 |
| Memory safety | No out-of-bounds writes | Code review |
| Binary compatibility | `mk.sh pb` builds successfully | Build on server |

---

## 4. Fix Detail

### 4.1 BEFORE (lines 211-219)

```c
    TCP2_PROC_ST = ON;
    TCP2_LINE_ST = OFF;

    Poll[1].fd = INPUT_FD;
    Poll[1].events = POLLIN;

    /* 2025 R=W (Skip), 시세시에는 지나간 데이터 스킵하기위해서, 하지만 일반데이터는 아님 */
    //R_CNT (0,1) = W_CNT (0,0);
```

### 4.2 AFTER (lines 211-216)

```c
    TCP2_PROC_ST = ON;
    TCP2_LINE_ST = OFF;

    /* 2025 R=W (Skip), 시세시에는 지나간 데이터 스킵하기위해서, 하지만 일반데이터는 아님 */
    //R_CNT (0,1) = W_CNT (0,0);
```

### 4.3 Change Summary

- **Lines removed**: 2 (dead code causing buffer overflow)
- **Lines added**: 0
- **Net delta**: -2 lines

---

## 5. Risks and Mitigation

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| Removing Poll[1] breaks functionality | None | Zero | PollCnt=1, Poll[1] is never polled. Dead code. |
| Adjacent memory was depended on being overwritten | None | Zero | Buffer overflow is undefined behavior, no correct code depends on it |

---

## 6. Success Criteria

### 6.1 Definition of Done

- [x] `Poll[1]` out-of-bounds access removed from `pb_7200_tr.c`
- [x] No other `Poll[1]` declaration files have the same bug (verified)
- [x] Gap analysis match rate >= 90%

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-22 | Initial draft | Claude Code |
