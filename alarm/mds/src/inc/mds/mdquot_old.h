#ifndef	__MDQUOT_OLD_H__
#define	__MDQUOT_OLD_H__
#include "mds.h"

//
// Quote schema
//
#ifdef __cplusplus
extern "C" {
#endif


typedef	struct	{
	char	 symb[SYMB_LEN];	/* symbol(PK) 			*/
	uint32_t xymd;			/* date 			*/
	uint32_t xhms;			/* time 			*/
	uint32_t kymd;			/* local date			*/
	uint32_t khms;			/* local time 			*/
	uint32_t tymd;			/* trading day			*/
	uint32_t symd;			/* date for settlment price	*/
	uint32_t seqn;			/* sequence number		*/
	double	 setp;			/* settlement price		*/
	double	 base;			/* base price			*/
	double	 open;			/* open				*/
	double	 high;			/* High				*/
	double	 low;			/* Low				*/
	double	 last;			/* Last				*/
	double	 diff;			/* diff	(signed)		*/
	double	 rate;			/* rate (signed)		*/
	int	 sign;			/* sign				*/
	double	 cvol;			/* traded volume		*/
	double	 tvol;			/* Curm. volume			*/
	double	 tamt;			/* 누적거래대금			*/
	double	 pask;			/* top of ask price		*/
	double	 pbid;			/* top of bid price		*/
	uint32_t opin;			/* open interest		*/
					/* -- additional information--	*/
	int	 tick;			/* if TRUE, tick 		*/
	int	 side;			/* 매도/매수체결 구분 Tick SIDE :+,-,=		*/
	int	 dirf;			/* 직전대비체결가 tick direction : '+','-'	*/
	int	 tsid;			/* trading session id		*/
	int	 stat;			/* trading status		*/
					/* -- statistics --		*/
	double 	bvol;			/* contracted volumd on bid price */
	double	svol;			/* contracted volumd on ask price */
	uint32_t bcnt;			/* contracted counts on bid price */
	uint32_t scnt;			/* contracted counts on bid price */

	uint32_t otim;			/* open time(HHMMSS)		*/
	uint32_t htim;			/* high time(HHMMSS)		*/
	uint32_t ltim;			/* low time(HHMMSS)		*/
    double   delt;          // 옵션민감도 : 델타
    double   thet;          //              세타
    double   vega;          //				베가
    double   gama;          //              감마
    double   rho;           //              로우
} MDQUOT_OLD;

#ifdef	_SCHEMA_H_
#define OF_QUOT(member)    ((long) &(((MDQUOT_OLD *)0)->member))
#define SZ_QUOT(member)    sizeof(((MDQUOT_OLD *)0)->member)

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
