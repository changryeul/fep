#include "mds2.h"
#include "wgate.h"
#include "agque.h"
#include "agdef.h"
#include "comrdb.h"

// Global Variables ========================================================
char	*whoami="wqmng";
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
	int		ii, rc;

	/*------------------------------------------------------
	 * SHARED MEMORY √ ±‚»≠
	------------------------------------------------------*/
	int 	shmid;
	int		shmsz;
	
	// SharedMemory : WGMST + WGREQ[MX_WGREQ]
	shmid = shmget(WG_SHMKEY, 0, 0);
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

	WGREQ *preq;

	int		iival;	
	char 	item[32], ival[32];

	while (1) 
	{
		memset(item, 0x00, sizeof(item));
		memset(ival, 0x00, sizeof(ival));

		printf("=====================================================\n");
		printf("[%.8s]  markup=[%.6s] mxpos=[%3d]\n", wgmst->bizdate, wgmst->updtmkup, wgmst->mxreqpos);
		printf("-----------------------------------------------------\n");

		for (ii=0; ii<=wgmst->mxreqpos; ii++)
		{
			preq = (WGREQ *)(wgreq + ii);
    	
			if (preq->useyn != DF_ON)	continue;
			
			printreq(ii, preq);
		}

		printf("\nitem (m:markup/r:req/q:quit) : ");
		scanf("%s", item);
		if (item[0] == 'q')		break;
		else if (item[0] == 'm') {
			printf("markup tm(HHMMSS) : ");
			scanf("%s", ival);
			l_rtrim(ival);
			if (strlen(ival) != sizeof(wgmst->updtmkup))	{ printf("wrong input..\n");	continue; }
			memcpy(wgmst->updtmkup, ival, sizeof(wgmst->updtmkup));
		}
		else if (item[0] == 'r') {
			printf("req seq : ");
			scanf("%d", &iival);
			if (iival < 0 || iival > wgmst->mxreqpos)	{ printf("wrong input..\n");	continue; }
			
			preq = (WGREQ *)(wgreq + iival);
			printreq(iival, preq);
			
			printf("\n item(mrgn/mkup/etime) : ");
			scanf("%s", item);
			printf("value : ");
			scanf("%s", ival);

			l_rtrim(item);
			l_rtrim(ival);

			if (strcmp(item, "mrgn") == 0)	preq->margin = atof(ival);
			else if (strcmp(item, "mkup") == 0)		{
				if (strlen(ival) == sizeof(preq->updtmkup))
					memcpy(preq->updtmkup, ival, sizeof(preq->updtmkup));
			}
			else if (strcmp(item, "etime") == 0)		{
				if (strlen(ival) == sizeof(preq->req.etime))
					memcpy(preq->req.etime, ival, sizeof(preq->req.etime));
			}
		}
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

	printf("seq=%3d] use=%d (%.15s/%.15s) (%s/%.30s) (%.1s/%.6s/%.6s) bid/ask(%.*f:%.*f) mkup(%.6s) qid(%s)\n", pos, 
		p->useyn, k->sendcompid, k->sendsubid, (p->req.reqtp[0]==REQTP_ABT?"ABT":(p->req.reqtp[0]==REQTP_API?"API":(p->req.reqtp[0]==REQTP_RFS?"RFS":"RFQ"))),
		p->reqID, (p->stype[0]==DF_FXSPT?"SPT":(p->stype[0]==DF_FXFWD?"FWD":(p->stype[0]==DF_FXSWP?"SWP":"N/A"))), 
		p->symb, p->symb4r, p->zCustdiv, p->quot[0].bid/pow(10, p->zCustdiv), p->zCustdiv, p->quot[0].ask/pow(10, p->zCustdiv),
		p->updtmkup, p->quot[0].quotID);

	printf("  RQ) id=%s  mrgn=%.6f  qty=%.f  side=%.1s  tm=(%.2s:%.2s:%.2s ~ %.2s:%.2s:%.2s)\n", 
		p->reqID, p->margin, p->req.orderqty, p->req.side,
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
