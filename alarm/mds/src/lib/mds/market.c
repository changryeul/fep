#include "context.h"

#define	MAX_POOL	16

static 	struct pool {
	MARKET	*market;
	void	(*close)(MARKET *);
	time_t	when;
} pool[MAX_POOL];

static int n_pool;
static int cmpool();

//
// market_pop()
//
MARKET *market_pop(const char *exnm)
{
	MARKET	*market;
	int	ii;

	for (ii = 0; ii < n_pool; ii++)
	{
		if (strcmp(pool[ii].market->exnm, exnm) == 0)
		{
			pool[ii].when = time(0);
			market = pool[ii].market;
			// 20170810
			putenv(market->TZ);
			tzset();
			//////////////////
			qsort(pool, n_pool, sizeof(struct pool), cmpool);
			return(market);
		}
	}
	return(NULL);
}

void market_push(MARKET *market, void (*close)(MARKET *))
{
	if (n_pool >= MAX_POOL)
		pool[MAX_POOL-1].close(pool[MAX_POOL-1].market);

	if (n_pool < MAX_POOL)
	{
		pool[n_pool].market = market;
		pool[n_pool].close  = close;
		pool[n_pool].when = time(0);
		n_pool++;
		qsort(pool, n_pool, sizeof(struct pool), cmpool);
	}
}

//
// market_del()
//
void market_del(const char *exnm)
{
	int	ii;

	for (ii = 0; ii < n_pool; ii++)
	{
		if (strcmp(pool[ii].market->exnm, exnm) == 0)
		{
			memmove(&pool[ii], &pool[ii+1], (MAX_POOL-ii-1)*sizeof(struct pool));
			n_pool--;
			break;
		}
	}
}

static int cmpool(struct pool *p1, struct pool *p2)
{
	if (p1->when > p2->when)
		return(-1);
	if (p1->when < p2->when)
		return(1);
	return(0);
}
