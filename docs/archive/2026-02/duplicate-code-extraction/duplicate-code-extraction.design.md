# duplicate-code-extraction 설계 문서

> **요약**: 3개의 중복 코드 패턴을 공유 함수로 추출 + 2개 패턴 감사
>
> **프로젝트**: FEP (Front-End Processor for KRX)
> **작성자**: Claude Code
> **날짜**: 2026-02-22
> **상태**: 초안
> **계획 참조**: `docs/01-plan/features/duplicate-code-extraction.plan.md`

---

## 1. 설계 개요

### 1.1 상세 분석 후 수정된 범위

모든 영향을 받는 파일의 상세 읽기는 계획을 개선하는 중요한 변형을 드러냈습니다:

| 패턴 | 계획 | 수정됨 | 이유 |
|---------|------|---------|--------|
| A: IP 포매팅 | 8개 파일 | **4개 파일** | 4개 파일은 TCP2_IP 매크로 사용 (이미 1줄 sprintf, 추출 불가능) |
| B: UDP 소켓 초기화 | 6개 파일 | **6개 파일** | 매개변수화된 함수가 변형 처리 (1D/2D 배열, 옵션) |
| C: Poll 이벤트 감지 | 8개 파일 | **7개 파일** | pa_2100_ts.c는 근본적으로 다른 합계 루프 패턴 |
| D: TCP 재시도 | 7개 파일 감사 | 7개 파일 감사 | 변경 없음 — 감사 전용 확인됨 |
| E: KRX memcmp | 5개 파일 감사 | 5개 파일 감사 | 변경 없음 — 감사 전용 확인됨 |

### 1.2 새로운 파일

| 파일 | 함수 | 헤더 선언 |
|------|----------|-------------------|
| `sub/ip_format.c` | `format_ip_addr()` | `inc/fep_common.h` |
| `sub/udp_init.c` | `init_udp_socket()` | `inc/fep_common.h` |
| `sub/poll_event.c` | `detect_poll_event()` | `inc/fep_common.h` |

세 개 모두 `make/SUB/Make_Lib_P_c.sh`에 의해 자동으로 컴파일됨 (모든 `sub/*.c` 반복).

---

## 2. 패턴 A — IP 주소 포매팅

### 2.1 새로운 함수: `format_ip_addr()`

```c
/* sub/ip_format.c */
#include "fep_fepp.h"

/*------------------------------------------------------------------------
    Function    : format_ip_addr
    Description : Convert 12-byte packed IP (3 chars per octet) to dotted notation
    Parameters  : packed_ip   - 12-byte packed IP string (e.g. "010001002003")
                  dotted_ip   - output buffer for "x.x.x.x" string
                  buf_size    - size of output buffer
------------------------------------------------------------------------*/
void format_ip_addr(const char *packed_ip, char *dotted_ip, int buf_size)
{
    int in1, in2, in3, in4;

    in1 = AtoIf((char *)packed_ip,      3);
    in2 = AtoIf((char *)&packed_ip[3],  3);
    in3 = AtoIf((char *)&packed_ip[6],  3);
    in4 = AtoIf((char *)&packed_ip[9],  3);

    memset(dotted_ip, 0x00, buf_size);
    sprintf(dotted_ip, "%d.%d.%d.%d", in1, in2, in3, in4);
}
```

### 2.2 호출자 변경 (4개 파일)

**FR-A01: `src/PA/pa_7000_us.c` 라인 150-162**

이전:
```c
    int     rt, i, bufflen, oplen;
    int     in1, in2, in3, in4;
    char    Svr_IP[20];

    for (i = 0; i < ACC_NO_CNT; i++)
    {
        in1 = in2 = in3 = in4 = 0;
        memset (Svr_IP, 0x00, sizeof(Svr_IP));
        in1 = AtoIf(ACCNO(D_K,i).ip_addr, 3);
        in2 = AtoIf(&ACCNO(D_K,i).ip_addr[3], 3);
        in3 = AtoIf(&ACCNO(D_K,i).ip_addr[6], 3);
        in4 = AtoIf(&ACCNO(D_K,i).ip_addr[9], 3);

        sprintf(Svr_IP, "%d.%d.%d.%d", in1, in2, in3, in4);
```

이후:
```c
    int     rt, i, bufflen, oplen;
    char    Svr_IP[20];

    for (i = 0; i < ACC_NO_CNT; i++)
    {
        format_ip_addr(ACCNO(D_K,i).ip_addr, Svr_IP, sizeof(Svr_IP));
```

**FR-A02: `src/PA/pa_7010_us.c` 라인 150-162** — 같은 변경. 루프 경계 `i < 4`.

**FR-A03: `src/PA/pa_7030_us.c` 라인 209-220** — 같은 변경. 중첩 루프 컨텍스트.

**FR-A04: `src/PA/pa_9999_us.c` 라인 178-189** — 같은 변경. `ACCNO(D_K,i+Acc_No)` 사용.

이후 (pa_9999_us.c):
```c
        format_ip_addr(ACCNO(D_K,i+Acc_No).ip_addr, Svr_IP, sizeof(Svr_IP));
```

---

## 3. 패턴 B — UDP 소켓 초기화 + 바인드

### 3.1 새로운 함수: `init_udp_socket()`

```c
/* sub/udp_init.c */
#include "fep_fepp.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*------------------------------------------------------------------------
    Function    : init_udp_socket
    Description : Create UDP socket, set options, bind to client address
    Parameters  : svr_addr    - server address struct to fill
                  clnt_addr   - client address struct to fill
                  svr_ip      - server IP string (dotted notation)
                  svr_port    - server port number
                  sndbuf_size - SO_SNDBUF size (0 to skip)
                  rcvbuf_size - SO_RCVBUF size (0 to skip)
                  broadcast   - 1 to enable SO_BROADCAST, 0 to skip
    Return      : socket fd on success, -1 on failure
------------------------------------------------------------------------*/
int init_udp_socket(struct sockaddr_in *svr_addr, struct sockaddr_in *clnt_addr,
                    const char *svr_ip, int svr_port,
                    int sndbuf_size, int rcvbuf_size, int broadcast)
{
    int sockfd, rt;
    int optval, optlen;

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        Log(UDP_FATAL, "Socket Open Error[%d:%s]", SYS_NO, SYS_STR);
        return -1;
    }

    svr_addr->sin_family      = AF_INET;
    svr_addr->sin_addr.s_addr = inet_addr(svr_ip);
    svr_addr->sin_port        = htons(svr_port);

    clnt_addr->sin_family      = AF_INET;
    clnt_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    clnt_addr->sin_port        = htons(0);

    optlen = sizeof(optval);

    if (sndbuf_size > 0)
    {
        optval = sndbuf_size;
        setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (void *)&optval, optlen);
    }

    if (rcvbuf_size > 0)
    {
        optval = rcvbuf_size;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void *)&optval, optlen);
    }

    if (broadcast)
    {
        optval = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (void *)&optval, optlen);
    }

    rt = bind(sockfd, (struct sockaddr *)clnt_addr, sizeof(*clnt_addr));
    if (rt < 0)
    {
        Log(UDP_FATAL, "Socket Bind Error[%d:%s]", SYS_NO, SYS_STR);
        close(sockfd);
        return -1;
    }

    return sockfd;
}
```

### 3.2 호출자 변경 (6개 파일)

**FR-B01: `src/PA/pa_7000_us.c` 라인 164-194**

이전 (루프 내): 소켓/setsockopt/바인드 코드 31줄.

이후:
```c
        Sockfd[i] = init_udp_socket(&Svr_Addr[i], &Clnt_Addr[i],
                                     Svr_IP, SVR_PORT_NO,
                                     1024*64, 0, 1);
        if (Sockfd[i] < 0) return 0;

        Log(UDP_OK, "socket %d created [%s:%s:%d]", i, ACCNO(D_K,i).ip_addr,
                Svr_IP, SVR_PORT_NO);
```

**FR-B02: `src/PA/pa_7010_us.c` 라인 164-194** — FR-B01과 같음. `ACCNO(D_K,i)` 사용.

**FR-B03: `src/PA/pa_7030_us.c` 라인 222-254** — 2D 배열. 포트는 `j`로 변함.

이후:
```c
        Sockfd[i][j] = init_udp_socket(&Svr_Addr[i][j], &Clnt_Addr[i][j],
                                        Svr_IP, port,
                                        1024*64, 1024*64, 1);
        if (Sockfd[i][j] < 0) return 0;

        Log(UDP_OK, "socket %d created [%s]", i, Svr_IP);
```

**FR-B04: `src/PA/pa_9999_us.c` 라인 191-221** — FR-B01과 같음. `ACCNO(D_K,i+Acc_No)` 사용.

**FR-B05: `src/PA/pa_7000_mp.c` 라인 347-386** — 2D 배열, inet_pton 사용 (별도 유지).

주의: pa_7000_mp.c는 `inet_addr()` 대신 `inet_pton()`을 사용. 공유 함수는 `inet_addr()`을 사용. **결정**: pa_7000_mp.c를 인라인으로 유지 (inet_pton + 조건부 #ifdef 포트 논리 사용). **5개 호출자**로 감소.

**FR-B06: `src/PA/pa_7500_us.c` 라인 166-191** — SO_BROADCAST 없음.

이후:
```c
        Sockfd[i] = init_udp_socket(&Svr_Addr[i], &Clnt_Addr[i],
                                     &Svr_IP[20*i], SVR_PORT_NO,
                                     1024*64, 1024*64, 0);
        if (Sockfd[i] < 0) return 0;

        Log(UDP_OK, "socket %d created [%s:%d]", i, &Svr_IP[20*i], SVR_PORT_NO);
```

### 3.3 수정된 패턴 B 파일 목록

| ID | 파일 | sndbuf | rcvbuf | broadcast | 주의 |
|----|------|:------:|:------:|:---------:|-------|
| FR-B01 | pa_7000_us.c | 64K | 0 | 1 | 표준 |
| FR-B02 | pa_7010_us.c | 64K | 0 | 1 | 표준 |
| FR-B03 | pa_7030_us.c | 64K | 64K | 1 | 2D 배열, 변수 포트 |
| FR-B04 | pa_9999_us.c | 64K | 0 | 1 | 표준 |
| ~~FR-B05~~ | ~~pa_7000_mp.c~~ | — | — | — | **제외됨**: inet_pton + #ifdef 포트 |
| FR-B06 | pa_7500_us.c | 64K | 64K | 0 | 브로드캐스트 없음 |

---

## 4. 패턴 C — Poll 이벤트 감지

### 4.1 새로운 함수: `detect_poll_event()`

```c
/* sub/poll_event.c */
#include "fep_fepp.h"
#include <poll.h>

/*------------------------------------------------------------------------
    Function    : detect_poll_event
    Description : Check poll array for POLLHUP and POLLIN events
    Parameters  : poll_arr         - poll file descriptor array
                  poll_cnt         - number of entries in poll_arr
                  socket_event_idx - index of socket fd in poll_arr
    Return      : >= 0: index of fd with POLLIN event
                  -1: socket disconnected (POLLHUP on socket_event_idx)
                  -2: no POLLIN event found
------------------------------------------------------------------------*/
int detect_poll_event(struct pollfd *poll_arr, int poll_cnt, int socket_event_idx)
{
    int i;

    /* Check for hangup events */
    for (i = 0; i < poll_cnt; i++)
    {
        if (poll_arr[i].revents & POLLHUP)
        {
            if (i == socket_event_idx)
            {
                Log(TCP_ERROR, "socket disconnected[%#06x]",
                    poll_arr[i].revents);
                return -1;
            }

            Log(SYS_ERROR, "poll hangup[%d,%d]", i, poll_cnt);
        }
    }

    /* Find first POLLIN event */
    for (i = 0; i < poll_cnt; i++)
    {
        if (poll_arr[i].revents & POLLIN)
        {
            poll_arr[i].revents = 0;
            return i;
        }
    }

    return -2;  /* no event */
}
```

### 4.2 호출자 변경 (7개 파일)

**대표 이전/이후 (pa_1100_ts.c 라인 246-269)**:

이전:
```c
        for (i = 0; i < PollCnt; i ++)
        {
            if (Poll[i].revents & POLLHUP)
            {
                if (i == SOCKET_EVENT)
                {
                    Log (TCP_ERROR, "socket disconnected[%#06x]",
                        Poll[i].revents);
                    return;
                }

                Log (SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
                continue;
            }
        }

        for (i = 0; i < PollCnt; i ++)
        {
            if (Poll[i].revents & POLLIN)
            {
                Poll[i].revents = 0;
                break;
            }
        }
```

이후:
```c
        i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);
        if (i == -1) return;        /* socket disconnected */
        if (i == -2) continue;      /* no event */
```

### 4.3 파일 목록

| ID | 파일 | 라인 (이전) | Log 변형 | 이후 스위치 경우 |
|----|------|:--------------:|:-----------:|-------------------|
| FR-C01 | pa_1100_ts.c | 246-269 | Log | FIFO, SOCKET, DATA |
| FR-C02 | pb_1100_ts.c | 267-290 | Log | FIFO, SOCKET, DATA |
| FR-C03 | pa_3100_ts.c | 212-235 | SLog | FIFO, SOCKET, DATA |
| FR-C04 | pa_7100_ts.c | 146-168 | Log | SOCKET, DATA |
| FR-C05 | pa_8100_ts.c | 198-220 | Log | SOCKET, DATA |
| FR-C06 | pb_8200_tr.c | 176-198 | Log | SOCKET |
| FR-C07 | pa_7800_tr.c | 146-169 | Log | FIFO, SOCKET |

**제외됨**: pa_2100_ts.c — 단일 합계 루프 사용 (POLLIN 먼저, 그 후 POLLHUP). 다른 제어 흐름.

**pa_3100_ts.c (FR-C03)의 주의**: 현재 `Log()` 대신 `SLog()`를 사용. 공유 함수는 `Log()`를 사용. 이것은 작은 변경 — 두 로그 모두 같은 목적지로, `SLog`는 소스 정보를 추가. 일관성 개선으로 수용됨.

---

## 5. 패턴 D — TCP 재시도 논리 (감사 전용)

### 5.1 표준 패턴

```c
if (OpenFlag == OFF)
{
    if (LogOnFlag == OFF)
        Device_Open(TR_LOON);

    if (LogOnFlag == OFF)
    {
        ConnectRetryCnt++;
        if (ConnectRetryCnt >= 3)
        {
            Line_Change();
            ConnectRetryCnt = 0;
        }
        PollCnt = 1;
        TimeOut = DEVICE_TIME;
    }
}
```

### 5.2 감사 체크리스트

| ID | 파일 | 재시도 횟수 | 타임아웃 | 주의 |
|----|------|:-----------:|---------|-------|
| FR-D01 | pa_1100_ts.c:176 | 3 | DEVICE_TIME | 표준 |
| FR-D02 | pb_1100_ts.c:195 | 3 | DEVICE_TIME | 표준 |
| FR-D03 | pa_2100_ts.c | TBD | TBD | 검증 |
| FR-D04 | pa_3100_ts.c | TBD | TBD | 검증 |
| FR-D05 | pb_1200_tr.c | TBD | TBD | 검증 |
| FR-D06 | pa_5020_mp.c | TBD | TBD | 검증 (더 긴 블록, ~143줄) |
| FR-D07 | pa_7000_mp.c | TBD | TBD | 검증 |

**조치**: Do 단계 중 읽기 전용 감사. 발견 사항을 문서화하되 코드 변경 없음.

---

## 6. 패턴 E — KRX memcmp 체인 (감사 전용)

### 6.1 공통 코드

| 코드 | 의미 | 예상 처리 |
|------|---------|-------------------|
| `"0020"` | TPS 한계 초과됨 | sleep(1), continue |
| `"0101"` | 시장이 열려있지 않음 | sleep(3), retry |
| `"0004"` | 세션 오류 | reconnect |

### 6.2 감사 체크리스트

| ID | 파일 | 처리된 코드 | 주의 |
|----|------|---------------|-------|
| FR-E01 | pa_1100_ts.c:410 | 0020, 0101, others | 검증 |
| FR-E02 | pb_1100_ts.c:438 | 0020, 0101, others | 검증 |
| FR-E03 | pa_2100_ts.c | TBD | 검증 |
| FR-E04 | pa_3100_ts.c | TBD | 검증 |
| FR-E05 | pb_1200_tr.c | TBD | 검증 |

**조치**: Do 단계 중 읽기 전용 감사. 공통 코드의 일관된 처리 검증.

---

## 7. 헤더 변경

**파일**: `inc/fep_common.h`

`#endif` 전에 추가:

```c
/*------------------------------------------------------------------------
    Shared utility function prototypes
------------------------------------------------------------------------*/
extern void format_ip_addr(const char *packed_ip, char *dotted_ip, int buf_size);
extern int  init_udp_socket(struct sockaddr_in *svr_addr, struct sockaddr_in *clnt_addr,
                            const char *svr_ip, int svr_port,
                            int sndbuf_size, int rcvbuf_size, int broadcast);
extern int  detect_poll_event(struct pollfd *poll_arr, int poll_cnt,
                              int socket_event_idx);
```

---

## 8. 구현 순서

| Batch | 단계 | 파일 | 설명 |
|:-----:|-------|:-----:|-------------|
| 1 | 공유 함수 생성 | 3개 신규 | `sub/ip_format.c`, `sub/udp_init.c`, `sub/poll_event.c` |
| 2 | 헤더 선언 | 1개 편집 | `inc/fep_common.h` |
| 3 | 패턴 A 호출자 | 4개 편집 | AtoIf 인라인 제거, `format_ip_addr()` 호출 |
| 4 | 패턴 B 호출자 | 5개 편집 | 소켓 초기화 인라인 제거, `init_udp_socket()` 호출 |
| 5 | 패턴 C 호출자 | 7개 편집 | Poll 루프 제거, `detect_poll_event()` 호출 |
| 6 | 감사 D + E | 12개 읽기 | 일관성 검증, 발견 사항 문서화 |

---

## 9. 검증 기준

| ID | 체크 | 방법 |
|----|-------|--------|
| V-01 | 3개의 새로운 sub/ 파일이 존재하고 컴파일됨 | 함수 정의에 대해 Grep |
| V-02 | 헤더 선언이 fep_common.h에 추가됨 | 읽기 |
| V-03 | 패턴 A: 4개의 호출자 파일에서 AtoIf+sprintf IP 포매팅 없음 | Grep |
| V-04 | 패턴 A: 4개 파일에서 `format_ip_addr()` 호출됨 | Grep |
| V-05 | 패턴 B: 5개의 호출자 파일에서 인라인 소켓/setsockopt/바인드 없음 | Grep |
| V-06 | 패턴 B: 5개 파일에서 `init_udp_socket()` 호출됨 | Grep |
| V-07 | 패턴 C: 7개의 호출자 파일에서 POLLHUP for-루프 없음 | Grep |
| V-08 | 패턴 C: 7개 파일에서 `detect_poll_event()` 호출됨 | Grep |
| V-09 | 감사 D: 모든 7개 파일이 재시도 횟수 3 사용 | 읽기 |
| V-10 | 감사 E: 공통 코드가 일관되게 처리됨 | 읽기 |

---

## 버전 이력

| 버전 | 날짜 | 변경 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-22 | 초기 초안 | Claude Code |
