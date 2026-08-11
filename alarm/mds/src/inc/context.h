#ifndef _CONTEXT_H_
#define	_CONTEXT_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "mds.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	MAX_ISAM_F	32
#define	MAX_ISAM_L	200	
#define	MAX_ROW_SIZE	(32*1024)

#define	IPCK(x, n)	(x + n)

typedef	struct {
	int	seqn;			// sequencial number
	char	name[16];		// port ID name
	int	port;			// port number
	struct	{
		char	  ipad[20];	// group IP 
		in_addr_t host[4];	// casting host
	} line[2];
	struct	{
		int	howto;		// sending method (1:broadcasting 2: rabbitq)
		char	qnam[20];	// queue name
		char	ipad[20];	// ip address 
		int	port;		// port
	} send;
	int	srcid;			// SRC ID
} PORTCFG;


typedef	struct {
	int	exid;			// exchange id
	time_t	rtim;			// receive time stamp
	int	recv[24];		// received account
	int	lost[24];		// lost 
	struct	{
		int	recv[24];	// delayed receive
		time_t	rtim;
	} delay;
	struct	{
		int	port;		// port number
		char	name[16];	// port name
		time_t	rtim;		// latest time stamp for ports
		int	recv;		// latest received counts
		int	rsum;		// daily total
	} port[MAX_PORT];
} MDACCT;


typedef	struct {
	XCHG	xchg;				// exchange information
	MDACCT	acct;				// traffic account for exchange
	int		tymd;				// ¿µ¾÷ÀÏ
	int		xxxx[1];			// reserved
	int		year[3];			// previous/current/next year
	char	hday[3][12][31];	// holiday information
} MDINFO;

// MDINFO.hday
#define	NODATE		0
#define	TRADDAY		1
#define	SATURDAY	2
#define	SUNDAY		3
#define	HOLIDAY		4
struct	islist {
	char	 name[20];	// CISAM file name
	uint32_t xymd;		// date for CISAM file
	int	 rows;		// # of rows
}; 

typedef struct {
	int	 dbid;			// ISAM number
	int	 dodo;			// do automatic open ?
	int 	 type;			// isam file type
	uint32_t size;			// size of record
	SCHEMA	*schema;		// SCHEMA information
	uint32_t xymd;			// cisam file path
	uint32_t lymd;			// most recent 
	char	 path[128];		// file path
	char	 name[20];		// file name
	char	 prepare[20];		// prepared file name
	int	 isfd;			// isam file descriptor
	int	 many;			// no of file
	struct	islist islist[MAX_ISAM_L];
} ISAM;

typedef	struct	{
//	int		(*init)(MARKET *);				// shm initializer
//	void   *(*roff)(MARKET *, void *, int);			// offset of records on folder
//	void    (*sync)(MARKET *, void *, int);			// synchronize shm and file
//	void    (*push)(MARKET *, void *, int);			// push real-time
//	void	(*clear)(MARKET *, void *);			// clear folder
#ifndef NO_WHERE
//	int		(*seek)(MARKET *, const char *, WHERE *); 	// seek symbols of product family
#endif
//	void	(*type)(void *, int *, int *, double *);	// folder's instrument type
//	int		(*fetch)(MARKET *, int, void *, int, int, int);	// data fetcher
	char	*dummy;
} METHOD;

typedef	struct {
	MARKET	*market;		// double link pointer
	MDINFO	*info;			// INFO pointer to the shared memory
	int	fsiz;				// folder size
	int	foff[MAX_ISAM_F];	// offset of folder's entry
	MDSPROC	proc;			// procedure
	void	*mydq[2];		// delay queue
	int	mqid[MAX_PORT];		// message queue id to receive (local)
	int	delay;				// delay time
	int	sock[MAX_PORT];		// socket to receive
	struct	{
		int	sock;			// market data notify socket
		struct	sockaddr_in sin;// notify sockaddr
	} posta;
	int	chck;			// general checking flags
	void	*schema;		// CISAM schema information
	ISAM	isam[MAX_ISAM_F];	// CISAM interface
	METHOD	*func;			// function procedure to access shared memory
	pthread_mutex_t lock;
	pthread_mutex_t mutex;		
	pthread_mutex_t islock;	
	pthread_t scheduler;		// schedule thread
	pthread_t delayer[2];		// delayer
	pthread_t dispatcher[MAX_PORT];	// message dispatcher
	pthread_t notifier[MAX_PORT];	// notifier
	pthread_t recver[MAX_PORT];	// receiver thread from UDP
	pthread_t syncer;		// shm synchronizer
} MDCTX;

typedef	struct	{
	MDINFO	info;			// book for a exchange
	int	mrec;			// max record
	int	nrec;			// current record numbers of whereis
	int	drec;			// number of deleted records of folder
	int	vrec;			// current record numbers of folder
} MDARCH;				// shared memory pointer

typedef	struct {
	char	symb[SYMB_LEN];		// symbol
	int	sync;			// deferred folder's entry
} FOLDER;

typedef	struct  {
	char	symb[SYMB_LEN];			// symbol code
	int	indx;				// position
} INDEX;

struct	argval {
	void	*market;		// FOX to interface
	char	name[40];		// ID name buffer
	int	argv;			// argument value
	int	seqn;			// sequencial number
};

MARKET *mds_market_alloc();
void    mds_setenv();

// shared memory 
int	mds_shminit(MARKET *market, int folder_size);
INDEX * mds_shmget(MARKET *market, const char *symb);
INDEX * mds_shmseek(MARKET *market, const char *symb);
INDEX * mds_shmadd(MARKET *market, const char *symb);
void	mds_shmdel(MARKET *market, const char *symb);
int	mds_shmadj(MARKET *market);
void  * mds_shmfold(MARKET *market, INDEX *index);

// data access functions
int  	mds_islist(MARKET *market, ISAM *isam, struct islist *islist, int howmany);
void	*mds_syncer(void *);

// market control
MARKET *market_pop(const char *exnm);
void    market_push(MARKET *market, void (*close)(MARKET *));
void    market_del(const char *exnm);


const MDACCT *mds_getacct(MARKET *market);
int 	 mds_portcfg(MARKET *market, const char *name, PORTCFG *portcfg);
PORTCFG *mds_getport(MARKET *market);

int	getacct(XCHG *xchg, MDACCT *acct);
int 	getinterface(char *ipad, char *ifnm, char *ifba);
int 	getbroadaddr(char *broadcast_addr);

#define	SELECT_CLR(x)	(x = 0)
#define	SELECT_ON(x, s)	(x |= (1 << s))
#define	SELECT_OFF(x,s)	(x &= ~(1 << s))
#define	SELECT_IS(x, s)	(x & (1<< s))


#ifdef __cplusplus 
} 
#endif 
#endif
