#ifndef	_STREAM_H_
#define	_STREAM_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define	MAX_PACKET_SIZE		2048

// market data input format
typedef	struct {
	int		 port;			// port number
	char	 name[8];		// port naem
	int		 seqn;			// input port sequence number
	int	 	 type;			// data type
	struct	 timeval timeval;	// time stamp
	uint32_t tymd;			// trading YYYYMMDD
	uint32_t xymd;			// YYYYMMDD
	uint32_t xhms;			// HHMMSS
	uint32_t kymd;			// YYYYMMDD
	uint32_t khms;			// HHMMSS
	uint32_t msec;			// mili second
	int		 tpos;			// time position(BEFORE, AFTER, TRADING, BETWEEN)
	int	 	 toff;			// time offset
	int	 	 rcvl;			// receive data length
	char	*rcvb;			// receive message data
} TOKEN;

#define	SOH	0x01
#define	PIP	0x7c

typedef	struct {
	int		tagn;			// tag number
	char   *vptr;			// value pointer
	double	dval;			// double value
	int		ival;			// interger value
} FIXFLD;

typedef	struct {
	char	symb[20];		// (local)symbol code
	char	csym[20];		// clearing symbol
	char	isym[20];		// (internal)symbol code
	char	enam[128];		// symbol name (english)
	char	knam[128];		// symbol name(korean)
	int		styp;			// security type
	char	feed[16];		// feeding channel
	char	curr[4];		// currency
	struct	{
		int	frhm;		// trading start hour
		int	tohm;		// trading end hour
		int	fwdy;		// trading day of week
		int	twdy;		// trading day of week
		int	hfhm;		// trading halt time
		int	hthm;		// trading halt time
		int	tfhm;		// T session start time
	} session;
	double	tsiz;			// tick size
	double	tval;			// tick value
	double	pmul;			// price multiple 
	int		zdiv;			// no of decimal
	int		xdiv;			// divisor
	int		pind;			// price indicator
	int		aval;			// price adjustment value
	int		sect;			// product(=symbol) sector
    int		sind;       	// strike indicator	
    int		sad1;       	// strike adjust-1	
    int		sad2;       	// strike adjust-2	
    int		sfmt;       	// strike format
    int		sdiv;       	// strike denominator
	double	sint;			// strike interval
	double	atma;			// ATM adjust
} SYMBOL;



#ifdef	__cplusplus
extern	"C" {
#endif

int	mds_recv(MARKET *market, TOKEN *token, char *buf, int size, int timeout);
int 	mds_msg2fld(FIXFLD *fixfld, int howmany, char *msgb, int msgl, int seperator);
FIXFLD *mds_getfld(FIXFLD *fixfld, int tag_number);

int 	mds_symbinit(MARKET *market);
SYMBOL *mds_symbget(MARKET *market, const char *symb);
SYMBOL *mds_symbol(MARKET *market, int *nsym);

#ifdef	__cplusplus
}
#endif

#endif
