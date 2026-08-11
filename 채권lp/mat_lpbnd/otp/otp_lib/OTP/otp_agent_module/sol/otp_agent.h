/* otp_agent.h */
/********************************************************/
/* Copyright 2007 Cyclops Inc.                    */
/********************************************************/


/********************************************************/
/* 인증 결과 코드 */
/********************************************************/
#define OTP_000	"OTP000"     	/* 인증 성공 */
#define OTP_001	"OTP001"     	/* 인증 실 패  -> 오류횟수를 증가 */
#define OTP_002	"OTP002"     	/* 보정 필요*/

/********************************************************/
/* 보정 결과 코드*/
/********************************************************/
#define OTP_003	"OTP003"     	/* 보정 성공  */
#define OTP_004	"OTP004"    	/* 보정 실 패 -> 오류횟수를 증가 */

/********************************************************/
/* 기타  결과 코드*/
/********************************************************/
#define OTP_005	"OTP005"     	/* 미정의 오류 - 내부 오류 */
#define OTP_006	"OTP006"    	/* 해당 OTP기기일련번호 없음 */
#define OTP_007	"OTP007"    	/* 입력값오류 */
#define OTP_008	"OTP008"     	/* DB에러 */
#define OTP_009	"OTP009"     	/* 잠김해제 실 패 */
#define OTP_010	"OTP010"     	/* OTP응답값 입력형식 오류  */
#define OTP_011  "OTP200"		/* Session 실패 - 연결 실패  */
#define OTP_201  "OTP201"       /* 초기화 실패 */
#define OTP_202  "OTP202"       /* 타임 아웃*/

/********************************************************/
/* 각 필드 길이             */
/********************************************************/
#define LEN_RES_CODE		6
#define LEN_ERR_COUNT		2
#define LEN_BIZ_CODE 		6
#define LEN_OTHER_ORG		1

#define LEN_VENDER_CODE	3
#define LEN_USER_CODE		13
#define LEN_TOKEN_SERIAL	12
#define LEN_TOKEN_CODE		8

/********************************************************/
/* 입력 Structure            */
/********************************************************/
typedef struct OTP_DATA  {
	int otherOrg;           						/* 타행 1, 자행 0 */
	char venderCode[LEN_VENDER_CODE+2];   	/* HOST 조회 시 가져옴 */
	char userCode[LEN_USER_CODE+2];			/* 실명번호(텔레뱅킹), 이용자번호(인터넷뱅킹) */
	char tokSerial[LEN_TOKEN_SERIAL+2];  		/* HOST 조회 시 가져옴 */
	char tokCode[LEN_TOKEN_CODE+2];      		/* OTP 응답값 -> 이용자 입력 */
}stOtpData;

/*===========================================================================
FUNCTION		: Af_AuthOtpUser
PARAMETERS		: in 입력 구조체
                              retCode : 인증 수해 후, 결과값
				  errCount 인증 실패 시, 타행인 경우 에러 count
DESCRIPTION		: OTP 인증 요구
DEPENDENCIES	: None
RETURNS			: return code
SIDE EFFECTS	: None
===========================================================================*/
 void Af_AuthOtpUser(stOtpData *in, char *retCode, int *errCount);

/*===========================================================================
FUNCTION		: Af_ResyncOtpUser
PARAMETERS		: in 입력 구조체
				  retCode : 인증 수해 후, 결과값
DESCRIPTION		: OTP 보정 요구
DEPENDENCIES	: None
RETURNS			: return code 
SIDE EFFECTS	: None
===========================================================================*/
void Af_ResyncOtpUser(stOtpData *in, char *retCode );

