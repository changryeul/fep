# duplicate-code-extraction 계획 문서

> **요약**: 5개 카테고리의 중복 코드 패턴을 sub/의 공유 함수로 추출
>
> **프로젝트**: FEP (KRX용 프론트엔드 프로세서)
> **작성자**: Claude Code
> **날짜**: 2026-02-22
> **상태**: Draft

---

## 1. 개요

### 1.1 목적

반복되는 패턴을 `sub/`의 공유 유틸리티 함수로 추출하여 src/ 모듈 간 코드 중복을 줄입니다 (`libfepP.a`로 컴파일됨). 식별된 5개 카테고리, 총 34개 파일 인스턴스에 걸침. 추출이 유지보수 부담을 줄이고 버그 수정이 모든 호출자에게 전파되도록 보장합니다.

### 1.2 배경

코드베이스 스캔이 PA (거래), PB (채권), 유틸리티 모듈에 걸친 병렬 개발에서 진화한 5개 중복 코드 패턴을 식별했습니다. 추출이 유지보수 부담을 줄이고 모든 호출자에게 버그 수정 전파를 보장합니다.

### 1.3 위험 평가

이것은 **프로덕션 KRX 거래 시스템**입니다. 주요 제약:
- 자동화된 테스트 스위트 없음 — 검증은 코드 검토 + 수동 테스트만
- 전역 변수가 광범위하게 사용됨 — 함수가 매개변수를 받거나 공유 상태 접근해야 함
- 각 `.c` 파일이 `-D` 정의를 통해 여러 바이너리로 컴파일됨 — 공유 함수가 모든 변형에서 작동해야 함
- `sub/` 함수가 `libfepP.a`로 링크됨 — 모든 프로세스에 버그가 영향을 미침

**접근**: 안전한 자체 포함된 추출을 우선화. 프로세스별 로직에 밀접하게 결합된 패턴은 연기.

---

## 2. 범위

### 2.1 범위 내 — 3개 추출 가능 패턴 (우선순위 1)

| # | 패턴 | 파일 | 새 함수 | 위치 |
|:-:|---------|:-----:|--------------|----------|
| A | IP 주소 형식화 | 8 | `format_ip_addr()` | `sub/ip_format.c` (new) |
| B | UDP 소켓 초기화 + 바인드 | 6 | `init_udp_socket()` | `sub/udp_init.c` (new) |
| C | Poll 이벤트 감지 | 8 | `detect_poll_event()` | `sub/poll_event.c` (new) |

### 2.2 범위 내 — 2개 표준화 패턴 (우선순위 2)

| # | 패턴 | 파일 | 조치 | 비고 |
|:-:|---------|:-----:|--------|-------|
| D | TCP 재시도 로직 | 7 | 인라인 표준화 | 전역에 결합되어 추출하기에 너무 밀접 |
| E | KRX memcmp 체인 | 5 | 코드 테이블 + 도우미 | 각 파일이 다른 코드 처리 |

### 2.3 범위 외

- 프로세스 로직이나 제어 흐름 변경
- 빌드 시스템 수정 (새 `.c` 파일이 `Make_Lib_P_c.sh`에 의해 자동 포함됨)
- 기존 범위 이상의 에러 처리 추가

---

## 3. 요구사항

### 3.1 패턴 A — IP 주소 형식화 (8개 파일)

**현재 패턴** (각 파일에서 반복):
```c
int in1, in2, in3, in4;
char Svr_IP[20];

in1 = in2 = in3 = in4 = 0;
memset(Svr_IP, 0x00, sizeof(Svr_IP));
in1 = AtoIf(ACCNO(D_K,i).ip_addr, 3);
in2 = AtoIf(&ACCNO(D_K,i).ip_addr[3], 3);
in3 = AtoIf(&ACCNO(D_K,i).ip_addr[6], 3);
in4 = AtoIf(&ACCNO(D_K,i).ip_addr[9], 3);
sprintf(Svr_IP, "%d.%d.%d.%d", in1, in2, in3, in4);
```

**제안된 함수**:
```c
/* sub/ip_format.c */
void format_ip_addr(const char *packed_ip, char *dotted_ip, int dotted_ip_size);
```

12바이트 패킹된 IP (옥텟당 3글자)를 받아 `"x.x.x.x"`을 출력 버퍼에 씁니다.

**업데이트할 파일**:

| ID | 파일 | 줄 |
|----|------|-------|
| FR-A01 | `src/PA/pa_7000_us.c` | 155-162 |
| FR-A02 | `src/PA/pa_7010_us.c` | 155-162 |
| FR-A03 | `src/PA/pa_7030_us.c` | ~155-162 |
| FR-A04 | `src/PA/pa_9999_us.c` | ~184-189 |
| FR-A05 | `src/PA/pa_7000_mp.c` | ~similar |
| FR-A06 | `src/PA/pa_1100_ts.c` | ~314-315 |
| FR-A07 | `src/PB/pb_1100_ts.c` | ~338-339 |
| FR-A08 | `src/PA/pa_2100_ts.c` | ~203-204 |

### 3.2 패턴 B — UDP 소켓 초기화 + 바인드 (6개 파일)

**현재 패턴** (소켓당 반복):
```c
Sockfd[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
// error check
Svr_Addr[i].sin_family      = AF_INET;
Svr_Addr[i].sin_addr.s_addr = inet_addr(Svr_IP);
Svr_Addr[i].sin_port        = htons(SVR_PORT_NO);
Clnt_Addr[i].sin_family      = AF_INET;
Clnt_Addr[i].sin_addr.s_addr = htonl(INADDR_ANY);
Clnt_Addr[i].sin_port        = htons(0);
bufflen = 1024 * 64;
setsockopt(Sockfd[i], SOL_SOCKET, SO_SNDBUF, ...);
bufflen = 1;
setsockopt(Sockfd[i], SOL_SOCKET, SO_BROADCAST, ...);
bind(Sockfd[i], ...);
```

**제안된 함수**:
```c
/* sub/udp_init.c */
int init_udp_socket(int *sockfd, struct sockaddr_in *svr_addr,
                    struct sockaddr_in *clnt_addr,
                    const char *svr_ip, int svr_port,
                    int sndbuf_size, int broadcast);
```

성공 시 sockfd 반환, 실패 시 -1. 로그 메시지는 함수 내부에 남음.

**업데이트할 파일**:

| ID | 파일 |
|----|------|
| FR-B01 | `src/PA/pa_7000_us.c` |
| FR-B02 | `src/PA/pa_7010_us.c` |
| FR-B03 | `src/PA/pa_7030_us.c` |
| FR-B04 | `src/PA/pa_9999_us.c` |
| FR-B05 | `src/PA/pa_7000_mp.c` |
| FR-B06 | `src/PA/pa_7500_us.c` |

### 3.3 패턴 C — Poll 이벤트 감지 (8개 파일)

**현재 패턴** (두 동일한 for-루프):
```c
for (i = 0; i < PollCnt; i++) {
    if (Poll[i].revents & POLLHUP) {
        if (i == SOCKET_EVENT) {
            Log(TCP_ERROR, "socket disconnected[%#06x]", Poll[i].revents);
            return;
        }
        Log(SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
        continue;
    }
}
for (i = 0; i < PollCnt; i++) {
    if (Poll[i].revents & POLLIN) {
        Poll[i].revents = 0;
        break;
    }
}
```

**제안된 함수**:
```c
/* sub/poll_event.c */
int detect_poll_event(struct pollfd *poll_arr, int poll_cnt, int socket_event_idx);
```

반환: POLLIN 시 이벤트 인덱스 (0..poll_cnt-1), socket_event_idx에서 POLLHUP 시 -1 (호출자가 반환해야 함), 다른 POLLHUP 시 -2 (로깅됨, 계속).

**주**: POLLHUP이 `SOCKET_EVENT`에서 호출자가 `return`을 하도록 함 — 이것은 함수 내부가 아닌 호출자가 반환값을 검사하여 처리해야 합니다. 함수가 감지하고 로깅, 호출자가 제어 흐름 결정.

**업데이트할 파일**:

| ID | 파일 |
|----|------|
| FR-C01 | `src/PA/pa_1100_ts.c` |
| FR-C02 | `src/PB/pb_1100_ts.c` |
| FR-C03 | `src/PA/pa_2100_ts.c` |
| FR-C04 | `src/PA/pa_3100_ts.c` |
| FR-C05 | `src/PA/pa_7100_ts.c` |
| FR-C06 | `src/PA/pa_8100_ts.c` |
| FR-C07 | `src/PB/pb_8200_tr.c` |
| FR-C08 | `src/PA/pa_7800_tr.c` |

### 3.4 패턴 D — TCP 재시도 로직 (7개 파일) — 표준화만

**결정**: 공유 함수로 추출하지 않음. 패턴이 프로세스별 전역 (`OpenFlag`, `LogOnFlag`, `ConnectRetryCnt`, `PollCnt`, `TimeOut`)에 의존하고 프로세스 특화 함수 (`Device_Open`, `Line_Change`)를 호출합니다. 추출이 5개+ 포인터 매개변수를 필요로 하여 이점 무효화.

**조치**: 모든 7개 파일이 일관된 재시도 횟수 (3)와 일관된 타임아웃 상수 (`DEVICE_TIME`)를 사용하는지 검증. 패턴을 코멘트 블록으로 문서화. 불일치 발견 시에만 코드 변경.

**감시할 파일**:

| ID | 파일 |
|----|------|
| FR-D01 | `src/PA/pa_1100_ts.c` |
| FR-D02 | `src/PB/pb_1100_ts.c` |
| FR-D03 | `src/PA/pa_2100_ts.c` |
| FR-D04 | `src/PA/pa_3100_ts.c` |
| FR-D05 | `src/PB/pb_1200_tr.c` |
| FR-D06 | `src/PA/pa_5020_mp.c` |
| FR-D07 | `src/PA/pa_7000_mp.c` |

### 3.5 패턴 E — KRX memcmp 체인 (5개 파일) — 평가만

**결정**: 추출하지 않음. 각 파일이 다른 KRX 응답 코드를 다른 조치 (수면 기간, 재시도 로직, 에러 처리)로 처리합니다. memcmp 비교 자체는 1-2줄 — 둘러싼 로직이 다양함.

**조치**: 감시 전용. `"0020"` (TPS 제한) 및 `"0101"` (시장 미개장) 같은 공통 코드의 일관된 처리 검증. 코드 변경 없음.

**감시할 파일**:

| ID | 파일 |
|----|------|
| FR-E01 | `src/PA/pa_1100_ts.c` |
| FR-E02 | `src/PB/pb_1100_ts.c` |
| FR-E03 | `src/PA/pa_2100_ts.c` |
| FR-E04 | `src/PA/pa_3100_ts.c` |
| FR-E05 | `src/PB/pb_1200_tr.c` |

---

## 4. 구현 계획

### 단계 1: 새 공유 함수 (sub/의 3개 새 파일)

| 순서 | 파일 | 함수 | 의존 |
|:-----:|------|----------|-------------|
| 1 | `sub/ip_format.c` | `format_ip_addr()` | `atoif.c` (AtoIf) |
| 2 | `sub/udp_init.c` | `init_udp_socket()` | `<sys/socket.h>`, `log_proc.c` |
| 3 | `sub/poll_event.c` | `detect_poll_event()` | `<poll.h>`, `log_proc.c` |

헤더 선언은 `inc/fep_common.h` 또는 새 `inc/fep_dedup.h`.

### 단계 2: 호출자 업데이트 (패턴 A, B, C에 걸친 22개 파일 변경)

인라인 코드를 함수 호출로 대체.

### 단계 3: 패턴 D 및 E 감시 (12개 파일, 읽기 전용)

일관성 검증. 발견사항 문서화. 불일치 발견 시에만 코드 변경.

---

## 5. 성공 기준

- [ ] 3개의 새 공유 함수가 컴파일되고 `libfepP.a`로 링크됨
- [ ] 모든 22개 호출자 파일이 공유 함수를 사용하도록 업데이트됨
- [ ] 0 함수 변경 — 이전과 이후의 동일한 동작
- [ ] 패턴 D 및 E가 발견사항 문서화됨
- [ ] 갭 분석 >= 90%

---

## 6. 위험 및 완화

| 위험 | 영향 | 가능성 | 완화 |
|------|--------|------------|------------|
| 공유 함수 버그가 모든 호출자 영향 | High | Low | 기존 인라인 코드의 정확한 복제 |
| 복사 간 약간의 동작 변형 | Medium | Medium | 모든 복사 읽기, 가장 흔한 패턴 사용 |
| 빌드 시스템이 새 .c 파일 선택 안 함 | Medium | Low | `Make_Lib_P_c.sh`가 모든 `sub/*.c` 자동 컴파일 |
| 헤더 포함 이슈 | Medium | Low | 기존 `fep_common.h`에 추가 |
| sub/ 함수에서 전역 변수 접근 | High | Medium | 매개변수로 전달, 절대 전역 접근 안 함 |

---

## 7. 예상 영향

| 메트릭 | 값 |
|--------|-------|
| 새 파일 | 3 (sub/) + 1 헤더 |
| 수정된 파일 | ~22 호출자 |
| 제거된 줄 (대략) | ~400-500 (함수 호출로 대체) |
| 추가된 줄 (대략) | ~100 (3개 새 함수 + 헤더) |
| 순 감소 | ~300-400 줄 |

---

## 8. 다음 단계

1. [ ] 각 파일별 정확한 이전/이후가 있는 설계 문서 작성
2. [ ] 단계 1 구현 (새 공유 함수)
3. [ ] 단계 2 구현 (호출자 업데이트)
4. [ ] 단계 3 감시
5. [ ] 갭 분석

---

## 버전 이력

| 버전 | 날짜 | 변경 사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-22 | 초기 초안 | Claude Code |
