# 설계: sub-audit

> sub/ 모듈 (libfepP.a 공유 라이브러리) 포괄적 코드 품질 감사

## 참조

- 계획: `docs/01-plan/features/sub-audit.plan.md`
- 범위: `st01/sub/` 내 50개 C 소스 파일
- 총 FR 수: 17

---

## 배치 1: CRITICAL 수정 (3개 파일)

### FR-01: tcpip_accept.c — NULL 포인터 역참조 + inet_ntop (CRITICAL)

**FR-04 (전처리기) 및 FR-14 (inet_ntoa)도 포함**

**파일**: `st01/sub/tcpip_accept.c`

**문제**: 라인 53은 `hp`가 NULL인지 확인하지 않고 `hp->h_name`을 역참조. `gethostbyaddr()`은 DNS 실패 시 NULL 반환으로 SIGSEGV 발생. 또한 라인 25는 전처리기 버그, 라인 47은 사용 중단된 `inet_ntoa()` 사용.

**수정 전** (라인 22-55):
```c
	char				*hostname, *hostip;
	int					rt;
#if defined __hpux || _AIX || __linux
	socklen_t			addrlen;
#else
	size_t				addrlen;
#endif
	struct sockaddr_in	address;
	struct hostent		*hp;
    ...
	hp = gethostbyaddr ((char *)&address.sin_addr,
		sizeof (struct in_addr), address.sin_family);
	hostip = (char *)inet_ntoa (address.sin_addr);
	*p_port = ntohs (address.sin_port);
	sprintf (p_ip, "%s", hostip);
#ifdef sun
	Log (TCP_OK, "request from (%s:%u)", hostip, *p_port);
#else
	hostname = hp->h_name;
	Log (TCP_OK, "request from %s (%s:%u)", hostname, hostip, *p_port);
#endif
```

**수정 후**:
```c
	char				hostip[INET_ADDRSTRLEN];
	int					rt;
#if defined __hpux || defined _AIX || defined __linux
	socklen_t			addrlen;
#else
	size_t				addrlen;
#endif
	struct sockaddr_in	address;
	struct hostent		*hp;
    ...
	hp = gethostbyaddr ((char *)&address.sin_addr,
		sizeof (struct in_addr), address.sin_family);
	inet_ntop (AF_INET, &address.sin_addr, hostip, sizeof (hostip));
	*p_port = ntohs (address.sin_port);
	sprintf (p_ip, "%s", hostip);
	if (hp != NULL)
		Log (TCP_OK, "request from %s (%s:%u)", hp->h_name, hostip, *p_port);
	else
		Log (TCP_OK, "request from (%s:%u)", hostip, *p_port);
```

**변경사항**:
1. `char *hostname, *hostip` → `char hostip[INET_ADDRSTRLEN]` (inet_ntop용 스택 버퍼)
2. `#if defined __hpux || _AIX || __linux` → `#if defined __hpux || defined _AIX || defined __linux`
3. `inet_ntoa()` → `inet_ntop(AF_INET, ...)`
4. `#ifdef sun` 블록 제거 — 모든 플랫폼용 통일 NULL 안전 분기
5. 사용되지 않는 `hostname` 변수 제거

---

### FR-02: setsigfatal.c — 신호-안전하지 않은 strlen() (CRITICAL)

**파일**: `st01/sub/setsigfatal.c`

**문제**: 라인 65는 신호 핸들러 내에서 `strlen(name)` 호출. `strlen()`은 POSIX async-signal-safe로 보장되지 않음.

**수정 전** (라인 65):
```c
		write(STDERR_FILENO, name, strlen(name));
```

**수정 후**:
```c
		{
			const char *p = name;
			size_t nlen = 0;
			while (p[nlen] != '\0') nlen++;
			write(STDERR_FILENO, name, nlen);
		}
```

**원리**: 인라인 루프는 async-signal-safe (libc 호출 없음). 신호명 문자열은 짧음(최대 7자), 루프 오버헤드 최소.

---

### FR-03: queue.c — K&R 함수 정의 (CRITICAL)

**파일**: `st01/sub/queue.c`

**문제**: `ReceiveQueue()` (라인 27)과 `MakeQueue()` (라인 115)는 K&R 구식 매개변수 선언 사용.

**수정 전** (라인 27-31):
```c
int ReceiveQueue(QID, owner, data)
int QID;
long owner;
char *data;
{
```

**수정 후**:
```c
int ReceiveQueue(int QID, long owner, char *data)
{
```

**수정 전** (라인 115-117):
```c
int MakeQueue(KEY)
    size_t KEY;
{
```

**수정 후**:
```c
int MakeQueue(size_t KEY)
{
```

---

### FR-13d: queue.c — C++ 주석 (MEDIUM, 번들됨)

**수정 전** (라인 125):
```c
    // ¼±ÅÃ¿©ºÎ¿¡ µû¶ó¼­ ³ÖµçÁö »©µçÁö
```

**수정 후**:
```c
    /* select and insert/remove */
```

---

### FR-15: queue.c — 매직 숫자 문서화 (LOW, 번들됨)

**수정 전** (라인 3,6):
```c
#define     QUEUE_MAX_BYTES 10485760
...
#define     MAXSIZE         40960
```

**수정 후**:
```c
#define     QUEUE_MAX_BYTES 10485760    /* 10 MB max queue size */
...
#define     MAXSIZE         40960       /* 40 KB max message size */
```

---

### FR-16: queue.c — 사용되지 않는 변수 (LOW, 번들됨)

**수정 전** (라인 118):
```c
    char buff[MAXSIZE];
```

**수정 후**: 라인 삭제.

---

## 배치 2: HIGH 전처리기 + 문자열 안전성 (5개 파일)

### FR-05: check_exist.c — Missing `defined` 키워드 (HIGH)

**파일**: `st01/sub/check_exist.c`

**수정 전** (라인 14):
```c
#elif defined sun || __linux
```

**수정 후**:
```c
#elif defined sun || defined __linux
```

**수정 전** (라인 36):
```c
#if defined sun || __linux
```

**수정 후**:
```c
#if defined sun || defined __linux
```

**참고**: 라인 46의 `#elif defined sun` 확인 — 단일 피연산자이므로 OK.

---

### FR-06: check_proc.c — Missing `defined` 키워드 (HIGH)

**파일**: `st01/sub/check_proc.c`

**수정 전** (라인 23):
```c
#if defined sun || __linux
```

**수정 후**:
```c
#if defined sun || defined __linux
```

---

### FR-07: stat_save.c — 안전하지 않은 strcat() (HIGH)

**파일**: `st01/sub/stat_save.c`

**문제**: 라인 119와 123의 `strcat(buf, tmp)`. buf는 1024바이트이고 현재 최대 내용은 ~150바이트이지만, 경계 작업 사용이 안전.

**수정 전** (라인 111-123):
```c
		for (i = 0; i < 2; i ++)
		{
			if (PROC(D_K,P_K).l.t2.l[i])
				sprintf (tmp, "%d%d%d", TCP2_LSTAT(D_K,P_K,i),
				TCP2_PSTAT(D_K,P_K,i), TCP2_NSTAT(D_K,P_K,i));
			else
				sprintf (tmp, "XXX");

			strcat (buf, tmp);
		}

		sprintf (tmp, "%-12.12s", " ");
		strcat (buf, tmp);
```

**수정 후**:
```c
		for (i = 0; i < 2; i ++)
		{
			if (PROC(D_K,P_K).l.t2.l[i])
				sprintf (tmp, "%d%d%d", TCP2_LSTAT(D_K,P_K,i),
				TCP2_PSTAT(D_K,P_K,i), TCP2_NSTAT(D_K,P_K,i));
			else
				sprintf (tmp, "XXX");

			strncat (buf, tmp, sizeof (buf) - strlen (buf) - 1);
		}

		sprintf (tmp, "%-12.12s", " ");
		strncat (buf, tmp, sizeof (buf) - strlen (buf) - 1);
```

---

### FR-08: config_db.c — 안전하지 않은 strcat() (HIGH)

**파일**: `st01/sub/config_db.c`

**문제**: tbuf[20]에 4개 `strcat()` 호출. 현재 안전(strncpy는 14 + 5 접미사 = 19 < 20으로 제한)하지만 취약.

**수정 전** (라인 538, 549, 584, 597):
```c
strcat (tbuf, "_exit");
...
strcat (tbuf, "_ctrl");
```

**수정 후** (모든 4개 발생, replace_all):
```c
strncat (tbuf, "_exit", sizeof (tbuf) - strlen (tbuf) - 1);
...
strncat (tbuf, "_ctrl", sizeof (tbuf) - strlen (tbuf) - 1);
```

---

### FR-09: file_rw.c + shm_rw.c — strlen+memcpy 안티패턴 (HIGH)

**문제**: `memcpy(&tmp[strlen(tmp)], ...)`는 이진 버퍼에서 `strlen()`을 사용하여 추가 오프셋 찾음. file_rw.c 쓰기 경로는 NUL→공간 대체이지만, shm_rw.c의 SHM 경로는 공유 메모리에서 읽음. 여기서 NUL 바이트는 정상 존재 가능. 수정: 정수 변수로 오프셋 추적.

#### FR-09a: file_rw.c `F_R_Proc()` (라인 132-195)

**수정 전**:
```c
	int				rt, cnt, i, len, pt;
	char			buf[FILE_BUF_LEN], tmp[FILE_BUF_LEN];
    ...
	memset (tmp, 0, sizeof (tmp));

	for (i = 0; i < rt; i += p_size)
	{
        ...
		memcpy (&tmp[strlen(tmp)], buff_rw.Seq, sizeof (BUFF_RW_HEAD));
		memcpy (&tmp[strlen(tmp)], buf+i+sizeof(FILE_RW_HEAD),
			p_size - sizeof (FILE_RW_HEAD));
	}

	memcpy (p_buf, tmp, strlen (tmp));
```

**수정 후**:
```c
	int				rt, cnt, i, len, pt, tmp_off;
	char			buf[FILE_BUF_LEN], tmp[FILE_BUF_LEN];
    ...
	memset (tmp, 0, sizeof (tmp));
	tmp_off = 0;

	for (i = 0; i < rt; i += p_size)
	{
        ...
		memcpy (&tmp[tmp_off], buff_rw.Seq, sizeof (BUFF_RW_HEAD));
		tmp_off += sizeof (BUFF_RW_HEAD);
		memcpy (&tmp[tmp_off], buf+i+sizeof(FILE_RW_HEAD),
			p_size - sizeof (FILE_RW_HEAD));
		tmp_off += p_size - sizeof (FILE_RW_HEAD);
	}

	memcpy (p_buf, tmp, tmp_off);
```

#### FR-09b: file_rw.c `F_W2_Proc()` (라인 659-660)

**수정 전**:
```c
	memcpy (&buf[strlen(buf)], file_rw.Seq, sizeof (FILE_RW_HEAD));
	memcpy (&buf[strlen(buf)], p_buf+sizeof(BUFF_RW_HEAD),
		p_size - sizeof (BUFF_RW_HEAD));
```

**수정 후** — 참고: F_W2_Proc는 이미 루프 오프셋 변수로 `k` 사용 (라인 310, 337-338 패턴 참조). 동일 패턴이 여기 적용. `buf_off` 변수 추가 필요:

선언에 추가:
```c
	int		buf_off;
```

두 memcpy 라인 앞에 설정: `buf_off = strlen(buf);` 그 후:
```c
	buf_off = strlen (buf);
	memcpy (&buf[buf_off], file_rw.Seq, sizeof (FILE_RW_HEAD));
	buf_off += sizeof (FILE_RW_HEAD);
	memcpy (&buf[buf_off], p_buf+sizeof(BUFF_RW_HEAD),
		p_size - sizeof (BUFF_RW_HEAD));
```

**참고**: F_W2_Proc는 파일에 쓰고 NUL→공간 대체가 직후에 따름(라인 666-673), 이중 strlen은 여기서 안전. 하지만 명시적 오프셋 사용이 더 깔끔하고 일관성 있음.

#### FR-09c: shm_rw.c `DSHM_R()` (라인 29-140)

**수정 전**:
```c
	int				i, fd, rt, rec_size, r_cnt, data_cnt;
    ...
	memset (tmp, 0, sizeof (tmp));
    ...
		memcpy (&tmp[strlen(tmp)], &brw, sizeof (BUFF_RW_HEAD));
		memcpy (&tmp[strlen(tmp)], buf+sizeof(FILE_RW_HEAD),
			rec_size - sizeof (FILE_RW_HEAD));
    ...
	memcpy (p_buf, tmp, strlen (tmp));
```

**수정 후**:
```c
	int				i, fd, rt, rec_size, r_cnt, data_cnt, tmp_off;
    ...
	memset (tmp, 0, sizeof (tmp));
	tmp_off = 0;
    ...
		memcpy (&tmp[tmp_off], &brw, sizeof (BUFF_RW_HEAD));
		tmp_off += sizeof (BUFF_RW_HEAD);
		memcpy (&tmp[tmp_off], buf+sizeof(FILE_RW_HEAD),
			rec_size - sizeof (FILE_RW_HEAD));
		tmp_off += rec_size - sizeof (FILE_RW_HEAD);
    ...
	memcpy (p_buf, tmp, tmp_off);
```

---

## 배치 3: MEDIUM C++ 주석 (5개 파일)

### FR-10: select_recv.c — 8개 C++ 주석

**파일**: `st01/sub/select_recv.c`

| 라인 | 수정 전 | 수정 후 |
|------|--------|--------|
| 476 | `// 누적 버퍼` | `/* cumulative buffer */` |
| 477 | `// 현재까지 쌓인 길이` | `/* accumulated length */` |
| 499 | `// 버퍼에 수신 데이터 누적` | `/* accumulate received data */` |
| 510 | `// 1건 패킷 구분자(예: 0xFF 0x0D 0x0A) 탐색` | `/* search for packet delimiter (0xFF 0x0D 0x0A) */` |
| 520 | `ipos++;  // 패킷 길이 = ipos` | `ipos++;  /* packet length = ipos */` |
| 522 | `// 나머지 데이터 앞으로 당김` | `/* shift remaining data forward */` |
| 526 | `return ipos;  // 1건 패킷 길이 리턴` | `return ipos;  /* return 1 packet length */` |
| 531 | `return 0;  // 아직 패킷 완료 아님` | `return 0;  /* packet not yet complete */` |

### FR-11: file_rw.c — 10개 C++ 주석

**파일**: `st01/sub/file_rw.c`

모두 라인 305, 308, 313, 335, 631, 657, 960, 963, 968, 990의 `// 2025EDIT` 마커.

**모두 대체** (replace_all=true):
- `// 2025EDIT` → `/* 2025EDIT */`

### FR-12: log_proc.c — 6개 C++ 주석

**파일**: `st01/sub/log_proc.c`

모두 라인 169, 174, 225, 230, 423, 595의 `// 2025EDIT` 마커.

**모두 대체** (replace_all=true):
- `// 2025EDIT` → `/* 2025EDIT */`

### FR-13: key_search.c + make_daemon.c — C++ 주석

**FR-13a**: key_search.c 라인 30:
- `// 채권_KTS` → `/* NOTE_MK_KTS */`

**FR-13b**: key_search.c 라인 44:
- `// 금융파생, 실제 파생은 순차적으로 오니 필요없다.` → `/* DEV_MK_FIF */`

**FR-13c**: make_daemon.c 라인 51:
- `// 0:정상, -1:오류` → `/* 0:OK, -1:error */`

---

## 배치 4: LOW 정리 (2개 파일)

### FR-17: key_search.c — 복사-붙여넣기 주석 오류

**수정 전** (라인 62):
```c
}	/* End of LtoU ()	*/
```

**수정 후**:
```c
}	/* End of Key_Search ()	*/
```

**수정 전** (라인 210):
```c
	End of Program (ltou.c)
```

**수정 후**:
```c
	End of Program (key_search.c)
```

---

## 구현 순서

| 순서 | 배치 | FR | 파일 | 예상 변경 |
|------|------|-----|------|----------|
| 1 | 배치 1 | FR-01,02,03,04,13d,14,15,16 | tcpip_accept.c, setsigfatal.c, queue.c | 3개 파일, ~30줄 |
| 2 | 배치 2 | FR-05,06,07,08,09 | check_exist.c, check_proc.c, stat_save.c, config_db.c, file_rw.c, shm_rw.c | 6개 파일, ~40줄 |
| 3 | 배치 3 | FR-10,11,12,13abc | select_recv.c, file_rw.c, log_proc.c, key_search.c, make_daemon.c | 5개 파일, ~28개 주석 |
| 4 | 배치 4 | FR-17 | key_search.c | 1개 파일, 2줄 |

## 검증 체크리스트

모든 배치 후:

| 확인 | 명령/방법 |
|------|----------|
| 0개 `//` 주석 | `grep '//' st01/sub/*.c` |
| 0개 `defined X \|\| Y` (defined 없음) | `grep '#if.*defined.*\|\|.*[^d]' st01/sub/*.c` |
| 0개 `strcat()` | `grep 'strcat' st01/sub/*.c` |
| 0개 K&R 정의 | 시각: `func(A,B)\ntype A;\ntype B;` 패턴 없음 |
| 0개 `inet_ntoa` | `grep 'inet_ntoa' st01/sub/*.c` |
| 0개 `strlen(tmp)]` in file_rw/shm_rw | `grep 'strlen.*tmp\|strlen.*buf.*]' st01/sub/{file_rw,shm_rw}.c` |
| 신호 핸들러 안전 | 시각: End_Routine에 libc 호출 없음 |

## 위험 완화

| FR | 위험 | 완화 |
|----|------|------|
| FR-01 | 모니터링에서 보이는 로그 형식 변경 | IP만 표시하는 대체는 기존 `#ifdef sun` 동작 일치 |
| FR-09 | 이진 프로토콜 오프셋 오계산 | `tmp_off` 산술 검증: 레코드당 `sizeof(BUFF_RW_HEAD) + (p_size - sizeof(FILE_RW_HEAD))` = NUL 없는 데이터 strlen 결과 일치 |
| FR-03 | 호출자 시그니처 불일치 | `ReceiveQueue(int, long, char*)`와 `MakeQueue(size_t)` — 호출자는 이미 이 타입 전달 |
