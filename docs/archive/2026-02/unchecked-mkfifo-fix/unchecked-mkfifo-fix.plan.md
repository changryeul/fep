# 계획: unchecked-mkfifo-fix

## 개요

FEP 초기화 코드 전체에서 확인되지 않은 모든 `mknod()` FIFO 생성 및 `mkdir()` 디렉토리 생성 호출에 오류 확인을 추가합니다. 현재 19개 `mknod()+chmod()` 쌍과 10개 `mkdir()+chmod()` 쌍은 실패를 자동으로 무시하여, 프로세스가 존재하지 않는 FIFO에서 차단되거나 데이터 파일이 잘못된 위치에 작성되는 등 진단하기 어려운 런타임 이슈를 유발할 수 있습니다.

## 배경

데몬 시작 중 `config_db.c` 및 `pz_memory_conf.c`/`pz_memory_proc.c`는 IPC 및 데이터 저장소에 필요한 FIFO (명명된 파이프) 및 디렉토리를 생성합니다. 현재 패턴:

```c
mknod (file_name, S_IFIFO | 0777, 0);
chmod (file_name, 0777);
```

두 호출의 반환 값도 확인하지 않습니다. `mknod()`가 실패하면 (권한, 파일시스템 가득참, 부모 디렉토리 누락), FIFO가 생성되지 않지만 오류는 기록되지 않습니다. 이후 FIFO를 열려고 시도하는 프로세스는 무한정 차단되거나 실패합니다.

마찬가지로 디렉토리의 경우:
```c
mkdir (dir_name, 0777);
chmod (dir_name, 0777);
```

### POSIX 참고

- `mknod()`는 FIFO 생성을 위해 POSIX.1-2008에서 구식으로 표시됨. `mkfifo()`는 선호되는 대안이며 모든 대상 플랫폼 (HP-UX, SunOS, AIX, Linux)에서 사용 가능.
- `mkfifo()` 및 `mkdir()` 모두 실패 시 -1을 반환하고 `errno` 설정. `EEXIST`는 예상되고 허용해야 함 (데몬 재시작 시나리오).

## 범위

### 범위 내

1. **2개 파일에서 19개 `mknod()+chmod()` FIFO 쌍**:
   - `sub/config_db.c` (10개 쌍: 줄 557-573, 611-622, 965-966, 1258-1259)
   - `src/PZ/pz_memory_conf.c` (9개 쌍: 줄 365-401, 414-449, 775-776, 1083-1084)

2. **3개 파일에서 10개 `mkdir()+chmod()` 디렉토리 쌍**:
   - `sub/config_db.c` (2개 쌍: 줄 867-868, 1709-1710)
   - `src/PZ/pz_memory_conf.c` (3개 쌍: 줄 693-694, 1465-1467, 2332-2333)
   - `src/PZ/pz_memory_proc.c` (5개 쌍: 줄 121-122, 132-133, 218-219, 252-253, 310-311)

3. **헬퍼 함수**: sub/ 라이브러리에 `Create_FIFO()` 및 `Create_Dir()` 공유 헬퍼 추가

4. **헤더 업데이트**: `inc/fep_sub.h`에 외부 선언 추가

### 범위 외

- 기존 파일의 독립 `chmod()`/`fchmod()` 호출 (log_proc.c, px_cfgback.c) — 더 낮은 위험, 별도 기능
- `sprintf()` → `snprintf()` 변환 (별도 기능)
- 빌드 시스템 변경 (새 .c 파일이 Make_Lib_P_c.sh에서 자동 감지됨)

## 위험 평가

- **위험**: LOW — 가산식 변경만 (오류 로깅), 성공 시 동작 변화 없음
- **테스트**: 데몬 시작, 설정 로딩, FIFO 및 디렉토리 생성 확인
- **롤백**: 단순 되돌리기
- **호환성**: `mkfifo()`는 POSIX.1, 모든 대상 플랫폼에서 사용 가능

## 예상 영향

- 3개 소스 파일 수정 (config_db.c, pz_memory_conf.c, pz_memory_proc.c)
- 1개 헤더 업데이트 (fep_sub.h)
- 1개 새 공유 소스 파일 (sub/file_util.c)로 2개 헬퍼 함수
- 29개 확인되지 않은 시스템 호출 → 헬퍼를 통한 29개 오류 확인 호출
- 순 변화: ~+20 줄 (헬퍼 정의), ~-10 줄 (대체된 자세한 패턴)
- `mknod()` → `mkfifo()` 현대화 (POSIX 준수)
