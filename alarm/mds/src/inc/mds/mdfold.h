#ifndef	__MDFOLD_H__
#define	__MDFOLD_H__
#include "mdmstr.h"
#include "mdquot.h"
#include "mdbook.h"
#include "mdtick.h"
#include "mdtrad.h"
//#include "mdheod.h"
//#include "mdteod.h"
//#include "mdheow.h"
//#include "mdheom.h"


//
// MDFOLD schema
//
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char	symb[SYMB_LEN];
	int		sync;
	MDMSTR	mstr;			// symbol master
	MDQUOT	quot;			// realtime quote
	MDBOOK	book;			// market depth
	MDTRAD	trad;			// traddata
} MDFOLD;

#define SDATA_MEMBER(schm, sdata, member)  (sdata)->s##schm.member

#ifdef	_SCHEMA_H_
#define	MDTICK_OF(member)	((long) &(((MDTICK *)0)->member))
#define	MDTRAD_OF(member)	((long) &(((MDTRAD *)0)->member))
#define	MDHEOD_OF(member)	((long) &(((MDHEOD *)0)->member))
#define	MDTEOD_OF(member)	((long) &(((MDTEOD *)0)->member))


#endif

#ifdef __cplusplus
}
#endif
#endif
