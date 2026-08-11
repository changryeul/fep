# pb-ha-hotfix Analysis Report

> **Analysis Type**: Gap Analysis (Design vs Implementation)
>
> **Project**: FEP (Front-End Processor)
> **Analyst**: Claude Code (gap-detector)
> **Date**: 2026-03-11
> **Design Doc**: [pb-ha-hotfix.design.md](../02-design/features/pb-ha-hotfix.design.md)
> **Plan Doc**: [pb-ha-hotfix.plan.md](../01-plan/features/pb-ha-hotfix.plan.md)

---

## 1. Analysis Overview

### 1.1 Analysis Purpose

PB-HA heartbeat 수신 불가 긴급 버그 수정(7건) + PA SO_RCVBUF 수정(FR-07) + 테스트 프로그램 동기화에 대해 설계 문서와 실제 구현의 일치도를 검증한다.

### 1.2 Analysis Scope

| Item | Path |
|------|------|
| Design Document | `docs/02-design/features/pb-ha-hotfix.design.md` |
| Plan Document | `docs/01-plan/features/pb-ha-hotfix.plan.md` |
| Implementation (PB) | `st01/src/PB/pb_7100_ur.c` |
| Implementation (PA) | `st01/src/PA/pa_7100_ur.c` |
| Test Program | `st01/test/ha_test.c` |

---

## 2. Gap Analysis (Design vs Implementation)

### 2.1 Bug Fix Verification

| BUG ID | Design Specification | Implementation | Status | Notes |
|--------|---------------------|----------------|:------:|-------|
| BUG-01 | SO_REUSEADDR/SO_RCVBUF 분리 호출 | pb_7100_ur.c:556-570 SO_REUSEADDR(val=1) + SO_RCVBUF(val=UDP_SOCK_RCVBUF_SIZE) 분리 | ✅ Match | 정확히 설계대로 구현 |
| BUG-02 | recvfrom 에러체크 + rt>=2 범위 확장 | pb_7100_ur.c:694-721 rt<0 에러로그+break, rt>=HB_PKT_LEN 정규, rt>=2 호환 | ✅ Match | 설계의 3단계 분기 완전 구현 |
| BUG-03 | HB 포트 프로세스별 분리 (B7102=base+0, B7103=base+1) | pb_7100_ur.c:89-95 컴파일타임 + pb_7100_ur.c:979-985 환경변수 offset | ✅ Match | #if B7102/B7103 + env base port 정확 |
| BUG-04 | Device_Open: Connect -> Connect2(2초 타임아웃) | pb_7100_ur.c:203 `Connect2(SockTcpfd, IpAddr, TCP2_PORT_NO, 2)` | ✅ Match | alarm 기반 2초 타임아웃 적용 |
| BUG-05 | Device_Close: sleep(3) 제거 | pb_7100_ur.c:220-232 sleep 호출 없음, 주석으로 제거 이유 설명 | ✅ Match | Device_Close_HA도 sleep 없이 별도 구현 |
| BUG-06 | continue 제거 + TCP_RETRY_INTVL 도입 | pb_7100_ur.c:349-380 continue 없음, TCP_RETRY_INTVL=5초 간격 재시도 | ✅ Match | 핵심 버그 수정 완전 일치 |
| BUG-07 | HB_TIMEOUT 3초 -> 5초 | pb_7100_ur.c:97 `#define HB_TIMEOUT 5` | ✅ Match | |

### 2.2 Functional Requirements Verification

| FR ID | Requirement | Implementation | Status |
|-------|------------|----------------|:------:|
| FR-01 | SO_RCVBUF 1.2MB 정확 설정 | SO_REUSEADDR(val=1) + SO_RCVBUF(val=UDP_SOCK_RCVBUF_SIZE) 분리 | ✅ |
| FR-02 | HB recvfrom 에러시 로그+루프탈출 | rt<0 시 Log(UDP_WARN) + break | ✅ |
| FR-03 | rt==2 -> rt>=2 확장 | `rt >= 2 && memcmp(hb_buf, "HB", 2) == 0` | ✅ |
| FR-04 | 포트 분리 (50000/50001) | #if B7102 base+0, #elif B7103 base+1 | ✅ |
| FR-05 | Connect -> Connect2(2초) | `Connect2(SockTcpfd, IpAddr, TCP2_PORT_NO, 2)` | ✅ |
| FR-06 | Device_Close sleep(3) 제거 | sleep 호출 제거, 주석 설명 포함 | ✅ |
| FR-07 | pa_7100_ur.c SO_RCVBUF 수정 | pa_7100_ur.c:289-304 SO_REUSEADDR(val=1) + SO_RCVBUF 분리 | ✅ |
| FR-08 | TCP 실패 시 continue 제거 | continue 없음, Recv_Data 항상 호출 보장 | ✅ |
| FR-09 | TCP_RETRY_INTVL(5초) 도입 | pb_7100_ur.c:162 `#define TCP_RETRY_INTVL 5` + L361 간격 체크 | ✅ |
| FR-10 | HB_TIMEOUT 3->5초 | `#define HB_TIMEOUT 5` | ✅ |
| FR-11 | HA 진단 로그 추가 | HA_Init/Send/Recv 전 구간 Log 호출 존재 | ✅ |

### 2.3 Main Loop Flow Comparison

**Design (Section 2.2):**
```
while (START_S != JOB_END) {
    HA_Heartbeat_Send();          or HA_Check_Failover()
    if (ha_active == ON) {
        if (SockTcpfd < 0) {
            if (now - tcp_last_try >= TCP_RETRY_INTVL) {
                Device_Open();
                tcp_last_try = now;
            }
        }
        /* continue 없음 */
    } else {
        if (SockTcpfd >= 0) Device_Close_HA();
    }
    rt = Recv_Data(rbuf);
    ...
    if (ha_active == OFF || SockTcpfd < 0) continue;
    /* TCP 전송 */
}
```

**Implementation (pb_7100_ur.c:326-460):**
```
while (START_S != JOB_END) {
    if (ha_role == HA_PRIMARY) HA_Heartbeat_Send();
    else if (ha_role == HA_SECONDARY) HA_Check_Failover();

    if (ha_active == ON) {
        if (SockTcpfd < 0) {
            time_t now = time(NULL);
            if (now - tcp_last_try >= TCP_RETRY_INTVL) {
                Device_Open();
                tcp_last_try = now;
            }
        }
        /* continue 없음 — 주석으로 이유 설명 */
    } else {
        if (SockTcpfd >= 0) Device_Close_HA();
    }

    rt = Recv_Data(rbuf);
    ...
    if (ha_active == OFF || SockTcpfd < 0) continue;
    /* TCP 전송 */
}
```

| Comparison Point | Design | Implementation | Status |
|-----------------|--------|----------------|:------:|
| HB 처리 (role 분기) | `HA_Heartbeat_Send() or HA_Check_Failover()` | `if (ha_role == HA_PRIMARY) ... else if ...` | ✅ Match |
| TCP_RETRY_INTVL 간격 제한 | `now - tcp_last_try >= TCP_RETRY_INTVL` | 동일 | ✅ Match |
| continue 제거 | 주석으로 "continue 제거" 명시 | 제거 + 상세 이유 주석 | ✅ Match |
| Device_Close_HA 호출 | `Device_Close_HA()` | 동일 | ✅ Match |
| Recv_Data 항상 호출 | `rt = Recv_Data(rbuf)` | 동일 | ✅ Match |
| TCP 전송 조건 | `ha_active == OFF \|\| SockTcpfd < 0` | 동일 | ✅ Match |

### 2.4 Timing Constants Comparison

| Constant | Design | Implementation | Status |
|----------|--------|----------------|:------:|
| HB_SEND_INTVL | 1초 | 1 (L96) | ✅ |
| HB_TIMEOUT | 5초 | 5 (L97) | ✅ |
| HB_STABLE_COUNT | 3회 | 3 (L98) | ✅ |
| TCP_RETRY_INTVL | 5초 | 5 (L162) | ✅ |
| Connect2 timeout | 2초 | 2 (L203) | ✅ |
| HB_DATA_TIMEOUT | 5회 | 5 (L99) | ✅ |

### 2.5 PA Module (FR-07) Verification

| Item | Design | Implementation (pa_7100_ur.c) | Status |
|------|--------|-------------------------------|:------:|
| SO_REUSEADDR | val=1, SO_REUSEADDR | L290-295: `val = 1; setsockopt(SO_REUSEADDR)` | ✅ |
| SO_RCVBUF | val=UDP_SOCK_RCVBUF_SIZE, SO_RCVBUF | L298-304: `val = UDP_SOCK_RCVBUF_SIZE; setsockopt(SO_RCVBUF)` | ✅ |
| 에러 처리 | WARN 로그 | L293-295, L301-303: SLog(UDP_WARN) | ✅ |
| HA 미적용 | PA는 HA 없음, BUG-01만 수정 | HA 관련 코드 없음 (설계대로) | ✅ |

### 2.6 Test Program (ha_test.c) Verification

| Test Mode | Design | Implementation | Status |
|-----------|--------|----------------|:------:|
| primary | HB 전송 동작 확인 | `run_primary()` (L419-433) | ✅ |
| secondary | HB 수신 + 절체/복귀 판단 | `run_secondary(0)` (L442-501) | ✅ |
| auto | Failover -> Failback 자동 시나리오 | `run_auto()` (L506-582) | ✅ |
| oscillation | FR-08 진동 재현 (sleep(3) 버그) | `run_oscillation()` (L587-662) | ✅ |
| fixed | FR-08 수정 검증 (진동 없음) | `run_fixed()` (L670-742) | ✅ |
| nodata | 시세 기반 절체 (DATA FAILOVER) | `run_nodata()` (L750-839) | ✅ |
| **tcpfail-bug** | **BUG-06 재현: TCP실패 -> HB스킵** | `run_tcpfail(1)` (L944-1032) | ✅ |
| **tcpfail-fix** | **BUG-06 수정: TCP실패 -> HB수신** | `run_tcpfail(0)` (L944-1032) | ✅ |

**Test Constants Alignment:**

| Constant | pb_7100_ur.c | ha_test.c | Status |
|----------|-------------|-----------|:------:|
| HB_TIMEOUT | 5 | 5 | ✅ |
| HB_STABLE_COUNT | 3 | 3 | ✅ |
| HB_DATA_TIMEOUT | 5 | 5 | ✅ |
| HB_SEND_INTVL | 1 | 1 | ✅ |
| HB_PACKET struct | hb_mark[2] + recv_cnt(int) | 동일 | ✅ |

---

## 3. Code Quality Analysis

### 3.1 Bug Fix Quality

| BUG | Fix Quality | Notes |
|-----|:-----------:|-------|
| BUG-01 | Excellent | 분리 호출 + 에러 로그 + 실패 시 NOTOK 리턴 |
| BUG-02 | Excellent | 3단계 분기(정규/호환/비정상) + 에러 로그 + break |
| BUG-03 | Excellent | 컴파일 타임 기본값 + 환경변수 override + 범위 검증 |
| BUG-04 | Excellent | Connect2(2초) + 상세 주석으로 이유 설명 |
| BUG-05 | Excellent | sleep 제거 + Device_Close_HA 별도 함수 + 주석 |
| BUG-06 | Excellent | continue 제거 + TCP_RETRY_INTVL + 상세 주석 |
| BUG-07 | Good | 값 변경 완료, 주석에 "HB_TIMEOUT=3초" 잔재 1건 (아래 참조) |

### 3.2 Minor Issues Found

| Type | File | Location | Description | Severity |
|------|------|----------|-------------|----------|
| 주석 불일치 | pb_7100_ur.c | L202 | `HB_TIMEOUT=3초` 주석이 있으나 실제 값은 5초 | Info |
| 주석 불일치 | pb_7100_ur.c | L872 | `HB_TIMEOUT(3초)` 주석, 실제 5초 | Info |

이 주석들은 Connect2 도입 시점의 설명으로, HB_TIMEOUT 변경(3->5) 이전에 작성된 것으로 보인다. 기능에는 영향 없으나 정리 권장.

### 3.3 Documentation Quality

모든 주요 수정에 상세한 주석이 포함되어 있어 유지보수성이 우수하다:
- Device_Open: Connect2 사용 이유 설명 (L198-202)
- Device_Close: sleep 제거 이유 설명 (L223-225)
- 메인루프 continue 제거: 버그 원인 및 수정 이유 상세 설명 (L366-370)
- TCP_RETRY_INTVL: 도입 이유 및 타이밍 분석 (L352-358)
- HA_Init: 포트 분리 로직 상세 설명 (L959-967)

---

## 4. Architecture Compliance

### 4.1 Convention Compliance

| Category | Convention | Status | Notes |
|----------|-----------|:------:|-------|
| Naming | C89 함수명 (PascalCase/snake_case 혼용) | ✅ | FEP 기존 관례 준수 |
| Constants | UPPER_SNAKE_CASE | ✅ | HB_TIMEOUT, TCP_RETRY_INTVL 등 |
| Error handling | Log() 호출 + NOTOK 리턴 | ✅ | 기존 패턴 일관 |
| HA 로그 prefix | "HA " prefix 통일 | ✅ | "HA HB sent", "HA FAILOVER" 등 |
| setsockopt 순서 | SO_REUSEADDR -> SO_RCVBUF | ✅ | 설계 표준 준수 |

### 4.2 Build Compatibility

| Item | Status | Notes |
|------|:------:|-------|
| C89 호환 | ✅ | 블록 내부 변수 선언 (C99) 일부 있으나 FEP 빌드에서 허용 |
| -DB7102/-DB7103 분기 | ✅ | HB_PORT_DEFAULT + 환경변수 offset 모두 정상 |
| Connect2 의존 | ✅ | sub/tcpip_connect.c에 구현 완료 |
| 헤더 의존 | ✅ | pa_struct.h, fep_fepp.h 기존 그대로 |

---

## 5. Match Rate Summary

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

| Category | Score | Status |
|----------|:-----:|:------:|
| Design Match | 100% | ✅ |
| Architecture Compliance | 100% | ✅ |
| Convention Compliance | 100% | ✅ |
| **Overall** | **100%** | ✅ |

---

## 6. Differences Found

### Missing Features (Design O, Implementation X)

None.

### Added Features (Design X, Implementation O)

None. (구현이 설계를 정확히 따름)

### Changed Features (Design != Implementation)

| Item | Design | Implementation | Impact |
|------|--------|----------------|--------|
| 주석 HB_TIMEOUT 값 | (설계에서 변경 명시) | L202, L872에 "3초" 잔재 | None (Info) |

---

## 7. Plan vs Design Consistency

Plan 문서는 초기 5건 버그를 식별했고, Design 문서는 이를 7건으로 확장했다.

| Plan Bug ID | Design Bug ID | Mapping |
|-------------|---------------|---------|
| BUG-01 (SO_RCVBUF) | BUG-01 | 동일 |
| BUG-02 (recvfrom) | BUG-02 | 동일 |
| BUG-03 (HB 포트) | BUG-03 | 동일 |
| BUG-04 (sleep(3)) | BUG-05 | Plan의 Device_Open = Design의 Device_Close |
| BUG-05 (진단 로그) | FR-11 | Plan에서 BUG로 분류, Design에서 FR로 재분류 |
| - | BUG-04 (Connect 블로킹) | Design에서 신규 식별 |
| - | BUG-06 (continue 스킵) | Design에서 신규 식별 (핵심 버그) |
| - | BUG-07 (HB_TIMEOUT) | Design에서 신규 식별 |

Plan과 Design 간 버그 번호 체계가 다르나, 모든 항목이 구현에 반영되어 실질적 gap은 없다.

---

## 8. Recommended Actions

### 8.1 Immediate (Minor)

| Priority | Item | File | Impact |
|----------|------|------|--------|
| Info | L202 주석 "HB_TIMEOUT=3초" -> "HB_TIMEOUT=5초" 수정 | pb_7100_ur.c:202 | None |
| Info | L872 주석 "HB_TIMEOUT(3초)" -> "HB_TIMEOUT(5초)" 수정 | pb_7100_ur.c:872 | None |

### 8.2 Server Test (Plan 4.1 Definition of Done)

| Item | Status | Verification Method |
|------|:------:|---------------------|
| mk.sh pb 빌드 성공 | Pending | `mk.sh pb ur` on server |
| Primary "HA HB sent" 로그 | Pending | `tail -f st03/LOG/PB/pb_7102_ur.log` |
| Secondary "HA HB recv ok" 로그 | Pending | `tail -f st03/LOG/PB/pb_7102_ur.log` |
| 24시간 오판 절체 0건 | Pending | 운영 모니터링 |
| SO_RCVBUF 확인 | Pending | `ss -ulnm` 또는 `cat /proc/net/udp` |
| 포트 50000/50001 분리 | Pending | `netstat -ulnp` |

---

## 9. Conclusion

설계 문서와 구현 코드의 일치율은 **100%**이다.

7개 버그 수정(BUG-01~07), 11개 기능 요구사항(FR-01~11), PA 모듈 동일 수정(FR-07), 테스트 프로그램 8개 모드 모두 설계대로 정확하게 구현되었다. 메인루프 흐름, 타이밍 상수, 에러 처리 패턴이 완전히 일치하며, 코드 주석도 수정 이유를 상세히 설명하고 있어 유지보수성이 우수하다.

주석 내 HB_TIMEOUT 값 잔재(3초->5초 미반영) 2건은 기능에 영향 없는 Info 수준 이슈이다.

남은 작업은 서버 빌드/배포 후 실환경 검증이다.

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2026-03-11 | Initial gap analysis - 100% match rate | Claude Code (gap-detector) |
