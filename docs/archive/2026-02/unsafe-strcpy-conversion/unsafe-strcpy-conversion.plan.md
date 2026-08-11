# 계획: unsafe-strcpy-conversion

## 개요

FEP 활성 코드베이스 전체에서 모든 `strcpy()` 호출을 명시적 버퍼 크기 경계를 가진 `strncpy()`로 변환합니다. 원본 문자열이 대상 버퍼 크기를 초과할 수 있는 경우 버퍼 오버플로우 취약성을 방지합니다.

## 배경

`strcpy()`는 경계 확인을 수행하지 않습니다 — 원본 문자열이 대상 버퍼를 초과하면, 인접 메모리를 자동으로 덮어씁니다. 대부분 현재 사용은 안전하게 보이지만 (원본이 알려진 경계 데이터에서 옴), `strncpy()` 사용은 향후 변경 또는 예기치 않은 입력에 대한 심층 방어를 제공합니다.

FEP 코드베이스는 **11개 파일에서 64개 활성 `strcpy()` 호출**이 있습니다. `strcat()` 호출은 없습니다 (이미 정리됨).

참고: `sprintf()` 안전 (2000+ 호출)은 규모가 크기 때문에 의도적으로 이 기능 범위에서 제외됩니다.

## 범위

### 패턴 그룹

#### 그룹 A: readlink 경로 복사 (2개 파일에서 10개 호출)
`readlink()` 후 `strcpy(path, full_path)` — 경로 버퍼는 char[256] 또는 유사.
- `pz_daemon_proc.c` — 5개 호출 (줄 330, 398, 468, 540, 612)
- `pz_fepp.c` — 5개 호출 (줄 317, 485, 614, 777, 938)

#### 그룹 B: 설정 문자열 파싱 (3개 파일에서 25개 호출)
`if (sp2 == NULL) strcpy(tmp2, sp1+1)` — tmp2 버퍼 다양.
- `px_cfgback.c` — 8개 호출
- `px_cfgload.c` — 8개 호출
- `px_memok.c` — 9개 호출

#### 그룹 C: 환경 문자열 복사 (1개 파일에서 3개 호출)
`strcpy(env_str, "REAL1"/"REAL2"/"TEST")` — 고정 알려진 길이 문자열.
- `px_cfgload.c` — 3개 호출 (줄 127, 129, 131)

#### 그룹 D: FIFO 이름 할당 (2개 파일에서 12개 호출)
`strcpy(INFO(...).exit_FIFO_name, temp_buf)` — struct 필드 복사.
- `pz_memory_conf.c` — 6개 호출
- `sub/config_db.c` — 6개 호출

#### 그룹 E: 파일/데이터 이름 복사 (2개 파일에서 6개 호출)
`strcpy(old_file, IFN(...))` — 매크로 소싱 이름.
- `px_setfname.c` — 3개 호출
- `px_setdname.c` — 3개 호출

#### 그룹 F: 프로세스/경로 이름 복사 (1개 파일에서 3개 호출)
`strcpy(aptr->exec_name, ...)`, `strcpy(Dname[0], ...)` — struct 및 배열 복사.
- `pz_procchk.c` — 3개 호출

#### 그룹 G: 네트워크 인터페이스 이름 (1개 파일에서 5개 호출)
`strcpy(IpAddr, "10.x.x.x")`, `strcpy(ifnm, ifa->ifa_name)` — IP 및 인터페이스 이름.
- `pb_7100_ur.c` — 5개 호출 (줄 90, 95, 510, 517, 523)

### 변환 패턴

```c
/* 이전 */
strcpy(dest, src);

/* 이후 */
strncpy(dest, src, sizeof(dest) - 1);
dest[sizeof(dest) - 1] = '\0';
```

`sizeof(dest)`가 작동하지 않는 struct 필드 (포인터 컨텍스트)의 경우, 알려진 필드 크기를 직접 또는 `sizeof(struct.field)`로 사용합니다.

### 합계: 11개 파일에서 64개 호출

| 그룹 | 파일 | 호출 | 패턴 |
|------|------|------|---------|
| A | 2 | 10 | readlink 경로 |
| B | 3 | 25 | 설정 파싱 |
| C | 1 | 3 | env 문자열 |
| D | 2 | 12 | FIFO 이름 |
| E | 2 | 6 | 파일/데이터 이름 |
| F | 1 | 3 | 프로세스 이름 |
| G | 1 | 5 | 네트워크 인터페이스 |
| **총합** | **11** | **64** | |

## 제외

- `sprintf()` 호출 (2000+): 별도 기능, 너무 큼
- `BACK2025/`, `BACKUP/`, `JC_OLD/`, `.back`, `.org` 파일
- 이전 확인으로 이미 경계가 있는 `sub/config_db.c` strcpy 호출

## 위험 평가

- **위험**: LOW — 명시적 경계를 사용한 기계적 치환
- **strncpy 보장**: 명시적 `dest[size-1] = '\0'` 추가로 항상 null 종료
- **컴파일**: `strncpy`는 표준 C89, 모든 대상 플랫폼에서 사용 가능
- **테스트**: 빌드 검증으로 충분
- **롤백**: 단순 되돌리기

## 예상 영향

- 11개 파일의 64개 strcpy -> strncpy 변환
- 각 변환은 ~1줄 (null 종료자) 추가
- 순 변화: ~+64 줄
- 함수 변화 없음 (심층 방어만)
