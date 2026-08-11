#ifndef	__MDQUOT_H__
#define	__MDQUOT_H__
//#include "mds.h"
#include "mdcommon.h"
#define MAX_QUOT    5
//
// Quote schema
//
#ifdef __cplusplus
extern "C" {
#endif




struct virtualquot{
    double  bidprice;
    int     bidqty;
    double  offerprice;
    int     offerqty;
};
 

typedef	struct	{
	char    		symb[SYMB_RLEN];
	int					sseq;
	int					seqn;
	uint32_t    tymd;
	uint32_t    xymd;
	uint32_t    xhms;
	uint32_t    kymd;
	uint32_t    khms;
	int   		bidmarkup;		//가상잔량 마크업 틱
	int    		askmarkup;		//가상잔량 마크업 틱
	double		usdkrwbidpirce;	//반영당시의 USDKRW가격
	double		usdkreaskprice;	//반영당시의 USDKRW가격
	struct      q_price     spotdata;
	struct      q_price     foworddata[MAX_TENNER];
	struct      q_price     swapdata[MAX_TENNER];
	struct      q_Markup    markupdata[MAX_TENNER];
} MDQUOT;


#ifdef	_SCHEMA_H_
#define OF_QUOT(member)    ((long) &(((MDQUOT *)0)->member))
#define SZ_QUOT(member)    sizeof(((MDQUOT *)0)->member)

struct	keydesc mdquot_key[] = {
	{ ISNODUPS, 1, 
	  {{ OF_QUOT(symb), SZ_QUOT(symb), CHARTYPE }}},
	{       -1, 0,
	  {{               0,               0,        0 }}},
};
#endif

#ifdef __cplusplus
}
#endif
#endif
