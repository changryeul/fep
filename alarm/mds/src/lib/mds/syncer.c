//
// Databse UPSERT control
//
#include "context.h"


//
// mds_syncer()
// Insert/Update data base row
//
void *mds_syncer(void *argv)
{
	MARKET	*market = argv;
	MDCTX	*ctx  = market->ctx;			// context for market
	MDARCH	*arch = market->arch;			// archive of folder
	INDEX	*indx = market->indx;
	FOLDER	*folder;
	void	*row;
	uint32_t xymd, xhms, hhmm;
	int		from, to;
	int	ii;

	mds_log(market, LOG_MUST, "The syncer thread has been started.");
	while (1)
	{
		from = market->xchg->trading.from;
		to   = market->xchg->trading.to;
		mds_time(market, 0, &xymd, &xhms, NULL, NULL);
		if (mds_holiday(market, xymd, 1) || ctx->func->seek == NULL)
		{
			// holiday
			sleep(10*60);
			continue;
		}

		//mds_log(market, LOG_DEBUG, "[%4d] syncer..  ", __LINE__);
		hhmm = xhms / 100;
		if (from < to)
		{
			if (!(hhmm >= from && hhmm <= to))
			{
				// no trading time
				sleep(60);
				continue;
			}
		}

		if (to < from)
		{
			if ((hhmm >= to && hhmm <= from))
			{
				// no trading time
				sleep(60);
				continue;
			}
		}

		//mds_log(market, LOG_DEBUG, "[%4d] syncer..  ", __LINE__);
		for (ii = 0; ii < arch->nrec; ii++)
		{
			mds_sleep(1000);			// 1/ 1000 seconds

			folder = mds_shmfold(market, &indx[ii]);
			if (folder == NULL || folder->sync == 0)
				continue;
			//mds_log(market, LOG_DEBUG, "syncer..  [%s] ", folder->symb);
			if (folder->sync & (1 << MSTR))
			{
				if ((row = ctx->func->roff(market, folder, MSTR)) != NULL)
					mds_isupsert(market, MSTR, (char *)row);
				folder->sync &= ~(1 << MSTR);
			}
			if (folder->sync & (1 << QUOT))
			{
				if ((row = ctx->func->roff(market, folder, QUOT)) != NULL)
					mds_isupsert(market, QUOT, (char *)row);
				folder->sync &= ~(1 << QUOT);
			}
			if (folder->sync & (1 << BOOK))
			{
				if ((row = ctx->func->roff(market, folder, BOOK)) != NULL)
					mds_isupsert(market, BOOK, (char *)row);
				folder->sync &= ~(1 << BOOK);
			}
			if (folder->sync & (1 << INTR))
			{
				if ((row = ctx->func->roff(market, folder, INTR)) != NULL)
					mds_isupsert(market, INTR, (char *)row);
				folder->sync &= ~(1 << INTR);
			}
		}
		sleep(3);
	}
}

//
// Databse UPSERT control
//
#include "context.h"

//
// mds_upsert()
// Insert/Update data base row
//
int mds_upsert(MARKET *market, void *fptr, int dbid, const void *row)
{
	MDCTX	*ctx = market->ctx;
	FOLDER	*folder = fptr;

	if (!SELECT_IS(market->dbis, dbid))
		return(0);

	switch (market->whoami)
	{
	case I_AM_COOKER:
		if (folder != NULL && ctx->isam[dbid].dodo & _DF_)
		{
			switch (dbid)
			{
			case MSTR:
			case QUOT:
			case BOOK:
				folder->sync |= (1 << dbid);
				return(0);
			default:
				break;
			}
		}
	default:
		return(mds_isupsert(market, dbid, (char *)row));
	}
}
