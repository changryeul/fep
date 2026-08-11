#ifndef	__MDINTR_H__
#define	__MDINTR_H__
#include "mds.h"

//
// MDINTR schema
//
#ifdef __cplusplus
extern "C" {
#endif

typedef	struct	{
	char	 symb[SYMB_LEN];// 종목코드
	uint32_t xymd;			// 한국 일자 
	uint32_t xhms;			// 한국 시간 HHMM00 
	uint32_t yymd;			// 거래소 일자
	uint32_t yhms;			// 거래소 시간 HHMM00
	uint32_t tymd;			// trading day
	double	 open;			// 시가
	double	 high;			// 고가
	double	 low;			// 저가
	double	 clos;			// 종가
	double   cvol;			// 거래량
	double   tvol;			// 누적 거래량
	double	 tamt;			// 거래대금
	int		 flush;			// flushed 

	uint32_t	bvol;			// 매수체결수량 합계
	uint32_t	svol;			// 매도체결수량 함계
	double		upvo;			// 직전대비 상승거래량
	double		dnvo;			// 직전대비 하락거래량
	uint32_t	upno;			// 직전대비 uptick 건수
	uint32_t	dnno;			// 직전대비 downtick 건수
	double		pcls;			// previous day close price
	uint32_t	opin;			// open interest
	uint32_t	seqn;			// sequence number in day
	uint32_t	sseq;			// sequence number in seccond
	uint32_t	kfrhm;			// start time hhmm (KOR)
	char		fill[32];		// filler

} MDINTR;

#ifdef	_SCHEMA_H_
#define OF_INTR(member)    ((long) &(((MDINTR*)0)->member))
#define SZ_INTR(member)    sizeof(((MDINTR*)0)->member)

struct	keydesc mdintr_key[] = {
	{ ISNODUPS, 3, 
	  {{ OF_INTR(symb), SZ_INTR(symb), CHARTYPE },
	   { OF_INTR(xymd), SZ_INTR(xymd), LONGTYPE|ISDESC },
	   { OF_INTR(xhms), SZ_INTR(xhms), LONGTYPE|ISDESC }}},
	{       -1, 0,
	  {{               0,               0,        0 }}},
};
#endif


#ifdef __cplusplus
}
#endif
#endif
