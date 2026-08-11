#ifndef	__MDHEOD_H__
#define	__MDHEOD_H__
#include "mds.h"

//
// MDHEOD schema
//
#ifdef __cplusplus
extern "C" {
#endif

typedef	struct	{
	char	 symb[SYMB_LEN];	// 종목코드
	char	 knam[80];		// 한글명
	char	 exnm[8];		// 거래소이름
	int		 zdiv;			// 소수점자리수
	uint32_t xymd;			// 일자
	double	 open;			// 시가
	double	 high;			// 고가
	double	 low;			// 저가
	double	 last;			// 최종가격
	double	 clos;			// 종가(정산가)
	int		 sign;			// SIGN
	double	 diff;			// 전일대비
	double	 rate;			// 전일대비등락율
	double   tvol;			// 누적체결수량
	double	 tamt;			// 누적거래대금
	uint32_t opin;			// 미결제약정수량
	uint32_t seqn;			// sequence number 
	double	 upvo;			// 
	double	 dnvo;			// 
	uint32_t upno;			// 
	uint32_t dnno;			// 
	uint32_t shms;			// 시작시간(KST)
	uint32_t ehms;			// 종료시간(KST)
	uint32_t sseq;			// last sequence number in 1 seccond
	char	 fill[28];		// filler
} MDHEOD;


#ifdef	_SCHEMA_H_
#define OF_HEOD(member)    ((long) &(((MDHEOD*)0)->member))
#define SZ_HEOD(member)    sizeof(((MDHEOD*)0)->member)


#endif

#ifdef __cplusplus
}
#endif
#endif
