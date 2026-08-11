#include "context.h"
#include "config.h"

#define	MAX_XCHG	80

static	XCHG	exchange[MAX_XCHG];
static	int	n_xchg = 0;
static	PORTCFG ports[MAX_PORT];
static 	int	n_port = 0;

static void exchange_init();
static int  exchange_get(const char *exnm, XCHG *xchg);
static int  exchange_ipck(int exid);

static struct	xmltag xmltag[256];

static	struct cfgtag {
	int		mark;
	char	tags[16];
} cfgtag[] = {
	{  1, "xchg"	},	// <exchange name='name' exid='99' desc='description' />
	{  2, "recv"	},	// <recv from='udp|rabbitq' multicast='yes|no' rabbitq='queue-name' threads='nn' parallel='yes'/>
	{  3, "port"	},	// <port id='#n,ipad.port' id='#n,ipad.port' ... />
	{  4, "data"	},	// <data path="/home/data/NYS" room="2000" tablespace='xxx' mstr='yes' .... />
	{  5, "time"	},	// <time TZ="EST5EDT" trading="hhmm,hhmm" wday='Sun-Fri' 24hours='yes|no' />
	{  6, "batch"   },	// <batch open='hhmm' close='hhmm' batch='hhmm,hhmm,...' eod='hhmm,hhmm, ...' />
	{  7, "delay"	},	// <delay time='999' recn=999' />
	{  8, "keep"	}, 	// <keep tick="10" intr="20" />
	{  9, "real"	},	// <real push="yes" id="?" />
	{ 10, "watch"	},	// <watch time="hhmm,hhmm,seconds" time="hhmm,hhmm,seconds" .... />
	{ 11, "alert"	},	// <alert interval="seconds" phone="010-1234-1234" phone="011-1234-1234" .../>
	{ 12, "log"	},	// <log level="error|warning|progrerss|debug" />
	{ 13, "notify"	},	// <notify cast='yes|no' ipad='1.1.1.1' port='n' />
	{  0, ""	}
};

//
// mds_exchange()
// Get exchange
//
int mds_exchange(const char *exnm, XCHG *xchg)
{
	int	ii;

	exchange_init();
	for (ii = 0; ii < n_xchg; ii++)
	{
		if (strcasecmp(exchange[ii].exnm, exnm) == 0)
		{
			memcpy(xchg, &exchange[ii], sizeof(XCHG));
			return(0);
		}
	}
	errno = ENOENT;
	return(-1);
}

//
// mds_exchanges()
// Get all exchange
//
XCHG *mds_exchanges()
{
	exchange_init();
	return(exchange);
}

XCHG *mds_exchange_by_exid(int exid)
{
	int	ii;

	exchange_init();
	for (ii = 0; ii < n_xchg; ii++)
	{
		if (exchange[ii].exid == exid)
			return(&exchange[ii]);
	}
	return(NULL);
}

//
//  exchange_get()
//  Get exchange information
//
static int exchange_get(const char *exnm, XCHG *xchg)
{
	char	wdaystr[7][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	char	xmlpath[128];
	int	nconf;
	char	args[80], argv[8][32];
	char	*lptr;
	int	from, to, interval;
	int	hh, mm;
	int	wf, wt, wd;
	int	ii, jj;


	sprintf(xmlpath, "%s/%s.cfg", ETC_DIR, exnm);
	if ((nconf = getxmlcfg(xmlpath, xmltag)) <= 0)
	{
		errno = ESRCH;
		return(-1);
	}
	xchg->llog = LOG_ERROR;
	xchg->batch.open = -1;
	xchg->batch.close = -1;
	for (ii = 0; ii < nconf; ii++)
	{
		for (jj = 0; cfgtag[jj].mark != 0; jj++)
		{
			if (strcasecmp(xmltag[ii].tags, cfgtag[jj].tags) == 0)
				break;
		}
		if (!cfgtag[jj].mark)
			continue;
		switch (cfgtag[jj].mark)
		{
		case 1: // <xchg name='name' exid='999' desc=description' />
			getargs(&xmltag[ii], "name", xchg->exnm);
			getargv(&xmltag[ii], "exid", &xchg->exid);
			getargs(&xmltag[ii], "desc", xchg->desc);
			getargv(&xmltag[ii], "schema", &xchg->schm);	// schema
			if (strlen(xchg->exnm) <= 0 || xchg->exid < 0 || xchg->exid > 999)
			{
				errno = EINVAL;
				return(-1);
			}
			getargs(&xmltag[ii], "type", argv[0]);
			if (strcasecmp(argv[0], "future") == 0)
				xchg->type = XT_FUTURE;
			else if (strcasecmp(argv[0], "option") == 0)
				xchg->type = XT_OPTION;
			else if (strcasecmp(argv[0], "futopt") == 0)
				xchg->type = XT_FUTOPT;
			else 
				xchg->type = XT_STOCKS;
			break;
		case 2: // <recv from='udp" multicast='yes|no'/>
			getargs(&xmltag[ii], "threads", argv[0]);
			if ((xchg->nofr = atoi(argv[0])) <= 0)
				xchg->nofr = 1;
			break;
		case 3: // <port seqn='n' name='alpari' ipad='1.1.1.1' port='n' />
			getargs(&xmltag[ii], "seqn", argv[0]);
			getargs(&xmltag[ii], "name", argv[1]);
			getargs(&xmltag[ii], "ipad", argv[2]);
			getargs(&xmltag[ii], "port", argv[3]);
			jj = atoi(argv[0]);
			if (jj <= 0 || jj > MAX_PORT)
				break;
			jj--;
			if (strlen(argv[1]) <= 0 || strlen(argv[2]) <= 0 || strlen(argv[3]) <= 0)
				break;

			xchg->from[jj].seqn = jj+1;				// PORT SEQUENCE
			strcpy(xchg->from[jj].name, argv[1]);			// PRODUCT
			strcpy(xchg->from[jj].ipad, argv[2]);			// IPAD
			xchg->from[jj].port = atoi(argv[3]);			// PORT-NO
			if (xchg->from[jj].port <= 0)
			{
				xchg->from[jj].seqn = 0;
				break;
			}
			if (xchg->nofp <= jj)
				xchg->nofp = jj+1;
			break;
		case 4: // <data path="/home/..." room="n" dbsvr="yes|no"/>
			getargs(&xmltag[ii], "path",  xchg->dirp);
			getargv(&xmltag[ii], "room", &xchg->room);
			getargv(&xmltag[ii], "book_level", &xchg->book_level);
			getargs(&xmltag[ii], "months-offset", args);
			if (strlen(args) > 0)
				xchg->moff = atoi(args);
			break;
		case 5: // <time TZ="EST5EDT"  trading='hhmm-hhmm'  wday='Sun-Fri' 24hours='yes' />
			getargs(&xmltag[ii], "TZ", xchg->TZ);
			getargs(&xmltag[ii], "trading",  args);			// trading hour
			if ((lptr = strchr(args, '-')) != NULL)
				*lptr = ' ';
			argv[0][0] = '\0';
			argv[1][0] = '\0';
			sscanf(args, "%s %s", argv[0], argv[1]);
			xchg->trading.from = atoi(argv[0]);
			xchg->trading.to = atoi(argv[1]);
			getargs(&xmltag[ii], "hour24", args);
			if (strcasecmp(args, "yes") == 0)
				xchg->trading.h24t = 1;
			xchg->trading.wday = 0;
			for (wd = 1; wd <= 5; wd++)
				xchg->trading.wday |= (1 << wd);
			getargs(&xmltag[ii], "wday", args);
			if (args <= 0)
				break;
			while ((lptr = strchr(args, '-')) != NULL)
				*lptr = ' ';
			sscanf(args, "%s %s", argv[0], argv[1]);
			for (wd = 0, wf = -1, wt = -1; wd < 7; wd++)
			{
				if (strcasecmp(wdaystr[wd], argv[0]) == 0)
					wf = wd;
				if (strcasecmp(wdaystr[wd], argv[1]) == 0)
					wt = wd;
			}
			if (wf != -1 && wt != -1 && wt > wf)
			{
				for (wd = wf; wd <= wt; wd++)
					xchg->trading.wday |= (1 << wd);
			}
			break;
		case 6: // <batch open='hhmm' close='hhmm' ...' />
			getargv(&xmltag[ii], "open",  &xchg->batch.open);	// open batch time
			getargv(&xmltag[ii], "close", &xchg->batch.close);	// trading end time
			break;
		case 7: // <delay index='minutes' stock='minutes' />
			getargv(&xmltag[ii], "index",  &xchg->delay_t[0]);	// delay time
			getargv(&xmltag[ii], "stock",  &xchg->delay_t[1]);	// delay time
			break;
		case 8: // <keep tick="10" intr="20" />
			getargv(&xmltag[ii], "tick",  &xchg->keep[0]);		// TICK saving days
			getargv(&xmltag[ii], "intr",  &xchg->keep[1]);		// INTR saving days
			break;
		case 9: // <real push='yes' id='?' filter='yes' />
			getargs(&xmltag[ii], "push", args);
			if (strlen(args) > 0 && strcasecmp(args, "yes") == 0)
				xchg->push = 1;
			getargs(&xmltag[ii], "id", args);
			if (strlen(args) > 0)
				xchg->push_id = args[0];
			getargs(&xmltag[ii], "bookfilter", args);
			if (strlen(args) > 0 && strcasecmp(args, "yes") == 0)
				xchg->bfil = 1;
			getargs(&xmltag[ii], "quotfilter", args);
			if (strlen(args) > 0 && strcasecmp(args, "yes") == 0)
				xchg->qfil = 1;
			break;
		case 10: // <watch time="hhmm,hhmm,seconds" .... />
			while (1)
			{
				getargs(&xmltag[ii], "time", args);					// time to checking
				if (strlen(args) <= 0)
					break;
				while ((lptr = strchr(args, ',')) != NULL)
					*lptr = ' ';
				argv[0][0] = '\0'; argv[1][0] = '\0'; argv[2][0] = '\0';
				sscanf(args, "%s %s %s", argv[0], argv[1], argv[2]);// from,to,interval(seconds)
				from = atoi(argv[0]);
				hh = from / 100;
				mm = from % 100;
				if (hh < 0 || hh >= 24 || mm < 0 || mm >= 60)
					continue;
				from = (hh * 60) + mm;

				to = atoi(argv[1]);
				hh = to / 100;
				mm = to % 100;
				if (hh < 0 || hh >= 24 || mm < 0 || mm >= 60)
					continue;
				to = (hh * 60) + mm;

				interval = atoi(argv[2]);
				if (from >= to || interval <= 0)
					continue;
				for (jj = from; jj <= to; jj++)
				{
					hh = jj / 60;
					mm = jj % 60;
					xchg->watch[hh][mm] = interval;
				}
			}
			break;
		case 11: // <alert interval="seconds" phone="010-1234-1234" phone="011-1234-1234" ... />
			getargv(&xmltag[ii], "interval",  &xchg->alert.interval);// alerting interval
			while (1)
			{
				getargs(&xmltag[ii], "phone", args);		// time to checking
				if (strlen(args) <= 0)
					break;
				if (xchg->alert.many >= 8)
					break;
				if (strlen(args) >= 16)
					continue;
				strcpy(xchg->alert.phone[xchg->alert.many++], args);
			}
			break;
		case 12: // <log level="error|warning|progress|debug" path='pathname'/>
			getargs(&xmltag[ii], "path", xchg->logf);
			getargs(&xmltag[ii], "level", args);
			if (strcasecmp(args, "error") == 0)
				xchg->llog = LOG_ERROR;
			else if (strcasecmp(args, "warning") == 0)
				xchg->llog = LOG_WARNING;
			else if (strcasecmp(args, "progress") == 0)
				xchg->llog = LOG_PROGRESS;
			else if (strcasecmp(args, "debug") == 0)
				xchg->llog = LOG_DEBUG;
			else
				xchg->llog = LOG_ERROR;
			break;
		case 13: // <notify cast='yes|no' ipad='1.1.1.1' port='udp-port-number' />
			getargs(&xmltag[ii], "cast", argv[0]);
			getargs(&xmltag[ii], "ipad", argv[1]);
			getargs(&xmltag[ii], "port", argv[2]);
			getargs(&xmltag[ii], "neta", argv[3]);
			if ((xchg->notify.port = atoi(argv[2])) > 0)
			{
				if (strcasecmp(argv[0], "yes") == 0)
					xchg->notify.cast = 1;
				strcpy(xchg->notify.ipad, argv[1]);
				strcpy(xchg->notify.neta, argv[3]);
			}
			break;
		}
	}

	xchg->delay_t[0] *= 60;			// index delay time (minutes->seconds)
	xchg->delay_t[1] *= 60;			// index delay time (minutes->seconds)
	if (xchg->room <= 0)
		xchg->room = 1000;
	if (strlen(xchg->TZ) > 0)
	{
		sprintf(args, "TZ=%s", xchg->TZ);
		strcpy(xchg->TZ, args);
	}
	xchg->ipck = exchange_ipck(xchg->exid);
	return(0);
}

//
// exchange_init()
// Initialize all exchanges
//
static void exchange_init()
{
	static	int initialized = 0;
	char	xmlpath[128];
	char	args[80];
	char	exnm[MAX_XCHG][50];
	int	exid[MAX_XCHG];
	int	nxml, many = 0;
	int	ii;

	if (initialized)
		return;

	n_xchg = 0;
	sprintf(xmlpath, "%s/exchanges.cfg", ETC_DIR);
	nxml = getxmlcfg(xmlpath, xmltag);
	if (nxml <= 0)
		return;

	for (ii = 0; ii < nxml && many < MAX_XCHG; ii++)
	{
		if (strcmp(xmltag[ii].tags, "exchange") != 0 || !xmltag[ii].eotf)
			continue;
		getargs(&xmltag[ii], "fep", args);
		if (strcasecmp(args, "yes") == 0)
			continue;
		getargs(&xmltag[ii], "name", args);
		if (strlen(args) <= 0)
			continue;
		strcpy(exnm[many], args);
		getargs(&xmltag[ii], "exid", args);
		exid[many] = atoi(args);
		if (exid[many] < 0)
			continue;
		many++;
	}
	for (ii = 0; ii < many; ii++)
	{
		if (exchange_get(exnm[ii], &exchange[n_xchg]) == 0)
			n_xchg++;
	}
	initialized = 1;
	return;
}

//
// Generate IPC keyvalue by exid
//
static int exchange_ipck(int exid)
{
	char	name[30], decimal;
	int	hexa;
	key_t	ipck;
	int	ii;

	sprintf(name, "%03d", exid);	
	for (ii = 0, hexa = 0; ii < strlen(name) && ii < 3; ii++)
	{
		hexa <<= 4;
		decimal = name[ii] & 0x0f;
		hexa |= decimal;
	}
	hexa |= 0x9000;
	ipck = (hexa << 16);				// 0x9{EXID}??##
	return(ipck);
}

static void portcfg_init(MARKET *market)
{
	struct	xmltag xmltag[512];
	char	xmlpath[128];
	int	nconf;
	char	argv[8][32];
	PORTCFG	cfg;
	int	end;
	int	ii, jj = 0, nn;

	if (n_port != 0)
		return;
	sprintf(xmlpath, "%s/%s-port.cfg", ETC_DIR, market->exnm);
	if ((nconf = getxmlcfg(xmlpath, xmltag)) <= 0)
		return;
	for (ii = 0; ii < nconf; ii++)
	{
		if (strcasecmp(xmltag[ii].tags, "port") != 0)
			continue;
		memset(&cfg, 0, sizeof(PORTCFG));
		for (end = 0; ii < nconf; ii++)
		{
			if (strcasecmp(xmltag[ii].tags, "/port") == 0)
			{
				end = 1;
				break;
			}
			getargs(&xmltag[ii], "seqn", argv[0]);
			getargs(&xmltag[ii], "name", argv[1]);
			cfg.seqn = atoi(argv[0]);
			strcpy(cfg.name, argv[1]);
			getargs(&xmltag[ii], "port", argv[2]);
			cfg.port = atoi(argv[2]);
			ii++;
			for (; ii < nconf; ii++)
			{
				if (strcasecmp(xmltag[ii].tags, "/port") == 0)
				{
					end = 1;
					break;
				}
				nn = 0;
				if (strcasecmp(xmltag[ii].tags, "feed-A") == 0)
					jj = 0;
				else if (strcasecmp(xmltag[ii].tags, "feed-B") == 0)
					jj = 1;
				else if (strcasecmp(xmltag[ii].tags, "sendto") == 0)
				{
					getargs(&xmltag[ii], "to", argv[0]);
					if (strcasecmp(argv[0], "rabbitq") == 0)
					{
						cfg.send.howto = 2;
						getargs(&xmltag[ii], "rabbitq", argv[1]);
						strcpy(cfg.send.qnam, argv[1]);
					}
					else if (strcasecmp(argv[0], "udp") == 0)
					{
						cfg.send.howto = 1;
						getargs(&xmltag[ii], "ip", argv[1]);
						strcpy(cfg.send.ipad, argv[1]);
						getargs(&xmltag[ii], "port", argv[2]);
						cfg.send.port = atoi(argv[2]);
					}
					continue;
				}
				else 
					continue;
				getargs(&xmltag[ii], "ip", argv[0]);
				strcpy(cfg.line[jj].ipad, argv[0]);

				getargs(&xmltag[ii], "host", argv[1]);
				if (strlen(argv[1]) > 0)
					cfg.line[jj].host[nn++] = inet_addr(argv[1]);
				getargs(&xmltag[ii], "host", argv[1]);
				if (strlen(argv[1]) > 0)
					cfg.line[jj].host[nn++] = inet_addr(argv[1]);
				getargs(&xmltag[ii], "host", argv[1]);
				if (strlen(argv[1]) > 0)
					cfg.line[jj].host[nn++] = inet_addr(argv[1]);
				getargs(&xmltag[ii], "host", argv[1]);
				if (strlen(argv[1]) > 0)
					cfg.line[jj].host[nn++] = inet_addr(argv[1]);
			}
			if (end)
				break;
		}
		if (cfg.seqn <= 0 || cfg.seqn > MAX_PORT)
			continue;
		if (strlen(cfg.name) <= 0 || cfg.port <= 0)
			continue;
		switch (cfg.send.howto)
		{
		case 1: // broadcasting
			if (strlen(cfg.send.ipad) <= 0 || cfg.send.port <= 0)
				continue;
			break;
		case 2: // queue
			if (strlen(cfg.send.qnam) <= 0)
				continue;
			break;
		default:
			continue;
		}
		memcpy(&ports[n_port], &cfg, sizeof(PORTCFG));
		n_port++;
		if (n_port >= MAX_PORT)
			break;
	}
}

//
// mds_portcfg()
// Get receive port configuration
//
int mds_portcfg(MARKET *market, const char *name, PORTCFG *portcfg)
{
	int	ii;

	portcfg_init(market);
	memset(portcfg, 0, sizeof(PORTCFG));
	for (ii = 0; ii < n_port; ii++)
	{
		if (strcmp(ports[ii].name, name) == 0)
		{
			memcpy(portcfg, &ports[ii], sizeof(PORTCFG));
			return(0);
		}
	}
	return(-1);
}

PORTCFG *mds_getport(MARKET *market)
{
	portcfg_init(market);
	return(ports);
}
