# 설계: dead-code-cleanup

## Reference
- Plan: `docs/01-plan/features/dead-code-cleanup.plan.md`

## 구현 Order

1. FR-01 ~ FR-04: sub/ library files (4 files)
2. FR-05 ~ FR-08: src/PZ/ system management (4 files)
3. FR-09 ~ FR-14: src/PX/ utilities (6 files)
4. FR-15: src/PW/ daemon (1 file)
5. FR-16 ~ FR-27: src/PA/ trading logic (12 files)
6. FR-28 ~ FR-31: src/PB/ bonds (4 files)

## Change Specification

For each FR item: **delete the entire `#if 0` ... `#endif` block** (both preprocessor lines inclusive). No other changes to any file.

---

## Batch 1: sub/ (4 files, ~387 dead lines)

### FR-01: sub/hoga_check.c — 3 blocks, ~184 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 83-144 | 62 | Contains nested #if 0 at 100-105 |
| 2 | 157-191 | 35 | |
| 3 | 198-284 | 87 | To end of file |

### FR-02: sub/key_search.c — 1 block, ~116 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 61-176 | 116 | Large alternative implementation |

### FR-03: sub/getfileno.c — 2 blocks, ~82 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 47-89 | 43 | Comment: "202109" |
| 2 | 127-165 | 39 | Comment: "202109" |

### FR-04: sub/stat_save.c — 1 block, ~5 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 105-109 | 5 | Small block |

---

## Batch 2: src/PZ/ (4 files, ~486 dead lines)

### FR-05: pz_memory_conf.c — 3 blocks, ~436 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 2299-2473 | 175 | Largest single block |
| 2 | 2619-2637 | 19 | |
| 3 | 3381-3622 | 242 | Second-largest block |

### FR-06: pz_daemon_proc.c — 1 block, ~31 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 227-257 | 31 | |

### FR-07: pz_procchk.c — 2 blocks, ~13 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 236-238 | 3 | Tiny block |
| 2 | 307-316 | 10 | |

### FR-08: pz_memory_shm.c — 1 block, ~6 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 240-245 | 6 | |

---

## Batch 3: src/PX/ (6 files, ~444 dead lines)

### FR-09: px_chkgap.c — 1 block, ~186 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 16-201 | 186 | 90% of file; ~20 lines remain (stub main) |

### FR-10: px_jisudat_c.c — 1 block, ~85 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 27-111 | 85 | Entire body dead; ~25 lines remain |

### FR-11: px_jisudat.c — 1 block, ~83 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 27-109 | 83 | Entire body dead; ~25 lines remain |

### FR-12: px_elwjisudat.c — 1 block, ~50 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 27-76 | 50 | Entire body dead; ~25 lines remain |

### FR-13: px_showsise.c — 2 blocks, ~30 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 244-258 | 15 | |
| 2 | 286-300 | 15 | |

### FR-14: px_chkshm.c — 2 blocks, ~10 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 319-325 | 7 | |
| 2 | 339-341 | 3 | |

---

## Batch 4: src/PW/ (1 file, ~4 dead lines)

### FR-15: pw_1000_mp.c — 1 block, ~4 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 229-232 | 4 | Comment: "test" |

---

## Batch 5: src/PA/ (12 files, ~316 dead lines)

### FR-16: pa_1100_ts.c — 5 blocks, ~63 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 57-61 | 5 | |
| 2 | 695-703 | 9 | |
| 3 | 862-867 | 6 | |
| 4 | 900-923 | 24 | |
| 5 | 1105-1123 | 19 | |

### FR-17: pa_2100_ts.c — 2 blocks, ~66 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 1064-1093 | 30 | |
| 2 | 1113-1148 | 36 | |

### FR-18: pa_1490_mp.c — 3 blocks, ~42 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 178-181 | 4 | |
| 2 | 311-318 | 8 | |
| 3 | 612-641 | 30 | |

### FR-19: pa_9000_mp.c — 1 block, ~27 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 311-337 | 27 | |

### FR-20: pa_3100_ts.c — 2 blocks, ~19 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 48-52 | 5 | |
| 2 | 301-314 | 14 | |

### FR-21: pa_5020_mp.c — 2 blocks, ~19 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 333-340 | 8 | |
| 2 | 869-879 | 11 | |

### FR-22: pa_7800_tr.c — 1 block, ~17 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 123-139 | 17 | |

### FR-23: pa_7000_tr.c — 1 block, ~15 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 526-540 | 15 | |

### FR-24: pa_8200_tr.c — 1 block, ~15 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 491-505 | 15 | |

### FR-25: pa_7100_ur.c — 2 blocks, ~12 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 163-171 | 9 | |
| 2 | 309-311 | 3 | |

### FR-26: pa_1600_tr.c — 1 block, ~7 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 680-686 | 7 | |

### FR-27: pa_2200_tr.c — 1 block, ~7 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 750-756 | 7 | |

Note: pa_2700_tr.c was listed in the plan (line 500-506, ~7 lines) but needs verification during Do phase.

---

## Batch 6: src/PB/ (4 files, ~258 dead lines)

### FR-28: pb_1100_ts.c — 6 blocks, ~109 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 59-63 | 5 | |
| 2 | 754-762 | 9 | |
| 3 | 926-931 | 6 | |
| 4 | 989-1011 | 23 | |
| 5 | 1110-1156 | 47 | Largest block in file |
| 6 | 1253-1271 | 19 | |

### FR-29: pb_1800_ts.c — 5 blocks, ~72 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 950-963 | 14 | |
| 2 | 1030-1048 | 19 | |
| 3 | 1075-1083 | 9 | |
| 4 | 1219-1224 | 6 | |
| 5 | 1282-1305 | 24 | |

### FR-30: pb_8200_tr.c — 2 blocks, ~61 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 488-502 | 15 | |
| 2 | 629-674 | 46 | |

### FR-31: pb_7800_tr.c — 2 blocks, ~16 lines

| Block | Delete Lines | Size | Notes |
|:-----:|:------------|:----:|-------|
| 1 | 130-138 | 9 | |
| 2 | 143-149 | 7 | |

---

## Summary

| Batch | Module | Files | Blocks | Dead Lines |
|-------|--------|:-----:|:------:|:----------:|
| 1 | sub/ | 4 | 7 | ~387 |
| 2 | PZ | 4 | 7 | ~486 |
| 3 | PX | 6 | 8 | ~444 |
| 4 | PW | 1 | 1 | ~4 |
| 5 | PA | 12 | 22 | ~316 |
| 6 | PB | 4 | 15 | ~258 |
| **Total** | | **31** | **60** | **~1,895** |

Note: FR count is 31 (pa_2700_tr.c deferred to verification). Total blocks ~60 across 31 files.

## Verification Checklist

1. `grep -r "#if 0" st01/sub/` returns 0 matches
2. `grep -r "#if 0" st01/src/PA/` returns 0 matches in active .c files (exclude BACK2025/)
3. `grep -r "#if 0" st01/src/PB/` returns 0 matches in active .c files (exclude .org)
4. `grep -r "#if 0" st01/src/PW/` returns 0 matches
5. `grep -r "#if 0" st01/src/PX/` returns 0 matches
6. `grep -r "#if 0" st01/src/PZ/` returns 0 matches in active .c files (exclude BACKUP/)
7. No lines changed other than block deletions (no edits to remaining code)
8. Build: `mk.sh sub && mk.sh src` produces identical binaries
