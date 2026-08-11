#ifndef	__MDTICK_H__
#define	__MDTICK_H__
#include "mds.h"

//
// MDTICK schema
//
#ifdef __cplusplus
extern "C" {
#endif

typedef	struct	{
	char	 symb[SYMB_LEN];// 종목코드
	uint32_t tymd;			// trading date
	uint32_t xymd;			// 한국일자 
	uint32_t xhms;			// 한국시간 (HHMMSS)
	uint32_t seqn;			// sequencial number in same trading day
	uint32_t sseq;			// sequencial number in same time(second)
	uint32_t yymd;			// 거래소일자
	uint32_t yhms;			// 거래소시간
	double	 last;			// 체결가
	int		 sign;			// SIGN
	double	 diff;			// 전일대비
	double	 rate;			// 전일대비등락율
	double   cvol;			// 체결수량
	double   tvol;			// 누적체결수량
	double	 tamt;			// 누적거래대금
	int		 side;			// 매도/매수체결 구분 ('+' : 매수체결, '-' : 매도체결)
	int		 dirf;			// 직전대비 가격방향
	double	 pask;			// 최우선매도호가
	double	 pbid;			// 최우선매수호가
	double	 bvol;			// 매수체결수량 합계	
	double	 svol;			// 매도체결수량 합계	
	uint32_t opin;			// 미결제잔량
	double	 pcls;			// previous day close price
	int		 udtx;			// lastest traded up/down type
	int		 fill[3];
} MDTICK;


#ifdef	_SCHEMA_H_
#define OF_TICK(member)    ((long) &(((MDTICK *)0)->member))
#define SZ_TICK(member)    sizeof(((MDTICK *)0)->member)

#endif


#ifdef __cplusplus
}
#endif
#endif
