# fifo-event-rtn-adoption 계획 문서

> **요약**: Extend shared Fifo_Event_Rtn to 13 remaining process files by isolating it in its own object file
>
> **프로젝트**: FEP (Front-End Processor)
> **Author**: Claude Code
> **Date**: 2026-02-22
> **Status**: Draft

---

## 1. 개요

### 1.1 목표

Eliminate 13 remaining local copies of the identical `Fifo_Event_Rtn()` function across PA, PB, and PW process files. Phase 3 of the PA/PB dedup series moved `Fifo_Event_Rtn` into `sub/fep_common.c` for 6 KRX-direct files. This feature extends adoption to the remaining 13 files that still maintain byte-for-byte identical local copies.

### 1.2 배경

After PA/PB dedup Phase 3, `Fifo_Event_Rtn` exists in:
- **Shared library** (`sub/fep_common.c`): Used by 6 files via `fep_common.h`
- **13 local copies**: Byte-for-byte identical 4-line function in remaining process files

All 13 local copies are identical:
```c
void Fifo_Event_Rtn(void) {
    char tmp[2];
    read(START_FD, tmp, 1);
    return;
}
```

### 1.3 Linker Constraint

The 6 existing files that use `fep_common.h` are KRX-direct connection processes. They define globals like `ConnectRetryCnt`, `FmtPtr`, `NoTime[]` that `fep_common.o` depends on. The 13 target files do NOT define these globals — they are client comm, market data, auto-trading, and PW processes.

**Problem**: If these 13 files reference `Fifo_Event_Rtn` from `fep_common.o`, the linker will pull in the entire `fep_common.o`, which has undefined references to `ConnectRetryCnt`, `FmtPtr`, etc. — causing link failures.

**Solution**: Move `Fifo_Event_Rtn` from `fep_common.c` into its own `sub/fifo_event.c` file. The resulting `fifo_event.o` only depends on `START_FD` (a macro), so the linker can pull it in without any global variable conflicts.

### 1.4 Related Documents

- Phase 3 report: `docs/archive/2026-02/pa-pb-dedup-phase3/pa-pb-dedup-phase3.report.md`
- Architecture: `docs/FEP_Architecture_Analysis.md`
- Shared headers: `st01/inc/fep_common.h`

---

## 2. 범위

### 2.1 포함 범위

- [x] Move Fifo_Event_Rtn from `sub/fep_common.c` to new `sub/fifo_event.c`
- [x] Keep prototype in `fep_common.h` (no change)
- [x] Remove local Fifo_Event_Rtn definition from 13 process files
- [x] Keep existing forward declarations in process files (they serve as prototypes)
- [x] 13 target files across PA, PB, PW modules

### 2.2 제외 범위

- Adding `fep_common.h` to non-KRX files (linker incompatible due to globals)
- Other function extractions from these 13 files
- pb_1800_ts.c migration to fep_common.h (has unique Device_Write/Read)
- pa_3100_ts.c migration (uses SLog instead of Log, different protocol)

---

## 3. 요구사항

### 3.1 기능 요구사항

| ID | 요구사항 | 우선순위 | 상태 |
|----|-------------|----------|--------|
| FR-01 | Create `sub/fifo_event.c` with Fifo_Event_Rtn moved from fep_common.c | High | Pending |
| FR-02 | Remove Fifo_Event_Rtn from `sub/fep_common.c` | High | Pending |
| FR-03 | Remove local Fifo_Event_Rtn definitions from 13 process files | High | Pending |
| FR-04 | Keep forward declarations in process files as prototypes | High | Pending |
| FR-05 | Build compatibility: `mk.sh sub && mk.sh src` passes | High | Pending |

### 3.2 비기능 요구사항

| 분류 | 기준 | 측정 방법 |
|----------|----------|-------------------|
| Binary compatibility | All 75 process binaries build | `mk.sh all` on build server |
| Zero behavior change | Identical runtime behavior | Code review (identical function body) |
| Code reduction | 13 local copies removed (~65 lines) | `wc -l` before/after |

---

## 4. Target Files

### 4.1 File Inventory (13 files)

| # | File | Module | Fifo_Event_Rtn Decl | Fifo_Event_Rtn Def | Header |
|---|------|--------|--------------------|--------------------|--------|
| 1 | `src/PA/pa_2100_ts.c` | Derivatives order TX | line 63 | lines 417-425 | fep_fepp.h |
| 2 | `src/PA/pa_3100_ts.c` | Equity order TX | line 76 | lines 330-338 | fep_fepp.h |
| 3 | `src/PA/pa_5020_mp.c` | Auto-trading strategy | line 57 | lines 891-899 | fep_fepp.h |
| 4 | `src/PA/pa_7000_tr.c` | Market data TR | line 45 | lines 255-263 | fep_fepp.h |
| 5 | `src/PA/pa_7100_ts.c` | Market data TS | line 51 | lines 244-252 | fep_fepp.h |
| 6 | `src/PA/pa_8100_ts.c` | Client comm TS | line 52 | lines 294-302 | fep_fepp.h |
| 7 | `src/PA/pa_8200_tr.c` | Client comm TR | line 50 | lines 275-283 | fep_fepp.h |
| 8 | `src/PB/pb_1800_ts.c` | Bond order TX | line 118 | lines 383-391 | fep_fepp.h |
| 9 | `src/PB/pb_7100_ts.c` | Bond market data TS | line 62 | lines 255-262 | fep_fepp.h |
| 10 | `src/PB/pb_7200_tr.c` | Bond market data TR | line 48 | lines 231-238 | fep_fepp.h |
| 11 | `src/PB/pb_8100_ts.c` | Bond client comm TS | line 65 | lines 294-302 | fep_fepp.h |
| 12 | `src/PB/pb_8200_tr.c` | Bond client comm TR | line 54 | lines 270-278 | fep_fepp.h |
| 13 | `src/PW/pw_4000_ts.c` | PW module TS | line 51 | lines 294-302 | fep_fepj.h |

All include either `fep_fepp.h` or `fep_fepj.h`, both of which transitively include `fep_interface.h` where `START_FD` is defined (line 138).

### 4.2 Library Files Modified

| File | Change |
|------|--------|
| `sub/fep_common.c` | Remove Fifo_Event_Rtn function definition |
| `sub/fifo_event.c` | **New file** — Fifo_Event_Rtn definition |
| `inc/fep_common.h` | No change (prototype remains) |

---

## 5. Architecture

### 5.1 Object File Isolation

```
BEFORE:
┌─────────────────────────────┐
│ fep_common.o                │
│  Get_Msec()                 │
│  Line_Change()              │
│  Device_Read()       ──────►│ Requires: ConnectRetryCnt,
│  Device_Write()             │  Sockfd, FmtPtr, NoTime[], ...
│  Device_Close_Base()        │
│  Err_Msg()                  │
│  Log_Out_Base()             │
│  Device_Open_Logon()        │
│  Time_Out_Disconnect()      │
│  Fifo_Event_Rtn()    ──────►│ Requires: START_FD (macro only)
└─────────────────────────────┘

AFTER:
┌─────────────────────────────┐
│ fep_common.o                │
│  Get_Msec()                 │
│  Line_Change()              │
│  Device_Read()       ──────►│ Requires: ConnectRetryCnt,
│  Device_Write()             │  Sockfd, FmtPtr, NoTime[], ...
│  Device_Close_Base()        │
│  Err_Msg()                  │
│  Log_Out_Base()             │
│  Device_Open_Logon()        │
│  Time_Out_Disconnect()      │
└─────────────────────────────┘

┌─────────────────────────────┐
│ fifo_event.o                │
│  Fifo_Event_Rtn()    ──────►│ Requires: START_FD (macro only)
└─────────────────────────────┘  ← Safe for ALL process binaries
```

### 5.2 Build System Integration

The build script `make/SUB/Make_Lib_P_c.sh` iterates over all `*.c` files in `sub/` (excluding only `dbipc.c`). Adding `sub/fifo_event.c` automatically includes it in `libfepP.a`.

---

## 6. Risks and Mitigation

| 위험 | 영향 | 가능성 | 완화 |
|------|--------|------------|------------|
| Forward decl mismatch with library | Low | None | Forward decl `void Fifo_Event_Rtn(void)` matches exactly |
| Build breaks on link for non-KRX processes | High | None | fifo_event.o has no external variable dependencies |
| Existing 6 PA/PB files break | Medium | None | fep_common.h prototype unchanged; fifo_event.o resolves it |
| pw_4000_ts.c uses different header chain | Low | None | START_FD defined in fep_interface.h, included by all headers |

---

## 7. Success Criteria

### 7.1 Definition of Done

- [x] `sub/fifo_event.c` created with Fifo_Event_Rtn
- [x] Fifo_Event_Rtn removed from `sub/fep_common.c`
- [x] 13 process files: local definition removed, forward declaration retained
- [x] Gap analysis match rate >= 90%

### 7.2 Quality Criteria

- [x] Build succeeds: `mk.sh sub && mk.sh src`
- [x] Existing 6 PA/PB files still build correctly
- [x] No new compiler warnings

---

## 8. Estimated Impact

### 8.1 Code Reduction

| Target | Before | After | Delta |
|--------|--------|-------|-------|
| 13 process files (definitions) | 13 x ~9 lines | 0 | **-117 lines** |
| sub/fep_common.c | 401 lines | ~384 lines | **-17 lines** |
| sub/fifo_event.c | 0 | ~17 lines | **+17 lines** |
| **Net** | | | **-117 lines** |

### 8.2 Maintenance Impact

- 13 copies → 1 copy: **92% reduction** in maintenance surface for Fifo_Event_Rtn
- Combined with Phase 3's 6 files: 19 total copies → 1 copy (95% reduction)

---

## 9. Next Steps

1. [ ] Write design document with actual code citations
2. [ ] Implement changes
3. [ ] Run gap analysis
4. [ ] Build verification on server

---

## 버전 이력

| 버전 | 날짜 | 변경사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-22 | Initial draft | Claude Code |
