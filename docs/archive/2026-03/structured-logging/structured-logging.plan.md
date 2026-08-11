# Plan: structured-logging

> FEP 로그 구조화 — §3.7 로그 구조화 부재 해결

## 1. 개요

### 배경
FEP 로그 시스템(`sub/log_proc.c`)은 고정 헤더 + 자유 형식 메시지로 구성됨.
헤더(`[process,time,type,level]`)는 구조화되어 있으나, 메시지 본문이 자유 형식이라
로그 파싱/모니터링 자동화가 어려움.

### 목표
- 기존 `Log()` API(호출 시그니처) 변경 없이 로그 출력 포맷 구조화
- 기존 포맷과의 완전한 하위 호환성 유지 (기본값 = 기존 포맷)
- 환경변수/설정으로 구조화 모드 ON/OFF 전환

### 범위
- `sub/log_proc.c` 1개 파일 수정 (핵심)
- `inc/fep_sub.h` 또는 `inc/def_error.h` 매크로 추가 (선택)
- 호출자 104개 파일, 2,010 호출 — 변경 없음 (API 호환)

## 2. 현황 분석

### 현재 로그 포맷
```
[pb_1101_ts    ,09:30:15.123456,TCP,INFO]OK22 W[5] R[3]
```

헤더 구조 (LOG_HEAD_SIZE = 42 bytes):
- `[%-15.15s` — 프로세스명 (15자)
- `,%02d:%02d:%02d.%06ld` — 시간 (HH:MM:SS.usec)
- `,%s` — 타입 (USR/SYS/PRO/SAM/FIF/ORA/TCP/UDP/DSH)
- `,%s]` — 레벨 (INFO/FATL/DBUG/WARN/EROR)
- 이후 자유 형식 메시지

### 에러코드 체계 (def_error.h)
```
code = type * 100 + level
type: USR(0) SYS(1) PRO(2) SAM(3) FIF(4) ORA(5) TCP(6) UDP(7) DSH(8)
level: OK(00) FATAL(01) DEBUG(02) WARN(03) ERROR(05)
```

### 관련 함수
| 함수 | 역할 | 출력 대상 |
|------|------|-----------|
| `Log()` | 메인 로그 | 파일 + emergency |
| `SLog()` | SHM 로그 | 공유메모리 링버퍼 |
| `Write_SLog()` | SHM→파일 | 파일 + emergency |
| `Log_Proc()` | 파일 쓰기 | 개별 프로세스 로그 |
| `Log_Emergency()` | 긴급 로그 | pz_emergency 통합 |
| `Log_Save()` | 크기 관리 | 60MB 초과 시 백업 |

### 호출 규모
- **2,010회** across 104 files (src/ 전체)
- sub/ 내부에서도 추가 호출 존재

## 3. 구조화 방식

### 3.1 선택: 구분자 기반 구조화 (Delimited Structured Format)

JSON은 C89에서 이스케이프 처리가 복잡하고 오버헤드가 큼.
대신 파이프(`|`) 구분자 기반의 고정 필드 포맷을 채택:

**구조화 포맷:**
```
DATETIME|PROCESS|TYPE|LEVEL|ERRCODE|MSG
2026-03-01 09:30:15.123456|pb_1101_ts|TCP|INFO|0600|OK22 W[5] R[3]
```

**장점:**
- C89 완전 호환 (snprintf만으로 구현)
- 기존 `awk -F'|'`, `cut -d'|'` 등으로 즉시 파싱 가능
- grep/sort/uniq 등 기존 도구 활용 유지
- 헤더 크기 예측 가능 (고정 길이 필드)
- 성능 오버헤드 거의 없음 (snprintf 포맷 문자열만 변경)

### 3.2 대안 비교

| 방식 | C89 호환 | 파싱 용이 | 성능 | 구현 난이도 |
|------|----------|-----------|------|-------------|
| **파이프 구분자** | ✅ | ✅ | ✅ | 낮음 |
| JSON | ❌ (이스케이프) | ✅ | ❌ | 높음 |
| Key=Value | ✅ | ⚠️ | ✅ | 중간 |
| 기존 유지 | ✅ | ❌ | ✅ | 없음 |

### 3.3 모드 전환 메커니즘

```c
/* log_proc.c 상단 */
static int log_structured = -1;  /* -1: 미초기화, 0: legacy, 1: structured */

static int is_structured_log(void)
{
    if (log_structured < 0) {
        char *env = getenv("FEP_LOG_FORMAT");
        log_structured = (env && strcmp(env, "structured") == 0) ? 1 : 0;
    }
    return log_structured;
}
```

- `FEP_LOG_FORMAT=structured` → 구조화 모드
- 미설정 또는 다른 값 → 기존 포맷 (기본값)
- 프로세스 시작 시 1회 평가, 이후 캐시

## 4. 구현 요구사항 (FR)

### FR-01: Log() 구조화 출력
- `is_structured_log()` 분기 추가
- 구조화 모드: `snprintf(msg, ...)` 포맷 문자열을 파이프 구분자로 변경
- 기존 모드: 현재 포맷 유지
- emergency 로그도 동일 적용 (ANSI 컬러는 구조화 모드에서 제거)

### FR-02: Write_SLog() 구조화 출력
- SHM 로그 → 파일 쓰기 시 동일 구조화 포맷 적용
- SHM 내부 포맷(SLog)은 변경 없음 (바이너리 고정 레코드)

### FR-03: 날짜 필드 추가
- 기존: 시간만 (`09:30:15.123456`)
- 구조화: 날짜+시간 (`2026-03-01 09:30:15.123456`)
- 일자 변경 시 로그 파일이 분리되지만, 로그 단독으로도 날짜 식별 가능해야 함

### FR-04: 에러코드 숫자 필드 추가
- 기존: TYPE/LEVEL만 출력 (문자열)
- 구조화: 원본 에러코드 숫자도 포함 (`0600` = TCP_OK)
- 정확한 에러코드 기반 필터링/집계 가능

### FR-05: 환경변수 기반 모드 전환
- `FEP_LOG_FORMAT` 환경변수로 제어
- `pkg_env.sh`에 주석으로 사용법 문서화
- 기본값: 기존 포맷 (무설정 시)

### FR-06: emergency 로그 구조화
- 구조화 모드에서 ANSI 이스케이프 시퀀스 제거
- 파이프 구분자 포맷으로 통일
- `pz_emergency` 파일도 자동화 파싱 가능

## 5. 변경 범위

| 파일 | 변경 내용 | 영향도 |
|------|-----------|--------|
| `sub/log_proc.c` | Log(), Write_SLog() 포맷 분기 | 핵심 |
| `inc/fep_sub.h` | LOG_STRUCTURED_HEAD_SIZE 매크로 (선택) | 부수 |
| `env/pkg_env.sh` | FEP_LOG_FORMAT 환경변수 주석 | 문서 |

### 변경하지 않는 것
- `SLog()` 내부 포맷 (SHM 바이너리 레코드 — 변경 시 전체 프로세스 재배포 필요)
- `Log()` 함수 시그니처 (호출자 2,010곳 변경 불필요)
- `Log_Proc()`, `Log_Emergency()`, `Log_Save()` — 파일 I/O 로직 불변
- 로그 파일 경로/디렉토리 구조

## 6. 위험 요소

| 위험 | 영향 | 완화 |
|------|------|------|
| 기존 모니터링 스크립트 호환성 | 높음 | 기본값을 기존 포맷으로 유지 |
| 메시지 본문에 파이프 문자 포함 시 | 중간 | MSG 필드가 마지막이므로 영향 없음 |
| LOG_HEAD_SIZE 상수 의존 코드 | 낮음 | LOG_HEAD_SIZE는 기존 모드에서만 사용 |
| SLog 레코드 크기 변경 | 높음 | SLog 내부 포맷은 변경하지 않음 |

## 7. 테스트 방법

1. `FEP_LOG_FORMAT` 미설정 → 기존 포맷 출력 확인
2. `FEP_LOG_FORMAT=structured` → 구조화 포맷 출력 확인
3. emergency 로그 (`pz_emergency`) 포맷 확인
4. `awk -F'|' '{print $3, $4, $6}'` 등으로 파싱 검증
5. 로그 크기 비교 (60MB 제한 동작 확인)
6. 날짜 변경 시 로그 분리 동작 확인

## 8. 예상 효과

- 로그 자동 파싱/모니터링 가능 (`awk -F'|'`)
- 에러코드 기반 집계 (`grep '|FATL|' | cut -d'|' -f5 | sort | uniq -c`)
- 날짜+시간 포함으로 로그 단독 분석 가능
- 기존 시스템 무중단 전환 (환경변수 토글)
