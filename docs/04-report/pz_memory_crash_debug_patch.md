# pz_memory_mp Proc Config Crash — Debug Patch

**Date**: 2026-03-02
**Issue**: `pz_memory_mp`가 `pz_fepp_mp`에서 호출될 때 Proc_Config_Read 중 크래시 (단독 실행 시 정상)

---

## 증상

- `pz_fepp_mp` → `Creat_SHM_Process()` → `pz_memory_mp a` 실행 시 로그가 "start PA proc config ..."에서 멈추고 프로세스 소멸
- "end PA proc config" 메시지 미출력
- `pz_memory_mp a` 직접 실행 시 exit code 0 (정상)

## 근본 원인 분석

### 의심 경로 (확인 필요)

1. **FD 전체 닫기**: `Creat_SHM_Process()`에서 `close(0)` ~ `close(NOFILE)` 후 `execl` → stdout/stderr 없음. Log_Proc는 open/write/close 패턴이라 영향 적을 수 있으나, 일부 시스템 함수가 fd 0/1/2 부재 시 비정상 동작 가능
2. **AUTO 모드 DB 탐색**: `_FEP_DB_MODE` 미설정 시 AUTO 모드로 `fep_config.db` 파일 탐색 → 예기치 않은 SQLite 동작
3. **DAEMON 필드 미초기화**: `Daemon_Config_Read(1)` 호출에도 불구하고 특정 환경에서 DAEMON 필드가 0으로 남아 Proc_Config_Read 내 배열 경계 위반

## 수정된 파일 (4개)

### 1. `st01/src/PZ/pz_memory_proc.c`

**변경**: INFO→DAEMON 필드 복사 강화 (defense-in-depth)

- 기존: process_count, file_count 등 count 필드만 복사
- 변경: start_time, end_time, date_flag, date 필드도 추가 복사
- 복사 후 디버그 로그 출력: `"02. INFO->DAEMON copy done D_K=%d pcount=%d fcount=%d"`

### 2. `st01/src/PZ/pz_memory_conf.c`

**변경**: Proc_Config_Read에 3곳 디버그 로깅 추가

| 위치 | 로그 메시지 | 목적 |
|------|------------|------|
| CONF_START 직후 | `start %c%c proc config ... pcnt=%d dflag=%d pcount=%d fcount=%d` | DAEMON 필드 값 확인 |
| PROC_COUNT 검증 후 | `proc(%c) PROC_COUNT=%d OK (max=%d)` | 프로세스 수 범위 확인 |
| Proc_End | `proc(%c) Proc_End cnt=%d id=%s` | 어느 프로세스까지 읽었는지 확인 |

### 3. `st01/sub/config_loader.c`

**변경**: `cfg_loader_load_ini()` 내 각 Config_Read 호출 전후 진행 로그

```
cfg_loader_load_ini: START flag=%d
cfg_loader_load_ini: File_Config_Read done
cfg_loader_load_ini: Dshm_Config_Read done
cfg_loader_load_ini: Tcp1_Config_Read done
cfg_loader_load_ini: Tcp2_Config_Read done
cfg_loader_load_ini: Udpip_Config_Read done
cfg_loader_load_ini: calling Proc_Config_Read ...
cfg_loader_load_ini: Proc_Config_Read done
```

### 4. `st01/env/pkg_env.sh`

**변경**: `_FEP_DB_MODE=INI` 환경변수 추가 (line 7)

- AUTO 모드의 DB 탐색을 우회하여 INI 전용 모드로 강제 설정
- DB 관련 잠재적 크래시 원인 제거

## 빌드 & 배포 절차

### 서버에서 빌드

```bash
cd ~/fep/st01
source env/pkg_env.sh

# 공유 라이브러리 리빌드 (config_loader.c 변경)
mk.sh sub

# PZ 모듈 리빌드 (pz_memory_proc.c, pz_memory_conf.c 변경)
mk.sh pz
```

### 테스트

```bash
# IPC 리소스 정리
ipcrm.sh

# pz_fepp_mp 실행
pz_fepp_mp &

# 로그 확인 (크래시 지점 특정)
tail -f ~/fep/st03/LOG/PZ/00000000/pz_memory_mp_*

# 커널 로그 확인 (segfault 시)
dmesg | tail -20
```

### 기대 로그 출력 (정상 시)

```
00. pz_memory_proc Mem_SHM_Creat Start!!
01. pz_memory_proc Mem_SHM_Creat End!!
02. INFO->DAEMON copy done D_K=0 pcount=18 fcount=30
cfg_loader_load_ini: START flag=1
cfg_loader_load_ini: File_Config_Read done
cfg_loader_load_ini: Dshm_Config_Read done
cfg_loader_load_ini: Tcp1_Config_Read done
cfg_loader_load_ini: Tcp2_Config_Read done
cfg_loader_load_ini: Udpip_Config_Read done
cfg_loader_load_ini: calling Proc_Config_Read ...
start PA proc config ... pcnt=0 dflag=1 pcount=18 fcount=30
proc(A) PROC_COUNT=18 OK (max=18)
proc(A) Proc_End cnt=1 id=pa_1101_ts
...
cfg_loader_load_ini: Proc_Config_Read done
```

### 크래시 시 분석 방법

로그가 어디서 멈추는지에 따라 원인 특정:

| 마지막 로그 | 의미 | 다음 조치 |
|------------|------|----------|
| `02. INFO->DAEMON copy done` 없음 | Mem_SHM_Creat 또는 Check_Work_Dir 내 크래시 | 해당 함수 디버깅 |
| `cfg_loader_load_ini: START` 후 멈춤 | File_Config_Read 크래시 | file.ini 확인 |
| `calling Proc_Config_Read ...` 후 멈춤 | Proc_Config_Read 진입 직후 크래시 | proc.ini 첫 라인 확인 |
| `start PA proc config ...` 후 멈춤 | 프로세스 파싱 중 크래시 | pcnt/fcount 값이 0이면 DAEMON 미초기화 |
| `Proc_End cnt=N` 후 멈춤 | N번째 프로세스 파싱 후 크래시 | proc.ini의 N+1번째 프로세스 확인 |
