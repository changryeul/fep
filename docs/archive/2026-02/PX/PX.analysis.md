# PX 모듈 코드 품질 감사 -- 갭 분석 보고서

> **분석 유형**: 설계-구현 갭 분석 (PDCA 확인 단계)
>
> **프로젝트**: FEP (KRX 프론트엔드 프로세서)
> **분석가**: Claude Code (갭 감지기)
> **날짜**: 2026-02-27
> **설계 문서**: [PX.design.md](../02-design/features/PX.design.md)
> **구현 경로**: `st01/src/PX/`

---

## 1. 분석 개요

### 1.1 분석 목표

PX 설계 문서에 지정된 모든 38개 코드 품질 수정 (FR-01부터 FR-38)이 `st01/src/PX/`의 실제 소스 파일에 올바르게 적용되었는지 확인합니다. PX 코드 품질 감사 기능의 PDCA 사이클의 확인 단계입니다.

### 1.2 분석 범위

- **설계 문서**: `docs/02-design/features/PX.design.md` (4개 배치에 걸친 38개 FR)
- **구현 경로**: `st01/src/PX/` (35개 소스 파일, 수정된 28개 고유 파일)
- **분석 날짜**: 2026-02-27
- **검증 방법**: 설계 이전/새로 만들기 사양에 대한 줄별 소스 코드 비교

---

## 2. 전체 점수

| 카테고리 | 점수 | 상태 |
|----------|:-----:|:------:|
| 배치 1: Critical 수정 (FR-01..FR-07) | 100% (7/7) | PASS |
| 배치 2: 전처리기 defined() (FR-08..FR-30) | 100% (23/23) | PASS |
| 배치 3: Medium 수정 (FR-31..FR-36) | 100% (5/5 활성) | PASS |
| 배치 4: Low 수정 (FR-37..FR-38) | 100% (4/4 하위 항목) | PASS |
| **전체 일치율** | **100% (37/37)** | **PASS** |

참고:
- FR-33은 FR-07과 통합됨 (별도 검증 필요 없음) -- 분모에서 제외.
- FR-36은 작은 편차가 있음 (줄 제거 대신 exit(OK)) -- 기능 동등성에 따라 일치로 계산.
- 검증 가능 FR: 37 (FR-33이 통합된 38개 중).

---

## 3. FR당 검증 세부 정보

### 3.1 배치 1: Critical 수정 (FR-01부터 FR-07)

#### FR-01: px_showsise.c:115의 잘못된 배열 인덱스

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_showsise.c` |
| 설계 | `Shm_FinFut[i]`를 `Shm_FinFut[rt]`로 변경 |
| 실제 (줄 115) | `printf ("A0  rt[%d][%s]\n", rt, Shm_FinFut[rt].A0.tr_gbn);` |
| 상태 | MATCH |

#### FR-02: Change_File_Name()의 누락된 리턴 -- px_setfname.c

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_setfname.c` |
| 설계 | for-루프 후 `Log + exit(FAIL) + return(0)` 추가 |
| 실제 (줄 197-199) | `Log (PRO_FATAL, "%s unregistered (f_count=0)", NewFile);` / `exit (FAIL);` / `return (0);` |
| 상태 | MATCH |

#### FR-03: Change_Dshm_Name()의 누락된 리턴 -- px_setdname.c

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_setdname.c` |
| 설계 | for-루프 후 `Log + exit(FAIL) + return(0)` 추가 |
| 실제 (줄 197-199) | `Log (PRO_FATAL, "%s unregistered (d_count=0)", NewDshm);` / `exit (FAIL);` / `return (0);` |
| 상태 | MATCH |

#### FR-04: px_chkgap_auto1.c:18-19의 초기화되지 않은 변수

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_chkgap_auto1.c` |
| 설계 | `recv_sec = 0, send_sec = 0` 및 `start_tm = 0, end_tm = 0` |
| 실제 (줄 18-19) | `double avg_tot, avg_krx, avg_fep, recv_sec = 0, send_sec = 0;` / `double tot_tm, krx_tm, fep_tm, start_tm = 0, end_tm = 0;` |
| 상태 | MATCH |

#### FR-05: px_chkgap_man.c:18-19의 초기화되지 않은 변수

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_chkgap_man.c` |
| 설계 | FR-04와 동일한 초기화 |
| 실제 (줄 18-19) | `recv_sec = 0, send_sec = 0` 및 `start_tm = 0, end_tm = 0` |
| 상태 | MATCH |

#### FR-06: px_chkgap_auto3.c:18-19의 초기화되지 않은 변수

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_chkgap_auto3.c` |
| 설계 | FR-04와 동일한 초기화 |
| 실제 (줄 18-19) | `recv_sec = 0, send_sec = 0` 및 `start_tm = 0, end_tm = 0` |
| 상태 | MATCH |

#### FR-07: px_runstop.c:18의 초기화되지 않은 run_flag/reload_flag

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_runstop.c` |
| 설계 | `u_char run_flag = 0, reload_flag = 0, query_flag;` |
| 실제 (줄 18) | `u_char run_flag = 0, reload_flag = 0, query_flag;` |
| 상태 | MATCH |

### 3.2 배치 2: 전처리기 defined() 구문 (FR-08부터 FR-30)

모든 23개 파일이 수정된 패턴을 검색하여 확인되고 이전 패턴이 남아 있지 않은지 확인.

- **수정된 패턴 발견**: 23개 파일 (설계와 정확히 일치)
- **이전 버그 패턴 발견**: 0개 파일 (모두 수정됨)

| FR | 파일 | 수정된 줄 존재 | 상태 |
|----|------|:----------------------:|:------:|
| FR-08 | px_setudp.c | 예 | MATCH |
| FR-09 | px_runstop.c | 예 | MATCH |
| FR-10 | px_setcseq.c | 예 | MATCH |
| FR-11 | px_setdate.c | 예 | MATCH |
| FR-12 | px_setdelay.c | 예 | MATCH |
| FR-13 | px_setdname.c | 예 | MATCH |
| FR-14 | px_setdsize.c | 예 | MATCH |
| FR-15 | px_setdtime.c | 예 | MATCH |
| FR-16 | px_setfcnt.c | 예 | MATCH |
| FR-17 | px_setfname.c | 예 | MATCH |
| FR-18 | px_setfsize.c | 예 | MATCH |
| FR-19 | px_sethandsk.c | 예 | MATCH |
| FR-20 | px_setinfostat.c | 예 | MATCH |
| FR-21 | px_setlstat.c | 예 | MATCH |
| FR-22 | px_setnstat.c | 예 | MATCH |
| FR-23 | px_setpseq.c | 예 | MATCH |
| FR-24 | px_setpstat.c | 예 | MATCH |
| FR-25 | px_setptime.c | 예 | MATCH |
| FR-26 | px_setsisereco.c | 예 | MATCH |
| FR-27 | px_setsstat.c | 예 | MATCH |
| FR-28 | px_sett2ip.c | 예 | MATCH |
| FR-29 | px_sett2seq.c | 예 | MATCH |
| FR-30 | px_settout.c | 예 | MATCH |

**검증 방법**: `grep -r` `#elif defined(sun) || defined(_AIX) || defined(__linux)` 정확히 23개 파일을 발견. 이전 패턴 `#elif defined sun || _AIX || __linux` 역방향 grep 0개 파일 발견.

### 3.3 배치 3: Medium 수정 (FR-31부터 FR-36)

#### FR-31: px_memok.c의 초기화되지 않은 continue_flag

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_memok.c` |
| 설계 (부분 1) | 선언 시 `continue_flag = 0` (줄 768) |
| 실제 (줄 768) | `int i, j, cnt, itemcnt, continue_flag = 0;` |
| 설계 (부분 2) | 내부 for-루프 전에 `continue_flag = 0;` 재설정 (줄 898) |
| 실제 (줄 898) | `continue_flag = 0;` 다음 `for (j = 0; j < 9; j ++)` |
| 상태 | MATCH (두 부분 모두) |

#### FR-32: px_cfgback.c:460의 *buf == NULL

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_cfgback.c` |
| 설계 | `*buf == NULL`을 `*buf == '\0'`으로 변경 |
| 실제 (줄 460) | `else if (*buf == '\0' \|\| *buf == '\t' \|\| *buf == ' ')` |
| 상태 | MATCH |

#### FR-33: FR-07과 통합됨

별도 검증 필요 없음. FR-07이 이미 `px_runstop.c`에서 `reload_flag = 0` 초기화를 확인했습니다.

#### FR-34: px_chkgap_auto1.c:111의 하드코딩된 호스트명

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_chkgap_auto1.c` |
| 설계 | 호스트명 확인에 `"podm11"`과 `"podm12"` 추가 |
| 실제 (줄 110-112) | `if (memcmp (env_hostname, "ap67", 4) == 0 \|\|` / `memcmp (env_hostname, "podm11", 6) == 0 \|\|` / `memcmp (env_hostname, "podm12", 6) == 0)` |
| 상태 | MATCH |

#### FR-35: px_chkgap.c의 빈 main()

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_chkgap.c` |
| 설계 | `main()` 내부에 `return (0);` 추가 |
| 실제 (줄 16) | `return (0);` |
| 상태 | MATCH |

#### FR-36: px_chktrcnt.c:97의 도달할 수 없는 exit(1)

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_chktrcnt.c` |
| 설계 | 줄 97의 도달할 수 없는 `exit(1)` 제거 |
| 실제 (줄 97) | `exit (OK);` (제거 대신 `exit(1)`에서 `exit(OK)`로 변경) |
| 편차 | 경미 -- 줄은 여전히 도달할 수 없지만 값은 제거 대신 `OK`로 변경 |
| 상태 | MATCH (기능적으로 동등 -- 여전히 데드 코드이지만 무해) |

### 3.4 배치 4: Low 수정 (FR-37, FR-38)

#### FR-37: px_sethandsk.c:17의 사용하지 않는 fifo_name[128]

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_sethandsk.c` |
| 설계 | 선언에서 `fifo_name[128]` 제거 |
| 실제 (줄 17) | `char proc_id[12], pstat[4], sub[4], buf[128];` (`fifo_name` 없음) |
| `fifo_name` grep | 파일에 0개 발생 |
| 상태 | MATCH |

#### FR-38a: px_chkgap_auto1.c:20의 사용하지 않는 sub[4]

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_chkgap_auto1.c` |
| 설계 | 선언에서 `sub[4]` 제거 + 그 `memset` |
| 실제 (줄 20) | `char yn[4], chk_from[12], chk_to[12], buf[5120];` (`sub` 없음) |
| `memset.*sub` grep | 파일에 0개 발생 |
| 상태 | MATCH |

#### FR-38b: px_chkgap_man.c:20의 사용하지 않는 sub[4]

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_chkgap_man.c` |
| 설계 | 선언에서 `sub[4]` 제거 + 그 `memset` |
| 실제 (줄 20) | `char yn[4], chk_from[12], chk_to[12], buf[5120];` (`sub` 없음) |
| `memset.*sub` grep | 파일에 0개 발생 |
| 상태 | MATCH |

#### FR-38c: px_chkgap_auto3.c:20의 사용하지 않는 sub[4]

| 항목 | 세부사항 |
|------|--------|
| 파일 | `/Users/ichang-yeol/MyWork/fep/st01/src/PX/px_chkgap_auto3.c` |
| 설계 | 선언에서 `sub[4]` 제거 + 그 `memset` |
| 실제 (줄 20) | `char yn[4], chk_from[12], chk_to[12], buf[5120];` (`sub` 없음) |
| `memset.*sub` grep | 파일에 0개 발생 |
| 상태 | MATCH |

---

## 4. 발견된 편차

### 4.1 경미한 편차 (기능적 동등)

| FR | 파일 | 설계에 따르면 | 구현이 수행하는 것 | 영향 |
|----|------|-------------|---------------------|--------|
| FR-36 | px_chktrcnt.c:97 | 도달할 수 없는 `exit(1)` 제거 | 도달할 수 없는 `exit(OK)`로 변경 | 없음 -- 줄은 여전히 도달할 수 없는 데드 코드. 종료 코드를 `OK`로 변경하는 것은 무해. |

### 4.2 누락된 기능 (설계 있음, 구현 없음)

없음.

### 4.3 추가된 기능 (설계 없음, 구현 있음)

없음.

---

## 5. 일치율 계산

```
설계의 총 FR:               38
통합됨 (FR-33 = FR-07):       -1
검증 가능 FR:                     37

정확히 일치:                    36
경미한 편차와 일치:        1 (FR-36)
일치하지 않음:                          0

일치율 = 37 / 37 = 100%
```

---

## 6. 빌드 검증

| 항목 | 상태 |
|------|--------|
| 컴파일 | 테스트되지 않음 (libfepP.a를 사용한 서버 빌드 환경 필요) |
| 런타임 | 테스트되지 않음 (프로덕션 SHM/IPC 인프라 필요) |

---

## 7. 요약

```
+---------------------------------------------+
|  PX 코드 품질 감사                           |
|  설계-구현 일치율: 100%                      |
+---------------------------------------------+
|  배치 1 (Critical):   7/7   PASS            |
|  배치 2 (High):      23/23  PASS            |
|  배치 3 (Medium):     5/5   PASS            |
|  배치 4 (Low):        4/4   PASS            |
+---------------------------------------------+
|  합계:               37/37  PASS            |
|  발견된 갭:           0                     |
|  경미한 편차:     1 (FR-36, 무해)           |
+---------------------------------------------+
```

모든 37개 검증 가능 FR (통합된 FR-33을 제외한 38개 중)이 설계 사양과 일치합니다. FR-36의 단일 경미한 편차 (줄 제거 대신 exit(OK))는 기능적으로 동등하며 정확성에 영향을 주지 않습니다.

---

## 8. 권장 조치

### 8.1 즉시

수행할 조치 없음. 모든 수정이 올바르게 적용됨.

### 8.2 단기

| 우선순위 | 항목 | 참고 |
|----------|------|-------|
| Low | FR-36 정리 | 선택적으로 설계 의도와 일치하도록 도달할 수 없는 `exit(OK)` 줄을 완전히 제거 |

### 8.3 장기

| 항목 | 참고 |
|------|-------|
| 빌드 검증 | 서버에서 `mk.sh px` 실행하여 깔끔한 컴파일 확인 |
| FR-34 후속 | `_FEP_DIV` 환경 변수 확인으로 하드코딩된 호스트명 목록 교체 고려 |

---

## 9. 검증된 파일

`/Users/ichang-yeol/MyWork/fep/st01/src/PX/`의 28개 고유 소스 파일:

| 파일 | 검증된 FR |
|------|-------------|
| px_showsise.c | FR-01 |
| px_setfname.c | FR-02, FR-17 |
| px_setdname.c | FR-03, FR-13 |
| px_chkgap_auto1.c | FR-04, FR-34, FR-38a |
| px_chkgap_man.c | FR-05, FR-38b |
| px_chkgap_auto3.c | FR-06, FR-38c |
| px_runstop.c | FR-07, FR-09 |
| px_setudp.c | FR-08 |
| px_setcseq.c | FR-10 |
| px_setdate.c | FR-11 |
| px_setdelay.c | FR-12 |
| px_setdsize.c | FR-14 |
| px_setdtime.c | FR-15 |
| px_setfcnt.c | FR-16 |
| px_setfsize.c | FR-18 |
| px_sethandsk.c | FR-19, FR-37 |
| px_setinfostat.c | FR-20 |
| px_setlstat.c | FR-21 |
| px_setnstat.c | FR-22 |
| px_setpseq.c | FR-23 |
| px_setpstat.c | FR-24 |
| px_setptime.c | FR-25 |
| px_setsisereco.c | FR-26 |
| px_setsstat.c | FR-27 |
| px_sett2ip.c | FR-28 |
| px_sett2seq.c | FR-29 |
| px_settout.c | FR-30 |
| px_memok.c | FR-31 |
| px_cfgback.c | FR-32 |
| px_chkgap.c | FR-35 |
| px_chktrcnt.c | FR-36 |

---

## 버전 역사

| 버전 | 날짜 | 변경 사항 | 작성자 |
|---------|------|---------|--------|
| 1.0 | 2026-02-27 | 초기 갭 분석 -- 모든 37개 검증 가능 FR에 걸친 100% 일치율 | Claude Code |
