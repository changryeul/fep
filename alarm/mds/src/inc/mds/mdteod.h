#ifndef	__MDTEOD_H__
#define	__MDTEOD_H__
#include "mds.h"

//
// MDTEOD schema
//
#ifdef __cplusplus
extern "C" {
#endif

typedef	struct {
	char	 symb[SYMB_LEN];	// 상품 ID
	char	 who[4+1];		// 투자자유형
	uint32_t xymd;			// 일자
	double	 sqty[2];		// 매도약정수량 / 스프레드에 의한 약정
	double	 samt[2];		// 매도약정대금 / 스프레드에 의한 약정
	double	 bqty[2];		// 매수약정수량 / 스프레드에 의한 약정
	double	 bamt[2];		// 매수약정대금 / 스프레드에 의한 약정

} MDTEOD;

#ifdef	_SCHEMA_H_
#define OF_TEOD(member)    ((long) &(((MDTEOD*)0)->member))
#define SZ_TEOD(member)    sizeof(((MDTEOD*)0)->member)

struct	keydesc mdteod_key[] = {
	{ ISNODUPS, 3, 
	  {{ OF_TEOD(symb), SZ_TEOD(symb), CHARTYPE },
	   { OF_TEOD(who),  SZ_TEOD(who),  CHARTYPE },
	   { OF_TEOD(xymd), SZ_TEOD(xymd), LONGTYPE|ISDESC }}},
	{       -1, 0,
	  {{               0,               0,        0 }}},
};
#endif

#ifdef __cplusplus
}
#endif
#endif
