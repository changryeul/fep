# tcp-nodelay Design Document

> **Summary**: TCP 소켓 TCP_NODELAY 설정 — Connect/Accept 내부에서 Nagle 비활성화
>
> **Plan Reference**: `docs/01-plan/features/tcp-nodelay.plan.md`
> **Date**: 2026-02-27
> **Status**: Draft

---

## 1. Design Overview

### 1.1 접근 방식

공유 라이브러리의 `Connect()`, `Connect2()`, `Accept()` 함수 내부에서 연결 성공 직후 `setsockopt(IPPROTO_TCP, TCP_NODELAY, 1)`을 호출. 모든 TCP 연결이 자동으로 Nagle 비활성화.

- **호출자 변경 0건** — 15개 Socket() 호출 위치 불변
- **실패 시 안전** — setsockopt 실패해도 Log 경고만, 연결 계속 진행
- **POSIX 표준** — HP-UX, SunOS, AIX, Linux 모두 지원

### 1.2 변경 파일 목록

| # | File | Change Type | Lines |
|---|------|-------------|-------|
| 1 | `inc/fep_sub.h` | 수정 | 1줄 추가 |
| 2 | `sub/tcpip_connect.c` | 수정 | ~10줄 추가 |
| 3 | `sub/tcpip_accept.c` | 수정 | ~5줄 추가 |

---

## 2. Implementation Order

```
Step 1: inc/fep_sub.h — <netinet/tcp.h> include 추가
Step 2: sub/tcpip_connect.c — Connect(), Connect2()에 TCP_NODELAY
Step 3: sub/tcpip_accept.c — Accept()에 TCP_NODELAY
```

---

## 3. Detailed Design per FR

### FR-01: Connect() 성공 후 TCP_NODELAY (tcpip_connect.c)

**현재 코드** (`sub/tcpip_connect.c:32-34`):
```c
	rt = connect (p_sfd, (struct sockaddr *)&address, sizeof (struct sockaddr));

	return (rt);
```

**변경**: `connect()` 성공(rt == 0) 시 TCP_NODELAY 설정 후 반환.

**변경할 코드**:
```c
	rt = connect (p_sfd, (struct sockaddr *)&address, sizeof (struct sockaddr));

	if (rt == 0)
	{
		int flag = 1;
		if (setsockopt (p_sfd, IPPROTO_TCP, TCP_NODELAY,
			(char *)&flag, sizeof (flag)) < 0)
			Log (TCP_WARN, "Connect:TCP_NODELAY fail {%d:%s}",
				SYS_NO, SYS_STR);
	}

	return (rt);
```

**핵심 사항**:
- `rt == 0`인 경우만 — 실패 시에는 설정 불필요
- `(char *)&flag` 캐스팅 — HP-UX/AIX 호환 (void * 아닌 char *)
- `TCP_WARN` 로그 레벨 — 연결은 성공했으므로 경고만
- setsockopt 실패해도 `return (rt)` 그대로 — 연결 중단하지 않음

### FR-02: Accept() 성공 후 TCP_NODELAY (tcpip_accept.c)

**현재 코드** (`sub/tcpip_accept.c:40-43`):
```c
	rt = accept (p_sfd, (struct sockaddr *)&address, &addrlen);

	if (rt < 0)
		return (rt);
```

**변경**: `accept()` 성공(rt >= 0) 후, 반환 전에 TCP_NODELAY 설정.

**삽입 위치**: line 53 (`Log` 호출) 이후, line 55 (`return (rt)`) 이전.

**삽입할 코드**:
```c
	{
		int flag = 1;
		if (setsockopt (rt, IPPROTO_TCP, TCP_NODELAY,
			(char *)&flag, sizeof (flag)) < 0)
			Log (TCP_WARN, "Accept:TCP_NODELAY fail {%d:%s}",
				SYS_NO, SYS_STR);
	}

	return (rt);
```

**핵심 사항**:
- `rt`가 accept된 소켓 fd — `p_sfd`(listening)가 아닌 `rt`(connected)에 설정
- 블록 `{}`로 감싸 — `flag` 변수 scope 제한 (기존 변수와 충돌 방지)
- `rt < 0` 검사(line 42-43) 이후이므로 성공 경로에서만 실행

### FR-03: <netinet/tcp.h> include (fep_sub.h)

**현재 코드** (`inc/fep_sub.h:103-107`):
```c
/* TCP/IP headers	*/
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
```

**변경**: `<netinet/in.h>` 뒤에 `<netinet/tcp.h>` 추가.

**변경할 코드**:
```c
/* TCP/IP headers	*/
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netinet/tcp.h>
#include	<arpa/inet.h>
#include	<netdb.h>
```

**핵심 사항**:
- `<netinet/tcp.h>`는 `TCP_NODELAY`, `IPPROTO_TCP` 정의 제공
- `<netinet/in.h>` 뒤에 배치 — in.h가 tcp.h의 전제 조건
- POSIX 표준 헤더 — 모든 타겟 플랫폼 지원

### FR-04: Connect2() TCP_NODELAY (tcpip_connect.c)

**현재 코드** (`sub/tcpip_connect.c:61-71`):
```c
    rt = connect (p_sfd, (struct sockaddr *)&address, sizeof (struct sockaddr));
    if (rt != 0)
    {
        alarm (0);

        return (-1);
    }

    alarm (0);

    return (rt);
```

**변경**: `alarm(0)` 이후, `return (rt)` 이전에 TCP_NODELAY 설정.

**변경할 코드**:
```c
    rt = connect (p_sfd, (struct sockaddr *)&address, sizeof (struct sockaddr));
    if (rt != 0)
    {
        alarm (0);

        return (-1);
    }

    alarm (0);

    {
        int flag = 1;
        if (setsockopt (p_sfd, IPPROTO_TCP, TCP_NODELAY,
            (char *)&flag, sizeof (flag)) < 0)
            Log (TCP_WARN, "Connect2:TCP_NODELAY fail {%d:%s}",
                SYS_NO, SYS_STR);
    }

    return (rt);
```

**핵심 사항**:
- `alarm(0)` 이후 — timeout 해제 후에 설정
- `rt != 0` 분기 이후이므로 성공 경로에서만 실행
- 블록 `{}`로 감싸 — flag 변수 scope 제한

### FR-05: setsockopt 실패 시 Log 경고만

**설계**: 모든 3곳(Connect, Connect2, Accept)에서 동일 패턴:

```c
if (setsockopt (..., TCP_NODELAY, ...) < 0)
    Log (TCP_WARN, "...:TCP_NODELAY fail {%d:%s}", SYS_NO, SYS_STR);
```

- `TCP_WARN` 레벨 사용 — 연결 자체는 성공했으므로 ERROR가 아닌 WARN
- `SYS_NO` = `errno`, `SYS_STR` = `strerror(errno)` — 기존 매크로 사용
- 실패해도 함수 반환값 변경 없음 — 호출자에게 성공 반환

---

## 4. Exact Code Changes

### 4.1 `inc/fep_sub.h` — 1줄 추가

**위치**: line 105 (`#include <netinet/in.h>`) 뒤에 삽입.

```
Before (lines 104-106):

#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>

After:

#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netinet/tcp.h>
#include	<arpa/inet.h>
```

### 4.2 `sub/tcpip_connect.c` — Connect() 변경

**위치**: lines 32-34

```
Before (lines 32-34):

	rt = connect (p_sfd, (struct sockaddr *)&address, sizeof (struct sockaddr));

	return (rt);

After:

	rt = connect (p_sfd, (struct sockaddr *)&address, sizeof (struct sockaddr));

	if (rt == 0)
	{
		int flag = 1;
		if (setsockopt (p_sfd, IPPROTO_TCP, TCP_NODELAY,
			(char *)&flag, sizeof (flag)) < 0)
			Log (TCP_WARN, "Connect:TCP_NODELAY fail {%d:%s}",
				SYS_NO, SYS_STR);
	}

	return (rt);
```

### 4.3 `sub/tcpip_connect.c` — Connect2() 변경

**위치**: lines 69-71

```
Before (lines 69-71):

    alarm (0);

    return (rt);

After:

    alarm (0);

    {
        int flag = 1;
        if (setsockopt (p_sfd, IPPROTO_TCP, TCP_NODELAY,
            (char *)&flag, sizeof (flag)) < 0)
            Log (TCP_WARN, "Connect2:TCP_NODELAY fail {%d:%s}",
                SYS_NO, SYS_STR);
    }

    return (rt);
```

### 4.4 `sub/tcpip_accept.c` — Accept() 변경

**위치**: lines 53-55

```
Before (lines 53-55):

	else
		Log (TCP_OK, "request from (%s:%u)", hostip, *p_port);

	return (rt);

After:

	else
		Log (TCP_OK, "request from (%s:%u)", hostip, *p_port);

	{
		int flag = 1;
		if (setsockopt (rt, IPPROTO_TCP, TCP_NODELAY,
			(char *)&flag, sizeof (flag)) < 0)
			Log (TCP_WARN, "Accept:TCP_NODELAY fail {%d:%s}",
				SYS_NO, SYS_STR);
	}

	return (rt);
```

---

## 5. Verification Checklist

| # | 검증 항목 | 확인 방법 |
|---|-----------|----------|
| V-01 | `<netinet/tcp.h>` include가 `fep_sub.h`에 추가됨 | grep 확인 |
| V-02 | `Connect()`에서 connect 성공 시 TCP_NODELAY 설정 | 코드 확인 |
| V-03 | `Connect2()`에서 connect 성공 시 TCP_NODELAY 설정 | 코드 확인 |
| V-04 | `Accept()`에서 accept 성공 시 TCP_NODELAY 설정 | 코드 확인 |
| V-05 | Accept의 setsockopt 대상이 `rt`(accepted fd)임 (`p_sfd` 아님) | 코드 확인 |
| V-06 | setsockopt 실패 시 Log 경고만 (연결 중단하지 않음) | 에러 경로 확인 |
| V-07 | `(char *)&flag` 캐스팅 사용 (HP-UX/AIX 호환) | 코드 확인 |
| V-08 | 기존 호출자 15곳 변경 없음 | grep Socket/Connect/Accept 확인 |
| V-09 | 빌드 성공 (`mk.sh sub`) | 빌드 실행 |

---

## 6. Edge Cases

### 6.1 이미 닫힌 소켓에 setsockopt

불가능. Connect()는 connect() 성공 직후, Accept()는 accept() 성공 직후에 호출. 소켓이 열려있는 상태.

### 6.2 UDP 소켓에 TCP_NODELAY

불가능. `Socket()`은 `SOCK_STREAM`(TCP)만 생성. UDP는 별도 경로(`init_udp_socket()`)로 생성되며 Connect/Accept를 사용하지 않음.

### 6.3 Connect() 실패 후 setsockopt

`rt == 0` 검사로 방지. connect 실패 시 setsockopt 호출하지 않음.

### 6.4 Connect2() timeout 발생

`rt != 0` 분기에서 이미 `return (-1)`. TCP_NODELAY 코드에 도달하지 않음.

---

## 7. Build & Test

```sh
mk.sh sub          # libfepP.a 재빌드 (tcpip_connect.o, tcpip_accept.o 재컴파일)
mk.sh src          # 전체 바이너리 재링크
```

검증:
```sh
grep -n "TCP_NODELAY" st01/inc/fep_sub.h st01/sub/tcpip_connect.c st01/sub/tcpip_accept.c
grep -n "netinet/tcp" st01/inc/fep_sub.h
```

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-27 | Initial design | Claude |
