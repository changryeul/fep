# file-rw-optimize Design Document

> **Summary**: F_R/F_W 파일 잠금 범위 축소 — 전체 파일 잠금을 레코드/append 단위 잠금으로 변경
>
> **Plan Reference**: `docs/01-plan/features/file-rw-optimize.plan.md`
> **Date**: 2026-02-28
> **Status**: Draft

---

## 1. Design Overview

### 1.1 접근 방식

`sub/file_rw.c`의 7개 함수에서 `fcntl` 잠금 범위를 축소:

- **F_R 계열** (F_R, F_R2, F_R3): `F_WRLCK` 전체 파일 → `F_RDLCK` 레코드 범위
- **F_W 계열** (F_W, F_W2, F_W3, F_WB): `SEEK_SET` 전체 파일 → `SEEK_END` append 영역

핵심 원칙:
- F_R은 특정 offset의 레코드만 읽음 → 해당 범위만 공유 잠금(F_RDLCK)
- F_W는 파일 끝에 append → EOF 이후만 배타적 잠금(F_WRLCK)
- 읽기와 쓰기가 서로 다른 영역을 잠그므로 동시 수행 가능
- **호출자 변경 0건** — 함수 시그니처/반환값 불변

### 1.2 변경 파일 목록

| # | File | Change Type | Lines |
|---|------|-------------|-------|
| 1 | `sub/file_rw.c` | 수정 | ~35줄 변경 |

---

## 2. Implementation Order

```
Step 1: F_R() — 읽기 잠금 + 레코드 범위 (가장 많은 호출 31회)
Step 2: F_W() — append 영역 잠금 (43회 호출)
Step 3: F_R2() — 동일 패턴 적용
Step 4: F_R3() — 동일 패턴 적용
Step 5: F_W2() — 동일 패턴 적용
Step 6: F_W3() — 동일 패턴 적용
Step 7: F_WB() — 동일 패턴 적용
```

---

## 3. Detailed Design per FR

### FR-01: F_R() 레코드 단위 읽기 잠금 (lines 77-97)

**현재 코드** (`sub/file_rw.c:77-97`):
```c
	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = 0;
		lock.l_start = 0L;
		lock.l_len = 0L;

		rt = fcntl (fd, F_SETLKW, &lock);

		if (rt == -1)
		{
			if (SYS_NO == EINTR)
				continue;

			close (fd);
			Log (SAM_FATAL, "F_R:cannot lock (fcntl)[%d,%s] {%d:%s}",
				p_type, f_name, SYS_NO, SYS_STR);
			return (NOTOK);
		}
	} while (rt == -1);

	offset = IFR(D_K,P_K,Fk,Ck) * rec_size;
```

**변경할 코드**:
```c
	offset = IFR(D_K,P_K,Fk,Ck) * rec_size;

	do {
		lock.l_type = F_RDLCK;
		lock.l_whence = SEEK_SET;
		lock.l_start = offset;
		lock.l_len = (long)(rec_size * p_cnt);

		rt = fcntl (fd, F_SETLKW, &lock);

		if (rt == -1)
		{
			if (SYS_NO == EINTR)
				continue;

			close (fd);
			Log (SAM_FATAL, "F_R:cannot lock (fcntl)[%d,%s] {%d:%s}",
				p_type, f_name, SYS_NO, SYS_STR);
			return (NOTOK);
		}
	} while (rt == -1);
```

**핵심 사항**:
- `offset` 계산을 lock 전으로 이동 — `IFR(D_K,P_K,Fk,Ck)`는 본 프로세스 전용 SHM 카운터이므로 안전
- `F_WRLCK` → `F_RDLCK` — 읽기 전용이므로 공유 잠금. 복수 읽기 동시 수행 가능
- `l_start = offset`, `l_len = rec_size * p_cnt` — 읽을 레코드 범위만 잠금
- 기존 line 97의 `offset = ...` 제거 (lock 전으로 이동했으므로)
- 잠금 실패 시 에러 처리, EINTR 재시도 로직 완전 유지

### FR-02: F_W() append 영역 잠금 (lines 261-265)

**현재 코드** (`sub/file_rw.c:261-265`):
```c
	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = 0;
		lock.l_start = 0L;
		lock.l_len = 0L;
```

**변경할 코드**:
```c
	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = SEEK_END;
		lock.l_start = 0L;
		lock.l_len = 0L;
```

**추가 변경**: unlock 부분 (`sub/file_rw.c:285-286`):

**현재 코드**:
```c
	lock.l_type = F_UNLCK;
	fcntl (fileno (fp), F_SETLK, &lock);
```

**변경할 코드**:
```c
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0L;
	lock.l_len = 0L;
	fcntl (fileno (fp), F_SETLK, &lock);
```

**핵심 사항**:
- `lock.l_whence = SEEK_END` — 파일 끝(EOF) 기준. `l_start=0, l_len=0` = EOF부터 무한대까지 잠금
- 기존 데이터 영역(0 ~ EOF)은 잠기지 않음 → F_R 읽기와 동시 수행 가능
- **unlock 시 SEEK_SET 리셋 필수**: SEEK_END 잠금 후 쓰기하면 EOF가 이동하여 SEEK_END 기반 unlock이 정확한 범위를 해제하지 못함. `SEEK_SET, 0, 0`으로 전체 해제가 안전
- `fclose(fp)`가 바로 뒤따르므로 모든 잠금이 해제되지만, 명시적 해제가 올바른 관행

### FR-03: F_R2() 레코드 단위 읽기 잠금 (lines 479-499)

**현재 코드** (`sub/file_rw.c:479-499`):
```c
	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = 0;
		lock.l_start = 0L;
		lock.l_len = 0L;

		rt = fcntl (fd, F_SETLKW, &lock);
		...
	} while (rt == -1);

	offset = r_seq * rec_size;
```

**변경할 코드**:
```c
	offset = r_seq * rec_size;

	do {
		lock.l_type = F_RDLCK;
		lock.l_whence = SEEK_SET;
		lock.l_start = offset;
		lock.l_len = (long)rec_size;

		rt = fcntl (fd, F_SETLKW, &lock);
		...
	} while (rt == -1);
```

**핵심 사항**:
- F_R2는 단일 레코드 읽기 (`p_cnt=1`로 F_R_Proc 호출) → `l_len = rec_size`
- `offset = r_seq * rec_size` — 파라미터에서 직접 계산, SHM 무관 → lock 전 이동 안전
- 기존 line 499의 `offset = ...` 제거

### FR-04: F_R3() 레코드 단위 읽기 잠금 (lines 768-788)

**현재 코드** (`sub/file_rw.c:768-788`):
```c
	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = 0;
		lock.l_start = 0L;
		lock.l_len = 0L;

		rt = fcntl (fd, F_SETLKW, &lock);
		...
	} while (rt == -1);

	offset = FILEM(dk,fk).r_cnt[0] * rec_size;
```

**변경할 코드**:
```c
	offset = FILEM(dk,fk).r_cnt[0] * rec_size;

	do {
		lock.l_type = F_RDLCK;
		lock.l_whence = SEEK_SET;
		lock.l_start = offset;
		lock.l_len = (long)rec_size;

		rt = fcntl (fd, F_SETLKW, &lock);
		...
	} while (rt == -1);
```

**핵심 사항**:
- `FILEM(dk,fk).r_cnt[0]`는 SHM의 파일별 읽기 카운터 — 이 프로세스만 갱신하므로 lock 전 이동 안전
- 단일 레코드 읽기 → `l_len = rec_size`
- 기존 line 788의 `offset = ...` 제거

### FR-05: F_W2() append 영역 잠금 (lines 580-584)

**현재 코드** (`sub/file_rw.c:580-584`):
```c
	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = 0;
		lock.l_start = 0L;
		lock.l_len = 0L;
```

**변경할 코드**:
```c
	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = SEEK_END;
		lock.l_start = 0L;
		lock.l_len = 0L;
```

**추가 변경**: unlock 부분 (`sub/file_rw.c:604-605`):

**현재 코드**:
```c
	lock.l_type = F_UNLCK;
	fcntl (fileno (fp), F_SETLK, &lock);
```

**변경할 코드**:
```c
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0L;
	lock.l_len = 0L;
	fcntl (fileno (fp), F_SETLK, &lock);
```

### FR-06: F_W3() append 영역 잠금 (lines 858-862)

**현재 코드** (`sub/file_rw.c:858-862`):
```c
	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = 0;
		lock.l_start = 0L;
		lock.l_len = 0L;
```

**변경할 코드**:
```c
	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = SEEK_END;
		lock.l_start = 0L;
		lock.l_len = 0L;
```

**추가 변경**: unlock 부분 (`sub/file_rw.c:882-883`):

**현재 코드**:
```c
	lock.l_type = F_UNLCK;
	fcntl (fileno (fp), F_SETLK, &lock);
```

**변경할 코드**:
```c
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0L;
	lock.l_len = 0L;
	fcntl (fileno (fp), F_SETLK, &lock);
```

### FR-07: F_WB() append 영역 잠금 (lines 939-943)

**현재 코드** (`sub/file_rw.c:939-943`):
```c
	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = 0;
		lock.l_start = 0L;
		lock.l_len = 0L;
```

**변경할 코드**:
```c
	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = SEEK_END;
		lock.l_start = 0L;
		lock.l_len = 0L;
```

**추가 변경**: unlock 부분 (`sub/file_rw.c:1063-1064`):

**현재 코드**:
```c
	lock.l_type = F_UNLCK;
	fcntl (fileno (fp), F_SETLK, &lock);
```

**변경할 코드**:
```c
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0L;
	lock.l_len = 0L;
	fcntl (fileno (fp), F_SETLK, &lock);
```

### FR-08: 잠금 실패 시 기존 에러 처리 동작 유지

**설계**: 모든 7개 함수에서 잠금 실패 시 동작을 변경하지 않음:
- `EINTR` → `continue` (재시도)
- 기타 오류 → `close(fd)` 또는 `fclose(fp)` + `Log(SAM_FATAL, ...)` + `return (NOTOK)` 또는 `Exit_Process()`

**에러 경로의 unlock**: 에러 경로에서의 unlock (lseek 실패 등)은 `lock` 구조체에 잠금 시 설정한 값이 남아있으므로 정확한 범위를 해제. F_R 계열은 `l_start=offset, l_len=range`가 남아있어 정확. F_W 계열은 에러 경로에서도 `SEEK_SET, 0, 0` 리셋 적용.

### FR-09: EINTR 재시도 로직 유지

**설계**: 모든 `do { ... if (SYS_NO == EINTR) continue; ... } while (rt == -1)` 패턴 완전 유지. 잠금 범위만 변경하고 재시도 로직은 동일.

---

## 4. Exact Code Changes

### 4.1 F_R() — offset 이동 + 읽기 잠금 + 레코드 범위

**위치**: lines 77-97

```
Before (lines 77-97):

	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = 0;
		lock.l_start = 0L;
		lock.l_len = 0L;

		rt = fcntl (fd, F_SETLKW, &lock);

		if (rt == -1)
		{
			if (SYS_NO == EINTR)
				continue;

			close (fd);
			Log (SAM_FATAL, "F_R:cannot lock (fcntl)[%d,%s] {%d:%s}",
				p_type, f_name, SYS_NO, SYS_STR);
			return (NOTOK);
		}
	} while (rt == -1);

	offset = IFR(D_K,P_K,Fk,Ck) * rec_size;

After:

	offset = IFR(D_K,P_K,Fk,Ck) * rec_size;

	do {
		lock.l_type = F_RDLCK;
		lock.l_whence = SEEK_SET;
		lock.l_start = offset;
		lock.l_len = (long)(rec_size * p_cnt);

		rt = fcntl (fd, F_SETLKW, &lock);

		if (rt == -1)
		{
			if (SYS_NO == EINTR)
				continue;

			close (fd);
			Log (SAM_FATAL, "F_R:cannot lock (fcntl)[%d,%s] {%d:%s}",
				p_type, f_name, SYS_NO, SYS_STR);
			return (NOTOK);
		}
	} while (rt == -1);
```

### 4.2 F_W() — SEEK_END 잠금 + unlock 리셋

**위치 1**: lines 262-264 (lock)

```
Before:
		lock.l_type = F_WRLCK;
		lock.l_whence = 0;
		lock.l_start = 0L;
		lock.l_len = 0L;

After:
		lock.l_type = F_WRLCK;
		lock.l_whence = SEEK_END;
		lock.l_start = 0L;
		lock.l_len = 0L;
```

**위치 2**: lines 285-286 (unlock)

```
Before:
	lock.l_type = F_UNLCK;
	fcntl (fileno (fp), F_SETLK, &lock);

After:
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0L;
	lock.l_len = 0L;
	fcntl (fileno (fp), F_SETLK, &lock);
```

### 4.3 F_R2() — offset 이동 + 읽기 잠금 + 레코드 범위

**위치**: lines 479-499

```
Before (lines 479-499):

	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = 0;
		lock.l_start = 0L;
		lock.l_len = 0L;

		rt = fcntl (fd, F_SETLKW, &lock);

		if (rt == -1)
		{
			if (SYS_NO == EINTR)
				continue;

			close (fd);
			Log (SAM_FATAL, "F_R2:cannot lock (fcntl)[%s] {%d:%s}",
				f_name, SYS_NO, SYS_STR);
			return (NOTOK);
		}
	} while (rt == -1);

	offset = r_seq * rec_size;

After:

	offset = r_seq * rec_size;

	do {
		lock.l_type = F_RDLCK;
		lock.l_whence = SEEK_SET;
		lock.l_start = offset;
		lock.l_len = (long)rec_size;

		rt = fcntl (fd, F_SETLKW, &lock);

		if (rt == -1)
		{
			if (SYS_NO == EINTR)
				continue;

			close (fd);
			Log (SAM_FATAL, "F_R2:cannot lock (fcntl)[%s] {%d:%s}",
				f_name, SYS_NO, SYS_STR);
			return (NOTOK);
		}
	} while (rt == -1);
```

### 4.4 F_R3() — offset 이동 + 읽기 잠금 + 레코드 범위

**위치**: lines 768-788

```
Before (lines 768-788):

	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = 0;
		lock.l_start = 0L;
		lock.l_len = 0L;

		rt = fcntl (fd, F_SETLKW, &lock);

		if (rt == -1)
		{
			if (SYS_NO == EINTR)
				continue;

			close (fd);
			Log (SAM_FATAL, "F_R3:cannot lock (fcntl)[%s] {%d:%s}",
				f_name, SYS_NO, SYS_STR);
			return (NOTOK);
		}
	} while (rt == -1);

	offset = FILEM(dk,fk).r_cnt[0] * rec_size;

After:

	offset = FILEM(dk,fk).r_cnt[0] * rec_size;

	do {
		lock.l_type = F_RDLCK;
		lock.l_whence = SEEK_SET;
		lock.l_start = offset;
		lock.l_len = (long)rec_size;

		rt = fcntl (fd, F_SETLKW, &lock);

		if (rt == -1)
		{
			if (SYS_NO == EINTR)
				continue;

			close (fd);
			Log (SAM_FATAL, "F_R3:cannot lock (fcntl)[%s] {%d:%s}",
				f_name, SYS_NO, SYS_STR);
			return (NOTOK);
		}
	} while (rt == -1);
```

### 4.5 F_W2() — SEEK_END 잠금 + unlock 리셋

**위치 1**: line 582 (lock)

```
Before:
		lock.l_whence = 0;

After:
		lock.l_whence = SEEK_END;
```

**위치 2**: lines 604-605 (unlock)

```
Before:
	lock.l_type = F_UNLCK;
	fcntl (fileno (fp), F_SETLK, &lock);

After:
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0L;
	lock.l_len = 0L;
	fcntl (fileno (fp), F_SETLK, &lock);
```

### 4.6 F_W3() — SEEK_END 잠금 + unlock 리셋

**위치 1**: line 860 (lock)

```
Before:
		lock.l_whence = 0;

After:
		lock.l_whence = SEEK_END;
```

**위치 2**: lines 882-883 (unlock)

```
Before:
	lock.l_type = F_UNLCK;
	fcntl (fileno (fp), F_SETLK, &lock);

After:
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0L;
	lock.l_len = 0L;
	fcntl (fileno (fp), F_SETLK, &lock);
```

### 4.7 F_WB() — SEEK_END 잠금 + unlock 리셋

**위치 1**: line 941 (lock)

```
Before:
		lock.l_whence = 0;

After:
		lock.l_whence = SEEK_END;
```

**위치 2**: lines 1063-1064 (unlock)

```
Before:
	lock.l_type = F_UNLCK;
	fcntl (fileno (fp), F_SETLK, &lock);

After:
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0L;
	lock.l_len = 0L;
	fcntl (fileno (fp), F_SETLK, &lock);
```

---

## 5. Verification Checklist

| # | 검증 항목 | 확인 방법 |
|---|-----------|----------|
| V-01 | F_R의 lock.l_type이 F_RDLCK으로 변경됨 | 코드 확인 |
| V-02 | F_R의 lock.l_start = offset, l_len = rec_size * p_cnt | 코드 확인 |
| V-03 | F_R의 offset 계산이 lock 전으로 이동됨 | 코드 확인 |
| V-04 | F_R2의 동일 패턴 적용 (F_RDLCK, 레코드 범위, offset 이동) | 코드 확인 |
| V-05 | F_R3의 동일 패턴 적용 (F_RDLCK, 레코드 범위, offset 이동) | 코드 확인 |
| V-06 | F_W의 lock.l_whence가 SEEK_END로 변경됨 | 코드 확인 |
| V-07 | F_W의 unlock에서 SEEK_SET, 0, 0 리셋 적용 | 코드 확인 |
| V-08 | F_W2의 동일 패턴 적용 (SEEK_END + unlock 리셋) | 코드 확인 |
| V-09 | F_W3의 동일 패턴 적용 (SEEK_END + unlock 리셋) | 코드 확인 |
| V-10 | F_WB의 동일 패턴 적용 (SEEK_END + unlock 리셋) | 코드 확인 |
| V-11 | EINTR 재시도 로직이 모든 함수에서 유지됨 | 코드 확인 |
| V-12 | 에러 경로의 unlock이 정상 동작함 | 에러 경로 확인 |
| V-13 | 기존 호출자 74곳 변경 없음 | grep 확인 |
| V-14 | 빌드 성공 (`mk.sh sub`) | 빌드 실행 |

---

## 6. Edge Cases

### 6.1 offset이 파일 크기를 초과하는 경우

F_R에서 레코드 범위 잠금 후 lseek하면 파일이 해당 위치까지 없을 수 있음. 이 경우 `read()`가 0을 반환하여 F_R_Proc에서 정상 처리됨 (line 146-157). 현재 동작과 동일.

### 6.2 F_R과 F_W의 동시 접근

F_R은 기존 레코드를 읽고, F_W는 파일 끝에 append. 잠금 범위가 겹치지 않으므로 동시 수행 가능. 새로 append된 데이터를 F_R이 읽으려면 다음 F_R 호출에서 갱신된 IFR 값으로 접근.

### 6.3 두 F_W가 동시에 같은 파일에 append

둘 다 SEEK_END + 0 + 0 잠금 → 같은 범위이므로 하나가 대기. 이는 올바른 동작 — append 직렬화 필요.

### 6.4 F_W2의 에러 경로 (Exit_Process)

F_W2는 잠금 실패 시 `fclose(fp); Exit_Process()` 호출 (line 593-596). Exit_Process에서 fclose가 fd를 닫으면 모든 잠금이 자동 해제됨. 변경 없이 안전.

---

## 7. Build & Test

```sh
mk.sh sub          # libfepP.a 재빌드 (file_rw.o 재컴파일)
mk.sh src          # 전체 바이너리 재링크
```

검증:
```sh
grep -n "F_RDLCK\|SEEK_END\|l_whence" st01/sub/file_rw.c
```

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-28 | Initial design | Claude |
