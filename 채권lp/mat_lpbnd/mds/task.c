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

#include "mds.h"

#include "main.h"
#include "task.h"

/********** USER FUNCTION **********/
int	CmdTest( int argc, char *argv[]);
int	CmdSet( int argc, char *argv[]);
int	CmdList( int argc, char *argv[]);
int	CmdView( int argc, char *argv[]);
int	CmdFold( int argc, char *argv[]);
int	CmdMon( int argc, char *argv[]);
/***********************************/
int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

enum MarketNum { BEST=0, REUTER, KMB, SMB, EBS };
enum TenorNum	{ SPT=0, TOD, TOM, W01, M01, M02, M03, M06, Y01 };
char TnrStr[10][4] = { "SPT", "TOD", "TOM", "W01", "M01", "M02", "M03", "M06", "Y01", "\0"};

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"test",			CmdTest,		"none",					"command test"},
	{	1,		"set",			CmdSet,			"[name]",				"공유메모리 set B/R/K/S/E" },
	{	1,		"list",			CmdList,		"",						"list "},
	{	1,		"view",			CmdView,		"",						"list "},
	{	1,		"fold",			CmdFold,		"none",					"command test"},
	{	1,		"mon",			CmdMon,			"none",					"command test"},
	{	100,	"help",			CmdHelp,		"none",					"help message"},
	{	101,	"quit",			CmdQuit,		"none",					"stop process"},
	{	102,	"exit",			CmdQuit,		"none",					"stop process"},
	{	-1,		"\0",			NULL,			"\0",					"\0"}
};

enum MarketNum		Market = BEST;		/* 1=BEST 2=REUTER 3=KMB 4=SMB 5=EBS */
char				Symbol[ 32] = "USD/KRW";
enum TenorNum		Tenor = SPT;		/* "SPT", "TOD", "TOM", "W01", "M01", "M02", "M03", "M06", "Y01", "\0" */


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
	int		i, cols = 100;
	int		mdpos = 0;
	MDARCH	*arch;
	MDFOLD	*fold, *f_ptr;

	arch = MdsMem[ MdsMemCurr].base;
	fold =  MdsMem[ MdsMemCurr].base + sizeof( MDARCH);

	MDARCH_Print( arch);


	for( i = 0; i < cols; i++)	LogRaw( "-");
	LogRaw( "\n");
	LogRaw( "no symb   trd_day    \n");
	for( i = 0; i < cols; i++)	LogRaw( "-");
	LogRaw( "\n");

	while( fold[ mdpos].symb[ 0] != 0)
	{
		f_ptr = &fold[ mdpos];
		LogRaw( "%2d ", mdpos);
		LogRaw( "%-7s ", f_ptr->symb);
		LogRaw( "%08d", f_ptr->tymd);
		LogRaw( "\n");

		mdpos++;
	}

	MDFOLD_Print( fold);

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
int CmdSet( int argc, char *argv[])
{
	switch( argc)
	{
		default:
		case 3:
			switch( toupper( argv[ 1][ 0]))
			{
				case 'M':
					switch( toupper( argv[ 2][ 0]))
					{
						case 'B': Market = BEST;	break;
						case 'R': Market = REUTER;	break;
						case 'K': Market = KMB;		break;
						case 'S': Market = SMB;		break;
						case 'E': Market = EBS;		break;
						default:					break;
					}
					break;
				case 'S':
					memcpy( Symbol, argv[ 2], strlen( argv[ 2]) +1);
					break;
				case 'T':
					if( !memcmp( argv[ 2], "SPT", 3))		Tenor = SPT;
					else if( !memcmp( argv[ 2], "TOD", 3))	Tenor = TOD;
					else if( !memcmp( argv[ 2], "TOM", 3))	Tenor = TOM;
					else if( !memcmp( argv[ 2], "W01", 3))	Tenor = W01;
					else if( !memcmp( argv[ 2], "M01", 3))	Tenor = M01;
					else if( !memcmp( argv[ 2], "M02", 3))	Tenor = M02;
					else if( !memcmp( argv[ 2], "M03", 3))	Tenor = M03;
					else if( !memcmp( argv[ 2], "M06", 3))	Tenor = M06;
					else if( !memcmp( argv[ 2], "Y01", 3))	Tenor = Y01;
					break;
			}
			break;
		case 2:
		case 0:
		case 1:
			break;
	}
	LogRaw( "    Market = [%-10s]    B/R/K/S/E\n", MdsMem[ Market].full_name);
	LogRaw( "    Symbol = [%-10s]    (ex:USD/KRW)\n", Symbol);
	LogRaw( "    Tenor  = [%-10s]    SPT/TOD/TOM/W01/M01/M02/M03/M06/Y01\n", TnrStr[ Tenor]);

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
	int		cols = 100;
	MDARCH	*arch[ 10];

	for( i = 0; i < cols; i++)	LogRaw( "-");
	LogRaw( "\n");
	LogRaw( "name     id c ip               port  sum\n");
	for( i = 0; i < cols; i++)	LogRaw( "-");
	LogRaw( "\n");

	for( i = 0; MdsMem[ i].no > 0; i++)
	{
		arch[ i] = MdsMem[ i].base;
		LogRaw( "%-8s ", MdsMem[ i].full_name);
		LogRaw( "%2d ", arch[ i]->xchg.exid);
		LogRaw( "%1s ", arch[ i]->xchg.excode);
		LogRaw( "%-16s ", arch[ i]->xchg.apsnd.ipad);
		LogRaw( "%5d ", arch[ i]->xchg.apsnd.port);
		LogRaw( "%8d ", arch[ i]->rsum);

		LogRaw( "\n");
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
int CmdView( int argc, char *argv[])
{
	int		i, cols=100;
	MDARCH	*arch;
	MDFOLD	*fold, *f_ptr;

	LogRaw( "##### MDARCH [%s] #####\n", MdsMem[ Market].full_name);
	arch = MdsMem[ Market].base;
	MDARCH_Print( arch);

	LogRaw( "##### MDFOLD [%s] #####\n", MdsMem[ Market].full_name);
	fold =  MdsMem[ Market].base + sizeof( MDARCH);
	PrintFoldList( fold);
	LogRaw( "##### MDFOLD List [%s] [%s] #####\n", MdsMem[ Market].full_name, Symbol);

	f_ptr = fold;
	while( f_ptr->symb[ 0] != 0)
	{
		if( !memcmp( f_ptr->symb, Symbol, strlen( Symbol)))
		{
			MDFOLD_Print( f_ptr);
			break;
		}
		f_ptr++;
	}

	LogRaw( "##### MDFOLD mdquot [%s] [%s] [%s] #####\n", MdsMem[ Market].full_name, Symbol, TnrStr[ Tenor]);
	mdquot_t_Print( &f_ptr->mdquot[ Tenor]);

	for( i = 0; i < cols; i++)	LogRaw( "-"); LogRaw( "\n");
	LogRaw( "sid o open       high       lowp       last       last vol base       best       best vol s   \n");
	for( i = 0; i < cols; i++)	LogRaw( "-"); LogRaw( "\n");
	SidePrint( &f_ptr->mdquot[ Tenor].bid, "bid");
	SidePrint( &f_ptr->mdquot[ Tenor].ask, "ask");
	SidePrint( &f_ptr->mdquot[ Tenor].mid, "mid");
	for( i = 0; i < cols; i++)	LogRaw( "-"); LogRaw( "\n");
	mdside_t_Print( &f_ptr->mdquot[ Tenor].bid);
	/*
	MDFOLD_Print( fold);
	*/

	return 1;
}

int SidePrint( mdside_t *side, char *title)
{
	LogRaw( "%s ", title);
	LogRaw( "%1s ", side->excode);
	LogRaw( "%10.5f ", side->open);
	LogRaw( "%10.5f ", side->high);
	LogRaw( "%10.5f ", side->lowp);
	LogRaw( "%10.5f ", side->last);
	LogRaw( "%8.0f ", side->lastvol);
	LogRaw( "%10.5f ", side->base);
	LogRaw( "%10.5f ", side->best);
	LogRaw( "%8.0f ", side->bestvol);
	LogRaw( "%1d ", side->sign);

	LogRaw( "\n");
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
int CmdFold( int argc, char *argv[])
{
	int		mem_curr = MdsMemCurr;
	int		symb = 0;
	MDARCH	*arch;
	MDFOLD	*fold, *f_ptr;

	switch( argc)
	{
		case 3:
			symb = atoi( argv[ 2]);
		case 2:
			switch( argv[ 1][ 0])
			{
				case 'B':	mem_curr = 0; 			break;
				case 'R':	mem_curr = 1; 			break;
				case 'K':	mem_curr = 2; 			break;
				case 'S':	mem_curr = 3; 			break;
				case 'E':	mem_curr = 4; 			break;
				default:							break;
			}
			fold =  MdsMem[ mem_curr].base + sizeof( MDARCH);
			LogRaw( "name=[%s]\n", fold);
		case 1:
		case 0:
			break;
	}

	arch = MdsMem[ mem_curr].base;
	MDARCH_Print( arch);

	fold =  MdsMem[ mem_curr].base + sizeof( MDARCH);
	PrintFoldList( fold);


	/*
	MDFOLD_Print( fold);
	*/

	return 1;
}

int PrintFold( MDFOLD *fold, int symb)
{
	int		i, cols = 100;
	int		mdpos = 0;
	MDFOLD	*f_ptr;

	for( i = 0; i < cols; i++)	LogRaw( "-");
	LogRaw( "\n");
	LogRaw( "no symb   trd_day    \n");
	for( i = 0; i < cols; i++)	LogRaw( "-");
	LogRaw( "\n");

	while( fold[ mdpos].symb[ 0] != 0)
	{
		f_ptr = &fold[ mdpos];
		LogRaw( "%2d ", mdpos +1);
		LogRaw( "%-7s ", f_ptr->symb);
		LogRaw( "%08d", f_ptr->tymd);
		LogRaw( "\n");

		mdpos++;
	}

	if( symb > 0)
	{
		MDFOLD_Print( &fold[ symb -1]);
		PrintSwap( &fold[ symb -1]);
	}

}

int PrintFoldList( MDFOLD *fold)
{
	int		i, cols = 100;
	int		mdpos = 0;
	MDFOLD	*f_ptr;

	for( i = 0; i < cols; i++)	LogRaw( "-"); LogRaw( "\n");
	LogRaw( "no symb   trd_day   update      seq\n");
	for( i = 0; i < cols; i++)	LogRaw( "-"); LogRaw( "\n");

	while( fold[ mdpos].symb[ 0] != 0)
	{
		f_ptr = &fold[ mdpos];
		LogRaw( "%2d ", mdpos +1);
		LogRaw( "%-7s ", f_ptr->symb);
		LogRaw( "%08d ", f_ptr->tymd);
		LogRaw( "%06d.%03d ", f_ptr->khms / 1000, f_ptr->khms % 1000);
		LogRaw( "%7d ", f_ptr->mdquot[ 0].tick_seqn);
		LogRaw( "\n");

		mdpos++;
	}

	for( i = 0; i < cols; i++)	LogRaw( "-"); LogRaw( "\n");

	return 1;
}

int PrintSwap( MDFOLD *mdfold)
{
	int			i, cols = 100;
	char		str[10][4] = { "SPT", "TOD", "TOM", "W01", "M01", "M02", "M03", "M06", "Y01", "\0"};
	mdquot_t	*quot;

	for( i = 0; i < cols; i++)	LogRaw( "-");
	LogRaw( "\n");
	LogRaw( "tnr swap_bid     swap_ask   swap_bid_man swap_ask_man \n");
	for( i = 0; i < cols; i++)	LogRaw( "-");
	LogRaw( "\n");

	for( i = 0; str[ i][0] != 0; i++)
	{
		quot = &mdfold->mdquot;
		LogRaw( "%s ", str[ i]);
		LogRaw( "%10.5f ", quot->swap_bid);
		LogRaw( "%10.5f ", quot->swap_ask);
		LogRaw( "%10.5f ", quot->swap_bid_man);
		LogRaw( "%10.5f ", quot->swap_ask_man);
		LogRaw( "\n");
	}
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
int CmdMon( int argc, char *argv[])
{
	int		rtn;

	MapInit();

	rtn = Mon_Main();

	MapEnd();
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

