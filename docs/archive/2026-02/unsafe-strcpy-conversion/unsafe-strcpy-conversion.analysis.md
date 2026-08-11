# unsafe-strcpy-conversion 분석 Report

> **분석 Type**: Gap 분석 (설계 vs Implementation)
>
> **프로젝트**: FEP (Front-End Processor for KRX)
> **Analyst**: gap-detector
> **날짜**: 2026-02-22
> **설계 Doc**: [unsafe-strcpy-conversion.design.md](../02-design/features/unsafe-strcpy-conversion.design.md)

---

## 1. 분석 개요

### 1.1 분석 목적

Verify that all 64 `strcpy()` calls identified in the design document have been properly addressed: 10 dead-code blocks deleted, 53 active calls converted to bounded `strncpy()` with explicit null-termination, and 1 call intentionally kept (malloc'd exact-size buffer).

### 1.2 분석 범위

- **설계 Document**: `docs/02-design/features/unsafe-strcpy-conversion.design.md`
- **Implementation Paths**: `st01/src/PB/`, `st01/src/PX/`, `st01/src/PZ/`, `st01/sub/`
- **Files Examined**: 11 active source files across 7 batches
- **분석 날짜**: 2026-02-22

---

## 2. Overall Scores

| 범주 | Score | 상태 |
|----------|:-----:|:------:|
| Design| 일치 | 100% | PASS |
| Architecture Compliance | 100% | PASS |
| Convention Compliance | 100% | PASS |
| **Overall** | **100%** | **PASS** |

---

## 3. Detailed FR 검증

### Batch 1: Dead Code Deletion (FR-01, FR-02)

| FR | 파일 | Design Action | Verified | Notes |
|----|------|---------------|:--------:|-------|
| FR-01 | `st01/src/PZ/pz_daemon_proc.c` | Delete 5 `/* */` blocks with old strcpy | PASS | `grep strcpy` returns 0 matches in file. All 5 fork/exec functions now use active `strncpy(path, full_path, sizeof(path))` code only. File is 1031 lines, no commented-out strcpy remnants. |
| FR-02 | `st01/src/PZ/pz_fepp.c` | Delete 5 `/* */` blocks with old strcpy | PASS | `grep strcpy` returns 0 matches in file. All 5 fork/exec functions (Creat_SHM_Process, Daemon_SHM_Process, File_Compact_Process, Start_Process, Proc_Check_Process) use active strncpy code only. File is 951 lines. |

### Batch 2: Config String Parsing (FR-03, FR-04, FR-05)

Pattern verified: `if (sp2 == NULL) { strncpy(tmp2, sp1+1, sizeof(tmp2) - 1); tmp2[sizeof(tmp2) - 1] = '\0'; }`

| FR | 파일 | Calls | Lines Verified | 상태 |
|----|------|:-----:|----------------|:------:|
| FR-03 | `st01/src/PX/px_cfgback.c` | 8 | 206, 348, 507, 650, 856, 998, 1166, 1308 | PASS |
| FR-04 | `st01/src/PX/px_cfgload.c` | 8 | 241, 393, 562, 715, 931, 1083, 1261, 1413 | PASS |
| FR-05 | `st01/src/PX/px_memok.c` | 9 | 120, 380, 527, 691, 840, 1007, 1166, 1352, 1501 | PASS |

All 25 calls converted with identical `{ strncpy(..., sizeof(tmp2) - 1); tmp2[sizeof(tmp2) - 1] = '\0'; }` pattern. Null-termination explicitly guaranteed on every site.

### Batch 3: Environment String (FR-06)

| FR | 파일 | Calls | Lines Verified | 상태 |
|----|------|:-----:|----------------|:------:|
| FR-06 | `st01/src/PX/px_cfgload.c` | 3 | 127, 129, 131 | PASS |

Line 127: `strncpy(env_str, "REAL1", sizeof(env_str) - 1); env_str[sizeof(env_str) - 1] = '\0';`
Line 129: `strncpy(env_str, "REAL2", sizeof(env_str) - 1); env_str[sizeof(env_str) - 1] = '\0';`
Line 131: `strncpy(env_str, "TEST", sizeof(env_str) - 1); env_str[sizeof(env_str) - 1] = '\0';`

All 3 conversions match design exactly. `env_str[10]` bounds used via `sizeof(env_str) - 1`.

### Batch 4: FIFO Name Assignment (FR-07, FR-08)

| FR | 파일 | Calls | 상태 | Notes |
|----|------|:-----:|:------:|-------|
| FR-07 | `st01/src/PZ/pz_memory_conf.c` | 6 | PASS | Lines 375, 390, 409-410, 424-427, 441-444. All 6 FIFO name assignments use `strncpy(..., sizeof(...) - 1)` with explicit null-termination. Implementation uses intermediate temp_buf variables with size-aware construction before strncpy into struct fields. |
| FR-08 | `st01/sub/config_db.c` | 6 | PASS | Lines 539, 550, 589-592, 602-605. All 6 FIFO name assignments use `strncpy(..., sizeof(...) - 1)` with explicit null-termination. Implementation uses local `tbuf[20]` with `mlen = sizeof(tbuf) - 6` for suffix concatenation safety. |

### Batch 5: File/Data Name Copy (FR-09, FR-10)

| FR | 파일 | Calls | 상태 | Notes |
|----|------|:-----:|:------:|-------|
| FR-09 | `st01/src/PX/px_setfname.c` | 3 | PASS | Lines 137, 141, 150: `strncpy(old_file, IFN/OFN(...), sizeof(old_file) - 1); old_file[sizeof(old_file) - 1] = '\0';` -- also old_fifo converted at line 141. |
| FR-10 | `st01/src/PX/px_setdname.c` | 3 | PASS | Lines 137, 141, 150: `strncpy(old_dshm, IDN/ODN(...), sizeof(old_dshm) - 1); old_dshm[sizeof(old_dshm) - 1] = '\0';` -- also old_fifo converted at line 141. |

### Batch 6: Process/Path Names (FR-11a, FR-11b, FR-11c)

| FR | 파일 | Line | 상태 | Notes |
|----|------|:----:|:------:|-------|
| FR-11a | `st01/src/PZ/pz_procchk.c` | 832 | PASS (kept) | `strcpy(aptr->exec_name, exec_name)` retained as designed. Buffer is `malloc(strlen(exec_name) + 1)` at line 825 -- safe by construction. |
| FR-11b | `st01/src/PZ/pz_procchk.c` | 901 | PASS | `strncpy(Dname[0], _FEP_DAT, sizeof(Dname[0]) - 1); Dname[0][sizeof(Dname[0]) - 1] = '\0';` -- `Dname[2][128]` bounds used. |
| FR-11c | `st01/src/PZ/pz_procchk.c` | 903 | PASS | `strncpy(Dname[1], _FEP_LOG, sizeof(Dname[1]) - 1); Dname[1][sizeof(Dname[1]) - 1] = '\0';` -- `Dname[2][128]` bounds used. |

### Batch 7: Network Interface (FR-12a through FR-12e)

| FR | 파일 | Line | 상태 | Notes |
|----|------|:----:|:------:|-------|
| FR-12a | `st01/src/PB/pb_7100_ur.c` | 90 | PASS | `strncpy(IpAddr, "10.38.111.79", sizeof(IpAddr) - 1); IpAddr[sizeof(IpAddr) - 1] = '\0';` |
| FR-12b | `st01/src/PB/pb_7100_ur.c` | 96 | PASS | `strncpy(IpAddr, "10.37.11.61", sizeof(IpAddr) - 1); IpAddr[sizeof(IpAddr) - 1] = '\0';` |
| FR-12c | `st01/src/PB/pb_7100_ur.c` | 512 | PASS | `strncpy(ifnm, ifa->ifa_name, IFNAMSIZ - 1); ifnm[IFNAMSIZ - 1] = '\0';` -- uses IFNAMSIZ as designed for pointer param. |
| FR-12d | `st01/src/PB/pb_7100_ur.c` | 520 | PASS | `strncpy(candidate10, ifa->ifa_name, sizeof(candidate10) - 1); candidate10[sizeof(candidate10) - 1] = '\0';` |
| FR-12e | `st01/src/PB/pb_7100_ur.c` | 527 | PASS | `strncpy(ifnm, candidate10, IFNAMSIZ - 1); ifnm[IFNAMSIZ - 1] = '\0';` -- uses IFNAMSIZ as designed. |

---

## 4. Global 검증

### 4.1 strcpy Residual Scan

Command equivalent: `grep -rn 'strcpy' st01/src/ st01/sub/` (excluding .back, BACKUP/, .org)

| Source | Matches | Expected | 상태 |
|--------|:-------:|:--------:|:------:|
| `st01/src/` active .c files | 1 | 1 | PASS |
| `st01/sub/` active .c files | 0 | 0 | PASS |

The single remaining match is:
- `st01/src/PZ/pz_procchk.c:832` -- FR-11a, intentionally kept (malloc'd exact-size buffer)

### 4.2 Backup 파일 Residuals

The following files contain `strcpy` but are backup/old versions (NOT counted as gaps):
- `st01/src/PX/px_cfgload.back` (9 matches)
- `st01/src/PX/px_cfgback.back` (9 matches)
- `st01/src/PX/px_memok.back` (10 matches)
- `st01/src/PZ/BACKUP/pz_memory_conf.c` (7 matches)
- `st01/src/PZ/BACKUP/pz_daemon_proc.c` (1 match)
- `st01/src/PZ/BACKUP/pz_procchk.c` (3 matches)

These are expected -- .back and BACKUP/ files preserve pre-conversion state.

### 4.3 Comment Block 검증

| 파일 | `strcpy` inside `/* */` | 상태 |
|------|:-----------------------:|:------:|
| `st01/src/PZ/pz_daemon_proc.c` | 0 | PASS |
| `st01/src/PZ/pz_fepp.c` | 0 | PASS |

---

## 5. Counts 요약

| 범주 | Design Count | Actual Count | 상태 |
|----------|:-----------:|:------------:|:------:|
| Dead code blocks deleted | 10 | 10 | PASS |
| Active calls converted to strncpy | 53 | 53 | PASS |
| Safe by construction (kept) | 1 | 1 | PASS |
| **Total** | **64** | **64** | **PASS** |

---

## 6. Differences Found

### Missing Features (설계 O, Implementation X)

None.

### Added Features (설계 X, Implementation O)

None.

### Changed Features (설계 != Implementation)

| 항목 | 설계 | Implementation | 영향 |
|------|--------|----------------|--------|
| FR-12b line number | 95 | 96 | None (minor line shift, same code) |
| FR-12c line number | 510 | 512 | None (minor line shift, same code) |
| FR-12d line number | 517 | 520 | None (minor line shift, same code) |
| FR-12e line number | 523 | 527 | None (minor line shift, same code) |

All differences are cosmetic line-number shifts due to prior edits. The code content matches design exactly.

---

## 7. 비기능 요구사항

| 요구사항 | 상태 | Notes |
|-------------|:------:|-------|
| Zero functional change | PASS | Pure defensive conversion; no logic change |
| C89 compatible | PASS | `strncpy` is standard C89 |
| Null-termination guaranteed | PASS | Every conversion includes explicit `dest[sizeof(dest)-1] = '\0'` |

---

## 8. Recommended Actions

### 일치 Rate >= 90%: No Actions Required

설계 and implementation match perfectly (100%). All 12 FR items verified across 11 files and 7 batches.

The only remaining `strcpy` in the entire `st01/src/` and `st01/sub/` active codebase is `pz_procchk.c:832`, which is safe by construction and intentionally preserved.

---

## 9. 다음 단계

- [ ] Build verification on server (NOT TESTED -- requires server)
- [ ] Write completion report (`unsafe-strcpy-conversion.report.md`)
- [ ] Archive PDCA documents

---

## 버전 기록

| 버전 | 날짜 | 변경사항 | 저자 |
|---------|------|---------|--------|
| 1.0 | 2026-02-22 | Initial gap analysis -- 100% match | gap-detector |
