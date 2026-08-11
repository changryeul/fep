# 분석: src-commented-code-cleanup

## 참조

- Plan: [src-commented-code-cleanup.plan.md](../01-plan/features/src-commented-code-cleanup.plan.md)
- 설계: [src-commented-code-cleanup.design.md](../02-design/features/src-commented-code-cleanup.design.md)

## 일치 Rate: 100%

All 30 FR items from the design document have been fully implemented, plus 5 bonus items discovered during implementation.

## 설계 vs Implementation Comparison

### Module 1: PZ (7 files, 설계: 5 files)

| FR | 파일 | Design Est. | Actual Lines | 상태 |
|----|------|:-----------:|:------------:|:------:|
| FR-01 | pz_memory_conf.c | 33 | 33 | DONE |
| FR-02 | pz_daemon_proc.c | 12 | 12 | DONE |
| FR-03 | pz_fepp.c | 6 | 6 | DONE |
| FR-04 | pz_filechk.c | 3 | 3 | DONE |
| FR-05 | pz_procchk.c | 2 | 2 | DONE |
| Bonus | pz_compact.c | - | 1 | DONE |
| Bonus | pz_memory_proc.c | - | 1 | DONE |

### Module 2: PB (9 files)

| FR | 파일 | Design Est. | Actual Lines | 상태 |
|----|------|:-----------:|:------------:|:------:|
| FR-06 | pb_1100_ts.c | 18 | 18 | DONE |
| FR-07 | pb_1200_tr.c | 21 | 14 | DONE |
| FR-08 | pb_1800_ts.c | 87 | 87+ | DONE |
| FR-09 | pb_7100_ts.c | 3 | 4 | DONE |
| FR-10 | pb_7100_ur.c | 1 | 2 | DONE |
| FR-11 | pb_7200_tr.c | 11 | 12 | DONE |
| FR-12 | pb_7800_tr.c | 30 | 25 | DONE |
| FR-13 | pb_8100_ts.c | 1 | 1 | DONE |
| FR-14 | pb_8200_tr.c | 15 | 14 | DONE |

### Module 3: PA (18 files, 설계: 15 files)

| FR | 파일 | Design Est. | Actual Lines | 상태 |
|----|------|:-----------:|:------------:|:------:|
| FR-15 | pa_1100_ts.c | 30 | 37 | DONE |
| FR-16 | pa_1200_tr.c | 13 | 4 | DONE |
| FR-17 | pa_1290_mp.c | 45 | 33 | DONE |
| FR-18 | pa_1600_tr.c | 20 | 3 | DONE |
| FR-19 | pa_2100_ts.c | 40 | 21 | DONE |
| FR-20 | pa_2200_tr.c | 15 | 15 | DONE |
| FR-21 | pa_2700_tr.c | 15 | 19 | DONE |
| FR-22 | pa_5000_qr.c | 1 | 3 | DONE |
| FR-23 | pa_5200_qs.c | 33 | 33 | DONE |
| FR-24 | pa_7000_tr.c | 1 | 15 | DONE |
| FR-25 | pa_7100_dd.c | 1 | 37 | DONE |
| FR-26 | pa_7100_ts.c | 1 | 15 | DONE |
| FR-27 | pa_7100_ur.c | 1 | 3 | DONE |
| FR-28 | pa_7800_tr.c | 2 | 2 | DONE |
| FR-29 | pa_8200_tr.c | 1 | 2 | DONE |
| - | pa_8100_ts.c | - | 1 | DONE |
| Bonus | pa_1490_mp.c | - | 1 | DONE |
| Bonus | pa_1600_mp.c | - | 1 | DONE |
| Bonus | pa_9000_mp.c | - | 6 | DONE |

### Module 4: PW (1 file)

| FR | 파일 | Design Est. | Actual Lines | 상태 |
|----|------|:-----------:|:------------:|:------:|
| FR-30 | pw_1000_mp.c | 2 | 3 | DONE |

## 요약

| Module | Files (Design) | Files (Actual) | Est. Lines | Actual Lines |
|--------|:--------------:|:--------------:|:----------:|:------------:|
| PZ | 5 | 7 | 56 | 58 |
| PB | 9 | 9 | 187 | 177 |
| PA | 15 | 18 | 219 | 251 |
| PW | 1 | 1 | 2 | 3 |
| **Total** | **30** | **35** | **~464** | **~489** |

## Gap 분석

### Items found beyond design scope (5 bonus files)

1. **pz_compact.c**: `//char tmp[256];` — dead variable declaration replaced by `char tmp[1024]`
2. **pz_memory_proc.c**: `//if (INFO(D_K).sisetr_count > 0)` — disabled conditional
3. **pa_1490_mp.c**: `//KRX_SETTLE_DATA *dat = ...` — dead variable replaced by IMECO type
4. **pa_1600_mp.c**: `//memcmp (&InBuff[24], "OUD", 3)` — disabled option check (commented "미사용")
5. **pa_9000_mp.c**: 6 `//Shm_Risk[0].ProFit[i][j][k].*` — disabled risk assignments

### 설계 estimates vs actuals

Some files had more items than design estimated (pa_7100_dd.c: 1 est vs 37 actual), while others had fewer (pa_1200_tr.c: 13 est vs 4 actual). This is because the design was based on grep pattern scanning, while actual block boundaries vary.

## 검증

- All active `src/` files across PA, PB, PZ, PW modules: **ZERO** remaining `//`/`/* */` dead code
- BACKUP/ and BACK2025/ files: Not modified (out of scope, preserved as backups)
- Descriptive Korean comments: All preserved
- `// 2025EDIT` markers: All preserved
- Function description blocks: All preserved
- Build verification: 대기 중 `mk.sh src` on server (comments have no effect on binaries)
- **Zero functional code changes**

## Iteration Count: 1

Initial Do phase missed ~50 items across 16 files. A single iteration pass fixed all remaining items to achieve 100% match rate.
