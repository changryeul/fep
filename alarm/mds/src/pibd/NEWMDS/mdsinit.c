/*#####################################################################
//#######################################################################*/
#define	MDSTEST	1

#include "mds2.h"

MARKET	*market=NULL;

char	*whoami="mdsinit";

/*===============================================================
 * EXITPROC
===============================================================*/
void exitproc(int exitval)
{
	if (market)
	{
		//pthread_mutex_lock(&market->ctx.lock);
		shmdt(market->arch);
		//pthread_mutex_unlock(&market->ctx.lock);
		
		free(market);
	}

	printf("\nProgram Terminated.. exitval[%d]\n\n", exitval);
	
	exit(exitval);
}

/*===============================================================
 * MAIN PROCESS
===============================================================*/
int main(int argc, char *argv[])
{
	int		rtn;	
	char	msg[128];

	/*===============================================================
	 * PROCESS 초기화
	===============================================================*/
	if (argc < 2)
	{
		printf("\n\t Usage : %s [SMBS/KMB/CMBS/EBS/CUST/BEST] \n", argv[0]);
		exitproc(-1);
	}

	if ((market = (MARKET *)malloc(sizeof(MARKET))) == NULL)
	{
		printf("\n\tMarket alloc error\n");
		return(NULL);
	}

	memset(market, 0x00, sizeof(MARKET));

	strcpy(market->procname, whoami);
	strcpy(market->exnm, argv[1]);

	/*===============================================================
	 * 원천별 초기화
	  > BEST 인 경우는 DB 데이터가 없으므로 초기화 로직 재정의 필요
	===============================================================*/
	MDARCH	*arch=NULL;

	memset(msg, 0x00, sizeof(msg));
	
	if ((arch = mds_getarch(1, market->exnm, msg)) == NULL)
	{
		mds_log(market, MLOG_ERROR, "Cannot attach SHM '%s'", market->exnm);
		exitproc(-1);
	}

	mds_log(market, MLOG_MUST, "Start shared memory RESET for '%s'", market->exnm);

	market->excode[0] = arch->xchg.excode[0];
	market->exid = arch->xchg.exid;
	market->arch = arch;
	market->indx = (INDEX *)((char *)arch + sizeof(MDARCH));
	market->fold = (MDFOLD *)((char *)arch + sizeof(MDARCH) + (sizeof(INDEX)*arch->xchg.maxcnt));
	market->fptr = market->fold;
	arch->rsum   = 0L;

	rtn = LoadMaster(market);

	mds_log(market, MLOG_MUST, "[%s] Master SHM Initialize for [%s:%c] is DONE.. Load Count=[%d:%d] [%08d]", __func__, 
				arch->xchg.exnm, arch->xchg.excode[0], arch->nrec, rtn, arch->tymd);
				
	printf("\n\tMaster SHM Initialize for [%s:%c] is DONE.. Load Count=[%d:%d] [%08d]\n",
				arch->xchg.exnm, arch->xchg.excode[0], arch->nrec, rtn, arch->tymd);

	exitproc(0);
}
