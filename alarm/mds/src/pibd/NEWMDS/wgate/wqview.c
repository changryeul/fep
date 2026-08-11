#include "mds2.h"
#include "wgate.h"
#include "agque.h"
#include "agdef.h"
#include "comrdb.h"

// Defines ===========================================================

#define		BID0  			q->markup[0].bsmkup[BIDMKUP].prc
#define		ASK0  			q->markup[0].bsmkup[ASKMKUP].prc
#define		BID1  			q->markup[1].bsmkup[BIDMKUP].prc
#define		ASK1  			q->markup[1].bsmkup[ASKMKUP].prc
#define		BID2  			q->markup[2].bsmkup[BIDMKUP].prc
#define		ASK2  			q->markup[2].bsmkup[ASKMKUP].prc
#define		BID3  			q->markup[3].bsmkup[BIDMKUP].prc
#define		ASK3  			q->markup[3].bsmkup[ASKMKUP].prc

// Global Variables ========================================================
char	*whoami="wQuoteGate";
WGMST	*wgmst;
WGREQ	*wgreq;

// Function Define =========================================================
void exitproc(int );
void printreq(int pos, WGREQ *p);

/*===============================================================
 * EXITPROC
===============================================================*/
void exitproc(int exitval)
{
	if (wgmst)	shmdt(wgmst);
	
	printf("\nProgram Terminated.. exitval[%d]\n\n", exitval);
	
	exit(exitval);
}

/*===============================================================
 * MAIN PROCESS
===============================================================*/
int main(int argc, char *argv[])
{
	int		ii, rc, allflag=0;

	/*------------------------------------------------------
	 * SHARED MEMORY ÃÊ±âÈ­
	------------------------------------------------------*/
	int 	shmid;
	int		shmsz;
	
	if (argc > 1 && strcmp(argv[1], "all") == 0)
		allflag = 1;
	
	// SharedMemory : WGMST + WGREQ[MX_WGREQ]
	shmid = shmget(WG_SHMKEY, 0, 0666);
	if (shmid < 0)
	{
		printf("\n(%s) create shared memory error (%d/%s)\n", __func__, errno, strerror(errno));
		exitproc(-5);
	}

	wgmst = (WGMST *)shmat(shmid, NULL, 0);
	if (wgmst == NULL)
	{
		printf("\n(%s) shared memory attach error (%d/%s)\n", __func__, errno, strerror(errno));
		exitproc(-6);
	}
	
	wgreq = (WGREQ *)((char *)wgmst + sizeof(WGMST));
	
	printf("=====================================================\n");
	printf("[%.8s]  UPDTMKUP=[%.6s] MXPOS=[%3d]\n", wgmst->bizdate, wgmst->updtmkup, wgmst->mxreqpos);
	printf("-----------------------------------------------------\n");
	
	WGREQ *preq;
	
	for (ii=0; ii<=wgmst->mxreqpos; ii++)
	{
		preq = (WGREQ *)(wgreq + ii);

		if (preq->useyn != DF_ON && allflag == 0)	continue;
		
		if (preq->reqID[0] == 0x00)	continue;
		
		printreq(ii, preq);
	}

	exitproc(0);
}

void printreq(int pos, WGREQ *p)
{
	int		ii;
	struct tm	*tm1, *tm2, *tm;
	WGKEY	*k = &p->wgkey;

	tm = localtime(&p->quot[0].qtm);
	tm1 = localtime(&p->quot[1].qtm);
	tm2 = localtime(&p->quot[2].qtm);

	printf("[%03d] use=%d (%.32s/%.32s) (%s/%.20s) (%.1s/%.6s/%.6s) bid/ask(%.*f:%.*f) mkup(%.6s) qid(%s)\n", pos, 
		p->useyn, k->sendcompid, k->sendsubid, (p->req.reqtp[0]==REQTP_ABT?"ABT":(p->req.reqtp[0]==REQTP_API?"API":(p->req.reqtp[0]==REQTP_RFS?"RFS":"RFQ"))),
		p->reqID, (p->stype[0]==DF_FXSPT?"SPT":(p->stype[0]==DF_FXFWD?"FWD":(p->stype[0]==DF_FXSWP?"SWP":"N/A"))), 
		p->symb, p->symb4r, p->zCustdiv, p->quot[0].bid/pow(10, p->zCustdiv), p->zCustdiv, p->quot[0].ask/pow(10, p->zCustdiv),
		p->updtmkup, p->quot[0].quotID);

	printf("  RQ) id=(%.20s)  mrgn=(%g)  mrkupgrp=(%.3s)  rfs(%.15s/%.f/%.1s)  tm=(%.2s:%.2s:%.2s ~ %.2s:%.2s:%.2s)\n", 
		p->reqID, p->margin, p->mkupgroup, p->req.rfskey, p->req.orderqty, p->req.side,
		p->regtime, &p->regtime[2], &p->regtime[4], p->req.etime, &p->req.etime[2], &p->req.etime[4]);
		
	for (ii=0; ii<4; ii++)
	{
		if (p->markup[ii].symb[0] == 0x00)	continue;

		printf("\t[MK%d] %.6s(%.8s) last(%g:%g) org(%g:%g) bidmm(%g/%g/%g) askmm(%g/%g/%g)\n", ii, 
			p->markup[ii].symb, p->markup[ii].setldate, p->markup[ii].bsmkup[0].prc, p->markup[ii].bsmkup[1].prc, p->markup[ii].bsmkup[0].orgprc, p->markup[ii].bsmkup[1].orgprc, 
			p->markup[ii].bsmkup[0].spotmkup, p->markup[ii].bsmkup[0].swapmkup, p->markup[ii].bsmkup[0].swap,
			p->markup[ii].bsmkup[1].spotmkup, p->markup[ii].bsmkup[1].swapmkup, p->markup[ii].bsmkup[1].swap);
	}
	printf("--------------------------------------------\n");
	return ;
}
