//
// intr.c
// update for last INTR
//
#include <dirent.h>
#include "context.h"
#include "mdfold.h"

int get_intr(MARKET *market, MDFOLD *folder, MDINTR *out)
{
	int	many;
	int	ii, jj;
	int	record = 0;
	MDMSTR *m = &folder->mstr;
	MDINTR	intr[10];
	struct	timeval s, e;

	memset(&intr, 0x00, sizeof(MDINTR));
	strcpy(intr[0].symb, folder->symb);
	intr[0].xymd = 99999999;
	intr[0].xhms = m->kfrhm*100;

	for (;;)
	{
		many = mds_fetch(market, INTR, &intr[0], ISGREAT, 1, 1);
		if (many <= 0)
			return(0);

		mds_log(market, LOG_DEBUG, "[%s] %08d %06d symd:%08d", __func__, intr[0].xymd, intr[0].xhms, m->p.symd);
		if (intr[0].tymd > m->p.symd)
			continue;

		if (intr[0].tymd <= m->p.symd)
			break;
	}
	memcpy(out, &intr[0], sizeof(MDINTR));
	return(0);
}

int update_intr(MARKET *market, MDFOLD *folder, double setp)
{
	int	many;
	int	ii, jj;
	int	record = 0;
	MDFOLD	*fold = folder;
	MDINTR	*ip = &fold->intr;
	MDMSTR	*m  = &fold->mstr;
	MDINTR	intr;
	int		rc;

	memset(&intr, 0x00, sizeof(MDINTR));
	get_intr(market, folder, &intr);

	if (strlen(intr.symb) == 0)
		return(0);

	if ((intr.tymd == m->p.symd) && (intr.clos != setp))
	{

		// Update 1 minute candle data on shared memory
		if (ip->xymd == intr.xymd && ip->xhms == intr.xhms)
		{
			mds_log(market, LOG_DEBUG, "shm update : symb=%s %08d %06d  clos:%f => %f ", 
				ip->symb, ip->xymd, ip->xhms, ip->clos, setp);

			if (ip->high < setp)
				ip->high = setp;
			if (ip->low <= 0. || ip->low > setp)
				ip->low = setp;
			ip->clos = setp;
			rc = mds_isupsert(market, INTR, ip);
			if (rc < 0)
			{
				//mds_log(market, LOG_ERROR, "mds_isupsert error.(shm) E:%d ", iserrno);
				return(-1);
			}
			return(0);
		}
		mds_log(market, LOG_DEBUG, "intr update: symb=%s %08d %06d  clos:%f => %f ", 
			intr.symb, intr.xymd, intr.xhms, intr.clos, setp);

		// update isam file
		if (intr.high < setp)
			intr.high = setp;
		if (intr.low <= 0. || intr.low > setp)
			intr.low = setp;
		intr.clos = setp;
		rc = mds_isupsert(market, INTR, &intr);
		if (rc < 0)
		{
			//mds_log(market, LOG_ERROR, "mds_isupsert error. E:%d ", iserrno);
			return(-1);
		}
	}

	return(0);
}
