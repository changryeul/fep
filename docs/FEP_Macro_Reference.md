# FEP Macro Reference Guide

| 항목 | 내용 |
|------|------|
| 작성일 | 2026-02-18 |
| 대상 시스템 | FEP (Front-End Processor) st01 모듈 |
| 분석 범위 | 전체 매크로 정의, 전개 체인, 사용법 |
| 관련 문서 | [FEP_Architecture_Analysis.md](FEP_Architecture_Analysis.md) |

---

## 목차

1. [헤더 파일 의존성 구조](#1-헤더-파일-의존성-구조)
2. [SHM 접근 매크로](#2-shm-접근-매크로-shm_memoryh) (가장 복잡)
3. [인터페이스 프로토콜 매크로](#3-인터페이스-프로토콜-매크로-fep_interfaceh)
4. [에러코드/로그레벨 매크로](#4-에러코드로그레벨-매크로-def_errorh)
5. [공통 유틸리티 매크로](#5-공통-유틸리티-매크로-fep_subh)
6. [TCP/네트워크 매크로](#6-tcp네트워크-매크로-fep_tcpiph)
7. [파일 I/O 플래그 매크로](#7-파일-io-플래그-매크로-fep_fileh)
8. [SHM 키/시장 매크로](#8-shm-키시장-매크로-shm_memoryh-하단)
9. [KRX 전문 관련 매크로](#9-krx-전문-관련-매크로-pa_structh)
10. [주요 함수 프로토타입](#10-주요-함수-프로토타입-fep_subh)
11. [자주 사용되는 매크로 TOP 10](#11-자주-사용되는-매크로-top-10)

---

## 1. 헤더 파일 의존성 구조

```
fep_fepp.h (마스터 헤더 - 모든 소스 파일이 include)
  |
  +-- fep_sub.h (공통 유틸리티 + 시스템 헤더 + 함수 프로토타입)
  |     |
  |     +-- def_error.h           에러코드, 로그레벨 (#define USR_ERROR 등)
  |     |
  |     +-- shm_memory.h          SHM 구조체 + 접근 매크로 (PROC, FILEM 등)
  |     |     +-- krx_mk.h        시세 TR 구조체 (CO_A001F, CO_B601K 등)
  |     |     +-- key_code.h      종목코드 검색 구조체/상수
  |     |     +-- strategy.h      전략 구조체 (SHM_SAMPLE01 등)
  |     |
  |     +-- fep_file.h            파일 R/W 플래그 (PS_R_FLAG, TS_W1_FLAG 등)
  |     +-- fep_tcpip.h           TCP 통신 구조체/상수 (TCP_HEAD, STX 등)
  |     +-- pa_struct.h           KRX 주문/체결 전문 구조체 (KRX_HEADER 등)
  |     +-- cli_interface.h       클라이언트 인터페이스
  |     +-- key_code.h            종목코드 구조체
  |
  +-- fep_interface.h             인터페이스 상수 + 축약 매크로 (INT_SEQ, START_S 등)
```

**참고**: 모든 `.c` 소스 파일은 `fep_fepp.h`를 include하며, 이를 통해 위 전체 헤더 체인이 포함된다.
프로세스별 추가 헤더(`pb_struct.h`, `strategy01.h` 등)는 개별 소스에서 직접 include.

---

## 2. SHM 접근 매크로 (`shm_memory.h`)

이 시스템에서 **가장 복잡하고 혼란을 일으키는** 매크로 체계.
3단계 중첩 매크로 체인으로 구성되어 있으며, 암묵적으로 글로벌 변수 `D_K`, `P_K`에 의존한다.

### 2.1 핵심 글로벌 변수

| 변수 | 타입 | 초기값 | 의미 |
|------|------|--------|------|
| `D_K` | int | -1 | 데몬 인덱스 (Daemon Key). 어떤 서브시스템에 속하는지 |
| `P_K` | int | -1 | 프로세스 인덱스 (Process Key). 해당 데몬 내 몇 번째 프로세스인지 |
| `S_K` | int | - | 라인 인덱스. TCP2 회선 구분용 |
| `T_K` | int | -1 | 시세TR 인덱스 |
| `Shm_Mem[]` | SHM_MEMORY[26] | - | 전체 시스템의 SHM 메모리 배열 (최대 26개 서브데몬, A~Z) |

`Init_Proc()` 함수에서 `D_K`와 `P_K`가 설정되며, 이후 모든 SHM 매크로가 이 값을 기반으로 동작한다.

### 2.2 레벨 1: 기본 SHM 접근자

SHM 세그먼트 내 특정 영역을 인덱스로 접근하는 매크로.

| 매크로 | 전개 | 반환 타입 | 의미 |
|--------|------|----------|------|
| `INFO(i)` | `SHM_All_Daemon_Info[i]` | `ALL_DAEMON_INFO` | i번 전체데몬 관리 정보 |
| `DAEMON(i)` | `Shm_Mem[i].Daemon[0]` | `SUB_DAEMON_INFO` | i번 서브데몬 정보 |
| `PROC(i,j)` | `Shm_Mem[i].Proc[j]` | `PROCESS_INFO` | i번 데몬의 j번 프로세스 |
| `FILEM(i,j)` | `Shm_Mem[i].File[j]` | `FILE_INFO` | i번 데몬의 j번 파일 정보 |
| `DSHM(i,j)` | `Shm_Mem[i].DShm[j]` | `DSHM_INFO` | i번 데몬의 j번 데이터SHM |
| `TCP1(i,j)` | `Shm_Mem[i].Tcp1[j]` | `TCP1_INFO` | i번 데몬의 j번 TCP1 정보 |
| `TCP2(i,j)` | `Shm_Mem[i].Tcp2[j]` | `TCP2_INFO` | i번 데몬의 j번 TCP2 정보 |
| `UDPIP(i,j)` | `Shm_Mem[i].Udpip[j]` | `UDPIP_INFO` | i번 데몬의 j번 UDP 정보 |
| `SISETR(i,j)` | `Shm_Mem[i].Sisetr[j]` | `SISETR_INFO` | i번 데몬의 j번 시세TR |

### 2.3 레벨 2: 프로세스 필드 접근자

`PROC(i,j)` 구조체의 개별 필드에 접근하는 매크로.

| 매크로 | 전개 | 타입 | 의미 |
|--------|------|------|------|
| `IF_SEQ(i,j)` | `PROC(i,j).if_seq` | int | 인터페이스 시퀀스 번호 |
| `IF_MEG_SEQ(i,j,k)` | `PROC(i,j).if_meg_seq[k]` | int | ME그룹 k의 시퀀스 번호 |
| `START_STAT(i,j)` | `PROC(i,j).start_status` | u_char | 시작 상태 (0:init 1:start 2:end 3:stop) |
| `SESSION_STAT(i,j)` | `PROC(i,j).session_stat` | short | 세션/암복호화 상태 (0:정상 2:미적용) |
| `TIME_VALUE(i,j)` | `PROC(i,j).timeout` | short | 타임아웃 값 (초) |
| `LAST_TR(i,j)` | `PROC(i,j).last_tr` | char[11] | 최종 송수신 TR코드 |
| `LOGON_ID(i,j)` | `PROC(i,j).logon_id` | char[20] | KRX 로그온 ID |
| `LOGON_PW(i,j)` | `PROC(i,j).logon_pw` | char[20] | KRX 로그온 비밀번호 |
| `DATA_CNT(i,j)` | `PROC(i,j).data_cnt` | u_char | SHM 데이터 버퍼 수 |
| `IFIFD(i,j,k)` | `PROC(i,j).in_FIFO_fd[k]` | int | 입력 FIFO 파일디스크립터 (k=0,1,2) |
| `FFN(i,j,k)` | `PROC(i,j).fifo_f[k]` | char[20] | FIFO 파일명 (k=0,1,2) |
| `TCP1_NSTAT(i,j)` | `PROC(i,j).l.t1.network_status` | u_char | TCP1 네트워크 상태 |
| `TCP2_LINE_GUBUN(i,j)` | `PROC(i,j).l.t2.line_gubun` | u_char | TCP2 라인 유형 |
| `TCP2_CSTAT(i,j)` | `PROC(i,j).l.t2.connect_status` | u_char | TCP2 접속 상태 |

### 2.4 레벨 3: 현재 프로세스 축약 매크로 (`fep_interface.h`)

**D_K와 P_K를 자동 주입**하여 현재 프로세스의 SHM에 바로 접근한다.
소스 코드에서 가장 많이 사용되는 매크로 그룹.

| 매크로 | 레벨2 전개 | 최종 전개 | 의미 |
|--------|-----------|----------|------|
| `INT_SEQ` | `IF_SEQ(D_K,P_K)` | `Shm_Mem[D_K].Proc[P_K].if_seq` | 현재 프로세스 시퀀스 |
| `INT_MEG_SEQ(k)` | `IF_MEG_SEQ(D_K,P_K,k)` | `Shm_Mem[D_K].Proc[P_K].if_meg_seq[k]` | 현재 프로세스 ME그룹 k 시퀀스 |
| `START_S` | `START_STAT(D_K,P_K)` | `Shm_Mem[D_K].Proc[P_K].start_status` | 현재 프로세스 시작 상태 |
| `SESSION_S` | `SESSION_STAT(D_K,P_K)` | `Shm_Mem[D_K].Proc[P_K].session_stat` | 현재 프로세스 세션 상태 |
| `TIME_OUT` | `TIME_VALUE(D_K,P_K)` | `Shm_Mem[D_K].Proc[P_K].timeout` | 현재 프로세스 타임아웃 |
| `DELAY_TIME` | `PROC(D_K,P_K).delay` | (직접 접근) | PS 딜레이 시간(ms) |
| `DATE_CURR` | `PROC(D_K,P_K).date` | (직접 접근) | 현재 영업일자 |
| `LOAD_CNT` | `DATA_CNT(D_K,P_K)` | `Shm_Mem[D_K].Proc[P_K].data_cnt` | 데이터 적재 건수 |
| `DR_FLAG` | `PROC(D_K,P_K).dr_flag` | (직접 접근) | DR 전환 플래그 |
| `PROCI` | `PROC(D_K,P_K)` | `Shm_Mem[D_K].Proc[P_K]` | 현재 프로세스 구조체 전체 |

**전개 예시**: 소스에서 `INT_SEQ++` 를 만나면

```
INT_SEQ++
  → IF_SEQ(D_K, P_K)++
  → PROC(D_K, P_K).if_seq++
  → Shm_Mem[D_K].Proc[P_K].if_seq++
```

= 현재 프로세스의 SHM 시퀀스 번호를 1 증가시킨다.

### 2.5 FIFO 파일 디스크립터 매크로

| 매크로 | 전개 | 의미 |
|--------|------|------|
| `INPUT_FD` | `IFIFD(D_K,P_K,0)` → `PROC(D_K,P_K).in_FIFO_fd[0]` | 입력 FIFO fd #1 |
| `INPUT_FD2` | `IFIFD(D_K,P_K,1)` → `PROC(D_K,P_K).in_FIFO_fd[1]` | 입력 FIFO fd #2 |
| `INPUT_FD3` | `IFIFD(D_K,P_K,2)` → `PROC(D_K,P_K).in_FIFO_fd[2]` | 입력 FIFO fd #3 |
| `START_FD` | `SFIFD(D_K)` → `Shm_Mem[D_K].start_FIFO_fd` | 데몬 시작 FIFO fd |
| `EXIT_FD` | `EFIFD(D_K)` → `Shm_Mem[D_K].exit_FIFO_fd` | 데몬 종료 FIFO fd |
| `DTART_FD` | `DFIFD(D_K)` → `Shm_Mem[D_K].daemon_FIFO_fd` | 데몬 제어 FIFO fd |

### 2.6 파일 읽기/쓰기 카운터 매크로

파일 기반 IPC의 W/R 카운터. 생산자가 W_CNT를 올리고 소비자가 R_CNT를 올리는 구조.

#### 입력(Input) 파일 카운터

| 매크로 | 전개 | 의미 |
|--------|------|------|
| `W_CNT(i,j)` | `IFW(D_K,P_K,i,j)` | 입력파일 i의 쓰기카운터 j |
| `R_CNT(i,j)` | `IFR(D_K,P_K,i,j)` | 입력파일 i의 읽기카운터 j |
| `WRITE_CNT` | `W_CNT(0,0)` | 입력파일1 쓰기카운터 (축약) |
| `READ_CNT` | `R_CNT(0,0)` | 입력파일1 읽기카운터 (축약) |
| `WRITE_CNT2` | `W_CNT(1,0)` | 입력파일2 쓰기카운터 |
| `READ_CNT2` | `R_CNT(1,0)` | 입력파일2 읽기카운터 |

**전개 체인 예시**: `WRITE_CNT`

```
WRITE_CNT
  → W_CNT(0,0)
  → IFW(D_K,P_K,0,0)
  → FILEM(D_K, PROC(D_K,P_K).in_f[0]-1).w_cnt[0]
  → Shm_Mem[D_K].File[ Shm_Mem[D_K].Proc[P_K].in_f[0] - 1 ].w_cnt[0]
```

= 현재 프로세스의 1번째 입력파일의 쓰기 카운터 값

#### 출력(Output) 파일 카운터

| 매크로 | 전개 | 의미 |
|--------|------|------|
| `OFW_CNT(i,j)` | `OFW(D_K,P_K,i,j)` | 출력파일 i의 쓰기카운터 j |
| `OFR_CNT(i,j)` | `OFR(D_K,P_K,i,j)` | 출력파일 i의 읽기카운터 j |
| `O_W_CNT1` ~ `O_W_CNT9` | `OFW_CNT(0..8, 0)` | 출력파일 1~9 쓰기카운터 |
| `O_R_CNT1` ~ `O_R_CNT9` | `OFR_CNT(0..8, 0)` | 출력파일 1~9 읽기카운터 |

**전개 체인 예시**: `O_W_CNT3`

```
O_W_CNT3
  → OFW_CNT(2,0)
  → OFW(D_K,P_K,2,0)
  → FILEM(D_K, PROC(D_K,P_K).out_f[2]-1).w_cnt[0]
  → Shm_Mem[D_K].File[ Shm_Mem[D_K].Proc[P_K].out_f[2] - 1 ].w_cnt[0]
```

= 현재 프로세스의 3번째 출력파일의 쓰기 카운터 값

#### 데이터 SHM 카운터

| 매크로 | 전개 | 의미 |
|--------|------|------|
| `IDW_CNT(i,j)` | `IDW(D_K,P_K,i,j)` | 입력 데이터SHM i의 쓰기카운터 j |
| `IDR_CNT(i,j)` | `IDR(D_K,P_K,i,j)` | 입력 데이터SHM i의 읽기카운터 j |
| `ODW_CNT(i,j)` | `ODW(D_K,P_K,i,j)` | 출력 데이터SHM i의 쓰기카운터 j |
| `ODR_CNT(i,j)` | `ODR(D_K,P_K,i,j)` | 출력 데이터SHM i의 읽기카운터 j |

### 2.7 TCP/UDP 네트워크 상태 매크로

#### TCP1 (대고객 접속용)

| 매크로 | 의미 |
|--------|------|
| `TCP1_NET_STA` = `TCP1_NSTAT(D_K,P_K)` | 현재 프로세스 TCP1 네트워크 상태 (0:off 1:on 2:end) |
| `TCP1_PORT(i,j)` | 프로세스 (i,j)의 TCP1 포트번호 |
| `TCP1_SPORT(i,j,k)` | 프로세스 (i,j)의 k번째 서비스 포트 |
| `TCP1_IP(i,j,k)` | 프로세스 (i,j)의 IP 주소 k번째 옥텟 |
| `TCP1_P_ST(i,j)` | 프로세스 (i,j)의 포트 상태 |
| `TCP1_S_ST(i,j,k)` | 프로세스 (i,j)의 k번째 서비스 상태 |

#### TCP2 (KRX 직결용)

| 매크로 | 의미 |
|--------|------|
| `TCP2_LINE_GU` = `TCP2_LINE_GUBUN(D_K,P_K)` | 라인 유형 (1,3:주, 2,4:백업) |
| `TCP2_CON_STA` = `TCP2_CSTAT(D_K,P_K)` | 접속 상태 (0:off 1:connected) |
| `TCP2_PORT_NO` = `TCP2_PORT(D_K,P_K,S_K)` | TCP2 포트 번호 |
| `TCP2_LINE_ST` = `TCP2_LSTAT(D_K,P_K,S_K)` | 라인 상태 (0:off 1:on) |
| `TCP2_LINE_ST1(k)` = `TCP2_LSTAT(D_K,P_K,k)` | k번 라인 상태 |
| `TCP2_PROC_ST` = `TCP2_PSTAT(D_K,P_K,S_K)` | 프로세스 상태 (0:init 1:on) |
| `TCP2_PROC_ST1(k)` = `TCP2_PSTAT(D_K,P_K,k)` | k번 프로세스 상태 |
| `TCP2_NET_STA(k)` = `TCP2_NSTAT(D_K,P_K,k)` | k번 네트워크 상태 |

#### UDP

| 매크로 | 의미 |
|--------|------|
| `UDP_ID(i,j)` | 프로세스 (i,j)의 UDP 라인 구분 ID |
| `UDP_PORT(i,j,k)` | 프로세스 (i,j)의 k번째 UDP 포트 |
| `UDP_IP1~4(i,j,k)` | 프로세스 (i,j)의 k번째 IP 주소 각 옥텟 |
| `UDP_P_ST(i,j)` | UDP 포트 상태 |
| `UDP_S_ST(i,j)` | UDP 서비스 상태 |

### 2.8 입출력 파일/데이터 SHM 메타 매크로

#### 입력 파일 (Input File)

| 매크로 | 의미 |
|--------|------|
| `IFN(i,j,k)` | 프로세스 (i,j)의 k번째 입력파일명 |
| `IFW(i,j,k,l)` | 프로세스 (i,j)의 k번째 입력파일 l번째 쓰기카운터 |
| `IFR(i,j,k,l)` | 프로세스 (i,j)의 k번째 입력파일 l번째 읽기카운터 |
| `IFS(i,j,k)` | 프로세스 (i,j)의 k번째 입력파일 레코드 크기 |
| `IFI(i,j,k)` | 프로세스 (i,j)의 k번째 입력파일 정보 문자열 |
| `IFC(i,j,k)` | 프로세스 (i,j)의 k번째 입력파일 FIFO 수 |

#### 출력 파일 (Output File)

| 매크로 | 의미 |
|--------|------|
| `OFN(i,j,k)` | 프로세스 (i,j)의 k번째 출력파일명 |
| `OFW(i,j,k,l)` | 프로세스 (i,j)의 k번째 출력파일 l번째 쓰기카운터 |
| `OFR(i,j,k,l)` | 프로세스 (i,j)의 k번째 출력파일 l번째 읽기카운터 |
| `OFS(i,j,k)` | 프로세스 (i,j)의 k번째 출력파일 레코드 크기 |

#### 입력 데이터 SHM

| 매크로 | 의미 |
|--------|------|
| `IDN(i,j,k)` | 데이터SHM 이름 |
| `IDK(i,j,k)` | 데이터SHM 키 정보 |
| `IDS(i,j,k)` | 데이터SHM 레코드 크기 |
| `IDM(i,j,k)` | 데이터SHM 최대 레코드 수 |
| `IDO(i,j,k)` | 데이터SHM 오프셋 |

#### 출력 데이터 SHM

| 매크로 | 의미 |
|--------|------|
| `ODN(i,j,k)` | 데이터SHM 이름 |
| `ODK(i,j,k)` | 데이터SHM 키 정보 |
| `ODS(i,j,k)` | 데이터SHM 레코드 크기 |
| `ODM(i,j,k)` | 데이터SHM 최대 레코드 수 |
| `ODO(i,j,k)` | 데이터SHM 오프셋 |

### 2.9 SHM 로그 관련

| 매크로 | 전개 | 의미 |
|--------|------|------|
| `SLOGW(i)` | `DAEMON(i).w_cnt` | i번 데몬의 로그 쓰기 카운터 |
| `SLOGR(i)` | `DAEMON(i).r_cnt` | i번 데몬의 로그 읽기 카운터 |
| `SHM_LOG_SIZE` | 1057 | SHM 로그 1건 크기 (head 33 + msg 1024) |
| `SHM_LOG_MAX` | 10000 | SHM 로그 최대 건수 |

---

## 3. 인터페이스 프로토콜 매크로 (`fep_interface.h`)

### 3.1 시스템 모드 (운영 구분)

| 매크로 | 값 | 의미 |
|--------|---|------|
| `MAIN` | 0 | 주 시스템 (운영서버) |
| `BACKUP` | 1 | 백업 시스템 |
| `EMERGENCY` | 2 | 비상 모드 |
| `CHANGE` | 3 | 전환 진행 중 |
| `ALTERNATE_P` | 4 | P(Primary) 대체 |
| `ALTERNATE_B` | 5 | B(Backup) 대체 |

### 3.2 KRX FEP 직결 세션 메시지 타입 (TR_/RP_ 쌍)

KRX 직결 TCP2 회선에서 사용하는 세션 제어 메시지.
**TR_** = 송신(Transmit), **RP_** = 수신(Reply). 항상 쌍으로 동작.

| 송신 (TR_) | 값 | 수신 (RP_) | 값 | 의미 |
|-----------|---|-----------|---|------|
| `TR_LINK` | 0 | `RP_LINK` | 1 | 접속 요청/응답 |
| `TR_RESD` | 2 | `RP_RESD` | 3 | 재접속(Resume) 요청/응답 |
| `TR_POLL` | 4 | `RP_POLL` | 5 | 회선시험(Heartbeat) 요청/응답 |
| `TR_DATA` | 6 | `RP_DATA` | 7 | 데이터 송수신 |
| `TR_STOP` | 8 | `RP_STOP` | 9 | 종료 요청/응답 |
| `TR_DEND` | 10 | `RP_DEND` | 11 | 데이터 완료 (End of Data) |
| `TR_RESE` | 12 | `RP_RESE` | 13 | 리셋 요청/응답 |
| `TR_ERCD` | 14 | `RP_ERCD` | 15 | 에러코드 전달 |
| `TR_LOON` | 16 | `RP_LOON` | 17 | 로그온(세션개시) 요청/응답 |
| `TR_LOOU` | 18 | `RP_LOOU` | 19 | 로그아웃(세션종료) 요청/응답 |
| `TR_RELI` | 21 | `RP_RELI` | 22 | 재접속(Re-Link) 요청 |
| `TR_CONT` | 23 | `RP_CONT` | 24 | 연속 처리 |
| `TR_HEAD` | 25 | `RP_HEAD` | 26 | 헤더 전달 |
| `TR_TAIL` | 27 | `RP_TAIL` | 28 | 테일(종료마크) 전달 |
| `TR_SEQU` | 29 | `RP_SEQU` | 30 | 시퀀스 확인/동기화 |

#### 핸드셰이크 (단방향)

| 매크로 | 값 | 의미 |
|--------|---|------|
| `TR_HSI` | 50 | 핸드셰이크 개시 (Initiate) |
| `TR_HSU` | 52 | 핸드셰이크 업데이트 (Update) |
| `TR_HSF` | 54 | 핸드셰이크 종료 (Finish) |

#### 메시지 길이 상수

| 매크로 | 값 | 용도 |
|--------|---|------|
| `LIOK_LEN` | 20 | 접속응답 메시지 길이 |
| `STOP_LEN` | 12 | 종료 메시지 길이 |
| `RESE_LEN` | 18 | 리셋 메시지 길이 |
| `HEAD_LEN` | 80 | 헤더 메시지 길이 |
| `ERCD_LEN` | 92 | 에러코드 메시지 길이 |
| `PROC_LEN` | 99 | 프로세스 정보 길이 |

### 3.3 내부 메시지 타입 (업무계 ↔ FEP 통신)

FEP 내부 프로세스 간 또는 업무계(HTS 등)와의 통신에 사용.

| 그룹 | 매크로 | 값 | 의미 |
|------|--------|---|------|
| **접속** | `LINK` | 1 | 접속 요구 |
| | `LIOK` | 2 | 접속 확인 응답 |
| | `RELI` | 3 | 재접속 요구 |
| | `REOK` | 4 | 재접속 확인 응답 |
| | `RESN` | 5 | 재전송 요청 |
| **상태** | `LIVE` | 6 | 생존확인 (Heartbeat) |
| | `MMJC` | 7 | 매매정지 통보 |
| | `LAST` | 8 | 최종 메시지 표시 |
| | `LAOK` | 9 | 최종 메시지 응답 |
| **선물옵션** | `FO01` | 11 | 선물옵션 업무 1 |
| | `FO02` | 12 | 선물옵션 업무 2 |
| | `FO03` | 13 | 선물옵션 업무 3 |
| **재확인** | `AW54` | 21 | 선물옵션 접수내역 재확인 |
| | `AW55` | 22 | 선물옵션 체결내역 재확인 |
| | `CW01` | 23 | 추가 업무 1 |
| | `CW02` | 24 | 추가 업무 2 |
| **데이터** | `FOCL` | 31 | 선물옵션 종료 |
| | `DATA` | 32 | 주문/체결 데이터 |
| | `ERCD` | 33 | 에러코드 |
| | `REST` | 34 | 재시작 |
| | `STOP` | 35 | 정지 |
| | `RESE` | 36 | 리셋 |
| **거부** | `REJE` | 99 | FEP 거부 |

### 3.4 TCP 업무접속 코드 (문자열)

업무계 TCP1 통신 프로토콜에서 사용하는 4바이트 문자열 코드.

| 매크로 | 값 | 의미 |
|--------|---|------|
| `TCP_LINK_CD` | `"LINK"` | 접속 요구 |
| `TCP_LIOK_CD` | `"LIOK"` | 접속 응답 |
| `TCP_STRT_CD` | `"STRT"` | 개시 요구 |
| `TCP_STOK_CD` | `"STOK"` | 개시 응답 |
| `TCP_DATA_CD` | `"DATA"` | 데이터 처리 요구 |
| `TCP_DAOK_CD` | `"DAOK"` | 데이터 처리 응답 |
| `TCP_RSND_CD` | `"RSND"` | 재전송 요구 |
| `TCP_RSOK_CD` | `"RSOK"` | 재전송 응답 |
| `TCP_POLL_CD` | `"POLL"` | 회선시험 요구 |
| `TCP_POOK_CD` | `"POOK"` | 회선시험 응답 |
| `TCP_STOP_CD` | `"STOP"` | 종료 요구 |

### 3.5 TCP 내부 상태코드 (정수형 T_ prefix)

위 문자열 코드를 내부 처리용 정수로 매핑.

| 매크로 | 값 | 대응 문자열 |
|--------|---|-----------|
| `T_LINK` | 1 | LINK |
| `T_LIOK` | 2 | LIOK |
| `T_STRT` | 3 | STRT |
| `T_STOK` | 4 | STOK |
| `T_DATA` | 5 | DATA |
| `T_DAOK` | 6 | DAOK |
| `T_RSND` | 7 | RSND |
| `T_RSOK` | 8 | RSOK |
| `T_POLL` | 9 | POLL |
| `T_POOK` | 10 | POOK |
| `T_STOP` | 11 | STOP |
| `T_LNTH` | 12 | (에러) 전문길이 오류 |
| `T_EROR` | 13 | (에러) 기타 오류 |

### 3.6 라인 구분

| 매크로 | 값 | 의미 |
|--------|---|------|
| `LK` | 1 | 접속 (Link) |
| `LC` | 2 | 접속 확인 (Link Confirm) |
| `RE` | 3 | 재접속 (Reconnect) |

### 3.7 기타 상수

| 매크로 | 값 | 의미 |
|--------|---|------|
| `TCP_COMPANY` | `"00012"` | 회원사 번호 |
| `TR_AW54` | `"AW54"` | 선물옵션 접수내역 재확인 TR코드 (문자열) |
| `TR_AW55` | `"AW55"` | 선물옵션 체결내역 재확인 TR코드 (문자열) |

---

## 4. 에러코드/로그레벨 매크로 (`def_error.h`)

### 4.1 로그 심각도 매트릭스

카테고리(영역) x 레벨(심각도) 조합. `Log()` 함수의 첫 번째 인자로 사용.

```
           OK(0)  FATAL(1)  DEBUG(2)  WARN(3)  ERROR(5)
           ─────  ────────  ────────  ───────  ────────
  USR       0       1         2         3        5       사용자/업무 영역
  SYS     100     101       102       103      105       시스템 영역
  PRO     200     201       202       203      205       프로세스 영역
  SAM     300     301       302       303      305       파일(SAM) 영역
  FIF     400     401       402       403      405       FIFO 영역
  ORA     500     501       502       503      505       DB 영역
  TCP     600     601       602       603      605       TCP/IP 영역
  UDP     700     701       702       703      705       UDP/IP 영역
  DSH     800     801       802       803      805       데이터SHM 영역
```

사용 예:
```c
Log(USR_ERROR, "주문전문 오류: TR=%11.11s", trcode);    // 업무 에러 로그
Log(TCP_FATAL, "KRX 접속 실패: %s", SYS_STR);           // TCP 치명적 오류
Log(SYS_DEBUG, "recv len=%d", len);                      // 시스템 디버그
```

### 4.2 시스템 에러 접근

| 매크로 | 전개 | 의미 |
|--------|------|------|
| `SYS_NO` | `errno` | 시스템 에러 번호 |
| `SYS_STR` | `strerror(errno)` | 시스템 에러 메시지 문자열 |

### 4.3 업무 에러코드 (문자열 상수)

#### 공통 응답코드

| 매크로 | 값 | 의미 |
|--------|---|------|
| `RES_NORMAL` | `"0000"` | 정상 처리 |
| `RES_ERROR` | `"9999"` | HTS 거부, 비정상 처리 |

#### TCP 헤더 오류 (`T0xx`)

| 매크로 | 값 | 검증 대상 필드 |
|--------|---|--------------|
| `ERR_HEAD_T1` | `"T001"` | Stx (전문시작) |
| `ERR_HEAD_T2` | `"T002"` | Length (전문길이) |
| `ERR_HEAD_T3` | `"T003"` | ApType (업무구분) |
| `ERR_HEAD_T4` | `"T004"` | Date (일자) |
| `ERR_HEAD_T5` | `"T005"` | Time (시각) |
| `ERR_HEAD_T6` | `"T006"` | ResponseCode (응답코드) |
| `ERR_HEAD_T7` | `"T007"` | MsgType (메시지타입) |
| `ERR_HEAD_T8` | `"T008"` | SeqNo (일련번호) |
| `ERR_HEAD_T9` | `"T009"` | DataCnt (데이터건수) |
| `ERR_HEAD_B1` | `"T101"` | BatchTr (배치TR) |
| `ERR_HEAD_B2` | `"T102"` | BatchSeq (배치순번) |

#### 데이터 헤더 오류 (`D0xx`)

| 매크로 | 값 | 검증 대상 |
|--------|---|----------|
| `ERR_HEAD_D1` | `"D001"` | Length |
| `ERR_HEAD_D2` | `"D002"` | DataSeq |
| `ERR_HEAD_D3` | `"D003"` | LineFlag |

#### 장운영 오류 (`F1xx`, `K1xx`)

| 매크로 | 값 | 의미 |
|--------|---|------|
| `ERR_TIME_BEFORE` | `"F100"` | 장전 시간 이전 접속 |
| `ERR_PROC_RUN` | `"F101"` | 기동 중인 프로세스 중복 접속 불가 |
| `ERR_TIME_END` | `"F102"` | 장종료 시간 이후 접속 |
| `ERR_TIME_STOP` | `"F103"` | 장중지 시간 접속 |
| `ERR_REJECT` | `"F104"` | 거래소 거부로 클라이언트 종료 |
| `ERR_JANG_BEFORE` | `"K100"` | 장개시 전 거래소 전문 거부 |
| `ERR_JANG_ERROR` | `"K101"` | 거래소 기동 시 프로세스 오류 |
| `ERR_JANG_END` | `"K102"` | 장종료 후 거래소 전문 거부 |
| `ERR_JANG_STOP` | `"K103"` | 정지 요청에 의한 stop 처리 |
| `ERR_TIME_OUT` | `"K104"` | 타임아웃 |
| `ERR_SEND_ERR` | `"K105"` | 거래소 전문 송신 오류 |
| `ERR_TCP_STATUS` | `"K106"` | 데이터 요구 시 프로세스 미기동 |

#### 데이터 오류 (`D5xx`)

| 매크로 | 값 | 의미 |
|--------|---|------|
| `ERR_DATA_H1` | `"D500"` | TR코드 오류 |
| `ERR_DATA_S1` | `"D501"` | 종목코드 불일치 |
| `ERR_DATA_S2` | `"D502"` | 계좌번호 불일치 |
| `ERR_DATA_S3` | `"D503"` | 비밀번호 불일치 |
| `ERR_DATA_S4` | `"D504"` | IP 주소 불일치 |
| `ERR_DATA_S5` | `"D505"` | 자동주문 기동 한도 초과 |
| `ERR_DATA_S6` | `"D506"` | 자동주문 기동 입력 값 오류 |
| `ERR_DATA_S10` | `"D510"` | 종료 요청 미처리 |
| `ERR_DATA_S11` | `"D511"` | Auto Process Full |
| `ERR_DATA_S12` | `"D512"` | Auto Process 런타임 오류 |
| `ERR_DATA_S15` | `"D515"` | Auto 건수 초과 |
| `ERR_DATA_S16` | `"D516"` | Auto 계좌 건수 초과 |
| `ERR_DATA_S20` | `"D520"` | 송신 회선 이상 발견 |

#### 기타 오류

| 매크로 | 값 | 의미 |
|--------|---|------|
| `ERR_FILE_WRITE` | `"E901"` | 파일 쓰기 오류 |
| `ERR_BATCH_DUP` | `"E902"` | 거래소 송신 완료된 배치TR 중복 |
| `ERR_DD_DIVIDE` | `"K201"` | 분배 오류 |
| `KRX_DATA_LOSS` | `"K999"` | DR 전환 시 KSE/KSQ 데이터 유실 |

---

## 5. 공통 유틸리티 매크로 (`fep_sub.h`)

### 5.1 상태/플래그 상수

| 매크로 | 값 | 의미 | 사용 예 |
|--------|---|------|---------|
| `OK` | 0 | 성공 (함수 반환) | `if (rt == OK)` |
| `NOTOK` | -1 | 실패 (함수 반환) | `if (rt == NOTOK)` |
| `FAIL` | 1 | 실패 (양수) | `if (rt == FAIL)` |
| `OFF` | 0 | 꺼짐/비활성 | `DeviceSendFlag = OFF` |
| `ON` | 1 | 켜짐/활성 | `DeviceSendFlag = ON` |
| `END` | 2 | 종료 | 프로세스 상태 판별 |

### 5.2 잡(Job) 상태 (`shm_memory.h`)

PROCESS_INFO.start_status 및 프로세스 상태 판별에 사용.

| 매크로 | 값 | 의미 |
|--------|---|------|
| `JOB_INIT` | 0 | 초기화됨 |
| `JOB_START` | 1 | 시작됨 |
| `JOB_END` | 2 | 정상 종료 |
| `JOB_STOP` | 3 | 강제 정지 |

사용 예:
```c
if (START_S == JOB_START)    // 현재 프로세스가 시작 상태인지
START_S = JOB_END;           // 현재 프로세스를 종료 상태로 변경
```

### 5.3 크기/버퍼 상수

| 매크로 | 값 | 의미 |
|--------|---|------|
| `SIZE_MB` | 1,048,576 | 1MB |
| `SIZE_GB` | 1,073,741,824 | 1GB |
| `LOG_SIZE` | 5,120 | 로그 버퍼 최대 크기 |
| `LOG_HEAD_SIZE` | 42 | 로그 헤더 크기 |
| `SHM_DATA_SIZE` | 5,120 | SHM 데이터 버퍼 1건 크기 |
| `DATA_BUF_CNT` | 30 | 데이터 버퍼 수 |
| `FILE_BUF_LEN` | 8,192 | 파일 버퍼 크기 (8KB) |
| `SHM_MAX_SUB` | 26 | 서브데몬 최대 수 (A~Z) |
| `MAX_DSHM_SEG` | 50 | 데이터SHM 세그먼트 최대 수 |

### 5.4 프로세스 타입

PROCESS_INFO.type 필드에 사용.

| 매크로 | 값 | 의미 | 프로세스 예시 |
|--------|---|------|-------------|
| `TY_MP` | 1 | Manager Process | pa_9001_mp |
| `TY_DD` | 2 | Division Demanded | 분배 프로세스 |
| `TY_TRS1` | 3 | TCP/IP 1 송수신 | 대고객 접속 |
| `TY_TRS2` | 4 | TCP/IP 2 송수신 | KRX 직결 접속 |
| `TY_BRS` | 5 | DB 송수신 | DB 연동 |
| `TY_URS` | 6 | UDP/IP 송수신 | 시세 멀티캐스트 |

### 5.5 FEP 경로 타입

`Get_Environment()` 함수에서 환경 경로를 설정할 때 사용.

| 매크로 | 값 | 전역 변수 | 실제 경로 예시 |
|--------|---|----------|--------------|
| `FEP_LOG` | 1 | `_FEP_LOG` | `$ST_HOME/log/` |
| `FEP_DAT` | 2 | `_FEP_DAT` | `$ST_HOME/dat/` |
| `FEP_BIN` | 3 | `_FEP_BIN` | `$ST_HOME/bin/` |
| `FEP_TMP` | 4 | `_FEP_TMP` | `$ST_HOME/tmp/` |
| `FEP_CFG` | 5 | `_FEP_CFG` | `$ST_HOME/cfg/` |
| `FEP_SHL` | 6 | `_FEP_SHL` | `$ST_HOME/shl/` |
| `FEP_FIFO` | 7 | `_FEP_FIFO` | `$ST_HOME/fifo/` |

### 5.6 기타 유틸리티

| 매크로 | 전개 | 의미 |
|--------|------|------|
| `Max(a,b)` | `((a>b)?a:b)` | 두 값 중 최대값 |
| `PROCI` | `PROC(D_K,P_K)` | 현재 프로세스 (PROCI와 동일) |
| `PGMLIN` | `__FILE__,__LINE__` | 디버그용 파일명:라인번호 |
| `YES` / `NO` / `QUIT` | 1 / 0 / -1 | 범용 상태값 (py_cm.h) |

---

## 6. TCP/네트워크 매크로 (`fep_tcpip.h`)

### 6.1 버퍼/길이 상수

| 매크로 | 값 | 의미 |
|--------|---|------|
| `STX` | `0x02` | 전문 시작 구분자 (Start of Text) |
| `TCP_MESSAGE_MAX_LEN` | 4,300 | TCP 전문 최대 길이 |
| `TCP_BUFF_MAX_LEN` | 5,120 | TCP 수신 버퍼 크기 |

### 6.2 헤더/데이터 크기

| 매크로 | 값 | 산출 | 구조체 |
|--------|---|------|--------|
| `TCP_HEAD_LEN` | 80 | `sizeof(TCP_HEAD)` | 업무계 통신 헤더 |
| `TCP_DATA_HEAD_LEN` | 70 | `sizeof(TCP_DATA_HEAD)` | 데이터 부가 헤더 |
| `TCP_REJE_HEAD_LEN` | 80 | `sizeof(TCP_RJ_HEADER)` | FEP 거부 헤더 |
| `AP_HEAD_LEN` | 60 | `sizeof(AP_HEAD)` | 업무계 간소화 헤더 |
| `HEAD_SIZE` | 20 | `sizeof(FILE_DATA_HEAD)` | 파일 R/W 용 헤더 |
| `TCP_DATA_LEN` | 4,220 | 4300 - 80 | 순수 데이터 영역 |
| `IMECO_HEAD_LEN` | 20 | `sizeof(IMECO_TCP_HEAD)` | IMECO FEP 헤더 |
| `IMECO_TCP_DATA_LEN` | 4,280 | 4300 - 20 | IMECO 데이터 영역 |

### 6.3 전문 구조 도식

```
업무계(HTS) 전문:
  [TCP_HEAD 80B][TCP_DATA_HEAD 70B][KRX_JUMUN_DATA 최대294B]
   └─ Stx(1)     └─ Filler(70)     └─ 주문/응답 Body
      Length(4)
      ApType(4)
      ...

KRX 직결 전문:
  [KRX_HEADER 82B][Body 가변길이]
   └─ BeginString(8)  └─ DataSeq(11)
      BodyLength(6)       Transaction_Code(11)
      MsgType(11)         Megrp_no(2)
      ...                 ...

IMECO 전문:
  [IMECO_TCP_HEAD 20B][Body 최대4280B]
   └─ Length(4)          └─ 주문/체결 데이터
      MsgType(1)
      ResponseCode(4)
      SeqNo(10)
      MsgCount(1)
```

---

## 7. 파일 I/O 플래그 매크로 (`fep_file.h`)

파일 기반 IPC에서 `F_R()`, `F_W()` 등의 함수에 전달하는 플래그 값.

### 7.1 네이밍 규칙

```
{프로세스타입}_{R/W}{파일번호}_{카운터번호}

  PS  = 주문(PS) 프로세스
  TS  = 송신(TS) 프로세스
  R   = Read
  W   = Write
  숫자 = 파일 순번 (1~9) 또는 카운터 순번 (1~9)
```

### 7.2 읽기 플래그

| 그룹 | FLAG | 카운터 범위 | 의미 |
|------|------|-----------|------|
| `PS_R_FLAG` (1) | `PS_R_1`~`PS_R_9` (10~18) | 카운터 1~9 | 주문 프로세스 입력파일 읽기 |
| `TS_R1_FLAG` (2) | `TS_R1_1`~`TS_R1_9` (20~28) | 카운터 1~9 | 송신 프로세스 입력파일1 읽기 |
| `TS_R2_FLAG` (3) | `TS_R2_1`~`TS_R2_9` (30~38) | 카운터 1~9 | 송신 프로세스 입력파일2 읽기 |
| `TS_R3_FLAG` (4) | `TS_R3_1`~`TS_R3_9` (40~48) | 카운터 1~9 | 송신 프로세스 입력파일3 읽기 |

### 7.3 쓰기 플래그

| 그룹 | FLAG | 카운터 범위 | 의미 |
|------|------|-----------|------|
| `TS_W1_FLAG` (1) | `TS_W1_1`~`TS_W1_2` (10~11) | 카운터 1~2 | 출력파일1 쓰기 |
| `TS_W2_FLAG` (2) | `TS_W2_1`~`TS_W2_2` (20~21) | 카운터 1~2 | 출력파일2 쓰기 |
| ... | ... | ... | 최대 TS_W9까지 (90~91) |

### 7.4 사용 예

```c
// 송신 프로세스에서 입력파일1의 데이터를 읽고, 카운터1 갱신
rt = F_R(TS_R1_FLAG, DataBuff, TS_R1_1);

// 주문 프로세스에서 입력파일의 데이터를 읽고, 카운터1 갱신
rt = F_R(PS_R_FLAG, DataBuff, PS_R_1);

// 출력파일2에 데이터 쓰기, 카운터1 갱신
F_W(TS_W2_FLAG, DataBuff, TS_W2_1);
```

---

## 8. SHM 키/시장 매크로 (`shm_memory.h` 하단)

### 8.1 SHM 키 체계

```
키 형식: 0x(1)(2)(3)(4)(5)(6)(7)(8)
  (1)     = 4: FEP용
  (2)     = 1: Real 환경
  (3)~(6) = 0000 (시세/업무 영역)
  (7)~(8) = SHM 세그먼트 구분
```

| 매크로 | 키 값 | 용도 | 타입 |
|--------|------|------|------|
| `BASE_SHM_KEY` | `0x41000000` | 프로세스 환경/관리 SHM | `SHM_MEMORY` |
| `NOTE_SHM_KEY` | `0x41000001` | KTS 채권 시세 | `SHM_NOTE` |
| `RDS01_SHM_KEY` | `0x41000002` | 채권종목정보 | `CO_A001_RDS01` |
| `FF_SHM_KEY` | `0x41000003` | 금융상품선물 시세 | `SHM_FIN_FUT` |
| `B_SHM_KEY` | `0x41000011` | 백오피스 연동 | - |
| `STRRG_SHM_KEY` | `0x41000012` | 전략 데이터 | `STRRG` |
| `MK_PM_SHM_KEY` | `0x41000013` | 미체결 처리 | `MK_PREMATCH` |
| `RISK_SHM_KEY` | `0x41000014` | 한도관리 | `RISK` |
| `ITEM_SHM_KEY` | `0x41000030` | 종목코드 검색(qsort/bsearch) | `SHM_KEY_ARRY` |

### 8.2 SHM 포인터 전역 변수

| 변수 | 타입 | SHM 키 | 의미 |
|------|------|--------|------|
| `Shm_Note` | `SHM_NOTE *` | `NOTE_SHM_KEY` | KTS 채권 시세 데이터 |
| `Shm_Rds01` | `CO_A001_RDS01 *` | `RDS01_SHM_KEY` | 채권종목정보 |
| `Shm_FinFut` | `SHM_FIN_FUT *` | `FF_SHM_KEY` | 금융상품선물 시세 |
| `Shm_Mk_PreMatch` | `MK_PREMATCH *` | `MK_PM_SHM_KEY` | 시장별 미체결 |
| `Shm_Strrg` | `STRRG *` | `STRRG_SHM_KEY` | 전략 영역 |
| `Shm_Risk` | `RISK *` | `RISK_SHM_KEY` | 한도관리 |
| `Shm_Item` | `SHM_KEY_ARRY *` | `ITEM_SHM_KEY` | 종목코드 검색 |

### 8.3 시장/종목 상수

| 매크로 | 값 | 의미 |
|--------|---|------|
| `NOTE_MK_KTS` | 0 | 국채(KTS) 시장 인덱스 |
| `DEV_MK_FIF` | 1 | 금융상품선물 시장 인덱스 |
| `RISK_MK_CNT` | 2 | 실사용 시장 수 (채권+파생) |
| `SHM_MAX_NOTE` | 10,000 | 채권 최대 종목 수 |
| `SHM_MAX_RDS01` | 100,000 | 채권종목정보 최대 건수 |
| `SHM_MAX_DEV_FIF` | 1,000 | 금융상품선물 최대 종목 수 |

### 8.4 자동주문/계좌 상수

| 매크로 | 값 | 의미 |
|--------|---|------|
| `MAX_AUTO_PROC` | 40 | 자동주문 프로세스 최대 수 |
| `MAX_MICHE` | 10,000 | 미체결 최대 건수 (시장별) |
| `ACC_NO_CNT` | 20 | 사용 계좌 수 |
| `ACC_NO_BASE` | 31 | 계좌 기준 수 |

### 8.5 주문번호 채번 체계

| 매크로 | 값 | 의미 |
|--------|---|------|
| `MK01_ORDER_NO_BASE` | 200,000,000 | 채권 주문번호 시작 (메인서버) |
| `MK02_ORDER_NO_BASE` | 200,000,000 | 파생 주문번호 시작 (메인서버) |
| `MK01_ORDER_NO_BACK_GAP` | 100,000,000 | 채권 백업서버 오프셋 |
| `MK02_ORDER_NO_BACK_GAP` | 100,000,000 | 파생 백업서버 오프셋 |
| `MK01_ORDER_NO_GAP` | 1,000,000 | 채권 프로세스별 주문번호 구간 |
| `MK02_ORDER_NO_GAP` | 1,000,000 | 파생 프로세스별 주문번호 구간 |

주문번호 배정 구조:
```
메인서버: 200,000,000 ~ 299,999,999 (1억개)
  └ 프로세스 0: 200,000,000 ~ 200,999,999 (100만개)
  └ 프로세스 1: 201,000,000 ~ 201,999,999 (100만개)
  └ ...
  └ 프로세스 39: 239,000,000 ~ 239,999,999 (100만개)
  └ Client용:   241,000,000 ~ 241,999,999

백업서버: 300,000,000 ~ 399,999,999 (1억개, BASE + BACK_GAP)
```

---

## 9. KRX 전문 관련 매크로 (`pa_struct.h`)

### 9.1 전문 길이 상수

| 매크로 | 값 | 의미 |
|--------|---|------|
| `KRX_HEAD_LEN` | 82 | `sizeof(KRX_HEADER)` - KRX 공통 헤더 크기 |
| `SEARCH_HEADER_LEN` | 50 | `sizeof(SEARCH_HEADER)` - 조회 헤더 크기 |

### 9.2 KRX 헤더 구조 (`KRX_HEADER`, 82 bytes)

```
오프셋  크기  필드명              의미
 0       8   BeginString         전문유형 (API프로토콜 버전, "KMAPv1.0")
 8       6   BodyLength          메시지 길이 (Body 전체 길이)
14      11   MsgType             메시지 타입 / TR코드
25      11   MsgSeqNum           일련번호
36       5   SenderCompID        회원번호
41      10   DeliverToCompID     연계 도착 회원사 번호
51      10   OnBehalfOfCompID    회신 송신 회원사 번호
61      17   SendingTime         전송일시 (YYYYMMDDHHMMSSMS)
78       3   DataCnt             데이터 건수
81       1   Encrypt             암호화 유무 (Y/N)
```

---

## 10. 주요 함수 프로토타입 (`fep_sub.h`)

매크로와 함께 자주 사용되는 핵심 함수 목록.

### 10.1 변환 함수

| 함수 | 인자 | 반환 | 의미 |
|------|------|------|------|
| `AtoIf(char*, int)` | 문자열, 길이 | int | 고정길이 문자열 → 정수 |
| `AtoLf(char*, int)` | 문자열, 길이 | long | 고정길이 문자열 → long |
| `AtoDf(char*, int)` | 문자열, 길이 | double | 고정길이 문자열 → double |
| `ItoAf(int, char*, int)` | 정수, 버퍼, 길이 | char* | 정수 → 고정길이 문자열 |

### 10.2 프로세스/시스템

| 함수 | 의미 |
|------|------|
| `Init_Proc(argc, argv)` | 프로세스 초기화 (D_K, P_K 설정, SHM 연결) |
| `Exit_Process()` | 프로세스 정상 종료 |
| `End_Routine(sig)` | 시그널 핸들러 (종료 루틴) |
| `Get_Environment()` | 환경변수에서 경로 정보 로드 |
| `Stat_Save()` | 프로세스 상태 파일 저장 |
| `Check_Exist()` | 프로세스 중복 실행 체크 |
| `Check_Proc(name)` | 프로세스 존재 여부 확인 |

### 10.3 로그

| 함수 | 의미 |
|------|------|
| `Log(level, fmt, ...)` | 로그 기록 (레벨은 def_error.h 상수) |
| `SLog(level, fmt, ...)` | SHM 로그 기록 (지연 기록) |
| `Log_Proc(file, msg)` | 프로세스 로그 기록 |

### 10.4 SHM/IPC

| 함수 | 의미 |
|------|------|
| `SHM_Creat(key, size)` | 공유메모리 생성 |
| `SHM_Attach(shmid)` | 공유메모리 연결 |
| `SHM_Detach(ptr)` | 공유메모리 해제 |
| `SEM_Creat(key)` | 세마포어 생성 |
| `SEM_Lock(semid)` | 세마포어 잠금 |
| `SEM_UnLock(semid)` | 세마포어 해제 |

### 10.5 네트워크

| 함수 | 의미 |
|------|------|
| `Socket()` | TCP 소켓 생성 |
| `Connect(fd, ip, port)` | TCP 접속 |
| `Sendn(fd, buf, len)` | n바이트 전송 |
| `Recvn(fd, buf, len)` | n바이트 수신 |
| `Select(fd, timeout, mode)` | select 기반 대기 |
| `Select_Receive(fd, buf)` | 업무계 전문 수신 |
| `Select_Receive_Krx(fd, buf, hlen)` | KRX 직결 전문 수신 |
| `Select_Send(fd, buf, len)` | 전문 송신 |

### 10.6 파일 I/O

| 함수 | 의미 |
|------|------|
| `F_R(flag, buf, cnt)` | 파일 읽기 (카운터 갱신) |
| `F_W(flag, buf, cnt)` | 파일 쓰기 (카운터 갱신) |
| `F_R_Proc(flag, buf, cnt, proc)` | 프로세스 지정 파일 읽기 |
| `DSHM_R(idx, buf, size)` | 데이터SHM 읽기 |
| `DSHM_W(idx, buf, size)` | 데이터SHM 쓰기 |
| `Key_Search(mk, type, code)` | 종목코드 이진검색 |

---

## 11. 자주 사용되는 매크로 TOP 10

소스 코드에서 실제 가장 빈번하게 등장하는 매크로.

| 순위 | 매크로 | 헤더 파일 | 의미 | 전형적 사용 패턴 |
|------|--------|----------|------|----------------|
| 1 | `INT_SEQ` | fep_interface.h | 현재 프로세스 시퀀스 번호 | `INT_SEQ++` (시퀀스 증가) |
| 2 | `START_S` | fep_interface.h | 프로세스 시작 상태 | `if (START_S == JOB_START)` |
| 3 | `ON` / `OFF` | fep_sub.h | 플래그 활성/비활성 | `DeviceSendFlag = ON` |
| 4 | `KRX_HEAD_LEN` | pa_struct.h | KRX 헤더 82바이트 | `memcpy(&buf[KRX_HEAD_LEN], ...)` |
| 5 | `WRITE_CNT` / `READ_CNT` | fep_interface.h | 파일 R/W 카운터 | `if (WRITE_CNT > READ_CNT)` |
| 6 | `Log(level, ...)` | fep_sub.h (함수) | 로그 기록 | `Log(USR_ERROR, "에러 %d", err)` |
| 7 | `TIME_OUT` | fep_interface.h | 타임아웃(초) | `Select(fd, TIME_OUT, 0)` |
| 8 | `TCP2_LINE_ST` | fep_interface.h | TCP2 라인 상태 | `if (TCP2_LINE_ST == ON)` |
| 9 | `INPUT_FD` | fep_interface.h | 입력 FIFO fd | `Poll_File(INPUT_FD)` |
| 10 | `SESSION_S` | fep_interface.h | 세션/암복호화 상태 | `if (SESSION_S == 0)` (정상) |

---

## 부록: 매크로 전개 체인 완전 예시

### 예시 1: `INT_SEQ++` (시퀀스 증가)

```
INT_SEQ++
  └→ IF_SEQ(D_K, P_K)++                       [fep_interface.h:129]
       └→ PROC(D_K, P_K).if_seq++             [shm_memory.h:871]
            └→ Shm_Mem[D_K].Proc[P_K].if_seq++ [shm_memory.h:859]

의미: 현재 프로세스의 SHM 인터페이스 시퀀스 번호를 1 증가
```

### 예시 2: `O_W_CNT3` (출력파일3 쓰기 카운터 읽기)

```
O_W_CNT3
  └→ OFW_CNT(2, 0)                            [fep_interface.h:175]
       └→ OFW(D_K, P_K, 2, 0)                 [fep_interface.h:166]
            └→ FILEM(D_K, PROC(D_K,P_K).out_f[2]-1).w_cnt[0]
                                                [shm_memory.h:925]
                 └→ Shm_Mem[D_K].File[
                      Shm_Mem[D_K].Proc[P_K].out_f[2] - 1
                    ].w_cnt[0]                  [shm_memory.h:860]

의미: 현재 프로세스의 3번째 출력파일의 쓰기 카운터 값
```

### 예시 3: `TCP2_PORT_NO` (현재 TCP2 포트번호)

```
TCP2_PORT_NO
  └→ TCP2_PORT(D_K, P_K, S_K)                 [fep_interface.h:146]
       └→ TCP2(D_K, PROC(D_K,P_K).l.t2.l[S_K]-1).port_no
                                                [shm_memory.h:895]
            └→ Shm_Mem[D_K].Tcp2[
                 Shm_Mem[D_K].Proc[P_K].l.t2.l[S_K] - 1
               ].port_no                        [shm_memory.h:863]

의미: 현재 프로세스(P_K)가 사용하는 S_K번 TCP2 라인의 포트 번호
```

### 예시 4: `if (START_S == JOB_START)` (프로세스 상태 확인)

```
START_S == JOB_START
  └→ START_STAT(D_K, P_K) == 1                [fep_interface.h:127]
       └→ PROC(D_K, P_K).start_status == 1    [shm_memory.h:873]
            └→ Shm_Mem[D_K].Proc[P_K].start_status == 1

의미: 현재 프로세스의 SHM 시작상태가 "시작됨(1)"인지 확인
```
