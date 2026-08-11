# inet-addr-to-pton Planning Document

> **요약**: Replace deprecated `inet_addr()` with POSIX `inet_pton()` across 7 active files. Fix 3 stale error-check bugs in px_memok.c from incomplete 2021 migration. Clean up 5 commented-out `inet_addr` blocks.
>
> **프로젝트**: FEP (Front-End Processor) for KRX
> **저자**: Claude
> **날짜**: 2026-02-25
> **상태**: 초안

---

## 1. 개요

### 1.1 목적

`inet_addr()` is deprecated (POSIX.1-2001 legacy, discouraged since POSIX.1-2008):
- Returns `in_addr_t` (-1 on error) which is indistinguishable from valid address 255.255.255.255
- Thread-unsafe on some implementations
- `inet_pton()` is the POSIX replacement: returns 1 on success, 0 for invalid format, -1 on error

### 1.2 배경

A partial migration was done in 2021 (`/* 202108 */` comment markers):
- 5 calls migrated to `inet_pton` (pa_7100_ur.c, pa_7000_mp.c, pb_7100_ur.c, tcpip_connect.c)
- 3 calls in px_memok.c migrated BUT left **stale `ul` error checks** — the `inet_addr` line was commented out but `if (ul == -1)` error check was not updated
- 9 calls remain unmigrated in config_db.c, udp_init.c, px_sett2ip.c, px_setudp.c, pz_memory_conf.c

### 1.3 Current State

| 파일 | inet_addr | inet_pton | 상태 |
|------|:---------:|:---------:|--------|
| `sub/tcpip_connect.c` | 0 | 2 | Clean (migrated 2021) |
| `src/PA/pa_7100_ur.c` | 1 (comment) | 1 | Migrated, commented dead code |
| `src/PA/pa_7000_mp.c` | 1 (comment) | 1 | Migrated, commented dead code |
| `src/PB/pb_7100_ur.c` | 0 | 1 | Clean (migrated 2021) |
| **`src/PX/px_memok.c`** | **3 (comment)** | **3** | **BUG: stale `ul` check** |
| **`sub/config_db.c`** | **3** | 0 | Active, needs migration |
| **`sub/udp_init.c`** | **1** | 0 | Active, needs migration |
| **`src/PX/px_sett2ip.c`** | **1** | 0 | Active, needs migration |
| **`src/PX/px_setudp.c`** | **1** | 0 | Active, needs migration |
| **`src/PZ/pz_memory_conf.c`** | **3** | 0 | Active, needs migration |

---

## 2. 범위

### 2.1 범위 내

- [x] Migrate 9 active `inet_addr()` calls to `inet_pton()` (5 files)
- [x] Fix 3 stale `ul` error-check bugs in `px_memok.c`
- [x] Clean up 5 commented-out `inet_addr` blocks (3 files)

### 2.2 범위 외

- Already-clean inet_pton calls (tcpip_connect.c, pb_7100_ur.c)
- `.org` / `.back` / `BACKUP/` files
- `utl/` test utilities (sample_recv.c, t_send.c, etc.)

---

## 3. 요구사항

### 3.1 범주 A: Active Migrations (9 calls, 5 files)

| ID | 파일 | Line | Pattern | Error Handling |
|----|------|------|---------|----------------|
| FR-01 | `sub/config_db.c` | 1350 | `ul = inet_addr(val)` + `memcpy(TCP1.ip_addr, &ul, ...)` | `if (ul == (unsigned int)-1)` → Log + return |
| FR-02 | `sub/config_db.c` | 1480 | Same pattern for TCP2 | Same |
| FR-03 | `sub/config_db.c` | 1607 | Same pattern for UDPIP | Same |
| FR-04 | `sub/udp_init.c` | 38 | `sin_addr.s_addr = inet_addr(svr_ip)` | **No error check** (add one) |
| FR-05 | `src/PX/px_sett2ip.c` | 91 | `ul = inet_addr(ip)` | `if (ul == -1)` → exit |
| FR-06 | `src/PX/px_setudp.c` | 124 | `ul = inet_addr(ip_addr)` + `memcpy(&UDP_IP1, &ul, ...)` | `if (ul == -1)` → exit |
| FR-07 | `src/PZ/pz_memory_conf.c` | 1709 | `ul = inet_addr(tmp2)` + `memcpy(TCP1.ip_addr, &ul, ...)` | `if (ul == -1)` → Log + exit |
| FR-08 | `src/PZ/pz_memory_conf.c` | 1996 | Same pattern for TCP2 | Same |
| FR-09 | `src/PZ/pz_memory_conf.c` | 2187 | Same pattern for UDPIP | Same |

### 3.2 범주 B: Bug Fixes — Stale `ul` Check (3 calls, 1 file)

| ID | 파일 | Line | Bug | Fix |
|----|------|------|-----|-----|
| FR-10 | `src/PX/px_memok.c` | 872~880 | `inet_addr` commented out but `if (ul == -1)` still references stale `ul` | Replace with `inet_pton` return check |
| FR-11 | `src/PX/px_memok.c` | 1039~1047 | Same bug for TCP2 | Same fix |
| FR-12 | `src/PX/px_memok.c` | 1204~1213 | Same bug for UDPIP | Same fix |

### 3.3 범주 C: Dead Comment Cleanup (5 blocks, 3 files)

| ID | 파일 | Lines | Content |
|----|------|-------|---------|
| FR-13 | `src/PA/pa_7100_ur.c` | 278-280 | `/* 202108 inet_addr ... */` |
| FR-14 | `src/PA/pa_7000_mp.c` | 359-361 | `/* inet_addr ... */` |
| FR-15 | `src/PX/px_memok.c` | 871-873 | `/* 202108 inet_addr ... */` (TCP1) |
| FR-16 | `src/PX/px_memok.c` | 1038-1040 | `/* 202108 inet_addr ... */` (TCP2) |
| FR-17 | `src/PX/px_memok.c` | 1203-1205 | `/* 202108 inet_addr ... */` (UDPIP) |

### 3.4 설계 Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Target function | `inet_pton(AF_INET, ...)` | POSIX.1-2001 standard replacement |
| Error check | `inet_pton() != 1` | Returns 1 on success (0=invalid format, -1=af error) |
| `ul` variable | Eliminate where possible | Write directly to struct via inet_pton 3rd arg |
| udp_init.c (FR-04) | Add error check (was missing) | Currently no validation on server IP |

---

## 4. 성공 기준

- [x] Zero `inet_addr` calls in active `sub/` and `src/` code (excluding comments, BACKUP, utl)
- [x] All 3 px_memok.c stale `ul` checks fixed
- [x] All 5 dead comment blocks removed
- [x] `grep -rn "inet_addr" st01/sub/ st01/src/ --include="*.c"` shows 0 active matches

---

## 5. 위험 및 완화

| 위험 | 영향 | 확률 | 완화 |
|------|--------|------------|------------|
| `inet_pton` writes `struct in_addr` (4 bytes) directly; must match existing `ip_addr` field size | High | Low | Verify field sizes match `sizeof(unsigned long)` = 4 |
| `memcpy(&ip_field, &ul, sizeof(ul))` pattern changes | Medium | Low | Replace `ul` intermediate with direct inet_pton write to same destination |
| udp_init.c is shared library (used by 5+ callers) | 중간 | 낮음 | Adding error check is backward-compatible; failure returns early |

---

## 버전 기록

| 버전 | 날짜 | 변경사항 | 저자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-25 | Initial draft | Claude |
