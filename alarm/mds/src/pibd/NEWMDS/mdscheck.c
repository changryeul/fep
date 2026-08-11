
#include <curses.h>
#include "mds2.h"

typedef struct {
	char    excode  [ 1];       // 원천 : 'S'=SMB, 'K'=KMB, 'E'=EBS, 'C'=CMB
	char    exnm    [ 8];
	int     ipck    ;
	MDARCH  *arch   ;           // MDARCH
	INDEX   *indx   ;           // INDEX
	MDFOLD  *fold   ;           // FOLD
} EX ;

enum {
	DF_INIT_NONE    = 0,
	DF_INIT_READ    ,
	DF_INIT_DONE
};

/* -----------  prototype  ------------------------------- */
key_t get_ipck2(int exid) ;
int mds_attach(int mode, char *msg) ;
int printall(char *exnm) ;
int iscrosstarget(char *basesymb) ;

/* -----------  for global  ------------------------------- */
EX  g_ex[MAX_XCHG];

MARKET *market ;


int main(int argc, char **argv)
{
	int		ncnt;
	char	iexnm[32], iexcd[32], isymb[32];
	char	msg[512];
	MDFOLD	*mdfold=NULL;
	int 	i, j, z ;
	int 	wid ;

	MDARCH *arch ;
	MDFOLD *fold ;
	XCHG	xchg ;

	// get the all of the arch
	if ((ncnt=mds_attach(0, msg)) < 0)
	{
		fprintf(stdout, "unknown error (%d)\n", ncnt);
		return -1;
	}


	for (i = 0 ; i<ncnt; i++)
	{
		fprintf(stdout, " (%d) -----------------------------------------------------------------------\n", i);
		if (g_ex[i].excode[0] == 0x00 || g_ex[i].exnm[0] == 0x00)
			continue ;

		fprintf(stdout, " * excode:%-*.*s %-*.*s IPCKEY:0x%x\n", sizeof(g_ex[i].excode),  sizeof(g_ex[i].excode), g_ex[i].excode, sizeof(g_ex[i].exnm), sizeof(g_ex[i].exnm), g_ex[i].exnm, g_ex[i].ipck);

		// for the test
		if ((market = mds_open2(g_ex[i].exnm, O_RDWR|O_CREAT)) == NULL)
		{
			fprintf(stdout,  "failed to mds_open2 !\n");
			exit(0);
		}

#if 0
		char sTime[21];
		while (mdfold = mds_popfolder(market))
		{
			memset(sTime, 0, sizeof(sTime));
			GetTime(sTime);
			//fprintf(stdout, " mdfold-> seq:%d symb:%s pricestat(%d) trdf:%d custfeed:%.1s bidex(%.1s/%f)\n",  mdfold->seqn, mdfold->symb, mdfold->pricestat, mdfold->trdf, mdfold->custfeedtp, mdfold->bidex, mdfold->bidlast);
			fprintf(stdout, " check! mds_popfoler check ... mdfold-> seq:%d symb:%s\n",  mdfold->seqn, mdfold->symb);
		}
#endif

		arch = g_ex[i].arch ;
		fold = g_ex[i].fold ;
		xchg = arch->xchg ;
		fprintf(stdout, " > %-4.4s exid(%d) code(%.1s) custflg(%c) quenm(%s) dirp(%s) loglevel(%d) logf(%s) max.%d IP:%s->%s xchg.num(ma:%d / ap:%d / db:%d)\n", 
				xchg.exnm, xchg.exid, xchg.excode, xchg.custflag, xchg.quenm, xchg.dirp, xchg.llog, xchg.logf, xchg.maxcnt,  xchg.from.ipad, xchg.apsnd.ipad, xchg.ma_pnum,  xchg.ap_pnum,xchg.db_pnum);

		for (j=0 ; j<xchg.maxcnt; j++)
		{
			if (fold->symb[0] == 0x00)
				break ;

			z = fold->zdiv ;
			wid = 9; // for test 

			fprintf(stdout, " seqn:%d", fold->seqn		);			// MDFOLD OFFSET
			fprintf(stdout, " %s",   fold->symb) ;
			if (iscrosstarget(fold->symb) != 0)
				fprintf(stdout, " C/F");
			else
				fprintf(stdout, " C/T");


			fprintf(stdout, " kymd:%d", fold->kymd		);			// last update date
			fprintf(stdout, " khms:%d", fold->khms		);			// last update time
			fprintf(stdout, " usdpos:%d", fold->usdpos		);			// pos
			fprintf(stdout, "\n     ");
#if  1
			fprintf(stdout, " zdiv:%15d", fold->zdiv		);			// no of Employ decimals
			fprintf(stdout, " zCustdiv:%15d", fold->zCustdiv	);			// no of decimals
			fprintf(stdout, " swapdiv:%15d", fold->swapzdiv	);			// no of Swap decimals
			fprintf(stdout, "\n     ");
			fprintf(stdout, " pricestat:%15d", fold->pricestat);			// 현재 가격원천
			fprintf(stdout, " custfeedtp:%15.1s", fold->custfeedtp);			// 고객시세 제공 타입 : TSKEIAM97.PRC_ORIGN_DSTCD 값 (중단/수기/우선/기본)
			fprintf(stdout, " custfinclyn:%15.1s", fold->custfinclyn);			// 고객재정시세 제공여부 : TSKEIAM97.FINCL_ANOUN_STOP_YN
			fprintf(stdout, "\n     ");
			fprintf(stdout, " trdf:%d",   fold->trdf		);			// tradable flag
			fprintf(stdout, " tymd:%d",   fold->tymd		);			// current trading day
			fprintf(stdout, " spotdate:%8.8s", fold->spotdate	);			// spot date
			fprintf(stdout, " custtm:%8.8s.%6.6s~%8.8s.%6.6s", fold->custstm, &fold->custstm[8], fold->custetm, &fold->custetm[8]);			// current trading day
			fprintf(stdout, "\n     ");
#endif
			fprintf(stdout, " baseprc:%*.*f", wid, z, fold->baseprc );
			fprintf(stdout, " ex:%*s %*s",  1, fold->bidex, 1, 	fold->offerex	);
			fprintf(stdout, "\n     ");
			fprintf(stdout, " last:%*.*f %*.*f mid:%*.*f", wid,z, fold->bidlast, wid, z, fold->offerlast, wid, z, fold->midlast	);
			fprintf(stdout, " open:%*.*f %*.*f mid:%*.*f", wid, z, fold->bidopen, wid, z, fold->offeropen, wid, z, fold->midopen	);
			fprintf(stdout, " high:%*.*f %*.*f mid:%.*f", wid, z, fold->bidhigh, wid, z, fold->offerhigh, wid, z, fold->midhigh	);
			fprintf(stdout, " low:%*.*f %*.*f mid:%*.*f", wid, z, fold->bidlow , wid, z, fold->offerlow , wid, z, fold->midlow	);
			fprintf(stdout, " best:%*.*f %*.*f %*.*f", wid, z, fold->bidbest, wid, z, fold->offerbest, wid, z, fold->midbest	);
			fprintf(stdout, "\n     ");
#if  0
			fprintf(stdout, " %15.*f %15.*f", z, fold->prebid,z, fold->preoffer	);
			fprintf(stdout, " %15.2f %15.2f %15.2f", fold->bidvol,	 fold->offervol	);
			fprintf(stdout, " %15.2f %15.2f", 	fold->bidbestvol, fold->offerbestvol );
			fprintf(stdout, "\n     ");
			fprintf(stdout, " %15d] %15d %15d",	fold->bidsign, fold->offersign,fold->midsign);
			fprintf(stdout, " %15.*f %15.*f %15.*f", z, fold->biddiff, z, fold->offerdiff,z, fold->middiff);
			fprintf(stdout, " %15.2f %15.2f %15.2f", fold->bidrate, fold->offerrate,fold->midrate);
			fprintf(stdout, " %15d %15d %15d ^",   	fold->biddirf, fold->offerdirf,fold->middirf);
#endif			
			fprintf(stdout, "\n");

			fold++;
		}


		fflush(stdout);

	} /* End of  for (i = 0 ; ; i++) */

#if 0
	if (argc == 2)
	{
		printall(argv[1]);
		return 0;
	}
	else if (argc == 3)
	{
		if ((mdfold = mds_getfold(argv[1], argv[2], msg)) != NULL)
			printfold(mdfold);
		
		return 0;
	}

	while (1)
	{
		memset(msg,   0x00, sizeof(msg));
		memset(iexcd, 0x00, sizeof(iexcd));
		memset(iexnm, 0x00, sizeof(iexnm));
		memset(isymb, 0x00, sizeof(isymb));
		
		printf("\nEnter EXNAME('S'MBS/'K'MBS/'C'MBS/'B'EST/'Z'CUST/'q'uit) : ");
		scanf("%s", iexcd);
		if (strlen(iexcd) < 1)	continue;
		
		     if (iexcd[0] == 'S')	strcpy(iexnm, "SMBS");
		else if (iexcd[0] == 'K')	strcpy(iexnm, "KMB");
		else if (iexcd[0] == 'E')	strcpy(iexnm, "EBS");
		else if (iexcd[0] == 'C')	strcpy(iexnm, "CMBS");
		else if (iexcd[0] == 'B')	strcpy(iexnm, "BEST");
		else if (iexcd[0] == 'Z')	strcpy(iexnm, "CUST");
		else if (iexcd[0] == 'T')	strcpy(iexnm, "TEST");
		else if (iexcd[0] == 'q')	break;
		else {
			printf("\nInput Error ");
			continue;
		}

		printf("Enter Symbol(all) : ");
		scanf("%s", isymb);
		if (strcmp(isymb, "all") == 0)
		{
			printall(iexnm);

			getchar();
			continue;
		}
		else if (strlen(isymb) < 6)		continue;

		printf("-------------------------------------------------------------------\n");

		mdfold = mds_getfold(iexnm, isymb, msg);
		printf("%s\n", msg);
		printf("===================================================================\n");
		if (mdfold==NULL)
		{
			printf("\n code not found.. \n");
			continue;
		}
		
		printfold(mdfold);

		getchar();
	}
#endif

	return 0;

} /* End of int main(int argc, char **argv) */



int mds_attach(int mode, char *msg)
{
	int     ii=0, excnt=0, nxml;
	char    xmlpath[128], args[52];
	struct  xmltag xmltags[256];
	static  __thread int initialized = 0;

	struct EXINFO {
		int     exid;
		char    exnm [64];
		char    quenm[64];
	} exinfo[MAX_XCHG];

	if (initialized == DF_INIT_DONE)    return 0;

	if (!initialized)
	{
		/*===============================================================
		* XML파일(exchanges.cfg) 에서 원천 리스트 정보 READ
		===============================================================*/
		memset(xmltags, 0x00, sizeof(xmltags));
		memset(&exinfo, 0x00, sizeof(exinfo));

		sprintf(xmlpath, "%s/exchanges.new.cfg", ETC_DIR);
		nxml = getxmlcfg(xmlpath, xmltags);

		fprintf(stdout, " --- config : %s   and nxml:%d \n", xmlpath, nxml);

		if (nxml <= 0)  return (-1);

		for (ii = 0; ii < nxml; ii++)
		{
			getargs(&xmltags[ii], "name",  exinfo[excnt].exnm);
			getargs(&xmltags[ii], "exid",  args);
			exinfo[excnt].exid = atoi(args);
			getargs(&xmltags[ii], "qname", exinfo[excnt].quenm);

			if (exinfo[excnt].exid <= 0 || strlen(exinfo[excnt].exnm) <= 0 || strlen(exinfo[excnt].quenm) <= 0)
				continue;

			excnt++;
		}

		if (!excnt)     return 0;

		memset(&g_ex, 0x00, sizeof(g_ex));

		for (ii=0; ii < excnt; ii++)
		{
			strcpy(g_ex[ii].exnm, exinfo[ii].exnm);
			g_ex[ii].ipck = get_ipck2(exinfo[ii].exid);
		}

		initialized = DF_INIT_READ;
	}

	MDARCH  *arch;
	int     shmid, setstat=1, setcnt=0;

	for (ii=0; ; ii++)
	{
		if (strlen(g_ex[ii].exnm) <= 0)     break;

		if (g_ex[ii].arch)  continue;

		if ((shmid = shmget(IPCK(g_ex[ii].ipck, 0), 0, 0666)) < 0)
		{
			//sprintf(&msg[strlen(msg)], "\n<%s : shmget error : %s>\n", g_ex[ii].exnm, strerror(errno));
			setstat = 0;
			continue;
		}

		arch = (MDARCH *)shmat(shmid, (char *)0, (mode==0?SHM_RDONLY:0));
		if (!arch)
		{
			//sprintf(&msg[strlen(msg)], "\n%s %s\n", g_ex[ii].exnm, strerror(errno));
			setstat = 0;
			continue;
		}
		memcpy(g_ex[ii].excode, arch->xchg.excode, sizeof(arch->xchg.excode));

		g_ex[ii].arch = (MDARCH *)arch;
		g_ex[ii].indx = (INDEX *)((char *)arch + sizeof(MDARCH));
		g_ex[ii].fold = (MDFOLD *)((char *)arch + sizeof(MDARCH) + (sizeof(INDEX)*arch->xchg.maxcnt));

		sprintf(&msg[strlen(msg)], " %s", g_ex[ii].exnm);
		setcnt++;
	}

	if (setcnt) sprintf(&msg[strlen(msg)], " 마스터메모리 ATTACH OK");

	if (setstat)    initialized = DF_INIT_DONE;

	return nxml;

} /* End of int mds_attach(int mode, char *msg) */

key_t get_ipck2(int exid)
{
	char    name[30], decimal;
	int     hexa;
	key_t   ipck;
	int     ii;

	sprintf(name, "%03d", exid);
	for (ii = 0, hexa = 0; ii < strlen(name) && ii < 3; ii++)
	{
		hexa <<= 4;
		decimal = name[ii] & 0x0f;
		hexa |= decimal;
	}

	hexa |= 0x9000;
	ipck = (hexa << 16);                // 0x9{EXID}??##

	return(ipck);
}

int printall(char *exnm)
{
	int		ii, con=0;
	char	msg[128], ctime[16];
	time_t	clock;
	struct	tm *tm;
	MDFOLD	*mdfold=NULL, *p;
	MDARCH	*arch=NULL;

	clock = time(0);
	tm=localtime(&clock);

	sprintf(ctime, "%4d%02d%02d%02d%02d%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	arch = mds_getarch(0, exnm, msg);
	if (arch == NULL)
	{
		printf("\n mds_getarch error..(%s)\n", msg);
		return (-1);
	}

	mdfold = (MDFOLD *)((char *)arch + sizeof(MDARCH) + (sizeof(INDEX)*arch->xchg.maxcnt));
	printf("= TOT [%d] [%02d:%02d:%02d] ===============================================\n", 
			arch->rsum, tm->tm_hour, tm->tm_min, tm->tm_sec);

	p = mdfold;

	for (ii=0; ii<arch->nrec; ii++, p++)
	{
		if (memcmp(ctime, p->custstm, sizeof(p->custstm)) >= 0 && memcmp(ctime, p->custetm, sizeof(p->custstm)) <= 0)
			con = 1;
		else	con = 0;

		printf("[%.6s:%02d] [%d:%d] [%d][%02u:%02u.%02u] [%.1s/%11.5f] [%.1s/%11.5f] %s\n"
				,p->symb
				,p->seqn
				,p->pricestat
				,p->trdf
				,p->tymd
				,p->khms/10000000, (p->khms/100000)%100, (p->khms/1000)%100
				,p->bidex
				,p->bidlast
				,p->offerex
				,p->offerlast
				,(con? "\033[32mCUST\033[0m":""));
	}

	fflush(stdout);

	return 0;
}

int printfold(MDFOLD *mdfold)
{
	int z;

	if (mdfold == NULL)		return (-1);
	z = mdfold->zdiv;
	
	printf("-----------------------------------------------\n");
	printf("\tsymb      = [%15s]\n", mdfold->symb		);
	printf("\tseqn      = [%15d]\n", mdfold->seqn		);			// MDFOLD OFFSET
	printf("\tkymd      = [%15d]\n", mdfold->kymd		);			// last update date
	printf("\tkhms      = [%15d]\n", mdfold->khms		);			// last update time
	printf("\tzdiv      = [%15d]\n", mdfold->zdiv		);			// no of Employ decimals
	printf("\tzCustdiv  = [%15d]\n", mdfold->zCustdiv	);			// no of decimals
	printf("\tswapzdiv  = [%15d]\n", mdfold->swapzdiv	);			// no of Swap decimals
	printf("\tpricestat = [%15d]\n", mdfold->pricestat	);			// 현재 가격원천
	printf("\tfeed      = [%15.1s]\n", mdfold->custfeedtp);			// 고객시세 제공 타입 : TSKEIAM97.PRC_ORIGN_DSTCD 값 (중단/수기/우선/기본)
	printf("\tfinclyn   = [%15.1s]\n", mdfold->custfinclyn);			// 고객재정시세 제공여부 : TSKEIAM97.FINCL_ANOUN_STOP_YN
	printf("\ttrdf      = [%15d]\n", mdfold->trdf		);			// tradable flag
	printf("\ttymd      = [%15d]\n", mdfold->tymd		);			// current trading day
	printf("\tspot      = [%15.8s]\n", mdfold->spotdate	);			// spot date
	printf("\ttime      = [%8.8s.%6.6s] - [%8.8s.%6.6s]\n", mdfold->custstm, &mdfold->custstm[8], mdfold->custetm, &mdfold->custetm[8]);			// current trading day
	printf("\tbaseprc   = [%15.*f]\n\n", z, mdfold->baseprc );
	printf("\tbidex     = [%15s]    offerex   = [%15s]\n",     mdfold->bidex,	mdfold->offerex	);
	printf("\tbidlast   = [%15.*f]    offerlast = [%15.*f]    midlast = [%15.*f]\n\n", z, mdfold->bidlast,z, mdfold->offerlast, z, mdfold->midlast	);
	printf("\tbidopen   = [%15.*f]    offeropen = [%15.*f]    midopen = [%15.*f]\n", z, mdfold->bidopen,z, mdfold->offeropen, z, mdfold->midopen	);
	printf("\tbidhigh   = [%15.*f]    offerhigh = [%15.*f]    midhigh = [%15.*f]\n", z, mdfold->bidhigh,z, mdfold->offerhigh, z, mdfold->midhigh	);
	printf("\tbidlow    = [%15.*f]    offerlow  = [%15.*f]    midlow  = [%15.*f]\n", z, mdfold->bidlow ,z, mdfold->offerlow , z, mdfold->midlow	);
	printf("\tbidbest   = [%15.*f]    offerbest = [%15.*f]    midbest = [%15.*f]\n", z, mdfold->bidbest,z, mdfold->offerbest, z, mdfold->midbest	);
	printf("\tprebid    = [%15.*f]    preoffer  = [%15.*f]\n", z, mdfold->prebid,z, mdfold->preoffer	);
	printf("\tbidvol    = [%15.2f]    offervol  = [%15.2f]    midvol  = [%15.2f]\n", mdfold->bidvol,	 mdfold->offervol	);
	printf("\tbidbestvol= [%15.2f]    offerbestv= [%15.2f]\n", mdfold->bidbestvol, mdfold->offerbestvol );
	printf("\tbidsign   = [%15d]    offersign = [%15d]    midsign = [%15d]\n",   mdfold->bidsign, mdfold->offersign,mdfold->midsign);
	printf("\tbiddiff   = [%15.*f]    offerdiff = [%15.*f]    middiff = [%15.*f]\n", z, mdfold->biddiff, z, mdfold->offerdiff,z, mdfold->middiff);
	printf("\tbidrate   = [%15.2f]    offerrate = [%15.2f]    midrate = [%15.2f]\n", mdfold->bidrate, mdfold->offerrate,mdfold->midrate);
	printf("\tbiddirf   = [%15d]    offerdirf = [%15d]    middirf = [%15d]\n",   mdfold->biddirf, mdfold->offerdirf,mdfold->middirf);
	printf("-----------------------------------------------\n");
	
	fflush(stdout);
	
	return 0;
}


#if 0
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

MARKET *mds_open2(const char *exnm, int flag)
{
	int         rtn;
	uint32_t    ymd, hms;
	MARKET      *m;
	MDARCH      *arch;
	XCHG        xchg;

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

	m->flag = flag;
	m->exid = xchg.exid;
	strcpy(m->exnm, xchg.exnm);
	memcpy(m->excode, xchg.excode, sizeof(m->excode));

	mds_procname(m->procname);          // 실행프로그램명

	strcpy(m->TZ, xchg.TZ);
	mds_timezone(m);
mds_time(m, 0, &ymd, &hms, NULL, NULL); // get local date & time

	// SET CTX
	strcpy(m->ctx.logf, xchg.logf);
	m->ctx.llog = xchg.llog;

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
	if (mds_shminit2(m, &xchg) != 0)
    {
		mds_log(m, LOG_MUST, "[%s] mds_shminit2 ERROR !! [%s:%.1s]", __func__, xchg.exnm, xchg.excode);
		return(-1);
	}

	if (m->whoami != I_AM_COOKER)   return(m);

	/*===============================================================
	* MDARCH, XCHG 메모리 초기화
	===============================================================*/
	arch = m->arch;                     // archive for shared memory
	memcpy(&(arch->xchg), &xchg, sizeof(XCHG)); // exchange information from 'cfg' to shared memory
	arch->tymd = ymd;

	/*===============================================================
	* 통화별 마스터메모리 초기화 (INDEX포함)
	===============================================================*/
	rtn = LoadMaster(m);
	if (rtn <= 0)
	{
		mds_log(m, LOG_MUST, "[%s] Master SHM Initialize ERROR !! [%s:%c] rtn=[%d]", __func__, arch->xchg.exnm, arch->xchg.excode[0], rtn);
		l_db2rollback();
		return(NULL);
	}

	mds_log(m, LOG_MUST, "[%s] Master SHM Initialize for [%s:%c] is DONE.. Load Count=[%d:%d] [%08d]", __func__,
	arch->xchg.exnm, arch->xchg.excode[0], arch->nrec, rtn, arch->tymd);

	return(m);
} /*  MARKET *mds_open2(const char *exnm, int flag) */
#endif
