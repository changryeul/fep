
#include "mds2.h"
#include <curses.h>

int		excnt=0;

MDFOLD	*SP=NULL, *CP=NULL, *KP=NULL, *EP=NULL, *ZP=NULL, *BP=NULL, *RP=NULL;
MDARCH	*SA=NULL, *CA=NULL, *KA=NULL, *EA=NULL, *ZA=NULL, *BA=NULL, *RA=NULL;

static key_t get_ipck2(int exid)
{
    char    name[30], decimal;
    int     hexa;
    key_t   ipck;
    int     ii;

    sprintf(name, "%03d", exid);
    for (ii = 0, hexa = 0; ii < strlen(name) && ii < 3; ii++)
    {
        hexa <<= 4;
        decimal = name[ii] & 0x0f;
        hexa |= decimal;
    }
    hexa |= 0x9000;
    ipck = (hexa << 16);                // 0x9{EXID}??##

    return(ipck);
}

int get_mast()
{
	int		ii=0, nxml;
	char	xmlpath[128], args[52];
	struct	xmltag xmltags[256];

	struct EX {
		char	excode	[ 1];		// 원천 : 'S'=SMB, 'K'=KMB, 'E'=EBS, 'C'=CMB
		char	exnm	[ 8];
		int		ipck	;
		MDARCH	*arch	;			// MDARCH
		INDEX	*indx	;			// INDEX
		MDFOLD	*fold	;			// FOLD
	} ;
	struct EX	exch[10];

	/*===============================================================
	 * XML파일(exchanges.cfg) 에서 원천 리스트 정보 READ
	===============================================================*/	
	memset(xmltags, 0x00, sizeof(xmltags));

	sprintf(xmlpath, "/fsfxwin/mds/etc/exchanges.new.cfg");
	nxml = getxmlcfg(xmlpath, xmltags);

	if (nxml <= 0)	return (-1);

	int		exid;
	char	exnm[32];
	memset(&exch, 0x00, sizeof(exch));

	for (ii=0, excnt=0; ii < nxml; ii++)
	{
		memset(exnm, 0x00, sizeof(exnm));

		getargs(&xmltags[ii], "name",  exnm);
		getargs(&xmltags[ii], "exid",  args);
		exid = atoi(args);

		if (exid <= 0 || strlen(exnm) <= 0)
			continue;

		exch[excnt].ipck = get_ipck2(exid);
		strcpy(exch[excnt].exnm, exnm);

		excnt++;
	}

	if (excnt <= 0) {
		printf(" 마스터메모리 정보 오류..\n");
		return (-1);
	}

	/*===============================================================
	 * 각 원천별 SHM 메모리 ATTACH
	===============================================================*/	
	MDARCH	*arch;
	int		shmid;

	for (ii=0, excnt=0; ii<MAX_XCHG; ii++)
	{
		if (strlen(exch[ii].exnm) <= 0)		break;

		if (exch[ii].arch && exch[ii].ipck)	{
			excnt++;
			continue;
		}

		if ((shmid = shmget(IPCK(exch[ii].ipck, 0), 0, 0666)) < 0)
			continue;

		arch = (MDARCH *)shmat(shmid, (char *)0, SHM_RDONLY);
		if (!arch)	continue;

			 if (arch->xchg.excode[0] == 'S')	{ SP = (MDFOLD *)((char *)arch + sizeof(MDARCH) + (sizeof(INDEX)*arch->xchg.maxcnt)); SA = arch; }
		else if (arch->xchg.excode[0] == 'E')	{ EP = (MDFOLD *)((char *)arch + sizeof(MDARCH) + (sizeof(INDEX)*arch->xchg.maxcnt)); EA = arch; }
		else if (arch->xchg.excode[0] == 'C')	{ CP = (MDFOLD *)((char *)arch + sizeof(MDARCH) + (sizeof(INDEX)*arch->xchg.maxcnt)); CA = arch; }
		else if (arch->xchg.excode[0] == 'K')	{ KP = (MDFOLD *)((char *)arch + sizeof(MDARCH) + (sizeof(INDEX)*arch->xchg.maxcnt)); KA = arch; }
		else if (arch->xchg.excode[0] == 'Z')	{ ZP = (MDFOLD *)((char *)arch + sizeof(MDARCH) + (sizeof(INDEX)*arch->xchg.maxcnt)); ZA = arch; }
		else if (arch->xchg.excode[0] == 'B')	{ BP = (MDFOLD *)((char *)arch + sizeof(MDARCH) + (sizeof(INDEX)*arch->xchg.maxcnt)); BA = arch; }
		else if (arch->xchg.excode[0] == 'R')	{ RP = (MDFOLD *)((char *)arch + sizeof(MDARCH) + (sizeof(INDEX)*arch->xchg.maxcnt)); RA = arch; }
	}

	if (!SP)	{ printf("shm attach error : SMBS\n");	return (-1); }
	if (!EP)	{ printf("shm attach error : EBS\n");	return (-1); }
	if (!CP)	{ printf("shm attach error : CMBS\n");	return (-1); }
	if (!KP)	{ printf("shm attach error : KMB\n");	return (-1); }
	if (!ZP)	{ printf("shm attach error : CUST\n");	return (-1); }
	if (!BP)	{ printf("shm attach error : BEST\n");	return (-1); }
	if (!RP)	{ printf("shm attach error : REUT\n");	return (-1); }

	return 0;	
}

int main(int argc, char **argv)
{
	int		ii, rtn, term=2, rnd=0, tot;
	char	iexnm[32], iexcd[32], isymb[32];
	char	msg[512];
	MDFOLD	*sp=NULL, *cp=NULL, *kp=NULL, *ep=NULL, *zp=NULL, *bp=NULL, *rp=NULL;

	if (get_mast() < 0)	return 0;

	char	sbuf[512];

	printf("\033[H\033[J");

	while (1)
	{
		sp = SP;  ep = EP;  cp = CP;  kp = KP;  zp = ZP;  bp = BP;  rp = RP; 

tot = SA->rsum + EA->rsum + CA->rsum + KA->rsum + RA->rsum;

if (rnd%2 == 0)
printf("          CUST (%7d) (%7d)                   BEST (%7d)                          SMB (%7d)                       EBS  (%7d)                      \n", ZA->rsum, tot, BA->rsum, SA->rsum, EA->rsum);
else
printf("          CUST (%7d) (%7d)                   KMB  (%7d)                      CMBS(%7d)                       REUT (%7d)                      \n", ZA->rsum, tot, KA->rsum, CA->rsum, RA->rsum);

printf("----------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
		for (ii=0; ii<ZA->nrec; ii++)
		{
			memset(sbuf, 0x00, sizeof(sbuf));

			if (zp) {
				sprintf(&sbuf[strlen(sbuf)], "[%.6s] [\033[92m%1.1s\033[0m %s %s %02u:%02u.%02u %9g %9g\033[0m]",zp->symb, zp->cust.excode
						,(zp->cust.feedtp[0]=='1'?"기본":(zp->cust.feedtp[0]=='2'?"\033[35m수기\033[0m":(zp->cust.feedtp[0]=='3'?"\033[31m중단\033[0m":(zp->cust.feedtp[0]=='4'?"\033[36m우선\033[0m":"?"))))
						,(zp->trdf==9?"\033[90m제외":(zp->trdf==0?"\033[91m대기\033[90m":"\033[33m가능\033[0m"))
						,zp->khms/10000000, (zp->khms/100000)%100, (zp->khms/1000)%100, zp->bidlast, zp->offerlast);
				zp++;
			}

if (rnd%2 == 0) {
			if (bp) {
				sprintf(&sbuf[strlen(sbuf)], " [%s %02u:%02u.%02u \033[32m%.1s\033[0m %9g \033[32m%.1s\033[0m %9g]", (bp->quotalarm>0?"\033[104m알람\033[0m":(bp->trdf==9?"\033[36m제외\033[0m":(bp->trdf==0?"\033[91m대기\033[90m":"\033[33m가능\033[0m")))
											,bp->khms/10000000, (bp->khms/100000)%100, (bp->khms/1000)%100, bp->bidex, bp->bidlast, bp->offerex, bp->offerlast);
				bp++;
			}

			if (sp) {
				sprintf(&sbuf[strlen(sbuf)], " [%s %02u:%02u.%02u %9g %9g\033[0m]" ,(sp->quotalarm>0?"\033[104m알람\033[0m":(sp->trdf==9?"\033[90m제외":(sp->trdf==0?"\033[91m대기\033[90m":"\033[33m가능\033[0m")))
											,sp->khms/10000000, (sp->khms/100000)%100, (sp->khms/1000)%100, sp->bidlast, sp->offerlast);
				sp++;
			}
			if (ep) {
				sprintf(&sbuf[strlen(sbuf)], " [%s %02u:%02u.%02u %9g %9g\033[0m]",(ep->quotalarm>0?"\033[104m알람\033[0m":(ep->trdf==9?"\033[90m제외":(ep->trdf==0?"\033[91m대기\033[90m":"\033[33m가능\033[0m")))
											,ep->khms/10000000, (ep->khms/100000)%100, (ep->khms/1000)%100, ep->bidlast, ep->offerlast);
				ep++;
			}
} else {
			if (kp) {
				sprintf(&sbuf[strlen(sbuf)], " [%s %02u:%02u.%02u %9g %9g\033[0m]",(kp->quotalarm>0?"\033[104m알람\033[0m":(kp->trdf==9?"\033[90m제외":(kp->trdf==0?"\033[91m대기\033[90m":"\033[33m가능\033[0m")))
											,kp->khms/10000000, (kp->khms/100000)%100, (kp->khms/1000)%100, kp->bidlast, kp->offerlast);
				kp++;
			}
			if (cp) {
				sprintf(&sbuf[strlen(sbuf)], " [%s %02u:%02u.%02u %9g %9g\033[0m]",(cp->quotalarm>0?"\033[104m알람\033[0m":(cp->trdf==9?"\033[90m제외":(cp->trdf==0?"\033[91m대기\033[90m":"\033[33m가능\033[0m")))
											,cp->khms/10000000, (cp->khms/100000)%100, (cp->khms/1000)%100, cp->bidlast, cp->offerlast);
				cp++;
			}
			if (rp) {
				sprintf(&sbuf[strlen(sbuf)], " [%s %02u:%02u.%02u %9g %9g\033[0m]",(rp->quotalarm>0?"\033[104m알람\033[0m":(rp->trdf==9?"\033[90m제외":(rp->trdf==0?"\033[91m대기\033[90m":"\033[33m가능\033[0m")))
											,rp->khms/10000000, (rp->khms/100000)%100, (rp->khms/1000)%100, rp->bidlast, rp->offerlast);
				rp++;
			}
}
			printf("%s\n", sbuf);
			fflush(stdout);
		}

		printf("\n Enter to see next LP");
		fflush(stdout);

		// wait_input()
		{
			int		rc;
			char	buff[32];
			struct timeval tv;
			fd_set fds;

			tv.tv_sec  = 2;
			tv.tv_usec = 0;

			FD_ZERO(&fds);
			FD_SET(STDIN_FILENO, &fds);

			rc = select (STDIN_FILENO+1, &fds, NULL, NULL, &tv);
			if (rc < 0) {
				perror("wait error : ");
				break;
			}
			else if (rc > 0) {
				fgets(buff,sizeof(buff), stdin);
				rnd++;
			}
		}

		printf("\033[H\033[J");
	}

	return 0;
}
