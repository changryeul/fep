#include <strings.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <linux/tcp.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include "def.h"

/*----------------------------------------------------------------------*/
char  *l_get_microTime(char *p_time)
/*----------------------------------------------------------------------*/
{
    struct timeval  tv;
    struct tm       *date, date1;

    gettimeofday (&tv, NULL);
    date = (struct tm *)localtime_r(&(tv.tv_sec), &date1);

    /* HHMMSSmmmmmm (12) = 12 bytes  */
    sprintf(p_time, "%02d%02d%02d%06ld", date->tm_hour, date->tm_min, date->tm_sec, tv.tv_usec);

    return(p_time);
}
// ------------------------------------------------------------
// 주문 로그에 주문 추가
// ------------------------------------------------------------
int setord_log_append(SetOrdLog *log, long ord_id, int leg, int side, int qty, double px, int ts_hhmmss)
{
    if (log == NULL) return -1;
    if (log->n >= MAX_SET_ORD_LOG) return -2;

    SetOrdLogEnt *e = &log->ent[log->n];
    e->ord_id = ord_id;
    e->leg  = leg;
    e->side = side;
    e->qty  = qty;
    e->px   = px;
    e->ts_hhmmss = ts_hhmmss;
    e->filled_qty = 0;
    e->filled_px = 0.0;
    e->valid = 1;

    log->n++;
    return 0;
}

// ------------------------------------------------------------
// generate_set_id
//  기능 : 세트 id
// ------------------------------------------------------------
void l_set_id()
{
	G_SET_CNT++;
	g_set_idx++;
	G_SET->set_id++;

    G_SET->valid = 1;
	fut_stat  = 0;
	spot_stat = 0;
    l_dbg(L_INF, "[SETID] 세트번호 채번 g_set_idx[%d] set_id[%d] fut_stat[%d] spot_stat[%d]", g_set_idx, G_SET->set_id, fut_stat, spot_stat);
}

/* 평균 체결 가격 계산 */
double update_avg_price(double old_avg, int old_qty, double new_px, int new_qty) 
{
    int total_qty = old_qty + new_qty;
    if (total_qty == 0) return 0.0;
    return ((old_avg * old_qty) + (new_px * new_qty)) / total_qty;
}

// ------------------------------------------------------------
// 특정 주문 ID에 대한 체결 정보 갱신
// ------------------------------------------------------------
int setord_log_update_fill_by_id(SetOrdLog *log, long ord_id, int filled_qty, double avg_px)
{
	int i;
    if (log == NULL) return -1;

    for (i = 0; i < log->n; ++i)
    {
        if (log->ent[i].ord_id == ord_id)
        {
            log->ent[i].filled_qty +=filled_qty;
            log->ent[i].filled_px  = avg_px;

            /* 주문수량과 체결수량이 같은지 체크 */
            if (log->ent[i].qty == log->ent[i].filled_qty)
                log->ent[i].valid = 0;

            return 0;
        }
    }
    return -2; // not found
}

// ------------------------------------------------------------
// 주문 ID로 로그 인덱스 검색
// ------------------------------------------------------------
int setord_log_find_index_by_id(const SetOrdLog *log, long ord_id)
{
	int i;
    if (log == NULL) return -1;

    for (i = 0; i < log->n; ++i)
    {
        if (log->ent[i].ord_id == ord_id)
            return i;
    }
    return -1;
}

extern Config g_cfg;
int load_config(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    char line[512];
    char section[64] = "";

    while (fgets(line, sizeof(line), fp)) 
    {
        if (line[0] == '#' || line[0] == '\n') continue;

        // Remove leading/trailing whitespace
        char *start = line;
        while (*start == ' ' || *start == '\t') ++start;

        if (*start == '\0' || *start == '#') continue;

        // SECTION: [NAME]
        if (*start == '[') {
            sscanf(start, "[%63[^]]", section);
            continue;
        }

        // key=value parsing
        char *key = strtok(start, "=\r\n");
        char *val = strtok(NULL, "\r\n");

        if (!key || !val) continue;

        // Trim leading spaces in val
        while (*val == ' ' || *val == '\t') ++val;

        // --- MODE ---
        if (strcmp(section, "MODE") == 0) {
            if (strcmp(key, "test_mode") == 0) {
                g_cfg.test_mode = atoi(val);
            }
            else if (strcmp(key, "init_memory") == 0) {
                g_cfg.init_memory = atoi(val);
            }
            else if (strcmp(key, "day_end_time") == 0) {
                g_cfg.day_end_time = atoi(val);
            }
            else if (strcmp(key, "ngt_end_time") == 0) {
                g_cfg.ngt_end_time = atoi(val);
            }
        }
        // --- STRATEGY : DAY ---
        else if (strcmp(section, "STRATEGY_DAY") == 0) {
            if (strcmp(key, "strat_shm_key") == 0) 
			{
                g_cfg.d_strat_shm_key = (key_t)strtol(val, NULL, 0);
            } else if (strcmp(key, "strat_shm_size") == 0) 
			{
                g_cfg.d_strat_shm_size = (size_t)strtoul(val, NULL, 10);
            } else if (strcmp(key, "max_strats") == 0) 
			{
                g_cfg.d_max_strats = atoi(val);
            } else if (strcmp(key, "sets_per_strat") == 0) 
			{
                g_cfg.d_sets_per_strat = atoi(val);
            }
        }
        // --- STRATEGY : NIGHT ---
        else if (strcmp(section, "STRATEGY_NGT") == 0) {
            if (strcmp(key, "strat_shm_key") == 0) 
			{
                g_cfg.n_strat_shm_key = (key_t)strtol(val, NULL, 0);
            } else if (strcmp(key, "strat_shm_size") == 0) 
			{
                g_cfg.n_strat_shm_size = (size_t)strtoul(val, NULL, 10);
            } else if (strcmp(key, "max_strats") == 0) 
			{
                g_cfg.n_max_strats = atoi(val);
            } else if (strcmp(key, "sets_per_strat") == 0) 
			{
                g_cfg.n_sets_per_strat = atoi(val);
            }
        }
        // --- LOG ---
        else if (strcmp(section, "LOG") == 0) {
            if (strcmp(key, "log_level") == 0) {
                g_cfg.log_level = atoi(val);
            } else if (strcmp(key, "log_file") == 0) {
                strncpy(g_cfg.log_file, val, sizeof(g_cfg.log_file) - 1);
            }
        }
        // --- order id ---
        else if (strcmp(section, "ORDER_ID") == 0) {
            if (strcmp(key, "futoid") == 0) {
                g_cfg.futoid = atol(val);
            } else if (strcmp(key, "futmaxoid") == 0) {
                g_cfg.futoidlast = atol(val);
            } else if (strcmp(key, "nhspotoid") == 0) {
                g_cfg.nhspotoid = atol(val);
            } else if (strcmp(key, "nhspotmaxoid") == 0) {
                g_cfg.nhspotoidlast = atol(val);
            } else if (strcmp(key, "jpmspotoid") == 0) {
                g_cfg.jpmspotoid = atol(val);
            } else if (strcmp(key, "jpmspotmaxoid") == 0) {
                g_cfg.jpmspotoidlast = atol(val);
            } else if (strcmp(key, "shspotoid") == 0) {
                g_cfg.shspotoid = atol(val);
            } else if (strcmp(key, "shspotmaxoid") == 0) {
                g_cfg.shspotoidlast = atol(val);
			}
        }

		// -- new : fut order id ---
		else if (strcmp(section, "FUT_OID_SHM") == 0)
		{
			if (strcmp(key, "fut_oid_shm_key") == 0)
            {
                g_cfg.fut_oid_shm_key = (key_t)strtol(val, NULL, 0);
            } else if (strcmp(key, "fut_oid_shm_size") == 0)
            {
                g_cfg.fut_oid_shm_size = (size_t)strtoul(val, NULL, 10);
            }	
		}
		// -- new : spot order id 
		else if (strcmp(section, "SPOT_OID_SHM") == 0)
		{
			if (strcmp(key, "nhspot_oid_shm_key") == 0) {
                g_cfg.nhspot_oid_shm_key = (key_t)strtol(val, NULL, 0);
            } else if (strcmp(key, "jpmspot_oid_shm_key") == 0) {
                g_cfg.jpmspot_oid_shm_key = (key_t)strtol(val, NULL, 0);
            } else if (strcmp(key, "shspot_oid_shm_key") == 0)  {
                g_cfg.shspot_oid_shm_key = (key_t)strtol(val, NULL, 0);
            } else if (strcmp(key, "spot_oid_shm_size") == 0)  {
                g_cfg.spot_oid_shm_size = (size_t)strtoul(val, NULL, 10);
            }	
		}

        // --- NH ORDER ---
        else if (strcmp(section, "NH_ORDER") == 0) 
		{
            if (strcmp(key, "order_msg_key") == 0) {
                g_cfg.nh_order_msg_key = (key_t)strtol(val, NULL, 0);
            } else if (strcmp(key, "order_msg_mtype") == 0) {
                g_cfg.nh_order_msg_mtype = atol(val);
            }
        }
        // --- JPM ORDER ---
        else if (strcmp(section, "JPM_ORDER") == 0) 
		{
            if (strcmp(key, "order_msg_key") == 0) {
                g_cfg.jpm_order_msg_key = (key_t)strtol(val, NULL, 0);
            } else if (strcmp(key, "order_msg_mtype") == 0) {
                g_cfg.jpm_order_msg_mtype = atol(val);
            }
        }
        // --- SH ORDER ---
        else if (strcmp(section, "SH_ORDER") == 0) 
		{
            if (strcmp(key, "order_msg_key") == 0) {
                g_cfg.sh_order_msg_key = (key_t)strtol(val, NULL, 0);
            } else if (strcmp(key, "order_msg_mtype") == 0) {
                g_cfg.sh_order_msg_mtype = atol(val);
            }
        }

        // --- KRX_ORDER_DAY ---
        else if (strcmp(section, "KRX_ORDER_DAY") == 0) 
		{
            if (strcmp(key, "order_msg_key") == 0) {
                g_cfg.krx_order_day_msg_key = (key_t)strtol(val, NULL, 0);
            } else if (strcmp(key, "order_msg_mtype") == 0) {
                g_cfg.krx_order_day_msg_mtype = atol(val);
            }
        }

        // --- KRX_ORDER_NGT ---
        else if (strcmp(section, "KRX_ORDER_NGT") == 0) 
		{
            if (strcmp(key, "order_msg_key") == 0) {
                g_cfg.krx_order_ngt_msg_key = (key_t)strtol(val, NULL, 0);
            } else if (strcmp(key, "order_msg_mtype") == 0) {
                g_cfg.krx_order_ngt_msg_mtype = atol(val);
            }
        }

        // --- ORDERLOGQ ---
        else if (strcmp(section, "ORDERLOGQ") == 0) {
            if (strcmp(key, "orderlogq_fut_msg_key_text") == 0) {
                strncpy(g_cfg.orderlogq_fut_msg_key_text ,val, sizeof(g_cfg.orderlogq_fut_msg_key_text) -1);
            } else if (strcmp(key, "orderlogq_fut_msg_mtype") == 0) {
                g_cfg.orderlogq_fut_msg_mtype = atol(val);
            } else if (strcmp(key, "orderlogq_spot_msg_key") == 0) {
                g_cfg.orderlogq_spot_msg_key = (key_t)strtol(val, NULL, 0);
            } else if (strcmp(key, "orderlogq_spot_msg_mtype") == 0) {
                g_cfg.orderlogq_spot_msg_mtype = atol(val);
            } else if (strcmp(key, "orderlogq_jpm_msg_key") == 0) {
                g_cfg.orderlogq_jpm_msg_key = (key_t)strtol(val, NULL, 0);
            } else if (strcmp(key, "orderlogq_jpm_msg_mtype") == 0) {
                g_cfg.orderlogq_jpm_msg_mtype = atol(val);
		    } else if (strcmp(key, "orderlogq_sh_msg_key") == 0) {
                g_cfg.orderlogq_sh_msg_key = (key_t)strtol(val, NULL, 0);
            } else if (strcmp(key, "orderlogq_sh_msg_mtype") == 0) {
                g_cfg.orderlogq_sh_msg_mtype = atol(val);
		    }
        }

    }

    fclose(fp);
    return 0;
}

// ------------------------------------------------------------
// orderlogq_msg_build
// ------------------------------------------------------------
int orderlogq_msg_build(OrderLogMsg *msg, int packet_type, SMB_STRGMSG *spot_out, YSK_STRGMSG *fut_out)
{
	char body_len[3+1] = {0, };
	char strg_grp_no[10+1] = {0, };
	(void)msg;

	if (packet_type == 1) // 1-현물 
	{
		memcpy(spot_out->lgen_no        , G_STRAT->lgen_no, 10);   // 거래참여자번호
		memcpy(spot_out->bk_no          , G_STRAT->bk_no  , 11);   // 북번호
		memcpy(spot_out->rulez_no       , G_STRAT->ref_no , 15);   // 참조번호
		snprintf(strg_grp_no, sizeof(strg_grp_no), "%010d", g_set_idx+1);
		memcpy(spot_out->strg_grp_no    , strg_grp_no     , 10);   // 전략그룹번호
		memcpy(spot_out->trdr_no        , G_STRAT->trdr_no,  4);   // 트레이더번호
		memcpy(spot_out->auto_tran_dstcd, G_STRAT->ref_no ,  1);   // 자동거래구분코드 

	}
	else // 2- 선물 
 	{
		snprintf(body_len, sizeof(body_len), "%03d", (int)YSKMSG_ORD_SZ); 
		memcpy(fut_out->body_len      , body_len                ,  3);   // Body Length
		memcpy(fut_out->channel       , "E"                     ,  1);   // E:Ecm
		memcpy(fut_out->product_dstcd , G_STRAT->ref_no         ,  1);   // A.USD B.JPY C.CNH D:NDF 
		memcpy(fut_out->fcm_id        , G_STRAT->fcm_id_fut     , 10);   // 증권/선물 회원사 번호
		memcpy(fut_out->rulez_no      , G_STRAT->strategy_name  , 15);   // Rule 번호 (with 전략번호)
		memcpy(fut_out->ref_no        , G_STRAT->ref_no         , 15);   // 참조번호: 자동거래구분코드(1자리) + 실행일자(8자리) + 일련번호(6자리)
		snprintf(strg_grp_no, sizeof(strg_grp_no), "%010d" , g_set_idx+1);
		memcpy(fut_out->strg_grp_no   , strg_grp_no, 10);                // 전략그룹번호. OMS에서는 기본 셋팅. 화면에서는 0 셋팅(신규/정정/취소 모두).
		memcpy(fut_out->bk_no         , G_STRAT->bk_no          , 11);   // 북번호
		memcpy(fut_out->trdr_no       , G_STRAT->trdr_no        , 4 );    // 트레이더번호
		memcpy(fut_out->day_ngt_dstcd , G_STRAT->day_ngt_cd     , 1 );    // 주야간구분코드: 1-주간 2-야간 
	}

	return 0;
}

int arb_stop_msg_send(char *msg)
{
	int rc = 0, seq_no = 0;
	char tmp[256+1];

	ALRTMSG alrt;
	memset(&alrt, 0x20, sizeof(ALRTMSG));	
	
	memset(tmp, 0x00, sizeof(tmp));
	sprintf(tmp, "%d", G_STRAT->today); 
	memcpy(alrt.exe_ymd, tmp, sizeof(alrt.exe_ymd));
	
	memset(tmp, 0x00, sizeof(tmp));
	
	if (G_STRAT->strategy_name[0] == 'A')
		seq_no = G_STRAT->seq_no;
	else if (G_STRAT->strategy_name[0] == 'B')
		seq_no = G_STRAT->seq_no - 50; 
	else if (G_STRAT->strategy_name[0] == 'C')
		seq_no = G_STRAT->seq_no - 100;
	else 
		seq_no = G_STRAT->seq_no - 150;
		 	
	sprintf(tmp, "%05d", seq_no); 
	memcpy(alrt.exe_no, tmp, sizeof(alrt.exe_no));

	memset(tmp, 0x00, sizeof(tmp));
	sprintf(tmp, "%s", G_STRAT->bk_no); 
	memcpy(alrt.bk_no, tmp, sizeof(alrt.bk_no));

	memset(tmp, 0x00, sizeof(tmp));
	sprintf(tmp, "%s", G_STRAT->trdr_no); 
	memcpy(alrt.trdr_no, tmp, sizeof(alrt.trdr_no));

	memset(tmp, 0x00, sizeof(tmp));
	sprintf(tmp, "%s", G_STRAT->strategy_name); 
	memcpy(alrt.rule_id, tmp, sizeof(alrt.rule_id));

	memcpy(alrt.stop_msg, msg, strlen(msg));
	rc = alrt_msg(551001, (char *)&alrt, sizeof(ALRTMSG));
	if (rc < 0)
    {
        return -1;
    }
	return 0;
}

unsigned short s_crctab[256] = {
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};

int l_ftok(ipc_name, ipc_type)
char    *ipc_name;
int ipc_type;
{
    unsigned char w_cc;
    int  w_crc,  w_token, nmlen;
    register int ii;

    switch (ipc_type)
    {
    case 'Q': break;        /* message queue */
    case 'M': break;        /* shared memory */
    case 'S': break;        /* semaphore     */
    default:  errno = EFAULT;   /* bad address   */
          return (-1);
    }

    w_crc = 0;
    nmlen = strlen(ipc_name);
    for (ii=0; ii < nmlen; ii++)
    {
        w_cc  = ipc_name[ii];
        w_crc = CRC(w_cc, w_crc);
    }
    w_token = (ipc_type << 16) | w_crc;
    //LOG_DEBUG("w_token [%d]", w_token);
    return (w_token);
}
