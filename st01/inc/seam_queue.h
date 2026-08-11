#ifndef SEAM_QUEUE_H
#define SEAM_QUEUE_H

/*------------------------------------------------------------------------
 *  SEAM 스테이징 큐 — 부문 간(pc_/pb_ 수신 → po_ OMS코어) 메시지 전달용
 *  전역 고정키 SHM 링버퍼. 시장 재편 A안 §5.1.1 (2026-08-07 확정).
 *
 *  파일큐/DSHM 큐는 부문별 카운터라 크로스부문 직접연결 불가 → 전역 고정키
 *  링버퍼로 교체. po_ 미체결/체결 등록 로직은 무변경, 전달경로만 이 큐로.
 *
 *  replay-safe: w_seq/r_seq가 SHM 상주 → 재기동해도 커서 보존(재소비 없음).
 *  다중 writer(pc_+pb_ 동시): SEAM_W가 세마포어로 w_seq 구간 보호.
 *  multi-reader: (qidx,ridx)별 독립 r_seq = 프로덕션 파일큐 PS_R_1/PS_R_2 의미.
 *------------------------------------------------------------------------*/

#define     SEAM_Q_SHM_KEY  0x41000015L     /* 부문간 스테이징 큐(+TEST 0x01000000) */
#define     SEAM_Q_SLOTS    4096            /* 큐당 링 슬롯 수                       */
#define     SEAM_Q_RECSZ    480             /* 슬롯 크기 ≥ FILE_BUFF_FORMAT(471@DATA_SIZE=400):
                                               레코드 전체 저장 → po_ 슬롯→R_Fmt 복사만으로
                                               Data/DataHeader 등 모든 필드 참조 동일        */
#define     SEAM_Q_NQUEUE   2               /* 큐 개수                               */
#define     SEAM_Q_NREADER  4               /* 큐당 최대 리더(multi-reader)          */

#define     SEAM_MAGIC      0x5345414DL     /* "SEAM" — 최초 1회 init 판별           */

/* 큐 인덱스 */
#define     SEAM_Q_RESP     0               /* 응답(회원처리호가)                    */
#define     SEAM_Q_EXEC     1               /* 체결                                  */

/* 리더 인덱스 — 응답 큐(SEAM_Q_RESP) */
#define     SEAM_R_MICHE    0               /* po_1290_mp 미체결등록                 */
#define     SEAM_R_DIST     1               /* po_1200_mp 분배                       */
/* 리더 인덱스 — 체결 큐(SEAM_Q_EXEC) */
#define     SEAM_R_LEDGER   0               /* po_1490_mp 체결원장                   */

typedef struct {
    long    w_seq;                          /* 총 write 수(단조증가, 발행 커서)      */
    long    r_seq[SEAM_Q_NREADER];          /* 리더별 read 커서                      */
    long    dropped[SEAM_Q_NREADER];        /* 리더별 오버플로 유실 누계(무음유실 금지)*/
    char    slot[SEAM_Q_SLOTS][SEAM_Q_RECSZ];
}   SEAM_QUEUE;

typedef struct {
    long        magic;                      /* SEAM_MAGIC 이면 초기화 완료           */
    SEAM_QUEUE  q[SEAM_Q_NQUEUE];
}   SEAM_SHM;

/*------------------------------------------------------------------------
 *  SEAM 주문큐 — 전략(po_) → 시장 송신부(pb_/pc_/pf_ 1101_ts) (§5.3, Model B)
 *  방향 core→send (이벤트 SEAM의 역방향). 이벤트와 별도 세그먼트로 분리.
 *    - 시장별 주문큐(send process 1:1 소비). writer=다수 전략(multi).
 *    - 링 코어(seam_ring_*)·SEAM_QUEUE 재사용.
 *------------------------------------------------------------------------*/
#define     SEAM_ORD_Q_SHM_KEY  0x41000016L /* 주문큐 세그먼트(+TEST 0x01000000)     */
#define     SEAM_ORDQ_BOND  0               /* 채권 주문 → pb_1101_ts                */
#define     SEAM_ORDQ_DERIV 1               /* 파생/통화선물 주문 → pc_1101_ts       */
#define     SEAM_ORDQ_FX    2               /* FX 주문 → pf_1101_ts(신규)            */
#define     SEAM_ORD_NQUEUE 3
#define     SEAM_ORD_R_SEND 0               /* 리더: 시장별 송신부(큐당 1)           */
#define     SEAM_ORD_MAGIC  0x53454F52L     /* "SEOR"                                */

/*  주문 wire 레코드 계약(§5.3 P2, 2026-08-10):
 *  전략(DATA_SIZE=2048, W_Fmt≈2119B) ↔ 송신부(pb_1100_ts DATA_SIZE=400).
 *  ⚠ SEAM_ORD_W에 sizeof(전략 W_Fmt)를 넘기면 슬롯(480) 오버플로.
 *  → 양측이 합의한 wire 레코드 = 송신부 FILE_BUFF_FORMAT@DATA_SIZE=400 = 70+400+1.
 *  DSHM/SEAM은 바이너리 고정레코드라 LineFeed 무관(주문 TCHODR4=254B로 여유).      */
#define     SEAM_ORD_WIRE_RECSZ  471        /* 전략 emit·송신부 read 공통 전송 길이   */

typedef struct {
    long        magic;
    SEAM_QUEUE  q[SEAM_ORD_NQUEUE];
}   SEAM_ORD_SHM;

/*------------------------------------------------------------------------
 *  순수 링 코어 (락/SHM 무관 — 단위테스트 대상)
 *    seam_ring_write : 1건 기록, w_seq++. 성공 1.
 *    seam_ring_read  : 리더 ridx가 미소비분을 최대 maxrec건까지 out에 복사,
 *                      r_seq[ridx] 전진. 오버플로 시 dropped[ridx] 누계 +
 *                      최신창으로 점프. 반환 = 읽은 건수.
 *  out 은 maxrec * SEAM_Q_RECSZ 바이트 이상.
 *------------------------------------------------------------------------*/
int     seam_ring_write(SEAM_QUEUE *q, const void *rec, int recsz);
int     seam_ring_read (SEAM_QUEUE *q, int ridx, void *out, int maxrec);

/*------------------------------------------------------------------------
 *  at-least-once(peek→처리→commit) 지원:
 *    seam_ring_peek   : r_seq 전진 없이 미소비분을 out에 복사(오버플로 유실
 *                       점프는 즉시 반영). 반환 = 조회 건수.
 *    seam_ring_commit : 처리 완료분 n건만큼 r_seq[ridx] 전진.
 *  peek 후 commit 전 크래시 시 재기동 시 재peek(재처리) → 소비자 멱등 필요.
 *------------------------------------------------------------------------*/
int     seam_ring_peek  (SEAM_QUEUE *q, int ridx, void *out, int maxrec);
void    seam_ring_commit(SEAM_QUEUE *q, int ridx, int n);

/*------------------------------------------------------------------------
 *  Seam_Order_Queue — 시장 letter → 주문큐 인덱스 (순수, §5.3).
 *    'b'→BOND 'c'→DERIV 'f'→FX. 미지원 letter → -1.
 *    전략이 자기 시장으로 주문큐를 선택하는 라우팅 키. (차익은 leg별 2회 호출)
 *------------------------------------------------------------------------*/
int     Seam_Order_Queue(char letter);

/*------------------------------------------------------------------------
 *  SHM 생명주기 + 락 적용 프로덕션 API
 *    SEAM_Init : SHM attach/create + 최초 1회 zero-init. 성공 0.
 *    SEAM_W    : qidx 큐에 1건 기록(세마포어 보호). 성공 1.
 *    SEAM_R    : qidx/ridx 리더 소비. 반환 = 읽은 건수(음수 오류).
 *------------------------------------------------------------------------*/
int     SEAM_Init(void);
int     SEAM_W(int qidx, const void *rec, int recsz);
int     SEAM_R(int qidx, int ridx, void *out, int maxrec);
int     SEAM_Peek  (int qidx, int ridx, void *out, int maxrec);  /* r_seq 불변 조회 */
int     SEAM_Commit(int qidx, int ridx, int n);                  /* 처리분 커밋(전진) */

/*------------------------------------------------------------------------
 *  주문큐(0x41000016) 프로덕션 API — 전략 write / 송신부 read (§5.3).
 *    at-most-once(주문은 중복 위험 > 유실 위험). 오버플로 dropped 회계.
 *------------------------------------------------------------------------*/
int     SEAM_ORD_Init(void);
int     SEAM_ORD_W(int qidx, const void *rec, int recsz);        /* 전략 → 주문큐   */
int     SEAM_ORD_R(int qidx, int ridx, void *out, int maxrec);   /* 송신부 소비      */

#endif  /* SEAM_QUEUE_H */
