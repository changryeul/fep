# PDCA Completion Report: PB-HA (pb_7100_ur 이중화)

> **Summary**: Active/Standby UDP heartbeat-based HA implementation for pb_7100_ur (KRX UDP multicast bond market data relay)
>
> **Feature**: PB-HA (pb_7100_ur 이중화/HA)
> **Target File**: `st01/src/PB/pb_7100_ur.c`
> **Test File**: `st01/test/ha_test.c`
> **Match Rate**: 100% (12/12 FR + 7 NEW)
> **Iteration**: 2
> **Status**: Completed
> **Date**: 2026-03-07 (Final)
> **Initial Date**: 2026-03-06

---

## 1. Overview

### Feature Description

PB-HA implements automatic failover for pb_7100_ur process, which receives KRX UDP multicast bond market data and forwards it via TCP to connected clients.

**Problem Statement**:
- Two production servers (REAL1/REAL2) run simultaneously
- Without HA: both servers forward the same UDP data via TCP → receiver sees duplicates
- UDP data has NO sequence number → receiver-side deduplication impossible
- **Solution Required**: Both servers receive UDP, but only one forwards TCP with automatic failover

### Architecture

```
┌─────────────────────────────────────────────────────┐
│  KRX UDP Multicast (Port 5000)                      │
│  (Market data: 채권 시세 - bonds)                    │
└────────────────┬────────────────────────────────────┘
                 │
        ┌────────┴────────┐
        ▼                 ▼
    REAL1 (podm11)    REAL2 (podm12)
    Primary            Secondary
    UDP RX: O          UDP RX: O
    TCP TX: O          TCP TX: X (standby)
        │               │
        │◄──── UDP HB ───┤ 1sec interval
        │   HB_PACKET    │ "HB" + recv_cnt
        │   (Port 50000) │
        │                │
        └─────── TCP ───────► Receiver
                (Port per config)
```

**Failover Mechanisms** (2가지):
1. **HB Failover**: Primary heartbeat 3초 미수신 → Secondary Active 전환
2. **DATA Failover**: Primary는 살아있으나 시세 미수신 5회 연속 → Secondary Active 전환

**Failback**:
- Primary HB 연속 3회 수신 + Primary 시세 수신 중 (또는 양쪽 없음) → Standby 복귀
- Primary가 살아있지만 시세 미수신 시 Failback 차단

### Key Information

| Item | Value |
|------|-------|
| Module | PB (pb_7100_ur process) |
| Environment Variable | `_FEP_DIV` (REAL1/REAL2/TEST) |
| Heartbeat Port | 50000 (configurable via `_HA_HB_PORT`) |
| Heartbeat Interval | 1 second |
| HB Packet | `HB_PACKET` = "HB"(2B) + recv_cnt(int) |
| Failover Timeout | 3 seconds (HB_TIMEOUT) |
| Data Failover | 5 consecutive (HB_DATA_TIMEOUT) |
| Failback Stable | 3 consecutive HB (HB_STABLE_COUNT) |
| Lines Added | ~300 lines |
| Functions Added | 4 (HA_Init, HA_Heartbeat_Send, HA_Check_Failover, Device_Close_HA) |
| Functions Modified | 2 (Recv_Data, PB_7100_UR) |

---

## 2. PDCA Cycle Summary

### Plan Phase
- **Status**: Complete
- **Key Decisions**:
  - UDP heartbeat pattern (not TCP-based) for simplicity
  - _FEP_DIV environment variable for role determination
  - Active/Standby model (not Active/Active)

### Design Phase
- **Status**: Complete
- **Key Design Points**:
  - HA_Init() at startup to determine role and bind heartbeat socket
  - Recv_Data() modified for 1-second timeout
  - Heartbeat socket integration into select()
  - TCP gating via ha_active flag

### Do Phase (Implementation)
- **Status**: Complete
- **Iterations**:
  - Iteration 0: Core HA (FR-01~12, FR-08 GAP)
  - Iteration 1: FR-08 해결 (Device_Close_HA), 시세 기반 절체 (NEW-1~7)
  - Iteration 2: my_recv_cnt 버그 수정, 테스트 검증 완료

### Check Phase (Gap Analysis)
- **Status**: Complete (100% match rate)
- **Analysis Results**: 12/12 FR PASS + 7 NEW features verified

### Act Phase (Improvement)
- **Status**: Complete
- **Actions taken**:
  - FR-08 해결: Device_Close_HA() 추가 (sleep 없는 TCP close)
  - 시세 기반 Failover/Failback 추가
  - my_recv_cnt 스냅샷+리셋 버그 수정

---

## 3. Functional Requirements Validation

### Original Requirements (FR-01 ~ FR-12)

| # | Requirement | Status | Implementation |
|---|-------------|:------:|----------------|
| FR-01 | _FEP_DIV determines role | PASS | L860-878: REAL1→Primary, REAL2→Secondary, else→Standalone |
| FR-02 | Heartbeat socket creation/bind | PASS | L921-963: UDP socket + SO_REUSEADDR + role-based bind |
| FR-03 | Primary 1-sec heartbeat TX | PASS | L984-1009: HB_PACKET with recv_cnt, delta check |
| FR-04 | Secondary HB RX (select integration) | PASS | L611-677: hb_sockfd in select, non-blocking drain |
| FR-05 | Failover: HB timeout 3sec → Active | PASS | L1045-1052: elapsed >= HB_TIMEOUT |
| FR-06 | Failback: HB stable 3x → Standby | PASS | L1097-1107: hb_stable_cnt >= HB_STABLE_COUNT |
| FR-07 | ha_active==OFF: TCP skip, UDP continue | PASS | L386-388: continue after my_recv_cnt++ |
| FR-08 | TCP safe close during failback | PASS | L216+: Device_Close_HA() (no sleep) |
| FR-09 | TEST → STANDALONE (always TX) | PASS | L875-878: HA_STANDALONE, return OK |
| FR-10 | HA_Init fail → STANDALONE fallback | PASS | L295-297: NOTOK → STANDALONE |
| FR-11 | Exit cleanup: hb_sockfd close | PASS | L430-433: if >= 0 close + set -1 |
| FR-12 | select() timeout 1 second | PASS | L623-624: tv_sec=1, tv_usec=0 |

### Additional Features (NEW-1 ~ NEW-7)

| # | Feature | Status | Implementation |
|---|---------|:------:|----------------|
| NEW-1 | HB_PACKET with recv_cnt | PASS | L97-102: typedef struct { hb_mark[2]; int recv_cnt; } |
| NEW-2 | DATA Failover (시세 기반 절체) | PASS | L1064-1078: pri_recv_cnt==0 && my_data>0, 5회 연속 |
| NEW-3 | Failback blocked (시세 미수신 Primary) | PASS | L1109-1116: pri_recv_cnt<=0 && my_data>0 → 차단 |
| NEW-4 | my_recv_cnt snapshot+reset | PASS | L1037-1038: my_data=my_recv_cnt; my_recv_cnt=0 |
| NEW-5 | Legacy 2-byte HB compatibility | PASS | L662-667: rt==2 → pri_recv_cnt=-1 |
| NEW-6 | Device_Close_HA (no sleep) | PASS | L216-228: FR-08 해결 |
| NEW-7 | hb_stable_cnt reset (log spam 방지) | PASS | L1112: hb_stable_cnt=0 in blocked branch |

---

## 4. Test Verification

### ha_test.c — Standalone HA Test Program

6개 테스트 모드로 모든 시나리오 검증 완료:

| Mode | Scenario | Result | Verified |
|------|----------|:------:|:--------:|
| `primary` | Primary HB 송신 (수동 테스트) | PASS | 2026-03-07 |
| `secondary` | Secondary HB 수신 (수동 테스트) | PASS | 2026-03-07 |
| `auto` | HB Failover → Failback 자동 시나리오 | PASS | 2026-03-07 |
| `oscillation` | FR-08 진동 재현 (Device_Close sleep) | PASS (진동 재현) | 2026-03-06 |
| `fixed` | FR-08 수정 확인 (Device_Close_HA) | PASS (진동 없음) | 2026-03-07 |
| `nodata` | DATA Failover + Failback | PASS | 2026-03-07 |

### Test Results Detail

**auto mode** (HB 기반 Failover/Failback):
```
Phase 1: Primary=Active, Secondary=Standby (8초) → 정상
Phase 2: Primary 종료 → 3초 후 FAILOVER(HB) → Secondary Active
Phase 3: Primary 재기동 → HB 3회 연속 → FAILBACK → Secondary Standby
Phase 4: 정상 복귀 확인, 진동 없음
```

**nodata mode** (시세 기반 Failover/Failback):
```
Phase 1: 양쪽 정상 (5초) → Standby 유지
Phase 2: Primary 시세 끊김 → 5회 연속 → FAILOVER(DATA) → Secondary Active
         FAILBACK blocked: Primary alive but no data (정상 차단)
Phase 3: Primary 시세 복구 → HB 3회 연속 + pri_data>0 → FAILBACK
```

### Bugs Found and Fixed During Testing

| Bug | Root Cause | Fix |
|-----|-----------|-----|
| auto mode Secondary 출력 안 보임 | MSG_DONTWAIT 값 macOS/Linux 차이 → 무한 drain 루프 | select()+blocking recvfrom 방식으로 변경 |
| fork 자식 stdout 안 보임 | fork 후 full buffered stdout → pipe에 flush 안 됨 | fflush+setvbuf(_IOLBF) |
| FR-08 진동 | Device_Close sleep(3) → HB 수신 차단 → 재 failover | Device_Close_HA (sleep 없음) |
| 장 종료 시 false DATA FAILOVER | my_recv_cnt 누적 → 시세 없어도 my_recv_cnt>0 | snapshot+reset (my_data=my_recv_cnt; my_recv_cnt=0) |
| FAILBACK blocked 로그 반복 | hb_stable_cnt가 리셋 없이 3 도달 반복 | hb_stable_cnt=0 in blocked branch |

---

## 5. Deployment Checklist

| Item | Status | Action |
|------|:------:|--------|
| pkg_env.sh HA 환경변수 | TODO | `_HA_PEER_IP`, `_HA_HB_PORT` 추가 |
| 방화벽 UDP 50000 | TODO | REAL1↔REAL2 양방향 개통 |
| HB_PACKET 구조체 패딩 확인 | TODO | 양쪽 동일 컴파일러/옵션 확인 또는 `#pragma pack` |
| 운영 테스트 | TODO | Primary kill → Secondary Active → Primary 복구 → Failback |

### pkg_env.sh 추가 설정

```bash
case "${_FEP_DIV}" in
  REAL1)
    export _HA_PEER_IP="192.168.151.12"  # REAL2(podm12) IP
    export _HA_HB_PORT="50000"
    ;;
  REAL2)
    export _HA_PEER_IP="192.168.151.11"  # REAL1(podm11) IP
    export _HA_HB_PORT="50000"
    ;;
esac
```

---

## 6. Known Limitations & Future Improvements

| # | Item | Severity | Description |
|---|------|:--------:|-------------|
| 1 | HB_PACKET 구조체 패딩 | Medium | `char[2]+int` 사이 2B 패딩 가능. 양쪽 동일 환경이면 무해하나, `#pragma pack(1)` 명시 권장 |
| 2 | 바이트 오더 | Low | 양쪽 x86_64 Linux → 문제없음. 이기종 혼용 시 `htonl/ntohl` 필요 |
| 3 | HB_TIMEOUT 고정값 | Low | `#define 3` 고정. 환경변수화 가능 |
| 4 | 역방향 heartbeat 없음 | Low | Primary→Secondary 단방향. 양방향으로 확장 가능 |
| 5 | Failover 이력 카운터 없음 | Low | 운영 모니터링용 카운터 추가 가능 |

---

## 7. Source File Reference

### pb_7100_ur.c Key Line Ranges

| Section | Lines | Description |
|---------|-------|-------------|
| HB_PACKET typedef | 97-102 | HB 패킷 구조체 정의 |
| HA constants | 85-113 | HB_PORT, HB_TIMEOUT, HB_STABLE_COUNT, HB_DATA_TIMEOUT |
| HA global variables | 139-150 | ha_role, ha_active, hb_sockfd, recv_cnt 등 |
| Device_Close_HA | 216-228 | sleep 없는 TCP close (FR-08 해결) |
| PB_7100_UR main loop | 308-436 | HA heartbeat + TCP gating + UDP recv |
| Recv_Data HB handling | 611-677 | select + HB_PACKET 파싱 + drain loop |
| HA_Init | 848-973 | 역할 결정, 소켓 생성, bind |
| HA_Heartbeat_Send | 984-1009 | Primary HB 전송 + my_recv_cnt 리셋 |
| HA_Check_Failover | 1026-1119 | Failover/Failback 판단 (HB + DATA) |

### ha_test.c

| Section | Description |
|---------|-------------|
| HB_PACKET | pb_7100_ur.c와 동일 구조 |
| HA_Check_Failover | pb_7100_ur.c와 동일 로직 (my_data snapshot+reset 포함) |
| 6 test modes | primary, secondary, auto, oscillation, fixed, nodata |

---

## 8. Sign-Off

### Deliverables Completed

- [x] HA role determination (REAL1/REAL2/TEST)
- [x] Heartbeat UDP socket creation and management
- [x] HB_PACKET with market data count (recv_cnt)
- [x] Primary → Secondary heartbeat transmission (1 sec interval)
- [x] Secondary → Primary heartbeat reception (3 sec timeout)
- [x] Automatic failover on heartbeat loss (HB Failover)
- [x] Automatic failover on market data loss (DATA Failover)
- [x] Automatic failback on Primary recovery (with data check)
- [x] Failback blocked when Primary alive but no data
- [x] TCP transmission gated by ha_active flag
- [x] Device_Close_HA for oscillation-free failback (FR-08)
- [x] my_recv_cnt snapshot+reset (false failover prevention)
- [x] Legacy 2-byte HB backward compatibility
- [x] Graceful fallback to STANDALONE if HA init fails
- [x] Environment variable integration (_FEP_DIV, _HA_PEER_IP, _HA_HB_PORT)
- [x] Process exit cleanup (heartbeat socket close)
- [x] select() timeout 1 second for heartbeat responsiveness
- [x] Standalone test program (ha_test.c) with 6 test modes
- [x] All test modes verified (auto, fixed, nodata)

### Match Rate: 100% (19/19 Requirements PASS)

### Overall Assessment

**COMPLETED** — All original requirements met (12/12), all additional features verified (7/7), FR-08 GAP resolved, comprehensive test coverage with ha_test.c.

---

**Report Generated**: 2026-03-06
**Last Updated**: 2026-03-07
**Status**: Completed
