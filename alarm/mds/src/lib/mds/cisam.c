//
// cisam.c
// Control database
//
#include <dirent.h>
#include "context.h"



static ISAMF *is_alloc()
{
	return(NULL);
}

static void is_free(ISAMF *isamf)
{
}

static void ispath(MARKET *market, int dbid, int xymd, char *path)
{
}

static uint16_t swap16(uint16_t n)
{
	return (((n & 0xffU) << 8) | 
		((n & 0xff00U) >> 8));
}

static uint32_t swap32(uint32_t n)
{
	return (((n & 0xffU) << 24)     | 
		((n & 0xff00U) << 8)    | 
		((n & 0xff0000U) >> 8)  | 
		((n & 0xff000000U) >> 24));
}

static uint64_t swap64(uint64_t n)
{
	return (((n & (((uint64_t) 0xff)      )) << 56) |
		((n & (((uint64_t) 0xff) <<  8)) << 40) |
		((n & (((uint64_t) 0xff) << 16)) << 24) |
		((n & (((uint64_t) 0xff) << 24)) <<  8) |
		((n & (((uint64_t) 0xff) << 32)) >>  8) |
		((n & (((uint64_t) 0xff) << 40)) >> 24) |
		((n & (((uint64_t) 0xff) << 48)) >> 40) |
		((n & (((uint64_t) 0xff) << 56)) >> 56));
}


void mds_key2isam(MARKET *market, void *row, struct keydesc *keydesc)
{
//	char	*rec = (char*)row;
//	int	off;
//	int	ii;
//
//	if (market->endian != L_ENDIAN)
//		return;
//	for (ii = 0; ii < keydesc->k_nparts; ii++)
//	{
//		switch (keydesc->k_part[ii].kp_type & TYPEMASK)
//		{
//		case INTTYPE:
//		     {
//		     	uint16_t v, *p;
//
//			off = keydesc->k_part[ii].kp_start;
//			p = (uint16_t *)&rec[off];
//			v = *p;
//			*p = swap16(v);
//			break;
//		     }
//		case LONGTYPE:
//		case FLOATTYPE:
//		     {
//		     	uint32_t v, *p;
//
//			off = keydesc->k_part[ii].kp_start;
//			p = (uint32_t *)&rec[off];
//			v = *p;
//			*p = swap32(v);
//			break;
//		     }
//		case DOUBLETYPE:
//		     {
//		     	uint64_t v, *p;
//
//			off = keydesc->k_part[ii].kp_start;
//			p = (uint64_t *)&rec[off];
//			v = *p;
//			*p = swap64(v);
//			break;
//		     }
//		case CHARTYPE:
//		default:
//			break;
//		}
//	}
}

void mds_key2host(MARKET *market, void *row, struct keydesc *keydesc)
{
	mds_key2isam(market, row, keydesc);
}

void mds_islock(MARKET *market)
{
	MDCTX *ctx = (MDCTX*)market->ctx;

	pthread_mutex_lock(&ctx->islock);
}
void mds_isunlock(MARKET *market)
{
	MDCTX *ctx = (MDCTX*)market->ctx;

	pthread_mutex_unlock(&ctx->islock);
}


//
// mds_isinit()
// Initialize CISAM data file
//
int mds_isinit(MARKET *market)
{
	return(0);
}


//
// mds_isopen()
// Open CISAM files for market
//
int mds_isopen(MARKET *market)
{
	return(0);
}

//
// mds_isbuild()
// Build a new isam file
//
int mds_isbuild(MARKET *market, int dbid, uint32_t xymd)
{
	return(-1);
}

//
// mds_isrenewal()
//
int mds_isrenewal(MARKET *market)
{
	return(0);
}

//
// mds_isupsert()
// Upsert a record to CISAM file
//
int mds_isupsert(MARKET *market, int dbid, void *record)
{
	return(0);
}

int mds_isdelete(MARKET *market, int dbid, void *record)
{
	return(0);
}

//
// mds_islist()
// Get CISAM file list
//
int mds_islist(MARKET *market, ISAM *isam, struct islist *islist, int howmany)
{
	return(0);

}

//
// mds_iscleanup()
// Clean CISAM record except key parts
//
int mds_iscleanup(MARKET *market, int dbid)
{
	return(0);
}

//
// mds_isdrop()
// Drop file
//
int mds_isdrop(MARKET *market, int dbid, const char *file_name)
{
	return(0);
}

//
// mds_iskeep()
//
int mds_iskeep(MARKET *market, int dbid, int days)
{
	return(0);
}

//
// mds_isfd()
// Return ISAM file descriptor
//
int mds_isfd(MARKET *market, int dbid, void *record, int *cflg)
{
	return(0);
}

//
// mds_fetch_isfd()
// Return ISAM file descriptor
//
int mds_fetch_isfd(MARKET *market, int dbid, int xymd, int *cflg)
{

	return(0);
}

//
// mds_isclose()
// Close all CISAM files
//
void mds_isclose(MARKET *market)
{
}

///////////////////////////////////////////////////////////////////////////
// CIAM raw function by market
//////////////////////////////////////////////////////////////////////////
ISAMF * is_open(MARKET *market, int dbid, int xymd, int mode)
{
	return(NULL);
}

//
// is_build()
// Build a new isam file
//
ISAMF * is_build(MARKET *market, int dbid, int xymd)
{
	return(NULL);
}

int is_read(ISAMF *isamf, char *record, int mode)
{
	return(0);
}

int is_write(ISAMF *isamf, char *record)
{
	return(0);
}

int is_rewrite(ISAMF *isamf, char *record)
{
	return(0);
}

int is_upsert(ISAMF *isamf, char *record)
{
	return(0);
}

void is_close(ISAMF *isamf)
{
}

int is_list(MARKET *market, int dbid, int *xymd)
{
	return(0);
}


static int cmplist(const void *p1, const void *p2)
{
	struct islist *l1 = (struct islist*)p1;
	struct islist *l2 = (struct islist*)p2;
	return(strcmp(l2->name, l1->name));
}
