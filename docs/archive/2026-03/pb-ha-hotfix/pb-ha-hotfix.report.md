# PDCA Completion Report: pb-ha-hotfix (PB-HA Heartbeat 수신 불가 긴급 수정)

> **Summary**: PB-HA heartbeat 수신 불가 긴급 버그 수정 — 7개 버그 식별 및 전량 수정
>
> **Feature**: pb-ha-hotfix
> **Target Files**: `st01/src/PB/pb_7100_ur.c`, `st01/src/PA/pa_7100_ur.c`
> **Test File**: `st01/test/ha_test.c`
> **Match Rate**: 100% (7/7 BUG + 11/11 FR)
> **Iteration**: 0 (첫 분석에서 100% 달성)
> **Status**: Completed
> **Date**: 2026-03-11
> **Plan**: `docs/01-plan/features/pb-ha-hotfix.plan.md`
> **Design**: `docs/02-design/features/pb-ha-hotfix.design.md`
> **Analysis**: `docs/03-analysis/pb-ha-hotfix.analysis.md`

---

## 1. Overview

### 1.1 Feature Description

PB-HA 이중화 기능(2026-03-07 완료) 배포 후, 실 서버에서 **Secondary가 Primary의 heartbeat를 수신하지 못하는** 긴급 버그가 발견되었다. UDP 자체는 서버 간 전달되나 프로그램 레벨에서 수신 실패하여 양쪽 모두 ACTIVE 상태로 고착되는 현상이 발생했다.

### 1.2 Root Cause Chain

```
[시작] 야간 운용 (TCP 서버 미가동)

① Primary: Device_Open() → Connect() 블로킹 (~75초)
   └ TCP 서버 미응답 → OS 기본 타임아웃까지 차단
   └ HB 전송 간격 ~75초로 늘어남

② Secondary: HB_TIMEOUT(3초) 초과 → FAILOVER (ha_active=ON)
   └ 정상적 절체 — Primary가 죽은 것으로 판단

③ Secondary(ACTIVE): Device_Open() → Connect 실패 → SockTcpfd=-1
   └ if (SockTcpfd < 0) continue;  ← ★ 핵심 버그 (BUG-06) ★
   └ Recv_Data() 스킵 → heartbeat recvfrom() 호출 안 됨
   └ HB가 소켓에 쌓이지만 프로그램이 읽지 않음

④ Primary: Connect 성공 → HB 정상 전송 재개
   └ Secondary는 ③에서 HB를 못 읽으므로 Failback 불가
   └ 양쪽 모두 ACTIVE → 시세 이중 전달

⑤ 야간: TCP 서버 미가동 → ③ 반복 → 영구 고착
```

---

## 2. Bug Summary

| ID | 분류 | 내용 | 심각도 |
|----|------|------|--------|
| BUG-01 | Socket | SO_RCVBUF 미설정 (SO_REUSEADDR로 잘못 지정) | High |
| BUG-02 | HB수신 | recvfrom 에러 무시 + 크기 조건 gap (rt==2) | Medium |
| BUG-03 | 포트 | pb_7102/7103 HB 포트 동일 (동시 실행 시 충돌) | Medium |
| BUG-04 | 블로킹 | Device_Open: Connect() 타임아웃 없음 (~75초) | **Critical** |
| BUG-05 | 블로킹 | Device_Close: sleep(3) → HB 차단 | High |
| **BUG-06** | **메인루프** | **TCP실패 시 `continue`가 Recv_Data 스킵** | **Critical** |
| BUG-07 | 타이밍 | HB_TIMEOUT(3초)이 Connect2+select와 동일 | High |

---

## 3. Implementation Results

### 3.1 Bug Fixes (7/7)

| BUG | 수정 내용 | 파일:라인 |
|-----|-----------|-----------|
| BUG-01 | SO_REUSEADDR(val=1) + SO_RCVBUF(val=1.2MB) 분리 호출 | pb_7100_ur.c:556-570 |
| BUG-02 | recvfrom rt<0 에러 로그+break, rt>=2 호환 범위 확장 | pb_7100_ur.c:694-721 |
| BUG-03 | `#if B7102` base+0, `#elif B7103` base+1 포트 분리 | pb_7100_ur.c:89-95, 979-985 |
| BUG-04 | Connect→Connect2(alarm 2초 타임아웃) | pb_7100_ur.c:203 |
| BUG-05 | Device_Close sleep(3) 제거 + Device_Close_HA 별도 함수 | pb_7100_ur.c:220-232 |
| BUG-06 | `continue` 제거 + TCP_RETRY_INTVL(5초) 간격 재시도 | pb_7100_ur.c:349-380 |
| BUG-07 | HB_TIMEOUT 3초→5초 | pb_7100_ur.c:97 |

### 3.2 Functional Requirements (11/11)

| FR | Requirement | Status |
|----|-------------|:------:|
| FR-01 | SO_RCVBUF 1.2MB 정확 설정 | ✅ |
| FR-02 | HB recvfrom 에러 시 로그 출력 + 루프 탈출 | ✅ |
| FR-03 | rt==2 → rt>=2 크기 범위 확장 | ✅ |
| FR-04 | pb_7102_ur=50000, pb_7103_ur=50001 포트 분리 | ✅ |
| FR-05 | Connect→Connect2(2초 타임아웃) | ✅ |
| FR-06 | Device_Close sleep(3) 제거 | ✅ |
| FR-07 | pa_7100_ur.c 동일 SO_RCVBUF 수정 | ✅ |
| FR-08 | TCP 실패 시 continue 제거 (Recv_Data 스킵 방지) | ✅ |
| FR-09 | TCP_RETRY_INTVL(5초) 재연결 간격 도입 | ✅ |
| FR-10 | HB_TIMEOUT 3초→5초 변경 | ✅ |
| FR-11 | HA Init/Send/Recv 진단 로그 추가 | ✅ |

### 3.3 Main Loop Flow (수정 후)

```c
while (START_S != JOB_END) {
    if (ha_role == HA_PRIMARY) HA_Heartbeat_Send();
    else if (ha_role == HA_SECONDARY) HA_Check_Failover();

    if (ha_active == ON) {
        if (SockTcpfd < 0) {
            time_t now = time(NULL);
            if (now - tcp_last_try >= TCP_RETRY_INTVL) {  /* 5초 간격 */
                Device_Open();          /* Connect2(2초 타임아웃) */
                tcp_last_try = now;
            }
        }
        /* continue 제거: Recv_Data 항상 호출 보장 */
    } else {
        if (SockTcpfd >= 0) Device_Close_HA();
    }

    rt = Recv_Data(rbuf);              /* HB + 시세 항상 수신 */
    ...
    if (ha_active == OFF || SockTcpfd < 0) continue;  /* TCP 전송만 건너뜀 */
    Select_Send(SockTcpfd, rbuf, ...);
}
```

### 3.4 Timing Constants

| Constant | Value | Purpose |
|----------|:-----:|---------|
| HB_SEND_INTVL | 1초 | HB 전송 간격 |
| HB_TIMEOUT | 5초 | 절체 판단 타임아웃 |
| HB_STABLE_COUNT | 3회 | 복귀 판단 안정 횟수 |
| TCP_RETRY_INTVL | 5초 | TCP 재연결 간격 |
| Connect2 timeout | 2초 | TCP 연결 타임아웃 |
| HB_DATA_TIMEOUT | 5회 | 시세 기반 절체 횟수 |

---

## 4. Gap Analysis Results

### 4.1 Match Rate

```
+---------------------------------------------+
|  Overall Match Rate: 100%                    |
+---------------------------------------------+
|  Bug Fixes (7/7):          100%    ✅        |
|  Functional Reqs (11/11):  100%    ✅        |
|  Main Loop Flow:           100%    ✅        |
|  Timing Constants:         100%    ✅        |
|  PA Module (FR-07):        100%    ✅        |
|  Test Program (8/8 modes): 100%    ✅        |
|  Architecture/Convention:  100%    ✅        |
+---------------------------------------------+
```

### 4.2 Minor Issues (Info only)

주석 내 HB_TIMEOUT=3초 잔재 2건 → 5초로 수정 완료 (기능 무영향).

---

## 5. Test Results

### 5.1 ha_test.c 테스트 모드 (8종)

| 모드 | 명령어 | 검증 내용 | 결과 |
|------|--------|-----------|:----:|
| primary | `./ha_test primary` | HB 전송 동작 | ✅ |
| secondary | `./ha_test secondary` | HB 수신 + 절체/복귀 | ✅ |
| auto | `./ha_test auto` | Failover→Failback | ✅ |
| oscillation | `./ha_test oscillation` | sleep(3) 진동 재현 | ✅ |
| fixed | `./ha_test fixed` | 진동 수정 검증 | ✅ |
| nodata | `./ha_test nodata` | DATA FAILOVER | ✅ |
| **tcpfail-bug** | `./ha_test tcpfail-bug` | **BUG-06 재현** | ✅ |
| **tcpfail-fix** | `./ha_test tcpfail-fix` | **BUG-06 수정 검증** | ✅ |

### 5.2 야간 타이밍 분석

```
                           Connect2
루프 1: HB전송 → (스킵) → Recv_Data(1초) → ~1초
루프 2: HB전송 → (스킵) → Recv_Data(1초) → ~1초
...
루프 5: HB전송 → Connect2(2초) → Recv_Data(1초) → ~3초  ← 5초 경과, 재시도
...

HB 전송 간격: 대부분 ~1초, 5초에 1번 ~3초
HB_TIMEOUT(5초) 이내: 최소 3~4회 HB 전송 보장 → 오판 절체 없음
```

---

## 6. PDCA Cycle Summary

| Phase | Date | Output | Status |
|-------|------|--------|:------:|
| Plan | 2026-03-10 | `docs/01-plan/features/pb-ha-hotfix.plan.md` | ✅ |
| Design | 2026-03-11 | `docs/02-design/features/pb-ha-hotfix.design.md` | ✅ |
| Do | 2026-03-11 | pb_7100_ur.c + pa_7100_ur.c + ha_test.c | ✅ |
| Check | 2026-03-11 | `docs/03-analysis/pb-ha-hotfix.analysis.md` (100%) | ✅ |
| Act | - | 불필요 (100% 달성) | ✅ |

### PDCA Metrics

| Metric | Value |
|--------|-------|
| Match Rate | 100% |
| Iterations | 0 (첫 분석 100%) |
| Bugs Identified | 7 |
| FRs Implemented | 11 |
| Files Modified | 3 (pb_7100_ur.c, pa_7100_ur.c, ha_test.c) |
| Test Modes | 8 (기존 6 + 신규 2) |

---

## 7. Remaining Tasks (Server Deployment)

| Item | Status | Verification |
|------|:------:|-------------|
| mk.sh pb ur 빌드 | Pending | 서버에서 실행 |
| mk.sh pa ur 빌드 | Pending | 서버에서 실행 |
| Primary "HA HB sent" 로그 | Pending | `tail -f st03/LOG/PB/pb_7102_ur.log` |
| Secondary "HA HB recv ok" 로그 | Pending | `tail -f st03/LOG/PB/pb_7102_ur.log` |
| SO_RCVBUF 확인 | Pending | `ss -ulnm` 또는 `/proc/net/udp` |
| 24시간 오판 절체 0건 | Pending | 운영 모니터링 |

### 서버 배포 절차

```sh
# 1. 빌드
mk.sh pb ur             # pb_7102_ur 빌드
mk.sh pa ur             # pa_7102_ur 빌드

# 2. 로그 확인
tail -f st03/LOG/PB/pb_7102_ur.log | grep "HA "

# 3. 절체 테스트 (podm11에서)
kill $(pgrep pb_7102_ur)
# podm12 로그: 5초 후 "FAILOVER" 확인
# podm11 재기동 후: podm12 로그 "FAILBACK" 확인
```

---

## 8. Lessons Learned

### 8.1 핵심 교훈

1. **`continue`의 영향 범위를 항상 확인**: `if (fail) continue;`가 어떤 코드를 건너뛰는지 전체 루프 흐름에서 확인해야 함. BUG-06은 TCP 관련 `continue`가 무관한 HB 수신까지 차단한 사례.

2. **블로킹 함수는 HA 환경에서 치명적**: `Connect()`의 75초 블로킹, `sleep(3)`은 단독 프로세스에서는 무해하나, HB 기반 HA에서는 오판 절체를 유발함. 타임아웃 있는 대안(`Connect2`) 사용 필수.

3. **실 환경 시나리오 기반 디버깅**: 초기에 SO_RCVBUF, 포트 충돌 등 여러 가설을 세웠으나, 사용자의 실 운영 정보("시세 없을 때도 안 됨", "pb_7102_ur만 사용", "야간 TCP 미가동")를 통해 BUG-06이 핵심임을 확인함.

### 8.2 PB-HA 기존 보고서 대비 변경

| 항목 | PB-HA (2026-03-07) | pb-ha-hotfix (2026-03-11) |
|------|---------------------|---------------------------|
| HB_TIMEOUT | 3초 | 5초 |
| Device_Open | Connect() | Connect2(2초) |
| Device_Close | sleep(3) 포함 | sleep 제거 |
| 메인루프 | TCP실패 시 continue | continue 제거 + TCP_RETRY_INTVL |
| 테스트 모드 | 6종 | 8종 (+tcpfail-bug, tcpfail-fix) |

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2026-03-11 | PDCA completion report — 7 bugs, 11 FRs, 100% match | Claude Code |
