#ifndef	__MDBOOK_H__
#define	__MDBOOK_H__
#include "mds.h"
#include "mdcommon.h"
//
// MDBOOK schema
//
#ifdef __cplusplus
extern "C" {
#endif

#define BOOK_LEVEL 10
#define	DISPLAYBOOK_LEVEL 5
typedef struct {
    char	pask[20];          // 매도호가
    char	vask[20];          // 매도호가 잔량
    char	pbid[20];          // 매수호가
		char	vbid[20];          // 매수호가 잔량
} DEPT;
typedef struct {
	char     symb[SYMB_RLEN];// 종목코드
    char	 xymd[8];          // 일자 
    char 	 xhms[6];          // 호가시간
    DEPT  	 price[BOOK_LEVEL];   // 호가
} S_SENDBOOK;





 
typedef struct  {
	char     symb[SYMB_RLEN];// 종목코드
	uint32_t xymd;          // 일자
	uint32_t xhms;			// 호가시간
	uint32_t kymd;          // 일자
	uint32_t khms;			// 호가시간
	int  qoutsource ;  ///해당 시세 원천 
	double		usdkrwbidpirce;	//반영당시의 USDKRW가격
	double		usdkreaskprice;	//반영당시의 USDKRW가격
	//	BOOKSTRUCT    spotdata;
	//  BOOKSTRUCT    foworddata[MAX_TENNER];
	MDBOOKSTRUCT    spotdata;
	MDBOOKSTRUCT    foworddata[MAX_TENNER];
} MDBOOK;


#ifdef	_SCHEMA_H_
#define OF_BOOK(member)    ((long) &(((MDBOOK*)0)->member))
#define SZ_BOOK(member)    sizeof(((MDBOOK*)0)->member)



#endif

#ifdef __cplusplus
}
#endif
#endif
