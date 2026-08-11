# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 프로젝트 개요

KRX(한국거래소) 대상 FEP(Front-End Processor). 순수 C(ANSI/C89)로 작성된 멀티 프로세스, 데몬 기반 실시간 트레이딩 인프라 시스템. 채권·파생·주식의 주문 전송, 응답/체결 수신, 시세 수신, 자동매매 전략, 포지션 관리를 담당한다.

대상 플랫폼: HP-UX, SunOS, AIX, Linux. 운영 호스트: `podm11`(REAL1), `podm12`(REAL2), 그 외는 TEST로 동작.

## 빌드 시스템

먼저 `st01/env/pkg_env.sh`를 source하여 환경을 설정해야 한다(보통 `.bash_profile`에서 자동 source). 이 스크립트가 `_FEP_HOME`, `_FEP_SYSTEM`, `_FEP_SUBDIR` 및 빌드/운영 스크립트가 사용하는 모든 경로 변수와 alias를 정의한다.

### 빌드 명령 (`st01/shl/mk.sh`)

```sh
mk.sh all           # 전체 재빌드: 오브젝트/라이브러리 삭제 → 라이브러리 재빌드 → 전체 모듈(A B W X Z)
mk.sh sub           # 공유 라이브러리만 재빌드 (libfepP.a)
mk.sh src           # 전체 소스 모듈(PA, PB, PX, PZ) 병렬 재빌드
mk.sh pb            # 특정 모듈만 재빌드 (PB)
mk.sh pa ts         # 모듈 내 특정 프로세스 타입만 재빌드 (PA 송신 프로세스)
mk.sh pa us         # PA UDP 송신 프로세스만 재빌드
```

### 빌드 체인

1. **라이브러리**: `sub/*.c` → 개별 컴파일 → `lib/libfepP.a`로 아카이브
   - Makefile: `make/SUB/Make_Lib_P_c.mk`, 구동 스크립트: `make/SUB/Make_Lib_P_c.sh`
   - `dbipc.c`는 제외
2. **실행 파일**: `src/{PA,PB,PX,PZ}/*.c` → `libfepP.a`와 링크 → `bin/`에 출력
   - 각 모듈의 진입점은 `make/{module}/Make_{module}.mk`
   - 타입별 셸 스크립트(`Make_PB_ts.sh`, `Make_PB_tr.sh` 등)가 프로세스 ID를 순회하며 `-D` define으로 빌드
   - OS별 컴파일러/링커 플래그는 `$osname`으로 선택 (HP-UX, SunOS, AIX, Linux)
   - Linux: `-ltirpc -lm -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE` 사용

### 주요 컴파일 Define

- `-DA{NNNN}` / `-DB{NNNN}` — 프로세스 ID 번호 (예: PA 주문 송신은 `-DA1101`, PB 채권 주문 송신은 `-DB1101`)
- `-DNO_INISAFE` — INISAFE-Net 라이브러리 없이 빌드 (`pkg_env.sh`에서 기본 활성화)
- `-DSAM_USE` — SAM 파일 처리 활성화
- `-DHOLIDAY_CHECK` / `-DHOLIDAY_APPLY` — 휴일 날짜 처리
- `-D_REENTRANT` — 스레드 안전 코드
- `-D_TR_IMPROVE` — 거래 개선 기능

### 빌드 시 주의사항 (Pitfalls)

- **`make` 타임스탬프 스킵**: 서버로 파일 동기화(scp/rsync) 시 타임스탬프 때문에 재컴파일이 안 될 수 있음. 해결: `rm -f obj/SUB/*.o && mk.sh sub`로 라이브러리 강제 재빌드.
- **Linux 링커**: 모든 모듈은 `-ltirpc`를 사용해야 함 (`-lnsl` 아님). `-lnsl`은 최신 glibc에서 제거됨.
- **`// 주석 {` 버그**: `//` 주석 안에 `{`를 절대 넣지 말 것 — 컴파일러가 중괄호를 인식하지 못해 `if`/`switch` 블록이 깨짐. `if (...) // comment {` 대신 `if (...) { // comment` 사용.

## 테스트

### 단위 테스트 (`st01/test/unit/`, Unity 프레임워크)

변환/유틸 함수 대상 단위 테스트 (`atodf`, `atoif`, `atolf`, `itoaf`, `ltou`, `utol`, `chk_kor`, `ip_format`, `bsearch` 등). HP-UX, SunOS, AIX, Linux, macOS(Darwin) 지원.

```sh
cd st01/test/unit
make              # 빌드 + 전체 테스트 실행
make build        # 컴파일만
make run          # 전체 테스트 바이너리 실행
make clean        # obj/, bin/ 정리
bin/test_atoif    # 단일 테스트 실행 (build 후 개별 바이너리 직접 실행)
```

### 통합 테스트 (`st01/test/integ/`)

Mock KRX 서버 기반 PB 통합 테스트. 3단계 구성:

```sh
cd st01/test/integ
make              # 전체 phase 빌드
make run1         # Phase 1: KRX 프로토콜 + TCP 레이어 테스트 (mock 서버 자동 기동)
make run2         # Phase 2: FIFO I/O 테스트
sh run_pb_integ.sh  # Phase 3: 실제 PB 바이너리 E2E 테스트 (Linux 서버 전용 — SHM/FIFO/데몬 인프라 필요)
```

### Config-DB 테스트 빌드 (`st01/test/Makefile`)

```sh
cd st01/test
make              # 전체 테스트: 빌드 + import + export + 라운드트립 검증
make build        # ini2db, db2ini만 컴파일
make import       # INI → SQLite DB
make export       # DB → INI 파일
make verify       # 라운드트립 비교 (INI 10개 + IP 파일 2개)
make dbcheck      # DB 스키마, 키 개수, 환경 목록 확인
make dryrun       # 파싱만 수행, DB 미기록
make cfgverify    # cfg_verify 빌드 (서버 전용, libfepP 필요)
make install      # ini2db, db2ini를 st01/bin/에 복사
make clean        # 테스트 산출물 전체 삭제
make ENV_ID=REAL1 # 특정 환경으로 테스트
```

원격 배포: `./deploy_test.sh fepp@10.x.x.x` (macOS → 서버) 또는 `./deploy_test.sh local` (서버에서 직접).

## 환경 변수

`st01/env/pkg_env.sh`에서 호스트명 기반 자동 감지로 설정:

| 변수 | 값 | 용도 |
|------|-----|------|
| `_FEP_DIV` | `REAL1`/`REAL2`/`TEST` | 환경 구분자 (podm11→REAL1, podm12→REAL2, 그 외→TEST) |
| `_FEP_HOME` | `$HOME/fep` | FEP 기본 디렉토리 |
| `_FEP_SYSTEM` | `"p"` | 시스템 ID (라이브러리 접미사 선택: libfep**P**.a) |
| `_FEP_SUBDIR` | `"A B W X Z"` | 모듈 문자 (PA, PB, PW, PX, PZ) |
| `NO_INISAFE` | `1` | INISAFE-Net 암호화 비활성화 (기본 on) |
| `osname` | `uname -s` | 컴파일러 플래그용 OS 감지 |
| `_P_BIN`, `_P_CFG`, `_P_INC`, `_P_LIB`, `_P_UTL` | 동적 | 모듈별 경로 변수 |
| `_HA_PEER_IP` | 상대 서버 IP | HA 이중화 하트비트 상대 주소 |
| `_PA_HA_HB_PORT` | 포트 번호 | PA HA 하트비트 포트 오버라이드 |

셸 alias: `pbin`, `pcfg`, `pinc`, `plib`, `pshl`, `putl`, `psub`, `penv`, `psrc`, `pobj`, `pmake` — 모듈 디렉토리로 즉시 `cd`. `pbsrc`, `pbobj`, `pblog` 같은 모듈별 alias도 존재.

## 디렉토리 구조 (`st01/`)

| 디렉토리 | 용도 |
|-----------|------|
| `bin/` | 컴파일된 프로세스 바이너리 약 66개 |
| `cfg/` | INI 설정 파일 (데몬, 프로세스, TCP, UDP, 파일, SHM 정의) |
| `env/` | 환경 설정 (`pkg_env.sh`, `.bash_profile` 템플릿, `.vimrc`) |
| `inc/` | 공유 헤더 파일 25개 |
| `lib/` | 정적 라이브러리 `libfepP.a` |
| `make/` | 모듈별 Makefile 및 빌드 스크립트 (`PA/`, `PB/`, `PX/`, `PZ/`, `SUB/`) |
| `obj/` | 컴파일된 오브젝트 파일 (`src/` 구조 미러링) |
| `shl/` | 운영 셸 스크립트 53개 (빌드, 모니터링, 기동, 유지보수) |
| `src/` | 모듈별 C 소스 (`PA/`, `PB/`, `PX/`, `PZ/`) |
| `sub/` | 공통 서브루틴 C 소스 52개 (`libfepP.a`로 컴파일) |
| `utl/` | 유틸리티/마이그레이션 도구 (`ini2db.c`, `db2ini.c`, `cfg_verify.c`) + 테스트/샘플 프로그램 |
| `test/` | 테스트 인프라 (`unit/`, `integ/`, Config-DB Makefile, `deploy_test.sh`) |

참고: `OB/`, `OW/`, `OX/`, `OZ/` 디렉토리는 존재하지만 비어있거나 레거시.

### 런타임 디렉토리 (`st02/`, `st03/`)

| 디렉토리 | 용도 |
|-----------|------|
| `st02/FIFO/{PA,PB,PX,PZ}` | 프로세스 간 통신용 네임드 파이프 |
| `st02/DAT/{PA,PB,PX,PZ}` | 런타임 데이터 파일 |
| `st03/LOG/{PA,PB,PX,PZ}` | 프로세스 로그 파일 |
| `st03/SEQ/` | 시퀀스 파일 (BATCH, DISC, TOTAL) |

시스템 기동 전에 반드시 존재해야 함. `mkdir -p`로 수동 생성.

## 아키텍처

### 모듈 구성

- **PA** — 트레이딩 로직: 주문 송수신, 체결, 자동매매 전략, 시세, 클라이언트 통신, 한도
- **PB** — 채권 전용 KRX FEP: 채권 주문 및 시세용 거래소 직접 연결
- **PW** — 데몬 및 연결 관리 (소스 5개: daemon mp, connection TR, TS)
- **PX** — 유틸리티/모니터링 프로세스 (설정 로딩, 프로세스 체크, 상태 표시)
- **PZ** — 시스템 관리: 데몬 제어, 메모리 관리, 로그 컴팩션, 프로세스 체크

### 프로세스 명명 규칙

형식: `p{module}_{NNNN}_{type}`

- 모듈: `a`=트레이딩, `b`=채권, `x`=유틸리티, `z`=관리
- NNNN: 4자리 기능 코드
  - 1xxx: 채권/KRX 직접 연결 (1101=주문 송신, 1201=응답 수신, 1401=체결)
  - 2xxx: 파생상품 (2101=주문 송신, 2201=응답 수신)
  - 5xxx: 자동매매 전략
  - 6xxx: 조회 처리
  - 7xxx: 시세 수신/분배
  - 8xxx: 클라이언트 통신
  - 9xxx: 한도/리스크 관리
- 타입: `mp`=매니저, `ts`=TCP 송신, `tr`=TCP 수신, `ur`=UDP 수신, `dd`=데이터 분배, `qr`=조회 요청, `qs`=조회 서비스, `us`=UDP 송신

### IPC 메커니즘

- **공유 메모리(SHM)**: 주 데이터 교환 수단 — 계좌 정보, 주문 상태, 시세, 전략. 레이아웃은 `inc/shm_memory.h`(60KB 헤더)에 정의. 최대 계좌 20개, 미체결 주문 10,000건, 자동매매 프로세스 40개.
- **FIFO/네임드 파이프**: 프로세스 간 메시지 전달 (`cfg/file.ini`에 정의)
- **세마포어**: 프로세스 동기화 (`sub/semipc.c`)
- **TCP/IP**: KRX 및 클라이언트 직접 연결 (`cfg/tcp1.ini`, `cfg/tcp2.ini`에 설정)
- **UDP/IP**: 시세 멀티캐스트 수신 (`cfg/udpip.ini`에 설정)

### HA 이중화 (Active/Standby 페일오버)

REAL1/REAL2 두 운영 서버가 UDP 시세를 동시에 수신하되, Primary만 하류로 중계한다. UDP 하트비트 기반 자동 절체:

- **PB-HA** (`pb_7100_ur`): TCP 중계 이중화 — Primary만 TCP 전송
- **PA-HA** (`pa_7100_ur`, 5개 변형: A7102/A7103/A7201/A7202/A7203): SHM 기록 + FIFO 중계 이중화
- **역할 결정**: `_FEP_DIV` 환경 변수 (REAL1→Primary, REAL2→Secondary, TEST→Standalone)
- **하트비트**: UDP 포트 50010~50014 (변형별 분리), 송신 1초 간격, 수신 타임아웃 5초
- **절체 조건**: 하트비트 5초 미수신, 또는 Primary 데이터 5회 연속 미수신 → Secondary 활성화
- **복귀(Failback)**: 하트비트 3회 연속 수신 + Primary 데이터 확인 시 Secondary 대기 복귀. Primary가 살아있지만 데이터가 없고 Secondary는 수신 중이면 복귀 차단(오복귀 방지)
- **Graceful Fallback**: `HA_Init` 실패 시 STANDALONE 모드(항상 활성, HA 없음)
- **네트워크 설정**: `_HA_PEER_IP`, `_PA_HA_HB_PORT` 환경 변수

### KRX 메시지 구조

모든 KRX 주문/체결 메시지는 `inc/pa_struct.h`에 정의된 공통 헤더 레이아웃을 공유:

```
KRX_MSG_COMMON (106 bytes)
├── KRX_HEADER (82 bytes)     — BeginString, BodyLength, MsgType, MsgSeqNum, SenderCompID, SendingTime, ...
└── KRX_BODY_COMMON (24 bytes) — DataSeq[11], Transaction_Code[11], Megrp_no[2]
```

`DataBuff[82]`, `DataBuff[KRX_HEAD_LEN+11]` 같은 하드코딩 바이트 오프셋 대신 `KRX_MSG_COMMON` 구조체 기반 필드 접근을 사용할 것. 상수: `KRX_HEAD_LEN = sizeof(KRX_HEADER) = 82`.

### 설정 시스템

`cfg/`의 INI 형식 파일:

- `daemon.ini` — 데몬 라이프사이클 (기동/종료 시각, 날짜 플래그 D/D-1/D+1, 컴팩트 일수, IPC 개수)
- `proc.ini` — 프로세스 정의 (18개 이상, TCP/UDP 포트, 기동/종료 시각, I/O 파일 매핑)
- `file.ini` — 파일/FIFO 정의 (레코드 크기 포함)
- `tcp1.ini` / `tcp2.ini` — TCP 마스터 데몬 및 직접 연결 설정
- `udpip.ini` — UDP 멀티캐스트 설정
- `dshm.ini` — 데이터 공유 메모리 정의
- `sisetr.ini` — 시세 TR 정의
- `client.ini` / `pc.ini` — IP 화이트리스트 항목

**Config-DB 마이그레이션**: `utl/ini2db.c`로 INI 파일을 SQLite DB(`fep_config.db`)에 적재 가능. `sub/config_loader.c`가 3가지 모드의 통합 로딩 제공: `AUTO`(DB 우선, INI 폴백), `DB`(DB 전용), `INI`(레거시 INI 전용).

### 공유 라이브러리 (libfepP.a) — `sub/` 구성

기능별로 정리된 소스 52개:

| 분류 | 파일 | 용도 |
|------|------|------|
| Config/DB | `config_db.c`, `config_loader.c` | SQLite 설정 로딩, INI/DB 통합 접근 |
| 공통/중복제거 | `fep_common.c`, `fep_encrypt.c`, `fifo_event.c`, `device_rw.c`, `sock_linger.c` | PA/PB 공유 이벤트 루프 함수, INISAFE 암호화, FIFO 핸들러, 디바이스 I/O, 소켓 linger |
| SHM | `shmipc.c`, `shmsub.c`, `shm_rw.c` | 공유 메모리 생성/attach/읽기/쓰기 |
| TCP/IP | `tcpip_{accept,bind,connect,listen,recv,select,send,sock}.c`, `udp_init.c` | 네트워크 소켓 계층 |
| 파일 I/O | `file_rw.c`, `file_util.c`, `poll_file.c`, `poll_event.c`, `select_{recv,send}.c` | FIFO/파일 연산 |
| 프로세스 | `make_daemon.c`, `init_proc.c`, `init_mana.c`, `check_proc.c`, `check_env.c`, `check_exist.c` | 프로세스 라이프사이클 |
| 로깅 | `log_proc.c`, `stat_save.c`, `seq_save.c` | 감사 및 상태 추적 |
| 변환 | `atodf.c`, `atolf.c`, `atoif.c`, `itoaf.c` | 숫자/문자열 변환 |
| 유틸리티 | `key_search.c`, `queue.c`, `gettime.c`, `getfileno.c`, `getenvironment.c`, `set_trtime.c`, `ltou.c`, `utol.c`, `chk_kor.c`, `hoga_check.c`, `display_errmsg.c`, `setsigfatal.c`, `semipc.c`, `ip_format.c` | 기타 유틸리티 |

### 핵심 헤더 파일

| 파일 | 크기 | 역할 |
|------|------|------|
| `inc/krx_mk.h` | 98KB | KRX 시세 구조체 (채권, 파생, 주식) |
| `inc/shm_memory.h` | 60KB | 공유 메모리 레이아웃, 계좌 관리, 주문 번호 범위 |
| `inc/pa_struct.h` | 69KB | PA 모듈 전체 데이터 구조체, KRX_MSG_COMMON |
| `inc/fep_interface.h` | ~7KB | 프로토콜 상수, 핸드셰이크 코드, 상태 정의 |
| `inc/def_error.h` | 5KB | 에러 코드 정의 |
| `inc/strategy.h` | 6KB | 자동매매 전략 정의 |
| `inc/fep_common.h` | ~2KB | PA/PB 공통 이벤트 루프 extern 및 프로토타입 |
| `inc/fep_encrypt.h` | ~1KB | INISAFE-Net 암호화 extern 및 프로토타입 (PB 전용) |

### PA/PB 중복제거 아키텍처

PA(트레이딩)와 PB(채권) 모듈은 거의 동일한 이벤트 루프 패턴을 공유한다. 공통 함수는 공유 라이브러리로 추출됨:

- **`fep_common.c`** — PA/PB 공유 핵심 이벤트 루프 함수: `Get_Msec`, `Line_Change`, `Device_Close_Base`, `Err_Msg`, `Log_Out_Base`, `Device_Open_Logon`, `Time_Out_Disconnect`
- **`device_rw.c`** — `Device_Read`, `Device_Write` (자체 정의를 가진 프로세스(`pb_8100_ts`, `pb_1800_ts` 등)와의 다중 정의 충돌 방지를 위해 `fep_common.c`에서 분리)
- **`sock_linger.c`** — `Set_Socket_Linger` (`device_rw.c`와 같은 이유로 분리)
- **`fep_encrypt.c`** — PB 전용 INISAFE 암호화: `Free_All`, `Handshake` (PB는 암호화된 KRX 연결 사용, PA는 스텁 `Handshake` 사용)
- **`fifo_event.c`** — `Fifo_Event_Rtn` 격리 (FIFO 핸들러만 필요한 프로세스가 `fep_common.o` 의존성을 끌어오지 않도록 별도 .o)

**핵심 패턴**: `extern void *FmtPtr`가 PA의 `S_Fmt`와 PB의 `KR_Fmt` 구조체 차이를 해소 — 각 프로세스가 자신의 포맷 구조체를 `FmtPtr`에 할당하고, 공통 함수는 캐스트로 사용. 프로세스별 동작은 얇은 래퍼로 처리 (예: `Device_Close`는 `Device_Close_Base` 호출 후 PB 전용 정리 수행).

## 시스템 기동 순서

기동 순서가 중요 — 프로세스들이 앞선 프로세스가 생성한 공유 자원에 의존한다:

1. **`pz_memory_mp`** (최우선) — `Create_FIFO()`로 공유 메모리 세그먼트와 FIFO 네임드 파이프 생성
2. **`pa_daemon_mp`** / **`pb_daemon_mp`** — FIFO를 열고 `execl()`로 자식 프로세스 기동
3. 자식 프로세스 (`*_ts`, `*_tr`, `*_ur`, `*_us` 등) — 부모 데몬이 기동

`pa_daemon_mp`가 "cannot open FIFO"로 실패하면 `pz_memory_mp`가 아직 실행되지 않았거나 런타임 디렉토리가 없는 것.

## 운영 스크립트 (`shl/`)

| 스크립트 | 용도 |
|----------|------|
| `mk.sh` / `mkall.sh` | 빌드 오케스트레이션 |
| `master.sh` | 마스터 데몬 기동 |
| `initA0.sh` | 시스템 초기화 (_dd 프로세스 리셋) |
| `ps.sh` | 프로세스 상태 표시 |
| `chkproc.sh` | 프로세스 헬스 체크 |
| `chktcp1.sh` / `chktcp2.sh` / `chkudp.sh` | 네트워크 연결 확인 |
| `chkshm.sh` / `ipcs.sh` | 공유 메모리/IPC 상태 |
| `compact.sh` | 오래된 데이터/로그 정리 |
| `cfgload.sh` / `cfgback.sh` | 설정 로드/백업 |
| `ipcrm.sh` | IPC 자원 정리 (파괴적 — 주의) |
| `setpstat.sh` / `setfcnt.sh` | 프로세스 상태 및 파일 카운터 조작 |

## 문서

`docs/`의 참조 문서:

| 파일 | 용도 |
|------|------|
| `FEP_Architecture_Analysis.md` | 시스템 아키텍처 심층 분석 (4개 모듈, IPC, 보안) |
| `FEP_Macro_Reference.md` | 전체 매크로 레퍼런스 (SHM, 에러 코드, TCP 플래그, KRX 메시지) |
| `PA_Architecture.md` / `PB_Architecture.md` | 모듈별 아키텍처 + 데이터 흐름도 (`PA_*.mermaid`, `PB_*.mermaid`) |
| `04-report/changelog.md` | PDCA 지표 포함 프로젝트 산출물 이력 |
| `archive/2026-02/_INDEX.md` | 아카이브된 기능 인덱스 (완료 기능 42건, PDCA 문서 포함) |
| `archive/2026-03/_INDEX.md` | 아카이브된 기능 인덱스 (2026년 3월: 통합 테스트, PB-HA, PA-HA 등) |

PDCA 문서 구조: `docs/01-plan/`, `02-design/`, `03-analysis/`, `04-report/`. 완료된 기능은 `docs/archive/YYYY-MM/{feature}/`로 아카이브.

## 보안

KRX 암호화 연결에 INISAFE-Net 라이브러리(v7.2.47, 64비트) 사용. 라이브러리 위치: `~/fep/krx/INISAFE_Net_for_C_v7.2.47_64/`. include 경로와 `LD_LIBRARY_PATH`는 `pkg_env.sh`에서 설정. 라이브러리가 없는 환경을 위해 `pkg_env.sh`에서 `NO_INISAFE=1`이 기본 export되며, PB 소스의 모든 암호화 호출은 `#ifndef NO_INISAFE` 가드로 감싸져 있다.

## 코딩 컨벤션

- **언어**: 순수 C89/ANSI C — C++ 기능 사용 금지
- **탭 폭**: 4칸 (`.vimrc`에 설정)
- **구조체 명명**: `*_FMT`, `*_S` 접미사
- **SHM 변수**: 타입은 `SHM_*` 접두사, C 변수는 `Shm_*`
- **주석**: 한글/영문 혼용
- **소스 백업**: 활성 소스 옆에 `BACK2025/`, `JC_OLD/` 디렉토리; 구버전은 `.org`, `.back` 접미사
- **하나의 소스 → 여러 바이너리**: 단일 `.c` 파일이 `-D` define으로 여러 바이너리 생성 (예: `pb_1100_ts.c`가 `-DB1101`로 `pb_1101_ts` 빌드)
- **KRX 메시지 접근**: raw 바이트 오프셋(`DataBuff[82]`)이 아닌 `KRX_MSG_COMMON` 구조체 사용
- **KRX vs IMECO 구조체**: PA 2xxx 프로세스(파생)는 IMECO 프로토콜을 사용하며 구조체 레이아웃이 다름 (예: `DataSeq` 필드 없음, `int ErrCd` 대신 `char ErrCd[8]`). `fep_common.h` 같은 공유 헤더는 IMECO 프로세스와 충돌하는 타입을 선언하면 안 됨 — 프로세스별 변형 extern은 `fep_common.c`에 직접 둔다.
- **구조화 로깅**: `FEP_LOG_FORMAT=structured` 환경 변수 설정 시 파이프 구분 로그 출력 (기본 비활성)
