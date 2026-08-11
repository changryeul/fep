# pa-pb-dedup-phase3 계획 문서

> **요약**: PA/PB 모듈 중복 제거 Phase 3 - Time_Out_Rtn 매개변수화된 추출, Fifo_Event_Rtn 통합, Write_Data TR 기본 추출
>
> **프로젝트**: FEP (Front-End Processor)
> **작성자**: Claude Code
> **날짜**: 2026-02-21
> **상태**: 초안

---

## 1. 개요

### 1.1 목적

Phase 1: 6개 함수, Phase 2: 4개 함수 시리즈 이후 PA/PB 공통 함수 추출을 계속합니다. 6개 핵심 PA/PB 프로세스 파일에 걸쳐 중복된 상태로 남아있는 다음 계층의 중복 함수를 대상으로 합니다.

### 1.2 배경

Phase 1 (fep_common.c: 6개 함수, -1,133줄) 및 Phase 2 (fep_common.c + fep_encrypt.c: 4개 함수, -605줄) 이후, PA/PB 모듈 쌍에서 대략 2,000+ 줄의 중복이 남아있습니다. 남은 중복 함수는 3개 카테고리로 분류됩니다:

1. **기계적 중복** - PA와 PB 사이에 복사된 동일한 코드 (Time_Out_Rtn, Fifo_Event_Rtn)
2. **거의 중복** - PA/PB는 암호화 처리 또는 사소한 파라미터만 다름 (TR 파일의 Write_Data)
3. **비즈니스 로직 인터리빙됨** - 함수가 프로세스 특화 논리에 너무 밀접하게 결합 (Analyze_Data, Make_Send_Msg) — 지연됨

Phase 3는 카테고리 1 및 2를 대상으로 합니다.

### 1.3 관련 문서

- Phase 1: `docs/archive/2026-02/pa-pb-dedup/pa-pb-dedup.report.md`
- Phase 2: `docs/archive/2026-02/pa-pb-dedup-phase2/pa-pb-dedup-phase2.report.md`
- 아키텍처: `docs/FEP_Architecture_Analysis.md`
- 공유 헤더: `st01/inc/fep_common.h`, `st01/inc/fep_encrypt.h`

---

## 2. 범위

### 2.1 범위 포함

- [x] Time_Out_Rtn 추출: 6개 파일의 3개 변형 → fep_common.c의 매개변수화된 기본 함수
- [x] Fifo_Event_Rtn 추출: 6개 파일의 동일 4줄 함수 → fep_common.c의 공유 함수
- [x] 1200_tr 및 7800_tr 쌍의 Write_Data 기본 추출 → fep_common.c의 공유 Write_Data_TR_Base
- [x] 헤더 업데이트 (fep_common.h) - 새 프로토타입 및 extern 선언
- [x] 6개 대상 프로세스 파일: pa_1100_ts.c, pa_1200_tr.c, pa_7800_tr.c, pb_1100_ts.c, pb_1200_tr.c, pb_7800_tr.c

### 2.2 범위 제외

- Socket_Event_Rtn: 프로세스별 비즈니스 논리에 너무 밀접하게 결합 (6가지 서로 다른 구현)
- Analyze_Data: TR 코드 라우팅이 파일당 완전히 다름
- Make_Send_Msg: 메시지 구조가 파일당 다르며 암호화 처리 포함
- Init_Parameters: 보일러플레이트이지만 프로세스 특화 변수 초기화와 인터리빙됨
- 비 PA/PB 파일의 Fifo_Event_Rtn (PA pa_2100_ts.c, pa_5020_mp.c, pa_8100_ts.c 등) — 향후 범위
- Device_Open TR_LINK 부분: 6가지 변형, 너무 다양

---

## 3. 요구사항

### 3.1 기능 요구사항

| ID | 요구사항 | 우선순위 | 상태 |
|----|-------------|----------|--------|
| FR-01 | Time_Out_Rtn을 fep_common.c의 매개변수화된 Time_Out_Rtn_Base(int mode)로 추출합니다. 3개 변형이 모드 상수로 매핑됩니다. 프로세스 파일은 기본 함수를 호출합니다. | 높음 | 보류 중 |
| FR-02 | Fifo_Event_Rtn을 fep_common.c의 공유 함수로 추출합니다. 6개 대상 파일 모두가 공유 버전을 호출합니다. | 중간 | 보류 중 |
| FR-03 | Write_Data 공통 골격을 1200_tr 쌍의 fep_common.c의 Write_Data_TR_Base()로 추출합니다. PA/PB Write_Data()는 얇은 래퍼가 됩니다. | 중간 | 보류 중 |
| FR-04 | 새 프로토타입, extern 선언 및 모드 상수로 fep_common.h를 업데이트합니다 | 높음 | 보류 중 |
| FR-05 | mk.sh와의 빌드 호환성 유지 (libfepP.a + 모든 모듈) | 높음 | 보류 중 |

### 3.2 비기능 요구사항

| 카테고리 | 기준 | 측정 방법 |
|----------|----------|-------------------|
| 바이너리 호환성 | 75개 모든 프로세스 바이너리가 링크되고 빌드됨 | 빌드 서버에서 `mk.sh all` |
| 0 동작 변경 | 리팩토링 전/후 동일한 런타임 동작 | 수동 코드 검토 + 로그 비교 |
| 코드 감소 | 순 줄 수 감소 >= 150줄 | 변경 전/후 `wc -l` 비교 |

---

## 4. 성공 기준

### 4.1 완료 정의

- [x] 모든 기능 요구사항 구현됨
- [x] 갭 분석을 통한 코드 검토 >= 90% 일치율
- [x] fep_common.c/h가 새 함수로 업데이트됨
- [x] 6개 대상 파일 모두 공유 함수 호출로 수정됨
- [x] 새로운 컴파일러 경고 없음

### 4.2 품질 기준

- [x] 빌드 성공: `mk.sh sub && mk.sh src`
- [x] 갭 분석 일치율 >= 90%
- [x] Phase 1/2 패턴 유지 (extern 전역 참조, FmtPtr 규칙)

---

## 5. 상세 분석

### 5.1 FR-01: Time_Out_Rtn 추출

**현재 상태: 6개 파일에 걸친 3개 변형 (각 PA/PB 쌍은 동일)**

| 변형 | 파일 | 줄 | PollCnt 처리 |
|---------|-------|-------|-----------------|
| A (TS) | pa_1100_ts:670-725, pb_1100_ts:729-784 | 56 각각 | case 2: disconnect+reconnect, case 3: heartbeat via Make_Send_Msg(TR_POLL)+FmtPtr |
| B (TR) | pa_1200_tr:564-601, pb_1200_tr:709-746 | 38 각각 | case 1: FOREVER_TIME 로그, case 2/3: disconnect+reconnect |
| C (7800) | pa_7800_tr:423-465, pb_7800_tr:582-624 | 43 각각 | case 1: FOREVER_TIME 로그, case 2: heartbeat via Reply struct (PA) or no-op (PB) |

**동일한 쌍:**
- pa_1100_ts.c === pb_1100_ts.c (56줄, 바이트 단위 동일 `#if 0` 블록 포함)
- pa_1200_tr.c === pb_1200_tr.c (38줄, 바이트 단위 동일)
- pa_7800_tr.c ≈ pb_7800_tr.c (43줄, case 2만 다름: PA는 heartbeat 전송, PB는 no-op)

**제안된 추출:**

```c
/* fep_common.h의 모드 상수 */
#define TIMEOUT_MODE_TS     1   /* 1100_ts: disconnect + heartbeat */
#define TIMEOUT_MODE_TR     2   /* 1200_tr: FOREVER_TIME + disconnect */
#define TIMEOUT_MODE_7800   3   /* 7800_tr: FOREVER_TIME + heartbeat(Reply) */
#define TIMEOUT_MODE_7800N  4   /* pb_7800_tr: FOREVER_TIME + no-op */

/* fep_common.c에서 */
void Time_Out_Rtn_Base(int mode);
```

각 프로세스 파일의 Time_Out_Rtn()은 1줄 호출이 됩니다: `Time_Out_Rtn_Base(TIMEOUT_MODE_TS)`.

**의존성:** extern 전역 (PollCnt, DataBuff, SendLen, FmtPtr, TimeOut, INT_SEQ 등)을 사용하며 이미 fep_common.h에 선언되어 있습니다. fep_common.c 및 fep_encrypt.c에서 호출되는 Make_Send_Msg() 및 Device_Write()를 호출합니다.

**절감:** 6개 파일에서 ~274줄 → fep_common.c의 ~60줄 = ~214 순 감소.

### 5.2 FR-02: Fifo_Event_Rtn 추출

**현재 상태: 모든 6개 파일에서 동일한 4줄 함수**

```c
void Fifo_Event_Rtn(void) {
    char tmp[2];
    read(START_FD, tmp, 1);
    return;
}
```

pa_1100_ts, pa_1200_tr, pa_7800_tr, pb_1100_ts, pb_1200_tr, pb_7800_tr에서 바이트 단위 동일합니다. 또한 20+ 다른 FEP 소스 파일에서 동일합니다 (pa_2100_ts, pa_8100_ts, pb_1800_ts, pb_7100_ts, pb_8100_ts, pw_4000_ts 등).

**제안된 추출:** fep_common.c로 이동합니다. 각 파일은 로컬 정의를 제거하고 공유 버전을 사용합니다.

**의존성:** extern `START_FD`를 사용합니다 (PROC 구조체 매크로를 통해 이미 선언됨).

**절감:** 6개 파일에서 ~24줄 → fep_common.c의 ~4줄 = ~20 순 감소. 작지만 6개 동일 코드 사본을 제거합니다. 향후 범위: 더 넓은 영향을 위해 20+ 파일로 확장.

### 5.3 FR-03: Write_Data TR 기본 추출

**현재 상태: TR (수신) 파일의 2개 PA/PB 쌍**

| 쌍 | PA 파일 | PB 파일 | PA 서명 | PB 서명 |
|------|---------|---------|-------------|-------------|
| 1200_tr | pa_1200_tr:644 | pb_1200_tr:798 | `Write_Data(int p_flag)` | `Write_Data(int p_flag)` |
| 7800_tr | pa_7800_tr:520 | pb_7800_tr:856 | `Write_Data(int p_flag)` | `Write_Data(int p_flag, int YN)` |

**주요 차이:** pb_7800_tr은 `int YN` 암호화 플래그 파라미터를 추가합니다. PB 버전은 파일 쓰기 전에 DecryptBody를 호출할 수 있습니다.

**설계 단계 중 조사 필요:**
- pa_1200_tr과 pb_1200_tr 간 Write_Data의 줄별 비교
- 추출 가능한 공통 골격 결정 (파일 헤더 쓰기, 타임스탬프, FIFO 쓰기)
- 암호화 처리를 FmtPtr 스타일 패턴이나 콜백으로 처리할 수 있는지 평가

**추정 절감:** 추출 가능하면 ~100-150줄 (설계 단계가 가능성을 확인합니다).

---

## 6. 위험 및 완화

| 위험 | 영향 | 가능성 | 완화 |
|------|--------|------------|------------|
| Time_Out_Rtn 모드 파라미터가 복잡도 추가 | 낮음 | 낮음 | 모드 상수는 컴파일 시간이며 단일 switch 문 |
| Fifo_Event_Rtn 추출이 콜백 등록 중단 | 중간 | 낮음 | Fifo_Event_Rtn의 함수 포인터 유지, 구현만 이동 |
| Write_Data 추출이 암호화 차이로 인해 불가능 | 중간 | 중간 | 설계 단계가 가능성 확인; 차단되면 Phase 4로 지연 |
| pb_7800_tr Time_Out_Rtn이 pa_7800_tr과 다름 | 낮음 | 없음 | 이미 식별됨: TIMEOUT_MODE_7800 vs TIMEOUT_MODE_7800N 사용 |
| Time_Out_Rtn heartbeat의 Make_Send_Msg/Device_Write 의존성 | 중간 | 낮음 | 이들은 프로세스 정의 함수; fep_common.h에서 extern 선언 사용 |

---

## 7. 아키텍처 고려사항

### 7.1 패턴 연속성

Phase 1 및 Phase 2로부터 확립된 패턴을 따릅니다:
- **Extern 전역 참조** — fep_common.c 함수는 extern 선언을 통해 프로세스 전역 접근
- **FmtPtr 패턴** — Time_Out_Rtn 변형 A는 heartbeat에 대해 `memcpy(DataBuff, FmtPtr, SendLen)` 사용
- **얇은 래퍼 패턴** — 프로세스 파일은 함수 이름 (Time_Out_Rtn, Fifo_Event_Rtn)을 기본 함수 호출 래퍼로 유지
- **헤더 전용 프로토타입 변경** — 새로운 .h 파일이 필요하지 않음; 기존 fep_common.h 확장

### 7.2 파일 영향 요약

| 파일 | 변경 |
|------|---------|
| `sub/fep_common.c` | +Time_Out_Rtn_Base(), +Fifo_Event_Rtn(), 선택적으로 +Write_Data_TR_Base() |
| `inc/fep_common.h` | +TIMEOUT_MODE_* 상수, +새 프로토타입, +extern for Reply, TimeOut, PollCnt |
| `src/PA/pa_1100_ts.c` | Time_Out_Rtn 본문을 기본 호출로 대체, Fifo_Event_Rtn 제거 |
| `src/PA/pa_1200_tr.c` | Time_Out_Rtn 본문을 기본 호출로 대체, Fifo_Event_Rtn 제거 |
| `src/PA/pa_7800_tr.c` | Time_Out_Rtn 본문을 기본 호출로 대체, Fifo_Event_Rtn 제거 |
| `src/PB/pb_1100_ts.c` | Time_Out_Rtn 본문을 기본 호출로 대체, Fifo_Event_Rtn 제거 |
| `src/PB/pb_1200_tr.c` | Time_Out_Rtn 본문을 기본 호출로 대체, Fifo_Event_Rtn 제거 |
| `src/PB/pb_7800_tr.c` | Time_Out_Rtn 본문을 기본 호출로 대체, Fifo_Event_Rtn 제거 |

---

## 8. 추정 영향

### 8.1 코드 감소

| 대상 | 이전 (6개 파일 총합) | 이후 (공유) | 순 감소 |
|--------|----------------------|----------------|---------------|
| Time_Out_Rtn | 274줄 | 60줄 | -214줄 |
| Fifo_Event_Rtn | 24줄 | 4줄 | -20줄 |
| Write_Data (가능하면) | ~200줄 | ~80줄 | -120줄 |
| **전체** | **~498줄** | **~144줄** | **~354줄** |

### 8.2 누적 영향 (Phase 1 + 2 + 3)

| 메트릭 | Phase 1 | Phase 2 | Phase 3 (est.) | 누적 |
|--------|---------|---------|---------------|------------|
| 추출된 함수 | 6 | 4 | 3-5 | 13-15 |
| 제거된 순 줄 | 1,133 | 605 | 234-354 | 1,972-2,092 |
| 감소 % | 13% | 7% | 3-4% | ~23-24% |
| 원본 기준선 | 8,843 | 7,710 | 7,105 | 8,843 → ~6,800 |

---

## 9. 다음 단계

1. [ ] 설계 문서 작성 (`pa-pb-dedup-phase3.design.md`) 실제 코드 인용 포함
2. [ ] 설계 단계에서 Write_Data 가능성 확인
3. [ ] 설계 문서 이후 구현 시작
4. [ ] 갭 분석 실행

---

## 버전 이력

| 버전 | 날짜 | 변경 사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-21 | 초기 초안 | Claude Code |
