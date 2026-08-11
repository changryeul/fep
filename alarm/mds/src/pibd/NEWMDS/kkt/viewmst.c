
#include "mds2.h"
#include <curses.h>

int printall(char *exnm)
{
	int		ii, con=0;
	char	msg[128], ctime[16], stemp[128], tmchk[16], tmchk2[16];
	time_t	clock;
	struct	tm *tm;
	MDFOLD	*pfold=NULL, *p;
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
printf("\n");
	pfold = (MDFOLD *)((char *)arch + sizeof(MDARCH) + (sizeof(INDEX)*arch->xchg.maxcnt));
	printf("[%s] [%08d] Rec [%d] Receive [%d] Curr [%02d:%02d:%02d] Custflag [%c] \n", 
			arch->xchg.exnm, arch->tymd, arch->nrec, arch->rsum, tm->tm_hour, tm->tm_min, tm->tm_sec, arch->xchg.custflag);

	p = pfold;

if (strcmp(exnm, "CUST") == 0) {
printf("------------------------------------------------------------------------------------------------------\n");
printf(" PRDCD       STAT   SPOTDATE  EX:TYPE:FICL   STIME   ETIME   LAST TM   EX:BID          EX:ASK   \n");
printf("------------------------------------------------------------------------------------------------------\n");
} else {
printf("---------------------------------------------------------------------\n");
printf(" PRDCD       STAT   SPOTDATE    LAST TM   EX:BID          EX:ASK   \n");
printf("---------------------------------------------------------------------\n");
//      [AUDUSD:00] [가능] [20240408]  [18:00.19] [E/    0.65956] [E/    0.65962] 
}

	for (ii=0; ii<arch->nrec; ii++, p++)
	{
		memset(tmchk, 0x00, sizeof(tmchk));
		memset(tmchk2,0x00, sizeof(tmchk2));
		memset(stemp, 0x00, sizeof(stemp));
		
		if (memcmp(ctime, p->cust.stm, sizeof(p->cust.stm)) < 0 || memcmp(ctime, p->cust.etm, sizeof(p->cust.etm)) > 0)
			strcpy(tmchk,  "\033[31m");
		if (memcmp(p->cust.stm, p->cust.etm, 8) < 0)
			strcpy(tmchk2, "\033[33m");
		
		if (strcmp(exnm, "CUST") == 0) {
			sprintf(stemp, "[%1.1s:%s:%s]"
				,  p->cust.excode
				, (p->cust.feedtp[0]=='1'?"기본":(p->cust.feedtp[0]=='2'?"\033[35m수기\033[0m":(p->cust.feedtp[0]=='3'?"\033[31m중단\033[0m":(p->cust.feedtp[0]=='4'?"\033[36m우선\033[0m":"?"))))
				, (p->cust.finclstop[0]=='0'?"의존":"\033[31m중단\033[0m"));
			sprintf(&stemp[strlen(stemp)], " [%s%2.2s:%2.2s ~ %s%2.2s:%2.2s\033[0m]", tmchk
				, &(p->cust.stm[8]), &(p->cust.stm[10]), tmchk2
				, &(p->cust.etm[8]), &(p->cust.etm[10]));
		}

		printf("[%.6s:%02d] [%s] [%.8s] %s [%02u:%02u.%02u] [%1.1s/%11.5f] [%1.1s/%11.5f] [%d]\n"
				,p->symb
				,p->seqn
				,(p->trdf==9?"\033[36m제외\033[0m":(p->trdf==0?"대기":"\033[33m가능\033[0m"))
				,p->spotdate
				,stemp
				,p->khms/10000000, (p->khms/100000)%100, (p->khms/1000)%100
				,p->bidex
				,p->bidlast
				,p->offerex
				,p->offerlast, p->quotalarm);
	}

	fflush(stdout);

	return 0;
}

int printfold(MDFOLD *pfold, char *exnm)
{
	int z, ii;

	if (pfold == NULL)		return (-1);
	z = pfold->zdiv;
	
	printf("-----------------------------------------------\n");
	printf("\tsymb      = [%15s]\n", pfold->symb		);
	printf("\tseqn      = [%15d]\n", pfold->seqn		);			// MDFOLD OFFSET
	printf("\tkymd      = [%15d]\n", pfold->kymd		);			// last update date
	printf("\tkhms      = [%15d]\n", pfold->khms		);			// last update time
	printf("\tzdiv      = [%15d]\n", pfold->zdiv		);			// no of Employ decimals
	printf("\tzCustdiv  = [%15d]\n", pfold->zCustdiv	);			// no of decimals
	printf("\tquotalarm = [%15d]\n", pfold->quotalarm	);			// quote alarm
	printf("\tusdpos     =[%15d]\n", pfold->usdpos	    );			// 현재 가격원천
	printf("\ttymd      = [%15d]\n", pfold->tymd		);			// current trading day
	printf("\tspot      = [%15.8s]\n", pfold->spotdate	);			// spot date
	printf("\ttrdf      = [%15d]\n", pfold->trdf		);			// tradable flag
	printf("\tcustex    = [%15.1s]\n", pfold->cust.excode );		// 고객시세 제공 원천
	printf("\tfeedtp    = [%15.1s]\n", pfold->cust.feedtp );		// 고객시세 제공 타입 : TSKEIAM97.PRC_ORIGN_DSTCD 값 (중단/수기/우선/기본)
	printf("\tfinclstop = [%15.1s]\n", pfold->cust.finclstop);		// 고객재정시세 제공여부 : TSKEIAM97.FINCL_ANOUN_STOP_YN
	printf("\ttime      = [%8.8s.%6.6s] - [%8.8s.%6.6s]\n", pfold->cust.stm, &pfold->cust.stm[8], pfold->cust.etm, &pfold->cust.etm[8]);			// current trading day
	printf("\tbaseprc   = [%15.*f]\n\n", z, pfold->baseprc );
	printf("\tbidex     = [%15.1s]    offerex   = [%15.1s]\n",     pfold->bidex,	pfold->offerex	);
	printf("\tbidlast   = [%15.*f]    offerlast = [%15.*f]    midlast = [%15.*f]\n\n", z, pfold->bidlast,z, pfold->offerlast, z, pfold->midlast	);
	printf("\tbidopen   = [%15.*f]    offeropen = [%15.*f]    midopen = [%15.*f]\n", z, pfold->bidopen,z, pfold->offeropen, z, pfold->midopen	);
	printf("\tbidhigh   = [%15.*f]    offerhigh = [%15.*f]    midhigh = [%15.*f]\n", z, pfold->bidhigh,z, pfold->offerhigh, z, pfold->midhigh	);
	printf("\tbidlow    = [%15.*f]    offerlow  = [%15.*f]    midlow  = [%15.*f]\n", z, pfold->bidlow ,z, pfold->offerlow , z, pfold->midlow	);
	printf("\tbidbest   = [%15.*f]    offerbest = [%15.*f]    midbest = [%15.*f]\n", z, pfold->bidbest,z, pfold->offerbest, z, pfold->midbest	);
	printf("\tprebid    = [%15.*f]    preoffer  = [%15.*f]\n", z, pfold->prebid,z, pfold->preoffer	);
	printf("\tbidvol    = [%15.2f]    offervol  = [%15.2f]    midvol  = [%15.2f]\n", pfold->bidvol,	 pfold->offervol	);
	printf("\tbidbestvol= [%15.2f]    offerbestv= [%15.2f]\n", pfold->bidbestvol, pfold->offerbestvol );
	printf("\tbidsign   = [%15d]    offersign = [%15d]    midsign = [%15d]\n",   pfold->bidsign, pfold->offersign,pfold->midsign);
	printf("\tbiddiff   = [%15.*f]    offerdiff = [%15.*f]    middiff = [%15.*f]\n", z, pfold->biddiff, z, pfold->offerdiff,z, pfold->middiff);
	printf("\tbidrate   = [%15.2f]    offerrate = [%15.2f]    midrate = [%15.2f]\n", pfold->bidrate, pfold->offerrate,pfold->midrate);
	printf("\tbiddirf   = [%15d]    offerdirf = [%15d]    middirf = [%15d]\n",   pfold->biddirf, pfold->offerdirf,pfold->middirf);
	printf("-----------------------------------------------\n");

	if (exnm[0] == 'E' | exnm[0] == 'Z')
	{	
		for (ii = 0; ii < 5; ii++)
		{
			printf("\t[%d]bidsign[%d]		asksign[%d]\n", ii, pfold->book[ii].bidsign, pfold->book[ii].asksign);
			printf("\t[%d]bidprc [%15.*f]	askprc [%15.*f]\n", ii, z, pfold->book[ii].bidprc, z, pfold->book[ii].askprc);
			printf("\t[%d]bidqty [%15.2f]	askqty [%15.2f]\n", ii, z, pfold->book[ii].bidqty, z, pfold->book[ii].askqty);
		}
	}
	
	fflush(stdout);
	
	return 0;
}

int main(int argc, char **argv)
{
	int		rtn;
	char	iexnm[32], iexcd[32], isymb[32];
	char	msg[512];
	MDFOLD	*pfold=NULL;

	if (argc == 2)
	{
		printall(argv[1]);
		return 0;
	}
	else if (argc == 3)
	{
		if ((pfold = mds_getfold(argv[1], argv[2], msg)) != NULL)
			printfold(pfold, argv[1]);
		
		return 0;
	}

	while (1)
	{
		memset(msg,   0x00, sizeof(msg));
		memset(iexcd, 0x00, sizeof(iexcd));
		memset(iexnm, 0x00, sizeof(iexnm));
		memset(isymb, 0x00, sizeof(isymb));
		
		printf("\nEnter EXNAME('S'MBS/'K'MBS/'E'BS/'C'MBS/'B'EST/'Z'CUST/'q'uit) : ");
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

		pfold = mds_getfold(iexnm, isymb, msg);
		printf("%s\n", msg);
		printf("===================================================================\n");
		if (pfold==NULL)
		{
			printf("\n code not found.. \n");
			continue;
		}
		
		printfold(pfold, iexnm);

		getchar();
	}

	return 0;
}
