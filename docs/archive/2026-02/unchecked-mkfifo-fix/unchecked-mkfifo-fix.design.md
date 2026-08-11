# 설계: unchecked-mkfifo-fix

## 참조

- Plan: `docs/01-plan/features/unchecked-mkfifo-fix.plan.md`

## 요약

Replace 30 unchecked `mknod()+chmod()` / `mkdir()+chmod()` pairs with error-checked shared helpers `Create_FIFO()` and `Create_Dir()` across 3 source files. Modernize `mknod()` to `mkfifo()` (POSIX.1-2008). Tolerate `EEXIST` for daemon restart scenarios. Log real failures via `Log(SYS_FATAL, ...)`.

**Correction from plan**: Actual count is 18 mknod + 12 mkdir = 30 pairs (plan counted 19+10=29, missing config_db.c:1178 and pz_memory_conf.c:1009).

## Current Pattern

All FIFO creation sites follow this unchecked pattern:
```c
mknod (file_name, S_IFIFO | 0777, 0);
chmod (file_name, 0777);
```

All directory creation sites follow this unchecked pattern:
```c
mkdir (file_name, 0777);
chmod (file_name, 0777);
```

One exception — `pz_memory_conf.c:1465` has a partial check:
```c
rt = mkdir (ppath, 0777);
if (rt == 0)
    chmod (ppath, 0777);
```

**Problems**:
- Neither `mknod()` nor `mkdir()` return values are checked
- If creation fails (permissions, filesystem full, parent missing), no error is logged
- `mknod()` for FIFOs is marked obsolescent in POSIX.1-2008; `mkfifo()` is preferred
- The partial check at L1465 skips chmod on EEXIST (existing file may have wrong permissions)

## Helper Functions 설계

### Create_FIFO()

```c
/*
 * Create_FIFO: Create named pipe (FIFO) with error checking.
 * Uses mkfifo() (POSIX) instead of deprecated mknod().
 * Tolerates EEXIST (daemon restart). Always ensures 0777 mode.
 * Returns 0 on success/EEXIST, -1 on error (logged).
 */
int Create_FIFO(const char *path)
{
    if (mkfifo(path, 0777) == -1)
    {
        if (errno != EEXIST)
        {
            Log(SYS_FATAL, "mkfifo failed [%s] errno=%d:%s",
                path, errno, strerror(errno));
            return -1;
        }
    }
    chmod(path, 0777);
    return 0;
}
```

### Create_Dir()

```c
/*
 * Create_Dir: Create directory with error checking.
 * Tolerates EEXIST (daemon restart). Always ensures 0777 mode.
 * Returns 0 on success/EEXIST, -1 on error (logged).
 */
int Create_Dir(const char *path)
{
    if (mkdir(path, 0777) == -1)
    {
        if (errno != EEXIST)
        {
            Log(SYS_FATAL, "mkdir failed [%s] errno=%d:%s",
                path, errno, strerror(errno));
            return -1;
        }
    }
    chmod(path, 0777);
    return 0;
}
```

**설계 decisions**:
- `chmod()` always called (on success AND EEXIST) — preserves original behavior where both lines execute unconditionally, and ensures correct permissions on restart
- Return value provided but not required — callers may ignore (same resilience as before) or check for critical paths
- `strerror(errno)` included in log for diagnostics
- `SYS_FATAL` severity matches other system-level failures in the codebase

## 기능 요구사항

### 배치 1: Helper functions — `sub/file_util.c` + `inc/fep_sub.h`

| FR | 파일 | Action |
|----|------|--------|
| FR-01 | `sub/file_util.c` | **New file**: Create `Create_FIFO()` and `Create_Dir()` with includes (`stdio.h`, `string.h`, `errno.h`, `sys/types.h`, `sys/stat.h`, `fep_fepp.h`) |
| FR-02 | `inc/fep_sub.h` | Add `extern int Create_FIFO(const char *);` and `extern int Create_Dir(const char *);` after the `Set_Socket_Linger` declaration block (around L351) |

### 배치 2: config_db.c — Replace mknod+chmod (9 pairs)

| FR | Lines | Context | Before → After |
|----|:-----:|---------|----------------|
| FR-03 | 557-573 | daemon FIFOs, flag==0 | 4 × `mknod+chmod` → 4 × `Create_FIFO(file_name)` |
| FR-04 | 611-622 | daemon FIFOs, flag==1 | 3 × `mknod+chmod` → 3 × `Create_FIFO(file_name)` |
| FR-05 | 965-966 | file.ini FIFO loop | `mknod+chmod` → `Create_FIFO(file_name)` |
| FR-06 | 1258-1259 | dshm FIFO loop | `mknod+chmod` → `Create_FIFO(file_name)` |

Each replacement changes:
```c
/* Before */
mknod (file_name, S_IFIFO | 0777, 0);
chmod (file_name, 0777);
/* After */
Create_FIFO(file_name);
```

### 배치 3: config_db.c — Replace mkdir+chmod (3 pairs)

| FR | Lines | Context | Before → After |
|----|:-----:|---------|----------------|
| FR-07 | 867-868 | file data dir | `mkdir+chmod` → `Create_Dir(file_name)` |
| FR-08 | 1178-1179 | dshm data dir | `mkdir+chmod` → `Create_Dir(file_name)` |
| FR-09 | 1709-1710 | proc stat dir | `mkdir+chmod` → `Create_Dir(dir_name)` |

Each replacement changes:
```c
/* Before */
mkdir (file_name, 0777);
chmod (file_name, 0777);
/* After */
Create_Dir(file_name);
```

### 배치 4: pz_memory_conf.c — Replace mknod+chmod (9 pairs)

| FR | Lines | Context | Before → After |
|----|:-----:|---------|----------------|
| FR-10 | 365-401 | daemon FIFOs, flag==0 | 4 × `mknod+chmod` → 4 × `Create_FIFO(file_name)` |
| FR-11 | 414-449 | daemon FIFOs, flag==1 | 3 × `mknod+chmod` → 3 × `Create_FIFO(file_name)` |
| FR-12 | 775-776 | file FIFO loop | `mknod+chmod` → `Create_FIFO(file_name)` |
| FR-13 | 1083-1084 | dshm FIFO loop | `mknod+chmod` → `Create_FIFO(file_name)` |

### 배치 5: pz_memory_conf.c — Replace mkdir+chmod (4 pairs)

| FR | Lines | Context | Before → After |
|----|:-----:|---------|----------------|
| FR-14 | 693-694 | file data/log dir | `mkdir+chmod` → `Create_Dir(file_name)` |
| FR-15 | 1009-1010 | dshm data dir | `mkdir+chmod` → `Create_Dir(file_name)` |
| FR-16 | 1465-1467 | CISAM data dir | `rt = mkdir(); if (rt==0) chmod()` → `Create_Dir(ppath)` |
| FR-17 | 2332-2333 | proc stat dir | `mkdir+chmod` → `Create_Dir(dir_name)` |

FR-16 note: The existing partial check `rt = mkdir(); if (rt == 0) chmod()` is upgraded. `Create_Dir()` also chmod's on EEXIST, which is an improvement (ensures correct permissions on restart).

### 배치 6: pz_memory_proc.c — Replace mkdir+chmod (5 pairs)

| FR | Lines | Context | Notes |
|----|:-----:|---------|-------|
| FR-18 | 121-122 | work date dir | Inside `if (rt == -1)` after stat(); keep existing `Log(SYS_OK, ...)` on L123 |
| FR-19 | 132-133 | data dir | Inside `if (rt == -1)` after stat(); keep existing `Log(SYS_OK, ...)` on L134 |
| FR-20 | 218-219 | move work date dir | Inside `if (rt == -1)` after stat() |
| FR-21 | 252-253 | data dir reset | Unchecked (no stat guard) |
| FR-22 | 310-311 | log dir | Inside `if (rt == -1)` after stat(); keep existing `Log(SYS_OK, ...)` on L312 |

For FR-18/19/22, the caller's success Log() messages are preserved (they follow the `Create_Dir()` call). Example:
```c
/* Before */
if (rt == -1)
{
    mkdir (file_name, 0777);
    chmod (file_name, 0777);
    Log (SYS_OK, "work date directory (%s) created", file_name);
}
/* After */
if (rt == -1)
{
    Create_Dir(file_name);
    Log (SYS_OK, "work date directory (%s) created", file_name);
}
```

## Implementation Order

1. Batch 1 (FR-01, FR-02) — Create helpers and header declarations
2. Batch 2 (FR-03 ~ FR-06) — config_db.c FIFO replacements (9 pairs)
3. Batch 3 (FR-07 ~ FR-09) — config_db.c directory replacements (3 pairs)
4. Batch 4 (FR-10 ~ FR-13) — pz_memory_conf.c FIFO replacements (9 pairs)
5. Batch 5 (FR-14 ~ FR-17) — pz_memory_conf.c directory replacements (4 pairs)
6. Batch 6 (FR-18 ~ FR-22) — pz_memory_proc.c directory replacements (5 pairs)

## 검증

- `grep -rn 'mknod\s*(' st01/sub/config_db.c st01/src/PZ/pz_memory_conf.c` — should return 0 matches
- `grep -rn 'Create_FIFO' st01/sub/config_db.c` — should return 9 matches
- `grep -rn 'Create_FIFO' st01/src/PZ/pz_memory_conf.c` — should return 9 matches
- `grep -c 'Create_Dir' st01/sub/config_db.c` — should return 3
- `grep -c 'Create_Dir' st01/src/PZ/pz_memory_conf.c` — should return 4
- `grep -c 'Create_Dir' st01/src/PZ/pz_memory_proc.c` — should return 5
- Build: `mk.sh sub && mk.sh src` — no compilation errors
- `grep -rn 'S_IFIFO' st01/sub/ st01/src/` — should only appear in `sub/file_util.c`

## Counts 요약

| 범주 | FR 항목 | Call Sites | Files |
|----------|:--------:|:----------:|:-----:|
| Helper functions | 2 | — | 2 (new file_util.c + fep_sub.h) |
| config_db.c FIFOs | 4 | 9 | 1 |
| config_db.c dirs | 3 | 3 | 1 |
| pz_memory_conf.c FIFOs | 4 | 9 | 1 |
| pz_memory_conf.c dirs | 4 | 4 | 1 |
| pz_memory_proc.c dirs | 5 | 5 | 1 |
| **Total** | **22** | **30** | **5** (3 modified + 1 new + 1 header) |

## 비기능 요구사항

- Zero functional change on success path (FIFOs/directories created with same 0777 mode)
- `EEXIST` silently tolerated with chmod (preserves restart behavior)
- Real failures now logged with `SYS_FATAL` severity, path, and errno details
- `mknod()` → `mkfifo()` modernization (POSIX.1-2008 compliance)
- C89 compatible — `mkfifo()` is POSIX.1, available on all target platforms (HP-UX, SunOS, AIX, Linux)
- Build system auto-detects new `sub/file_util.c` via `Make_Lib_P_c.sh`
- Net line change: ~+30 lines (helpers) − ~30 lines (replaced pairs) ≈ 0
