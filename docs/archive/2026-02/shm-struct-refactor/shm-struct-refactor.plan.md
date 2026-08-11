# shm-struct-refactor 계획 문서

> **요약**: SHM(Shared Memory) 접근 코드의 type-safety 강화 및 구조체 기반 리팩토링
>
> **프로젝트**: FEP (Front-End Processor) for KRX
> **Author**: Claude Code
> **Date**: 2026-02-21
> **Status**: Draft

---

## 1. 개요

### 1.1 목표

FEP 시스템의 SHM(공유 메모리) 접근 코드에서 raw pointer arithmetic, hardcoded offset, 매직넘버 기반 접근 패턴을 구조체 기반 type-safe 접근으로 개선한다.

### 1.2 배경

- `shm_memory.h` (1,079줄)에 26개 주요 SHM 구조체 정의
- `shmsub.c`에서 raw `char*` 포인터에 `sizeof()` 누적 덧셈으로 SHM 섹션 매핑
- `shm_rw.c`에서 circular buffer offset을 수동 계산 (3개 유사 함수 중복)
- PA/PB/PX 소스 137개 파일 중 30+개가 직접 SHM 배열 인덱스 접근
- `tr-struct-refactor` (완료, 92%)에서 KRX 메시지의 byte offset→struct 전환 성공 경험 활용

### 1.3 관련 문서

- Archive: `docs/archive/2026-02/tr-struct-refactor/` (선행 리팩토링)
- Reference: `docs/FEP_Macro_Reference.md` (SHM 매크로 전체 목록)
- Reference: `docs/FEP_Architecture_Analysis.md` (IPC 아키텍처)

---

## 2. 범위

### 2.1 포함 범위

- [ ] `sub/shmsub.c` — raw pointer offset 체인을 구조체 기반으로 전환
- [ ] `sub/shm_rw.c` — circular buffer 3함수(DSHM_W/W2/WT)를 1개 통합 함수로 리팩토링
- [ ] `inc/shm_memory.h` — SHM 버전 마커 추가, 접근자 매크로 개선
- [ ] `sub/shmipc.c` — attach/detach 시 size 검증 추가
- [ ] `src/PZ/pz_memory_shm.c` — SHM 생성 로직 정합성 검증

### 2.2 제외 범위

- PA/PB 비즈니스 로직 소스의 `Shm_Risk[0].ProFit[mk_gbn][i]` 등 접근 패턴 변경 (Phase 2에서 별도 진행)
- SHM 크기 동적 할당 (현재 고정 크기 유지)
- 시장 데이터 구조체(`SHM_NOTE`, `SHM_FIN_FUT`) 내부 필드 변경
- 새로운 SHM 세그먼트 추가

---

## 3. 요구사항

### 3.1 기능 요구사항

| ID | 요구사항 | 우선순위 | 상태 |
|----|-------------|----------|--------|
| FR-01 | shmsub.c의 ptr += sizeof() 체인을 구조체 포인터 캐스팅으로 교체 | High | Pending |
| FR-02 | shm_rw.c의 DSHM_W/DSHM_W2/DSHM_WT를 단일 파라미터화 함수로 통합 | High | Pending |
| FR-03 | shm_memory.h에 SHM_VERSION 매크로 및 magic number 필드 추가 | Medium | Pending |
| FR-04 | shmipc.c attach 시 size mismatch 검증 로직 추가 | Medium | Pending |
| FR-05 | 중첩 매크로(TCP1_PORT 등 5단계) 접근 경로 문서화 및 inline 함수 대안 제시 | Low | Pending |
| FR-06 | pz_memory_shm.c의 SHM 생성 코드와 shmsub.c attach 코드 간 size 계산 일관성 검증 | High | Pending |

### 3.2 비기능 요구사항

| 분류 | 기준 | 측정 방법 |
|----------|----------|-------------------|
| 호환성 | 기존 75개 바이너리와 메모리 레이아웃 100% 호환 | 기존 바이너리로 SHM attach 테스트 |
| 성능 | SHM 접근 성능 저하 없음 (추가 함수 호출 오버헤드 무시 가능) | 기존 동작 동일 검증 |
| 이식성 | HP-UX, SunOS, AIX, Linux 4개 플랫폼 컴파일 보장 | 플랫폼별 컴파일 테스트 |
| 안정성 | ANSI C89 준수, 새로운 의존성 없음 | 컴파일러 경고 0 |

---

## 4. 성공 기준

### 4.1 완료 정의

- [ ] FR-01~FR-06 구현 완료
- [ ] `sub/` 라이브러리 빌드 성공 (`mk.sh sub`)
- [ ] 전체 모듈 빌드 성공 (`mk.sh all`)
- [ ] SHM 메모리 레이아웃 바이너리 호환성 확인 (sizeof 비교)
- [ ] Gap Analysis >= 90%

### 4.2 품질 기준

- [ ] 컴파일러 경고 0 (Linux `-Wall`, HP-UX `-Ae`)
- [ ] DSHM write 함수 코드 중복 50% 이상 감소
- [ ] shmsub.c raw pointer 연산 0건 (sizeof 체인 제거)

---

## 5. 위험 및 완화

| 위험 | 영향 | 가능성 | 완화 |
|------|--------|------------|------------|
| SHM 메모리 레이아웃 변경으로 기존 프로세스 호환성 파괴 | High | Medium | 구조체 내부 필드/순서 변경 없이 접근 방식만 개선. sizeof() 비교 자동 검증 추가 |
| C89 제약으로 inline 함수 사용 불가 (HP-UX) | Medium | High | static 함수 또는 매크로 유지. inline은 Linux/GCC 전용 주석으로 표시 |
| DSHM_W 통합 시 기존 3함수 호출부 전체 수정 필요 | Medium | Low | 기존 함수명을 wrapper 매크로로 유지하여 호출부 변경 최소화 |
| 다중 플랫폼(HP-UX/SunOS/AIX/Linux) 호환성 | Medium | Medium | Linux에서 개발/검증 후 서버 배포 전 컴파일 테스트 |

---

## 6. 아키텍처 고려사항

### 6.1 프로젝트 레벨

| 레벨 | 선택 |
|-------|:--------:|
| **Dynamic** (기존 FEP 프로젝트 레벨) | O |

### 6.2 핵심 아키텍처 결정

| 결정 | 옵션 | 선택 | 근거 |
|----------|---------|----------|-----------|
| SHM 접근 방식 | (A) 매크로 유지+문서화 / (B) static 함수 전환 / (C) 혼합 | (C) 혼합 | 기존 매크로 호환 유지하면서 핵심부만 함수화 |
| DSHM write 통합 | (A) 파라미터 플래그 / (B) 함수 포인터 | (A) 파라미터 플래그 | C89 호환, 단순성 |
| SHM 버전 관리 | (A) 헤더 magic number / (B) 별도 메타 영역 | (A) 헤더 magic | SUB_DAEMON_INFO 첫 필드에 추가, 레이아웃 최소 변경 |
| 바이너리 호환성 | (A) 레이아웃 변경 허용 / (B) 100% 유지 | (B) 100% 유지 | 운영 중인 시스템과 혼재 가능성 |

### 6.3 영향 범위 분석

```
변경 파일 (직접):
┌─────────────────────────────────────────┐
│ inc/shm_memory.h   — SHM_VERSION 추가    │
│ sub/shmipc.c       — size 검증 추가       │
│ sub/shmsub.c       — struct 기반 매핑     │
│ sub/shm_rw.c       — DSHM write 통합     │
│ src/PZ/pz_memory_shm.c — 정합성 검증     │
└─────────────────────────────────────────┘

영향 파일 (간접 — 빌드 검증만):
┌─────────────────────────────────────────┐
│ src/PA/*.c  (30+ files)                  │
│ src/PB/*.c  (18 processes)               │
│ src/PX/*.c  (utility)                    │
│ src/PZ/*.c  (management)                 │
└─────────────────────────────────────────┘

변경 안함:
┌─────────────────────────────────────────┐
│ cfg/*.ini           — 설정 파일 무관      │
│ utl/*.c             — DB 유틸리티 무관    │
│ sub/config_*.c      — Config 로더 무관   │
│ sub/tcpip_*.c       — TCP 레이어 무관     │
└─────────────────────────────────────────┘
```

---

## 7. 규칙 사전 조건

### 7.1 기존 프로젝트 규칙

- [x] `CLAUDE.md` has coding conventions section
- [x] `docs/FEP_Macro_Reference.md` exists (SHM 매크로 참조)
- [x] Pure C89/ANSI C, tab width 4, `*_FMT`/`*_S` struct suffix

### 7.2 Conventions for This Feature

| Category | Rule |
|----------|------|
| 새 함수명 | `Shm_` prefix (기존 패턴 유지) |
| 새 매크로 | `SHM_` prefix (기존 패턴 유지) |
| 구조체 변경 | 기존 필드 순서/크기 불변, 새 필드는 끝에만 추가 |
| 통합 함수 | 기존 함수명은 `#define` wrapper로 유지 |

---

## 8. Implementation Phases (예상)

| Phase | 내용 | 예상 변경 |
|-------|------|----------|
| Phase 1 | shmsub.c — struct 기반 매핑 (FR-01) | ~150줄 수정 |
| Phase 2 | shm_rw.c — DSHM write 통합 (FR-02) | ~200줄 수정, ~100줄 삭제 |
| Phase 3 | shm_memory.h — SHM_VERSION 추가 (FR-03) | ~20줄 추가 |
| Phase 4 | shmipc.c — size 검증 (FR-04) | ~30줄 추가 |
| Phase 5 | pz_memory_shm.c — 정합성 검증 (FR-06) | ~20줄 수정 |
| Phase 6 | 빌드 검증 (`mk.sh all`) | 코드 변경 없음 |

---

## 9. Data Sources

| 파일 | 줄 수 | 역할 |
|------|-------|------|
| `inc/shm_memory.h` | 1,079 | 26개 SHM 구조체 정의, 95개 접근 매크로 |
| `sub/shmsub.c` | ~200 | SHM attach/매핑 (raw ptr 체인) |
| `sub/shm_rw.c` | ~300 | Circular buffer R/W (3중복 함수) |
| `sub/shmipc.c` | ~150 | SHM create/attach/detach/remove |
| `src/PZ/pz_memory_shm.c` | ~300 | SHM 생성 및 size 계산 |

---

## 10. 다음 단계

1. [ ] Write design document (`shm-struct-refactor.design.md`)
2. [ ] 코드 리뷰 및 승인
3. [ ] Phase 1부터 순차 구현

---

## 버전 이력

| 버전 | 날짜 | 변경사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-21 | Initial draft | Claude Code |
