# setsockopt-return-check Planning Document

> **요약**: Add return value checks to 6 unchecked setsockopt() calls in 2 files — log failures but don't abort (non-critical UDP buffer/broadcast options)
>
> **프로젝트**: FEP (Front-End Processor) for KRX
> **저자**: Claude
> **날짜**: 2026-02-25
> **상태**: 초안

---

## 1. 개요

### 1.1 목적

Six `setsockopt()` calls for UDP socket options (SO_SNDBUF, SO_RCVBUF, SO_BROADCAST) silently ignore failures. If the kernel rejects a buffer size or broadcast flag, the process continues without any indication, potentially causing performance degradation or multicast failures.

### 1.2 배경

All other `setsockopt()` calls in the codebase (10+ instances) already check return values:
- TCP options (SO_REUSEADDR, SO_LINGER) — checked with `rt = setsockopt(...)`, log on failure
- UDP multicast (IP_ADD_MEMBERSHIP) — checked with abort on failure
- Only these 6 UDP buffer/broadcast calls are unchecked

### 1.3 Pattern 분석

| 파일 | Checked | Unchecked | Pattern |
|------|:-------:|:---------:|---------|
| `sub/fep_common.c` | 1 (SO_LINGER) | 0 | `rt = ...; if (rt < 0) Log(TCP_ERROR, ...)` |
| `src/PA/pa_8100_ts.c` | 1 (SO_REUSEADDR) | 0 | `rt = ...; if (rt < 0) { Log; shutdown; close; }` |
| `src/PA/pa_7100_ur.c` | 2 (SO_REUSEADDR, IP_ADD_MEMBERSHIP) | 0 | `rt = ...; if (rt == -1) { SLog; return NOTOK; }` |
| `src/PB/pb_7100_ur.c` | 2 | 0 | Same pattern |
| `src/PB/pb_8200_tr.c` | 1 | 0 | Same pattern |
| `src/PB/pb_8100_ts.c` | 1 | 0 | Same pattern |
| `src/PA/pa_8200_tr.c` | 1 | 0 | Same pattern |
| `src/PA/pa_7000_tr.c` | 1 | 0 | Same pattern |
| **`sub/udp_init.c`** | **0** | **3** | No check at all |
| **`src/PA/pa_7000_mp.c`** | **0** | **3** | No check at all |

---

## 2. 범위

### 2.1 범위 내

- [x] Add return value checks to 3 setsockopt() calls in `sub/udp_init.c` (lines 50, 56, 62)
- [x] Add return value checks to 3 setsockopt() calls in `src/PA/pa_7000_mp.c` (lines 381, 382, 384)

### 2.2 범위 외

- Already-checked setsockopt() calls (10+ instances across 8 files)
- `.org` backup files (`pb_7100_ur.org`, `pb_7100_ts.org`)
- Test utilities (`utl/t_recv.c`)

---

## 3. 요구사항

### 3.1 기능 요구사항

| ID | 요구사항 | 파일 | Line(s) | 우선순위 |
|----|-------------|------|---------|----------|
| FR-01 | Check SO_SNDBUF return, log on failure | `sub/udp_init.c` | 50 | High |
| FR-02 | Check SO_RCVBUF return, log on failure | `sub/udp_init.c` | 56 | High |
| FR-03 | Check SO_BROADCAST return, log on failure | `sub/udp_init.c` | 62 | High |
| FR-04 | Check SO_SNDBUF return, log on failure | `src/PA/pa_7000_mp.c` | 381 | High |
| FR-05 | Check SO_RCVBUF return, log on failure | `src/PA/pa_7000_mp.c` | 382 | High |
| FR-06 | Check SO_BROADCAST return, log on failure | `src/PA/pa_7000_mp.c` | 384 | High |

### 3.2 설계 Decision: Log-Only, No Abort

UDP buffer/broadcast options are **non-critical optimizations**:
- `SO_SNDBUF` / `SO_RCVBUF`: Socket works with kernel defaults if custom size rejected
- `SO_BROADCAST`: Only needed for multicast — failure here should warn, not kill the process

Pattern: `UDP_WARN` severity, consistent with existing `pa_7100_ur.c` / `pb_7100_ur.c` patterns.

---

## 4. 성공 기준

- [x] All 6 setsockopt() calls check return value
- [x] `grep -n "setsockopt" st01/sub/udp_init.c st01/src/PA/pa_7000_mp.c` shows `rt =` prefix on all calls
- [x] Zero unchecked setsockopt() calls remain in active `sub/` and `src/` code

---

## 5. 위험 및 완화

| 위험 | 영향 | 확률 | 완화 |
|------|--------|------------|------------|
| Log noise in production | 낮음 || 낮음 | Use UDP_WARN (not FATAL), only fires on actual failure |
| Variable `rt` not declared | 낮음 || 낮음 | `udp_init.c` already has `int rt`; `pa_7000_mp.c` already has `int rt` |

---

## 버전 기록

| 버전 | 날짜 | 변경사항 | 저자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-25 | Initial draft | Claude |
