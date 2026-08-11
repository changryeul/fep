# config-db-migration 설계 문서

> **요약**: FEP 설정값 DB화 - SQLite 기반 Config Loader 추상화 레이어 상세 설계
>
> **프로젝트**: FEP (Front-End Processor for KRX)
> **Author**: FEP Dev Team
> **Date**: 2026-02-18
> **Status**: Draft
> **Planning Doc**: [config-db-migration.plan.md](../../01-plan/features/config-db-migration.plan.md)

---

## 1. 개요

### 1.1 설계 목표

- 기존 `pz_memory_conf.c`의 .ini 파싱 로직을 **Config Loader 추상화 레이어**로 감싸기
- SQLite DB를 Primary 소스로, .ini 파일을 Fallback으로 사용하는 하이브리드 구조
- **SHM 구조체 및 매크로 접근 방식 100% 유지** (비즈니스 프로세스 무변경)
- 변경 이력 자동 추적, 설정값 타입 검증, LogonPW 암호화

### 1.2 설계 원칙

- **Zero Impact**: 기존 비즈니스 프로세스(PA/PB/PW) 코드 변경 없음
- **Fail-Safe**: DB 장애 시 자동 .ini fallback, 무중단 운영 보장
- **Minimal Footprint**: SQLite 단일 파일, 추가 서버/인프라 불필요
- **Backward Compatible**: px_cfgload, px_cfgback 등 기존 운영 도구 호환

---

## 2. 아키텍처

### 2.1 컴포넌트 다이어그램

```
┌──────────────────────────────────────────────────────────────────┐
│                        FEP Config System                         │
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────────────┐│
│  │ ini2db   │  │ db2ini   │  │px_cfgload│  │ px_cfgback       ││
│  │ (신규)   │  │ (신규)   │  │ (수정)   │  │ (수정)           ││
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────────────┘│
│       │              │              │              │              │
│       ▼              ▼              ▼              ▼              │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │                config_db.c / config_db.h                   │  │
│  │  ┌──────────┐ ┌───────────┐ ┌──────────┐ ┌─────────────┐ │  │
│  │  │cfg_db_   │ │cfg_db_    │ │cfg_db_   │ │cfg_db_      │ │  │
│  │  │open/close│ │load_*     │ │save_*    │ │history_*    │ │  │
│  │  └──────────┘ └───────────┘ └──────────┘ └─────────────┘ │  │
│  └────────────────────────┬──────────────────────────────────┘  │
│                           │                                      │
│  ┌────────────────────────┴──────────────────────────────────┐  │
│  │              config_loader.c / config_loader.h              │  │
│  │                                                             │  │
│  │  Config_Load_All(flag)                                      │  │
│  │    ├─ 1. cfg_db_open() → DB 열기                           │  │
│  │    ├─ 2. cfg_db_load_daemon() → SHM에 기록                │  │
│  │    ├─ 3. cfg_db_load_file()   → SHM에 기록                │  │
│  │    ├─ 4. cfg_db_load_tcp1()   → SHM에 기록                │  │
│  │    ├─ 5. cfg_db_load_tcp2()   → SHM에 기록                │  │
│  │    ├─ 6. cfg_db_load_udpip()  → SHM에 기록                │  │
│  │    ├─ 7. cfg_db_load_sisetr() → SHM에 기록                │  │
│  │    ├─ 8. cfg_db_load_proc()   → SHM에 기록                │  │
│  │    ├─ 9. cfg_db_close()                                    │  │
│  │    └─ 실패 시 → Fallback (기존 *_Config_Read 호출)        │  │
│  │                                                             │  │
│  └────────────────────────┬──────────────────────────────────┘  │
│                           │                                      │
│                           ▼                                      │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │                     SHM (기존 구조 유지)                    │  │
│  │  Shm_Mem[D_K].Proc[P_K]  ← PROCESS_INFO                  │  │
│  │  Shm_Mem[D_K].Tcp2[n]    ← TCP2_INFO                     │  │
│  │  Shm_Mem[D_K].Udpip[n]   ← UDPIP_INFO                    │  │
│  │  Shm_Mem[D_K].File[n]    ← FILE_INFO                     │  │
│  │  Shm_Mem[D_K].Tcp1[n]    ← TCP1_INFO                     │  │
│  │  Shm_Mem[D_K].SiseTr[n]  ← SISETR_INFO                   │  │
│  └───────────────────────────────────────────────────────────┘  │
│                           │                                      │
│                           ▼                                      │
│            PA/PB/PW 비즈니스 프로세스 (변경 없음)               │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
```

### 2.2 데이터 흐름

```
[데몬 시작]
    │
    ▼
Config_Load_All(flag)
    │
    ├─ _FEP_DB_MODE 확인
    │   ├─ "INI"  → 기존 *_Config_Read() 직접 호출 (변경 없음)
    │   ├─ "DB"   → DB만 사용 (fallback 없음)
    │   └─ "AUTO" → DB 우선, 실패 시 .ini fallback (기본값)
    │
    ├─ [AUTO/DB 모드]
    │   ├─ cfg_db_open(_FEP_CFG/fep_config.db)
    │   │   ├─ 성공 → DB 로드 진행
    │   │   └─ 실패 → .ini fallback
    │   │
    │   ├─ cfg_db_load_daemon(flag) → INFO()/DAEMON() 구조체에 기록
    │   ├─ cfg_db_load_file(flag)   → FILEM() 구조체에 기록
    │   ├─ cfg_db_load_tcp1(flag)   → TCP1() 구조체에 기록
    │   ├─ cfg_db_load_tcp2(flag)   → TCP2() 구조체에 기록
    │   ├─ cfg_db_load_udpip(flag)  → UDPIP() 구조체에 기록
    │   ├─ cfg_db_load_sisetr(flag) → SISETR() 구조체에 기록
    │   ├─ cfg_db_load_proc(flag)   → PROC() 구조체에 기록
    │   │
    │   ├─ cfg_db_sync_to_ini()  (DB → .ini 동기화)
    │   └─ cfg_db_close()
    │
    └─ [INI fallback]
        ├─ Daemon_Config_Read(flag)  (기존 코드 그대로)
        ├─ File_Config_Read(flag)
        ├─ Tcp1_Config_Read(flag)
        ├─ Tcp2_Config_Read(flag)
        ├─ Udpip_Config_Read(flag)
        ├─ SiseTr_Config_Read(flag)
        └─ Proc_Config_Read(flag)
```

### 2.3 의존성

| 컴포넌트 | 의존 대상 | 목적 |
|-----------|-----------|---------|
| config_loader.c | config_db.c, fep_sub.h | DB/INI 로드 추상화 |
| config_db.c | sqlite3.h | SQLite DB 접근 |
| pz_memory_conf.c | config_loader.h | Config_Load_All() 호출 |
| ini2db.c | config_db.c, fep_sub.h | 마이그레이션 도구 |
| db2ini.c | config_db.c | 역방향 내보내기 |

---

## 3. 데이터 모델

### 3.1 SQLite 스키마 (완전 정의)

```sql
-- ============================================================
-- fep_config.db - FEP Configuration Database
-- ============================================================

PRAGMA journal_mode=WAL;        -- 동시 읽기 성능 향상
PRAGMA foreign_keys=ON;

-- ------------------------------------------------------------
-- 1. 환경 정의
-- ------------------------------------------------------------
CREATE TABLE fep_environment (
    env_id      TEXT PRIMARY KEY,           -- 'TEST', 'REAL1', 'REAL2'
    hostname    TEXT NOT NULL,              -- 서버 호스트명
    description TEXT,
    is_active   INTEGER DEFAULT 1,
    created_at  TEXT DEFAULT (datetime('now','localtime'))
);

-- 초기 데이터
INSERT INTO fep_environment VALUES ('TEST',  'at05',    '테스트서버',  1, datetime('now','localtime'));
INSERT INTO fep_environment VALUES ('REAL1', 'podm11',  '운영1서버',   1, datetime('now','localtime'));
INSERT INTO fep_environment VALUES ('REAL2', 'podm12',  '운영2서버',   1, datetime('now','localtime'));

-- ------------------------------------------------------------
-- 2. 설정 그룹 (기존 .ini 파일 단위)
-- ------------------------------------------------------------
CREATE TABLE fep_config_group (
    group_id    TEXT PRIMARY KEY,           -- 'daemon', 'proc', 'tcp1', ...
    ini_file    TEXT NOT NULL,              -- 원본 .ini 파일명
    description TEXT,
    load_order  INTEGER NOT NULL            -- 로딩 순서 (daemon=1, file=2, ...)
);

INSERT INTO fep_config_group VALUES ('daemon',  'daemon.ini',  '데몬 프로세스 정의',       1);
INSERT INTO fep_config_group VALUES ('file',    'file.ini',    'SAM 파일 정의',            2);
INSERT INTO fep_config_group VALUES ('dshm',    'dshm.ini',    '데이터 SHM 정의',          3);
INSERT INTO fep_config_group VALUES ('tcp1',    'tcp1.ini',    'TCP 포트 데몬 설정',       4);
INSERT INTO fep_config_group VALUES ('tcp2',    'tcp2.ini',    'TCP 직접 접속 정보',       5);
INSERT INTO fep_config_group VALUES ('udpip',   'udpip.ini',   'UDP 멀티캐스트 설정',      6);
INSERT INTO fep_config_group VALUES ('sisetr',  'sisetr.ini',  '시세 TR 코드 정의',        7);
INSERT INTO fep_config_group VALUES ('proc',    'proc.ini',    '프로세스별 속성',          8);
INSERT INTO fep_config_group VALUES ('client',  'client.ini',  '클라이언트 IP 화이트리스트', 9);
INSERT INTO fep_config_group VALUES ('pc',      'pc.ini',      '관리자 PC IP 리스트',      10);

-- ------------------------------------------------------------
-- 3. 설정 값 (핵심 테이블)
-- ------------------------------------------------------------
CREATE TABLE fep_config (
    config_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    env_id      TEXT NOT NULL,
    group_id    TEXT NOT NULL,
    section     TEXT NOT NULL,              -- 'Daemon_B', 'Tcp2_11', 'Proc_4'
    key_name    TEXT NOT NULL,              -- 'Ip', 'Port', 'Time_Out', 'ID'
    key_value   TEXT,                       -- 실제 값 (문자열 저장)
    data_type   TEXT DEFAULT 'STRING',      -- 'INT', 'STRING', 'IP', 'PORT', 'PW'
    is_encrypted INTEGER DEFAULT 0,         -- 1이면 key_value가 암호화됨
    description TEXT,
    updated_at  TEXT DEFAULT (datetime('now','localtime')),
    updated_by  TEXT DEFAULT 'system',
    UNIQUE(env_id, group_id, section, key_name),
    FOREIGN KEY(env_id) REFERENCES fep_environment(env_id),
    FOREIGN KEY(group_id) REFERENCES fep_config_group(group_id)
);

-- 인덱스
CREATE INDEX idx_config_env_group ON fep_config(env_id, group_id);
CREATE INDEX idx_config_section ON fep_config(section);

-- ------------------------------------------------------------
-- 4. 변경 이력 (자동 트리거)
-- ------------------------------------------------------------
CREATE TABLE fep_config_history (
    history_id    INTEGER PRIMARY KEY AUTOINCREMENT,
    config_id     INTEGER NOT NULL,
    env_id        TEXT NOT NULL,
    group_id      TEXT NOT NULL,
    section       TEXT NOT NULL,
    key_name      TEXT NOT NULL,
    old_value     TEXT,
    new_value     TEXT,
    changed_at    TEXT DEFAULT (datetime('now','localtime')),
    changed_by    TEXT DEFAULT 'system',
    change_reason TEXT
);

CREATE INDEX idx_history_config ON fep_config_history(config_id);
CREATE INDEX idx_history_time ON fep_config_history(changed_at);

-- 자동 이력 기록 트리거
CREATE TRIGGER trg_config_update
AFTER UPDATE OF key_value ON fep_config
WHEN OLD.key_value != NEW.key_value
BEGIN
    INSERT INTO fep_config_history
        (config_id, env_id, group_id, section, key_name,
         old_value, new_value, changed_by)
    VALUES
        (NEW.config_id, NEW.env_id, NEW.group_id, NEW.section, NEW.key_name,
         OLD.key_value, NEW.key_value, NEW.updated_by);
END;

-- ------------------------------------------------------------
-- 5. IP 화이트리스트 (client.ini + pc.ini 통합)
-- ------------------------------------------------------------
CREATE TABLE fep_ip_whitelist (
    ip_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    env_id      TEXT NOT NULL,
    ip_address  TEXT NOT NULL,
    ip_type     TEXT NOT NULL CHECK(ip_type IN ('CLIENT', 'ADMIN')),
    description TEXT,
    is_active   INTEGER DEFAULT 1,
    updated_at  TEXT DEFAULT (datetime('now','localtime')),
    FOREIGN KEY(env_id) REFERENCES fep_environment(env_id)
);

CREATE INDEX idx_whitelist_env ON fep_ip_whitelist(env_id, ip_type);

-- ------------------------------------------------------------
-- 6. TR 코드 정의 (sisetr.ini 전용)
-- ------------------------------------------------------------
CREATE TABLE fep_tr_definition (
    tr_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    env_id      TEXT NOT NULL,
    tr_code     TEXT NOT NULL,              -- 'A0014', 'B6034', ...
    msg_length  INTEGER NOT NULL,           -- 메시지 길이
    queue_no    INTEGER NOT NULL,           -- 큐 번호
    description TEXT,
    is_active   INTEGER DEFAULT 1,
    UNIQUE(env_id, tr_code),
    FOREIGN KEY(env_id) REFERENCES fep_environment(env_id)
);

-- ------------------------------------------------------------
-- 7. DB 메타정보
-- ------------------------------------------------------------
CREATE TABLE fep_db_meta (
    key_name    TEXT PRIMARY KEY,
    key_value   TEXT,
    updated_at  TEXT DEFAULT (datetime('now','localtime'))
);

INSERT INTO fep_db_meta VALUES ('schema_version', '1.0', datetime('now','localtime'));
INSERT INTO fep_db_meta VALUES ('created_at', datetime('now','localtime'), datetime('now','localtime'));
INSERT INTO fep_db_meta VALUES ('last_loaded_at', NULL, datetime('now','localtime'));
INSERT INTO fep_db_meta VALUES ('last_loaded_env', NULL, datetime('now','localtime'));
```

### 3.2 .ini → DB 매핑 규칙

각 .ini 파일의 키-값이 `fep_config` 테이블에 어떻게 매핑되는지:

#### daemon.ini 매핑

| .ini 키 | section | key_name | data_type | 예시 값 |
|---------|---------|----------|-----------|---------|
| `Daemon_B_ID` | `Daemon_B` | `ID` | STRING | `pb_daemon_mp` |
| `Daemon_B_Start_Time` | `Daemon_B` | `Start_Time` | STRING | `0430` |
| `Daemon_B_End_Time` | `Daemon_B` | `End_Time` | STRING | `0400` |
| `Daemon_B_Date_Flag` | `Daemon_B` | `Date_Flag` | INT | `2` |
| `Daemon_B_Compact_Days` | `Daemon_B` | `Compact_Days` | INT | `9` |
| `Daemon_B_Proc_Count` | `Daemon_B` | `Proc_Count` | INT | `20` |
| `Daemon_B_File_Count` | `Daemon_B` | `File_Count` | INT | `8` |
| `Daemon_B_Tcp2_Count` | `Daemon_B` | `Tcp2_Count` | INT | `20` |
| `Daemon_B_Udpip_Count` | `Daemon_B` | `Udpip_Count` | INT | `5` |
| `Daemon_B_FIFO` | `Daemon_B` | `FIFO` | STRING | `pb_FIFO` |
| `Daemon_B_Shm_Log` | `Daemon_B` | `Shm_Log` | INT | `1` |

#### proc.ini 매핑

| .ini 키 | section | key_name | data_type | 예시 값 |
|---------|---------|----------|-----------|---------|
| `Proc_1_Comment` | `Proc_1` | `Comment` | STRING | `채권KTS_시세수신` |
| `Proc_1_ID` | `Proc_1` | `ID` | STRING | `pb_7102_ur` |
| `Proc_1_Status` | `Proc_1` | `Status` | STRING | `S` |
| `Proc_1_Type` | `Proc_1` | `Type` | STRING | `UR` |
| `Proc_1_Udp_Port` | `Proc_1` | `Udp_Port` | STRING | `0,10402` |
| `Proc_1_Start_Time` | `Proc_1` | `Start_Time` | STRING | `0500` |
| `Proc_1_End_Time` | `Proc_1` | `End_Time` | STRING | `2300` |
| `Proc_1_Time_Out` | `Proc_1` | `Time_Out` | INT | `0` |
| `Proc_4_LogonID` | `Proc_4` | `LogonID` | STRING | `M60501O001` |
| `Proc_4_LogonPW` | `Proc_4` | `LogonPW` | PW | `(AES암호화)` |

#### tcp2.ini 매핑

| .ini 키 | section | key_name | data_type | 예시 값 |
|---------|---------|----------|-----------|---------|
| `Tcp2_Count` | `_global` | `Tcp2_Count` | INT | `16` |
| `Tcp2_11_Ip` | `Tcp2_11` | `Ip` | IP | `192.168.157.119` |
| `Tcp2_11_Port` | `Tcp2_11` | `Port` | PORT | `37221` |
| `Tcp2_11_Comment` | `Tcp2_11` | `Comment` | STRING | `채권주문송신KTS` |

#### 공통 파싱 규칙

```
.ini 파일 라인: "Daemon_B_Start_Time=0430"
                 ~~~~~~~~ ~ ~~~~~~~~~~  ~~~~
                 section  | key_name    key_value
                          |
              'Daemon_' + 'B' → section = "Daemon_B"
              나머지 → key_name = "Start_Time"
```

**파싱 정규식**: `^({Prefix}_{SectionId}_)?{KeyName}={Value}`

---

## 4. API 사양 (C Function Interface)

### 4.1 config_db.h - DB 접근 계층

```c
/* config_db.h - SQLite DB Access Layer for FEP Configuration */
#ifndef __CONFIG_DB_H
#define __CONFIG_DB_H

#include <sqlite3.h>

/* ============================================================
   Return Codes
   ============================================================ */
#define CFG_DB_OK           0
#define CFG_DB_ERR_OPEN    -1
#define CFG_DB_ERR_QUERY   -2
#define CFG_DB_ERR_NODATA  -3
#define CFG_DB_ERR_TYPE    -4
#define CFG_DB_ERR_ENCRYPT -5

/* ============================================================
   DB Connection Management
   ============================================================ */

/*
 * DB 파일 열기
 * @param db_path  DB 파일 경로 (NULL이면 _FEP_CFG/fep_config.db)
 * @return CFG_DB_OK or CFG_DB_ERR_OPEN
 */
int     cfg_db_open(const char *db_path);

/*
 * DB 파일 닫기
 */
void    cfg_db_close(void);

/*
 * DB 연결 상태 확인
 * @return 1(연결됨) or 0(미연결)
 */
int     cfg_db_is_open(void);

/* ============================================================
   Environment Detection
   ============================================================ */

/*
 * 현재 환경 ID 결정 (hostname 기반)
 * @return env_id 문자열 ("TEST", "REAL1", "REAL2")
 *
 * 규칙:
 *   hostname == "podm11" → "REAL1"
 *   hostname == "podm12" → "REAL2"
 *   otherwise            → "TEST"
 */
const char* cfg_db_detect_env(void);

/* ============================================================
   Config Load Functions (DB → SHM)
   각 함수는 기존 *_Config_Read(flag)와 동일한 SHM 기록 결과를 생성
   ============================================================ */

/*
 * daemon.ini 설정을 DB에서 로드 → INFO()/DAEMON() 구조체에 기록
 * @param flag  0: INFO만, 1: INFO+DAEMON, 2: 임시버퍼, 3: 특정 재로드
 * @return CFG_DB_OK or error code
 */
int     cfg_db_load_daemon(int flag);

/*
 * file.ini 설정을 DB에서 로드 → FILEM() 구조체에 기록
 * @param flag  0: 전체, 1: 현재 데몬만
 * @return CFG_DB_OK or error code
 */
int     cfg_db_load_file(int flag);

/*
 * dshm.ini 설정을 DB에서 로드 → DSHM() 구조체에 기록
 */
int     cfg_db_load_dshm(int flag);

/*
 * tcp1.ini 설정을 DB에서 로드 → TCP1() 구조체에 기록
 */
int     cfg_db_load_tcp1(int flag);

/*
 * tcp2.ini 설정을 DB에서 로드 → TCP2() 구조체에 기록
 */
int     cfg_db_load_tcp2(int flag);

/*
 * udpip.ini 설정을 DB에서 로드 → UDPIP() 구조체에 기록
 */
int     cfg_db_load_udpip(int flag);

/*
 * sisetr.ini 설정을 DB에서 로드 → SISETR() 구조체에 기록
 */
int     cfg_db_load_sisetr(int flag);

/*
 * proc.ini 설정을 DB에서 로드 → PROC() 구조체에 기록
 * LogonPW는 자동 복호화
 */
int     cfg_db_load_proc(int flag);

/*
 * client.ini + pc.ini IP 화이트리스트를 DB에서 로드
 * @param ip_list  결과 배열 (caller가 할당)
 * @param max_cnt  배열 최대 크기
 * @param ip_type  "CLIENT" or "ADMIN"
 * @return 로드된 IP 수 or error code
 */
int     cfg_db_load_ip_whitelist(char ip_list[][20], int max_cnt,
                                  const char *ip_type);

/* ============================================================
   Config Save Functions (값 저장 + 자동 이력)
   ============================================================ */

/*
 * 단일 설정값 저장 (UPDATE)
 * fep_config_history에 자동 이력 기록 (DB 트리거)
 * @return CFG_DB_OK or error code
 */
int     cfg_db_set_value(const char *env_id, const char *group_id,
                          const char *section, const char *key_name,
                          const char *new_value, const char *changed_by);

/* ============================================================
   DB → .ini Sync (Fallback 최신화)
   ============================================================ */

/*
 * 특정 그룹의 DB 설정을 .ini 파일로 내보내기
 * @param group_id  대상 그룹 ("daemon", "proc", ...)
 * @param ini_path  출력 .ini 파일 경로
 * @return CFG_DB_OK or error code
 */
int     cfg_db_export_ini(const char *group_id, const char *ini_path);

/*
 * 모든 그룹을 .ini로 내보내기 (fallback 동기화)
 */
int     cfg_db_sync_all_ini(void);

/* ============================================================
   Password Encryption
   ============================================================ */

/*
 * LogonPW 암호화 (평문 → AES-256-CBC)
 * @param plain    평문 패스워드
 * @param cipher   암호문 출력 버퍼 (64바이트 이상)
 * @return CFG_DB_OK or CFG_DB_ERR_ENCRYPT
 */
int     cfg_db_encrypt_pw(const char *plain, char *cipher);

/*
 * LogonPW 복호화 (AES-256-CBC → 평문)
 * @param cipher   암호문
 * @param plain    평문 출력 버퍼 (20바이트 이상)
 * @return CFG_DB_OK or CFG_DB_ERR_ENCRYPT
 */
int     cfg_db_decrypt_pw(const char *cipher, char *plain);

#endif /* __CONFIG_DB_H */
```

### 4.2 config_loader.h - 추상화 레이어

```c
/* config_loader.h - Config Loader Abstraction Layer */
#ifndef __CONFIG_LOADER_H
#define __CONFIG_LOADER_H

/* ============================================================
   Load Mode
   ============================================================ */
typedef enum {
    CFG_MODE_AUTO,      /* DB 우선, 실패 시 .ini fallback (기본값) */
    CFG_MODE_DB,        /* DB만 사용 (fallback 없음) */
    CFG_MODE_INI        /* .ini만 사용 (기존 방식) */
} CFG_LOAD_MODE;

/* ============================================================
   Load Source (실제 로드된 소스)
   ============================================================ */
typedef enum {
    CFG_SRC_NONE,
    CFG_SRC_DB,
    CFG_SRC_INI
} CFG_LOAD_SOURCE;

/* ============================================================
   Load Result
   ============================================================ */
typedef struct {
    CFG_LOAD_SOURCE source;     /* 실제 로드된 소스 */
    int             total_keys; /* 로드된 설정 키 수 */
    int             db_ok;      /* DB 로드 성공 여부 */
    int             ini_ok;     /* INI fallback 사용 여부 */
    char            loaded_at[20]; /* 로드 시각 (HH:MM:SS) */
    char            env_id[10];    /* 환경 ID */
} CFG_LOAD_RESULT;

/* ============================================================
   Main API
   ============================================================ */

/*
 * 전체 설정 로드 (메인 진입점)
 * pz_memory_conf.c에서 호출
 *
 * @param flag  기존 *_Config_Read(flag)와 동일한 의미
 *              0: 전체 초기화 (INFO만)
 *              1: 데몬 시작 (INFO+DAEMON)
 *              2: 임시 버퍼
 *              3: 특정 재로드
 * @return CFG_LOAD_RESULT
 *
 * 동작:
 *   1. _FEP_DB_MODE 환경변수 확인 → CFG_LOAD_MODE 결정
 *   2. AUTO 모드: cfg_db_open() 시도
 *      - 성공 → DB에서 모든 설정 로드 → .ini 동기화
 *      - 실패 → .ini fallback (기존 *_Config_Read)
 *   3. 결과를 CFG_LOAD_RESULT에 기록
 *   4. 로드 소스를 SHM에 기록 (운영 모니터링용)
 */
CFG_LOAD_RESULT Config_Load_All(int flag);

/*
 * 특정 그룹만 리로드
 * px_cfgload에서 호출
 *
 * @param group_id  "daemon", "proc", "tcp1", ...
 * @param flag      로드 플래그
 * @return CFG_LOAD_RESULT
 */
CFG_LOAD_RESULT Config_Reload_Group(const char *group_id, int flag);

/*
 * 현재 로드 모드 반환
 */
CFG_LOAD_MODE Config_Get_Mode(void);

/*
 * 마지막 로드 결과 반환
 */
CFG_LOAD_RESULT Config_Get_Last_Result(void);

#endif /* __CONFIG_LOADER_H */
```

### 4.3 핵심 구현 로직

#### Config_Load_All() 구현 의사코드

```c
CFG_LOAD_RESULT Config_Load_All(int flag)
{
    CFG_LOAD_RESULT result;
    memset(&result, 0, sizeof(result));

    /* 1. 로드 모드 결정 */
    CFG_LOAD_MODE mode = CFG_MODE_AUTO;
    char *mode_env = getenv("_FEP_DB_MODE");
    if (mode_env != NULL) {
        if (strcmp(mode_env, "INI") == 0)  mode = CFG_MODE_INI;
        if (strcmp(mode_env, "DB") == 0)   mode = CFG_MODE_DB;
    }

    /* 2. INI 모드면 기존 로직 직접 호출 */
    if (mode == CFG_MODE_INI) {
        Daemon_Config_Read(flag);
        File_Config_Read(flag);
        Dshm_Config_Read(flag);
        Tcp1_Config_Read(flag);
        Tcp2_Config_Read(flag);
        Udpip_Config_Read(flag);
        Proc_Config_Read(flag);
        result.source = CFG_SRC_INI;
        result.ini_ok = 1;
        return result;
    }

    /* 3. DB 경로 결정 */
    char db_path[256];
    char *db_env = getenv("_FEP_DB");
    if (db_env != NULL)
        strncpy(db_path, db_env, sizeof(db_path));
    else
        sprintf(db_path, "%s/fep_config.db", _FEP_CFG);

    /* 4. DB 로드 시도 */
    if (cfg_db_open(db_path) == CFG_DB_OK) {
        int rc = CFG_DB_OK;
        rc |= cfg_db_load_daemon(flag);
        rc |= cfg_db_load_file(flag);
        rc |= cfg_db_load_dshm(flag);
        rc |= cfg_db_load_tcp1(flag);
        rc |= cfg_db_load_tcp2(flag);
        rc |= cfg_db_load_udpip(flag);
        rc |= cfg_db_load_proc(flag);

        if (rc == CFG_DB_OK) {
            result.source = CFG_SRC_DB;
            result.db_ok = 1;
            Log(USR_OK, "Config loaded from DB [%s]", db_path);

            /* DB → .ini 동기화 (fallback 최신화) */
            cfg_db_sync_all_ini();
        } else {
            cfg_db_close();
            Log(USR_WARN, "DB load partial failure, fallback to INI");
            goto fallback_ini;
        }
        cfg_db_close();
    }
    /* 5. DB 실패 → .ini fallback (AUTO 모드) */
    else if (mode == CFG_MODE_AUTO) {
fallback_ini:
        Log(USR_WARN, "Config DB unavailable, using INI fallback");
        Daemon_Config_Read(flag);
        File_Config_Read(flag);
        Dshm_Config_Read(flag);
        Tcp1_Config_Read(flag);
        Tcp2_Config_Read(flag);
        Udpip_Config_Read(flag);
        Proc_Config_Read(flag);
        result.source = CFG_SRC_INI;
        result.ini_ok = 1;
    }
    /* DB only 모드에서 DB 실패 → 치명적 오류 */
    else {
        Log(SAM_FATAL, "Config DB open failed and no fallback [%s]", db_path);
        sleep(3);
        exit(FAIL);
    }

    Get_DateTime(result.loaded_at);
    strncpy(result.env_id, cfg_db_detect_env(), sizeof(result.env_id));
    return result;
}
```

#### cfg_db_load_proc() 구현 의사코드

```c
/*
 * proc.ini에 해당하는 DB 데이터를 PROC() SHM 구조체에 로드
 * 기존 Proc_Config_Read(flag)와 동일한 SHM 결과를 생성해야 함
 */
int cfg_db_load_proc(int flag)
{
    const char *env_id = cfg_db_detect_env();
    sqlite3_stmt *stmt;
    int rc, proc_no, p_cnt;
    char section[20], key[40], value[200];

    /* proc.ini의 _global 설정 (Proc_Count) */
    const char *sql_count =
        "SELECT key_value FROM fep_config "
        "WHERE env_id=? AND group_id='proc' "
        "AND section='_global' AND key_name='Proc_Count'";

    rc = sqlite3_prepare_v2(g_db, sql_count, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return CFG_DB_ERR_QUERY;
    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);

    /* Proc_Count 로드 (생략: 상세 구현) */

    /* 각 프로세스별 설정 로드 */
    const char *sql_proc =
        "SELECT section, key_name, key_value, data_type, is_encrypted "
        "FROM fep_config "
        "WHERE env_id=? AND group_id='proc' AND section LIKE 'Proc_%' "
        "ORDER BY CAST(SUBSTR(section, 6) AS INTEGER), key_name";

    rc = sqlite3_prepare_v2(g_db, sql_proc, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        strncpy(section, (const char*)sqlite3_column_text(stmt, 0), 20);
        strncpy(key,     (const char*)sqlite3_column_text(stmt, 1), 40);
        strncpy(value,   (const char*)sqlite3_column_text(stmt, 2), 200);
        int is_enc = sqlite3_column_int(stmt, 4);

        /* section에서 프로세스 번호 추출: "Proc_4" → 4 */
        proc_no = atoi(section + 5);

        /* D_K 기반 필터링 (기존 로직과 동일) */
        if (flag == 1 && D_K != p_cnt) continue;

        /* SHM 구조체에 매핑 */
        if (strcmp(key, "ID") == 0) {
            strncpy(PROC(p_cnt, proc_no-1).process_id, value,
                    sizeof(PROC(p_cnt, proc_no-1).process_id));
            sprintf(PROC(p_cnt, proc_no-1).process_path,
                    "%s/%s", _FEP_BIN, value);
        }
        else if (strcmp(key, "Time_Out") == 0) {
            PROC(p_cnt, proc_no-1).timeout = (short)atoi(value);
        }
        else if (strcmp(key, "LogonPW") == 0 && is_enc) {
            /* 암호화된 PW → 복호화 후 SHM에 기록 */
            char plain[20];
            cfg_db_decrypt_pw(value, plain);
            strncpy(PROC(p_cnt, proc_no-1).logon_pw, plain, 20);
        }
        /* ... 나머지 키 매핑 ... */
    }

    sqlite3_finalize(stmt);
    return CFG_DB_OK;
}
```

---

## 5. pz_memory_conf.c 수정 설계

### 5.1 변경 범위

`pz_memory_conf.c`의 기존 10개 `*_Config_Read()` 함수는 **그대로 유지**합니다.
변경은 이 함수들을 **호출하는 측** (pz_memory.c 또는 최상위)에만 적용합니다.

```c
/* 기존 코드 (pz_memory.c 내) */
Daemon_Config_Read(flag);
File_Config_Read(flag);
Tcp1_Config_Read(flag);
Tcp2_Config_Read(flag);
Udpip_Config_Read(flag);
Proc_Config_Read(flag);

/* 변경 후 */
#include "config_loader.h"

CFG_LOAD_RESULT cfg_result = Config_Load_All(flag);
Log(USR_OK, "Config loaded from %s, keys=%d",
    cfg_result.source == CFG_SRC_DB ? "DB" : "INI",
    cfg_result.total_keys);
```

### 5.2 기존 함수 보존 전략

| 함수 | 변경 | 역할 |
|------|------|------|
| `Daemon_Config_Read(flag)` | 변경 없음 | INI fallback용으로 유지 |
| `File_Config_Read(flag)` | 변경 없음 | INI fallback용으로 유지 |
| `Dshm_Config_Read(flag)` | 변경 없음 | INI fallback용으로 유지 |
| `Tcp1_Config_Read(flag)` | 변경 없음 | INI fallback용으로 유지 |
| `Tcp2_Config_Read(flag)` | 변경 없음 | INI fallback용으로 유지 |
| `Udpip_Config_Read(flag)` | 변경 없음 | INI fallback용으로 유지 |
| `SiseTr_Config_Read(flag)` | 변경 없음 | INI fallback용으로 유지 |
| `Proc_Config_Read(flag)` | 변경 없음 | INI fallback용으로 유지 |
| `Accno_Config_Read(flag)` | 변경 없음 | INI fallback용으로 유지 |

---

## 6. ini2db 마이그레이션 도구 설계

### 6.1 사용법

```bash
# 전체 .ini 파일을 DB로 임포트
ini2db -e TEST -d $HOME/fep/st01/cfg/fep_config.db

# 특정 .ini 파일만 임포트
ini2db -e TEST -f proc.ini -d $HOME/fep/st01/cfg/fep_config.db

# LogonPW 암호화 포함
ini2db -e TEST -d $HOME/fep/st01/cfg/fep_config.db --encrypt-pw

# Dry-run (실제 DB 기록 없이 파싱 결과만 출력)
ini2db -e TEST --dry-run
```

### 6.2 파싱 로직

```c
/*
 * .ini 파일 공통 파싱 패턴
 * 모든 .ini 파일이 동일한 형식:
 *   - '#'으로 시작하는 줄: 주석
 *   - 공백/탭으로 시작하는 줄: 무시
 *   - "Key=Value" 형식
 *   - "Xxx_End": 섹션 종료
 *   - "{SYS}_CONF_START" / "{SYS}_CONF_END": 서브시스템 블록
 */
typedef struct {
    char section[50];
    char key_name[50];
    char key_value[200];
    char data_type[10];
} INI_ENTRY;

/*
 * .ini 파일 파싱
 * @param ini_path   .ini 파일 경로
 * @param group_id   설정 그룹명 ("daemon", "proc", ...)
 * @param entries    결과 배열
 * @param max_entries 최대 항목 수
 * @return 파싱된 항목 수
 */
int parse_ini_file(const char *ini_path, const char *group_id,
                   INI_ENTRY *entries, int max_entries);
```

### 6.3 파싱 규칙 (그룹별)

| 그룹 | 섹션 구분자 | 키 접두사 패턴 | 예시 |
|------|------------|---------------|------|
| daemon | `Daemon_{A-Z}_` | `Daemon_B_ID` → section=`Daemon_B`, key=`ID` | `Daemon_B` |
| proc | `Proc_{N}_` | `Proc_4_LogonID` → section=`Proc_4`, key=`LogonID` | `Proc_1`~`Proc_18` |
| tcp1 | `Tcp1_{N}_` | `Tcp1_1_Master_Port` → section=`Tcp1_1`, key=`Master_Port` | `Tcp1_1`, `Tcp1_2` |
| tcp2 | `Tcp2_{N}_` | `Tcp2_11_Ip` → section=`Tcp2_11`, key=`Ip` | `Tcp2_1`~`Tcp2_16` |
| udpip | `Udpip_{N}_` | `Udpip_1_Ip` → section=`Udpip_1`, key=`Ip` | `Udpip_1`, `Udpip_2` |
| file | `File_{N}_` | `File_1_Name` → section=`File_1`, key=`Name` | `File_1`~`File_8` |
| sisetr | `SiseTr_{N}_` | `SiseTr_1_TR` → section=`SiseTr_1`, key=`TR` | `SiseTr_1`~`SiseTr_29` |
| client | (순번) | IP 한줄씩 → section=`Client_{N}`, key=`Ip` | `Client_1`~`Client_20` |
| pc | (순번) | IP 한줄씩 → section=`Pc_{N}`, key=`Ip` | `Pc_1`~`Pc_7` |

---

## 7. px_cfgload 호환 설계

### 7.1 기존 동작

```bash
px_cfgload tcp2    # tcp2.ini.tmp → tcp2.ini 복사 후 SHM 리로드
px_cfgload all     # 모든 .ini 리로드
```

### 7.2 변경 후 동작

```bash
px_cfgload tcp2
# 1. 기존: tcp2.ini.tmp → tcp2.ini 복사
# 2. 신규: tcp2.ini → DB 동기화 (cfg_db_import_ini("tcp2"))
# 3. 기존: SHM 리로드 (Tcp2_Config_Read 또는 cfg_db_load_tcp2)
```

### 7.3 코드 변경

```c
/* px_cfgload.c 수정 */
#include "config_loader.h"

/* 기존 Tcp2_Config_Load() 함수 내 */
void Tcp2_Config_Load(void)
{
    /* 기존: .ini.tmp → .ini 복사 (유지) */
    copy_tmp_to_ini("tcp2");

    /* 신규: .ini → DB 동기화 */
    if (cfg_db_open(NULL) == CFG_DB_OK) {
        cfg_db_import_ini("tcp2", ini_path);
        cfg_db_close();
        Log(USR_OK, "tcp2 config synced to DB");
    }

    /* 기존: SHM 리로드 (Config_Reload_Group 사용) */
    Config_Reload_Group("tcp2", 1);
}
```

---

## 8. 에러 처리

### 8.1 에러 코드 정의

| Code | Constant | 원인 | 처리 |
|------|----------|------|------|
| 0 | `CFG_DB_OK` | 정상 | - |
| -1 | `CFG_DB_ERR_OPEN` | DB 파일 열기 실패 | .ini fallback |
| -2 | `CFG_DB_ERR_QUERY` | SQL 쿼리 실행 실패 | 해당 그룹 .ini fallback |
| -3 | `CFG_DB_ERR_NODATA` | 필수 데이터 없음 | Log(WARN) + .ini fallback |
| -4 | `CFG_DB_ERR_TYPE` | 데이터 타입 검증 실패 | Log(ERROR) + 해당 키 스킵 |
| -5 | `CFG_DB_ERR_ENCRYPT` | 암호화/복호화 실패 | Log(FATAL) + 해당 PW 공백 |

### 8.2 Fallback 흐름

```
cfg_db_load_daemon() 실패
    │
    ├─ 부분 실패 (일부 키 누락)
    │   └─ Log(WARN) → 해당 키만 기본값 사용 → 계속 진행
    │
    └─ 전체 실패 (DB 쿼리 오류)
        └─ Log(ERROR) → Daemon_Config_Read(flag) 호출
           └─ .ini도 실패 → Log(FATAL) → exit(FAIL)
```

---

## 9. 보안 고려사항

- [x] LogonPW AES-256-CBC 암호화 저장 (평문/base64 제거)
- [x] 암호화 키: `$INISAFENET_HOME/conf/` 기존 KRX 인증서 활용 또는 별도 키 파일
- [x] DB 파일 권한: 0640 (owner: fep, group: fep)
- [x] .ini fallback 파일의 LogonPW: DB 동기화 시 마스킹 (`****`)
- [x] fep_config_history에 PW 변경 이력은 `old_value='[ENCRYPTED]'`로 기록
- [x] SQL Injection 방지: 모든 쿼리 prepared statement 사용

---

## 10. 테스트 계획

### 10.1 테스트 범위

| 유형 | 대상 | 방법 |
|------|--------|--------|
| 단위 테스트 | config_db.c 각 함수 | 독립 실행 프로그램 |
| 통합 테스트 | Config_Load_All() | 데몬 시작/종료 사이클 |
| Fallback 테스트 | DB 장애 시나리오 | DB 파일 삭제/권한 제거 |
| 호환 테스트 | px_cfgload/px_cfgback | 운영 시나리오 재현 |
| byte-level 검증 | SHM 내용 비교 | INI로드 vs DB로드 결과 비교 |

### 10.2 테스트 케이스

- [ ] **TC-01**: ini2db로 전체 .ini 임포트 후 DB 행 수 검증
- [ ] **TC-02**: DB 로드 후 SHM 내용이 INI 로드와 byte-level 동일
- [ ] **TC-03**: DB 파일 삭제 후 데몬 시작 → .ini fallback 정상 동작
- [ ] **TC-04**: DB 파일 권한 000 설정 → .ini fallback 정상 동작
- [ ] **TC-05**: `_FEP_DB_MODE=INI` 설정 → DB 무시, 기존 방식 동작
- [ ] **TC-06**: `_FEP_DB_MODE=DB` + DB 없음 → exit(FAIL)
- [ ] **TC-07**: px_cfgload tcp2 실행 → DB + .ini 동시 업데이트
- [ ] **TC-08**: LogonPW 암호화 저장 확인 (DB에 평문 없음)
- [ ] **TC-09**: LogonPW 복호화 후 SHM에 정상 기록 확인
- [ ] **TC-10**: fep_config_history에 변경 이력 자동 기록 확인
- [ ] **TC-11**: 환경별(TEST/REAL1) 다른 설정 로드 확인
- [ ] **TC-12**: 전체 설정 로드 시간 < 100ms 확인

### 10.3 SHM byte-level 검증 도구

```c
/* utl/cfg_verify.c - DB로드 vs INI로드 비교 검증 */

int main(int argc, char *argv[])
{
    /* 1. INI 모드로 로드 → SHM 스냅샷 A */
    setenv("_FEP_DB_MODE", "INI", 1);
    Config_Load_All(1);
    memcpy(snapshot_a, Shm_Mem, sizeof(SHM_MEMORY));

    /* 2. DB 모드로 로드 → SHM 스냅샷 B */
    setenv("_FEP_DB_MODE", "DB", 1);
    Config_Load_All(1);
    memcpy(snapshot_b, Shm_Mem, sizeof(SHM_MEMORY));

    /* 3. byte-level 비교 */
    int diff = memcmp(snapshot_a, snapshot_b, sizeof(SHM_MEMORY));
    if (diff == 0)
        printf("PASS: DB load matches INI load (byte-level identical)\n");
    else {
        /* 차이 위치 상세 출력 */
        find_and_report_differences(snapshot_a, snapshot_b);
    }
}
```

---

## 11. 구현 가이드

### 11.1 파일 구조

```
st01/
├── cfg/
│   └── fep_config.db          (신규: SQLite DB)
├── inc/
│   ├── config_db.h            (신규: DB 접근 API)
│   └── config_loader.h        (신규: Loader 추상화 API)
├── sub/
│   ├── config_db.c            (신규: DB 접근 구현)
│   └── config_loader.c        (신규: Loader 구현)
├── src/PZ/
│   ├── pz_memory_conf.c       (기존 유지: fallback용)
│   └── pz_memory.c            (수정: Config_Load_All 호출)
├── utl/
│   ├── ini2db.c               (신규: 마이그레이션 도구)
│   ├── db2ini.c               (신규: 역방향 내보내기)
│   └── cfg_verify.c           (신규: byte-level 검증 도구)
└── make/
    └── Makefile               (수정: -lsqlite3 추가)
```

### 11.2 구현 순서

```
1. [ ] config_db.h/c 작성 (DB 접근 계층)
   ├── cfg_db_open/close
   ├── cfg_db_detect_env
   ├── cfg_db_load_daemon  ← 가장 복잡 (date_flag 로직)
   ├── cfg_db_load_proc    ← LogonPW 암호화 포함
   ├── cfg_db_load_tcp2    ← IP/Port 매핑
   ├── cfg_db_load_file
   ├── cfg_db_load_tcp1
   ├── cfg_db_load_udpip
   ├── cfg_db_load_sisetr
   └── cfg_db_load_dshm

2. [ ] config_loader.h/c 작성 (추상화 레이어)
   ├── Config_Load_All
   ├── Config_Reload_Group
   └── Fallback 로직

3. [ ] ini2db.c 작성 (마이그레이션 도구)
   ├── .ini 파싱 엔진
   ├── DB 스키마 초기화
   └── 데이터 임포트

4. [ ] pz_memory.c 수정 (최소 변경)
   └── Config_Load_All(flag) 호출로 교체

5. [ ] cfg_verify.c 작성 (검증 도구)
   └── INI vs DB byte-level 비교

6. [ ] px_cfgload.c 수정 (호환 레이어)
   └── DB 동기화 추가

7. [ ] Makefile 수정
   └── -lsqlite3 링크 추가

8. [ ] db2ini.c 작성 (역방향 내보내기)
```

### 11.3 Makefile 변경

```makefile
# sub/Makefile에 추가
SQLITE3_CFLAGS = -I/usr/include
SQLITE3_LDFLAGS = -lsqlite3

# config_db.o 빌드 규칙
config_db.o: config_db.c $(INC)/config_db.h
	$(CC) $(CFLAGS) $(SQLITE3_CFLAGS) -c config_db.c

config_loader.o: config_loader.c $(INC)/config_loader.h $(INC)/config_db.h
	$(CC) $(CFLAGS) -c config_loader.c

# PZ 데몬 링크에 sqlite3 추가
pz_daemon_mp: pz_daemon_mp.o pz_memory.o pz_memory_conf.o \
              config_loader.o config_db.o
	$(CC) -o $@ $^ $(LDFLAGS) $(SQLITE3_LDFLAGS)
```

---

## 12. 코딩 규칙

### 12.1 신규 모듈 코딩 규칙

| 항목 | 규칙 |
|------|------|
| 언어 표준 | ANSI C89 (기존 프로젝트와 동일) |
| 인덴트 | 탭 (기존 코드와 동일) |
| 함수 접두사 | `cfg_db_*` (DB계층), `Config_*` (Loader) |
| 에러 처리 | OK(0)/NOTOK(-N), Log() 호출 후 반환 |
| 메모리 | sqlite3 자동 관리, prepared statement 사용 |
| 전역 변수 | `static sqlite3 *g_db` (config_db.c 내부) |
| 문자열 | strncpy (sizeof 보호), sprintf 금지 → snprintf |

### 12.2 DB 쿼리 패턴

```c
/* 표준 패턴: prepare → bind → step → finalize */
sqlite3_stmt *stmt = NULL;
int rc;

rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
if (rc != SQLITE_OK) {
    Log(USR_ERROR, "SQL prepare failed: %s", sqlite3_errmsg(g_db));
    return CFG_DB_ERR_QUERY;
}

sqlite3_bind_text(stmt, 1, env_id, -1, SQLITE_STATIC);

while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    /* 데이터 처리 */
}

sqlite3_finalize(stmt);
```

---

## 버전 이력

| 버전 | 날짜 | 변경사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-18 | Initial draft - 상세 설계 문서 작성 | FEP Dev Team |
