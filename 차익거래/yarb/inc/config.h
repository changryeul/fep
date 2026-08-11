#ifndef CONFIG_H
#define CONFIG_H

#include <sys/types.h>
#include <sys/ipc.h>

typedef struct {

    int test_mode;
    int init_memory;

    // STRATEGY_DAY
    key_t  d_strat_shm_key;
    int    d_strat_shm_size;
    int    d_max_strats;
    int    d_sets_per_strat;

    // STRATEGY_NGT
    key_t  n_strat_shm_key;
    int    n_strat_shm_size;
    int    n_max_strats;
    int    n_sets_per_strat;

    // LOG
    int log_level;
    char log_file[128];

    // fill
    key_t  fill_shm_key;
    int fill_shm_size;
    char fill_pipe[256];

    // orderid
    long futoid;	     // 선물대역 6400001~
    long futoidlast;     // 선물대역 
    long nhspotoid;      // 현물대역 - NH
    long nhspotoidlast;  // 현물대역 - NH
    long jpmspotoid;     // 현물대역 - JPM
    long jpmspotoidlast; // 현물대역 - JPM
    long shspotoid;      // 현물대역 - Shinhan
    long shspotoidlast;  // 현물대역 - Shinhan

	// new orderid
	key_t fut_oid_shm_key;   // 선물주문번호 Shm Key
	int   fut_oid_shm_size;  // 선물주문번호 Shm Size

	key_t nhspot_oid_shm_key;   // 현물주문번호 Shm Key - NH
	key_t jpmspot_oid_shm_key;  // 현물주문번호 Shm Key - JPM
	key_t shspot_oid_shm_key;   // 현물주문번호 Shm Key - Shinhan
	int   spot_oid_shm_size;    // 현물주문번호 Shm Size  

	// order
	key_t nh_order_msg_key;
	long  nh_order_msg_mtype;

	key_t jpm_order_msg_key;
	long  jpm_order_msg_mtype;

	key_t sh_order_msg_key;
	long  sh_order_msg_mtype;

	key_t krx_order_day_msg_key;
	long  krx_order_day_msg_mtype;

	key_t krx_order_ngt_msg_key;
	long  krx_order_ngt_msg_mtype;

    // ORDERLOGQ
    char   orderlogq_fut_msg_key_text[256];
    long   orderlogq_fut_msg_mtype;
    key_t  orderlogq_spot_msg_key;
    long   orderlogq_spot_msg_mtype;
    key_t  orderlogq_jpm_msg_key;
    long   orderlogq_jpm_msg_mtype;
    key_t  orderlogq_sh_msg_key;
    long   orderlogq_sh_msg_mtype;

    char   fallback_log[256];

    // msgque id;
	int   nh_spot_order_id;
	int   jpm_spot_order_id;
	int   sh_spot_order_id;

	int   krx_day_order_id;
	int   krx_ngt_order_id;

	int   spot_ordlog_id;
	int   fut_ordlog_id;;
	int   jpm_ordlog_id;
	int   sh_ordlog_id;

	// 전략자동종료시간
	int   day_end_time;
	int   ngt_end_time;
 
} Config;

extern Config g_cfg;

int load_config(const char *path);

#endif // CONFIG_H
