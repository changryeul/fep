# 계획: dead-code-cleanup

## 개요

Remove all `#if 0` ... `#endif` dead code blocks from 32 active source files across sub/, PA, PB, PW, PX, and PZ modules. Pure deletion with zero functional change.

## 문제

The FEP codebase contains ~1,895 lines of dead code wrapped in `#if 0` / `#endif` preprocessor blocks. These blocks:

1. **Obscure active code** — readers must mentally skip over large dead sections
2. **Inflate line counts** — some files are 10-30% dead code (e.g., px_chkgap.c is 90% dead)
3. **Cause false positives** — grep/search results include irrelevant dead code matches
4. **Accumulate over time** — leftover from refactors, experiments, and commented-out alternatives

All dead code is preserved in version control history and BACK2025/ backups. There is no reason to keep it in active source.

## Scope

### sub/ (4 files, ~387 lines)

| # | File | Blocks | Dead Lines | Block Ranges |
|---|------|:------:|:----------:|-------------|
| 1 | hoga_check.c | 3 | 184 | 83-144, 157-191, 198-284 |
| 2 | key_search.c | 1 | 116 | 61-176 |
| 3 | getfileno.c | 2 | 82 | 47-89, 127-165 |
| 4 | stat_save.c | 1 | 5 | 105-109 |

### src/PA/ (12 files, ~316 lines)

| # | File | Blocks | Dead Lines | Block Ranges |
|---|------|:------:|:----------:|-------------|
| 5 | pa_1100_ts.c | 5 | 63 | 57-61, 695-703, 862-867, 900-923, 1105-1123 |
| 6 | pa_1490_mp.c | 3 | 42 | 178-181, 311-318, 612-641 |
| 7 | pa_1600_tr.c | 1 | 7 | 680-686 |
| 8 | pa_2100_ts.c | 2 | 66 | 1064-1093, 1113-1148 |
| 9 | pa_2200_tr.c | 1 | 7 | 750-756 |
| 10 | pa_2700_tr.c | 1 | 7 | 500-506 |
| 11 | pa_3100_ts.c | 2 | 19 | 48-52, 301-314 |
| 12 | pa_5020_mp.c | 2 | 19 | 333-340, 869-879 |
| 13 | pa_7000_tr.c | 1 | 15 | 526-540 |
| 14 | pa_7100_ur.c | 2 | 12 | 163-171, 309-311 |
| 15 | pa_7800_tr.c | 1 | 17 | 123-139 |
| 16 | pa_8200_tr.c | 1 | 15 | 491-505 |
| 17 | pa_9000_mp.c | 1 | 27 | 311-337 |

### src/PB/ (4 files, ~258 lines)

| # | File | Blocks | Dead Lines | Block Ranges |
|---|------|:------:|:----------:|-------------|
| 18 | pb_1100_ts.c | 6 | 109 | 59-63, 754-762, 926-931, 989-1011, 1110-1156, 1253-1271 |
| 19 | pb_1800_ts.c | 5 | 72 | 950-963, 1030-1048, 1075-1083, 1219-1224, 1282-1305 |
| 20 | pb_7800_tr.c | 2 | 16 | 130-138, 143-149 |
| 21 | pb_8200_tr.c | 2 | 61 | 488-502, 629-674 |

### src/PW/ (1 file, ~4 lines)

| # | File | Blocks | Dead Lines | Block Ranges |
|---|------|:------:|:----------:|-------------|
| 22 | pw_1000_mp.c | 1 | 4 | 229-232 |

### src/PX/ (6 files, ~444 lines)

| # | File | Blocks | Dead Lines | Block Ranges |
|---|------|:------:|:----------:|-------------|
| 23 | px_chkgap.c | 1 | 186 | 16-201 |
| 24 | px_elwjisudat.c | 1 | 50 | 27-76 |
| 25 | px_jisudat.c | 1 | 83 | 27-109 |
| 26 | px_jisudat_c.c | 1 | 85 | 27-111 |
| 27 | px_showsise.c | 2 | 30 | 244-258, 286-300 |
| 28 | px_chkshm.c | 2 | 10 | 319-325, 339-341 |

### src/PZ/ (4 files, ~486 lines)

| # | File | Blocks | Dead Lines | Block Ranges |
|---|------|:------:|:----------:|-------------|
| 29 | pz_memory_conf.c | 3 | 436 | 2299-2473, 2619-2637, 3381-3622 |
| 30 | pz_memory_shm.c | 1 | 6 | 240-245 |
| 31 | pz_daemon_proc.c | 1 | 31 | 227-257 |
| 32 | pz_procchk.c | 2 | 13 | 236-238, 307-316 |

## 접근법

### Method

For each file, delete the entire `#if 0` ... `#endif` block (inclusive of both preprocessor lines). No other changes.

### Special cases

1. **px_chkgap.c**: 186 of 206 lines are dead. After removal, only ~20 lines remain (includes, main() stub that does nothing). File is still needed as a build target (produces `px_chkgap_mp` binary). Leave the live stub.

2. **px_jisudat*.c trio**: Each file has its entire body dead (lines 27+). After removal, ~25 live lines remain (headers, includes, empty main). These are utility binaries that may still be invoked by operational scripts. Leave the live stubs.

3. **Nested #if 0**: hoga_check.c has a nested `#if 0` at line 100 inside the outer block starting at line 83. The entire outer block (83-144) is deleted, which includes the nested inner block.

4. **pz_memory_conf.c**: Has the largest dead code volume (436 lines in 3 blocks). A separate future feature may migrate this file to config_loader, but this cleanup removes the dead blocks regardless.

### 구현 order

1. sub/ files (4 files) — library files first
2. src/PZ/ files (4 files) — system management
3. src/PX/ files (6 files) — utilities
4. src/PW/ file (1 file) — daemon
5. src/PA/ files (12 files) — trading logic
6. src/PB/ files (4 files) — bonds

## 영향

| Metric | Value |
|--------|-------|
| Files modified | 32 (4 sub + 28 src) |
| Total blocks removed | ~55 |
| Total lines removed | ~1,895 |
| Lines added | 0 |
| Net change | ~-1,895 lines |
| Functional change | None (dead code only) |
| Build impact | None (preprocessor already excluded these blocks) |

## 위험 평가

- **NONE**: `#if 0` blocks are never compiled — removing them has zero effect on compiled binaries
- **Version control**: All removed code is preserved in git history and BACK2025/ backups
- **Build verification**: `mk.sh sub && mk.sh src` should produce byte-identical binaries

## 제외 범위

- Removing `/* ... */` style commented-out code blocks (separate concern)
- Removing `//` single-line commented-out code
- Removing `BACK2025/` or `BACKUP/` directories (not active code)
- Any functional code changes
- `#ifdef`/`#ifndef` blocks that ARE conditionally compiled (e.g., HOLIDAY_APPLY, SAM_USE)

## 성공 기준

1. `grep -r "#if 0" st01/sub/ st01/src/` returns 0 matches in active files (excluding BACK2025/, BACKUP/, .org, .back)
2. All 32 files modified with only `#if 0` ... `#endif` block deletions
3. No other lines changed
4. Build: `mk.sh sub && mk.sh src` produces identical binaries
