
#include <curses.h>
#include "mds2.h"

typedef struct {
	char    excode  [ 1];       // ¿øÃµ : 'S'=SMB, 'K'=KMB, 'E'=EBS, 'C'=CMB
	char    exnm    [ 8];
	int     ipck    ;
	MDARCH  *arch   ;           // MDARCH
	INDEX   *indx   ;           // INDEX
	MDFOLD  *fold   ;           // FOLD
} EX ;

enum {
	DF_INIT_NONE    = 0,
	DF_INIT_READ    ,
	DF_INIT_DONE
};

/* -----------  prototype  ------------------------------- */
key_t get_ipck2(int exid) ;
int mds_attach(int mode, char *msg) ;
int printall(char *exnm) ;
int iscrosstarget(char *basesymb) ;

/* -----------  for global  ------------------------------- */
EX  g_ex[MAX_XCHG];

MARKET *market ;

extern int getxmlcfg_t(const char *xmlpath, struct xmltag *xmltag);

int main(int argc, char **argv)
{
	int		ncnt;
	char	iexnm[32], iexcd[32], isymb[32];
	char	msg[512];
	MDFOLD	*mdfold=NULL;
	int 	i, j, z ;
	int 	wid ;

	MDARCH *arch ;
	MDFOLD *fold ;
	XCHG	xchg ;

	chk_mxlconf();


#if 0
	if (argc == 2)
	{
		printall(argv[1]);
		return 0;
	}
	else if (argc == 3)
	{
		if ((mdfold = mds_getfold(argv[1], argv[2], msg)) != NULL)
			printfold(mdfold);
		
		return 0;
	}

	while (1)
	{
		memset(msg,   0x00, sizeof(msg));
		memset(iexcd, 0x00, sizeof(iexcd));
		memset(iexnm, 0x00, sizeof(iexnm));
		memset(isymb, 0x00, sizeof(isymb));
		
		printf("\nEnter EXNAME('S'MBS/'K'MBS/'C'MBS/'B'EST/'Z'CUST/'q'uit) : ");
		scanf("%s", iexcd);
		if (strlen(iexcd) < 1)	continue;
		
		     if (iexcd[0] == 'S')	strcpy(iexnm, "SMBS");
		else if (iexcd[0] == 'K')	strcpy(iexnm, "KMB");
		else if (iexcd[0] == 'E')	strcpy(iexnm, "EBS");
		else if (iexcd[0] == 'C')	strcpy(iexnm, "CMBS");
		else if (iexcd[0] == 'B')	strcpy(iexnm, "BEST");
		else if (iexcd[0] == 'Z')	strcpy(iexnm, "CUST");
		else if (iexcd[0] == 'T')	strcpy(iexnm, "TEST");
		else if (iexcd[0] == 'q')	break;
		else {
			printf("\nInput Error ");
			continue;
		}

		printf("Enter Symbol(all) : ");
		scanf("%s", isymb);
		if (strcmp(isymb, "all") == 0)
		{
			printall(iexnm);

			getchar();
			continue;
		}
		else if (strlen(isymb) < 6)		continue;

		printf("-------------------------------------------------------------------\n");

		mdfold = mds_getfold(iexnm, isymb, msg);
		printf("%s\n", msg);
		printf("===================================================================\n");
		if (mdfold==NULL)
		{
			printf("\n code not found.. \n");
			continue;
		}
		
		printfold(mdfold);

		getchar();
	}
#endif

	return 0;

} /* End of int main(int argc, char **argv) */

int chk_mxlconf()
{
	int ii, jj;
	char	xmlpath[128];
	struct xmltag tags[32];
	sprintf(xmlpath, "%s/exchanges.new.cfg", ETC_DIR);
	int n = getxmlcfg_t(xmlpath, tags);
	printf("Total tags = %d\n", n);

	for (ii = 0; ii < n; ii++)
	{
		printf("[%d] tag=%s (%d defs)\n", ii, tags[ii].tags, tags[ii].many);
		for (jj = 0; jj < tags[ii].many; jj++)
			printf("   %s = %s\n", tags[ii].defs[jj].name, tags[ii].defs[jj].vals);
	}
	
	return 0;
}

