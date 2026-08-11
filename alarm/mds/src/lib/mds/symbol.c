#include "context.h"
#include "config.h"
#include "mdfold.h"
#include "stream.h"

#define	MAX_SYMBOL	1500

static	int cmpsymb();
static	int cmpzymd();

static	SYMBOL	symbol[MAX_SYMBOL];
static	int	n_symbol = 0;

//static	char wdaystr[7][4]= { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

//
// mds_symbinit()
// Initialzie product symbol
//
int mds_symbinit(MARKET *market) 
{
	SYMBOL	symb;
	int	i_clearing = -1, i_symbol = -1, i_intsymbol = -1, i_feed = -1;
	int	i_tradinghour = -1, i_tsessionhour = -1, i_price = -1; 
	int	i_priceindc = -1, i_priceadjust = -1;
	int	i_sector = -1;
	int	i_name = -1, i_hangul = -1;
	int	i_strikeindc = -1, i_strikeadjust1 = -1;
	int	i_pricemultipler = -1;
	int	i_atmadjust = -1, i_strikeinterval = -1;
	char	path_b[128];
	char	line_b[512];
	char	csv_b[32][128], chck_b[256];
	int	csv_n;
	FILE	*csvF;
	char	args[80], argv[4][32];
	char	*lptr;
	int	dodo = 0;
	int	ii;

	sprintf(path_b, "%s/%s-symb.csv", ETC_DIR, market->exnm);
	csvF = fopen(path_b, "r");
	if (csvF == NULL)
	{
		errno = ESRCH;
		return(-1);
	}
	while ((fgets(line_b, sizeof(line_b), csvF)) == line_b)
	{
		chck_b[0] = '\0';
		sscanf(line_b, "%s", chck_b);
		switch (chck_b[0])
		{
		case '#':
		case '*':
		case '/':
		case '\0':
			continue;
		case '=':
			csv_n = csvform(&line_b[1], csv_b);
			if (csv_n <= 0)
				continue;
			for (ii = 0; ii < csv_n; ii++)
			{
				if (strcasecmp(csv_b[ii], "Clearing") == 0)
					i_clearing = ii;
				else if (strcasecmp(csv_b[ii], "Symbol") == 0)
					i_symbol = ii;
				else if (strcasecmp(csv_b[ii], "IntSymbol") == 0)
					i_intsymbol = ii;
				else if (strcasecmp(csv_b[ii], "TradingHour") == 0)
					i_tradinghour = ii;
				else if (strcasecmp(csv_b[ii], "TsessionHour") == 0)
					i_tsessionhour = ii;
				else if (strcasecmp(csv_b[ii], "Price") == 0)
					i_price = ii;
				else if (strcasecmp(csv_b[ii], "PriceIndc") == 0)
					i_priceindc = ii;
				else if (strcasecmp(csv_b[ii], "PriceAdjust") == 0)
					i_priceadjust = ii;
				else if (strcasecmp(csv_b[ii], "Sector") == 0)
					i_sector = ii;
				else if (strcasecmp(csv_b[ii], "Name") == 0)
					i_name = ii;
				else if (strcasecmp(csv_b[ii], "Hangul") == 0)
					i_hangul = ii;
				else if (strcasecmp(csv_b[ii], "Feeder") == 0)
					i_feed = ii;
				else if (strcasecmp(csv_b[ii], "StrikeIndc") == 0)
					i_strikeindc = ii;
				else if (strcasecmp(csv_b[ii], "StrikeAdjust1") == 0)
					i_strikeadjust1 = ii;
				else if (strcasecmp(csv_b[ii], "PriceMultipler") == 0)
					i_pricemultipler = ii;
				else if (strcasecmp(csv_b[ii], "AtmAdjust") == 0)
					i_atmadjust = ii;
				else if (strcasecmp(csv_b[ii], "StrikeInterval") == 0)
					i_strikeinterval = ii;
			}
			dodo = 1;
			continue;
		default:
			break;
		}
		if (!dodo)
			continue;
		csv_n = csvform(line_b, csv_b);
		if (csv_n <= 0)
			continue;

		if (i_symbol == -1)
			continue;
			
		if (strlen(csv_b[i_symbol]) <= 0)	// no product symbol
			continue;
		memset(&symb, 0, sizeof(SYMBOL));
		symb.xdiv = 1;
		symb.zdiv = 0;
		symb.pmul = 1;

/*
		switch (market->xchg->type)
		{
		case FOREX:  	symb.styp = XT_FOREX;	break;
		case OPTION: 	symb.styp = XT_OPTION;	break;
		default:	symb.styp = XT_FUTURE;	break;
		}
*/
		symb.styp = market->xchg->type;
		if (i_intsymbol != -1)
			strcpy(symb.isym, csv_b[i_intsymbol]);		// internal symbol
		if (i_clearing != -1)
			strcpy(symb.csym, csv_b[i_clearing]);		// cklearing symbol
		if (i_priceindc != -1)
			symb.pind = atoi(csv_b[i_priceindc]);	// price indicator
		if (i_priceadjust != -1)
			symb.aval = atoi(csv_b[i_priceadjust]);	// price adjust value
		if (i_sector != -1)
			symb.sect = atoi(csv_b[i_sector]);		// product sector
		if (i_name != -1)
			strcpy(symb.enam, csv_b[i_name]);		// Product name (ENG)
		if (i_hangul != -1)
			strcpy(symb.knam, csv_b[i_hangul]);		// Product name (HANGUL)
		if (strlen(symb.knam) <= 0)
			strcpy(symb.knam, symb.enam);			// Product name (HANGUL)
		if (i_feed != -1)
			strcpy(symb.feed, csv_b[i_feed]);
		if (i_pricemultipler != -1)
			symb.pmul = atof(csv_b[i_pricemultipler]);
		if (symb.pmul == 0) symb.pmul = 1;
		if (i_atmadjust != -1)
			symb.atma = atof(csv_b[i_atmadjust]);
		if (i_strikeinterval != -1)
			symb.sint = atof(csv_b[i_strikeinterval]);

		if (i_tradinghour != -1)
		{
			// trading halt hour
			if (strlen(csv_b[i_tradinghour]) > 0 && strchr(csv_b[i_tradinghour], '-') != NULL)
			{
				strcpy(args, csv_b[i_tradinghour]);
				if ((lptr = strchr(args, '-')) != NULL)
					*lptr = ' ';
				argv[0][0] = '\0';	// from halting hour
				argv[1][0] = '\0';	// to   halting hour
				sscanf(args, "%s %s", argv[0], argv[1]);
				symb.session.frhm = atoi(argv[0]);
				symb.session.tohm = atoi(argv[1]);
			}
		}
		if (i_tsessionhour != -1)
			symb.session.tfhm = atoi(csv_b[i_tsessionhour]);
		else
			symb.session.tfhm = 9999;
		if (i_strikeindc != -1)
			symb.sind = atoi(csv_b[i_strikeindc]);
		if (i_strikeadjust1 != -1)
			symb.sad1 = atoi(csv_b[i_strikeadjust1]);

		strcpy(symb.symb, csv_b[i_symbol]);
		mds_log(market, LOG_MUST, "%d receive set symbol=%s pind=%d aval=%d", 
			n_symbol, symb.symb, symb.pind, symb.aval);
		if (n_symbol < MAX_SYMBOL)
		{
			memcpy(&symbol[n_symbol], &symb, sizeof(SYMBOL));
			n_symbol++;
		}
	}
	fclose(csvF);

	mds_log(market, LOG_MUST, "%d symbol products initialized !!!!", n_symbol);
	qsort(symbol, n_symbol, sizeof(SYMBOL), cmpsymb);

	return(0);
}

//
// mds_symbinfo()
// Get symbol information
//
int mds_symbinfo(MARKET *market, MDMSTR *mstr)
{
	SYMBOL	*prod;

	prod = mds_symbget(market, mstr->symb);
	
	mstr->exid = market->xchg->exid;
	strcpy(mstr->exnm, market->xchg->exnm);
	strcpy(mstr->clrs, prod->csym);			// clearing symbol
	strcpy(mstr->enam, prod->enam);			// english name
	strcpy(mstr->snam, prod->enam);			// english name
	strcpy(mstr->knam, prod->knam);			// korean name
	mstr->zdiv = prod->zdiv;			// no of decimal
	mstr->pind = prod->pind;			// price indicator
	mstr->aval = prod->aval;			// price price adjust value
	mstr->feed = atoi(prod->feed);		 	// market data feeder
	return(0);
}

//
// mds_symbget()
// Get symbol information
//
SYMBOL *mds_symbget(MARKET *market, const char *symb)
{
	SYMBOL	_symb, *prod;

	memset(&_symb, 0, sizeof(SYMBOL));
	strcpy(_symb.symb, symb);
	prod = bsearch(&_symb, symbol, n_symbol, sizeof(SYMBOL), cmpsymb);
	return(prod);
}

SYMBOL *mds_symbol(MARKET *market, int *nsym)
{
	*nsym = n_symbol;
	return(symbol);
}

struct	sequence {
	int	zymd;
	MDFOLD	*fold;
} sequence[255];

//
// Sequence numbering for folders
//
void mds_symbseqn(MARKET *market, MDFOLD *folder)
{
	MDARCH	*mdarch = market->arch;
	MDFOLD	*s = market->fold;
	MDFOLD	*x, *f, *t, *e;
	char	root[SYMB_LEN];
	int	l, n;
	int	ii;

	if (folder != NULL)
	{
		f = folder;
		t = folder;
		e = &s[mdarch->vrec];
		l = strlen(folder->mstr.root);
		for (x = folder - 1; x >= s; x--)
		{
			if (strcmp(x->mstr.root, folder->mstr.root) != 0 && 
			    strncmp(x->mstr.root, folder->mstr.root, l) != 0)
				break;
			f = x;
		}
		for (x = folder + 1; x < e; x++)
		{
			if (strcmp(x->mstr.root, folder->mstr.root) != 0 && 
			    strncmp(x->mstr.root, folder->mstr.root, l) != 0)
				break;
			t = x;
		}
		for (n = 0, x = f; x <= t && n < 255; x++)
		{
			if (x->mstr.symb[l] == '0')
			{
				x->mstr.seqn = 0;
				continue;
			}
			if (strcmp(folder->mstr.root, x->mstr.root) == 0)
			{
				sequence[n].zymd = x->mstr.zymd;
				sequence[n].fold = x;
				n++;
			}
		}
		if (n > 0)
		{
			qsort(sequence, n, sizeof(struct sequence), cmpzymd);
			for (ii = 0; ii < n; ii++)
			{
				x = sequence[ii].fold;
				x->mstr.seqn = ii+1;
			}
		}
		return;
	}

	root[0] = '\0';
	x = s;
	for (ii = 0; ii < mdarch->vrec; ii++)
	{
		if (strcmp(root, x[ii].mstr.root) != 0)
		{
			strcpy(root, x[ii].mstr.root);
			mds_symbseqn(market, &x[ii]);
		}
	}
}

static int cmpsymb(SYMBOL *s1, SYMBOL *s2)
{
	return(strcmp(s1->symb, s2->symb));
}
static int cmpzymd(struct sequence *s1, struct sequence *s2)
{
	if (s1->zymd > s2->zymd)
		return(1);
	if (s1->zymd < s2->zymd)
		return(-1);
	return(0);
}
