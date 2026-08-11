/** ***************************************************************************
**  @file       task.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  명령어 수행 프로그램
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "otp.h"
#include "main.h"
#include "task.h"

/********** USER FUNCTION **********/
int	CmdTest( int argc, char *argv[]);
/***********************************/
int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"test",			CmdTest,		"none",					"command test"},
	{	100,	"help",			CmdHelp,		"none",					"help message"},
	{	101,	"quit",			CmdQuit,		"none",					"stop process"},
	{	102,	"exit",			CmdQuit,		"none",					"stop process"},
	{	-1,		"\0",			NULL,			"\0",					"\0"}
};

/********** USER DEFINE VALIABLE **********/
/******************************************/

/** ***************************************************************************
**  @function   CMD 사용을 위한 초기화  ... 변경 불필요
**  @brief      HELP/QUIT
***************************************************************************** */
int TaskInit()
{
	Cmd = Cmd_Open( CmdTable);
	if( Cmd == NULL)
	{
		LogCri( "Cmd_Open error. cmd_table=[%p]", CmdTable);
		return -1;
	}

	return 1;
}

/** ***************************************************************************
**  @function   기능별 함수 작성
**  @brief      TODO
**  1. 함수 프로토타입 정의
**  2. CmdTable에 등록
**  3. 함수 작성
***************************************************************************** */
/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdTest( int argc, char *argv[])
{
	int			nRet_len;
	stOtpData	otpData, out;
	char		retCode[LEN_RES_CODE+2];
	int			errCount;

	LogDbg( "test.............");

	if( argc < 2 )  
	{
		printf("\n usage : " );
		printf("\n testagent <argv 1>");
		printf("\n argv 1 -> <OTP응답값>  \n\n");
		return 0;
	}

	memset( &otpData, 0x00, sizeof(stOtpData) );
	memset( retCode, 0x00, LEN_RES_CODE+2 );
	errCount = 0;

	setenv( "OTP_INI_PATH", "./sdotpagent.ini", 1);


#if 0
	strcpy( otpData.otherOrg  , "1"             );			/* 타행 2, 자행 1 */
	strcpy( otpData.venderCode, "004"           );			/* vender code : HOST 조회 시 가져옴 */
	strcpy( otpData.userCode  , "7603011007718" );			/* 실명번호(텔레뱅킹), 이용자번호(인터넷뱅킹) */
	strcpy( otpData.tokSerial , "TEST000428"    );			/* HOST 조회 시 가져옴 */
	strcpy( otpData.tokCode   , argv[1]         );			/* OTP 응답값 -> 이용자 입력 */
#else
	strcpy( otpData.otherOrg  , "1"             );			/* 타행 2, 자행 1 */
	strcpy( otpData.venderCode, "004"           );			/* vender code : HOST 조회 시 가져옴 */
	strcpy( otpData.userCode  , "7603011007718" );			/* 실명번호(텔레뱅킹), 이용자번호(인터넷뱅킹) */
	strcpy( otpData.tokSerial , "TEST000428"    );			/* HOST 조회 시 가져옴 */
	strcpy( otpData.tokCode   , argv[1]         );			/* OTP 응답값 -> 이용자 입력 */
#endif

	Af_AuthOtpUser(&otpData, &out);

	nRet_len = strlen (out.resCode);
	if( nRet_len == 0)
	{
		printf ("CHECK OTP ---- Not Found Configure file !!!! (/*/*/sdotpagent.ini)\n\n");
	}
	else 
	if (strcmp (out.resCode, "000000") == 0)
	{
		printf ("CHECK OTP ---- OK !!!!\n\n");
	}
	else
	{
		printf ("CHECK OTP ---- ERROR !!!!\n\n");
	}

	printf (" resCode      [%s]\n", out.resCode      );
	printf (" otherOrg     [%s]\n", out.otherOrg     );
	printf (" venderCode   [%s]\n", out.venderCode   );
	printf (" userCode     [%s]\n", out.userCode     );
	printf (" tokSerial    [%s]\n", out.tokSerial    );
	printf (" tokCode      [%s]\n", out.tokCode      );
	printf (" lastAuthDate [%s]\n", out.lastAuthDate );
	printf (" lastAuthTime [%s]\n", out.lastAuthTime );
	printf (" errCount     [%s]\n", out.errCount     );

	return 1;
}

/** ***************************************************************************
**  @function   정의된 함수 ... 변경 불필요
**  @brief      HELP/QUIT
***************************************************************************** */
int CmdHelp( int argc, char *argv[])
{
	int		rtn;

	if( argc >= 2)
	{
		rtn = Cmd_HelpCommand( Cmd, argv[ 1]);
		return rtn;
	}

	rtn = Cmd_IntHelp( Cmd, argc, argv);
	return rtn;
}

int CmdQuit( int argc, char *argv[])
{
	LogDbg( "quit.............");

	exit( 1);
	return 1;
}

