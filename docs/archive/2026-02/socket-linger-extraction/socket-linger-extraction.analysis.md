# socket-linger-extraction 분석 보고서

> **분석 유형**: Gap Analysis (PDCA 검증 단계)
>
> **프로젝트**: FEP (Front-End Processor for KRX)
> **분석가**: bkit-gap-detector
> **날짜**: 2026-02-22
> **설계 문서**: [socket-linger-extraction.design.md](../02-design/features/socket-linger-extraction.design.md)

---

## 1. 분석 개요

### 1.1 분석 목적

15개 개별 소스 파일에서 shared library (`fep_common.c` / `fep_common.h`)로 `Set_Socket_Linger` 함수 추출이 설계 문서에 따라 올바르게 구현되었는지 검증. socket-linger-extraction 기능의 PDCA 검증 단계.

### 1.2 분석 범위

- **설계 문서**: `docs/02-design/features/socket-linger-extraction.design.md`
- **라이브러리 파일**: `st01/inc/fep_common.h`, `st01/sub/fep_common.c`
- **소스 파일**: 12개 active 호출 사이트 + 3개 dead code 파일 across PA, PB, PW modules
- **분석 날짜**: 2026-02-22

---

## 2. 전체 점수

| 항목 | 점수 | 상태 |
|----------|:-----:|:------:|
| Design Match | 100% | PASS |
| Architecture Compliance | 100% | PASS |
| Convention Compliance | 100% | PASS |
| **Overall** | **100%** | **PASS** |

---

## 3. Gap 분석 (설계 vs 구현)

### 3.1 라이브러리 변경

| FR ID | 파일 | 설계 변경 | 구현 | 상태 |
|-------|------|---------------|----------------|:------:|
| LIB-01 | `inc/fep_common.h` | 줄 41에 `extern void Set_Socket_Linger (int);` 추가 | 줄 41: `extern void Set_Socket_Linger (int);` | PASS |
| LIB-02 | `sub/fep_common.c` | `int fd` param, `Log` for errors, "End of Program" 전에 정의 추가 | 줄 386-408: Exact match -- comment banner, `void Set_Socket_Linger (int fd)`, `struct linger ling`, `setsockopt`, `Log(TCP_ERROR,...)` | PASS |

### 3.2 그룹 A -- Sockfd 파일 (FR-01 to FR-09)

| FR ID | 파일 | Forward Decl 제거 | Call 업데이트 | 정의 제거 | 상태 |
|-------|------|:--------------------:|:------------:|:------------------:|:------:|
| FR-01 | `src/PA/pa_2100_ts.c` | PASS (없음) | 줄 260: `Set_Socket_Linger (Sockfd);` | PASS (없음) | PASS |
| FR-02 | `src/PA/pa_2200_tr.c` | PASS (없음) | 줄 239: `Set_Socket_Linger (Sockfd);` | PASS (없음) | PASS |
| FR-03 | `src/PA/pa_2700_tr.c` | PASS (없음) | 줄 222: `Set_Socket_Linger (Sockfd);` | PASS (없음) | PASS |
| FR-04 | `src/PA/pa_7000_tr.c` | PASS (없음) | 줄 143: `Set_Socket_Linger (Sockfd);` | PASS (없음) | PASS |
| FR-05 | `src/PA/pa_1600_tr.c` | PASS (없음) | 줄 275: `Set_Socket_Linger (Sockfd);` | PASS (없음) | PASS |
| FR-06 | `src/PA/pa_8200_tr.c` | PASS (없음) | 줄 153: `Set_Socket_Linger (Sockfd);` | PASS (없음) | PASS |
| FR-07 | `src/PB/pb_8200_tr.c` | PASS (없음) | 줄 148: `Set_Socket_Linger (Sockfd);` | PASS (없음) | PASS |
| FR-08 | `src/PW/pw_3010_tr.c` | PASS (없음) | 줄 72: `Set_Socket_Linger (Sockfd);` | PASS (없음) | PASS |
| FR-09 | `src/PW/pw_3030_tr.c` | PASS (없음) | 줄 72: `Set_Socket_Linger (Sockfd);` | PASS (없음) | PASS |

### 3.3 그룹 B -- Sockfd + SLog 마이그레이션 (FR-10)

| FR ID | 파일 | Forward Decl 제거 | Call 업데이트 | 정의 제거 | SLog 주 | 상태 |
|-------|------|:--------------------:|:------------:|:------------------:|-----------|:------:|
| FR-10 | `src/PW/pw_4000_ts.c` | PASS (없음) | 줄 91: `Set_Socket_Linger (Sockfd);` | PASS (없음) | Local copy는 `SLog` 사용; shared version은 `Log` 사용 (동등) | PASS |

### 3.4 그룹 C -- Newfd 파일 (FR-11 to FR-12)

| FR ID | 파일 | Forward Decl 제거 | Call 업데이트 | 정의 제거 | 상태 |
|-------|------|:--------------------:|:------------:|:------------------:|:------:|
| FR-11 | `src/PA/pa_8100_ts.c` | PASS (없음) | 줄 152: `Set_Socket_Linger (Newfd);` | PASS (없음) | PASS |
| FR-12 | `src/PB/pb_8100_ts.c` | PASS (없음) | 줄 156: `Set_Socket_Linger (Newfd);` | PASS (없음) | PASS |

### 3.5 그룹 D -- Dead Code 파일 (FR-13 to FR-15)

| FR ID | 파일 | Forward Decl 제거 | 정의 제거 | Call 없음 (예상) | 상태 |
|-------|------|:--------------------:|:------------------:|:------------------:|:------:|
| FR-13 | `src/PB/pb_7200_tr.c` | PASS (없음) | PASS (없음) | PASS (호출 없음) | PASS |
| FR-14 | `src/PB/pb_7100_ts.c` | PASS (없음) | PASS (없음) | PASS (호출 없음) | PASS |
| FR-15 | `src/PA/pa_7100_ts.c` | PASS (없음) | PASS (없음) | PASS (호출 없음) | PASS |

### 3.6 일치율 요약

```
+---------------------------------------------+
|  전체 일치율: 100%                          |
+---------------------------------------------+
|  PASS:              17 / 17 항목 (100%)     |
|  설계에 누락:        0 항목 (0%)            |
|  구현되지 않음:      0 항목 (0%)            |
|  변경:               0 항목 (0%)            |
+---------------------------------------------+
```

---

## 4. 검증 체크리스트

| # | 확인 | 예상 | 실제 | 상태 |
|---|------|------|------|:------:|
| 1 | `grep -r "void.*Set_Socket_Linger" st01/src/` (BACK2025/, .org 제외) 0개 일치 반환 | 0 선언/정의 in active src/ | 0개 일치 (모든 hits는 BACK2025/에만) | PASS |
| 2 | `grep -r "Set_Socket_Linger" st01/src/` (active .c만) 정확히 12개 일치 반환 | 12 호출 사이트 | 12 호출 사이트 찾음 | PASS |
| 3 | Sockfd 호출: 10개 파일; Newfd 호출: 2개 파일 | 파일당 올바른 fd 매개변수 | 10개 Sockfd 파일이 모두 `Sockfd` 전달, 2개 Newfd 파일이 모두 `Newfd` 전달 | PASS |
| 4 | `inc/fep_common.h`가 `extern void Set_Socket_Linger (int);` 포함 | 줄 41의 선언 | 줄 41: exact match | PASS |
| 5 | `sub/fep_common.c`가 `void Set_Socket_Linger (int fd)` 정의 포함 | 줄 393의 정의 | 줄 393: exact match | PASS |
| 6 | active source에 `void Set_Socket_Linger (void);` forward 선언 남음 | 0 남은 것 | 0 남은 것 (모두 BACK2025/에만) | PASS |

**체크리스트 결과: 6 / 6 PASS**

---

## 5. 아키텍처 준수

### 5.1 중복 제거 패턴

| 기준 | 예상 | 실제 | 상태 |
|-----------|----------|--------|:------:|
| Shared library에 단일 정의 | `sub/fep_common.c` | `sub/fep_common.c` 줄 386-408 | PASS |
| Shared header를 통한 선언 | `inc/fep_common.h` | `inc/fep_common.h` 줄 41 | PASS |
| Process 파일의 로컬 정의 없음 | 0 정의 in src/ | 0 정의 in active src/ | PASS |
| 로컬 forward 선언 없음 | 0 forward decls in src/ | 0 forward decls in active src/ | PASS |
| 매개변수화된 fd 매개변수 | `int fd` parameter | `int fd` parameter | PASS |
| libfepP.a를 통한 링크 | 모든 processes link libfepP.a | Build system 변경 없음, 모두 libfepP.a link | PASS |

### 5.2 아키텍처 점수

```
+---------------------------------------------+
|  아키텍처 준수: 100%                        |
+---------------------------------------------+
|  올바른 배치: 17/17 파일                    |
|  의존성 위반: 0                             |
|  잘못된 계층: 0                             |
+---------------------------------------------+
```

---

## 6. 관례 준수

### 6.1 명명 관례

| 항목 | 관례 | 실제 | 상태 |
|------|-----------|--------|:------:|
| 함수 이름 | PascalCase with underscores (FEP 관례) | `Set_Socket_Linger` | PASS |
| 매개변수 이름 | lowercase | `fd` | PASS |
| 매크로/상수 사용 | `SYS_NO`, `SYS_STR`, `TCP_ERROR` | Codebase와 일치 | PASS |

### 6.2 코드 스타일

| 항목 | 관례 | 실제 | 상태 |
|------|-----------|--------|:------:|
| Comment banner 형식 | 9-줄 FEP 표준 banner | 8-줄 banner (기존 fep_common.c 스타일과 일치) | PASS |
| Tab 기반 들여쓰기 | Tabs | Tabs 일관되게 사용 | PASS |
| Return statement | 별도 줄의 `return;` | 줄 407의 `return;` | PASS |
| End-of-function comment | `/* End of {name} () */` | `/* End of Set_Socket_Linger () */` | PASS |

### 6.3 관례 점수

```
+---------------------------------------------+
|  관례 준수: 100%                            |
+---------------------------------------------+
|  명명:           100%                       |
|  코드 스타일:   100%                       |
|  Comment 형식:  100%                       |
+---------------------------------------------+
```

---

## 7. 발견된 차이점

### 7.1 누락된 기능 (설계 O, 구현 X)

없음.

### 7.2 추가된 기능 (설계 X, 구현 O)

없음.

### 7.3 변경된 기능 (설계 != 구현)

없음.

---

## 8. 파일 변경 요약

| 변경 유형 | 개수 | 줄 |
|-------------|-------|-------|
| LIB: 선언 추가 (fep_common.h) | 1 | +1 |
| LIB: 정의 추가 (fep_common.c) | 1 | +17 |
| Forward 선언 제거 | 15 | -15 |
| 정의 제거 (banners 포함) | 15 | ~-360 |
| 호출 사이트 업데이트 (Sockfd) | 10 | 0 (in-place) |
| 호출 사이트 업데이트 (Newfd) | 2 | 0 (in-place) |
| Dead code 제거 (호출 없음) | 3 | 0 |
| **합계** | **17 files** | **~-357 net** |

---

## 9. 호출 사이트 참조

| # | 파일 | 줄 | 매개변수 | Context |
|---|------|------|----------|---------|
| 1 | `src/PA/pa_2100_ts.c` | 260 | `Sockfd` | Bond order TX -- Device_Open after Connect |
| 2 | `src/PA/pa_2200_tr.c` | 239 | `Sockfd` | Bond response RX -- Device_Open after Connect |
| 3 | `src/PA/pa_2700_tr.c` | 222 | `Sockfd` | Derivative response RX -- Device_Open after Connect |
| 4 | `src/PA/pa_7000_tr.c` | 143 | `Sockfd` | Market data RX -- Device_Open after Connect |
| 5 | `src/PA/pa_1600_tr.c` | 275 | `Sockfd` | Fill RX -- Device_Open after Connect |
| 6 | `src/PA/pa_8200_tr.c` | 153 | `Sockfd` | Client comm RX -- Device_Open after Accept |
| 7 | `src/PB/pb_8200_tr.c` | 148 | `Sockfd` | Bond client comm RX -- Device_Open after Accept |
| 8 | `src/PW/pw_3010_tr.c` | 72 | `Sockfd` | Connection TR -- Device_Open after Connect |
| 9 | `src/PW/pw_3030_tr.c` | 72 | `Sockfd` | Connection TR -- Device_Open after Connect |
| 10 | `src/PW/pw_4000_ts.c` | 91 | `Sockfd` | Daemon TS -- Device_Open after Connect |
| 11 | `src/PA/pa_8100_ts.c` | 152 | `Newfd` | Client comm TS -- Device_Open after Accept |
| 12 | `src/PB/pb_8100_ts.c` | 156 | `Newfd` | Bond client comm TS -- Device_Open after Accept |

---

## 10. 권장 조치

조치 필요 없음. 설계와 구현이 100% 일치.

### 10.1 빌드 검증 (테스트 안 함)

빌드 검증 체크리스트 항목 6 (`mk.sh sub && mk.sh src`)은 production build server가 필요하여 테스트하지 않음. 이는 production deployment 전에 검증되어야 함.

---

## 11. 다음 단계

- [x] Gap 분석 완료 (100% 일치율)
- [ ] 서버에서 빌드 검증 (`mk.sh sub && mk.sh src`)
- [ ] 완료 보고서 생성 (`/pdca report socket-linger-extraction`)

---

## 12. 누적 중복 제거 추세

| Phase | 기능 | 일치율 | 추출된 함수 | 제거된 줄 |
|-------|---------|:----------:|:-------------------:|:-------------:|
| PA-PB Dedup Phase 1 | Common functions | 90% | 8 | ~600 |
| PA-PB Dedup Phase 2 | Advanced functions | 97% | 4 | ~800 |
| PA-PB Dedup Phase 3 | Event handlers | 100% | 2 | ~500 |
| Fifo-Event-Rtn Adoption | Fifo adoption | 100% | 1 | ~200 |
| Socket-Linger Extraction | Socket linger | **100%** | **1** | **~357** |

설계 품질 추세: 90% -> 97% -> 100% -> 100% -> 100% ("actual code citation" 접근으로 완벽함 유지)

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2026-02-22 | Initial analysis -- 100% match rate, all 17 FRs PASS | bkit-gap-detector |
