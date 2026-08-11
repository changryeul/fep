#include "context.h"
#include "mdfold.h"
//#include "schema.h"

//
// EOD to EOW
//
//int mds_eod2eow(MDHEOW *mdheow, MDHEOD *mdheod, int neod)
//{
//	int	neow = 0;
//	int	year = -1, ymd, yy;
//	int	wday = -1, week;
//	int	ja = 0, js = 0, jd;
//	int	ii;
//	
//	MDHEOD *eod = mdheod;
//	MDHEOW *eow = mdheow;
//
//	eow[0].nday = 0;
//	for (ii = 0; ii < neod; ii++)
//	{
//		yy = YEAR(eod[ii].xymd);
//		if (yy != year)
//		{
//			year = yy;
//			ymd = YMD(year, 1, 1);
//			wday = mds_day4week(ymd);
//			ja = 7 - wday;			// 3
//			js = mds_date2julian(ymd);
//		}
//		jd = mds_date2julian(eod[ii].xymd);
//		jd -= js;
//		jd += ja;
//		week = jd / 7;
//		if (week != wday)
//		{
//			if (ii != 0)
//				neow++;
//			memset(&eow[neow], 0, sizeof(MDHEOW));
//			strcpy(eow[neow].symb, eod[0].symb);
//			eow[neow].xymd = eod[ii].xymd;
//			eow[neow].zymd = eod[ii].xymd;
//			eow[neow].nday++;
//			eow[neow].open = eod[ii].open;
//			eow[neow].high = eod[ii].high;
//			eow[neow].low  = eod[ii].low;
//			eow[neow].clos = eod[ii].clos;
//			eow[neow].tvol = eod[ii].tvol;
//			eow[neow].tamt = eod[ii].tamt;
//			wday = week;
//		}
//		else
//		{
//			eow[neow].xymd = eod[ii].xymd;
//			eow[neow].nday++;
//			if (eod[ii].high > eow[neow].high)
//				eow[neow].high = eod[ii].high;
//			if (eod[ii].low < eow[neow].low)
//				eow[neow].low  = eod[ii].low;
//			eow[neow].open = eod[ii].open;
//			eow[neow].tvol += eod[ii].tvol;
//			eow[neow].tamt += eod[ii].tamt;
//		}
//	}
//	if (eow[neow].nday > 0)
//		neow++;
//	for (ii = 0; ii < neow; ii++)
//	{
//		if (ii == neow-1)
//		{
//			eow[ii].sign = _NC_;
//			eow[ii].diff = 0.;
//			eow[ii].rate = 0.;
//			continue;
//		}
//		eow[ii].diff = eow[ii].clos - eow[ii+1].clos;
//		if (eow[ii].diff > 0)		eow[ii].sign = _UP_;
//		else if (eow[ii].diff < 0)	eow[ii].sign = _DN_;
//		else 				eow[ii].sign = _NC_;
//		eow[ii].rate = (eow[ii].diff * 100) / eow[ii+1].clos;
//	}
//	return(neow);
//}
//
////
//// EOD to EOM
////
//int mds_eod2eom(MDHEOM *mdheom, MDHEOD *mdheod, int neod)
//{
//	int	neom = 0;
//	int	yy, mm, ymd, pym = -1;
//	int	ii;
//
//	MDHEOD *eod = mdheod;
//	MDHEOM *eom = mdheom;
//	eom[0].nday = 0;
//	for (ii = 0; ii < neod; ii++)
//	{
//		yy = YEAR(eod[ii].xymd);
//		mm = MONTH(eod[ii].xymd);
//		ymd = YMD(yy, mm, 0);
//
//		if (ymd != pym)
//		{
//			if (ii != 0)
//				neom++;
//			memset(&eom[neom], 0, sizeof(MDHEOM));
//			strcpy(eom[neom].symb, eod[0].symb);
//			eom[neom].xymd = ymd;
//			eom[neom].nday++;
//			eom[neom].open = eod[ii].open;
//			eom[neom].high = eod[ii].high;
//			eom[neom].low  = eod[ii].low;
//			eom[neom].clos = eod[ii].clos;
//			eom[neom].tvol = eod[ii].tvol;
//			eom[neom].tamt = eod[ii].tamt;
//			pym = ymd;
//		}
//		else
//		{
//			eom[neom].nday++;
//			if (eod[ii].high > eom[neom].high)
//				eom[neom].high = eod[ii].high;
//			if (eod[ii].low < eom[neom].low)
//				eom[neom].low  = eod[ii].low;
//			eom[neom].open = eod[ii].open;
//			eom[neom].tvol += eod[ii].tvol;
//			eom[neom].tamt += eod[ii].tamt;
//		}
//	}
//	if (eom[neom].nday > 0)
//		neom++;
//	for (ii = 0; ii < neom; ii++)
//	{
//		if (ii == neom-1)
//		{
//			eom[ii].sign = _NC_;
//			eom[ii].diff = 0.;
//			eom[ii].rate = 0.;
//			continue;
//		}
//		eom[ii].diff = eom[ii].clos - eom[ii+1].clos;
//		if (eom[ii].diff > 0)		eom[ii].sign = _UP_;
//		else if (eom[ii].diff < 0)	eom[ii].sign = _DN_;
//		else 				eom[ii].sign = _NC_;
//		eom[ii].rate = (eom[ii].diff * 100) / eom[ii+1].clos;
//	}
//	return(neom);
//}
