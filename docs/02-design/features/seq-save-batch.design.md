# seq-save-batch Design Document

> **Summary**: 매 주문 실행되는 `Seq_Save`(fopen+fcntl×2+fseek+fwrite+fflush+fclose ≈ 6 syscall)를 fd 캐시 + 시간 스로틀로 경량화. 기본 동작 불변(opt-in), 종료 시 최종 저장 보장.
>
> **Project**: FEP (Front-End Processor)
> **Author**: Claude
> **Date**: 2026-08-05
> **Status**: Draft
> **Plan**: `docs/01-plan/features/oms-performance-roadmap.plan.md` §3 F3

---

## 1. 조사 결과 (I-2: 복구 시맨틱)

- **커서의 권위 사본은 SHM** (`FILEM(dk,fk).r_cnt[]`). `_seq` 파일은 미러이며, **시스템 기동 시(`pz_memory_mp` → `pz_memory_conf.c:692-696`) SHM을 새로 만들 때만** 파일→SHM으로 복원된다. 프로세스 단독 재기동은 SHM 커서를 그대로 쓰므로 `_seq` 파일과 무관.
- **기존 `fflush`는 사실상 중복**: 바로 다음 `fclose`가 stdio 버퍼를 커널로 밀어낸다. `fsync`가 아니므로 현행 코드도 OS 크래시 시 최신 커서를 보장하지 않는다. 즉 진짜 비용은 fflush가 아니라 **매건 반복되는 fopen/잠금/fclose 전체**다.
- **뒤처진 커서의 위험**: 시스템 재기동 시 `_seq`가 실제보다 작으면 파일 큐의 처리 완료 레코드를 재읽어 **중복 주문 전송 위험**. 완화 요소: KRX 헤더 `MsgSeqNum`(INT_SEQ, `_stat` 파일에서 별도 복원 — `pz_memory_conf.c:2138`)의 중복 시퀀스 거부. 그래도 커서 저장은 보수적으로 다뤄야 함 → **스로틀은 opt-in, 기본은 매건 저장 유지**.
- `compact.sh`는 `_seq` 파일/`00000000` 디렉토리를 삭제하지 않음 → 프로세스 수명 동안 fd 캐시 유효. 파일 경로는 고정 디렉토리(`00000000`)라 일자 롤오버도 없음.
- 다중 프로세스가 같은 `_seq` 파일의 다른 슬롯(r_cnt[0..8])을 기록하므로 **fcntl 잠금은 유지**.
- `Exit_Process`는 `sub/setsigfatal.c:84` — 종료 시 최종 저장 훅 위치. `_in_signal_handler` 전역으로 시그널 컨텍스트 구분 가능(async-signal-safe 아님 → 시그널 중이면 스킵).

## 2. 설계

### 2.1 스로틀 판정 (`sub/seq_throttle.c` 신규 — 순수 로직, 단위 테스트 대상)

```c
/* inc/fep_sub.h */
extern int  Seq_Throttle_Interval (void);              /* env 1회 캐시 */
extern int  Seq_Throttle_Check (const char *, long);   /* 1=skip, 0=저장 */
```

- `Seq_Throttle_Interval()`: `FEP_SEQ_SAVE_INTERVAL`(초) 1회 평가 후 캐시. 미설정/0/음수/비숫자 → 0 (**기본: 매건 저장, 기존 동작**).
- `Seq_Throttle_Check(key, now)`: interval 0이면 항상 0(저장). interval>0이면 키별 last_time 테이블(정적 8슬롯, 초과 시 항상 저장)로 `now - last < interval`이면 1(skip), 아니면 last 갱신 후 0. 키는 `"<file>:<dk>:<fk>"`.

### 2.2 `Seq_Save` 개편 (`sub/seq_save.c`)

1. **fd 캐시** (항상 적용, 시맨틱 불변): `(f_name)` → `FILE*` 정적 테이블(8슬롯). 최초 호출 시 `fopen("r+")`(실패 시 기존과 동일 FATAL 로그+NOTOK), 이후 재사용. `fclose` 제거 → 매건 syscall 6회 → 4회(fcntl×2+fseek+fwrite... fflush 유지 시 5회).
2. **스로틀** (opt-in): 잠금 전에 `Seq_Throttle_Check` — skip이면 즉시 OK 리턴 (SHM 커서는 항상 최신이므로 다음 저장 때 자연 만회. 저장 값은 호출 시점의 SHM 스냅샷이라 손실 없음).
3. **`Seq_Save_Flush(void)` 신설**: 캐시된 모든 (file,dk,fk) 슬롯에 대해 스로틀 무시하고 즉시 저장. `Exit_Process`(setsigfatal.c)에서 호출 — 단 `_in_signal_handler`면 스킵. Seq_Save를 쓰지 않는 프로세스는 슬롯 0개로 no-op.
4. `Dshm_Seq_Save`는 **fd 캐시만** 적용 (ck별 부분 잠금 구조라 스로틀 키가 복잡 — 1차 범위 제외, 후속 확장).

### 2.3 동작 요약

| 모드 | 주문당 Seq_Save 비용 | `_seq` 최대 지연 |
|------|---------------------|-----------------|
| 기본 (env 미설정) | fcntl×2+fseek+fwrite+fflush (fopen/fclose 제거) | 0 (기존과 동일) |
| `FEP_SEQ_SAVE_INTERVAL=1` | 대부분 0 syscall (skip), 1초당 1회 저장 | 정상 종료 시 0 (Flush), 크래시 시 ≤1초 |

## 3. 검증 항목 (V-items)

| ID | 항목 | 방법 |
|----|------|------|
| V-01 | interval 파싱(미설정/0/양수/쓰레기) | unit (기본 모드 바이너리 + interval 모드 바이너리 분리 — env 캐시) |
| V-02 | skip/저장 판정 시퀀스, 키 격리, 슬롯 초과 시 저장 | unit |
| V-03 | 기본 모드에서 저장 호출 시퀀스 기존과 동일(매건) | unit + 코드 검토 |
| V-04 | fd 캐시 재사용/최초 실패 FATAL 경로 보존 | 코드 검토 + 서버 |
| V-05 | Exit_Process 경유 최종 저장, 시그널 중 스킵 | 코드 검토 + 서버 |
| V-06 | 서버 빌드 + kill -9 후 시스템 재기동 커서 복원 (integ) | 서버 (이연) |
