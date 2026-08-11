# structured-logging Analysis Report

> **Analysis Type**: Gap Analysis (Design vs Implementation)
>
> **Project**: FEP (Front-End Processor for KRX)
> **Analyst**: gap-detector agent
> **Date**: 2026-03-01
> **Design Doc**: [structured-logging.design.md](../02-design/features/structured-logging.design.md)
> **Plan Doc**: [structured-logging.plan.md](../01-plan/features/structured-logging.plan.md)

---

## 1. Analysis Overview

### 1.1 Analysis Purpose

Compare the structured-logging design document against the actual implementation
to verify all 6 functional requirements (FR-01 through FR-06) and 5 design
sections (3.1 through 3.5) are correctly implemented.

### 1.2 Analysis Scope

- **Design Document**: `docs/02-design/features/structured-logging.design.md`
- **Implementation Files**:
  - `st01/sub/log_proc.c` (core implementation)
  - `st01/env/pkg_env.sh` (environment variable comment)
- **Analysis Date**: 2026-03-01

---

## 2. FR-Level Gap Analysis

### FR-01: Log() Structured Output

| Item | Design | Implementation | Status |
|------|--------|----------------|--------|
| Branch condition | `if (is_structured_log())` at line 164 | `if (is_structured_log())` at line 174 | PASS |
| Structured format string | `"%.4s-%.2s-%.2s %02d:%02d:%02d.%06ld\|%s\|%s\|%s\|%s\|%.*s\n"` | Identical format at lines 175-180 | PASS |
| Legacy format preserved | `snprintf(msg, sizeof(msg), "[%-15.15s,...]")` in else branch | Identical at lines 182-185 | PASS |
| Field order | DATETIME\|PROCESS\|TYPE\|LEVEL\|ERRCODE\|MSG | Matches: `sys_date` parts, `_Exe_Name`, `err_type`, `err_level`, `error_cd`, `buf` | PASS |

**Result**: FR-01 PASS

### FR-02: Write_SLog() Structured Output

| Item | Design | Implementation | Status |
|------|--------|----------------|--------|
| Branch condition | `if (is_structured_log())` at line 523 | `if (is_structured_log())` at line 551 | PASS |
| Structured format string | `"%.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.6s\|%.10s\|%s\|%s\|%.4s\|%.*s\n"` | Identical format at lines 552-557 | PASS |
| Legacy format preserved | `sprintf(msg, "[%-10.10s ...]")` in else branch | Identical at lines 559-561 | PASS |
| Field sources | `hd->Time`, `hd->LogName`, `hd->ErrCd` (%.4s) | All correctly used with `%.2s` pointer arithmetic | PASS |
| `sprintf` to `snprintf` upgrade | Structured branch uses `snprintf(msg, sizeof(msg), ...)` | Confirmed at line 552 | PASS |
| SLog() internal unchanged | SHM binary record format must NOT change | SLog() (lines 377-463) has no structured branching | PASS |

**Result**: FR-02 PASS

### FR-03: Date Field Addition

| Item | Design | Implementation | Status |
|------|--------|----------------|--------|
| Log() date format | `%.4s-%.2s-%.2s` from `sys_date` (YYYYMMDD -> YYYY-MM-DD) | `sys_date, sys_date+4, sys_date+6` at line 177 | PASS |
| Write_SLog() date format | Same `%.4s-%.2s-%.2s` from `sys_date` | `sys_date, sys_date+4, sys_date+6` at line 554 | PASS |
| ISO 8601 compatible | `YYYY-MM-DD HH:MM:SS.usec` | Output example matches design Section 7 | PASS |
| No extra buffer needed | Design says "no extra buffer" | No additional `char[]` declarations | PASS |

**Result**: FR-03 PASS

### FR-04: Error Code Numeric Field

| Item | Design | Implementation | Status |
|------|--------|----------------|--------|
| Log() error code | `%s` from `error_cd` (4-digit string) | `error_cd` at line 179 | PASS |
| Write_SLog() error code | `%.4s` from `hd->ErrCd` (char[4], not null-terminated) | `hd->ErrCd` with `%.4s` at line 556 | PASS |
| Field position | 5th field (between LEVEL and MSG) | Confirmed in format string | PASS |

**Result**: FR-04 PASS

### FR-05: Environment Variable Mode Switch

| Item | Design | Implementation | Status |
|------|--------|----------------|--------|
| Static variable | `static int log_structured = -1;` after line 19 | Identical at line 20 | PASS |
| Function signature | `static int is_structured_log(void)` | Identical at line 22 | PASS |
| Lazy initialization | `if (log_structured < 0)` -> `getenv("FEP_LOG_FORMAT")` | Identical at lines 24-26 | PASS |
| Exact match logic | `strcmp(env, "structured") == 0` | Identical at line 26 | PASS |
| NULL check | `env &&` before `strcmp` | Identical at line 26 | PASS |
| Default = legacy | Non-matching or missing env -> `0` (legacy) | Ternary `? 1 : 0` at line 26 | PASS |
| Caching | Single `getenv()` call, then cached in `log_structured` | Confirmed by control flow | PASS |
| pkg_env.sh comment | Usage comment for `FEP_LOG_FORMAT` | Lines 28-29: comment + commented-out export | PASS |

**Result**: FR-05 PASS

### FR-06: Emergency Log Structured Format

| Item | Design | Implementation | Status |
|------|--------|----------------|--------|
| Log() emergency branch | `if (is_structured_log())` at design line 216 | Lines 235-247 in implementation | PASS |
| ANSI escape removal | No `\033[3%dm` or `\033[0m` in structured branch | Confirmed: structured branch has clean format | PASS |
| BEL character removal | No `err_beep` in structured branch | Confirmed: `err_beep` only in legacy else branch | PASS |
| Color index removal | No `p_err_no % 100` color index | Confirmed: not in structured branch | PASS |
| Write_SLog() emergency branch | `if (is_structured_log())` at design line 573 | Lines 610-623 in implementation | PASS |
| Write_SLog() ANSI removal | Same clean format in structured branch | Confirmed at lines 611-616 | PASS |

**Result**: FR-06 PASS

---

## 3. Design Section Match Analysis

### 3.1 Mode Switch (Design Section 3.1)

Design specifies a static variable and lazy-init function at `log_proc.c` line 19+.

**Implementation** (lines 20-29):
```c
static int      log_structured = -1; /* -1:uninitialized, 0:legacy, 1:structured */

static int is_structured_log(void)
{
    if (log_structured < 0) {
        char *env = getenv("FEP_LOG_FORMAT");
        log_structured = (env && strcmp(env, "structured") == 0) ? 1 : 0;
    }
    return log_structured;
}
```

**Comparison**: Character-for-character match with design pseudocode. Comment style
is slightly abbreviated (`-1:uninitialized` vs `-1: uninitialized`) -- functionally
identical, cosmetic only.

**Status**: PASS

### 3.2 Log() Normal Output (Design Section 3.2)

Design specifies `if/else` block wrapping the existing `snprintf` at "Change Point A".

**Implementation** (lines 173-187):
```c
/* 2025EDIT */
if (is_structured_log()) {
    snprintf(msg, sizeof(msg),
            "%.4s-%.2s-%.2s %02d:%02d:%02d.%06ld|%s|%s|%s|%s|%.*s\n",
            sys_date, sys_date+4, sys_date+6,
            date->tm_hour, date->tm_min, date->tm_sec, tv.tv_usec,
            _Exe_Name, err_type, err_level, error_cd,
            (int)strlen(buf), buf);
} else {
    snprintf(msg, sizeof(msg), "[%-15.15s,%02d:%02d:%02d.%06ld,%s,%s]%.*s\n",
            _Exe_Name, date->tm_hour, date->tm_min, date->tm_sec,
            tv.tv_usec, err_type, err_level,
            (int)strlen(buf), buf);
}
/* 2025EDIT */
```

**Comparison**: Format string, argument order, and else branch all match design
exactly. The `/* 2025EDIT */` markers are implementation bookmarks not in design
-- acceptable annotation.

**Status**: PASS

### 3.3 Log() Emergency Output (Design Section 3.3)

Design specifies `if/else` block at "Change Point B" removing ANSI escapes and BEL.

**Implementation** (lines 234-248): Matches design exactly. Structured branch
outputs clean pipe-delimited format. Legacy branch preserves `err_beep`, `\033`
color codes, and `p_err_no % 100` color index.

**Status**: PASS

### 3.4 Write_SLog() Normal Output (Design Section 3.4)

Design specifies `if/else` block at "Change Point C" with `%.10s` for `hd->LogName`
and `%.4s` for `hd->ErrCd`.

**Implementation** (lines 551-562): Format string matches design exactly.
The `snprintf` upgrade (from `sprintf`) in the structured branch is confirmed.
The legacy else branch preserves the original `sprintf` call.

**Status**: PASS

### 3.5 Write_SLog() Emergency Output (Design Section 3.5)

Design specifies `if/else` block at "Change Point D".

**Implementation** (lines 609-623): Matches design exactly. Structured branch
uses clean format, legacy branch preserves ANSI escapes and BEL character.

**Status**: PASS

---

## 4. Unchanged Functions Verification

Design Section 6 specifies functions that must NOT be modified.

| Function | Design: Unchanged | Implementation | Status |
|----------|-------------------|----------------|--------|
| `SLog()` | SHM binary format must not change | Lines 377-463: no `is_structured_log()` calls, binary record format intact | PASS |
| `Log_Proc()` | File I/O logic (open/write/close) | Lines 264-286: unchanged | PASS |
| `Log_Emergency()` | Emergency file I/O logic | Lines 296-318: unchanged | PASS |
| `Log_Save()` | 60MB rotation logic | Lines 328-363: unchanged | PASS |
| `LOG_HEAD_SIZE` | Legacy mode constant | Not modified (used implicitly in legacy branches) | PASS |
| `SHM_LOG_HEAD_SIZE` | SHM record parsing constant | Not modified (still used in Write_SLog) | PASS |

Note: The `/* 2025EDIT */` marker on line 435 (inside `SLog()`) marks the existing
`snprintf` call for the SHM binary record. This is an annotation only -- the actual
format string is unchanged from the original design.

---

## 5. Environment Variable Documentation

| Item | Design | Implementation (pkg_env.sh) | Status |
|------|--------|----------------------------|--------|
| Comment explaining usage | Required | Line 28: `# FEP_LOG_FORMAT: set "structured" for pipe-delimited structured log format` | PASS |
| Commented-out export example | Implied by plan | Line 29: `# export FEP_LOG_FORMAT=structured` | PASS |
| Location | After existing exports | Between `subname` (line 27) and `case $osname` (line 31) | PASS |

---

## 6. Call Site Analysis

Design states: "Callers 2,010 locations: `Log(err_no, fmt, ...)` signature unchanged."

| Item | Verification | Status |
|------|-------------|--------|
| `Log()` signature | `void Log(int p_err_no, const char *p_fmt, ...)` -- unchanged at line 53 | PASS |
| `SLog()` signature | `void SLog(int p_err_no, const char *p_fmt, ...)` -- unchanged at line 377 | PASS |
| `Write_SLog()` signature | `void Write_SLog(char *p_msg)` -- unchanged at line 472 | PASS |
| No caller changes needed | API-compatible: format change is internal to output | PASS |

---

## 7. Match Rate Summary

### FR-Level Summary

| FR | Description | Status |
|----|-------------|--------|
| FR-01 | Log() structured output | PASS |
| FR-02 | Write_SLog() structured output | PASS |
| FR-03 | Date field addition (ISO 8601) | PASS |
| FR-04 | Error code numeric field | PASS |
| FR-05 | Environment variable mode switch | PASS |
| FR-06 | Emergency log structured format | PASS |

**FR Match**: 6/6 (100%)

### Design Section Summary

| Section | Description | Status |
|---------|-------------|--------|
| 3.1 | Mode switch (`is_structured_log`) | PASS |
| 3.2 | Log() normal output (Change Point A) | PASS |
| 3.3 | Log() emergency output (Change Point B) | PASS |
| 3.4 | Write_SLog() normal output (Change Point C) | PASS |
| 3.5 | Write_SLog() emergency output (Change Point D) | PASS |

**Section Match**: 5/5 (100%)

### Verification Summary

| Category | Items | Passed | Failed | Rate |
|----------|:-----:|:------:|:------:|:----:|
| Functional Requirements (FR) | 6 | 6 | 0 | 100% |
| Design Sections (3.x) | 5 | 5 | 0 | 100% |
| Unchanged Functions | 6 | 6 | 0 | 100% |
| Env Var Documentation | 3 | 3 | 0 | 100% |
| API Compatibility | 4 | 4 | 0 | 100% |
| **Total** | **24** | **24** | **0** | **100%** |

```
+---------------------------------------------+
|  Overall Match Rate: 100%  (24/24)           |
+---------------------------------------------+
|  FR Match:           6/6   (100%)            |
|  Design Match:       5/5   (100%)            |
|  Unchanged Check:    6/6   (100%)            |
|  Env Var Doc:        3/3   (100%)            |
|  API Compat:         4/4   (100%)            |
+---------------------------------------------+
```

---

## 8. Gaps Found

None. All design requirements are fully implemented as specified.

---

## 9. Minor Observations (Non-Gap)

These are cosmetic or informational notes, not deviations from design.

| # | Observation | Location | Impact |
|---|-------------|----------|--------|
| 1 | `/* 2025EDIT */` markers added around changed blocks | log_proc.c:173,187,234,248,435,609 | None -- useful code annotation for future maintenance |
| 2 | Line numbers shifted from design (e.g., design "line 164" -> impl line 174) | Throughout | Expected: the static variable + function insertion at top shifted all subsequent lines by ~10 |
| 3 | Comment style abbreviated (`-1:uninitialized` vs `-1: uninitialized`) | log_proc.c:20 | Cosmetic only |

---

## 10. Build Verification

| Item | Status |
|------|--------|
| Compilation test | NOT TESTED (requires server environment with libfepP.a build chain) |
| Runtime test | NOT TESTED (requires FEP runtime environment) |

---

## 11. Recommendation

**PASS** -- Design and implementation match at 100%. No iteration needed.

The implementation faithfully reproduces every aspect of the design document:
- All 6 FRs implemented exactly as specified
- All 5 design change points applied correctly
- Format strings are character-for-character matches
- Unchanged functions remain untouched
- Environment variable documentation is in place
- API compatibility is preserved (zero caller impact)

**Next step**: `/pdca report structured-logging`

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2026-03-01 | Initial gap analysis | gap-detector agent |
