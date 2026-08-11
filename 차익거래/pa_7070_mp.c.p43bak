#define		_GLOBAL
/*--------------------------------------------------------
#	Module	: 현선물차익거래(yarb - Ver.YSK) 
#	File	: pa_7070_mp.c
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
#include "def.h"
#include "sem.h"

#define		DATA_SIZE	  2048
#define     ARB_STOP_CD   99
#include 	"buf_struct.h"
#include	"shm_memory.h"

/* 외부 소스 */
#ifndef _OMS_SOURCE_
#define	_OMS_SOURCE_	1
#endif /* _OMS_SOURCE_ */
/*--------------------------------------------------------
	FOR BLP
--------------------------------------------------------*/
int			Continue = 1;

/*--------------------------------------------------------
	Constants and Structures
--------------------------------------------------------*/
#define		FIFO_EVENT 	0
#define		FILE_EVENT	1

#define		TCP_TIME_OUT 30
#define		DATA_TIME	60 * 1000
#define		READ_MAX	1

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
static struct  pollfd	   Poll[4];
static int	   PollCnt;
int		Pk_mp[2];
char	ApType[10], Cli_handler[5];
char	W_DFmt[1024];
char	Order_St[1024];

int     succ_flag=0;

pthread_t    tid[2];
int          thr_usr2flg=0, extflg=0;
static struct sigaction thr_sigact;

//SharedRoot   g_shared_mem;
//SharedRoot  *g_shared = &g_shared_mem;
SharedRoot  *g_shared = NULL;

int          g_strat_idx   = 0;
int          g_set_idx     = 0;       // 현재 운용중인 세트 인덱스
int          g_test_mode   = 0;
int          g_initialized = 0;       // arb_started gubn
int          g_log_level   = 0xff;
char         g_progname[64]= "unknwon";
char        *pname="arb_main";
int          OD_SEQ      = 0;

Config         g_cfg;

SHM_FIN_FUT   *g_md_shm_fut= NULL;       // 선물주간시세
SHM_FIN_FUT   *g_md_shm_fut_ngt = NULL;  // 선물야간시세

RISK          *g_md_shm_spot = NULL;     // 현물시세 Read 용
SHM_FX        *shm_fx = NULL;            // 현물시세 auto_use 용

FutOrdNo      *g_shm_oid_fut = NULL;     // 선물주문번호 Shm
SpotOrdNo     *g_shm_oid_spot = NULL;    // 현물주문번호 Shm

int fut_sem_id = 0;
int spot_sem_id = 0;

/*--------------------------------------------------------
	Function Prototypes
--------------------------------------------------------*/
void	l_arb_main(void);

int		l_arb_init_param(void);
int 	l_arb_set_start(void);	// 500200(기동Setting);
void	l_arb_fifo_event(void);
void    l_arb_data_event(void);

int		l_arb_proc();
void    l_arb_term(int);
void    l_arb_pop_fifo(int idx);
void    l_arb_auto_use_clear();

void   *l_fut_dat_proc();
void   *l_spot_dat_proc();

void   l_arb_signal();
void   l_arb_end_routine();

/*--------------------------------------------------------*/
int		main(int argc, char *argv[])
/*--------------------------------------------------------*/
{
	Init_Proc(argc, argv);
	strcpy(g_progname, basename(argv[0]));
	l_arb_main();

}	/* End of main */

void
l_thr_sighandler(int signo)
{
    thr_usr2flg = 1;
}

/*--------------------------------------------------------*/
void	l_arb_main(void)
/*--------------------------------------------------------*/
{
	int	 rt, i, rc;
	
	rt = l_arb_init_param();
	l_dbg(L_DBG, "l_arb_init_param rt[%d] WR_CNT[%d] RD_CNT[%d]", rt, WR_CNT, RD_CNT);
	if (rt != OK)
	{
		l_arb_term(9);
		sleep(1);
	}
	
	PollCnt = 4;
    thr_sigact.sa_flags   = 0;
    thr_sigact.sa_handler = l_thr_sighandler;
    sigaction(SIGUSR1, &thr_sigact, NULL);

    pthread_create(&tid[0], NULL, l_fut_dat_proc,  NULL);
    pthread_create(&tid[1], NULL, l_spot_dat_proc, NULL);

	while (START_S < JOB_END)
	{
		if (extflg)
		{
			l_dbg(L_DBG, "============= 쓰레드에서 종료처리 무조건 정지.. Arb Process Stop..");
			l_arb_auto_use_clear();
			l_arb_term(9); 
		}

		Stat_Save();

		while (1)
		{
			if (WR_CNT > RD_CNT)
			{
				l_arb_data_event();		// 응답/체결 처리
				continue;
			}
			else
				break;
		}
		
		rt = poll(Poll, PollCnt, 100);
		//rt = poll(Poll, PollCnt, 5000); // for test
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
				rc = l_arb_proc(NULL);
			    if (rc == ARB_STOP_CD)
				{
				 	l_arb_auto_use_clear();	

				 	l_dbg(L_DBG, "ARB 강제 전략 종료 .. rc=[%d]", rc);
				 	l_arb_term(9);
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
				l_arb_fifo_event();
				break;
			case FILE_EVENT:			//File Event, 응답/체결처리, 설정/종료수신등
				while(START_S < JOB_END)
				{
					if (WR_CNT > RD_CNT)
					{
						l_arb_data_event();
						continue;
					}
					else
						break;
				}
				break;

			case 2:			
				//l_dbg(L_DBG, "fut poll event..");
				rc = l_arb_proc(NULL);
				l_arb_pop_fifo(2); // 데이터 Read
				if (rc == ARB_STOP_CD)
				{
				 	l_arb_auto_use_clear();	
					l_dbg(L_DBG, "ARB 강제 전략 종료 .. rc=[%d]", rc);
				 	l_arb_term(9);
				}
				break;

			case 3:		
				//l_dbg(L_DBG, "spt poll event..");
				rc = l_arb_proc(NULL);
				l_arb_pop_fifo(3); // 데이터 Read
				if (rc == ARB_STOP_CD)
				{
				 	l_arb_auto_use_clear();	
				 	l_dbg(L_DBG, "ARB 강제 전략 종료 .. rc=[%d]", rc);
				 	l_arb_term(9);
				}
				break;

			default:
				SLog(USR_ERROR, "event case error[%d,%d]", i, PollCnt);
				return;
				break;
		}
	}
	
}	/* End of PA_7070_MP () */

/*--------------------------------------------------------*/
void	l_arb_data_event(void)
/*--------------------------------------------------------*/
{
	int    rt, R_Cnt;
	char   tmp[128];
	int    fut_side = 0, spot_side = 0;
	int    fut_ord_id = 0, fut_fill_qty = 0;
	long   spot_ord_id = 0, spot_fill_qty = 0;
	double fut_fill_px = 0, spot_fill_px = 0;
	int    rc = 0;

	R_Cnt = DSHM_R(PS_R_1, (void *)R_Fmt, 1);

	//l_dbg(L_DBG, "WR_CNT=[%d] RD_CNT=[%d] R_Fmt[%s]", WR_CNT, RD_CNT, R_Fmt[0].Data);

	if (R_Cnt < 0)
	{
		SLog(SAM_FATAL, "cannot read file[%s,%d:%s] W[%d]R[%d]",
			IFN(D_K,P_K,0), SYS_NO, SYS_STR, WRITE_CNT, RD_CNT);
		sleep(1);
		Exit_Process();
	}
	else if (R_Cnt == 0)
		return;
	
	Dshm_Add_Count(TS_W1_1, 1);
	
	if (memcmp(R_Fmt[0].Data, "500100", 6) == 0)             // 자동기동 Setting
	{		   
		int ret;
		l_dbg(L_DBG, "[ARB] 현선차익거래전략구동 Data=[%.2048s]", (char *)R_Fmt[0].Data);
		ret = l_arb_set_start();
		if (ret < 0) {
			l_arb_term(9);
		}
	}
	else if (memcmp(R_Fmt[0].Data, "500300", 6) == 0)		// 자동종료요청
	{
		if (succ_flag == 1) {                               // Arb_Init 정상처리시에만 call
			l_arb_auto_use_clear();
			//if (g_md_shm_fut[G_STRAT->fut_idx].auto_use < 0) g_md_shm_fut[G_STRAT->fut_idx].auto_use = 0;
			//if (g_md_shm_spot[0].FX_Sise[0][G_STRAT->spot_idx].auto_use < 0) g_md_shm_spot[0].FX_Sise[0][G_STRAT->spot_idx].auto_use = 0;
		} 
		l_dbg(L_DBG, "[ARB] 자동종료요청 수신.. succ_flag[%d] auto_use[%d][%d]", succ_flag,
									   g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ], shm_fx[G_STRAT->spot_idx].auto_use[OD_SEQ]);

		l_arb_term(9);

	}
	else if (memcmp(R_Fmt[0].Data, "100120", 6) == 0) // 응답(100120)
	{
		if (memcmp(R_Fmt[0].Data+6, "01", 2) == 0) // 시장구분 - '01' 선물(파생) 
		{

			YSK_DATA_HEAD *yhdr = (YSK_DATA_HEAD *)&R_Fmt[0].Data[20]; // OMS헤더(20byte)
			KRX_SETTLE_RESP_DATA *i = (KRX_SETTLE_RESP_DATA *)&R_Fmt[0].Data[120]; //OMS헤더(20byte) + YSK 헤더(100byte)

			if (memcmp(yhdr->sRpCode, "0000", sizeof(yhdr->sRpCode)) == 0)
			{ 
				l_dbg(L_DBG, "선물 주문응답전문 : [%.*s][%.318s]", (int)sizeof(i->TrCode), i->TrCode, (char *)i);
				if (memcmp(i->TrCode, "TTRODP11321", 11) == 0) // TTRODP11321 거부 
				{
					 fut_ord_id = l_stoi(i->OrderNo, sizeof(i->OrderNo)); // 주문번호
					 l_dbg(L_DBG, "선물 주문거부응답 fut_ord_id [%ld]", fut_ord_id);

					 rc = l_arb_fut_exec(-1, 0, fut_ord_id, 0, 0); 
					 if (rc == ARB_STOP_CD)
					 {
						l_arb_auto_use_clear();	
						l_dbg(L_DBG, "l_arb_fut_exec: 강제 전략 종료 .. 선물 주문 거부 rc=[%d]", rc);
						l_arb_term(9);
					 }
				}
				else if (memcmp(i->TrCode, "TTRODP11303", 11) == 0) // TTRODP11303 자동취소 
				{
					 fut_ord_id   = AtoIf(i->OrderNo, sizeof(i->OrderNo)); // 주문번호
					 l_dbg(L_DBG, "선물 KRX Auto Cancel 응답 fut_ord_id [%ld]", fut_ord_id);
					 rc = l_arb_fut_exec(-2, 0, fut_ord_id, 0, 0); 
					 if (rc == ARB_STOP_CD)
					 {
						l_arb_auto_use_clear();	
						l_dbg(L_DBG, "l_arb_fut_exec: 강제 전략 종료 .. rc=[%d]", rc);
						l_arb_term(9);
					 }
				} else {
					l_dbg(L_DBG, "OTHER CASE!!! TrCode[%.*s] rc=[%d]", sizeof(i->TrCode), i->TrCode, rc);
				}
			}
			else
			{
				l_dbg(L_DBG, "OMS Reject... response code[%.*s]", sizeof(yhdr->sRpCode), yhdr->sRpCode);
				arb_stop_msg_send(E5201_MSG);
				l_arb_auto_use_clear();	
				l_dbg(L_DBG, "OMS Reject 강제 전략 종료 .. ");
				l_arb_term(9);
			}
		}
		else if (memcmp(R_Fmt[0].Data+6, "04", 2) == 0) // 시장구분 - '04' 현물 
		{
			SMB_ST *s = (SMB_ST *)&R_Fmt[0].Data[20];
			l_dbg(L_DBG, "현물 주문응답 전문 : [%.*s][%.*s]", (int)sizeof(s->smb_MsgType), s->smb_MsgType, sizeof(SMB_ST), (char*)s);
			if (memcmp(s->smb_MsgType, "3", 1) == 0 || memcmp(s->smb_MsgType, "8", 1) == 0  ) // '3'-거부, '8'-주문확인 및 체결
			{
				// 거부/취소 떨어지면 양쪽 수량 맞을때까지 retry...
				if ((memcmp(s->smb_OrdStatus, "8", 1) == 0 && memcmp(s->smb_ExecType, "8", 1) == 0) || 
					(memcmp(s->smb_OrdStatus, "4", 1) == 0 && memcmp(s->smb_ExecType, "4", 1) == 0)
				    ) // '8'-거부
				{
					spot_ord_id   = AtoLf(s->smb_ClOrdID + 8, 10);  // 주문번호
			  	 	l_dbg(L_DBG, "현물 거부응답.. spot_ord_id [%ld]", spot_ord_id);
					rc = l_arb_spot_exec(-1, 0, spot_ord_id, 0, 0);
					if (rc == ARB_STOP_CD)
					{
				 		l_arb_auto_use_clear();	
						l_dbg(L_DBG, "l_arb_spot_exec 강제 전략 종료.. 현물 주문 거부 rc=[%d]", rc);
						l_arb_term(9);
					}

				}
			} 
			else if (memcmp(s->smb_MsgType, "D", 1) == 0 || memcmp(s->smb_MsgType, "5", 1) == 0  ) // 'D' 신규주문거부 '5' FIX 끊김
			{
				spot_ord_id   = AtoLf(s->smb_ClOrdID + 8, 10);  // 주문번호
			 	l_dbg(L_DBG, "현물 거부응답.. spot_ord_id [%ld]", spot_ord_id);
				rc = l_arb_spot_exec(-1, 0, spot_ord_id, 0, 0);
				if (rc == ARB_STOP_CD)
				{
			 		l_arb_auto_use_clear();	
					l_dbg(L_DBG, "l_arb_spot_exec 강제 전략 종료.. 현물 주문 거부 rc=[%d]", rc);
					l_arb_term(9);
				}
			}
			else {
				l_dbg(USR_OK, "INCORRECT CHECK CASE!!! smb_MsgType[%.1s] rc=[%d]", s->smb_MsgType, rc);
			}
		}
	}
	else if (memcmp(R_Fmt[0].Data, "100140", 6) == 0)// 체결(100140)
	{
		if (memcmp(R_Fmt[0].Data+6, "01", 2) == 0) // 시장구분 - '01' 선물(파생) 
		{
			KRX_SETTLE_DATA *i = (KRX_SETTLE_DATA *)&R_Fmt[0].Data[120]; // OMS헤더(20byte) + YSK 헤더(100byte)
			l_dbg(L_DBG, "선물 체결 전문 : [%.*s]", sizeof(KRX_SETTLE_DATA), (char*)i);
			if (memcmp(i->TrCode, "TTRTDP21301", 11) == 0) // TTRTDP21301 (체결결과) 
			{
				fut_ord_id   = l_stoi(i->OrderNo, sizeof(i->OrderNo));               // 주문번호
			    fut_side	 = l_stoi(i->TradeFlag, sizeof(i->TradeFlag));           // 매도매수구분코드 - 1:Sell, 2.Buy 
				fut_fill_qty = l_stoi(i->Trading_Volumn, sizeof(i->Trading_Volumn)); // 체결수량 - 계약단위
				fut_fill_px  = AtoDf(i->Trading_price, sizeof(i->Trading_price));    // 체결가격				
				rc = l_arb_fut_exec(1, fut_side, fut_ord_id, fut_fill_qty, fut_fill_px);
				if (rc == ARB_STOP_CD)
				{
				 	l_arb_auto_use_clear();	
					l_dbg(L_DBG, "l_arb_fut_exec: 강제 전략 종료.. 선물 주문 거부 rc=[%d]", rc);
					l_arb_term(9);
				}

			} else {
				l_dbg(L_DBG, "INCORRECT CHECK CASE!!! TrCode [%.*s] rc=[%d]", sizeof(i->TrCode), i->TrCode, rc);
			}
		}
		else if (memcmp(R_Fmt[0].Data+6, "04", 2) == 0) // 시장구분 - '04' 현물 
		{
			SMB_ST *s = (SMB_ST *)&R_Fmt[0].Data[20]; // oms헤더(20byte)
			l_dbg(L_DBG, "현물 체결 전문 : [%.*s]", sizeof(SMB_ST), (char*)s);
			if (memcmp(s->smb_MsgType, "8", 1) == 0) // '8'-주문확인 및 체결 
			{
				spot_ord_id   = AtoLf(s->smb_ClOrdID + 8, 10);             // 주문번호
				spot_side     = AtoIf(s->smb_Side, sizeof(s->smb_Side));                                // 매매구분 - 1:Buy, 2:Sell
				spot_fill_qty = (long)(AtoDf(s->smb_LastQty, sizeof(s->smb_LastQty)) / 1000000.0);      // 체결수량 - arb에서 수량을 M 단위로 관리하므로 백만으로 나눈 후 전송 
				spot_fill_px  = AtoDf(s->smb_LastPx, sizeof(s->smb_LastPx));                            // 체결가격
				rc = l_arb_spot_exec(1, spot_side, spot_ord_id, spot_fill_qty, spot_fill_px);
				if (rc == ARB_STOP_CD)
				{
				 	l_arb_auto_use_clear();	
					l_dbg(L_DBG, "l_arb_spot_exec 강제 전략 종료.. 현물 주문 거부 rc=[%d]", rc);
					l_arb_term(9);
				}

			} else {
				l_dbg(L_DBG, "INCORRECT CHECK CASE!!! smb_MsgType[%.1s] rc=[%d]", s->smb_MsgType, rc);
			}
		}
	}
	else
	{
		l_dbg(USR_ERROR, "Other Tr Code Check Plz[%20.20s]", R_Fmt[0].Data);
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
}	/* l_arb_data_event() */

/*--------------------------------------------------------*/
int		l_arb_init_param()
/*--------------------------------------------------------*/
{
	char 	fifo_name[256];
	int 	i=0, shmsz, shmid, first=0;
	key_t   k;
	
	/* attach daemon SHM (INFO) : ALL_DAEMON_INFO */
    Sub_SHM();

	Log(USR_OK, "l_arb_init_param...ARB ");
	
	Poll[0].fd     = START_FD;		// of Daemon
	Poll[0].events = POLLIN;
	Poll[1].fd     = INPUT_FD; 		// of SelfFile
	Poll[1].events = POLLIN;

	/* 선물 */
	memset(fifo_name, 0, sizeof(fifo_name));
//	sprintf(fifo_name, "%s/PA/pa_55%5.5s1", _FEP_FIFO, _Exe_Name+5);  // 선물시세수신
	sprintf(fifo_name, "%s/PA/pa_75%5.5s1", _FEP_FIFO, _Exe_Name+5);

	Poll[2].fd = open(fifo_name, O_RDWR|O_NDELAY);
	Poll[2].events = POLLIN;
	Log(USR_OK, "_Exe_Name [%s] fifo_name[%s] fd[%d]", _Exe_Name, fifo_name, Poll[2].fd);
	if (Poll[2].fd < 0)
	{
		SLog(FIF_FATAL, "cannot open i{%d] FIFO[%s][%d][%d:%s]",
			i, fifo_name, Poll[2].fd, SYS_NO, SYS_STR);
		return(-1);
	}
	
	// 기동시 2로 변경
	// 종료시 0으로 변경해야함
	PROC(D_K,P_K).tr_seq = 2; // tr_seq : 시장구분 - 1:채권, 2:선물, 3:채권/선물 둘다 수신필요시
	Log(USR_OK, "tr_seq[%d]", PROC(D_K,P_K).tr_seq);

	/* 현물 */
	memset(fifo_name, 0, sizeof(fifo_name));
//	sprintf(fifo_name, "%s/PA/pa_7402_ur1", _FEP_FIFO);  // 현물시세수신
	sprintf(fifo_name, "%s/PA/pa_75%4.4s11", _FEP_FIFO, _Exe_Name+5);

	OD_SEQ = (AtoIf (_Exe_Name+4, 2) - 1) * 10;
    OD_SEQ =  AtoIf (_Exe_Name+6, 2) + OD_SEQ - 1;	
	
	Poll[3].fd = open(fifo_name, O_RDWR|O_NDELAY);
	Poll[3].events = POLLIN;
	Log(USR_OK, "_Exe_Name [%s] fifo_name[%s] fd[%d]", _Exe_Name, fifo_name, Poll[3].fd);

	if (Poll[3].fd < 0)
	{
		SLog(FIF_FATAL, "cannot open i{%d] FIFO[%s][%d][%d:%s]",
			i, fifo_name, Poll[3].fd, SYS_NO, SYS_STR);
		return(-1);
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
    if (load_config(ARB_CFG_PATH) != 0) {
        SLog(FIF_FATAL, "config.ini load failed!\n");
        return(-1);
    }

    shmsz = sizeof(SharedRoot);
    g_cfg.d_strat_shm_size = g_cfg.d_strat_shm_size*shmsz;

    // 1. 전략 공유 메모리 세그먼트 생성 (이미 있으면 기존 것 사용)
    shmid = shmget(g_cfg.d_strat_shm_key, 0, 0666);
    if (shmid < 0)
    {
    	shmid = shmget(g_cfg.d_strat_shm_key, sizeof(SharedRoot), IPC_CREAT|0666);
		if (shmid > 0) {
			first = 1;

		} else {
			SLog(FIF_FATAL, "shmget failed!!! key[%x] shmid[%d] shmsz[%d] errno[%d]\n", g_cfg.d_strat_shm_key, shmid, sizeof(SharedRoot), errno);
			return(-2);
		}
    }

    // 2. 전략 공유 메모리를 프로세스 주소 공간에 attach
    g_shared = (SharedRoot *)shmat(shmid, NULL, 0);
    if (g_shared == (void *)-1) {
        SLog(FIF_FATAL, "shmat failed!!! key[%x] shmid[%d] shmsz[%d] errno[%d]\n", g_cfg.d_strat_shm_key, shmid, sizeof(SharedRoot), errno);
        return(-3);
    }

    SLog(USR_OK, "====> SIZE[%ld] key[%x] first[%d]", g_cfg.d_strat_shm_size, g_cfg.d_strat_shm_key, first);
	if (first) {
		memset(g_shared, 0x00, sizeof(SharedRoot));
	}

	// 3. 선물 주문번호 공유메모리 세마포어 생성 및 attach 
    first = 0;
	shmsz = sizeof(FutOrdNo);
    g_cfg.fut_oid_shm_size = g_cfg.fut_oid_shm_size*shmsz;
    
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

    fut_sem_id = Sem_Create(g_cfg.fut_oid_shm_key);
    if (fut_sem_id < 0)  
    {   
        SLog(FIF_FATAL, "FutOrdNo Sem_Create error.");
        return(-6);
    }   
    SLog(USR_OK, "====> !!!SIZE[%ld] key[%x] first[%d] sem_id[%d]", g_cfg.fut_oid_shm_size, g_cfg.fut_oid_shm_key, first, fut_sem_id);  

	//==============================================================
    // 메시지 큐 init
	// MSG_TYPE_SPOT_ORD
    g_cfg.nh_spot_order_id = l_que_create(g_cfg.nh_order_msg_key); 
    if (g_cfg.nh_spot_order_id == -1)
    {
        SLog(FIF_FATAL, "NH 현물 주문 msgque init 실패 !!");
        return(-10);
    }
    
    g_cfg.jpm_spot_order_id = l_que_create(g_cfg.jpm_order_msg_key); 
    if (g_cfg.jpm_spot_order_id == -1)
    {
        SLog(FIF_FATAL, "JPM 현물 주문 msgque init 실패 !!");
        return(-10);
    }

    g_cfg.sh_spot_order_id = l_que_create(g_cfg.sh_order_msg_key); 
    if (g_cfg.sh_spot_order_id == -1)
    {
        SLog(FIF_FATAL, "SH 현물 주문 msgque init 실패 !!");
        return(-10);
    }

	// MSG_TYPE_SPOT_ORDLOG
    g_cfg.spot_ordlog_id = l_que_create(g_cfg.orderlogq_spot_msg_key); 
    if (g_cfg.spot_ordlog_id == -1)
    {
        SLog(FIF_FATAL, "[INIT] 현물 NH 주문 후처리 msgque init 실패 !!");
        return(-11);
    }

    g_cfg.jpm_ordlog_id = l_que_create(g_cfg.orderlogq_jpm_msg_key); 
    if (g_cfg.jpm_ordlog_id  == -1)
    {
        SLog(FIF_FATAL, "[INIT] 현물 JPM 주문 후처리 msgque init 실패 !!");
        return(-11);
    }

    g_cfg.sh_ordlog_id = l_que_create(g_cfg.orderlogq_sh_msg_key); 
    if (g_cfg.sh_ordlog_id  == -1)
    {
        SLog(FIF_FATAL, "[INIT] 현물 SH 주문 후처리 msgque init 실패 !!");
        return(-11);
    }
	//==============================================================

	//==============================================================
	// MSG_TYPE_FUT_ORD
    g_cfg.krx_day_order_id = l_que_create(g_cfg.krx_order_day_msg_key); 
    if (g_cfg.krx_day_order_id == -1)
    {
        SLog(FIF_FATAL, "KRX 선물 주간 주문 msgque init 실패 !!");
        return(-12);
    }
    
    g_cfg.krx_ngt_order_id = l_que_create(g_cfg.krx_order_ngt_msg_key); 
    if (g_cfg.krx_ngt_order_id == -1)
    {
        SLog(FIF_FATAL, "KRX 선물 야간 주문 msgque init 실패 !!");
        return(-12);
    }

	// MSG_TYPE_FUT_ORDLOG
	{
	key_t key;
	key = l_ftok(g_cfg.orderlogq_fut_msg_key_text, 'Q'); 
    g_cfg.fut_ordlog_id = l_que_create(key);
	}
    if (g_cfg.fut_ordlog_id == -1)
    {
        SLog(FIF_FATAL, "[INIT] 선물 주문 후처리 msgque init 실패 !!");
        return(-13);
    }
	//==============================================================

	/* 주간선물시세 Shm Attach  --------------------------------------------------------------*/
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
        return(-14);
    }
    // 2. 공유 메모리를 프로세스 주소 공간에 attach
    g_md_shm_fut = (SHM_FIN_FUT *)shmat(shmid, NULL, 0);
    if (g_md_shm_fut == (void *)-1) {
        SLog(FIF_FATAL, "주간선물시세 메모리 attach failed . errrno[%d:%s] shmid[%d]", errno, strerror(errno), shmid);
        return(-15);
    }

	/* 야간선물시세 Shm Attach  --------------------------------------------------------------*/
	shmsz = sizeof(SHM_FIN_FUT) * SHM_MAX_DEV_FIF_N;   // 1,000
    
    // 1. 공유 메모리 세그먼트 생성 (이미 있으면 기존 것 사용)
    shmid = shmget(FF_N_SHM_KEY+k, shmsz, IPC_CREAT | 0666);
    if (shmid < 0)
    {
        SLog(FIF_FATAL, "선물시세 메모리 생성 . errrno[%d:%s] key[%x] shmsz[%d] k[%d]", errno, strerror(errno), FF_N_SHM_KEY+k, shmsz, k);
        return(-14);
    }
    // 2. 공유 메모리를 프로세스 주소 공간에 attach
    g_md_shm_fut_ngt = (SHM_FIN_FUT *)shmat(shmid, NULL, 0);
    if (g_md_shm_fut_ngt == (void *)-1) {
        SLog(FIF_FATAL, "야간선물시세 메모리 attach failed . errrno[%d:%s] shmid[%d]", errno, strerror(errno), shmid);
        return(-15);
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

	// LOG LEVEL SETTING -- 0:ERR 1:WAR 2:INF 3:DBG
	l_loglvl_set(g_cfg.log_level);
    SLog(USR_OK, "log_level[%d]", g_cfg.log_level);

	// use_cnt 클리어 시그널
	//l_arb_signal();

	return(0);
}	/* End of l_arb_init_param() */

/*--------------------------------------------------------*/
int	l_arb_set_start(void)
/*--------------------------------------------------------*/
{
//	int     today = get_today();
	int     type  = 0,  tmp_idx = 0;
	char	arbmsg[16];
	int 	shmsz, shmid, first=0;

	IN_SAMPLE01	*dat;
	
	l_dbg(L_DBG, "l_arb_set_start ... ARB ");
	dat = (IN_SAMPLE01 *)&R_Fmt[0].Data[sizeof(SEARCH_HEADER)];

	succ_flag = 0;

	memcpy(arbmsg, dat, 6);
    g_strat_idx = atoi(arbmsg) - 1; // 전략실행일련번호(전략인덱스)

	STRATEGY_BODY  *bd = (STRATEGY_BODY *)dat; 

	char log_sym[3+1]={0, };
	memcpy(log_sym, bd->fx_prdct_cd, 3);
	
	if (bd->sf_mrgn_rule_id[0] == 'A') 
		tmp_idx = g_strat_idx + 1; 
	else if (bd->sf_mrgn_rule_id[0] == 'B')
		tmp_idx = g_strat_idx - 50 + 1;
	else if (bd->sf_mrgn_rule_id[0] == 'C')
		tmp_idx = g_strat_idx - 100 + 1;
	else {
		tmp_idx = g_strat_idx - 150 + 1; 
		memcpy(log_sym, "NDF", 3);
	}
	
    g_log_level = g_cfg.log_level;

	int ftrs_bsns_dt = AtoIf(bd->ftrs_bsns_dt, sizeof(bd->ftrs_bsns_dt));
	int spot_bsns_dt = AtoIf(bd->spot_bsns_dt, sizeof(bd->spot_bsns_dt));

	snprintf(l_dbgfile, sizeof(l_dbgfile), "%s/ARB_%.6s_%d-%d", g_cfg.log_file, log_sym, ftrs_bsns_dt, tmp_idx);
	Log(USR_OK, "START -------> [%s]", l_dbgfile);

	// 현물 주문번호 공유메모리 세마포어 생성 및 attach 
	key_t spot_oid_shm_key;
	
	if (bd->fx_exch_dstcd[0] == 'N')
		spot_oid_shm_key = g_cfg.nhspot_oid_shm_key;
	else if (bd->fx_exch_dstcd[0] == 'J')
		spot_oid_shm_key = g_cfg.jpmspot_oid_shm_key;
	else if (bd->fx_exch_dstcd[0] == 'S')
		spot_oid_shm_key = g_cfg.shspot_oid_shm_key;
	else {
		SLog(FIF_FATAL, "Undefined spot exchange code.. Please Check fx_exch_dstcd [%.1s]!!!\n", bd->fx_exch_dstcd);
		return(-7);
	}  

	first = 0;
	shmsz = sizeof(SpotOrdNo);
    g_cfg.spot_oid_shm_size = g_cfg.spot_oid_shm_size*shmsz;

    shmid = shmget(spot_oid_shm_key, 0, 0666);
    if (shmid < 0)
    {
    	shmid = shmget(spot_oid_shm_key, sizeof(SpotOrdNo), IPC_CREAT|0666);
		if (shmid > 0) {
			first = 1;

		} else {
			SLog(FIF_FATAL, "shmget failed!!! key[%x] shmid[%d] shmsz[%d] errno[%d]\n", spot_oid_shm_key, shmid, sizeof(SpotOrdNo), errno);
			return(-7);
		}
    }
    
	g_shm_oid_spot = (SpotOrdNo *)shmat(shmid, NULL, 0);
    if (g_shm_oid_spot == (void *)-1) {
		SLog(FIF_FATAL, "shmat failed!!! key[%x] shmid[%d] shmsz[%d] errno[%d]\n", spot_oid_shm_key, shmid, sizeof(SpotOrdNo), errno);
        return(-8);
    }

    SLog(USR_OK, "fx_exch_dstcd[%.1s]===> SIZE[%ld] key[%x] first[%d]", bd->fx_exch_dstcd, g_cfg.spot_oid_shm_size, spot_oid_shm_key, first);
	if (first) {
		memset(g_shm_oid_spot, 0x00, sizeof(SpotOrdNo));
	}

    spot_sem_id = Sem_Create(spot_oid_shm_key);
    if (spot_sem_id < 0)  
    {   
        SLog(FIF_FATAL, "SpotOrdNo Sem_Create error.");
        return(-6);
    }   
    
	SLog(USR_OK, "====> !!!SIZE[%ld] key[%x] first[%d] sem_id[%d]", g_cfg.spot_oid_shm_size, spot_oid_shm_key, first, spot_sem_id);  

    // 전략 메모리 초기화 (선물 영업일 변경 기준으로 관리)
    if (G_STRAT->today == 0 || G_STRAT->today != ftrs_bsns_dt)
    {
        l_dbg(L_ERR, "[INIT] 선물영업일 변경으로 전략 슬롯[%d]메모리 초기화 today[%d][%d]", g_strat_idx, ftrs_bsns_dt, G_STRAT->today);
        memset(&g_shared->slots[g_strat_idx].strategy, 0 , sizeof(StrategyMem));
        memset(&g_shared->slots[g_strat_idx].ordno   , 0 , sizeof(OrdNo));
//        memset(g_shared->slots[g_strat_idx].sets     , 0 , sizeof(SetState) * MAX_SETS_PER_STRAT);
        memset(&g_shared->slots[g_strat_idx].sets     , 0 , sizeof(SetState) * MAX_SETS_PER_STRAT);
        G_STRAT->today = ftrs_bsns_dt;
        memcpy(G_STRAT->aptype, ApType, strlen(ApType));
        G_STRAT->aptype[7] = '\0';
        G_SET_CNT = 0;
        //G_SET->valid = SET_VALID;
    }
	
	// 당일 New 선물 주문번호 채번 --> (선물 영업일 변경 기준으로 관리) 
	//if (G_OID_FUT->ord_dt == 0 || G_OID_FUT->ord_dt != G_STRAT->today)
	if (G_OID_FUT->ord_dt == 0 || G_OID_FUT->ord_dt != ftrs_bsns_dt)
	{
		Sem_Lock(fut_sem_id);
	    G_OID_FUT->ord_dt     = ftrs_bsns_dt;	
	    G_OID_FUT->fut_ordno  = 0;
		Sem_Unlock(fut_sem_id);
	}

	// 당일 New 현물 주문번호 채번
	if (G_OID_SPOT->ord_dt == 0 || G_OID_SPOT->ord_dt != spot_bsns_dt)
	{
		Sem_Lock(spot_sem_id);
	    G_OID_SPOT->ord_dt     = spot_bsns_dt;
		G_OID_SPOT->spot_ordno = 0;
		Sem_Unlock(spot_sem_id);
	}

    /* 주문번호 대역 초기 셋팅 */
    G_ORDNO->fut_ordno = 0; // 전략인덱스별 주문번호 초기화
    G_ORDNO->spot_ordno = 0; // 전략인덱스별 주문번호 초기화
    //-------------------------------------------------------------------------------------------------------

	g_set_idx = G_ORDNO->set_id;
	g_initialized = 1;

	// 운영 모드
	l_arb_set_data((char *)dat);

    // 전략에 해당하는 선물, 현물 종목 시세 수신처리 등록 및 접근
    KS_EXPCODE key; // 선물시세 key
    memset(&key, 0, sizeof(key));
    memcpy(key.expcode, G_STRAT->fut_sym, sizeof(key.expcode)); // key.expcode - 종목표준코드(12byte)
 
	/* 선물종목 Search */
	if (G_STRAT->day_ngt_cd[0] == '0')
		type = DEV_MK_FIF;
	else
		type = DEV_MK_FIF_N;		

	G_STRAT->fut_idx = Key_Search(type, KEY_EXPCODE, (char *)&key);
    if (G_STRAT->fut_idx == -1)
    {
        SLog(FIF_FATAL, "선물시세 fut_key_search fail!! (종목마스터 미수신 or 이상상태) [%.*s]", 12 , G_STRAT->fut_sym); 
		if (arb_stop_msg_send(E9999_MSG) < 0) return(-12);
		return(-2);
    }

    //g_md_shm_fut[G_STRAT->fut_idx].auto_use += 1;   // 선물 : 자동전략기동시 해당종목 + 설정(시세수신처리)
	g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ] = 1; 

	/* 현물 종목 Search */
	G_STRAT->spot_idx = Key_Search_FX(G_STRAT->exch_spot, G_STRAT->spot_sym);
	shm_fx[G_STRAT->spot_idx].auto_use[OD_SEQ] = 1;  // 현물 : 자동전략기동시 해당종목 + 설정(시세수신처리)

	l_dbg(L_ERR, "STRAT->spot_sym[%.12s] spot_idx[%d]", G_STRAT->spot_sym, G_STRAT->spot_idx);
	
	l_dbg(L_ERR, "fut_use_cnt=[%d], spot_use_cnt=[%d]", g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ]
                                                      , shm_fx[G_STRAT->spot_idx].auto_use[OD_SEQ]);

	succ_flag = 1;
	return(0);
}	/* End of l_arb_set_start */

/*----------------------------------------------------------------------*/
void    l_arb_fifo_event(void)
/*----------------------------------------------------------------------*/
{
    char tmp[2];
    read(START_FD, tmp, 1);
    return;
}   /* End of l_arb_fifo_event () */

/*----------------------------------------------------------------------*/ 
void    l_arb_term(int option) 
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

	extflg = 1;
	pthread_kill(tid[0], SIGUSR1);
	pthread_kill(tid[1], SIGUSR1);
	Exit_Process();
}

/*--------------------------------------------------------*/
void    l_arb_pop_fifo(int idx)
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

/*--------------------------------------------------------*/
void    l_arb_auto_use_clear()
/*--------------------------------------------------------*/
{
	if (G_STRAT->day_ngt_cd[0] == '1')
		g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ] = 0; 
	else 
		g_md_shm_fut_ngt[G_STRAT->fut_idx].auto_use[OD_SEQ] = 0; 

	shm_fx[G_STRAT->spot_idx].auto_use[OD_SEQ] = 0; 
}

/*----------------------------------------------------------------------*/
void    l_arb_signal (void)
/*----------------------------------------------------------------------*/
{ 
    signal (SIGINT, l_arb_end_routine);
    signal (SIGKILL, l_arb_end_routine);
    signal (SIGQUIT, l_arb_end_routine);
    signal (SIGILL, l_arb_end_routine);
    signal (SIGTERM, l_arb_end_routine);
    signal (SIGBUS, l_arb_end_routine);
    signal (SIGSEGV, l_arb_end_routine);
    signal (SIGHUP, l_arb_end_routine);

    return;
}   /* End of l_setsigfatal ()    */


void  l_arb_end_routine (int p_signo)
{
    if (g_md_shm_fut != NULL)
	{ 
		g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ] = 0;
        SLog(USR_OK, "fut auto_use[%d] clear..", g_md_shm_fut[G_STRAT->fut_idx].auto_use[OD_SEQ]);
	}
	if (g_md_shm_fut_ngt != NULL)
	{
		g_md_shm_fut_ngt[G_STRAT->fut_idx].auto_use[OD_SEQ] = 0;
        SLog(USR_OK, "fut_ngt auto_use[%d] clear..", g_md_shm_fut_ngt[G_STRAT->fut_idx].auto_use[OD_SEQ]);
	}
	if (shm_fx != NULL)
	{
		shm_fx[G_STRAT->spot_idx].auto_use[OD_SEQ] = 0; 
        SLog(USR_OK, "fx auto_use[%d] clear..", shm_fx[G_STRAT->spot_idx].auto_use[OD_SEQ]);
	}

    End_Routine(p_signo);
}

/*************************************************************************
    End of Program (pa_7070_mp.c)
*************************************************************************/

