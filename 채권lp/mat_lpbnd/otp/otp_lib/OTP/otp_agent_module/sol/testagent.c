/* otp_rxfsa.c */
/********************************************************/
/* Copyright 2005 Vid Technology.                       */
/********************************************************/
#include <stdio.h>
#include <stdlib.h>


#include "otp_agent.h"

/***************************************************************************************
* main()
* This is main function of banking protocol processing 
* which communicate with each Banking server(Tele, Mobile, Internet) on TCP/IP protocol 
****************************************************************************************/
main(int arg, char *args[])
{
	stOtpData otpData;
	char retCode[LEN_RES_CODE+2];
	int errCount;

	if( arg < 2 )  {
		printf("\n usage : " );
		printf("\n testagent args1 args2");
		printf("\n args1 -> 0:인증자행 1:인증타행, 2:보정자행 3:보정타행  \n\n");
		return;
	}
	
	memset( &otpData, 0x00, sizeof(stOtpData) );
	memset( retCode, 0x00, LEN_RES_CODE+2 );
	errCount = 0;

	
	otpData.otherOrg = atoi(args[1]);    				/* 타행 1, 자행 0 */
	strcpy( otpData.venderCode, "002" );  			/* vender code : HOST 조회 시 가져옴 */
	strcpy( otpData.userCode, "u12345-123456" );  		/* 실명번호(텔레뱅킹), 이용자번호(인터넷뱅킹) */
	strcpy( otpData.tokSerial, "t00012345678" );		/* HOST 조회 시 가져옴 */
	strcpy( otpData.tokCode, "c12345" );				/* OTP 응답값 -> 이용자 입력 */


	if( strncmp(args[1], "0", 1 ) == 0 ) {
		otpData.otherOrg = 0;
		Af_AuthOtpUser(&otpData, retCode, &errCount);
	}
	else if( strncmp(args[1], "1", 1 ) == 0 ) {
		otpData.otherOrg = 1;
		Af_AuthOtpUser(&otpData, retCode, &errCount);
	}
	else if( strncmp(args[1], "2", 1 ) == 0 ) {
		otpData.otherOrg = 0;
		Af_ResyncOtpUser(&otpData, retCode);
	}
	else if( strncmp(args[1], "3", 1 ) == 0 ) {
		otpData.otherOrg = 1;
		Af_ResyncOtpUser(&otpData, retCode);
	}
		
	

	printf( "\n retCode : %s, errCount : %d\n\n", retCode, errCount );



}
/* end of main ******************************************************************/

