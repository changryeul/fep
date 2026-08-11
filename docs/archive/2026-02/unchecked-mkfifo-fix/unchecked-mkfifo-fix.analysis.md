# unchecked-mkfifo-fix 분석 Report

> **분석 Type**: Gap 분석 (설계 vs Implementation)
>
> **프로젝트**: FEP (Front-End Processor for KRX)
> **Analyst**: gap-detector agent
> **날짜**: 2026-02-25
> **설계 Doc**: [unchecked-mkfifo-fix.design.md](../02-design/features/unchecked-mkfifo-fix.design.md)

---

## 1. 분석 개요

### 1.1 분석 목적

Verify that all 22 Functional Requirements (FR-01 through FR-22) from the design document have been correctly implemented. The feature replaces 30 unchecked `mknod()+chmod()` / `mkdir()+chmod()` pairs with error-checked shared helpers `Create_FIFO()` and `Create_Dir()`, modernizing `mknod()` to `mkfifo()` (POSIX.1-2008).

### 1.2 분석 범위

- **설계 Document**: `docs/02-design/features/unchecked-mkfifo-fix.design.md`
- **Implementation Files**:
  - `st01/sub/file_util.c` (NEW)
  - `st01/inc/fep_sub.h` (modified)
  - `st01/sub/config_db.c` (modified)
  - `st01/src/PZ/pz_memory_conf.c` (modified)
  - `st01/src/PZ/pz_memory_proc.c` (modified)
- **분석 날짜**: 2026-02-25

---

## 2. FR-by-FR Gap Analysis

### 배치 1: Helper Functions

| FR | 설계 | 요구사항 | Implementation | 상태 | Notes |
|----|-------------------|----------------|:------:|-------|
| FR-01 | `sub/file_util.c`: New file with `Create_FIFO()` and `Create_Dir()` | File exists at `st01/sub/file_util.c` (52 lines). Both functions present. | PASS | See detail below |
| FR-02 | `inc/fep_sub.h`: Add extern declarations after Set_Socket_Linger block (~L351) | Declarations at L372-373: `extern int Create_FIFO(const char *);` and `extern int Create_Dir(const char *);` | PASS | Placed after `Poll_File` at end of function prototype block |

**FR-01 Detail Verification**:

| Sub-item | 설계 | Implementation | 일치 |
|----------|--------|----------------|:-----:|
| Uses `mkfifo()` not `mknod()` | `mkfifo(path, 0777)` | `mkfifo(path, 0777)` at L20 | PASS |
| Tolerates EEXIST | `if (errno != EEXIST)` | `if (errno != EEXIST)` at L22 | PASS |
| Logs with SYS_FATAL | `Log(SYS_FATAL, "mkfifo failed [%s] errno=%d:%s", ...)` | Identical at L24-25 | PASS |
| Returns -1 on error, 0 on success/EEXIST | `return -1` / `return 0` | L26 / L30 | PASS |
| `chmod(path, 0777)` always called | After if-block, before return 0 | L29 | PASS |
| `Create_Dir()` uses `mkdir(path, 0777)` | Per design | L41 | PASS |
| `Create_Dir()` EEXIST + SYS_FATAL + chmod | Per design | L42-50 | PASS |
| Includes: errno.h, sys/types.h, sys/stat.h, stdio.h, string.h | Design says `fep_fepp.h` | Includes `fep_sub.h` | PASS |

Note on include: 설계 specified `fep_fepp.h` but implementation uses `fep_sub.h`. This is correct -- 38 of 47 `sub/` source files use `fep_sub.h` (the standard convention). `fep_sub.h` transitively includes all required headers: `errno.h` (L32), `stdio.h` (L34), `sys/stat.h` (L43), `sys/types.h` (L44), `string.h` (L88/91/95).

### 배치 2: config_db.c -- FIFO replacements (9 pairs)

| FR | 설계 | Implementation | 상태 | Evidence |
|----|--------|----------------|:------:|----------|
| FR-03 | L557-573: 4x `mknod+chmod` -> 4x `Create_FIFO(file_name)` (flag==0 daemon FIFOs) | 4 calls at L557, L561, L565, L569 | PASS | 0 mknod remaining |
| FR-04 | L611-622: 3x `mknod+chmod` -> 3x `Create_FIFO(file_name)` (flag==1 daemon FIFOs) | 3 calls at L607, L611, L615 | PASS | 0 mknod remaining |
| FR-05 | L965-966: 1x `mknod+chmod` -> `Create_FIFO(file_name)` (file.ini FIFO loop) | 1 call at L957 | PASS | 0 mknod remaining |
| FR-06 | L1258-1259: 1x `mknod+chmod` -> `Create_FIFO(file_name)` (dshm FIFO loop) | 1 call at L1248 | PASS | 0 mknod remaining |

**검증**: `grep -c 'Create_FIFO' st01/sub/config_db.c` = **9** (matches design expectation of 9)
**검증**: `grep -c 'mknod' st01/sub/config_db.c` = **0** (matches design expectation of 0)

### 배치 3: config_db.c -- Directory replacements (3 pairs)

| FR | 설계 | Implementation | 상태 | Evidence |
|----|--------|----------------|:------:|----------|
| FR-07 | L867-868: `mkdir+chmod` -> `Create_Dir(file_name)` (file data dir) | 1 call at L860 | PASS | |
| FR-08 | L1178-1179: `mkdir+chmod` -> `Create_Dir(file_name)` (dshm data dir) | 1 call at L1169 | PASS | |
| FR-09 | L1709-1710: `mkdir+chmod` -> `Create_Dir(dir_name)` (proc stat dir) | 1 call at L1698 | PASS | |

**검증**: `grep -c 'Create_Dir' st01/sub/config_db.c` = **3** (matches design expectation of 3)

### 배치 4: pz_memory_conf.c -- FIFO replacements (9 pairs)

| FR | 설계 | Implementation | 상태 | Evidence |
|----|--------|----------------|:------:|----------|
| FR-10 | L365-401: 4x `mknod+chmod` -> 4x `Create_FIFO(file_name)` (flag==0 daemon FIFOs) | 4 calls at L365, L379, L393, L397 | PASS | 0 mknod remaining |
| FR-11 | L414-449: 3x `mknod+chmod` -> 3x `Create_FIFO(file_name)` (flag==1 daemon FIFOs) | 3 calls at L410, L426, L442 | PASS | 0 mknod remaining |
| FR-12 | L775-776: 1x `mknod+chmod` -> `Create_FIFO(file_name)` (file FIFO loop) | 1 call at L767 | PASS | 0 mknod remaining |
| FR-13 | L1083-1084: 1x `mknod+chmod` -> `Create_FIFO(file_name)` (dshm FIFO loop) | 1 call at L1073 | PASS | 0 mknod remaining |

**검증**: `grep -c 'Create_FIFO' st01/src/PZ/pz_memory_conf.c` = **9** (matches design expectation of 9)
**검증**: `grep -c 'mknod' st01/src/PZ/pz_memory_conf.c` = **0** (matches design expectation of 0)

### 배치 5: pz_memory_conf.c -- Directory replacements (4 pairs)

| FR | 설계 | Implementation | 상태 | Evidence |
|----|--------|----------------|:------:|----------|
| FR-14 | L693-694: `mkdir+chmod` -> `Create_Dir(file_name)` (file data/log dir) | 1 call at L686 | PASS | |
| FR-15 | L1009-1010: `mkdir+chmod` -> `Create_Dir(file_name)` (dshm data dir) | 1 call at L1000 | PASS | |
| FR-16 | L1465-1467: `rt = mkdir(); if (rt==0) chmod()` -> `Create_Dir(ppath)` (CISAM data dir) | 1 call at L1454 (`Create_Dir(ppath)`) | PASS | Partial check upgraded |
| FR-17 | L2332-2333: `mkdir+chmod` -> `Create_Dir(dir_name)` (proc stat dir) | 1 call at L2319 | PASS | |

**검증**: `grep -c 'Create_Dir' st01/src/PZ/pz_memory_conf.c` = **4** (matches design expectation of 4)

### 배치 6: pz_memory_proc.c -- Directory replacements (5 pairs)

| FR | 설계 | Implementation | 상태 | Evidence |
|----|--------|----------------|:------:|----------|
| FR-18 | L121-122: `mkdir+chmod` -> `Create_Dir(file_name)` (work date dir); preserve Log on L123 | Call at L121; `Log(SYS_OK, "work date directory (%s) created", ...)` preserved at L122 | PASS | |
| FR-19 | L132-133: `mkdir+chmod` -> `Create_Dir(file_name2)` (data dir); preserve Log on L134 | Call at L131; `Log(SYS_OK, "data directory (%s) created", ...)` preserved at L132 | PASS | |
| FR-20 | L218-219: `mkdir+chmod` -> `Create_Dir(file_name5)` (move work date dir) | Call at L216 inside `if (rt == -1)` block | PASS | |
| FR-21 | L252-253: `mkdir+chmod` -> `Create_Dir(file_name6)` (data dir reset, unchecked) | Call at L249 (no stat guard, as expected) | PASS | |
| FR-22 | L310-311: `mkdir+chmod` -> `Create_Dir(file_name9)` (log dir); preserve Log on L312 | Call at L306; `Log(SYS_OK, "log directory (%s) created", ...)` preserved at L307 | PASS | |

**검증**: `grep -c 'Create_Dir' st01/src/PZ/pz_memory_proc.c` = **5** (matches design expectation of 5)

---

## 3. Global 검증 Checks

### 3.1 No Remaining mknod() in Active Source

```
grep -rn 'mknod\s*(' st01/sub/ st01/src/  (excluding BACKUP/)
```

| Location | Matches | 상태 |
|----------|:-------:|:------:|
| `st01/sub/` active source | 0 | PASS |
| `st01/src/` active source | 0 | PASS |
| `st01/src/PZ/BACKUP/pz_memory_conf.c` | 9 | Expected (backup file) |

Only match in `st01/sub/file_util.c` is in a comment: "Uses mkfifo() (POSIX) instead of deprecated mknod()." -- not an active call.

### 3.2 No Remaining S_IFIFO in Active Source

```
grep -rn 'S_IFIFO' st01/sub/ st01/src/  (excluding BACKUP/)
```

| Location | Matches | 상태 |
|----------|:-------:|:------:|
| `st01/sub/` active source | 0 | PASS |
| `st01/src/` active source | 0 | PASS |
| `st01/src/PZ/BACKUP/pz_memory_conf.c` | 9 | Expected (backup file) |

설계 stated "should only appear in `sub/file_util.c`" but `file_util.c` does not reference `S_IFIFO` at all since it uses `mkfifo()`. This is correct -- `mkfifo()` does not require `S_IFIFO`.

### 3.3 Create_FIFO Uses mkfifo() Not mknod()

`st01/sub/file_util.c` L20: `if (mkfifo(path, 0777) == -1)` -- confirmed POSIX `mkfifo()`.

### 3.4 Both Helpers Tolerate EEXIST

| Function | EEXIST check | Log on real error | chmod on both paths | 상태 |
|----------|:------------:|:-----------------:|:-------------------:|:------:|
| Create_FIFO | L22: `if (errno != EEXIST)` | L24: `Log(SYS_FATAL, ...)` | L29: `chmod(path, 0777)` | PASS |
| Create_Dir | L43: `if (errno != EEXIST)` | L45: `Log(SYS_FATAL, ...)` | L50: `chmod(path, 0777)` | PASS |

### 3.5 Build 검증

Build requires server environment (`mk.sh sub && mk.sh src`). **NOT TESTED** (macOS development host, requires HP-UX/Linux target).

---

## 4. Call Count 요약

| 파일 | Create_FIFO | Create_Dir | Design Expected | 상태 |
|------|:-----------:|:----------:|:---------------:|:------:|
| `st01/sub/config_db.c` | 9 | 3 | 9 FIFO + 3 Dir | PASS |
| `st01/src/PZ/pz_memory_conf.c` | 9 | 4 | 9 FIFO + 4 Dir | PASS |
| `st01/src/PZ/pz_memory_proc.c` | 0 | 5 | 0 FIFO + 5 Dir | PASS |
| **Total** | **18** | **12** | **18 + 12 = 30** | PASS |

---

## 5. Overall Score

| FR | 설명 | 결과 |
|:--:|-------------|:------:|
| FR-01 | file_util.c with Create_FIFO + Create_Dir | PASS |
| FR-02 | fep_sub.h extern declarations | PASS |
| FR-03 | config_db.c: 4x daemon FIFO (flag==0) | PASS |
| FR-04 | config_db.c: 3x daemon FIFO (flag==1) | PASS |
| FR-05 | config_db.c: file.ini FIFO loop | PASS |
| FR-06 | config_db.c: dshm FIFO loop | PASS |
| FR-07 | config_db.c: file data dir | PASS |
| FR-08 | config_db.c: dshm data dir | PASS |
| FR-09 | config_db.c: proc stat dir | PASS |
| FR-10 | pz_memory_conf.c: 4x daemon FIFO (flag==0) | PASS |
| FR-11 | pz_memory_conf.c: 3x daemon FIFO (flag==1) | PASS |
| FR-12 | pz_memory_conf.c: file FIFO loop | PASS |
| FR-13 | pz_memory_conf.c: dshm FIFO loop | PASS |
| FR-14 | pz_memory_conf.c: file data/log dir | PASS |
| FR-15 | pz_memory_conf.c: dshm data dir | PASS |
| FR-16 | pz_memory_conf.c: CISAM data dir (partial check upgraded) | PASS |
| FR-17 | pz_memory_conf.c: proc stat dir | PASS |
| FR-18 | pz_memory_proc.c: work date dir (Log preserved) | PASS |
| FR-19 | pz_memory_proc.c: data dir (Log preserved) | PASS |
| FR-20 | pz_memory_proc.c: move work date dir | PASS |
| FR-21 | pz_memory_proc.c: data dir reset (unchecked) | PASS |
| FR-22 | pz_memory_proc.c: log dir (Log preserved) | PASS |

```
Match Rate: 22 / 22 = 100%

PASS: 22   FAIL: 0   N/A: 0
```

---

## 6. 비기능 요구사항 검증

| NFR | 요구사항 | 상태 | Evidence |
|-----|-------------|:------:|----------|
| Zero functional change on success path | FIFOs/dirs created with 0777 | PASS | `mkfifo(path, 0777)` / `mkdir(path, 0777)` |
| EEXIST silently tolerated with chmod | chmod always called | PASS | L29/L50 in file_util.c |
| Real failures logged SYS_FATAL | errno + strerror in log | PASS | L24-25/L45-46 in file_util.c |
| mknod -> mkfifo modernization | POSIX.1-2008 compliance | PASS | `mkfifo()` used, no `mknod()` in active source |
| C89 compatible | mkfifo is POSIX.1 | PASS | Available on all target platforms |
| Build auto-detects file_util.c | Make_Lib_P_c.sh iterates sub/*.c | PASS | Existing build convention |
| Net line change ~0 | +52 (helpers) - ~60 (removed pairs) | PASS | Slight net reduction |

---

## 7. Differences Found

### Missing Features (설계 O, Implementation X)

None.

### Added Features (설계 X, Implementation O)

None.

### Changed Features (설계 != Implementation)

| 항목 | 설계 | Implementation | 영향 |
|------|--------|----------------|--------|
| Include header in file_util.c | `fep_fepp.h` | `fep_sub.h` | None -- `fep_sub.h` is the standard convention for 38/47 sub/ files and provides all required headers |

This is a trivial documentation difference, not a functional gap. The implementation choice is actually more consistent with the existing codebase convention.

---

## 8. Recommended Actions

### Immediate Actions

None required. All 22 FRs pass.

### Documentation Update

1. **Minor**: Design doc FR-01 mentions `fep_fepp.h` as include; implementation uses the more conventional `fep_sub.h`. Consider updating design doc to match.

### Build 검증

1. Build test (`mk.sh sub && mk.sh src`) should be run on the target server to confirm linkage.

---

## 9. 결론

**일치 Rate: 100% -- 설계 and implementation match perfectly.**

All 30 unchecked `mknod()+chmod()` / `mkdir()+chmod()` pairs have been replaced with error-checked `Create_FIFO()` and `Create_Dir()` helpers across 3 source files. The helpers correctly use POSIX `mkfifo()`, tolerate `EEXIST`, always `chmod(0777)`, and log `SYS_FATAL` on real errors. Zero `mknod()` calls and zero `S_IFIFO` references remain in active source code.

---

## 버전 기록

| 버전 | 날짜 | 변경사항 | 저자 |
|---------|------|---------|--------|
| 1.0 | 2026-02-25 | Initial gap analysis -- 22/22 FR PASS (100%) | gap-detector |
