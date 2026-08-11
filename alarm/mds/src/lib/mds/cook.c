//
// init.c
// Set procedure for market data cooking
//
#include "context.h"

void *scheduler(void *);
void *notifier(void *);
void *udprcv(void *);

#define	MULTICAST_F	"224.0.0.0"
#define	MULTICAST_T	"239.255.255.255"

#define	QSZ	(1024*1024*20)

int mds_cooker(MARKET *market, MDSPROC *proc, int noreturn)
{
	MDCTX	*ctx = market->ctx;
	SCHEMA	*schema = ctx->schema;
	struct	argval argval[MAX_PORT];
	struct	argval argval2[MAX_PORT];
	struct	msqid_ds msqid_ds;
	struct	sockaddr_in sockin;
	struct	ip_mreq ip_mreq;
	int	size, options = 1;
	in_addr_t addr, f, t;
	struct	in_addr local;
	pthread_attr_t attr;
	size_t	stacksize;
	char	loopch;
	int	sock;
	char	TTL = 1;
	int	ii;

	switch (market->whoami)
	{
	case I_AM_COOKER:
		break;
	default:
		errno = EPERM;
		return(-1);
	}
	ctx->proc.open  = proc->open;		// openmarket
	ctx->proc.recv  = proc->recv;		// receiver (stream)
	ctx->proc.clos  = proc->clos;		// close market
	ctx->proc.xday  = proc->xday;		// open a new calendar day

	for (ii = 0; ii < market->xchg->nofr && ii < MAX_PORT; ii++)
	{
		// Create receive queue
		if ((ctx->mqid[ii] = msgget(IPCK(market->xchg->ipck, ii), 0666|IPC_CREAT)) < 0)
		{
			mds_log(market, LOG_ERROR, "Cannot create message queue to receive !!");
			return(-1);
		}
		if (msgctl(ctx->mqid[ii], IPC_STAT, &msqid_ds) != 0)
		{
			mds_log(market, LOG_ERROR, "Cannot check a message queue to receive");
			return(-1);
		}
		if (msqid_ds.msg_qbytes != QSZ)
		{
			msqid_ds.msg_qbytes = QSZ;
			msgctl(ctx->mqid[ii], IPC_SET, &msqid_ds);
		}
	}

	// use UDP direct
	f = ntohl(inet_addr(MULTICAST_F));
	t = ntohl(inet_addr(MULTICAST_T));

	size = 80 * 1024;
	for (ii = 0; ii < market->xchg->nofp; ii++)
	{
		ctx->sock[ii] = -1;
		if (market->xchg->from[ii].seqn <= 0)
			continue;
		addr = ntohl(inet_addr(market->xchg->from[ii].ipad));
		if (addr >= f && addr <= t)
		{
			// multicasting
			if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
			{
				mds_log(market, LOG_ERROR, "Cannot open a socket for %s.%d", 
					market->xchg->from[ii].ipad, market->xchg->from[ii].port);
				return(-1);
			}
			memset(&sockin, 0, sizeof(sockin));
			sockin.sin_family = AF_INET;
			//sockin.sin_addr.s_addr = INADDR_ANY;
			sockin.sin_addr.s_addr = inet_addr(market->xchg->from[ii].ipad);
			sockin.sin_port = htons(market->xchg->from[ii].port);
			options = 1;
			setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options));
			if (bind(sock, (struct sockaddr *)&sockin, sizeof(sockin)) != 0)
			{
				close(sock);
				mds_log(market, LOG_ERROR, "Cannot bind a socket for %s.%d", 
					market->xchg->from[ii].ipad, market->xchg->from[ii].port);
			}
			ip_mreq.imr_multiaddr.s_addr = inet_addr(market->xchg->from[ii].ipad);
			ip_mreq.imr_interface.s_addr = INADDR_ANY;
			if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&ip_mreq, sizeof(ip_mreq)) != 0)
			{
				mds_log(market, LOG_ERROR, "Cannnot add a multicast member for %s.%d", 
					market->xchg->from[ii].ipad, market->xchg->from[ii].port);
				close(sock);
				return(-1);
			}
			mds_log(market, LOG_MUST, "Start to receive from %s.%d", 
				market->xchg->from[ii].ipad, market->xchg->from[ii].port);
		}
		else
		{
			// broadcasting or unicasting
			if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
			{
				mds_log(market, LOG_ERROR, "Cannot open a socket for a port %d", 
					market->xchg->from[ii].port);
				return(-1);
			}
			memset(&sockin, 0, sizeof(sockin));
			sockin.sin_family = AF_INET;
			sockin.sin_addr.s_addr = INADDR_ANY;
			sockin.sin_port = htons(market->xchg->from[ii].port);
			options = 1;
			setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options));
			if (bind(sock, (struct sockaddr *)&sockin, sizeof(sockin)) != 0)
			{
				close(sock);
				mds_log(market, LOG_ERROR, "Cannot bind a socket for a port %d", 
					market->xchg->from[ii].port);
				return(-1);
			}
			mds_log(market, LOG_MUST, "Start to receive from UDP port %d", market->xchg->from[ii].port);
		}
        	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(int));
		ctx->sock[ii] = sock;
	}

	pthread_attr_init(&attr);
	pthread_attr_getstacksize(&attr, &stacksize);
	if (stacksize < (2048*1024))
	{
		stacksize = 2048 * 1024;
		pthread_attr_setstacksize(&attr, stacksize);
	}
	for (ii = 0; ii < market->xchg->nofp; ii++)
	{
		if (ctx->sock[ii] < 0 || market->xchg->from[ii].seqn <= 0)
			continue;
		argval[ii].market = market;
		argval[ii].seqn = ii;
		argval[ii].argv = market->xchg->from[ii].port;

		pthread_create(&ctx->recver[ii], &attr, udprcv, &argval[ii]);
	}

	// notifier market data to another process 
	sock = -1;
	ctx->posta.sock = sock;
	if (market->xchg->notify.port > 0)
	{
		memset(&sockin, 0, sizeof(sockin));
		sockin.sin_family = AF_INET;
		sockin.sin_port = htons(market->xchg->notify.port);
		sockin.sin_addr.s_addr = inet_addr(market->xchg->notify.ipad);

		addr = ntohl(inet_addr(market->xchg->notify.ipad));
		f = ntohl(inet_addr(MULTICAST_F));
		t = ntohl(inet_addr(MULTICAST_T));
		if (addr >= f && addr <= t)
		{
			if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) >= 0)
			{
    			setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&TTL, sizeof(TTL));
				// loopback so you do not receive your own datagram.
				loopch = 1;
				setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch));

				// Set local interface for outbound multicast datagrams.
				// The IP address specified must be associated with a local,
				// multicast-capable interface.
				local.s_addr = inet_addr(market->xchg->notify.neta);
				setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (char *)&local, sizeof(local));
			}
		}
		else
		{
			if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
				setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &options, sizeof(int));
		}
		memcpy(&ctx->posta.sin, &sockin, sizeof(sockin));
		ctx->posta.sock = sock;
	}

	if (proc->recv != NULL)
	{
		for (ii = 0; ii < market->xchg->nofr && ii < MAX_PORT; ii++)
		{
			argval2[ii].market = market;
			argval2[ii].seqn = ii;
			pthread_create(&ctx->notifier[ii], &attr, notifier, &argval2[ii]);
		}
	}

	// create syncer thread
	if (schema != NULL)
	{
		for (ii = 0; schema[ii].dbid >= 0; ii++)
		{
			if (!(schema[ii].dodo & _DF_))		// deferred synchronize
				continue;
			switch(schema[ii].dbid)
			{
			case MSTR: break;
			case BOOK: break;
			case QUOT: break;
			default:   continue;
			}
			break;
		}
		if (schema[ii].dbid >= 0)
			pthread_create(&ctx->syncer, &attr, mds_syncer, market);
	}
	

	switch (market->whoami)
	{
	case I_AM_COOKER:
		if (noreturn)
			scheduler(market);
		else
			pthread_create(&ctx->scheduler, &attr, scheduler, market);
		break;
	default:
		while (noreturn)
		{
			sleep(1000);
		}
		break;
	}
	return(0);
}
