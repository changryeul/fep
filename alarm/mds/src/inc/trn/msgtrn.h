#ifndef	_MDTRAN_H_
#define	_MDTRAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/msg.h>

#define PN_TCP_SISE 9997
#define	PN_TCP_LIST 9998
#define      FCD_APBP        'P'             /* AP2BP  : ¿øAaSVR <-> ½A¼¼SVR */

#define	SYMB_SZ	20
#define	TP_MSTR	'M'		/* 종목마스터	*/
#define	TP_QUOT	'Q'		/* 종목현재가	*/
#define	TP_SETT	'S'		/* 종목정산가	*/

// feed format
typedef struct {
	char	type[1];			// 'M'
	char	isym[SYMB_SZ];		// internal symbol
	char	symb[SYMB_SZ];		// exchange symbol
	char	seqn[5];			// symbol sequence
	char	root[12];			// root symbol
	char	gsym[12];			// security group
	char	clrs[12];			// clearing symbol
	char	enam[128];			// english name
	char	exnm[8];			// exchange name
	char	stat[2];			// trading status
	char	unpd[2];			// underlying product
	char	unps[12];			// underlying product symbol
	char	pmul[12];			// price multiplier
	char	xdiv[6];			// price denominator
	char	ydiv[6];			// price numerator
	char	zdiv[6];			// no of decimals
	char	styp[1];			// instrument type : 'F': Future 'O': Option
	char	curr[3];			// trading currency 
	char	exym[6];			// expired year month : YYYYMM
	char	lymd[8];			// listed date(YYYYMMDD)
	char	zymd[8];			// expire dat : last trading day(YYYYMMDD)
	char	jjis[4];			// days to last trading day
	char	csiz[6];			// contract size
	char	pinc[12];			// minimum price increment
	char	opts[1];			// option : Style
	char	corp[1];			// Call or Put
	char	strk[12];			// Strike Price (종목코드 행사가격)

	// 2017.04.06 add  for span
	char	unp_mcod[1];			// 기초자산월물코드
	char	unp_root[12];			// 기초자산선물상품코드
	char	org_strk[12];			// 실제행사가격 (gemizip 수신 행사가격)
} TRNMSTR;

typedef struct {
	char	type[1];			// 'Q'
	char	isym[SYMB_SZ];		// internal symbol
	char	symb[SYMB_SZ];		// exchange symbol
	char	tymd[8];			// trading day
	char	xymd[8];			// date
	char	xhms[6];			// time
	char	kymd[8];			// KOR date
	char	khms[6];			// KOR time
	char	open[15];			// open
	char	high[15];			// high
	char	low[15];			// low
	char	last[15];			// last price
	char	sign[1];			// change sign
	char	diff[15];			// net change
	char	rate[6];			// change rate
	char	evol[7];			// traded volume
	char	tvol[9];			// Curm. volume
	char	opin[9];			// open interest
	char	side[1];			// tick side
	char	dirf[1];			// tick direction
	char	tsid[1];			// trading session id
	char	stat[2];			// trading status
} TRNQUOT;

typedef struct {
	char	type[1];			// 'S'
	char	isym[SYMB_SZ];		// internal rsymbol
	char	symb[SYMB_SZ];		// exchange symbol
	char	tymd[8];			// trrading day
	char	setp[15];			// settlement price
	char	diff[15];			// net change
	char	tvol[9];			// estimated volume
	char	opin[9];			// open interest
} TRNSETT;

typedef struct {
	char	code[16];		/* symbol code			*/
	char	pind[ 1];		/* price indicator		*/
	char	curr[15];		/* current price		*/
	char	stlp[15];		/* settlement price		*/
	char	sday[ 8];		/* settlement date		*/
	char	bday[ 8];		/* 영업일자			*/
} st_lt700;

#ifdef __cplusplus
}
#endif
#endif
