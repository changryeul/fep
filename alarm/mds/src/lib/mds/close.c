#include "context.h"


static char TZ[40];
//
// mds_close()
// Open exchange
//
void mds_close(MARKET *market)
{
	MDCTX	*ctx = market->ctx;
	pthread_t this;
	struct	msqid_ds msqid_ds;
	int	ii;

	if (market == NULL)
		return;
	market_del(market->exnm);

	mds_lock(market);
	switch (market->whoami)
	{
	case I_AM_COOKER:
		this = pthread_self();
		if (ctx->scheduler != (pthread_t)0 && pthread_kill(ctx->scheduler, 0) == 0 && !pthread_equal(this, ctx->scheduler))
			pthread_cancel(ctx->scheduler);
		for (ii = 0; ii < MAX_PORT; ii++)
		{
			if (ctx->dispatcher[ii] != (pthread_t)0 && pthread_kill(ctx->dispatcher[ii], 0) == 0 && !pthread_equal(this, ctx->dispatcher[ii]))
				pthread_cancel(ctx->dispatcher[ii]);
		}
	
		for (ii = 0; ii < MAX_PORT; ii++)
		{
			if (ctx->recver[ii] != (pthread_t)0 && pthread_kill(ctx->recver[ii], 0) == 0 && !pthread_equal(this, ctx->recver[ii]))
				pthread_cancel(ctx->recver[ii]);
		}
		for (ii = 0; ii < MAX_PORT; ii++)
		{
			if (ctx->notifier[ii] != (pthread_t)0 && pthread_kill(ctx->notifier[ii], 0) == 0 && !pthread_equal(this, ctx->notifier[ii]))
				pthread_cancel(ctx->notifier[ii]);
		}
		for (ii = 0; ii < market->xchg->nofp; ii++)
		{
			if (ctx->sock[ii] >= 0)
				close(ctx->sock[ii]);
		}
		for (ii = 0; ii < MAX_PORT; ii++)
		{
			if (ctx->mqid[ii] >= 0 && msgctl(ctx->mqid[ii], IPC_STAT, &msqid_ds) == 0)
				msgctl(ctx->mqid[ii], IPC_RMID, &msqid_ds);
		}
		if (ctx->syncer != (pthread_t)0 && pthread_kill(ctx->syncer, 0) == 0)
			pthread_cancel(ctx->syncer);
		break;
	default:
		break;
	}
	mds_isclose(market);
	if (market->arch != NULL)
		shmdt(market->arch);
	mds_unlock(market);
	//free(market);
	sprintf(TZ, TZ_KST);
	putenv(TZ);
	tzset();
	free(market);
}
