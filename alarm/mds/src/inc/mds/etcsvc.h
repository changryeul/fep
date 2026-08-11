#ifndef	_ETCSVC_H
#define	_ETCSVC_H

#define MAX_XCHG    16
#define MAX_PROD 	100
#define INFOSYMB_FILE	"information-symb.csv"
#define OPTSYMB_INFO_FILE	"optsymb-info.csv"
#define FUTSYMB_INFO_FILE	"futsymb-info.csv"

#include "mdfold.h"

typedef struct {
    char    esym[16];   /* exchange symbol	*/
    char    isym[16];   /* internal symbol	*/
	char 	excd[ 8];	/* exchange code	*/
	char 	exnm[32];	/* exchange name	*/
    double	tsiz;       /* tick size      	*/
    double	tval;       /* tick value      	*/
    double	adjv;       /* adjust value    	*/
    int		aval;       /* adjust value(interval)		*/
    double	csiz;       /* cab	size      	*/
    int		cval;       /* cab 	value		*/
    int		pind;       /* price indicator	*/
    int		padj;       /* price adjust 	*/
    int		sind;       /* strike indicator	*/
    int		sad1;       /* strike adjust-1	*/
    int		sad2;       /* strike adjust-2	*/
    int		sfmt;       /* strike format	*/
    int		sdiv;       /* strike denominator	*/
	int		sect;		/* sector 			*/
	int		osty;		/* option style 	*/
	int		trdf;		/* tradable 		*/
    char    curr[3];    /* trade currency   */
    char    enam[80];   /* commodity name   */
    double	atma;   	/* atm adjust 		*/
    double	sint;   	/* strike interval 	*/
    double	mrgn;   	/* margin 			*/
} SYMINF;

int get_symbinf(SYMINF *, char *);
int esym2isym(MDMSTR *, char *);
int isym2esym(char *, char *, char *);
void symbconv1(char *, char *);
void symbconv2(char *, char *);
int	get_loctz(char *, char *);
int	get_exid(char *excd);
int get_diff_rate(double last, double base, double *diff, double *rate);
int get_comd(char *isymb, char *comd);
void *getfolder_leadmonth(MARKET *market, char *root);
void *getfolder_isym(MARKET *market, const char *symb);

void mds_symbseqn(MARKET *market, MDFOLD *folder);
int update_intr(MARKET *market, MDFOLD *folder, double setp);

#endif
