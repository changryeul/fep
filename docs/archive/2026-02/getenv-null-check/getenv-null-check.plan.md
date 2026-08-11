# getenv-null-check 계획

> **요약**: 22개 활성 파일의 42개 안전하지 않은 `getenv()` 호출에 NULL 안전성 추가
>
> **프로젝트**: FEP (Front-End Processor for KRX)
> **작성자**: Claude Code
> **날짜**: 2026-02-22
> **상태**: 초안
> **Feature #**: 13

---

## 1. 문제 설명

22개 활성 파일(5개 sub/ + 17개 src/)의 42개 `getenv()` 호출이 NULL 체크 없이 반환값을 `memcmp()`, `sprintf()`, `strcpy()`에 직접 전달합니다. 환경 변수가 설정되지 않으면 NULL 포인터 역참조로 인한 프로덕션 크래시가 발생합니다.

기존 `getenvironment.c`는 7개 경로 변수(`_P_LOG`, `_P_DAT` 등)를 적절한 NULL 체크와 함께 캐시하지만, `_FEP_DIV`(가장 자주 사용됨: 14개 파일에서 20개 호출)는 캐시되지 않습니다.

## 2. 근본 원인

- `_FEP_DIV`는 원래 7개 캐시 변수 이후에 추가되었으며 `Get_Environment()`에 통합되지 않음
- PX 유틸리티 프로세스는 캐시된 전역 변수 대신 경로 변수에 대해 `getenv()`를 직접 사용
- `host_name`, `INISAFENET_HOME`은 환경 특화적이며 안전 래퍼 없음

## 3. 접근 방식

### Category A: `_FEP_DIV`를 전역으로 캐시 (HIGH 영향, 14개 파일)

`getenvironment.c` + `fep_sub.h`에 `_FEP_DIV`를 추가하여 ~20개 직접 `getenv("_FEP_DIV")` 호출을 캐시 전역 변수로 교체합니다.

**파일**: getenvironment.c, fep_sub.h, + 14개 호출자 파일:
- sub/: shmsub.c (3), shm_rw.c (1), init_proc.c (1), config_db.c (1)
- PA/: pa_9999_us.c (1), pa_7100_ur.c (1), pa_5000_qr.c (1), pa_9000_mp.c (2), pa_5200_qs.c (1)
- PB/: pb_7100_ur.c (1)
- PW/: pw_1000_mp.c (1)
- PZ/: pz_memory_shm.c (3), pz_memory_conf.c (1), pz_fepp.c (1)

### Category B: `host_name` NULL 체크 (3개 파일, 5개 호출)

`getenv("host_name")`으로 `memcmp()` 전에 인라인 NULL 체크 추가합니다.

**파일**: pa_7000_mp.c (3), px_chkgap_auto1.c (1), pb_7100_ur.c (1+1 Log)

### Category C: `INISAFENET_HOME` NULL 체크 (3개 파일, 3개 호출)

`getenv("INISAFENET_HOME")`으로 `sprintf()` 전에 인라인 NULL 체크 추가합니다.

**파일**: pb_1100_ts.c, pb_1800_ts.c, pb_7800_tr.c

### Category D: PX 경로 변수 NULL 체크 (6개 파일, 8개 호출)

캐시된 전역 변수 대신 `getenv("_P_LOG")`, `getenv("_J_LOG")`, `getenv("_P_DAT")`를 직접 사용하는 PX 유틸리티입니다. 인라인 NULL 체크를 추가합니다.

**파일**: px_chkgap_man.c, px_chkgap_auto1.c, px_chkgap_auto3.c, px_autoju_chk.c, px_chkche.c, px_orderchk.c

### Category E: 나머지 NULL 체크 (3개 파일, 6개 호출)

- pa_5010_mp.c: `_FEP_HOME`, `_FEP_SYSTEM` (1개 sprintf에 2개)
- pa_9000_mp.c: `_P_BIN` (2개 호출)
- sub/config_loader.c: `_FEP_DB_MODE` (1개), `_FEP_DB` (2개)

## 4. FR 요약

| 범주 | 설명 | 파일 | 호출 |
|:----:|------|:---:|:---:|
| A | _FEP_DIV 전역 캐시 | 2 + 14 | 20 |
| B | host_name NULL 체크 | 3 | 5 |
| C | INISAFENET_HOME NULL 체크 | 3 | 3 |
| D | PX 경로 변수 NULL 체크 | 6 | 8 |
| E | 나머지 NULL 체크 | 3 | 6 |
| **합계** | | **22개 고유** | **42** |

## 5. 제외 항목

- `getenvironment.c` — 이미 NULL 체크 있음 (안전함)
- `px_cfgload.c`, `px_cfgback.c` — 이미 NULL 체크 있음 (안전함)
- BACK2025/, BACKUP/, .back, .org 파일 — 레거시 백업, 활성 코드 아님
- `config_db.c` line 1099 — 이미 로컬에 할당 후 체크함
- `config_db.c` line 2458 — cfg_dir 할당됨, 나중에 체크할 가능성 있음

## 6. 위험 평가

| 위험 | 수준 | 완화 |
|------|:---:|------|
| 프로덕션에서 누락된 환경 변수 | LOW | `pkg_env.sh`가 모든 변수 설정; 심화 방어 |
| NULL일 때 동작 변경 | LOW | 프로세스가 크래시 대신 조기 종료 — 더 안전 |
| 컴파일 breakage | LOW | 체크만 추가, 함수 서명 변경 없음 |

## 7. 검증 기준

- V-01: `_FEP_DIV`가 getenvironment.c에서 NULL 체크와 함께 캐시됨
- V-02: `_FEP_DIV`가 fep_sub.h에서 extern으로 선언됨
- V-03: 활성 src/sub/ 파일에서 직접 `getenv("_FEP_DIV")` 호출 0개
- V-04: 모든 `getenv("host_name")` 호출에 NULL guards가 있음
- V-05: 모든 `getenv("INISAFENET_HOME")` 호출에 NULL guards가 있음
- V-06: 모든 PX 경로 getenv() 호출에 NULL guards가 있음
- V-07: 모든 나머지 getenv() 호출에 NULL guards가 있음
- V-08: 정상 동작에 대한 기능 변경 없음 (환경 변수 설정됨)

---

## 버전 이력

| 버전 | 날짜 | 변경 사항 | 작성자 |
|------|------|---------|--------|
| 0.1 | 2026-02-22 | 초기 초안 | Claude Code |
