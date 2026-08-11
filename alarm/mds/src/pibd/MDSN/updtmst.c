
#include "mds2.h"
#include <curses.h>

int printfold(MDFOLD *pfold)
{
	int z;

	if (pfold == NULL)		return (-1);
	z = pfold->zdiv;
	
	printf("-----------------------------------------------\n");
	printf("\tlog       = [%15.1s]\n", pfold->filler    );			// SISE LOG
	printf("\tsymb      = [%15s]\n", pfold->symb		);
	printf("\tseqn      = [%15d]\n", pfold->seqn		);			// MDFOLD OFFSET
	printf("\tkymd      = [%15d]\n", pfold->kymd		);			// last update date
	printf("\tkhms      = [%15d]\n", pfold->khms		);			// last update time
	printf("\tzdiv      = [%15d]\n", pfold->zdiv		);			// no of Employ decimals
	printf("\tzCustdiv  = [%15d]\n", pfold->zCustdiv	);			// no of decimals
	printf("\tquotalarm = [%15d]\n", pfold->quotalarm	);			// quotalarm
	printf("\tusdpos     =[%15d]\n", pfold->usdpos	    );			// 현재 가격원천
	printf("\ttymd      = [%15d]\n", pfold->tymd		);			// current trading day
	printf("\tspot      = [%15.8s]\n", pfold->spotdate	);			// spot date
	printf("\ttrdf      = [%15d]\n", pfold->trdf		);			// tradable flag
	printf("\tcustex    = [%15.1s]\n", pfold->cust.excode );		// 고객시세 제공 원천
	printf("\tfeedtp    = [%15.1s]\n", pfold->cust.feedtp );		// 고객시세 제공 타입 : TSKEIAM97.PRC_ORIGN_DSTCD 값 (중단/수기/우선/기본)
	printf("\tfinclstop = [%15.1s]\n", pfold->cust.finclstop);		// 고객재정시세 제공여부 : TSKEIAM97.FINCL_ANOUN_STOP_YN
	printf("\ttime      = [%8.8s.%6.6s] - [%8.8s.%6.6s] (stime/etime)\n", pfold->cust.stm, &pfold->cust.stm[8], pfold->cust.etm, &pfold->cust.etm[8]); // current trading day
	printf("\tbaseprc   = [%15.*f]\n\n", z, pfold->baseprc );
	printf("\tbidex     = [%15s]    offerex   = [%15s]\n",     pfold->bidex,	pfold->offerex	);
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
	
	fflush(stdout);
	
	return 0;
}

int main(int argc, char **argv)
{
	int		rtn;
	char	iexnm[32], iexcd[32], isymb[32];
	char	msg[512];
	MDFOLD	*pfold=NULL;

	if (argc != 3)
	{
		printf("%s exnm symb \n", argv[0]);
		return 0;
	}

	char 	item[32], ival[32];

	if (strcmp(argv[2], "mast") == 0)
	{
		MDARCH	*arch;

		arch = mds_getarch(1, argv[1], msg);
		printf("-----------------------------------------------\n");
		printf(" tymd    = [%08d] \n", arch->tymd);
		printf(" rsum    = [%d] \n", arch->rsum);
		printf(" nrec    = [%d] \n", arch->nrec);
		printf(" ma_pnum = [%d] \n", arch->xchg.ma_pnum);
		printf(" ap_pnum = [%d] \n", arch->xchg.ap_pnum);
		printf(" db_pnum = [%d] \n", arch->xchg.db_pnum);

		printf("\nitem (q:quit) : ");
		scanf("%s", item);
		if (item[0] == 'q' && strlen(item) == 1)		return 0;
		printf("value : ");
		scanf("%s", ival);

		l_rtrim(item);
		l_rtrim(ival);

		     if (strcmp(item, "tymd")      == 0) arch->tymd	= atoi(ival);
		else if (strcmp(item, "rsum")      == 0) arch->rsum	= atoi(ival);
		else printf("wrong input (%s)\n", ival);

		return 0;
	}

	pfold = mds_getfold(argv[1], argv[2], msg);
	if (pfold == NULL)
	{
		printf("can't find symbol.. (%s)\n", argv[2]);
		return 0;
	}

	while (1)
	{
		memset(item, 0x00, sizeof(item));
		memset(ival, 0x00, sizeof(ival));
		
		printfold(pfold);

		printf("\nitem (q:quit) : ");
		scanf("%s", item);
		if (item[0] == 'q' && strlen(item) == 1)		return 0;
		printf("value : ");
		scanf("%s", ival);

		l_rtrim(item);
		l_rtrim(ival);

		     if (strcmp(item, "symb")      == 0) memcpy(pfold->symb, ival, 6);
		else if (strcmp(item, "log")       == 0) pfold->filler[0]   = ival[0];
		else if (strcmp(item, "seqn")      == 0) pfold->seqn        = atoi(ival);
		else if (strcmp(item, "kymd")      == 0) pfold->kymd        = (uint32_t)atoi(ival);
		else if (strcmp(item, "khms")      == 0) pfold->khms        = (uint32_t)atoi(ival);
		else if (strcmp(item, "zdiv")      == 0) pfold->zdiv        = atoi(ival);
		else if (strcmp(item, "zCustdiv")  == 0) pfold->zCustdiv    = atoi(ival);
		else if (strcmp(item, "quotalarm") == 0) pfold->quotalarm   = (atoi(ival)==0?0:time(0));
		else if (strcmp(item, "usdpos")    == 0) pfold->usdpos      = atoi(ival);
		else if (strcmp(item, "custex")    == 0) pfold->cust.excode[0] = ival[0];
		else if (strcmp(item, "feedtp")    == 0) pfold->cust.feedtp[0] = ival[0];
		else if (strcmp(item, "finclstop") == 0) pfold->cust.finclstop[0] = ival[0];
		else if (strcmp(item, "trdf")      == 0) pfold->trdf        = atoi(ival);
		else if (strcmp(item, "stime")     == 0) memcpy(pfold->cust.stm, ival, strlen(ival)<14?strlen(ival):14);
		else if (strcmp(item, "etime")     == 0) memcpy(pfold->cust.etm, ival, strlen(ival)<14?strlen(ival):14);
		else if (strcmp(item, "tymd")      == 0) pfold->tymd        = (uint32_t)atoi(ival);
		else if (strcmp(item, "spot")      == 0) memcpy(pfold->spotdate, ival, strlen(ival)<8?strlen(ival):8);
		else if (strcmp(item, "baseprc")   == 0) pfold->baseprc     = atof(ival);
		else if (strcmp(item, "bidex")     == 0) pfold->bidex[0]    = ival[0];
		else if (strcmp(item, "bidopen")   == 0) pfold->bidopen     = atof(ival);
		else if (strcmp(item, "bidhigh")   == 0) pfold->bidhigh     = atof(ival);
		else if (strcmp(item, "bidlow")    == 0) pfold->bidlow      = atof(ival);
		else if (strcmp(item, "bidlast")   == 0) pfold->bidlast     = atof(ival);
		else if (strcmp(item, "bidbest")   == 0) pfold->bidbest     = atof(ival);
		else if (strcmp(item, "prebid")    == 0) pfold->prebid      = atof(ival);
		else if (strcmp(item, "bidvol")    == 0) pfold->bidvol      = atof(ival);
		else if (strcmp(item, "bidbestvol")== 0) pfold->bidbestvol  = atof(ival);
		else if (strcmp(item, "bidsign")   == 0) pfold->bidsign     = atoi(ival);
		else if (strcmp(item, "biddiff")   == 0) pfold->biddiff     = atof(ival);
		else if (strcmp(item, "bidrate")   == 0) pfold->bidrate     = atof(ival);
		else if (strcmp(item, "biddirf")   == 0) pfold->biddirf     = atoi(ival);
		else if (strcmp(item, "offerex")   == 0) pfold->offerex[0]  = ival[0];
		else if (strcmp(item, "offeropen") == 0) pfold->offeropen   = atof(ival);
		else if (strcmp(item, "offerhigh") == 0) pfold->offerhigh   = atof(ival);
		else if (strcmp(item, "offerlow")  == 0) pfold->offerlow    = atof(ival);
		else if (strcmp(item, "offerlast") == 0) pfold->offerlast   = atof(ival);
		else if (strcmp(item, "offerbest") == 0) pfold->offerbest   = atof(ival);
		else if (strcmp(item, "preoffer")  == 0) pfold->preoffer    = atof(ival);
		else if (strcmp(item, "offervol")  == 0) pfold->offervol    = atof(ival);
		else if (strcmp(item, "offerbestv")== 0) pfold->offerbestvol= atof(ival);
		else if (strcmp(item, "offersign") == 0) pfold->offersign   = atoi(ival);
		else if (strcmp(item, "offerdiff") == 0) pfold->offerdiff   = atof(ival);
		else if (strcmp(item, "offerrate") == 0) pfold->offerrate   = atof(ival);
		else if (strcmp(item, "offerdirf") == 0) pfold->offerdirf   = atoi(ival);
		else if (strcmp(item, "midopen")   == 0) pfold->midopen     = atof(ival);
		else if (strcmp(item, "midhigh")   == 0) pfold->midhigh     = atof(ival);
		else if (strcmp(item, "midlow")    == 0) pfold->midlow      = atof(ival);
		else if (strcmp(item, "midlast")   == 0) pfold->midlast     = atof(ival);
		else if (strcmp(item, "midbest")   == 0) pfold->midbest     = atof(ival);
		else if (strcmp(item, "midvol")    == 0) pfold->midvol      = atof(ival);
		else if (strcmp(item, "midsign")   == 0) pfold->midsign     = atoi(ival);
		else if (strcmp(item, "middiff")   == 0) pfold->middiff     = atof(ival);
		else if (strcmp(item, "midrate")   == 0) pfold->midrate     = atof(ival);
		else if (strcmp(item, "middirf")   == 0) pfold->middirf     = atoi(ival);
		else printf("wrong input (%s)\n", ival);
		
		printf("enter to see new value.. \n");

		getchar();
	}
	return 0;
}
