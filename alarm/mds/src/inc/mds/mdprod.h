#ifndef __MDPROD_H__
#define __MDPROD_H__

#ifdef __cplusplus
extern "C" {
#endif

// 서버 상품코드
#define UNPD_BM3_S	"BM3"				// 3년 국채
#define UNPD_BM5_S	"BM5"				// 4년 국채
#define UNPD_BMA_S	"BMA"				// 10년 국채
#define UNPD_USD_S	"USD"				// 미국달러
#define UNPD_JPY_S	"JPY"				// 엔
#define UNPD_EUR_S	"EUR"				// 유로
#define UNPD_CNH_S	"CNH"				// 위안
#define UNPD_LHG_S	"LHG"				// 돈육
#define UNPD_KGD_S	"KGD"				// 금
#define UNPD_K2I_S	"K2I"				// KOSPI 200
#define UNPD_MKI_S	"MKI"				// MINI KOSPI 200

#define MAX_PRODUCT 150

typedef struct {
	char	unpd[12];					// 기초자산코드(서버)
	char	uncd[8];					// 기초자산코드(종목마스트)
	char	clss[12];					// 구분
	char	enam[32];					// 영문명
	char	knam[32];					// 한글명
	char	cunpd[12];					// 기초자산코드(클라이언트)
	char	lunpd[12];					// 기초자산코드(원장에서 주는 코드)
	char	usymb[12];					// 기초자산종목코드(주식코드)
	char	marketgb[12];				// 마켓구분
	char	mini[12];					// 미니기초자산코드
	char	root[12];					// 미니상품인경우 원기초자산코드
	int		zdiv;						// 소수점자리수
	int 	option;						// 0: 옵션 없음 1:옵션 있음
	double  hvol;						// 90일 역사적변동성
	struct  { 
		char	future[8];				// 선물
		char	spread[8];				// 스프레드
		char	option[8];				// 옵션
	} optgb;
	struct {
		char	underlying[8];			// 기초자산이 있는 거래소코드
		char	future[2][8];			// 0:주간 1:야간 
		char	option[2][8];			// 0:주간 1:야간
	} exchange;

	struct {
		double hvol;					// 90일 역사적변동성
		double dint;					// 국내 이자율
		double fint;					// 국외 이자율
		double didx;					// 배당액지수
	} gjcd;								// 원장에서 읽어오는데이터
} PRODUCT;

typedef struct {
	char	key[12];
	int		index;
} prodkey_t;

typedef struct {
	time_t	  mtim;						// 파일수정시간
	int		  nrec;						// 데이터 개수
	prodkey_t key_unpd[MAX_PRODUCT];	// 기초자산검색키(거래소코드)
	prodkey_t key_cunpd[MAX_PRODUCT];	// 기초자산검색키(클라이언트코드)
	prodkey_t key_lunpd[MAX_PRODUCT];	// 기초자산검색키(원장코드)
	prodkey_t key_usymb[MAX_PRODUCT];	// 기조자산검색키(종목코드:주식)
	PRODUCT	  prod[MAX_PRODUCT];		
	char	  tuja_future_prefix[12];
	char	  tuja_option_prefix[12];
} MDPROD;

#ifdef __cplusplus
}
#endif

#endif
