#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : 주문체결처리
#   File    : pa_1400_mp.c
#
#   설명  : KRX에서 수신한 체결(Fill) 데이터를 FIFO에서 읽어
#             매체구분(A/C/T)을 확인 후 자동매매 전략(DSHM) 및
#             Client FIFO로 전달하는 프로세스.
#             A1401=채권체결(KRX직접), A2401=파생체결(IMECO).
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

/*
 * DATA_SIZE: 빌드 옵션에 따라 수신 데이터 크기 결정
 *   A1401: 채권체결 → 400바이트 (KRX 직접연결 전문)
 *   A2401: 파생체결 → 200바이트 (IMECO 전문)
 */
#if defined A1401
#define     DATA_SIZE       400
#elif defined A2401
#define     DATA_SIZE       200
#endif
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DATA_TIME       60 * 1000       /* Poll 대기 시간 (60초, 밀리초 단위) */
#define     READ_MAX        1           /* FIFO에서 한 번에 읽을 최대 건수 */

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int         R_Cnt;                  /* FIFO 읽기 건수 */
FILE_BUFF_FORMAT    R_Fmt[READ_MAX];            /* FIFO 읽기용 버퍼 */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_1400_MP(void);
void    Analyze_Data(void);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 체결처리 루프 실행.       */
/*----------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
    Init_Proc(argc, argv);             /* 프로세스 초기화 */
    PA_1400_MP();                  /* 메인 처리 루프 */
    Exit_Process();                /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PA_1400_MP: 메인 처리 루프                    */
/*    1) FIFO에서 체결 데이터 읽기                   */
/*    2) Analyze_Data()로 체결 데이터 분석 및 전달         */
/*    3) 데이터가 없으면 Poll_File()로 60초 대기           */
/*----------------------------------------------------------------------*/
void    PA_1400_MP(void) {
    int     rt, read_flag;

    /* === 외부 루프: 프로세스 종료 신호(JOB_END)까지 반복 === */
    while (START_S != JOB_END) {
        Stat_Save();                   /* 프로세스 상태 저장 */

        /* === 내부 루프: FIFO에 데이터가 있는 동안 계속 읽음 === */
        while (START_S != JOB_END) {
            Stat_Save();
            memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * READ_MAX);

            R_Cnt = F_R(PS_R_1, (void *)R_Fmt, READ_MAX);

            if (R_Cnt < 0) {
                /* 읽기 실패: 오류 로그 후 프로세스 종료 */
                Log(SAM_FATAL, "cannot read File[%s,%d:%s]",
                        IFN(D_K,P_K,2), SYS_NO, SYS_STR);
                sleep(1);
                Exit_Process();
            }
            else if (R_Cnt == 0)
                break;              /* 읽을 데이터 없음 → Poll 대기 */

            Log(USR_OK, "RD [%s:%d][%d]",
                    IFN(D_K,P_K,0), R_Cnt, IFR(D_K,P_K,0,0));
            Analyze_Data();                    /* 체결 데이터 분석 및 전달 */
        }

        /* FIFO에 데이터가 없으면 최대 60초간 대기 */
        rt = Poll_File(DATA_TIME);

        if (rt == 1)
            Log(USR_OK, "poll timeout <%d>", OFW(D_K,P_K,0,0));
        else if (rt == -1)
            continue;               /* 오류 → 다음 루프 */
    }
}   /* End of PA_1400_MP () */

/*************************************************************************
    Function        : Analyze_Data
    Parameters IN   : (없음, 전역변수 R_Fmt/R_Cnt 사용)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 체결 데이터를 분석하여:
                      1) 매체구분(A/C/T) 확인 — 자사 주문만 처리
                      2) 자동매매 전략번호가 있으면 DSHM에 전달
                      3) Client FIFO(TS_W1_1)에 체결 데이터 기록

    데이터 포맷:
      A1401(채권): 82바이트 KRX헤더 + 체결전문
      A2401(파생): 20바이트 IMECO헤더 + 체결전문
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Analyze_Data(void) {
    int     i, j, rt, w_flag, target, imeco_gbn;
    char    W_Fmt[8192];                    /* FIFO/DSHM 쓰기용 버퍼 */
    char    tr_code[12];                    /* TR코드 (체결 유형 식별) */

    target = -1;

    for (i = 0; i < R_Cnt; i++) {
        w_flag = 0;
        memset(tr_code, 0, sizeof (tr_code));

        /* 빌드 옵션별 TR코드 추출 위치 */
#if defined A1401
        memcpy(tr_code, &R_Fmt[i].Data[11], 11);       /* KRX: 11바이트 오프셋에서 11자 */
#elif defined A2401
        memcpy(tr_code, R_Fmt[i].Data, 1);         /* IMECO: 첫 1바이트 (T=체결) */
#endif

#if defined A1401
        /*
         * [A1401 채권] 회원체결결과 처리
         * TR코드: TTRTDP42301 = KRX 채권 체결결과
         * 데이터 포맷: 82바이트 KRX헤더 + 체결전문
         * Write는 체결전문만 저장함
         */
        if (memcmp(tr_code, "TTRTDP42301", 11) == 0) {
            KRX_NOTE_SETTLE_DATA    *dat =
            (KRX_NOTE_SETTLE_DATA *)&R_Fmt[i].Data[0];

            imeco_gbn = 0;                  /* KRX 직접연결: 오프셋 보정값 없음 */

            /*
             * 매체구분 확인: MembershipItem+30 위치
             *   A = API 주문, C = Client 주문, T = 자동매매 주문
             *   이 세 가지가 아니면 타 매체 → 경고 로그만 출력
             */
            if ((memcmp(dat->MembershipItem+imeco_gbn+30, "A", 1) != 0) &&
                    (memcmp(dat->MembershipItem+imeco_gbn+30, "C", 1) != 0) &&
                    (memcmp(dat->MembershipItem+imeco_gbn+30, "T", 1) != 0)) {
                Log(USR_ERROR, "Other Media Recv!! Check Plz [%3.3s] [%.10s]",
                        dat->MembershipItem+imeco_gbn+30, dat->OrderNo);
            }
            else {
                /* 자사 주문: 쓰기 버퍼에 헤더+체결전문 복사 */
                memset(W_Fmt, 0x20, sizeof (W_Fmt));
                memcpy(&W_Fmt, &R_Fmt[i], sizeof(BUFF_RW_HEAD));
                memcpy(&W_Fmt[sizeof(BUFF_RW_HEAD)],
                        &R_Fmt[i].Data[0], sizeof (KRX_NOTE_SETTLE_DATA));
                W_Fmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

                /*
                 * 자동매매 전략 번호 확인 (MembershipItem+47, 4자리)
                 * "0000"보다 크면 전략 번호가 있음 → DSHM에 전달
                 * target = (앞2자리 - 1) × 10 + 뒤2자리
                 */
                if (memcmp(dat->MembershipItem+imeco_gbn+47, "0000", 4) > 0) {
                    target = (AtoIf(dat->MembershipItem+imeco_gbn+47, 2) - 1) * 10;
                    target = AtoIf(dat->MembershipItem+imeco_gbn+49, 2) + target;

                    /* 자동매매 전략에 체결 데이터 전달 */
                    rt = DSHM_W(target*10, (void *)&W_Fmt, 1);
                    if (rt != 1) {
                        Log(SAM_FATAL, "Dshm write fail[%s:%d]", ODN(D_K,P_K,target-1), rt);
                        Exit_Process();
                    }
                }

                /* Client FIFO에 체결 데이터 전달 */
                rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);
                if (rt != 1) {
                    Log(SAM_FATAL, "file write fail[%s:%d]", OFN(D_K,P_K,0), rt);
                    Exit_Process();
                }
            }
        }
#elif defined A2401
        /*
         * [A2401 파생] 회원체결결과 처리
         * TR코드: T = IMECO 체결결과 (TTRTDP42301에 해당)
         * 데이터 포맷: 20바이트 IMECO헤더 + 체결전문
         * Write는 IMECO헤더를 제외한 체결전문만 저장함
         */
        if (memcmp(tr_code, "T", 1) == 0) {
            IMECO_JUMUN_DATA    *dat =
            (IMECO_JUMUN_DATA *)&R_Fmt[i].Data[0];

            imeco_gbn = -30;                    /* IMECO: 오프셋 보정값 -30 */

            /*
             * 매체구분 확인: 동일 로직 (A/C/T만 처리)
             * imeco_gbn=-30이므로 실제 위치는 dat->MembershipItem+0
             */
            if ((memcmp(dat->MembershipItem+imeco_gbn+30, "A", 1) != 0) &&
                    (memcmp(dat->MembershipItem+imeco_gbn+30, "C", 1) != 0) &&
                    (memcmp(dat->MembershipItem+imeco_gbn+30, "T", 1) != 0)) {
                Log(USR_ERROR, "Other Media Recv!! Check Plz [%3.3s] [%.10s]",
                        dat->MembershipItem+imeco_gbn+30, dat->OrderNo);
            }
            else {
                /* 자사 주문: IMECO헤더 제외하고 체결전문만 복사 */
                memset(W_Fmt, 0x20, sizeof (W_Fmt));
                memcpy(&W_Fmt, &R_Fmt[i], sizeof(BUFF_RW_HEAD));
                memcpy(&W_Fmt[sizeof(BUFF_RW_HEAD)],
                        &R_Fmt[i].Data[sizeof(IMECO_HEADER)], sizeof (IMECO_JUMUN_DATA));
                W_Fmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

                /* 자동매매 전략 번호 확인 및 DSHM 전달 */
                if (memcmp(dat->MembershipItem+imeco_gbn+47, "0000", 4) > 0) {
                    target = (AtoIf(dat->MembershipItem+imeco_gbn+47, 2) - 1) * 10;
                    target = AtoIf(dat->MembershipItem+imeco_gbn+49, 2) + target;

                    rt = DSHM_W(target*10, (void *)&W_Fmt, 1);
                    if (rt != 1) {
                        Log(SAM_FATAL, "Dshm write fail[%s:%d]", ODN(D_K,P_K,target-1), rt);
                        Exit_Process();
                    }
                }

                /* Client FIFO에 체결 데이터 전달 */
                rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);
                if (rt != 1) {
                    Log(SAM_FATAL, "file write fail[%s:%d]", OFN(D_K,P_K,0), rt);
                    Exit_Process();
                }
            }
        }
#endif
        else
            Log(USR_ERROR, "Else Case TR Recv Check Plz [%s] [%s]",
                    tr_code, &R_Fmt[i].Data[0]);

        Add_Count(PS_R_1, 1);                  /* 처리 건수 카운터 증가 */
    }   /* End of for */

}

/*************************************************************************
    End of Program (pa_1400_mp.c)
*************************************************************************/
