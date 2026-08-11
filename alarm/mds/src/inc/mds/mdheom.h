#ifndef	__MDHEOM_H__
#define	__MDHEOM_H__
#include "mds.h"

//
// MDHEOM schema
//
#ifdef __cplusplus
extern "C" {
#endif
typedef	struct {
	char	symb[SYMB_LEN];		// symbol(PK)	
	uint32_t xymd;			// 일자 (YYYYMM00)
	int	 nday;			// 입회일수
	double	 open;			// 시가
	double	 high;			// 고가
	double	 low;			// 저가
	double	 clos;			// 종가
	int	 sign;			// 대비구분
	double	 diff;			// 대비
	double	 rate;			// 등락율
	double   tvol;			// 누적거래량
	double	 tamt;			// 누적거래대금
} MDHEOM;

/*
typedef	HEOM000	HEOM001;
typedef	HEOM000	HEOM004;
typedef	HEOM000	HEOM005;
typedef	HEOM000	HEOM006;
typedef	HEOM000	HEOM044;
typedef	HEOM000	HEOM054;
typedef	HEOM000	HEOM126;
typedef	HEOM000	HEOM999;

typedef	HEOM004	HEOM000;
typedef union {
	HEOM000 s000;
	HEOM001 s001;
	HEOM004 s004;
	HEOM005	s005;
	HEOM006	s006;
	HEOM044	s044;
	HEOM054	s054;
	HEOM126 s126;
	HEOM999 s999;
} MDHEOM;
*/


#ifdef __cplusplus
}
#endif
#endif
