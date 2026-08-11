#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : RDS시세수신 DD (Data Distribution)
#   File    : pa_7800_dd.c
#
#   설명  : RDS(시세분배시스템)에서 수신한 데이터를 FIFO 파일에서 읽어
#             조건에 맞는 종목만 필터링하여 다른 FIFO로 전달하는 프로세스.
#             A7801=채권종목정보, A7802=KTS종목정보, A7803=기타.
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

/*
 * DATA_SIZE: 빌드 옵션(-DA7801 등)에 따라 수신 데이터 크기 결정
 *   A7801: 채권종목정보 → 1200바이트
 *   A7802: KTS종목정보  → 350바이트
 *   A7803: 기타         → 100바이트
 */
#if defined A7801
#define     DATA_SIZE       1200
#elif defined A7802
#define     DATA_SIZE       350
#elif defined A7803
#define     DATA_SIZE       100
#endif

#include    "buf_struct.h"
/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DATA_TIME       60 * 1000           /* Poll 대기 시간 (60초, 밀리초 단위) */
#define     S_H_SIZE        17                  /* 시세 헤더: TR(5)+Code(12) */
#define     M_H_SIZE        30                  /* 마스터 헤더: TR(5)+JCNT(5)+DATE(8)+Code(12) */
KS_EXPCODE  Key;                                /* 종목코드 검색용 키 구조체 */

int         Che_Gbn;                            /* 체결 구분값 */
char        ApType[10];                         /* 어플리케이션 타입 (프로세스 식별용) */
char        d_time[16];                         /* 현재 날짜시간 (yyyymmddhhmmss) */
FILE_BUFF_FORMAT        R_Fmt[1];               /* FIFO 읽기용 버퍼 (1건) */
BUFF_RW_HEAD            f_head;                 /* 버퍼 읽기/쓰기 헤더 */

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_7800_DD(void);
void    Set_Sise(char *);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 메인 루프 실행.                 */
/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);                     /* 프로세스 초기화 (설정파일 로딩, SHM 연결 등) */
    PA_7800_DD();                              /* 메인 처리 루프 시작 */
    Exit_Process();                            /* 프로세스 정상 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PA_7800_DD: 메인 처리 루프                                            */
/*    1) FIFO에서 RDS 시세 데이터를 읽음                              */
/*    2) Set_Sise()로 조건 필터링 후 대상 FIFO에 전달                   */
/*    3) 데이터가 없으면 Poll_File()로 60초 대기                       */
/*----------------------------------------------------------------------*/
void    PA_7800_DD(void) {
    int             i, rt, len, R_Cnt;
    char            m_time[24];
    char            fifo_name[100], bumun[4];
    char            W2_Fmt[2048];

    /* 현재 날짜시간 조회 (yyyymmddhhmmss 형식) */
    Get_DateTime(d_time);

    /*
     * RDS파일에서 읽어서 A0파일 대상으로 Write하기
     * 예: 7801(RDS수신) → 필터링 → 7181(내부배포)
     */

    /* === 외부 루프: 프로세스 종료 신호(JOB_END)까지 반복 === */
    while (START_S != JOB_END) {
        Stat_Save();                           /* 프로세스 상태 저장 (alive 표시) */

        /* === 내부 루프: FIFO에 데이터가 있는 동안 계속 읽음 === */
        while (START_S != JOB_END) {
            Stat_Save();
            memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT));

            /* FIFO에서 RDS 시세 데이터 1건 읽기 */
            R_Cnt = F_R(PS_R_1, (void *)R_Fmt, 1);
            if (R_Cnt < 0) {
                /* 읽기 실패: 오류 로그 후 프로세스 종료 */
                Log(SAM_FATAL, "cannot read file[%s,%d:%s]",
                        IFN(D_K,P_K,0), SYS_NO, SYS_STR);
                sleep(1);
                Exit_Process();
            }
            else if (R_Cnt == 0) {
                break;                          /* 읽을 데이터 없음 → Poll 대기로 전환 */
            }

            Set_Sise(R_Fmt[0].Data);           /* 조건 필터링 후 대상 FIFO에 전달 */

            Set_TR_Time();                     /* TR 시간 갱신 */
            INT_SEQ ++;                         /* 내부 시퀀스 번호 증가 */

            Add_Count(PS_R_1, 1);               /* 처리 건수 카운터 증가 */
        }

        /* FIFO에 데이터가 없으면 최대 60초간 대기 */
        rt = Poll_File(DATA_TIME);

        if (rt == 1)
            Log(USR_OK, "poll timeout <%d>", INT_SEQ);     /* 60초 타임아웃 발생 */
        else if (rt == -1)
            continue;                                       /* 오류 발생 시 루프 재시작 */
    }

    return;
}   /* End of PA_7800_DD () */

/*************************************************************************
    Function        : Set_Sise
    Parameters IN   : p_buf - 수신된 RDS 시세 데이터 포인터
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : RDS 시세 데이터를 조건 검사하여, 조건에 맞으면
                      내부 배포용 FIFO(7181 등)에 기록한다.
                      A7801=채권종목정보, A7802=KTS종목정보.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Set_Sise(char *p_buf) {
    int         ii, s_k, idx, rt;
    char        W_Fmt[2048];                    /* FIFO 쓰기용 임시 버퍼 */

#if defined A7801
    /*
     * [A7801] RDS 채권종목정보 필터링
     * 조건: TR코드가 TRDESP50101 + 당일 영업일 + 상장여부=Y + 채권유형=GB(국채)
     * → 조건 충족 시 7181 FIFO로 전달
     */
    CO_A001_RDS01 *dat = (CO_A001_RDS01 *)p_buf;

    if ((memcmp(dat->tr_code,          "TRDESP50101", 11) == 0) &&     /* TR코드: 채권종목정보 */
            (memcmp(dat->biz_date,          d_time,         8) == 0) &&    /* 영업일자: 당일 */
            (memcmp(dat->bond_Ipo_gbn_cd,   "Y",            1) == 0) &&    /* 상장여부: Y(상장) */
            (memcmp(dat->bond_type_cd,      "GB",           2) == 0)  )    /* 채권유형: GB(국채) */
#elif defined A7802
    /*
     * [A7802] RDS KTS종목정보 필터링
     * 조건: TR코드가 TRDESP50102 + 당일 영업일 + 보드ID=G1(정규장) + 상장여부=Y
     * → 조건 충족 시 7181 FIFO로 전달
     */
    CO_A001_RDS02 *dat = (CO_A001_RDS02 *)p_buf;

    if ((memcmp(dat->tr_code,          "TRDESP50102", 11) == 0) &&     /* TR코드: KTS종목정보 */
            (memcmp(dat->biz_date,          d_time,         8) == 0) &&    /* 영업일자: 당일 */
            (memcmp(dat->board_id,          "G1",           2) == 0) &&    /* 보드ID: G1(정규장) */
            (memcmp(dat->trade_susp_yn,     "Y",            1) == 0)  )    /* 상장여부: Y(거래가능) */
#elif defined A7802
    /* 기타 빌드 옵션: 무조건 통과 */
    if (1)
#endif
        {
        /* 조건 충족: 데이터를 공백으로 초기화 후 원본 복사, FIFO에 기록 */
        memset(W_Fmt, 0x20, sizeof (W_Fmt));
        memcpy(&W_Fmt, &R_Fmt[0], sizeof (FILE_BUFF_FORMAT)-1);
        W_Fmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

        rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);  /* 대상 FIFO에 1건 기록 */
        if (rt != 1) {
            /* 쓰기 실패: 오류 로그 후 프로세스 종료 */
            Log(SAM_FATAL, "file write fail[%s:%d]", OFN(D_K,P_K,0), rt);
            Exit_Process();
        }
    }

    return;
}   /* End of Set_Sise () */

/*************************************************************************
    End of program (pa_7800_dd.c)
*************************************************************************/
