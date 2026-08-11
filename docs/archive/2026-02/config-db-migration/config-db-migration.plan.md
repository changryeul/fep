# config-db-migration 계획 문서

> **요약**: FEP 설정값 DB화 - 10개 .ini 파일을 SQLite DB로 전환하고, 기존 SHM 로딩 구조를 유지하면서 중앙 관리/이력 추적 기능 추가
>
> **프로젝트**: FEP (Front-End Processor for KRX)
> **작성자**: FEP Dev Team
> **날짜**: 2026-02-18
> **상태**: 초안

---

## 1. 개요

### 1.1 목표

현재 FEP 시스템의 설정 관리를 `.ini` 파일 기반에서 DB 기반으로 전환하여:
- 설정값 중앙 집중 관리
- 변경 이력 자동 추적 (audit trail)
- 환경별(TEST/REAL1/REAL2) 설정 통합 관리
- 설정 오류 사전 검증 (DB 제약조건)
- 운영 편의성 향상 (원격 관리, 검색/조회)

### 1.2 배경

**현재 설정 관리 방식**:
```
pkg_env.sh (환경변수 7개)
    ↓
pz_memory_conf.c (10개 .ini 파일 개별 읽기)
    ↓
SHM (Shared Memory - 런타임 설정 저장소)
    ↓
모든 비즈니스 프로세스 (SHM 매크로로 접근)
```

**현재 문제점**:
1. **이력 관리 부재**: .ini 파일 변경 시 이전 설정값 복원 불가 (수동 백업만 존재)
2. **환경 동기화 어려움**: TEST/REAL1/REAL2 서버 간 설정 scp/rsync로 수동 동기화
3. **검증 부재**: .ini 파일 오타/잘못된 값 입력 시 사전 검증 불가 → 장애 유발
4. **보안 취약점**: proc.ini에 LogonID/LogonPW가 평문/base64로 저장
5. **분산 관리**: 서버별로 cfg/ 디렉토리에 개별 .ini 파일 관리 → 설정 불일치 위험
6. **운영 불편**: 설정 변경 시 SSH 접속 → vi 편집 → px_cfgload 실행 필요

**.ini 파일 목록 (10개)**:

| 파일 | 용도 | 항목 수 | 런타임 변경 |
|------|------|---------|------------|
| daemon.ini | 데몬 프로세스 정의 | ~15 | 거의 없음 |
| proc.ini | 프로세스별 속성 (18개) | ~180 | 타임아웃 등 가끔 |
| tcp1.ini | TCP 포트 데몬 설정 | ~22 | 거의 없음 |
| tcp2.ini | TCP 직접 접속 정보 (16개) | ~48 | IP 변경 시 |
| udpip.ini | UDP 멀티캐스트 설정 | ~6 | 거의 없음 |
| file.ini | SAM 파일 정의 (8개) | ~32 | 사이즈 변경 시 |
| sisetr.ini | 시세 TR 코드 정의 (29개) | ~116 | TR 추가 시 |
| dshm.ini | 데이터 SHM 정의 | 0 (비활성) | - |
| client.ini | 클라이언트 IP 화이트리스트 | 20 | 서버 추가 시 |
| pc.ini | 관리자 PC IP 리스트 | 7 | PC 변경 시 |

### 1.3 관련 문서

- 아키텍처 분석: `docs/FEP_Architecture_Analysis.md`
- 매크로 참조: `docs/FEP_Macro_Reference.md`
- TR 구조 리팩토링 설계: `docs/02-design/features/tr-struct-refactor.design.md`

---

## 2. 범위

### 2.1 포함 범위

- [x] SQLite DB 스키마 설계 (설정 테이블, 이력 테이블, IP 화이트리스트)
- [x] Config Loader 추상화 레이어 개발 (DB 우선 → .ini fallback)
- [x] 10개 .ini 파일 데이터를 DB로 마이그레이션
- [x] pz_memory_conf.c 리팩토링 (Config Loader 적용)
- [x] DB → .ini 동기화 함수 (fallback 최신화)
- [x] .ini → DB 초기 임포트 도구 (ini2db)
- [x] 변경 이력 자동 기록 (트리거)
- [x] 환경별(TEST/REAL1/REAL2) 설정 분리 관리

### 2.2 제외 범위

- 관리 UI/웹 인터페이스 (추후 과제)
- 환경변수 (pkg_env.sh) 변경 - OS 레벨 경로는 DB 이전 불가
- #define 상수 변경 - 컴파일 타임 결정값
- SHM 키 값 변경 - DB 접속 이전에 SHM 생성 필요
- 소스 내 하드코딩 IP 제거 (별도 리팩토링: ini로 먼저 이동)
- px_setXXX 운영도구 전면 개편 (기존 호환 유지)

---

## 3. 요구사항

### 3.1 기능 요구사항

| ID | 요구사항 | 우선순위 | 상태 |
|----|-------------|----------|--------|
| FR-01 | SQLite DB에 모든 .ini 설정값 저장 | High | Pending |
| FR-02 | 데몬 시작 시 DB에서 설정 1회 로드 → SHM 기록 | High | Pending |
| FR-03 | DB 접속 실패 시 .ini 파일 자동 fallback | High | Pending |
| FR-04 | DB 로드 성공 시 .ini 파일 자동 동기화(백업) | Medium | Pending |
| FR-05 | 설정 변경 시 이전 값 자동 이력 기록 | Medium | Pending |
| FR-06 | 환경별(TEST/REAL1/REAL2) 설정 분리 저장 | Medium | Pending |
| FR-07 | .ini → DB 초기 마이그레이션 도구 제공 | High | Pending |
| FR-08 | DB → .ini 역방향 내보내기 도구 제공 | Medium | Pending |
| FR-09 | 설정값 데이터 타입 검증 (IP, PORT, INT, STRING) | Medium | Pending |
| FR-10 | LogonPW 암호화 저장 (평문/base64 제거) | High | Pending |
| FR-11 | px_cfgload 호환 - 기존 운영 스크립트 동작 보장 | High | Pending |
| FR-12 | 다중 서버 환경에서 동일 DB 파일 사용 가능 | Low | Pending |

### 3.2 비기능 요구사항

| 분류 | 기준 | 측정 방법 |
|----------|----------|-------------------|
| 성능 | DB 로드 시간 < 100ms (전체 설정) | 시간 측정 로그 |
| 안정성 | DB 장애 시 5초 이내 .ini fallback 전환 | 장애 시뮬레이션 |
| 호환성 | 기존 SHM 구조 100% 유지 (비즈니스 프로세스 무변경) | 기존 프로세스 정상 동작 |
| 보안 | LogonPW AES-256 암호화 저장 | 암호문 검증 |
| 운영성 | 기존 px_cfgload/px_cfgback 정상 동작 | 운영 시나리오 테스트 |

---

## 4. 성공 기준

### 4.1 완료 정의

- [ ] SQLite DB 스키마 생성 및 초기 데이터 로드 완료
- [ ] Config Loader가 DB에서 정상 로드하여 SHM에 기록
- [ ] DB 장애 시 .ini fallback이 자동 동작
- [ ] 기존 모든 비즈니스 프로세스(PA/PB/PW/PX/PZ)가 무변경으로 정상 동작
- [ ] 설정 변경 이력이 DB에 자동 기록
- [ ] ini2db 마이그레이션 도구 동작 확인
- [ ] px_cfgload 호환성 확인

### 4.2 품질 기준

- [ ] 전체 설정 로드 시간 100ms 이내
- [ ] SHM 내 설정값이 기존 .ini 로드 결과와 byte-level 동일
- [ ] DB 파일 크기 1MB 이내
- [ ] 컴파일 경고(warning) 0건
- [ ] 기존 Makefile에 sqlite3 라이브러리 추가

---

## 5. 위험 및 완화

| 위험 | 영향 | 가능성 | 완화 |
|------|--------|------------|------------|
| DB 파일 손상 시 FEP 기동 불가 | **치명적** | Low | .ini fallback 자동 전환 + DB 파일 주기적 백업 |
| SQLite 라이브러리 의존성 추가 | Medium | Medium | 정적 링크 또는 시스템 패키지 활용 |
| 기존 px_cfgload 동작 변경 | High | Medium | 호환 레이어 - 기존 .ini 쓰기 + DB 동시 업데이트 |
| 다중 프로세스 동시 DB 접근 충돌 | Medium | Low | 설정은 읽기 전용, 쓰기는 단일 관리 프로세스만 |
| 거래 시간 중 잘못된 설정 변경 | **치명적** | Low | 변경 잠금 + 변경 전 검증 + 즉시 rollback 기능 |
| SHM 구조 불일치 (로딩 로직 오류) | High | Medium | byte-level 비교 검증 도구 제작 |
| 운영팀 학습 곡선 | Medium | High | 기존 도구 호환 유지 + 단계적 전환 |

---

## 6. 아키텍처 고려사항

### 6.1 프로젝트 레벨

| 레벨 | 선택 |
|-------|:--------:|
| **Starter** | - |
| **Dynamic** | **O** |
| **Enterprise** | - |

**선정 이유**: 기존 C 기반 다중 프로세스 시스템에 DB 계층 추가. 마이크로서비스 수준은 아니지만 단순 정적 구조 이상.

### 6.2 핵심 아키텍처 결정

| 결정 | 옵션 | 선택 | 근거 |
|----------|---------|----------|-----------|
| DB 엔진 | Oracle / PostgreSQL / SQLite | **SQLite** | 별도 서버 불필요, C 네이티브, 파일 기반, 읽기 전용 사용에 최적 |
| DB 위치 | 원격 DB 서버 / 로컬 파일 | **로컬 파일** | 네트워크 의존성 제거, .ini와 동일 위치 |
| 로딩 전략 | DB 직접 / DB→SHM / DB→.ini→SHM | **DB→SHM (직접)** | 기존 .ini→SHM과 동일 패턴, 중간 단계 제거 |
| Fallback | .ini 유지 / DB only | **.ini 유지 (하이브리드)** | 안정성 최우선 - 트레이딩 시스템 |
| 암호화 | 없음 / Base64 / AES | **AES-256** | LogonPW 보안 강화, KRX 보안 요건 충족 |
| 동기화 방향 | DB→.ini / .ini→DB / 양방향 | **DB→.ini (단방향)** | DB가 Master, .ini는 항상 DB의 복사본 |

### 6.3 시스템 아키텍처

```
┌─────────────────────────────────────────────────────────────┐
│                    Config Architecture (신규)                │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  [ini2db 도구]    [px_cfgload 호환]    [향후: 관리UI]       │
│       │                 │                    │              │
│       ▼                 ▼                    ▼              │
│  ┌──────────────────────────────────────────────────┐      │
│  │              fep_config.db (SQLite)               │      │
│  │  ┌────────────┐ ┌────────────┐ ┌───────────────┐ │      │
│  │  │ fep_config │ │ fep_config │ │ fep_ip_       │ │      │
│  │  │            │ │ _history   │ │ whitelist     │ │      │
│  │  └────────────┘ └────────────┘ └───────────────┘ │      │
│  └──────────────────────┬───────────────────────────┘      │
│                         │                                   │
│  ┌──────────────────────┴───────────────────────────┐      │
│  │           Config Loader (config_loader.c)         │      │
│  │  ┌─────────┐   ┌──────────┐   ┌──────────────┐  │      │
│  │  │ DB 로드 │──▶│ 검증     │──▶│ SHM 기록     │  │      │
│  │  │ (우선)  │   │ (타입/   │   │ (기존 구조   │  │      │
│  │  └────┬────┘   │  범위)   │   │  100% 유지)  │  │      │
│  │       │실패    └──────────┘   └──────────────┘  │      │
│  │       ▼                                          │      │
│  │  ┌─────────┐                                     │      │
│  │  │.ini 로드│ (fallback)                          │      │
│  │  │(기존코드)│                                     │      │
│  │  └─────────┘                                     │      │
│  └──────────────────────────────────────────────────┘      │
│                         │                                   │
│                         ▼                                   │
│              ┌─────────────────┐                           │
│              │      SHM        │ (기존 구조 100% 유지)     │
│              │  Shm_Mem[D_K]   │                           │
│              └────────┬────────┘                           │
│                       ▼                                     │
│         PA/PB/PW 비즈니스 프로세스 (변경 없음)              │
│         PROC(i,j), TCP2(i,j), UDPIP(i,j) 매크로 그대로    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 6.4 파일 구조

```
st01/
├── cfg/
│   ├── fep_config.db          (신규: SQLite DB 파일)
│   ├── daemon.ini             (유지: fallback용)
│   ├── proc.ini               (유지: fallback용)
│   ├── tcp1.ini               (유지: fallback용)
│   ├── tcp2.ini               (유지: fallback용)
│   ├── udpip.ini              (유지: fallback용)
│   ├── file.ini               (유지: fallback용)
│   ├── sisetr.ini             (유지: fallback용)
│   ├── dshm.ini               (유지: fallback용)
│   ├── client.ini             (유지: fallback용)
│   └── pc.ini                 (유지: fallback용)
├── inc/
│   ├── config_loader.h        (신규: Config Loader 헤더)
│   └── config_db.h            (신규: DB 스키마/쿼리 정의)
├── sub/
│   ├── config_loader.c        (신규: Config Loader 구현)
│   └── config_db.c            (신규: SQLite 접근 계층)
├── src/PZ/
│   └── pz_memory_conf.c       (수정: Config Loader 호출로 변경)
└── utl/
    ├── ini2db.c               (신규: .ini → DB 마이그레이션 도구)
    └── db2ini.c               (신규: DB → .ini 내보내기 도구)
```

---

## 7. 규칙 사전 조건

### 7.1 기존 프로젝트 규칙

- [x] CLAUDE.md 존재
- [x] 기존 코딩 컨벤션: ANSI C89, 4-space indent, snake_case 함수명
- [x] 기존 빌드 시스템: Makefile per module
- [x] 기존 헤더 의존성 체인: fep_fepp.h → fep_sub.h → shm_memory.h
- [ ] 별도 conventions.md 없음

### 7.2 신규 모듈 코딩 규칙

| Category | Rule |
|----------|------|
| **DB 함수 접두사** | `cfg_db_` (예: `cfg_db_open`, `cfg_db_load_proc`) |
| **Loader 함수 접두사** | `Config_` (예: `Config_Load`, `Config_Fallback`) |
| **에러 처리** | DB 함수는 OK/NOTOK 반환, 실패 시 Log() 기록 후 fallback |
| **메모리 관리** | sqlite3_open/close 쌍 보장, prepared statement 사용 |
| **헤더 포함** | config_loader.h는 fep_sub.h 이후에 include |
| **환경변수** | `_FEP_DB` 추가 (DB 파일 경로, 없으면 `_P_CFG/fep_config.db`) |

### 7.3 환경변수

| 변수 | 목적 | 범위 | 신규 |
|----------|---------|-------|:---:|
| `_P_CFG` | 설정 디렉토리 경로 (기존) | 모든 프로세스 | - |
| `_FEP_DB` | DB 파일 경로 (선택적) | 모든 프로세스 | **O** |
| `_FEP_DB_MODE` | DB/INI/AUTO (로드 모드 지정) | 데몬 프로세스 | **O** |

---

## 8. 구현 계획

### 8.1 단계별 구현 순서

```
Phase 1: 기반 구조 (1주차)
├── SQLite DB 스키마 생성
├── config_db.c/h (DB 접근 계층)
├── config_loader.c/h (추상화 레이어)
└── Makefile에 sqlite3 링크 추가

Phase 2: 마이그레이션 도구 (1주차)
├── ini2db.c (.ini → DB 변환)
├── db2ini.c (DB → .ini 내보내기)
└── 전체 .ini 파일 DB 임포트 검증

Phase 3: 저위험 설정 전환 (2주차)
├── client.ini → DB (IP 화이트리스트)
├── pc.ini → DB (관리자 PC)
├── sisetr.ini → DB (시세 TR 정의)
└── byte-level 검증

Phase 4: 중위험 설정 전환 (2주차)
├── tcp2.ini → DB (TCP 접속정보)
├── udpip.ini → DB (UDP 멀티캐스트)
├── file.ini → DB (SAM 파일 정의)
└── 기존 px_cfgload 호환 확인

Phase 5: 고위험 설정 전환 (3주차)
├── proc.ini → DB (프로세스 정의 + 암호화)
├── tcp1.ini → DB (TCP 포트 데몬)
├── daemon.ini → DB (데몬 설정)
├── dshm.ini → DB
└── 전체 통합 테스트

Phase 6: 운영 도구 호환 (3주차)
├── px_cfgload 수정 (DB 동시 업데이트)
├── px_cfgback 수정 (DB 백업 포함)
├── 운영 매뉴얼 업데이트
└── 최종 검증 (TEST → REAL)
```

### 8.2 롤백 계획

| 상황 | Rollback 방법 |
|------|---------------|
| Phase 3~5 중 문제 발생 | `_FEP_DB_MODE=INI` 설정 → 즉시 기존 방식 복원 |
| DB 파일 손상 | 자동 .ini fallback 동작 |
| 성능 이슈 | Config Loader에서 DB 비활성화 (1줄 변경) |
| 전체 롤백 | pz_memory_conf.c 원본 복원 (BACKUP/ 존재) |

---

## 9. DB 스키마 미리보기

```sql
-- 핵심 테이블 (상세는 Design 문서에서)
CREATE TABLE fep_environment (
    env_id      TEXT PRIMARY KEY,     -- 'TEST', 'REAL1', 'REAL2'
    hostname    TEXT,
    description TEXT
);

CREATE TABLE fep_config_group (
    group_id    TEXT PRIMARY KEY,     -- 'daemon', 'proc', 'tcp1', ...
    description TEXT,
    load_order  INTEGER
);

CREATE TABLE fep_config (
    config_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    env_id      TEXT NOT NULL,
    group_id    TEXT NOT NULL,
    section     TEXT NOT NULL,        -- 'Daemon_B', 'Tcp2_11', 'Proc_4'
    key_name    TEXT NOT NULL,        -- 'Ip', 'Port', 'Time_Out'
    key_value   TEXT,                 -- 실제 값
    data_type   TEXT DEFAULT 'STRING',-- 'INT', 'STRING', 'IP', 'PORT'
    description TEXT,
    UNIQUE(env_id, group_id, section, key_name),
    FOREIGN KEY(env_id) REFERENCES fep_environment(env_id),
    FOREIGN KEY(group_id) REFERENCES fep_config_group(group_id)
);

CREATE TABLE fep_config_history (
    history_id  INTEGER PRIMARY KEY AUTOINCREMENT,
    config_id   INTEGER,
    old_value   TEXT,
    new_value   TEXT,
    changed_at  TEXT DEFAULT (datetime('now','localtime')),
    changed_by  TEXT,
    change_reason TEXT
);

CREATE TABLE fep_ip_whitelist (
    ip_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    env_id      TEXT,
    ip_address  TEXT NOT NULL,
    ip_type     TEXT NOT NULL,        -- 'CLIENT', 'ADMIN'
    description TEXT,
    is_active   INTEGER DEFAULT 1
);

CREATE TABLE fep_tr_definition (
    tr_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    env_id      TEXT,
    tr_code     TEXT NOT NULL,
    msg_length  INTEGER,
    queue_no    INTEGER,
    description TEXT
);
```

---

## 10. 다음 단계

1. [ ] Design 문서 작성 (`/pdca design config-db-migration`)
   - DB 스키마 상세 설계
   - Config Loader API 상세 설계
   - ini2db 파싱 로직 설계
   - pz_memory_conf.c 수정 범위 상세화
2. [ ] 팀 리뷰 및 승인
3. [ ] 구현 시작 (`/pdca do config-db-migration`)

---

## 버전 이력

| 버전 | 날짜 | 변경사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-18 | Initial draft - 설정 분석 및 DB화 계획 수립 | FEP Dev Team |
