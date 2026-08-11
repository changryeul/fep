# fifo-struct-refactor 완료 보고서

> **요약**: Refactored FIFO/SAM file I/O buffer construction and KRX message parsing from raw byte offsets to struct-based type-safe access
>
> **프로젝트**: FEP (Front-End Processor) for KRX
> **Author**: Claude Code
> **Date**: 2026-02-21
> **Match Rate**: 97%
> **Status**: PASS

---

## 1. Executive Summary

Completed the third struct-based refactoring cycle for the FEP system. Replaced raw byte offset patterns (`w_data[0/8/16/24/28/38/50/70]`, `DataBuff[8+6]`, `DataBuff[82+...]`, `ADD_HEADER_SIZE`) with struct-based type-safe access across 10 source files. Match rate 97% achieved on first check with 1 gap fixed during the Check phase (no formal Act iteration required).

This feature is the natural continuation of `tr-struct-refactor` (92%, KRX protocol structs) and `shm-struct-refactor` (95%, SHM access), targeting the FIFO/SAM file I/O layer.

---

## 2. 범위 & Requirements

### 2.1 Functional Requirements

| FR | Description | Priority | Result |
|----|-------------|----------|--------|
| FR-01 | `w_data[]` raw BUFF_RW_HEAD construction -> struct-based | High | COMPLETE |
| FR-02 | `DataBuff[8+6]` MsgType access -> KRX_HEADER struct cast | High | COMPLETE |
| FR-03 | `DataBuff[82+...]` body parsing -> KRX_HEAD_LEN constant | High | COMPLETE |
| FR-04 | `DataBuff[80]` in pa_3100_ts.c | High | DE-SCOPED |
| FR-05 | `ADD_HEADER_SIZE` dead macro removal | Medium | COMPLETE |
| FR-06 | `Buff_RW_Head_Set()` helper function | Medium | ALTERNATIVE |

### 2.2 Design Deviations

| Item | Design | Actual | Rationale |
|------|--------|--------|-----------|
| FR-04 | Fix DataBuff[80] -> KRX_HEAD_LEN | De-scoped | pa_3100_ts.c not in active build (Make_PA_ts.sh); has type error (char vs char*) |
| FR-06 | Static helper in fep_file.h | Direct struct access | Include order: fep_file.h included at fep_sub.h:170, but ItoAf() declared at line 283 — helper can't call ItoAf(). Design section 7.4 alternative used. |
| KRX_BODY_ERR_LEN | #define constant | Literal 4 in KRX_HEAD_LEN+4 | Low severity; contextually clear without named constant |

---

## 3. Implementation Summary

### 3.1 Files Modified

| # | File | Module | FRs | Changes |
|---|------|--------|-----|---------|
| 1 | `pa_2700_tr.c` | PA | FR-01 | 1 w_data block -> BUFF_RW_HEAD struct |
| 2 | `pa_7800_tr.c` | PA | FR-01, FR-02 | 1 w_data block + 1 DataBuff[8+6] |
| 3 | `pb_7800_tr.c` | PB | FR-01, FR-02, FR-03 | 2 w_data blocks + 6 DataBuff[8+6] + 6 DataBuff[82+] |
| 4 | `pa_1200_tr.c` | PA | FR-02 | 2 DataBuff[8+6] casts |
| 5 | `pa_1100_ts.c` | PA | FR-02, FR-05 | 3 DataBuff[8+6] + ADD_HEADER_SIZE removed |
| 6 | `pb_1200_tr.c` | PB | FR-02 | 6 DataBuff[8+6] casts |
| 7 | `pb_1100_ts.c` | PB | FR-02, FR-05 | 6 DataBuff[8+6] + 1 DataBuff+14 + ADD_HEADER_SIZE removed |
| 8 | `pb_1800_ts.c` | PB | FR-02 | 6 DataBuff[8+6] casts |
| 9 | `pa_5000_qr.c` | PA | FR-05 | 14 ADD_HEADER_SIZE usages removed |
| 10 | `pa_1200_mp.c` | PA | FR-05 | 13 ADD_HEADER_SIZE usages removed |

**Total**: 10 files modified, 0 headers changed, ~65 pattern conversions

### 3.2 Anti-Pattern Elimination

| Anti-Pattern | Before | After |
|-------------|:------:|:-----:|
| `w_data[0/8/16/24/28/38/50/70]` raw offsets | 4 blocks / 3 files | 0 |
| `DataBuff[8+6]` array index | ~32 occurrences / 7 files | 0 |
| `DataBuff+14` pointer arithmetic | 1 occurrence | 0 |
| `DataBuff[82+...]` literal 82 | ~8 occurrences / 1 file | 0 |
| `ADD_HEADER_SIZE` macro | 4 defines + ~47 usages | 0 |

### 3.3 Conversion Patterns Used

**FR-01 (BUFF_RW_HEAD)**:
```c
/* Before: raw offsets */
ItoAf(seq, &w_data[0], 8);
memcpy(&w_data[50], &File_Data_Head, sizeof(HEAD_SIZE));
memcpy(&w_data[70], data, size);

/* After: struct-based */
BUFF_RW_HEAD f_head;
memset(&f_head, 0x20, sizeof(f_head));
ItoAf(seq, f_head.Seq, sizeof(f_head.Seq));
memcpy(f_head.DataHeader, &File_Data_Head, sizeof(FILE_DATA_HEAD));
memcpy(w_data, &f_head, sizeof(BUFF_RW_HEAD));
memcpy(&w_data[sizeof(BUFF_RW_HEAD)], data, size);
```

**FR-02 (KRX_HEADER cast)**:
```c
/* Before */
memcmp(&DataBuff[8+6], "SCHLIR00000", 11)

/* After */
memcmp(((KRX_HEADER *)DataBuff)->MsgType, "SCHLIR00000", 11)
```

**FR-05 (ADD_HEADER_SIZE removal)**:
```c
/* Before */
memcmp(&r_buf[ADD_HEADER_SIZE+11], "TCHODR4000", 10)
memcpy(dst, src, ADD_HEADER_SIZE + sizeof(KRX_NOTE_JUMUN_DATA));

/* After */
memcmp(&r_buf[11], "TCHODR4000", 10)
memcpy(dst, src, sizeof(KRX_NOTE_JUMUN_DATA));
```

---

## 4. PDCA Cycle Summary

| Phase | Status | Details |
|-------|--------|---------|
| Plan | Complete | 6 FRs, 12 target files identified |
| Design | Complete | 17-step implementation order, FR-04 de-scoped, FR-06 alternative |
| Do | Complete | 10 files modified, ~65 conversions |
| Check | PASS (97%) | 1 gap found and fixed (DataBuff+14 in pb_1100_ts.c) |
| Act | Skipped | Match rate >= 90% on first check |
| Report | Complete | This document |

```
[Plan] -> [Design] -> [Do] -> [Check] 97% -> [Report]
```

---

## 5. Metrics

### 5.1 Quantitative

| Metric | Value |
|--------|-------|
| Match Rate | 97% |
| Iterations | 0 (first check passed) |
| Files Modified | 10 source files |
| Headers Modified | 0 |
| Pattern Conversions | ~65 |
| Net Code Change | ~+20 lines (struct declarations and memset/memcpy) |
| Anti-Patterns Eliminated | 5 categories, 100% removal |

### 5.2 Comparison with Previous Refactors

| Feature | Duration | Files | Match Rate | Iterations |
|---------|----------|:-----:|:----------:|:----------:|
| tr-struct-refactor | ~9h | 8 | 72% -> 92% | 1 |
| shm-struct-refactor | ~4h | 6 | 95% | 0 |
| **fifo-struct-refactor** | **~2h** | **10** | **97%** | **0** |

Trend: Increasing match rate (92% -> 95% -> 97%), decreasing iterations (1 -> 0 -> 0), established patterns accelerate subsequent refactors.

---

## 6. Residual Items & Future Work

### 6.1 Known Residual Patterns (Not in Scope)

| Pattern | Files | Notes |
|---------|-------|-------|
| `DataBuff+68/70/72/74/77` | pa_1100_ts.c, pb_1100_ts.c | SendingTime/DataCnt raw offsets. Same class as FR-02 but different KRX_HEADER fields. Candidate for micro-refactor. |
| `DataBuff[80]` in pa_3100_ts.c | pa_3100_ts.c | File not in active build. Contains type error. Fix if file is reactivated. |
| `R_Fmt[i].Data[...]` business parsing | Various | Business data parsing varies per TR type. Uses typed struct casts in some places already. Separate effort. |

### 6.2 Recommendations

1. **Immediate**: No action needed — all scoped FRs complete, 97% match rate.
2. **Follow-up micro-refactor**: Convert `DataBuff+68/70/72/74/77` SendingTime offsets to `((KRX_HEADER *)DataBuff)->SendingTime` struct access (2 files, ~10 lines).
3. **Long-term**: Consider removing pa_3100_ts.c from the source tree if it is permanently dead code, or fixing the type error if it might be reactivated.

---

## 7. Lessons Learned

### 7.1 What Worked Well

- **Pattern-based approach**: Grep for anti-patterns, replace systematically, verify zero remaining. Simple and effective.
- **Design document quality**: Clear before/after code examples in design doc made implementation nearly mechanical.
- **`replace_all` edits**: Simple string patterns like `&DataBuff[8+6]` could be replaced across entire files in one operation, dramatically reducing implementation time.
- **Established conventions**: Reusing patterns from previous refactors (BUFF_RW_HEAD, KRX_HEADER cast) eliminated design uncertainty.

### 7.2 What Could Improve

- **Grep coverage**: Initial grep for `DataBuff[8+6]` missed the pointer arithmetic variant `DataBuff+14`. Future FR-02-class refactors should also grep for `DataBuff+{offset}` patterns.
- **Include order awareness**: FR-06 helper function was planned but hit an include dependency issue discovered only at implementation time. Reading `fep_sub.h` include chain during Design phase would have caught this earlier.

---

## 8. Documents

| Document | Path |
|----------|------|
| Plan | `docs/01-plan/features/fifo-struct-refactor.plan.md` |
| Design | `docs/02-design/features/fifo-struct-refactor.design.md` |
| Analysis | `docs/03-analysis/fifo-struct-refactor.analysis.md` |
| Report | `docs/04-report/features/fifo-struct-refactor.report.md` |

---

## 버전 이력

| 버전 | 날짜 | 변경사항 | 작성자 |
|---------|------|---------|--------|
| 1.0 | 2026-02-21 | Initial completion report | Claude Code |
