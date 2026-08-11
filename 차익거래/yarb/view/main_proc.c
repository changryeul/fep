#include <sys/shm.h>
#include <time.h>
#include <errno.h>
#include <libgen.h>
#include "config.h"
#include "arb.h"
#include "log.h"
#include "executor.h"
#include "price_rules.h"
#include "market.h"
#include "events.h"
#include "msgq.h"

// 방향성 보존 비교(양수 임계는 이상, 음수 임계는 이하)
int order_plan_auto(int *gb);
int execute_order_pair(int gb);
int log_open_with_id(char *base_dir, char *basename, int id);
int arb_stop_msg_send(char *msg);
int spot_order_process(FillEvent *fill);

static int   arb_shm_init    (void);
long  generate_set_id (void);
static int   order_process();
static int   BaseName(void);

SharedRoot  g_shared_mem;
SharedRoot *g_shared = &g_shared_mem;
int         g_strat_idx = 0; 
int         g_set_idx   = 0;       // 현재 운용중인 세트 인덱스
int         g_test_mode = 0; 
int         g_init_memory = 0; 
int         g_initialized = 0;
char        g_progname[64]= "unknwon";
char       *pname="arb_main";

extern  SHM_FIN_FUT *g_md_shm_fut;
extern  SHM_FX      *g_md_shm_spot;

static int BaseName(void)
{
    char exe_path[256];
    memset(exe_path, 0x00, sizeof(exe_path));

	int len = readlink("/proc/self/exe", exe_path, sizeof(exe_path)-1);

	if (len != -1)
	{
	    exe_path[len] = '\0';
	    char *name = basename(exe_path);
		snprintf(g_progname, sizeof(g_progname), "%s", name);
    }
	return 0;
}
// ------------------------------------------------------------
// 전략 공유메모리 생성
// ------------------------------------------------------------
static int arb_shm_init(void)
{
    int shmid;
	int shmsz;

	shmsz = sizeof(SharedRoot);
    g_cfg.strat_shm_size = g_cfg.strat_shm_size*shmsz;
	
	//LOG_DEBUG( " SIZE [%d][%d]",  sizeof(SharedRoot), g_cfg.strat_shm_size);
	LOG_DEBUG( "SIZE [%ld]", g_cfg.strat_shm_size);
    // 1. 공유 메모리 세그먼트 생성 (이미 있으면 기존 것 사용)
    shmid = shmget(g_cfg.strat_shm_key, sizeof(SharedRoot), IPC_CREAT | 0666);
    if (shmid < 0) 
    {
        LOG_ERR("전략 메모리 생성 . errrno[%d]",errno );
        return ARB_STOP_CD;
    }

    // 2. 공유 메모리를 프로세스 주소 공간에 attach
    g_shared = (SharedRoot *)shmat(shmid, NULL, 0);
    if (g_shared == (void *)-1) {
        LOG_ERR("전략 메모리 attach failed . errrno[%d]",errno);
        return ARB_STOP_CD;
    }
    //LOG_DEBUG("Strategy Shared memory created/attached (%d bytes)", sizeof( SharedRoot));

    return 0;
}

// ------------------------------------------------------------
// 공유메모리 detach
// ------------------------------------------------------------
int arb_shm_fini(void)
{
    if (g_shared != NULL) 
    {
        if (shmdt((void *)g_shared) == -1) 
        {
            // perror("shmdt");
            LOG_ERR("전략 메모리 detach failed . errrno[%d]",errno );
            return ARB_STOP_CD;
        }

        g_shared = NULL;
        LOG_INFO(" 공유 메모리 detach 완료.");
    }
    return 0;
}
// ------------------------------------------------------------
// generate_set_id
//  기능 : 세트 id
// ------------------------------------------------------------
long  generate_set_id(void)
{
    long setid=0;

    if (G_SET_CNT ==  0) 
    {
        G_SET_CNT = 1; 
        G_SET->set_id  = 1;
    }
	else
    {
	    G_SET_CNT ++;
        g_set_idx ++;
        G_SET->set_id ++;
    }

    G_SET->valid  = SET_VALID;
    LOG_INFO("[SET] 세트번호 채번 g_set_idx[%d] set_id[%d] ", g_set_idx, G_SET->set_id);

    return setid;
}

// ------------------------------------------------------------
// 전략 시작전 체크 사항
// 전략 중지후 재기동 되었을 때 어떤 처리를 해야 할까?
// 체크만 했지 다른 조치를 지금은 하지 않음.
// ------------------------------------------------------------
static int startup_set_check()
{
    LOG_INFO("Startup Check: 세트 상태 점검 시작...");

    StrategySlot *slot = &g_shared->slots[g_strat_idx];
    StrategyMem *strat = &slot->strategy;

    //LOG_INFO("set count :%d",slot->n );
    for (int i = 0; i < slot->n; i++)
    {
        SetState *set = &slot->sets[i];
        LOG_WARN("id= %d name=%s set_idx=%d  set_loid=%d pos_fut=%d pot_spot=%d ",
             strat->strategy_id, strat->strategy_name,
             i, set->set_id, set->pos_spot, set->pos_fut );

        SetOrdLog *log = &set->ordlog;
        LOG_WARN("log count :%d",log->n );
		for( int j = 0; j< log->n ; j++)
		{
              SetOrdLogEnt *ent = &log->ent[j];
		      LOG_WARN( "          seq:%d ordid:%d leg:%d  side:%d qty:%d px:%f time:%d", 
			         j,ent->ord_id,ent->leg, ent->side, ent->qty,ent->px,ent->ts_hhmmss);  
		}
    }

    g_set_idx = G_ORDNO->set_id ;

    return 0;
}

// ------------------------------------------------------------
// 전략 메모리 정보를 set
// ------------------------------------------------------------
static void set_strategy_data(char *msg)
{
    STRATEGY_BODY *bd = (STRATEGY_BODY *)(msg);

	int arb_tp = 0;      
    int exec_mode = 0;   
    int close_t1_tp = 0, close_t2_tp = 0;
    char ord_type_fut[1+1]={0, }, ord_type_spot[1+1]={0, };

    // G_STRAT->strategy_id = strategy_no;
    memcpy(G_STRAT->strategy_name  , bd->sf_mrgn_rule_id                   , sizeof(bd->sf_mrgn_rule_id         )); // 현선물차익거래룰ID (전략명)
    memcpy(G_STRAT->spot_sym       , bd->fx_prdct_cd                       , sizeof(bd->fx_prdct_cd             ));
    memcpy(G_STRAT->fut_sym        , bd->ftrs_items_cd                     , sizeof(bd->ftrs_items_cd           ));
    G_STRAT->base_lot_fut          = str2int(bd->ftrs_base_ordr_unit_qty   , sizeof(bd->ftrs_base_ordr_unit_qty ));
    G_STRAT->base_lot_spot         = str2int(bd->fx_base_ordr_unit_qty     , sizeof(bd->fx_base_ordr_unit_qty   ));
    G_STRAT->max_lot_abs_fut       = str2int(bd->ftrs_max_ordr_unit_qty    , sizeof(bd->ftrs_max_ordr_unit_qty  ));
    G_STRAT->max_lot_abs_spot      = str2int(bd->fx_max_ordr_unit_qty      , sizeof(bd->fx_max_ordr_unit_qty    ));
    G_STRAT->entry_spread_bp       = str2double(bd->ent_sprd               , sizeof(bd->ent_sprd                ));
    G_STRAT->exit_spread_bp        = str2double(bd->lqdt_sprd              , sizeof(bd->lqdt_sprd               ));
	G_STRAT->entry_start_time      = str2int(bd->ent_sched_hms             , sizeof(bd->ent_sched_hms           ));
	
	close_t1_tp = str2int(bd->lqdt_start_sprd_dstcd , sizeof(bd->lqdt_start_sprd_dstcd));
    if (close_t1_tp != 9)
    {
        G_STRAT->close_spread_enabled_t1 = 1;     // 1차청산 사용   : +1
        G_STRAT->close_t1              = str2int(bd->lqdt_start_sched_hms     , sizeof(bd->lqdt_start_sched_hms      ));
        G_STRAT->close_spread_bp_t1    = str2double(bd->lqdt_start_sprd       , sizeof(bd->lqdt_start_sprd           ));
    }
    else
	{
        G_STRAT->close_spread_enabled_t1 = -1;   // 1차청산 미사용 : -1
		G_STRAT->close_t1 = 999999;
	}

	close_t2_tp = str2int(bd->lqdt_cmpl_sprd_dstcd  , sizeof(bd->lqdt_cmpl_sprd_dstcd ));
    if (close_t2_tp != 9)
    {
        G_STRAT->close_spread_enabled_t2 = 1;     // 2차청산 사용   : +1
        G_STRAT->close_t2                = str2int(bd->lqdt_cmpl_sched_hms    , sizeof(bd->lqdt_cmpl_sched_hms       ));
        G_STRAT->close_spread_bp_t2      = str2double(bd->lqdt_cmpl_sprd      , sizeof(bd->lqdt_cmpl_sprd            ));
    }
    else
	{
        G_STRAT->close_spread_enabled_t2 = -1;    // 2차청산 미사용   : -1
		G_STRAT->close_t2 = 999999;
	}

    G_STRAT->hard_cut_time  = str2int(bd->last_lqdt_end_sched_hms  , sizeof(bd->last_lqdt_end_sched_hms ));
    if (G_STRAT->hard_cut_time != 999999)
    {
        G_STRAT->hard_cut_enabled  = 1;     // 하드컷 사용 : +1
    }
    else
        G_STRAT->hard_cut_enabled  = -1;    // 하드컷 미사용 : -1

	arb_tp = str2int(bd->mrgn_drct_dstcd , sizeof(bd->mrgn_drct_dstcd)); // 차익방향구분코드 - 1.매수, 2.매도, 3.양방향
    if (arb_tp == 1)
        G_STRAT->arb_dir_mode   = ARB_BUY;
    else if (arb_tp == 2)
        G_STRAT->arb_dir_mode   = ARB_SELL;
    else
        G_STRAT->arb_dir_mode   = ARB_BOTH;

    exec_mode = str2int(bd->ordr_prity_dstcd , sizeof(bd->ordr_prity_dstcd)); // 주문우선순위구분코드 - 1.동시, 2.선물
    if (exec_mode == 1)
        G_STRAT->exec_mode = EX_SIMUL;
    else
        G_STRAT->exec_mode = EX_FUT_FIRST;

    G_STRAT->basis_cd             = str2int(bd->basis_calc_dstcd   , sizeof(bd->basis_calc_dstcd   )); // 베이시스계산구분코드 - 1.Swap, 2.이론스프레드(딜러수기입력)
    G_STRAT->basis_spread         = str2double(bd->basis_calc_sprd , sizeof(bd->basis_calc_sprd    )); // 베이시스계산스프레드
    G_STRAT->strategy_stat        = str2int(bd->sf_mrgn_stus_dstcd , sizeof(bd->sf_mrgn_stus_dstcd )); // 현선물차익거래상태구분코드 - 1.구동, 2:일시정지, 3:정지
    G_STRAT->seq_no               = str2int(bd->seq_no             , sizeof(bd->seq_no             )); // 전략인덱스(전략Slot 인덱스 결정)

	memcpy(ord_type_fut , bd->ftrs_ofpr_dstcd     , sizeof(bd->ftrs_ofpr_dstcd));
    if (ord_type_fut[0] == 'T')
	{
        G_STRAT->ord_type_fut  = ORD_TYPE_M;  // 시장가주문
		G_STRAT->tif_type_fut  = OT_FAS;      // 시장가일때 FAS 주문만 가능함
    }
	else
	{
        G_STRAT->ord_type_fut  = ORD_TYPE_L;  // 지정가주문
		G_STRAT->tif_type_fut  = OT_FOK;      // 지정가일때는 FOK 
	}

    memcpy(ord_type_spot, bd->fx_ofpr_dstcd  , sizeof(bd->fx_ofpr_dstcd));
    if (ord_type_spot[0] == '2')
        G_STRAT->ord_type_spot = ORD_TYPE_L;  // 지정가주문
	G_STRAT->tif_type_spot = OT_FAK;  		  // 현물은 무조건 IOC 주문처리 

    memcpy(G_STRAT->fcm_acno_spot      , bd->fx_fcm_acno                , sizeof(bd->fx_fcm_acno       )); // 현물FCM계좌번호
    memcpy(G_STRAT->fcm_acno_fut       , bd->ftrs_fcm_acno              , sizeof(bd->ftrs_fcm_acno     )); // 선물FCM계좌번호
    memcpy(G_STRAT->fcm_id_fut         , bd->ftrs_fcm_id                , sizeof(bd->ftrs_fcm_id       )); // 선물FCMID
    memcpy(G_STRAT->ref_no             , bd->ref_no                     , sizeof(bd->ref_no            )); // 참조번호(1 구분자 + 8 주문일자 + 6 일런번호)
    memcpy(G_STRAT->pay_dt             , bd->pay_dt                     , sizeof(bd->pay_dt            )); // 선물결제일자 (Swap Point 조회용)
    memcpy(G_STRAT->bk_no              , bd->bk_no                      , sizeof(bd->bk_no             )); // 북번호
    memcpy(G_STRAT->lgen_no            , bd->lgen_no                    , sizeof(bd->lgen_no           )); // 거래참여자번호
    memcpy(G_STRAT->trdr_no            , bd->trdr_no                    , sizeof(bd->trdr_no           )); // 트레이더번호
    
	G_STRAT->order_interval_ms         = str2int(bd->ordr_intval_ms     , sizeof(bd->ordr_intval_ms    )); // 주문간격(ms)
	G_STRAT->retry_cnt                 = str2int(bd->retry_cnt          , sizeof(bd->retry_cnt         )); // 현물주문재시도횟수 
    G_STRAT->retry_interval_ms         = str2int(bd->retry_intval_ms    , sizeof(bd->retry_intval_ms   )); // 현물주문재시도간격
	
    G_STRAT->auto_mode                 = 1;   // 전략 구동
    G_STRAT->auto_mode_deadband        = 0.0; // 시세 노이즈 방지용. 사용이 필요할 경우 셋팅하여 사용
	
	LOG_ERR("============================== ARB 전략 Setting Start ==================================");    
	LOG_ERR(" 1.today                   [%d]", G_STRAT->today                    );    
    LOG_ERR(" 2.strategy_id             [%d]", G_STRAT->strategy_id              ); 
    LOG_ERR(" 3.strategy_stat           [%d]", G_STRAT->strategy_stat            );    
    LOG_ERR(" 4.aptype                  [%s]", G_STRAT->aptype                   ); 
    LOG_ERR(" 5.strategy_name           [%s]", G_STRAT->strategy_name            );    
    LOG_ERR(" 6.spot_sym                [%s]", G_STRAT->spot_sym                 );      
    LOG_ERR(" 7.fut_sym                 [%s]", G_STRAT->fut_sym                  );    
    LOG_ERR(" 8.base_lot_spot           [%d]", G_STRAT->base_lot_spot            );        
    LOG_ERR(" 9.base_lot_fut            [%d]", G_STRAT->base_lot_fut             );        
    LOG_ERR("10.max_lot_abs_fut         [%d]", G_STRAT->max_lot_abs_fut          );        
    LOG_ERR("11.max_lot_abs_spot        [%d]", G_STRAT->max_lot_abs_spot         );        
    LOG_ERR("12.entry_spread_bp         [%f]", G_STRAT->entry_spread_bp          ); 
    LOG_ERR("13.exit_spread_bp          [%f]", G_STRAT->exit_spread_bp           );
    LOG_ERR("14.entry_start_time        [%d]", G_STRAT->entry_start_time         ); 
    LOG_ERR("15.close_t1                [%d]", G_STRAT->close_t1                 ); 
    LOG_ERR("16.close_t2                [%d]", G_STRAT->close_t2                 ); 
    LOG_ERR("17.hard_cut_time           [%d]", G_STRAT->hard_cut_time            ); 
    LOG_ERR("18.close_spread_enabled_t1 [%d]", G_STRAT->close_spread_enabled_t1  ); 
    LOG_ERR("19.close_spread_bp_t1      [%f]", G_STRAT->close_spread_bp_t1       ); 
    LOG_ERR("20.close_spread_enabled_t2 [%d]", G_STRAT->close_spread_enabled_t2  ); 
    LOG_ERR("21.close_spread_bp_t2      [%f]", G_STRAT->close_spread_bp_t2       ); 
    LOG_ERR("22.hard_cut_enabled        [%d]", G_STRAT->hard_cut_enabled         ); 
    LOG_ERR("23.hard_cut_bp             [%f]", G_STRAT->hard_cut_bp              ); 
    LOG_ERR("24.tif_type_fut            [%d]", G_STRAT->tif_type_fut             ); 
    LOG_ERR("25.tif_type_spot           [%d]", G_STRAT->tif_type_spot            ); 
    LOG_ERR("26.fill_pref_entry         [%d]", G_STRAT->fill_pref_entry          ); 
    LOG_ERR("27.ord_type_fut            [%d]", G_STRAT->ord_type_fut             ); 
    LOG_ERR("28.ord_type_spot           [%d]", G_STRAT->ord_type_spot            ); 
    LOG_ERR("29.exec_mode               [%d]", G_STRAT->exec_mode                ); 
    LOG_ERR("30.auto_mode               [%d]", G_STRAT->auto_mode                ); 
    LOG_ERR("31.auto_mode_deadband      [%f]", G_STRAT->auto_mode_deadband       ); 
    LOG_ERR("32.arb_dir_mode            [%d]", G_STRAT->arb_dir_mode             ); 
    LOG_ERR("33.order_interval_ms       [%d]", G_STRAT->order_interval_ms        );
    LOG_ERR("34.pos_spot                [%d]", G_STRAT->pos_spot                 );
    LOG_ERR("35.pos_fut                 [%d]", G_STRAT->pos_fut                  ); 
    LOG_ERR("36.basis_cd                [%d]", G_STRAT->basis_cd                 ); 
    LOG_ERR("37.basis_spread            [%f]", G_STRAT->basis_spread             ); 
    LOG_ERR("38.fcm_id_fut              [%s]", G_STRAT->fcm_id_fut               );  
    LOG_ERR("39.fcm_acno_spot           [%s]", G_STRAT->fcm_acno_spot            );  
    LOG_ERR("40.fcm_acno_fut            [%s]", G_STRAT->fcm_acno_fut             ); 
    LOG_ERR("41.ref_no                  [%s]", G_STRAT->ref_no                   ); 
    LOG_ERR("42.pay_dt                  [%s]", G_STRAT->pay_dt                   );
    LOG_ERR("43.bk_no                   [%s]", G_STRAT->bk_no                    );  
    LOG_ERR("44.lgen_no                 [%s]", G_STRAT->lgen_no                  ); 
    LOG_ERR("45.trdr_no                 [%s]", G_STRAT->trdr_no                  );   
    LOG_ERR("46.seq_no                  [%d]", G_STRAT->seq_no                   );     
    LOG_ERR("47.retry_cnt               [%d]", G_STRAT->retry_cnt                );
    LOG_ERR("48.retry_interval_ms       [%d]", G_STRAT->retry_interval_ms        );
	LOG_ERR("============================== ARB 전략 Setting End   ==================================");    

    return ;
}

// ------------------------------------------------------------
//  주문 단계
// ------------------------------------------------------------
static int order_process( )
{
    int rc = 0;
	int gb = 0;
    int now = get_hhmmss_now();

	if (now >= 180000)
	{
        LOG_ERR("[주문불가시간] now[%d] 영업일이 변경되어 주문이 불가합니다.", now);
		if (arb_stop_msg_send(E9999_MSG) < 0) // system error
	    	LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
		return ARB_STOP_CD;
	}

	// 새 세트번호로 진입 -- 20251114 주문 완료시 ARB_MAIN으로 갈 수도 있음 전량 체결 후에 세트 인덱스 증가하는것으로 변경 
	if (G_SET_CNT ==  0) // 최초진입시 셋팅 
    {
        G_SET_CNT = 1;
        G_SET->set_id  = 1;
		G_SET->valid  = SET_VALID;
		LOG_DEBUG("[SET] 세트번호 채번 g_set_idx[%d] set_id[%d] ", g_set_idx, G_SET->set_id);
    }
	
	// 시세 스냅샷
	rc = market_update_snapshot();
	if (rc != 0)
	{
        return rc; // 호가이상케이스.. 일단은 대기하면서 정상호가 수신을 기다린다. 
	}

	// 주문플랜검증
	rc = order_plan_auto(&gb);
    if (rc != 0)
    {
        LOG_WARN("[주문플랜검증] 플랜 미충족.. rc = [%d]", rc);
        return rc;
    }

    LOG_INFO("*************************************************************************");
    LOG_INFO("주문START 전략Idx[%d] setIdx[%d] exec_mode=[%d] phase=[%d]", g_strat_idx, g_set_idx, G_STRAT->exec_mode, gb); 
    LOG_INFO("**************************************************************************");
    LOG_INFO("******************총 선물포지션 [%d] 총 현물포지션[%d]********************", G_STRAT->pos_fut, G_STRAT->pos_spot ); 

	rc = execute_order_pair(gb); // 현물 /선물 주문
    if (rc != 0)
    {
        return rc;
    }
	return 0;
}

// ------------------------------------------------------------
// 전체 초기화 
// ------------------------------------------------------------
int Arb_Init(char *aptype, char *msg)
{
    int rc = 0;
	int today = get_today();
	char tmp[6+1] = {0,} ;
	char logname[64] ;

	memcpy(tmp, msg, 6);
	g_strat_idx = atoi(tmp) - 1; // 전략실행일련번호(전략인덱스)

    // config
    if (load_config("/fsfxwin/fep/arb/conf/config.ini") != 0) {
        fprintf(stderr, "Config load failed!\n");
        return ARB_STOP_CD;
    }
    // 로그 초기화
    // log_init_default();
    //if (log_add_file(g_cfg.log_file, 1) != 0)
	
#if 1	
	snprintf(logname, sizeof(logname),"%s", pname);
	if (log_open_with_id(g_cfg.log_file, pname, g_strat_idx+1) != 0)
	{
        LOG_WARN("failed to open log file, keep stderr only");
        return ARB_STOP_CD;
	}
    log_set_level(g_cfg.log_level);

    BaseName();
	LOG_ERR("Arb INIT Start.....[%s] IDX [%d] ", g_progname, g_strat_idx);
#else 
	snprintf(logname, sizeof(logname),"%s_%d", pname, g_strat_idx + 1);
	if (log_open_daily(g_cfg.log_file, logname) != 0)
    {
        LOG_WARN("failed to open log file, keep stderr only");
    }
    log_set_level(g_cfg.log_level);

    BaseName();
	LOG_INFO("Arb INIT Start.....[%s] IDX [%d]", g_progname, g_strat_idx);
#endif

    // 전략 공유메모리 생성
    rc = arb_shm_init();
    if (rc != 0) 
    { 
	    LOG_ERR("[INIT] 전략공유메모리 생성 실패 !!!! --- rc = %d", rc );
		if (arb_stop_msg_send(E9999_MSG) < 0) // system error
	    	LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
	    return ARB_STOP_CD; 
    }
    
    // 메시지 큐 init
	g_cfg.spot_order_id  = msg_queue_init(MSG_TYPE_SPOT_ORD);
	if( g_cfg.spot_order_id == -1)
	{
	    LOG_ERR("현물 주문 msgque init 실패 !!");
		if (arb_stop_msg_send(E9999_MSG) < 0) // system error
	    	LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
	    return ARB_STOP_CD;
    }
	
	g_cfg.spot_ordlog_id = msg_queue_init(MSG_TYPE_SPOT_ORDLOG);
	if( g_cfg.spot_ordlog_id == -1)
	{
	    LOG_ERR("[INIT] 현물 주문 후처리 msgque init 실패 !!");
		if (arb_stop_msg_send(E9999_MSG) < 0) // system error
	    	LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
	    return ARB_STOP_CD;
    }
	
	g_cfg.fut_ordlog_id  = msg_queue_init(MSG_TYPE_FUT_ORDLOG);
	if( g_cfg.fut_ordlog_id == -1)
	{
	    LOG_ERR("[INIT] 선물 주문 후처리 msgque init 실패 !!");
		if (arb_stop_msg_send(E9999_MSG) < 0) // system error
	    	LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
	    return ARB_STOP_CD;
    }

	// 전략 메모리 초기화 (일자가 바뀌면 초기화)
    if( G_STRAT->today == 0 || G_STRAT->today != today) 
	{
	    LOG_ERR("[INIT] 영업일 변경으로 전략 슬롯[%d]메모리 초기화", g_strat_idx);
        memset(&g_shared->slots[g_strat_idx].strategy, 0 , sizeof(StrategyMem));
        memset(&g_shared->slots[g_strat_idx].ordno   , 0 , sizeof(OrdNo));
        memset(g_shared->slots[g_strat_idx].sets     , 0 , sizeof(SetState) * MAX_SETS_PER_STRAT);
		G_STRAT->today = get_today();
		memcpy(G_STRAT->aptype, aptype, strlen(aptype));
		G_STRAT->aptype[7] = '\0';
		G_SET_CNT = 0;  
		G_SET->valid = SET_VALID;
	}
	// if( G_SET_CNT == 0) G_SET_CNT = 1;

	/* 주문번호 대역 초기 셋팅 */
	G_ORDNO->fut_ordno_base  = g_cfg.futoid + (g_strat_idx * g_cfg.oidrange);   // 선물주문번호 대역베이스
	G_ORDNO->fut_ordno_end   = G_ORDNO->fut_ordno_base + g_cfg.oidrange -1;     // 선물주문번호 대역마지막
	G_ORDNO->fut_ordno_last  = g_cfg.futoidlast;                                // 선물주문번호 최종
	G_ORDNO->fut_ordno = 0; // 전략인덱스별 주문번호 초기화

	G_ORDNO->spot_ordno_base = g_cfg.spotoid + (g_strat_idx * g_cfg.oidrange);  // 현물주문번호 대역베이스
	G_ORDNO->spot_ordno_end  = G_ORDNO->spot_ordno_base + g_cfg.oidrange -1;    // 현물주문번호 대역마지막
	G_ORDNO->spot_ordno_last = g_cfg.spotoidlast;                               // 현물주문번호 최종
	G_ORDNO->spot_ordno = 0; // 전략인덱스별 주문번호 초기화

    // 기동전 체크 사항 
    rc = startup_set_check();
	if( rc != 0)
	{
	    LOG_ERR("[INIT] 기동전 상태 오류 ... rc = (%d)", rc );
		if (arb_stop_msg_send(E9999_MSG) < 0) // system error
	    	LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
		return ARB_STOP_CD;
	}

    g_initialized = 1;
	LOG_ERR("[INIT] 기동전 상태 점검 완료");

    // 테스트 모드 여부 판단 
    g_test_mode = g_cfg.test_mode; 

    if(!g_test_mode) // 운영 모드
    {
        LOG_ERR("[INIT] 전략 셋팅 진입 START");
	    set_strategy_data(msg);
    }
    else // 테스트 모드 .. 기본값 주입
    {
		;
    }

    // 상품 정보 attach 
    rc = md_shm_init(); 
    if (rc != 0) 
    {
        LOG_ERR("md_shm_init failed");
        return ARB_STOP_CD;
    }
    LOG_INFO("[INIT] Market Shared memory attached" );

    return 0;
}

// ------------------------------------------------------------
// Arb_Process
// ------------------------------------------------------------
int Arb_Process(char *msg)
{
    int rc;
	int now = get_hhmmss_now();
	(void)msg;

    if (g_initialized != 1)
    {
		LOG_ERR("** 전략 초기화 실패로 전략 재기동 필요함**");
		if (arb_stop_msg_send(E9999_MSG) < 0) // 9999 system error
	    	LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
		return ARB_STOP_CD;
	}
	
	if (now >= 180000) // 현재 장운영 정보를 받지 않음. 원장과 협의하여 선물 영업일이 변경되는 18시 이후는 주문을 막는것으로 조치 
	{
        LOG_ERR("[주문불가시간] now[%d] 영업일이 변경되어 주문이 불가합니다.", now);
		if (arb_stop_msg_send(E5002_MSG) < 0) // 5002 영업일 변경 
	    	LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
		return ARB_STOP_CD;
	}

    // 세트 잔고 체크
    LOG_INFO("[%d]set valid[%d] position fut[%d] spot[%d]", g_set_idx, G_SET->valid, POS_FUT, POS_SPOT);

	if ((abs(POS_FUT)  == G_STRAT->base_lot_fut) 
	&&  (abs(POS_SPOT) == G_STRAT->base_lot_spot)) // 세트잔고와 기본수량이 같으면 세트 주문이 정상적으로 나간것이니 다음 주문 가능
    {
	    if (G_SET->valid != SET_NOT_VALID) // but, vaild 가 완료상태가 아니면..
		{
		    LOG_INFO("[%d] Set가 끝나지 않아 신규 진입 대기..", g_set_idx);
		    return 0;
        }

	    /* 해당 주문로그를 확인해서 해당주문set이 완료되지 않은 주문을 확인 */
		SetOrdLog *log = &G_SET->ordlog;
		for (int i=0; i< log->n; i++ )
		{
		     SetOrdLogEnt *oent = &log->ent[i]; 
			 LOG_INFO("leg=(%d) ord_id=(%ld) ordtype=(%d), side=(%d), qty=(%d),fill_qty(%d) time=(%d)", 
				        oent->leg, oent->ord_id, oent->ordtype, oent->side, oent->qty, oent->filled_qty ,oent->ts_hhmmss );
			 if (oent->valid == 1)
			 {
			     LOG_WARN("[%d]Set 완료되지 않은 주문 존재해서 진입 Skip ...", g_set_idx);
			     LOG_WARN("leg=(%d) ord_id=(%ld) ordtype=(%d), side=(%d), qty=(%d),fill_qty(%d) time=(%d)", 
				        oent->leg, oent->ord_id, oent->ordtype, oent->side, oent->qty, oent->filled_qty ,oent->ts_hhmmss );
				 return 0;
			 }
		}
        rc = order_process( );
		if (rc !=0)
		{
            //LOG_WARN( "진입 주문 대기 ....rc = (%d) ", rc );
			return rc;
		}
    }
	else if (POS_FUT == 0 && POS_SPOT == 0) // 둘다 0 이므로 신규 진입 가능
	{ 
		if (G_SET->valid != SET_VALID)
		{
			LOG_ERR("종료된 세트면 진입 안함. fut[%d], spot[%d]", POS_FUT, POS_SPOT);
			return -1;
		}
		
		/* 해당 주문로그를 확인해서 해당주문set이 완료되지 않은 주문을 확인 */
		SetOrdLog *log = &G_SET->ordlog;
		
		//LOG_DEBUG("log->n=(%d)", log->n); 
		for (int i=0; i< log->n; i++ )
		{
		     SetOrdLogEnt *oent = &log->ent[i]; 
			 if (oent->valid == 1)
			 {
			     LOG_WARN("[%d] Set 완료되지 않은 주문 존재해서 진입 Skip..", g_set_idx);
			     LOG_WARN("leg=(%d) ord_id=(%ld) ordtype=(%d), side=(%d), qty=(%d), fill_qty(%d) time=(%d)", 
				        oent->leg, oent->ord_id, oent->ordtype, oent->side, oent->qty, oent->filled_qty, oent->ts_hhmmss);
				 return 0;
			 }
		}
		
		rc = order_process( );
		if (rc !=0)
		{
            //LOG_WARN( "진입 주문 대기 ....rc = (%d) ", rc );
			return rc;
		}
	}
	else // 한쪽만 포지션이 있는 상태.. 실시간으로 체결 응답이 추가적으로 들어올 수 있으므로 대기한다. 
	{
	 	LOG_WARN( "[%d]set position 불균형 상태로 다음 세트 진입 대기 fut[%d]spot[%d]",g_set_idx, POS_FUT, POS_SPOT); 
		return -1;
	}

    return 0;

}

// ------------------------------------------------------------
//  현물 주문/체결 응답 처리
// ------------------------------------------------------------
int Arb_Spot_Exec(int gb, int side, long ord_id, long fill_qty, double fill_px)
{
    int rc =0;
	int wait_ms    = 0;
    
	FillEvent fill;

	// 현물 체결시 주문 Flow by.kong 
	// case 1. 양방향 주문(시장가) : 선물, 현물중 어느것이 먼저 체결될지 모른다.
	// 현물 체결이 들어왔다면, 선물과 현물 세트 수량이 일치하는지 확인한다. 일치하면 해당 세트는 완료 된다.
	// 거부가 되었으면 retry 횟수만큼 retry 간격을 두고 재주문한다.
	// 단, 다시 거부응답을 받고나서 진행한다. 응답을 받지 않고 진행하면 의도치 않게 현물 포지션이 확대될 수 있다.
	// case 2. 선물우선주문(시장가/지정가) : 선물 체결 후 현물 주문을 전송한다 -> 선물 지정가주문은 체결안될수도 있음
	// 이 경우에는 해당 세트를 무효화(완료) 처리한 후 다음 세트로 진행한다. -> 이 Flow는 현물에서 처리할 필요없이 선물 응답에서 진행
	// 현물체결이 들어왔다면, 선물과 현물 세트 수량이 일치하는지 확인한다. 일치하면 해당 세트는 완료 된다.

    memset(&fill, 0x00, sizeof(FillEvent));

	// 현세트에 대한 주문임을 확인한다. 맞으면 fill ++
	if (G_ORDNO->spot_ordno != ord_id) 
		return 0;
		
	if (gb == 1) // 체결일때 
	{
		G_SET->spot_reject_stat  = 1; // 체결응답 
		G_SET->spot_fill_stat = 1; // 체결

		fill.ord_id = ord_id;
		if (side == 1) // 1-매수 (**현물방향주의)
			fill.side = +1;
		else           // 2-매도 (**현물방향주의)
			fill.side = -1;
	
		fill.fill_px  = fill_px;
		fill.fill_qty = fill_qty;
		
		LOG_ERR("[Arb_Spot_Exec] 현물 체결 발생 [ord_id=%ld side=%d qty=%d px=%.2f]", ord_id, side, fill_qty, fill_px);
		rc = handle_spot_fill_event(&fill);
		if( rc !=0)
		{
			LOG_ERR("현물 체결 handle_spot_fill_event.. continue. rc=%d",rc);
			return rc ;
		}

	}
	else if(gb == -1)  // 거부일때 
	{
		G_SET->spot_reject_stat  = -1; // 거부응답 

		LOG_ERR("[Arb_Spot_Exec] 현물 거부 발생 [ord_id=%ld]", ord_id);
		if (POS_FUT != 0 && POS_SPOT == 0)
		{	
			if (G_SET->real_retry_cnt < G_STRAT->retry_cnt)
			{
				// 선물주문이 체결된 상태이므로 현물주문 retry
				if (POS_FUT < 0)
					fill.side = 1;  //선물매도주문이 나간 상태이므로 현물매수주문
				else
					fill.side = -1; //선물매수주문이 나간 상태이므로 현물매도주문

			    int now = get_hhmmss_now();
			    if (now >= G_STRAT->hard_cut_time)
				{
				    fill.fill_qty  = abs(G_STRAT->pos_spot);
					LOG_INFO("[SPOT_RETRY] 현물 주문 하드컷 수량[%d]",fill.fill_qty);
			    }
				else
					fill.fill_qty  = G_STRAT->base_lot_spot;
				
				G_SET->spot_reject_stat = 0; // 미응답상태로 clear..
				wait_ms = G_STRAT->retry_interval_ms;
				if (wait_ms > 0)
				{
					LOG_ERR("[SPOT_RETRY] retry_cnt = %d wait=%d ms before re-order", G_STRAT->retry_cnt, wait_ms);
					usleep(wait_ms * 1000);
					LOG_ERR("[SPOT_RETRY] wake..");
				}
				rc = spot_order_process(&fill);
				if (rc != 0)
				{
					LOG_ERR("[SPOT_RETRY] 현물 재주문 spot_order_process error.. rc=%d",rc);
					return rc ;
				}
				G_SET->real_retry_cnt ++; // 재시도 횟수 증가 - 세트메모리에 관리. 실제 사용 카운트
				return 0;
			}
			else
			{
				LOG_ERR("[SPOT_RETRY] 현물 재주문 시도 횟수초과 (max=%d). ARB 전략 중단!!", G_STRAT->retry_cnt);
				if (arb_stop_msg_send(E5103_MSG) < 0) // 5103 현물재시도횟수 초과 
					LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
				return ARB_STOP_CD;
			}
		}
		else
		{
 			 if (G_SET->fut_reject_stat == -1) // 선물도 거부상태
			 {
			 	LOG_ERR("현물[spot=%ld]거부/선물[fut=%ld] 거부상태. ARB 전략 중단!!", G_ORDNO->spot_ordno, G_ORDNO->fut_ordno);
				if (arb_stop_msg_send(E5203_MSG) < 0) // 5203 현물선물주문거부  
					LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
			 	 return ARB_STOP_CD;
			 }
			 else if (G_SET->fut_reject_stat == 0) // 선물미응답 상태
			 {
			 	LOG_ERR("현물[spot=%ld]거부/선물[fut=%ld] 주문 미응답 상태", G_ORDNO->spot_ordno, G_ORDNO->fut_ordno);
				return 0; // 일단 나간다..
			 }
 			 else if (G_SET->fut_reject_stat == -2) // 선물 Auto Cancel
			 									    // 발생불가한 케이스 - 선물우선주문에서만 Auto Cancel 발생하고, 
													// Auto Cancel 시 현물주문을 내지 않고 다음 세트로 넘어가기 때문에 발생하지 않을 케이스다.)
			 {
			 	LOG_ERR("현물[spot=%ld]거부/선물[fut=%ld] Auto Cancel 상태. ARB 전략 중단!!", G_ORDNO->spot_ordno, G_ORDNO->fut_ordno);
				if (arb_stop_msg_send(E5203_MSG) < 0) // 5203 현물선물주문거부  
					LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
			 	return ARB_STOP_CD;
			 }
		}
	}
	else
	{
	 	LOG_ERR("Code Error.. gb [%d].. Arb process Stop..", gb);
		if (arb_stop_msg_send(E9999_MSG) < 0) // system error 
			LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
		return ARB_STOP_CD; 
	}

	return 0;
}
// ------------------------------------------------------------
//  선물 주문/체결 응답처리
// ------------------------------------------------------------
int Arb_Fut_Exec(int gb, int side,  long ord_id, long fill_qty, double fill_px)
{
    int rc =0;
    FillEvent fill;
	int wait_ms    = 0;

	// 선물 주문 Flow by.kong 
	// case 1. 양방향 주문(시장가) : 선물, 현물중 어느것이 먼저 체결될지 모른다.
	// 선물 체결이 들어왔다면, 선물과 현물 세트 수량이 일치하는지 확인한다. 일치하면 해당 세트는 완료 된다.
	// 거부가 되었으면, 거부 케이스를 확인한다. 시장가/FAS 주문이므로 Auto Cancel은 발생하지 않을것. 거래소 거부이므로 전략 종료
	// case 2. 선물우선주문(시장가/지정가) : 선물 체결 후 현물 주문을 전송한다 -> 선물 지정가주문은 체결안될수도 있음
	// 이 경우에는 해당 세트를 무효화(완료) 처리한 후 다음 세트로 진행한다. -> 선물 응답에서 진행
	// 선물 체결이 들어왔다면, 현물 주문을 낸다.

    memset(&fill, 0x00, sizeof(FillEvent));

	// 현세트에 대한 주문임을 확인한다. 
	if (G_ORDNO->fut_ordno != ord_id)
		return 0;

	if (gb == 1) // 체결일때 
	{
		G_SET->fut_reject_stat  = 1; // 체결응답 
		G_SET->fut_fill_stat = 1; // 체결응답 

		fill.ord_id = ord_id;
		if (side == 1) // 1-매도 (선물**방향주의)
			fill.side = -1;
		else           // 2-매수 (선물**방향주의)
			fill.side = 1;
		fill.fill_px  = fill_px;
		fill.fill_qty = fill_qty;
		
		LOG_ERR("[Arb_Fut_Exec] 선물 체결 발생 ord_id=%ld side=%d qty=%d px=%.2f", ord_id, side, fill_qty, fill_px);
		rc = handle_fut_fill_event(&fill);
		if (rc !=0)
		{
			LOG_ERR("fut 체결 handle_fut_fill_event.. continue. rc=%d",rc);
			return rc ;
		}
		
		if (G_SET->spot_reject_stat == -1) // 현물이 거부응답 받은 상태라면..
		{
			if (G_SET->real_retry_cnt < G_STRAT->retry_cnt)
			{
				// 선물주문이 체결된 상태이므로 현물주문 retry
				if (POS_FUT < 0)
					fill.side = 1;  //선물매도주문이 나간 상태이므로 현물매수주문
				else
					fill.side = -1; //선물매수주문이 나간 상태이므로 현물매도주문

			   // 시간체크 추가
			    int now = get_hhmmss_now();
			    if (now >= G_STRAT->hard_cut_time)
				{
				    fill.fill_qty  = abs(G_STRAT->pos_spot);
					LOG_INFO("[SPOT_RETRY] 현물 주문 하드컷 수량[%d]",fill.fill_qty);
			    }
				else
					fill.fill_qty  = G_STRAT->base_lot_spot;
				
				G_SET->spot_reject_stat = 0; // 미응답상태로 clear..
				wait_ms = G_STRAT->retry_interval_ms;
				if (wait_ms > 0)
				{
					LOG_ERR("[SPOT_RETRY] retry_cnt = %d wait=%d ms before re-order", G_STRAT->retry_cnt, wait_ms);
					usleep(wait_ms * 1000);
					LOG_ERR("[SPOT_RETRY] wake..");
				}
				rc = spot_order_process(&fill);
				if (rc != 0)
				{
					LOG_ERR("[SPOT_RETRY] 현물 재주문 spot_order_process error.. rc=%d",rc);
					return rc ;
				}
				G_SET->real_retry_cnt ++; // 재시도 횟수 증가 - 세트메모리에 관리. 실제 사용 카운트
				return 0;
			}
			else
			{
				LOG_ERR("[SPOT_RETRY] 현물 재주문 시도 초과 (max=%d). ARB 전략 중단!!", G_STRAT->retry_cnt);
				if (arb_stop_msg_send(E5103_MSG) < 0) // 5103 재시도 횟수 초과 
					LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
				return ARB_STOP_CD;
			}
		}
	}
	else if (gb == -1)  // 거부일때 (거래소 거부 Case 로 판단.) 
	{
		G_SET->fut_reject_stat  = -1; // 거부응답 
		G_SET->fut_fill_stat = 0; // 미체결

		if (POS_SPOT != 0 && POS_FUT == 0)
		{	
			LOG_ERR("(선물[fut=%ld])주문 거부상태.. 거부사유 확인 요망 ARB 전략 중단!!", G_ORDNO->fut_ordno);
			if (arb_stop_msg_send(E5201_MSG) < 0) // 5201 선물주문거부 
				LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
			return ARB_STOP_CD;
		}
		else
		{
			 // 전략 중지시키고 alarm 
			 if (G_SET->spot_reject_stat == -1) // 현물도 거부상태일때..
			 {
			 	 LOG_ERR("현물[%ld]/선물[%ld]거부 상태 Set 무효화.. ARB 전략 중단!!", G_ORDNO->spot_ordno, G_ORDNO->fut_ordno);
				 if (arb_stop_msg_send(E5203_MSG) < 0) // 5203 현물선물주문거부 
					LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
			 	 return ARB_STOP_CD;
			 }
			 else if (G_SET->spot_reject_stat == 0) // 현물미응답 상태이나.. Auto Cancle Case가 아니므로 중단.
			 {
				LOG_ERR("현물[%ld]미응답/선물[%ld]거부 상태.. 선물거부사유 확인 ARB 전략 중단!!", G_ORDNO->spot_ordno, G_ORDNO->fut_ordno);
				if (arb_stop_msg_send(E5201_MSG) < 0) // 5201 선물주문거부 
					LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
				return ARB_STOP_CD;
			 }
		}
	}
	else if (gb == -2) // KRX Auto Cancel 일경우 - 선물우선(지정가FOK) 주문일때 발생
	{
		// 20251118 류익상 수석 요청, 지정가 FOK 주문일때 선물 취소된 경우에는 해당 세트를 무효화하고 다음세트로 진행한다.
		G_SET->fut_reject_stat  = -2; // Auto Cancel
		G_SET->fut_fill_stat = 0; // 미체결
		
		//  case 1.선물우선이라 현물주문이 안나간 경우 - 다음 세트로 넘겨서 진행
		if (G_SET->spot_reject_stat == 0) 
		{
			 if (G_STRAT->exec_mode == EX_FUT_FIRST)
			 {
				G_SET->valid = SET_NOT_VALID;
				LOG_ERR("[Cancel-Fut] 선물[fut=%ld])주문 Auto Cancel! 현물미완료(%d) 완료..다음 Set 진입", G_ORDNO->fut_ordno, POS_SPOT);
				LOG_ERR("[Cancel-Fut] Sleep... [%d]", G_STRAT->order_interval_ms * 1000);
				usleep(G_STRAT->order_interval_ms * 1000);
				LOG_ERR("[Cancel-Fut] Wake...");
				// 새 세트 증가
				generate_set_id();
				return 0;
			 }
			 else
			 {   // 발생하지 않을 케이스(양방향 : 선물 -시장가(FAS 이므로 발생X))
				 LOG_ERR("현물[spot=%ld]미응답/선물[fut=%ld]취소 상태 skip..", G_ORDNO->spot_ordno, G_ORDNO->fut_ordno);
			 	 return 0;
  		   	 }
		}
		else
		{
			LOG_ERR("Not supposed to Error.. Arb process Stop..");
			if (arb_stop_msg_send(E9999_MSG) < 0) // system error 
				LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
			return ARB_STOP_CD; 
		}
	}
	else
	{
		LOG_ERR("Code Error.. gb [%d].. Arb process Stop..", gb);
		if (arb_stop_msg_send(E9999_MSG) < 0) // system error 
			LOG_ERR("Alarm Msg Send Error.. Arb Stop..");
		return ARB_STOP_CD; 
	}

	return 0;
}

// ------------------------------------------------------------
// 시세  event 
// ------------------------------------------------------------
int Arb_Feed()
{
	int rc = 0;

	rc = Arb_Process(NULL);
	if (rc == ARB_STOP_CD)
	{
		market_auto_use_init();
		LOG_ERR("Arb 전략중단 요청..");
		return ARB_STOP_CD;
	}

	return 0;
}

