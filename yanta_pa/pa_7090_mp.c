#define		_GLOBAL
/*--------------------------------------------------------
#	Module	: MAR 선물거래(mar) 
#	File	: pa_7090_mp.c
--------------------------------------------------------*/
/*------------------------------------------------------*/
/*------------------------------------------------------*/

/*--------------------------------------------------------
	Header Files
--------------------------------------------------------*/
#include <pthread.h>
#include <libgen.h>
#include "fep_fepp.h"
#include "pa_struct.h"
#include "arb.h"
#include "config.h"
#include "l_def.h"
#include "sem.h"

#define		DATA_SIZE	  2048
#define     MAR_STOP_CD   99
#include 	"buf_struct.h"
#include	"shm_memory.h"

/* 외부 소스 */
#ifndef _OMS_SOURCE_
#define	_OMS_SOURCE_	1
#endif /* _OMS_SOURCE_ */
/*--------------------------------------------------------
	Constants and Structures
--------------------------------------------------------*/
#define	 FIFO_EVENT   0
#define	 FILE_EVENT	  1

#define	 TCP_TIME_OUT 30
#define	 DATA_TIME	  60 * 1000
#define	 READ_MAX	  1

#ifdef  SAM_USE
#define  WR_CNT   W_CNT(0,0)
#define  RD_CNT   R_CNT(0,0)
#else
#define  WR_CNT   IDW_CNT(0,0)
#define  RD_CNT   IDR_CNT(0,0)
#endif

/*--------------------------------------------------------
	Global Variables
--------------------------------------------------------*/
extern	void Sub_SHM (void);
FILE_BUFF_FORMAT   W_Fmt, R_Fmt[READ_MAX];
struct  pollfd	   Poll[5];
int		PollCnt;
int		Pk_mp[2];
char	ApType[10], Cli_handler[5];
char	W_DFmt[1024];
char	Order_St[1024];

int		OD_SEQ;
int		Set_Flag;
int     succ_flag=0;

pthread_t    tid[2];
int          thr_usr2flg=0, extflg=0, sem_id=0;
struct sigaction thr_sigact;

SharedRoot  *g_shared;
int          g_strat_idx   = 0;
int          g_set_idx     = 0;       // 현재 운용중인 세트 인덱스
int          g_test_mode   = 0;
int          g_initialized = 0;       // arb_started gubn
int          g_log_level   = 0xff;
char         g_progname[64]= "unknwon";
char        *pname="mar_main";

Config       g_cfg;
SHM_FIN_FUT *g_md_shm_fut;
FutOrdNo    *g_shm_oid_fut;           // 선물주문번호 Shm
RISK        *g_md_shm_spot;           // 현물시세 Read 용
SHM_FX      *shm_fx;                  // 현물시세 auto_use 용

/*--------------------------------------------------------
	Function Prototypes
--------------------------------------------------------*/
void	l_mar_main(void);
int		l_mar_init_param(void);
int 	l_mar_set_start (void);	// 500200(기동Setting);
void	l_mar_fifo_event(void);
void    l_mar_data_event(void);
int		l_mar_proc();
void    l_mar_term(int);
void    l_mar_pop_fifo(int idx);
void   *l_wrk_dat_proc();

/*--------------------------------------------------------*/
int		main(int argc, char *argv[])
/*--------------------------------------------------------*/
{
	Init_Proc(argc, argv);
	strcpy(g_progname, basename(argv[0]));
	l_mar_main();

}	/* End of main */

void
l_thr_sighandler(int signo)
{
    thr_usr2flg = 1;
}

/*--------------------------------------------------------*/
void	l_mar_main(void)
/*--------------------------------------------------------*/
{
	int		i, j, rt, rc;
	
	Set_Flag = 0;
	rt = l_mar_init_param();
	if (rt != OK)
	{
		l_mar_term(9);
		sleep(1);
	}
	
	PollCnt = 4;
    thr_sigact.sa_flags   = 0;
    thr_sigact.sa_handler = l_thr_sighandler;
    sigaction(SIGUSR1, &thr_sigact, NULL);

    pthread_create(&tid[0], NULL, l_wrk_dat_proc, NULL);

	while (START_S < JOB_END)
	{
		Stat_Save();

		while (1)
		{
			if (WR_CNT > RD_CNT)
			{
				l_mar_data_event();		// 응답/체결 처리
				continue;
			}
			else
				break;
		}
		rt = poll(Poll, PollCnt, 5000);
		if (rt > 0)
		{
			for (i = 0; i < PollCnt; i++)
			{	
				if (Poll[i].revents & POLLIN)
				{
					Poll[i].revents = 0;
					break;
				}
				
				if (Poll[i].revents & POLLHUP)
				{
					SLog(SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
					continue;
				}
		
			}
		}
		else
		{
			if (rt == 0)
			{
				rc = l_mar_proc();
			    if (rc == ARB_STOP_CD)
				{
					g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ] = 0;
					shm_fx[G_STRAT->spot_idx].auto_use[OD_SEQ] = 0;
				 	l_dbg(L_DBG, "<%s:%d> MAR 강제 전략 종료 .. rc=[%d]", __FUNCTION__, __LINE__, rc);
				 	l_mar_term(9);
				}
				continue;
			}
			else
			{
				if (SYS_NO == EINTR)
					SLog(SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
				else
					SLog(SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);
				
				continue;
			}
		}
		
		switch (i)
		{
			case FIFO_EVENT:
				l_mar_fifo_event();
				break;

			case FILE_EVENT:			//File Event, 응답/체결처리, 설정/종료수신등
				while(START_S < JOB_END)
				{
					if (WR_CNT > RD_CNT)
					{
						l_mar_data_event();
						continue;
					}
					else
						break;
				}
				break;

			case 2:			
				rc = l_mar_proc();
				l_mar_pop_fifo(2); // 데이터 Read
				if (rc == MAR_STOP_CD)
				{
					g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ] = 0;
					shm_fx[G_STRAT->spot_idx].auto_use[OD_SEQ] = 0;
				 	l_dbg(L_DBG, "<%s:%d> MAR 강제 전략 종료 .. rc=[%d]", __FUNCTION__, __LINE__, rc);
				 	l_mar_term(9);
				}
				break;

			case 3:		
				rc = l_mar_proc();
				l_mar_pop_fifo(3); // 데이터 Read
				if (rc == MAR_STOP_CD)
				{
				 	l_dbg(L_DBG, "<%s:%d> MAR 강제 전략 종료 .. rc=[%d]", __FUNCTION__, __LINE__, rc);
				 	l_mar_term(9);
				}
				break;

			default:
				SLog(USR_ERROR, "event case error[%d,%d]", i, PollCnt);
				return;
				break;
		}
	}
	
}	/* End of PA_7090_MP () */

/*--------------------------------------------------------*/
void	l_mar_data_event(void)
/*--------------------------------------------------------*/
{
	int    rt, R_Cnt;
	char   tmp[128];
	int    fut_side = 0;
	int    fut_ord_id = 0, fut_fill_qty = 0;
	double fut_fill_px = 0;
	int    rc = 0;

	R_Cnt = DSHM_R(PS_R_1, (void *)R_Fmt, 1);
	if (R_Cnt < 0)
	{
		SLog(SAM_FATAL, "[MAR] cannot read file[%s,%d:%s] W[%d]R[%d]", IFN(D_K,P_K,0), SYS_NO, SYS_STR, WRITE_CNT, RD_CNT);
		sleep(1);
		Exit_Process();
	}
	else if (R_Cnt == 0)
		return;
	
	Dshm_Add_Count(TS_W1_1, 1);
	
	if (memcmp(R_Fmt[0].Data, "500100", 6) == 0)             // 자동기동 Setting
	{		   
		int ret;
		l_dbg(L_DBG, "<%s:%d> 거래전략구동 Data=[%.2048s]", __FUNCTION__, __LINE__, (char *)R_Fmt[0].Data);
		ret = l_mar_set_start();
		if (ret < 0) {
			l_dbg(L_DBG, "<%s:%d> l_mar_set_start failed!!! ret[%d]", __FUNCTION__, __LINE__, ret); 
			l_mar_term(9);
		}
	}
	else if (memcmp(R_Fmt[0].Data, "500300", 6) == 0)		// 자동종료요청
	{
		if (succ_flag == 1) {                               // Arb_Init 정상처리시에만 call
			g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ] = 0;
			shm_fx[G_STRAT->spot_idx].auto_use[OD_SEQ] = 0;
		} 
		l_dbg(L_DBG, "<%s:%d> 자동종료요청 수신.. succ_flag[%d] auto_use[%d]", 
			__FUNCTION__, __LINE__, succ_flag, g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ]);

		l_mar_term(9);

	}
	else if (memcmp(R_Fmt[0].Data, "100120", 6) == 0) // 응답(100120)
	{
		if (memcmp(R_Fmt[0].Data+6, "01", 2) == 0)    // 시장구분 - '01' 선물(파생) 
		{
			YSK_DATA_HEAD *yhdr = (YSK_DATA_HEAD *)&R_Fmt[0].Data[20];             //OMS헤더(20byte)
			KRX_SETTLE_RESP_DATA *i = (KRX_SETTLE_RESP_DATA *)&R_Fmt[0].Data[120]; //OMS헤더(20byte) + YSK 헤더(100byte)

			l_dbg(L_DBG, "<%s:%d> 선물 주문응답전문 : [%.*s][%.318s] sRpCode[%.4s]", __FUNCTION__, __LINE__, (int)sizeof(i->TrCode), i->TrCode, (char *)i, yhdr->sRpCode);
			if (memcmp(i->TrCode, "TTRODP11321", 11) == 0) // TTRODP11321 거부 
			{
				fut_ord_id = l_stoi(i->OrderNo, sizeof(i->OrderNo)); // 주문번호
				l_dbg(L_DBG, "<%s:%d> 선물 주문거부응답 fut_ord_id [%ld]", __FUNCTION__, __LINE__, fut_ord_id);

				rc = l_mar_fut_exec(-1, 0, fut_ord_id, 0, 0, yhdr->sRpCode); 
				if (rc == ARB_STOP_CD)
				{
					g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ] = 0;
					shm_fx[G_STRAT->spot_idx].auto_use[OD_SEQ] = 0;
					l_dbg(L_DBG, "<%s:%d> l_mar_fut_exec: 강제 전략 종료 .. rc=[%d]", __FUNCTION__, __LINE__, rc);
					l_mar_term(9);
				}
			}
			else if (memcmp(i->TrCode, "TTRODP11303", 11) == 0) // TTRODP11303 자동취소 
			{
				fut_ord_id = AtoIf(i->OrderNo, sizeof(i->OrderNo)); // 주문번호
				l_dbg(L_DBG, "<%s:%d> 선물 KRX Auto Cancel 응답 fut_ord_id [%ld]", __FUNCTION__, __LINE__, fut_ord_id);
				rc = l_mar_fut_exec(-2, 0, fut_ord_id, 0, 0, yhdr->sRpCode); 
				if (rc == ARB_STOP_CD)
				{
					g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ] = 0;
					shm_fx[G_STRAT->spot_idx].auto_use[OD_SEQ] = 0;
					l_dbg(L_DBG, "<%s:%d> l_mar_fut_exec: 강제 전략 종료 .. rc=[%d]", __FUNCTION__, __LINE__, rc);
					l_mar_term(9);
				}
			}
			else if (memcmp(i->TrCode, "TTRODP11301", 11) == 0) // TTRODP11301 주문확인 
			{
				fut_ord_id = AtoIf(i->OrderNo, sizeof(i->OrderNo)); // 주문번호
				l_dbg(L_DBG, "<%s:%d> 선물 KRX 응답 fut_ord_id [%ld]", __FUNCTION__, __LINE__, fut_ord_id);
				rc = l_mar_fut_exec(2, 0, fut_ord_id, 0, 0, yhdr->sRpCode); 
				if (rc == ARB_STOP_CD)
				{
					g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ] = 0;
					shm_fx[G_STRAT->spot_idx].auto_use[OD_SEQ] = 0;
					l_dbg(L_DBG, "<%s:%d> l_mar_fut_exec: 강제 전략 종료 .. rc=[%d]", __FUNCTION__, __LINE__, rc);
					l_mar_term(9);
				}
			} else {
				;
			}

			/*
			mar_stop_msg_send(E5201_MSG);
			g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ] = 0;
			shm_fx[G_STRAT->spot_idx].auto_use[OD_SEQ] = 0;
			l_dbg(L_DBG, "OMS Reject 강제 전략 종료 .. ");
			l_mar_term(9);
			*/
		} else {
			l_dbg(L_DBG, "<%s:%d> CHECK!!! OTHER CASE!!! [%s]", __FUNCTION__, __LINE__, R_Fmt[0].Data);
		}
	}
	else if (memcmp(R_Fmt[0].Data, "100140", 6) == 0)// 체결(100140)
	{
		if (memcmp(R_Fmt[0].Data+6, "01", 2) == 0) // 시장구분 - '01' 선물(파생) 
		{
			KRX_SETTLE_DATA *i = (KRX_SETTLE_DATA *)&R_Fmt[0].Data[120]; // OMS헤더(20byte) + YSK 헤더(100byte)
			l_dbg(L_DBG, "<%s:%d> 선물 체결 전문 : [%.*s]", __FUNCTION__, __LINE__, sizeof(KRX_SETTLE_DATA), (char*)i);
			if (memcmp(i->TrCode, "TTRTDP21301", 11) == 0) // TTRTDP21301 (체결결과) 
			{
				fut_ord_id   = l_stoi(i->OrderNo, sizeof(i->OrderNo));               // 주문번호
			    fut_side	 = l_stoi(i->TradeFlag, sizeof(i->TradeFlag));           // 매도매수구분코드 - 1:Sell, 2.Buy 
				fut_fill_qty = l_stoi(i->Trading_Volumn, sizeof(i->Trading_Volumn)); // 체결수량 - 계약단위
				fut_fill_px  = AtoDf(i->Trading_price, sizeof(i->Trading_price));    // 체결가격				
				rc = l_mar_fut_exec(1, fut_side, fut_ord_id, fut_fill_qty, fut_fill_px, "0000");
				if (rc == ARB_STOP_CD)
				{
					g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ] = 0;
					shm_fx[G_STRAT->spot_idx].auto_use[OD_SEQ] = 0;
					l_dbg(L_DBG, "<%s:%d> l_mar_fut_exec: 강제 전략 종료.. 선물 주문 거부 rc=[%d]", __FUNCTION__, __LINE__, rc);
					l_mar_term(9);
				}

			} else {
				l_dbg(L_ERR, "<%s:%d> INCORRECT CHECK CASE!!! TrCode [%.*s] rc=[%d]", __FUNCTION__, __LINE__, sizeof(i->TrCode), i->TrCode, rc);
			}
		} else {
			l_dbg(L_ERR, "<%s:%d> INCORRECT CHECK CASE!!! [%s]", __FUNCTION__, __LINE__, R_Fmt[0].Data); 
		}
	}
	else
	{
		l_dbg(L_ERR, "<%s:%d> Other Tr Code Check Plz[%20.20s]", __FUNCTION__, __LINE__, R_Fmt[0].Data);
	}
	
	while(1)
	{
		rt = read(INPUT_FD, tmp, sizeof(tmp));
#if defined __linux
		if (rt == 0 || errno == EAGAIN)
#else
		if (rt == 0)
#endif
			break;
	}
	
	return;
}	/* l_mar_data_event() */

/*--------------------------------------------------------*/
int		l_mar_init_param()
/*--------------------------------------------------------*/
{
	char 	fifo_name[256];
	int 	return_flag = 0;
	int 	i, rt, shmsz, shmid, first=0;
	key_t   k;
	
	/* attach daemon SHM (INFO) : ALL_DAEMON_INFO */
    Sub_SHM();

	Log(USR_OK, "l_mar_init_param...MAR");
	
	Poll[0].fd     = START_FD;		// of Daemon
	Poll[0].events = POLLIN;
	Poll[1].fd     = INPUT_FD; 		// of SelfFile
	Poll[1].events = POLLIN;

	/* 선물 */
	memset(fifo_name, 0, sizeof(fifo_name));
	sprintf(fifo_name, "%s/PA/pa_75%5.5s1", _FEP_FIFO, _Exe_Name+5);  // 선물시세수신

	Poll[2].fd = open(fifo_name, O_RDWR|O_NDELAY);
	Poll[2].events = POLLIN;
	Log(USR_OK, "_Exe_Name [%s] fifo_name[%s] fd[%d]", _Exe_Name, fifo_name, Poll[2].fd);

	if (Poll[2].fd < 0)
	{
		l_dbg(L_ERR, "<%s:%d> cannot open i{%d] FIFO[%s][%d][%d:%s]",
			__FUNCTION__, __LINE__, i, fifo_name, Poll[2].fd, SYS_NO, SYS_STR);

		SLog(FIF_FATAL, "<%s:%d> cannot open i{%d] FIFO[%s][%d][%d:%s]",
			__FUNCTION__, __LINE__, i, fifo_name, Poll[2].fd, SYS_NO, SYS_STR);
		return_flag = -1;
	}
	
	// 기동시 2로 변경
	// 종료시 0으로 변경해야함
	PROC(D_K,P_K).tr_seq = 2; // tr_seq : 시장구분 - 1:채권, 2:선물, 3:채권/선물 둘다 수신필요시

	/* 현물 */
	memset(fifo_name, 0, sizeof(fifo_name));
	sprintf(fifo_name, "%s/PA/pa_7402_ur1", _FEP_FIFO);  // 현물시세수신
	sprintf(fifo_name, "%s/PA/pa_75%4.4s11", _FEP_FIFO, _Exe_Name+5);

	OD_SEQ = (AtoIf (_Exe_Name+4, 2) - 1) * 10;
    OD_SEQ =  AtoIf (_Exe_Name+6, 2) + OD_SEQ - 1;	

	Poll[3].fd = open(fifo_name, O_RDWR|O_NDELAY);
	Poll[3].events = POLLIN;
	Log(USR_OK, "_Exe_Name [%s] fifo_name[%s] fd[%d]", _Exe_Name, fifo_name, Poll[3].fd);

	if (Poll[3].fd < 0)
	{
		SLog(FIF_FATAL, "<%s:%d> cannot open i{%d] FIFO[%s][%d][%d:%s]",
			__FUNCTION__, __LINE__, i, fifo_name, Poll[3].fd, SYS_NO, SYS_STR);
		return_flag = -1;
	}

	/* 전략 전달용 시그널 설정 */
	for (i=0; i < 4; i++) {
		Log(USR_OK, "    IDX[%d] FD[%d]", i, Poll[i].fd);
	}

	if (TIME_OUT == 0) TIME_OUT = TCP_TIME_OUT;
	
	memset(ApType, 0, sizeof(ApType));
	sprintf(ApType, "%-2.2s%-5.5s", _Exe_Name, _Exe_Name+3);

	LtoU(ApType, strlen(ApType));
	Log(USR_OK, "---> ApType=[%s]", ApType);
	
	/* Process 증가될때마다 수정검증을 해야됨. */
	OD_SEQ = ((AtoIf(&ApType[3], 2) -1) * 10) + AtoIf(&ApType[5], 2) -1;
	Log(USR_OK, "OD_SEQ=[%d]", OD_SEQ);

	// 2025114 추가
	Log(USR_OK, "1 WR_CNT [%d] RD_CNT[%d]", WR_CNT, RD_CNT);
	if (WR_CNT > RD_CNT) RD_CNT = WR_CNT -1;

	Log(USR_OK, "2 WR_CNT [%d] RD_CNT[%d]", WR_CNT, RD_CNT);

    // config
    if (load_config("/fsfxwin/fep/mar/conf/config.ini") != 0) {
        SLog(FIF_FATAL, "config.ini load failed!\n");
        return(-1);
    }

    g_cfg.strat_shm_size = sizeof(SharedRoot);
	if (g_cfg.strat_shm_key == 0x00) {
        SLog(FIF_FATAL, "config.ini DATA CHECK strat_shm_key[%x]!\n", g_cfg.strat_shm_key);
        return(-1);
	}

    // 1. 전략 공유 메모리 세그먼트 생성 (이미 있으면 기존 것 사용)
    shmid = shmget(g_cfg.strat_shm_key, 0, 0666);
    if (shmid < 0)
    {
    	shmid = shmget(g_cfg.strat_shm_key, sizeof(SharedRoot), IPC_CREAT|0666);
		if (shmid > 0) {
			first = 1;

		} else {
			SLog(FIF_FATAL, "shmget failed!!! key[%x] shmid[%d] shmsz[%d] errno[%d]\n", g_cfg.strat_shm_key, shmid, sizeof(SharedRoot), errno);
			return(-2);
		}
    }

    // 2. 전략 공유 메모리를 프로세스 주소 공간에 attach
    g_shared = (SharedRoot *)shmat(shmid, NULL, 0);
    if (g_shared == (void *)-1) {
        SLog(FIF_FATAL, "shmat failed!!! key[%x] shmid[%d] shmsz[%d] errno[%d]\n", g_cfg.strat_shm_key, shmid, sizeof(SharedRoot), errno);
        return(-3);
    }

    SLog(USR_OK, "====> SIZE[%ld] key[%x] first[%d]", g_cfg.strat_shm_size, g_cfg.strat_shm_key, first);
	if (first) {
		memset(g_shared, 0x00, sizeof(SharedRoot));
	}

	// 3. 선물 주문번호 공유메모리 세마포어 생성 및 attach 
    first = 0;
	shmsz = sizeof(FutOrdNo);
    g_cfg.fut_oid_shm_size = g_cfg.fut_oid_shm_size*shmsz;
    
	if (g_cfg.fut_oid_shm_key == 0x00) {
        SLog(FIF_FATAL, "config.ini DATA CHECK fut_oid_shm_key[%x]!\n", g_cfg.fut_oid_shm_key);
        return(-1);
	}

    shmid = shmget(g_cfg.fut_oid_shm_key, 0, 0666);
    if (shmid < 0)
    {
    	shmid = shmget(g_cfg.fut_oid_shm_key, sizeof(FutOrdNo), IPC_CREAT|0666);
		if (shmid > 0) {
			first = 1;

		} else {
			SLog(FIF_FATAL, "shmget failed!!! key[%x] shmid[%d] shmsz[%d] errno[%d]\n", g_cfg.fut_oid_shm_key, shmid, sizeof(FutOrdNo), errno);
			return(-4);
		}
    }
    
	g_shm_oid_fut = (FutOrdNo *)shmat(shmid, NULL, 0);
    if (g_shm_oid_fut == (void *)-1) {
		SLog(FIF_FATAL, "shmat failed!!! key[%x] shmid[%d] shmsz[%d] errno[%d]\n", g_cfg.fut_oid_shm_key, shmid, sizeof(FutOrdNo), errno);
        return(-5);
    }

    SLog(USR_OK, "====> SIZE[%ld] key[%x] first[%d]", g_cfg.fut_oid_shm_size, g_cfg.fut_oid_shm_key, first);
	if (first) {
		memset(g_shm_oid_fut, 0x00, sizeof(FutOrdNo));
	}

    sem_id = Sem_Create(g_cfg.fut_oid_shm_key);
    if (sem_id < 0) 
    {
        SLog(FIF_FATAL, "FutOrdNo Sem_Create error.");
        return(-6);
	}
    SLog(USR_OK, "====> !!!SIZE[%ld] key[%x] first[%d] sem_id[%d]", g_cfg.fut_oid_shm_size, g_cfg.fut_oid_shm_key, first, sem_id); 

	g_cfg.krx_day_order_id = l_que_create(g_cfg.krx_order_day_msg_key);
    if (g_cfg.krx_day_order_id == -1)
    {
        SLog(FIF_FATAL, "KRX 선물 주간 주문 msgque init 실패 !!");
        return(-12);
    }

    // 메시지 큐 init
	// MSG_TYPE_FUT_ORDLOG
	{
	key_t key;
	key = l_ftok(g_cfg.orderlogq_fut_msg_key_text, 'Q'); 
    g_cfg.fut_ordlog_id = l_que_create(key);
	}
    if (g_cfg.fut_ordlog_id == -1)
    {
        SLog(FIF_FATAL, "[INIT] 선물 주문 후처리 msgque init 실패 !!");
        return(-12);
    }

	/* 선물시세 Shm Attach  --------------------------------------------------------------*/
	shmsz = sizeof(SHM_FIN_FUT) * SHM_MAX_DEV_FIF;   // 1,000
    if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
        k = 0x01000000L;
    else
        k = 0x00000000L; 
    
    // 1. 공유 메모리 세그먼트 생성 (이미 있으면 기존 것 사용)
    shmid = shmget(FF_SHM_KEY+k, shmsz, IPC_CREAT | 0666);
    if (shmid < 0)
    {
        SLog(FIF_FATAL, "선물시세 메모리 생성 . errrno[%d:%s] key[%x] shmsz[%d] k[%d]", errno, strerror(errno), FF_SHM_KEY+k, shmsz, k);
        return(-13);
    }

    // 2. 공유 메모리를 프로세스 주소 공간에 attach
    g_md_shm_fut = (SHM_FIN_FUT *)shmat(shmid, NULL, 0);
    if (g_md_shm_fut == (void *)-1) {
        SLog(FIF_FATAL, "현물시세 메모리 attach failed . errrno[%d:%s] shmid[%d]", errno, strerror(errno), shmid);
        return(-14);
    }

    /* 현물시세 Risk Shm Attach ----------------------------------------------------------------*/
    shmsz = sizeof(RISK) * 1;   // 1

    // 1. 공유 메모리 세그먼트 생성 - FX_SHM -> RISK_SHM 로 변경
    shmid = shmget(RISK_SHM_KEY+k, shmsz, IPC_CREAT | 0666);
    if (shmid < 0)
    {
        SLog(FIF_FATAL, "현물시세 메모리 생성. errrno[%d:%s] key[%x] shmsz[%d] k[%d]", errno, strerror(errno), RISK_SHM_KEY+k, shmsz, k);
        return(-16);
    }
    // 2. 공유 메모리를 프로세스 주소 공간에 attach
    g_md_shm_spot = (RISK *)shmat(shmid, NULL, 0);
    if (g_md_shm_spot == (void *)-1) {
        SLog(FIF_FATAL, "현물시세 메모리 attach failed . errrno[%d:%s] shmsz[%d]", errno, strerror(errno), shmsz);
        return(-17);
    }

     /* 현물시세 FX Shm Attach ----------------------------------------------------------------*/
    shmsz = sizeof(SHM_FX) * SHM_MAX_FX;   // 1

    // 1. 공유 메모리 세그먼트 생성 (이미 있으면 기존 것 사용)
    shmid = shmget(FX_SHM_KEY+k, shmsz, IPC_CREAT | 0666);
    if (shmid < 0)
    {
        SLog(FIF_FATAL, "현물시세 메모리 생성. errrno[%d:%s] key[%x] shmsz[%d] k[%d]", errno, strerror(errno), FX_SHM_KEY+k, shmsz, k);
        return(-15);
    }
    // 2. 공유 메모리를 프로세스 주소 공간에 attach
    shm_fx = (SHM_FX *)shmat(shmid, NULL, 0);
    if (shm_fx == (void *)-1) {
        SLog(FIF_FATAL, "현물시세 메모리 attach failed . errrno[%d:%s] shmsz[%d]", errno, strerror(errno), shmsz);
        return(-16);
    }

     /* 현물시세 FX Shm Attach ----------------------------------------------------------------*/
    shmsz = sizeof(SHM_FX) * SHM_MAX_FX;   // 1

    // 1. 공유 메모리 세그먼트 생성 (이미 있으면 기존 것 사용)
    shmid = shmget(FX_SHM_KEY+k, shmsz, IPC_CREAT | 0666);
    if (shmid < 0)
    {
        SLog(FIF_FATAL, "현물시세 메모리 생성. errrno[%d:%s] key[%x] shmsz[%d] k[%d]", errno, strerror(errno), FX_SHM_KEY+k, shmsz, k);
        return(-15);
    }
    // 2. 공유 메모리를 프로세스 주소 공간에 attach
    shm_fx = (SHM_FX *)shmat(shmid, NULL, 0);
    if (shm_fx == (void *)-1) {
        SLog(FIF_FATAL, "현물시세 메모리 attach failed . errrno[%d:%s] shmsz[%d]", errno, strerror(errno), shmsz);
        return(-16);
    }

    SLog(USR_OK, "shmkey[%x][%d] return!!! [%x]", FF_SHM_KEY+k, shmid, g_md_shm_fut);
	return(0);
}	/* End of l_mar_init_param() */

/*--------------------------------------------------------*/
int	l_mar_set_start(void)
/*--------------------------------------------------------*/
{
	int		i, rt, arry, shmid, shmsz;
	int     today = get_today();
	char 	fifo_name[256];
	char	err_no[10], head_size[10], arbmsg[16]="";
	IN_SAMPLE01	  *dat;
	STRATEGY_BODY *bd;
	
	dat = (IN_SAMPLE01 *)&R_Fmt[0].Data[sizeof(SEARCH_HEADER)];
	bd  = (STRATEGY_BODY *)dat;
	succ_flag = 0;

	memcpy(arbmsg, dat, 6);
    g_strat_idx = atoi(arbmsg) - 1; // 전략실행일련번호(전략인덱스)
	snprintf(l_dbgfile, sizeof(l_dbgfile), "%s/mar_%d", g_cfg.log_file, g_strat_idx);
    SLog(USR_OK, "===> g_strat_idx[%d]", g_strat_idx); 

	l_dbg(L_DBG, "<%s:%d> l_mar_set_start ... MAR ", __FUNCTION__, __LINE__);
	int ftrs_bsns_dt = AtoIf(bd->ftrs_bsns_dt, sizeof(bd->ftrs_bsns_dt));

	l_dbg(L_DBG, "START[%d]-[%d][%.*s] -------> [%s] g_strat_idx[%d][%s][%s]", 
		g_shared->today, ftrs_bsns_dt, 
		sizeof(bd->ftrs_bsns_dt), bd->ftrs_bsns_dt,
		l_dbgfile, g_strat_idx, arbmsg, dat);

    g_log_level = g_cfg.log_level;
	l_dbg(L_DBG, "===>dat: seq_no[%.*s] proc_id[%.*s] ftrs_items_cd[%.*s] ref_no[%.*s][%.*s]",
		sizeof(bd->seq_no), bd->seq_no,
		sizeof(bd->proc_id), bd->proc_id,
		sizeof(bd->ftrs_items_cd), bd->ftrs_items_cd,
		sizeof(bd->ref_no), bd->ref_no,
		sizeof(bd->ftrs_bsns_dt), bd->ftrs_bsns_dt);

    // 전략 메모리 초기화 (일자가 바뀌면 초기화)
    if (g_shared->today == 0 || g_shared->today != ftrs_bsns_dt) {
		memset(g_shared, 0x00, sizeof(SharedRoot));
		g_shared->today = get_today();
		l_dbg(L_DBG, "[INIT] 영업일 변경으로 전략 슬롯[%d]메모리 초기화 today[%d][%d]", g_strat_idx, today, g_shared->today);
	}
	memcpy(G_STRAT->aptype, ApType, strlen(ApType));
	G_STRAT->aptype[7] = '\0';
	
	l_dbg(L_DBG, "--->today[%d][%d]-[%x] [%d]", today, g_shared->today, G_OID_FUT, G_OID_FUT->ord_dt);

	// 당일 New 주문번호 채번
	if (G_OID_FUT->ord_dt == 0 || G_OID_FUT->ord_dt != today)
	{
		Sem_Lock(sem_id);
		l_dbg(L_DBG, "CHECK!!!--->today[%d][%d]-[%x] [%d]", today, g_shared->today, G_OID_FUT, G_OID_FUT->ord_dt);
	    G_OID_FUT->ord_dt     = get_today();	
	    G_OID_FUT->fut_ordno  = 0;
		Sem_Unlock(sem_id);
	}
	l_dbg(L_DBG, "--->today[%d][%d]", today, g_shared->today);

	// oid 채번 방식 -- 신규 채번 로직 확인 후 삭제 예정----------------------------------------------------
    /* 주문번호 대역 초기 셋팅 */
    G_ORDNO->fut_ordno = 0; // 전략인덱스별 주문번호 초기화
    //-------------------------------------------------------------------------------------------------------

	g_set_idx = G_ORDNO->set_id;
	g_initialized = 1;

	// 운영 모드
	if (l_mar_set_data((char *)dat) < 0) {
        l_dbg(L_DBG, "l_mar_set_data failed!!! errno[%d]", errno);
		if (mar_stop_msg_send(E9999_MSG) < 0) return(-12);
		return(-2);
	}

    // 전략에 해당하는 선물, 현물 종목 시세 수신처리 등록 및 접근
    KS_EXPCODE key; // 선물시세 key
    memset(&key, 0, sizeof(key));
    memcpy(key.expcode, G_STRAT->fut_sym, sizeof(key.expcode)); // key.expcode - 종목표준코드(12byte)
  
	/* 선물종목 Search */
    int tot_cnt = g_md_shm_fut[0].total_item_cnt; // total 종목 count
	l_dbg(L_DBG, "--->tot_cnt[%d]", tot_cnt); 
   
    //G_STRAT->fut_idx = fut_key_search(tot_cnt, (char*)&key); // DEV_MK_FIF 금융상품선물(국채,금리,통화)
	G_STRAT->fut_idx = Key_Search(DEV_MK_FIF, KEY_EXPCODE, (char *)&key);
    if (G_STRAT->fut_idx == -1)
    {
        l_dbg(L_DBG, "선물시세 fut_key_search fail!! (종목마스터 미수신 or 이상상태) [%.*s]", 12 , G_STRAT->fut_sym); 
		if (mar_stop_msg_send(E9999_MSG) < 0) return(-12);
		return(-2);
    }
    g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ] = 1;   // 선물 : 자동전략기동시 해당종목 + 설정(시세수신처리)

	/* 현물 종목 Search */
	/* Key_Search_FX(char *excode, char *symb) -> excode: 'J', 'N', 'S' */
	G_STRAT->spot_idx = Key_Search_FX("N", "USDKRW");
	//G_STRAT->spot_idx = Key_Search_FX("J", "USDKRO");
	shm_fx[G_STRAT->spot_idx].auto_use[OD_SEQ] = 1;        // 현물 : 자동전략기동시 해당종목 + 설정(시세수신처리)

	l_dbg(L_DBG, "<%s:%d> STRAT->fut_sym[%.12s] fut_idx[%d] spot_sym[%.12s] spot_idx[%d]", 
		__FUNCTION__, __LINE__, key.expcode, G_STRAT->fut_idx, "USDKRW", G_STRAT->spot_idx);
	succ_flag = 1;
	return(0);
}	/* End of l_mar_set_start */

/*----------------------------------------------------------------------*/
void    l_mar_fifo_event(void)
/*----------------------------------------------------------------------*/
{
    char tmp[2];
    read(START_FD, tmp, 1);
    return;
}   /* End of l_mar_fifo_event () */

/*----------------------------------------------------------------------*/ 
void    l_mar_term(int option) 
/*----------------------------------------------------------------------*/ 
{ 
	char	fifo_name[128];

    PROC(D_K,P_K).process_status = option;      /* 1:죽을시 재실행  9:kill */

	Log(USR_OK, "PROC(D_K,P_K).process_status = %d", PROC(D_K,P_K).process_status);
	PROC(D_K,P_K).tr_seq = 0;                   /* 시세 시그널 해제                 */
	
    /* Daemon에게 즉각적인 종료요청을 위해 signal 주기 */
	sprintf(fifo_name, "%s/PA/%s", _FEP_FIFO, INFO(D_K).daemon_FIFO_name);
	DFIFD(D_K) = open(fifo_name, O_RDWR | O_NDELAY);
	if (DFIFD(D_K) == -1)
	{
		Log(USR_OK, "cannot open daemon FIFO[%s] {%d:%s}",
			fifo_name, SYS_NO, SYS_STR);
		//exit(FAIL);

	} else {
		write(DFIFD(D_K), "1", 1);
	}

	if (mar_stop_msg_send(E9999_MSG) < 0) // system error
		Log(USR_OK, "mar_stop_msg_send failed!!! errno[%d][%s]", errno, ApType); 

	Log(USR_OK, "mar_stop_msg_send[%s]", ApType); 

	extflg = 1;
	pthread_kill(tid[0], SIGUSR1);
	Exit_Process();
}

/*--------------------------------------------------------*/
void    l_mar_pop_fifo(int idx)
/*--------------------------------------------------------*/
{
    int     rt;
    char    tmp[ 64];

    while(1)
    {
        rt = read(Poll[idx].fd, tmp, sizeof(tmp));
        if (rt == 0 || errno == EAGAIN) break;
    }

}
/*************************************************************************
    End of Program (pa_7090_mp.c)
*************************************************************************/

