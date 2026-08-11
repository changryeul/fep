# select-recv-bounds-check Completion Report

> **Feature**: select-recv-bounds-check (Feature #22)
> **요약**: select_recv.c 6개 TCP 수신 함수에 pkt_len 상한/하한 bounds check 추가 — 원격 버퍼 오버플로 방지
>
> **프로젝트**: FEP (Front-End Processor)
> **저자**: Claude
> **날짜**: 2026-02-25
> **상태**: PASS (100% 일치 Rate)

---

## 1. PDCA Cycle 요약

| 단계 | 상태 | Document |
|-------|--------|----------|
| Plan | Complete | `select-recv-bounds-check.plan.md` |
| 설계 | Complete | `select-recv-bounds-check.design.md` |
| Do | Complete | 1 file, 6 functions, +24 lines |
| Check | PASS (100%) | `select-recv-bounds-check.analysis.md` |
| Act | Skipped (100% at first check) | — |

---

## 2. Problem Statement

`sub/select_recv.c`의 6개 TCP 수신 함수가 네트워크에서 파싱한 `pkt_len` 값에 대해 **상한(upper-bound) 검증 없이** `Recvn()`을 호출. 공격자나 손상된 패킷이 거대한 길이 필드를 보내면 호출자의 고정 크기 버퍼를 넘겨 쓸 수 있는 critical 취약점.

추가로, 4개 함수의 기존 하한 검사가 **log-only** (Log 후 return 없이 계속 진행) — 실질적으로 검사 무효.

---

## 3. 변경사항 Applied

### 3.1 파일 영향

| 파일 | Lines Before | Lines After | Delta |
|------|-------------|-------------|-------|
| `st01/sub/select_recv.c` | 513 | 537 | +24 |

### 3.2 Per-Function 변경사항

| Function | FR | Change | Buffer | Max Safe |
|----------|-----|--------|--------|----------|
| `Select_Receive` | FR-01,02 | 상한 추가 + return NOTOK | `RecvPkt[5120]` | 5115 |
| `Select_Receive2` | FR-03,04 | 신규 guard 삽입 | `TCP_BUFF_MAX_LEN` | 5120 |
| `Select_Receive_Krx` | FR-05 | 상한 추가 (nested) | `DataBuff[4096]` | 4014 |
| `Select_Receive_Cli` | FR-06,07 | 상한 추가 + return NOTOK | `RecvPkt[4096]` | 4092 |
| `Select_Receive_Imeco` | FR-08,09 | 상한 추가 + return NOTOK | `RecvPkt[5120]` | 5116 |
| `Select_Receive_Imeco_Sise` | FR-10,11 | 상한 추가 + return NOTOK | `RecvPkt[5120]` | 5110 |

### 3.3 Bonus Fixes

- Log 메시지 함수명 4곳 수정 (`"Select_Receive:"` → 정확한 함수명: `_Cli`, `_Imeco`, `_Imeco_Sise`)
- C++ inline comment 1개 제거 (`// 헤더 50` → C89 호환)

### 3.4 범위 외 (확인)

- `Sise_Select_Receive` — 미수정 (이미 `recv()` + `SZ_FEEDDATA_MAX - sockpos` cap으로 안전)

---

## 4. Caller 영향 분석

모든 caller에서 `Select_Receive*()` 반환값 `NOTOK`은 이미 연결 끊김/에러로 처리하는 기존 로직으로 처리됨. 새로 추가된 bounds-check 실패도 동일한 `NOTOK` 반환이므로 caller 코드 변경 불필요.

| Caller | Function | Existing NOTOK Handling |
|--------|----------|------------------------|
| pa_1600_tr | Select_Receive | `if (rt == NOTOK)` → disconnect |
| pw_3010_tr | Select_Receive | `if (rt == NOTOK)` → disconnect |
| pw_3030_tr | Select_Receive | `if (rt == NOTOK)` → disconnect |
| pw_4000_ts | Select_Receive | `if (rt == NOTOK)` → disconnect |
| pa_3100_ts | Select_Receive_Krx | `if (rt == NOTOK)` → disconnect |
| pb_1800_ts | Select_Receive_Krx | `if (rt == NOTOK)` → disconnect |
| pa_8100_ts | Select_Receive_Cli | `if (rt == NOTOK)` → disconnect |
| pa_8200_tr | Select_Receive_Cli | `if (rt == NOTOK)` → disconnect |
| pb_7200_tr | Select_Receive_Cli | `if (rt == NOTOK)` → disconnect |
| pb_8100_ts | Select_Receive_Cli | `if (rt == NOTOK)` → disconnect |
| pb_8200_tr | Select_Receive_Cli | `if (rt == NOTOK)` → disconnect |
| pa_2100_ts | Select_Receive_Imeco | `if (rt == NOTOK)` → disconnect |
| pa_2200_tr | Select_Receive_Imeco | `if (rt == NOTOK)` → disconnect |
| pa_2700_tr | Select_Receive_Imeco_Sise | `if (rt == NOTOK)` → disconnect |

---

## 5. Metrics

| Metric | Value |
|--------|-------|
| **FR Items** | 11 (Plan: 12, FR-12 is defensive note) |
| **FR Match Rate** | 100% (11/11 PASS) |
| **Iterations** | 0 |
| **Files Modified** | 1 |
| **Functions Fixed** | 6 (of 7 total, 1 already safe) |
| **Lines Changed** | +24 |
| **Callers Affected** | 14 processes (zero code change needed) |
| **Bugs Fixed** | 6 upper-bound missing + 4 log-only (no return) = 10 |
| **Bonus Fixes** | 4 Log function name corrections + 1 C89 comment cleanup |
| **Functional Change** | Zero (정상 패킷 경로 불변) |
| **Security Impact** | Critical — 원격 버퍼 오버플로 6건 차단 |

---

## 6. 교훈

1. **Log-only validation은 검증이 아님** — 4개 함수에서 하한 검사를 수행했지만 `return` 없이 진행하여 실질적으로 검사가 무효. 모든 validation은 반드시 reject path를 포함해야 함.

2. **AtoIf()는 안전하지 않음** — 네트워크 데이터를 파싱하는 `AtoIf()`가 음수, 0, 거대한 값 등 어떤 값이든 반환할 수 있으나 caller가 이를 신뢰함. 향후 `AtoIf()` 자체에 범위 제한 또는 에러 반환 추가 고려.

3. **버퍼 크기 상수 활용** — `TCP_BUFF_MAX_LEN`, `KRX_DATA_BUFF_SIZE`, `CLI_BUFF_MAX_LEN` 등 이미 정의된 상수를 활용하여 하드코딩 없이 상한 설정 가능.

---

## 버전 기록

| 버전 | 날짜 | 변경사항 | 저자 |
|---------|------|---------|--------|
| 1.0 | 2026-02-25 | Completion report | Claude |
