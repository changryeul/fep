# 설계: if1-block-cleanup

## 참조

- 계획: `docs/01-plan/features/if1-block-cleanup.plan.md`

## 설계 원칙

**언래핑 전용**: 각 `#if (1)` / `#if 1` 블록에 대해 `#if` 라인과 일치하는 `#endif` 라인을 삭제. 모든 내부 코드를 정확히 그대로 유지 (동일한 들여쓰기, 동일한 내용). 이것은 제로 기능 변경을 생성 — 전처리기가 이미 이 코드를 무조건부로 포함함.

## 구현 순서

각 파일 내에서 하향식으로 처리 (먼저 더 높은 라인 번호)하여 라인 참조 유지.

### Batch 1: PA 모듈 — UDP 전송 파일 (7개 파일, 11개 블록)

#### pa_7000_us.c (3개 블록 — 하향식 처리: FR-01, FR-08, FR-06)

| FR | 작업 | 삭제할 라인 |
|----|--------|-----------------|
| FR-01 | 라인 189 및 193 삭제 | SO_BROADCAST setsockopt 주변 `#if (1)` 및 `#endif` |
| FR-08 | 라인 143 및 145 삭제 | UDP 전송 로그 주변 `#if (1)` 및 `#endif` |
| FR-06 | 라인 132 및 140 삭제 | sendto() 블록 주변 `#if (1)` 및 `#endif` |

#### pa_7010_us.c (3개 블록 — 하향식 처리: FR-02, FR-09, FR-07)

| FR | 작업 | 삭제할 라인 |
|----|--------|-----------------|
| FR-02 | 라인 189 및 193 삭제 | SO_BROADCAST setsockopt 주변 `#if (1)` 및 `#endif` |
| FR-09 | 라인 143 및 145 삭제 | UDP 전송 로그 주변 `#if (1)` 및 `#endif` |
| FR-07 | 라인 132 및 140 삭제 | sendto() 블록 주변 `#if (1)` 및 `#endif` |

#### pa_7030_us.c (2개 블록 — 하향식 처리: FR-03, FR-10)

| FR | 작업 | 삭제할 라인 |
|----|--------|-----------------|
| FR-03 | 라인 250 및 252 삭제 | SO_BROADCAST setsockopt 주변 `#if (1)` 및 `#endif` |
| FR-10 | 라인 145 및 148 삭제 | sleep 테스트 로그 주변 `#if (1)` 및 `#endif`; 또한 라인 146의 `/* Test Log */` 주석 삭제 |

#### pa_9999_us.c (1개 블록: FR-04)

| FR | 작업 | 삭제할 라인 |
|----|--------|-----------------|
| FR-04 | 라인 212 및 216 삭제 | SO_BROADCAST setsockopt 주변 `#if (1)` 및 `#endif` |

#### pa_7000_mp.c (1개 블록: FR-05)

| FR | 작업 | 삭제할 라인 |
|----|--------|-----------------|
| FR-05 | 라인 379 및 381 삭제 | SO_BROADCAST setsockopt 주변 `#if (1)` 및 `#endif` |

#### pa_7500_us.c (1개 블록: FR-11)

| FR | 작업 | 삭제할 라인 |
|----|--------|-----------------|
| FR-11 | 라인 144 및 147 삭제 | UDP 전송 테스트 로그 주변 `#if (1)` 및 `#endif`; 또한 라인 145의 `/* Test Log	*/` 주석 삭제 |

### Batch 2: PA 모듈 — 자동 거래 및 기타 (1개 파일, 2개 블록)

#### pa_5010_mp.c (2개 블록 — 하향식 처리: FR-15, FR-14)

| FR | 작업 | 삭제할 라인 |
|----|--------|-----------------|
| FR-15 | 라인 1829 및 1867 삭제 | 선물 주문 디버그 주변 `#if (1)` 및 `#endif`; 또한 라인 1830의 `/* Test Log */` 삭제 |
| FR-14 | 라인 1545 및 1583 삭제 | 옵션 주문 디버그 주변 `#if (1)` 및 `#endif`; 또한 라인 1546의 `/* Test Log */` 삭제 |

### Batch 3: PB 모듈 (1개 파일, 1개 블록)

#### pb_1100_ts.c (1개 블록: FR-16)

| FR | 작업 | 삭제할 라인 |
|----|--------|-----------------|
| FR-16 | 라인 158 및 160 삭제 | sleep(1) 주변 `#if	1	/* HONG : ... */` 및 `#endif` |

주의: `#if` 라인은 `/* HONG : 장종료후 CPU 부하 상승으로 임시 추가 검토해 보세요 */` 주석을 포함. 이 주석은 `sleep(1)`의 이유를 문서화. `sleep(1)` 자체는 유지되므로 선택적으로 주석을 `sleep(1)` 라인 위의 독립형 `/* ... */`으로 유지. 그러나 주석은 "임시 추가 검토해 보세요" (이 임시 추가를 검토하세요)라고 말하며, 이것이 수년 동안 프로덕션 코드이므로 주석은 오래됨. **결정**: 주석을 포함하여 `#if` 라인을 완전히 삭제. `sleep(1)`은 컨텍스트에서 자명함.

### Batch 4: PW + PZ 모듈 (2개 파일, 2개 블록)

#### pw_3030_tr.c (1개 블록: FR-12)

| FR | 작업 | 삭제할 라인 |
|----|--------|-----------------|
| FR-12 | 라인 385 및 388 삭제 | DSHM 쓰기 로그 주변 `#if 1   /* test */` 및 `#endif` |

#### pz_memory_conf.c (1개 블록: FR-13)

| FR | 작업 | 삭제할 라인 |
|----|--------|-----------------|
| FR-13 | 라인 727 및 729 삭제 | 파일 경로 로그 주변 `#if (1)` 및 `#endif` |

## FR 상세 — 이전/이후

### FR-01: pa_7000_us.c:189-193

**이전:**
```c
#if (1)
		bufflen = 1;
		oplen   = sizeof(bufflen);
		setsockopt (Sockfd[i],SOL_SOCKET,SO_BROADCAST, (void *)&bufflen, oplen);
#endif
```
**이후:**
```c
		bufflen = 1;
		oplen   = sizeof(bufflen);
		setsockopt (Sockfd[i],SOL_SOCKET,SO_BROADCAST, (void *)&bufflen, oplen);
```

### FR-02: pa_7010_us.c:189-193
FR-01과 같은 패턴.

### FR-03: pa_7030_us.c:250-252

**이전:**
```c
#if (1)
            setsockopt (Sockfd[i][j],SOL_SOCKET,SO_BROADCAST, (void *)&bufflen, oplen);
#endif
```
**이후:**
```c
            setsockopt (Sockfd[i][j],SOL_SOCKET,SO_BROADCAST, (void *)&bufflen, oplen);
```

### FR-04: pa_9999_us.c:212-216

**이전:**
```c
#if (1)
		bufflen = 1;
		oplen   = sizeof(bufflen);
		setsockopt (Sockfd[i],SOL_SOCKET,SO_BROADCAST, (void *)&bufflen, oplen);
#endif
```
**이후:**
```c
		bufflen = 1;
		oplen   = sizeof(bufflen);
		setsockopt (Sockfd[i],SOL_SOCKET,SO_BROADCAST, (void *)&bufflen, oplen);
```

### FR-05: pa_7000_mp.c:379-381

**이전:**
```c
#if (1)
			setsockopt (Sockfd[i][j],SOL_SOCKET,SO_BROADCAST, (void *)&bufflen, oplen);
#endif
```
**이후:**
```c
			setsockopt (Sockfd[i][j],SOL_SOCKET,SO_BROADCAST, (void *)&bufflen, oplen);
```

### FR-06: pa_7000_us.c:132-140

**이전:**
```c
#if (1)
			rt = sendto (Sockfd[i], send_data, strlen(send_data),
				0, (struct sockaddr *)&Svr_Addr[i], add_len);
			if (rt < 0)
			{
				Log (UDP_FATAL, "Data Send Error [%d:%s]", SYS_NO, SYS_STR);
				break;
			}
#endif
```
**이후:**
```c
			rt = sendto (Sockfd[i], send_data, strlen(send_data),
				0, (struct sockaddr *)&Svr_Addr[i], add_len);
			if (rt < 0)
			{
				Log (UDP_FATAL, "Data Send Error [%d:%s]", SYS_NO, SYS_STR);
				break;
			}
```

### FR-07: pa_7010_us.c:132-140
FR-06과 같은 패턴.

### FR-08: pa_7000_us.c:143-145

**이전:**
```c
#if (1)
		Log (USR_OK, "UDP S [%.10s](%d)", send_data, strlen (send_data));
#endif
```
**이후:**
```c
		Log (USR_OK, "UDP S [%.10s](%d)", send_data, strlen (send_data));
```

### FR-09: pa_7010_us.c:143-145
FR-08과 같은 패턴.

### FR-10: pa_7030_us.c:145-148

**이전:**
```c
#if (1)
/* Test Log */
Log (USR_OK, "sleep [%d] sleep_speed [%d]", sltime, sleep_speed);
#endif
```
**이후:**
```c
Log (USR_OK, "sleep [%d] sleep_speed [%d]", sltime, sleep_speed);
```
주의: `/* Test Log */` 주석도 제거됨.

### FR-11: pa_7500_us.c:144-147

**이전:**
```c
#if (1)
/* Test Log	*/
				Log (USR_OK, "UDP S [%.100s](%d)", send_data, strlen(send_data));
#endif
```
**이후:**
```c
				Log (USR_OK, "UDP S [%.100s](%d)", send_data, strlen(send_data));
```
주의: `/* Test Log	*/` 주석도 제거됨.

### FR-12: pw_3030_tr.c:385-388

**이전:**
```c
#if 1   /* test */
    Log (USR_OK, "DSHM write[%s:%d:%d]",
        ODN(D_K,P_K,0), ODW(D_K,P_K,0,0), DataCnt);
#endif
```
**이후:**
```c
    Log (USR_OK, "DSHM write[%s:%d:%d]",
        ODN(D_K,P_K,0), ODW(D_K,P_K,0,0), DataCnt);
```

### FR-13: pz_memory_conf.c:727-729

**이전:**
```c
#if (1)
				Log (USR_OK, "file [%s]", file_name);
#endif
```
**이후:**
```c
				Log (USR_OK, "file [%s]", file_name);
```

### FR-14: pa_5010_mp.c:1545-1583

**이전**: 라인 1545의 `#if (1)`, 라인 1546의 `/* Test Log */`, 35줄의 디버그 로깅, 라인 1583의 `#endif`.
**이후**: 35줄의 디버그 로깅 유지. 라인 1545, 1546, 1583 삭제 (-3줄).

### FR-15: pa_5010_mp.c:1829-1867

**이전**: 라인 1829의 `#if (1)`, 라인 1830의 `/* Test Log */`, 35줄의 디버그 로깅, 라인 1867의 `#endif`.
**이후**: 35줄의 디버그 로깅 유지. 라인 1829, 1830, 1867 삭제 (-3줄).

### FR-16: pb_1100_ts.c:158-160

**이전:**
```c
#if	1	/* HONG : 장종료후 CPU 부하 상승으로 임시 추가 검토해 보세요 */
			sleep(1);
#endif
```
**이후:**
```c
			sleep(1);
```

## 요약 테이블

| FR | 파일 | 삭제된 라인 | 설명 |
|----|------|:------------:|-------------|
| FR-01 | pa_7000_us.c | 2 | SO_BROADCAST `#if`/`#endif` |
| FR-02 | pa_7010_us.c | 2 | SO_BROADCAST `#if`/`#endif` |
| FR-03 | pa_7030_us.c | 2 | SO_BROADCAST `#if`/`#endif` |
| FR-04 | pa_9999_us.c | 2 | SO_BROADCAST `#if`/`#endif` |
| FR-05 | pa_7000_mp.c | 2 | SO_BROADCAST `#if`/`#endif` |
| FR-06 | pa_7000_us.c | 2 | sendto `#if`/`#endif` |
| FR-07 | pa_7010_us.c | 2 | sendto `#if`/`#endif` |
| FR-08 | pa_7000_us.c | 2 | UDP 로그 `#if`/`#endif` |
| FR-09 | pa_7010_us.c | 2 | UDP 로그 `#if`/`#endif` |
| FR-10 | pa_7030_us.c | 3 | sleep 로그 `#if`/`/* Test Log */`/`#endif` |
| FR-11 | pa_7500_us.c | 3 | UDP 로그 `#if`/`/* Test Log */`/`#endif` |
| FR-12 | pw_3030_tr.c | 2 | DSHM 로그 `#if`/`#endif` |
| FR-13 | pz_memory_conf.c | 2 | 파일 로그 `#if`/`#endif` |
| FR-14 | pa_5010_mp.c | 3 | 옵션 디버그 `#if`/`/* Test Log */`/`#endif` |
| FR-15 | pa_5010_mp.c | 3 | 선물 디버그 `#if`/`/* Test Log */`/`#endif` |
| FR-16 | pb_1100_ts.c | 2 | sleep(1) `#if`/`#endif` |
| **합계** | **11개 파일** | **~36** | |

## 검증

모든 편집 후:
1. `grep -rn '#if.*1' st01/src/{PA,PB,PW,PX,PZ}/*.c` — `#if 1` / `#if (1)` 일치 제로 반환해야 함 (`#if defined`, `#elif`, `#ifdef`만 유지)
2. 빌드: `mk.sh src` — 동일한 바이너리로 깔끔하게 컴파일해야 함
