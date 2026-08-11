# 설계: unsafe-strcpy-conversion

## 참조

- Plan: `docs/01-plan/features/unsafe-strcpy-conversion.plan.md`

## 요약

Convert 54 active `strcpy()` calls to `strncpy()` with explicit bounds, and delete 10 dead-code `strcpy` blocks (already replaced by active strncpy code) in 2 files. 합계: 64 grep matches resolved across 11 files.

## Conversion Pattern

```c
/* Before */
strcpy(dest, src);

/* After */
strncpy(dest, src, sizeof(dest) - 1);
dest[sizeof(dest) - 1] = '\0';
```

For pointer parameters (no sizeof), use the known buffer constant (e.g., `IFNAMSIZ`).

## 기능 요구사항

### 배치 1: Dead code deletion — Group A (10 blocks in 2 files)

The `strcpy(path, full_path)` calls are inside `/* */` block comments. Active code already uses `strncpy`. Delete the entire commented-out blocks.

| FR | 파일 | Lines | Action |
|----|------|-------|--------|
| FR-01 | `src/PZ/pz_daemon_proc.c` | 327-338, 395-406, 465-476, 537-548, 609-620 | Delete 5 `/* ... */` blocks containing old strcpy code |
| FR-02 | `src/PZ/pz_fepp.c` | 314-325, 482-493, 611-622, 774-785, 935-946 | Delete 5 `/* ... */` blocks containing old strcpy code |

### 배치 2: Config string parsing — Group B (25 calls in 3 files)

Pattern: `if (sp2 == NULL) strcpy(tmp2, sp1+1);` where `tmp2[40]`.

| FR | 파일 | Calls | Lines |
|----|------|:-----:|-------|
| FR-03 | `src/PX/px_cfgback.c` | 8 | 206, 348, 507, 650, 856, 998, 1166, 1308 |
| FR-04 | `src/PX/px_cfgload.c` | 8 | 241, 393, 562, 715, 931, 1083, 1261, 1413 |
| FR-05 | `src/PX/px_memok.c` | 9 | 120, 380, 527, 691, 840, 1007, 1166, 1352, 1501 |

Conversion:
```c
/* Before */
if (sp2 == NULL)    strcpy (tmp2, sp1+1);
/* After */
if (sp2 == NULL)    { strncpy(tmp2, sp1+1, sizeof(tmp2) - 1); tmp2[sizeof(tmp2) - 1] = '\0'; }
```

### 배치 3: Environment string — Group C (3 calls in 1 file)

Pattern: `strcpy(env_str, "REAL1")` where `env_str[10]`.

| FR | 파일 | Calls | Lines |
|----|------|:-----:|-------|
| FR-06 | `src/PX/px_cfgload.c` | 3 | 127, 129, 131 |

Conversion:
```c
/* Before */
strcpy (env_str, "REAL1");
/* After */
strncpy(env_str, "REAL1", sizeof(env_str) - 1);
env_str[sizeof(env_str) - 1] = '\0';
```

### 배치 4: FIFO name assignment — Group D (12 calls in 2 files)

Pattern: `strcpy(INFO(...).exit_FIFO_name, temp_buf)` where FIFO fields are `char[20]`.

| FR | 파일 | Calls | Lines |
|----|------|:-----:|-------|
| FR-07 | `src/PZ/pz_memory_conf.c` | 6 | 375, 389, 422, 423, 437, 438 |
| FR-08 | `sub/config_db.c` | 6 | 539, 549, 587, 588, 598, 599 |

Conversion: Use `sizeof(field)` where the destination is a struct field accessed directly, or `sizeof(INFO(...).exit_FIFO_name)` for macro-accessed fields. For pz_memory_conf.c, `temp_buf01/02/03` are already declared with `sizeof(field)` so use the same.

### 배치 5: 파일/data name copy — Group E (6 calls in 2 files)

Pattern: `strcpy(old_file, IFN(...))` where `old_file[20]`.

| FR | 파일 | Calls | Lines |
|----|------|:-----:|-------|
| FR-09 | `src/PX/px_setfname.c` | 3 | 137, 140, 148 |
| FR-10 | `src/PX/px_setdname.c` | 3 | 137, 140, 148 |

Conversion:
```c
strncpy(old_file, IFN(Dk,Pk,k), sizeof(old_file) - 1);
old_file[sizeof(old_file) - 1] = '\0';
```

### 배치 6: Process/path name — Group F (3 calls in 1 file)

| FR | 파일 | Line | Before | After | Notes |
|----|------|:----:|--------|-------|-------|
| FR-11a | `src/PZ/pz_procchk.c` | 832 | `strcpy(aptr->exec_name, exec_name)` | Keep as-is | Buffer is `malloc(strlen+1)` — safe by construction |
| FR-11b | `src/PZ/pz_procchk.c` | 901 | `strcpy(Dname[0], _FEP_DAT)` | `strncpy(Dname[0], _FEP_DAT, sizeof(Dname[0]) - 1); Dname[0][sizeof(Dname[0]) - 1] = '\0';` | `Dname[2][128]` |
| FR-11c | `src/PZ/pz_procchk.c` | 902 | `strcpy(Dname[1], _FEP_LOG)` | `strncpy(Dname[1], _FEP_LOG, sizeof(Dname[1]) - 1); Dname[1][sizeof(Dname[1]) - 1] = '\0';` | `Dname[2][128]` |

Note: FR-11a left unchanged — malloc'd to exact size, strncpy adds no value.

### 배치 7: Network interface — Group G (5 calls in 1 file)

| FR | 파일 | Line | Before | After | Notes |
|----|------|:----:|--------|-------|-------|
| FR-12a | `src/PB/pb_7100_ur.c` | 90 | `strcpy(IpAddr, "10.38.111.79")` | `strncpy(IpAddr, "10.38.111.79", sizeof(IpAddr) - 1); IpAddr[sizeof(IpAddr) - 1] = '\0';` | `IpAddr[20]` |
| FR-12b | `src/PB/pb_7100_ur.c` | 95 | `strcpy(IpAddr, "10.37.11.61")` | `strncpy(IpAddr, "10.37.11.61", sizeof(IpAddr) - 1); IpAddr[sizeof(IpAddr) - 1] = '\0';` | `IpAddr[20]` |
| FR-12c | `src/PB/pb_7100_ur.c` | 510 | `strcpy(ifnm, ifa->ifa_name)` | `strncpy(ifnm, ifa->ifa_name, IFNAMSIZ - 1); ifnm[IFNAMSIZ - 1] = '\0';` | param, use `IFNAMSIZ` |
| FR-12d | `src/PB/pb_7100_ur.c` | 517 | `strcpy(candidate10, ifa->ifa_name)` | `strncpy(candidate10, ifa->ifa_name, sizeof(candidate10) - 1); candidate10[sizeof(candidate10) - 1] = '\0';` | `candidate10[IFNAMSIZ]` |
| FR-12e | `src/PB/pb_7100_ur.c` | 523 | `strcpy(ifnm, candidate10)` | `strncpy(ifnm, candidate10, IFNAMSIZ - 1); ifnm[IFNAMSIZ - 1] = '\0';` | param, use `IFNAMSIZ` |

## Implementation Order

1. Batch 1 (dead code deletion) — FR-01, FR-02
2. Batch 2 (config parsing) — FR-03, FR-04, FR-05
3. Batch 3 (env string) — FR-06
4. Batch 4 (FIFO names) — FR-07, FR-08
5. Batch 5 (file/data names) — FR-09, FR-10
6. Batch 6 (process names) — FR-11b, FR-11c (skip FR-11a)
7. Batch 7 (network) — FR-12a through FR-12e

## 검증

- `grep -rn 'strcpy' st01/src/ st01/sub/` (excluding BACKUP/.back/.org/BACK2025/JC_OLD) should return:
  - 1 match: `pz_procchk.c:832` (FR-11a, kept as-is)
  - 0 matches in all other active files
- Zero `strcpy` remaining in `/* */` block comments in pz_daemon_proc.c and pz_fepp.c

## Counts 요약

| 범주 | Calls | Action |
|----------|:-----:|--------|
| Dead code deletion | 10 | Delete `/* */` blocks |
| Active -> strncpy | 53 | Convert |
| Safe by construction | 1 | Keep (FR-11a) |
| **Total** | **64** | |

## 비기능 요구사항

- Zero functional change
- C89 compatible (`strncpy` is standard C89)
- Null-termination explicitly guaranteed on every conversion
