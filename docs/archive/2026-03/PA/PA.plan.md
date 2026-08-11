# PA-HA Planning Document

> **Summary**: PA 시세수신(pa_7100_ur.c) HA 이중화 구현 — PB-HA 동일 구조 적용
>
> **Project**: FEP (Front-End Processor)
> **Version**: -
> **Author**: Claude Code
> **Date**: 2026-03-11
> **Status**: Draft

---

## 1. Overview

### 1.1 Purpose

pa_7100_ur.c(KRX UDP 멀티캐스트 시세 수신 → SHM/FIFO 전달)에 Active/Standby HA 이중화를 구현한다. PB-HA(pb_7100_ur.c)와 동일한 heartbeat 기반 아키텍처를 적용하되, pb-ha-hotfix에서 발견된 7개 버그의 교훈을 처음부터 반영한다.

### 1.2 Background

- PA 시세 수신 프로세스(pa_7102_ur, pa_7103_ur, pa_7201_ur, pa_7202_ur, pa_7203_ur)는 현재 HA 미지원
- REAL1/REAL2 양쪽에서 동시 가동 시, 동일 시세를 SHM에 이중 기록 → DD 프로세스가 중복 전달 가능
- PB-HA 완료(2026-03-07) + pb-ha-hotfix(2026-03-11) 경험으로 검증된 HA 패턴 확보

### 1.3 PB-HA vs PA-HA 차이점

| 항목 | PB (pb_7100_ur.c) | PA (pa_7100_ur.c) |
|------|-------------------|-------------------|
| UDP 수신 | KRX 채권 시세 | KRX 채권+파생 시세 |
| TCP 전달 | SockTcpfd로 TCP 전달 | **없음** — SHM + FIFO만 |
| 빌드 변형 | B7102, B7103 (2종) | A7102, A7103, A7201, A7202, A7203 (5종) |
| HA 적용 시 | TCP 전달을 Active만 수행 | **SHM 기록 + FIFO 기록을 Active만 수행** |
| Device_Open/Close | TCP 연결 관리 | **해당 없음** — TCP 없음 |
| BUG-06 (continue) | TCP 실패 시 Recv_Data 스킵 | **해당 없음** — TCP 없음 |

### 1.4 Related Documents

- PB-HA 완료 보고서: `docs/archive/2026-03/pb-ha-hotfix/pb-ha-hotfix.report.md`
- PB-HA 이중화 보고서: `docs/04-report/features/PB-HA.report.md`
- PA 코드 감사 완료: `docs/archive/2026-02/PA/PA.report.md`
- 대상 소스: `st01/src/PA/pa_7100_ur.c`
- 참조 소스: `st01/src/PB/pb_7100_ur.c` (HA 구현 완료)

---

## 2. Scope

### 2.1 In Scope

- [ ] FR-01: HA 상수 및 HB_PACKET 구조체 정의
- [ ] FR-02: HA_Init() — UDP heartbeat 소켓 초기화 (환경변수 + 프로세스별 포트 분리)
- [ ] FR-03: HA_Heartbeat_Send() — Primary: HB 1초 간격 전송
- [ ] FR-04: HA_Check_Failover() — Secondary: HB 수신 + 절체/복귀 판단
- [ ] FR-05: 메인루프 HA 통합 — ha_active에 따른 SHM/FIFO 기록 제어
- [ ] FR-06: HB 포트 프로세스별 분리 (A7102=base+0, A7103=+1, A7201=+2, A7202=+3, A7203=+4)
- [ ] FR-07: ha_test.c PA 모드 추가 (또는 별도 pa_ha_test.c)
- [ ] FR-08: HA 진단 로그 추가 (HA Init/Send/Recv 전 구간)

### 2.2 Out of Scope

- TCP 전달 관련 HA (PA는 TCP 없음 — PB-HA의 Device_Open/Close/BUG-06 해당 없음)
- HA 아키텍처 변경 (PB-HA와 동일한 Active/Standby 구조 유지)
- PA 다른 소스 파일(pa_1100_ts, pa_8100_ts 등)에 대한 HA 적용

---

## 3. Requirements

### 3.1 Functional Requirements

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-01 | HB_PACKET 구조체 + HA 상수 (HB_TIMEOUT=5, HB_STABLE_COUNT=3, HB_SEND_INTVL=1) | High | - |
| FR-02 | HA_Init: UDP HB 소켓 생성, SO_REUSEADDR+SO_RCVBUF, bind, 환경변수(_HA_PEER_IP, _HA_HB_PORT) | High | - |
| FR-03 | HA_Heartbeat_Send: 1초 간격 HB 전송, recv_cnt 포함 | High | - |
| FR-04 | HA_Check_Failover: HB 수신, FAILOVER(timeout), FAILBACK(stable), DATA_FAILOVER(5연속 무수신) | High | - |
| FR-05 | 메인루프: ha_active==OFF → SHM 기록 + FIFO 기록 skip (UDP 수신은 유지) | **Critical** | - |
| FR-06 | HB 포트 분리: 5종 프로세스별 base+offset (0~4) | High | - |
| FR-07 | 테스트 프로그램 PA 모드 추가 또는 별도 작성 | Medium | - |
| FR-08 | HA 진단 로그: Init OK/FAIL, HB sent/recv, FAILOVER/FAILBACK | High | - |

### 3.2 Non-Functional Requirements

| Category | Criteria | Measurement Method |
|----------|----------|-------------------|
| 호환성 | PB-HA와 동일 HB_PACKET 프로토콜 | 구조체 동일 확인 |
| 안정성 | 24시간 연속 운용 시 HB 누락 0건 | 양쪽 서버 로그 확인 |
| 빌드 | mk.sh pa ur 정상 빌드 (5종 바이너리) | Linux 빌드 확인 |
| 성능 | HB 처리가 시세 수신 지연에 영향 없음 | select 타임아웃 내 HB 처리 |

---

## 4. Architecture

### 4.1 PA-HA 동작 구조

```
┌─────────────────────────────────────────────────────┐
│  KRX UDP Multicast                                   │
│  채권(A301K,G701K,B601K) + 파생(A006F,A306F,G706F)   │
└────────────────┬────────────────────────────────────┘
                 │
        ┌────────┴────────┐
        ▼                 ▼
    REAL1 (podm11)    REAL2 (podm12)
    Primary            Secondary
    UDP RX: O          UDP RX: O
    SHM WR: O          SHM WR: X (standby)
    FIFO:   O          FIFO:   X (standby)
        │               │
        │◄──── UDP HB ───┤  1sec interval
        │   HB_PACKET    │  "HB" + recv_cnt
        │   (Port 50010) │
        │                │
        └──── SHM ────► DD 프로세스 → 자동전략
```

### 4.2 PB-HA와의 핵심 차이

**PB-HA**: ha_active가 TCP **전달** 을 제어 (UDP 수신 + SHM 기록은 항상)
```c
// PB: Active만 TCP 전달
if (ha_active == OFF || SockTcpfd < 0) continue;  // TCP send skip
Select_Send(SockTcpfd, rbuf, ...);
```

**PA-HA**: ha_active가 SHM **기록** + FIFO **전달** 을 제어 (UDP 수신은 항상)
```c
// PA: Active만 SHM 기록 + FIFO 전달
rt = Recv_Data(r_buf);      // UDP 수신은 항상
if (ha_active == OFF) {
    continue;               // SHM 기록 + FIFO 전달 skip
}
Set_Sise(r_buf);            // SHM 기록
F_W(TS_W1_1, &W_Fmt, 1);   // FIFO 전달
```

### 4.3 HB 포트 분리 설계

PA는 5종 프로세스가 있으므로 PB(2종)보다 넓은 포트 범위 필요:

| 프로세스 | Define | HB Port (base=50010) |
|----------|--------|---------------------|
| pa_7102_ur | A7102 | 50010 (+0) |
| pa_7103_ur | A7103 | 50011 (+1) |
| pa_7201_ur | A7201 | 50012 (+2) |
| pa_7202_ur | A7202 | 50013 (+3) |
| pa_7203_ur | A7203 | 50014 (+4) |

PB 포트(50000~50001)과 충돌 방지를 위해 base=50010 사용.

환경변수: `_PA_HA_HB_PORT=50010` (미설정 시 컴파일 기본값)

### 4.4 Key Architectural Decisions

| Decision | Options | Selected | Rationale |
|----------|---------|----------|-----------|
| HA 제어 대상 | SHM기록 / FIFO전달 / 둘다 | **둘다** | Standby에서 SHM 기록하면 DD가 이중 전달 |
| HB 포트 base | 50000대 / 50010대 | **50010** | PB(50000-50001)와 충돌 방지 |
| Connect2 필요성 | 필요 / 불필요 | **불필요** | PA는 TCP 없음, 블로킹 이슈 없음 |
| 코드 재사용 | PB 코드 복사 / 공통 함수 추출 | **PB 코드 복사 후 PA 맞춤 수정** | TCP 관련 코드 제거, SHM/FIFO 제어 추가 |
| recv_cnt 소스 | O_R_CNT1 (UDP) | **O_R_CNT1** | PB와 동일하게 UDP 수신 카운터 사용 |

---

## 5. PB-HA Hotfix 교훈 반영 (처음부터 적용)

pb-ha-hotfix에서 발견된 7개 버그 중 PA에 해당하는 항목:

| PB BUG | PA 적용 | 이유 |
|--------|---------|------|
| BUG-01 (SO_RCVBUF) | **이미 수정됨** | FR-07으로 적용 완료 |
| BUG-02 (recvfrom 에러) | **적용** | HB recvfrom에 에러체크 + rt>=2 |
| BUG-03 (포트 분리) | **적용** | 5종 프로세스 포트 분리 |
| BUG-04 (Connect 블로킹) | **해당 없음** | PA는 TCP 없음 |
| BUG-05 (sleep(3)) | **해당 없음** | PA는 TCP 없음 |
| BUG-06 (continue 스킵) | **해당 없음** | PA는 TCP 없음, 다른 continue 구조 |
| BUG-07 (HB_TIMEOUT) | **적용** | 처음부터 5초로 설정 |

---

## 6. Success Criteria

### 6.1 Definition of Done

- [ ] pa_7100_ur.c HA 코드 구현 완료
- [ ] mk.sh pa ur 빌드 성공 (5종 바이너리)
- [ ] Primary 로그에 "HA HB sent" 출력
- [ ] Secondary 로그에 "HA HB recv ok" 출력
- [ ] Standby 시 SHM 기록 안 됨 확인 (DD 프로세스 이중 전달 없음)
- [ ] 절체/복귀 동작 확인 (Primary kill → Secondary FAILOVER → Primary 재기동 → FAILBACK)

### 6.2 Quality Criteria

- [ ] HB 포트 50010~50014 분리 확인
- [ ] 야간(시세 없는 시간) 안정 운영 확인
- [ ] 24시간 오판 절체 0건

---

## 7. Risks and Mitigation

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| SHM 이중 기록 시 DD 중복 전달 | High | Medium | ha_active==OFF 시 Set_Sise + F_W 모두 skip |
| FIFO 알림 이중 전달 | Medium | Medium | Write_Read_Fifo도 ha_active 체크 |
| 5종 프로세스 포트 충돌 | Medium | Low | 컴파일타임 offset + 환경변수 base |
| PB와 PA HB 패킷 혼재 | Low | Low | 포트 대역 분리 (PB:50000, PA:50010) |
| 시세 수신 지연 | Medium | Low | HB 처리는 select 타임아웃 내 완료 |

---

## 8. Implementation Order

1. [ ] HA 상수/구조체 정의 (HB_PACKET, HB_TIMEOUT=5, 포트 매크로)
2. [ ] HA_Init() 함수 구현 (PB에서 복사 + PA 맞춤 수정)
3. [ ] HA_Heartbeat_Send() 함수 구현
4. [ ] HA_Check_Failover() 함수 구현
5. [ ] 메인루프(PA_7100_UR) HA 통합 — Recv_Data 후 ha_active 체크
6. [ ] 진단 로그 추가
7. [ ] mk.sh pa ur 빌드 확인
8. [ ] 테스트 프로그램 작성/수정

---

## 9. Convention Prerequisites

### 9.1 Environment Variables (신규)

| Variable | Purpose | Scope | Default |
|----------|---------|-------|---------|
| `_PA_HA_PEER_IP` | PA HA 상대 서버 IP | Server | (필수) |
| `_PA_HA_HB_PORT` | PA HB UDP base 포트 | Server | 50010 |

### 9.2 HA 로그 Prefix

PB-HA와 동일하게 "HA " prefix 사용:
- `HA init OK: role=N active=N hb_fd=N hb_port=NNNNN`
- `HA HB sent (rt=N, cnt=N, to=IP:PORT)`
- `HA HB recv ok (rt=N, cnt=N, from=IP:PORT)`
- `HA FAILOVER: no heartbeat for N seconds`
- `HA FAILBACK: N consecutive HB received`

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-03-11 | Initial draft — PA-HA 이중화 계획. PB-HA + hotfix 교훈 반영. | Claude Code |
