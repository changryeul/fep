#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : 채권 주문 forwarder (SEAM 주문큐 → 시장 송신부) — PB 부문
#   File    : pb_1109_mp.c
#
#   시장 재편 A안 §5.3 / Phase 4 P2. Model B에서 전략(po_)은 주문을 전역
#   SEAM 주문큐(SEAM_ORDQ_BOND)에 emit한다. 이 forwarder가 'b' 부문에서
#   그 큐를 소비해 채권 송신부(pb_1100_ts) 입력으로 중계(OFN 기록 + doorbell).
#
#   왜 forwarder인가: pb_1100_ts는 doorbell 구동·단일 DSHM 입력·클라이언트+
#   전략 주문 공용의 최고 정합성-핵심 송신부. 입력루프 직접 교체는 고위험 →
#   송신부 무변경, 신규 프로세스로 SEAM→부문내 큐 어댑팅(수신측 SEAM 소비 대칭).
#
#   입력 : SEAM_ORDQ_BOND (전역 0x41000016, SEAM_ORD_R). replay는 r_seq 상주.
#   출력 : OFN_1 (proc.ini 배선 — 운영은 pb_1100_ts 입력, 검증은 관측 파일큐).
#   레코드: SEAM_ORD_WIRE_RECSZ(471) 고정 wire 레코드(§5.3 P2 계약).
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "seam_queue.h"

#define     DATA_SIZE   400         /* 송신부(pb_1100_ts) 컨벤션 = wire 레코드 471 */
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants
------------------------------------------------------------------------*/
#define     DATA_TIME   60 * 1000
#define     SEAM_POLL   200         /* SEAM 재스캔 주기(ms). 저지연은 doorbell 후속 */
#define     READ_MAX    1

/*------------------------------------------------------------------------
    Globals
------------------------------------------------------------------------*/
int         R_Cnt;
char        ApType[10];
FILE_BUFF_FORMAT    W_Fmt;
char        Seam_Buf[READ_MAX * SEAM_Q_RECSZ];  /* SEAM 슬롯(480) 수신 → W_Fmt로 복사 */

void    PB_1109_MP(void);
void    Analyze_Data(void);

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    Init_Proc(argc, argv);
    SEAM_ORD_Init();        /* 전역 SEAM 주문큐 attach */
    PB_1109_MP();
    Exit_Process();
}   /* End of main () */

/*----------------------------------------------------------------------*/
void    PB_1109_MP(void)
/*----------------------------------------------------------------------*/
{
    int     rt, si;

    while (START_S != JOB_END) {
        Stat_Save();

        while (START_S != JOB_END) {
            Stat_Save();
            memset(&W_Fmt, 0, sizeof (FILE_BUFF_FORMAT));

            /* SEAM 주문큐(채권) 소비. 리더=시장별 송신 forwarder 1개. */
            R_Cnt = SEAM_ORD_R(SEAM_ORDQ_BOND, SEAM_ORD_R_SEND, Seam_Buf, READ_MAX);

            if (R_Cnt < 0) {
                Log(SAM_FATAL, "SEAM_ORD_R fail q=%d r=%d", SEAM_ORDQ_BOND, SEAM_ORD_R_SEND);
                sleep(1);
                Exit_Process();
            }
            else if (R_Cnt == 0)
                break;

            /* wire 레코드(471) → W_Fmt(송신부 FILE_BUFF_FORMAT). 나머지는 공백. */
            for (si = 0; si < R_Cnt; si ++) {
                memcpy(&W_Fmt, &Seam_Buf[si * SEAM_Q_RECSZ], SEAM_ORD_WIRE_RECSZ);
                W_Fmt.LineFeed[0] = '\n';

                rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);   /* OFN_1 → (운영)pb_1100_ts 입력 */
                if (rt != 1) {
                    Log(SAM_FATAL, "forwarder F_W fail[%s:%d]", OFN(D_K,P_K,0), rt);
                    Exit_Process();
                }
                Log(USR_OK, "forwarder OUT[%s] TR[%.11s]",
                        OFN(D_K,P_K,0), &W_Fmt.Data[11]);
            }
            /* SEAM 커서는 SEAM_ORD_R 시점에 전진(별도 Add_Count 불필요) */
        }

        rt = Poll_File(SEAM_POLL);
        if (rt == -1)
            continue;
    }
}   /* End of PB_1109_MP () */

/*************************************************************************
    End of Program (pb_1109_mp.c)
*************************************************************************/
