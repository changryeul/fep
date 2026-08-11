# pa-pb-dedup Planning Document

> **Summary**: PA/PB 모듈 간 중복 코드를 sub/ 공용 라이브러리로 추출하여 유지보수성 향상
>
> **Project**: FEP (Front-End Processor) for KRX
> **Author**: Claude Code
> **Date**: 2026-02-21
> **Status**: Draft

---

## 1. Overview

### 1.1 Purpose

PA(Trading)와 PB(Bond KRX Direct) 모듈의 3개 파일 쌍(*_1100_ts, *_1200_tr, *_7800_tr)에서 55~70% 수준의 코드 중복을 해소한다. 공통 로직을 `sub/` 라이브러리로 추출하여 단일 소스에서 관리하고, 모듈 고유 로직(암호화, TR코드, 데이터소스)은 파라미터 또는 콜백으로 분리한다.

### 1.2 Background

- PA와 PB는 동일한 이벤트 루프 구조(Init→Device_Open→Event_Loop→Device_Close)를 공유
- PB는 PA 대비 INISAFE-Net 암호화 레이어 3함수(Free_All, Handshake, Encrypt/Decrypt) 추가
- PA는 주문 데이터를 DSHM_R(공유메모리)에서, PB는 F_R(FIFO)에서 읽음
- 현재 버그 수정 시 PA/PB 양쪽에 동일 수정을 반복 적용해야 하는 유지보수 부담
- 이전 3개 struct-refactor PDCA(tr:92%, shm:95%, fifo:97%)로 코드 정규화 완료 — 추출 가능 상태

### 1.3 Related Documents

- Archive: `docs/archive/2026-02/tr-struct-refactor/` (KRX 메시지 구조체화)
- Archive: `docs/archive/2026-02/fifo-struct-refactor/` (FIFO 버퍼 구조체화)
- Archive: `docs/archive/2026-02/shm-struct-refactor/` (SHM 접근 구조체화)
- Reference: `docs/FEP_Architecture_Analysis.md` (PA/PB 모듈 아키텍처)

---

## 2. Scope

### 2.1 In Scope

- [ ] 3개 파일 쌍의 공통 유틸리티 함수 추출 (Get_Msec, Log_Out, Err_Msg)
- [ ] 이벤트 루프 프레임워크 함수 추출 (Line_Change, Time_Out_Rtn 기본형)
- [ ] Device I/O 프레임워크 추출 (Device_Open/Close/Read/Write 공통 골격)
- [ ] 데이터 처리 공통 패턴 추출 (Write_Data/Write_Response_Data 공통 부분)
- [ ] PA/PB 고유 로직 분리를 위한 콜백/파라미터 인터페이스 정의

### 2.2 Out of Scope

- PA/PB 비즈니스 로직 통합 (Analyze_Data, Make_Send_Msg의 TR코드별 분기는 각 모듈에 유지)
- INISAFE-Net 암호화 코드 변경 (PB 고유, 기존 유지)
- pa_1200_mp.c (PB 대응 파일 없음, 단독 유지)
- pb_1800_ts.c (PA 대응 파일 없음, 단독 유지)
- pa_5000_qr.c, pa_1100_ts.c 내부의 Chk_Risk_All (PA 고유 기능)
- 새로운 프로세스 바이너리 추가 또는 삭제

---

## 3. Requirements

### 3.1 Functional Requirements

| ID | Requirement | Priority | Status | Target Files |
|----|-------------|----------|--------|-------------|
| FR-01 | Get_Msec, Log_Out, Err_Msg 3개 유틸리티 함수를 sub/ 공용 함수로 추출 | High | Pending | 6 files (3 PA + 3 PB) |
| FR-02 | Line_Change 공통 함수 추출 (소켓 상태 변경 + 로깅) | High | Pending | 6 files |
| FR-03 | Device_Read 공통 골격 추출 (select/recv + 데이터 분석 콜백) | High | Pending | 6 files |
| FR-04 | Device_Open 공통 골격 추출 (connect + 후처리 훅) | Medium | Pending | 6 files |
| FR-05 | Device_Close 공통 골격 추출 (cleanup + 소켓 닫기) | Medium | Pending | 6 files |
| FR-06 | Device_Write 공통 골격 추출 (길이 계산 + send) | Medium | Pending | 6 files |
| FR-07 | Write_Data / Write_Response_Data 파일 쓰기 공통 패턴 추출 | Low | Pending | 4 files (*_7800_tr, *_1200_tr) |
| FR-08 | sub/ 헤더 파일에 공용 함수 프로토타입 및 콜백 타입 정의 | High | Pending | inc/fep_common.h (신규) |

### 3.2 Non-Functional Requirements

| Category | Criteria | Measurement Method |
|----------|----------|-------------------|
| 호환성 | 75개 바이너리 동작 100% 호환 | mk.sh all 빌드 + 기존 프로세스 동작 동일 |
| 성능 | 함수 호출 오버헤드 무시 가능 (nanosecond 수준) | 프로세스 로직 동일 검증 |
| 이식성 | HP-UX, SunOS, AIX, Linux 컴파일 보장 | ANSI C89 준수 검증 |
| 코드 감소 | PA+PB 합산 순 코드 15% 이상 감소 | wc -l 비교 |

---

## 4. Success Criteria

### 4.1 Definition of Done

- [ ] FR-01~FR-08 구현 완료
- [ ] `sub/` 라이브러리 빌드 성공 (`mk.sh sub`)
- [ ] 전체 모듈 빌드 성공 (`mk.sh all`)
- [ ] 추출된 함수가 PA/PB 양쪽에서 동일하게 호출됨
- [ ] Gap Analysis >= 90%

### 4.2 Quality Criteria

- [ ] 컴파일러 경고 0 (Linux `-Wall`)
- [ ] 6개 소스 파일의 총 줄 수 15% 이상 감소 (현재 합계: ~8,843줄)
- [ ] sub/ 내 추출 함수의 중복 코드 0건
- [ ] 기존 PA/PB 함수 시그니처 유지 (wrapper 허용)

---

## 5. Risks and Mitigation

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| 함수 추출 시 미묘한 동작 차이 누락 | High | Medium | 추출 전 diff 기반 상세 비교, Design 단계에서 차이점 완전 문서화 |
| 글로벌 변수 참조 차이 (PA: SHM_DATA, PB: enc 관련) | High | High | 글로벌 변수 목록 사전 조사, 파라미터로 전달하거나 모듈별 유지 |
| 대규모 변경으로 인한 regression | High | Medium | 한 파일쌍씩 순차 적용, 쌍 단위 빌드 검증 |
| C89 제약: 함수 포인터 콜백 패턴의 복잡도 증가 | Medium | Medium | 단순 파라미터 플래그 우선, 콜백은 최소 필요 시에만 |
| PA/PB 간 #define 네이밍 충돌 (A1101 vs B1101) | Low | Low | 공용 함수는 프로세스 ID를 파라미터로 받도록 설계 |

---

## 6. Architecture Considerations

### 6.1 Project Level

| Level | Selected |
|-------|:--------:|
| **Dynamic** (기존 FEP 프로젝트 레벨) | O |

### 6.2 Key Architectural Decisions

| Decision | Options | Selected | Rationale |
|----------|---------|----------|-----------|
| 공용 코드 위치 | (A) sub/ 기존 라이브러리에 추가 / (B) 별도 sub/common/ 디렉토리 | (A) sub/ | 기존 빌드 체인(Make_Lib_P_c.mk) 그대로 활용 |
| PA/PB 분기 방식 | (A) 함수 파라미터 플래그 / (B) 함수 포인터 콜백 / (C) 혼합 | (C) 혼합 | 단순 분기는 플래그, Device_Open 같은 복잡 분기는 콜백 |
| 헤더 구조 | (A) fep_common.h 신규 / (B) fep_sub.h에 추가 | (A) fep_common.h 신규 | fep_sub.h 이미 과대(283줄), 분리 유지 |
| 기존 함수 호환 | (A) 기존 함수명 제거 / (B) 매크로 wrapper 유지 | (B) wrapper 유지 | 점진적 마이그레이션, 호출부 변경 최소화 |
| 추출 전략 | (A) 한번에 전체 / (B) 파일쌍별 순차 / (C) 함수그룹별 순차 | (C) 함수그룹별 순차 | 가장 안전. 유틸리티→프레임워크→비즈니스 순 |

### 6.3 Duplication Analysis Summary

```
File Pair Comparison:
┌───────────────────────────────────────────────────────────────┐
│ Pair              │ PA Lines │ PB Lines │ Shared % │ PB-unique        │
│───────────────────│──────────│──────────│──────────│──────────────────│
│ *_1100_ts.c       │ 1,532    │ 1,925    │ ~67%     │ Free_All,        │
│ (Order TX)        │          │          │          │ Handshake,       │
│                   │          │          │          │ EncryptAndMake.. │
│───────────────────│──────────│──────────│──────────│──────────────────│
│ *_1200_tr.c       │ 1,114    │ 1,540    │ ~70%     │ DecryptBody,     │
│ (Response RX)     │          │          │          │ Free_All,        │
│                   │          │          │          │ Handshake        │
│───────────────────│──────────│──────────│──────────│──────────────────│
│ *_7800_tr.c       │  987     │ 1,745    │ ~55%     │ Free_All,        │
│ (Market Data)     │          │          │          │ Handshake,       │
│                   │          │          │          │ Check999         │
└───────────────────────────────────────────────────────────────┘

Total: 3,633 (PA) + 5,210 (PB) = 8,843 lines
Estimated extractable: ~2,500 lines → ~1,200 lines shared in sub/
Expected net reduction: ~1,300 lines (15%)
```

### 6.4 Function Extraction Groups

```
Group 1 — Trivially Identical Utilities (FR-01):
┌─────────────────────────────────────────────────┐
│ Get_Msec()       — millisecond timestamp calc   │
│ Log_Out()        — structured log output        │
│ Err_Msg()        — error message handler        │
└─────────────────────────────────────────────────┘
  Risk: LOW, Complexity: LOW, Impact: 6 files

Group 2 — Simple Event Handlers (FR-02):
┌─────────────────────────────────────────────────┐
│ Line_Change()    — socket line state change     │
└─────────────────────────────────────────────────┘
  Risk: LOW, Complexity: LOW, Impact: 6 files

Group 3 — Device I/O Framework (FR-03~06):
┌─────────────────────────────────────────────────┐
│ Device_Read()    — select + recv + dispatch     │
│ Device_Open()    — connect + post-connect hook  │
│ Device_Close()   — cleanup + socket close       │
│ Device_Write()   — length calc + send           │
└─────────────────────────────────────────────────┘
  Risk: MEDIUM, Complexity: MEDIUM, Impact: 6 files
  Note: PB adds encryption hooks at each stage

Group 4 — Data Processing (FR-07):
┌─────────────────────────────────────────────────┐
│ Write_Data()     — FIFO write with header build │
│ Write_Response_Data() — response file write     │
└─────────────────────────────────────────────────┘
  Risk: MEDIUM-HIGH, Complexity: HIGH, Impact: 4 files
  Note: 비즈니스 로직과 밀접, 추출 범위 제한적
```

---

## 7. Convention Prerequisites

### 7.1 Existing Project Conventions

- [x] `CLAUDE.md` has coding conventions section
- [x] Pure C89/ANSI C, tab width 4, `*_FMT`/`*_S` struct suffix
- [x] `sub/` 함수: 대문자 시작 CamelCase (Get_Msec, Log_Out)
- [x] 프로세스별 `-D` define으로 컴파일 분기

### 7.2 Conventions for This Feature

| Category | Rule |
|----------|------|
| 새 파일명 | `sub/fep_common.c` (공용 함수), `inc/fep_common.h` (헤더) |
| 새 함수명 | `FEP_` prefix (기존 sub/ 함수와 구분) — e.g., `FEP_Get_Msec()` |
| 콜백 타입 | `typedef int (*FEP_Hook_Fn)(void *ctx)` — 범용 훅 인터페이스 |
| wrapper 매크로 | `#define Get_Msec FEP_Get_Msec` — 기존 호출부 호환 |
| 글로벌 변수 | 공용 함수는 글로벌 참조 금지, 모든 상태는 파라미터 전달 |

---

## 8. Implementation Phases

| Phase | 내용 | FR | 예상 변경 | Risk |
|-------|------|-----|----------|------|
| Phase 1 | fep_common.h 헤더 + 유틸리티 함수 추출 (Get_Msec, Log_Out, Err_Msg) | FR-01, FR-08 | ~150줄 신규, 6파일 수정 | Low |
| Phase 2 | Line_Change 추출 | FR-02 | ~50줄 신규, 6파일 수정 | Low |
| Phase 3 | Device_Read 공통 골격 추출 | FR-03 | ~100줄 신규, 6파일 수정 | Medium |
| Phase 4 | Device_Open/Close/Write 골격 추출 | FR-04~06 | ~200줄 신규, 6파일 수정 | Medium |
| Phase 5 | Write_Data 공통 패턴 추출 | FR-07 | ~80줄 신규, 4파일 수정 | Medium-High |
| Phase 6 | 빌드 검증 (`mk.sh sub` + `mk.sh all`) | - | 코드 변경 없음 | - |

---

## 9. Data Sources

| File | Lines | Role | Shared % |
|------|-------|------|----------|
| `src/PA/pa_1100_ts.c` | 1,532 | PA 주문 전송 | 67% with PB |
| `src/PB/pb_1100_ts.c` | 1,925 | PB 주문 전송 (암호화) | 67% with PA |
| `src/PA/pa_1200_tr.c` | 1,114 | PA 체결 수신 | 70% with PB |
| `src/PB/pb_1200_tr.c` | 1,540 | PB 체결 수신 (암호화) | 70% with PA |
| `src/PA/pa_7800_tr.c` | 987 | PA 시세 수신 | 55% with PB |
| `src/PB/pb_7800_tr.c` | 1,745 | PB 시세 수신 (암호화) | 55% with PA |
| `sub/` (existing) | 44 files | 공용 라이브러리 | Base |
| `inc/fep_sub.h` | 283 | sub/ 함수 프로토타입 | Reference |

---

## 10. Next Steps

1. [ ] Write design document (`pa-pb-dedup.design.md`)
2. [ ] 각 함수 쌍의 line-by-line diff 분석 (Design 단계)
3. [ ] Phase 1(유틸리티 함수)부터 순차 구현

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-21 | Initial draft | Claude Code |
