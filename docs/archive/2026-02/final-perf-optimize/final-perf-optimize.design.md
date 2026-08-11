# final-perf-optimize Design Document

> **Summary**: §4.9 잔여 항목 — 시세 수신 경로 최적화(pb_7100_ur.c) + sleep→poll 대체(pb_1100_ts.c). 순위6은 이미 완료.
>
> **Plan**: `docs/01-plan/features/final-perf-optimize.plan.md`
> **Author**: Claude
> **Date**: 2026-02-28
> **Status**: Draft

---

## 1. Implementation Overview

### 1.1 변경 전략

1. **Step 1**: `src/PB/pb_7100_ur.c` — 시세 수신 경로 5개 FR
2. **Step 2**: `src/PB/pb_1100_ts.c` — sleep(1) → poll + FIFO 처리

---

## 2. File-by-File Changes

### 2.1 Step 1: `src/PB/pb_7100_ur.c`

#### 2.1.1 FR-01 + FR-02 + FR-03: memset/strlen/Log 제거 (lines 211-232)

**Before**:
```c
		/* UDP 데이터 수신 */
		memset(rbuf, 0, sizeof (rbuf));
		rt = Recv_Data(rbuf);
		if (rt == 0)
		{
			Log (USR_OK, "poll timeout");
			continue;
		}
		else if (rt < 0)
		{
			Log (UDP_ERROR, "receive fail {%d:%s}", SYS_NO, SYS_STR);
			close (Sockfd);

			/* UDP 소켓 재생성 */
			rt = Socket_Connect();
			if (rt == NOTOK)
				return;

			continue;
		}

		rbuf_len = strlen(rbuf);
		Log (USR_OK, "RD[%s] <%d>", rbuf, rbuf_len);
```

**After**:
```c
		/* UDP 데이터 수신 */
		rt = Recv_Data(rbuf);
		if (rt == 0)
		{
			Log (USR_OK, "poll timeout");
			continue;
		}
		else if (rt < 0)
		{
			Log (UDP_ERROR, "receive fail {%d:%s}", SYS_NO, SYS_STR);
			close (Sockfd);

			/* UDP 소켓 재생성 */
			rt = Socket_Connect();
			if (rt == NOTOK)
				return;

			continue;
		}

		rbuf[rt] = '\0';
		rbuf_len = rt;
```

**변경 요약**:
- `memset(rbuf, 0, sizeof(rbuf))` 제거 — 2048바이트 초기화 불필요, recvfrom이 덮어씀
- `rbuf[rt] = '\0'` 추가 — Log의 %s 포맷용 null 종료
- `strlen(rbuf)` → `rt` — recvfrom 리턴값이 정확한 수신 길이
- `Log(USR_OK, "RD[%s] <%d>", ...)` 제거 — TCP SD 로그(line 257)가 동일 데이터 + INT_SEQ 기록

#### 2.1.2 FR-04: TrCode sprintf → memcpy (lines 235-236)

**Before**:
```c
		/* TR코드 추출 (앞 2바이트) */
		memset (TrCode, 0, sizeof(TrCode));
		sprintf (TrCode, "%-2.2s", rbuf);
```

**After**:
```c
		/* TR코드 추출 (앞 2바이트) */
		memcpy (TrCode, rbuf, 2);
		TrCode[2] = '\0';
```

**변경 요약**:
- `memset(TrCode, 0, sizeof(TrCode))` 제거 — 8바이트 초기화 불필요
- `sprintf(TrCode, "%-2.2s", rbuf)` → `memcpy` + null 종료 — printf 계열 오버헤드 제거
- TrCode[8]에 2바이트 + '\0'만 필요. memcmp(TrCode, "A3", 2) 비교에 충분

#### 2.1.3 FR-05: LK 응답 경로 최적화 (lines 261-266)

**Before**:
```c
		else
		{
			/* 기타 데이터 → LK 응답 (heartbeat 역할) */
			memset(rbuf, 0, sizeof (rbuf));
			memcpy(rbuf, "0011", 4);			/* 길이 헤더 (4바이트) */
			memcpy(&rbuf[4], "LK000000000", 11);	/* LK 응답 코드 */

			rbuf_len = strlen (rbuf);
```

**After**:
```c
		else
		{
			/* 기타 데이터 → LK 응답 (heartbeat 역할) */
			memcpy(rbuf, "0011", 4);			/* 길이 헤더 (4바이트) */
			memcpy(&rbuf[4], "LK000000000", 11);	/* LK 응답 코드 */
			rbuf[15] = '\0';

			rbuf_len = 15;
```

**변경 요약**:
- `memset(rbuf, 0, sizeof(rbuf))` 제거 — 2048바이트 초기화 불필요
- `rbuf[15] = '\0'` 추가 — Log %s용 null 종료
- `strlen(rbuf)` → 상수 `15` — "0011"(4) + "LK000000000"(11) = 15바이트 확정

---

### 2.2 Step 2: `src/PB/pb_1100_ts.c`

#### 2.2.1 FR-06: sleep(1) → poll + FIFO 처리 (lines 155-159)

**Before**:
```c
            if (LogOnFlag == ON && TCP2_NET_STA(S_K) == END)
			{
                Device_Close ();
			}

			sleep(1);
            continue;
```

**After**:
```c
            if (LogOnFlag == ON && TCP2_NET_STA(S_K) == END)
			{
                Device_Close ();
			}

			rt = poll (Poll, 1, 1000);
			if (rt > 0 && (Poll[FIFO_EVENT].revents & POLLIN))
				Fifo_Event_Rtn ();
            continue;
```

**변경 요약**:
- `sleep(1)` → `poll(Poll, 1, 1000)` — 1초 대기, FIFO(Poll[0]) 감시
- nfds=1: FIFO만 감시 (Poll[0].fd=START_FD, line 329에서 초기화)
- FIFO 이벤트 도착 시 `Fifo_Event_Rtn()` 호출 — 장외에도 운영자 명령 즉시 처리
- 이벤트 없으면 1초 후 타임아웃 → continue → 상태 재확인
- `rt` 변수는 함수 스코프에서 이미 선언됨 (line 138)
- 들여쓰기: space (기존 스타일 유지)

---

## 3. Verification Items

| ID | Item | Target | Method |
|----|------|--------|--------|
| V-01 | memset(rbuf) 제거 (수신 전) | pb_7100_ur.c | 코드 검사 |
| V-02 | rbuf[rt] = '\0' 추가 (수신 후) | pb_7100_ur.c | 코드 검사 |
| V-03 | rbuf_len = rt (strlen 대체) | pb_7100_ur.c | 코드 검사 |
| V-04 | USR_OK "RD" 로그 제거 | pb_7100_ur.c | 코드 검사 |
| V-05 | TrCode memcpy + '\0' (sprintf 대체) | pb_7100_ur.c | 코드 검사 |
| V-06 | LK 경로 memset 제거, rbuf[15]='\0' | pb_7100_ur.c | 코드 검사 |
| V-07 | LK 경로 rbuf_len = 15 (strlen 대체) | pb_7100_ur.c | 코드 검사 |
| V-08 | sleep(1) → poll(Poll,1,1000) | pb_1100_ts.c | 코드 검사 |
| V-09 | FIFO 이벤트 시 Fifo_Event_Rtn() 호출 | pb_1100_ts.c | 코드 검사 |
| V-10 | 들여쓰기 스타일 보존 (space) | pb_1100_ts.c | 코드 검사 |
| V-11 | 빌드 성공 | mk.sh src/PB | 서버 환경 |

---

## 4. Design Decisions

| Decision | Selected | Rationale |
|----------|----------|-----------|
| RD 로그 처리 | 제거 | TCP SD 로그가 동일 데이터+INT_SEQ 기록. USR_OK = 정보성. 초당 수천건 I/O 병목 |
| LK rbuf_len | 상수 15 | "0011"(4) + "LK000000000"(11) = 15. 변경 가능성 없음 |
| poll nfds | 1 (FIFO만) | 장외에 TCP 소켓 불필요. FIFO로 운영자 명령 수신 가능 |
| poll timeout | 1000ms | 기존 sleep(1) 동일. 상태 확인 주기 유지 |
| F_R_Proc | 변경 없음 | 이미 tmp_off 추적 방식으로 최적화 완료 |

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2026-02-28 | Initial design — 2 files, 11 V items | Claude |
