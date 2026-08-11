# FEP System Architecture Analysis Report

| 항목 | 내용 |
|------|------|
| 작성일 | 2026-02-18 |
| 대상 시스템 | FEP (Front-End Processor) st01 모듈 |
| 분석 범위 | 아키텍처, 보안/안정성 개선, 성능 최적화 |

---

## 1. 시스템 개요

### 1.1 목적

KRX(한국거래소)와 증권사 내부 시스템 사이의 **실시간 주문/체결/시세 데이터 중계** 미들웨어.
채권, 금융파생상품의 주문 송수신, 시세 수신, 자동매매 전략 실행, 한도관리를 처리한다.

### 1.2 기술 스택

| 구분 | 기술 |
|------|------|
| 언어 | C (ANSI C89) |
| 아키텍처 | 멀티프로세스 데몬 기반 |
| 플랫폼 | HP-UX, SunOS, AIX, Linux |
| IPC | 공유메모리, FIFO, 세마포어, 메시지큐 |
| 네트워크 | TCP/IP (주문/응답), UDP/IP (시세 멀티캐스트) |
| 보안 | INISAFE-Net v7.2.47 (64bit) - KRX 표준 암호화 |
| 빌드 | GNU Make + Shell Scripts |

### 1.3 운영 환경

| 호스트 | 용도 |
|--------|------|
| podm11 | REAL1 (운영 1번 서버) |
| podm12 | REAL2 (운영 2번 서버) |
| 기타 | TEST (테스트 환경) |

---

## 2. 아키텍처

### 2.1 전체 구조

```
┌─────────────────────────────────────────────────────────────┐
│                        KRX (한국거래소)                        │
│              TCP/IP (INISAFE-Net 암호화)  /  UDP              │
└──────┬──────────────┬──────────────┬──────────────┬──────────┘
       │주문/응답      │체결           │시세(UDP)      │RDS일괄
       ▼              ▼              ▼              ▼
┌──────────────────────────────────────────────────────────────┐
│                     FEP 프로세스군 (st01)                      │
│                                                              │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐        │
│  │ PB 모듈  │  │ PA 모듈  │  │ PX 모듈  │  │ PZ 모듈  │        │
│  │ 채권FEP  │  │ 거래처리  │  │ 유틸리티  │  │ 시스템관리│        │
│  └────┬─────┘  └────┬─────┘  └─────────┘  └─────────┘        │
│       │             │                                        │
│       └──────┬──────┘                                        │
│              ▼                                               │
│       ┌─────────────┐                                        │
│       │  공유메모리   │  ← 계좌, 주문, 시세, 전략 데이터         │
│       │  (SHM)      │                                        │
│       └──────┬──────┘                                        │
│              │ FIFO                                           │
└──────────────┼───────────────────────────────────────────────┘
               ▼
        ┌─────────────┐
        │  Client 시스템 │  (자동매매 API, 조회 클라이언트 등)
        └─────────────┘
```

### 2.2 디렉토리 구조

```
st01/
├── bin/     75개 프로세스 바이너리
├── cfg/     INI 설정 파일 (daemon, process, TCP, UDP, file, SHM)
├── env/     환경 설정 (pkg_env.sh, .bash_profile, .vimrc)
├── inc/     29개 공유 헤더 파일
├── lib/     정적 라이브러리 libfepP.a
├── make/    모듈별 Makefile 및 빌드 스크립트
├── obj/     컴파일된 오브젝트 파일
├── shl/     53개 운영 셸 스크립트
├── src/     모듈별 C 소스 (PA, PB, PX, PZ)
├── sub/     공유 라이브러리 소스 (41개 → libfepP.a)
└── utl/     테스트/유틸리티 프로그램
```

### 2.3 4개 모듈 상세

#### PA (Process A) - 거래 처리 엔진 (32개 소스)

시스템의 핵심. 모든 시장(채권/파생/현물)의 주문, 체결, 시세, 전략을 처리.

| 기능번호 | 프로세스 | 역할 |
|----------|---------|------|
| 1xxx | pa_1100_ts, pa_1200_tr/mp | 채권 주문송신, 응답/체결 수신 |
| 2xxx | pa_2100_ts, pa_2200_tr | 금융파생 주문송신, 응답 수신 |
| 5xxx | pa_5010_mp, pa_5020_mp | 자동매매 전략 실행 (선물/옵션 행사가 기반) |
| 6xxx | pa_6000_mp | 조회 처리 (잔고, 미체결, 시세, 손익) |
| 7xxx | pa_7000~7800 | 시세 UDP수신/SHM적재/배분 |
| 8xxx | pa_8100_ts, pa_8200_tr | Client 통신 (외부 API 연동) |
| 9xxx | pa_9000_mp | 한도관리 및 전략 기동/종료 제어 |

#### PB (Process B) - 채권 KRX 직접접속 FEP (12개 소스)

KRX의 채권(KTS) 전용 회선에 직접 연결하는 프로세스.

| 프로세스 | 역할 | 포트 |
|---------|------|------|
| pb_1101_ts | 채권 주문 KRX 송신 | TCP 37221 |
| pb_1201_tr | 채권 응답/체결 KRX 수신 | TCP 57221 |
| pb_1211_tr | 체결 DropCopy 수신 | TCP 61726 |
| pb_1601_tr | 공개장운영정보 수신 | TCP 61725 |
| pb_7102_ur | 채권 KTS 시세 UDP 수신 | UDP 10402 |
| pb_7801_tr | RDS 일괄데이터 수신 | TCP 63725 |
| pb_1301_tr | 내부 주문수신 중계 | TCP 41001 |
| pb_1401~1411_ts | 응답/체결/RDS 내부 전달 | TCP 41002~41005 |

#### PX (Process X) - 유틸리티 (51개 소스)

운영/모니터링 도구. `px_chkproc`(프로세스 체크), `px_chktcp`(네트워크 체크), `px_chkshm`(SHM 확인), `px_cfgload/cfgback`(설정 관리), `px_orderchk`(주문 정합성), `px_set*`(SHM 값 설정).

#### PZ (Process Z) - 시스템 관리 (10개 소스)

`pz_daemon`(데몬 마스터), `pz_memory`(SHM 생성/관리), `pz_compact`(데이터 정리), `pz_procchk`(프로세스 점검).

### 2.4 공유 라이브러리 (sub/ -> libfepP.a)

| 분류 | 파일 | 설명 |
|------|------|------|
| IPC | shmipc.c, shmsub.c, shm_rw.c, semipc.c, queue.c | SHM/세마포어/메시지큐 |
| 네트워크 | tcpip_*.c (7개), select_recv/send.c | TCP 소켓 전체 라이프사이클 |
| 파일 I/O | file_rw.c, poll_file.c | FIFO/파일 읽기쓰기 |
| 프로세스 | init_proc.c, init_mana.c, check_proc.c, make_daemon.c | 초기화/데몬화/상태체크 |
| 로깅 | log_proc.c, stat_save.c, seq_save.c | 로그, 통계, 시퀀스 |
| 데이터 | key_search.c, hoga_check.c, gettime.c | 종목검색, 호가검증, 시간 |
| 변환 | itoaf.c, atoif.c, atolf.c, dtoaf.c, ltou.c, utol.c | 숫자/문자열 변환 |

### 2.5 프로세스 명명 규칙

형식: `p{모듈}_{NNNN}_{타입}`

- **모듈**: a=거래, b=채권, x=유틸리티, z=관리
- **NNNN**: 4자리 기능 코드
- **타입**: mp=매니저, ts=TCP송신, tr=TCP수신, ur=UDP수신, dd=데이터배분, qr=조회요청, qs=조회서비스, us=UDP송신

### 2.6 IPC 구조

```
┌───────────────────────────────────────────────────┐
│              공유메모리 (SHM)                        │
│  ┌──────────┬──────────┬──────────┬──────────┐     │
│  │계좌정보   │주문/미체결 │시세데이터  │자동전략   │     │
│  │(20계좌)  │(10,000건) │(종목별)   │(40프로세스)│     │
│  └──────────┴──────────┴──────────┴──────────┘     │
│  shm_memory.h (60KB)에 전체 레이아웃 정의             │
└──────────┬──────────────────────────┬──────────────┘
           │                          │
    ┌──────▼──────┐           ┌───────▼───────┐
    │ FIFO (명명파이프)│           │ 세마포어        │
    │ 이벤트 전달    │           │ 동시접근 제어     │
    └─────────────┘           └───────────────┘
```

### 2.7 데이터 흐름

#### 주문 흐름 (송신)

```
Client(자동매매 API)
  → pa_8200_tr (수신)
  → FIFO
  → pa_1100_ts / pa_2100_ts (주문생성)
  → pb_1101_ts (KRX 채권회선) 또는 직접 KRX TCP
  → KRX 거래소
```

#### 체결 흐름 (수신)

```
KRX
  → pb_1201_tr (체결수신)
  → 파일/FIFO
  → pa_1200_mp (응답처리)
  → pa_1400_mp (체결처리)
  → SHM 업데이트
  → pa_8100_ts
  → Client 전달
```

#### 시세 흐름

```
KRX
  → pb_7102_ur (UDP 멀티캐스트 수신)
  → SHM 메모리 적재
  → pa_7xxx_dd (배분)
  → 자동전략에 FIFO 통보 / 한도체크 평가손익 계산
```

#### 자동매매 전략 흐름

```
Client → pa_9000_mp (전략기동 TR:500110)
  → pa_5010_mp (전략실행)
  → 시세 변동 감지
  → 주문 생성 → pa_1100_ts로 전달
  → pa_9000_mp (전략종료 TR:500120) / 강제종료(TR:500310)
```

### 2.8 설정 체계

| 파일 | 용도 |
|------|------|
| daemon.ini | 데몬 라이프사이클 (기동/종료 시간, 날짜 플래그 D/D-1/D+1, 데이터 보존일, IPC 수) |
| proc.ini | 프로세스 정의 (18+개, 유형/TCP/UDP 포트, 기동 시간, LogonID/PW, I/O 매핑) |
| file.ini | 파일/FIFO 정의 (레코드 크기) |
| tcp1.ini | TCP 마스터 데몬 설정 |
| tcp2.ini | TCP 직접접속 설정 |
| udpip.ini | UDP 멀티캐스트 설정 |
| dshm.ini | 데이터 공유메모리 정의 |
| sisetr.ini | 시세 TR 정의 |

### 2.9 KRX 프로토콜

| TR코드 | 서비스 | 길이 |
|--------|--------|------|
| TCHODR10001 | 신규호가 입력 | 261 |
| TCHODR10002 | 정정호가 | 261 |
| TCHODR10003 | 취소호가 | 261 |
| TTRODP11301 | 회원처리호가 (정상) | 287 |
| TTRODP11321 | 거부 | 287 |
| TTRTDP21301 | 체결결과 | 242 |

세션 관리: `LINK`(접속요구) -> `LIOK`(접속응답) -> `LIVE`(HeartBeat) -> `STOP`(종료)

### 2.10 빌드 특성: 하나의 소스 -> 다수의 바이너리

하나의 `.c` 파일에서 `#ifdef`를 통해 여러 바이너리를 생성한다.

예시: `pb_1200_tr.c`

```c
#if defined(B1201) || defined(B1211)
#define DATA_SIZE 400     // 체결수신용
#elif defined B1601
#define DATA_SIZE 200     // 장운영정보용
#endif
```

빌드 시 `-DB1201`, `-DB1211`, `-DB1601`을 각각 적용하여 `pb_1201_tr`, `pb_1211_tr`, `pb_1601_tr` 세 개의 바이너리를 생성. `bin/`에 75개 바이너리가 있지만 실제 소스는 훨씬 적은 이유.

### 2.11 핵심 헤더 파일

| 파일 | 크기 | 역할 |
|------|------|------|
| inc/krx_mk.h | 98KB | KRX 시장 데이터 구조체 (채권, 파생, 현물) |
| inc/shm_memory.h | 60KB | 공유메모리 레이아웃, 계좌관리, 주문번호 영역 |
| inc/pa_struct.h | 69KB | PA 모듈 전체 데이터 구조체 |
| inc/fep_interface.h | ~7KB | 프로토콜 상수, 핸드셰이크 코드, 상태 정의 |
| inc/def_error.h | 5KB | 에러 코드 정의 |
| inc/strategy.h | 6KB | 자동매매 전략 정의 |

---

## 3. 보안/안정성 개선 사항

### 3.1 [긴급] 네트워크 수신 버퍼 길이 검증 미흡 — ✅ 완료 (select-recv-bounds-check #30)

**위치**: `sub/select_recv.c:67-73`

**현재 코드**:
```c
pkt_len = AtoIf (p_recv+1, 4);     // 네트워크에서 받은 길이 값
// pkt_len 상한 검사 없음
rt = Recvn (p_sfd, p_recv+5, pkt_len);  // 버퍼 크기 검증 없이 수신
```

**문제**: 네트워크에서 수신한 길이 값을 검증 없이 `Recvn`에 전달. 악의적이거나 오류 패킷으로 인한 버퍼 오버플로우 가능.

**개선안**: `pkt_len`에 대한 상한 검사 추가.
```c
if (pkt_len < TCP_HEAD_LEN - 5 || pkt_len > RECV_BUF_MAX - 5) {
    Log (USR_ERROR, "Select_Receive:invalid Length[%d]", pkt_len);
    return (NOTOK);
}
```

**위험도**: 높음 | **난이도**: 낮음

---

### 3.2 [긴급] 설정 파일 내 인증정보 평문 노출

**위치**: `cfg/proc.ini:74-76`

**현재 설정**:
```ini
Proc_4_LogonID=M60501O001
Proc_4_LogonPW=TTYwNTAxTzAwMQo=O001    # Base64 인코딩 (암호화 아님)
```

**문제**: Base64는 암호화가 아니다. 파일 접근 권한만 얻으면 KRX 인증정보가 즉시 노출.

**개선안**: 별도의 암호화된 크레덴셜 저장소(vault) 사용 또는 최소한 AES 암호화 후 런타임 복호화.

**위험도**: 높음 | **난이도**: 중간

---

### 3.3 [높음] SHM 권한 설정 과다 — ✅ 완료 (0664→0600, 2026-02-28)

**위치**: `sub/shmipc.c:24`

**현재 코드**:
```c
rt = shmget (p_shmkey, p_shmsize, 0664 | IPC_CREAT);
```

**문제**: 같은 그룹의 모든 사용자가 SHM을 읽을 수 있음. 주문/계좌/전략 데이터 포함.

**개선안**: `0600`으로 제한하거나, 운영 환경에 맞춰 최소 권한 적용.

**위험도**: 중간 | **난이도**: 낮음

---

### 3.4 [높음] INISAFE-Net 암호화 코드 상태 불분명

**위치**: `src/PB/pb_1100_ts.c:86-91`, `src/PB/pb_1200_tr.c:16`

**현재 코드**:
```c
//extern net_ctx *EnCtx;
//#define CLIENT_CTX 0
// #include "INISAFENet.h"

net_ctx *EnCtx = NULL;   // 전역 선언은 되어 있음
```

**문제**: 암호화 관련 코드가 주석/활성 상태가 혼재. 실제 암호화가 적용되고 있는지 코드만으로 확인 어려움.

**개선안**: 암호화 활성/비활성 상태를 명확히 하고, 컴파일 플래그(`-DUSE_ENCRYPTION`)로 제어.

**위험도**: 중간 | **난이도**: 중간

---

### 3.5 [높음] 프로세스 장애 시 자동 복구 부재

**위치**: `src/PZ/pz_daemon.c`, `shl/chkproc.sh`

**현재 상태**: `pz_daemon`이 자식 프로세스를 기동하지만, 장애 감지는 `px_chkproc`를 수동/cron으로 실행하는 방식.

**문제**: 주문송신(`pb_1101_ts`) 프로세스가 비정상 종료 시 수동으로 감지/재기동해야 함.

**개선안**: 데몬 프로세스에 `waitpid()`/`SIGCHLD` 기반 자동 재기동 로직 추가. 또는 systemd/supervisord 연동.

**위험도**: 중간 | **난이도**: 중간

---

### 3.6 [중간] CPU busy-wait 임시 조치 — ✅ 완료 (final-perf-optimize #42)

**위치**: `src/PB/pb_1100_ts.c:179`

**현재 코드**:
```c
#if 1  /* HONG : 장종료후 CPU 부하 상승으로 임시 추가 검토해 보세요 */
    sleep(1);
#endif
```

**문제**: 장종료 후 busy-wait으로 CPU 100%를 `sleep(1)`로 임시 조치. 비효율적이며 응답성 저하.

**개선안**: poll/select의 timeout을 상황에 따라 동적으로 조절 (장중: 5ms, 장외: 5초 등).

**위험도**: 낮음 | **난이도**: 낮음

---

### 3.7 [중간] 로그 구조화 부재 — ✅ 완료 (structured-logging #44)

**위치**: `sub/log_proc.c`

**기존 포맷**:
```
[pb_1101_ts    ,09:30:15.123456,TCP,INFO]OK22 W[5] R[3]
```

**구조화 포맷** (`FEP_LOG_FORMAT=structured`):
```
2026-03-01 09:30:15.123456|pb_1101_ts|TCP|INFO|0600|OK22 W[5] R[3]
```

**해결**: 파이프(`|`) 구분자 기반 구조화 포맷 모드 추가. `is_structured_log()` 함수로 환경변수 1회 캐시 후 4곳(Log 일반/emergency, Write_SLog 일반/emergency) 분기. 기본값 legacy 모드로 하위 호환 유지. 호출자 2,010곳 변경 없음.

**위험도**: 낮음 | **난이도**: 중간

---

### 3.8 [중간] 설정 변경 시 재기동 필요

**현재 상태**: INI 파일 설정은 프로세스 기동 시 1회 로드.

**문제**: TCP 포트, 타임아웃 등 설정 변경 시 프로세스 재기동 필수. 장중 변경 불가.

**개선안**: SIGHUP 기반 설정 리로드 또는 SHM 기반 동적 설정 변경 메커니즘.

**위험도**: 낮음 | **난이도**: 중간

---

### 3.9 [낮음] 전역변수 과다 사용

**위치**: 거의 모든 소스파일

**현재 코드**:
```c
int   ConnectRetryCnt;
int   Sockfd, ErrCd, SendLen, MsgLen, RecvLen, ReTrCode;
int   PollCnt, FirstSeq, TimeOut, Pk;
char  DeviceSendFlag, LogOnFlag, OpenFlag, DataBuff[4096], IpAddr[20];
```

**문제**: 함수 간 의존성이 암묵적. 부작용 추적 어려움. 멀티스레드 확장 불가능.

**개선안**: 컨텍스트 구조체(`SESSION_CTX`)에 묶어서 함수 인자로 전달하는 패턴으로 점진적 리팩토링.

**위험도**: 낮음 | **난이도**: 높음

---

### 3.10 [낮음] 버전 관리 부재

**현재 상태**: 소스 백업이 `BACK2025/`, `JC_OLD/`, `.org`, `.back` 파일로 수동 관리.

```
src/PA/BACK2025/pa_1100_ts.c
src/PA/BACK2025/pa_1100_ts.c20220218
src/PA/BACK2025/pa_1100_ts.c20220223
src/PA/BACK2025/pa_1100_ts.org
```

**문제**: 변경 이력 추적 불가. 롤백 시 어느 버전으로 돌아가야 하는지 판단 불가.

**개선안**: Git 도입. 기존 백업 디렉토리는 초기 커밋 후 정리.

**위험도**: 중간 | **난이도**: 낮음

---

### 3.11 보안/안정성 개선 우선순위 (3/8 완료 — 2026-02-28)

| 우선순위 | 항목 | 위험도 | 난이도 | 완료 |
|---------|------|-------|-------|------|
| 1 (긴급) | 네트워크 수신 버퍼 길이 검증 | 높음 | 낮음 | ✅ select-recv-bounds-check (#30) |
| 2 (긴급) | 인증정보 암호화 저장 | 높음 | 중간 | |
| 3 (높음) | SHM 권한 강화 | 중간 | 낮음 | ✅ 0664→0600 |
| 4 (높음) | Git 버전 관리 도입 | 중간 | 낮음 | |
| 5 (중간) | 프로세스 자동 재기동 | 중간 | 중간 | |
| 6 (중간) | CPU busy-wait 해소 | 낮음 | 낮음 | ✅ final-perf-optimize (#42) |
| 7 (중간) | 로그 구조화 | 낮음 | 중간 | ✅ structured-logging (#44) |
| 8 (낮음) | 전역변수 구조체화 리팩토링 | 낮음 | 높음 | |

---

## 4. 성능 최적화 개선 사항

### 4.1 [최우선] TCP_NODELAY 미설정 — ✅ 완료 (tcp-nodelay #37)

**위치**: `sub/tcpip_connect.c` 및 각 `_ts.c`, `_tr.c`

**현재 상태**: TCP 소켓에 성능 관련 옵션이 설정되지 않음.

**문제**: Nagle 알고리즘이 활성 상태. 소규모 주문 패킷(~300B)이 커널 버퍼에서 병합 대기하며 **수~수십 ms 지연** 발생 가능.

**개선안**:
```c
int flag = 1;
setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &(int){65536}, sizeof(int));
setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &(int){65536}, sizeof(int));
```

**예상 효과**: 주문당 수~수십 ms 절감 | **난이도**: 매우 낮음 (코드 1줄)

---

### 4.2 [최우선] Stat_Save() 매 루프 호출 — 불필요한 파일 I/O — ✅ 완료 (stat-save-optimize #36)

**위치**: `sub/stat_save.c`, 각 프로세스 메인루프

**현재 코드**:
```c
// pb_1100_ts.c 메인루프
while (START_S != END) {
    Stat_Save ();           // ← 매번 호출
    ...
    rt = poll (Poll, PollCnt, TimeOut);   // 5ms~15초 대기
}
```

`Stat_Save()` 내부:
```c
fp = fopen (f_name, "r+");              // open
fcntl (fileno(fp), F_SETLKW, &lock);   // lock
fwrite (buf, strlen(buf), 1, fp);       // write
fflush (fp);                            // flush → 디스크 I/O
fcntl (fileno(fp), F_SETLK, &lock);    // unlock
fclose (fp);                            // close
```

**문제**: poll timeout이 5ms(장중)이면 **초당 200회** `fopen/fwrite/fclose` 발생. 상태 저장일 뿐인데 디스크 I/O를 대량 소비.

**개선안**:
- SHM 기반 상태 저장: 파일 대신 공유메모리에 기록. 별도 프로세스가 주기적으로 파일 동기화
- 시간 기반 호출: 매 루프가 아닌 1~5초 간격으로만 호출

**예상 효과**: 파일 I/O 95% 이상 감소 | **난이도**: 낮음

---

### 4.3 [높음] F_R/F_W — 매 건마다 open/lock/close 반복 — ✅ 완료 (file-rw-optimize #38)

**위치**: `sub/file_rw.c:29-126 (F_R)`, `sub/file_rw.c:211-287 (F_W)`

**현재 흐름** (주문 1건 처리 시):
```
F_R():  open → 전체파일잠금 → seek → read → unlock → close
F_W():  fopen → 전체파일잠금 → write → fflush → unlock → fclose
```

**문제**:
- `open()`/`close()`는 커널 syscall. 주문 1건당 최소 4회 syscall 오버헤드
- `F_WRLCK`로 **파일 전체**를 잠금. 여러 프로세스가 같은 파일에 접근하면 직렬화
- `fflush()` 매 건마다 디스크 I/O 발생

**개선안**:
- **fd 풀링**: 프로세스 시작 시 fd를 열어두고 재사용
- **레코드 단위 잠금**: `lock.l_start = offset; lock.l_len = rec_size;`
- **배치 쓰기**: 매 건 fflush 대신 N건 또는 일정 시간 간격으로 flush

**예상 효과**: 주문당 수백 us -> 수십 us | **난이도**: 중간

---

### 4.4 [높음] 시세 수신 경로 — 불필요 복사 및 매 건 로깅 — ✅ 완료 (final-perf-optimize #42)

**위치**: `src/PB/pb_7100_ur.c:199-223`

**현재 코드**:
```c
memset(rbuf, 0, sizeof(rbuf));           // 2048바이트 0초기화
rt = Recv_Data(rbuf);                     // UDP 수신
rbuf_len = strlen(rbuf);                  // 바이너리 데이터에 strlen 사용 (위험+느림)
Log (USR_OK, "RD[%s] <%d>", ...);        // 정상 시세 매 건마다 로그 I/O
memset (TrCode, 0, sizeof(TrCode));      // 8바이트 0초기화
sprintf (TrCode, "%-2.2s", rbuf);        // sprintf로 2바이트 복사
```

**문제**:
- `memset(rbuf, 0, 2048)`: 매 패킷마다 2KB 초기화. 실제 데이터는 수백 바이트
- `strlen(rbuf)`: 바이너리 데이터에 strlen은 위험하고 느림. 수신 길이는 리턴값으로 확인 가능
- `Log(USR_OK, ...)`: 시세가 초당 수천 건이면 로그 I/O가 병목
- `sprintf`로 2바이트 복사: `memcpy`가 훨씬 빠름

**개선안**:
```c
rt = Recv_Data(rbuf);        // rt = 수신 길이
rbuf[rt] = '\0';             // memset 제거, 필요 위치만 null 종료
memcpy(TrCode, rbuf, 2);    // sprintf 대신 memcpy
TrCode[2] = '\0';
// Log는 DEBUG 레벨로 변경하거나 N건 단위 샘플링
```

**예상 효과**: 시세 처리량 20~30% 향상 | **난이도**: 낮음

---

### 4.5 [중간] F_R_Proc() — strlen 반복 호출 — ✅ 완료 (기존 tmp_off 추적 방식 적용)

**위치**: `sub/file_rw.c:176-195`

**현재 코드**:
```c
for (i = 0; i < rt; i += p_size) {
    // 레코드당 9회 memcpy (헤더 필드별 개별 복사)
    memcpy (&tmp[strlen(tmp)], buff_rw.Seq, sizeof(BUFF_RW_HEAD));  // strlen 매번
    memcpy (&tmp[strlen(tmp)], buf+i+sizeof(FILE_RW_HEAD), ...);    // strlen 매번
}
memcpy (p_buf, tmp, strlen(tmp));  // 최종 strlen
```

**문제**: `strlen(tmp)`를 매 반복마다 호출. O(n) 탐색이 레코드 수만큼 누적.

**개선안**:
```c
int pos = 0;
for (i = 0; i < rt; i += p_size) {
    memcpy(tmp + pos, buff_rw.Seq, sizeof(BUFF_RW_HEAD));
    pos += sizeof(BUFF_RW_HEAD);
    memcpy(tmp + pos, buf + i + sizeof(FILE_RW_HEAD), data_size);
    pos += data_size;
}
memcpy(p_buf, tmp, pos);
```

**예상 효과**: 파일 읽기 10~20% 향상 | **난이도**: 낮음

---

### 4.6 [중간] Select_Send — 불필요한 select syscall — ✅ 완료 (select-send-optimize #41)

**위치**: `sub/select_send.c:20-55`

**현재 코드**:
```c
int Select_Send (int p_sfd, char *p_send, int p_length) {
    rt = select (p_sfd+1, NULL, &write_set, ...); // select syscall
    rt = Sendn (p_sfd, p_send, p_length);          // send syscall
}
```

**문제**: 소켓 송신 버퍼가 충분하면 `select()`는 거의 항상 즉시 리턴. 불필요한 syscall. 200ms timeout도 장중 주문에 비해 너무 긺.

**개선안**: Non-blocking 소켓 + 직접 send. `EAGAIN` 시에만 `poll()`.

**예상 효과**: 전송당 수십 us 절감 | **난이도**: 낮음

---

### 4.7 [중간] AtoIf() — 비효율적 문자열 변환 — ✅ 완료 (atoif-optimize #39)

**위치**: `sub/atoif.c:19-45`

**현재 코드**:
```c
int AtoIf (char *p_ascii, int p_len) {
    for (i = 0; i < p_len; i++) {
        for (j = 0; j < 10; j++) {           // 문자당 최대 10회 비교
            if (*(p_ascii+i) == ('0' + j))
                break;
        }
        if (j < 10) jj = jj * 10 + j;
    }
}
```

**문제**: 숫자 하나 변환에 O(n*10) 비교. 시스템 전반에서 수천 회 호출 (패킷 길이 파싱, 시간 계산, 시퀀스 읽기).

**개선안**:
```c
int AtoIf_Fast (char *p, int len) {
    int val = 0, neg = 0;
    for (int i = 0; i < len; i++) {
        if (p[i] >= '0' && p[i] <= '9')
            val = val * 10 + (p[i] - '0');
        else if (p[i] == '-')
            neg = 1;
    }
    return neg ? -val : val;
}
```

**예상 효과**: 함수 자체 5~10배 향상 | **난이도**: 매우 낮음

---

### 4.8 [낮음] poll 이벤트 이중 순회 — ✅ 완료 (poll-traversal #40)

**위치**: `src/PB/pb_1100_ts.c:290-313`

**현재 코드**:
```c
// 1차 순회: POLLHUP 체크
for (i = 0; i < PollCnt; i++) {
    if (Poll[i].revents & POLLHUP) { ... }
}
// 2차 순회: POLLIN 체크 (첫 번째 이벤트만 처리)
for (i = 0; i < PollCnt; i++) {
    if (Poll[i].revents & POLLIN) { Poll[i].revents = 0; break; }
}
switch (i) { ... }
```

**문제**: 첫 번째 POLLIN 이벤트만 처리. FIFO와 소켓에 동시 이벤트 시 한 쪽이 다음 cycle까지 지연.

**개선안**:
```c
for (i = 0; i < PollCnt; i++) {
    if (Poll[i].revents & POLLHUP) { /* 끊김 처리 */ }
    if (Poll[i].revents & POLLIN) {
        switch (i) {
            case FIFO_EVENT:   Fifo_Event_Rtn();   break;
            case SOCKET_EVENT: Socket_Event_Rtn();  break;
            case DATA_EVENT:   Data_Event_Rtn();    break;
        }
    }
}
```

**예상 효과**: 동시 이벤트 시 1 poll cycle 절약 | **난이도**: 낮음

---

### 4.9 성능 개선 우선순위 요약 (전체 완료 — 2026-02-28)

| 순위 | 항목 | 예상 효과 | 난이도 | 위치 | 완료 |
|------|------|----------|-------|------|------|
| 1 | TCP_NODELAY 설정 | 주문당 수~수십ms | 매우 낮음 | tcpip_connect.c | tcp-nodelay (#37) |
| 2 | Stat_Save 호출 빈도 축소 | I/O 95% 감소 | 낮음 | 각 메인루프 | stat-save-optimize (#36) |
| 3 | F_R/F_W fd 풀링 | 주문당 수백us | 중간 | file_rw.c | file-rw-optimize (#38) |
| 4 | 시세 로그 샘플링 + memset/strlen 제거 | 시세 처리량 20~30% | 낮음 | pb_7100_ur.c | final-perf-optimize (#42) |
| 5 | 파일 잠금 범위 축소 (전체->레코드) | 동시 처리량 향상 | 중간 | file_rw.c | file-rw-optimize (#38) |
| 6 | F_R_Proc strlen 제거 | 읽기 10~20% | 낮음 | file_rw.c | 기존 완료 (tmp_off 추적) |
| 7 | Select_Send 불필요 select 제거 | 전송당 수십us | 낮음 | select_send.c | select-send-optimize (#41) |
| 8 | AtoIf 최적화 | 함수 5~10배 | 매우 낮음 | atoif.c | atoif-optimize (#39) |
| 9 | poll 이벤트 단일 순회 | 동시이벤트 시 1cycle | 낮음 | 각 메인루프 | poll-traversal (#40) |
| 10 | sleep(1) -> 동적 timeout | 장외 CPU 절감 | 낮음 | pb_1100_ts.c | final-perf-optimize (#42) |

---

## 5. 통합 개선 로드맵

### Phase 1: 즉시 적용 (1주 이내) — 5/5 완료

코드 변경이 최소이며 영향 범위가 제한적인 항목.

| 항목 | 분류 | 변경 파일 | 완료 |
|------|------|----------|------|
| TCP_NODELAY 설정 | 성능 | tcpip_connect.c | ✅ tcp-nodelay (#37) |
| AtoIf 최적화 | 성능 | atoif.c | ✅ atoif-optimize (#39) |
| 수신 버퍼 길이 검증 | 보안 | select_recv.c | ✅ select-recv-bounds-check (#30) |
| SHM 권한 강화 | 보안 | shmipc.c | ✅ 0664→0600 |
| sleep(1) -> 동적 timeout | 성능 | pb_1100_ts.c | ✅ final-perf-optimize (#42) |

### Phase 2: 단기 적용 (2~4주) — 5/7 완료

메인루프 로직 변경이 필요하지만 구조 변경은 아닌 항목.

| 항목 | 분류 | 변경 파일 | 완료 |
|------|------|----------|------|
| Stat_Save 호출 빈도 축소 | 성능 | stat_save.c, 각 메인루프 | ✅ stat-save-optimize (#36) |
| 시세 수신 최적화 (memset/strlen/Log) | 성능 | pb_7100_ur.c | ✅ final-perf-optimize (#42) |
| F_R_Proc strlen 제거 | 성능 | file_rw.c | ✅ 기존 완료 (tmp_off) |
| Select_Send 최적화 | 성능 | select_send.c | ✅ select-send-optimize (#41) |
| poll 이벤트 단일 순회 | 성능 | 각 메인루프 | ✅ poll-traversal (#40) |
| Git 버전 관리 도입 | 안정성 | 전체 | |
| 인증정보 암호화 저장 | 보안 | proc.ini, init_proc.c | |

### Phase 3: 중기 적용 (1~3개월) — 2/5 완료

파일 I/O 구조 변경, 프로세스 관리 강화 등 설계 변경이 수반되는 항목.

| 항목 | 분류 | 변경 파일 | 완료 |
|------|------|----------|------|
| F_R/F_W fd 풀링 | 성능 | file_rw.c | ✅ file-rw-optimize (#38) |
| 파일 잠금 범위 축소 | 성능 | file_rw.c | ✅ file-rw-optimize (#38) |
| 프로세스 자동 재기동 | 안정성 | pz_daemon.c | |
| 설정 동적 리로드 | 안정성 | init_proc.c, 각 프로세스 | |
| 로그 구조화 | 운영 | log_proc.c | ✅ structured-logging (#44) |

### Phase 4: 장기 적용 (3개월+)

아키텍처 수준의 리팩토링.

| 항목 | 분류 | 비고 |
|------|------|------|
| 전역변수 구조체화 | 유지보수 | 전체 소스 점진적 리팩토링 |
| #ifdef 분리/빌드 현대화 | 유지보수 | make 시스템 재구성 |
| 미사용 플랫폼 코드 정리 | 유지보수 | HP-UX/SunOS/AIX 분리 |
| 헬스체크/메트릭 시스템 | 운영 | 모니터링 인프라 구축 |
