#include "mdmstr.h"
#include "mdquot.h"
#include "mdbook.h"
#include "mdtick.h"
#include "mdintr.h"
#include "mdheod.h"
#include "mdteod.h"
#include "mdheow.h"
#include "mdheom.h"
#include "mdpred.h"
#include "mdtuja.h"
#include "mdveod.h"
#include "mdcprt.h"
#include "mdtrad.h"
#ifndef	__MDFOLD_H__
#define	__MDFOLD_H__
//
// MDFOLD schema
//
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char	symb[SYMB_LEN];
	int		sync;
	MDMSTR	mstr;
	MDQUOT	quot;
	MDBOOK	book;
	MDTRAD	trad;
} MDFOLD;

#ifdef	_SCHEMA_H_
#define	MDTICK_OF(member)	((long) &(((MDTICK *)0)->member))
#define	MDINTR_OF(member)	((long) &(((MDINTR *)0)->member))
#define	MDHEOD_OF(member)	((long) &(((MDHEOD *)0)->member))
#define	MDVEOD_OF(member)	((long) &(((MDVEOD *)0)->member))
#define	MDTUJA_OF(member)	((long) &(((MDTUJA *)0)->member))
#define	MDTEOD_OF(member)	((long) &(((MDTUJA *)0)->member))
#define	MDPRED_OF(member)	((long) &(((MDPRED *)0)->member))
#define	MDCPRT_OF(member)	((long) &(((MDCPRT *)0)->member))

SCHEMA 	schema[] = {
	{ MSTR, _OP_, 	    "MDMSTR", MSTR_TYPE, -1,   	    	  sizeof(MDMSTR), mdmstr_key },
	{ QUOT, _OP_,       "MDQUOT", MSTR_TYPE, -1,   	    	  sizeof(MDQUOT), mdquot_key },
	{ TICK, _OP_|_CR_,  "MT",     TICK_TYPE, MDTICK_OF(xymd), sizeof(MDTICK), mdtick_key },
	{ INTR, _OP_|_CR_,  "MI",     TICK_TYPE, MDINTR_OF(xymd), sizeof(MDINTR), mdintr_key },
	{ HEOD, _OP_, 	    "MH",     HEOD_TYPE, MDHEOD_OF(xymd), sizeof(MDHEOD), mdheod_key },
	{ TUJA,      _CR_,  "MDTUJA", DAY1_TYPE, -1,		  sizeof(MDTUJA), mdtuja_key},
	{ TEOD,      _CR_,  "MJ",     HEOD_TYPE, MDTUJA_OF(xymd), sizeof(MDTUJA), mdteod_key },
	{ CPRT,      _CR_,  "MDCPRT", DAY1_TYPE, -1,		  sizeof(MDCPRT), mdcprt_key },
	{ REOD,      _CR_,  "MR",     HEOD_TYPE, MDCPRT_OF(xymd), sizeof(MDCPRT), mdcprt_key },
	{ VEOD, 0,          "MV",     HEOD_TYPE, MDVEOD_OF(xymd), sizeof(MDVEOD), mdveod_key },
	{ -1,	0, 	    "",	      0,         -1,	            0,		  NULL	     }
};

#else
extern SCHEMA scheam[];
#endif

#ifdef __cplusplus
}
#endif
#endif
