# Design: structured-logging

> FEP 로그 구조화 — 파이프 구분자 기반 구조화 포맷 구현 설계

## 1. 개요

### Plan 참조
- Plan: `docs/01-plan/features/structured-logging.plan.md`
- 목표: `Log()`, `Write_SLog()` 출력 포맷에 파이프 구분자 기반 구조화 모드 추가
- 범위: `sub/log_proc.c` 1개 파일 핵심 변경

### 포맷 비교

**기존 포맷 (legacy):**
```
[pb_1101_ts      ,09:30:15.123456,TCP,INFO]OK22 W[5] R[3]
```

**구조화 포맷 (structured):**
```
2026-03-01 09:30:15.123456|pb_1101_ts|TCP|INFO|0600|OK22 W[5] R[3]
```

## 2. 현행 코드 분석

### 2.1 Log() 함수 흐름 (line 34~226)

```
Log(p_err_no, p_fmt, ...)
  ├── vsprintf(mbuf, p_fmt, ap)          [line 52-58]
  ├── gettimeofday(&tv, NULL)            [line 64]
  ├── localtime_r → date                 [line 65]
  ├── sprintf(error_cd, "%04d", p_err_no) [line 67]
  ├── strftime(sys_date, "%Y%m%d", date) [line 79]
  ├── err_type  ← p_err_no / 100        [line 106-136]
  ├── err_level ← p_err_no % 100        [line 141-161]
  ├── snprintf(msg, ..., normal format)  [line 164] ← 변경 지점 A
  ├── Log_Proc(log_file_name, msg)       [line 196]
  ├── if (INFO) return                   [line 198]
  ├── snprintf(msg, ..., emergency fmt)  [line 216] ← 변경 지점 B
  └── Log_Emergency(log_file_name, msg)  [line 223]
```

### 2.2 Write_SLog() 함수 흐름 (line 432~583)

```
Write_SLog(p_msg)
  ├── hd = (SHM_LOG_HEAD *)p_msg         [line 453]
  ├── len = AtoIf(hd->Length, 4)         [line 458]
  ├── err_no = AtoIf(hd->ErrCd, 4)      [line 459]
  ├── gettimeofday → date (sys_date용)   [line 462-464]
  ├── err_type  ← err_no / 100          [line 466-498]
  ├── err_level ← err_no % 100          [line 502-521]
  ├── sprintf(msg, ..., normal format)   [line 523] ← 변경 지점 C
  ├── Log_Proc(log_file_name, msg)       [line 553]
  ├── if (INFO) return                   [line 555]
  ├── snprintf(msg, ..., emergency fmt)  [line 573] ← 변경 지점 D
  └── Log_Emergency(log_file_name, msg)  [line 580]
```

### 2.3 기존 변수 활용 가능 목록

| 변수 | 위치 | 값 | 구조화 필드 |
|------|------|-----|-------------|
| `sys_date` | Log:79, WSLog:464 | `"20260301"` (8자) | DATETIME 날짜 부분 |
| `date->tm_hour/min/sec` | Log:65 | 시분초 | DATETIME 시간 부분 |
| `tv.tv_usec` | Log:64 | 마이크로초 | DATETIME 마이크로초 |
| `hd->Time` | WSLog:453 | `"093015123456"` (12자) | DATETIME 시간 (WSLog) |
| `_Exe_Name` | extern | 프로세스명 | PROCESS (Log) |
| `hd->LogName` | WSLog:453 | 프로세스명 (10자) | PROCESS (WSLog) |
| `err_type` | Log:104, WSLog:460 | `"TCP"` 등 3자 | TYPE |
| `err_level` | Log:138, WSLog:501 | `"INFO"` 등 4자 | LEVEL |
| `error_cd` | Log:67 | `"0600"` 4자 | ERRCODE (Log) |
| `hd->ErrCd` | WSLog:453 | `"0600"` 4자 (char[4]) | ERRCODE (WSLog) |

**핵심**: 구조화 포맷에 필요한 모든 데이터가 이미 기존 변수에 존재. 추가 데이터 소스 불필요.

## 3. 구현 설계

### 3.1 모드 전환 (FR-05)

`log_proc.c` 상단 (line 19 이후)에 추가:

```c
static int log_structured = -1;  /* -1: uninitialized, 0: legacy, 1: structured */

static int is_structured_log(void)
{
    if (log_structured < 0) {
        char *env = getenv("FEP_LOG_FORMAT");
        log_structured = (env && strcmp(env, "structured") == 0) ? 1 : 0;
    }
    return log_structured;
}
```

**설계 근거:**
- `static` 변수: 프로세스 당 1회만 `getenv()` 호출 (이후 캐시)
- `-1` 초기값: 미초기화 상태 구분 (0이 기본값이 되어야 하므로)
- `strcmp` 정확 매치: `"structured"` 외의 모든 값은 legacy 모드
- `#include <string.h>`: 이미 `fep_sub.h`를 통해 포함됨

### 3.2 Log() 일반 출력 구조화 (FR-01, FR-03, FR-04)

**변경 지점 A** — `log_proc.c:164`

현재:
```c
snprintf(msg, sizeof(msg), "[%-15.15s,%02d:%02d:%02d.%06ld,%s,%s]%.*s\n",
        _Exe_Name, date->tm_hour, date->tm_min, date->tm_sec,
        tv.tv_usec, err_type, err_level,
        (int)strlen(buf), buf);
```

변경:
```c
if (is_structured_log()) {
    snprintf(msg, sizeof(msg),
            "%.4s-%.2s-%.2s %02d:%02d:%02d.%06ld|%s|%s|%s|%s|%.*s\n",
            sys_date, sys_date+4, sys_date+6,
            date->tm_hour, date->tm_min, date->tm_sec, tv.tv_usec,
            _Exe_Name, err_type, err_level, error_cd,
            (int)strlen(buf), buf);
} else {
    snprintf(msg, sizeof(msg), "[%-15.15s,%02d:%02d:%02d.%06ld,%s,%s]%.*s\n",
            _Exe_Name, date->tm_hour, date->tm_min, date->tm_sec,
            tv.tv_usec, err_type, err_level,
            (int)strlen(buf), buf);
}
```

**필드 매핑:**

| # | 필드 | 포맷 | 소스 | 예시 |
|---|------|------|------|------|
| 1 | DATETIME | `%.4s-%.2s-%.2s %02d:%02d:%02d.%06ld` | `sys_date` + `date->tm_*` + `tv.tv_usec` | `2026-03-01 09:30:15.123456` |
| 2 | PROCESS | `%s` | `_Exe_Name` | `pb_1101_ts` |
| 3 | TYPE | `%s` | `err_type` | `TCP` |
| 4 | LEVEL | `%s` | `err_level` | `INFO` |
| 5 | ERRCODE | `%s` | `error_cd` | `0600` |
| 6 | MSG | `%.*s` | `buf` | `OK22 W[5] R[3]` |

**날짜 포맷 설계 (FR-03):**
- `sys_date`는 `strftime "%Y%m%d"` 결과 (`"20260301"`)
- `%.4s-%.2s-%.2s`로 `YYYY-MM-DD` 변환 — 추가 버퍼 불필요
- ISO 8601 호환 날짜 형식

### 3.3 Log() emergency 출력 구조화 (FR-06)

**변경 지점 B** — `log_proc.c:216`

현재:
```c
snprintf(msg, sizeof(msg), "%s\033[3%dm[%-15.15s,%02d:%02d:%02d.%06ld,%s,%s]%.*s\033[0m\n",
        err_beep, p_err_no % 100, _Exe_Name, date->tm_hour, date->tm_min,
        date->tm_sec, tv.tv_usec, err_type, err_level,
        (int)strlen(buf), buf);
```

변경:
```c
if (is_structured_log()) {
    snprintf(msg, sizeof(msg),
            "%.4s-%.2s-%.2s %02d:%02d:%02d.%06ld|%s|%s|%s|%s|%.*s\n",
            sys_date, sys_date+4, sys_date+6,
            date->tm_hour, date->tm_min, date->tm_sec, tv.tv_usec,
            _Exe_Name, err_type, err_level, error_cd,
            (int)strlen(buf), buf);
} else {
    snprintf(msg, sizeof(msg), "%s\033[3%dm[%-15.15s,%02d:%02d:%02d.%06ld,%s,%s]%.*s\033[0m\n",
            err_beep, p_err_no % 100, _Exe_Name, date->tm_hour, date->tm_min,
            date->tm_sec, tv.tv_usec, err_type, err_level,
            (int)strlen(buf), buf);
}
```

**구조화 모드에서 제거되는 것:**
- `err_beep` (BEL 문자 `\007`) — 자동화 파싱 시 불필요
- `\033[3%dm ... \033[0m` (ANSI 컬러) — 파일 파싱 시 방해
- `p_err_no % 100` (컬러 인덱스) — 구조화 포맷의 LEVEL 필드로 대체

### 3.4 Write_SLog() 일반 출력 구조화 (FR-02, FR-03, FR-04)

**변경 지점 C** — `log_proc.c:523`

현재:
```c
sprintf(msg, "[%-10.10s     ,%.2s:%.2s:%.2s.%.6s,%s,%s]%.*s\n",
        hd->LogName, hd->Time, hd->Time+2, hd->Time+4, hd->Time+6, err_type,
        err_level, (int)(len - SHM_LOG_HEAD_SIZE), &p_msg[SHM_LOG_HEAD_SIZE]);
```

변경:
```c
if (is_structured_log()) {
    snprintf(msg, sizeof(msg),
            "%.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.6s|%.10s|%s|%s|%.4s|%.*s\n",
            sys_date, sys_date+4, sys_date+6,
            hd->Time, hd->Time+2, hd->Time+4, hd->Time+6,
            hd->LogName, err_type, err_level, hd->ErrCd,
            (int)(len - SHM_LOG_HEAD_SIZE), &p_msg[SHM_LOG_HEAD_SIZE]);
} else {
    sprintf(msg, "[%-10.10s     ,%.2s:%.2s:%.2s.%.6s,%s,%s]%.*s\n",
            hd->LogName, hd->Time, hd->Time+2, hd->Time+4, hd->Time+6, err_type,
            err_level, (int)(len - SHM_LOG_HEAD_SIZE), &p_msg[SHM_LOG_HEAD_SIZE]);
}
```

**Write_SLog 고유 차이점:**
- 시간: `hd->Time` (char[12], `"093015123456"`) — `%.2s` 포인터 산술로 추출
- 날짜: `sys_date` (Write_SLog 자체 계산, line 464) — SHM에는 날짜 없음
- 프로세스명: `hd->LogName` (char[10]) — `%.10s`로 패딩 없이 출력
- 에러코드: `hd->ErrCd` (char[4]) — `%.4s` (null 종단 아님, 길이 지정 필수)
- `sprintf` → `snprintf`: 구조화 분기에서는 `snprintf(msg, sizeof(msg), ...)` 사용

### 3.5 Write_SLog() emergency 출력 구조화 (FR-06)

**변경 지점 D** — `log_proc.c:573`

현재:
```c
snprintf(msg, sizeof(msg),
        "%s\033[3%dm[%-10.10s     ,%.2s:%.2s:%.2s.%.6s,%s,%s]%.*s\033[0m\n",
        err_beep, err_no % 100, hd->LogName, hd->Time, hd->Time+2, hd->Time+4,
        hd->Time+6, err_type, err_level,
        (int)(len - SHM_LOG_HEAD_SIZE), &p_msg[SHM_LOG_HEAD_SIZE]);
```

변경:
```c
if (is_structured_log()) {
    snprintf(msg, sizeof(msg),
            "%.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.6s|%.10s|%s|%s|%.4s|%.*s\n",
            sys_date, sys_date+4, sys_date+6,
            hd->Time, hd->Time+2, hd->Time+4, hd->Time+6,
            hd->LogName, err_type, err_level, hd->ErrCd,
            (int)(len - SHM_LOG_HEAD_SIZE), &p_msg[SHM_LOG_HEAD_SIZE]);
} else {
    snprintf(msg, sizeof(msg),
            "%s\033[3%dm[%-10.10s     ,%.2s:%.2s:%.2s.%.6s,%s,%s]%.*s\033[0m\n",
            err_beep, err_no % 100, hd->LogName, hd->Time, hd->Time+2, hd->Time+4,
            hd->Time+6, err_type, err_level,
            (int)(len - SHM_LOG_HEAD_SIZE), &p_msg[SHM_LOG_HEAD_SIZE]);
}
```

## 4. 구현 순서

| 순서 | 작업 | 라인 | 의존 |
|------|------|------|------|
| 1 | `is_structured_log()` 함수 + 정적 변수 추가 | 19 이후 | 없음 |
| 2 | Log() 일반 출력 분기 (변경 지점 A) | 164 | 1 |
| 3 | Log() emergency 출력 분기 (변경 지점 B) | 216 | 1 |
| 4 | Write_SLog() 일반 출력 분기 (변경 지점 C) | 523 | 1 |
| 5 | Write_SLog() emergency 출력 분기 (변경 지점 D) | 573 | 1 |
| 6 | `pkg_env.sh`에 환경변수 주석 추가 | - | 없음 |

## 5. FR-설계 추적 매트릭스

| FR | 설계 섹션 | 변경 지점 |
|----|-----------|-----------|
| FR-01: Log() 구조화 출력 | §3.2 | A (line 164) |
| FR-02: Write_SLog() 구조화 출력 | §3.4 | C (line 523) |
| FR-03: 날짜 필드 추가 | §3.2, §3.4 | A, C |
| FR-04: 에러코드 숫자 필드 | §3.2, §3.4 | A, C |
| FR-05: 환경변수 모드 전환 | §3.1 | 신규 추가 (line 19+) |
| FR-06: emergency 로그 구조화 | §3.3, §3.5 | B (line 216), D (line 573) |

## 6. 변경하지 않는 것

- `SLog()` 함수: SHM 바이너리 레코드 포맷 불변 (변경 시 전체 프로세스 재배포 필요)
- `Log_Proc()` 함수: 파일 쓰기 로직 (open/write/close) 불변
- `Log_Emergency()` 함수: emergency 파일 쓰기 로직 불변
- `Log_Save()` 함수: 60MB 로테이션 로직 불변
- `LOG_HEAD_SIZE` (42): legacy 모드에서 계속 사용
- `SHM_LOG_HEAD_SIZE` (30): SHM 레코드 파싱에 계속 사용
- 호출자 2,010곳: `Log(err_no, fmt, ...)` 시그니처 변경 없음

## 7. 출력 예시

### Log() 출력 비교

```
# Legacy (FEP_LOG_FORMAT 미설정)
[pb_1101_ts      ,09:30:15.123456,TCP,INFO]OK22 W[5] R[3]

# Structured (FEP_LOG_FORMAT=structured)
2026-03-01 09:30:15.123456|pb_1101_ts|TCP|INFO|0600|OK22 W[5] R[3]
```

### Write_SLog() 출력 비교

```
# Legacy
[pb_1101_ts ,09:30:15.123456,TCP,INFO]OK22 W[5] R[3]

# Structured
2026-03-01 09:30:15.123456|pb_1101_ts|TCP|INFO|0600|OK22 W[5] R[3]
```

### Emergency 로그 비교

```
# Legacy (pz_emergency)
^G\033[30m[pb_1101_ts      ,09:30:15.123456,TCP,FATL]Connection lost\033[0m

# Structured (pz_emergency)
2026-03-01 09:30:15.123456|pb_1101_ts|TCP|FATL|0601|Connection lost
```

### 파싱 예시

```bash
# 에러만 필터링
grep '|FATL\||EROR|' logfile

# 필드별 추출 (TYPE, LEVEL, MSG)
awk -F'|' '{print $3, $4, $6}' logfile

# 에러코드별 집계
grep -v '|INFO|' logfile | cut -d'|' -f5 | sort | uniq -c | sort -rn

# 특정 프로세스 로그
grep '|pb_1101_ts|' logfile
```

## 8. 위험 관리

| 위험 | 완화 |
|------|------|
| MSG 본문에 `\|` 포함 시 파싱 오류 | MSG가 마지막 필드 → `awk -F'\|'` 시 $6 이후 전부 합치면 됨. 실제 FEP 로그에 파이프 문자 사용 사례 없음 |
| `getenv()` thread safety | FEP는 멀티프로세스(fork) 구조, 멀티스레드 아님. 각 프로세스에서 독립 호출 |
| 구조화 헤더 길이 증가 | LOG_SIZE(5120)에 비해 헤더 ~50바이트 — 충분한 여유 |
| `sys_date` 가용성 | Log: line 79에서 이미 계산. Write_SLog: line 464에서 이미 계산. 추가 계산 불필요 |
