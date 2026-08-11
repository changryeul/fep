//
// recv.c
// Receive market data from foreign
//
#include "context.h"
#include "stream.h"

//
// _mds_recv()
// Receive stream message via socket
//
static int _mds_recv(MARKET *market, int seqn, TOKEN *token, char *buf, int size, int timeout)
{
	MDCTX	*ctx = market->ctx;
	char	rcvb[MAX_PACKET_SIZE+512];
	struct	msgbuf *msgbuf= (struct msgbuf *)rcvb;
	TOKEN	*_token;
	int	hhmm;
	int	rcvl;

	rcvl = msgrcv(ctx->mqid[seqn], msgbuf, MAX_PACKET_SIZE, 0, MSG_NOERROR);
	if (rcvl <= sizeof(TOKEN))
		return(0);
	_token = (TOKEN *)msgbuf->mtext;
	rcvl -= sizeof(TOKEN);
	if (rcvl > size)
		rcvl = size;

	token->port = _token->port;
	strcpy(token->name, _token->name);
	token->seqn = _token->seqn;
	mds_time(market, token->timeval.tv_sec, &token->xymd, &token->xhms, &token->kymd, &token->khms);
	token->msec = token->timeval.tv_usec / 1000;

	token->rcvl = rcvl;
	token->rcvb = buf;
	token->tymd = token->xymd;
	memcpy(buf, &msgbuf->mtext[sizeof(TOKEN)], rcvl);

#if 0
	if (!market->xchg->trading.h24t)
	{
		int from = market->xchg->trading.from;
		int to   = market->xchg->trading.to;
		if (mds_holiday(market, token->xymd, 1))
			return(0);
		hhmm = token->xhms / 100;
		if (from < to)
			if (!(hhmm >= from && hhmm <= to))
				return(0);
		if (from > to)
			if (hhmm >= to && hhmm <= from)
				return(0);
	}
#endif
	return(rcvl);
}

//
// mds_recv()
// Receive stream message via socket
//
int mds_recv(MARKET *market, TOKEN *token, char *buf, int size, int timeout)
{
	MDCTX	*ctx  = market->ctx;

	if (market->whoami != I_AM_COOKER)
	{
		errno = EPERM;			// sorry no cooker !!!
		return(-1);
	}
	if (ctx->proc.recv != NULL)		// sorry !!! you must using mds_notify()'s function
		return(0);
	return(_mds_recv(market, 0, token, buf, size, timeout));
}

void *notifier(void *argv)
{
	struct	argval *argval = argv;
	MARKET	*market = argval->market;
	MDCTX	*ctx = market->ctx;
	int	seqn = argval->seqn;
	TOKEN	token = {0,};
	char	msgb[MAX_PACKET_SIZE+512];
	int	msgl;

	mds_log(market, LOG_MUST, "The receive thread is ready !!!!");
	while (1)
	{
		msgl = _mds_recv(market, seqn, &token, msgb, sizeof(msgb), 0);
		if (msgl < 0)
			break;
		if (msgl > 0)
		{
			token.rcvb = msgb;
			token.rcvl = msgl;
			pthread_mutex_lock(&ctx->mutex);
			ctx->proc.recv(market, &token);
			pthread_mutex_unlock(&ctx->mutex);
		}
	}
	pthread_exit(NULL);
}
