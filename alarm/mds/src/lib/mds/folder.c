//
// folder.c
// Control shared memory folder
//
#include "context.h"

//
// mds_folder()
// Insert a new folder for a symbol
//
static void *mds_folder(MARKET *market, INDEX *w)
{
	MDARCH	*arch = market->arch;
	MDCTX	*ctx = market->ctx;
	char	*fb  = market->fold;

	if (arch == NULL || ctx == NULL || fb == NULL)
		return(NULL);
	if (w->indx < 0 || w->indx >= arch->vrec)
		return(NULL);
	fb += (ctx->fsiz * w->indx);
	return(fb);
}

//
//
// mds_getfolder()
// Get folder for a symbol
//
void *mds_getfolder(MARKET *market, const char *symb)
{
	INDEX	*w;

	if ((w = mds_shmget(market, symb)) == NULL)
		return(NULL);
	return(mds_folder(market, w));
}
// mds_setfolder()
// Set 1st position of folder
//
void mds_setfolder(MARKET *market, const char *symb)
{
	MDARCH	*arch = market->arch;
	INDEX	*indx = market->indx;
	int	from, to, mid, pos = 0;
	int	result, dir, len;
	int	ii;
	
	if (symb == NULL || strlen(symb) <= 0)
	{
		market->fptr = &indx[0];
		return;
	}
	if (arch->nrec <= 0 || strcmp(symb, indx[arch->nrec-1].symb) > 0)
	{
		market->fptr = NULL;
		return;
	}

	len = strlen(symb);

	from = 0;
	to = arch->nrec;
	pos = 0;
	dir = 1;
	while (from < to)
	{
		mid = from + (to - from) / 2;
		result = strncmp(symb, indx[mid].symb, len);
		if (result == 0)
		{
			pos = mid;
			dir = -1;
			break;
		}
		if (result < 0)
		{
			to = mid;
			pos = mid;
			dir = -1;
		}
		else 
		{
			from = mid + 1;
			pos = mid;
			dir = 1;
		}
	}
	switch (dir)
	{
	case -1:  // seek backwarding
		for (ii = pos; ii >= 0; ii--)
		{
			if (strncmp(symb, indx[ii].symb, len) <= 0)
				pos = ii;
			else
				break;
		}
		break;
	default: // seek forwading
		for (ii = pos; ii < arch->nrec; ii++)
		{
			if (strncmp(indx[ii].symb, symb, len) < 0)
				continue;
			break;
		}
		pos = ii;
		break;
	}
	if (pos > arch->nrec)
		market->fptr = NULL;
	else
		market->fptr = &indx[pos];
}

//
// mds_rewfolder()
// Rewindind folder
//
int mds_rewfolder(MARKET *market)
{
	MDARCH	*arch = market->arch;
	INDEX	*indx = market->indx;
	INDEX	*curr = market->fptr;
	INDEX	*top, *bot;
	int	pos;

	if (curr == NULL)
		return(-1);
	top = &indx[0];
	bot = &indx[arch->nrec];

	curr--;
	if (curr < top || curr >= bot)
		return(-1);
	market->fptr = curr;
	pos = (curr - top) / sizeof(INDEX);
	return(pos);
}

//
// mds_popfolder()
// Get a next folder
//
void *mds_popfolder(MARKET *market)
{
	MDARCH	*arch = market->arch;
	INDEX	*indx = market->indx;
	INDEX	*curr = market->fptr;
	INDEX	*top, *bot;
	void	*folder;

	if (curr == NULL)
		return(NULL);
	top = &indx[0];
	bot = &indx[arch->nrec];

	if (curr < top || curr >= bot)
		return(NULL);
	folder = mds_folder(market, curr);
	curr++;
	market->fptr = curr;
	return(folder);
}

//
// mds_newfolder()
// Insert a new folder
//
void *mds_newfolder(MARKET *market, const char *symb)
{
	MDARCH	*arch = market->arch;
	MDCTX	*ctx = market->ctx;
	char	*fb  = market->fold;
	FOLDER	*folder;
	INDEX	*w;
	char	*r;

	if (arch == NULL || ctx == NULL || fb == NULL)
		return(NULL);
	if ((w = mds_shmget(market, symb)) != NULL)
		return(mds_folder(market, w));

	w = mds_shmadd(market, symb);
	if (w == NULL)
		return(NULL);
	fb += (ctx->fsiz * w->indx);
	folder = (FOLDER *)fb;
	strcpy(folder->symb, symb);
	if (ctx->func->roff != NULL)
	{
		if ((r = ctx->func->roff(market, fb, MSTR)) != NULL)
		{
			strcpy(r, symb);
			mds_isupsert(market, MSTR, r);
		}
		if ((r = ctx->func->roff(market, fb, QUOT)) != NULL)
			strcpy(r, symb);
		if ((r = ctx->func->roff(market, fb, BOOK)) != NULL)
			strcpy(r, symb);
		if ((r = ctx->func->roff(market, fb, INTR)) != NULL)
			strcpy(r, symb);
	}
	return(fb);
}

//
// mds_delfolder()
// Delete folder on shared memory
//
void mds_delfolder(MARKET *market, const char *symb)
{
	mds_shmdel(market, symb);
}

//
// mds_syncfolder()
// Updated folder's data
//
int mds_syncfolder(MARKET *market, void *folder, int dbid)
{
	MDCTX	*ctx = market->ctx;

	if (ctx->func->sync != NULL)
		ctx->func->sync(market, folder, dbid);
	return(0);
}

//
// mds_clrfolder()
// Clear folder
//
void mds_clrfolder(MARKET *market, void *folder)
{
	MDCTX	*ctx = market->ctx;
	
	if (ctx->func->clear != NULL)
	{
		ctx->func->clear(market, folder);
		mds_syncfolder(market, folder, QUOT);
		mds_syncfolder(market, folder, BOOK);
	}
}

void mds_pushfolder(MARKET *market, void *folder, int event)
{
	MDCTX	*ctx = market->ctx;

	if (ctx->func->push != NULL)
		ctx->func->push(market, folder, event);
} 

int mds_seekfolder(MARKET *market, const char *prefix, WHERE *where)
{
	MDCTX	*ctx = market->ctx;

	if (ctx->func->push != NULL)
		return(ctx->func->seek(market, prefix, where));
	return(0);
}

static int cmpstrk();

//
// mds_optfolder()
// Spread Call & Put Option folders based on strike price
//
//int mds_optfolder(MARKET *market, WHERE *where, OPTFOLD *optf)
//{
//	MDCTX	*ctx = market->ctx;
//	void	*folder;
//	struct	cnp cnp, *p;
//	int	styp, corp;
//	double	strk;
//	int	ii;
//
//	memset(optf, 0, sizeof(OPTFOLD));
//	if (ctx->func->type == NULL)
//		return(0);
//	for (ii = 0; ii < where->n_folder; ii++)
//	{
//		folder = where->p_folder[ii];
//		ctx->func->type(folder, &styp, &corp, &strk);
//		//if (styp != ST_OPTION)
//		if (styp != 'O')
//			continue;
//		if (strk == 0.)
//			continue;
//		cnp.strk = strk;		// strike price
//		cnp.call = NULL;
//		cnp.put  = NULL;
//		switch (corp)
//		{
//		case 'C': cnp.call = folder; break;
//		case 'P': cnp.put  = folder; break;
//		default:		     continue;
//		}
//		p = bsearch(&cnp, optf->cnp, optf->n_strk, sizeof(struct cnp), cmpstrk);
//		if (p == NULL)
//		{
//			if (optf->n_strk >= 3000)
//				continue;
//			memcpy(&optf->cnp[optf->n_strk++], &cnp, sizeof(struct cnp));
//			qsort(optf->cnp, optf->n_strk, sizeof(struct cnp), cmpstrk);
//		}
//		else
//		{
//			if (cnp.call != NULL)
//				p->call = folder;
//			else
//				p->put = folder;
//		}
//	}
//	return(optf->n_strk);
//}

int mds_lmsfolder(MARKET *market, WHERE *where, const char *prefix)
{
	char	symbol[512], *pptr, *lptr;
	WHERE	w;

	memset(where, 0, sizeof(WHERE));
	strcpy(symbol, prefix);
	for (pptr = symbol; pptr != NULL; pptr = lptr)
	{
		lptr = strchr(pptr, ',');
		if (lptr != NULL)
		{
			*lptr = '\0';
			lptr++;
		}
		mds_seekfolder(market, pptr, &w);
		if (w.l_folder != NULL)
			where->p_folder[where->n_folder++] = w.l_folder;
	}
	return(where->n_folder);
}

//static int cmpstrk(struct cnp *c1, struct cnp *c2)
//{
//	if (c1->strk == c2->strk)
//		return(0);
//	if (c1->strk > c2->strk)
//		return(1);
//	return(-1);
//}

