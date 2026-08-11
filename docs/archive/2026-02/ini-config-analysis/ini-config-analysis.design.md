# 설계: ini-config-analysis

> PB FEP INI Configuration — Complete Analysis & Reference

## 개요

| Item | Detail |
|------|--------|
| Feature | ini-config-analysis |
| Phase | Design |
| Date | 2026-02-18 |
| Plan Ref | docs/01-plan/features/ini-config-analysis.plan.md |
| Environment | TEST (at05, 10.214.100.5) |
| Daemon | PB (Bond KRX FEP) |

---

## 1. INI File Architecture (FR-05, FR-06)

### 1.1 File Dependency Map

```
daemon.ini ─── Daemon_B (PB) 정의 ─── IPC 자원 할당량
    |
    +-- proc.ini ......... 18개 프로세스 정의
    |     |
    |     +-- tcp2.ini ... 16개 TCP 직접접속 포트 (Tcp2_Primary 매핑)
    |     +-- udpip.ini .. 2개 UDP 시세 포트 (Udp_Port 매핑)
    |     +-- file.ini ... 8개 파일/FIFO (OFN/IFN 매핑)
    |
    +-- tcp1.ini ......... Master Daemon 포트 (PB 미사용)
    +-- dshm.ini ......... 데이터 SHM (PB 미사용, Count=0)
    +-- sisetr.ini ....... 29개 시세 TR 코드 정의
    |
    +-- client.ini ....... 클라이언트 IP 허용목록
    +-- pc.ini ........... 관리자 PC IP 허용목록
```

### 1.2 Daemon Slot Structure (daemon.ini)

daemon.ini는 A~Z 총 26개 슬롯을 정의. PB는 **Daemon_B** 사용.

| Slot | Comment | Status | 비고 |
|------|---------|--------|------|
| **A** | STR | - | 빈 슬롯 (PA 용도 추정) |
| **B** | KB_SISE | **1 (RUN)** | PB 채권 FEP (활성) |
| C~V | 예비 | - | 예약 슬롯 19개 |
| **W** | TCPIP업무접속 | - | Comment만 존재 |
| **X** | Utility | - | FIFO: px_FIFO |
| **Y** | 운영 | - | FIFO: py_FIFO |
| **Z** | 관리 | - | FIFO: pz_FIFO, Compact=3일 |

### 1.3 Daemon_B 상세 설정

| 항목 | 값 | 설명 |
|------|------|------|
| ID | pb_daemon_mp | 데몬 마스터 프로세스 |
| Start_Time | **04:30** | 새벽 기동 |
| End_Time | **04:00** | 익일 새벽 종료 |
| Date_Flag | **2 (D~D+1)** | 당일 시작 ~ 익일까지 |
| Compact_Days | **9** | 9일 이전 데이터/로그 삭제 |
| Shm_Log | **1** | SHM 지연 로그 사용 |
| FIFO | pb_FIFO | 프로세스간 통신 |

### 1.4 IPC Resource Allocation

| 자원 | daemon.ini 할당 | 실제 사용 | 여유 | 상태 |
|------|----------------|----------|------|------|
| Proc_Count | 20 | 18 | 2 | OK |
| File_Count | 8 | **8** | **0** | **FULL** |
| Dshm_Count | 0 | 0 | - | 미사용 |
| Tcp2_Count | 20 | 16 | 4 | OK |
| Udpip_Count | 5 | 2 | 3 | OK |

### 1.5 Date_Flag 운영 시간

```
Date_Flag=2 (D ~ D+1):

  D-1일                      D일 (영업일)                  D+1일
   |                          |                              |
   +-------------|--|---------+--------|---------|-----------|---+
                           04:30                          04:00
                 데몬기동 ──|◄──── 23시간 30분 연속 운영 ───►|── 데몬종료
```

---

## 2. Process Relationship Map (FR-01)

### 2.1 Process Summary (proc.ini — 18 processes)

| # | Process ID | Comment | Type | Status | Port | KRX Login | Time |
|---|-----------|---------|------|--------|------|-----------|------|
| 1 | pb_7102_ur | 채권KTS 시세수신 | UR | S | UDP 10402 | - | 05:00~23:00 |
| 2 | pb_7103_ur | 채권KTS M4수신 | UR | S | UDP 10406 | - | 05:00~23:00 |
| 3 | pb_1301_tr | 주문수신 | TS2 | S | 41001 | - | 06:00~23:00 |
| 4 | pb_1101_ts | 채권주문송신(시장FEP) | TS2 | S | 37221 | M60501O001 | 07:30~19:00 |
| 5 | pb_1401_ts | 주문송신(오류) | TS2 | S | 41002 | - | 06:00~23:00 |
| 6 | pb_1201_tr | 채권응답체결수신(시장FEP) | TR2 | S | 57221 | M60501T001 | 07:30~19:00 |
| 7 | pb_1402_ts | 응답체결송신 | TS2 | S | 41003 | - | 06:00~23:00 |
| 8 | pb_1211_tr | 채권응답체결DP | TR2 | S | 61726 | M60501D001 | 07:30~19:00 |
| 9 | pb_8111_ts | 채권체결DP송신 | TS2 | S | 32009 | - | 06:00~23:00 |
| 10 | pb_1601_tr | KTS Market Info | TS2 | S | 61725 | M60501M001 | 08:00~18:10 |
| 11 | pb_1403_ts | 공개장운영정보 | TS2 | S | 41004 | - | 06:00~23:00 |
| 12 | pb_7801_tr | 채권TCP RDS수신 | TR2 | S | 63725 | M00501S001 | 08:00~02:30 |
| 13 | pb_1411_ts | 일괄송신 RDS | TS2 | S | 41005 | - | 06:00~02:00 |
| 14 | pb_8211_tr | 일괄수신 지점정보 | TS2 | S | 32019 | - | 06:00~23:00 |
| 15 | pb_1801_ts | 채권일괄수신 | TR2 | S | 63724 | M00501R001 | 07:00~02:00 |
| 16 | pb_8116_ts | 일괄송신 RDS | TS2 | S | 32016 | - | 06:00~23:00 |
| 17 | pb_1001_mp | SyncManager | MP | S | - | - | 05:00~03:30 |
| 18 | pb_2001_mp | LogManager | MP | **R** | - | - | 05:00~03:30 |

> Status: R=Run, S=Stop. TEST 환경에서는 LogManager만 실행 중.

### 2.2 Process Type Classification

| Type | 의미 | 프로세스 수 |
|------|------|-----------|
| UR | UDP Receive (시세 멀티캐스트) | 2 |
| TS2 | TCP Send (직접접속 방식) | 9 |
| TR2 | TCP Receive (직접접속 방식) | 5 |
| MP | Manager Process | 2 |

### 2.3 Data Flow Diagrams

#### Flow 1: KRX 주문 (Order)

```
사내시스템 ──TCP:41001──> pb_1301_tr (주문수신)
                              |
                         OFN=pb_1101_ts  [file.ini #1, 400B]
                              |
                              v
                         pb_1101_ts (주문송신) ──TCP:37221──> KRX
                         [LogonID: M60501O001]
                              |
                         OFN=pb_1401_ts  [file.ini #3, 400B]
                              |
                              v
                         pb_1401_ts (오류송신) ──TCP:41002──> 사내시스템
```

#### Flow 2: KRX 체결 (Execution Response)

```
KRX ──TCP:57221──> pb_1201_tr (체결수신)
                   [LogonID: M60501T001]
                        |
                   OFN=pb_1402_ts  [file.ini #4, 400B]
                        |
                        v
                   pb_1402_ts (체결송신) ──TCP:41003──> 사내시스템
```

#### Flow 3: KRX 체결 DropCopy

```
KRX ──TCP:61726──> pb_1211_tr (체결DP수신)
                   [LogonID: M60501D001]
                        |
                   OFN=pb_8111_ts  [file.ini #7, 400B]
                        |
                        v
                   pb_8111_ts (체결DP송신) ──TCP:32009──> 사내시스템
```

#### Flow 4: KRX 공개장운영정보 (Market Info)

```
KRX ──TCP:61725──> pb_1601_tr (장운영수신)
                   [LogonID: M60501M001]
                   [Timeout: 65초]
                        |
                   OFN=pb_1403_ts  [file.ini #5, 200B]
                        |
                        v
                   pb_1403_ts (장운영송신) ──TCP:41004──> 사내시스템
```

#### Flow 5: KRX 일괄송신 RDS (KRX -> Internal)

```
KRX ──TCP:63725──> pb_7801_tr (RDS수신)
                   [LogonID: M00501S001]
                        |
                   OFN=pb_1411_ts  [file.ini #6, 1300B]
                        |
                        v
                   pb_1411_ts (RDS송신) ──TCP:41005──> 사내시스템
```

#### Flow 6: KRX 일괄수신 (Internal -> KRX -> Internal)

```
사내시스템 ──TCP:32019──> pb_8211_tr (지점정보수신)
                              |
                         OFN=pb_1801_ts  [file.ini #2, 600B]
                              |
                              v
                         pb_1801_ts (채권일괄수신) ──TCP:63724──> KRX
                         [LogonID: M00501R001]
                              |
                         OFN=pb_8116_ts  [file.ini #8, 2048B]
                              |
                              v
                         pb_8116_ts (결과송신) ──TCP:32016──> 사내시스템
```

> Flow 6은 유일한 3단계 체인 (사내->KRX->사내 왕복)

#### Flow 7: 시세 수신 (Market Data — UDP)

```
KRX ──UDP:10402 (233.38.231.91)──> pb_7102_ur (채권KTS 시세)  --> SHM
KRX ──UDP:10406 (233.38.231.153)-> pb_7103_ur (채권KTS M4)    --> SHM

* 파일 IPC 없음 — 직접 SHM에 기록
* tcp2.ini 42001/42002 포트로 사내 TCP 배분 (proc에 별도 프로세스 미정의)
```

### 2.4 Operating Time Window

```
04:30  05:00   06:00  07:00 07:30  08:00      18:10 19:00       23:00  02:00 02:30 03:30 04:00
  |     |       |      |     |      |           |     |           |      |     |     |     |
  v     v       v      v     v      v           v     v           v      v     v     v     v
데몬  관리MP  사내TS  일괄  KRX접속  장운영/RDS  장운영  KRX종료   사내TS  일괄  RDS  관리  데몬
기동  시작    시작    수신  시작     시작        종료   종료       종료   종료  종료  종료  종료

계층별:
  pb_daemon_mp : |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| 04:30~04:00
  pb_1001/2001 :  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| 05:00~03:30
  사내 TS/TR   :    ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||     06:00~23:00
  KRX 직접접속 :         |||||||||||||||||||||||||||||                              07:30~19:00
  장운영정보   :            |||||||||||||||||||||                                   08:00~18:10
  RDS/일괄     :            ||||||||||||||||||||||||||||||||||||||||||||||           08:00~02:30
```

---

## 3. Network Topology (FR-02)

### 3.1 TCP Direct Connections (tcp2.ini — 16 ports)

#### Internal Ports (0.0.0.0 bind — 사내시스템 접속)

| # | Port | Comment | Process | Direction |
|---|------|---------|---------|-----------|
| 1 | **32009** | 채권체결DP송신 | pb_8111_ts | FEP -> 사내 |
| 2 | **32019** | 채권Batch송신 | pb_8211_tr | 사내 -> FEP |
| 3 | **32016** | 일괄수신결과송신 | pb_8116_ts | FEP -> 사내 |
| 4 | **41001** | 주문수신 | pb_1301_tr | 사내 -> FEP |
| 5 | **41002** | 주문오류송신 | pb_1401_ts | FEP -> 사내 |
| 6 | **41003** | 응답송신 | pb_1402_ts | FEP -> 사내 |
| 7 | **41004** | 공개장운영송신 | pb_1403_ts | FEP -> 사내 |
| 8 | **41005** | 일괄송신 RDS | pb_1411_ts | FEP -> 사내 |
| 9 | **42001** | 채권체결호가송신 | *(시세배분)* | FEP -> 사내 |
| 10 | **42002** | 채권M4송신 | *(시세배분)* | FEP -> 사내 |

#### KRX Ports (192.168.157.x — KRX TEST Gateway)

| # | Port | IP | Comment | Process | Direction |
|---|------|----|---------|---------|-----------|
| 11 | **37221** | .119 | 채권주문송신 | pb_1101_ts | FEP -> KRX |
| 12 | **57221** | .119 | 채권응답체결수신 | pb_1201_tr | KRX -> FEP |
| 13 | **61726** | .120 | 채권체결DP | pb_1211_tr | KRX -> FEP |
| 14 | **61725** | .119 | 공개장운영 | pb_1601_tr | KRX -> FEP |
| 15 | **63724** | .120 | RDS일괄수신 | pb_1801_ts | FEP -> KRX |
| 16 | **63725** | .120 | 일괄송신_KRX | pb_7801_tr | KRX -> FEP |

### 3.2 KRX Gateway IP Distribution

| IP | Processes | Port Range |
|----|-----------|------------|
| **192.168.157.119** | pb_1101_ts, pb_1201_tr, pb_1601_tr | 37221, 57221, 61725 |
| **192.168.157.120** | pb_1211_tr, pb_1801_ts, pb_7801_tr | 61726, 63724, 63725 |

- .119: 주문/체결/장운영 (주요 거래)
- .120: DropCopy/RDS/일괄 (보조 업무)

### 3.3 Port Range Convention

| 대역 | 방향 | 용도 |
|------|------|------|
| **3xxxx** (37221) | FEP -> KRX | 주문 송신 |
| **3200x** (32009/16/19) | 사내 <-> FEP | 체결DP, 일괄 |
| **4100x** (41001~05) | FEP -> 사내 | 주문응답, 체결, 장운영, RDS |
| **4200x** (42001~02) | FEP -> 사내 | 시세 배분 |
| **5xxxx** (57221) | KRX -> FEP | 체결 수신 |
| **6xxxx** (61725~63725) | KRX <-> FEP | DropCopy, 장운영, RDS |

### 3.4 TCP Master Daemon (tcp1.ini — PB 미사용)

| Group | Comment | Master Port | Service Ports |
|-------|---------|-------------|---------------|
| Tcp1_1 | TCP업무접속(P) | 35010 | 35011~35019 (9개) |
| Tcp1_2 | TCP업무접속(B) | 45010 | 45011~45019 (9개) |

PB 데몬은 모두 TCP2(직접접속)를 사용하여 tcp1은 사용하지 않음. PA 모듈에서 사용 추정.

### 3.5 UDP Multicast (udpip.ini — 2 channels)

| # | Comment | Multicast IP | Port | Process |
|---|---------|-------------|------|---------|
| 1 | 채권시세수신 | **233.38.231.91** | **10402** | pb_7102_ur |
| 2 | 채권장운영수신 | **233.38.231.153** | **10406** | pb_7103_ur |

#### KRX UDP Port Scheme (Reference)

```
채권시세 (8M): 233.38.231.91/152
  운영: 10402 | TEST1: 11402 | TEST2: 12402 | 재전송: 20401

채권장운영 (8M/512K): 233.38.231.153
  운영: 10406 | TEST1: 11406 | TEST2: 12406 | 재전송: 20401

현재 설정: 운영 포트 (10402, 10406) 사용
```

### 3.6 Complete Network Diagram

```
                    +------------------------------------+
                    |         KRX (한국거래소)              |
                    |   192.168.157.119 / .120 (TEST)     |
                    +------+------+------+------+---------+
                           |      |      |      |
        +------------------+------+------+------+------------------+
        |                  |      |      |      |                  |
   37221|주문  57221|체결  61726|DP  61725|장운영  63724|일괄up 63725|일괄dn
        |          |       |       |       |          |
        v          v       v       v       v          v
  +------------------------------------------------------------+
  |                    PB FEP Server (at05)                     |
  |                                                            |
  |  [pb_1101_ts] [pb_1201_tr] [pb_1211_tr] [pb_1601_tr]      |
  |   주문송신     체결수신     DP수신       장운영수신         |
  |      |파일         |파일        |파일        |파일          |
  |  [pb_1301_tr] [pb_1402_ts] [pb_8111_ts] [pb_1403_ts]      |
  |   주문수신     체결송신     DP송신       장운영송신         |
  |      |41001       |41003      |32009      |41004           |
  |                                                            |
  |  [pb_7801_tr] [pb_1801_ts] [pb_8211_tr]                   |
  |   RDS수신     일괄수신     지점정보      + pb_1401(41002)  |
  |      |            |파일        |파일       + pb_1411(41005) |
  |      |파일    [pb_8116_ts]                 + pb_8116(32016) |
  |      |         결과송신                                     |
  |  [pb_1411_ts]    |32016                                    |
  |   RDS송신                                                  |
  |      |41005                                                |
  |                                                            |
  |  [pb_7102_ur](UDP:10402) [pb_7103_ur](UDP:10406)          |
  |  [pb_1001_mp](Sync)     [pb_2001_mp](Log)                 |
  +----+-----------------------------------------------+-------+
       |                                               |
  41001~41005, 42001~42002, 32009, 32016, 32019        |
       |                                          UDP Multicast
       v                                          233.38.231.x
  +-------------------+                        +---------------+
  |   사내 시스템       |                        |   KRX 시세    |
  |  (업무계, 지점)     |                        |   UDP Feed   |
  +-------------------+                        +---------------+
```

---

## 4. File/FIFO IPC Map (FR-03)

### 4.1 File Definitions (file.ini — 8 files)

| # | File Name | Comment | Size | Net Data | Producer | Consumer |
|---|-----------|---------|------|----------|----------|----------|
| 1 | pb_1101_ts | 채권주문송신 | 400B | 315B | pb_1301_tr | pb_1101_ts |
| 2 | pb_1801_ts | 채권일괄수신_KRX | 600B | 515B | pb_8211_tr | pb_1801_ts |
| 3 | pb_1401_ts | 주문송신_오류 | 400B | 315B | pb_1101_ts | pb_1401_ts |
| 4 | pb_1402_ts | 회원처리호가및체결 | 400B | 315B | pb_1201_tr | pb_1402_ts |
| 5 | pb_1403_ts | 공개장운영정보 | 200B | 115B | pb_1601_tr | pb_1403_ts |
| 6 | pb_1411_ts | 일괄송신_RDS | 1300B | 1215B | pb_7801_tr | pb_1411_ts |
| 7 | pb_8111_ts | 채권체결DP송신 | 400B | 315B | pb_1211_tr | pb_8111_ts |
| 8 | pb_8116_ts | 일괄수신결과송신 | 2048B | 1963B | pb_1801_ts | pb_8116_ts |

### 4.2 Record Structure

```
+--------------------------------------------------------------------+
| File Header (62B) | Data Header (22B) |    Data Payload    | LF(1B) |
+--------------------------------------------------------------------+
                                        |<--- Net Data Size --->|

Net Data = Size - 62 - 22 - 1 = Size - 85
```

### 4.3 Size Rationale

| Size | Net Data | Files | 전문 크기 근거 |
|------|----------|-------|--------------|
| 200B | 115B | pb_1403 | 장운영정보 A701K(68B) + 여유 |
| 400B | 315B | pb_1101/1401/1402/8111 | 주문(254~255B), 체결(317B) |
| 600B | 515B | pb_1801 | 일괄수신 전문 |
| 1300B | 1215B | pb_1411 | RDS 일괄 대용량 |
| 2048B | 1963B | pb_8116 | 일괄수신결과 최대 크기 |

### 4.4 FIFO Notification

모든 파일의 `Fifo=1` — 각 파일마다 1개의 FIFO가 연결됨.
proc.ini에서 `FFN_1` 필드로 FIFO 이름 정의:

| File | FIFO (FFN_1) | Reader Process |
|------|-------------|----------------|
| pb_1101_ts | pb_1101_ts1 | pb_1101_ts |
| pb_1401_ts | pb_1401_ts1 | pb_1401_ts |
| pb_1402_ts | pb_1402_ts1 | pb_1402_ts |
| pb_1403_ts | pb_1403_ts1 | pb_1403_ts |
| pb_1411_ts | pb_1411_ts1 | pb_1411_ts |
| pb_8111_ts | pb_8111_ts1 | pb_8111_ts |
| pb_8116_ts | pb_8116_ts1 | pb_8116_ts |
| pb_1801_ts | *(없음)* | pb_1801_ts |

> pb_1801_ts는 FFN 없이 IFN만 사용 — polling 방식 추정.

---

## 5. Market Data TR Catalog (FR-04)

### 5.1 TR Summary (sisetr.ini — 29 TRs)

| Queue | Market | TRs |
|-------|--------|-----|
| 5 | 유가증권/코스닥 | 6 |
| 10 | 지수 | 3 |
| 3 | ELW/주식선물 | 8 |
| 1 | 지수선물 | 4 |
| 2 | 지수옵션 | 4 |
| 4 | 주식옵션 | 4 |

### 5.2 Full TR List

#### 유가증권 (Queue=5)

| TR | Comment | Length | Market |
|----|---------|--------|--------|
| A0011 | 종목정보 | 800 | 유가증권 |
| A3011 | 체결 | 160 | 유가증권 |
| B6011 | 호가 | 560 | 유가증권 |
| A0012 | 종목정보 | 800 | 코스닥 |
| A3012 | 체결 | 160 | 코스닥 |
| B6012 | 호가 | 560 | 코스닥 |

#### 지수 (Queue=10)

| TR | Comment | Length |
|----|---------|--------|
| C8011 | KRX100지수 | 50 |
| D0011 | KOSPI지수 | 50 |
| D2011 | KOSPI200지수 | 50 |

#### ELW (Queue=3)

| TR | Comment | Length |
|----|---------|--------|
| A1011 | 종목정보(ELW) | 910 |
| I7011 | LP정보 | 250 |
| A3021 | ELW체결 | 160 |
| B7021 | ELW호가잔량(LP포함) | 800 |

#### 지수선물 (Queue=1)

| TR | Comment | Length |
|----|---------|--------|
| A0014 | 종목배치 | 1300 |
| A3014 | 체결 | 117 |
| B6014 | 우선호가 | 220 |
| G7014 | 체결/우선호가 | 299 |

#### 지수옵션 (Queue=2)

| TR | Comment | Length |
|----|---------|--------|
| A0034 | 종목배치 | 1300 |
| A3034 | 체결 | 104 |
| B6034 | 우선호가 | 223 |
| G7034 | 체결/우선호가 | 288 |

#### 주식선물 (Queue=3)

| TR | Comment | Length |
|----|---------|--------|
| A0015 | 종목배치 | 1300 |
| A3015 | 체결 | 113 |
| B6015 | 우선호가 | 448 |
| G7015 | 체결/우선호가 | 539 |

#### 주식옵션 (Queue=4)

| TR | Comment | Length |
|----|---------|--------|
| A0025 | 종목배치 | 1300 |
| A3025 | 체결 | 114 |
| B6025 | 우선호가 | 448 |
| G7025 | 체결/우선호가 | 520 |

### 5.3 TR Code Naming Convention

```
TR Code = {Type}{Seq}{Market}

Type:
  A0 = 종목정보/배치    B6 = 우선호가      C8 = KRX100지수
  A1 = 종목정보(ELW)    B7 = ELW호가       D0 = KOSPI지수
  A3 = 체결             G7 = 체결+호가     D2 = KOSPI200지수
  I7 = LP정보

Market (last digit):
  1 = 유가증권     4 = 지수선물/옵션
  2 = 코스닥       5 = 주식선물/옵션
```

### 5.4 Derivatives TR Size Comparison

| Market | 종목배치 | 체결 | 호가 | 체결+호가 |
|--------|---------|------|------|----------|
| 지수선물 | 1300 | 117 | 220 | 299 |
| 지수옵션 | 1300 | 104 | 223 | 288 |
| 주식선물 | 1300 | 113 | 448 | 539 |
| 주식옵션 | 1300 | 114 | 448 | 520 |

주식선물/옵션 호가가 지수 대비 2배 — 호가 단계 수 차이.

### 5.5 UDP Receiver Mapping

```
pb_7102_ur (UDP:10402, 233.38.231.91)
  -> 채권 시세 TR: A701K(68B), B601K(462B), A301K(223B),
                   G701K(643B), A601K(57B), I2000(10B)
  -> TCP 배분: 42001 (채권체결호가)

pb_7103_ur (UDP:10406, 233.38.231.153)
  -> 장운영 TR: M4(83B), R3, R4
  -> TCP 배분: 42002 (채권M4)
```

---

## 6. Data SHM (dshm.ini)

PB 데몬은 데이터 SHM을 사용하지 않음 (`Dshm_Count=0`).
프로세스 간 통신은 전부 **파일/FIFO 기반**.

부문 SHM(shm_memory.h 정의)은 별도로 사용:

```
SHM Key Structure: 0x(Sys)(Env)(Division)(Segment)(Type)
  Sys=2(PK), Env=1(Real)/2(Test), Division=02(PB)

  예: 0x12020000 = Test, PB, 부문SHM
```

---

## 7. KRX Login Credentials (6 accounts)

| LogonID | Process | Purpose | Port | Gateway |
|---------|---------|---------|------|---------|
| M60501O001 | pb_1101_ts | 주문 송신 | 37221 | .119 |
| M60501T001 | pb_1201_tr | 체결 수신 | 57221 | .119 |
| M60501D001 | pb_1211_tr | 체결 DropCopy | 61726 | .120 |
| M60501M001 | pb_1601_tr | 장운영정보 | 61725 | .119 |
| M00501S001 | pb_7801_tr | RDS 일괄(KRX->FEP) | 63725 | .120 |
| M00501R001 | pb_1801_ts | 일괄수신(FEP->KRX) | 63724 | .120 |

ID Pattern: `M{code}X001` where X=O(Order), T(Trade), D(DropCopy), M(Market), S(Send), R(Receive)

---

## 8. Cross-file Consistency Matrix (FR-06)

### 8.1 daemon.ini vs Actual Counts

| Resource | Allocated | Used | Spare | Status |
|----------|-----------|------|-------|--------|
| Proc_Count | 20 | 18 | 2 | OK |
| File_Count | 8 | 8 | 0 | FULL |
| Dshm_Count | 0 | 0 | - | OK |
| Tcp2_Count | 20 | 16 | 4 | OK |
| Udpip_Count | 5 | 2 | 3 | OK |

### 8.2 proc.ini Tcp2_Primary vs tcp2.ini

| proc Port | tcp2 Port | Match |
|-----------|-----------|-------|
| 41001 | Tcp2_4 (41001) | OK |
| 37221 | Tcp2_11 (37221) | OK |
| 41002 | Tcp2_5 (41002) | OK |
| 57221 | Tcp2_12 (57221) | OK |
| 41003 | Tcp2_6 (41003) | OK |
| 61726 | Tcp2_13 (61726) | OK |
| 32009 | Tcp2_1 (32009) | OK |
| 61725 | Tcp2_14 (61725) | OK |
| 41004 | Tcp2_7 (41004) | OK |
| 63725 | Tcp2_16 (63725) | OK |
| 41005 | Tcp2_8 (41005) | OK |
| 32019 | Tcp2_2 (32019) | OK |
| 63724 | Tcp2_15 (63724) | OK |
| 32016 | Tcp2_3 (32016) | OK |
| - | Tcp2_9 (42001) | **Orphan** |
| - | Tcp2_10 (42002) | **Orphan** |

**14/16 matched.** Port 42001, 42002 have no proc.ini process — used for UDP-to-TCP market data distribution.

### 8.3 proc.ini Udp_Port vs udpip.ini

| proc Port | udpip Port | Match |
|-----------|-----------|-------|
| 0,10402 | Udpip_1 (10402) | OK |
| 0,10406 | Udpip_2 (10406) | OK |

**2/2 matched.**

### 8.4 proc.ini OFN/IFN vs file.ini

| File Name | OFN Owner | IFN Owner | file.ini # | Match |
|-----------|-----------|-----------|------------|-------|
| pb_1101_ts | pb_1301_tr | pb_1101_ts | #1 | OK |
| pb_1401_ts | pb_1101_ts | pb_1401_ts | #3 | OK |
| pb_1402_ts | pb_1201_tr | pb_1402_ts | #4 | OK |
| pb_8111_ts | pb_1211_tr | pb_8111_ts | #7 | OK |
| pb_1403_ts | pb_1601_tr | pb_1403_ts | #5 | OK |
| pb_1411_ts | pb_7801_tr | pb_1411_ts | #6 | OK |
| pb_1801_ts | pb_8211_tr | pb_1801_ts | #2 | OK |
| pb_8116_ts | pb_1801_ts | pb_8116_ts | #8 | OK |

**8/8 matched.**

---

## 9. Findings & Recommendations

### 9.1 Key Findings

| # | Finding | Severity |
|---|---------|----------|
| 1 | File_Count at capacity (8/8) | Medium |
| 2 | tcp2 ports 42001/42002 orphaned from proc.ini | Low |
| 3 | tcp1.ini defined but unused by PB | Info |
| 4 | All proc Status=S except pb_2001_mp | Info (TEST normal) |
| 5 | pb_1801_ts has no FFN (FIFO) — polling mode | Info |
| 6 | sisetr.ini has 29 TRs but only 2 UDP receivers | Info |

### 9.2 Operational Notes

- **File_Count 확장 필요 시**: daemon.ini `Daemon_B_File_Count` 증가 + file.ini 항목 추가 + SHM 재생성
- **새 프로세스 추가 시**: Proc_Count 여유 2개 있음. proc.ini + tcp2.ini 동시 업데이트 필요
- **KRX 로그온 계정**: LogonPW는 Base64 인코딩. 환경별(TEST/REAL)로 다를 수 있음
- **시세 42001/42002**: UDP 수신 프로세스가 내부 TCP로 배분하는 포트. 별도 proc 정의 없이 동일 프로세스 내에서 처리

---

## Appendix A: Configuration File Quick Reference

| File | Section Prefix | Count Field | Items |
|------|---------------|-------------|-------|
| daemon.ini | DAEMON_{A-Z}_ | - | 26 slots |
| proc.ini | Proc_{1-N}_ | Proc_Count=18 | 18 processes |
| tcp1.ini | Tcp1_{1-N}_ | Tcp1_Count=2 | 2 master groups |
| tcp2.ini | Tcp2_{1-N}_ | Tcp2_Count=16 | 16 direct ports |
| udpip.ini | Udpip_{1-N}_ | Udpip_Count=2 | 2 UDP channels |
| file.ini | File_{1-N}_ | File_Count=8 | 8 files |
| dshm.ini | Dshm_{1-N}_ | Dshm_Count=0 | 0 segments |
| sisetr.ini | Sise_{1-N}_ | Sise_Count=29 | 29 TRs |
| client.ini | *(IP list)* | - | Client IPs |
| pc.ini | *(IP list)* | - | Admin PC IPs |

## Appendix B: Process ID Naming Convention

```
Format: pb_{NNNN}_{type}

pb = PB module (Bond)

NNNN function codes:
  1001 = SyncManager
  1101 = Bond order transmit
  1201 = Bond response/execution receive
  1211 = Bond execution DropCopy
  1301 = Order receive (from internal)
  1401 = Order error forward
  1402 = Execution response forward
  1403 = Market info forward
  1411 = RDS batch forward
  1601 = Market info receive
  1801 = Batch receive (to KRX)
  2001 = LogManager
  7102 = Market data receive (quote)
  7103 = Market data receive (M4)
  7801 = RDS batch receive (from KRX)
  8111 = DropCopy forward
  8116 = Batch result forward
  8211 = Branch info receive

Type: mp=manager, ts=TCP send, tr=TCP receive, ur=UDP receive
```
