# PZ 모듈 코드 품질 감사 — 설계 문서

> **기능**: PZ (시스템 관리 모듈 코드 품질 감사)
> **프로젝트**: FEP (KRX Front-End Processor)
> **작성자**: Claude Code
> **날짜**: 2026-02-27
> **상태**: 초안
> **계획 참조**: `docs/01-plan/features/PZ.plan.md`

---

## 개요

10개 PZ 소스 파일 전체에 걸친 27개 발견 사항에 대한 상세한 수정 사양. 심각도별로 4개 구현 배치로 조직됨. 각 FR은 정확한 수정 전/후 코드 변경을 지정합니다.

## 수정된 파일

| 파일 | FR | 배치 |
|------|-----|-------|
| pz_daemon_proc.c | FR-01, FR-13, FR-17, FR-25 | 1,2,3,4 |
| pz_memory_conf.c | FR-02, FR-07, FR-18, FR-23 | 1,3,4 |
| pz_procchk.c | FR-03, FR-08, FR-09, FR-10, FR-14, FR-15, FR-16, FR-19, FR-26, FR-27 | 1,2,3,4 |
| pz_memory_proc.c | FR-04, FR-05, FR-06 | 1 |
| pz_fepp.c | FR-11, FR-12, FR-26 | 2,4 |
| pz_compact.c | FR-20, FR-26 | 3,4 |
| pz_memory.c | FR-21 | 3 |
| pz_memory_shm.c | FR-22, FR-24, FR-26 | 3,4 |
| pz_daemon.c | FR-26 | 4 |
| pz_filechk.c | (발견 사항 없음) | — |

---

## 배치 1: CRITICAL 수정 (FR-01부터 FR-07)

### FR-01: pz_daemon_proc.c의 형식 문자열 `[%]`

**파일**: `st01/src/PZ/pz_daemon_proc.c`
**라인**: 760

**수정 전**:
```c
	Log (SYS_FATAL, "cannot change directory[%] {%d:%s}",
		path, SYS_NO, SYS_STR);
```

**수정 후**:
```c
	Log (SYS_FATAL, "cannot change directory[%s] {%d:%s}",
		path, SYS_NO, SYS_STR);
```

**근거**: `[%]`는 잘못된 변환 지시자입니다. `%` 뒤에 `]`가 오는 것은 UB입니다 — `path` 인자는 소비되지만 형식 출력은 정의되지 않습니다. 디렉토리 경로를 올바르게 출력하기 위해 `[%s]`로 수정하세요.

---

### FR-02: pz_memory_conf.c에서 `%s` 형식으로 전달된 `strlen(buf)`

**파일**: `st01/src/PZ/pz_memory_conf.c`
**라인**: 125

**수정 전**:
```c
		Log (USR_FATAL, "daemon(%c):[%s][%s] line(%d)", cnt, buf, strlen(buf), __LINE__);
```

**수정 후**:
```c
		Log (USR_FATAL, "daemon(%c):[%s] len(%d) line(%d)", cnt, buf, (int)strlen(buf), __LINE__);
```

**근거**: 세 번째 vararg `strlen(buf)`는 `size_t` (정수)를 반환하지만 형식은 `%s` (char 포인터)를 예상합니다. Log 함수는 정수를 포인터 주소로 역참조할 것입니다 — **보장된 크래시** (SIGSEGV/SIGBUS). 수정: 두 번째 `%s`를 `(int)` 캐스트와 함께 `%d`로 변경하고, 명확성을 위해 라벨을 업데이트하세요.

---

### FR-03: pz_procchk.c의 전처리기 `defined()`

**파일**: `st01/src/PZ/pz_procchk.c`
**라인**: 998

**수정 전**:
```c
#if defined __hpux || sun || _AIX
```

**수정 후**:
```c
#if defined(__hpux) || defined(sun) || defined(_AIX)
```

**근거**: `defined()` 없는 `sun`과 `_AIX`는 매크로 확장으로 평가됩니다 — 정의되지 않으면 0 (거짓)으로 확장됩니다. `rt < 0` 확인 블록은 SunOS/AIX에서 조용히 건너뜁니다. PX 감사와 동일한 패턴이지만 `#elif` 대신 `#if`를 사용하고 `__linux`는 없습니다.

---

### FR-04: pz_memory_proc.c의 전처리기 `defined()` (1/3)

**파일**: `st01/src/PZ/pz_memory_proc.c`
**라인**: 199

**수정 전**:
```c
#if defined __hpux || sun || _AIX
```

**수정 후**:
```c
#if defined(__hpux) || defined(sun) || defined(_AIX)
```

---

### FR-05: pz_memory_proc.c의 전처리기 `defined()` (2/3)

**파일**: `st01/src/PZ/pz_memory_proc.c`
**라인**: 225

FR-04와 동일한 수정.

---

### FR-06: pz_memory_proc.c의 전처리기 `defined()` (3/3)

**파일**: `st01/src/PZ/pz_memory_proc.c`
**라인**: 280

FR-04와 동일한 수정.

---

### FR-07: pz_memory_conf.c의 전처리기 `defined()`

**파일**: `st01/src/PZ/pz_memory_conf.c`
**라인**: 1468

**수정 전**:
```c
#if defined __hpux || sun || _AIX
```

**수정 후**:
```c
#if defined(__hpux) || defined(sun) || defined(_AIX)
```

---

## 배치 2: HIGH 수정 (FR-08부터 FR-13)

### FR-08: pz_procchk.c에서 `continue` 후 데드 코드 `NR = 0`

**파일**: `st01/src/PZ/pz_procchk.c`
**라인**: 238-242

**수정 전**:
```c
		if ((plp = Search_Node (Data[i].pname[k])) != Tail)
		{
			continue;
			NR = 0;
		}
```

**수정 후**:
```c
		if ((plp = Search_Node (Data[i].pname[k])) != Tail)
		{
			NR = 0;
			continue;
		}
```

**근거**: `continue` 후 `NR = 0`은 데드 코드입니다 — 절대 실행되지 않습니다. NR은 "실행 안 함" 사이클을 추적합니다; 프로세스가 발견되었을 때 초기화되지 않으면 오래된 카운트가 누적됩니다. 프로세스가 발견되지 않은 다음 사이클에서 `NR > 0`은 프로세스가 최근에 살아있는 것으로 확인되었음에도 불구하고 `ps -ef` 확인을 트리거합니다. `NR = 0`을 `continue` 전으로 이동하면 연결 리스트에서 프로세스가 발견될 때 카운터가 초기화됩니다.

---

### FR-09: pz_procchk.c의 파일 디스크립터 누수 `fp_b`

**파일**: `st01/src/PZ/pz_procchk.c`
**라인**: 258-266

**수정 전**:
```c
                    if (NR > 0)
                    {
                        /* add */
                        rt = 0;
                        sprintf (path, "ps -ef|grep %s|egrep -v 'grep|ps|sh|ls|vi|view|tail|ls'|wc -l > /tmp/mrt1", Data[i].pname[k]);
                        system(path);
                        if ((fp_b = fopen ("/tmp/mrt1", "r")) == NULL)
                        {
                            Log (USR_ERROR, "cannot open:[/tmp/mrt1]");
                        }
                        else
                        {
                            if (fgets (buf, sizeof (buf), fp_b) != NULL)
                                rt = atoi (buf);
                        }
                        /* add */
```

**수정 후**:
```c
                    if (NR > 0)
                    {
                        /* add */
                        rt = 0;
                        sprintf (path, "ps -ef|grep %s|egrep -v 'grep|ps|sh|ls|vi|view|tail|ls'|wc -l > /tmp/mrt1", Data[i].pname[k]);
                        system(path);
                        if ((fp_b = fopen ("/tmp/mrt1", "r")) == NULL)
                        {
                            Log (USR_ERROR, "cannot open:[/tmp/mrt1]");
                        }
                        else
                        {
                            if (fgets (buf, sizeof (buf), fp_b) != NULL)
                                rt = atoi (buf);
                            fclose (fp_b);
                        }
                        /* add */
```

**근거**: `fp_b`는 모니터링 루프의 내부 분기에서 열리지만 닫히지 않습니다. 시간이 지남에 따라 (60초마다 실행) 파일 디스크립터가 누적되어 프로세스가 FD 제한에 도달하고 더 이상 파일을 열 수 없습니다. `fgets` 후 `fclose(fp_b)` 추가하세요.

---

### FR-10: pz_procchk.c의 `system()` 초기화되지 않은 `cmd`

**파일**: `st01/src/PZ/pz_procchk.c`
**라인**: 970-996

**수정 전**:
```c
	if (flag == 1)
	{
		...
		sprintf (cmd, ...);
	}
	else if (flag == 2)
	{
		...
		sprintf (cmd, ...);
	}
	else if (flag == 3)
	{
		...
		sprintf (cmd, ...);
	}

	rt = system (cmd);
```

**수정 후** (system 호출 이전에 else 절 추가):
```c
	else if (flag == 3)
	{
		...
		sprintf (cmd, ...);
	}
	else
	{
		Log (USR_ERROR, "Report_Save:invalid flag[%d]", flag);
		return;
	}

	rt = system (cmd);
```

**근거**: `flag`가 1, 2, 또는 3이 아니면 `cmd` (891라인에서 선언되었지만 사용하지 않는 변수의 일부 — FR-17 참고 참조)가 초기화되지 않고 `system(cmd)`는 쓰레기를 실행합니다. 실제로 `flag`는 항상 1-3이지만 UB 경로는 보호되어야 합니다. 오류 로그와 조기 반환이 있는 `else` 분기를 추가하세요.

**참고**: 891라인의 변수 `cmd`는 `Stop_Process()`에서 선언됩니다 (FR-17이 그곳에서 사용되지 않는 것으로 표시). 996라인에서 사용되는 `cmd`는 `Report_Save()` 함수 범위의 **다른** `cmd`입니다 — 구현 중 확인하세요.

---

### FR-11: pz_fepp.c의 체크되지 않은 `write()` (1/2)

**파일**: `st01/src/PZ/pz_fepp.c`
**라인**: 405

**수정 전**:
```c
		write (FIFO_fd, "1", 1);
```

**수정 후**:
```c
		if (write (FIFO_fd, "1", 1) != 1)
			Log (FIF_ERROR, "write to daemon exit FIFO failed {%d:%s}", SYS_NO, SYS_STR);
```

---

### FR-12: pz_fepp.c의 체크되지 않은 `write()` (2/2)

**파일**: `st01/src/PZ/pz_fepp.c`
**라인**: 784

**수정 전**:
```c
			write (FIFO_fd, "1", 1);
```

**수정 후**:
```c
			if (write (FIFO_fd, "1", 1) != 1)
				Log (FIF_ERROR, "write to process exit FIFO failed {%d:%s}", SYS_NO, SYS_STR);
```

---

### FR-13: pz_daemon_proc.c의 체크되지 않은 `write()`

**파일**: `st01/src/PZ/pz_daemon_proc.c`
**라인**: 979

**수정 전**:
```c
	if (p_signo == SIGUSR1)
		write (DTART_FD, "1", 1);
```

**수정 후**:
```c
	if (p_signo == SIGUSR1)
	{
		if (write (DTART_FD, "1", 1) != 1)
			Log (FIF_ERROR, "write to DTART_FD failed {%d:%s}", SYS_NO, SYS_STR);
	}
```

**참고**: 이것은 `Sig_Handler()` 내부에 있습니다. `Log` 호출은 비동기 신호 안전이 아닐 수 있지만 이 핸들러는 이미 초기 메시지에 대해 `SIG_WRITE_MSG`를 사용하고 신호 핸들러 안전 감사는 중요한 경로를 다루었습니다. 여기서 쓰기 확인은 최선의 노력 개선입니다.

---

## 배치 3: MEDIUM 수정 (FR-14부터 FR-23)

### FR-14: pz_procchk.c의 `Initialize_Link()`의 댕글링 Head 포인터

**파일**: `st01/src/PZ/pz_procchk.c`
**라인**: 788-795

**수정 전**:
```c
	Tail = (PROCESS_T *)malloc (sizeof (PROCESS_T));
	Head->next = Tail;

	if (Tail == NULL)
	{
		Log (SYS_ERROR, "memory allocation failed (Initialize_Link)");
		free(Head);  // SPARROW
	}
```

**수정 후**:
```c
	Tail = (PROCESS_T *)malloc (sizeof (PROCESS_T));
	Head->next = Tail;

	if (Tail == NULL)
	{
		Log (SYS_ERROR, "memory allocation failed (Initialize_Link)");
		free(Head);  /* SPARROW */
		Head = NULL;
	}
```

**근거**: `free(Head)` 후 전역 `Head` 포인터는 댕글링됩니다. 이후 모든 액세스 (예: `Head->next`를 역참조하는 `Search_Node` 또는 `Ordered_Insert`)는 use-after-free가 됩니다. `Head = NULL`을 설정하면 오류를 감지할 수 있습니다. 또한 SPARROW 주석을 C89로 변환하세요.

---

### FR-15: pz_procchk.c의 모니터링 루프에서 반복되는 `atexit()` 호출

**파일**: `st01/src/PZ/pz_procchk.c`
**라인**: 207

`atexit(Release_Link)` 호출은 `sleep(60)`이 있는 `while(1)` 루프에서 실행되는 `Proc_Check()` 내부에 있습니다. 이것은 `Release_Link`를 ~1440회/일 등록합니다. POSIX는 최소 32개 등록을 보장합니다; 그 이상에서는 동작이 구현 정의입니다. 종료 시 `Release_Link`는 수백 번 호출됩니다 (이중 해제 위험).

**수정**: 메인 루프 이전으로 `atexit()` 호출 이동. 루프 진입점을 식별해야 합니다.

**수정 전** (`Proc_Check` 함수 본문 내, 60초마다 호출):
```c
	/* register Release_Link () to be called at program termination	*/
	atexit (Release_Link);

	/* initialize linked list	*/
	Initialize_Link ();
```

**수정 후**: `atexit`을 일회 초기화로 이동. 단일 등록을 보장하는 정적 플래그 추가:
```c
	{
		static int atexit_registered = 0;
		if (!atexit_registered)
		{
			atexit (Release_Link);
			atexit_registered = 1;
		}
	}

	/* initialize linked list	*/
	Initialize_Link ();
```

---

### FR-16: pz_procchk.c의 `Ordered_Insert()`에 PID로 전달된 `r_uid`

**파일**: `st01/src/PZ/pz_procchk.c`
**라인**: 705

**수정 전**:
```c
			Ordered_Insert (r_uid, r_pname);
```

**수정 후**:
```c
			Ordered_Insert ((pid_t)atoi(dirp->d_name), r_pname);
```

**근거**: `r_uid` (680라인)는 `/proc/PID/status` Uid 필드에서 읽은 UID를 포함합니다. `Ordered_Insert`는 첫 번째 매개변수로 `pid_t pid`를 취하고 `aptr->pid`로 저장합니다. 실제 PID는 `/proc/` 아래의 디렉토리 이름입니다. 변수 이름 `r_uid`는 오해의 소지가 있습니다. `Search_Node` 함수는 exec_name으로만 일치합니다 (pid 아님), 영향은 제한적이지만 `pid` 필드에 잘못된 데이터를 저장하는 것은 잘못되었습니다.

---

### FR-17: pz_daemon_proc.c의 `Stop_Process()`의 사용하지 않는 변수

**파일**: `st01/src/PZ/pz_daemon_proc.c`
**라인**: 889-891

**수정 전**:
```c
	int		pk, k, rt;
	long 	proc_no, proc_x_no;
	char	cmd[128];
```

**수정 후**:
```c
	int		pk, rt;
	long 	proc_no;
```

**근거**: `k`, `proc_x_no`, 및 `cmd`는 선언되었지만 `Stop_Process()`에서 사용되지 않습니다. 이들을 제거하세요. 참고: 자신의 `cmd` 변수가 있는 별도의 `Report_Save()` 함수가 있습니다 — 그것은 사용됩니다 (FR-10).

---

### FR-18: pz_memory_conf.c의 C99 mid-block 선언

**파일**: `st01/src/PZ/pz_memory_conf.c`
**라인**: 368, 412-414, 428-430

세 개의 2025EDIT 블록이 mid-block (C99 스타일)에서 변수를 선언합니다. 함수 최상단으로 선언 이동.

**위치 1** (368라인, `Daemon_Config_Read` 내부):
```c
			// 2025EDIT,START
			char temp_buf01[sizeof(INFO(cnt - 'A').exit_FIFO_name)];
			size_t max_tmp_len01 = sizeof(temp_buf01) - strlen("_exit");
```

**위치 2** (412-414라인):
```c
			// 2025EDIT,START
			char temp_buf02[sizeof(DAEMON(cnt - 'A').exit_FIFO_name)];
			size_t max_tmp_len02 = sizeof(temp_buf02) - strlen("_exit");
```

**위치 3** (428-430라인):
```c
			// 2025EDIT,START
			char temp_buf03[sizeof(DAEMON(cnt - 'A').daemon_FIFO_name)];
			size_t max_tmp_len03 = sizeof(temp_buf03) - strlen("_ctrl");
```

**수정**: 모든 6개 선언 (`temp_buf01-03`, `max_tmp_len01-03`)을 `Daemon_Config_Read()` 함수 최상단으로 이동. 구조체 멤버의 `sizeof()`는 컴파일 시간 상수이고 `strlen("_exit")`는 상수이므로 초기값은 유지할 수 있습니다.

**참고**: `sizeof(INFO(cnt-'A').field)` 표현식은 구조체 타입에 따라 다르며 `cnt`의 런타임 값에는 따르지 않습니다. 이들은 컴파일 시간 상수로 해결됩니다. VLA 문제는 여기에 적용되지 않습니다.

---

### FR-19: `return` instead of `continue` on /proc failures — pz_procchk.c

**File**: `st01/src/PZ/pz_procchk.c`
**Lines**: 632, 647 (also 665, 675 — same pattern)

**Before** (line 632):
```c
			if ((fp = fopen (proc_path, "rt")) == 0)
			{
				return;
			}
```

**After**:
```c
			if ((fp = fopen (proc_path, "rt")) == NULL)
			{
				continue;
			}
```

**Before** (line 647-648):
```c
					fclose (fp);    //SPARROW
					return;
```

**After**:
```c
					fclose (fp);    /* SPARROW */
					continue;
```

**Before** (line 665-666):
```c
					fclose (fp);    //SPARROW
					return;
```

**After**:
```c
					fclose (fp);    /* SPARROW */
					continue;
```

**Before** (line 675-676):
```c
					fclose (fp);    //SPARROW
					return;
```

**After**:
```c
					fclose (fp);    /* SPARROW */
					continue;
```

**근거**: `return`은 전체 `Compose_List_LINUX()` 스캔을 중단합니다. 스캔 중에 시작되거나 종료된 프로세스는 일시적 `/proc/PID/status` 열기 실패를 야기합니다 — 이것은 정상입니다. `continue`를 사용하면 한 프로세스 항목을 건너뛰고 계속 스캔합니다. 또한 fopen에 대해 `== 0`을 `== NULL`로 수정 (FR-23과 정렬)하고 C++ 주석을 C89로 변환 (FR-26과 정렬)합니다.

---

### FR-20: pz_compact.c의 `Unlink_Proc()`의 `memcmp` 길이 불일치

**파일**: `st01/src/PZ/pz_compact.c`
**라인**: 242

**수정 전**:
```c
	if (memcmp (dir->d_name, ProcDate, strlen (dir->d_name)) <= 0)
```

**수정 후**:
```c
	if (strlen(dir->d_name) == 8 && memcmp (dir->d_name, ProcDate, 8) <= 0)
```

**근거**: `ProcDate`는 8자 YYYYMMDD 문자열입니다. `strlen(dir->d_name)`을 비교 길이로 사용하는 것은 위험합니다: 8자보다 짧은 비날짜 디렉토리 이름은 너무 적은 문자로 비교되어 잘못 일치할 수 있고 뒤따르는 `rm -rf`로 삭제될 수 있습니다. 길이 확인을 추가하면 유효한 8자 날짜 디렉토리만 비교됩니다.

---

### FR-21: pz_memory.c의 경계 확인 없는 `_Exe_Name`에 대한 `sprintf`

**파일**: `st01/src/PZ/pz_memory.c`
**라인**: 32

**수정 전**:
```c
	sprintf (_Exe_Name, "%s", argv[0]);
```

**수정 후**:
```c
	snprintf (_Exe_Name, sizeof(_Exe_Name), "%s", argv[0]);
```

**또한 33-34라인에 적용**:

**수정 전**:
```c
	sprintf (_SubSystem_Name, "%-2.2s", argv[0]);
	sprintf (_Process_Name, "%s", argv[0]);
```

**수정 후**:
```c
	snprintf (_SubSystem_Name, sizeof(_SubSystem_Name), "%-2.2s", argv[0]);
	snprintf (_Process_Name, sizeof(_Process_Name), "%s", argv[0]);
```

**근거**: `_Exe_Name`, `_SubSystem_Name`, `_Process_Name`은 고정 크기의 전역 버퍼입니다. `argv[0]`은 이론상 버퍼 크기를 초과할 수 있습니다. `snprintf`는 오버플로우를 방지합니다.

---

### FR-22: pz_memory_shm.c의 C99 mid-block 선언

**파일**: `st01/src/PZ/pz_memory_shm.c`
**라인**: 146-147

**수정 전**:
```c
		/* write SHM version marker */
		{
			int ver = SHM_VERSION;
			int mag = SHM_MAGIC;
			memcpy (&DAEMON(i).process_info[32], &mag, 4);
			memcpy (&DAEMON(i).process_info[36], &ver, 4);
		}
```

**수정 후**: 이것은 실제로 유효한 C89 복합 블록입니다 — `{}` 블록의 최상단에서의 선언은 C89에서 허용됩니다. 변수는 블록의 시작 부분에 있으므로 이것은 **이미 C89 준수**입니다. 변경 필요 없음.

**상태**: **제거됨** — 거짓 양성. C89는 모든 복합 명령문 블록의 시작 부분에서 선언을 허용합니다.

---

### FR-23: pz_memory_conf.c의 `NULL` 대신 `0`과 비교된 `fopen`

**파일**: `st01/src/PZ/pz_memory_conf.c`
**라인**: 61, 658, 931, 1286, 1493, 1592, 1877, 2061, 2271

모든 9개 위치는 동일한 패턴을 따릅니다:

**수정 전**:
```c
	if ((fp = fopen (buf, "r")) == 0)
```

**수정 후**:
```c
	if ((fp = fopen (buf, "r")) == NULL)
```

**또한 pz_procchk.c 627라인**:

**수정 전**:
```c
		if ((fp = fopen (proc_path, "rt")) == 0)
```

**수정 후** (이미 FR-19로 다룸):
```c
		if ((fp = fopen (proc_path, "rt")) == NULL)
```

**근거**: `0`과 `NULL`은 포인터 컨텍스트에서 동등하지만 `NULL`을 명시적으로 사용하면 포인터 비교 의도를 전달합니다. 합계: 10개 위치 (pz_memory_conf.c 9개 + pz_procchk.c 1개 FR-19로 다룸).

---

## 배치 4: LOW 수정 (FR-24부터 FR-27)

### FR-24: pz_memory_shm.c 및 pz_memory_conf.c의 사용하지 않는 변수

**파일**: `st01/src/PZ/pz_memory_shm.c`

**라인 28** (`Sub_SHM_Creat`):
```c
	int		i;
```
변수 `i`는 선언되었지만 사용되지 않습니다. **제거**.

**라인 57** (`Mem_SHM_Creat`):
```c
	int		i, j;
```
변수 `j`는 선언되었지만 사용되지 않습니다. `int i;`로 변경하세요.

**파일**: `st01/src/PZ/pz_memory_conf.c`

**라인 640** (`File_Config_Read`):
```c
	int 		fd, rt, cnt = 0, i, j, itemcnt, len, pt, idx_flag, sub_flag;
```
사용되지 않는 변수를 확인하고 제거하세요.

**라인 1864** (`Tcp2_Config_Read`):
```c
	int 	i, cnt = 0, itemcnt = 0, sub_flag;
```
사용되지 않는 변수를 확인하고 제거하세요.

**참고**: pz_memory_conf.c 변수의 경우 구현 중 사용을 확인하세요 — 이 긴 함수들은 본문 깊은 곳에서 변수를 사용할 수 있습니다.

---

### FR-25: pz_daemon_proc.c의 K&R 빈 괄호 → `(void)`

**파일**: `st01/src/PZ/pz_daemon_proc.c`

**라인 467**:
```c
void    SHM_Load_Process ()
```
**수정 후**:
```c
void    SHM_Load_Process (void)
```

**라인 527**:
```c
void    SHM_Backup_Process ()
```
**수정 후**:
```c
void    SHM_Backup_Process (void)
```

**근거**: C89에서 `()`는 "지정되지 않은 매개변수" (모든 인자 수락)를 의미하고 `(void)`는 명시적으로 "매개변수 없음"을 의미합니다. `(void)` 형식은 인자가 없는 함수에 대해 올바릅니다.

---

### FR-26: C++ `//` 주석 → C89 `/* */`

활성 PZ 소스 파일의 모든 C++ 스타일 주석을 C89 `/* */` 주석으로 변환합니다.

**파일별 범위**:

| 파일 | 개수 | 주석 유형 |
|------|-------|---------------|
| pz_memory_conf.c | 41 | `// 2025EDIT,START`, `// 2025EDIT,END`, `// 2025EDIT`, 데이터 주석 |
| pz_daemon_proc.c | 9 | `// KSW` 디버그 마커, `// 2025EDIT` |
| pz_procchk.c | 6 | `// SPARROW` 정적 분석 마커 |
| pz_memory_shm.c | 3 | `// N,NNN` 숫자 주석 |
| pz_fepp.c | 2 | `// Added 20210113`, `// pid <= 0` |
| **합계** | **61** | |

**변환 패턴**:
```c
/* 수정 전 */
// 2025EDIT,START
// KSW
// SPARROW

/* 수정 후 */
/* 2025EDIT,START */
/* KSW */
/* SPARROW */
```

**다중 토큰 주석**:
```c
/* 수정 전 */
// pid <= 0 Added 20210113

/* 수정 후 */
/* pid <= 0 Added 20210113 */
```

---

### FR-27: pz_procchk.c의 하드코딩된 `/tmp/mrt1` 임시 파일

**파일**: `st01/src/PZ/pz_procchk.c`
**라인**: 256 (라인 258에서도 참조)

**수정 전**:
```c
	sprintf (path, "ps -ef|grep %s|egrep -v 'grep|ps|sh|ls|vi|view|tail|ls'|wc -l > /tmp/mrt1", Data[i].pname[k]);
	system(path);
	if ((fp_b = fopen ("/tmp/mrt1", "r")) == NULL)
```

**수정 후**: PID를 기반으로 하는 프로세스별 임시 파일 사용:
```c
	sprintf (path, "ps -ef|grep %s|egrep -v 'grep|ps|sh|ls|vi|view|tail|ls'|wc -l > /tmp/mrt1_%d", Data[i].pname[k], getpid());
	system(path);
	if ((fp_b = fopen ("/tmp/mrt1_%d" ... )) == NULL)
```

**구현 세부사항**: 임시 경로 변수 생성:
```c
	char tmp_file[64];
	sprintf (tmp_file, "/tmp/mrt1_%d", (int)getpid());
	sprintf (path, "ps -ef|grep %s|egrep -v 'grep|ps|sh|ls|vi|view|tail|ls'|wc -l > %s", Data[i].pname[k], tmp_file);
	system(path);
	if ((fp_b = fopen (tmp_file, "r")) == NULL)
```

**참고**: `tmp_file` 선언은 C89를 위해 함수/블록 최상단에 있어야 합니다. 기존 선언에 추가하세요.

---

## 의도적으로 연기됨

| ID | 이슈 | 파일 | 이유 |
|----|-------|------|--------|
| D-1 | readdir/SHM 데이터로 `system()`을 통한 셸 주입 | pz_procchk.c:256, pz_compact.c:245 | `fork()/exec()` 리팩터 필요 — 이 감사에는 범위가 너무 큼. 내부 데이터 소스만. |
| D-2 | 2025EDIT 블록의 NUL 보장 없는 `strncpy` | pz_memory_conf.c 복수 | 최근 추가 코드. 대부분 위치에 이미 수동 NUL 종료가 있음. 엣지 케이스는 비즈니스 로직 검토 필요. |
| D-3 | `Ordered_Insert`의 NULL 확인 후 `free(aptr)` | pz_procchk.c:822 | `free(NULL)`은 no-op입니다. SPARROW 추가 코드 — 무해, 건드릴 가치 없음. |

---

## 구현 순서

```
배치 1 (CRITICAL):  FR-01 → FR-02 → FR-03..FR-07
배치 2 (HIGH):      FR-08 → FR-09 → FR-10 → FR-11..FR-13
배치 3 (MEDIUM):    FR-14 → FR-15 → FR-16 → FR-17 → FR-18 → FR-19 → FR-20 → FR-21 → FR-23
배치 4 (LOW):       FR-24 → FR-25 → FR-26 → FR-27
```

**참고**: FR-22 제거됨 (거짓 양성 — 이미 C89 준수). 유효한 개수: **26개 발견사항**.

## 검증

구현 후 실행:
```bash
# 남은 전처리기 버그 확인
grep -rn '#if defined __hpux || sun || _AIX' st01/src/PZ/*.c

# fopen == 0이 없는지 확인
grep -rn 'fopen.*== 0)' st01/src/PZ/*.c

# 활성 파일에서 C++ 주석이 없는지 확인
grep -rn '// ' st01/src/PZ/*.c | grep -v BACKUP

# 형식 문자열 수정 확인
grep -n '\[%\]' st01/src/PZ/*.c

# strlen을 %s로 수정했는지 확인
grep -n 'strlen.*%s' st01/src/PZ/pz_memory_conf.c
```

---

## 버전 이력

| 버전 | 날짜 | 변경 사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-27 | 26개 실행 가능한 FR이 있는 초기 설계 (거짓 양성으로 FR-22 제거) | Claude Code |
