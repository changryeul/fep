#ifndef _OTP_AGENT_H_
#define _OTP_AGENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* otp_agent.h */
/********************************************************/
/* Copyright 2007 Cyclops Inc.                    */
/********************************************************/


/* 자행,타행 구분 값 */
#define IBK_ORG_CODE                    "1"
#define OTHER_ORG_CODE                  "2"


/********************************************************/
/* 각 필드 길이             */
/********************************************************/
#define LEN_RES_CODE            6
#define LEN_ERR_COUNT           2
#define LEN_BIZ_CODE            6
#define LEN_OTHER_ORG           1

#define LEN_VENDER_CODE 3
#define LEN_USER_CODE           13
#define LEN_TOKEN_SERIAL        12
#define LEN_TOKEN_CODE          8

#define LEN_AUTH_DATE           8
#define LEN_AUTH_TIME           6

/********************************************************/
/* 입력 Structure            */
/********************************************************/
typedef struct OTP_DATA  {
        char resCode[LEN_RES_CODE+2];
        char otherOrg[LEN_OTHER_ORG+2];                 /* 타행 2, 당행 1 */
        char venderCode[LEN_VENDER_CODE+2];             /* HOST 조회 시 가져옴 */
        char userCode[LEN_USER_CODE+2];                 /* 실명번호(텔레뱅킹), 이용자번호(인터넷뱅킹) */
        char tokSerial[LEN_TOKEN_SERIAL+2];             /* HOST 조회 시 가져옴 */
        char tokCode[LEN_TOKEN_CODE+2];                 /* OTP 응답값 -> 이용자 입력 */
        char lastAuthDate[LEN_AUTH_DATE+2];             /* 마지막 인증 성공일자 */
        char lastAuthTime[LEN_AUTH_TIME+2];             /* 마지막 인증 성공시간 */
        char errCount[LEN_ERR_COUNT+2];
}stOtpData;

/*===========================================================================
FUNCTION                : Af_AuthOtpUser
PARAMETERS              : in 입력 구조체
                              out 결과 구조체
DESCRIPTION             : OTP 인증 요구
DEPENDENCIES    : None
RETURNS                 : return code
SIDE EFFECTS    : None
===========================================================================*/
 extern void Af_AuthOtpUser(stOtpData *in, stOtpData *out);

/*===========================================================================
FUNCTION                : Af_ResyncOtpUser
PARAMETERS              : in 입력 구조체
                                  out 결과 구조체
DESCRIPTION             : OTP 보정 요구
DEPENDENCIES    : None
RETURNS                 : return code 
SIDE EFFECTS    : None
===========================================================================*/
extern void Af_ResyncOtpUser(stOtpData *in, stOtpData *out );

#ifdef __cplusplus
}
#endif

#endif