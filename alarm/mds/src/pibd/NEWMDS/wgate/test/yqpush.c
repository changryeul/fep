#include "mds2.h"
#include "wfaapi.h"
#include "../wgate.h"

int sndcnt=1;

void *qpush(void *);

/*===============================================================
 * MAIN PROCESS
===============================================================*/
int main(int argc, char *argv[])
{
	int		pos;
	int		rc, seq=0;
	int		slen;
	pid_t	pid;
	char	sbuf[1024];

	char	*dpath  = "/fsfxwin/wfa";
	char	*ddir	= "dat";
	char	*dfnam	= "YARIQ";
	int		semk	= 0x16000351;
	int		dsiz	= 1024;

	AG_DQUE dque;
	AG_DQUECLS(&dque);

	dque.xha |= AQ_XHA_OPN;
	dque.xha |= AG_XAS_LIV;
/*
//int agdque_wrn (Ag_Dque dque, char *dpath, char *ddir, char *dfnam, int semk, int rsiz, char *pbuf, int plen)
rc = agdque_wrn (&q_rionm, g_dpath, g_ddir, g_rionm, k_rionm, z_irqsz, rbuf, rlen);
*/
	while (1)
	{
		memset(sbuf, 0x00, sizeof(sbuf));

		printf("sendbuf : ");
		scanf("%s", sbuf);
		slen = strlen(sbuf);

		//rc = agdque_wrn (&dque, dpath, ddir, dfnam, semk, dsiz, sbuf, slen);
		rc = agdque_wrn (&dque, WG_DAT_DIR, NULL, DQ_RCVRQ, SEM_RCVRQ, dsiz, sbuf, slen);
		if (rc <= 0)
		{
			printf("agdque_wrn error [%d]\n", rc);
			break;
		}

		//printf("write:%d) [%s]\n", seq, sbuf);
	}

	//printf("write:%d) [%s]\n", seq, sbuf);

	exit(0);
}
