
#ifndef __l_def_h__
#define __l_def_h__

#include "arb.h"
#include "config.h"
#include "msgq.h"
#include "buf_struct.h"
#include "fep_file.h"
#include "fep_tcpip.h"

#define DBGLINESZ		16
#define CHR_DOT			'.'
#define CHR_TILDE		'~'
#define CHR_SPACE		' '
#define L_num(c)		(c - '0')
#define L_MIN(a,b)		(a>b)?(b):(a)

#define L_ERR			0x01
#define L_INF			0x02
#define L_WAR			0x04
#define L_DBG			0x10

void __l_dbg(const char *func, int line, int loglvl, const char *fmt,...);
void l_loglvl_set(int log_level);
#define l_dbg(loglvl,fmt,args...) __l_dbg(__FUNCTION__, __LINE__, loglvl, fmt, ##args)
#define L_DATA_LEN		8192

#define ORDER_IP_FUT    "            " // ¹ÌÁ¤

#define CRC(data, accum) ((accum>>8)^s_crctab[(accum^(data&0x00ff))&0x00ff])

extern Config g_cfg;
extern char   l_dbgfile[255];
extern int    DSHM_W(int, char *, int);
extern int    alrt_msg(int trcode, char *msg, int msg_len);
extern int    fut_stat, spot_stat, g_initialized;

extern FILE_BUFF_FORMAT W_Fmt, R_Fmt[1];

void   l_set_id();

int    l_arb_set_hoga();
int    l_arb_spot_fill(FillEvent *fill);
int    l_arb_fut_fill (FillEvent *fill);
int    l_arb_spot_ord (FillEvent *fill);

double update_avg_price(double old_avg, int old_qty, double new_px, int new_qty);
int    setord_log_update_fill_by_id(SetOrdLog *log, long ord_id, int filled_qty, double avg_px);

int    setord_log_find_index_by_id(const SetOrdLog *log, long ord_id);
int    setord_log_append(SetOrdLog *log, long ord_id, int leg, int side, int qty, double px, int ts_hhmmss);

int    l_stoi(char *in, int ilen);
void   l_itos(int val, char *out, int olen);
int    l_que_snd(int qid, void *qdata, int qdatalen, int qtry_cnt);
int    l_que_create(int qkey);
int    l_ftok(char *ipc_name, int ipc_type);
void   l_arb_set_data(char *msg);

char  *l_get_microTime(char *p_time);
void   l_add_wqlist_fut (int ord_typ, int ord_qty, int side, int tif_type, double px);
void   l_add_wqlist_spot(int ord_typ, int ord_qty, int side, int tif_type, double px);

int    l_arb_fut_exec(int gb, int side, long ord_id, long fill_qty, double fill_px);
int    l_arb_spot_exec(int gb, int side, long ord_id, long fill_qty, double fill_px);

time_t make_end_time(int ymd, int hms);

char  *ItoAf(int p_int, char *p_ascii, int p_len);
#endif
