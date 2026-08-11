# order-pipeline-dshm Design Document (Stage 1: 주문 인바운드 홉)

> **Summary**: PB 주문 인바운드 홉(`pb_1301_tr → pb_1101_ts`)의 프로세스 간 전달을 디스크 파일 큐에서 DSHM(SHM 링 + FIFO 웨이크업)으로 전환. **런타임 설정 감지**(proc.ini의 IFN/IDN) 이중 경로라 재빌드 없이 설정만으로 전환/롤백. 실측 기준 홉당 파일 큐 왕복 13.5~19.6µs 제거.
>
> **Project**: FEP (Front-End Processor)
> **Author**: Claude
> **Date**: 2026-08-06
> **Status**: Draft
> **Plan**: `docs/01-plan/features/oms-performance-roadmap.plan.md` §3 F5

---

## 1. 조사 결과 — DSHM이 이미 F5의 설계를 내장

`sub/shm_rw.c` 분석 (PA의 `pa_8200_tr → DSHM_W → pa_1100_ts`가 검증된 참조 구현):

1. **`DSHM_W_Core`(:192)**: 세마포어 보호 SHM 링에만 기록(디스크 무접촉) + 소비자 FIFO 통지(캐시 fd) + **FIFO[0]으로 SyncManager에 통지**
2. **SyncManager**(`pw_1000_mp.c` → `pb_1001_mp`): 링을 후행 소비해 백킹 디스크 파일에 기록 — **원문 파일 write-behind 보존이 이미 아키텍처에 내장** (2026-08-06 조회수준 판단의 전제 충족)
3. **`DSHM_R`(:25)**: SHM 링에서 읽되, 링 랩으로 놓친 데이터는 **백킹 파일로 자동 폴백** — 느린 소비자 안전망
4. **API 1:1 대응**: `F_R↔DSHM_R`, `Add_Count↔Dshm_Add_Count`, `F_W↔DSHM_W` (인자 규약 동일 — `TS_W1_1=10`, `PS_R_1` 그대로 사용 가능). 레코드 형식(BUFF_RW_HEAD/FILE_RW_HEAD)도 동일
5. **`Init_Proc`(:158-175)**: proc.ini의 `FIF`/`IDN`/`in_d` 설정으로 FIFO 오픈과 DSHM attach를 자동 처리 — 프로세스 코드는 입력 종류를 몰라도 됨

## 2. 설계 — 런타임 설정 감지 이중 경로

### 2.1 전환 방식 결정

`#ifdef` 빌드 스위치 대신 **`PROC(D_K,P_K).in_d[0]`/`out_d[0]` 런타임 감지**:

- proc.ini가 `IFN_1`(파일)이면 기존 경로 그대로 (기본 — **기존 운영 설정에서 동작 무변경**)
- proc.ini를 `IDN_1`/`ODN_1`(DSHM)로 바꾸면 DSHM 경로
- 빌드-설정 불일치 위험 원천 제거, 롤백 = 설정 원복 + 프로세스 재기동 (재빌드 불필요)

### 2.2 코드 변경 (분기 3지점)

| 파일 | 지점 | before | after |
|------|------|--------|-------|
| `src/PB/pb_7200_tr.c:251` (pb_1301_tr) | 주문 쓰기 | `rt = F_W(TS_W1_1, &W_Fmt, 1)` | `out_d[0] != 0`이면 `DSHM_W(TS_W1_1, &W_Fmt, 1)`, 아니면 기존 |
| `src/PB/pb_1100_ts.c` Make_Data_Block | 주문 읽기 | `r_cnt = F_R(PS_R_1, R_Fmt, MAX_CNT)` | `in_d[0] != 0`이면 `DSHM_R(...)`, 아니면 기존 (FATAL 로그도 IDN/IFN 분기) |
| `src/PB/pb_1100_ts.c` Data_Event_Rtn | 커서 전진 | `Add_Count(PS_R_1, 1)` | `in_d[0] != 0`이면 `Dshm_Add_Count(...)`, 아니면 기존 |

### 2.3 설정 변경 (DSHM 활성화 시 운영 절차 — 코드 배포와 독립)

1. `cfg/dshm.ini`: `PB_CONF_START` 섹션 신설 + 세그먼트 정의 (기존 PA `Dshm_1` 스펙 준용):
   ```ini
   PB_CONF_START
   Dshm_Count=1
   Dshm_1_Comment=채권주문송신01
   Dshm_1_Name=pb_1101_ts
   Dshm_1_Key=0101,S
   Dshm_1_Fifo=1
   Dshm_1_Size=400
   Dshm_1_Max=10000
   Dshm_End
   ```
2. `proc.ini`: `pb_1301_tr`의 `OFN_1=pb_1101_ts` → `ODN_1=pb_1101_ts`, `pb_1101_ts`의 `IFN_1` → `IDN_1`
3. **SyncManager `pb_1001_mp` 기동 필수** (현 운영 Status S → R): write-behind 파일 기록과 DSHM_R 폴백의 전제. 미기동 시 FIFO[0] 통지 실패 FATAL 로그 + 랩 시 데이터 유실 위험
4. 시스템 재기동 (pz_memory_mp가 dshm.ini 기반 세그먼트/FIFO 생성)

### 2.4 Stage 2 (후속): 체결 아웃바운드

`pb_1201_tr → pb_1402_ts → pb_1211_tr → pb_8111_ts` — 동일 패턴. Stage 1 안정화 후. MAX_CNT 배칭(F4에서 이관)도 DSHM_R 다건 읽기와 함께 Stage 2에서 설계.

## 3. 검증 항목 (V-items)

| ID | 항목 | 방법 |
|----|------|------|
| V-01 | 기본(IFN 설정)에서 기존 경로 그대로 — 코드 분기만 추가 | 코드 검토 + 서버 빌드 + 기존 설정 무변경 확인 |
| V-02 | 문법 전 변형 (B1101/B1801, B1301 등) | 로컬 + 서버 |
| V-03 | 서버 전체 빌드 | `mk.sh sub && mk.sh src` |
| V-04 | DSHM 경로 E2E: 설정 전환 + pz/SyncManager/mock KRX 기동 → 주문 주입 → KRX 송신 확인 + 백킹 파일 기록 확인 | 서버 (하니스 구축 필요 — 별도 세션) |
| V-05 | 롤백: 설정 원복 후 파일 큐 경로 정상 | V-04와 함께 |
| V-06 | F1 계측으로 홉 레이턴시 전/후 비교 | V-04와 함께 |

## 3.1 E2E 검증 결과 (2026-08-06, `test/e2e/` 하니스)

`test/e2e/run_e2e.sh` + `mock_oms.c`로 실제 파이프라인(pz SHM 로드 → mock_krx → pb_1101_ts → mock_oms → pb_1301_tr)을 서버에서 기동.

- **V-02/V-03 통과**: 전 변형 문법 + 서버 전 소스 빌드 에러 0, 단위 156 통과
- **V-01(파일큐 경로) 통과**: file 모드에서 주문 20건 전량 pb_1101_ts 처리(W20 R20), **크래시 없이** KRX 송신 확인
- **V-04(DSHM 경로) 통과 (2026-08-06, I-8 해결 후)**: DSHM 세그먼트 `0x42020100` 생성(4.85MB, 권한 600), pb_1101이 DSHM에서 20/50건 전량 읽음(R=count), 전건 mock KRX 도달, SyncManager 백킹 파일 write-behind 확인, 크래시 0.
  - **I-8 근본원인 = 설정 문제(코드 아님)**: `Dshm_Config_Read`는 `key_info[5]`가 `'E'`(끝)/`'O'`(단일)일 때만 세그먼트 생성. PB 항목이 `0101,S`(시작)인데 뒤에 `'E'`가 없어 영영 미생성 → 프로세스가 size=0으로 create 시도 → EINVAL. 단일 큐이므로 `0101,O`로 수정(dshm.ini).
- **V-05(파일 롤백) 통과**: dshm.ini 변경 후에도 file 모드 정상, 설정 원복으로 롤백 확인.
- **V-06(계측 비교) 완료 — 예상과 다른 결론**: `_LAT_TRACE=1` 빌드 + mock_oms IN(주입) ↔ pb_1101 OUT(KRX송신) 주문번호 조인으로 엔드투엔드 측정.

  | 부하 | file 모드 p50/p99 | DSHM 모드 p50/p99 |
  |------|------------------|-------------------|
  | 저부하 (50건, 8ms 간격) | 94 / 172 µs | 102 / 170 µs |
  | 버스트 (500건, 간격 0) | 18.7 / 28.8 ms | 19.8 / 29.0 ms |

  **결론: 이 주문 인바운드 파이프라인에서 DSHM 전환은 레이턴시를 유의미하게 개선하지 않는다.** 저부하에서는 홉 I/O 차이(~12µs, lat_bench §5.0)가 네트워크 왕복+스케줄링 지터에 묻히고, 버스트에서는 두 모드 모두 ~19ms로 포화. F1 계측을 먼저 만든 이유가 이것: 가설(디스크 큐가 병목)이 측정으로 반증됨.

  **⚠ 정정 (2026-08-06)**: 위 측정은 모두 `NO_INISAFE` 빌드 = **암호화 제외** 값이다(dev 서버 기본, `pkg_env.sh`가 `-DNO_INISAFE` 강제, `pb_1100_ts.c:1028` 암호화가 `#else` 평문 memcpy로 대체). 따라서 초기 결론의 "병목=암호화+TCP"에서 **암호화는 측정 경로에 없었으므로 오류**. 암호화 제외 실측 분해:
  - pb_1101 내부(F1 IN→OUT: read→format→평문복사→TCP send): 저부하 p50 37µs(file)/46µs(dshm)
  - 엔드투엔드(주입→KRX): ~95µs → 상류(OMS TCP + pb_1301 F_W + FIFO wake + 스케줄링)가 나머지 ~50µs
  - **즉 dev 병목은 TCP 송신 + 프로세스 스케줄링/홉**이지 암호화도 큐 I/O도 아님.
  - 운영에서는 여기에 INISAFE 주문당 암호화가 **고정 추가**되나, 이는 KRX 필수라 최적화 대상이 아니며 dev에서 측정 불가. 관련: 메모리 `inisafe-dev-excluded`.

  **부수 관찰 → I-9 조사 완료 (2026-08-06)**: 버스트 시 주문 중복전송(100건 주입 → mock KRX 8,202건 수신, pb_1101이 주문번호별 최대 42회 재전송) 확인. **근본원인 = 하니스 아티팩트**:
  1. mock KRX가 주문을 **송신 소켓으로 echo**(비현실적 — 실 KRX는 응답/체결을 별도 접속 pb_1201_tr로 전송)
  2. pb_1101(순수 송신 프로세스)이 예상 못한 인바운드 수신 → 연결 끊김 → 재접속
  3. 재접속 시 KRX 시퀀스 협상(`pb_1100_ts.c:559` `FirstSeq > INT_SEQ`, `:580` `RD_CNT -= mun`)이 **RD_CNT를 음수로**(-27 관측)
  4. 드레인 루프 `if (WR_CNT > RD_CNT) Data_Event_Rtn()`가 음수 커서부터 무한 재전송 → 중복 폭주

  **수정**: mock을 실 KRX처럼 주문 송신 소켓에 echo하지 않도록 변경(`mock_krx_server.c` TCHODR echo 제거). 재실행 시 **100건 주입 → 100건 수신, 중복 0**. F5/F2 변경과 무관(file 경로·seq 로직 모두 기존 코드).

  **⚠ 잠재 취약성(운영 판단 필요, 미수정)**: `RD_CNT`(IFR 읽기커서)가 KRX 시퀀스 협상으로 음수가 될 수 있고, 드레인 루프가 이를 무경계로 신뢰해 재전송한다. 실 KRX가 비정상적으로 낮은 FirstSeq를 응답하면 주문 중복전송 위험. 방어책(RD_CNT 하한 클램프 or 재전송 상한)은 **KRX 프로토콜 지식이 필요한 핵심 주문로직**이라 운영자 검토 후 별도 수정 권장 — 하니스 아티팩트가 아닌 실제 코드 경화 기회.

### E2E 하니스 부산물 — 실제 크래시 버그 발견·수정 (sfmt-overflow-fix)

하니스 첫 실행에서 pb_1101_ts가 3건째 주문에서 SIGSEGV. gdb 백트레이스로 근본원인 확정:
`memcpy(&S_Fmt, DataBuff, RecvLen)` (`pb_1100_ts.c:406,550`, `pb_1800_ts.c:399,573`)가 **RecvLen을 `KRX_SESSION_FMT` 크기로 제한하지 않아** KRX 응답(336B)이 구조체를 넘어 BSS를 오버런 → 인접 전역(`_seq_fd` fd캐시 테이블)을 오염 → `Seq_Save`의 `strcmp`에서 크래시. 4곳 모두 구조체 크기 클램프로 수정, 서버 재빌드·재현 없음 확인. **운영 코드의 잠재 원격 오버플로**였음 (KRX가 예상보다 긴 전문 전송 시 크래시).

## 4. Out of Scope

- Stage 2 (체결 아웃바운드 3홉), MAX_CNT 배칭
- 운영 proc.ini/dshm.ini 실제 전환 (운영 절차 — §2.3은 준비물)
- F8 (포워더 홉 축소)
