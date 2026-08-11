# pa-pb-dedup-phase2 계획 문서

> **요약**: PA/PB 모듈 중복 제거 2차 — Log_Out, Time_Out_Rtn, Device_Open LOGON 부분, PB Handshake/Free_All 추출
>
> **프로젝트**: FEP (Front-End Processor) for KRX
> **작성자**: Claude Code
> **날짜**: 2026-02-21
> **상태**: 초안
> **전제조건**: pa-pb-dedup (Phase 1) archived — 90%, 6개 함수 추출

---

## 1. 개요

### 1.1 목적

pa-pb-dedup Phase 1에서 de-scoped된 함수(Log_Out, Device_Open)와 추가 발견된 중복 함수(Time_Out_Rtn, Handshake, Free_All)를 `sub/fep_common.c`로 추출한다. Phase 1의 extern 글로벌 참조 패턴과 Device_Close wrapper 패턴을 그대로 활용한다.

### 1.2 배경

Phase 1 결과:
- 6개 함수 추출 완료 (Get_Msec, Line_Change, Device_Read, Device_Write, Device_Close_Base, Err_Msg)
- 8,843줄 → 7,710줄 (-13%) 감소
- De-scoped: Log_Out (Fmt 버퍼 타입 차이), Device_Open (PA/PB 구조적 차이)

추가 분석에서 발견된 추출 가능 함수:
- Time_Out_Rtn: 6개 파일에 near-identical
- Handshake/Free_All: PB 3개 파일에 identical (INISAFE-Net 암호화)

### 1.3 관련 문서

- Archive: `docs/archive/2026-02/pa-pb-dedup/` (Phase 1)
- Reference: `inc/fep_common.h`, `sub/fep_common.c` (Phase 1 output)

---

## 2. Scope

### 2.1 In Scope

- [ ] Log_Out 공통 골격 추출 (Fmt 버퍼를 파라미터로 분리)
- [ ] Time_Out_Rtn 공통 골격 추출 (타임아웃 처리 프레임워크)
- [ ] Device_Open TR_LOON 부분 추출 (Socket→Connect→[Handshake]→LOGON→Response 공통 골격)
- [ ] PB Handshake 함수 추출 (3개 PB 파일에서 sub/로)
- [ ] PB Free_All 함수 추출 (3개 PB 파일에서 sub/로)
- [ ] fep_common.h에 신규 함수 프로토타입 추가

### 2.2 Out of Scope

- Device_Open TR_LINK 부분 — 파일별 seq 처리/응답 코드/에러 분기가 모두 다름 (6가지 변형), 추출 시 가독성 저하
- Analyze_Data — 파일마다 TR코드 분기가 완전히 다름 (HARD)
- Make_Send_Msg — 파일마다 메시지 타입/구조가 다름, 암호화 핸들링 포함 (HARD)
- Write_Data — 비즈니스 로직 밀접, 암호화 처리 차이 (MEDIUM-HARD)
- Write_Response_Data — 2개 파일만 해당, 별도 PDCA가 더 적절

---

## 3. Requirements

### 3.1 Functional Requirements

| ID | Requirement | Priority | Target Files | Extractability |
|----|-------------|----------|-------------|---------------|
| FR-01 | Log_Out 공통 골격 추출 (Fmt 버퍼 → void* 파라미터) | High | 6 files | Medium |
| FR-02 | Time_Out_Rtn 공통 골격 추출 | High | 6 files | Medium |
| FR-03 | Device_Open_Logon 추출 (TR_LOON 부분만) | Medium | 6 files | Medium |
| FR-04 | PB Handshake 함수 추출 (INISAFE-Net) | Medium | 3 PB files | Easy |
| FR-05 | PB Free_All 함수 추출 | Medium | 3 PB files | Easy |
| FR-06 | fep_common.h 헤더 확장 | High | 1 file | Easy |

### 3.2 Non-Functional Requirements

| Category | Criteria |
|----------|----------|
| 호환성 | Phase 1과 동일: 모든 바이너리 동작 100% 호환 |
| 성능 | 함수 호출 오버헤드 무시 가능 |
| 이식성 | ANSI C89 준수 |
| 코드 감소 | 6파일 합산 순 코드 추가 10% 이상 감소 |

---

## 4. Function Analysis

### 4.1 Log_Out — 6개 파일 비교

| 파일 | Fmt 버퍼 | Fmt 타입 | Response Check | PB-특화 |
|------|----------|----------|----------------|---------|
| pa_1100_ts | S_Fmt | KRX_SESSION_FMT (82+41) | IS_RESP_OK | — |
| pa_1200_tr | KR_Fmt | KRX_R_SESSION_FMT (82+116) | IS_RESP_OK | — |
| pa_7800_tr | KR_Fmt | KRX_R_SESSION_FMT (82+116) | IS_RESP_OK | — |
| pb_1100_ts | S_Fmt | KRX_SESSION_FMT (82+41) | memcmp "0000" | PROC status + DTART_FD write |
| pb_1200_tr | KR_Fmt | KRX_R_SESSION_FMT (82+116) | IS_RESP_OK | — |
| pb_7800_tr | KR_Fmt | KRX_R_SESSION_FMT (82+116) | IS_RESP_OK | — |

**공통 로직** (~80%):
1. `Make_Send_Msg(TR_LOOU)` → `memset/memcpy DataBuff` → `Device_Write()` → `Device_Read()`
2. Response 검증 → 성공: `Device_Close()` + `TCP2_NET_STA(S_K) = END` / 실패: `Err_Msg()`

**차이점**:
- Fmt 버퍼 타입: S_Fmt (1100_ts) vs KR_Fmt (나머지) — memcpy 소스/크기가 다름
- pb_1100_ts: `PROC().start_status = JOB_END`, `PROC().process_status = 2`, `write(DTART_FD, "1", 1)` 추가
- pb_1100_ts: `memcmp(&DataBuff[KRX_HEAD_LEN], "0000", 4)` 사용 (IS_RESP_OK 대신)

**추출 전략**: Fmt 버퍼를 `void*` + `int size`로 파라미터화. pb_1100_ts의 추가 로직은 post-logout 콜백 또는 프로세스별 유지.

### 4.2 Time_Out_Rtn — 6개 파일 비교

**공통 구조** (~85%):
1. `Get_Msec(&NowMsec)` — 현재 시간 측정
2. 타임아웃 판정: `NowMsec - SendMsec > timeout` (HeartBeat, Reconnect 등)
3. HeartBeat 전송: `Make_Send_Msg(TR_POLL)` → `Device_Write()`
4. Reconnect 로직: `Device_Close()` → `Device_Open(TR_LOON)`

**차이점**:
- 타임아웃 값: 파일별로 다른 SHM 매크로 참조
- Reconnect 조건: 1100_ts는 ConnectRetryCnt 체크 포함
- 7800_tr 계열: RDS batch 모드 관련 추가 체크

### 4.3 Device_Open TR_LOON — 6개 파일 비교

**공통 구조** (~70%):
1. `Socket()` → `Connect(Sockfd, IpAddr, PORT_NO)` → [PB: `Handshake()`]
2. `Make_Send_Msg(TR_LOON)` → `memset/memcpy DataBuff` → `Device_Write()`
3. `Device_Read()` → MsgType "SCHLIR00000" 검증
4. Response 검증 → 성공: LogOnFlag=ON, Poll 설정 / 실패: Err_Msg(), close

**차이점**:
- PB: Handshake() 호출 (암호화 세션 수립)
- PB: connect 실패 시 sleep(5) (PA는 sleep 없음)
- 1100_ts: S_Fmt 사용 / 나머지: KR_Fmt 사용
- 7800_tr 계열: LOGON 후 즉시 OpenFlag=ON (TR_LINK 없음)
- pb_1100_ts: IS_RESP_OK 대신 memcmp "0000" 직접 비교

### 4.4 PB Handshake/Free_All — 3개 PB 파일 비교

**Handshake**: 3개 PB 파일에서 **byte-for-byte identical**. INISAFE-Net 암호화 세션 수립.
**Free_All**: 3개 PB 파일에서 **byte-for-byte identical**. 암호화 버퍼 메모리 해제.

---

## 5. Success Criteria

### 5.1 Definition of Done

- [ ] FR-01~FR-06 구현 완료
- [ ] `mk.sh sub` 빌드 성공
- [ ] `mk.sh all` 빌드 성공
- [ ] Gap Analysis >= 90%

### 5.2 Quality Criteria

- [ ] 컴파일러 경고 0 (Linux `-Wall`)
- [ ] 6개 소스 파일의 추가 줄 수 감소 (Phase 1 포함 누적 20%+ 감소 목표)
- [ ] sub/fep_common.c 내 중복 0건

---

## 6. Risks and Mitigation

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| Log_Out Fmt 버퍼 void* 캐스팅 타입 안전성 | Medium | Medium | memcpy size를 파라미터로 전달, 구조체 크기 상수 활용 |
| pb_1100_ts Log_Out 특화 로직 분리 복잡도 | Medium | Low | post-logout 콜백 패턴 또는 프로세스 파일에 유지 |
| Time_Out_Rtn 파일별 미묘한 차이 누락 | High | Medium | Design 단계에서 6파일 full diff 수행 |
| Device_Open TR_LOON + TR_LINK 경계 | High | Low | TR_LOON 부분만 추출, TR_LINK는 프로세스에 유지 |
| Handshake/Free_All 추출 시 INISAFE 링크 의존성 | Medium | Low | PB 빌드에서만 링크, sub/ 빌드에서는 object만 생성 |

---

## 7. Architecture Considerations

### 7.1 Key Architectural Decisions

| Decision | Options | Selected | Rationale |
|----------|---------|----------|-----------|
| Log_Out Fmt 처리 | (A) void* 파라미터 / (B) 프로세스별 유지 | (A) void* | 5/6 파일 동일 로직, 1파일만 추가 로직 |
| Device_Open 범위 | (A) TR_LOON만 / (B) TR_LOON + TR_LINK | (A) TR_LOON만 | TR_LINK는 6가지 변형, 추출 가치 대비 복잡도 과다 |
| Handshake/Free_All 위치 | (A) fep_common.c / (B) 별도 fep_encrypt.c | (B) 별도 고려 | 암호화 의존성 분리. Design에서 최종 결정 |
| Time_Out_Rtn 차이 처리 | (A) 플래그 파라미터 / (B) 콜백 | (A) 플래그 | 차이점이 조건 분기 수준이므로 플래그로 충분 |

### 7.2 Extraction Priority

```
Priority 1 — Easy, High Impact:
┌─────────────────────────────────────────────────┐
│ FR-04: PB Handshake    — 3 files, identical     │
│ FR-05: PB Free_All     — 3 files, identical     │
└─────────────────────────────────────────────────┘
  Risk: LOW, Complexity: LOW, Impact: 3 PB files

Priority 2 — Medium, High Impact:
┌─────────────────────────────────────────────────┐
│ FR-01: Log_Out         — 6 files, 80% common    │
│ FR-02: Time_Out_Rtn    — 6 files, 85% common    │
└─────────────────────────────────────────────────┘
  Risk: MEDIUM, Complexity: MEDIUM, Impact: 6 files

Priority 3 — Medium, Medium Impact:
┌─────────────────────────────────────────────────┐
│ FR-03: Device_Open_Logon — 6 files, 70% common  │
└─────────────────────────────────────────────────┘
  Risk: MEDIUM, Complexity: MEDIUM-HIGH, Impact: 6 files
```

---

## 8. Implementation Phases

| Phase | 내용 | FR | Risk |
|-------|------|-----|------|
| Phase 1 | PB Handshake + Free_All 추출 | FR-04, FR-05 | Low |
| Phase 2 | Log_Out 공통 골격 추출 | FR-01 | Medium |
| Phase 3 | Time_Out_Rtn 공통 골격 추출 | FR-02 | Medium |
| Phase 4 | Device_Open TR_LOON 추출 | FR-03 | Medium |
| Phase 5 | fep_common.h 확장 + 빌드 검증 | FR-06 | Low |

---

## 9. Data Sources

| File | Current Lines | Role |
|------|-------------|------|
| `src/PA/pa_1100_ts.c` | 1,279 | PA 주문 전송 |
| `src/PA/pa_1200_tr.c` | 895 | PA 체결 수신 |
| `src/PA/pa_7800_tr.c` | 767 | PA 시세 수신 |
| `src/PB/pb_1100_ts.c` | 1,667 | PB 주문 전송 (암호화) |
| `src/PB/pb_1200_tr.c` | 1,315 | PB 체결 수신 (암호화) |
| `src/PB/pb_7800_tr.c` | 1,519 | PB 시세 수신 (암호화) |
| `sub/fep_common.c` | 225 | 공용 함수 (Phase 1 output) |
| `inc/fep_common.h` | 43 | 공용 헤더 (Phase 1 output) |

---

## 10. Expected Metrics

### 10.1 Estimated Code Reduction

| Function | Files | Lines/file (avg) | Extracted | Net Savings |
|----------|-------|-----------------|-----------|-------------|
| PB Handshake | 3 | ~60 | ~60 shared | ~120 |
| PB Free_All | 3 | ~20 | ~20 shared | ~40 |
| Log_Out | 6 | ~35 | ~40 shared | ~170 |
| Time_Out_Rtn | 6 | ~40 | ~45 shared | ~195 |
| Device_Open LOGON | 6 | ~45 | ~50 shared | ~220 |
| **Total** | | | +~215 to sub/ | **~745 net** |

### 10.2 Cumulative Metrics (Phase 1 + Phase 2)

| Metric | Phase 1 | Phase 2 (est) | Cumulative |
|--------|---------|--------------|------------|
| Functions extracted | 6 | 5 | 11 |
| Net lines removed | 1,133 | ~745 | ~1,878 |
| Net reduction % | 13% | ~10% | ~21% |
| Maintenance copies | 6→1 per function | 6→1 / 3→1 | 11 functions unified |

---

## 11. Next Steps

1. [ ] Write design document (`pa-pb-dedup-phase2.design.md`)
2. [ ] 각 함수의 line-by-line diff 분석 (Design 단계)
3. [ ] Phase 1 (Handshake/Free_All) 부터 순차 구현

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-21 | Initial draft | Claude Code |
