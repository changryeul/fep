#include "context.h"
#include "mdfold.h"
#include "stream.h"

//
// mds_open()
// Open exchange
//
MARKET *mds_open(const char *exnm, int flag)
{
	MARKET	*market;
	MDCTX	*ctx;
	int		retval;
	int		b = 0x12345678;
	char	*c = (char *)&b;
	int		f;
	int (*init)(MARKET *market);
	init = NULL;
	switch (flag & O_ACCMODE)
	{
	case O_RDONLY:
		market = market_pop(exnm);
		if (market != NULL)
			return(market);
	case O_RDWR:
	case O_WRONLY:	
		break;
	default:
		errno = EINVAL;
		fprintf(stderr, "Falg Error.\n");
		return(NULL);
	}
	mds_setenv();
	if ((market = mds_market_alloc()) == NULL)
	{
		PRINTF("Market alloc error",1);
		fprintf(stderr, "Market alloc error.\n");
		return(NULL);
	}
	if (mds_exchange(exnm, market->xchg) != 0)
	{
		errno = ENOENT;
		fprintf(stderr, "Noexchange for");
		PRINTF("No exchange for '%s'", exnm);
		free(market);
		return(NULL);
	}
	// Print message for debugging may be written to a cisam file sometime
	// Set file descriptor 0, 1, 2 to /dev/null
	for (f = 0; f <= 2 && f >= 0;)
	{
		f = open("/dev/null", O_WRONLY);
		if (f > 2)
		{
			close(f);
			break;
		}
	}
	
	ctx = market->ctx;
	market->flag = flag;
	mds_procname(market->procname);
	market->exid = market->xchg->exid;
	strcpy(market->exnm, market->xchg->exnm);
	strcpy(market->TZ, market->xchg->TZ);
	market->type = market->xchg->type;
	strcpy(market->dirp, market->xchg->dirp);
	market->trading.from = market->xchg->trading.from;
	market->trading.to   = market->xchg->trading.to;
	market->batch.open   = market->xchg->batch.open;
	market->batch.close  = market->xchg->batch.close;

	strcpy(market->logf, market->xchg->logf);
	market->llog = market->xchg->llog;
	market->whoami = I_AM_READER;
	if (flag & O_CREAT)
		market->whoami = I_AM_COOKER;
	else
	{
		switch (flag & O_ACCMODE)
		{
		case O_RDONLY: market->whoami = I_AM_READER; break;
		case O_RDWR:   market->whoami = I_AM_WRITER; break;
		case O_WRONLY: market->whoami = I_AM_WRITER; break;
		}
	}
	mds_timezone(market);

	retval = mds_init(market);
	if (retval != 0)
	{
		errno = EINVAL;
		fprintf(stderr, "Initialization Fail");
		PRINTF("The market '%s' initialization failIure\n", exnm);
		free(market);
		return(NULL);
	}
	if (c[0] == 0x12)
		market->endian = B_ENDIAN;
	else
		market->endian = L_ENDIAN;

	switch (market->whoami)
	{
	case I_AM_COOKER:
		mkdir(market->dirp, 0755);
		if (ctx->schema != NULL && mds_isinit(market) != 0)	
		{
			fprintf(stderr, "Cannot initialize CISAM files.");
			PRINTF("Cannot initialize CISAM files.", 0);
			if (market->whoami == I_AM_COOKER)
				mds_log(market, LOG_ERROR, "The market %s cannot initialize CISAM files.", exnm);
			free(market);
			return(NULL);
		}
		break;
	default:
		break;
	}
	if (ctx->schema != NULL && mds_isopen(market) != 0)			// open cisam on schema
	{
		errno = EBADF;
		fprintf(stderr, "Cnnot open CISAM file interface...");
		PRINTF("Cnnot open CISAM file interface...", 0);
		if (market->whoami == I_AM_COOKER)
			mds_log(market, LOG_ERROR, "The market %s cannot open CISAM file interface...", exnm);
		free(market);
		return(NULL);
	}
	if (ctx->func->init(market) != 0)
	{
		if (market->whoami == I_AM_COOKER)
			mds_log(market, LOG_ERROR, "The market %s cannot initialize shared memory !!!", exnm);
		free(market);
		return(NULL);
	}
	switch (market->whoami)
	{
	case I_AM_COOKER:
		mds_calendar(market);		// initialize trading day & holiday to shared memory
		mds_symbinit(market);
		break;
	case I_AM_READER:
		market_push(market, mds_close);
		break;
	default:
		break;
	}
	return(market);
}

//
// Open trading day
//
void mds_market_open(MARKET *market)
{
	mds_isrenewal(market);
}

//
// Closing market
//
void mds_market_close(MARKET *market)
{
	MDARCH	*arch = market->arch;
	MDCTX	*ctx  = market->ctx;
	INDEX	*indx = market->indx;
	FOLDER	*folder;
	//MDTRAD	*mdtrad;
	int	ii;
	/*
	if (ctx->func->seek == NULL)
		return;
	for (ii = 0; ii < arch->nrec; ii++)
	{
		if ((folder = mds_shmfold(market, &indx[ii])) == NULL)
			continue;
		mdintr = ctx->func->roff(market, folder, INTR);
		if (mdintr == NULL)
			continue;
		if (mdintr->xymd == 0 || mdintr->flush)
			continue;
		mdtrad->nstart = 0;
		mdtrad->nEnd = 0;
		mds_isupsert(market, INTR, mdintr);
	}*/
}

