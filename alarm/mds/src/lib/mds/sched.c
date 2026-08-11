//
// sched.c
// Scheduler
//
#include "context.h"

#define	_OPEN_	0x001
#define	_CLOS_	0x002

static void scheduling(MARKET *market);
static void clracct(MARKET *market, int hour, int caller);
static void openmarket(MARKET *market);
static void closemarket(MARKET *market);
static void opennewday(MARKET *market);

static	time_t	t_open = 0;			// timestamp to open
static	time_t	t_close = 0;			// timestamp to close
static	int	done = 0;			// done batch job ?

//
// scheduler()
// Scheduler....
//
void *scheduler(void *argv)
{
	MARKET	*market= argv;
	MDCTX	*ctx   = market->ctx;
	uint32_t yday, cday, chms, khms, check, hour;
	time_t	clock;
	uint32_t	pday, kymd;
	int wday;

	scheduling(market);
	mds_time(market, 0, &yday, NULL, &pday, NULL);
	mds_log(market, LOG_MUST, "The scheduler has been started...");
	while (1)
	{
		sleep(20);
		clock = time(0);
		mds_time(market, clock, &cday, &chms, &kymd, &khms);
		check = mds_chckday(market, cday);
		if (check == TRADDAY)
		{
			// market opening ?
			if (!(done & _OPEN_) && t_open != 0 && clock >= t_open)
			{
				mds_log(market, LOG_MUST, "The market is opening");
	
				/////////////////////////////////////
				pthread_mutex_lock(&ctx->mutex);
				if (ctx->proc.open != NULL)
					(*ctx->proc.open)(market);
				else
					openmarket(market);
				pthread_mutex_unlock(&ctx->mutex);
				/////////////////////////////////////
				done |= _OPEN_;
				mds_log(market, LOG_MUST, "The market has been opened...");
			}

			// market closing ?
			if (!(done & _CLOS_) && t_close != 0 && clock >= t_close)
			{
				mds_log(market, LOG_MUST, "The market is closing");
				/////////////////////////////////////
				pthread_mutex_lock(&ctx->mutex);
				if (ctx->proc.clos != NULL)
					(*ctx->proc.clos)(market);
				else
					closemarket(market);
				pthread_mutex_unlock(&ctx->mutex);
				/////////////////////////////////////
				done |= _CLOS_;
				mds_log(market, LOG_MUST, "The market has been closed...");
			}
		}
		if (cday != yday)
		{
			// Change the calendar day. 
			mds_log(market, LOG_MUST, "Changed date %d->%d!!!!", yday, cday);
			yday = cday;
			
			/////////////////////////////////////
			pthread_mutex_lock(&ctx->mutex);
			clracct(market, 0, 1);
			if (ctx->proc.xday != NULL)
				(*ctx->proc.xday)(market);
			else
				opennewday(market);
			mds_calendar(market);
			scheduling(market);
			pthread_mutex_unlock(&ctx->mutex);
			/////////////////////////////////////

			mds_log(market, LOG_MUST, "Ready to processing for a new day %d!!!!",cday);
		}
	
	  if (pday != kymd)
		{
		  sleep(20);
			wday = mds_day4week(kymd);
			int retval =0;
			if(wday != 0)
			{
			  InitMarketDate(market);
			  retval = mds_init(market);
        if (retval != 0)
        {
				  mds_log(market, LOG_MUST, "The market initialization failIure\n");        
          l_db2rollback();
        }
        l_db2commit(); 
      }
			mds_log(market, LOG_MUST, "InitMarketDate for new day %d  , %d!!!!",kymd, wday);
			pday = kymd;
		}
		// clear account for the next hour
		hour = HOUR(khms); 
		clracct(market, hour, 0);
	}
}

static void clracct(MARKET *market, int hour, int caller)
{
	MDCTX	*ctx   = market->ctx;
	MDINFO	*info  = ctx->info;
	static	int next = -1;
	int	ii;

	switch (caller)
	{
	case 0: // hourly 
		hour++;
		if (hour >= 24)
			hour = 0;
		if (next == hour)
			return;
		next = hour;
		if (info == NULL)
			return;

		info->acct.recv[hour] = 0;
		info->acct.lost[hour] = 0;
		for (ii = 0; ii < MAX_PORT; ii++)
			info->acct.port[ii].recv = 0;
		break;
	case 1: // daily
		for (ii = 0; ii < MAX_PORT; ii++)
			info->acct.port[ii].rsum = 0;
		break;
	}	
}

static void scheduling(MARKET *market)
{
	time_t	clock1, clock2;
	struct	tm tm, tx;
	int	hhmm, hm, hh, mm;
	uint32_t xymd, xhms;
	int	ii;

	t_open  = 0;
	t_close = 0;
	done = 0;

	//mds_timezone(market);
	clock1 = time(0);
	localtime_r(&clock1, &tx);
	hm = tm.tm_hour * 100 + tm.tm_min;

	for (ii = 0; ii < 2; ii++)
	{
		hhmm = 0;
		switch (ii)
		{
		case 0: // market opening time
			hhmm = market->batch.open;
			break;
		case 1: // market closing time
			hhmm = market->batch.close;
			break;
		}
		if (hhmm < 0)
			continue;

		hh = hhmm / 100;
		mm = hhmm % 100;
		memset(&tm, 0, sizeof(struct tm));
		memcpy(&tm, &tx, sizeof(struct tm));
		tm.tm_hour = hh;
		tm.tm_min  = mm;
		tm.tm_sec  = 0;
		clock2 = mktime(&tm);
		if (clock2 <= clock1)
			clock2 += (24*60*60);

		do 
		{
			mds_time(market, clock2, &xymd, &xhms, NULL, NULL);
			if (!mds_holiday(market, xymd, 0))
				break;
			clock2 += (24*60*60);
	
		} while (1);
		switch (ii)
		{
		case 0: t_open  = clock2;     break;
		case 1: t_close = clock2;     break;
		}
	}
}

static int checkbin(const char *program)
{
	struct	stat bstat;
	char	path[256];

	sprintf(path, "%s/%s", BIN_DIR, program);
	if (stat(path, &bstat) != 0)
		return(-1);
	return(0);
}

//
// default open market procedure
//
static void openmarket(MARKET *market)
{
	char	program[80];
	char	command[256];
	uint32_t lymd;

	mds_time(market, 0, &lymd, NULL, NULL, NULL);
	if (mds_holiday(market, lymd, 0))
		return;
	mds_log(market, LOG_MUST, "The market open batch job starting");
	sprintf(program, "mdsopen");
	if (checkbin(program) == 0)
	{
		sprintf(command, "%s -e %s -d %d", program, market->exnm, lymd);
		mds_exec(market, command, 1);
	}
	else
	{
		mds_log(market, LOG_MUST, "Can not find market open batch job program!!! (%s)", program);
		return; 
	}
	mds_log(market, LOG_MUST, "The market open batch job completed");
}

//
// default close market procedure
//
static void closemarket(MARKET *market)
{
	char	program[80];
	char	command[256];
	uint32_t lymd, lhms;

	mds_time(market, 0, &lymd, &lhms, NULL, NULL);
	if (mds_holiday(market, lymd, 0))
		return;
	sprintf(program, "close%s", market->xchg->exnm);
	if (checkbin(program) == 0)
	{
		sprintf(command, "%s -e %s -d %d", program, market->exnm, lymd);
		mds_exec(market, command, 1);
	}
	else
	{
		mds_log(market, LOG_MUST, "Can not find market close batch job program!!! (%s)", program);
		return; 
	}
	mds_log(market, LOG_MUST, "The market closing batch job completed");
}

//
// cbange date
//
static void opennewday(MARKET *market)
{
	char	program[80];
	char	command[256];
	
	sprintf(program, "xday%s", market->xchg->exnm);
	if (checkbin(program) == 0)
	{
		sprintf(command, "%s -e %s", program, market->exnm);
		mds_log(market, LOG_MUST, "Start batch for a new trading day !!!!");
		mds_exec(market, command, 0);
	}
}
