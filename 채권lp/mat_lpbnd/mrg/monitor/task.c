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
#include <fcntl.h>

#include "cmd.h"
#include "map.h"

#include "main.h"


/********** USER FUNCTION **********/
int	CmdTest( int argc, char *argv[]);
int	CmdList( int argc, char *argv[]);
int	CmdStat( int argc, char *argv[]);
int	CmdMon( int argc, char *argv[]);
/***********************************/
char*	GetInput( char *msg);
/***********************************/
int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"test",			CmdTest,		"none",					"command test"},
	{	1,		"list",			CmdList,		"none",					"전행고객실명대체번호 목록"},
	{	1,		"stat",			CmdStat,		"전행고객실명대체번호", "stat"},
	{	1,		"mon",			CmdMon,			"none",					"command test"},
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
	char	prompt[ 32];

	Cmd = Cmd_Open( CmdTable);
	if( Cmd == NULL)
	{
		LogCri( "Cmd_Open error. cmd_table=[%p]", CmdTable);
		return -1;
	}

	sprintf( prompt, "[%s:0x%08x]", "test", 0);
	Cmd_SetPrompt( Cmd, prompt);

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
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	LogDbg( "test.............");

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdList( int argc, char *argv[])
{
	int		i;
	CUSTMRGN_ST *cust_mrgn;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	printf( " no id        group_id  cnt\n");
    for( i = 0; i < MAX_CUST_CNT; i++)
    {
        cust_mrgn = &CustMrgn->cinfo[ i];

		if( strlen( cust_mrgn->s_csac_idnt_no) <= 0) break;

		printf( "%5d ",      i);
		printf( "%-12.12s ", cust_mrgn->s_csac_idnt_no);
		printf( "%-12.12s ", cust_mrgn->s_cust_grp_id);
		printf( "%.2s ",     cust_mrgn->s_emp_grp_yn);
		printf( "%3d ",      cust_mrgn->n_fnl_cnt);
		printf( "\n");

	}


	return 1;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdStat( int argc, char *argv[])
{
	int		i;
	int		col = 120;

	CUSTMRGN_ST *cust_mrgn;
    PAIRMRGN_ST *fnl_pair;
    PAIRMRGN_ST *std_pair;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

    for( i = 0; i < MAX_CUST_CNT; i++)
    {
        cust_mrgn = &CustMrgn->cinfo[ i];

		if( argc < 2) break;
		if( !memcmp( argv[ 1], cust_mrgn->s_csac_idnt_no, strlen( argv[ 1]))) break;
		if( strlen( cust_mrgn->s_csac_idnt_no) <= 0) return 0;
	}

	/*
	CUSTMRGN_ST_Print( cust_mrgn);
	*/

	for( i = 0; i < col; i++)	printf( "-"); printf( "\n");
	printf( "전행고객실명대체번호=[%s]\n", cust_mrgn->s_csac_idnt_no);
	printf( "본점마진구룹ID      =[%s]\n", cust_mrgn->s_cust_grp_id);
//	printf( "직원그룹여부 Y,N    =[%s]\n", cust_mrgn->s_emp_grp_yn);
	printf( "재정Pair 건수       =[%d]\n", cust_mrgn->n_fnl_cnt);
	printf( "비재정Pair 건수     =[%d]\n", cust_mrgn->n_fnl_cnt);

	for( i = 0; i < col; i++)	printf( "-"); printf( "\n");
	printf( "%-7s ", "통화");
//	printf( "%s ", "g");
//	printf( "%s ", "d");
//	printf( "%s ", "digit ");
//	printf( "%s ", "unit  ");
	printf( "%s ", "m");
	printf( "%s ", "            ");
	printf( "%s ", "            ");
	printf( "%s ", "            ");
	printf( "%s ", "m");
	printf( "%s ", "            ");
	printf( "%s ", "            ");
	printf( "\n");
	for( i = 0; i < col; i++)	printf( "-"); printf( "\n");

	for( i = 0; i < cust_mrgn->n_fnl_cnt; i++)
    {
        fnl_pair = &cust_mrgn->fnlmgst[ i];

        printf( "%.7s ",	fnl_pair->s_pair_id);
//        printf( "%.1s ",	fnl_pair->s_clc_dsnc);
//        printf( "%d ",		fnl_pair->n_digit);
//        printf( "%6.2f ",	fnl_pair->d_digit_val);
//        printf( "%6.2f ",	fnl_pair->d_clc_unit);
        printf( "%1s ",		fnl_pair->s_spt_bomg_dcd);
        printf( "%12f ",	fnl_pair->d_spt_bymg);
        printf( "%12f ",	fnl_pair->d_spt_slmg);
		printf( "%s ", "            ");
        printf( "%1s ",		fnl_pair->s_fwd_bomg_dcd);
        printf( "%12f ",	fnl_pair->d_fwd_bymg);
        printf( "%12f ",	fnl_pair->d_fwd_slmg);
		printf( "\n");
    }


	for( i = 0; i < col; i++)	printf( "-"); printf( "\n");
	printf( "%-7s ", "통화");
//	printf( "%s ", "g");
//	printf( "%s ", "d");
//	printf( "%s ", "digit ");
//	printf( "%s ", "unit  ");
	printf( "%s ", "m");
	printf( "%s ", "            ");
	printf( "%s ", "            ");
	printf( "%s ", "            ");
	printf( "%s ", "m");
	printf( "%s ", "            ");
	printf( "%s ", "            ");
	printf( "\n");
	for( i = 0; i < col; i++)	printf( "-"); printf( "\n");

	for( i = 0; i < cust_mrgn->n_fnl_cnt; i++)
    {
        std_pair = &cust_mrgn->stdmgst[ i];

        printf( "%.7s ",	std_pair->s_pair_id);
//        printf( "%.1s ",	std_pair->s_clc_dsnc);
//        printf( "%d ",		std_pair->n_digit);
//        printf( "%6.2f ",	std_pair->d_digit_val);
//        printf( "%6.2f ",	std_pair->d_clc_unit);
        printf( "%1s ",		std_pair->s_spt_bomg_dcd);
        printf( "%12f ",	std_pair->d_spt_bymg);
        printf( "%12f ",	std_pair->d_spt_slmg);
		printf( "%s ", "            ");
        printf( "%1s ",		std_pair->s_fwd_bomg_dcd);
        printf( "%12f ",	std_pair->d_fwd_bymg);
        printf( "%12f ",	std_pair->d_fwd_slmg);
		printf( "\n");
    }

	for( i = 0; i < col; i++)	printf( "-"); printf( "\n");


	return 1;
}

/** ***************************************************************************
**  @fu         int CmdInsert( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdMon( int argc, char *argv[])
{
	int			i;
	int			rtn;
	CUSTMRGN_ST *cust_mrgn;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

    for( i = 0; i < MAX_CUST_CNT; i++)
    {
        cust_mrgn = &CustMrgn->cinfo[ i];

		if( argc < 2) break;
		if( !memcmp( argv[ 1], cust_mrgn->s_csac_idnt_no, strlen( argv[ 1]))) break;
		if( strlen( cust_mrgn->s_csac_idnt_no) <= 0) return 0;
	}

	MapInit();
	rtn = Mon_Main( cust_mrgn);
	MapEnd();

	return rtn;
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

