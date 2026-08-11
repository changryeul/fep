# inet-addr-to-pton 분석 Report

> **분석 Type**: Gap 분석 (설계 vs Implementation)
>
> **프로젝트**: FEP (Front-End Processor) for KRX
> **Analyst**: Claude
> **날짜**: 2026-02-25
> **설계 Doc**: [inet-addr-to-pton.design.md](../02-design/features/inet-addr-to-pton.design.md)
> **Plan Doc**: [inet-addr-to-pton.plan.md](../01-plan/features/inet-addr-to-pton.plan.md)

---

## 1. 분석 개요

### 1.1 분석 목적

Verify that the implementation of the `inet_addr()` to `inet_pton()` migration across 7 files and 17 FR items matches the design document. This includes active migrations (FR-01 to FR-09), bug fixes in px_memok.c (FR-10 to FR-12), and dead comment cleanup (FR-13 to FR-17).

### 1.2 분석 범위

- **설계 Document**: `docs/02-design/features/inet-addr-to-pton.design.md`
- **Implementation Files**: 7 files across `sub/`, `src/PA/`, `src/PX/`, `src/PZ/`
- **FR Items**: 17 (FR-01 through FR-17)
- **분석 날짜**: 2026-02-25

---

## 2. Gap 분석 (설계 vs Implementation)

### 2.1 FR-01: `sub/config_db.c` -- TCP1 IP (Pattern A)

| Criterion | 설계 | Implementation | 상태 |
|-----------|--------|----------------|--------|
| inet_pton direct write to `TCP1(p_cnt,n-1).ip_addr` | Yes | `inet_pton (AF_INET, val, TCP1(p_cnt,n-1).ip_addr) != 1` at line 1349 | PASS |
| `ul` variable removed | Yes | No `unsigned int ul` in `cfg_db_load_tcp1()` declarations (line 1275) | PASS |
| No memcpy intermediate | Yes | No memcpy present | PASS |
| Error check `!= 1` | Yes | `if (inet_pton (...) != 1)` | PASS |
| Log message preserved | `cfg_db tcp1(%c,%d):bad IP[%s]` | Matches at line 1352 | PASS |
| Error action: finalize + return | Yes | `sqlite3_finalize(stmt); return CFG_DB_ERR_TYPE;` | PASS |

**결과**: PASS (6/6 criteria met)

### 2.2 FR-02: `sub/config_db.c` -- TCP2 IP (Pattern A)

| Criterion | 설계 | Implementation | 상태 |
|-----------|--------|----------------|--------|
| inet_pton direct write to `TCP2(p_cnt,n-1).ip_addr` | Yes | `inet_pton (AF_INET, val, TCP2(p_cnt,n-1).ip_addr) != 1` at line 1475 | PASS |
| `ul` variable removed | Yes | No `unsigned int ul` in `cfg_db_load_tcp2()` declarations (line 1401) | PASS |
| No memcpy intermediate | Yes | No memcpy present | PASS |
| Error check `!= 1` | Yes | Matches | PASS |
| Log message preserved | `cfg_db tcp2(%c,%d):bad IP[%s]` | Matches at line 1478 | PASS |

**결과**: PASS (5/5 criteria met)

### 2.3 FR-03: `sub/config_db.c` -- UDPIP IP (Pattern A)

| Criterion | 설계 | Implementation | 상태 |
|-----------|--------|----------------|--------|
| inet_pton direct write to `UDPIP(p_cnt,n-1).ip_addr[m-1]` | Yes | `inet_pton (AF_INET, val, UDPIP(p_cnt,n-1).ip_addr[m-1]) != 1` at line 1598 | PASS |
| `ul` variable removed | Yes | No `unsigned int ul` in `cfg_db_load_udpip()` declarations (line 1522-1525) | PASS |
| No memcpy intermediate | Yes | No memcpy present | PASS |
| Error check `!= 1` | Yes | Matches | PASS |
| Log message preserved | `cfg_db udpip(%c,%d):bad IP_%d[%s]` | Matches at line 1601 | PASS |

**결과**: PASS (5/5 criteria met)

### 2.4 FR-04: `sub/udp_init.c` -- Server Address (Pattern C)

| Criterion | 설계 | Implementation | 상태 |
|-----------|--------|----------------|--------|
| `inet_pton(AF_INET, svr_ip, &svr_addr->sin_addr)` | Yes | Line 38: `inet_pton(AF_INET, svr_ip, &svr_addr->sin_addr) != 1` | PASS |
| New error check added | Yes | `if (inet_pton(...) != 1)` block at lines 38-43 | PASS |
| Error severity `UDP_FATAL` | Yes | `Log(UDP_FATAL, "inet_pton fail svr_ip[%s] {%d:%s}", svr_ip, SYS_NO, SYS_STR)` | PASS |
| Error action: `close(sockfd); return -1;` | Yes | Lines 41-42 | PASS |
| Log format matches design | `inet_pton fail svr_ip[%s] {%d:%s}` | Exact match | PASS |

**결과**: PASS (5/5 criteria met)

### 2.5 FR-05: `src/PX/px_sett2ip.c` -- TCP2 IP (Pattern B)

| Criterion | 설계 | Implementation | 상태 |
|-----------|--------|----------------|--------|
| `u_long ul` replaced with `struct in_addr tmp_ia` | Yes | Line 17: `struct in_addr tmp_ia;` | PASS |
| `inet_pton(AF_INET, ip, &tmp_ia) != 1` | Yes | Line 91 | PASS |
| Downstream memcpy uses `&tmp_ia, sizeof(tmp_ia)` | Yes | Lines 120, 121, 134 | PASS |
| Error message preserved | `puts ("malformed IP address")` | Line 93 | PASS |
| Error action: `exit(FAIL)` | Yes | Line 94 | PASS |

**결과**: PASS (5/5 criteria met)

### 2.6 FR-06: `src/PX/px_setudp.c` -- UDP IP (Pattern B)

| Criterion | 설계 | Implementation | 상태 |
|-----------|--------|----------------|--------|
| `u_long ul` replaced with `struct in_addr tmp_ia` | Yes | Line 17: `struct in_addr tmp_ia;` | PASS |
| `inet_pton(AF_INET, ip_addr, &tmp_ia) != 1` | Yes | Line 124 | PASS |
| Downstream memcpy uses `&tmp_ia, sizeof(tmp_ia)` | Yes | Line 130 | PASS |
| Error message preserved | `printf ("malformed IP address[%s]\n", ip_addr)` | Line 126 | PASS |
| Error action: `exit(0)` | Yes | Line 127 | PASS |

**결과**: PASS (5/5 criteria met)

### 2.7 FR-07: `src/PZ/pz_memory_conf.c` -- TCP1 IP (Pattern A)

| Criterion | 설계 | Implementation | 상태 |
|-----------|--------|----------------|--------|
| inet_pton direct write to `TCP1(p_cnt,cnt-1).ip_addr` | Yes | Line 1708: `inet_pton (AF_INET, tmp2, TCP1(p_cnt,cnt-1).ip_addr) != 1` | PASS |
| `ul` variable removed | Yes | No `unsigned int ul` declaration in `Tcp1_Config_Read()` (line 1579-1587) | PASS |
| No memcpy intermediate | Yes | No memcpy present | PASS |
| Log updated: no `ul` reference | Design: `"tcp1(%c,%d):IP address:%s[%s]"` | Line 1717: `"tcp1(%c,%d):IP address:%s[%s]"` | PASS |
| Log updated: no `[%c]` garbage | Yes | Removed -- only `%s[%s]` format args | PASS |

**결과**: PASS (5/5 criteria met)

### 2.8 FR-08: `src/PZ/pz_memory_conf.c` -- TCP2 IP (Pattern A)

| Criterion | 설계 | Implementation | 상태 |
|-----------|--------|----------------|--------|
| inet_pton direct write to `TCP2(p_cnt,cnt-1).ip_addr` | Yes | Line 1992: `inet_pton (AF_INET, tmp2, TCP2(p_cnt,cnt-1).ip_addr) != 1` | PASS |
| `ul` variable removed | Yes | No `unsigned int ul` in `Tcp2_Config_Read()` (line 1864-1872) | PASS |
| No memcpy intermediate | Yes | No memcpy present | PASS |
| Log updated: no `ul` reference | Design: `"tcp2(%c,%d):IP address:%s tmp2[%s][%d]"` | Line 2001: `"tcp2(%c,%d):IP address:%s tmp2[%s][%d]"` | PASS |
| Log updated: no `[%c]` garbage | Yes | Removed -- only `%s`, `%s`, `%d` format args | PASS |

**결과**: PASS (5/5 criteria met)

### 2.9 FR-09: `src/PZ/pz_memory_conf.c` -- UDPIP IP (Pattern A)

| Criterion | 설계 | Implementation | 상태 |
|-----------|--------|----------------|--------|
| inet_pton direct write to `UDPIP(p_cnt,cnt-1).ip_addr[j]` | Yes | Line 2180: `inet_pton (AF_INET, tmp2, UDPIP(p_cnt,cnt-1).ip_addr[j]) != 1` | PASS |
| `ul` variable removed | Yes | No `unsigned int ul` in `Udpip_Config_Read()` | PASS |
| No memcpy intermediate | Yes | No memcpy present | PASS |
| Log updated: no `ul` reference | Design: `"udpip(%c,%d):IP address:%s tmp2[%s][%d]"` | Line 2190: `"udpip(%c,%d):IP address:%s tmp2[%s][%d]"` | PASS |

**결과**: PASS (4/4 criteria met)

### 2.10 FR-10: `src/PX/px_memok.c` -- TCP1 Section Bug Fix (Pattern D)

| Criterion | 설계 | Implementation | 상태 |
|-----------|--------|----------------|--------|
| Block-scoped `struct in_addr tmp_ia` | Yes | Line 871: `struct in_addr tmp_ia;` inside `{` block at line 870 | PASS |
| No SHM write (writes to local `tmp_ia`) | Yes | `inet_pton(AF_INET, tmp2, &tmp_ia)` at line 872 | PASS |
| Correct struct target: `TCP1` in memcmp | Yes | `memcmp (TCP1(p_cnt,cnt-1).ip_addr, &tmp_ia, sizeof (tmp_ia))` at line 877 | PASS |
| Stale `ul` reference eliminated | Yes | No `ul` references in this block | PASS |
| Dead comment `/* 202108 */` deleted | Yes | No `202108` found in file | PASS |
| Error check `!= 1` | Yes | Line 872 | PASS |

**결과**: PASS (6/6 criteria met)

### 2.11 FR-11: `src/PX/px_memok.c` -- TCP2 Section Bug Fix (Pattern D)

| Criterion | 설계 | Implementation | 상태 |
|-----------|--------|----------------|--------|
| Block-scoped `struct in_addr tmp_ia` | Yes | Line 1036: `struct in_addr tmp_ia;` inside `{` block at line 1035 | PASS |
| No SHM write (writes to local `tmp_ia`) | Yes | `inet_pton(AF_INET, tmp2, &tmp_ia)` at line 1037 | PASS |
| Correct struct target: `TCP2` in memcmp | Yes | `memcmp (TCP2(p_cnt,cnt-1).ip_addr, &tmp_ia, sizeof (tmp_ia))` at line 1042 | PASS |
| Stale `ul` reference eliminated | Yes | No `ul` references in this block | PASS |
| Dead comment `/* 202108 */` deleted | Yes | No `202108` found in file | PASS |

**결과**: PASS (5/5 criteria met)

### 2.12 FR-12: `src/PX/px_memok.c` -- UDPIP Section Bug Fix (Pattern D)

| Criterion | 설계 | Implementation | 상태 |
|-----------|--------|----------------|--------|
| Block-scoped `struct in_addr tmp_ia` | Yes | Line 1199: `struct in_addr tmp_ia;` inside `{` block at line 1198 | PASS |
| No SHM write (writes to local `tmp_ia`) | Yes | `inet_pton(AF_INET, tmp2, &tmp_ia)` at line 1200 | PASS |
| Correct struct target: `UDPIP` in memcmp | Yes | `memcmp (UDPIP(p_cnt,cnt-1).ip_addr[j], &tmp_ia, sizeof (tmp_ia))` at line 1205 | PASS |
| Stale `ul` reference eliminated | Yes | No `ul` references in this block | PASS |
| Dead comment `/* 202108 */` deleted | Yes | No `202108` found in file | PASS |

**결과**: PASS (5/5 criteria met)

### 2.13 FR-13: `src/PA/pa_7100_ur.c` -- Dead Comment Removal (Pattern E)

| Criterion | 설계 | Implementation | 상태 |
|-----------|--------|----------------|--------|
| `/* 202108 ... inet_addr ... */` comment deleted | Yes | Zero `inet_addr` references in file | PASS |
| Existing `inet_pton` call preserved | Yes | Line 278: `inet_pton(AF_INET, ip_addr, &SvrAddr.sin_addr.s_addr)` | PASS |
| Zero `202108` markers | Yes | No `202108` found | PASS |

**결과**: PASS (3/3 criteria met)

### 2.14 FR-14: `src/PA/pa_7000_mp.c` -- Dead Comment Removal (Pattern E)

| Criterion | 설계 | Implementation | 상태 |
|-----------|--------|----------------|--------|
| `/* inet_addr ... */` comment deleted | Yes | Zero `inet_addr` references in file | PASS |
| Existing `inet_pton` call preserved | Yes | Line 359: `inet_pton(AF_INET, &Svr_IP[20*i], &Svr_Addr[i][j].sin_addr.s_addr)` | PASS |

**결과**: PASS (2/2 criteria met)

### 2.15 FR-15/16/17: `src/PX/px_memok.c` -- Dead Comment Cleanup

These are subsumed by FR-10, FR-11, and FR-12 transformations. Verified above: zero `/* 202108 */` comment blocks remain in px_memok.c.

**결과**: PASS

---

## 3. 검증 기준 Results (설계 Section 6)

### 3.1 Zero Active `inet_addr` (Section 6.1)

```
Search: inet_addr in sub/*.c, src/PA/*.c, src/PB/*.c, src/PX/*.c, src/PZ/*.c
        (excluding .org, .back, BACKUP/, utl/)

Active source matches: 0
BACKUP-only matches:   3 (all in src/PZ/BACKUP/pz_memory_conf.c -- expected)
```

**상태**: PASS

### 3.2 `inet_pton` Call Counts (Section 6.2)

| 파일 | Expected | Actual | 상태 |
|------|:--------:|:------:|--------|
| `sub/config_db.c` | 3 | 3 | PASS |
| `sub/udp_init.c` | 1 | 1 | PASS |
| `src/PZ/pz_memory_conf.c` | 3 | 3 | PASS |
| `src/PX/px_memok.c` | 3 | 3 | PASS |
| `src/PX/px_sett2ip.c` | 1 | 1 | PASS |
| `src/PX/px_setudp.c` | 1 | 1 | PASS |
| **Total** | **12** | **12** | **PASS** |

### 3.3 Zero `ul` 참조 in px_memok.c (Section 6.3)

```
Search: \bul\b in src/PX/px_memok.c
Result: 0 matches
```

**상태**: PASS

### 3.4 No SHM Write in px_memok.c inet_pton Calls (Section 6.4)

```
Search: inet_pton.*TCP1|inet_pton.*TCP2|inet_pton.*UDPIP in src/PX/px_memok.c
Result: 0 matches

All 3 inet_pton calls write to local &tmp_ia:
  Line 872:  inet_pton(AF_INET, tmp2, &tmp_ia)
  Line 1037: inet_pton(AF_INET, tmp2, &tmp_ia)
  Line 1200: inet_pton(AF_INET, tmp2, &tmp_ia)
```

**상태**: PASS

### 3.5 Zero `inet_addr` in PA Files (Section 6.5)

```
Search: inet_addr in pa_7100_ur.c: 0 matches
Search: inet_addr in pa_7000_mp.c: 0 matches
```

**상태**: PASS

---

## 4. Variable Cleanup 검증

### 4.1 `ul` Declaration Removal

| 파일 | Original Declaration | Expected Action | Actual | 상태 |
|------|---------------------|-----------------|--------|--------|
| `sub/config_db.c` (tcp1 func) | `unsigned int ul;` ~line 1278 | Delete | Not found (removed) | PASS |
| `sub/config_db.c` (tcp2 func) | `unsigned int ul;` ~line 1408 | Delete | Not found (removed) | PASS |
| `sub/config_db.c` (udpip func) | `unsigned int ul;` ~line 1534 | Delete | Not found (removed) | PASS |
| `src/PZ/pz_memory_conf.c` (tcp1 func) | `unsigned int ul;` ~line 1585 | Delete | Not found (removed) | PASS |
| `src/PZ/pz_memory_conf.c` (tcp2 func) | `unsigned int ul;` ~line 1873 | Delete | Not found (removed) | PASS |
| `src/PZ/pz_memory_conf.c` (udpip func) | `unsigned int ul;` ~line 2060 | Delete | Not found (removed) | PASS |
| `src/PX/px_memok.c` (tcp1 func) | `u_long ul;` ~line 774 | Delete | Not found (removed) | PASS |
| `src/PX/px_memok.c` (tcp2 func) | `u_long ul;` ~line 941 | Delete | Not found (removed) | PASS |
| `src/PX/px_memok.c` (udpip func) | `u_long ul;` ~line 1100 | Delete | Not found (removed) | PASS |
| `src/PX/px_sett2ip.c` | `u_long ul;` line 17 | Replace with `struct in_addr tmp_ia` | `struct in_addr tmp_ia;` at line 17 | PASS |
| `src/PX/px_setudp.c` | `u_long ul;` line 17 | Replace with `struct in_addr tmp_ia` | `struct in_addr tmp_ia;` at line 17 | PASS |

**결과**: 11/11 PASS -- All `ul` declarations removed or replaced as designed.

### 4.2 Global `ul` Reference Check

```
grep \bul\b across all 7 implementation files:
  config_db.c:     0 matches
  udp_init.c:      n/a (never had ul)
  pz_memory_conf.c: 0 matches
  px_memok.c:      0 matches
  px_sett2ip.c:    n/a (replaced with tmp_ia)
  px_setudp.c:     n/a (replaced with tmp_ia)
```

**상태**: PASS -- Zero stale `ul` references anywhere.

---

## 5. Pattern Compliance 검증

### 5.1 Transformation Pattern Adherence

| 파일 | Expected Pattern | Actual Pattern | 상태 |
|------|-----------------|----------------|--------|
| `sub/config_db.c` (x3) | Pattern A: Direct write | inet_pton writes directly to ip_addr field, no intermediate | PASS |
| `sub/udp_init.c` (x1) | Pattern C: sockaddr write + new error check | inet_pton to `&svr_addr->sin_addr`, error check added | PASS |
| `src/PZ/pz_memory_conf.c` (x3) | Pattern A: Direct write | inet_pton writes directly to ip_addr field | PASS |
| `src/PX/px_memok.c` (x3) | Pattern D: Block-scoped tmp_ia, read-only | Block-scoped `struct in_addr tmp_ia`, writes to local only | PASS |
| `src/PX/px_sett2ip.c` (x1) | Pattern B: Function-scoped tmp_ia | `struct in_addr tmp_ia` at function scope, downstream memcpy | PASS |
| `src/PX/px_setudp.c` (x1) | Pattern B: Function-scoped tmp_ia | `struct in_addr tmp_ia` at function scope, downstream memcpy | PASS |
| `src/PA/pa_7100_ur.c` (x1) | Pattern E: Dead comment removal | Comment block deleted, inet_pton call preserved | PASS |
| `src/PA/pa_7000_mp.c` (x1) | Pattern E: Dead comment removal | Comment block deleted, inet_pton call preserved | PASS |

**결과**: 8/8 files match their designated transformation pattern.

---

## 6. Overall Scores

```
+---------------------------------------------+
|  Overall Match Rate: 100%                    |
+---------------------------------------------+
|  FR Items Verified:    17/17 (100%)          |
|  Verification Criteria: 5/5  (100%)          |
|  Variable Cleanup:     11/11 (100%)          |
|  Pattern Compliance:    8/8  (100%)          |
+---------------------------------------------+
```

| 범주 | Score | 상태 |
|----------|:-----:|:------:|
| Design Match (FR compliance) | 100% | PASS |
| Verification Criteria (Section 6) | 100% | PASS |
| Variable Cleanup | 100% | PASS |
| Pattern Compliance | 100% | PASS |
| **Overall** | **100%** | **PASS** |

---

## 7. Differences Found

### Missing Features (설계 O, Implementation X)

None.

### Added Features (설계 X, Implementation O)

None.

### Changed Features (설계 != Implementation)

None.

---

## 8. Supplementary Observations

### 8.1 Positive Observations

1. **Exact pattern adherence**: Every file follows its designated transformation pattern (A/B/C/D/E) precisely as specified in the design document.

2. **Complete variable elimination**: All 11 `ul` declarations were removed or replaced. Zero residual `ul` references remain across all 7 files.

3. **Bug fixes correct**: The 3 px_memok.c bugs (wrong struct target, SHM write in read-only tool, stale `ul` reference) are all fixed. Each call site now uses a block-scoped `struct in_addr tmp_ia` that writes only to local memory.

4. **Error handling improvement**: FR-04 (udp_init.c) added error checking where none existed before, using the correct severity (`UDP_FATAL`) and error pattern (`close + return -1`) consistent with the existing socket-open failure path.

5. **Log message cleanup**: pz_memory_conf.c Log messages were correctly updated to remove `ul:[%lu]` and `[%c]` format specifiers that referenced the eliminated variable.

6. **64-bit safety**: The `sizeof(u_long)` to `sizeof(tmp_ia)` change in px_sett2ip.c and px_setudp.c fixes potential 8-byte overwrites into 4-byte `ip_addr` fields on 64-bit platforms.

### 8.2 BACKUP Files

The only `inet_addr` references found in `src/` are in `src/PZ/BACKUP/pz_memory_conf.c` (3 occurrences). These are expected as BACKUP files are explicitly out of scope per the plan document.

---

## 9. Recommended Actions

### 9.1 Immediate

No immediate actions required. Implementation matches design at 100%.

### 9.2 설계 Document Updates Needed

None. The design document accurately describes the implemented changes.

### 9.3 Optional Follow-up

| 항목 | 설명 | 우선순위 |
|------|-------------|----------|
| Build verification | Compile all 7 files on target platform (Linux) to confirm no warnings | 낮음 |
| Runtime test | Execute px_memok against a running SHM to verify read-only behavior | 낮음 |

---

## 10. 다음 단계

- [x] Gap analysis complete (this document)
- [ ] Write completion report (`inet-addr-to-pton.report.md`)
- [ ] Archive PDCA documents

---

## 버전 기록

| 버전 | 날짜 | 변경사항 | 저자 |
|---------|------|---------|--------|
| 1.0 | 2026-02-25 | Initial analysis -- 100% match rate | Claude |
