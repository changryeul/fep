#include "context.h"
#include "mdprod.h"
#include "mdttbl.h"

static int     str2words(char *lineB, char *wordB[], int wordN);
static char   *getwords(char *istr, char *word);

//
// Allocate market 
//
MARKET *mds_market_alloc()
{
	MARKET 	*market;
	MDCTX	*ctx;
	XCHG	*xchg;
	MDPROD  *prod;
	MDTTBL  *ttbl;
	size_t	size;
	int	ii;

	size = sizeof(MARKET) + sizeof(MDCTX) + sizeof(XCHG) + sizeof(MDPROD) + sizeof(MDTTBL);
	if ((market = (MARKET *)malloc(size)) == NULL)
		return(NULL);
	memset(market, 0, size);
	ctx = (MDCTX *)&market[1];
	xchg = (XCHG *)&ctx[1];
	prod = (MDPROD*)&xchg[1];
	ttbl = (MDTTBL*)&prod[1];
	market->ctx  = ctx;
	market->xchg = xchg;
	market->prod = prod;
	market->ttbl = ttbl;
	
	pthread_mutex_init(&ctx->lock, NULL);
	pthread_mutex_init(&ctx->mutex, NULL);
	pthread_mutex_init(&ctx->islock, NULL);
	market->ctx = ctx;
	ctx->market = market;
	for (ii = 0; ii < MAX_PORT; ii++)
	{
		ctx->mqid[ii] = -1;
		ctx->sock[ii] = -1;
	}
	return(market);
}


// 
// Setup run-time environment
//
void mds_setenv()
{
	char	profile[128];
	FILE	*chckF;
	char	lineX[512], lineB[512];
	char	chckB[4][256];
	char	*wordP[4], *equaL;
	char	*cenvP, *tagsP, *defsP;
	int	wordN;
	int	ii, jj;

	sprintf(profile, "%s/profile", ETC_DIR);
	chckF = fopen(profile, "r");
	if (chckF == NULL)
		return;

	for (ii = 0; ii < 4; ii++)
		wordP[ii] = chckB[ii];
	while ((fgets(lineB, sizeof(lineB)-1, chckF)) == lineB)
	{
		for (ii = 0, jj = 0; ii < strlen(lineB); ii++)
		{
			if (lineB[ii] == ' ' || lineB[ii] == '\t')
				continue;
			if (lineB[ii] == '*' || lineB[ii] == '#')
				break;
			lineX[jj++] = lineB[ii];
		}
		lineX[jj] = '\0';
		wordN = str2words(lineX, wordP, 4);
		if (wordN <= 0)
			continue;
		equaL = strstr(wordP[0], "=");
		if (equaL == NULL)
			continue;

		cenvP = malloc(strlen(wordP[0]) + 512);
		if (cenvP == NULL)
			continue;

		tagsP = wordP[0];
		defsP = equaL + 1;
		*equaL = '\0';

		sprintf(cenvP, "%s=%s", tagsP, defsP);
		putenv(cenvP);
	}
	fclose(chckF);
}

/*
 * str2words()
 * Seprator words
 */
static int str2words(char *lineB, char *wordB[], int wordN)
{
	char	*nextP;
	char	chckB[256];
	int	ii;

	nextP = lineB;
	for (ii = 0; ii < wordN; ii++)
	{
		if (strlen(nextP) <= 0)
			break;
		nextP = getwords(nextP, chckB);
		if (strlen(chckB) <= 0 || chckB[0] == '#')
			break;
		strcpy(wordB[ii], chckB);
	}
	return(ii);
}

/**
 * getwords()
 * Get word string from input string
 */
static char *getwords(char *istr, char *word)
{
	int	ii, jj;
	int	ilen;

	ilen = strlen(istr);
	for (ii = 0, jj = 0; ii < ilen; ii++, istr++)
	{
		switch (*istr)
		{
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			if (jj != 0)
				break;
			continue;
		default:
			word[jj++] = *istr;
			continue;
		}
		break;
	}
	word[jj] = '\0';
	if (ii < ilen && (*istr == '\n' || *istr == '\r'))
	{
		ii++;
		istr++;
	}
	if (ii < ilen && (*istr == '\n' || *istr == '\r'))
		istr++;
	return(istr);
}
