# 아카이브 인덱스 - 2026-02

## 아카이브된 기능 목록

| # | 기능 | 일치율 | 소요기간 | 아카이브일 |
|---|------|--------|----------|-----------|
| 1 | config-db-migration | 90% | 2026-02-18 (7시간) | 2026-02-18 |
| 2 | tr-struct-refactor | 92% | 2026-02-18 (9시간) | 2026-02-18 |
| 3 | ini-config-analysis | 97.5% | 2026-02-18~19 | 2026-02-19 |
| 4 | shm-struct-refactor | 95% | 2026-02-21 (4시간) | 2026-02-21 |
| 5 | fifo-struct-refactor | 97% | 2026-02-21 (2시간) | 2026-02-21 |
| 6 | pa-pb-dedup | 90% | 2026-02-21 (3시간) | 2026-02-21 |
| 7 | pa-pb-dedup-phase2 | 97% | 2026-02-21 (2.5시간) | 2026-02-21 |
| 8 | pa-pb-dedup-phase3 | 100% | 2026-02-21 (2시간) | 2026-02-22 |
| 9 | fifo-event-rtn-adoption | 100% | 2026-02-22 (1시간) | 2026-02-22 |
| 10 | poll-buffer-overflow-fix | 100% | 2026-02-22 (20분) | 2026-02-22 |
| 11 | poll-oob-write-fix | 100% | 2026-02-22 (20분) | 2026-02-22 |
| 12 | socket-linger-extraction | 100% | 2026-02-22 (35분) | 2026-02-22 |
| 13 | dead-code-cleanup | 98% | 2026-02-22 (1.5시간) | 2026-02-22 |
| 14 | pa-pb-8100-cleanup | 100% | 2026-02-22 (50분) | 2026-02-22 |
| 15 | commented-code-cleanup-round2 | 100% | 2026-02-22 | 2026-02-22 |
| 16 | sub-commented-code-final | 100% | 2026-02-22 | 2026-02-22 |
| 17 | src-commented-code-cleanup | 100% | 2026-02-22 | 2026-02-22 |
| 18 | if1-block-cleanup | 100% | 2026-02-22 | 2026-02-22 |
| 19 | sizeof-memset-bugfix | 100% | 2026-02-22 | 2026-02-22 |
| 20 | duplicate-code-extraction | 100% | 2026-02-22 | 2026-02-22 |
| 21 | getenv-null-check | 98% | 2026-02-22 | 2026-02-22 |
| 22 | header-cpp-comment-c89 | 100% | 2026-02-22 | 2026-02-22 |
| 23 | magic-numbers-extraction | 100% | 2026-02-22 | 2026-02-22 |
| 24 | unsafe-strcpy-conversion | 100% | 2026-02-22 | 2026-02-22 |
| 25 | signal-handler-safety | 100% | 2026-02-23 | 2026-02-23 |
| 26 | unchecked-mkfifo-fix | 100% | 2026-02-23~25 | 2026-02-25 |
| 27 | dtoaf-dead-code-delete | 100% | 2026-02-25 | 2026-02-25 |
| 28 | setsockopt-return-check | 100% | 2026-02-25 | 2026-02-25 |
| 29 | inet-addr-to-pton | 100% | 2026-02-25 | 2026-02-25 |
| 30 | select-recv-bounds-check | 100% | 2026-02-25 | 2026-02-25 |
| 31 | PB | 100% | 2026-02-26 | 2026-02-26 |
| 32 | PA | 100% | 2026-02-27 | 2026-02-27 |
| 33 | PX | 100% | 2026-02-27 | 2026-02-27 |
| 34 | PZ | 100% | 2026-02-27 | 2026-02-27 |
| 35 | sub-audit | 100% | 2026-02-27 | 2026-02-27 |
| 36 | stat-save-optimize | 100% | 2026-02-27 | 2026-02-27 |
| 37 | tcp-nodelay | 100% | 2026-02-27 | 2026-02-27 |
| 38 | file-rw-optimize | 100% | 2026-02-28 | 2026-02-28 |
| 39 | atoif-optimize | 100% | 2026-02-28 | 2026-02-28 |
| 40 | poll-traversal | 100% | 2026-02-28 | 2026-02-28 |
| 41 | select-send-optimize | 100% | 2026-02-28 | 2026-02-28 |
| 42 | final-perf-optimize | 100% | 2026-02-28 | 2026-02-28 |

## final-perf-optimize

- **설명**: §4.9 성능 개선 우선순위 잔여 3개 항목 일괄 완료. 순위4(pb_7100_ur.c 시세 수신 경로 memset/strlen/Log/sprintf 제거), 순위6(이미 완료 확인), 순위10(pb_1100_ts.c sleep(1)→poll+FIFO). §4.9 성능 표 10/10 전체 완결.
- **유형**: 성능 최적화
- **파일**: 2개 파일 수정: `src/PB/pb_7100_ur.c` (시세 수신 경로 5개 FR), `src/PB/pb_1100_ts.c` (sleep→poll 1개 FR)
- **주요 변경사항**: FR-01 memset(rbuf,0,2048) 제거→rbuf[rt]='\0'. FR-02 strlen(rbuf)→rt(recvfrom 리턴값). FR-03 Log(USR_OK,"RD") 매건 로그 제거(TCP SD 로그 대체). FR-04 sprintf(TrCode,"%-2.2s")→memcpy+'\0'. FR-05 LK 경로 memset 제거, strlen→상수 15. FR-06 sleep(1)→poll(Poll,1,1000)+Fifo_Event_Rtn(). 순위6 F_R_Proc strlen은 poll-traversal에서 이미 tmp_off 방식으로 최적화 완료.
- **일치율**: 100%, 반복 0회
- **영향**: ~-5줄 순. 시세 처리 20-30% 향상(memset/strlen/Log I/O 제거). FIFO 이벤트 응답 ~1s→<100ms. 성능 최적화 시리즈 7/7 완결 (#41~#48).
- **문서**:
  - [계획](final-perf-optimize/final-perf-optimize.plan.md)
  - [설계](final-perf-optimize/final-perf-optimize.design.md)
  - [분석](final-perf-optimize/final-perf-optimize.analysis.md)
  - [보고서](final-perf-optimize/final-perf-optimize.report.md)

## select-send-optimize

- **설명**: Select_Send()에서 불필요한 select() syscall 제거 — SO_SNDTIMEO 200ms 소켓 타임아웃으로 대체. FEP_Architecture_Analysis.md §4.6 성능 개선 항목. 매 전송마다 2번의 syscall(select+send)을 1번(send)으로 절감. 22개 호출 사이트(17개 파일) 변경 없음.
- **유형**: 성능 최적화
- **파일**: 3개 파일 수정: `sub/tcpip_connect.c` (Connect, Connect2에 SO_SNDTIMEO 추가), `sub/tcpip_accept.c` (Accept에 SO_SNDTIMEO 추가), `sub/select_send.c` (select/FD_ISSET 제거, Sendn 직접 호출)
- **주요 변경사항**: FR-01 Connect() SO_SNDTIMEO 200ms(TCP_NODELAY 패턴 동일). FR-02 Connect2() SO_SNDTIMEO 200ms(space 들여쓰기 보존). FR-03 Accept() accepted fd(rt)에 SO_SNDTIMEO 200ms. FR-04 Select_Send에서 select()/FD_ISSET/fd_set/FD_ZERO 13줄 제거. FR-05 Sendn() 에러 처리 유지(rt<=0, TCP_ERROR, NOTOK). 실패 시 Log(TCP_WARN)만, 연결 중단 없음. (char *) 캐스팅 HP-UX/AIX 호환.
- **일치율**: 100%, 반복 0회
- **영향**: +11줄 순. 전송당 syscall 50% 절감(2→1). 호출자 22곳(17개 파일) 변경 0건. 시세 분배(pb_7100_ur, 초당 수백~수천) 가장 큰 수혜. poll-epoll(Alternative A) 취소 후 대안 B로 선택됨.
- **문서**:
  - [계획](select-send-optimize/select-send-optimize.plan.md)
  - [설계](select-send-optimize/select-send-optimize.design.md)
  - [분석](select-send-optimize/select-send-optimize.analysis.md)
  - [보고서](select-send-optimize/select-send-optimize.report.md)

## poll-traversal

- **설명**: poll 이벤트 이중 순회(dual traversal) → 단일 순회(single traversal) 통합. FEP_Architecture_Analysis.md §4.8 성능 개선 항목. 공유 함수 `detect_poll_event()`에 `first_pollin` 변수 도입으로 O(2n) → O(n) 순회 + 11개 인라인 이중 for 루프를 단일 루프 + 내부 switch로 변환. 동시 이벤트(FIFO+SOCKET) 즉시 처리 가능(기존: 첫 번째 이벤트만 처리, 나머지 다음 poll 사이클 대기).
- **유형**: 성능 최적화
- **파일**: 12개 파일 수정: `sub/poll_event.c` (공유 함수) + `src/PA/pa_1200_tr.c`, `pa_7000_tr.c`, `pa_8200_tr.c`, `pa_3100_ts.c` + `src/PB/pb_1200_tr.c`, `pb_7200_tr.c`, `pb_7100_ts.c`, `pb_8100_ts.c`, `pb_1800_ts.c`, `pb_7800_tr.c` + `src/PW/pw_4000_ts.c`
- **주요 변경사항**: FR-01 detect_poll_event() 이중 루프 → 단일 루프 + first_pollin 변수(반환 의미 동일: -1=소켓 단절, -2=이벤트 없음, >=0=POLLIN 인덱스). FR-02 11개 인라인 이중 루프 → 단일 루프 + 내부 switch(POLLHUP 먼저, POLLIN 후). FR-03 POLLHUP 우선 처리 디스크립터 단위 보장. FR-04 C89 코딩 스타일(탭/스페이스 보존, 변수 스코프 상단 선언). 특수 케이스 보존: pw_4000_ts SLog + Receive_Packet, pa_3100_ts SLog + ANY POLLHUP→return.
- **일치율**: 100%, 반복 0회
- **영향**: ~-60줄 순감. 호출자 7곳(detect_poll_event 사용) 변경 0건. O(2n)→O(n) 디스크립터 순회. 동시 POLLIN 이벤트 모두 처리(기존: 첫 번째만 + break). 고빈도 프로세스(pb_1100_ts 채권 주문, pa_7100_ts 시세, pb_7800_tr 시세) 자동 수혜.
- **문서**:
  - [계획](poll-traversal/poll-traversal.plan.md)
  - [설계](poll-traversal/poll-traversal.design.md)
  - [분석](poll-traversal/poll-traversal.analysis.md)
  - [보고서](poll-traversal/poll-traversal.report.md)

## atoif-optimize

- **설명**: AtoIf/AtoLf/AtoDf 문자열→숫자 변환 함수 내부 루프 최적화 — O(n*10) → O(n). FEP_Architecture_Analysis.md §4.7 성능 개선 항목. 문자당 최대 10회 비교(`for(j=0;j<10;j++) if(c==('0'+j))`)를 1회 뺄셈+범위 검사(`if(c>='0'&&c<='9') val=val*10+(c-'0')`)로 대체.
- **유형**: 성능 최적화
- **파일**: 3개 파일 수정: `sub/atoif.c`, `sub/atolf.c`, `sub/atodf.c`
- **주요 변경사항**: FR-01 AtoIf 내부 for(j) 루프 제거, `char c` 캐시 추가, 직접 산술 적용. FR-02 AtoLf 동일 패턴(long 반환 타입 유지). FR-03 AtoDf 동일 패턴 + 소수점(`.`) 처리 로직 유지 + 제어 흐름 반전(digit-first) + 들여쓰기 정규화(혼합 스페이스→탭). 3개 함수 모두 변수 `j` 제거, `char c` 추가, 음수 부호 처리 유지.
- **일치율**: 100%, 반복 0회
- **영향**: ~-9줄 순감. 호출자 718곳(AtoIf 621 + AtoLf 25 + AtoDf 72) 변경 0건. 함수 5~10배 속도 향상(문자당 비교 10회→1회). 고빈도 호출자: pa_5010_mp.c(171회), config_db.c(35회), pz_memory_conf.c(33회), pa_1290_mp.c(31회).
- **문서**:
  - [계획](atoif-optimize/atoif-optimize.plan.md)
  - [설계](atoif-optimize/atoif-optimize.design.md)
  - [분석](atoif-optimize/atoif-optimize.analysis.md)
  - [보고서](atoif-optimize/atoif-optimize.report.md)

## file-rw-optimize

- **설명**: F_R/F_W 파일 I/O 잠금 범위 최적화 — 전체 파일 잠금(`l_start=0, l_len=0`)을 레코드/append 단위 잠금으로 변경. FEP_Architecture_Analysis.md §4.3 성능 개선 우선순위 3위 항목. 7개 함수(F_R, F_R2, F_R3, F_W, F_W2, F_W3, F_WB) 수정으로 다중 프로세스(PA 15개 + PB 9개) 동시 파일 접근 시 직렬화 대기 해소.
- **유형**: 성능 최적화
- **파일**: 1개 파일 수정: `sub/file_rw.c`
- **주요 변경사항**: FR-01~03 F_R 계열 `F_WRLCK` 전체 파일 → `F_RDLCK` 레코드 범위 잠금(`l_start=offset, l_len=rec_size*p_cnt`), offset 계산을 lock 전으로 이동. FR-02/05~07 F_W 계열 `l_whence=0`(SEEK_SET 전체 파일) → `SEEK_END`(append 영역만 잠금), unlock 시 `SEEK_SET, 0, 0` 리셋으로 정확한 해제 보장. FR-08 잠금 실패 에러 처리 완전 유지. FR-09 EINTR 재시도 로직 완전 유지. SF_W는 스코프 외(시세 연속 append).
- **일치율**: 100%, 반복 0회
- **영향**: ~35줄 변경. 호출자 74곳(F_R 31 + F_W 43) 변경 0건. 동시 다른 레코드 읽기 가능(기존 직렬화→병렬 실행). 읽기와 append 동시 수행 가능(범위 분리). F_R2/F_W2/F_R3/F_W3/F_WB는 src/에서 미사용이나 정합성을 위해 동일 패턴 적용.
- **문서**:
  - [계획](file-rw-optimize/file-rw-optimize.plan.md)
  - [설계](file-rw-optimize/file-rw-optimize.design.md)
  - [분석](file-rw-optimize/file-rw-optimize.analysis.md)
  - [보고서](file-rw-optimize/file-rw-optimize.report.md)

## tcp-nodelay

- **설명**: TCP 소켓 TCP_NODELAY 설정 — Connect/Accept 내부에서 Nagle 비활성화. FEP_Architecture_Analysis.md §4.1 성능 개선 우선순위 1위 항목. 모든 TCP 연결(15개 Socket() 호출 위치)에서 소규모 패킷(~300B 주문)의 0-200ms Nagle 버퍼링 지연을 자동 제거.
- **유형**: 성능 최적화
- **파일**: 3개 파일 수정: `inc/fep_sub.h`, `sub/tcpip_connect.c`, `sub/tcpip_accept.c`
- **주요 변경사항**: FR-01 Connect() 성공(rt==0) 후 setsockopt(IPPROTO_TCP, TCP_NODELAY). FR-02 Accept() 성공 후 accepted fd(rt)에 TCP_NODELAY 설정(p_sfd가 아닌 rt 대상). FR-03 fep_sub.h에 `<netinet/tcp.h>` include 추가. FR-04 Connect2() 동일 적용(미사용이지만 정합성). FR-05 setsockopt 실패 시 Log(TCP_WARN) 경고만, 연결 중단하지 않음. (char *)&flag 캐스팅으로 HP-UX/AIX 호환.
- **일치율**: 100%, 반복 0회
- **영향**: +23줄. 호출자 15곳 변경 0건. 주문 전송(ts) 0-200ms 지연 제거. 시세 전송 0-40ms 지연 제거. 클라이언트 통신 양방향 즉시 전송.
- **문서**:
  - [계획](tcp-nodelay/tcp-nodelay.plan.md)
  - [설계](tcp-nodelay/tcp-nodelay.design.md)
  - [분석](tcp-nodelay/tcp-nodelay.analysis.md)
  - [보고서](tcp-nodelay/tcp-nodelay.report.md)

## stat-save-optimize

- **설명**: Stat_Save() 함수의 파일 I/O 빈도를 시간 기반 스로틀링으로 95% 이상 감소. 매 메인 루프 반복마다 수행되던 fopen/fcntl/fwrite/fclose 사이클을 3초 간격으로 제한. FEP_Architecture_Analysis.md §4.2 성능 개선 항목 (#2 우선순위).
- **유형**: 성능 최적화
- **파일**: 3개 파일 수정: `inc/fep_sub.h`, `sub/stat_save.c`, `sub/setsigfatal.c`
- **주요 변경사항**: FR-01 시간 기반 스로틀(tv.tv_sec 재사용, 추가 syscall 없음). FR-02 영업시간 판정 매 호출 유지(스로틀 대상 아님). FR-03 STAT_SAVE_INTERVAL_SEC 매크로(기본 3, #ifndef 가드, 빌드 시 오버라이드 가능). FR-04 Stat_Save_Force() — _last_save_time 리셋 후 Stat_Save() 호출(프로세스 종료 시 강제 기록). FR-05 file-scope static time_t _last_save_time.
- **일치율**: 100%, 반복 0회
- **영향**: +17줄. 58개 호출자 변경 0건. 고빈도 프로세스(mp/dd/ur) 파일 I/O 95%+ 감소. 저빈도 프로세스(ts/tr)는 이미 3초 이상 간격이므로 영향 없음.
- **문서**:
  - [계획](stat-save-optimize/stat-save-optimize.plan.md)
  - [설계](stat-save-optimize/stat-save-optimize.design.md)
  - [분석](stat-save-optimize/stat-save-optimize.analysis.md)
  - [보고서](stat-save-optimize/stat-save-optimize.report.md)

## sub-audit

- **설명**: sub/ 공유 라이브러리(libfepP.a) 전체 50개 소스 파일에 대한 종합 코드 품질 감사. 13개 파일에서 17건의 결함을 4단계 심각도 순으로 수정: CRITICAL 3건(gethostbyaddr NULL 포인터 역참조, 시그널 핸들러 내 비안전 strlen, K&R 함수 정의), HIGH 6건(전처리기 `defined` 키워드 누락 3건, strcat 오버플로 2건, 바이너리 데이터에 대한 strlen+memcpy 3건), MEDIUM 5건(C++ 주석 28개를 C89로 변환), LOW 3건(매직 넘버, 미사용 변수, 복사-붙여넣기 주석 오류).
- **유형**: 코드 품질 감사 / 버그 수정 / C89 준수
- **파일**: 13개 파일 수정: `sub/tcpip_accept.c`, `sub/setsigfatal.c`, `sub/queue.c`, `sub/check_exist.c`, `sub/check_proc.c`, `sub/stat_save.c`, `sub/config_db.c`, `sub/file_rw.c`, `sub/shm_rw.c`, `sub/select_recv.c`, `sub/log_proc.c`, `sub/key_search.c`, `sub/make_daemon.c`
- **주요 변경사항**: FR-01 gethostbyaddr NULL 안전 처리 + 통합 분기(`#ifdef sun` 제거). FR-02 시그널 핸들러 내 인라인 while 루프 strlen. FR-03 K&R→ANSI 프로토타입(ReceiveQueue, MakeQueue). FR-04/05/06 전처리기 `defined` 키워드(4개 위치, 3개 파일). FR-07/08 strcat→strncat 경계 검사(6개 위치, 2개 파일). FR-09 strlen+memcpy→정수 오프셋 추적(tmp_off/buf_off, 3개 함수, 2개 파일). FR-10-13 C++ 주석→C89(총 28개, 5개 파일). FR-14 inet_ntoa→inet_ntop. FR-15 매직 넘버 문서화. FR-16 미사용 변수 제거. FR-17 복사-붙여넣기 주석 수정.
- **일치율**: 100%, 반복 0회
- **영향**: ~93줄 변경. CRITICAL 충돌/UB 버그 3건 수정(libfepP.a를 통한 시스템 전체 영향). 전처리기 로직 버그 4건 수정. strcat 오버플로 위험 6건 제거. 바이너리 데이터 손상 벡터 3건 수정(FR-09c shm_rw.c가 가장 치명적 — SHM에 NUL 바이트 빈번). C++ 주석 28개 제거. sub/ 모듈에 C89 위반 사항 없음.
- **문서**:
  - [계획](sub-audit/sub-audit.plan.md)
  - [설계](sub-audit/sub-audit.design.md)
  - [분석](sub-audit/sub-audit.analysis.md)
  - [보고서](sub-audit/sub-audit.report.md)

## PB

- **설명**: PB(채권 KRX FEP) 전체 9개 소스 파일에 대한 종합 코드 품질 감사. 8단계 심각도 순으로 28건의 결함 수정: CRITICAL 3건(누락된 break, 미초기화 datacnt), HIGH 9건(sizeof 버그, 전처리기 defined() 구문, 누락된 default 반환값, 미초기화 변수), MEDIUM 10건(공유 라이브러리 마이그레이션, C89 준수, 0 나누기 방어, 하드코딩된 IP/NIC 제거), LOW 6건(데드 코드 정리). FR-27(C++ 주석)은 의도적으로 보류.
- **유형**: 코드 품질 감사 / 버그 수정 / 중복 코드 마이그레이션
- **파일**: 9개 파일 수정: `src/PB/pb_1100_ts.c`, `src/PB/pb_1200_tr.c`, `src/PB/pb_1800_ts.c`, `src/PB/pb_7100_ts.c`, `src/PB/pb_7100_ur.c`, `src/PB/pb_7200_tr.c`, `src/PB/pb_7800_tr.c`, `src/PB/pb_8100_ts.c`, `src/PB/pb_8200_tr.c`
- **주요 변경사항**: FR-01 RP_POLL case에서 누락된 break. FR-02/03 datacnt=1 초기화. FR-04/05/06 Analyze_Data() default 반환값(3개 파일). FR-07/08/09 전처리기 defined() 구문(11개 지시자). FR-10/11 미초기화 변수 수정. FR-18/18b sizeof(HEAD_SIZE) 버그(4개 위치). FR-14-17 pb_1800_ts.c 공유 라이브러리 마이그레이션(~185줄 제거). FR-19 0 나누기 방어(3개 위치). FR-20/21 하드코딩된 IP/NIC를 설정/동적 탐색으로 대체. FR-22 memcpy→strncpy IFNAMSIZ. FR-23 로그 포맷 수정. FR-24-26/28 데드 코드 정리.
- **일치율**: 100%, 반복 0회
- **영향**: ~-230 순 라인 수. 치명적 제어 흐름/데이터 버그 3건 수정. sizeof 오계산 4건 수정. 전처리기 지시자 11건 수정. pb_1800_ts.c 공유 라이브러리 패턴으로 완전 마이그레이션.
- **문서**:
  - [계획](PB/PB.plan.md)
  - [설계](PB/PB.design.md)
  - [분석](PB/PB.analysis.md)
  - [보고서](PB/PB.report.md)

## PA

- **설명**: PA(매매 로직) 전체 20개 소스 파일에 대한 종합 코드 품질 감사. 7단계 심각도 순으로 16개 활성 FR에 걸쳐 69개 코드 위치 수정: CRITICAL 5건(미초기화 변수, 잘못된 배열 인덱스, 누락된 중괄호, 누락된 default 반환값), HIGH 42건(sizeof(HEAD_SIZE) 버그 6건, 전처리기 defined() 구문 36건), MEDIUM 12건(포맷 문자열 &dat 3건, 0 나누기 방어, 데드 코드 제거, fep_common.h include 누락 8건, 하드코딩된 IP 2건, 잘못된 TR 코드), LOW 3건(미사용 변수 2건, 중복 로그). FR-12(IMECO 플레이스홀더 IP)는 검토 전용으로 보류. FR-17(시그널 안전성)은 별도 기능으로 보류.
- **유형**: 코드 품질 감사 / 버그 수정 / POSIX 준수
- **파일**: 20개 파일 수정: `pa_1100_ts.c`, `pa_1200_mp.c`, `pa_1200_tr.c`, `pa_1290_mp.c`, `pa_1400_mp.c`, `pa_1490_mp.c`, `pa_1600_tr.c`, `pa_2100_ts.c`, `pa_2200_tr.c`, `pa_2700_tr.c`, `pa_3100_ts.c`, `pa_5200_qs.c`, `pa_7000_mp.c`, `pa_7000_tr.c`, `pa_7000_us.c`, `pa_7010_us.c`, `pa_7100_dd.c`, `pa_7100_ur.c`, `pa_7500_us.c`, `pa_7800_tr.c`
- **주요 변경사항**: FR-01 미초기화 rt + 미사용 변수. FR-02 R_Fmt[i]→R_Fmt[for_d] 잘못된 인덱스. FR-03 누락된 } 중괄호(컴파일 차단). FR-04 datacnt=1 초기화. FR-05 Analyze_Data() default 반환값(3개 파일). FR-06 sizeof(HEAD_SIZE)→HEAD_SIZE(6개 위치, 4개 파일). FR-07 전처리기 defined() 구문(36개 위치, 10개 파일 + 복사-붙여넣기 버그 1건 A1491→A1492). FR-08 &dat 포맷 문자열(3개 위치). FR-09 0 나누기 방어. FR-10+16 데드 코드 + 중복 로그 병합. FR-11 fep_common.h 누락(8개 파일). FR-13 하드코딩된 IP 제거/대체. FR-14 TTRODP11303→TTRMOP41303 LP 채권 TR 코드. FR-15 미사용 변수 정리.
- **일치율**: 100%, 분석 후 1회 반복(복사-붙여넣기 버그 pa_1490_mp.c:293)
- **영향**: 치명적 제어 흐름/데이터 버그 5건 수정. sizeof 오계산 6건 수정. 전처리기 지시자 36건 수정(숨겨진 로직 버그 1건 발견). 누락된 헤더 include 8건 추가. 하드코딩된 환경별 IP 2건 제거. 잘못된 TR 코드 1건 수정(업무 확인 필요).
- **문서**:
  - [계획](PA/PA.plan.md)
  - [설계](PA/PA.design.md)
  - [분석](PA/PA.analysis.md)
  - [보고서](PA/PA.report.md)

## PX

- **설명**: PX(유틸리티/모니터링) 전체 46개 소스 파일에 대한 종합 코드 품질 감사. 30개 고유 파일에서 38건의 결함을 4단계 심각도 순으로 수정: CRITICAL 7건(Shm_FinFut[i]→Shm_FinFut[rt] 잘못된 배열 인덱스, f_count/d_count==0일 때 함수 반환 누락, recv_sec/send_sec/start_tm/end_tm/run_flag/reload_flag 미초기화 변수), HIGH 23건(전처리기 `defined()` 구문 — `_AIX`와 `__linux` 미래핑), MEDIUM 5건(continue_flag 지속 버그로 설정 무시, *buf==NULL 타입 불일치, 하드코딩된 호스트명 "ap67"에 podm11/podm12 누락, 빈 main 스텁, 도달 불가 exit(1)), LOW 2건(미사용 fifo_name[128], 3개 파일에서 미사용 sub[4]). FR-33은 FR-07에 통합.
- **유형**: 코드 품질 감사 / 버그 수정 / POSIX 준수
- **파일**: `src/PX/` 내 30개 고유 파일 수정: px_showsise.c, px_setfname.c, px_setdname.c, px_chkgap_auto1.c, px_chkgap_man.c, px_chkgap_auto3.c, px_runstop.c, px_memok.c, px_cfgback.c, px_chkgap.c, px_chktrcnt.c, px_sethandsk.c, px_setudp.c, px_setcseq.c, px_setdate.c, px_setdelay.c, px_setdsize.c, px_setdtime.c, px_setfcnt.c, px_setfsize.c, px_setinfostat.c, px_setlstat.c, px_setnstat.c, px_setpseq.c, px_setpstat.c, px_setptime.c, px_setsisereco.c, px_setsstat.c, px_sett2ip.c, px_sett2seq.c, px_settout.c
- **주요 변경사항**: FR-01 Key_Search() 이후 Shm_FinFut[rt] 잘못된 인덱스. FR-02/03 f_count/d_count==0일 때 Change_File_Name/Change_Dshm_Name 반환+종료 누락. FR-04-06 3개 chkgap 파일에서 미초기화 double 변수. FR-07 미초기화 run_flag/reload_flag(중지 경로에서 reload_flag 실제 버그). FR-08-30 23개 파일에서 전처리기 defined() 구문. FR-31 내부 루프 전 continue_flag 초기화+리셋(지속 플래그 버그). FR-32 *buf==NULL→'\0' 타입 불일치. FR-34 운영 호스트 podm11/podm12 추가. FR-35 빈 main return(0). FR-36 exit(1)→exit(OK). FR-37 미사용 fifo_name. FR-38 3개 파일에서 미사용 sub[4].
- **일치율**: 100%, 반복 0회
- **영향**: ~+27 순 라인 수. 치명적 제어 흐름/데이터 버그 7건 수정. 전처리기 지시자 23건 수정. 설정 무시를 유발하는 지속 플래그 버그 1건. 타입 불일치 1건. 운영 호스트명 누락 1건. 미사용 변수 4건 제거. 경미한 편차 1건: FR-36 제거 대신 exit(OK)(기능적으로 동일).
- **문서**:
  - [계획](PX/PX.plan.md)
  - [설계](PX/PX.design.md)
  - [분석](PX/PX.analysis.md)
  - [보고서](PX/PX.report.md)

## PZ

- **설명**: PZ(시스템 관리) 전체 10개 소스 파일에 대한 종합 코드 품질 감사. 8개 활성 파일에서 26건의 결함을 4단계 심각도 순으로 수정: CRITICAL 7건(포맷 문자열 `[%]` 충돌, `strlen()`을 `%s`로 사용 시 충돌, 전처리기 `defined()` 구문 5건), HIGH 6건(continue 이후 NR=0 데드 코드, fopen 후 fclose 누락으로 fd 누수, system() 내 미초기화 cmd, 미검사 write() 3건), MEDIUM 9건(free 후 댕글링 포인터, atexit 오버플로 ~1440회/일, Ordered_Insert에서 UID/PID 불일치, 미사용 변수 3건, C99 중간 블록 선언 래핑 4건, /proc 스캔에서 return→continue 4건, memcmp 길이 가드, sprintf→snprintf 3건, fopen==0→NULL 10건), LOW 4건(미사용 변수 2건, K&R void 매개변수 2건, C++ 주석 61개→C89, /tmp/mrt1 레이스 컨디션). FR-22는 오탐으로 제거. 2건 보류(D-01: system("rm -rf"), D-02: sprintf 오버플로).
- **유형**: 코드 품질 감사 / 버그 수정 / C89 준수
- **파일**: 8개 파일 수정: `src/PZ/pz_daemon_proc.c`, `src/PZ/pz_memory_conf.c`, `src/PZ/pz_procchk.c`, `src/PZ/pz_memory_proc.c`, `src/PZ/pz_fepp.c`, `src/PZ/pz_compact.c`, `src/PZ/pz_memory.c`, `src/PZ/pz_memory_shm.c`
- **주요 변경사항**: FR-01 `[%]`→`[%s]` 포맷 문자열. FR-02 strlen을 %s로→(int)strlen을 %d로. FR-03/04/05/06/07 전처리기 defined()(5개 위치, 3개 파일). FR-08 continue 전 NR=0. FR-09 fclose(fp_b) 누수. FR-10 미초기화 cmd else+return 가드. FR-11/12/13 미검사 write()(3개 위치). FR-14 free 후 Head=NULL. FR-15 atexit 정적 플래그. FR-16 r_uid→(pid_t)atoi(dirp->d_name). FR-17 미사용 변수 3건 제거. FR-18 C99 선언 래핑(4개 블록). FR-19 /proc 루프에서 return→continue(4개 위치). FR-20 memcmp 전 strlen==8 가드. FR-21 sprintf→snprintf(전역 3건). FR-23 fopen==0→NULL(10개 위치). FR-24 미사용 변수 i,j. FR-25 K&R ()→(void). FR-26 C++ 주석(61→0). FR-27 /tmp/mrt1→PID별 tmp_file.
- **일치율**: 100%, 반복 0회(검증 시 FR-24b 갭 발견 및 수정: 변수 j가 루프에서 사용 중)
- **영향**: CRITICAL 충돌/UB 버그 7건 수정. HIGH 런타임 실패 경로 6건 방어. MEDIUM 미묘한 버그 9건 해결. C++ 주석 61개 제거. fopen 비교 10건 현대화. 전처리기 지시자 5건 수정. 기능 동작 변경 없음.
- **문서**:
  - [계획](PZ/PZ.plan.md)
  - [설계](PZ/PZ.design.md)
  - [분석](PZ/PZ.analysis.md)
  - [보고서](PZ/PZ.report.md)

## select-recv-bounds-check

- **설명**: `select_recv.c`의 TCP 수신 함수 6개에 pkt_len 상한/하한 검증 추가. 기존에는 네트워크에서 파싱된 `pkt_len`이 검증 없이 `Recvn()`에 전달되어 원격 버퍼 오버플로가 가능했음. 4개 함수는 기존 하한 검사가 로그 전용(return 없음)이었음. 7번째 함수(`Sise_Select_Receive`)는 이미 안전.
- **유형**: 보안 / 버퍼 오버플로 수정
- **파일**: 1개 파일 수정: `sub/select_recv.c`
- **주요 변경사항**: 6개 함수에 `Recvn()` 호출 전 경계 검사 가드 추가. 상한은 호출자 버퍼 크기에서 도출: `TCP_BUFF_MAX_LEN`(5120), `KRX_DATA_BUFF_SIZE`(4096), `CLI_BUFF_MAX_LEN`(4096). 모든 가드에 `Log(USR_ERROR)` + `return (NOTOK)` 적용. 로그 함수명 수정 4건. C++ 인라인 주석 1건 제거.
- **일치율**: 100%, 반복 0회
- **영향**: +24줄. 원격 버퍼 오버플로 벡터 6건 제거. 로그 전용 검증 4건을 적절한 거부 경로로 변환. 14개 호출 프로세스 영향 없음(코드 변경 0). 정상 패킷에 대한 기능 변경 없음.
- **문서**:
  - [계획](select-recv-bounds-check/plan.md)
  - [설계](select-recv-bounds-check/design.md)
  - [분석](select-recv-bounds-check/analysis.md)
  - [보고서](select-recv-bounds-check/report.md)

## inet-addr-to-pton

- **설명**: 폐기 예정인 `inet_addr()` 호출 9건 전부를 POSIX `inet_pton()`으로 대체. 2021년 불완전 마이그레이션으로 인한 px_memok.c 버그 5건 수정(잔존 `ul` 3건 + 잘못된 구조체 대상 2건 + SHM 기록). 데드 주석 블록 5건 정리. `ul` 변수 선언 11건 제거.
- **유형**: POSIX 현대화 + 버그 수정 + 데드 코드 정리
- **파일**: 7개 파일 수정: `sub/config_db.c`, `sub/udp_init.c`, `src/PZ/pz_memory_conf.c`, `src/PX/px_memok.c`, `src/PX/px_sett2ip.c`, `src/PX/px_setudp.c`, `src/PA/pa_7100_ur.c` (+ `pa_7000_mp.c` 주석만)
- **주요 변경사항**: 5가지 변환 패턴(A: 직접 기록, B: 임시 변수, C: sockaddr 기록, D: 읽기 전용 비교, E: 데드 주석 제거). 9개 `ul`+`memcpy` 중간자 제거. px_memok.c에 블록 스코프 `struct in_addr tmp_ia`로 SHM 기록 방지. udp_init.c에 누락된 오류 검사 추가. pz_memory_conf.c에서 로그 포맷 가비지(`%c`에 포인터 인수) 정리.
- **일치율**: 100%, 반복 0회
- **영향**: ~-30 순 라인 수. 버그 5건 수정(잔존 ul 3건 + 잘못된 구조체 2건). 누락된 오류 검사 1건 추가. 잠재적 64비트 버퍼 오버라이트 2건 수정(`sizeof(u_long)` = 8을 4바이트 `ip_addr`에 대입). 활성 코드베이스에 `inet_addr` 잔존 없음.
- **문서**:
  - [계획](inet-addr-to-pton/inet-addr-to-pton.plan.md)
  - [설계](inet-addr-to-pton/inet-addr-to-pton.design.md)
  - [분석](inet-addr-to-pton/inet-addr-to-pton.analysis.md)
  - [보고서](inet-addr-to-pton/inet-addr-to-pton.report.md)

## setsockopt-return-check

- **설명**: SO_SNDBUF, SO_RCVBUF, SO_BROADCAST에 대한 미검사 `setsockopt()` 호출 6건에 반환값 검사 추가 — 실패 시 로그 전용(UDP_WARN), 중단 없음.
- **유형**: 오류 처리 / 방어적 프로그래밍
- **파일**: 2개 파일 수정(`sub/udp_init.c`, `src/PA/pa_7000_mp.c`)
- **주요 변경사항**: 6개의 단독 `setsockopt()` 호출을 `rt = setsockopt(...); if (rt < 0) Log(UDP_WARN, "setsockopt SO_xxx fail {%d:%s}", SYS_NO, SYS_STR)`로 변환. 두 파일 모두 기존 `int rt` 재사용. 이 수정 후: 활성 코드베이스의 setsockopt 호출 16/16건 검사 완료 — 미검사 0건.
- **일치율**: 100%, 반복 0회
- **영향**: +6줄(새 로그-검사 블록 6개). 정상 코드 경로에 기능 변경 없음 — 커널이 비핵심 소켓 옵션을 거부할 때만 로그 발생.
- **문서**:
  - [계획](setsockopt-return-check/setsockopt-return-check.plan.md)
  - [설계](setsockopt-return-check/setsockopt-return-check.design.md)
  - [분석](setsockopt-return-check/setsockopt-return-check.analysis.md)
  - [보고서](setsockopt-return-check/setsockopt-return-check.report.md)

## dtoaf-dead-code-delete

- **설명**: 미사용 `sub/dtoaf.c`(31줄) 삭제 — 호출자 없음, extern 없음, 버퍼 오버플로 취약점(동적 포맷 문자열 + buf[20] 오버플로)이 있는 데드 코드 파일.
- **유형**: 데드 코드 삭제 / 보안 정리
- **파일**: 1개 파일 삭제: `sub/dtoaf.c`
- **주요 변경사항**: `DtoAf()` 함수 제거 — double을 고정 너비 ASCII로 변환. 전체 코드베이스에서 호출자 0건 확인. `fep_sub.h`에 extern 없음. 형제 변환 함수(`AtoDf`, `AtoLf`, `AtoIf`, `ItoAf`)는 모두 활발히 사용 중이며 변경 없음.
- **일치율**: 100%, 반복 0회
- **영향**: -31줄. 잠재적 보안 버그 2건 제거(동적 포맷 문자열 취약점, 고정 크기 버퍼 오버플로). 기능 변경 없음 — 데드 코드이므로 호출된 적 없음.
- **문서**:
  - [계획](dtoaf-dead-code-delete/dtoaf-dead-code-delete.plan.md)
  - [설계](dtoaf-dead-code-delete/dtoaf-dead-code-delete.design.md)
  - [분석](dtoaf-dead-code-delete/dtoaf-dead-code-delete.analysis.md)
  - [보고서](dtoaf-dead-code-delete/dtoaf-dead-code-delete.report.md)

## unchecked-mkfifo-fix

- **설명**: 미검사 mknod()+chmod() / mkdir()+chmod() 쌍 30건을 오류 검사가 포함된 Create_FIFO() 및 Create_Dir() 공유 헬퍼로 대체. mknod()를 mkfifo()로 현대화(POSIX.1-2008).
- **유형**: 오류 처리 / POSIX 현대화
- **파일**: 5개 파일(수정 3개 + 신규 1개 + 헤더 1개): sub/file_util.c(신규), inc/fep_sub.h, sub/config_db.c, src/PZ/pz_memory_conf.c, src/PZ/pz_memory_proc.c
- **주요 변경사항**: `Create_FIFO()`(mkfifo + EEXIST 허용 + SYS_FATAL 로깅) 및 `Create_Dir()`(mkdir + 동일) 생성. 3개 소스 파일에서 FIFO 쌍 18개와 디렉토리 쌍 12개 대체. 성공 및 EEXIST 경로 모두에서 chmod(0777) 유지. 계획에서 실제 30쌍 발견(초기 29에서 수정).
- **일치율**: 100%, 반복 0회
- **영향**: 미검사 시스템 호출 30건에 오류 검사 추가. 활성 코드에 mknod() 잔존 없음. 성공 경로에 기능 변경 없음. EEXIST 허용으로 데몬 재시작 시나리오 처리.
- **문서**:
  - [계획](unchecked-mkfifo-fix/unchecked-mkfifo-fix.plan.md)
  - [설계](unchecked-mkfifo-fix/unchecked-mkfifo-fix.design.md)
  - [분석](unchecked-mkfifo-fix/unchecked-mkfifo-fix.analysis.md)
  - [보고서](unchecked-mkfifo-fix/unchecked-mkfifo-fix.report.md)

## signal-handler-safety

- **설명**: POSIX 준수를 위해 12개 파일의 시그널 핸들러 11개에서 비동기-시그널-비안전 함수 호출 수정
- **유형**: 시그널 안전성 / POSIX 준수
- **파일**: 12개 파일 수정(헤더 1개 + sub 1개 + PA 4개 + PW 3개 + PZ 3개)
- **주요 변경사항**: `volatile sig_atomic_t _in_signal_handler` 플래그 + `SIG_WRITE_MSG()` 매크로 추가. 시그널 이름 조회 테이블과 `write(STDERR_FILENO, ...)`로 `End_Routine()` 재작성. 시그널 컨텍스트에서 비안전 연산(`sprintf`, `LtoU`, `Stat_Save`, `Log`, `exit`) 건너뛰도록 `Exit_Process()` 수정. PA/PW `Catch_Signal()` 7개와 PZ `Sig_Handler()` 3개에서 `Log()/SLog()`를 `SIG_WRITE_MSG()`로 대체. 캐치 불가 `signal(SIGKILL, ...)` 제거. 시그널 경로에서 `exit()`를 `_exit()`로 대체.
- **일치율**: 100%, 반복 0회
- **영향**: 시그널 핸들러 11개 모두 POSIX 비동기-시그널-안전 준수. 정상(비시그널) 코드 경로에 기능 변경 없음.
- **문서**:
  - [계획](signal-handler-safety/signal-handler-safety.plan.md)
  - [설계](signal-handler-safety/signal-handler-safety.design.md)
  - [분석](signal-handler-safety/signal-handler-safety.analysis.md)
  - [보고서](signal-handler-safety/signal-handler-safety.report.md)

## unsafe-strcpy-conversion

- **설명**: 모든 `strcpy()`를 명시적 null 종료가 포함된 경계 지정 `strncpy()`로 변환
- **범위**: 11개 파일에서 strcpy 호출 64건 해결(데드 코드 삭제 10건, 변환 53건, 안전한 것 유지 1건)
- **주요 변경사항**: `/* */` 데드 코드 블록 10개 삭제, 활성 strcpy 53건을 sizeof 경계의 strncpy로 변환
- **파일**: PB(1), PX(5), PZ(4), sub(1)
- **유지**: pz_procchk.c:832 strcpy(정확한 strlen+1로 malloc, 구조상 안전)
- **일치율**: 100%, 반복 0회
- **영향**: 기능 변경 없음, 심층 방어 버퍼 오버플로 예방

## magic-numbers-extraction

- **설명**: 원시 매직 넘버(4096, 1228800, 82)를 명명된 상수로 대체
- **정의된 상수**: `KRX_DATA_BUFF_SIZE`(4096), `UDP_SOCK_RCVBUF_SIZE`(1228800) — `inc/fep_fepp.h`
- **범위**: 18개 파일(소스 17 + 헤더 1)에서 29건 대체
- **주요 변경사항**: 26건 `4096`→`KRX_DATA_BUFF_SIZE`, 2건 `1228800`→`UDP_SOCK_RCVBUF_SIZE`, 1건 `82`→`KRX_HEAD_LEN`
- **파일**: PA(7), PB(7), PW(1), PX(2), inc(1)
- **일치율**: 100%, 반복 0회
- **영향**: 기능 변경 없음, 가독성 및 유지보수성 향상

## header-cpp-comment-c89

- **설명**: 활성 헤더 파일의 모든 C++ 스타일 주석(`//`)을 엄격한 ANSI C 준수를 위해 C89 블록 주석(`/* */`)으로 변환.
- **유형**: C89 준수 / 코드 스타일 정리
- **파일**: `st01/inc/`의 헤더 파일 10개. 활성 C++ 주석 ~235개 변환 + fep_sub.h에서 데드 코드 6줄 삭제.
- **주요 변경사항**: strategy01.h(100), strategy03.h(43), krx_mk.h(활성 29, `/* */` 블록 내부 오탐 8), pa_struct.h(25), shm_memory.h(20), fep_file.h(7), fep_sub.h(6 삭제), fep_tcpip.h(3), cli_interface.h(1), strategy.h(1). 기존 `/* text */` 뒤에 `// text`가 오는 경우 주석 병합 적용. 내장 `/* */`가 있는 주석 처리된 코드의 중첩 주석 회피.
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(100%) → 보고서 → 아카이브
- **반복**: 0회(첫 검증 100% 통과)
- **문서**:
  - [계획](header-cpp-comment-c89/header-cpp-comment-c89.plan.md)
  - [설계](header-cpp-comment-c89/header-cpp-comment-c89.design.md)
  - [분석](header-cpp-comment-c89/header-cpp-comment-c89.analysis.md)
  - [보고서](header-cpp-comment-c89/header-cpp-comment-c89.report.md)

## getenv-null-check

- **설명**: 활성 파일의 모든 비안전 `getenv()` 호출에 NULL 안전성 추가. `_FEP_DIV`를 getenvironment.c/fep_sub.h에 8번째 전역 변수로 캐시. `host_name`, `INISAFENET_HOME`, PX 경로 변수 및 나머지 환경 변수에 NULL 가드 추가.
- **유형**: 방어적 프로그래밍 / 버그 수정
- **파일**: 26개 파일(인프라 2 + 호출자 24). 비안전 getenv() 호출 41건 처리.
- **주요 변경사항**: 카테고리 A: _FEP_DIV 전역 캐시, 직접 호출 17건 대체 + "FEP_DIV" 버그 수정 1건(pb_7100_ur.c에서 잘못된 환경 변수명). 카테고리 B: host_name NULL 가드(3개 파일). 카테고리 C: INISAFENET_HOME NULL 가드(3개 파일). 카테고리 D: PX 경로 변수 NULL 가드(6개 파일). 카테고리 E: _FEP_HOME/_FEP_SYSTEM 가드 + _P_BIN을 캐시된 _FEP_BIN으로 대체.
- **범위 수정**: config_loader.c와 config_db.c 제외(이미 NULL 검사 있음). pa_7100_ur.c:175는 주석 블록 내부의 데드 코드로 확인.
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(98%) → 보고서 → 아카이브
- **반복**: 0회(첫 검증 98% 통과)
- **문서**:
  - [계획](getenv-null-check/getenv-null-check.plan.md)
  - [설계](getenv-null-check/getenv-null-check.design.md)
  - [분석](getenv-null-check/getenv-null-check.analysis.md)
  - [보고서](getenv-null-check/getenv-null-check.report.md)

## duplicate-code-extraction

- **설명**: 3가지 중복 코드 패턴(IP 포맷팅, UDP 소켓 초기화, poll 이벤트 감지)을 sub/ 라이브러리의 공유 함수로 추출. 추가 2가지 패턴(TCP 재시도, KRX memcmp)의 일관성 감사.
- **유형**: 함수 추출 + 코드 중복 제거
- **파일**: 16개 파일(새 sub/ 3 + 헤더 수정 1 + 호출자 수정 12), 감사 전용 읽기 12건
- **주요 변경사항**: `format_ip_addr()`(호출자 4개), `init_udp_socket()`(호출자 5개), `detect_poll_event()`(호출자 7개) 생성. 보너스: pa_7030_us.c에서 Svr_IP[3] 버퍼 오버플로 및 미초기화 bufflen/oplen 수정
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(100%) → 보고서 → 아카이브
- **반복**: 0회(첫 검증 100% 통과)
- **순 코드 변경**: ~-154줄(355줄 제거, 공유 함수 201줄 추가)
- **문서**:
  - [계획](duplicate-code-extraction/duplicate-code-extraction.plan.md)
  - [설계](duplicate-code-extraction/duplicate-code-extraction.design.md)
  - [분석](duplicate-code-extraction/duplicate-code-extraction.analysis.md)
  - [보고서](duplicate-code-extraction/duplicate-code-extraction.report.md)

## sub-commented-code-final

- **설명**: sub/ 공유 라이브러리 소스 파일 3개에서 마지막 `/* */` 주석 처리된 코드 블록 6건 제거. 최종 정리 — sub/에 주석 처리된 데드 코드 0건.
- **유형**: 데드 코드 제거(순수 삭제)
- **파일**: 3개 파일 수정(file_rw.c, tcpip_connect.c, queue.c)
- **주요 변경사항**: FR 항목 6건 — 이전 `sprintf` 직접 구조체 기록 3건(`sprintf(Tmp)+memcpy`로 대체됨), 폐기 예정 `inet_addr` 2건(`inet_pton`으로 대체됨), 비활성화된 `msgrcv` 큐 플러시 루프 1건
- **관련**: 데드 코드 제거의 네 번째이자 최종 단계. 누적: 4단계에 걸쳐 ~2,261줄
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(100%) → 보고서 → 아카이브

## commented-code-cleanup-round2

- **설명**: sub/, PB, PX 모듈의 활성 C 소스 파일 14개에서 `/* */` 주석 처리된 코드 블록 및 `//` 주석 처리된 라인 제거
- **유형**: 데드 코드 제거(순수 삭제)
- **파일**: 14개 파일 수정(sub 8 + PB 2 + PX 4)
- **주요 변경사항**: FR 항목 47건 + 보너스 2건 — 이전 sprintf/Tmp 버퍼 블록, 비활성화된 함수 호출(Exit_Process, Device_Close, INL_Initialize), 폐기된 변수 선언, 데드 대체 코드 경로, B1601 if/else 체인, 데드 #if B1201 조건부
- **관련**: 데드 코드 제거의 세 번째 단계(dead-code-cleanup #if 0 블록, pa-pb-8100-cleanup pa/pb_8100 내 /* */ 이후). 누적: ~2,222줄
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(100%) → 보고서 → 아카이브
- **반복**: 0회(첫 검증 100% 통과)
- **순 코드 변경**: ~-213줄(설계 190 + 보너스 Write_SLog 블록 23)
- **문서**:
  - [계획](commented-code-cleanup-round2/commented-code-cleanup-round2.plan.md)
  - [설계](commented-code-cleanup-round2/commented-code-cleanup-round2.design.md)
  - [분석](commented-code-cleanup-round2/commented-code-cleanup-round2.analysis.md)
  - [보고서](commented-code-cleanup-round2/commented-code-cleanup-round2.report.md)

## pa-pb-8100-cleanup

- **설명**: pa_8100_ts.c와 pb_8100_ts.c(클라이언트 통신 송신 프로세스)에서 `/* */` 주석 처리된 데드 코드, `//` 주석 처리된 호출, 미사용 변수 제거
- **유형**: 데드 코드 제거(순수 삭제)
- **파일**: 2개 파일 수정(pb_8100_ts.c, pa_8100_ts.c)
- **주요 변경사항**: FR 항목 8건 — HOLIDAY 검사 블록, 비활성화된 Device_Close 호출, 미사용 arry_cnt/arry_len/arry_dat 변수, Make_Send_Msg 검증 주석, seq 리셋 블록
- **관련**: dead-code-cleanup(`#if 0` 제거) 후속; 다음 카테고리(`/* */` 및 `//` 주석) 대상
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(100%) → 보고서 → 아카이브
- **반복**: 0회(첫 검증 100% 통과)
- **순 코드 변경**: -64줄(PB -39, PA -25)
- **문서**:
  - [계획](pa-pb-8100-cleanup/pa-pb-8100-cleanup.plan.md)
  - [설계](pa-pb-8100-cleanup/pa-pb-8100-cleanup.design.md)
  - [분석](pa-pb-8100-cleanup/pa-pb-8100-cleanup.analysis.md)
  - [보고서](pa-pb-8100-cleanup/pa-pb-8100-cleanup.report.md)

## dead-code-cleanup

- **설명**: 활성 C 소스 파일에서 모든 `#if 0` 및 `#if (0)` 데드 코드 블록 제거
- **유형**: 데드 코드 제거(순수 삭제)
- **파일**: 36개 파일 수정, 0개 생성
- **주요 변경사항**: sub/(5), PA(16), PB(4), PW(1), PX(6), PZ(4)에 걸쳐 총 1,945줄의 `#if 0`/`#if (0)` 블록 71개 제거. 원래 계획에 없던 `#if (0)` 변형의 보너스 파일 4개 포함.
- **설계 인사이트**: `grep "#if 0"`은 `#if (0)` 및 `#if\t0` 변형을 놓침; 구현에서 더 넓은 정규식 사용
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(98%) → 보고서 → 아카이브
- **반복**: 0회(첫 검증 98% 통과)
- **순 코드 변경**: -1,945줄
- **문서**:
  - [계획](dead-code-cleanup/dead-code-cleanup.plan.md)
  - [설계](dead-code-cleanup/dead-code-cleanup.design.md)
  - [분석](dead-code-cleanup/dead-code-cleanup.analysis.md)
  - [보고서](dead-code-cleanup/dead-code-cleanup.report.md)

## socket-linger-extraction

- **설명**: 15개 소스 파일에서 중복된 Set_Socket_Linger()를 매개변수화된 int fd 인수를 가진 sub/fep_common.c로 추출
- **유형**: 함수 추출 + 데드 코드 제거
- **파일**: 17개 파일 수정(라이브러리 2 + 소스 15), 0개 생성
- **주요 변경사항**: fep_common.c에 `Set_Socket_Linger(int fd)` 추가 + fep_common.h에 선언; 로컬 정의 15개 + 전방 선언 15개 제거; Sockfd 호출 10개 + Newfd 호출 2개 업데이트; 데드 코드 정의 3개 제거(pb_7200_tr, pb_7100_ts, pa_7100_ts)
- **설계 원칙**: fd 매개변수화 — 전역 변수를 사용하는 void 함수를 매개변수화된 함수로 변환하여 다중 변형 공유
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(100%) → 보고서 → 아카이브
- **반복**: 0회(첫 검증 100% 통과)
- **순 코드 변경**: ~-357줄, 15개 복사본 → 1개(93% 감소)
- **문서**:
  - [계획](socket-linger-extraction/socket-linger-extraction.plan.md)
  - [설계](socket-linger-extraction/socket-linger-extraction.design.md)
  - [분석](socket-linger-extraction/socket-linger-extraction.analysis.md)
  - [보고서](socket-linger-extraction/socket-linger-extraction.report.md)

## poll-oob-write-fix

- **설명**: 2개 파일에서 Poll[2] 범위 밖 쓰기 수정 + 2개 추가 파일에서 주석 처리된 데드 코드 제거
- **유형**: 치명적 버그 수정 + 데드 코드 정리
- **파일**: 4개 파일 수정(pb_1200_tr.c, pa_1200_tr.c, pb_7800_tr.c, pa_7800_tr.c)
- **근본 원인**: Poll[3]을 가진 조상 파일(pa_1100_ts.c 등)에서의 복사-붙여넣기. 배열이 Poll[2]로 축소되었으나 데드 Poll[2] 할당이 남아있음
- **영향**: 무음 BSS/스택 손상 — 프로세스 시작 시 인접 전역 변수 덮어쓰기
- **범위 검증**: `Poll[2]` 선언이 있는 8개 파일 전부 점검; 2개는 활성 버그, 2개는 주석 처리된 데드 코드, 4개는 정상
- **관련**: poll-buffer-overflow-fix(pb_7200_tr.c의 Poll[1] 범위 밖)의 동반 수정
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(100%) → 보고서 → 아카이브
- **반복**: 0회(첫 검증 100% 통과)
- **순 코드 변경**: -12줄
- **문서**:
  - [계획](poll-oob-write-fix/poll-oob-write-fix.plan.md)
  - [설계](poll-oob-write-fix/poll-oob-write-fix.design.md)
  - [분석](poll-oob-write-fix/poll-oob-write-fix.analysis.md)
  - [보고서](poll-oob-write-fix/poll-oob-write-fix.report.md)

## poll-buffer-overflow-fix

- **설명**: 치명적 버퍼 오버플로 수정 — pb_7200_tr.c가 배열 크기 [1](유효 인덱스: 0만)인 Poll[1]에 쓰기
- **유형**: 치명적 버그 수정
- **파일**: 1개 파일 수정(pb_7200_tr.c), 2줄 제거
- **근본 원인**: pb_7100_ts.c(Poll[2]을 가짐)에서의 복사-붙여넣기. 배열 크기가 1로 축소되었으나 데드 Poll[1] 할당이 남아있음
- **영향**: 매 프로세스 시작마다 무음 BSS 세그먼트 손상 — 인접 전역 변수 덮어쓰기
- **범위 검증**: `Poll[1]`이 있는 5개 파일 전부 점검; pb_7200_tr.c만 버그 있음
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(100%) → 보고서 → 아카이브
- **반복**: 0회(첫 검증 100% 통과)
- **순 코드 변경**: -2줄
- **문서**:
  - [계획](poll-buffer-overflow-fix/poll-buffer-overflow-fix.plan.md)
  - [설계](poll-buffer-overflow-fix/poll-buffer-overflow-fix.design.md)
  - [분석](poll-buffer-overflow-fix/poll-buffer-overflow-fix.analysis.md)
  - [보고서](poll-buffer-overflow-fix/poll-buffer-overflow-fix.report.md)

## fifo-event-rtn-adoption

- **설명**: 격리된 fifo_event.o를 통해 공유 Fifo_Event_Rtn을 나머지 13개 프로세스 파일로 확장
- **파일**: 15개 파일(신규 1 + 수정 14), 1개 함수를 자체 오브젝트 파일로 격리
- **주요 변경사항**: `fep_common.o`에서 `Fifo_Event_Rtn`을 격리하기 위해 `sub/fifo_event.c` 생성(ConnectRetryCnt/FmtPtr 전역에 대한 링커 의존성 회피); `fep_common.c`에서 정의 제거; PA/PB/PW 프로세스 파일에서 로컬 복사본 13개 제거; 링커 해결을 위한 전방 선언 유지
- **설계 원칙**: 오브젝트 파일 격리 — 최소 의존성을 가진 함수를 별도 .o로 분리
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(100%) → 보고서 → 아카이브
- **반복**: 0회(첫 검증 100% 통과)
- **순 코드 변경**: -195줄, 1~3단계 누적: -2,116줄
- **문서**:
  - [계획](fifo-event-rtn-adoption/fifo-event-rtn-adoption.plan.md)
  - [설계](fifo-event-rtn-adoption/fifo-event-rtn-adoption.design.md)
  - [분석](fifo-event-rtn-adoption/fifo-event-rtn-adoption.analysis.md)
  - [보고서](fifo-event-rtn-adoption/fifo-event-rtn-adoption.report.md)

## pa-pb-dedup-phase3

- **설명**: PA/PB 모듈 중복 제거 3단계 — Time_Out_Disconnect 헬퍼 추출, Fifo_Event_Rtn 통합(Write_Data 범위 제외)
- **파일**: 8개 파일 수정(헤더 1 + 라이브러리 1 + 프로세스 파일 6), 2개 함수 추출
- **주요 변경사항**: 연결 해제/재연결 패턴을 위한 Time_Out_Disconnect(const char *source) 헬퍼; Fifo_Event_Rtn() 통합; 데드 코드 제거(#if 0 블록, 주석 처리된 코드, 미사용 `int rt`)
- **설계 원칙**: 매개변수화보다 합성(모놀리식 모드 플래그보다 빌딩 블록 헬퍼)
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(100%) → 보고서 → 아카이브
- **반복**: 0회(첫 검증 100% 통과)
- **순 코드 변경**: -183줄, 1+2단계 누적: -1,921줄(-22%)
- **문서**:
  - [계획](pa-pb-dedup-phase3/pa-pb-dedup-phase3.plan.md)
  - [설계](pa-pb-dedup-phase3/pa-pb-dedup-phase3.design.md)
  - [분석](pa-pb-dedup-phase3/pa-pb-dedup-phase3.analysis.md)
  - [보고서](pa-pb-dedup-phase3/pa-pb-dedup-phase3.report.md)

## pa-pb-dedup-phase2

- **설명**: PA/PB 모듈 중복 제거 2단계 — FmtPtr 패턴, Log_Out/Device_Open LOGON 추출, PB Handshake/Free_All 분리
- **파일**: 10개 파일(신규 2 + 수정 8), 4개 함수 추출
- **주요 변경사항**: FmtPtr 패턴(extern void *FmtPtr)으로 S_Fmt vs KR_Fmt 해결; fep_common.c에 Log_Out_Base + Device_Open_Logon; fep_encrypt.c에 Free_All + Handshake; PA Handshake 스텁; PB의 fep_encrypt.h include 마이그레이션
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(97%) → 보고서 → 아카이브
- **반복**: 0회(첫 검증 97% 통과)
- **순 코드 변경**: ~-605줄, 1단계 누적: ~-1,738줄(-19%)
- **문서**:
  - [계획](pa-pb-dedup-phase2/pa-pb-dedup-phase2.plan.md)
  - [설계](pa-pb-dedup-phase2/pa-pb-dedup-phase2.design.md)
  - [분석](pa-pb-dedup-phase2/pa-pb-dedup-phase2.analysis.md)
  - [보고서](pa-pb-dedup-phase2/pa-pb-dedup-phase2.report.md)

## pa-pb-dedup

- **설명**: PA/PB 모듈 공통 함수 추출 - 6개 소스 파일의 중복 함수 6개를 sub/fep_common.c 공유 라이브러리로 추출
- **파일**: 8개 파일(신규 2 + 수정 6), 6개 함수 추출, SLog→Log 변환 117건
- **주요 변경사항**: Get_Msec, Line_Change, Device_Read, Device_Write, Device_Close_Base, Err_Msg를 sub/fep_common.c로 추출; PA 씬 래퍼 / PB 암호화 정리 래퍼로 Device_Close; 프로세스별 NO_TIME을 위한 extern NoTime[]
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(90%) → 보고서 → 아카이브
- **반복**: 0회(첫 검증 90% 통과)
- **순 코드 변경**: -1,133줄(-13%), 유지보수 비용 -83%(6개 복사본 → 1개)
- **문서**:
  - [계획](pa-pb-dedup/pa-pb-dedup.plan.md)
  - [설계](pa-pb-dedup/pa-pb-dedup.design.md)
  - [분석](pa-pb-dedup/pa-pb-dedup.analysis.md)
  - [보고서](pa-pb-dedup/pa-pb-dedup.report.md)

## fifo-struct-refactor

- **설명**: FIFO/SAM 파일 I/O 버퍼 구성 및 KRX 메시지 파싱을 원시 바이트 오프셋에서 구조체 기반 타입 안전 접근으로 리팩토링
- **파일**: 소스 파일 10개, ~65개 패턴 변환
- **주요 변경사항**: w_data[] → BUFF_RW_HEAD 구조체(3개 파일), DataBuff[8+6] → KRX_HEADER 캐스트(7개 파일), DataBuff[82+] → KRX_HEAD_LEN(1개 파일), ADD_HEADER_SIZE 매크로 제거(4개 파일)
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(97%) → 보고서 → 아카이브
- **반복**: 0회(첫 검증 97% 통과)
- **문서**:
  - [계획](fifo-struct-refactor/fifo-struct-refactor.plan.md)
  - [설계](fifo-struct-refactor/fifo-struct-refactor.design.md)
  - [분석](fifo-struct-refactor/fifo-struct-refactor.analysis.md)
  - [보고서](fifo-struct-refactor/fifo-struct-refactor.report.md)

## shm-struct-refactor

- **설명**: SHM 접근 코드를 원시 포인터 연산에서 구조체 기반 타입 안전 접근으로 리팩토링
- **파일**: 구현 파일 6개(헤더 2 + C 소스 4)
- **주요 구성요소**: shm_memory.h(SHM_VERSION/SHM_MAGIC), fep_sub.h(프로토타입), shmsub.c(Shm_Map_SubDaemon), shmipc.c(SHM_Attach_Verify), shm_rw.c(DSHM_W_Core), pz_memory_shm.c
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(95%) → 보고서 → 아카이브
- **반복**: 0회(첫 검증 95% 통과)
- **순 코드 변경**: +175 추가, -194 삭제 = -19 순
- **문서**:
  - [계획](shm-struct-refactor/shm-struct-refactor.plan.md)
  - [설계](shm-struct-refactor/shm-struct-refactor.design.md)
  - [분석](shm-struct-refactor/shm-struct-refactor.analysis.md)
  - [보고서](shm-struct-refactor/shm-struct-refactor.report.md)

## tr-struct-refactor

- **설명**: KRX TR 구조체 리팩토링 - 하드코딩된 바이트 오프셋을 구조체 기반 접근으로 변환
- **파일**: 구현 파일 8개(헤더 2 + C 소스 6), ~74개 변환
- **주요 구성요소**: krx_trcode.h(상수/매크로), pa_struct.h(KRX_MSG_COMMON), pb/pa_1200_tr.c, pb/pa_7800_tr.c, pb_1800_ts.c, pa_1100_ts.c
- **PDCA 주기**: 설계 → 실행(4단계) → 검증(72%) → 조치-1(92%) → 보고서 → 아카이브
- **반복**: 1회(17건 수정 적용)
- **문서**:
  - [설계](tr-struct-refactor/tr-struct-refactor.design.md)
  - [분석](tr-struct-refactor/tr-struct-refactor.analysis.md)
  - [보고서](tr-struct-refactor/tr-struct-refactor.report.md)

## ini-config-analysis

- **설명**: PB FEP INI 설정 전체 분석 및 문서화
- **유형**: 분석 / 문서화(코드 구현 없음)
- **범위**: INI 파일 10개(daemon, proc, tcp1, tcp2, udpip, file, dshm, sisetr, client, pc)
- **주요 산출물**: 프로세스 맵 18개, 데이터 흐름도 7개, 네트워크 토폴로지, TR 카탈로그 29개, 파일 간 일관성 매트릭스
- **일관성**: 97.5%(80개 항목 중 78개, 고아 포트 2개는 설명 완료)
- **PDCA 주기**: 계획 → 설계 → 보고서 → 아카이브(실행/검증 해당 없음)
- **문서**:
  - [계획](ini-config-analysis/ini-config-analysis.plan.md)
  - [설계](ini-config-analysis/ini-config-analysis.design.md)
  - [보고서](ini-config-analysis/ini-config-analysis.report.md)

## config-db-migration

- **설명**: FEP 시스템의 INI 파일 설정을 SQLite DB로 마이그레이션
- **파일**: 구현 파일 12개, ~4500줄
- **주요 구성요소**: config_db.h/c, config_loader.h/c, ini2db.c, db2ini.c, cfg_verify.c
- **PDCA 주기**: 계획 → 설계 → 실행(8단계) → 검증(90%) → 보고서 → 아카이브
- **문서**:
  - [계획](config-db-migration/config-db-migration.plan.md)
  - [설계](config-db-migration/config-db-migration.design.md)
  - [분석](config-db-migration/config-db-migration.analysis.md)
  - [보고서](config-db-migration/config-db-migration.report.md)

## src-commented-code-cleanup

- **설명**: 모든 활성 src/ 파일(PA, PB, PZ 모듈)에서 주석 처리된 데드 코드의 종합적 제거
- **파일**: 35개 파일, ~489줄 제거. FR 항목 30건 + 보너스 파일 5개
- **주요 변경사항**: `//` 단일행 및 `/* */` 다중행 주석 처리된 코드 블록 삭제. 기능 변경 없음. 데드 코드 정리 시리즈 완료(~90개 파일에 걸쳐 총 ~2,750줄 제거)
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(100%) → 조치(1회 반복) → 보고서 → 아카이브
- **문서**:
  - [계획](src-commented-code-cleanup/src-commented-code-cleanup.plan.md)
  - [설계](src-commented-code-cleanup/src-commented-code-cleanup.design.md)
  - [분석](src-commented-code-cleanup/src-commented-code-cleanup.analysis.md)
  - [보고서](src-commented-code-cleanup/src-commented-code-cleanup.report.md)

## if1-block-cleanup

- **설명**: 활성 src/ 파일에서 모든 `#if 1` / `#if (1)` 항상-참 전처리기 블록 제거
- **파일**: 11개 파일, ~36줄 제거(내부 코드 보존)
- **주요 변경사항**: 항상-참 조건부 블록 16개 풀기. 기능 변경 없음.
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(100%) → 보고서 → 아카이브
- **문서**:
  - [계획](if1-block-cleanup/if1-block-cleanup.plan.md)
  - [설계](if1-block-cleanup/if1-block-cleanup.design.md)
  - [분석](if1-block-cleanup/if1-block-cleanup.analysis.md)
  - [보고서](if1-block-cleanup/if1-block-cleanup.report.md)

## sizeof-memset-bugfix

- **설명**: `pb_1200_tr.c:520`의 치명적 `sizeof()` 포인터 연산 버그 수정 — `sizeof(DataBuff-82)`가 4014(버퍼 본체 크기) 대신 8(포인터 크기)로 평가됨. 시리즈 최초의 실제 버그 수정.
- **파일**: 1개 파일, 2줄 변경
- **주요 변경사항**: FR-01: `sizeof(DataBuff-82)` → `sizeof(DataBuff) - KRX_HEAD_LEN`. FR-02: 하드코딩된 `82` → `KRX_HEAD_LEN`
- **PDCA 주기**: 계획 → 설계 → 실행 → 검증(100%) → 보고서 → 아카이브
- **문서**:
  - [계획](sizeof-memset-bugfix/sizeof-memset-bugfix.plan.md)
  - [설계](sizeof-memset-bugfix/sizeof-memset-bugfix.design.md)
  - [분석](sizeof-memset-bugfix/sizeof-memset-bugfix.analysis.md)
  - [보고서](sizeof-memset-bugfix/sizeof-memset-bugfix.report.md)
