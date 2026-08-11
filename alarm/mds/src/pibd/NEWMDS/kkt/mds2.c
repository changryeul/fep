#include "mds2.h"
#include "agdef.h"

static	__thread struct	xmltag xmltag[256];

static	struct	cfgtag {
	int		mark;
	char	tags[16];
} cfgtag[] = {
	{  1, "xchg"	},	// <exchange name='name' exid='99' desc='description' />
	{  2, "recv"	},	// <recv from='udp|rabbitq' multicast='yes|no' rabbitq='queue-name' threads='nn' parallel='yes'/>
	{  3, "port"	},	// <port id='#n,ipad.port' id='#n,ipad.port' ... />
	{  4, "data"	},	// <data path="/home/data/NYS" room="2000" tablespace='xxx' mstr='yes' .... />
	{  5, "time"	},	// <time TZ="EST5EDT" trading="hhmm,hhmm" wday='Sun-Fri' 24hours='yes|no' />
	{  6, "batch"   },	// <batch open='hhmm' close='hhmm' batch='hhmm,hhmm,...' eod='hhmm,hhmm, ...' />
	{  7, "delay"	},	// <delay time='999' recn=999' />
	{  8, "keep"	}, 	// <keep tick="10" intr="20" />
	{  9, "real"	},	// <real push="yes" id="?" />
	{ 10, "watch"	},	// <watch time="hhmm,hhmm,seconds" time="hhmm,hhmm,seconds" .... />
	{ 11, "alert"	},	// <alert interval="seconds" phone="010-1234-1234" phone="011-1234-1234" .../>
	{ 12, "log"		},	// <log level="error|warning|progrerss|debug" />
	{ 13, "notify"	},	// <notify cast='yes|no' ipad='1.1.1.1' port='n' />
	{  0, ""		}
};

static int cmpindex(INDEX *i1, INDEX *i2);

//################################################################################################
// RESOURCE
//################################################################################################

/*===============================================================
 * MDS 환경 관련 함수
===============================================================*/
// Set Environment
void mds_setenv()
{
	char	profile[128];
	FILE	*chckF;
	char	lineX[512], lineB[512];
	char	chckB[4][256];
	char	*wordP[4], *equaL;
	char	*cenvP, *tagsP, *defsP;
	int	wordN;
	int	ii, jj;

	sprintf(profile, "%s/profile", ETC_DIR);
	chckF = fopen(profile, "r");
	if (chckF == NULL)
		return;

	for (ii = 0; ii < 4; ii++)
		wordP[ii] = chckB[ii];
	while ((fgets(lineB, sizeof(lineB)-1, chckF)) == lineB)
	{
		for (ii = 0, jj = 0; ii < strlen(lineB); ii++)
		{
			if (lineB[ii] == ' ' || lineB[ii] == '\t')
				continue;
			if (lineB[ii] == '*' || lineB[ii] == '#')
				break;
			lineX[jj++] = lineB[ii];
		}
		lineX[jj] = '\0';
		wordN = str2words(lineX, wordP, 4);
		if (wordN <= 0)
			continue;
		equaL = strstr(wordP[0], "=");
		if (equaL == NULL)
			continue;

		cenvP = malloc(strlen(wordP[0]) + 512);
		if (cenvP == NULL)
			continue;

		tagsP = wordP[0];
		defsP = equaL + 1;
		*equaL = '\0';

		sprintf(cenvP, "%s=%s", tagsP, defsP);
		putenv(cenvP);
	}
	fclose(chckF);
}

// Get current time
void mds_time(MARKET *m, time_t clock, uint32_t *xymd, uint32_t *xhms, uint32_t *kymd, uint32_t *khms)
{
	time_t	tclock;
	struct	tm tm;

	if (clock == 0)
		tclock = time(0);
	else
		tclock = clock;
	if (xymd != NULL || xhms != NULL)
	{
		localtime_r(&tclock, &tm);
		if (xymd != NULL)
			*xymd = YMD((tm.tm_year + 1900), (tm.tm_mon+1), tm.tm_mday);
		if (xhms != NULL)
			*xhms = HMS(tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
//	printf("tclock:%ld \n", tclock);
	localtime_r(&tclock, &tm);
	//printf("[%4d] %04d.%02d.%02d %02d:%02d:%02d \n", __LINE__, tm.tm_year + 1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	if (kymd != NULL || khms != NULL)
	{
		tclock += m->e2lt;
		//printf("[%4d] %04d.%02d.%02d %02d:%02d:%02d \n", __LINE__, tm.tm_year + 1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		localtime_r(&tclock, &tm);
		if (kymd != NULL)
			*kymd = YMD((tm.tm_year + 1900), (tm.tm_mon+1), tm.tm_mday);
		if (khms != NULL)
			*khms = HMS(tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
}

// Initialize time zone
void mds_timezone(MARKET *m)
{
	struct	tm tm, tx;
	time_t	clock1, clock2;
	int	isdst;

	// 20170810
	putenv(TZ_KST);			
	tzset();
	///////////
	clock1 = time(0);
	localtime_r(&clock1, &tm);
	putenv(m->TZ);			// couuntry of exchange
	tzset();
	localtime_r(&clock1, &tx);
	isdst = tx.tm_isdst;
	memcpy(&tx, &tm, sizeof(struct tm));
	tx.tm_isdst = isdst;
	m->isdst = isdst;
	clock2 = mktime(&tx);
	m->e2lt = clock2 - clock1;
	
	gmtime_r(&clock1, &tm);
	memcpy(&tx, &tm, sizeof(struct tm));
	tx.tm_isdst = isdst;
	clock2 = mktime(&tx);
	m->g2et = clock2 - clock1;
}

/*===============================================================
 * Master Shared Memory 관련 함수 (GET, LOCK, SORT, POP)
===============================================================*/
void mds_lock(MARKET *m)
{
	pthread_mutex_lock(&m->ctx.lock);
}

void mds_unlock(MARKET *m)
{
	pthread_mutex_unlock(&m->ctx.lock);
}

static int cmpindex(INDEX *i1, INDEX *i2)
{
	return(strcmp(i1->symb, i2->symb));
}

INDEX *mds_shmget(MARKET *m, const char *symbol)
{
	MDARCH	*arch = m->arch;
	INDEX	*indx = m->indx;
	INDEX	w, *W;

	memset(&w, 0, sizeof(INDEX));
	strcpy(w.symb, symbol);
	W = bsearch(&w, indx, arch->nrec, sizeof(INDEX), cmpindex);
	
	return(W);
}

int mds_shmidxsort(MARKET *m)
{
	MDARCH	*arch = m->arch;
	INDEX	*indx = m->indx;

	switch (m->whoami)
	{
		case I_AM_COOKER:	break;
		default: 	  		errno = EACCES; return(-1);
	}
	
	mds_lock(m);

	qsort(indx, arch->nrec, sizeof(INDEX), cmpindex);

	mds_unlock(m);
	
	return(0);
}

/*
void *mds_getfolder(MARKET *m, const char *symb)
{
	MDARCH	*arch	= m->arch;
	INDEX	w, *W	= NULL;
	INDEX	*indx	= m->indx;
	char	*fb		= m->fold;

	if (m == NULL || indx == NULL || fb == NULL)
		return(NULL);

	memset(&w, 0, sizeof(INDEX));
	strcpy(w.symb, symb);
	W = bsearch(&w, indx, arch->nrec, sizeof(INDEX), cmpindex);
	
	if (W == NULL)	return (NULL);
		
	if (W->indx < 0 || W->indx >= arch->nrec)
		return(NULL);

	fb += (sizeof(MDFOLD) * W->indx);
	
	return (fb);
}
*/

void *mds_getfolder(MDARCH *arch, const char *symb)
{
	INDEX	w, *W	= NULL;
	INDEX	*indx	= (INDEX  *)((char *)arch + sizeof(MDARCH));
	char	*fb		= (MDFOLD *)((char *)arch + sizeof(MDARCH) + (sizeof(INDEX)*arch->xchg.maxcnt));

	if (arch == NULL || indx == NULL || fb == NULL)
		return(NULL);

	memset(&w, 0, sizeof(INDEX));
	strcpy(w.symb, symb);
	W = bsearch(&w, indx, arch->nrec, sizeof(INDEX), cmpindex);
	
	if (W == NULL)	return (NULL);
		
	if (W->indx < 0 || W->indx >= arch->nrec)
		return(NULL);

	fb += (sizeof(MDFOLD) * W->indx);
	
	return (fb);
}

/*
void *mds_popfolder(MARKET *m)
{
	MDARCH	*arch	= m->arch;
	INDEX	*indx	= m->indx;
	INDEX	*curr	= m->fptr;
	INDEX	*top, *bot;
	char	*fb		= m->fold;

	if (m == NULL || indx == NULL || fb == NULL || curr == NULL)
		return(NULL);
		
	top = &indx[0];
	bot = &indx[arch->nrec];

	if (curr <  top)	return(NULL);

	if (curr >= bot)
	{
		m->fptr = &indx[0];			// POP이 한바퀴 다 돌았을 경우 최초 포인터로 원위치
		return (NULL);		
	}

	fb += (sizeof(MDFOLD) * curr->indx);

	curr++;
	m->fptr = curr;

	return(fb);
}
*/

//################################################################################################
// MDS INITIALIZE
//################################################################################################

int exchange_get2(const char *exnm, XCHG *xchg)
{
	int		ii, jj, nconf;
	char	xmlpath[128];
	char	args[80], argv[8][32];

	memset(args, 0x00, sizeof(args));
	memset(argv, 0x00, sizeof(argv));
	
	sprintf(xmlpath, "%s/%s.new.cfg", ETC_DIR, exnm);
//printf("%s\n", xmlpath);	
	if ((nconf = getxmlcfg(xmlpath, xmltag)) <= 0)
	{
		errno = ESRCH;
		return(-1);
	}

	xchg->llog = MLOG_ERROR;
	
	for (ii = 0; ii < nconf; ii++)
	{
		for (jj = 0; cfgtag[jj].mark != 0; jj++)
		{
			if (strcasecmp(xmltag[ii].tags, cfgtag[jj].tags) == 0)
				break;
		}
		if (!cfgtag[jj].mark)
			continue;
		switch (cfgtag[jj].mark)
		{
		case 1: // <xchg name='name' code='Z' exid='999' cust='Y' desc=description' />
			getargs(&xmltag[ii], "name", xchg->exnm);
			getargs(&xmltag[ii], "code", argv[0]);
			memcpy(xchg->excode, argv[0], sizeof(xchg->excode));
			getargv(&xmltag[ii], "exid", &xchg->exid);
//			xchg->exid = atoi(argv[1]);
			if (strlen(xchg->exnm) <= 0 || xchg->exid < 0 || xchg->exid > 99)
			{
				errno = EINVAL;
				return(-1);
			}
			getargs(&xmltag[ii], "cust", argv[2]);
			xchg->custflag = '0';
			if (strlen(argv[2]) > 0)
			{
				if (argv[2][0] == 'Y' || argv[2][0] == 'y')
					xchg->custflag = '1';
			}
			break;
			
		case 2: // <recv name='alpari' ipad='1.1.1.1' port='n' thread_ma='n' thread_ap='n' thread_db='n'/>
			getargs(&xmltag[ii], "name", xchg->from.name);
			getargs(&xmltag[ii], "ipad", xchg->from.ipad);
			getargv(&xmltag[ii], "port", &xchg->from.port);
			getargv(&xmltag[ii], "thread_ma", &xchg->ma_pnum);
			getargv(&xmltag[ii], "thread_ap", &xchg->ap_pnum);
			getargv(&xmltag[ii], "thread_db", &xchg->db_pnum);
			break;
			
		case 4: // <data path="/home/..." room="n" dbsvr="yes|no"/>
			getargs(&xmltag[ii], "path",   xchg->dirp);
			getargv(&xmltag[ii], "maxcnt", &xchg->maxcnt);
			break;
			
		case 5: // <time TZ="EST5EDT"  trading='hhmm-hhmm'  wday='Sun-Fri' 24hours='yes' />
			getargs(&xmltag[ii], "TZ", xchg->TZ);
			break;
			
		case 12: // <log level="error|warning|progress|debug" path='pathname'/>
			getargs(&xmltag[ii], "path", xchg->logf);
			getargs(&xmltag[ii], "level", args);
			if (strcasecmp(args, "error") == 0)
				xchg->llog = MLOG_ERROR;
			else if (strcasecmp(args, "warning") == 0)
				xchg->llog = MLOG_WARNING;
			else if (strcasecmp(args, "biz") == 0)
				xchg->llog = MLOG_BIZ;
			else if (strcasecmp(args, "debug") == 0)
				xchg->llog = MLOG_DEBUG;
			else
				xchg->llog = MLOG_ERROR;
			break;
		case 13: // <notify cast='yes|no' ipad='1.1.1.1' port='udp-port-number' qname='sisedq_smbs' />
			getargv(&xmltag[ii], "port", &xchg->apsnd.port);
			if (xchg->apsnd.port > 0)
			{
				getargs(&xmltag[ii], "cast", argv[1]);
				if (strcasecmp(argv[1], "yes") == 0)
					xchg->apsnd.cast = 1;
				getargs(&xmltag[ii], "ipad", xchg->apsnd.ipad);
				getargs(&xmltag[ii], "neta", xchg->apsnd.neta);
			}
			getargs(&xmltag[ii], "qname", xchg->quenm);
			break;
			
		}
	}
	if (xchg->maxcnt <= 0)	xchg->maxcnt = 500;
		
	if (strlen(xchg->TZ) > 0)
	{
		sprintf(args, "TZ=%s", xchg->TZ);
		strcpy(xchg->TZ, args);
	}

	return(0);
}

//#####################################
//##### mds/src/lib/mds/setup.c
MARKET *mds_market_alloc2()
{
	MARKET 	*m;

	if ((m = (MARKET *)malloc(sizeof(MARKET))) == NULL)
		return(NULL);
		
	memset(m, 0x00, sizeof(MARKET));
	
	pthread_mutex_init(&m->ctx.lock, NULL);
	pthread_mutex_init(&m->ctx.mutex, NULL);
	pthread_mutex_init(&m->ctx.islock, NULL);

	return(m);
}

//#####################################
//##### mds/src/lib/mds/shm.c
int mds_shminit2(MARKET *m, XCHG *xchg)
{
	key_t	ipck;
	int		shmid, shmsz;
	int		new = 0;
	char	*shmad=NULL;
	struct	shmid_ds shmid_ds;
	MDARCH	*arch;

	ipck = get_ipck(m->exid);

	switch (m->whoami)
	{
	case I_AM_READER:
		if ((shmid = shmget(IPCK(ipck, 0), 0, 0666)) < 0)
			return(-1);
		shmad = shmat(shmid, (char *)0, SHM_RDONLY); 
		break;
		
	case I_AM_WRITER:
		if ((shmid = shmget(IPCK(ipck, 0) , 0, 0666)) < 0)
		{
 			fprintf(stderr, "%s\n", strerror(errno));
			return(-2);
		}
		shmad = shmat(shmid, (char *)0, 0); 
		break;
		
	case I_AM_COOKER:
		shmsz = sizeof(MDARCH) + (sizeof(INDEX)*xchg->maxcnt) + (sizeof(MDFOLD)*xchg->maxcnt);

		if ((shmid = shmget(IPCK(ipck, 0), 0, 0)) >= 0)
		{
			// 메모리 사이즈가 달라진 경우에만 재생성
			shmctl(shmid, IPC_STAT, &shmid_ds);
			if (shmid_ds.shm_segsz != shmsz)
			{
				shmctl(shmid, IPC_RMID, &shmid_ds);
				mds_log(m, MLOG_MUST, "Shared memory need to be recreated..[%d]", shmid);
				new = 1;
			}
			mds_log(m, MLOG_MUST, "Shared memory is already exist..[%d]", shmid);
		}
		else
			mds_log(m, MLOG_MUST, "Shared memory is not exist .. ");

		shmid = shmget(IPCK(ipck, 0), shmsz, 0666|IPC_CREAT);
		if (shmid < 0)
		{
			mds_log(m, MLOG_ERROR, "Shared memory get error.. (%d:%s)", errno, strerror(errno));
			return (-3);
		}
/*
		if (shmctl(shmid, IPC_STAT, &shmid_ds) == 0 && shmid_ds.shm_segsz < shmsz)
		{
			shmid_ds.shm_segsz = shmsz;

			if (shmctl(shmid, IPC_SET, &shmid_ds) != 0)
				return(-1);

			if (shmctl(shmid, IPC_STAT, &shmid_ds) == 0)
				shmctl(shmid, SHM_LOCK, &shmid_ds);
			new = 2;
		}
*/
		shmad = shmat(shmid, (char *)0, 0);
		break;
		
	default:
		return (-4);
	}

	if (shmad == NULL)
	{
		mds_log(m, MLOG_MUST, "Shared memory attach error.. ");
		return (-5);
	}

	m->ipck = ipck;
	arch = (MDARCH *)shmad;
	m->arch = arch;
	m->indx = &shmad[sizeof(MDARCH)];
	m->fptr = &shmad[sizeof(MDARCH)];
	m->fold = &shmad[sizeof(MDARCH) + (sizeof(INDEX)*xchg->maxcnt)];

	if (m->whoami != I_AM_COOKER)		return(0);
		
	switch (new)
	{
	case 1: // all new one
		memset(shmad, 0, shmsz);
		arch->mrec = xchg->maxcnt;
		break;
		
	case 2: // enlarge size of shared memory
		arch->mrec = xchg->maxcnt;
		arch->nrec = 0;
		arch->drec = 0;
		arch->vrec = 0;
		arch->rtim = 0;
		arch->rsum = 0;
		mds_log(m, MLOG_MUST, "Shared memory enlarge size of shared memory ");
		break;
		
	default:
		if (arch->mrec <= 0)
		{
			arch->mrec = xchg->maxcnt;
			arch->nrec = 0;
			arch->drec = 0;
			arch->vrec = 0;
			arch->rtim = 0;
			arch->rsum = 0;
		}
		break;
	}
	
//	if (m->whoami == I_AM_COOKER)
		mds_log(m, MLOG_MUST, "[%s] Shared memory attach OK. (%d/%d) records [%d:0x%x]", __func__, arch->nrec, arch->mrec, shmid, ipck);
		
	return(0);
}

//#####################################
//##### mds/src/lib/mds/open.c
MARKET *mds_open2(const char *exnm, int flag)
{
	int			rtn;
	uint32_t	ymd, hms;
	MARKET		*m;
	MDARCH		*arch;
	XCHG		xchg;
	
	/*===============================================================
	 * 설정파일 READ (cfg -> XCHG)
	===============================================================*/
	mds_setenv();
	memset(&xchg, 0x00, sizeof(xchg));
	
	if (exchange_get2(exnm, &xchg) != 0)
	{
		errno = ENOENT;
		printf("\n\tNo exchange for '%s'\n", exnm);
		return(NULL);
	}
	/*===============================================================
	 * MARKET Local Memory 구성
	===============================================================*/	
	if ((m = (MARKET *)malloc(sizeof(MARKET))) == NULL)
	{
		printf("\n\tMarket alloc error\n");
		return(NULL);
	}
		
	memset(m, 0x00, sizeof(MARKET));
	
	pthread_mutex_init(&m->ctx.lock, NULL);
	pthread_mutex_init(&m->ctx.mutex, NULL);
	pthread_mutex_init(&m->ctx.islock, NULL);
		
	m->flag	= flag;
	m->exid	= xchg.exid;
	strcpy(m->exnm, xchg.exnm);
	memcpy(m->excode, xchg.excode, sizeof(m->excode));
	
	mds_procname(m->procname);			// 실행프로그램명 

	strcpy(m->TZ, xchg.TZ);
	mds_timezone(m);
	mds_time(m, 0, &ymd, &hms, NULL, NULL);	// get local date & time
	
	// SET CTX
	strcpy(m->ctx.logf, xchg.logf);
	m->ctx.llog = xchg.llog;
	//m->ctx.chck;
	//m->proc;
	//m->func;

	if (flag & O_CREAT)
		m->whoami = I_AM_COOKER;
	else
	{
		switch (flag & O_ACCMODE)
		{
		case O_RDONLY: m->whoami = I_AM_READER; break;
		case O_RDWR:   m->whoami = I_AM_WRITER; break;
		case O_WRONLY: m->whoami = I_AM_WRITER; break;
		}
	}

mds_log(m, MLOG_MUST, "[XCHG] ======================================================");	
mds_log(m, MLOG_MUST, "%s: %s/%d/%c/%c", __func__, xchg.exnm, xchg.exid, xchg.excode[0], xchg.custflag);
mds_log(m, MLOG_MUST, "%s: %s/%s/%d/%d/%d/%d", __func__, xchg.from.name, xchg.from.ipad, xchg.from.port, 
					xchg.ma_pnum, xchg.ap_pnum, xchg.db_pnum);
mds_log(m, MLOG_MUST, "%s: %s/%d", __func__, xchg.dirp, xchg.maxcnt);
mds_log(m, MLOG_MUST, "%s: %s", __func__, xchg.TZ);
mds_log(m, MLOG_MUST, "%s: %d/%s/%s/%s", __func__, xchg.apsnd.port, xchg.apsnd.ipad, xchg.apsnd.neta, xchg.quenm);
mds_log(m, MLOG_MUST, "============================================================");

mds_log(m, MLOG_MUST, "[%s] MARKET [%d/%s/%s/%.1s]\n", __func__, m->exid, m->procname, m->exnm, m->excode);

	/*===============================================================
	 * SHARED MEMORY 초기화 (MARKET -> SHM)
	  > 기존 folder_init() 내용 포함
	===============================================================*/
	if (mds_shminit2(m, &xchg) != 0)
	{
		mds_log(m, MLOG_ERROR, "[%s] mds_shminit2 ERROR !! [%s:%.1s]", __func__, xchg.exnm, xchg.excode);
		return(-1);
	}
		
	if (m->whoami != I_AM_COOKER)	return(m);

	/*===============================================================
	 * MDARCH, XCHG 메모리 초기화
	===============================================================*/
	arch = m->arch;						// archive for shared memory
	memcpy(&(arch->xchg), &xchg, sizeof(XCHG));	// exchange information from 'cfg' to shared memory	
//	arch->tymd = ymd;

	/*===============================================================
	 * 통화별 마스터메모리 초기화 (INDEX포함)
	===============================================================*/
	rtn = LoadMaster(m);
	if (rtn <= 0)
	{
		mds_log(m, MLOG_ERROR, "[%s] Master SHM Initialize ERROR !! [%s:%c] rtn=[%d]", __func__, arch->xchg.exnm, arch->xchg.excode[0], rtn);
		l_db2rollback();
		return(NULL);
	}
	
	mds_log(m, MLOG_MUST, "[%s] Master SHM Initialize for [%s:%c] is DONE.. Load Count=[%d:%d] [%08d]", __func__, 
				arch->xchg.exnm, arch->xchg.excode[0], arch->nrec, rtn, arch->tymd);

	return(m);
}


//################################################################################################
// PACKET
//################################################################################################

/*===============================================================
 * AP전송용 패킷 조립
===============================================================*/
// 아래 함수 통합
// int SendAndSavePriceData(MARKET *market, int iSource, char *msgb, int msgl) 함수내 로직 대체
int make_ap_sise(MARKET *m, MDFOLD *pfold, MDFOLD *pusdkrw, MDSSISE *psise, APSISE *preal)
{
	XCHG		*xchg;
	MDARCH		*arch;
	
	arch = (MDARCH *)(m->arch);
	xchg = (XCHG *)&(arch->xchg);
	
	preal->excode[0] 		= xchg->excode[0];
	preal->bidex[0]			= pfold->bidex[0];		//psise->source[0];
	preal->offerex[0]		= pfold->offerex[0];	//psise->source[0];
	
	memcpy(preal->symb, pfold->symb, sizeof(preal->symb));
	memcpy(preal->date, psise->date, sizeof(preal->date));
	memcpy(preal->time, psise->time, sizeof(preal->time));

/* 24.10.08) 재정통화 실시간은 안하기로 했으며, BEST 시에는 psise를 사용하면 안되기 때문에 Block 처리

	// pfold->symb == psise->symb 인 경우는 수신 통화이므로 수신패킷(psise) 사용
	// 다르면 재정통화용이므로 마스트메모리(pfold) 사용
	if (memcmp(pfold->symb, psise->symb, sizeof(pfold->symb)) == 0)
	{
		preal->bidprc		= psise->bidprc;
		preal->offerprc		= psise->offerprc;
		preal->bidqty		= psise->bidqty;
		preal->offerqty		= psise->offerqty;
		preal->bidbest		= psise->bidbestprc;
		preal->offerbest	= psise->offerbestprc;
	} else
	{
*/
		preal->bidprc		= pfold->bidlast;
		preal->offerprc		= pfold->offerlast;
		preal->bidqty		= pfold->bidvol;
		preal->offerqty		= pfold->offervol;
		preal->bidbest		= pfold->bidbest;
		preal->offerbest	= pfold->offerbest;
//	}
	
	preal->usdbid		= pusdkrw->bidlast;
	preal->usdoffer		= pusdkrw->offerlast;
/*
mds_log(m, MLOG_DEBUG, "[%s] [%.1s:%.1s:%.1s] [%.6s] [%.8s:%.9s] usd[%.6f/%.6f] [%.6f/%.6f]", __func__,
				preal->excode, preal->bidex, preal->offerex, preal->symb, preal->date, preal->time,
				preal->usdbid, preal->usdoffer, preal->bidprc, preal->offerprc);
*/
	return 0;
}

/*===============================================================
 * UPDATE된 MASTER SHM 기준으로 패킷 조합
   > 수신패킷 MDSSISE에는 시/고/저/종/전일종가 등의 정보가 없어서 MAST 활용
===============================================================*/
// 아래 함수 통합
// void upsert(MARKET *market, MDFOLD *folder, char *check)
// void mds_pushfolder(MARKET *market, void *folder, int event)
// static void folder_push(MARKET *market, void *folder, int event)
// static void push_quot(MARKET *market, MDQUOT *quot, int zdiv)
int make_cust_sise(MARKET *m, MDFOLD *pfold, MDSSISE *psise, CUSTSISE *preal)
{
	int			ii;
	char		stemp[64];
	XCHG		*xchg;
	MDARCH		*arch;
	
	arch	= (MDARCH *)(m->arch);
	xchg	= (XCHG *)&(arch->xchg);

	MEMCPY(preal->type, "FA");
	sprintf(stemp,"%c%.6s",	xchg->excode[0], pfold->symb);		MEMCPY(preal->rdcode, stemp);	// real symbol
	
	preal->excode[0] 		= xchg->excode[0];
	preal->bidex[0]			= pfold->bidex[0];		//psise->source[0];
	preal->offerex[0]		= pfold->offerex[0];	//psise->source[0];

	strcpy (stemp, 	pfold->symb);				MEMCPY(preal->symb, stemp);
	sprintf(stemp,	"%08d",	pfold->kymd);		MEMCPY(preal->kymd, stemp);
	sprintf(stemp,	"%06d",	pfold->khms/1000);	MEMCPY(preal->khms, stemp);

	//대비는 mid값 기준으로	
	sprintf(stemp,	"%c",	color(pfold->midopen, pfold->baseprc));			MEMCPY(preal->copen,		stemp);
	sprintf(stemp,	"%.8f",	pfold->midopen);								MEMCPY(preal->open,			stemp);
	sprintf(stemp,	"%c",	color(pfold->bidhigh, pfold->baseprc));			MEMCPY(preal->chigh,		stemp);
	sprintf(stemp,	"%.8f",	pfold->midhigh);								MEMCPY(preal->high,			stemp);
	sprintf(stemp,	"%c",	color(pfold->midlow,  pfold->baseprc));			MEMCPY(preal->clow,			stemp);
	sprintf(stemp,	"%.8f",	pfold->midlow);									MEMCPY(preal->low,			stemp);
	sprintf(stemp,	"%c",	color(pfold->midlast, pfold->baseprc));			MEMCPY(preal->clast,		stemp);
	sprintf(stemp,	"%.8f",	pfold->midlast);								MEMCPY(preal->last,			stemp);
	preal->sign[0]	= pfold->midsign | '0';
	sprintf(stemp,	"%c",	color(pfold->middiff, 0));						MEMCPY(preal->cdiff,		stemp);
	sprintf(stemp,	"%.*f",	pfold->zdiv, pfold->middiff);					MEMCPY(preal->diff,			stemp);
	
	sprintf(stemp,	"%c",	color(pfold->midrate, 0));						MEMCPY(preal->crate,		stemp);
	sprintf(stemp,	"%.2f",	pfold->midrate);								MEMCPY(preal->rate,			stemp);
	
	sprintf(stemp,	"%d",	pfold->offersign);								MEMCPY(preal->cpask,		stemp);
	sprintf(stemp,	"%.8f",	pfold->offerlast);								MEMCPY(preal->pask,			stemp);
	sprintf(stemp,	"%d",	pfold->bidsign);								MEMCPY(preal->cpbid,		stemp);
	sprintf(stemp,	"%.8f",	pfold->bidlast);								MEMCPY(preal->pbid,			stemp);
	sprintf(stemp,	"%.*f",	pfold->zdiv, pfold->offerlast - pfold->bidlast);MEMCPY(preal->spread,		stemp);
	
	sprintf(stemp,	"%d",	pfold->offersign);								MEMCPY(preal->bestcpask,	stemp);
	sprintf(stemp,	"%.8f",	pfold->offerbest);								MEMCPY(preal->bestpask,		stemp);
	sprintf(stemp,	"%d",	pfold->bidsign);								MEMCPY(preal->bestcpbid,	stemp);
	sprintf(stemp,	"%.8f",	pfold->bidbest);								MEMCPY(preal->bestpbid,		stemp);

	// 24.10.10) BEST 인 경우 LP중 하나라도 호가이상 중이면 마킹(bestspread[0]='1')하여 실시간 전송
	if (xchg->excode[0] == DF_EXCD_BEST)
	{
		if (pfold->quotalarm > 0)	preal->bestspread[0] = DF_C_ON;
		else						preal->bestspread[0] = DF_C_OFF;
	}
	else
	{
		sprintf(stemp,	"%.8f",	pfold->offerbest - pfold->bidbest);			MEMCPY(preal->bestspread,	stemp);
	}
	sprintf(stemp,	"%.0f",	pfold->offervol);								MEMCPY(preal->vask,			stemp);
	sprintf(stemp,	"%.0f",	pfold->bidvol);									MEMCPY(preal->vbid,			stemp);
	sprintf(stemp,	"%.0f", pfold->offerbestvol);							MEMCPY(preal->bestvask,		stemp);
	sprintf(stemp,	"%.0f",	pfold->bidbestvol);								MEMCPY(preal->bestvbid,		stemp);
	sprintf(stemp,	"%08d", arch->tymd);									MEMCPY(preal->bizdate,		stemp);

	for (ii = 0; ii < 5; ii++)
	{
		MXZINIT(stemp); sprintf(stemp, "%d", pfold->book[ii].bidsign); MEMCPY(preal->book[ii].cpbid, stemp); 
		MXZINIT(stemp); sprintf(stemp, "%.8f", pfold->book[ii].bidprc); MEMCPY(preal->book[ii].pbid, stemp); 
		MXZINIT(stemp); sprintf(stemp, "%.0f", pfold->book[ii].bidqty); MEMCPY(preal->book[ii].vbid, stemp); 
		MXZINIT(stemp); sprintf(stemp, "%d", pfold->book[ii].asksign); MEMCPY(preal->book[ii].cpask, stemp);   
		MXZINIT(stemp); sprintf(stemp, "%.8f", pfold->book[ii].askprc); MEMCPY(preal->book[ii].pask, stemp);  
		MXZINIT(stemp); sprintf(stemp, "%.0f", pfold->book[ii].askqty); MEMCPY(preal->book[ii].vask, stemp);
	}

	return 0;
}

// SMBS 체결데이터 실시간
int make_cust_xsise(MARKET *m, MDFOLD *pfold, MDSSISE *psise, CUSTXSISE *preal)
{
	char		stemp[64];
	XCHG		*xchg;
	MDARCH		*arch;
	
	arch	= (MDARCH *)(m->arch);
	xchg	= (XCHG *)&(arch->xchg);

	MEMCPY(preal->type, "FX");
	sprintf(stemp,"%c%.6s",	xchg->excode[0], pfold->symb);		MEMCPY(preal->rdcode, 	stemp);		// real symbol
	preal->excode[0] = xchg->excode[0];
	strcpy (stemp, 	pfold->symb);								MEMCPY(preal->symb,		stemp);

	memcpy(preal->trade_ymd,	psise->date, sizeof(preal->trade_ymd));
	memcpy(preal->trade_time,	psise->time, sizeof(preal->trade_time));

	if (psise->bidprc > 0.)
	{
		preal->trade_side[0] = '1';
		sprintf(stemp,	"%.8f",	psise->bidprc);		MEMCPY(preal->trade_prc,	stemp);
		sprintf(stemp,	"%.f",	psise->bidqty);		MEMCPY(preal->trade_vol,	stemp);
	}
	else
	{
		preal->trade_side[0] = '2';
		sprintf(stemp,	"%.8f",	psise->offerprc);	MEMCPY(preal->trade_prc,	stemp);
		sprintf(stemp,	"%.f",	psise->offerqty);	MEMCPY(preal->trade_vol,	stemp);
	}

	return 0;
}


//################################################################################################
// BUSINESS
//################################################################################################

/*===============================================================
 * 재정통화 생성 대상 체크
  > USDKRW는 NULL 리턴
  > USDCNH는 시장용으로 재정생성 안함.
★TODO-17 : 특정 통화에 대한 기존 로직 분석 및 적용 : CNH에 대한 는 재정을 만들지 않는다. 직접받는 CNHKRW시세 사용 
===============================================================*/
int iscrosstarget(char *basesymb)
{
	if (memcmp(basesymb, "USDKRW", 6) == 0 || memcmp(basesymb, "USDCNH", 6) == 0 ||
		memcmp(basesymb, "CNHKRW", 6) == 0 || memcmp(basesymb, "MARKRW", 6) == 0)
	{
		return (-1);
	}

	if (memcmp(basesymb, "USD", 3) != 0 && memcmp(&basesymb[3], "USD", 3) != 0 &&
	    memcmp(basesymb, "KRW", 3) != 0 && memcmp(&basesymb[3], "KRW", 3) != 0)
	{
		return (-2);
	}

	return 0;
}

/*===============================================================
 * MAST SHM (FOLDER) UPDATE
===============================================================*/
// 아래 함수 통합
// MDFOLD *getSpotquot(MARKET *m, char *msgb, int msgl, char *check)
// int  quote_proc(MARKET *m, MDFOLD *folder, struct q_data *qd, char *check, int nType)
int update_master(MARKET *m, MDFOLD *pfold, MDSSISE *psise)
{
	int ii;
	double diff, rate, prebid, preoffer;

	// CUST인 경우      : 최초 로딩시, 수기전환 시, 중지해제 시 > 거래가능으로 상태변경
	// CUST가 아닌 경우 : 첫 시세 수신 시 > 거래가능으로 상태변경
	if (pfold->trdf == DF_OFF)	{
		if (m->excode[0] == DF_EXCD_CUST) {
			if (pfold->cust.feedtp[0] != DF_CFEED_STOP)	pfold->trdf = DF_ON;
		} else
			pfold->trdf = DF_ON;

		mds_log(m, MLOG_BIZ, "[%s] [%.1s:%.6s] trdf changed.. trdf=[%d]", __func__, m->excode, pfold->symb, pfold->trdf);
	}

	pfold->kymd	= str2i(psise->date, sizeof(psise->date));
	pfold->khms	= str2i(psise->time, sizeof(psise->time));

	// 전일종가 없는 경우.. 최초 수신 시세로 전일종가 세팅	
	if (pfold->baseprc <= 0.)	pfold->baseprc = (psise->bidprc + psise->offerprc) / 2;

	// BID UPDATE
	if (pfold->bidlast <= 0)	prebid = psise->bidprc;
	else						prebid = pfold->bidlast;

	memcpy(pfold->bidex, psise->excode, sizeof(pfold->bidex));
	pfold->bidlast	= psise->bidprc;
	pfold->bidbest	= psise->bidbestprc;

	if (pfold->bidopen == 0)
		pfold->bidopen = psise->bidprc;
		
	if (pfold->bidhigh < psise->bidprc)
		pfold->bidhigh = psise->bidprc;

	if ((pfold->bidlow  > psise->bidprc ) || (pfold->bidlow == 0))
		pfold->bidlow  = psise->bidprc ;

	get_diff_rate(pfold->bidlast, prebid, &diff, &rate);	// 직전틱 bid기준 등락
	pfold->biddiff	= diff;
	pfold->bidrate	= rate;
	
	if (prebid - psise->bidprc < -0.0000001)
	{
		pfold->biddirf = '+';
		pfold->bidsign = _UP_;
	}
	else if (prebid - psise->bidprc > 0.0000001)
	{
		pfold->biddirf = '-';
		pfold->bidsign = _DN_;  
	}
	else
	{
		pfold->biddirf = '=';
		pfold->bidsign = _NC_;  
	}
	pfold->bidvol		= psise->bidqty;
	pfold->bidbestvol	= psise->bidbestqty;
	
	// OFFER UPDATE
	if (pfold->offerlast <= 0)	preoffer = psise->offerprc;
	else						preoffer = pfold->offerlast;
	
	memcpy(pfold->offerex, psise->excode, sizeof(pfold->offerex));
	pfold->offerlast	= psise->offerprc;
	pfold->offerbest	= psise->offerbestprc;
	
	if (pfold->offeropen == 0)
		pfold->offeropen = psise->offerprc;
		
	if (pfold->offerhigh < psise->offerprc)
		pfold->offerhigh = psise->offerprc;
	
	if ((pfold->offerlow  > psise->offerprc ) || (pfold->offerlow == 0))
		pfold->offerlow  = psise->offerprc ;
	
	get_diff_rate(pfold->offerlast, preoffer, &diff, &rate);		// 직전틱 offer기준 등락
	pfold->offerdiff	= diff;
	pfold->offerrate	= rate;
	
	if (preoffer < psise->offerprc)
	{
		pfold->offerdirf = '+';
		pfold->offersign = _UP_;
	}
	else if (preoffer > psise->offerprc)
	{
		pfold->offerdirf = '-';
		pfold->offersign = _DN_;  
	}
	else
	{
		pfold->offerdirf = '=';
		pfold->offersign = _NC_;  
	}
	pfold->offervol		= psise->offerqty;
	pfold->offerbestvol	= psise->offerbestqty;

	// MID UPDATE
	pfold->midlast	= (pfold->bidlast + pfold->offerlast)/2;
	pfold->midbest	= (pfold->bidbest + pfold->offerbest)/2;
	
	if (pfold->midopen == 0)
		pfold->midopen = pfold->midlast;
	
	if (pfold->midhigh < pfold->midlast)
		pfold->midhigh = pfold->midlast;
	
	if ((pfold->midlow  > pfold->midlast ) || (pfold->midlow == 0))
		pfold->midlow  = pfold->midlast;

	get_diff_rate(pfold->midlast, pfold->baseprc, &diff, &rate);	// 전일 mid종가기준 등락
	pfold->middiff	= diff;
	pfold->midrate	= rate;
	
	if (pfold->baseprc - pfold->midlast < -0.0000001)
	{
		pfold->middirf = '+';
		pfold->midsign = _UP_;
	}
	else if (pfold->baseprc - pfold->midlast > 0.0000001)
	{
		pfold->middirf = '-';
		pfold->midsign = _DN_;  
	}
	else
	{
		pfold->middirf = '=';
		pfold->midsign = _NC_;  
	}

	for (ii = 0; ii < 5; ii++)
	{
		if (prebid - psise->book[ii].pbid < -0.0000001)
			pfold->book[ii].bidsign = _UP_;
		else if (prebid - psise->book[ii].pbid > 0.0000001)
			pfold->book[ii].bidsign = _DN_;
		else
			pfold->book[ii].bidsign = _NC_;

		if (preoffer < psise->book[ii].pask)
			pfold->book[ii].asksign = _UP_;
		else if (preoffer > psise->book[ii].pask)
			pfold->book[ii].asksign = _DN_;
	    else
	    	pfold->book[ii].asksign = _NC_;

		pfold->book[ii].bidprc = psise->book[ii].pbid;
		pfold->book[ii].bidqty = psise->book[ii].vbid;
		pfold->book[ii].askprc = psise->book[ii].pask;
		pfold->book[ii].askqty = psise->book[ii].vask;
	}

	// ETC
	pfold->prebid    = prebid;
	pfold->preoffer  = preoffer;
/*
mds_log(m, MLOG_DEBUG, "[%s] [%.1s:%.6s:%d] div[%d:%d:%d] base[%.6f]", 
		__func__, m->excode, pfold->symb, pfold->seqn, pfold->zdiv, pfold->zCustdiv, pfold->swapzdiv, pfold->baseprc);
mds_log(m, MLOG_DEBUG, "\t\tbid[%.6f/%.6f/%.6f/%.6f]", pfold->bidopen, pfold->bidhigh, pfold->bidlow, pfold->bidlast);
mds_log(m, MLOG_DEBUG, "\t\toffer[%.6f/%.6f/%.6f/%.6f]", pfold->offeropen, pfold->offerhigh, pfold->offerlow, pfold->offerlast);
mds_log(m, MLOG_DEBUG, "\t\tmid[%.6f/%.6f/%.6f/%.6f]", pfold->midopen, pfold->midhigh, pfold->midlow, pfold->midlast);
*/

	// 시세 변동여부 체크
	if (pfold->bidlast != prebid || pfold->offerlast != preoffer)
		return 1;

	return 0;
}

int update_best(MARKET *m, MDFOLD *pfold, MDSSISE *psise)
{
	int		chk=0, rtn;
	double	diff, rate, prebid, preoffer;
	char	msg[256];
	MDFOLD	best;

	// CUST인 경우      : 최초 로딩시, 수기전환 시, 중지해제 시 > 거래가능으로 상태변경
	// CUST가 아닌 경우 : 첫 시세 수신 시 > 거래가능으로 상태변경
	if (pfold->trdf == DF_OFF)
	{
		pfold->trdf = DF_ON;
		mds_log(m, MLOG_BIZ, "[%s] [%.1s:%.6s] trdf changed.. trdf=[%d]", __func__, m->excode, pfold->symb, pfold->trdf);
	}

	memset(msg,   0x00, sizeof(msg));
	memset(&best, 0x00, sizeof(best));

	memcpy(best.symb, pfold->symb, sizeof(best.symb));

	rtn = mds_getbestfold(&best, psise, msg);
	if (rtn < 0)		// BEST 호가 대상 LP가 없는 경우 초기화
	{
		pfold->bidex[0]   = DF_EXCD_BEST;
		pfold->offerex[0] = DF_EXCD_BEST;
		pfold->quotalarm  = time(0);
		mds_log(m, MLOG_BIZ, "[%s] [%.1s:%.6s] no LP for this symbol.. reset excode. ", __func__, m->excode, pfold->symb);
		return 0;
	}
	
	pfold->quotalarm = best.quotalarm;

	pfold->kymd	= str2i(psise->date, sizeof(psise->date));
	pfold->khms	= str2i(psise->time, sizeof(psise->time));

	// BID UPDATE (BEST BID 가격이 달라졌을 경우)
	if (pfold->bidlast != best.bidlast || pfold->bidex[0] != best.bidex[0])
	{
		prebid = pfold->bidlast;
		
		memcpy(pfold->bidex, best.bidex, sizeof(pfold->bidex));
		pfold->bidlast	= best.bidlast;
		pfold->bidbest	= best.bidbest;
		
		if (pfold->bidopen == 0)
			pfold->bidopen = best.bidlast;
			
		if (pfold->bidhigh < best.bidlast)
			pfold->bidhigh = best.bidlast;

		if ((pfold->bidlow > best.bidlast ) || (pfold->bidlow <= 0))
			pfold->bidlow = best.bidlast;

		get_diff_rate(pfold->bidlast, prebid, &diff, &rate);	// 직전틱 bid기준 등락
		pfold->biddiff	= diff;
		pfold->bidrate	= rate;

		if (prebid - best.bidlast < -0.0000001)
		{
			pfold->biddirf = '+';
			pfold->bidsign = _UP_;
		}
		else if (prebid - best.bidlast > 0.0000001)
		{
			pfold->biddirf = '-';
			pfold->bidsign = _DN_;  
		}
		else
		{
			pfold->biddirf = '=';
			pfold->bidsign = _NC_;  
		}
		pfold->bidvol		= best.bidvol;
		pfold->bidbestvol	= best.bidbestvol;
		pfold->prebid		= prebid;

		chk=1;
	}
	// OFFER UPDATE (BEST OFFER 가격이 달라졌을 경우)
	if (pfold->offerlast != best.offerlast || pfold->offerex[0] != best.offerex[0])
	{
		preoffer = pfold->offerlast;
		
 		memcpy(pfold->offerex, best.offerex, sizeof(pfold->offerex));
		pfold->offerlast	= best.offerlast;
		pfold->offerbest	= best.offerbest;
		
		if (pfold->offeropen == 0)
			pfold->offeropen = best.offerlast;
			
		if (pfold->offerhigh < best.offerlast)
			pfold->offerhigh = best.offerlast;
		
		if ((pfold->offerlow  > best.offerlast ) || (pfold->offerlow == 0))
			pfold->offerlow  = best.offerlast ;
		
		get_diff_rate(pfold->offerlast, preoffer, &diff, &rate);		// 직전틱 offer기준 등락
		pfold->offerdiff	= diff;
		pfold->offerrate	= rate;
		
		if (preoffer < best.offerlast)
		{
			pfold->offerdirf = '+';
			pfold->offersign = _UP_;
		}
		else if (preoffer > best.offerlast)
		{
			pfold->offerdirf = '-';
			pfold->offersign = _DN_;  
		}
		else
		{
			pfold->offerdirf = '=';
			pfold->offersign = _NC_;  
		}
		pfold->offervol		= best.offervol;
		pfold->offerbestvol	= best.offerbestvol;
		pfold->preoffer		= preoffer;
		
		chk=1;
	}
	// BID/OFFER 중 변경 내용이 있는 경우 MID 계산
	if (chk || pfold->quotalarm)	// 24.10.10) BEST 인 경우 LP중 하나라도 호가이상 중이면 마킹(bestspread[0]='1')하여 실시간 전송
	{
		// MID UPDATE
		pfold->midlast	= (pfold->bidlast + pfold->offerlast)/2;
		pfold->midbest	= (pfold->bidbest + pfold->offerbest)/2;
		
		if (pfold->midopen == 0)
			pfold->midopen = pfold->midlast;
		
		if (pfold->midhigh < pfold->midlast)
			pfold->midhigh = pfold->midlast;
		
		if ((pfold->midlow  > pfold->midlast ) || (pfold->midlow == 0))
			pfold->midlow  = pfold->midlast ;
		
		get_diff_rate(pfold->midlast, pfold->baseprc, &diff, &rate);	// 전일 mid종가기준 등락
		pfold->middiff	= diff;
		pfold->midrate	= rate;
		
		if (pfold->baseprc - pfold->midlast < -0.0000001)
		{
			pfold->middirf = '+';
			pfold->midsign = _UP_;
		}
		else if (pfold->baseprc - pfold->midlast > 0.0000001)
		{
			pfold->middirf = '-';
			pfold->midsign = _DN_;  
		}
		else
		{
			pfold->middirf = '=';
			pfold->midsign = _NC_;  
		}
		return 1;
	}
/*
mds_log(m, MLOG_DEBUG, "[%s] [%.1s:%.6s:%d] div[%d:%d:%d] base[%.6f]", 
		__func__, m->excode, pfold->symb, pfold->seqn, pfold->zdiv, pfold->zCustdiv, pfold->swapzdiv, pfold->baseprc);
mds_log(m, MLOG_DEBUG, "\t\tbid[%.6f/%.6f/%.6f/%.6f]", pfold->bidopen, pfold->bidhigh, pfold->bidlow, pfold->bidlast);
mds_log(m, MLOG_DEBUG, "\t\toffer[%.6f/%.6f/%.6f/%.6f]", pfold->offeropen, pfold->offerhigh, pfold->offerlow, pfold->offerlast);
mds_log(m, MLOG_DEBUG, "\t\tmid[%.6f/%.6f/%.6f/%.6f]", pfold->midopen, pfold->midhigh, pfold->midlow, pfold->midlast);
*/
	return 0;
}

int mds_sise_valid(MARKET *m, MDFOLD *pfold, MDSSISE *psise)
{
	char	alrmsg[128];
	MDARCH	*parch = (MDARCH *)m->arch;

	// 35=X 체결데이터 인 경우, MARKRW 시세인 경우, 수시시세는 검증 예외
	if (memcmp(pfold->symb, "MARKRW", 6) == 0 || psise->quotid[0] == 'X')
		return 0;

	// 메모리 정보 초기화 오류인 경우
	if (pfold->baseprc <= 0.000001 || pfold->bidlast <= 0.000001 || pfold->offerlast <= 0.000001)
		return (0);	//(1);
/*
	// BID,ASK 호가 모두 '0'인 경우
	if (psise->bidprc <= 0.000001 && psise->offerprc <= 0.000001)
		return (1);
*/
	// 호가 중 하나 이상 빈 값일 경우
	if (psise->bidprc <= 0.0000001 || psise->offerprc <= 0.0000001)
	{
		if (pfold->trdf == DF_ON && pfold->quotalarm == DF_OFF) {

			pfold->quotalarm = (time_t)time(0);
mds_log(m, MLOG_ERROR, "(%s) quot alarm set : [%.6s]", __func__, pfold->symb);
		}
		return (2);
	}

	// 24.07.29) 알람발생 이후 정상호가 수신 시 알람상태 OFF
	if (pfold->quotalarm != DF_OFF)		// time_t 값으로 세팅되어 있기 때문에 DF_ON 으로 비교하면 안됨
	{
		time_t t = time(0);
mds_log(m, MLOG_ERROR, "(%s) quot alarm off : [%.6s] (%d sec)", __func__, pfold->symb, t - pfold->quotalarm);
		pfold->quotalarm = DF_OFF;
	}

	// 전일종가 대비 50%이상 변동인 경우 버림
	if ((pfold->baseprc * 1.5 < psise->bidprc   || pfold->baseprc * 0.5 > psise->bidprc) ||
		(pfold->baseprc * 1.5 < psise->offerprc || pfold->baseprc * 0.5 > psise->offerprc))
		return (3);

	// 고객 시세가 역전된 경우는 버림
	if (m->excode[0] == DF_EXCD_CUST && psise->bidprc >= psise->offerprc)
		return (4);

#if 0
	// 직전가 대비 1% 이상 변동 시 알람
	double midprc = (psise->bidprc + psise->offerprc)/2;

	if (pfold->midlast * 1.01 < midprc || pfold->midlast * 0.99 > midprc)
	{
		sprintf(alrmsg, "%s.%.6s %d|", parch->xchg.exnm, pfold->symb, t - pfold->quotalarm);
		if (mds_send_alarm(DF_ALRM_QUOT, alrmsg) == 0)	pfold->quotalarm = DF_OFF;
mds_log(market, MLOG_ERROR, "(%s) mds_send_alarm : [%.6s] (%s)", __func__, pfold->symb, alrmsg);
	}
#endif

	return 0;
}
