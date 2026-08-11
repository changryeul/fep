#include "context.h"
#include "stream.h"

#define	xHOUR(x)	(((x%(24*60*60)) / (60*60)) + 9) % 24

//
// udprcv
// Receive market data from UDP
//
void *udprcv(void *argv)
{
	struct	argval *argval = argv;
	MARKET	*market = argval->market;
	int	indx = argval->seqn;
	MDCTX	*ctx = market->ctx;
	MDINFO	*info = ctx->info;
	TOKEN	*token;
	struct	sockaddr_in sockin;
	char	msgb[MAX_PACKET_SIZE+512], *msgp;
	struct	msgbuf *msgbuf = (struct msgbuf *)msgb;
	int	sock, msgl, size, seqn;
	socklen_t socklen;
	int	hour;

	msgbuf->mtype = 1;
	sock = ctx->sock[indx];
	token = (TOKEN *)&msgbuf->mtext[0];
	msgp = &msgbuf->mtext[sizeof(TOKEN)];
	size = sizeof(msgb) - sizeof(TOKEN);

	memset(token, 0, sizeof(TOKEN));
	token->port = market->xchg->from[indx].port;
	strcpy(token->name, market->xchg->from[indx].name);
	token->seqn = market->xchg->from[indx].seqn;		// port seuence

	info->acct.port[indx].port = market->xchg->from[indx].port;
	strcpy(info->acct.port[indx].name, market->xchg->from[indx].name);
	seqn = argval->seqn % market->xchg->nofr;
	while (1)
	{
		socklen = sizeof(sockin);
		msgl = recvfrom(sock, msgp, size, 0, (struct sockaddr *)&sockin, &socklen);
		if (msgl < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
				continue;
			break;
		}
		gettimeofday(&token->timeval, NULL);
		hour = xHOUR(token->timeval.tv_sec);
		info->acct.rtim = token->timeval.tv_sec;
		if (msgsnd(ctx->mqid[seqn], msgbuf, msgl+sizeof(TOKEN), IPC_NOWAIT|MSG_NOERROR) != 0)
			info->acct.lost[hour]++;
		else
			info->acct.recv[hour]++;
		// port accounts
		if (indx >= 0 && indx < MAX_PORT)
		{
			info->acct.port[indx].rtim = token->timeval.tv_sec;
			info->acct.port[indx].recv++;
			info->acct.port[indx].rsum++;
		}
	}

	mds_log(market, LOG_MUST, "Stopped dispatch thread from a message queue !!!");
	pthread_exit(NULL);
}
