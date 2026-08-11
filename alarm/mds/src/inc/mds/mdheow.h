#ifndef	__MDHEOW_H__
#define	__MDHEOW_H__
#include "mds.h"

//
// MDHEOW schema
//
#ifdef __cplusplus
extern "C" {
#endif
typedef	struct {
	char	symb[SYMB_LEN];		// symbol(PK)	
	uint32_t xymd;			// 주초일자
	uint32_t zymd;			// 주말일자
	int		nday;			// 입회일수
	double	 open;			// 시가
	double	 high;			// 고가
	double	 low;			// 저가
	double	 clos;			// 종가
	int		sign;			// 대비구분
	double	 diff;			// 대비
	double	 rate;			// 등락율
	double 	 tvol;			// 누적거래량 
	double	 tamt;			// 누적거래대금
} MDHEOW;

/*
typedef	HEOW000	HEOW001;
typedef	HEOW000	HEOW004;
typedef	HEOW000	HEOW005;
typedef	HEOW000	HEOW006;
typedef	HEOW000	HEOW044;
typedef	HEOW000	HEOW054;
typedef	HEOW000	HEOW126;
typedef	HEOW000	HEOW999;

typedef union {
	HEOW000 s000;
	HEOW001 s001;
	HEOW004 s004;
	HEOW005	s005;
	HEOW006	s006;
	HEOW044	s044;
	HEOW054	s054;
	HEOW126 s126;
	HEOW999 s999;
} MDHEOW;
*/


#ifdef __cplusplus
}
#endif
#endif
