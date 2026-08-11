# getenv-null-check 설계

> **요약**: 활성 파일의 모든 안전하지 않은 `getenv()` 호출에 NULL 안전성 추가
>
> **프로젝트**: FEP (Front-End Processor for KRX)
> **작성자**: Claude Code
> **날짜**: 2026-02-22
> **상태**: 초안
> **Feature #**: 13
> **계획 참조**: `docs/01-plan/features/getenv-null-check.plan.md`

---

## 1. 범위 수정

각 `getenv()` 호출의 상세 검증 후 계획 수치가 수정되었습니다:

**범위에서 제거됨** (이미 안전함 — NULL 체크 존재):
- `sub/config_loader.c` — 모든 3개 호출 (lines 53, 286, 414) 로컬에 할당 + NULL 체크
- `sub/config_db.c` — 모든 3개 호출 (lines 93, 1099, 2458) 로컬에 할당 + NULL 체크
- `src/PX/px_cfgload.c` — 두 호출 (lines 52, 117) 이미 NULL 체크 있음
- `src/PX/px_cfgback.c` — 호출 (line 51) 이미 NULL 체크 있음

**범위에 추가됨** (검증 중 발견):
- `pb_7100_ur.c:88` — `"FEP_DIV"`(언더스코어 누락!)를 `"_FEP_DIV"` 대신 사용. 버그 수정: `_FEP_DIV` 캐시 글로벌로 교체.
- `pa_9000_mp.c:406,408` — `getenv("_P_BIN")`을 `_FEP_BIN`(Get_Environment()로 캐시됨)으로 교체 가능

**수정된 합계**: 24개 고유 호출자 파일 + 2개 인프라 파일 = 26개 파일 총계에서 41개 안전하지 않은 호출.

## 2. 설계 전략

### Category A: `_FEP_DIV`를 전역으로 캐시 (13개 파일에서 18개 호출)

**인프라 변경**:
1. `getenvironment.c`: 8번째 변수로 `_FEP_DIV` 캐싱 추가, NULL일 때 `-8` 반환
2. `fep_sub.h`: `_GLOBAL` 및 `extern` 블록 모두에 `char *_FEP_DIV` 추가

**호출자 변환 패턴** (모든 13개 파일이 동일한 패턴 사용):
```c
/* BEFORE: */
if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)

/* AFTER: */
if (memcmp (_FEP_DIV, "TEST", 4) == 0)
```

NULL 안전성은 `Get_Environment()`로 보장되며, `_FEP_DIV`가 NULL이면 `-8`을 반환하여 프로세스를 종료하고 호출자 코드에 도달하지 않습니다.

**특별한 경우 — `pb_7100_ur.c:88`**: `"FEP_DIV"`(언더스코어 누락)를 사용. 이는 잠재적 버그 — 잘못된 환경 변수를 읽습니다. 수정: `_FEP_DIV` 글로벌로 교체.

**특별한 경우 — `pa_9000_mp.c:406,408`**: `getenv("_P_BIN")`을 사용하지만 `_FEP_BIN`은 이미 캐시됨. `_FEP_BIN`으로 교체.

### Category B: NULL check `host_name` (3개 파일의 6개 호출)

**패턴**: 로컬 변수에 한 번 할당, NULL 체크, 그 후 로컬 사용.

```c
/* BEFORE: */
if ( (memcmp ((char *)getenv ("host_name"), "ap54", 4) == 0) ||
     (memcmp ((char *)getenv ("host_name"), "ap64", 4) == 0) )

/* AFTER: */
{
    char *env_hostname = getenv ("host_name");
    if (env_hostname == NULL) env_hostname = "";

    if ( (memcmp (env_hostname, "ap54", 4) == 0) ||
         (memcmp (env_hostname, "ap64", 4) == 0) )
```

`""`을 폴백으로 사용 (종료 안 함). host_name은 중요하지 않은 분기(예: 인터페이스 이름 선택)를 제어하기 때문. 빈 문자열은 호스트명 미일치를 의미 → 안전한 기본 경로.

### Category C: NULL check `INISAFENET_HOME` (3개 파일의 3개 호출)

**패턴**: 로컬에 할당, NULL 체크 + Log + return.

```c
/* BEFORE: */
memset (KRX_INITECH_CONF_PATH, 0, sizeof(KRX_INITECH_CONF_PATH));
sprintf(KRX_INITECH_CONF_PATH, "%s/conf/INISAFENet.cnf", getenv("INISAFENET_HOME"));

/* AFTER: */
{
    char *env_inisafe = getenv ("INISAFENET_HOME");
    if (env_inisafe == NULL)
    {
        Log (SYS_FATAL, "getenv(INISAFENET_HOME) is NULL");
        return;
    }
    memset (KRX_INITECH_CONF_PATH, 0, sizeof(KRX_INITECH_CONF_PATH));
    sprintf(KRX_INITECH_CONF_PATH, "%s/conf/INISAFENet.cnf", env_inisafe);
}
```

Log + return 사용. INISAFENET_HOME은 암호화에 중요 — 없으면 프로세스가 작동할 수 없음.

### Category D: NULL check PX path vars (6개 파일의 8개 호출)

**패턴**: 로컬에 할당, NULL 체크 + printf + exit(1).

```c
/* BEFORE: */
sprintf (path, "%s/PB/%s/pb_1252_ts", (char *)getenv ("_P_LOG"), day);

/* AFTER: */
{
    char *env_log = getenv ("_P_LOG");
    if (env_log == NULL)
    {
        printf ("ERROR: getenv(_P_LOG) is NULL\n");
        exit (1);
    }
    sprintf (path, "%s/PB/%s/pb_1252_ts", env_log, day);
}
```

printf + exit(1) 사용. PX 유틸리티는 독립형 CLI 도구 (Log 인프라 없음).

### Category E: 나머지 (2개 파일의 6개 호출)

**`pa_5010_mp.c:1047`** — `_FEP_HOME` + `_FEP_SYSTEM` 한 sprintf에서:
```c
/* BEFORE: */
sprintf (path, "%s%s1/utl/overfile/overatm.dat",
    (char *)getenv("_FEP_HOME"), (char *)getenv("_FEP_SYSTEM"));

/* AFTER: */
{
    char *env_home = getenv ("_FEP_HOME");
    char *env_sys  = getenv ("_FEP_SYSTEM");
    if (env_home == NULL || env_sys == NULL)
    {
        Log (SYS_FATAL, "getenv(_FEP_HOME or _FEP_SYSTEM) is NULL");
        return;
    }
    sprintf (path, "%s%s1/utl/overfile/overatm.dat", env_home, env_sys);
}
```

**`pa_9000_mp.c:406,408`** — `_P_BIN` (이미 `_FEP_BIN`으로 캐시됨):
```c
/* BEFORE: */
Log (USR_OK, "cp [%s/%s %s/%s]", (char *)getenv("_P_BIN"), st_name, (char *)getenv("_P_BIN"), pname);
sprintf (cmd, "cp %s/%s %s/%s", (char *)getenv("_P_BIN"), st_name, (char *)getenv("_P_BIN"), pname);

/* AFTER: */
Log (USR_OK, "cp [%s/%s %s/%s]", _FEP_BIN, st_name, _FEP_BIN, pname);
sprintf (cmd, "cp %s/%s %s/%s", _FEP_BIN, st_name, _FEP_BIN, pname);
```

## 3. FR 항목들

### Batch 1: 인프라 (2개 파일)

| FR | 파일 | 변경 |
|----|------|--------|
| FR-01 | sub/getenvironment.c | `_FEP_DIV` 캐싱 추가: `getenv("_FEP_DIV")`, NULL일 때 `-8` 반환 |
| FR-02 | inc/fep_sub.h | `_GLOBAL` 블록 (라인 ~240)과 `extern` 블록 (라인 ~255)에 `char *_FEP_DIV` 추가 |

### Batch 2: Category A — sub/ 호출자 (3개 파일, 5개 호출)

| FR | 파일 | 라인 | 변경 |
|----|------|-------|--------|
| FR-03 | sub/shmsub.c | 116, 142, 218 | `(char *)getenv ("_FEP_DIV")` → `_FEP_DIV` (3x) |
| FR-04 | sub/shm_rw.c | 405 | `(char *)getenv ("_FEP_DIV")` → `_FEP_DIV` (1x) |
| FR-05 | sub/init_proc.c | 130 | `(char *)getenv ("_FEP_DIV")` → `_FEP_DIV` (1x) |

### Batch 3: Category A — src/PA 호출자 (5개 파일, 6개 호출)

| FR | 파일 | 라인 | 변경 |
|----|------|-------|--------|
| FR-06 | PA/pa_9999_us.c | 164 | `(char *)getenv ("_FEP_DIV")` → `_FEP_DIV` |
| FR-07 | PA/pa_7100_ur.c | 175 | `(char *)getenv ("_FEP_DIV")` → `_FEP_DIV` |
| FR-08 | PA/pa_5000_qr.c | 107 | `(char *)getenv ("_FEP_DIV")` → `_FEP_DIV` |
| FR-09 | PA/pa_5200_qs.c | 108 | `(char *)getenv ("_FEP_DIV")` → `_FEP_DIV` |
| FR-10 | PA/pa_9000_mp.c | 172, 173 | `(char *)getenv ("_FEP_DIV")` → `_FEP_DIV` (2x) |

### Batch 4: Category A — src/PB+PW+PZ 호출자 (5개 파일, 6개 호출)

| FR | 파일 | 라인 | 변경 |
|----|------|-------|--------|
| FR-11 | PB/pb_7100_ur.c | 88 | `(char*)getenv ("FEP_DIV")` → `_FEP_DIV` (버그 수정: 잘못된 환경 변수 이름) |
| FR-12 | PW/pw_1000_mp.c | 61 | `(char *)getenv ("_FEP_DIV")` → `_FEP_DIV` |
| FR-13 | PZ/pz_memory_shm.c | 31, 63, 193 | `(char *)getenv ("_FEP_DIV")` → `_FEP_DIV` (3x) |
| FR-14 | PZ/pz_memory_conf.c | 1109 | `(char *)getenv ("_FEP_DIV")` → `_FEP_DIV` |
| FR-15 | PZ/pz_fepp.c | 244 | `(char *)getenv ("_FEP_DIV")` → `_FEP_DIV` |

### Batch 5: Category B — host_name NULL 체크 (3개 파일, 6개 호출)

| FR | 파일 | 라인 | 변경 |
|----|------|-------|--------|
| FR-16 | PA/pa_7000_mp.c | 61-67 | `env_hostname` 로컬 + NULL→`""` 폴백 추가. 3x getenv 교체 |
| FR-17 | PB/pb_7100_ur.c | 325, 330 | `env_hostname` 로컬 + NULL→`""` 폴백 추가. 2x getenv 교체 |
| FR-18 | PX/px_chkgap_auto1.c | 100 | `env_hostname` 로컬 + NULL→`""` 폴백 추가. 1x getenv 교체 |

### Batch 6: Category C — INISAFENET_HOME NULL 체크 (3개 파일, 3개 호출)

| FR | 파일 | 라인 | 변경 |
|----|------|-------|--------|
| FR-19 | PB/pb_1100_ts.c | 350-352 | `env_inisafe` 로컬 + NULL→Log+return 추가. getenv 교체 |
| FR-20 | PB/pb_1800_ts.c | 350-352 | 같은 패턴 |
| FR-21 | PB/pb_7800_tr.c | 255-257 | 같은 패턴 |

### Batch 7: Category D — PX path var NULL 체크 (6개 파일, 8개 호출)

| FR | 파일 | 라인 | 환경 변수 | 변경 |
|----|------|-------|---------|--------|
| FR-22 | PX/px_chkgap_man.c | 100 | `_P_LOG` | printf+exit(1) 패턴 |
| FR-23 | PX/px_chkgap_auto1.c | 101, 103 | `_P_LOG`, `_J_LOG` | printf+exit(1) 패턴 |
| FR-24 | PX/px_chkgap_auto3.c | 100 | `_P_LOG` | printf+exit(1) 패턴 |
| FR-25 | PX/px_autoju_chk.c | 106, 108 | `_J_LOG`, `_P_LOG` | printf+exit(1) 패턴 |
| FR-26 | PX/px_chkche.c | 55 | `_P_DAT` | printf+exit(1) 패턴 |
| FR-27 | PX/px_orderchk.c | 66 | `_P_DAT` | printf+exit(1) 패턴 |

### Batch 8: Category E — 나머지 NULL 체크 (2개 파일, 6개 호출)

| FR | 파일 | 라인 | 변경 |
|----|------|-------|--------|
| FR-28 | PA/pa_5010_mp.c | 1046-1047 | `_FEP_HOME`+`_FEP_SYSTEM` → 로컬 변수 + NULL→Log+return |
| FR-29 | PA/pa_9000_mp.c | 406, 408 | `(char *)getenv("_P_BIN")` → `_FEP_BIN` (4x, 캐시된 글로벌) |

## 4. 구현 순서

```
Batch 1: 인프라 (getenvironment.c, fep_sub.h)         — 2개 파일
Batch 2: Cat A sub/ 호출자                                     — 3개 파일
Batch 3: Cat A PA/ 호출자                                      — 5개 파일
Batch 4: Cat A PB+PW+PZ/ 호출자                               — 5개 파일
Batch 5: Cat B host_name (PA+PB+PX)                            — 3개 파일
Batch 6: Cat C INISAFENET_HOME (PB)                            — 3개 파일
Batch 7: Cat D PX path vars                                     — 6개 파일
Batch 8: Cat E 나머지 (PA)                                   — 2개 파일
                                                        합계: 24개 고유 파일에서 26개 편집
```

주의: `pa_9000_mp.c`는 Batch 3 (Cat A) + Batch 8 (Cat E)에서 편집. `pb_7100_ur.c`는 Batch 4 (Cat A) + Batch 5 (Cat B)에서 편집. `px_chkgap_auto1.c`는 Batch 5 (Cat B) + Batch 7 (Cat D)에서 편집.

## 5. 수정되지 않은 파일 (검증된 안전함)

| 파일 | 이유 |
|------|--------|
| sub/getenvironment.c | 이미 NULL 체크 (7개 변수) — 여기에 추가 |
| sub/config_db.c | 라인 93, 1099, 2458 — 모두 로컬 할당 + NULL 체크 |
| sub/config_loader.c | 라인 53, 286, 414 — 모두 로컬 할당 + NULL 체크 |
| PX/px_cfgload.c | 라인 52, 117 — 이미 NULL 체크 있음 |
| PX/px_cfgback.c | 라인 51 — 이미 NULL 체크 있음 |
| BACK2025/, BACKUP/ | 레거시 백업, 활성 코드 아님 |

## 6. 오류 처리 전략

| 카테고리 | NULL일 때 | 근거 |
|:--------:|---------|-----------|
| A | Get_Environment() 반환 -8를 통한 프로세스 종료 | _FEP_DIV는 모든 프로세스에 중요 |
| B | `""` (빈 문자열)로 폴백 | host_name은 중요하지 않은 분기 제어 |
| C | Log(SYS_FATAL) + return | 암호화 설정은 중요, 계속할 수 없음 |
| D | printf + exit(1) | PX 독립형 유틸리티, Log 인프라 없음 |
| E-home | Log(SYS_FATAL) + return | 중요 경로 구성 |
| E-bin | N/A (캐시된 _FEP_BIN 사용) | 이미 Get_Environment()로 보호됨 |

## 7. 검증 기준

| ID | 기준 | 방법 |
|----|-----------|--------|
| V-01 | getenvironment.c에서 NULL 체크와 함께 `_FEP_DIV` 캐시됨 | 파일 읽기 |
| V-02 | fep_sub.h에서 `char *` 글로벌 + extern로 선언된 `_FEP_DIV` | 파일 읽기 |
| V-03 | 활성 sub/src/에서 `getenv("_FEP_DIV")` 제로 | `grep -r 'getenv.*_FEP_DIV'` BACK/BACKUP 제외 |
| V-04 | `getenv("FEP_DIV")` 제로 (버그 수정됨) | `grep -r 'getenv.*FEP_DIV'` |
| V-05 | 모든 `getenv("host_name")`은 NULL 가드 있음 | grep + 읽기 |
| V-06 | 모든 `getenv("INISAFENET_HOME")`은 NULL 가드 있음 | grep + 읽기 |
| V-07 | 모든 PX path getenv()은 NULL 가드 있음 | grep + 읽기 |
| V-08 | 모든 나머지 getenv()는 NULL 가드 있음 | grep + 읽기 |
| V-09 | `pa_9000_mp.c`는 `getenv("_P_BIN")` 대신 `_FEP_BIN` 사용 | grep |
| V-10 | 컴파일 오류 없음 (모든 파일이 올바른 변수 이름 사용) | 빌드 체크 |

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-22 | Initial design with 29 FR items in 8 batches | Claude Code |
