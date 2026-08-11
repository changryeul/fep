# PX 모듈 코드 품질 감사 — 계획 문서

> **기능**: PX (유틸리티/모니터링 모듈 코드 품질 감사)
> **프로젝트**: FEP (KRX 프론트엔드 프로세서)
> **작성자**: Claude Code
> **날짜**: 2026-02-27
> **상태**: 초안

---

## 목표

모든 46개 PX (유틸리티/모니터링) 모듈 소스 파일의 체계적 코드 품질 감사. 버그, 안전하지 않은 패턴, C89 준수 문제 및 데드 코드 수정. 완료된 PB 모듈 감사 (100% 일치율, 28개 발견) 및 PA 모듈 감사 (100% 일치율, 38개 발견)의 패턴과 교훈 활용.

## 범위

- **범위 내**: `st01/src/PX/`의 모든 46개 `.c` 파일
- **범위 외**: 백업 파일 (`*.back`, `*.b`), 헤더 (`inc/`), 공유 라이브러리 (`sub/`), 다른 모듈 (PA/PB/PZ)
- **연기됨**: 빈 main 스텁 `px_chkgap.c` (자리 표시자 파일, 고칠 함수 코드 없음)

## 발견 사항 요약

| 심각도 | 개수 | 주제 |
|----------|-------|-------|
| CRITICAL | 7 | 잘못된 배열 인덱스, 누락된 리턴, 초기화되지 않은 변수 |
| HIGH | 23 | 전처리기 `#elif defined sun \|\| _AIX \|\| __linux` (누락된 `defined()`) |
| MEDIUM | 6 | 초기화되지 않은 플래그, `*buf==NULL`, 하드코딩된 호스트명, 도달할 수 없는 코드 |
| LOW | 2 | 사용하지 않는 변수 |
| **합계** | **38** | |

---

## 구현 배치

### 배치 1: CRITICAL 수정 (FR-01부터 FR-07) — 7개 발견

| FR | 파일 | 이슈 | 수정 |
|----|------|-------|-----|
| FR-01 | px_showsise.c:115 | 잘못된 배열 인덱스: 검색 결과가 `rt`일 때 `Shm_FinFut[i]` 사용 | `Shm_FinFut[i]`를 `Shm_FinFut[rt]`로 변경 |
| FR-02 | px_setfname.c:180-196 | `Change_File_Name()`은 `int`를 반환하지만 루프 후 리턴 없음 (루프 내에만 종료 경로 있음) | for-루프 후 `return 0;` 추가 또는 재구성 |
| FR-03 | px_setdname.c:180-196 | `Change_Dshm_Name()`은 `int`를 반환하지만 루프 후 리턴 없음 (FR-02와 동일 패턴) | for-루프 후 `return 0;` 추가 또는 재구성 |
| FR-04 | px_chkgap_auto1.c:18-19 | `send_sec`, `start_tm`, `end_tm` 초기화 안 됨; 루프가 비어 있으면 줄 211, 221에서 사용 | 선언 시 `= 0`으로 초기화 |
| FR-05 | px_chkgap_man.c:18-19 | FR-04와 동일한 초기화되지 않은 변수 패턴 | 선언 시 `= 0`으로 초기화 |
| FR-06 | px_chkgap_auto3.c:18-19 | FR-04와 동일한 초기화되지 않은 변수 패턴 | 선언 시 `= 0`으로 초기화 |
| FR-07 | px_runstop.c:18,174 | `run_flag=='s'`일 때 `reload_flag` 초기화 안 됨 (정지 경로가 두 번째 while 루프 건너뜀); `run_flag`도 선언 시 초기화 안 됨 | 둘 다 `0`으로 초기화하거나 조건부 로그 추가 |

### 배치 2: 전처리기 `defined()` 구문 (FR-08부터 FR-30) — 23개 파일

모든 23개 파일이 동일한 패턴을 가짐:
```c
// 현재 (버그 — _AIX와 __linux는 항상 참인 표현식, defined() 확인 아님):
#elif defined sun || _AIX || __linux

// 수정됨:
#elif defined(sun) || defined(_AIX) || defined(__linux)
```

| FR | 파일 | 줄 |
|----|------|------|
| FR-08 | px_cfgback.c | (확인 필요) |
| FR-09 | px_runstop.c | 167 |
| FR-10 | px_setcseq.c | 93 |
| FR-11 | px_setdate.c | 80 |
| FR-12 | px_setdelay.c | 95 |
| FR-13 | px_setdname.c | 163 |
| FR-14 | px_setdsize.c | 94 |
| FR-15 | px_setdtime.c | 105 |
| FR-16 | px_setfcnt.c | 184 |
| FR-17 | px_setfname.c | 163 |
| FR-18 | px_setfsize.c | 94 |
| FR-19 | px_sethandsk.c | 100 |
| FR-20 | px_setinfostat.c | 82 |
| FR-21 | px_setlstat.c | 144 |
| FR-22 | px_setnstat.c | 170 |
| FR-23 | px_setpseq.c | 93 |
| FR-24 | px_setpstat.c | 123 |
| FR-25 | px_setptime.c | 125 |
| FR-26 | px_setsisereco.c | 131 |
| FR-27 | px_setsstat.c | 108 |
| FR-28 | px_sett2ip.c | 153 |
| FR-29 | px_sett2seq.c | 109 |
| FR-30 | px_settout.c | 94 |

> **참고**: `px_setudp.c:151`도 이 패턴을 가짐 (합계 24개). grep 개수 확인 = 23개 파일. 설계 단계에서 `px_cfgback.c` 재확인 예정 — 이전 감사에서 수정됐을 수도 있거나 `*.back`만 사용할 수도 있음.

### 배치 3: MEDIUM 수정 (FR-31부터 FR-36) — 6개 발견

| FR | 파일 | 이슈 | 수정 |
|----|------|-------|-----|
| FR-31 | px_memok.c:768 | `Tcp1_Config_Read()`의 `continue_flag` 초기화 안 됨 — 줄 912에서 확인 | `continue_flag = 0`으로 초기화 |
| FR-32 | px_cfgback.c:460 | `*buf == NULL` — char를 NULL 포인터 상수와 비교 | `*buf == '\0'`으로 변경 |
| FR-33 | px_runstop.c:18,174 | 정지 경로 후 로그에 사용되는 `reload_flag` 초기화 안 됨 (FR-07 참조) | FR-07과 통합 |
| FR-34 | px_chkgap_auto1.c:111 | `memcmp` 확인에 하드코딩된 호스트명 `"ap67"` | 환경 변수 또는 설정 기반 확인으로 교체 |
| FR-35 | px_chkgap.c | 빈 main() 스텁 — 본문 없음, 리턴 없음 | `return 0;` 추가 또는 자리 표시자로 표시 |
| FR-36 | px_chktrcnt.c:97 | 모든 if/else 분기가 이미 종료된 후 도달할 수 없는 `exit(1)` | 도달할 수 없는 명령문 제거 |

> **참고**: FR-33은 FR-07과 통합됨 (동일한 `reload_flag` 이슈, 동일한 파일). 순 고유 중간 발견: 5개.

### 배치 4: LOW 수정 (FR-37, FR-38) — 정리

| FR | 파일 | 이슈 | 수정 |
|----|------|-------|-----|
| FR-37 | px_sethandsk.c:17 | 사용하지 않는 `fifo_name[128]` — 선언만 되고 사용 안 됨 | 변수 제거 |
| FR-38 | px_chkgap_auto1.c:20, px_chkgap_man.c:20, px_chkgap_auto3.c:20 | 사용하지 않는 `sub[4]` — 선언, memset, 읽기 안 됨 | 변수 제거 |

### 의도적으로 연기됨

| ID | 이슈 | 이유 |
|----|-------|--------|
| D-1 | px_chkgap.c 빈 main | 자리 표시자 파일 — 함수 코드 없음. 아마도 사용하지 않는 레거시 아티팩트. |
| D-2 | `*.back` 파일 버그 (px_cfgback.back, px_cfgload.back, px_memok.back) | 백업 파일 — 컴파일되지 않음, 배포되지 않음 |

---

## 위험 평가

| 위험 | 영향 | 완화 |
|------|--------|------------|
| FR-01 (잘못된 인덱스) 프로그램 출력 변경 | 표시 유틸리티만, 거래 영향 없음 | 낮은 위험: `px_showsise` 진단 도구만 영향 |
| FR-02/03 (누락된 리턴) 호출자 동작 변경 가능 | main()에서 직접 사용으로 호출됨 | 함수는 찾지 못 할 때 `exit(FAIL)` 호출 — 누락된 리턴은 루프 소진 후. 낮은 위험. |
| FR-34 (하드코딩된 호스트명) 자동 갭 확인 로직 영향 | 프로덕션 확인 자동 건너뜀 가능 | 호스트명 매핑에 대한 비즈니스 검토 필요 |
| 배치 2 전처리기 수정 비 HP-UX에서 컴파일 변경 | SunOS/AIX/Linux 빌드에 영향 | 현재 `_AIX`와 `__linux`는 정의된 상수로 평가됨 (보통 0이 아님), 따라서 동작이 대부분 플랫폼에서 "우연히 올바름". 수정은 명시적으로 만듦. |

## 성공 기준

- 모든 CRITICAL 및 HIGH 발견 수정
- 모든 MEDIUM 발견 수정
- 모든 LOW 발견 수정
- 대상 Linux 서버에서 빌드 검증 통과
- 갭 분석 >= 90% 일치율

---

## 버전 역사

| 버전 | 날짜 | 변경 사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-27 | 35/46 PX 파일 중 38개 발견과 함께 초기 계획 | Claude Code |
