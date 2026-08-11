#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : 장운영정보 처리
#   File    : pa_1600_mp.c
#
#   설명  : KRX에서 수신한 장운영정보(TTRMIP31301)를 FIFO에서 읽어
#             상품ID별로 필터링 후, 보드ID/보드이벤트 정보를
#             SHM(Shm_Risk)에 기록하는 프로세스.
#             장개시/장마감/임시정지 등 시장 상태를 관리한다.
#             A1601=채권(KTS), A2601=금융파생(국채선물/통화선물 등).
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

#define     DATA_SIZE       200
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DATA_TIME       60 * 1000           /* Poll 대기 시간 (60초, 밀리초 단위) */
#define     READ_MAX        1                   /* FIFO에서 한 번에 읽을 최대 건수 */

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
/*
 * 상품ID 매핑 (파생상품):
 *   유가증권 : STK, 코스닥 : KSQ, 파생 : FB3/FB5 등
 */
int                 R_Cnt;                      /* FIFO 읽기 건수 */
FILE_BUFF_FORMAT    R_Fmt[READ_MAX], W_Fmt;     /* 읽기/쓰기 버퍼 */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_1600_MP(void);
void    Analyze_Data(void);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 장운영정보 처리 루프 실행.           */
/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);                     /* 프로세스 초기화 */
    PA_1600_MP();                              /* 메인 처리 루프 */
    Exit_Process();                            /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PA_1600_MP: 메인 처리 루프                                            */
/*    1) FIFO에서 장운영정보 데이터 읽기                                */
/*    2) Analyze_Data()로 장운영 상태 분석 및 SHM 갱신                 */
/*    3) 데이터가 없으면 Poll_File()로 60초 대기                       */
/*----------------------------------------------------------------------*/
void    PA_1600_MP(void) {
    int     rt, i;
    char    m_time[24];
    char    O_Buff[2048], msgnum[100];
    char    w_fmt[6000];

    /* === 외부 루프: 프로세스 종료 신호(JOB_END)까지 반복 === */
    while (START_S != JOB_END) {
        Stat_Save();                           /* 프로세스 상태 저장 */

        /* === 내부 루프: FIFO에 데이터가 있는 동안 계속 읽음 === */
        while (START_S != JOB_END) {
            Stat_Save();

            memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * READ_MAX);
            R_Cnt = F_R(PS_R_1, (void *)R_Fmt, READ_MAX);

            if (R_Cnt < 0) {
                /* 읽기 실패: 오류 로그 후 프로세스 종료 */
                Log(SAM_FATAL, "cannot read File[%s,%d:%s]",
                        IFN(D_K,P_K,0), SYS_NO, SYS_STR);
                sleep(1);
                Exit_Process();
            }
            else if (R_Cnt == 0)
                break;                          /* 읽을 데이터 없음 → Poll 대기 */

            Log(USR_OK, "RD [%s:%d][%d]",
                    IFN(D_K,P_K,0), R_Cnt, IFR(D_K,P_K,0,0));
            Analyze_Data();                    /* 장운영 데이터 분석 및 SHM 갱신 */
        }

        /* FIFO에 데이터가 없으면 최대 60초간 대기 */
        rt = Poll_File(DATA_TIME);

        if (rt == 1) {
            Log(USR_OK, "poll timeout <%d>", OFW(D_K,P_K,0,0));
        }
        else if (rt == -1)
            continue;                           /* 오류 → 다음 루프 */
    }
}   /* End of PA_1600_MP () */

/*************************************************************************
    Function        : Analyze_Data
    Parameters IN   : (없음, 전역변수 R_Fmt/R_Cnt 사용)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 장운영정보(TTRMIP31301) 데이터를 분석하여
                      상품ID별로 필터링 후 보드ID/이벤트를 SHM에 기록.

    장운영정보 TR 목록:
      TTRMIP31301 : 장운영정보 (91바이트, 현/파/채)
      TTRMIP31304 : 종목마감 (105바이트, 현/파/채)
      TTRMIP31307 : VI(변동성완화장치) (117바이트, 현물)
      TCHEDP99001 : 장운영인터페이스종료 (153바이트)

    데이터 오프셋:
      InBuff[11]  : TR코드 (11바이트)
      InBuff[24]  : 상품ID (3바이트: KTS, FB3 등)
      InBuff[27]  : 보드ID (2바이트: G1=정규장, AL=전체 등)
      InBuff[29]  : 보드이벤트 (3바이트: AA1=단일가개시, BB1=매매개시 등)

    보드ID/이벤트 주요 조합:
      G1 AA1 : 시가단일가개시 (08:30)
      G1 BB1 : 매매거래개시 (09:00)
      G1 BC1 : 종가단일가개시 (15:20)
      G1 AC2 : 종가단일가마감 (15:30) → 장종료
      AL AE8 : 시장 임시정지 → 장종료
      AL AH8 : 시장 호가접수정지 → 장종료
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Analyze_Data(void) {
    int     i, rt, mk_gbn;                      /* mk_gbn: 시장 구분 인덱스 */
    char    InBuff[DATA_SIZE];
    char    m_time[24];

    memset(InBuff, 0, sizeof(InBuff));

    memcpy(InBuff, R_Fmt[0].Data, strlen(R_Fmt[0].Data));
    Log(USR_OK, "Input Data [%s][%d]", InBuff, strlen(InBuff));

    /* TTRMIP31301 (장운영정보)만 처리 */
    if (memcmp(&InBuff[11], "TTRMIP31301", 11) == 0) {
        /*
         * [Step 1] 상품ID(InBuff[24])로 mk_gbn(시장구분) 결정
         *   해당하지 않는 상품ID는 무시하고 리턴
         */
#if defined A1601
        /*
         * [A1601 채권] 상품ID 필터링
         *   KTS(국채) → mk_gbn=0, 그 외 무시
         *   (BND:채권, SMB:소액채권, RPO:레포는 미처리)
         */
        if (memcmp(&InBuff[24], "KTS", 3) == 0)
            mk_gbn = 0;
        else {
            Set_TR_Time();
            INT_SEQ ++;
            Add_Count(PS_R_1, 1);
            return;
        }
#elif defined A2601
        /*
         * [A2601 금융파생] 상품ID 필터링
         *   FB3=3년국채선물, FB5=5년국채선물, FEU=유로선물,
         *   FUD=미국달러선물, FYN=엔선물, FBA=10년국채선물,
         *   FBL=30년국채선물, XUD=미국달러플렉스선물
         *   → 모두 mk_gbn=1 (금융파생 단일시장으로 처리)
         */
        if ((memcmp(&InBuff[24], "FB3", 3) == 0) ||
                (memcmp(&InBuff[24], "FB5", 3) == 0) ||
                (memcmp(&InBuff[24], "FEU", 3) == 0) ||
                (memcmp(&InBuff[24], "FUD", 3) == 0) ||
                (memcmp(&InBuff[24], "FYN", 3) == 0) ||
                (memcmp(&InBuff[24], "FBA", 3) == 0) ||
                (memcmp(&InBuff[24], "FBL", 3) == 0) ||
                (memcmp(&InBuff[24], "XUD", 3) == 0)) {
            mk_gbn = 1;
        }
        else {
            Set_TR_Time();
            INT_SEQ ++;
            Add_Count(PS_R_1, 1);
            return;
        }
#endif

        /*
         * [Step 2] 보드ID/이벤트 필터링 및 SHM 기록
         *   G1(정규장), G2(시간외), AL+AE8(임시정지), AL+AF8(주식CB중단)만 처리
         */
        if ((memcmp(&InBuff[27], "G1", 2) == 0) ||
                (memcmp(&InBuff[27], "G2", 2) == 0) ||
                ((memcmp(&InBuff[27], "AL", 2) == 0) &&
                (memcmp(&InBuff[29], "AE8", 3) == 0)) ||
                ((memcmp(&InBuff[27], "AL", 2) == 0) &&
                (memcmp(&InBuff[29], "AF8", 3) == 0))) {
            /*
             * G1+AA1(단일가개시)는 주식선물스프레드(FST/FKQ)만 처리
             * 다른 시장의 단일가개시 처리를 방지
             */
            if ((memcmp(&InBuff[29], "AA1", 3) == 0) &&
                    (memcmp(&InBuff[24], "FST", 3) != 0 && memcmp(&InBuff[24], "FKQ", 3) != 0)) {
                Set_TR_Time();
                INT_SEQ ++;
                Add_Count(PS_R_1, 1);
                return;
            }

            /* SHM에 보드ID(2바이트) 기록 */
            memcpy(Shm_Risk[0].open_market_info[mk_gbn], &InBuff[27], 2);
            /* SHM에 보드이벤트(3바이트) 기록 */
            memcpy(Shm_Risk[0].sub_market_info[mk_gbn], &InBuff[29], 3);

            Log(USR_OK, "Board ID [%2.2s][%2.2s] [%3.3s][%3.3s] mk_gbn[%d]",
                    Shm_Risk[0].open_market_info[mk_gbn], &InBuff[27],
                    Shm_Risk[0].sub_market_info[mk_gbn], &InBuff[29], mk_gbn);

            /*
             * [Step 3] 장마감 판정
             *   G1+AC2(종가단일가마감) = 정규장 종료
             *   G1+AB2(장종료) = 장종료
             *   AB2 = 장종료 (현물 15:30, 파생 15:45)
             *   → 보드ID를 "GE"(장종료)로 덮어쓰기
             */
            if (((memcmp(&InBuff[27], "G1", 2) == 0) &&
                    (memcmp(&InBuff[29], "AC2", 3) == 0)) ||
                    ((memcmp(&InBuff[27], "G1", 2) == 0) &&
                    (memcmp(&InBuff[29], "AB2", 3) == 0)) ||
                    (memcmp(&InBuff[29], "AB2", 3) == 0)) {
                memcpy(Shm_Risk[0].open_market_info[mk_gbn], "GE", 2);
            }
            /*
             * [Step 4] 장중 임시정지 (KTS)
             *   AL+AE8(시장임시정지) 또는 AL+AH8(호가접수정지)
             *   → 보드ID를 "GE"(장종료)로 설정
             */
            else if (((memcmp(&InBuff[27], "AL", 2) == 0) &&
                    (memcmp(&InBuff[29], "AE8", 3) == 0)) ||
                    ((memcmp(&InBuff[27], "AL", 2) == 0) &&
                    (memcmp(&InBuff[29], "AH8", 3) == 0))) {
                memcpy(Shm_Risk[0].open_market_info[mk_gbn], "GE", 2);
            }
            /* 그 외(G1/AL 등): 위에서 기록한 보드ID/이벤트 유지 */
        }
    }

    Set_TR_Time();                             /* TR 시간 갱신 */
    INT_SEQ ++;                                 /* 내부 시퀀스 번호 증가 */
    Add_Count(PS_R_1, 1);                       /* 처리 건수 카운터 증가 */

    return;
}   /* End of Analyze_Data () */

/*************************************************************************
    End of Program (pa_1600_mp.c)
*************************************************************************/
