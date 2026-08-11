//
// shm.c
// Market data management
//
#include <limits.h>
#include "context.h"

static	int	cmpindex();
static	int	cmpfolder();

//
// mds_shminit()
// Initialize shared memory
//
int mds_shminit(MARKET *market, int fsz)
{
	MDCTX	*ctx = market->ctx;
	MDARCH	*arch;
	struct	shmid_ds shmid_ds;
	int	shmid, shmsz, isize, fsize;
	char	*shmad;
	int	new = 0;
	ctx->fsiz = fsz;		// size of folder
	switch (market->whoami)
	{
	case I_AM_READER:
		if ((shmid = shmget(IPCK(market->xchg->ipck, 0), 0, 0666)) < 0)
			return(-1);
		shmad = shmat(shmid, (char *)0, SHM_RDONLY); 
		break;
	case I_AM_WRITER:
		if ((shmid = shmget(IPCK(market->xchg->ipck, 0) , 0, 0666)) < 0)
		{
 			fprintf(stderr, "%s\n", strerror(errno));
			return(-1);
		}
		shmad = shmat(shmid, (char *)0, 0); 
		break;
	case I_AM_COOKER:
		isize = sizeof(INDEX) * market->xchg->room;		// size of index(whereis)
		fsize = ctx->fsiz * market->xchg->room;			// size of folder
		shmsz = sizeof(MDARCH) + isize + fsize;
		if ((shmid = shmget(IPCK(market->xchg->ipck, 0), 0, 0666)) < 0)
		{
			PRINTF("New SHM key=0x%08x", IPCK(market->xchg->ipck, 0));
			if ((shmid = shmget(IPCK(market->xchg->ipck, 0), shmsz, 0666|IPC_CREAT)) < 0)
				return(-1);
			new = 1;
		}
		if (shmctl(shmid, IPC_STAT, &shmid_ds) == 0 && shmid_ds.shm_segsz < shmsz)
		{
			shmid_ds.shm_segsz = shmsz;
#if	(defined(AIX))
			if (shmctl(shmid, SHM_SIZE, &shmid_ds) != 0)
				return(-1);
#else
			if (shmctl(shmid, IPC_SET, &shmid_ds) != 0)
				return(-1);
#endif
#ifdef	LINUX
			if (shmctl(shmid, IPC_STAT, &shmid_ds) == 0)
				shmctl(shmid, SHM_LOCK, &shmid_ds);
#endif
			new = 2;
		}
		shmad = shmat(shmid, (char *)0, 0);
		break;
	default:
		return (-1);
	}
	if (shmad == (char *)-1)
		return(-1);

	arch = (MDARCH *)shmad;
	market->arch = arch;

	ctx->info = &arch->info;				// book on a shared memory
	market->info = &arch->info;

	if (market->whoami != I_AM_COOKER)
	{
		market->indx = &shmad[sizeof(MDARCH)];			// pointer of whereis 
		market->fold = &shmad[sizeof(MDARCH) + (sizeof(INDEX) * arch->mrec)];
		return(0);
	}

	switch (new)
	{
	case 1: // all new one
		memset(shmad, 0, sizeof(shmsz));
		arch->mrec = market->xchg->room;
		break;
	case 2: // enlarge size of shared memory
		arch->mrec = market->xchg->room;
		arch->nrec = 0;
		arch->drec = 0;
		arch->vrec = 0;
		mds_log(market, LOG_MUST, "Shared memory enlarge size of shared memory ");
		break;
	default:
		if (arch->mrec <= 0)
		{
			arch->mrec = market->xchg->room;
			arch->nrec = 0;
			arch->drec = 0;
			arch->vrec = 0;
		}
		break;
	}
	market->indx = &shmad[sizeof(MDARCH)];			// pointer of whereis 
	market->fold = &shmad[sizeof(MDARCH)+(sizeof(INDEX) * arch->mrec)];
	memcpy(&arch->info.xchg, market->xchg, sizeof(XCHG));	// exchange information to shared memory

	if (market->whoami == I_AM_COOKER)
		mds_log(market, LOG_MUST, "Shared memory initialized to %d/%d records", arch->nrec, arch->mrec);
	return(0);
}

//
// mds_shmfold()
// Return folder pointer by key index
//
void *mds_shmfold(MARKET *market, INDEX *w)
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
// mds_shmpos()
// Search position for a new symbol on folder
// Note) If no use INDEX, Use this function for qsort
//
static int mds_shmpos(MARKET *market, const char *symb)
{
	MDCTX	*ctx = market->ctx;
	MDARCH	*arch = market->arch;
	char	*fp, *mp;
	int	f, t, m, r, i;

	f = 0;
	t = arch->vrec;

	fp = market->fold;
	while (t - f > 5)
	{
		m = (f + t) / 2;
		mp = fp + (m * ctx->fsiz);
		r = strcmp(symb, mp);
		if (r == 0)
			return(m);
		if (r > 0)
			f = m;
		else
			t = m;
	}
	for (i = f; i <= t; i++)
	{
		mp = fp + (i * ctx->fsiz);
		if (strcmp(symb, mp) <= 0)
			break;
	}
	return(i);
}

INDEX *mds_shmseek(MARKET *market, const char *symb)
{
	MDARCH	*arch = market->arch;
	INDEX	*indx = market->indx;
	int	plen = strlen(symb);
	int	from, to, mid, pos;
	int	many;
	int	result;
	int	ii;

	from = 0;
	to = arch->nrec;
	while (from < to)
	{
		many = to - from;
		mid = from + (to - from) / 2;
		result = strncmp(symb, indx[mid].symb, plen);
		if (result == 0)
			break;
		if (result < 0)
			to = mid;
		else 
			from = mid + 1;
	}
	if (to <= from)
		return(NULL);

	pos = mid;
	for (ii = mid; ii >= 0; ii--)
	{
		if (strncmp(symb, indx[ii].symb, plen) != 0)
			break;
		pos = ii;
	}
	return(&indx[pos]);
}

//
// mds_shmnew()
// Add a new folder recvord 
// NOTE : No use now, Use index of folder
//
void *mds_shmnew(MARKET *market, const void *fold)
{
	MDCTX	*ctx = market->ctx;
	MDARCH	*arch = market->arch;
	const char *symb = fold;
	char	*fb, *fp, *tp;
	int	shift;
	int	to;

	to = mds_shmpos(market, symb);
	if (to < 0)
		return(NULL);
	fb = market->fold;
	fp = fb + (to * ctx->fsiz);
	tp = fp + ctx->fsiz;
	shift = (arch->vrec - to) * ctx->fsiz;
	memmove(tp, fp, shift);
	memcpy(fp, (char *)fold, ctx->fsiz);
	return(fp);
}

//
// mds_shmadd()
// Get free index of shared memory
//
INDEX *mds_shmadd(MARKET *market, const char *symb)
{
	MDCTX	*ctx  = market->ctx;
	MDARCH	*arch = market->arch;
	INDEX	*indx = market->indx, *now;
	char	*fb = market->fold;
	INDEX	new;

	if ((now = mds_shmget(market, symb)) != NULL)
		return(now);

	if (arch->nrec >= arch->mrec)
		return(NULL);
	if (strlen(symb) <= 0 || strlen(symb) >= sizeof(new.symb))
		return(NULL);

	memset(&new, 0, sizeof(INDEX));
	strcpy(new.symb, symb);				// symbol code

	mds_lock(market);
	if (arch->drec > 0)
	{
		// use deleted folder's record
		new.indx = indx[arch->mrec - arch->drec].indx;
		arch->drec--;
	}
	else
	{
		if (arch->vrec >= arch->mrec)
		{
			mds_unlock(market);
			mds_log(market, LOG_ERROR, "Shared memory full(%s)", __FUNCTION__);
			return(NULL);
		}
		new.indx = arch->vrec++;
	}
	memcpy(&indx[arch->nrec++], &new, sizeof(INDEX));	// set indexes
	fb += (ctx->fsiz * new.indx);
	memset(fb, 0, ctx->fsiz);
	qsort(indx, arch->nrec, sizeof(INDEX), cmpindex);
	mds_unlock(market);
	return(mds_shmget(market, symb));
}

//
// mds_shmdel()
// Delete a folder on shared memry
//
void mds_shmdel(MARKET *market, const char *symb)
{
	MDCTX	*ctx  = market->ctx;
	MDARCH	*arch = market->arch;
	INDEX	*indx = market->indx;
	INDEX	del, *delidx;
	char	*fb = market->fold;
	char	*rp;
	char	*f, *t;
	int	i, l;

	memset(&del, 0, sizeof(INDEX));
	strcpy(del.symb, symb);
	delidx = bsearch(&del, indx, arch->nrec, sizeof(INDEX), cmpindex);
	if (delidx == NULL)
		return;

	mds_lock(market);
	del.indx = delidx->indx;				// folder's room to delete
	fb += (ctx->fsiz * del.indx);				// foler position to delete

	if (ctx->func->roff != NULL)
	{
		if ((rp = ctx->func->roff(market, fb, MSTR)) != NULL)
			mds_isdelete(market, MSTR, rp);
		if ((rp = ctx->func->roff(market, fb, QUOT)) != NULL)
			mds_isdelete(market, QUOT, rp);
		if ((rp = ctx->func->roff(market, fb, BOOK)) != NULL)
			mds_isdelete(market, BOOK, rp);
	}
	memset(fb, 0, ctx->fsiz);				// clear all data on folder
	memset(fb, 'z', SYMB_LEN-1);

	t = (char *)&delidx[0];
	f = (char *)&delidx[1];
	i = (int)(((unsigned long)delidx - (unsigned long)indx) / sizeof(INDEX));
	l = (arch->nrec - i - 1) * sizeof(INDEX);
	memmove(t, f, l);
	arch->nrec--;
	arch->drec++;
	memcpy(&indx[arch->mrec - arch->drec], &del, sizeof(INDEX));
	mds_unlock(market);
}

INDEX *mds_shmget(MARKET *market, const char *symbol)
{
	MDARCH	*arch = market->arch;
	INDEX	*indx = market->indx;
	INDEX	w, *W;

	memset(&w, 0, sizeof(INDEX));
	strcpy(w.symb, symbol);
	W = bsearch(&w, indx, arch->nrec, sizeof(INDEX), cmpindex);
	return(W);
}

int mds_shmadj(MARKET *market)
{
	MDARCH	*arch = market->arch;
	INDEX	*indx = market->indx;
	MDCTX	*ctx  = market->ctx;
	char	*fb   = market->fold;
	int	ii;

	switch (market->whoami)
	{
	case I_AM_COOKER: break;
	default: 	  errno = EACCES; return(-1);
	}
	mds_lock(market);
	if (arch->drec <= 0)
		goto done;
	qsort(fb, arch->vrec, ctx->fsiz, cmpfolder);
	for (ii = 0; ii < arch->nrec; ii++)
		indx[ii].indx = ii;
	arch->vrec = arch->nrec;
	arch->drec = 0;
done:
	mds_unlock(market);
	return(0);
}

static int cmpindex(INDEX *i1, INDEX *i2)
{
	return(strcmp(i1->symb, i2->symb));
}

static int cmpfolder(const char *s1, const char *s2)
{
	return(strcmp(s1, s2));
}
