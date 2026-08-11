# 설계: socket-linger-extraction

## 참조
- 계획: `docs/01-plan/features/socket-linger-extraction.plan.md`

## 구현 순서

1. LIB-01: `inc/fep_common.h`에 선언 추가
2. LIB-02: `sub/fep_common.c`에 정의 추가
3. FR-01 ~ FR-09: 그룹 A — Sockfd 파일 (정의 제거, 호출 업데이트)
4. FR-10: 그룹 B — pw_4000_ts.c (Sockfd + SLog→Log)
5. FR-11 ~ FR-12: 그룹 C — Newfd 파일 (정의 제거, 호출 업데이트)
6. FR-13 ~ FR-15: 그룹 D — Dead code 파일 (정의만 제거)

---

## LIB-01: inc/fep_common.h — 선언 추가

**파일**: `st01/inc/fep_common.h`

### BEFORE (줄 38-41)
```c
extern void		Time_Out_Disconnect (const char *);
extern void		Fifo_Event_Rtn (void);

/*------------------------------------------------------------------------
```

### AFTER
```c
extern void		Time_Out_Disconnect (const char *);
extern void		Fifo_Event_Rtn (void);
extern void		Set_Socket_Linger (int);

/*------------------------------------------------------------------------
```

**변경**: +1 줄

---

## LIB-02: sub/fep_common.c — 정의 추가

**파일**: `st01/sub/fep_common.c`

### BEFORE (줄 385-387)
```c
/*************************************************************************
	End of Program (fep_common.c)
*************************************************************************/
```

### AFTER
```c
/*************************************************************************
	Function		: . Set_Socket_Linger
	Parameters IN	: . fd : socket file descriptor
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . set linger option on socket (immediate RST on close)
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Set_Socket_Linger (int fd)
/*----------------------------------------------------------------------*/
{
	int				rt;
	struct linger	ling;

	/* close () returns after discarding any unsent data */
	ling.l_onoff = 1;
	ling.l_linger = 0;

	rt = setsockopt (fd, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof (ling));
	if (rt < 0)
		Log (TCP_ERROR, "setsockopt SO_LINGER {%d:%s}", SYS_NO, SYS_STR);

	return;
}	/* End of Set_Socket_Linger ()	*/

/*************************************************************************
	End of Program (fep_common.c)
*************************************************************************/
```

**변경**: +17 줄

---

## FR-01: pa_2100_ts.c — Sockfd, active call

**파일**: `st01/src/PA/pa_2100_ts.c`

### 변경 1: forward 선언 제거 (줄 71)
```
BEFORE: void	Set_Socket_Linger 	(void);
AFTER:  (줄 삭제)
```

### 변경 2: 호출 사이트 업데이트 (줄 261)
```
BEFORE: 	Set_Socket_Linger ();
AFTER:  	Set_Socket_Linger (Sockfd);
```

### 변경 3: 정의 제거 (줄 1038-1061)
```
줄 1038-1061 삭제:
/*************************************************************************
	Function  : . Set_Socket_Linger
	...
	return;
}
```
**제거된 줄**: -24 (comment banner 8 + body 16)

---

## FR-02: pa_2200_tr.c — Sockfd, active call

**파일**: `st01/src/PA/pa_2200_tr.c`

### 변경 1: forward 선언 제거 (줄 58)
```
BEFORE: void	Set_Socket_Linger	(void);
AFTER:  (줄 삭제)
```

### 변경 2: 호출 사이트 업데이트 (줄 241)
```
BEFORE: 	Set_Socket_Linger ();
AFTER:  	Set_Socket_Linger (Sockfd);
```

### 변경 3: 정의 제거 (줄 765-790)
```
줄 765-790 삭제:
/*************************************************************************
	Function		: . Set_Socket_Linger
	...
	return;
}
```
**제거된 줄**: -26 (comment banner 8 + body 18)

---

## FR-03: pa_2700_tr.c — Sockfd, active call

**파일**: `st01/src/PA/pa_2700_tr.c`

### 변경 1: forward 선언 제거 (줄 39)
```
BEFORE: void	Set_Socket_Linger	(void);
AFTER:  (줄 삭제)
```

### 변경 2: 호출 사이트 업데이트 (줄 224)
```
BEFORE: 	Set_Socket_Linger ();
AFTER:  	Set_Socket_Linger (Sockfd);
```

### 변경 3: 정의 제거 (줄 515-538)
```
줄 515-538 삭제:
/*************************************************************************
	Function		: . Set_Socket_Linger
	...
(함수 본문 끝까지 포함 줄)
```
**제거된 줄**: -24

---

## FR-04: pa_7000_tr.c — Sockfd, active call

**파일**: `st01/src/PA/pa_7000_tr.c`

### 변경 1: forward 선언 제거 (줄 52)
```
BEFORE: void	Set_Socket_Linger (void);
AFTER:  (줄 삭제)
```

### 변경 2: 호출 사이트 업데이트 (줄 145)
```
BEFORE: 				Set_Socket_Linger ();
AFTER:  				Set_Socket_Linger (Sockfd);
```

### 변경 3: 정의 제거 (줄 547-570)
```
줄 547-570 삭제:
/*************************************************************************
    Function        : . Set_Socket_Linger
	...
    return;
}
```
**제거된 줄**: -24

---

## FR-05: pa_1600_tr.c — Sockfd, active call

**파일**: `st01/src/PA/pa_1600_tr.c`

### 변경 1: forward 선언 제거 (줄 51)
```
BEFORE: void	Set_Socket_Linger	(void);
AFTER:  (줄 삭제)
```

### 변경 2: 호출 사이트 업데이트 (줄 277)
```
BEFORE: 	Set_Socket_Linger ();
AFTER:  	Set_Socket_Linger (Sockfd);
```

### 변경 3: 정의 제거 (줄 695-720)
```
줄 695-720 삭제:
/*************************************************************************
	Function		: . Set_Socket_Linger
	...
	return;
}
```
**제거된 줄**: -26

---

## FR-06: pa_8200_tr.c — Sockfd, active call

**파일**: `st01/src/PA/pa_8200_tr.c`

### 변경 1: forward 선언 제거 (줄 59)
```
BEFORE: void	Set_Socket_Linger (void);
AFTER:  (줄 삭제)
```

### 변경 2: 호출 사이트 업데이트 (줄 155)
```
BEFORE: 				Set_Socket_Linger ();
AFTER:  				Set_Socket_Linger (Sockfd);
```

### 변경 3: 정의 제거 (줄 671-694)
```
줄 671-694 삭제:
/*************************************************************************
    Function        : . Set_Socket_Linger
	...
    return;
}
```
**제거된 줄**: -24

---

## FR-07: pb_8200_tr.c — Sockfd, active call

**파일**: `st01/src/PB/pb_8200_tr.c`

### 변경 1: forward 선언 제거 (줄 63)
```
BEFORE: void	Set_Socket_Linger (void);
AFTER:  (줄 삭제)
```

### 변경 2: 호출 사이트 업데이트 (줄 150)
```
BEFORE: 				Set_Socket_Linger ();
AFTER:  				Set_Socket_Linger (Sockfd);
```

### 변경 3: 정의 제거 (줄 689-712)
```
줄 689-712 삭제:
/*************************************************************************
    Function        : . Set_Socket_Linger
	...
    return;
}
```
**제거된 줄**: -24

---

## FR-08: pw_3010_tr.c — Sockfd, active call

**파일**: `st01/src/PW/pw_3010_tr.c`

### 변경 1: forward 선언 제거 (줄 39)
```
BEFORE: void	Set_Socket_Linger (void);
AFTER:  (줄 삭제)
```

### 변경 2: 호출 사이트 업데이트 (줄 74)
```
BEFORE: 	Set_Socket_Linger ();
AFTER:  	Set_Socket_Linger (Sockfd);
```

### 변경 3: 정의 제거 (줄 604-628)
```
줄 604-628 삭제:
/*************************************************************************
	Function		: . Set_Socket_Linger
	...
	return;
}
```
**제거된 줄**: -25

---

## FR-09: pw_3030_tr.c — Sockfd, active call

**파일**: `st01/src/PW/pw_3030_tr.c`

### 변경 1: forward 선언 제거 (줄 39)
```
BEFORE: void	Set_Socket_Linger (void);
AFTER:  (줄 삭제)
```

### 변경 2: 호출 사이트 업데이트 (줄 74)
```
BEFORE: 	Set_Socket_Linger ();
AFTER:  	Set_Socket_Linger (Sockfd);
```

### 변경 3: 정의 제거 (줄 579-602)
```
줄 579-602 삭제:
/*************************************************************************
	Function		: . Set_Socket_Linger
	...
	return;
(함수 끝)
```
**제거된 줄**: -24

---

## FR-10: pw_4000_ts.c — Sockfd, SLog→Log, active call

**파일**: `st01/src/PW/pw_4000_ts.c`

### 변경 1: forward 선언 제거 (줄 58)
```
BEFORE: void	Set_Socket_Linger (void);
AFTER:  (줄 삭제)
```

### 변경 2: 호출 사이트 업데이트 (줄 93)
```
BEFORE: 	Set_Socket_Linger ();
AFTER:  	Set_Socket_Linger (Sockfd);
```

### 변경 3: 정의 제거 (줄 642-666)
```
줄 642-666 삭제:
/*************************************************************************
	Function		: . Set_Socket_Linger
	...  (본문은 SLog 사용 — shared version은 Log 사용)
	return;
}
```
**제거된 줄**: -25

**주**: Local copy는 `SLog` 사용. Shared version은 `Log` 사용. 둘 다 동등한 매크로.

---

## FR-11: pa_8100_ts.c — Newfd, active call

**파일**: `st01/src/PA/pa_8100_ts.c`

### 변경 1: forward 선언 제거 (줄 61)
```
BEFORE: void	Set_Socket_Linger (void);
AFTER:  (줄 삭제)
```

### 변경 2: 호출 사이트 업데이트 (줄 154)
```
BEFORE:                 Set_Socket_Linger ();
AFTER:                  Set_Socket_Linger (Newfd);
```

### 변경 3: 정의 제거 (줄 716-739)
```
줄 716-739 삭제:
/*************************************************************************
    Function        : . Set_Socket_Linger
	...
    return;
}
```
**제거된 줄**: -24

---

## FR-12: pb_8100_ts.c — Newfd, active call

**파일**: `st01/src/PB/pb_8100_ts.c`

### 변경 1: forward 선언 제거 (줄 74)
```
BEFORE: void	Set_Socket_Linger (void);
AFTER:  (줄 삭제)
```

### 변경 2: 호출 사이트 업데이트 (줄 158)
```
BEFORE:                 Set_Socket_Linger ();
AFTER:                  Set_Socket_Linger (Newfd);
```

### 변경 3: 정의 제거 (줄 710-733)
```
줄 710-733 삭제:
/*************************************************************************
    Function        : . Set_Socket_Linger
	...
    return;
}
```
**제거된 줄**: -24

---

## FR-13: pb_7200_tr.c — Dead code (Newfd, never called)

**파일**: `st01/src/PB/pb_7200_tr.c`

### 변경 1: forward 선언 제거 (줄 55)
```
BEFORE: void	Set_Socket_Linger (void);
AFTER:  (줄 삭제)
```

### 변경 2: 정의 제거 (줄 421-444)
```
줄 421-444 삭제:
/*************************************************************************
    Function        : . Set_Socket_Linger
	...
    return;
}
```
**제거된 줄**: -24

**호출 사이트 변경 없음** — 함수가 호출되지 않음.

---

## FR-14: pb_7100_ts.c — Dead code (Newfd, never called)

**파일**: `st01/src/PB/pb_7100_ts.c`

### 변경 1: forward 선언 제거 (줄 70)
```
BEFORE: void	Set_Socket_Linger (void);
AFTER:  (줄 삭제)
```

### 변경 2: 정의 제거 (줄 497-520)
```
줄 497-520 삭제:
/*************************************************************************
    Function        : . Set_Socket_Linger
	...
    return;
}
```
**제거된 줄**: -24

**호출 사이트 변경 없음** — 함수가 호출되지 않음.

---

## FR-15: pa_7100_ts.c — Dead code (Newfd, never called)

**파일**: `st01/src/PA/pa_7100_ts.c`

### 변경 1: forward 선언 제거 (줄 59)
```
BEFORE: void	Set_Socket_Linger (void);
AFTER:  (줄 삭제)
```

### 변경 2: 정의 제거 (줄 479-502)
```
줄 479-502 삭제:
/*************************************************************************
    Function        : . Set_Socket_Linger
	...
    return;
}
```
**제거된 줄**: -24

**호출 사이트 변경 없음** — 함수가 호출되지 않음.

---

## 요약

| 변경 유형 | 개수 | 줄 |
|-------------|-------|-------|
| LIB: 선언 추가 (fep_common.h) | 1 | +1 |
| LIB: 정의 추가 (fep_common.c) | 1 | +17 |
| Forward 선언 제거 | 15 | -15 |
| 정의 제거 (banners 포함) | 15 | ~-360 |
| 호출 사이트 업데이트 (Sockfd) | 10 | 0 (in-place) |
| 호출 사이트 업데이트 (Newfd) | 2 | 0 (in-place) |
| Dead code 호출 | 0 | 0 |
| **합계** | **17 files** | **~-357 net** |

## 검증 체크리스트

1. `grep -r "void.*Set_Socket_Linger" st01/src/`가 0개 일치 반환 (정의/선언 없음)
2. `grep -r "Set_Socket_Linger" st01/src/`가 정확히 12개 일치 반환 (호출 사이트만)
3. 모든 Sockfd 호출은 `Sockfd` 전달, 모든 Newfd 호출은 `Newfd` 전달
4. `inc/fep_common.h`가 `extern void Set_Socket_Linger (int);` 포함
5. `sub/fep_common.c`가 `void Set_Socket_Linger (int fd)` 정의 포함
6. 빌드: `mk.sh sub && mk.sh src`가 오류 없이 완료
