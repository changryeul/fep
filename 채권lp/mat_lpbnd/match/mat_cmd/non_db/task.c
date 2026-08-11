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

#include "map.h"
#include "mat.h"
#include "order.h"
#include "sise.h"

#include "main.h"
#include "task.h"
#include "order.h"

/********** USER FUNCTION **********/
int	CmdCreate( int argc, char *argv[]);
int	CmdRemove( int argc, char *argv[]);
int	CmdOpen( int argc, char *argv[]);
int	CmdClose( int argc, char *argv[]);
int	CmdStat( int argc, char *argv[]);
int	CmdRecord( int argc, char *argv[]);
int	CmdPipe( int argc, char *argv[]);
int	CmdLock( int argc, char *argv[]);
int	CmdMon( int argc, char *argv[]);
int	CmdReset( int argc, char *argv[]);
int	CmdMsg( int argc, char *argv[]);
int	CmdCurrent( int argc, char *argv[]);
int	CmdList( int argc, char *argv[]);
int	CmdGroup( int argc, char *argv[]);
int	CmdSize( int argc, char *argv[]);
int	CmdService( int argc, char *argv[]);
int	CmdPair( int argc, char *argv[]);
int	CmdPairAdd( int argc, char *argv[]);
int	CmdPriceTime( int argc, char *argv[]);
int	CmdIndex( int argc, char *argv[]);
int	CmdJang( int argc, char *argv[]);
int	CmdJangPair( int argc, char *argv[]);
int	CmdColor( int argc, char *argv[]);
int	CmdTest( int argc, char *argv[]);
/***********************************/
char*	GetInput( char *msg);
/***********************************/
int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	10,		"create",		CmdCreate,		"none",					"ipc 생성"},
	{	11,		"remove",		CmdRemove,		"none",					"ipc 삭제"},
	{	12,		"open",			CmdOpen,		"none",					"매칭엔진 open"},
	{	13,		"close",		CmdClose,		"none",					"매칭엔진 close"},
	{	14,		"stat",			CmdStat,		"none",					"매칭엔진 공유메모리 정보 조회\n"},
	{	15,		"record",		CmdRecord,		"[position]",			"주문 상태 조회"},
	{	15,		"pipe",			CmdPipe,		"[pos [pos [...]]]",	"체결 pipe send: 체결 정보가 mat_mat에서 mat_exe로 "
												"전달이 안되었을때 체결 position을 강제로 송신\n"
												"- mon 명령에서 \"체결 수량\"이 0이 아닐때 전송"},
	{	17,		"lock",			CmdLock,		"none",					"Lock - All stop!!! \n"
												"- 엔진에 lock을 걸어 일시 정지 시킴"},
	{	17,		"mon",			CmdMon,			"[option=(s|i|q|t|o [pos])]",	"모니터링\n"
																		"\'g\'aro    - 가로모드\n" 
																		"\'s\'mall   - index + smq + statis\n" 
																		"\'i\'ndex   - 매칭 테이블\n" 
																		"sm\'q\'     - 메모리 큐\n" 
																		"s\'t\'atis  - 통계\n" 
																		"\'o\'rder   - 주문/체결\n" 
																		},
	{	17,		"reset",		CmdReset,		"none",					"통계 초기화"},
	{	18,		"msg",			CmdMsg,			"[code|msg]",			"에러메세지 조회"},
	{	19,		"current",		CmdCurrent,		"none",					"통화 정보 조회"},
	{	20,		"list",			CmdList,		"[position [cnt]]",		"주문/체결 내역 조회"},
	{	21,		"group",		CmdGroup,		"none",					"그룹 리스트 조회"},
	{	22,		"size",			CmdSize,		"none",					"매칭엔진에서 사용하는 구조체 크기"},
	{	23,		"service",		CmdService,		"[0|1]",				"서비스 상태 조회 및 변경\n"
																		"0 - off : Mat_Open시 1일때까지 대기\n"
																		"1 - on  : 모든 프로세스가 정상 Mat_Open 수행"
																		},
	{	24,		"pair",			CmdPair,		"[load]",				"DB 통화쌍 정보를 조회 or 매칭시스템에 적용\n"
																		"load - DB COD_FXPAIR(통화거래정보)를 공유메모리에 load"},
	{	24,		"pair_add",		CmdPairAdd,		"<pair> <point> <unit>","DB 통화쌍 정보를 추가 \n"
																		"pair=\"USD/KRW\" point=2 unit=1\n"
																		"pair=\"JPY/KRW\" point=2 unit=100\n"
																		"pair=\"EUR/USD\" point=5 unit=1"},
	{	25,		"pricetime",	CmdPriceTime,	"[load]",				"DB PriceTime(시세 유효시간)정보를 조회 or 매징엔진에 적용\n"																		 "load - DB TB_FXB_CMC003D(PriceTime)정보를 load"},
	{	26,		"index",		CmdIndex,		"[position]",			"Index 정보 조회, pos=상세 조회"},
	{	27,		"jang",			CmdJang,		"[[s|e] HHMMSS] [l]",	"장운영시간 조회/변경/LOAD\n"
																		"HHMMSS: 당일 23시00분00초=[230000]\n"
																		"        익일 02시00분00초=[260000]\n"
																		"s=start(장시작),e=end(장마감),l=load(db load)"},
	{	28,		"jp",			CmdJangPair,	"[l]",					"통화별 장운영 조회 / 적용"},
	{	29,		"color",		CmdColor,		"none",					"color set for mon display"},
	{	99,		"test",			CmdTest,		"none",					"module test"},
	{	100,	"help",			CmdHelp,		"none",					"help message"},
	{	101,	"quit",			CmdQuit,		"none",					"stop process"},
	{	102,	"exit",			CmdQuit,		"none",					"stop process"},
	{	-1,		"\0",			NULL,			"\0",					"\0"}
};

/********** USER DEFINE VALIABLE **********/
extern MAT		*Mat;
extern SMQ		*Smq;
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

	sprintf( prompt, "[%s:%d]", "mat", Mat->mem->id);
	Cmd_SetLog( Cmd, Param->log_name);
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
**  @fu         int CmdCreate( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdCreate( int argc, char *argv[])
{
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat != NULL)
	{
		LogMsg( "Mat != NULL ... 이미 열려 있습니다.");
		return 0;
	}

	Mat = Mat_Create();
	if( Mat == NULL)
	{
		LogMsg( "매칭엔진 생성 오류");
		return 0;
	}

	LogMsg( "매칭엔진 생성... Mat=[%p]", Mat);

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdCreate( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdRemove( int argc, char *argv[])
{
	int		rtn;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ... 삭제할 수 없습니다.");
		return 0;
	}

	rtn = Conform( "매징엔진 자원들을 삭제 하려고 합니다. 계속 진행 하시겠습니까?");
	if( rtn == 0) 
	{
		printf( "취소되었습니다.\n");
		return 0;
	}

	rtn = Mat_Remove( Mat);
	if( rtn < 0)
	{
		LogMsg( "매칭엔진 삭제 오류");
		return 0;
	}
	Mat = NULL;

	LogMsg( "매칭엔진 삭제...");

	return CMD_EXIT;
}

/** ***************************************************************************
**  @fu         int CmdCreate( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdOpen( int argc, char *argv[])
{
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat != NULL)
	{
		LogMsg( "Mat != NULL ... 이미 열려 있습니다.");
		return 0;
	}

	Mat = Mat_Open( 1);
	if( Mat == NULL)
	{
		LogMsg( "매칭엔진 Open 오류");
		return 0;
	}

	LogMsg( "매칭엔진 Open ... Mat=[%p]", Mat);

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdCreate( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdClose( int argc, char *argv[])
{
	int		rtn;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	rtn = Mat_Close( Mat);
	if( rtn < 0)
	{
		LogMsg( "매칭엔진 닫기 오류");
		return 0;
	}
	Mat = NULL;

	LogMsg( "매칭엔진 close...");

	return CMD_EXIT;
}

/** ***************************************************************************
**  @fu         int CmdStat( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdStat( int argc, char *argv[])
{
	int		rtn;
	int		filter = 1;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	rtn = Mat_Stat( Mat);
	if( rtn < 0)
	{
		LogMsg( "Mat_Stat error.");
		return 0;
	}

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
int CmdLock( int argc, char *argv[])
{
	int		rtn;
	char	*ptr;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	rtn = Conform( "Lock을 걸면 매칭엔진이 멈추게 됩니다. 계속 진행 하시겠습니까?");
	if( rtn == 0) 
	{
		printf( "취소되었습니다.\n");
		return 0;
	}

	Mat_Lock( Mat);

	while( 1)
	{
		printf( "    1. lock 계속\n");
		printf( "    2. unlock 후 다시 lock\n");
		printf( "    3. unlock 후 종료\n");

		ptr = GetInput( "input number (1/2/3) "); 
		switch( ptr[ 0])
		{
			default:
			case '1':
				continue;
			case '2':
				Mat_Unlock( Mat);
				Mat_Lock( Mat);
				continue;
			case '3':
				Mat_Unlock( Mat);
				return 1;
		}
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdStat( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdRecord( int argc, char *argv[])
{
	int				pos;

	if( argc < 2)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	pos = atoi( argv[ 1]);

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	Mat_RecordToFile( Mat, pos, stdout);

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdStat( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdPipe( int argc, char *argv[])
{
	int				rtn;
	int				pos = 1, cnt = 1;

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	if( argc < 2)
	{
		Mat_StatExecute( Mat);
		return 0;
	}

	while( cnt < argc)
	{
		pos = atoi( argv[ cnt]);
		if( pos < 0 && pos >= MAT_MAX_RECORD) return 0;
		
		rtn = write( Mat->exe_fd, ( char *)&pos, sizeof( int));
		if( rtn < sizeof( int))
		{
			return 0;
		}
		cnt++;
	}

	sleep( 1);
	Mat_StatExecute( Mat);

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
char *GetInput( char *msg)
{
	static char rec[ 512];

	memset( rec, 0, 512);
	printf( "%s:", msg);
	fgets( rec, 512, stdin);

	return rec;
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
	int			rtn;
	int			pos;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	MapInit();

	if( argc < 2)
	{
		rtn = Mon_Main( Mat, Smq, NULL);
	}
	else
	{
		switch( argv[ 1][ 0])
		{
			case 's':		/* small */
				rtn = Mon_SmallMain( Mat, Smq);
				break;
			case 'i':		/* index */
				rtn = Mon_IndexMain( Mat, Smq);
				break;
			case 'q':		/* smq */
				rtn = Mon_SmqMain( Mat, Smq);
				break;
			case 't':		/* statis */
				rtn = Mon_StatisMain( Mat, Smq);
				break;
			case 'o':		/* order */
				if( argc >= 3)	pos = atoi( argv[ 2]);
				else			pos = 0;
				rtn = Mon_Order( Mat, pos);
				break;
			case 'g':		/* garo */
				rtn = Mon_Main( Mat, Smq, "garo.map");
				break;
			default:
				rtn = Mon_Main( Mat, Smq, NULL);
				break;
		}
	}
	MapEnd();

	return rtn;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통계 초기화
***************************************************************************** */
int CmdReset( int argc, char *argv[])
{
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	Mat_StatisReset( Mat);
	Smq_StatisReset( Smq);

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  메세지 조회
***************************************************************************** */
int CmdMsg( int argc, char *argv[])
{
	int			i, code;
	char		*ptr;
	MAT_MESSAGE	*msg;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	if( argc < 2)
	{
		Mat_MessageList( Mat);
		return 1;
	}

	code = atoi( argv[ 1]);
	for( i = 0; i < Mat->map->msg_cnt; i++)
	{
		msg = &Mat->map->msg[ i];
		if( msg->code <= 0) continue;
		if( code <= 0)
		{
			ptr = strstr( msg->msg, argv[ 1]);
			if( ptr != NULL) printf( "[%5d][%s]\n", msg->code, msg->msg);
		}
		else
		{
			if( code == msg->code) printf( "[%5d][%s]\n", msg->code, msg->msg);
		}
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통화 조회
***************************************************************************** */
int CmdCurrent( int argc, char *argv[])
{
	int			i;
	MAT_CURRENT	*current;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	for( i = 0; i < 32; i++)
	{
		current = &Mat->map->current[ i];
		if( strlen( current->str) <= 0) break;
		printf( "[%2d][%s][%s]\n",
			current->num,
			current->str,
			current->comment
		);
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통화 조회
***************************************************************************** */
int CmdList( int argc, char *argv[])
{
	int				i, j, col = 120;
	int				cnt = 0;
	int				start = 1;
	int				view_cnt = 100000;
	MAT_RECORD		*rec;
	MAT_HEAD		*head;
	ORDER			*obook;
	/* SPLIT_OUT_ST	*pee; */
#if 0
	char			*gubun[] = { "완료", "주문", "체결", "    ", "" };
	char			*ord_stat[] = { "    ", "주문", "체결", "취소", "강취", "정정", "그룹", "    ", "" };
	char			*side[] = { "매수", "매도", "" };
	char			*type[] = { "시장", "지정", "예약", "" };
	char			*prty[] = { "일반", "고정", "대행", "" };
	char			*orig[] = { "고객", "내부", "대행", "" };
	char			*tran[] = { "일반", "MAR ", "RFQ ", "RFS", "예약", "기간", "일괄", "", "", "햇지", "" };
#endif

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	if( argc > 1)
	{
		start = atoi( argv[ 1]);
		if( argc > 2)
		{
			view_cnt = atoi( argv[ 2]);
		}
	}

	/*
	printf( "g = 0:empty, 1:주문, 2:체결 \n");
	printf( "S = 1:매수, 2:매도 \n");
	printf( "O = 1:시장가, 2:지정가, 3:예약주문 \n");
	*/

	for( i = 0; i < col; i++) printf( "-"); 
	printf( "\n");
	printf( " pos  ");
	printf( "구분 ");
	printf( "상태 ");
	printf( "recv_time           ");
	printf( "ClOrdID     ");
	printf( "Symbol  ");
	printf( "매매 ");
	printf( "가격 ");
	printf( " 주문가격  ");
	printf( " 체결가격  ");
	printf( " ");
	printf( "상품 ");
	printf( "원천 ");
	printf( "타입 ");
	printf( "유형 ");
	printf( "\n");
	for( i = 0; i < col; i++) printf( "-"); 
	printf( "\n");

	for( i = start; i < MAT_MAX_RECORD; i++)
	{
		rec    = &Mat->map->rec[ i];
		head   = &rec->head;
		obook  = ( ORDER *)&rec->ord;
		/* pee    = ( SPLIT_OUT_ST *)&head->pee_out; */
		if( head->ord_stat == 0) continue;
		/* if( head->rcv_time.tv_sec == 0) continue; */

		printf( "%5d ", i);
		printf( "%s ", StrCodeGubun[ Mat_StrCode( head->gubun)]);
		printf( "%s ", StrCodeStat[ Mat_StrCode( head->ord_stat)]);
		if( head->rcv_time.tv_sec == 0)
		{
			printf( "%s ", TtoS( head->ord_time.tv_sec));
		}
		else
		{
			printf( "%s ", TtoS( head->rcv_time.tv_sec));
		}
		printf( "%.*s ", ( int)sizeof( obook->ClOrdID), obook->ClOrdID);
		printf( "%.*s ", ( int)sizeof( obook->Symbol), obook->Symbol);
		printf( "%s ", StrCodeSide[ Mat_StrCode( obook->Side[ 0] - '0')] );
		printf( "%s ", StrCodeType[ Mat_StrCode( obook->OrdType[ 0] - '0')]);
		printf( "%10.5f ", obook->Price);
		printf( "%10.5f ", head->exe_price);
		/* printf( "%15f ", pee->rec[ 0].d_fx_csac_prc); */
		printf( " ");
		printf( "%.*s  ", ( int)sizeof( obook->SettType), obook->SettType);
		printf( "%s ", StrCodeOrig[ Mat_StrCode( obook->OrgnGb[ 0] - '0')]);
		if( obook->TrdTypeDcd[ 0] - '1' >= 0)
			printf( "%s ", StrCodePrty[ Mat_StrCode( obook->TrdTypeDcd[ 0] - '0')]);
		else
			printf( "%s ", "    ");
		printf( "%s ", StrCodeTran[ Mat_StrCode( obook->TranPtrnCd[ 0] - '0')]);
		printf( "\n");
		cnt++;
		if( cnt % 50 == 0)
		{
			for( j = 0; j < col; j++) printf( "-"); 
			printf( "\n");
			printf( " pos  ");
			printf( "구분 ");
			printf( "상태 ");
			printf( "recv_time           ");
			printf( "ClOrdID     ");
			printf( "Symbol  ");
			printf( "매매 ");
			printf( "가격 ");
			printf( " 주문가격  ");
			printf( " 체결가격  ");
			printf( " ");
			printf( "상품 ");
			printf( "원천 ");
			printf( "타입 ");
			printf( "유형 ");
			printf( "\n");
			for( j = 0; j < col; j++) printf( "-"); 
			printf( "\n");
		}
		if( view_cnt > 0)
		{
			if( view_cnt == cnt) break;
		}
	}
	for( i = 0; i < col; i++) printf( "-"); 
	printf( "\n");

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통화 조회
***************************************************************************** */
int CmdGroup( int argc, char *argv[])
{
	int				i, col = 120;
	MAT_GROUP		*grp;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	Mat_StatGroup( Mat);

	/*
	printf( "g = 0:empty, 1:주문, 2:체결 \n");
	printf( "S = 1:매수, 2:매도 \n");
	printf( "O = 1:시장가, 2:지정가, 3:예약주문 \n");
	*/

#if 0
	for( i = 0; i < col; i++) printf( "-"); 
	printf( "\n");


	printf( " pos  ");
	printf( "srt ");
	printf( "id          ");
	printf( " seq  ");
	printf( " tot  ");
	printf( " cnt  ");
	printf( "\n");
	for( i = 0; i < col; i++) printf( "-"); 
	printf( "\n");

	for( i = 1; i < MAT_MAX_GROUP; i++)
	{
		grp = &Mat->map->grp[ i];

		printf( "%5d ", i);
		printf( "%3d ", grp->start);
		printf( "%11.11s ", grp->id);
		printf( "%5d ", grp->seq);
		printf( "%5d ", grp->tot);
		printf( "%5d ", grp->cnt);
		printf( "\n");
	}
	for( i = 0; i < col; i++) printf( "-"); 
	printf( "\n");
#endif

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통화 조회
***************************************************************************** */
int CmdSize( int argc, char *argv[])
{
	int				rtn;
	int				i, col = 120;
	MAT_GROUP		_grp, *grp = &_grp;
	MAT_GROUP		*gp;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	/*
	printf( "g = 0:empty, 1:주문, 2:체결 \n");
	printf( "S = 1:매수, 2:매도 \n");
	printf( "O = 1:시장가, 2:지정가, 3:예약주문 \n");
	*/

	printf( "sizeof( MAT_MAP)       = [%ld]\n", sizeof( MAT_MAP));
	printf( "sizeof( SMQ_MAP)       = [%ld]\n", sizeof( SMQ_MAP));
	printf( "sizeof( SMQ_INDEX)     = [%ld]\n", sizeof( SMQ_INDEX));
	printf( "sizeof( ORDER)         = [%ld]\n", sizeof( ORDER));
	printf( "sizeof( ORDER_RECV)    = [%ld]\n", sizeof( ORDER_RECV));
	printf( "sizeof( ORDER_SEND)    = [%ld]\n", sizeof( ORDER_SEND));
	printf( "sizeof( MAT_RECORD)    = [%ld]\n", sizeof( MAT_RECORD));
	printf( "sizeof( SPLIT_IN_ST)   = [%ld]\n", sizeof( SPLIT_IN_ST));
	printf( "sizeof( SPLIT_OUT_ST)  = [%ld]\n", sizeof( SPLIT_OUT_ST));
	printf( "sizeof( APSISE)        = [%ld]\n", sizeof( APSISE));
	printf( "sizeof( MATSISE)       = [%ld]\n", sizeof( MATSISE));
	printf( "\n");

	printf( "MAT_MAX_RECORD         = [%d]\n", MAT_MAX_RECORD);
	printf( "MAT_MAX_CURRENT        = [%d]\n", MAT_MAX_CURRENT);
	printf( "MAT_MAX_CURR(INDEX)    = [%d]\n", MAT_MAX_CURR);
	printf( "MAT_MAX_STATIS         = [%d]\n", MAT_MAX_STATIS);
	printf( "MAT_MAX_GROUP          = [%d]\n", MAT_MAX_GROUP);
	printf( "SMQ_MAX_REC            = [%d]\n", SMQ_MAX_REC);
	printf( "SMQ_REC_SZ             = [%d]\n", SMQ_REC_SZ);
	printf( "\n");

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통화 조회
***************************************************************************** */
int CmdService( int argc, char *argv[])
{
	int				rtn;
	char			*service[] = { "OFF", "ON", NULL };
	MAT_STATUS		*stat;


	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	/*
	printf( "g = 0:empty, 1:주문, 2:체결 \n");
	printf( "S = 1:매수, 2:매도 \n");
	printf( "O = 1:시장가, 2:지정가, 3:예약주문 \n");
	*/

	stat = &Mat->map->stat;

	if( argc >= 2)
	{
		rtn = Conform( "서비스 상태를 바꿉니다. 진행 하시겠습니까?");
		if( rtn == 0) 
		{
			printf( "취소되었습니다.\n");
			return 0;
		}

		stat->service = atoi( argv[ 1]);
	}

	printf( "\n서비스 상태 = [%d:%s]\n\n", stat->service, service[ stat->service]);
	printf( "    ON(1)  = 엔진이 정상 작동합니다.\n");
	printf( "    OFF(0) = 엔진오픈(Mat_Open)시 서비스 상태가 ON(1)일 때까지 기다립니다.\n");
	printf( "             엔진의 서비스 상태가 ON(1)로 변경하는 프로세스는 \n");
	printf( "             mat_bat에서 엔진에 필요한 정보를 로드한 후에 변경합니다..\n");

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통화 조회
***************************************************************************** */
int CmdPair( int argc, char *argv[])
{
	int				rtn;
	int				opt = 0;
	int				i, col = 120;



	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	if( DbCon == 0)
	{
		LogMsg( "DbCon == 0 ...  진행 할 수 없습니다.");
		return 0;
	}

	/*
	printf( "g = 0:empty, 1:주문, 2:체결 \n");
	printf( "S = 1:매수, 2:매도 \n");
	printf( "O = 1:시장가, 2:지정가, 3:예약주문 \n");
	*/

	if( argc >= 2)
	{
		rtn = Conform( "Database의 외화거래정보(INDEX)를 load합니다. 진행 하시겠습니까?");
		if( rtn == 0) 
		{
			printf( "취소되었습니다.\n");
			return 0;
		}
		if( !memcmp( argv[ 1], "load", 4))	opt = 1;
	}
#if DB_USED
	Db_GetCodFxPair( opt);
#endif

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통화 추가
***************************************************************************** */
int CmdPairAdd( int argc, char *argv[])
{
	int				rtn;

	char			pair_str[ 32];
	int				point, unit;

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	switch( argc)
	{
		default:
			sprintf( pair_str, "%s", argv[ 1]);
			point = AtoI( argv[ 2], strlen( argv[ 2]));
			unit = AtoI( argv[ 3], strlen( argv[ 3]));
			break;
		case 3:
		case 2:
		case 1:
			if( !memcmp( argv[ 1], "all", 3))
			{
				Mat_Lock( Mat);

				Mat_AddIndex( Mat, "USD/KRW", 2, 1);
				Mat_AddIndex( Mat, "JPY/KRW", 2, 100);
				Mat_AddIndex( Mat, "EUR/KRW", 2, 1);
				Mat_AddIndex( Mat, "GBP/KRW", 2, 1);
				Mat_AddIndex( Mat, "AUD/KRW", 2, 1);
				Mat_AddIndex( Mat, "NZD/KRW", 2, 1);
				Mat_AddIndex( Mat, "CAD/KRW", 2, 1);
				Mat_AddIndex( Mat, "CHF/KRW", 2, 1);
				Mat_AddIndex( Mat, "CNH/KRW", 2, 1);
				Mat_AddIndex( Mat, "HKD/KRW", 2, 1);
				Mat_AddIndex( Mat, "SGD/KRW", 2, 1);
				Mat_AddIndex( Mat, "SEK/KRW", 2, 1);
				Mat_AddIndex( Mat, "THB/KRW", 2, 1);
				Mat_AddIndex( Mat, "CNY/KRW", 2, 1);

				Mat_AddIndex( Mat, "EUR/USD", 5, 1);
				Mat_AddIndex( Mat, "GBP/USD", 5, 1);
				Mat_AddIndex( Mat, "AUD/USD", 5, 1);
				Mat_AddIndex( Mat, "NZD/USD", 5, 1);
				Mat_AddIndex( Mat, "USD/CAD", 5, 1);
				Mat_AddIndex( Mat, "USD/CHF", 5, 1);
				Mat_AddIndex( Mat, "USD/JPY", 3, 1);
				Mat_AddIndex( Mat, "USD/CNH", 5, 1);
				Mat_AddIndex( Mat, "USD/HKD", 5, 1);
				Mat_AddIndex( Mat, "USD/SGD", 5, 1);
				Mat_AddIndex( Mat, "USD/SEK", 5, 1);
				Mat_AddIndex( Mat, "USD/THB", 3, 1);
				Mat_AddIndex( Mat, "USD/CNY", 5, 1);

				Mat_Unlock( Mat);

				LogMsg( "모든 통화 거래정보를 추가 했습니다.");
				return 1;
			}
		case 0:
			printf( "argument error.\n");
			Cmd_HelpCommand( Cmd, argv[ 0]);
			return 0;
	}

	Mat_Lock( Mat);
	Mat_AddIndex( Mat, pair_str, point, unit);
	Mat_Unlock( Mat);
	LogMsg( "통화 거래정보를 추가 했습니다. pair=[%s] point=[%d] unit=[%d]", pair_str, point, unit);

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통화 조회
***************************************************************************** */
int CmdPriceTime( int argc, char *argv[])
{
	int				rtn;
	int				opt = 0;
	int				i, col = 120;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	if( DbCon == 0)
	{
		LogMsg( "DbCon == 0 ...  진행 할 수 없습니다.");
		return 0;
	}

	/*
	printf( "g = 0:empty, 1:주문, 2:체결 \n");
	printf( "S = 1:매수, 2:매도 \n");
	printf( "O = 1:시장가, 2:지정가, 3:예약주문 \n");
	*/

	if( argc >= 2)
	{
		rtn = Conform( "Database의 시세유효시간(Price time)을 load합니다. 진행 하시겠습니까?");
		if( rtn == 0) 
		{
			printf( "취소되었습니다.\n");
			return 0;
		}
		if( !memcmp( argv[ 1], "load", 4))	opt = 1;
	}
#if DB_USED
	Db_GetCmc003C( opt);
#endif

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통화 조회
***************************************************************************** */
int CmdIndex( int argc, char *argv[])
{
	int				rtn;
	int				pos = -1;
	int				i, col = 120;
	int				idx_pos = -1;;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	/*
	printf( "g = 0:empty, 1:주문, 2:체결 \n");
	printf( "S = 1:매수, 2:매도 \n");
	printf( "O = 1:시장가, 2:지정가, 3:예약주문 \n");
	*/

	switch( argc)
	{
		default:
		case 3:
			idx_pos = atoi( argv[ 2]);
		case 2:
			if( !memcmp( argv[ 1], "send", 4))
			{
				rtn = write( Mat->mat_fd, &idx_pos, sizeof( int));
				if( rtn < sizeof( int))
				{
					printf( "pipe write error. fd=[%d] rtn=[%d] err=[%d:%s]\n", Mat->mat_fd, rtn, errno, strerror( errno));
				}
                printf( "Index position send to pipe. fd=[%d] idx_pos=[%d]\n", Mat->mat_fd, idx_pos);
			}
			else
			{
				pos = atoi( argv[ 1]);
			}
		case 1:
		case 0:
			break;
	}

	if( pos >= 0)		Mat_StatIndexPos( Mat, pos);
	else				Mat_StatIndex( Mat);

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통화 조회
***************************************************************************** */
int CmdJang( int argc, char *argv[])
{
	int				rtn;
	int				arg_time;
	time_t			cur_time;
	struct tm		_tm, *tp = &_tm;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	/*
	printf( "g = 0:empty, 1:주문, 2:체결 \n");
	printf( "S = 1:매수, 2:매도 \n");
	printf( "O = 1:시장가, 2:지정가, 3:예약주문 \n");
	*/

	switch( argc)
	{
		default:
		case 3:
			arg_time = atoi( argv[ 2]);
		case 2:
			time( &cur_time);
			localtime_r( &cur_time, tp);
			tp->tm_hour = arg_time / 10000;
			tp->tm_min  = ( arg_time % 10000) / 100;
			tp->tm_sec  = arg_time % 100;

			if( argv[ 1][ 0] == 's') 		Mat->map->stat.start = mktime( tp);
			else if( argv[ 1][ 0] == 'e')	Mat->map->stat.end = mktime( tp);
			else if( argv[ 1][ 0] == 'l')
			{
#if DB_USED
				l_dbconnect();
				Db_GetTbFxbCmc001M();
				l_dbdisconnect();
#endif
			}
		case 1:
		case 0:
			printf( "장 시작시간 =[%s]\n", TtoS( Mat->map->stat.start));
			printf( "장 마감시간 =[%s]\n", TtoS( Mat->map->stat.end));
			break;
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통화 조회
***************************************************************************** */
int CmdJangPair( int argc, char *argv[])
{
	int				rtn;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	if( Mat == NULL)
	{
		LogMsg( "Mat == NULL ...  진행 할 수 없습니다.");
		return 0;
	}

	if( argc >= 2 && argv[ 1][ 0] == 'l')
	{
		rtn = Conform( "통화별 장운영 데이타를 DB에서 LOAD 하시겠습니까?");
		if( rtn > 0)
		{
#if DB_USED
    		rtn = l_dbconnect();
    		if( rtn != 0)
    		{
        		LogCri( "DB CONNECT ERROR. rtn=[%d] ", rtn);
        		return -1;
    		}
    		LogDbg( "DB CONNECT SUCCESS. rtn=[%d] ", rtn);
		
			Db_GetCmg039M();
	
    		rtn = l_dbdisconnect();
    		if( rtn != 0)
    		{
        		LogCri( "DB DISCONNECT ERROR. rtn=[%d] ", rtn);
        		return -1;
    		}
    		LogDbg( "DB DISCONNECT SUCCESS. rtn=[%d] ", rtn);
#endif
		}
		else
		{
			printf( "취소되었습니다.\n");
			sleep( 1);
		}
	}

	Mat_StatJangPair( Mat);

	/*
	printf( "g = 0:empty, 1:주문, 2:체결 \n");
	printf( "S = 1:매수, 2:매도 \n");
	printf( "O = 1:시장가, 2:지정가, 3:예약주문 \n");
	*/

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통화 조회
***************************************************************************** */
int CmdColor( int argc, char *argv[])
{
	int				rtn;
	int				arg_time;
	time_t			cur_time;
	struct tm		_tm, *tp = &_tm;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	/*
	printf( "g = 0:empty, 1:주문, 2:체결 \n");
	printf( "S = 1:매수, 2:매도 \n");
	printf( "O = 1:시장가, 2:지정가, 3:예약주문 \n");
	*/

	switch( argc)
	{
		default:
		case 2:
			Param->color = atoi( argv[ 1]);
		case 1:
		case 0:
			printf( "color =[%d]\n", Param->color);
			break;
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통화 조회
***************************************************************************** */
int CmdTest( int argc, char *argv[])
{
	int				rtn;
	int				arg_time;
	time_t			cur_time;
	struct tm		_tm, *tp = &_tm;

	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	/*
	printf( "g = 0:empty, 1:주문, 2:체결 \n");
	printf( "S = 1:매수, 2:매도 \n");
	printf( "O = 1:시장가, 2:지정가, 3:예약주문 \n");
	*/

#if DB_USED
    rtn = l_dbconnect();
    if( rtn != 0)
    {
        LogCri( "DB CONNECT ERROR. rtn=[%d] ", rtn);
        return -1;
    }
    LogDbg( "DB CONNECT SUCCESS. rtn=[%d] ", rtn);
	
	Db_GetCmg039M();

    rtn = l_dbdisconnect();
    if( rtn != 0)
    {
        LogCri( "DB DISCONNECT ERROR. rtn=[%d] ", rtn);
        return -1;
    }
    LogDbg( "DB DISCONNECT SUCCESS. rtn=[%d] ", rtn);
#endif

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

	return -1;
}

/** ***************************************************************************
**  @function   sub function
**  @brief      
***************************************************************************** */
/** ***************************************************************************
**  @fu         int CmdReset( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  통화 조회
***************************************************************************** */
int Conform( char *msg)
{
	int		rtn;
	char	buf[ 512];

	printf( "%s (y/n):", msg);
	fflush( stdout);
	fgets( buf, 512, stdin);

	if( buf[ 0] == 'Y' || buf[ 0] == 'y')	rtn = 1;
	else									rtn = 0;

	return rtn;
}






