#include "mds2.h"
#include "gcomdef.h"
#include "quelog.h"

#define	_SCHEMA_H_

#define	BLCK_OO			1
#define	BLCK_XX			0
#define	BLCK_XS			BLCK_XX

static __thread FILE *logF=NULL;
static __thread int  logdate=0;

extern	MARKET	*market;

static int _kba_mds_que_push(int *msgid, int idx, char *sbuf, int dlen);

//################################################################################################
// QUEUE
//################################################################################################
static int _kba_mds_que_push(int *msgid, int idx, char *sbuf, int dlen)
{
	int		ret=-1;
	struct message	msg;
	
	memset(&msg, 0x00, sizeof(msg));

	msg.mtype = idx;
	memcpy(msg.mtext, sbuf, dlen);

	while(1)
	{
		ret = msgsnd(*msgid, &msg, dlen, IPC_NOWAIT);
		if(ret < 0)
		{
			if(errno == EINTR)
				continue;

			if(errno == EAGAIN) {
				mds_log(market, MLOG_ERROR, "(%s) msgsnd error (%d:%s)", __func__, errno, strerror(errno));
				return 0;
			}

			*msgid = (-1);
			mds_log(market, MLOG_ERROR, "(%s) msgsnd error (%d:%s)", __func__, errno, strerror(errno));
		}
		break;
	}

	return (ret);
}

// ★TODO-02 : 모든 원천처리 데몬이 BEST 큐로 시세 PUSH함. 멀티 write 지원하는 DQ API 필요 > KBA_MDS_QUE_PUSH()에 반영
//int KBA_MDS_QUE_PUSH(Wfa_Hdl wfah, char *qtnm, char *sbuf, int dlen, int atr)
int KBA_MDS_QUE_PUSH(key_t ipck, int seqn, char *sbuf, int dlen)
{
#ifdef __USEDQ__
    int     rc;     
    int     xha;    

//  atr  = 0x00; 
    atr |= WAPINOXCHK;
    atr |= WAPINOHCHK;
    atr |= WAPIIAMWAA;
    xha  = 0x00; 
    xha |= AM_XHA_CRT;
    rc = WFA_queue_pushx (wfah, qtnm, pmsg, plen, atr, 0, (schr_t) AG_NULL, 0, xha, BLCK_OO);
    return (rc);
    
#else
	int     ret, idx;
	static	__thread int id_ma=(-1);
	static	__thread int id_ap=(-1);
	static	__thread int id_db=(-1);
	MDARCH	*arch;
	XCHG	*xchg;

	arch = (MDARCH *)(market->arch);
	xchg = (XCHG *)&(arch->xchg);

	// 받는 데몬이 생성 함
if (xchg->db_pnum > 0) {
	if (id_db < 0)	id_db = msgget(IPCK(ipck, DF_SISEQ4DB), 0);
	if (id_db > 0)
	{
		idx = seqn % xchg->db_pnum;
		ret = _kba_mds_que_push(&id_db, (idx+1), sbuf, dlen);
		if (ret < 0)	mds_log(market, MLOG_ERROR, "(%s) _kba_mds_que_push for DB error (%d:%s)", __func__, errno, strerror(errno));
	}
}

if (xchg->ma_pnum > 0) {
	if (id_ma < 0)	id_ma = msgget(IPCK(ipck, DF_SISEQ4MA), 0);
	if (id_ma > 0)
	{
		idx = seqn % xchg->ma_pnum;
		ret = _kba_mds_que_push(&id_ma, (idx+1), sbuf, dlen);
		if (ret < 0)	mds_log(market, MLOG_ERROR, "(%s) _kba_mds_que_push for MST error (%d:%s)", __func__, errno, strerror(errno));
//mds_log(market, MLOG_MUST, "(%s) ma push (%d) idx(%d) buf(%s)", __func__, id_ma, idx, sbuf);
	}
}

if (xchg->ap_pnum > 0) {
	if (id_ap < 0)	id_ap = msgget(IPCK(ipck, DF_SISEQ4AP), 0);
	if (id_ap > 0)
	{
		idx = seqn % xchg->ap_pnum;
		ret = _kba_mds_que_push(&id_ap, (idx+1), sbuf, dlen);
		if (ret < 0)	mds_log(market, MLOG_ERROR, "(%s) _kba_mds_que_push for AP error (%d:%s)", __func__, errno, strerror(errno));
//mds_log(market, MLOG_MUST, "(%s) ap push (%d) idx(%d) buf(%s)", __func__, id_ap, idx, sbuf);
	}
}
	return (ret);
	
#endif
}

// ★TODO-05 : 각 쓰레드에서 동일 큐를 각자 Read : 멀티 read 지원하는 DQ API 필요 > KBA_MDS_QUE_POP()에 반영
//int KBA_MDS_QUE_POP(Wfa_Hdl wfah, char *qtnm, char *rbuf, int buflen,int wmic, int aid)
int KBA_MDS_QUE_POP(key_t ipck, int idx, char *rbuf, int buflen)
{
#ifdef __USEDQ__
    int     rc;
    int     xha, aas;

    aas  = 0x00;
    aas |= WAPIDQUEIO;

    rc = WFA_queue_popx (wfah, qtnm, gmsg, gsiz, wmic, aid, 0, 0, (schr_t) AG_NULL, aas, 0);

    return (rc);
#else
	int			rtn, size;
	ssize_t		dlen;
	key_t		mkey;
	struct	message	msg;
	struct	msqid_ds msg_ds;
	static	__thread int msgid=(-1);

// ★TODO-13 : 현재 메시지큐로 되어 있는 KBA_MDS_QUE_PUSH, KBA_MDS_QUE_POP 는 하나의 원천(SMBS)만 처리 가능

	if (msgid < 0)
	{
		size = sizeof(MDSSISE)*2048;

		msgid = msgget(ipck, IPC_CREAT | 0666);
		if(msgid < 0)
		{
			return -1;
		}
		memset(&msg_ds, 0x00, sizeof(msg_ds));
		if (msgctl(msgid, IPC_STAT, &msg_ds) == 0)
		{
			msg_ds.msg_qbytes = size;
			msgctl(msgid, IPC_SET, &msg_ds);
		}
	}
	
	// idx=0은 all data 이기 때문에 idx 는 '0' 보다 커야 함.
	memset(&msg, 0x00, sizeof(msg));

//	dlen = msgrcv(msgid, &msg, buflen, idx, IPC_NOWAIT);
	dlen = msgrcv(msgid, &msg, buflen, idx, MSG_NOERROR);
	if(dlen == (ssize_t)-1)
	{
//		if(errno == ENOMSG || errno == EINTR)
		{
			usleep(500000);
//			return 0;
		}
		mds_log(market, MLOG_ERROR, "(%s) msgrcv error (%d:%s)", __func__, errno, strerror(errno));

		return -1;
	}
	
	memcpy(rbuf, msg.mtext, dlen);
	
	return ((int) dlen);
#endif
}

// 환율 관련된 오류 발생 시 직원에게 알람메시지(팝업+쪽지) 전송 (1613화면에서 관리)
int mds_send_alarm(char *mcod, char *msg)
{
	int		slen=-1, key;
	char	sbuf[1024];
	static	__thread int qid=(-1);

	if (qid < 0)
	{
		if((key = l_token(ECM_SMSSENDQUE, 'Q')) < 0) {		// /fsfxwin/wfg/inc/glb/gcomdef.h
			mds_log(market, MLOG_ERROR, "(%s) l_token error..(%d:%s) \n", __func__, errno, strerror(errno));
			return (-1);
		}
    	
		if ((qid = msgget(key, 0666)) < 0) {
			mds_log(market, MLOG_ERROR, "(%s) msgget error..(%d:%s) \n", __func__, errno, strerror(errno));
			return (-1);
		}
	}

	memset(sbuf, 0x00, sizeof(sbuf));

	// HEADER = 고객구분[1] + 알림메세지코드[5] + 거래ID[10] + 수신고객ID[10] + 발송직원번호[10] + 북번호[11]
	// 헤더 중 고객구분(C:고객,E:직원) 과 메시지코드(E0200) 만 세팅하고, 메시지 내용을 뒤로 붙임.
	// 세팅된 문구 중 '@'에 대체될 문자열과 대체열구분자'|'를 붙여서 전송함
	sprintf(sbuf, "E%-5.5s                                         %s|", mcod, msg);

	if (_kba_mds_que_push(&qid, 100L, sbuf, strlen(sbuf)) < 0)
		mds_log(market, MLOG_ERROR, "(%s) msgsnd error..(%d:%s) \n", __func__, errno, strerror(errno));

	return (0);
}

//################################################################################################
// BUSINESS BASE
//################################################################################################
int get_diff_rate(double pric, double base, double *diff, double *rate)
{
	int		irate;

	*diff = pric - base;
	if (base != 0)
	{
		irate = (*diff/base) * 10000;
		*rate = (double)irate / 100;
	}
	else 
		*rate = 0;

	return(0);
}

// 가격의 업다운 색상 뒤의가격 기준 상승 2 하락 5 보합 3 을 반환
int color(double val, double base)
{

	if (val > base) return 2;
	else if (val < base) return 5;
	else return 3;
}


//################################################################################################
// SYSTEM
//################################################################################################
uint32_t getnextday(int n)
{
    time_t  clock;  
    struct  tm tm;  
    int ymd;
    
    int d = n*24;
    
    clock = time(NULL);
    clock += (d*60*60);
    localtime_r(&clock, &tm);
    ymd = YMD((tm.tm_year+1900), (tm.tm_mon+1), tm.tm_mday);

    return (ymd);
}

key_t get_ipck(int exid)
{
	char	name[30], decimal;
	int		hexa;
	key_t	ipck;
	int		ii;

	sprintf(name, "%03d", exid);	
	for (ii = 0, hexa = 0; ii < strlen(name) && ii < 3; ii++)
	{
		hexa <<= 4;
		decimal = name[ii] & 0x0f;
		hexa |= decimal;
	}
	hexa |= 0x9000;
	ipck = (hexa << 16);				// 0x9{EXID}??##
	
	return(ipck);
}

// Print formating string for debugging
void mds_log(MARKET *m, int level, const char *format, ...)
{
	char	*cp = NULL;
	key_t	key = 0;
	key_t	shmkey = 0xff180010;
	int		mymqlv = MLOG_ERROR;
	int		templevel;
	char	logpath[128], logmsg[5*1024], lstr[40];
	va_list	vl;

	static int		shmid = -1;
	static int		*ptype = (void *) -1;

// QUELOG 로 전송 후 리턴
if (level == MLOG_QLOG && m != NULL)
{
	int rc;
	memset(logmsg, 0x00, sizeof(logmsg));

	va_start(vl, format);
	vsprintf(logmsg, format, vl);
	va_end(vl);

	rc = _sendtoLogQ(m->procname, m->exnm, strlen(logmsg), logmsg);
	va_end(vl);
	
	if (rc == 0)	return ;

	// QUELOG 실패 시 일반 로그로 전환
	level = MLOG_BIZ;
}

	if(shmid == -1 || ptype == (void *) -1)
	{
		cp = getenv("WIN_LOG_KEY");		// mymq shm
		if(cp != NULL) {
			key = strtol(cp, NULL, 16);
			if(key != 0x00)	shmkey = key;
		}

		shmid = shmget(shmkey, sizeof(int), 0660 | IPC_CREAT);
		if(shmid == -1)		return;

		ptype = (int *)shmat(shmid, 0, 0);
		if(ptype == (void *) -1)	return;

		templevel = *ptype; 
		if(templevel <= 0 || templevel > MLOG_DEBUG)  // 7
			*ptype = MLOG_ERROR;
	}

///////  MYMQ LOG LEVEL CHECK
	mymqlv = *ptype; 
	if (level > mymqlv)		return;

	time_t	clock;
	struct	tm tm;

	if (m == NULL)		return;

	if (strlen(m->procname) <= 0)
		return;

	clock = time(0);
	clock += m->e2lt;
	localtime_r(&clock, &tm);

	switch (level)
	{
	case MLOG_MUST:     strcpy(lstr, "***");	break;	// 0
	case MLOG_ERROR:    strcpy(lstr, "ERR");	break;	// 3
	case MLOG_WARNING:  strcpy(lstr, "WRN");	break;	// 4
	case MLOG_BIZ:      strcpy(lstr, "BIZ");	break;	// 6
	case MLOG_DEBUG:    strcpy(lstr, "DBG");	break;	// 7
	default:            strcpy(lstr, "   ");	break;	// 1,2,5
	}

	sprintf(logmsg, "%02d/%02d %02d:%02d:%02d %s %s ", 
		tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, lstr, m->procname);
	va_start(vl, format);
	vsprintf(&logmsg[strlen(logmsg)], format, vl);
	va_end(vl);

	struct stat fst;

	if (strlen(m->exnm) > 0)
		sprintf(logpath, "%s/%s_%s-%04d%02d%02d.log", LOG_DIR, m->procname, m->exnm, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);
	else
		sprintf(logpath, "%s/%s-%04d%02d%02d.log", LOG_DIR, m->procname, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);

	if (logF == NULL || logdate != tm.tm_mday)
	{
		if (logF)	fclose(logF);

		logF    = fopen(logpath, "a");
		logdate = tm.tm_mday;
	}

	fstat(fileno(logF), &fst);

	if (fst.st_nlink <= 0) {
		fclose(logF);
		logF = fopen(logpath, "a");
	}

	fprintf(logF, "%s\n", logmsg);
	fflush(logF);

	return ;
}

// Get process name by process id
void mds_procname(char *procname)
{
#ifdef	SunOS
	char	 procinfo[128];
	psinfo_t psinfo;
	pid_t	pid;
	char	*base;
	int	fd, rl;

	pid = getpid();
	procname[0] = '\0';
	sprintf(procinfo, "/proc/%d/psinfo", pid);

	fd = open(procinfo, O_RDONLY);
	if (fd < 0)
		return;
	rl = read(fd, &psinfo, sizeof(psinfo_t));
	close(fd);
	if (rl >= sizeof(psinfo_t))
	{
		base = basename(psinfo.pr_fname);
		strcpy(procname, base);
	}
#endif
#ifdef	LINUX
	char	 procinfo[128];
	char	cmdline[256];
	FILE	*pFile;
	char	*base;
	pid_t	pid;

	pid = getpid();
	procname[0] = '\0';
	sprintf(procinfo, "/proc/%d/cmdline", pid);
	pFile = fopen(procinfo, "r");
	if (pFile == NULL)
		return;
	cmdline[0] = '\0';
	if (fgets(cmdline, sizeof(cmdline), pFile) == cmdline)
	{
		base = basename(cmdline);
		strcpy(procname, base);
	}
	fclose(pFile);
#endif
}



