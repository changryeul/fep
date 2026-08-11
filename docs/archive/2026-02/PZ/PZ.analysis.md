# PZ 모듈 코드 품질 감사 -- 간격 분석 보고서

> **분석 유형**: 설계-구현 간격 분석 (PDCA 검증 단계)
>
> **프로젝트**: FEP (KRX Front-End Processor)
> **기능**: PZ (시스템 관리 모듈 코드 품질 감사)
> **분석자**: Claude Code (간격 감지기)
> **날짜**: 2026-02-27
> **설계 문서**: [PZ.design.md](../02-design/features/PZ.design.md)
> **계획 문서**: [PZ.plan.md](../01-plan/features/PZ.plan.md)

---

## 1. 분석 개요

### 1.1 분석 목적

PZ 설계 문서에 지정된 모든 26개 코드 품질 수정 (FR-01부터 FR-27, 제거된 FR-22 제외)이 8개 활성 PZ 소스 파일에서 올바르게 구현되었는지 확인합니다. 이것은 PZ 모듈 코드 품질 감사를 위한 PDCA 사이클의 검증 단계입니다.

### 1.2 분석 범위

- **설계 문서**: `docs/02-design/features/PZ.design.md` (4개 배치에 걸쳐 26개 FR)
- **계획 문서**: `docs/01-plan/features/PZ.plan.md` (27개 발견, FR-22 제거)
- **구현 파일** (8개 활성 소스 파일):
  - `st01/src/PZ/pz_daemon_proc.c`
  - `st01/src/PZ/pz_memory_conf.c`
  - `st01/src/PZ/pz_procchk.c`
  - `st01/src/PZ/pz_memory_proc.c`
  - `st01/src/PZ/pz_fepp.c`
  - `st01/src/PZ/pz_compact.c`
  - `st01/src/PZ/pz_memory.c`
  - `st01/src/PZ/pz_memory_shm.c`
- **분석 날짜**: 2026-02-27

### 1.3 방법론

각 FR에 대해 실제 구현을 설계 문서의 "수정 후" 코드 사양과 비교했습니다. 회귀 없음을 확인하기 위해 전역 검증 스캔이 실행되었습니다 (활성 소스의 이전 패턴의 남은 인스턴스가 0).

---

## 2. 전체 점수

| 카테고리 | 점수 | 상태 |
|----------|:-----:|:------:|
| 설계 일치 | 96% (25/26 FR) | 통과 |
| 회귀 확인 | 100% | 통과 |
| C89 준수 (주석) | 100% | 통과 |
| **전체** | **97%** | **통과** |

---

## 3. 배치 1: CRITICAL 수정 (FR-01부터 FR-07)

모든 7개 CRITICAL 수정이 구현된 것으로 검증되었습니다.

| FR | 파일 | 설계 | 구현 | 상태 |
|----|------|--------|----------------|:------:|
| FR-01 | pz_daemon_proc.c:309 | `[%]` -> `[%s]` | `"cannot change directory[%s] {%d:%s}"` | 일치 |
| FR-02 | pz_memory_conf.c:125 | `strlen(buf)` as `%s` -> `(int)strlen(buf)` as `%d` | `"daemon(%c):[%s] len(%d) line(%d)", cnt, buf, (int)strlen(buf), __LINE__` | 일치 |
| FR-03 | pz_procchk.c:1013 | `defined __hpux \|\| sun \|\| _AIX` -> `defined(__hpux) \|\| defined(sun) \|\| defined(_AIX)` | `#if defined(__hpux) \|\| defined(sun) \|\| defined(_AIX)` | 일치 |
| FR-04 | pz_memory_proc.c:199 | 동일한 전처리기 수정 | `#if defined(__hpux) \|\| defined(sun) \|\| defined(_AIX)` | 일치 |
| FR-05 | pz_memory_proc.c:225 | 동일한 전처리기 수정 | `#if defined(__hpux) \|\| defined(sun) \|\| defined(_AIX)` | 일치 |
| FR-06 | pz_memory_proc.c:280 | 동일한 전처리기 수정 | `#if defined(__hpux) \|\| defined(sun) \|\| defined(_AIX)` | 일치 |
| FR-07 | pz_memory_conf.c:1476 | 동일한 전처리기 수정 | `#if defined(__hpux) \|\| defined(sun) \|\| defined(_AIX)` | 일치 |

### 배치 1 — 전역 회귀 확인

| 패턴 | 활성 소스 개수 | BACKUP 개수 | 상태 |
|---------|:-------------------:|:------------:|:------:|
| `[%]` 형식 문자열 | 0 | 1 | 깨끗함 |
| `strlen(buf)` 전달됨 `%s`에 | 0 | (BACKUP에) | 깨끗함 |
| `#if defined __hpux \|\| sun \|\| _AIX` | 0 | 5 | 깨끗함 |

**배치 1 점수: 7/7 (100%)**

---

## 4. Batch 2: HIGH Fixes (FR-08 through FR-13)

All 6 HIGH fixes verified as implemented.

| FR | File | Design | Implementation | Status |
|----|------|--------|----------------|:------:|
| FR-08 | pz_procchk.c:248-249 | Move `NR = 0` before `continue` | `NR = 0;` at line 248, `continue;` at line 249 | MATCH |
| FR-09 | pz_procchk.c:274 | Add `fclose(fp_b)` after fgets | `fclose (fp_b);` at line 274 inside else branch | MATCH |
| FR-10 | pz_procchk.c:1005-1008 | Add `else { Log(error); return; }` | `else { Log (USR_ERROR, "Report_Save:invalid flag[%d]", flag); return; }` | MATCH |
| FR-11 | pz_fepp.c:405-406 | Wrap `write()` in `if()` with Log | `if (write (FIFO_fd, "1", 1) != 1) Log (FIF_ERROR, ...)` | MATCH |
| FR-12 | pz_fepp.c:785-786 | Wrap `write()` in `if()` with Log | `if (write (FIFO_fd, "1", 1) != 1) Log (FIF_ERROR, ...)` | MATCH |
| FR-13 | pz_daemon_proc.c:978-981 | Wrap `write()` in `if(){}` with Log | `if (p_signo == SIGUSR1) { if (write (DTART_FD, "1", 1) != 1) Log (...); }` | MATCH |

### Verification Details

**FR-08** -- Dead code fix confirmed. In the BACKUP version, `NR = 0` was after `continue` (dead code). Now `NR = 0` is at line 248 before `continue` at line 249.

**FR-09** -- File descriptor leak fixed. `fclose (fp_b);` added at line 274 inside the else branch after `fgets`.

**FR-10** -- Uninitialized cmd guard confirmed. The else clause at lines 1005-1008 returns early with error log for any flag value outside 1-3.

**FR-11/12** -- Both `write()` calls in `pz_fepp.c` now checked. Line 405-406 (daemon exit FIFO) and lines 785-786 (process exit FIFO).

**FR-13** -- Signal handler write check confirmed with braces around the compound statement at lines 978-981.

**Batch 2 Score: 6/6 (100%)**

---

## 5. Batch 3: MEDIUM Fixes (FR-14 through FR-23)

9 of 9 MEDIUM fixes verified (FR-22 was removed by design, not counted).

| FR | File | Design | Implementation | Status |
|----|------|--------|----------------|:------:|
| FR-14 | pz_procchk.c:804 | `Head = NULL` after `free(Head)` | `free(Head); /* SPARROW */ Head = NULL;` at lines 803-804 | MATCH |
| FR-15 | pz_procchk.c:208-215 | One-time atexit via static flag | `static int atexit_registered = 0; if (!atexit_registered) { ... }` | MATCH |
| FR-16 | pz_procchk.c:714 | `r_uid` -> `(pid_t)atoi(dirp->d_name)` | `Ordered_Insert ((pid_t)atoi(dirp->d_name), r_pname);` | MATCH |
| FR-17 | pz_daemon_proc.c:889-890 | Remove `k`, `proc_x_no`, `cmd` | `int pk, rt;` and `long proc_no;` (3 vars removed) | MATCH |
| FR-18 | pz_memory_conf.c | Mid-block decls wrapped in `{ }` | All 3 locations use compound blocks: `{ char temp_buf01[...]; ... }` | MATCH |
| FR-19 | pz_procchk.c:636,641,657,675,685 | `return` -> `continue`, `== 0` -> `== NULL` | All 4 locations now `continue` instead of `return`; `fopen == NULL` | MATCH |
| FR-20 | pz_compact.c:242 | Length-guarded memcmp | `if (strlen(dir->d_name) == 8 && memcmp (dir->d_name, ProcDate, 8) <= 0)` | MATCH |
| FR-21 | pz_memory.c:32-34 | `sprintf` -> `snprintf` with `sizeof` | All 3 lines use `snprintf(..., sizeof(...), ...)` | MATCH |
| FR-23 | pz_memory_conf.c (9 locations) + pz_procchk.c (1 via FR-19) | `fopen == 0` -> `== NULL` | 0 remaining `fopen == 0)` in active source | MATCH |

### Verification Details

**FR-14** -- The `Initialize_Link()` function now has proper null check structure. After `free(Head)` at line 803, `Head = NULL` is set at line 804. Also note the SPARROW comment was converted to C89 style: `/* SPARROW */`.

**FR-15** -- The `atexit()` call is now inside a compound block with `static int atexit_registered = 0;` guard at lines 208-215. This ensures single registration regardless of how many times `Process_Exist_Chk` is called.

**FR-18** -- All three 2025EDIT blocks in `pz_memory_conf.c` use compound blocks (`{ char temp_buf01[...]; ... }`). The design initially said "move declarations to function top" but the accepted alternative was "wrap in `{ }` compound blocks" which is valid C89 (declarations at top of block). Variable names differ slightly from design (`temp_buf001`/`max_tmp_len001` instead of `temp_buf01`/`max_tmp_len01` for the third block in the `flag == 0` branch) but this is functionally identical and acceptable.

**FR-22** -- Confirmed removed from scope. The code at `pz_memory_shm.c:143-149` already uses a valid C89 compound block with `int ver` and `int mag` at block top. No change was needed.

**FR-23** -- All 9 `fopen == 0)` in `pz_memory_conf.c` confirmed converted to `== NULL)`. Verified at lines 61, 666, 939, 1294, 1501, 1600, 1885, 2069, 2279. Zero remaining in active source (grep confirmed all instances only in BACKUP/).

**Batch 3 Score: 9/9 (100%)**

---

## 6. Batch 4: LOW Fixes (FR-24 through FR-27)

3 of 4 LOW fixes fully verified. 1 gap found in FR-24.

| FR | File | Design | Implementation | Status |
|----|------|--------|----------------|:------:|
| FR-24a | pz_memory_shm.c:28 | Remove unused `i` from `Sub_SHM_Creat` | `key_t shm_key = BASE_SHM_KEY;` (no `int i;`) | MATCH |
| FR-24b | pz_memory_shm.c:56 | Remove unused `j` from `Mem_SHM_Creat` | `int i;` (was `int i, j;`), BUT `j` used at line 133 | GAP |
| FR-25 | pz_daemon_proc.c:467,527 | `()` -> `(void)` | `SHM_Load_Process (void)` and `SHM_Backup_Process (void)` | MATCH |
| FR-26 | 5 files, 61 comments | `//` -> `/* */` | Zero `//` comments in active source (all 61 converted) | MATCH |
| FR-27 | pz_procchk.c:197,202,264,266 | `/tmp/mrt1` -> PID-specific path | `tmp_file` var at line 197, `sprintf (tmp_file, "/tmp/mrt1_%d", (int)getpid())` at line 202 | MATCH |

### Gap Detail: FR-24b

**File**: `st01/src/PZ/pz_memory_shm.c`
**Issue**: Variable `j` was removed from line 56 (`int i, j;` -> `int i;`) per the design which stated "Variable `j` is declared but never used." However, `j` IS actually used at lines 133 and 135:

```c
/* Line 133 */ for (j = 0; j < Info[i].data_count; j++)
/* Line 135 */     Data_Ptr[j] = Shmptr;
```

**Root Cause**: Design error -- the original audit incorrectly classified `j` as unused. The `for` loop using `j` exists deep within the function body (line 133 in a ~175-line function), and the design analysis missed it.

**Impact**: HIGH -- This will cause a compilation error on the target server because `j` is referenced but not declared. No global or header-level `j` declaration exists.

**Recommended Fix**: Restore `j` to the declaration:
```c
int     i, j;
```

### Verification Details

**FR-24a** -- `Sub_SHM_Creat` at line 25-44: The old `int i;` declaration is gone. The function now only declares `key_t shm_key = BASE_SHM_KEY;`. Variable `i` was genuinely unused in this function.

**FR-25** -- Both function definitions confirmed with `(void)` parameter:
- Line 467: `void    SHM_Load_Process (void)`
- Line 527: `void    SHM_Backup_Process (void)`

**FR-26** -- Zero C++ `//` comments found in active PZ source files. All matches from grep are exclusively in `BACKUP/` directory. The 61 conversions across 5 files are complete:
- pz_memory_conf.c: 41 comments converted (2025EDIT markers, data comments)
- pz_daemon_proc.c: 9 comments converted (KSW markers, 2025EDIT)
- pz_procchk.c: 6 comments converted (SPARROW markers)
- pz_memory_shm.c: 3 comments converted (numeric comments)
- pz_fepp.c: 2 comments converted (date markers)

**FR-27** -- The hardcoded `/tmp/mrt1` is now PID-specific. At line 197, `tmp_file[64]` is declared. At line 202, `sprintf (tmp_file, "/tmp/mrt1_%d", (int)getpid())` creates the unique path. The tmp_file is used at line 264 (sprintf system command) and line 266 (fopen). Zero remaining `/tmp/mrt1` hardcoded paths in active source (only in BACKUP/).

**Batch 4 Score: 3/4 (75%)**

---

## 7. Deferred Items Verification

The design specified 2 intentionally deferred items (D-01, D-02). Confirmed these are NOT in scope and remain unchanged.

| ID | Issue | Status |
|----|-------|--------|
| D-01 | `system("rm -rf ...")` in pz_compact.c:246 | Confirmed deferred -- `system (tmp)` still used with `rm -rf` |
| D-02 | sprintf overflow in pz_memory_conf.c 2025EDIT blocks | Confirmed deferred -- strncpy patterns remain as-is |

---

## 8. Differences Found

### 8.1 Missing Features (Design O, Implementation X)

None. All 26 designed FRs have corresponding implementation changes.

### 8.2 Added Features (Design X, Implementation O)

None. No undocumented changes were found beyond the design specification.

### 8.3 Changed Features (Design != Implementation)

| Item | Design | Implementation | Impact |
|------|--------|----------------|--------|
| FR-24b: `j` in `Mem_SHM_Creat` | "Variable `j` is declared but never used. Change to `int i;`" | `j` removed, but used at lines 133,135 | HIGH -- compilation error |
| FR-18: Variable names | `temp_buf01/02/03` | `temp_buf01/temp_buf001/temp_buf02/temp_buf03` (minor naming variation) | NONE -- functionally equivalent |

---

## 9. 요약 통계

```
설계의 전체 FR:     26 (거짓 양성으로 FR-22 제거)
완전히 일치:           25
발견된 간격:               1
일치율:              96.2%

배치별:
  배치 1 (CRITICAL):   7/7   100%
  배치 2 (HIGH):       6/6   100%
  배치 3 (MEDIUM):     9/9   100%
  배치 4 (LOW):        3/4    75%

회귀 확인:
  [%] 형식 문자열:      남은 것 0 (깨끗함)
  전처리기 defined():  남은 것 0 (깨끗함)
  fopen == 0:              남은 것 0 (깨끗함)
  C++ // 주석:         남은 것 0 (깨끗함)
  /tmp/mrt1 하드코딩:     남은 것 0 (깨끗함)
```

---

## 10. 권장 조치

### 10.1 즉시 (빌드 전 필요)

| 우선순위 | 항목 | 파일 | 조치 |
|----------|------|------|--------|
| 높음 | FR-24b: `j` 선언 복원 | `st01/src/PZ/pz_memory_shm.c:56` | `int i;`을 `int i, j;`로 변경 |

### 10.2 설계 문서 업데이트

| 항목 | 조치 |
|------|--------|
| FR-24b 설명 | 설계를 업데이트하여 `j`가 실제로 사용됨 (133,135라인)을 기록하고 제거되면 안 됨 |

### 10.3 빌드 검증

빌드 검증은 테스트되지 않음 (서버 환경 필요). FR-24b 간격은 빌드를 차단하는 컴파일 오류를 발생시킵니다. 배포 전에 수정해야 합니다.

---

## 11. 결론

PZ 모듈 코드 품질 감사 구현은 **96.2% 일치율** (25/26 FR)을 달성합니다. 모든 CRITICAL, HIGH, MEDIUM 수정이 올바르게 구현되었으며 회귀 없음. 단일 간격 (FR-24b)은 변수 `j`가 사용되지 않는 것으로 잘못 분류된 설계 오류입니다. 수정은 간단합니다 (`int i, j;` 복원).

FR-24b를 수정한 후 예상 일치율은 **100%** (26/26)입니다.

연기된 항목 D-01 및 D-02는 설계된 대로 유지되어 있으며 향후 별도의 작업이 필요합니다.

---

## 버전 이력

| 버전 | 날짜 | 변경 사항 | 작성자 |
|---------|------|---------|--------|
| 1.0 | 2026-02-27 | 초기 간격 분석, 26개 FR 검증, 1개 간격 발견 (FR-24b) | Claude Code |
