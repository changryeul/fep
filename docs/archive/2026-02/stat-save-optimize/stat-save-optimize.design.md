# stat-save-optimize Design Document

> **Summary**: Stat_Save() 파일 I/O 시간 기반 스로틀링 상세 설계
>
> **Plan Reference**: `docs/01-plan/features/stat-save-optimize.plan.md`
> **Date**: 2026-02-27
> **Status**: Draft

---

## 1. Design Overview

### 1.1 접근 방식

`Stat_Save()` 함수 내부에 file-scope static 타임스탬프를 두어, 마지막 파일 기록 이후 `STAT_SAVE_INTERVAL_SEC`초가 경과하지 않았으면 파일 I/O를 건너뛴다.

- **영업시간 종료 판정**은 매 호출 실행 (스로틀 대상 아님)
- **파일 I/O**만 스로틀 대상
- **호출자 변경 0건** — 함수 시그니처 불변

### 1.2 변경 파일 목록

| # | File | Change Type | Lines |
|---|------|-------------|-------|
| 1 | `sub/stat_save.c` | 수정 | ~15줄 추가 |
| 2 | `inc/fep_sub.h` | 수정 | ~2줄 추가 |
| 3 | `sub/setsigfatal.c` | 수정 | 1줄 변경 |

---

## 2. Implementation Order

```
Step 1: inc/fep_sub.h — 매크로 + extern 선언
Step 2: sub/stat_save.c — 스로틀 로직 + Stat_Save_Force()
Step 3: sub/setsigfatal.c — Stat_Save() → Stat_Save_Force()
```

---

## 3. Detailed Design per FR

### FR-01: 시간 기반 스로틀링 (stat_save.c)

**현재 코드** (`sub/stat_save.c:62-63`):
```c
	}                                       /* end of business-hours check */
	sprintf (bumun, "%s", _SubSystem_Name); /* file I/O starts here */
```

**변경**: 영업시간 판정(line 62) 직후, 파일 I/O(line 63) 직전에 스로틀 검사를 삽입.

**삽입할 코드**:
```c
	}
	/* FR-01: throttle file I/O — skip if interval not elapsed */
	if (tv.tv_sec - _last_save_time < STAT_SAVE_INTERVAL_SEC)
		return;
	_last_save_time = tv.tv_sec;

	sprintf (bumun, "%s", _SubSystem_Name);
```

**핵심 사항**:
- `tv`는 line 31의 `gettimeofday(&tv, NULL)`에서 이미 채워진 상태
- 추가 syscall 없음 — `tv.tv_sec` 재사용
- `time_t` 비교이므로 초 단위 정밀도 (충분)

### FR-02: 영업시간 판정 매 호출 유지

**변경 없음**. 현재 코드 lines 31-61의 영업시간 판정 + `Exit_Process()` 로직은 스로틀 검사(FR-01) 이전에 위치하므로 자동으로 매 호출 실행된다.

코드 흐름:
```
Stat_Save() 진입
  ├── lines 31-34: gettimeofday + localtime_r (항상)
  ├── lines 36-39: AtoIf × 4 — 기동/종료 시간 파싱 (항상)
  ├── lines 41-61: 영업시간 판정 → Exit_Process() (항상)
  ├── [NEW] FR-01 스로틀 검사 → 미달 시 return
  └── lines 63-137: 파일 I/O (스로틀 통과 시에만)
```

### FR-03: STAT_SAVE_INTERVAL_SEC 매크로 (fep_sub.h)

**위치**: `inc/fep_sub.h` — 기존 `Stat_Save` extern 선언(line 359) 근처.

**추가할 코드**:
```c
#ifndef STAT_SAVE_INTERVAL_SEC
#define STAT_SAVE_INTERVAL_SEC  3
#endif
```

`#ifndef` 가드로 감싸서, 빌드 시 `-DSTAT_SAVE_INTERVAL_SEC=5`처럼 오버라이드 가능.

### FR-04: 프로세스 종료 시 강제 기록 (Stat_Save_Force)

**배경 분석** — `setsigfatal.c`의 `Stat_Save()` 호출 위치:

```c
/* setsigfatal.c:130-148 — Exit_Process() 내부 */
if (!_in_signal_handler)                    /* 시그널 핸들러가 아닌 경우만 */
{
    ...
    if (PROC(D_K,P_K).process_status == 4)
        PROC(D_K,P_K).process_status = 2;
    else if (PROC(D_K,P_K).process_status == 8)
        PROC(D_K,P_K).process_status = 9;
    else
        Stat_Save ();                       /* ← 정상 종료 경로 (line 143) */
    ...
    Log (PRO_OK, "process STOP");
    exit (FAIL);
}
```

**핵심**: 이 `Stat_Save()` 호출은:
1. 시그널 핸들러가 아닌 **정상 종료 경로**에서만 실행 (`!_in_signal_handler`)
2. 프로세스가 `exit()` 직전이므로 **마지막 상태를 반드시 기록**해야 함
3. `process_status`가 4 또는 8이 아닌 경우에만 도달 (재귀 방지)

**설계**: `stat_save.c`에 `Stat_Save_Force()` 추가.

```c
/* stat_save.c — file-scope static */
static time_t _last_save_time = 0;

/*----------------------------------------------------------------------*/
void	Stat_Save_Force (void)
/*----------------------------------------------------------------------*/
{
	_last_save_time = 0;
	Stat_Save ();
}	/* End of Stat_Save_Force () */
```

`_last_save_time`을 0으로 리셋하면, 이어지는 `Stat_Save()` 호출에서 스로틀 조건(`tv.tv_sec - 0 < 3`)이 거짓이 되어 파일 I/O가 실행된다.

**setsigfatal.c 변경** (line 143):
```c
/* Before */
        Stat_Save ();
/* After */
        Stat_Save_Force ();
```

### FR-05: static time_t _last_save_time (stat_save.c)

**위치**: `sub/stat_save.c` — 함수 외부(file scope)에 선언.

```c
#include	"fep_sub.h"

/* FR-05: last file write timestamp for throttling */
static time_t _last_save_time = 0;
```

**file-scope static 선택 이유**: `Stat_Save_Force()`에서 접근해야 하므로 function-scope static(함수 내부)으로는 불가. file-scope static이면 같은 파일의 두 함수 모두 접근 가능하면서도 외부 파일에는 비노출.

---

## 4. Exact Code Changes

### 4.1 `inc/fep_sub.h` — 2줄 추가

**위치**: line 359 (`extern void Stat_Save`) 부근에 삽입.

```
Before (lines 358-360):

extern void		Stat_Save (void);
extern  int		Seq_Save (char *, int, int);

After:

#ifndef STAT_SAVE_INTERVAL_SEC
#define STAT_SAVE_INTERVAL_SEC  3
#endif
extern void		Stat_Save (void);
extern void		Stat_Save_Force (void);
extern  int		Seq_Save (char *, int, int);
```

### 4.2 `sub/stat_save.c` — 스로틀 로직 + Force 함수

**변경 A**: file-scope static 변수 추가 (line 9 이후)

```
Before (lines 9-10):

#include	"fep_sub.h"

After:

#include	"fep_sub.h"

/* throttle: last file write timestamp (FR-05) */
static time_t _last_save_time = 0;
```

**변경 B**: 스로틀 검사 삽입 (line 62 이후, line 63 이전)

```
Before (lines 61-63):

		Exit_Process ();
		}
	}
	sprintf (bumun, "%s", _SubSystem_Name);

After:

		Exit_Process ();
		}
	}

	/* FR-01: throttle — skip file I/O if interval not elapsed */
	if (tv.tv_sec - _last_save_time < STAT_SAVE_INTERVAL_SEC)
		return;
	_last_save_time = tv.tv_sec;

	sprintf (bumun, "%s", _SubSystem_Name);
```

**변경 C**: `Stat_Save_Force()` 함수 추가 (파일 끝, `End of Stat_Save()` 뒤)

```
Before (lines 140-144):

}	/* End of Stat_Save ()	*/

/*************************************************************************
	End of Program (stat_save.c)
*************************************************************************/

After:

}	/* End of Stat_Save ()	*/

/*----------------------------------------------------------------------*/
void	Stat_Save_Force (void)
/*----------------------------------------------------------------------*/
{
	_last_save_time = 0;
	Stat_Save ();
}	/* End of Stat_Save_Force ()	*/

/*************************************************************************
	End of Program (stat_save.c)
*************************************************************************/
```

### 4.3 `sub/setsigfatal.c` — 1줄 변경

**위치**: line 143

```
Before (line 143):

			Stat_Save ();

After:

			Stat_Save_Force ();
```

---

## 5. Verification Checklist

| # | 검증 항목 | 확인 방법 |
|---|-----------|----------|
| V-01 | `STAT_SAVE_INTERVAL_SEC` 매크로가 `fep_sub.h`에 정의됨 | grep 확인 |
| V-02 | `Stat_Save_Force` extern이 `fep_sub.h`에 선언됨 | grep 확인 |
| V-03 | `_last_save_time`이 file-scope static으로 `stat_save.c`에 선언됨 | 코드 확인 |
| V-04 | 스로틀 검사가 영업시간 판정(lines 31-61) 이후에 위치 | 코드 순서 확인 |
| V-05 | 스로틀 검사가 파일 I/O(line 63 이후) 이전에 위치 | 코드 순서 확인 |
| V-06 | `tv.tv_sec` 재사용 (추가 syscall 없음) | 변수 출처 확인 |
| V-07 | `Stat_Save_Force()`가 `_last_save_time = 0` 후 `Stat_Save()` 호출 | 코드 확인 |
| V-08 | `setsigfatal.c:143`이 `Stat_Save_Force()` 호출 | 코드 확인 |
| V-09 | 기존 호출자 58건 변경 없음 | grep `Stat_Save` — setsigfatal.c 외에 변경 없음 |
| V-10 | 빌드 성공 (`mk.sh sub`) | 빌드 실행 |

---

## 6. Edge Cases

### 6.1 첫 호출 (last_save_time == 0)

`_last_save_time` 초기값이 0이므로, 첫 호출 시 `tv.tv_sec - 0`는 항상 `>= STAT_SAVE_INTERVAL_SEC`. 따라서 프로세스 시작 시 첫 번째 Stat_Save()는 즉시 파일 기록. 정상.

### 6.2 Stat_Save → Exit_Process → Stat_Save_Force 경로

영업시간 종료 시:
1. `Stat_Save()` line 56: `process_status = 4` 설정
2. `Stat_Save()` line 60: `Exit_Process()` 호출
3. `Exit_Process()` line 138: `process_status == 4` → `process_status = 2` 설정
4. line 143의 `else` 분기에 도달하지 않음 → `Stat_Save_Force()` 호출 안 됨

결론: **재귀 없음**. 영업시간 종료 시 `process_status == 4`이면 line 138에서 처리되어 `Stat_Save_Force()`에 도달하지 않음.

### 6.3 정상 종료 (kill PID, setpstat.sh 2)

프로세스가 SIGTERM 수신 시:
1. `End_Routine()` → `_in_signal_handler = 1`
2. `Exit_Process()` → line 130: `!_in_signal_handler` 거짓 → Stat_Save/Force 미호출
3. `_exit(FAIL)`

결론: 시그널 종료 시 Stat_Save 호출 없음 (signal-handler-safety 설계와 일치).

### 6.4 time_t overflow / 시스템 시간 역행

`tv.tv_sec - _last_save_time < 3`에서 시스템 시간이 역행하면 음수 → 조건 참 → 스로틀 유지. 최악의 경우 3초간 기록 안 됨. 시간 역행이 끝나면 자동 복구. 위험도 없음.

---

## 7. Build & Test

```sh
mk.sh sub          # libfepP.a 재빌드 (stat_save.o, setsigfatal.o 재컴파일)
mk.sh src          # 전체 바이너리 재링크 (libfepP.a 의존)
```

검증:
```sh
grep -n "STAT_SAVE_INTERVAL_SEC" st01/inc/fep_sub.h st01/sub/stat_save.c
grep -n "Stat_Save_Force" st01/inc/fep_sub.h st01/sub/stat_save.c st01/sub/setsigfatal.c
grep -n "_last_save_time" st01/sub/stat_save.c
```

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-27 | Initial design | Claude |
