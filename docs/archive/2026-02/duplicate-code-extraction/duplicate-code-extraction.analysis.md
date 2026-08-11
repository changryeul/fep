# duplicate-code-extraction 분석 보고서

> **분석 유형**: 갭 분석 (설계 vs 구현)
>
> **프로젝트**: FEP (KRX용 프론트엔드 프로세서)
> **분석자**: Claude Code (gap-detector)
> **날짜**: 2026-02-22
> **설계 문서**: [duplicate-code-extraction.design.md](../02-design/features/duplicate-code-extraction.design.md)

---

## 1. 분석 개요

### 1.1 분석 목적

중복 코드 추출(3개 공유 함수 + 2개 패턴 감시)의 구현이 설계 문서와 일치하는지 검증합니다. 이것은 "duplicate-code-extraction" 기능의 PDCA 사이클 검증 단계입니다.

### 1.2 분석 범위

- **설계 문서**: `docs/02-design/features/duplicate-code-extraction.design.md`
- **새 공유 함수**: `st01/sub/ip_format.c`, `st01/sub/udp_init.c`, `st01/sub/poll_event.c`
- **헤더**: `st01/inc/fep_common.h`
- **패턴 A 호출자** (4개 파일): pa_7000_us.c, pa_7010_us.c, pa_7030_us.c, pa_9999_us.c
- **패턴 B 호출자** (5개 파일): pa_7000_us.c, pa_7010_us.c, pa_7030_us.c, pa_9999_us.c, pa_7500_us.c
- **패턴 C 호출자** (7개 파일): pa_1100_ts.c, pb_1100_ts.c, pa_3100_ts.c, pa_7100_ts.c, pa_8100_ts.c, pb_8200_tr.c, pa_7800_tr.c
- **감시 전용** (D+E): 12개 파일, 읽기 전용 검증
- **분석 날짜**: 2026-02-22

---

## 2. 전체 점수

| 카테고리 | 점수 | 상태 |
|----------|:-----:|:------:|
| 설계 일치 | 100% | PASS |
| 아키텍처 준수 | 100% | PASS |
| 규약 준수 | 100% | PASS |
| **전체** | **100%** | **PASS** |

---

## 3. 공유 함수 검증 (V-01)

### 3.1 sub/ip_format.c -- format_ip_addr()

| 검사 | 설계 | 구현 | 상태 |
|-------|--------|----------------|--------|
| 파일 있음 | `sub/ip_format.c` | `st01/sub/ip_format.c` (27줄) | PASS |
| 포함 | `fep_fepp.h` | `fep_fepp.h` | PASS |
| 서명 | `void format_ip_addr(const char *packed_ip, char *dotted_ip, int buf_size)` | 정확히 일치 | PASS |
| 본문: AtoIf 호출 | 오프셋 0,3,6,9로 4개 호출 | 정확히 일치 | PASS |
| 본문: memset + sprintf | `memset` 다음 `sprintf "%d.%d.%d.%d"` | 정확히 일치 | PASS |
| 코멘트 블록 | 함수/설명/매개변수 | 있음, 정확함 | PASS |

**점수: 100%** (6/6 검사 통과)

### 3.2 sub/udp_init.c -- init_udp_socket()

| 검사 | 설계 | 구현 | 상태 |
|-------|--------|----------------|--------|
| 파일 있음 | `sub/udp_init.c` | `st01/sub/udp_init.c` (75줄) | PASS |
| 포함 | `fep_fepp.h`, `sys/socket.h`, `netinet/in.h`, `arpa/inet.h` | 정확히 일치 (4개 헤더) | PASS |
| 서명 | 7개 매개변수: svr_addr, clnt_addr, svr_ip, svr_port, sndbuf_size, rcvbuf_size, broadcast | 정확히 일치 | PASS |
| 반환 유형 | `int` (fd 또는 -1) | 정확히 일치 | PASS |
| 소켓 생성 | `socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)` | 정확히 일치 | PASS |
| 에러 로그 (열기) | `Log(UDP_FATAL, "Socket Open Error[%d:%s]", SYS_NO, SYS_STR)` | 정확히 일치 | PASS |
| svr_addr 채우기 | sin_family, inet_addr을 통한 sin_addr, htons을 통한 sin_port | 정확히 일치 | PASS |
| clnt_addr 채우기 | INADDR_ANY, 포트 0 | 정확히 일치 | PASS |
| SO_SNDBUF 조건 | `if (sndbuf_size > 0)` setsockopt | 정확히 일치 | PASS |
| SO_RCVBUF 조건 | `if (rcvbuf_size > 0)` setsockopt | 정확히 일치 | PASS |
| SO_BROADCAST 조건 | `if (broadcast)` optval=1 | 정확히 일치 | PASS |
| 바인드 + 에러 | `bind()`, 실패 시 닫기, -1 반환 | 정확히 일치 | PASS |
| 에러 로그 (바인드) | `Log(UDP_FATAL, "Socket Bind Error[%d:%s]", SYS_NO, SYS_STR)` | 정확히 일치 | PASS |
| sockfd 반환 | 성공 시 `return sockfd` | 정확히 일치 | PASS |

**점수: 100%** (14/14 검사 통과)

### 3.3 sub/poll_event.c -- detect_poll_event()

| 검사 | 설계 | 구현 | 상태 |
|-------|--------|----------------|--------|
| 파일 있음 | `sub/poll_event.c` | `st01/sub/poll_event.c` (51줄) | PASS |
| 포함 | `fep_fepp.h`, `poll.h` | 정확히 일치 | PASS |
| 서명 | `int detect_poll_event(struct pollfd *poll_arr, int poll_cnt, int socket_event_idx)` | 정확히 일치 | PASS |
| 반환 값 | >= 0 (POLLIN idx), -1 (소켓 HUP), -2 (이벤트 없음) | 정확히 일치 | PASS |
| POLLHUP 루프 | revents & POLLHUP을 검사하는 for-루프 | 정확히 일치 | PASS |
| 소켓 HUP 검사 | `if (i == socket_event_idx)` return -1 | 정확히 일치 | PASS |
| 소켓 HUP 로그 | `Log(TCP_ERROR, "socket disconnected[%#06x]", ...)` | 정확히 일치 | PASS |
| 비소켓 HUP 로그 | `Log(SYS_ERROR, "poll hangup[%d,%d]", i, poll_cnt)` | 정확히 일치 | PASS |
| POLLIN 루프 | revents & POLLIN을 검사하는for-루프 | 정확히 일치 | PASS |
| POLLIN 클리어 | `poll_arr[i].revents = 0` | 정확히 일치 | PASS |
| 이벤트 없음 반환 | `return -2` | 정확히 일치 | PASS |

**점수: 100%** (11/11 검사 통과)

---

## 4. 헤더 선언 검증 (V-02)

**파일**: `st01/inc/fep_common.h` 줄 44-51

| 검사 | 설계 | 구현 | 상태 |
|-------|--------|----------------|--------|
| 코멘트 블록 | "공유 유틸리티 함수 프로토타입" | 줄 44에 있음 | PASS |
| format_ip_addr | `extern void format_ip_addr(const char *, char *, int)` | 줄 46: 정확히 일치 | PASS |
| init_udp_socket | `extern int init_udp_socket(struct sockaddr_in *, ...)` 7개 param | 줄 47-49: 정확히 일치 | PASS |
| detect_poll_event | `extern int detect_poll_event(struct pollfd *, int, int)` | 줄 50-51: 정확히 일치 | PASS |
| 배치 | `#endif` 이전 | 줄 44-51, 줄 62의 `#endif` 이전 | PASS |

**점수: 100%** (5/5 검사 통과)

---

## 5. 패턴 A -- IP 주소 형식화 (V-03, V-04)

### 5.1 호출자 대체

| FR ID | 파일 | format_ip_addr() 호출 | 이전 AtoIf+sprintf 제거 | 이전 변수 (in1-in4) 제거 | 점수 |
|-------|------|:-----------------------:|:-------------------------:|:--------------------------:|:-----:|
| FR-A01 | pa_7000_us.c:154 | PASS | PASS | PASS | 100% |
| FR-A02 | pa_7010_us.c:154 | PASS | PASS | PASS | 100% |
| FR-A03 | pa_7030_us.c:211 | PASS | PASS | PASS | 100% |
| FR-A04 | pa_9999_us.c:181 | PASS | PASS | PASS | 100% |

### 5.2 매개변수 검증

| FR ID | 인수 | 설계 | 실제 | 상태 |
|-------|----------|--------|--------|--------|
| FR-A01 | packed_ip | `ACCNO(D_K,i).ip_addr` | `ACCNO(D_K,i).ip_addr` | PASS |
| FR-A02 | packed_ip | `ACCNO(D_K,i).ip_addr` | `ACCNO(D_K,i).ip_addr` | PASS |
| FR-A03 | packed_ip | `ACCNO(D_K,i).ip_addr` | `ACCNO(D_K,i).ip_addr` | PASS |
| FR-A04 | packed_ip | `ACCNO(D_K,i+Acc_No).ip_addr` | `ACCNO(D_K,i+Acc_No).ip_addr` | PASS |

### 5.3 음성 검사 (V-03)

4개 호출자 파일에서 `in1 = AtoIf`를 grep: **0 일치** -- 모든 인라인 IP 형식화 제거됨.
4개 호출자 파일에서 `in1 = in2 = in3 = in4`를 grep: **0 일치** -- 모든 이전 초기화 제거됨.

**패턴 A 점수: 100%** (4/4 호출자, 모든 검사 통과)

---

## 6. 패턴 B -- UDP 소켓 초기화 (V-05, V-06)

### 6.1 호출자 대체

| FR ID | 파일 | init_udp_socket() 호출 | 이전 socket/setsockopt/bind 제거 | 점수 |
|-------|------|:------------------------:|:----------------------------------:|:-----:|
| FR-B01 | pa_7000_us.c:156-158 | PASS | PASS | 100% |
| FR-B02 | pa_7010_us.c:156-158 | PASS | PASS | 100% |
| FR-B03 | pa_7030_us.c:215-217 | PASS | PASS | 100% |
| FR-B04 | pa_9999_us.c:183-185 | PASS | PASS | 100% |
| ~~FR-B05~~ | ~~pa_7000_mp.c~~ | 올바르게 제외됨 | inet_pton 인라인 유지 | PASS |
| FR-B06 | pa_7500_us.c:163-165 | PASS | PASS | 100% |

### 6.2 매개변수 검증

| FR ID | sndbuf | rcvbuf | broadcast | 설계 | 실제 | 상태 |
|-------|:------:|:------:|:---------:|--------|--------|--------|
| FR-B01 | 1024*64 | 0 | 1 | 표준 | `1024*64, 0, 1` | PASS |
| FR-B02 | 1024*64 | 0 | 1 | 표준 | `1024*64, 0, 1` | PASS |
| FR-B03 | 1024*64 | 1024*64 | 1 | 2D+var 포트 | `1024*64, 1024*64, 1` | PASS |
| FR-B04 | 1024*64 | 0 | 1 | 표준 | `1024*64, 0, 1` | PASS |
| FR-B06 | 1024*64 | 1024*64 | 0 | 브로드캐스트 없음 | `1024*64, 1024*64, 0` | PASS |

### 6.3 구조적 검증

| FR ID | IP 소스 | 포트 소스 | 배열 차원 | 에러 처리 | 상태 |
|-------|-----------|-------------|:---------------:|:--------------:|--------|
| FR-B01 | `Svr_IP` (1D) | `SVR_PORT_NO` | 1D `[i]` | `return 0` | PASS |
| FR-B02 | `Svr_IP` (1D) | `SVR_PORT_NO` | 1D `[i]` | `return 0` | PASS |
| FR-B03 | `Svr_IP` (1D) | `ports[j]` | 2D `[i][j]` | `return 0` | PASS |
| FR-B04 | `Svr_IP` (1D) | `SVR_PORT_NO` | 1D `[i]` | `return 0` | PASS |
| FR-B06 | `&Svr_IP[20*i]` | `SVR_PORT_NO` | 1D `[i]` | `return 0` | PASS |

### 6.4 음성 검사 (V-05)

모든 5개 호출자 파일에서 `setsockopt|SO_SNDBUF|SO_RCVBUF|SO_BROADCAST|SOCK_DGRAM|IPPROTO_UDP`를 grep: **0 일치** -- 모든 인라인 소켓 초기화 코드 제거됨.

### 6.5 제외 검증

pa_7000_mp.c는 `init_udp_socket()`을 호출하지 않으며 줄 357에서 `inet_pton()` 인라인 유지. 설계 결정에 따라 올바르게 제외됨 (`inet_pton` + `#ifdef` 포트 로직이 공유 함수와 호환 불가).

**패턴 B 점수: 100%** (5/5 호출자 + 1 제외 검증, 모든 검사 통과)

---

## 7. 패턴 C -- Poll 이벤트 감지 (V-07, V-08)

### 7.1 호출자 대체

| FR ID | 파일 | detect_poll_event() 호출 | 이전 POLLHUP 루프 제거 | 이전 POLLIN 루프 제거 | 점수 |
|-------|------|:--------------------------:|:------------------------:|:-----------------------:|:-----:|
| FR-C01 | pa_1100_ts.c:246 | PASS | PASS | PASS | 100% |
| FR-C02 | pb_1100_ts.c:267 | PASS | PASS | PASS | 100% |
| FR-C03 | pa_3100_ts.c:212 | PASS | PASS | PASS | 100% |
| FR-C04 | pa_7100_ts.c:146 | PASS | PASS | PASS | 100% |
| FR-C05 | pa_8100_ts.c:198 | PASS | PASS | PASS | 100% |
| FR-C06 | pb_8200_tr.c:176 | PASS | PASS | PASS | 100% |
| FR-C07 | pa_7800_tr.c:146 | PASS | PASS | PASS | 100% |

### 7.2 반환 값 처리

모든 7개 호출자는 동일한 패턴 사용:
```c
i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);
if (i == -1) return;        /* 소켓 연결 끊김 */
if (i == -2) continue;      /* 이벤트 없음 */
```

| FR ID | 파일 | `-1` 처리 | `-2` 처리 | 상태 |
|-------|------|:-------------:|:-------------:|--------|
| FR-C01 | pa_1100_ts.c:247-248 | `return` | `continue` | PASS |
| FR-C02 | pb_1100_ts.c:268-269 | `return` | `continue` | PASS |
| FR-C03 | pa_3100_ts.c:213-214 | `return` | `continue` | PASS |
| FR-C04 | pa_7100_ts.c:147-148 | `return` | `continue` | PASS |
| FR-C05 | pa_8100_ts.c:199-200 | `return` | `continue` | PASS |
| FR-C06 | pb_8200_tr.c:177-178 | `return` | `continue` | PASS |
| FR-C07 | pa_7800_tr.c:147-148 | `return` | `continue` | PASS |

### 7.3 detect_poll_event 이후 Switch Cases

| FR ID | 파일 | Cases | 설계 | 상태 |
|-------|------|-------|--------|--------|
| FR-C01 | pa_1100_ts.c | FIFO, SOCKET, DATA | FIFO, SOCKET, DATA | PASS |
| FR-C02 | pb_1100_ts.c | FIFO, SOCKET, DATA | FIFO, SOCKET, DATA | PASS |
| FR-C03 | pa_3100_ts.c | FIFO, SOCKET, DATA | FIFO, SOCKET, DATA | PASS |
| FR-C04 | pa_7100_ts.c | SOCKET, DATA | SOCKET, DATA | PASS |
| FR-C05 | pa_8100_ts.c | SOCKET, DATA | SOCKET, DATA | PASS |
| FR-C06 | pb_8200_tr.c | SOCKET | SOCKET | PASS |
| FR-C07 | pa_7800_tr.c | FIFO, SOCKET | FIFO, SOCKET | PASS |

### 7.4 음성 검사 (V-07)

7개 특정 호출자 파일에서 `revents & POLLHUP`을 grep: **모든 7개 파일에서 0 일치**.
7개 특정 호출자 파일에서 `revents & POLLIN`을 grep: **모든 7개 파일에서 0 일치**.

주: pa_3100_ts.c는 줄 704에서 별개의 두 번째 이벤트 루프 함수가 여전히 인라인 POLLHUP/POLLIN을 사용합니다. 이것은 다른 함수로 다른 의미 체계 (모든 POLLHUP에서 반환, SOCKET_EVENT 개념 없음). 올바르게 범위 외.

### 7.5 제외 검증

pa_2100_ts.c는 `detect_poll_event()`를 호출하지 않습니다. 설계에 따라 올바르게 제외됨 (단일 통합 루프를 사용하며 POLLIN-먼저, 그 다음 POLLHUP -- 근본적으로 다른 제어 흐름).

### 7.6 SLog/Log 주 (FR-C03)

pa_3100_ts.c는 둘러싼 코드 (줄 200, 202, 228)에서 `SLog()`를 사용하지만 `detect_poll_event()`는 `Log()`를 사용합니다. 설계가 이것을 "수용된 일관성 개선" (섹션 4.3)으로 인정합니다. 둘 다 동일한 대상으로 로깅; `SLog`는 소스 정보 추가. 정확성에 영향 없음.

**패턴 C 점수: 100%** (7/7 호출자 + 1 제외 검증, 모든 검사 통과)

---

## 8. 패턴 D -- TCP 재시도 로직 감시 (V-09)

설계가 읽기 전용 감시를 명시합니다. 코드 변경이 이루어지지 않았음을 검증했습니다.

| FR ID | 파일 | ConnectRetryCnt 있음 | 재시도 >= 3 | 수정됨? | 상태 |
|-------|------|:-----------------------:|:----------:|:---------:|--------|
| FR-D01 | pa_1100_ts.c:183-184 | Yes | Yes (>= 3) | No | PASS |
| FR-D02 | pb_1100_ts.c:202-203 | Yes | Yes (>= 3) | No | PASS |
| FR-D03 | pa_2100_ts.c | 발견 안 됨 | N/A | No | PASS (주 1) |
| FR-D04 | pa_3100_ts.c | 발견 안 됨 | N/A | No | PASS (주 2) |
| FR-D05 | pb_1200_tr.c:155-156 | Yes | Yes (>= 3) | No | PASS |
| FR-D06 | pa_5020_mp.c | 발견 안 됨 | N/A | No | PASS (주 3) |
| FR-D07 | pa_7000_mp.c | 발견 안 됨 | N/A | No | PASS (주 4) |

**감시 주**:
1. pa_2100_ts.c는 `ConnectRetryCnt`를 사용하지 않음 -- 다른 재시도 메커니즘
2. pa_3100_ts.c는 `ConnectRetryCnt`를 사용하지 않음 -- 다른 아키텍처
3. pa_5020_mp.c는 `ConnectRetryCnt`를 사용하지 않음 -- 더 긴 재연결 블록 사용
4. pa_7000_mp.c는 `ConnectRetryCnt`를 사용하지 않음 -- 관리 프로세스, 다른 패턴

파일 FR-D01, D02, D05는 모두 표준 패턴 사용: 재시도 횟수 3, 오버플로우 시 `Line_Change()`, 타임아웃 `DEVICE_TIME`. 일관됨.

**패턴 D 점수: 100%** (감시 전용, 수정 없음, 관찰 문서화됨)

---

## 9. 패턴 E -- KRX memcmp 체인 감시 (V-10)

설계가 읽기 전용 감시를 명시합니다. 코드 변경이 이루어지지 않았음을 검증했습니다.

| FR ID | 파일 | "0020" 처리 | "0101" 처리 | 수정됨? | 상태 |
|-------|------|:--------------:|:--------------:|:---------:|--------|
| FR-E01 | pa_1100_ts.c:389 | Yes | Err_Msg에서 검증 | No | PASS |
| FR-E02 | pb_1100_ts.c:417 | Yes | Err_Msg에서 검증 | No | PASS |
| FR-E03 | pa_2100_ts.c:570 | Yes (다른 형식) | TBD | No | PASS |
| FR-E04 | pa_3100_ts.c | "0020"에 대해 발견 안 됨 | TBD | No | PASS (주 1) |
| FR-E05 | pb_1200_tr.c | "0020" 발견 안 됨 | TBD | No | PASS (주 2) |

**감시 주**:
1. pa_3100_ts.c는 "0020" TPS 코드 처리 안 함 -- 다른 메시지 흐름 (주식 vs 파생상품)
2. pb_1200_tr.c는 "0020"을 처리 안 함 -- 응답 수신자, 송신자 아님
3. FR-E01 및 FR-E02는 동일한 `memcmp(S_Fmt.Data, "0020", 4)` 패턴 사용 -- 일관됨
4. FR-E03은 `memcmp(R_Pkt->ResponseCode, "0020", sizeof(R_Pkt->ResponseCode))` 사용 -- 일관되지만 다른 구조체 접근 (응답 패킷 vs 송신 형식)

**패턴 E 점수: 100%** (감시 전용, 수정 없음, 관찰 문서화됨)

---

## 10. 검증 기준 요약

| V-ID | 설명 | 방법 | 결과 |
|------|-------------|--------|--------|
| V-01 | 3개의 새 sub/ 파일이 올바른 함수 정의로 존재 | 파일 읽기 + 서명 검사 | PASS |
| V-02 | 헤더 선언이 fep_common.h에 추가됨 | 파일 읽기, 줄 44-51 | PASS |
| V-03 | 4개 패턴 A 호출자에서 AtoIf+sprintf IP 형식화 없음 | Grep `in1 = AtoIf` = 0 일치 | PASS |
| V-04 | `format_ip_addr()`가 4개 파일에서 호출됨 | Grep = 4 호출자 일치 + 1 정의 | PASS |
| V-05 | 5개 패턴 B 호출자에서 인라인 socket/setsockopt/bind 없음 | Grep `setsockopt|SOCK_DGRAM` = 0 일치 | PASS |
| V-06 | `init_udp_socket()`이 5개 파일에서 호출됨 | Grep = 5 호출자 일치 + 1 정의 | PASS |
| V-07 | 7개 패턴 C 호출자에서 POLLHUP for-루프 없음 | Grep `revents & POLLHUP` = 모든 7개에서 0 | PASS |
| V-08 | `detect_poll_event()`가 7개 파일에서 호출됨 | Grep = 7 호출자 일치 + 1 정의 | PASS |
| V-09 | 감시 D: 모든 적용 파일이 재시도 횟수 3 사용 | Grep ConnectRetryCnt: 3개 파일 >= 3 사용 | PASS |
| V-10 | 감시 E: 공통 코드가 일관되게 처리됨 | Grep memcmp "0020": 일관된 패턴 | PASS |

**모든 10개 검증 기준: PASS**

---

## 11. 발견된 차이점

### 11.1 누락된 기능 (설계 O, 구현 X)

**없음.** 모든 설계된 기능이 구현됨.

### 11.2 추가된 기능 (설계 X, 구현 O)

**없음.** 문서화되지 않은 기능이 추가되지 않음.

### 11.3 변경된 기능 (설계 != 구현)

**없음.** 모든 구현이 설계 사양과 정확히 일치.

### 11.4 부수 관찰 (정보, 갭으로 계산 안 함)

| # | 항목 | 설명 | 영향 |
|---|------|-------------|--------|
| 1 | pa_3100_ts.c SLog vs Log | 공유 함수는 `Log()`를 사용하고 둘러싼 코드는 `SLog()`를 사용 | 없음 (설계 인정, 섹션 4.3) |
| 2 | pa_3100_ts.c 두 번째 루프 | 줄 704에 다른 함수의 별개 POLLHUP/POLLIN 루프 | 올바르게 범위 외 |
| 3 | 패턴 D 부분 범위 | 7개 감시 파일 중 3개만 ConnectRetryCnt 패턴 사용 | 정보 (설계가 7개 중 5개에 대해 "TBD" 포함) |
| 4 | 패턴 E 부분 범위 | 5개 감시 파일 중 3개만 "0020" 코드 처리 | 정보 (설계가 5개 중 3개에 대해 "TBD" 포함) |

---

## 12. 일치율 계산

### 12.1 패턴별

| 패턴 | 설계 항목 | 일치 | 추가 | 누락 | 변경 | 일치율 |
|---------|:-----------:|:-------:|:-----:|:-------:|:-------:|:-------:|
| A: IP 형식 | 4 호출자 + 1 함수 + 1 헤더 | 6 | 0 | 0 | 0 | 100% |
| B: UDP 초기화 | 5 호출자 + 1 제외 + 1 함수 + 1 헤더 | 8 | 0 | 0 | 0 | 100% |
| C: Poll 이벤트 | 7 호출자 + 1 제외 + 1 함수 + 1 헤더 | 10 | 0 | 0 | 0 | 100% |
| D: TCP 재시도 | 7 감시 항목 | 7 | 0 | 0 | 0 | 100% |
| E: KRX memcmp | 5 감시 항목 | 5 | 0 | 0 | 0 | 100% |

### 12.2 전체

```
총 설계 항목:   36
일치:              36  (100%)
추가 (문서화 안 됨):  0  (0%)
누락:               0  (0%)
변경:               0  (0%)

전체 일치율: 100%
```

---

## 13. 아키텍처 준수

| 검사 | 예상 | 실제 | 상태 |
|-------|----------|--------|--------|
| sub/의 새 파일 | 3개의 새 .c 파일 in sub/ | ip_format.c, udp_init.c, poll_event.c in sub/ | PASS |
| inc/의 프로토타입 | fep_common.h의 선언 | fep_common.h의 줄 46-51 | PASS |
| src/의 호출자 | 공유 함수만 호출, 인라인 중복 없음 | 모든 호출자 검증 깨끗함 | PASS |
| 라이브러리 자동 빌드 | Make_Lib_P_c.sh가 sub/*.c를 반복 | 3개의 새 파일이 자동 컴파일될 예정 | PASS |
| 제외된 파일 미변경 | pa_7000_mp.c (B), pa_2100_ts.c (C) | 둘 다 수정 안 됨 | PASS |
| 감시 전용 파일 미변경 | 12개 패턴 D+E 파일 | 어느 파일에서도 코드 변경 없음 | PASS |

**아키텍처 준수: 100%**

---

## 14. 규약 준수

| 규약 | 예상 | 실제 | 상태 |
|-----------|----------|--------|--------|
| 파일 명명 | lowercase_underscore.c | ip_format.c, udp_init.c, poll_event.c | PASS |
| 함수 명명 | snake_case | format_ip_addr, init_udp_socket, detect_poll_event | PASS |
| 코멘트 블록 | 대시 헤더가 있는 함수/설명/매개변수 | 모든 3개 파일이 적절한 블록 포함 | PASS |
| 포함 가드 | N/A (소스 파일) | N/A | PASS |
| 헤더 extern | 선언의 `extern` 접두사 | 모든 3개 프로토타입이 `extern` 사용 | PASS |
| 탭 들여쓰기 | 들여쓰기용 탭 (프로젝트 표준) | 모든 파일이 탭 사용 | PASS |

**규약 준수: 100%**

---

## 15. 코드 줄 영향

| 카테고리 | 파일 | 제거된 줄 (추정) | 추가된 줄 |
|----------|:-----:|:-------------------:|:-----------:|
| sub/ 새 파일 | 3 | 0 | 153 (27 + 75 + 51) |
| inc/ 헤더 | 1 | 0 | 8 |
| 패턴 A 호출자 | 4 | ~32 (파일당 8) | 4 (파일당 1) |
| 패턴 B 호출자 | 5 | ~155 (파일당 31) | 15 (파일당 3) |
| 패턴 C 호출자 | 7 | ~168 (파일당 24) | 21 (파일당 3) |
| **합계** | **20** | **~355** | **201** |
| **순 감소** | | | **~154줄** |

---

## 16. 권장 조치

### 16.1 즉각적 조치

필요 없음. 모든 항목이 검증 통과.

### 16.2 문서 업데이트

| 항목 | 설명 | 우선순위 |
|------|-------------|----------|
| 패턴 D 감시 결과 | 설계의 TBD 값 채우기 (pa_2100_ts, pa_3100_ts, pa_5020_mp, pa_7000_mp의 재시도 횟수) | Low |
| 패턴 E 감시 결과 | 설계의 TBD 값 채우기 (pa_3100_ts, pb_1200_tr의 코드) | Low |

### 16.3 향후 고려사항

| 항목 | 설명 | 우선순위 |
|------|-------------|----------|
| pa_3100_ts.c 두 번째 루프 | 줄 704의 별개 poll 이벤트 루프가 향후 반복에서 detect_poll_event()를 사용할 수 있지만 의미 체계가 다름 (모든 POLLHUP에서 반환) | Low |
| 남은 POLLHUP/POLLIN 파일 | 다른 파일 (pa_1200_tr, pa_8200_tr, pb_1800_ts, pb_7100_ts, pb_7200_tr, pb_7800_tr, pb_8100_ts, pb_1200_tr)은 여전히 인라인 poll 루프 포함 -- 향후 추출 후보 | Medium |

---

## 17. 빌드 검증

**상태**: NOT TESTED (서버 환경 필요)

3개의 새 sub/ 파일이 모든 `sub/*.c` 파일을 반복하는 `make/SUB/Make_Lib_P_c.sh`로 자동 컴파일될 예정. Makefile 변경 필요 없음.

---

## 18. 결론

"duplicate-code-extraction"의 구현이 설계 문서와 **100% 일치율** 달성. 모든 3개 공유 함수 (format_ip_addr, init_udp_socket, detect_poll_event)가 설계대로 정확하게 구현됨. 모든 16개 호출자 수정이 설계 사양과 일치. 모든 감시 전용 파일 (패턴 D 및 E)이 올바르게 미수정. fep_common.h의 헤더 선언이 완전하고 정확.

이 기능이 16개 호출 사이트에 걸쳐 약 355줄의 중복 코드를 성공적으로 제거하여 153줄에 걸친 3개의 잘 문서화된 공유 함수로 대체하여 약 154줄의 순 감소 달성.

---

## 버전 이력

| 버전 | 날짜 | 변경 사항 | 작성자 |
|---------|------|---------|--------|
| 1.0 | 2026-02-22 | 초기 분석 -- 100% 일치율 | Claude Code (gap-detector) |
