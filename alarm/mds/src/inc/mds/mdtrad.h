#ifndef	__MDTRAD_H__
#define	__MDTRAD_H__
#include "mds.h"

//
// MDTRAD schema
//
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SAVEORDER	1000  //종목당 최대 저장주문체결데이터 개수 . 1분정도 주문만 필요해서 크지 않아도됨.


typedef	struct	{
	//char	 symb[SYMB_RLEN];// 종목코드
	//uint32_t xymd;			// 체결일자
	uint32_t xhms;			// 체결시간
	uint32_t exymd;			// 체결유효일자
	uint32_t exhms;			// 주문유효시간
	int		 nhoga;			// 체결호가
	double   dAmount;		// 체결금액
	//double dprice;
	//int		 ncount
} DATAFIELD;

typedef	struct	{
	char	 symb[SYMB_LEN];
	uint32_t xymd;			// 한국일자 
	uint32_t xhms;			// 한국시간 (HHMMSS)
	int  nStart;
	int  nEnd;
	DATAFIELD orderdata[MAX_SAVEORDER];
} MDTRAD;

#ifdef	_SCHEMA_H_
#define OF_TRAD(member)    ((long) &(((MDTRAD*)0)->member))
#define SZ_TRAD(member)    sizeof(((MDTRAD*)0)->member)


#endif


#ifdef __cplusplus
}
#endif
#endif
