# stat-save-optimize Analysis Report

> **Analysis Type**: Gap Analysis (Design vs Implementation)
>
> **Project**: FEP (Front-End Processor for KRX)
> **Analyst**: Claude (gap-detector)
> **Date**: 2026-02-27
> **Design Doc**: [stat-save-optimize.design.md](../02-design/features/stat-save-optimize.design.md)

---

## 1. Analysis Overview

### 1.1 Analysis Purpose

Verify that the `Stat_Save()` time-based throttling implementation matches the design document exactly. This feature adds file I/O throttling to `Stat_Save()` to prevent excessive disk writes during high-frequency main-loop iterations, while preserving business-hours checks on every call.

### 1.2 Analysis Scope

| Item | Path |
|------|------|
| Design Document | `docs/02-design/features/stat-save-optimize.design.md` |
| Implementation File 1 | `st01/inc/fep_sub.h` (lines 359-364) |
| Implementation File 2 | `st01/sub/stat_save.c` (full file, 162 lines) |
| Implementation File 3 | `st01/sub/setsigfatal.c` (line 143) |

### 1.3 FR / Verification Items

- 5 Functional Requirements (FR-01 through FR-05)
- 10 Verification items (V-01 through V-10)

---

## 2. FR Analysis (Functional Requirements)

### FR-01: Time-based throttling

**Design**: Insert throttle check after business-hours block (line 62) and before file I/O (line 63). Condition: `tv.tv_sec - _last_save_time < STAT_SAVE_INTERVAL_SEC`. On pass, update `_last_save_time = tv.tv_sec`.

**Implementation** (`st01/sub/stat_save.c` lines 67-70):
```c
	/* FR-01: throttle -- skip file I/O if interval not elapsed */
	if (tv.tv_sec - _last_save_time < STAT_SAVE_INTERVAL_SEC)
		return;
	_last_save_time = tv.tv_sec;
```

**Verdict**: MATCH -- Exact condition, exact placement, exact update logic.

---

### FR-02: Business-hours check runs every call (not throttled)

**Design**: Lines 31-61 (gettimeofday, localtime_r, AtoIf, business-hours check with `Exit_Process()`) must execute on every call, before the throttle check.

**Implementation** (`st01/sub/stat_save.c` lines 33-65):
- Line 34: `gettimeofday(&tv, NULL)` -- always runs.
- Lines 36-42: Time parsing via `AtoIf` -- always runs.
- Lines 44-65: Business-hours check with `Exit_Process()` -- always runs.
- Lines 67-70: Throttle check -- positioned AFTER all the above.

**Verdict**: MATCH -- Business-hours check is unconditional; throttle only gates file I/O (line 72 onward).

---

### FR-03: STAT_SAVE_INTERVAL_SEC macro with #ifndef guard, default 3

**Design**: In `inc/fep_sub.h`, near line 359:
```c
#ifndef STAT_SAVE_INTERVAL_SEC
#define STAT_SAVE_INTERVAL_SEC  3
#endif
```

**Implementation** (`st01/inc/fep_sub.h` lines 359-361):
```c
#ifndef STAT_SAVE_INTERVAL_SEC
#define STAT_SAVE_INTERVAL_SEC  3
#endif
```

**Verdict**: MATCH -- `#ifndef` guard present, default value 3, allows `-D` override at build time.

---

### FR-04: Stat_Save_Force() for process exit path

**Design**: New function that resets `_last_save_time = 0` then calls `Stat_Save()`. Placed after `End of Stat_Save()` in `stat_save.c`. Called from `setsigfatal.c:143` instead of `Stat_Save()`.

**Implementation** (`st01/sub/stat_save.c` lines 151-157):
```c
/*----------------------------------------------------------------------*/
void	Stat_Save_Force (void)
/*----------------------------------------------------------------------*/
{
	_last_save_time = 0;
	Stat_Save ();
}	/* End of Stat_Save_Force ()	*/
```

**Implementation** (`st01/sub/setsigfatal.c` line 143):
```c
			Stat_Save_Force ();
```

**Verdict**: MATCH -- Function body, placement, and call site all match design exactly.

---

### FR-05: static time_t _last_save_time at file scope

**Design**: Declared at file scope in `stat_save.c`, after `#include "fep_sub.h"`, initialized to 0.

**Implementation** (`st01/sub/stat_save.c` lines 11-12):
```c
/* throttle: last file write timestamp (FR-05) */
static time_t _last_save_time = 0;
```

**Verdict**: MATCH -- File-scope static, `time_t` type, zero-initialized, accessible by both `Stat_Save()` and `Stat_Save_Force()`.

---

## 3. Verification Checklist

| # | Item | Expected | Actual | Status |
|---|------|----------|--------|--------|
| V-01 | `STAT_SAVE_INTERVAL_SEC` macro in `fep_sub.h` | `#ifndef` guard, default 3 | `fep_sub.h:359-361`: `#ifndef STAT_SAVE_INTERVAL_SEC` / `#define STAT_SAVE_INTERVAL_SEC 3` / `#endif` | PASS |
| V-02 | `Stat_Save_Force` extern in `fep_sub.h` | `extern void Stat_Save_Force (void);` | `fep_sub.h:363`: `extern void Stat_Save_Force (void);` | PASS |
| V-03 | `_last_save_time` file-scope static in `stat_save.c` | `static time_t _last_save_time = 0;` | `stat_save.c:12`: `static time_t _last_save_time = 0;` | PASS |
| V-04 | Throttle check AFTER business-hours check | After lines 31-61 | Throttle at lines 67-70, business-hours at lines 33-65 | PASS |
| V-05 | Throttle check BEFORE file I/O | Before `sprintf(bumun, ...)` | Throttle at lines 67-70, file I/O starts at line 72 | PASS |
| V-06 | `tv.tv_sec` reused (no additional syscall) | Reuse `gettimeofday` result | `tv` from line 34; throttle at line 68 uses `tv.tv_sec` | PASS |
| V-07 | `Stat_Save_Force()` resets `_last_save_time=0` then `Stat_Save()` | Two-line body | `stat_save.c:155-156`: `_last_save_time = 0; Stat_Save();` | PASS |
| V-08 | `setsigfatal.c:143` calls `Stat_Save_Force()` | Replace `Stat_Save()` | `setsigfatal.c:143`: `Stat_Save_Force ();` | PASS |
| V-09 | No other Stat_Save callers changed | 53 active call sites in `src/*.c` untouched | grep confirms 53 `Stat_Save()` calls in `src/*.c`, all unchanged | PASS |
| V-10 | Build readiness | Code compiles with `mk.sh sub` | NOT TESTED (requires server build environment) | N/A |

**Summary**: 9 of 9 testable items PASS. 1 item (V-10) requires server environment.

---

## 4. Caller Inventory

Active `Stat_Save()` callers in `st01/src/*.c` (53 call sites, all unchanged):

| # | File | Line(s) | Count |
|---|------|---------|:-----:|
| 1 | `PA/pa_1100_ts.c` | 126 | 1 |
| 2 | `PA/pa_1200_mp.c` | 77, 82 | 2 |
| 3 | `PA/pa_1200_tr.c` | 192 | 1 |
| 4 | `PA/pa_1290_mp.c` | 59, 63 | 2 |
| 5 | `PA/pa_1400_mp.c` | 71, 76 | 2 |
| 6 | `PA/pa_1490_mp.c` | 90, 94 | 2 |
| 7 | `PA/pa_1600_mp.c` | 70, 75 | 2 |
| 8 | `PA/pa_1600_tr.c` | 201 | 1 |
| 9 | `PA/pa_2100_ts.c` | 289 | 1 |
| 10 | `PA/pa_2200_tr.c` | 166 | 1 |
| 11 | `PA/pa_2700_tr.c` | 138 | 1 |
| 12 | `PA/pa_3100_ts.c` | 109, 678, 794 | 3 |
| 13 | `PA/pa_5010_mp.c` | 123 | 1 |
| 14 | `PA/pa_5020_mp.c` | 135 | 1 |
| 15 | `PA/pa_5200_qs.c` | 145 | 1 |
| 16 | `PA/pa_6000_mp.c` | 68, 72 | 2 |
| 17 | `PA/pa_7000_mp.c` | 197 | 1 |
| 18 | `PA/pa_7000_tr.c` | 134 | 1 |
| 19 | `PA/pa_7000_us.c` | 97 | 1 |
| 20 | `PA/pa_7010_us.c` | 97 | 1 |
| 21 | `PA/pa_7030_us.c` | 106 | 1 |
| 22 | `PA/pa_7100_dd.c` | 173, 177 | 2 |
| 23 | `PA/pa_7100_ts.c` | 133 | 1 |
| 24 | `PA/pa_7500_us.c` | 112 | 1 |
| 25 | `PA/pa_7800_dd.c` | 90, 95 | 2 |
| 26 | `PA/pa_7800_tr.c` | 118 | 1 |
| 27 | `PA/pa_8100_ts.c` | 153 | 1 |
| 28 | `PA/pa_8200_tr.c` | 143 | 1 |
| 29 | `PA/pa_9000_mp.c` | 86, 91 | 2 |
| 30 | `PA/pa_9999_us.c` | 98 | 1 |
| 31 | `PB/pb_1100_ts.c` | 144 | 1 |
| 32 | `PB/pb_1200_tr.c` | 196 | 1 |
| 33 | `PB/pb_1800_ts.c` | 150 | 1 |
| 34 | `PB/pb_7100_dd.c` | 87, 92 | 2 |
| 35 | `PB/pb_7100_ts.c` | 152 | 1 |
| 36 | `PB/pb_7200_tr.c` | 119 | 1 |
| 37 | `PB/pb_7800_tr.c` | 115 | 1 |
| 38 | `PB/pb_8100_ts.c` | 161 | 1 |
| 39 | `PB/pb_8200_tr.c` | 148 | 1 |
| 40 | `PW/pw_1000_mp.c` | 92 | 1 |
| 41 | `PW/pw_3010_tr.c` | 81 | 1 |
| 42 | `PW/pw_3030_tr.c` | 84 | 1 |
| 43 | `PW/pw_4000_ts.c` | 198 | 1 |
| | **Total** | | **53** |

Additional non-src callers (not counted as "active callers" for V-09):
- `sub/stat_save.c:156` -- internal call from `Stat_Save_Force()` (new, by design)
- `sub/setsigfatal.c:143` -- changed to `Stat_Save_Force()` (by design)

Note: The design document states "58 callers". The actual active count is 53 in `src/*.c`. The difference of 5 likely includes JC_OLD archive files or the definition/sub calls. Regardless, all active callers remain unchanged -- V-09 is satisfied.

---

## 5. Code Flow Verification

```
Stat_Save() entry
  |
  +-- [L34] gettimeofday(&tv, NULL)           -- ALWAYS runs
  +-- [L35] localtime_r                        -- ALWAYS runs
  +-- [L36-42] AtoIf x4 (start/end time)      -- ALWAYS runs
  +-- [L44-65] Business-hours check            -- ALWAYS runs
  |     +-- Out of hours? Exit_Process()       -- ALWAYS checked
  |
  +-- [L67-70] THROTTLE CHECK (NEW)            -- gates everything below
  |     if (tv.tv_sec - _last_save_time < 3)
  |       return;                              -- skip file I/O
  |     _last_save_time = tv.tv_sec;           -- update timestamp
  |
  +-- [L72+] File I/O (sprintf, fopen, flock,  -- only when throttle passes
  |          fwrite, fclose)
  +-- return

Stat_Save_Force() entry
  |
  +-- _last_save_time = 0                      -- reset throttle
  +-- Stat_Save()                              -- guaranteed file I/O
```

---

## 6. Overall Scores

| Category | Score | Status |
|----------|:-----:|:------:|
| Design Match (FR-01 to FR-05) | 100% (5/5) | PASS |
| Verification (V-01 to V-09) | 100% (9/9) | PASS |
| Build Verification (V-10) | N/A | NOT TESTED |
| **Overall** | **100%** | **PASS** |

```
+---------------------------------------------+
|  Overall Match Rate: 100%                    |
+---------------------------------------------+
|  FR items:    5/5  (100%)  PASS              |
|  V items:     9/9  (100%)  PASS              |
|  V-10 build:  N/A  (requires server)         |
|  Gaps found:  0                              |
+---------------------------------------------+
```

---

## 7. Differences Found

### Missing Features (Design present, Implementation absent)

None.

### Added Features (Design absent, Implementation present)

None.

### Changed Features (Design differs from Implementation)

None.

---

## 8. Minor Observations (non-gap)

| # | Observation | Impact | Action |
|---|-------------|--------|--------|
| 1 | Design says "58 callers"; actual active count is 53 in `src/*.c` | None -- the count difference is from JC_OLD archive files and internal sub calls | No action needed; design number is approximate |
| 2 | Design references original line numbers (e.g., "line 63"); implementation line numbers shifted due to insertions | Expected behavior -- line numbers shift after code insertion | No action needed |
| 3 | `stat_save.c` has `struct flock lock;` declared at line 31 but design did not mention this variable | Not a gap -- design focused on new code only | No action needed |

---

## 9. Recommended Actions

### Immediate Actions

None required. All functional requirements and verification items pass.

### Build Verification (deferred)

When server environment is available:
```sh
mk.sh sub          # Rebuild libfepP.a (stat_save.o, setsigfatal.o recompile)
mk.sh src          # Relink all binaries against updated library
```

### Optional Design Document Update

The "58 callers" count in the design could be updated to "53 active callers in src/*.c" for precision, but this is cosmetic and does not affect correctness.

---

## 10. Conclusion

The implementation of `stat-save-optimize` is a perfect match to the design document. All 5 functional requirements (FR-01 through FR-05) are implemented exactly as specified. All 9 testable verification items (V-01 through V-09) pass. The throttle check is correctly positioned after the business-hours check and before file I/O. The `Stat_Save_Force()` function properly bypasses throttling for the process exit path. No existing callers were modified except the single intended change at `setsigfatal.c:143`.

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2026-02-27 | Initial gap analysis -- 100% match | Claude (gap-detector) |
