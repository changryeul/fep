# poll-traversal Design Document

> **Summary**: poll 이벤트 이중 순회 → 단일 순회 통합 (12개 파일)
>
> **Project**: FEP (Front-End Processor)
> **Author**: Claude
> **Date**: 2026-02-28
> **Status**: Draft
> **Planning Doc**: [poll-traversal.plan.md](../../01-plan/features/poll-traversal.plan.md)

---

## 1. Implementation Steps

### Step 1: detect_poll_event() 단일 루프 통합

**File**: `st01/sub/poll_event.c`

**Before** (lines 19-50):
```c
int detect_poll_event(struct pollfd *poll_arr, int poll_cnt, int socket_event_idx)
{
	int		i;

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

	return -2;	/* no event */
}
```

**After**:
```c
int detect_poll_event(struct pollfd *poll_arr, int poll_cnt, int socket_event_idx)
{
	int		i, first_pollin;

	first_pollin = -2;

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

		if ((poll_arr[i].revents & POLLIN) && first_pollin == -2)
		{
			poll_arr[i].revents = 0;
			first_pollin = i;
		}
	}

	return first_pollin;
}
```

**변경 핵심**:
- 이중 루프 → 단일 루프
- `first_pollin` 변수로 첫 POLLIN 인덱스 기록 (모든 POLLHUP 체크 보장)
- 반환값 의미론 동일: -1 (소켓 끊김), -2 (이벤트 없음), >=0 (POLLIN 인덱스)
- 7개 호출자 수정 불필요

---

### Step 2: 인라인 이중 루프 → 단일 루프 (11개소)

각 파일의 이중 for 루프 + 외부 switch를 단일 for 루프 + 내부 switch로 통합.

#### 2-A: pa_1200_tr.c (lines 255-295)

**Before**:
```c
		/* POLLHUP(끊김) 체크 — 소켓이면 종료, FIFO면 무시하고 계속 */
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

		/* POLLIN(데이터 도착) 이벤트 감지 */
		for (i = 0; i < PollCnt; i ++)
		{
			if (Poll[i].revents & POLLIN)
			{
				Poll[i].revents = 0;
				break;
			}
		}

		/* 이벤트 종류별 분기 */
		switch (i)
		{
			case    FIFO_EVENT:					/* 데몬 제어 신호 수신 */
				Fifo_Event_Rtn ();
				break;
			case    SOCKET_EVENT:				/* KRX 데이터 수신 */
				Socket_Event_Rtn ();
				break;
			default:
				Log (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
				Exit_Process ();
				break;
		}
```

**After**:
```c
		/* poll 이벤트 단일 순회 — POLLHUP 체크 후 POLLIN 처리 */
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
			}

			if (Poll[i].revents & POLLIN)
			{
				Poll[i].revents = 0;

				switch (i)
				{
					case    FIFO_EVENT:
						Fifo_Event_Rtn ();
						break;
					case    SOCKET_EVENT:
						Socket_Event_Rtn ();
						break;
					default:
						Log (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
						Exit_Process ();
						break;
				}
			}
		}
```

#### 2-B: pb_1200_tr.c (lines 259-299)

pa_1200_tr.c와 동일한 패턴 (FIFO_EVENT, SOCKET_EVENT). 동일 변환 적용.

#### 2-C: pa_7000_tr.c (lines 209-244)

**switch cases**: SOCKET_EVENT only + default. 동일 변환.

#### 2-D: pa_8200_tr.c (lines 223-258)

**switch cases**: SOCKET_EVENT only + default. 동일 변환.

#### 2-E: pb_7200_tr.c (lines 165-201)

**switch cases**: SOCKET_EVENT only + default. 동일 변환.
**주의**: 2단계 들여쓰기 (외부 while 루프 내부).

#### 2-F: pb_7100_ts.c (lines 211-253)

**switch cases**: SOCKET_EVENT, DATA_EVENT + default. 동일 변환.
**주의**: 2단계 들여쓰기 (외부 while 루프 내부).

#### 2-G: pb_8100_ts.c (lines 258-302)

**Before** (주석 포함):
```c
		/*
		 * PB 고유: poll 이벤트를 수동 for 루프로 감지
		 * (PA는 detect_poll_event 함수 사용)
		 */

		/* 1) POLLHUP(접속 끊김) 감지 */
		for (i = 0; i < PollCnt; i ++)
		{
			if (Poll[i].revents & POLLHUP)
			{
				if (i == SOCKET_EVENT)
				{
					Log (TCP_ERROR, "socket disconnected[%#06x]", Poll[i].revents);
					return;
				}

				Log (SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
				continue;
			}
		}

		/* 2) POLLIN(데이터 도착) 감지 */
		for (i = 0; i < PollCnt; i ++)
		{
			if (Poll[i].revents & POLLIN)
			{
				Poll[i].revents = 0;
				break;
			}
		}

		/* 이벤트별 분기 처리 */
		switch (i)
		{
			case	SOCKET_EVENT:
				Socket_Event_Rtn ();
				break;
			case	DATA_EVENT:
				Data_Event_Rtn ();
				break;
			default:
				Log (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
				Exit_Process ();
				break;
		}
```

**After**:
```c
		/* poll 이벤트 단일 순회 — POLLHUP 체크 후 POLLIN 처리 */
		for (i = 0; i < PollCnt; i ++)
		{
			if (Poll[i].revents & POLLHUP)
			{
				if (i == SOCKET_EVENT)
				{
					Log (TCP_ERROR, "socket disconnected[%#06x]", Poll[i].revents);
					return;
				}

				Log (SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
			}

			if (Poll[i].revents & POLLIN)
			{
				Poll[i].revents = 0;

				switch (i)
				{
					case	SOCKET_EVENT:
						Socket_Event_Rtn ();
						break;
					case	DATA_EVENT:
						Data_Event_Rtn ();
						break;
					default:
						Log (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
						Exit_Process ();
						break;
				}
			}
		}
```

#### 2-H: pb_1800_ts.c (lines 259-299)

**switch cases**: FIFO_EVENT, SOCKET_EVENT, DATA_EVENT + default. 동일 변환.
**주의**: 스페이스 들여쓰기 (탭 아님).

#### 2-I: pb_7800_tr.c (lines 167-204)

**switch cases**: FIFO_EVENT, SOCKET_EVENT + default. 동일 변환.
**주의**: 스페이스 들여쓰기.

#### 2-J: pw_4000_ts.c (lines 236-278)

**Before**:
```c
		for (i = 0; i < PollCnt; i ++)
		{
			if (Poll[i].revents & POLLHUP)
			{
				if (i == SOCKET_EVENT)
				{
					SLog (TCP_ERROR, "socket disconnected[%#06x]",
						Poll[i].revents);
					return;
				}

				SLog (SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
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

		switch (i)
		{
			case	FIFO_EVENT:
				Fifo_Event_Rtn ();
				break;
			case	SOCKET_EVENT:
				rt = Receive_Packet ();
				if (rt == NOTOK)
					return;
				break;
			case	FILE_EVENT:
				File_Event_Rtn ();
				break;
			default:
				SLog (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
				return;
				break;
		}
```

**After**:
```c
		/* poll 이벤트 단일 순회 */
		for (i = 0; i < PollCnt; i ++)
		{
			if (Poll[i].revents & POLLHUP)
			{
				if (i == SOCKET_EVENT)
				{
					SLog (TCP_ERROR, "socket disconnected[%#06x]",
						Poll[i].revents);
					return;
				}

				SLog (SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
			}

			if (Poll[i].revents & POLLIN)
			{
				Poll[i].revents = 0;

				switch (i)
				{
					case	FIFO_EVENT:
						Fifo_Event_Rtn ();
						break;
					case	SOCKET_EVENT:
						rt = Receive_Packet ();
						if (rt == NOTOK)
							return;
						break;
					case	FILE_EVENT:
						File_Event_Rtn ();
						break;
					default:
						SLog (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
						return;
						break;
				}
			}
		}
```

**차이점**: SLog 사용, SOCKET_EVENT에 Receive_Packet 특수 처리, FILE_EVENT 추가, default에서 return (Exit_Process 아님).

#### 2-K: pa_3100_ts.c 보조 루프 (lines 705-735)

**Before**:
```c
        for (i = 0; i < PollCnt; i ++)
        {
            if (Poll[i].revents & POLLHUP)
            {
                SLog (SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
                return;
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

        switch (i)
        {
            case    FIFO_EVENT:
                Fifo_Event_Rtn ();
                break;
            case    1:
                Jang_End_Time_File_Rtn ();
                break;
            default:
                SLog (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                Exit_Process ();
                break;
        }
```

**After**:
```c
        /* poll 이벤트 단일 순회 */
        for (i = 0; i < PollCnt; i ++)
        {
            if (Poll[i].revents & POLLHUP)
            {
                SLog (SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
                return;
            }

            if (Poll[i].revents & POLLIN)
            {
                Poll[i].revents = 0;

                switch (i)
                {
                    case    FIFO_EVENT:
                        Fifo_Event_Rtn ();
                        break;
                    case    1:
                        Jang_End_Time_File_Rtn ();
                        break;
                    default:
                        SLog (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                        Exit_Process ();
                        break;
                }
            }
        }
```

**차이점**: POLLHUP → ANY fd에서 return (SOCKET_EVENT 체크 없음). SLog 사용. case 1 = Jang_End_Time_File_Rtn().

---

## 2. Implementation Order

| 순서 | 파일 | 이유 |
|:----:|------|------|
| 1 | `sub/poll_event.c` | 공유 함수 — 7개 호출자 자동 적용 |
| 2 | `src/PA/pa_1200_tr.c` | 대표 PA 인라인 패턴 |
| 3 | `src/PB/pb_1200_tr.c` | pa_1200_tr와 동일 패턴 |
| 4 | `src/PA/pa_7000_tr.c` | SOCKET only 패턴 |
| 5 | `src/PA/pa_8200_tr.c` | SOCKET only 패턴 |
| 6 | `src/PB/pb_7200_tr.c` | SOCKET only + 2단계 indent |
| 7 | `src/PB/pb_7100_ts.c` | SOCKET + DATA 패턴 |
| 8 | `src/PB/pb_8100_ts.c` | SOCKET + DATA + PB 주석 |
| 9 | `src/PB/pb_1800_ts.c` | FIFO + SOCKET + DATA |
| 10 | `src/PB/pb_7800_tr.c` | FIFO + SOCKET |
| 11 | `src/PW/pw_4000_ts.c` | 특수: SLog, FILE_EVENT, Receive_Packet |
| 12 | `src/PA/pa_3100_ts.c` | 보조 루프 (메인은 detect_poll_event 유지) |

---

## 3. 파일별 변환 요약

| # | 파일 | switch cases | Log 함수 | POLLHUP 처리 | 특이사항 |
|---|------|-------------|----------|-------------|---------|
| 1 | poll_event.c | N/A (return index) | Log | socket→-1, else→log | first_pollin 변수 추가 |
| 2 | pa_1200_tr.c | FIFO, SOCKET | Log | socket→return | 기본 패턴 |
| 3 | pb_1200_tr.c | FIFO, SOCKET | Log | socket→return | pa_1200_tr 동일 |
| 4 | pa_7000_tr.c | SOCKET | Log | socket→return | 2-case only |
| 5 | pa_8200_tr.c | SOCKET | Log | socket→return | 2-case only |
| 6 | pb_7200_tr.c | SOCKET | Log | socket→return | 2단계 indent |
| 7 | pb_7100_ts.c | SOCKET, DATA | Log | socket→return | 2단계 indent |
| 8 | pb_8100_ts.c | SOCKET, DATA | Log | socket→return | PB 고유 주석 제거 |
| 9 | pb_1800_ts.c | FIFO, SOCKET, DATA | Log | socket→return | 스페이스 indent |
| 10 | pb_7800_tr.c | FIFO, SOCKET | Log | socket→return | 스페이스 indent |
| 11 | pw_4000_ts.c | FIFO, SOCKET, FILE | SLog | socket→return | Receive_Packet, default→return |
| 12 | pa_3100_ts.c | FIFO, case 1 | SLog | any→return | 보조 루프, SOCKET 체크 없음 |

---

## 4. Verification Items

| ID | 검증 항목 | 대상 | 방법 |
|----|----------|------|------|
| V-01 | detect_poll_event 단일 루프 적용 | poll_event.c | 코드 확인 |
| V-02 | first_pollin 변수 선언 및 초기화 | poll_event.c | `-2` 초기화 확인 |
| V-03 | detect_poll_event 반환값 의미론 유지 | poll_event.c | -1, -2, >=0 확인 |
| V-04 | 이중 for 루프 완전 제거 (인라인) | 11개 소스 | grep 검증 |
| V-05 | switch가 POLLIN if 블록 내부에 위치 | 11개 소스 | 코드 확인 |
| V-06 | POLLHUP 체크가 POLLIN 전에 위치 | 12개 전체 | 코드 확인 |
| V-07 | Poll[i].revents = 0 유지 | 12개 전체 | 코드 확인 |
| V-08 | POLLHUP continue 제거 (단일 루프에서 불필요) | 인라인 11개 | `continue` → 삭제 확인 |
| V-09 | POLLIN break 제거 (switch의 break만 존재) | 인라인 11개 | 코드 확인 |
| V-10 | 각 파일 switch case 보존 | 11개 소스 | 파일별 case 목록 대조 |
| V-11 | Log vs SLog 함수 보존 | pw_4000_ts, pa_3100_ts | 코드 확인 |
| V-12 | pw_4000_ts Receive_Packet 특수 처리 보존 | pw_4000_ts | 코드 확인 |
| V-13 | pa_3100_ts 보조 루프 ANY POLLHUP→return 유지 | pa_3100_ts | 코드 확인 |
| V-14 | 들여쓰기 스타일 파일별 보존 | 12개 전체 | 탭/스페이스 확인 |
| V-15 | detect_poll_event 호출자 7개 수정 없음 | 7개 호출자 | grep 검증 |
| V-16 | 빌드 성공 | 전체 | mk.sh sub && mk.sh src |

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-28 | Initial draft — 12 files, 16 verification items | Claude |
