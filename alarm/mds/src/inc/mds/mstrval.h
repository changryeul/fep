#ifndef __MSTRVAL_H__
#define __MSTRVAL_H__
#include "mdfold.h"

static char* mstr_symb(MARKET *market, MDFOLD *folder)
{
	char *val = "";
	switch (market->xchg->schm)
    {
        case 1:		val = folder->mstr.s001.symb; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.symb; break;
        case 44:	val = folder->mstr.s044.symb; break;
        case 54:	val = folder->mstr.s054.symb; break;
        case 126:	val = folder->mstr.s126.symb; break;
    }
	return val;
}

static int mstr_exid(MARKET *market, MDFOLD *folder)
{
    int val = 0;
    switch (market->xchg->schm)
    {
        case 1:		val = folder->mstr.s001.exid; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.exid; break;
        case 44:	val = folder->mstr.s044.exid; break;
        case 54:	val = folder->mstr.s054.exid; break;
        case 126:	val = folder->mstr.s126.exid; break;
    }
    return val;
}

static char* mstr_knam(MARKET *market, MDFOLD *folder)
{
	char *val = "";
	switch (market->xchg->schm)
    {
        case 1:		val = folder->mstr.s001.knam; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.knam; break;
        case 44:	val = folder->mstr.s044.knam; break;
        case 54:	val = folder->mstr.s054.knam; break;
        case 126:	val = folder->mstr.s126.knam; break;
    }
	return val;
}

static char* mstr_enam(MARKET *market, MDFOLD *folder)
{
	char *val = "";
	switch (market->xchg->schm)
    {
        case 1:		val = folder->mstr.s001.enam; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.enam; break;
        case 44:	val = folder->mstr.s044.enam; break;
        case 54:	val = folder->mstr.s054.enam; break;
        case 126:	val = folder->mstr.s126.enam; break;
    }
	return val;
}

static char* mstr_anam(MARKET *market, MDFOLD *folder)
{
	char *val = "";
	switch (market->xchg->schm)
    {
        case 1:		val = folder->mstr.s001.anam; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.anam; break;
        case 44:	val = folder->mstr.s044.anam; break;
        case 54:	val = folder->mstr.s054.anam; break;
        case 126:	val = folder->mstr.s126.anam; break;
    }
	return val;
}

static char* mstr_hnam(MARKET *market, MDFOLD *folder)
{
	char *val = "";
	switch (market->xchg->schm)
    {
        case 1:		val = folder->mstr.s001.hnam; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.hnam; break;
        case 44:	val = folder->mstr.s044.enam; break;
        case 54:	val = folder->mstr.s054.hnam; break;
        case 126:	val = folder->mstr.s126.hnam; break;
    }
	return val;
}

static uint32_t mstr_lymd(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		val = folder->mstr.s001.lymd; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.lymd; break;
        case 44:	val = folder->mstr.s044.lymd; break;
        case 54:	val = folder->mstr.s054.lymd; break;
        case 126:	val = folder->mstr.s126.lymd; break;
    }
	return val;
}

static uint32_t mstr_eymd(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		val = folder->mstr.s001.eymd; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.eymd; break;
        case 44:	val = folder->mstr.s044.eymd; break;
        case 54:	val = folder->mstr.s054.eymd; break;
        case 126:	val = folder->mstr.s126.eymd; break;
    }
	return val;
}

static int mstr_styp(MARKET *market, MDFOLD *folder)
{
	int val = 0;
	switch (market->xchg->schm)
    {
        case 1:		val = folder->mstr.s001.styp; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.styp; break;
        case 44:	val = folder->mstr.s044.styp; break;
        case 54:	val = folder->mstr.s054.styp; break;
        case 126:	val = folder->mstr.s126.styp; break;
    }
	return val;
}

static char* mstr_code(MARKET *market, MDFOLD *folder)
{
	char *val = "";
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.code; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.code; break;
        case 44:	val = folder->mstr.s044.code; break;
        case 54:	val = folder->mstr.s054.code; break;
        case 126:	val = folder->mstr.s126.code; break;
    }
	return val;
}

static uint32_t mstr_seqn(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.seqn; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.seqn; break;
        case 44:	break; //val = folder->mstr.s044.seqn; break;
        case 54:	break; //val = folder->mstr.s054.seqn; break;
        case 126:	break; //val = folder->mstr.s126.seqn; break;
    }
	return val;
}

static char* mstr_prid(MARKET *market, MDFOLD *folder)
{
	char *val = "";
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.prid; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.prid; break;
        case 44:	break; //val = folder->mstr.s044.prid; break;
        case 54:	break; //val = folder->mstr.s054.prid; break;
        case 126:	break; //val = folder->mstr.s126.prid; break;
    }
	return val;
}

static int mstr_zdiv(MARKET *market, MDFOLD *folder)
{
	int val = 0;
	switch (market->xchg->schm)
    {
        case 1:		val = folder->mstr.s001.zdiv; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.zdiv; break;
        case 44:	val = folder->mstr.s044.zdiv; break;
        case 54:	val = folder->mstr.s054.zdiv; break;
        case 126:	val = folder->mstr.s126.zdiv; break;
    }
	return val;
}

static int mstr_spgb(MARKET *market, MDFOLD *folder)
{
	int val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.spgb; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.spgb; break;
        case 44:	break; //val = folder->mstr.s044.spgb; break;
        case 54:	break; //val = folder->mstr.s054.spgb; break;
        case 126:	break; //val = folder->mstr.s126.spgb; break;
    }
	return val;
}

static int mstr_stcd(MARKET *market, MDFOLD *folder)
{
	int val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.stcd; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.stcd; break;
        case 44:	break; //val = folder->mstr.s044.stcd; break;
        case 54:	break; //val = folder->mstr.s054.stcd; break;
        case 126:	break; //val = folder->mstr.s126.stcd; break;
    }
	return val;
}

static uint32_t mstr_symd(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.symd; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.symd; break;
        case 44:	val = folder->mstr.s044.symd; break;
        case 54:	val = folder->mstr.s054.symd; break;
        case 126:	val = folder->mstr.s126.symd; break;
    }
	return val;
}

static uint32_t mstr_mymd(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.mymd; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.mymd; break;
        case 44:	val = folder->mstr.s044.mymd; break;
        case 54:	val = folder->mstr.s054.mymd; break;
        case 126:	val = folder->mstr.s126.mymd; break;
    }
	return val;
}

static uint32_t mstr_jjis(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.jjis; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.jjis; break;
        case 44:	val = folder->mstr.s044.jjis; break;
        case 54:	val = folder->mstr.s054.jjis; break;
        case 126:	val = folder->mstr.s126.jjis; break;
    }
	return val;
}

static int mstr_lmdr(MARKET *market, MDFOLD *folder)
{
	int val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.lmdr; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.lmdr; break;
        case 44:	break; //val = folder->mstr.s044.lmdr; break;
        case 54:	break; //val = folder->mstr.s054.lmdr; break;
        case 126:	break; //val = folder->mstr.s126.lmdr; break;
    }
	return val;
}

static int mstr_fngd(MARKET *market, MDFOLD *folder)
{
	int val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.fngd; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.fngd; break;
        case 44:	break; //val = folder->mstr.s044.fngd; break;
        case 54:	break; //val = folder->mstr.s054.fngd; break;
        case 126:	break; //val = folder->mstr.s126.fngd; break;
    }
	return val;
}

static double mstr_uplp(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.uplp; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.uplp; break;
        case 44:	val = folder->mstr.s044.uplp; break;
        case 54:	val = folder->mstr.s054.uplp; break;
        case 126:	val = folder->mstr.s126.uplp; break;
    }
	return val;
}

static double mstr_dnlp(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.dnlp; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.dnlp; break;
        case 44:	val = folder->mstr.s044.dnlp; break;
        case 54:	val = folder->mstr.s054.dnlp; break;
        case 126:	val = folder->mstr.s126.dnlp; break;
    }
	return val;
}

static double mstr_up1p(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.up1p; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.up1p; break;
        case 44:	break; //val = folder->mstr.s044.up1p; break;
        case 54:	break; //val = folder->mstr.s054.up1p; break;
        case 126:	break; //val = folder->mstr.s126.up1p; break;
    }
	return val;
}

static double mstr_up2p(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.up2p; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.up2p; break;
        case 44:	break; //val = folder->mstr.s044.up2p; break;
        case 54:	break; //val = folder->mstr.s054.up2p; break;
        case 126:	break; //val = folder->mstr.s126.up2p; break;
    }
	return val;
}

static double mstr_up3p(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.up3p; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.up3p; break;
        case 44:	break; //val = folder->mstr.s044.up3p; break;
        case 54:	break; //val = folder->mstr.s054.up3p; break;
        case 126:	break; //val = folder->mstr.s126.up3p; break;
    }
	return val;
}

static double mstr_dn1p(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.dn1p; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.dn1p; break;
        case 44:	break; //val = folder->mstr.s044.dn1p; break;
        case 54:	break; //val = folder->mstr.s054.dn1p; break;
        case 126:	break; //val = folder->mstr.s126.dn1p; break;
    }
	return val;
}

static double mstr_dn2p(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.dn2p; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.dn2p; break;
        case 44:	break; //val = folder->mstr.s044.dn2p; break;
        case 54:	break; //val = folder->mstr.s054.dn2p; break;
        case 126:	break; //val = folder->mstr.s126.dn2p; break;
    }
	return val;
}

static double mstr_dn3p(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.dn3p; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.dn3p; break;
        case 44:	break; //val = folder->mstr.s044.dn3p; break;
        case 54:	break; //val = folder->mstr.s054.dn3p; break;
        case 126:	break; //val = folder->mstr.s126.dn3p; break;
    }
	return val;
}

static double mstr_upcb(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.upcb; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.upcb; break;
        case 44:	break; //val = folder->mstr.s044.upcb; break;
        case 54:	break; //val = folder->mstr.s054.upcb; break;
        case 126:	break; //val = folder->mstr.s126.upcb; break;
    }
	return val;
}

static double mstr_dncb(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.dncb; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.dncb; break;
        case 44:	break; //val = folder->mstr.s044.dncb; break;
        case 54:	break; //val = folder->mstr.s054.dncb; break;
        case 126:	break; //val = folder->mstr.s126.dncb; break;
    }
	return val;
}

static double mstr_uprp(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.uprp; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.uprp; break;
        case 44:	break; //val = folder->mstr.s044.uprp; break;
        case 54:	break; //val = folder->mstr.s054.uprp; break;
        case 126:	break; //val = folder->mstr.s126.uprp; break;
    }
	return val;
}

static double mstr_dnrp(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.dnrp; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.dnrp; break;
        case 44:	break; //val = folder->mstr.s044.dnrp; break;
        case 54:	break; //val = folder->mstr.s054.dnrp; break;
        case 126:	break; //val = folder->mstr.s126.dnrp; break;
    }
	return val;
}

static double mstr_base(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		val = folder->mstr.s001.base; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.base; break;
        case 44:	val = folder->mstr.s044.base; break;
        case 54:	val = folder->mstr.s054.base; break;
        case 126:	val = folder->mstr.s126.base; break;
    }
    return val;
}

static double mstr_strk(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.strk; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.strk; break;
        case 44:	break; //val = folder->mstr.s044.strk; break;
        case 54:	val = folder->mstr.s054.strk; break;
        case 126:	break; //break; //val = folder->mstr.s126.strk; break;
    }
    return val;
}

static double mstr_qhsp(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.qhsp; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.qhsp; break;
        case 44:	break; //val = folder->mstr.s044.qhsp; break;
        case 54:	break; //val = folder->mstr.s054.qhsp; break;
        case 126:	break; //val = folder->mstr.s126.qhsp; break;
    }
    return val;
}

static double mstr_uncp(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.uncp; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.uncp; break;
        case 44:	val = folder->mstr.s044.uncp; break;
        case 54:	val = folder->mstr.s054.uncp; break;
        case 126:	val = folder->mstr.s126.uncp; break;
    }
    return val;
}

static char* mstr_unpd(MARKET *market, MDFOLD *folder)
{
    char* val = "";
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.unpd; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.unpd; break;
        case 44:	val = folder->mstr.s044.unpd; break;
        case 54:	val = folder->mstr.s054.unpd; break;
        case 126:	val = folder->mstr.s126.unpd; break;
    }
    return val;
}

static char* mstr_uncd(MARKET *market, MDFOLD *folder)
{
    char* val = "";
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.uncd; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.uncd; break;
        case 44:	val = folder->mstr.s044.uncd; break;
        case 54:	val = folder->mstr.s054.uncd; break;
        case 126:	val = folder->mstr.s126.uncd; break;
    }
    return val;
}

static int mstr_khtc(MARKET *market, MDFOLD *folder)
{
    int val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.khtc; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.khtc; break;
        case 44:	break; //val = folder->mstr.s044.khtc; break;
        case 54:	break; //val = folder->mstr.s054.khtc; break;
        case 126:	break; //val = folder->mstr.s126.khtc; break;
    }
    return val;
}

static char* mstr_spct(MARKET *market, MDFOLD *folder)
{
    char* val = "";
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.spct; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.spct; break;
        case 44:	break; //val = folder->mstr.s044.spct; break;
        case 54:	break; //val = folder->mstr.s054.spct; break;
        case 126:	break; //val = folder->mstr.s126.spct; break;
    }
    return val;
}

static char* mstr_spc1(MARKET *market, MDFOLD *folder)
{
    char* val = "";
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.spc1; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.spc1; break;
        case 44:	break; //val = folder->mstr.s044.spc1; break;
        case 54:	break; //val = folder->mstr.s054.spc1; break;
        case 126:	break; //val = folder->mstr.s126.spc1; break;
    }
    return val;
}

static char* mstr_spc2(MARKET *market, MDFOLD *folder)
{
    char* val = "";
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.spc2; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.spc2; break;
        case 44:	break; //val = folder->mstr.s044.spc2; break;
        case 54:	break; //val = folder->mstr.s054.spc2; break;
        case 126:	break; //val = folder->mstr.s126.spc2; break;
    }
    return val;
}

static int mstr_mseq(MARKET *market, MDFOLD *folder)
{
    int val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.mseq; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.mseq; break;
        case 44:	break; //val = folder->mstr.s044.mseq; break;
        case 54:	break; //val = folder->mstr.s054.mseq; break;
        case 126:	break; //val = folder->mstr.s126.mseq; break;
    }
    return val;
}

static int mstr_jjgb(MARKET *market, MDFOLD *folder)
{
    int val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.jjgb; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.jjgb; break;
        case 44:	break; //val = folder->mstr.s044.jjgb; break;
        case 54:	break; //val = folder->mstr.s054.jjgb; break;
        case 126:	break; //val = folder->mstr.s126.jjgb; break;
    }
    return val;
}

static double mstr_uqty(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.uqty; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.uqty; break;
        case 44:	val = folder->mstr.s044.uqty; break;
        case 54:	val = folder->mstr.s054.uqty; break;
        case 126:	val = folder->mstr.s126.uqty; break;
    }
    return val;
}

static double mstr_tmul(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.tmul; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.tmul; break;
        case 44:	val = folder->mstr.s044.tmul; break;
        case 54:	val = folder->mstr.s054.tmul; break;
        case 126:	val = folder->mstr.s126.tmul; break;
    }
    return val;
}

static int mstr_sjgb(MARKET *market, MDFOLD *folder)
{
    int val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.sjgb; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.sjgb; break;
        case 44:	break; //val = folder->mstr.s044.sjgb; break;
        case 54:	break; //val = folder->mstr.s054.sjgb; break;
        case 126:	break; //val = folder->mstr.s126.sjgb; break;
    }
    return val;
}

static int mstr_sjcd(MARKET *market, MDFOLD *folder)
{
    int val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.sjcd; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.sjcd; break;
        case 44:	break; //val = folder->mstr.s044.sjcd; break;
        case 54:	break; //val = folder->mstr.s054.sjcd; break;
        case 126:	break; //val = folder->mstr.s126.sjcd; break;
    }
    return val;
}

static double mstr_eqhs(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.eqhs; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.eqhs; break;
        case 44:	break; //val = folder->mstr.s044.eqhs; break;
        case 54:	break; //val = folder->mstr.s054.eqhs; break;
        case 126:	break; //val = folder->mstr.s126.eqhs; break;
    }
    return val;
}

static int mstr_jjsc(MARKET *market, MDFOLD *folder)
{
    int val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.jjsc; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.jjsc; break;
        case 44:	break; //val = folder->mstr.s044.jjsc; break;
        case 54:	break; //val = folder->mstr.s054.jjsc; break;
        case 126:	break; //val = folder->mstr.s126.jjsc; break;
    }
    return val;
}

static double mstr_ujbp(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ujbp; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.ujbp; break;
        case 44:	break; //val = folder->mstr.s044.ujbp; break;
        case 54:	break; //val = folder->mstr.s054.ujbp; break;
        case 126:	break; //val = folder->mstr.s126.ujbp; break;
    }
    return val;
}

static int mstr_bpc1(MARKET *market, MDFOLD *folder)
{
    int val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.bpc1; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.bpc1; break;
        case 44:	break; //val = folder->mstr.s044.bpc1; break;
        case 54:	break; //val = folder->mstr.s054.bpc1; break;
        case 126:	break; //val = folder->mstr.s126.bpc1; break;
    }
    return val;
}

static int mstr_bpc2(MARKET *market, MDFOLD *folder)
{
    int val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.bpc2; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.bpc2; break;
        case 44:	break; //val = folder->mstr.s044.bpc2; break;
        case 54:	break; //val = folder->mstr.s054.bpc2; break;
        case 126:	break; //val = folder->mstr.s126.bpc2; break;
    }
    return val;
}

static double mstr_jjcp(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.jjcp; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.jjcp; break;
        case 44:	break; //val = folder->mstr.s044.jjcp; break;
        case 54:	break; //val = folder->mstr.s054.jjcp; break;
        case 126:	break; //val = folder->mstr.s126.jjcp; break;
    }
    return val;
}

static double mstr_jjbp(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.jjbp; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.jjbp; break;
        case 44:	break; //val = folder->mstr.s044.jjbp; break;
        case 54:	break; //val = folder->mstr.s054.jjbp; break;
        case 126:	break; //val = folder->mstr.s126.jjbp; break;
    }
    return val;
}

static int mstr_jjbc(MARKET *market, MDFOLD *folder)
{
    int val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.jjbc; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.jjbc; break;
        case 44:	break; //val = folder->mstr.s044.jjbc; break;
        case 54:	break; //val = folder->mstr.s054.jjbc; break;
        case 126:	break; //val = folder->mstr.s126.jjbc; break;
    }
    return val;
}

static double mstr_jsip(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.jsip; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.jsip; break;
        case 44:	break; //val = folder->mstr.s044.jsip; break;
        case 54:	break; //val = folder->mstr.s054.jsip; break;
        case 126:	break; //val = folder->mstr.s126.jsip; break;
    }
    return val;
}

static double mstr_gjip(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.gjip; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.gjip; break;
        case 44:	break; //val = folder->mstr.s044.gjip; break;
        case 54:	break; //val = folder->mstr.s054.gjip; break;
        case 126:	break; //val = folder->mstr.s126.gjip; break;
    }
    return val;
}

static int mstr_atmg(MARKET *market, MDFOLD *folder)
{
    int val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.atmg; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.atmg; break;
        case 44:	break; //val = folder->mstr.s044.atmg; break;
        case 54:	break; //val = folder->mstr.s054.atmg; break;
        case 126:	break; //val = folder->mstr.s126.atmg; break;
    }
    return val;
}

static double mstr_bdgc(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.bdgc; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.bdgc; break;
        case 44:	break; //val = folder->mstr.s044.bdgc; break;
        case 54:	break; //val = folder->mstr.s054.bdgc; break;
        case 126:	break; //val = folder->mstr.s126.bdgc; break;
    }
    return val;
}

static char* mstr_yorn(MARKET *market, MDFOLD *folder)
{
    char* val = "";
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.yorn; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.yorn; break;
        case 44:	break; //val = folder->mstr.s044.yorn; break;
        case 54:	break; //val = folder->mstr.s054.yorn; break;
        case 126:	break; //val = folder->mstr.s126.yorn; break;
    }
    return val;
}

static uint32_t mstr_cymd(MARKET *market, MDFOLD *folder)
{
    uint32_t val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.cymd; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.cymd; break;
        case 44:	break; //val = folder->mstr.s044.cymd; break;
        case 54:	break; //val = folder->mstr.s054.cymd; break;
        case 126:	break; //val = folder->mstr.s126.cymd; break;
    }
    return val;
}

static double mstr_ivol(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ivol; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.ivol; break;
        case 44:	break; //val = folder->mstr.s044.ivol; break;
        case 54:	break; //val = folder->mstr.s054.ivol; break;
        case 126:	break; //val = folder->mstr.s126.ivol; break;
    }
    return val;
}

static double mstr_hhip(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.hhip; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.hhip; break;
        case 44:	break; //val = folder->mstr.s044.hhip; break;
        case 54:	break; //val = folder->mstr.s054.hhip; break;
        case 126:	break; //val = folder->mstr.s126.hhip; break;
    }
    return val;
}

static uint32_t mstr_hhid(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.hhid; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.hhid; break;
        case 44:	break; //val = folder->mstr.s044.hhid; break;
        case 54:	break; //val = folder->mstr.s054.hhid; break;
        case 126:	break; //val = folder->mstr.s126.hhid; break;
    }
	return val;
}

static double mstr_hlop(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.hlop; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.hlop; break;
        case 44:	break; //val = folder->mstr.s044.hlop; break;
        case 54:	break; //val = folder->mstr.s054.hlop; break;
        case 126:	break; //val = folder->mstr.s126.hlop; break;
    }
    return val;
}

static uint32_t mstr_hlod(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.hlod; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.hlod; break;
        case 44:	break; //val = folder->mstr.s044.hlod; break;
        case 54:	break; //val = folder->mstr.s054.hlod; break;
        case 126:	break; //val = folder->mstr.s126.hlod; break;
    }
	return val;
}

static double mstr_ahip(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ahip; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.ahip; break;
        case 44:	break; //val = folder->mstr.s044.ahip; break;
        case 54:	break; //val = folder->mstr.s054.ahip; break;
        case 126:	break; //val = folder->mstr.s126.ahip; break;
    }
    return val;
}

static uint32_t mstr_ahid(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ahid; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.ahid; break;
        case 44:	break; //val = folder->mstr.s044.ahid; break;
        case 54:	break; //val = folder->mstr.s054.ahid; break;
        case 126:	break; //val = folder->mstr.s126.ahid; break;
    }
	return val;
}

static double mstr_alop(MARKET *market, MDFOLD *folder)
{
    double val = 0;
    switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.alop; break;
        case 4:
        case 5:
        case 6:		val = folder->mstr.s004.alop; break;
        case 44:	break; //val = folder->mstr.s044.alop; break;
        case 54:	break; //val = folder->mstr.s054.alop; break;
        case 126:	break; //val = folder->mstr.s126.alop; break;
    }
    return val;
}

static uint32_t mstr_alod(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.alod; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.alod; break;
        case 44:	break; //val = folder->mstr.s044.alod; break;
        case 54:	break; //val = folder->mstr.s054.alod; break;
        case 126:	break; //val = folder->mstr.s126.alod; break;
    }
	return val;
}

static uint32_t mstr_abdc(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.abdc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.abdc; break;
        case 44:	break; //val = folder->mstr.s044.abdc; break;
        case 54:	break; //val = folder->mstr.s054.abdc; break;
        case 126:	break; //val = folder->mstr.s126.abdc; break;
    }
	return val;
}

static uint32_t mstr_mtdc(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.mtdc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.mtdc; break;
        case 44:	break; //val = folder->mstr.s044.mtdc; break;
        case 54:	break; //val = folder->mstr.s054.mtdc; break;
        case 126:	break; //val = folder->mstr.s126.mtdc; break;
    }
	return val;
}

static uint32_t mstr_ytdc(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ytdc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.ytdc; break;
        case 44:	break; //val = folder->mstr.s044.ytdc; break;
        case 54:	break; //val = folder->mstr.s054.ytdc; break;
        case 126:	break; //val = folder->mstr.s126.ytdc; break;
    }
	return val;
}

static double mstr_cdir(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.cdir; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.cdir; break;
        case 44:	break; //val = folder->mstr.s044.cdir; break;
        case 54:	break; //val = folder->mstr.s054.cdir; break;
        case 126:	break; //val = folder->mstr.s126.cdir; break;
    }
	return val;
}

static uint32_t mstr_opiq(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.opiq; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.opiq; break;
        case 44:	break; //val = folder->mstr.s044.opiq; break;
        case 54:	break; //val = folder->mstr.s054.opiq; break;
        case 126:	break; //val = folder->mstr.s126.opiq; break;
    }
	return val;
}

static char* mstr_grop(MARKET *market, MDFOLD *folder)
{
	char* val = "";
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.grop; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.grop; break;
        case 44:	break; //val = folder->mstr.s044.grop; break;
        case 54:	break; //val = folder->mstr.s054.grop; break;
        case 126:	break; //val = folder->mstr.s126.grop; break;
    }
	return val;
}

static double mstr_opsr(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.opsr; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.opsr; break;
        case 44:	break; //val = folder->mstr.s044.opsr; break;
        case 54:	break; //val = folder->mstr.s054.opsr; break;
        case 126:	break; //val = folder->mstr.s126.opsr; break;
    }
	return val;
}

static char* mstr_lpcc(MARKET *market, MDFOLD *folder)
{
	char* val = "";
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.lpcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.lpcc; break;
        case 44:	break; //val = folder->mstr.s044.lpcc; break;
        case 54:	break; //val = folder->mstr.s054.lpcc; break;
        case 126:	break; //val = folder->mstr.s126.lpcc; break;
    }
	return val;
}

static char* mstr_mpcc(MARKET *market, MDFOLD *folder)
{
	char* val = "";
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.mpcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.mpcc; break;
        case 44:	break; //val = folder->mstr.s044.mpcc; break;
        case 54:	break; //val = folder->mstr.s054.mpcc; break;
        case 126:	break; //val = folder->mstr.s126.mpcc; break;
    }
	return val;
}

static char* mstr_cpcc(MARKET *market, MDFOLD *folder)
{
	char* val = "";
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.cpcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.cpcc; break;
        case 44:	break; //val = folder->mstr.s044.cpcc; break;
        case 54:	break; //val = folder->mstr.s054.cpcc; break;
        case 126:	break; //val = folder->mstr.s126.cpcc; break;
    }
	return val;
}

static char* mstr_ypcc(MARKET *market, MDFOLD *folder)
{
	char* val = "";
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.ypcc; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static int mstr_psetc(MARKET *market, MDFOLD *folder)
{
	int val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.setc; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static double mstr_psetp(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.setp; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static double mstr_pjidr(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.jidr; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static int mstr_pycgb(MARKET *market, MDFOLD *folder)
{
	int val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.ycgb; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static double mstr_pclos(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.clos; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static double mstr_popen(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.open; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static double mstr_phigh(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.high; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static double mstr_plow(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.low; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static uint32_t mstr_pchms(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.chms; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}


static uint32_t mstr_popin(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.opin; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static uint32_t mstr_pccnt(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.ccnt; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static double mstr_ptvol(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.tvol; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static double mstr_ptamt(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.tamt; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static double mstr_pmvol(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.mvol; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static double mstr_pmamt(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.mamt; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static double mstr_ppask(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.pask; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static double mstr_ppbid(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.pbid; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static double mstr_pfqty(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.fqty; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static double mstr_pfamt(MARKET *market, MDFOLD *folder)
{
	double val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.ypcc; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.p.famt; break;
        case 44:	break; //val = folder->mstr.s044.ypcc; break;
        case 54:	break; //val = folder->mstr.s054.ypcc; break;
        case 126:	break; //val = folder->mstr.s126.ypcc; break;
    }
	return val;
}

static int mstr_xage(MARKET *market, MDFOLD *folder)
{
	int val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.xage; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.xage; break;
        case 44:	val = folder->mstr.s044.xage; break;
        case 54:	val = folder->mstr.s054.xage; break;
        case 126:	val = folder->mstr.s126.xage; break;
    }
	return val;
}

static int mstr_stat(MARKET *market, MDFOLD *folder)
{
	int val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.stat; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.stat; break;
        case 44:	val = folder->mstr.s044.stat; break;
        case 54:	val = folder->mstr.s054.stat; break;
        case 126:	val = folder->mstr.s126.stat; break;
    }
	return val;
}

static int mstr_jchk(MARKET *market, MDFOLD *folder)
{
	int val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.jchk; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.jchk; break;
        case 44:	val = folder->mstr.s044.jchk; break;
        case 54:	val = folder->mstr.s054.jchk; break;
        case 126:	val = folder->mstr.s126.jchk; break;
    }
	return val;
}

static uint32_t mstr_pymd(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.pymd; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.pymd; break;
        case 44:	val = folder->mstr.s044.pymd; break;
        case 54:	val = folder->mstr.s054.pymd; break;
        case 126:	val = folder->mstr.s126.pymd; break;
    }
	return val;
}

static uint32_t mstr_tymd(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		break; //val = folder->mstr.s001.tymd; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.tymd; break;
        case 44:	val = folder->mstr.s044.tymd; break;
        case 54:	val = folder->mstr.s054.tymd; break;
        case 126:	val = folder->mstr.s126.tymd; break;
    }
	return val;
}




static uint32_t mstr_zymd(MARKET *market, MDFOLD *folder)
{
	uint32_t val = 0;
	switch (market->xchg->schm)
    {
        case 1:		val = folder->mstr.s001.zymd; break; 
		case 4:
        case 5:
        case 6:		val = folder->mstr.s004.zymd; break;
        case 44:	val = folder->mstr.s044.zymd; break;
        case 54:	val = folder->mstr.s054.zymd; break;
        case 126:	val = folder->mstr.s126.zymd; break;
    }
	return val;
}
#endif
