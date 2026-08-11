# select-send-optimize Design Document

> **Summary**: Select_Send()에서 불필요한 select() syscall 제거 — SO_SNDTIMEO 200ms로 대체. 3개 파일, 5 FR.
>
> **Plan**: `docs/01-plan/features/select-send-optimize.plan.md`
> **Author**: Claude
> **Date**: 2026-02-28
> **Status**: Draft

---

## 1. Implementation Overview

### 1.1 변경 전략

1. **Step 1**: `tcpip_connect.c` — Connect(), Connect2()에 SO_SNDTIMEO 추가
2. **Step 2**: `tcpip_accept.c` — Accept()에 SO_SNDTIMEO 추가
3. **Step 3**: `select_send.c` — select() 제거, 직접 Sendn() 호출

### 1.2 헤더 의존성

| 심볼 | 헤더 | 포함 경로 |
|------|------|-----------|
| `struct timeval` | `<sys/time.h>` | fep_sub.h:86 → 이미 포함 |
| `SOL_SOCKET`, `SO_SNDTIMEO` | `<sys/socket.h>` | fep_sub.h:104 → 이미 포함 |
| `setsockopt()` | `<sys/socket.h>` | fep_sub.h:104 → 이미 포함 |

**추가 include 불필요.**

---

## 2. File-by-File Changes

### 2.1 Step 1: `sub/tcpip_connect.c`

#### 2.1.1 Connect() — FR-01

**Before** (lines 34-41):
```c
	if (rt == 0)
	{
		int flag = 1;
		if (setsockopt (p_sfd, IPPROTO_TCP, TCP_NODELAY,
			(char *)&flag, sizeof (flag)) < 0)
			Log (TCP_WARN, "Connect:TCP_NODELAY fail {%d:%s}",
				SYS_NO, SYS_STR);
	}
```

**After**:
```c
	if (rt == 0)
	{
		int flag = 1;
		struct timeval snd_timeout;

		if (setsockopt (p_sfd, IPPROTO_TCP, TCP_NODELAY,
			(char *)&flag, sizeof (flag)) < 0)
			Log (TCP_WARN, "Connect:TCP_NODELAY fail {%d:%s}",
				SYS_NO, SYS_STR);

		snd_timeout.tv_sec = 0;
		snd_timeout.tv_usec = 200000;
		if (setsockopt (p_sfd, SOL_SOCKET, SO_SNDTIMEO,
			(char *)&snd_timeout, sizeof (snd_timeout)) < 0)
			Log (TCP_WARN, "Connect:SO_SNDTIMEO fail {%d:%s}",
				SYS_NO, SYS_STR);
	}
```

**변경 요약**:
- `struct timeval snd_timeout` 선언 추가 (블록 상단, C89 준수)
- TCP_NODELAY 블록 직후에 SO_SNDTIMEO 200ms 설정
- 실패 시 `Log(TCP_WARN)` — TCP_NODELAY와 동일 패턴
- `(char *)&snd_timeout` 캐스팅 — HP-UX/AIX 호환

#### 2.1.2 Connect2() — FR-02

**Before** (lines 80-86):
```c
    {
        int flag = 1;
        if (setsockopt (p_sfd, IPPROTO_TCP, TCP_NODELAY,
            (char *)&flag, sizeof (flag)) < 0)
            Log (TCP_WARN, "Connect2:TCP_NODELAY fail {%d:%s}",
                SYS_NO, SYS_STR);
    }
```

**After**:
```c
    {
        int flag = 1;
        struct timeval snd_timeout;

        if (setsockopt (p_sfd, IPPROTO_TCP, TCP_NODELAY,
            (char *)&flag, sizeof (flag)) < 0)
            Log (TCP_WARN, "Connect2:TCP_NODELAY fail {%d:%s}",
                SYS_NO, SYS_STR);

        snd_timeout.tv_sec = 0;
        snd_timeout.tv_usec = 200000;
        if (setsockopt (p_sfd, SOL_SOCKET, SO_SNDTIMEO,
            (char *)&snd_timeout, sizeof (snd_timeout)) < 0)
            Log (TCP_WARN, "Connect2:SO_SNDTIMEO fail {%d:%s}",
                SYS_NO, SYS_STR);
    }
```

**변경 요약**: Connect()와 동일 패턴. 로그 접두사만 "Connect2:" 차이.

**주의**: Connect2()는 space 들여쓰기 사용 (Connect()는 tab). 기존 스타일 유지.

---

### 2.2 Step 2: `sub/tcpip_accept.c`

#### 2.2.1 Accept() — FR-03

**Before** (lines 55-61):
```c
	{
		int flag = 1;
		if (setsockopt (rt, IPPROTO_TCP, TCP_NODELAY,
			(char *)&flag, sizeof (flag)) < 0)
			Log (TCP_WARN, "Accept:TCP_NODELAY fail {%d:%s}",
				SYS_NO, SYS_STR);
	}
```

**After**:
```c
	{
		int flag = 1;
		struct timeval snd_timeout;

		if (setsockopt (rt, IPPROTO_TCP, TCP_NODELAY,
			(char *)&flag, sizeof (flag)) < 0)
			Log (TCP_WARN, "Accept:TCP_NODELAY fail {%d:%s}",
				SYS_NO, SYS_STR);

		snd_timeout.tv_sec = 0;
		snd_timeout.tv_usec = 200000;
		if (setsockopt (rt, SOL_SOCKET, SO_SNDTIMEO,
			(char *)&snd_timeout, sizeof (snd_timeout)) < 0)
			Log (TCP_WARN, "Accept:SO_SNDTIMEO fail {%d:%s}",
				SYS_NO, SYS_STR);
	}
```

**변경 요약**:
- 대상 fd는 `rt` (accepted fd) — `p_sfd`(listen fd)가 아님. TCP_NODELAY와 동일 대상
- 로그 접두사 "Accept:"
- tab 들여쓰기 (tcpip_accept.c 기존 스타일)

---

### 2.3 Step 3: `sub/select_send.c`

#### 2.3.1 Select_Send() — FR-04, FR-05

**Before** (전체 함수, lines 20-55):
```c
int		Select_Send (int p_sfd, char *p_send, int p_length)
{
	int				rt;
	fd_set			write_set;
	struct timeval	timeout;

	FD_ZERO (&write_set);
	FD_SET (p_sfd, &write_set);
	timeout.tv_sec = 0;
	timeout.tv_usec = 200000;

	rt = select (p_sfd+1, NULL, &write_set, NULL, &timeout);
	if(rt < 0)
	{
		Log (TCP_ERROR, "Select_Send:select failure[%d] {%d:%s}",
			rt, SYS_NO, SYS_STR);
		return (NOTOK);
	}

	if (!FD_ISSET (p_sfd, &write_set))
	{
		Log (TCP_ERROR, "Select_Send:FD_ISSET {%d:%s}", SYS_NO, SYS_STR);
		return (NOTOK);
	}

	rt = Sendn (p_sfd, p_send, p_length);
	if (rt <= 0)
	{
		Log (TCP_ERROR, "Select_Send:send failure[%d] {%d:%s}",
			rt, SYS_NO, SYS_STR);
		return (NOTOK);
	}

	return (OK);
}
```

**After**:
```c
int		Select_Send (int p_sfd, char *p_send, int p_length)
{
	int		rt;

	rt = Sendn (p_sfd, p_send, p_length);
	if (rt <= 0)
	{
		Log (TCP_ERROR, "Select_Send:send failure[%d] {%d:%s}",
			rt, SYS_NO, SYS_STR);
		return (NOTOK);
	}

	return (OK);
}
```

**변경 요약**:
- `fd_set write_set`, `struct timeval timeout` 변수 제거
- `FD_ZERO`, `FD_SET`, `select()`, `FD_ISSET` 호출 제거 (13줄 제거)
- `Sendn()` 호출 및 에러 처리 유지 (동일)
- 반환값 의미 동일: `OK`(0) 성공, `NOTOK`(-1) 실패
- 함수 시그니처 변경 없음: `int Select_Send(int, char*, int)`

**에러 경로 비교**:

| 시나리오 | Before | After |
|---------|--------|-------|
| 정상 전송 | select OK → send OK → OK | send OK → OK |
| 송신 버퍼 풀 | select 200ms 대기 → OK/timeout | send 200ms 대기(SO_SNDTIMEO) → OK/ETIMEDOUT |
| 연결 끊김 | select 에러 또는 send 에러 → NOTOK | send 에러(EPIPE) → NOTOK |
| fd 무효 | select EBADF → NOTOK | send EBADF → NOTOK |

---

## 3. Verification Items

| ID | Item | Target | Method |
|----|------|--------|--------|
| V-01 | Connect()에 SO_SNDTIMEO 200ms 설정 | tcpip_connect.c | 코드 검사 |
| V-02 | Connect2()에 SO_SNDTIMEO 200ms 설정 | tcpip_connect.c | 코드 검사 |
| V-03 | Accept()에 SO_SNDTIMEO 200ms 설정 (rt 대상) | tcpip_accept.c | 코드 검사 |
| V-04 | SO_SNDTIMEO 실패 시 Log(TCP_WARN)만, return 안 함 | 3곳 모두 | 코드 검사 |
| V-05 | Select_Send에서 select()/FD_ISSET/fd_set 제거 | select_send.c | 코드 검사 |
| V-06 | Select_Send에서 Sendn() 호출 및 에러 처리 유지 | select_send.c | 코드 검사 |
| V-07 | Select_Send 시그니처 변경 없음 | select_send.c + fep_sub.h | 코드 검사 |
| V-08 | struct timeval 선언 위치 — 블록 상단 (C89) | tcpip_connect.c, tcpip_accept.c | 코드 검사 |
| V-09 | (char *) 캐스팅 적용 | 3곳 SO_SNDTIMEO | 코드 검사 |
| V-10 | 들여쓰기 스타일 보존 (Connect=tab, Connect2=space, Accept=tab) | 각 파일 | 코드 검사 |
| V-11 | 빌드 성공 | mk.sh sub | 서버 환경 |

---

## 4. Implementation Order

```
Step 1: sub/tcpip_connect.c
  └ FR-01: Connect() SO_SNDTIMEO
  └ FR-02: Connect2() SO_SNDTIMEO

Step 2: sub/tcpip_accept.c
  └ FR-03: Accept() SO_SNDTIMEO

Step 3: sub/select_send.c
  └ FR-04: select() 제거
  └ FR-05: Sendn() 에러 처리 유지
```

**순서 근거**: SO_SNDTIMEO를 먼저 설정한 후 select()를 제거해야, 이미 연결된 소켓도 다음 재연결 시 타임아웃이 적용됨. 순서가 바뀌면 select() 제거 후 SO_SNDTIMEO 없는 소켓에서 무한 blocking 위험.

---

## 5. Design Decisions

| Decision | Selected | Rationale |
|----------|----------|-----------|
| 타임아웃 값 | 200ms (tv_usec=200000) | 기존 Select_Send select() timeout과 동일 |
| 실패 처리 | Log(TCP_WARN), 연결 유지 | TCP_NODELAY 패턴과 일관 — SO_SNDTIMEO 실패해도 기존 blocking 동작 유지 |
| 함수명 유지 | Select_Send 그대로 | 22개 호출자 변경 방지. 내부 구현만 변경 |
| include 추가 | 없음 | sys/time.h, sys/socket.h 이미 fep_sub.h에 포함 |
| Sendn() 수정 | 없음 | Sendn()의 while(nleft>0) 루프가 SO_SNDTIMEO와 정상 동작 |

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2026-02-28 | Initial design — 3 files, 11 V items | Claude |
