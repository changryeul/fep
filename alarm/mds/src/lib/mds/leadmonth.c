#include <stdio.h>
#include <stdlib.h>
#include "context.h"
#include "stream.h"
#include "mdfold.h"

static MDFOLD *clr_leadmonth(MARKET *market, const char *symb)
{
	MDFOLD	*folder;

	folder = mds_getfolder(market, symb);
	if (folder != NULL)
	{
		folder->mstr.jchk &= ~JCHK_LM;
		mds_syncfolder(market, folder, MSTR);
	}
	return(folder);
}

static MDFOLD *set_leadmonth(MARKET *market, const char *symb)
{
	MDFOLD	*folder;
	folder = mds_getfolder(market, symb);
	if (folder != NULL)
	{
		folder->mstr.jchk |= JCHK_LM;
		mds_syncfolder(market, folder, MSTR);
	}
	return(folder);
}

//
// mds_leadmonth_symbol()
// initialize lead month symbol
//
void mds_leadmonth_symbol(MARKET *market)
{
	MDFOLD	*mdfold, *lmfold, *lf, *zf, *l_fold;
	SYMBOL	*symbol;
	WHERE	where;
	uint32_t exym, xymd;
	int	nsymb;
	char	csymb[SYMB_LEN], lsymb[SYMB_LEN], osymb[SYMB_LEN];
	char	*f, *t;
	int	l;
	int	li, sl, ok, nn, zi;
	int	ii, jj;

	if (market->xchg->type != XT_FUTURE)
		return;

	mds_time(market, 0, &xymd, NULL, NULL, NULL);
	exym = (xymd / 100) * 100;			// YYYYMM00

	symbol = mds_symbol(market, &nsymb);
	for (ii = 0; ii < nsymb; ii++)
	{
		if (symbol[ii].styp != XT_FUTURE)
			continue;

		sl = strlen(symbol[ii].symb);
		jj = 0; 
		li = -1; 
		zi = -1; 
		ok = -1, 
		nn = 0;
		lf = zf = NULL;

		mds_seekfolder(market, symbol[ii].symb, &where);
		l_fold = where.l_folder;
		for (jj = 0 ; jj < where.n_folder; jj++)
		{
			MDFOLD *f = where.p_folder[jj];
			// 20171030 add
			if (l_fold != NULL)
			{
				if (f->mstr.exym < l_fold->mstr.exym)
					continue;
			}

			if (!(f->mstr.symb[sl] >= 'A' && f->mstr.symb[sl] <= 'Z'))
				continue;	// continuous symbol

			if (f->mstr.exym < exym)	// expired ...
				continue;
			if (f->mstr.zymd < xymd)
				continue;
			nn++;

			if (f->mstr.jchk & JCHK_LM)
				ok = jj;
			if (zi == -1 || (zf != NULL && (f->mstr.exym < zf->mstr.exym)))
				zi = jj;
			if (li == -1 || (lf != NULL && (f->quot.tvol > lf->quot.tvol)))
			//if (li == -1 || (lf != NULL && (f->mstr.p.tvol > lf->mstr.p.tvol)))
				li = jj;

			zf = where.p_folder[zi];
			lf = where.p_folder[li];
		}
		if (!nn)
			continue;
		
		//if (li < 0 || !nn || lf->mstr.p.tvol <= 0)
		if (li < 0 || !nn || lf->quot.tvol <= 0)
		{
			if (zi < 0)
				continue;
			li = zi;
			lf = zf;
		}

		if (ok >= 0)
		{
			lf = where.p_folder[ok];
			strcpy(osymb, lf->mstr.symb);
		}
		lf = where.p_folder[li];
		strcpy(lsymb, lf->mstr.symb);				// leadmonth symbol
		sprintf(csymb, "%s%s", symbol[ii].symb, LM_SUFFIX);	// continuous symbol (ex:ES.1)
		mdfold = mds_getfolder(market, csymb);
		if (mdfold == NULL)
		{
			mdfold = mds_newfolder(market, csymb);
			if (mdfold == NULL)
				continue;
			if (ok >= 0)
			{
				if ((lmfold = set_leadmonth(market, osymb)) != NULL)
				{
					mds_log(market, LOG_MUST, "[%4d][%-15s]set leadmonth. lsymb[%s] osymb[%s] exym[%d]T[%.f] ", 
						__LINE__, __func__, lsymb, osymb, lf->mstr.exym, lf->quot.tvol);
					f  = (char *)&lmfold->mstr.ecym[0];
					t  = (char *)&mdfold->mstr.ecym[0];
					l  = sizeof(MDMSTR) - ((long)&mdfold->mstr.ecym[0] - (long)&mdfold->mstr);
					memcpy(t, f, l);
					sprintf(mdfold->mstr.enam, "%s Continuous", symbol[ii].enam);
					sprintf(mdfold->mstr.snam, "%s Con", symbol[ii].enam);
					sprintf(mdfold->mstr.knam, "%s 연속", symbol[ii].knam);
					sprintf(mdfold->mstr.isym, "%s", mdfold->mstr.symb);
					sprintf(mdfold->mstr.inrt, "%s", lmfold->mstr.inrt);
					sprintf(mdfold->mstr.root, "%s", lmfold->mstr.root);

					mdfold->mstr.zymd = 99999999;
					mds_syncfolder(market, mdfold, MSTR);
				}
				continue;
			}
		}
		if (ok != -1 && strcmp(osymb, lsymb))
		{
			clr_leadmonth(market, osymb);
			mds_log(market, LOG_MUST, "[%4d][%-15s]clear leadmonth. [%s] ", __LINE__, __func__, osymb);
		}

		if ((lmfold = set_leadmonth(market, lsymb)) == NULL)
			continue;
		
		mds_log(market, LOG_MUST, "[%4d][%-15s]set leadmonth. [%s][%s][%s][%s] exym[%d]T[%.f] ", 
			__LINE__, __func__, lsymb, mdfold->mstr.symb,  mdfold->mstr.inrt,lmfold->mstr.symb,  lmfold->mstr.exym, lmfold->quot.tvol);
		lmfold->mstr.jchk |= JCHK_LM;
		mds_syncfolder(market, lmfold, MSTR);

		f  = (char *)&lmfold->mstr.ecym[0];
		t  = (char *)&mdfold->mstr.ecym[0];
		l  = sizeof(MDMSTR) - ((long)&mdfold->mstr.ecym[0] - (long)&mdfold->mstr);
		memcpy(t, f, l);
		sprintf(mdfold->mstr.enam, "%s Continuous", symbol[ii].enam);
		sprintf(mdfold->mstr.snam, "%s Con", symbol[ii].enam);
		sprintf(mdfold->mstr.knam, "%s 연속", symbol[ii].knam);
		// 2017.08.04 add
		sprintf(mdfold->mstr.isym, "%s", mdfold->mstr.symb);
		sprintf(mdfold->mstr.inrt, "%s", lmfold->mstr.inrt);
		sprintf(mdfold->mstr.root, "%s", lmfold->mstr.root);
		mdfold->mstr.zymd = 99999999;
		mds_syncfolder(market, mdfold, MSTR);
	}
}

//
// mds_leadmonth_update()
// Update leadmonth symbol
//
void mds_leadmonth_update(MARKET *market, char *symb)
{
	MDFOLD	*mdfold, *lmfold, *lf, *zf, *l_fold;
	SYMBOL	*symbol;
	WHERE	where;
	uint32_t exym, xymd;
	int	nsymb;
	char	csymb[SYMB_LEN], lsymb[SYMB_LEN], psymb[SYMB_LEN];
	char	*f, *t;
	int	l;
	int	li, sl, pi, nn, zi;
	int	ii, jj;

	if (market->xchg->type != XT_FUTURE)
		return;
	//mds_time(market, 0, &xymd, NULL, NULL, NULL);
	mds_time(market, 0, NULL, NULL, &xymd, NULL);
	exym = (xymd / 100) * 100;			// YYYYMM00

	if (symb == NULL)
	{
		symbol = mds_symbol(market, &nsymb);
	}
	else
	{
		symbol = mds_symbget(market, symb);
		if (symbol == NULL)
		{
			return;
		}
		nsymb = 1;
	}
	//printf("nsymb:%d \n", nsymb);
	for (ii = 0; ii < nsymb; ii++)
	{
		if (symbol[ii].styp != XT_FUTURE)
			continue;
		sl = strlen(symbol[ii].symb);

		jj = 0; 
		li = -1; 
		zi = -1; 
		pi = -1; 
		nn = 0;
		lf = zf = NULL;

		mds_seekfolder(market, symbol[ii].symb, &where);
		l_fold = where.l_folder;
		for (jj = 0 ; jj < where.n_folder; jj++)
		{
			MDFOLD *f = where.p_folder[jj];
			// 20171030 add
			if (l_fold != NULL)
			{
				if (f->mstr.exym < l_fold->mstr.exym)
					continue;
			}
			//printf("%s \n", f->symb);

			if (!(f->mstr.symb[sl] >= 'A' && f->mstr.symb[sl] <= 'Z'))
				continue;	// continuous symbol

			if (f->mstr.exym < exym)	// expired ...
				continue;
			if (f->mstr.zymd < xymd)
				continue;
			nn++;

			if (f->mstr.jchk & JCHK_LM)
				pi = jj;
			if (zi == -1 || (zf != NULL && (f->mstr.exym < zf->mstr.exym)))
				zi = jj;
			if (li == -1 || (lf != NULL && (f->quot.tvol > lf->quot.tvol)))
			//if (li == -1 || (lf != NULL && (f->mstr.p.tvol > lf->mstr.p.tvol)))
				li = jj;

			zf = where.p_folder[zi];
			lf = where.p_folder[li];

		}
		if (!nn)
			continue;

		//if (li < 0 || !nn || lf->mstr.p.tvol <= 0)
		if (li < 0 || !nn || lf->quot.tvol <= 0)
		{
			if (zi < 0)
				continue;
			li = zi;
			lf = zf;
		}
		if (pi != -1)
		{
			mdfold = where.p_folder[pi];
			strcpy(psymb, mdfold->symb);
		}
		if (li != -1)
		{
			mdfold = where.p_folder[li];
			strcpy(lsymb, mdfold->symb);

		}
			
		sprintf(csymb, "%s%s", symbol[ii].symb, LM_SUFFIX);
		mdfold = mds_getfolder(market, csymb);
		if (mdfold == NULL)
		{
			mdfold = mds_newfolder(market, csymb);
			if (mdfold == NULL)
				continue;
		}
		if (pi != -1 && pi != li)
		{
			clr_leadmonth(market, psymb);
			mds_log(market, LOG_MUST, "[%4d][%-15s]clear leadmonth. [%s] ", __LINE__, __func__, psymb);
		}
		
		if ((lmfold = set_leadmonth(market, lsymb)) != NULL)
		{
			mds_log(market, LOG_MUST, "[%4d][%-15s]set leadmonth. [%s] ", __LINE__, __func__, lsymb);
			f  = (char *)&lmfold->mstr.ecym[0];
			t  = (char *)&mdfold->mstr.ecym[0];
			l  = sizeof(MDMSTR) - ((long)&mdfold->mstr.ecym[0] - (long)&mdfold->mstr);
			memcpy(t, f, l);
			sprintf(mdfold->mstr.enam, "%s Continuous", symbol[ii].enam);
			sprintf(mdfold->mstr.snam, "%s Con", symbol[ii].enam);
			sprintf(mdfold->mstr.knam, "%s 연속", symbol[ii].knam);
			// 2017.08.04 add
			sprintf(mdfold->mstr.isym, "%s", mdfold->mstr.symb);
			sprintf(mdfold->mstr.inrt, "%s", lmfold->mstr.inrt);
			sprintf(mdfold->mstr.root, "%s", lmfold->mstr.root);
			mdfold->mstr.zymd = 99999999;
			mds_syncfolder(market, mdfold, MSTR);
		}
	}
}

void mds_t1_leadmonth_update(MARKET *market, char *symb)
{
	MDFOLD  *mdfold, *lmfold, *lf, *zf, *l_fold;
	SYMBOL  *symbol;
	WHERE   where;
	uint32_t exym, xymd;
	int nsymb;
	char    csymb[SYMB_LEN], lsymb[SYMB_LEN], psymb[SYMB_LEN];
	char    *f, *t;
	int l;
	int li, sl, pi, nn, zi;
	int ii, jj;


	if (market->xchg->type != XT_FUTURE)
		return;

	mds_time(market, 0, NULL, NULL, &xymd, NULL);
	exym = (xymd / 100) * 100;          // YYYYMM00

	if (symb == NULL)
	{
		symbol = mds_symbol(market, &nsymb);
	}
	else
	{
		symbol = mds_symbget(market, symb);
		if (symbol == NULL)
		{
			return;
		}
			nsymb = 1;
	}

	for (ii = 0; ii < nsymb; ii++)
	{
		if (symbol[ii].styp != XT_FUTURE)
			continue;
		sl = strlen(symbol[ii].symb);

		jj = 0;
		li = -1;
		zi = -1;
		pi = -1;
		nn = 0;
		lf = zf = NULL;

		mds_seekfolder(market, symbol[ii].symb, &where);
		l_fold = where.l_folder;
		for (jj = 0 ; jj < where.n_folder; jj++)
		{
			MDFOLD *f = where.p_folder[jj];

			if (l_fold != NULL)
			{
				if (f->mstr.exym < l_fold->mstr.exym) // cmd/lmonupdate 실행하여 최근월물 정정할때 이부분을 막아야 이미최근월물지나간 종목까지 체크할수 있다
					continue;
			}

			if (!(f->mstr.symb[sl] >= 'A' && f->mstr.symb[sl] <= 'Z'))
				continue;   // continuous symbol

			if (f->mstr.exym < exym)    // expired ...
				continue;
			if (f->mstr.zymd < xymd) 
				continue;
			nn++;

			if (f->mstr.jchk & JCHK_LM)
				pi = jj;

			if (f->mstr.zymd <= xymd)
			{
				//mds_log(market, LOG_MUST, " expired day change [%s] today [%s] zymd[%d]", f->mstr.symb, f->mstr.zymd, xymd); 
				continue;
			}

			if (zi == -1 || (zf != NULL && (f->mstr.exym < zf->mstr.exym)))
				zi = jj;
			if (li == -1 || (lf != NULL && (f->quot.tvol > lf->quot.tvol)))
				li = jj;


			zf = where.p_folder[zi];
			lf = where.p_folder[li];


			mds_log(market, LOG_MUST, "[%4d][%-15s] change info [%d][%d][%d][%d][%.f][%.f][%s][%s]", __LINE__, __func__, jj,pi,zi,li,f->quot.tvol, lf->quot.tvol, f->mstr.symb, zf->mstr.symb);
		}
		if (!nn)
		continue;

		if (li < 0 || !nn || lf->quot.tvol <= 0)
		{
			if (zi < 0)
				continue;
			li = zi;
			lf = zf;
		}
		if (pi != -1)
		{
			mdfold = where.p_folder[pi];
			strcpy(psymb, mdfold->symb);
		}
		if (li != -1)
		{
			mdfold = where.p_folder[li];
			strcpy(lsymb, mdfold->symb);

			mds_log(market, LOG_MUST, "[%4d][%-15s] shift symb. [%s] [%d]", __LINE__, __func__, lsymb, li);
		}

		sprintf(csymb, "%s%s", symbol[ii].symb, LM_SUFFIX);
		mdfold = mds_getfolder(market, csymb);
		if (mdfold == NULL)
		{
			mdfold = mds_newfolder(market, csymb);
			if (mdfold == NULL)
				continue;
		}
		if (pi != -1 && pi != li)
		{
			clr_leadmonth(market, psymb);
			mds_log(market, LOG_MUST, "[%4d][%-15s]clear leadmonth. [%s] ", __LINE__, __func__, psymb);
		}

		if ((lmfold = set_leadmonth(market, lsymb)) != NULL)
		{
			mds_log(market, LOG_MUST, "[%4d][%-15s]set leadmonth. [%s] ", __LINE__, __func__, lsymb);
			f  = (char *)&lmfold->mstr.ecym[0];
			t  = (char *)&mdfold->mstr.ecym[0];
			l  = sizeof(MDMSTR) - ((long)&mdfold->mstr.ecym[0] - (long)&mdfold->mstr);
			memcpy(t, f, l);
			sprintf(mdfold->mstr.enam, "%s Continuous", symbol[ii].enam);
			sprintf(mdfold->mstr.snam, "%s Con", symbol[ii].enam);
			sprintf(mdfold->mstr.knam, "%s 연속", symbol[ii].knam);

			sprintf(mdfold->mstr.isym, "%s", mdfold->mstr.symb);
			sprintf(mdfold->mstr.inrt, "%s", lmfold->mstr.inrt);
			sprintf(mdfold->mstr.root, "%s", lmfold->mstr.root);
			mdfold->mstr.zymd = 99999999;
			mds_syncfolder(market, mdfold, MSTR);
		}
	}
}
