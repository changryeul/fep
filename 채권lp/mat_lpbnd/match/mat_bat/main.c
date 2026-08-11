/** ***************************************************************************
**  @file       main.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  프로그램 초기화/프로세싱/종료
**  파라메터 세팅 및 환경파일 로드
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>

#include "log.h"
#include "mat.h"
#include "smq.h"
#include "udp.h"
#include "order.h"
#include "sise.h"

#include "main.h"

#define	MYMQ_TEST	0
#define	MAX_TEST	10000000

/** ***************************************************************************
**	GLOBAL
***************************************************************************** */
CFG			*Cfg;
MAT			*Mat;
int			Continue = 1;

#if 0
MAT_ORDER		_Mo;
MAT_ORDER		*Mo = &_Mo;
#endif
FILE			*FilePtr;
char			RecvBuf[ 1024 * 256];

extern MAT_REJECT	MatReject[];

/** ***************************************************************************
**	PARAM
***************************************************************************** */
PARAM	ParamBuf =
{
	0,				/* argc */
	NULL,			/* argv */
	"",				/* argo */
	0,				/* args */
	"",				/* cfg_name */
	"",				/* holiday_file */
	"",				/* log_name */
	0,				/* log_flag */
	0,				/* log_level */
	0,				/* interval */
	0,				/* timeout */
	0,				/* start */
	0,				/* end */
	0,				/* last_time */
	0,				/* force */
};
PARAM	*Param = &ParamBuf;

/** ***************************************************************************
**	Argument & Option
***************************************************************************** */
char	*OptStr = "hsec:";
char	Usage[] = 
"Usage: %s [-hse] [-c config_file]\n"
"Option: -o, --option                \n"
"  h, help                     this help message\n"
"  c, config                   config file name (default:./main.cfg)\n"
"  s, start                    start batch 강제 수행\n"
"  e, end                      end batch 강제 수행\n"
;
struct option LongOption[] = 
{
	{ "help",		no_argument,		0,	'h'},
	{ "config",		required_argument,	0,	'c'},
	{ "start",		no_argument,		0,	's'},
	{ "end",		no_argument,		0,	'e'},
	{ 0,			0,					0,	0}
};
/** ***************************************************************************
**	
***************************************************************************** */

/** ***************************************************************************
**  @fn         int main( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패 종료
**  @retval     양수 성공 종료
**  @exception
**  @remark
**  @brief
**  GetOption   - 인수 분석 및 변수 초기화
**  InitProcess - 프로세스 초기상태 세팅 환경로드(config) 연관 모듈 오픈(open)
**  MainProcess - 프로세스 주요 작업
**  TermProcess - 포로세스 종료 작업 오픈모듈 정리
***************************************************************************** */
int main( int argc, char *argv[])
{
	int		i;
	int		rtn;

	rtn = GetOption( argc, argv);
	if( rtn <= 0)
	{
		printf( Usage, argv[ 0]);
		return -1;
	}

	rtn = InitProcess( argc, argv);
	if( rtn < 0)
	{
		LogCri( "InitProcess error. rtn=[%d]", rtn);
		printf( Usage, argv[ 0]);
	}

	if( rtn > 0)
	{
		rtn = MainProcess( argc, argv);
		if( rtn < 0)
		{
			LogCri( "MainProcess error. rtn=[%d]", rtn);
		}
	}

	rtn = TermProcess( argc, argv);
	if( rtn < 0)
	{
		LogCri( "MainProcess error. rtn=[%d]", rtn);
		return rtn;
	}

	return 1;
}

/** ***************************************************************************
**  @fn         int GetOption( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패 종료
**  @retval     양수 성공 종료
**  @exception
**  @remark
**  @brief
**  프로그램 시작시 받은 인수를 분석, 프로그램에서 사용할 변수를 초기화 한다.
**  Param->argc  - 인프로그램에서 받은 인수 갯수
**  Param->argv  - 프로그램에서 받은 인수
**  Param->argo  - 받은 옵션 저장, -x 의 x를 저장한다. - 환경에서 읽은 변수와 
                    중복되지 않게 이미 파라메터에서 세팅된 변수가 어느것인지 
					정보 표시를 위함
**  Param->args  - 받은 옵션 저장 갯수
***************************************************************************** */
int GetOption( int argc, char *argv[])
{
	int		opt;
	int		opt_ind = optind ? optind : 1;
	int		opt_idx = 0;
	char	*p;

	Param->argc = argc;
	Param->argv = argv;

	/************************************************/
	/* get parameter                                */
	/************************************************/
	while( 1)
	{
		opt = getopt_long( argc, argv, OptStr, LongOption, &opt_idx);
		if( opt < 0) break;

		switch( opt)
		{
			case -1  :
				return 1;
			case 0   :
				break;
			case 'h' :
				return 0;
			case 'c' :
				sprintf( Param->cfg_name, optarg, strlen( optarg) +1);
				LogMsg( "Param->cfg_name=[%s]", Param->cfg_name);
				break;
			case 's' :
				Param->force = 1;
				break;
			case 'e' :
				Param->force = 2;
				break;
			default  :
				break;
		}
		if( opt != 0) Param->argo[ Param->args++] = opt;
	}
	argc -=optind;
	argv = &argv[ optind];

	return 1;
}

/** ***************************************************************************
**  @fn         void SignalProcess( int sig_id)
**  @param      int sig_id - 시그널 번호
**  @return     없음
**  @exception
**  @remark
**  @brief
**  프로그램에서 사용할 시그널
***************************************************************************** */
void SignalProcess( int sig_id)
{
	LogMsg( "Received signal=[%d]", sig_id);
	switch( sig_id)
	{
		case SIGINT:
		case SIGSTOP:
		case SIGQUIT:
		case SIGTERM:
			Continue = 0;
			LogMsg( "signal=[%d] Continue=[%d]", sig_id, Continue);
			signal( sig_id, SignalProcess);
			break;
		default:
			LogMsg( "no action. signal=[%d]", sig_id);
			break;
	}
	LogMsg( "signal=[%d] process end.", sig_id);

	return;
}

/** ***************************************************************************
**  @fn         int InitProcess( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  프로그램 초기화
	    config file 로드
		로그파일 오픈
		전역변수 Param 값 세팅
**  전역변수 Param->argo의 값에 따라 세팅할 변수를 선택		
***************************************************************************** */
int InitProcess( int argc, char *argv[])
{
	int			rtn;
	char		*ptr;
	char		*path, *name;
	time_t		cur_time;

	char			*whoami;

	signal( SIGINT, SignalProcess);
	signal( SIGSTOP, SignalProcess);
	signal( SIGQUIT, SignalProcess);
	signal( SIGTERM, SignalProcess);

	if( argc < 1)
	{
		return -1;
	}

	/************************************************/
	/* config file load                             */
	/************************************************/
	if( strchr( Param->argo, 'c') == NULL)
	{
#if 1
		sprintf( Param->cfg_name, "%s/%s", getenv( "MAT_CFG"), "mat_bat.cfg");
#else
		sprintf( Param->cfg_name, "%s/%s", ".", "main.cfg");
#endif
		LogDbg( "config file load.  Param->cfg_name = [%s]", Param->cfg_name);
	}
	Cfg = Cfg_Open( Param->cfg_name);
	if( Cfg == NULL)
	{
		LogCri( "config file open error. name=[%s]", Param->cfg_name);
		return -1;
	}

	rtn = Cfg_Get( Cfg, "holiday_file", Param->holiday_file, 512);

	rtn = Cfg_Get( Cfg, "log_name", Param->log_name, 512);
	if( rtn <= 0)
	{
		LogCri( "Config not found. name=[%s]", "log_name");
		return -1;
	}
	Param->log_flag  = Cfg_GetInt( Cfg, "log_flag");
	Param->log_level = Cfg_GetInt( Cfg, "log_level");

	Param->interval = Cfg_GetInt( Cfg, "interval");
	Param->timeout  = Cfg_GetInt( Cfg, "timeout");

	Param->start = Cfg_GetInt( Cfg, "start");
	Param->end   = Cfg_GetInt( Cfg, "end");

	Param->last_time = Cfg_GetInt( Cfg, "last_time");

	/************************************************/
	/* log file open                                */
	/************************************************/
	LogLevel( Param->log_level);
	if( Param->log_flag == 1) 
	{
		LogMsg( "Redirect log message to file. name=[%s]", Param->log_name);
		LogFile( Param->log_name);
	}
	else
	if( Param->log_flag == 2) 
	{
		LogMsg( "Redirect log message to mem. name=[%s]", Param->log_name);
		LogMem( Param->log_name);
	}

	/************************************************/
	/* SMQ queue open                               */
	/************************************************/

	/************************************************/
	/* 매칭 엔진 open                               */
	/************************************************/
	Mat = Mat_Open( 1);
	if( Mat == NULL)
	{
		LogCri( "매칭 엔진 open error.");
		return -1;
	}

	time( &cur_time);
	LogMsg( "######################################################");
	LogMsg( "##### PROCESS START at %s pid=[%d]", TtoS( cur_time), getpid());
	LogMsg( "######################################################");

	ParamPrint( Param);

	switch( Param->force)
	{
		case 1:
			LogMsg( "시작 배치를 강제 수행합니다.");
			printf( "시작 배치를 강제 수행합니다.\n");
			rtn = StartBatchProcess();
			if( rtn < 0)
			{
				LogMsg( "시작 배치 오류=[%d]", rtn);
				printf( "시작 배치 오류=[%d]\n", rtn);
			}
			else
			{
				LogMsg( "시작 배치를 강제 수행했습니다..");
				printf( "시작 배치를 강제 수행했습니다.\n");
			}
			return 0;
		case 2:
			LogMsg( "종료 배치를 강제 수행합니다.");
			printf( "종료 배치를 강제 수행합니다.\n");
			EndBatchProcess();
			LogMsg( "종료 배치를 강제 수행했습니다.");
			printf( "종료 배치를 강제 수행했습니다.\n");
			return 0;
		default:
			return 1;
	}

	/************************************************/
	/* DB Connect                                   */
	/************************************************/
	/*
	Db_Process();
	*/

	return 1;
}

/** ***************************************************************************
**  @fn         int MainProcess( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  프로그램 수행
***************************************************************************** */
int MainProcess( int argc, char *argv[])
{
	int				rtn = 0;
	int				cnt = 0;
	time_t			cur_time, bat_time;
	time_t			s_gap, e_gap, c_gap;
	struct tm		_tm, *tp = &_tm;

	/************************************************/
	/* 배치 시간 변경 check                         */
	/************************************************/
	if( Mat->map->stat.bs_int != Param->start)
	{
		if( Mat->map->stat.bs_int == 0) /* create */
		{
			StartBatchProcess();
		}
		LogMsg( "시작 배치 시간이 변경 되었습니다. old=[%06d] new=[%06d]", Mat->map->stat.bs_int, Param->start);
		Mat->map->stat.bs_int  = Param->start;
		Mat->map->stat.bs_time = 0;
	}

	if( Mat->map->stat.be_int != Param->end)
	{
		LogMsg( "종료 배치 시간이 변경 되었습니다. old=[%06d] new=[%06d]", Mat->map->stat.be_int, Param->end);
		Mat->map->stat.be_int  = Param->end;
		Mat->map->stat.be_time = 0;
	}

	time( &cur_time);

	/************************************************/
	/* 배치 시간 setting                            */
	/************************************************/
	if( cur_time > Mat->map->stat.bs_time)
	{
		tp = localtime_r( &cur_time, tp);
		tp->tm_hour = Mat->map->stat.bs_int / 10000;
		tp->tm_min  = ( Mat->map->stat.bs_int % 10000) / 100;
		tp->tm_sec  = Mat->map->stat.bs_int % 100;
		Mat->map->stat.bs_time = mktime( tp);
		LogDbg( "start gap=[%d]", cur_time - Mat->map->stat.bs_time);
		if( Mat->map->stat.bs_time < cur_time) Mat->map->stat.bs_time += 24 * 3600;
		LogMsg( "start batch time=[%s]", TtoS( Mat->map->stat.bs_time));
	}

	if( cur_time > Mat->map->stat.be_time)
	{
		tp = localtime_r( &cur_time, tp);
		tp->tm_hour = Mat->map->stat.be_int / 10000;
		tp->tm_min  = ( Mat->map->stat.be_int % 10000) / 100;
		tp->tm_sec  = Mat->map->stat.be_int % 100;
		Mat->map->stat.be_time = mktime( tp);
		if( Mat->map->stat.be_time < cur_time) Mat->map->stat.be_time += 24 * 3600;
		LogMsg( "  end batch time=[%s]", TtoS( Mat->map->stat.be_time));
	}

	/************************************************/
	/* log write                                    */
	/************************************************/
	LogMsg( "시작배치 시간=[%s]", TtoS( Mat->map->stat.bs_time));
	LogMsg( "종료배치 시간=[%s]", TtoS( Mat->map->stat.be_time));
	LogMsg( "주문중지 시간=[%s]", TtoS( Mat->map->stat.cancel_start));
	LogMsg( "[시작배치][종료배치][주문중지][ 주문 ][ 체결 ][ 위치 ]");

	s_gap = Mat->map->stat.bs_time - cur_time;
	e_gap = Mat->map->stat.be_time - cur_time;
	c_gap = Mat->map->stat.cancel_start - cur_time;
	LogMsg( "[%02d:%02d:%02d][%02d:%02d:%02d][%02d:%02d:%02d][%6d][%6d][%6d]", 
			s_gap / 3600, ( s_gap % 3600) / 60, s_gap % 60,
			e_gap / 3600, ( e_gap % 3600) / 60, e_gap % 60,
			c_gap / 3600, ( c_gap % 3600) / 60, c_gap % 60,
			Mat->map->stat.rec_cnt, 
			Mat->map->stat.exe_cnt,
			Mat->map->stat.wpos
			);

	while( Continue)
	{
		time( &cur_time);
		s_gap = Mat->map->stat.bs_time - cur_time;
		e_gap = Mat->map->stat.be_time - cur_time;
		c_gap = Mat->map->stat.cancel_start - cur_time;

		if( s_gap <= 0)
		{
			LogDbg( "start batch start ...");
			rtn = StartBatchProcess();
			if( rtn < 0)
			{
				LogCri( "Start batch process error. rtn = [%d]", rtn);
				sleep( 60);
				continue;
			}
			Mat->map->stat.bs_time += 24 * 3600;
			continue;
		}

		if( e_gap <= 0)
		{
			LogDbg( "end batch start ...");
			EndBatchProcess();
			Mat->map->stat.be_time += 24 * 3600;
			continue;
		}

		if( c_gap <= 0)
		{
			LogDbg( "cancel batch start ...");
			ForceCancelProcess();
			Mat->map->stat.cancel_start += 24 * 3600;
			continue;
		}

		if( ( cur_time % 60) == 0)
		{
			if( cur_time % 3600 == 0) 
			{
				LogMsg( "시작배치 시간=[%s]", TtoS( Mat->map->stat.bs_time));
				LogMsg( "종료배치 시간=[%s]", TtoS( Mat->map->stat.be_time));
				LogMsg( "주문중지 시간=[%s]", TtoS( Mat->map->stat.cancel_start));
				LogMsg( "[시작배치][종료배치][주문중지][ 주문 ][ 체결 ][ 위치 ]");
			}
			LogMsg( "[%02d:%02d:%02d][%02d:%02d:%02d][%02d:%02d:%02d][%6d][%6d][%6d]", 
					s_gap / 3600, ( s_gap % 3600) / 60, s_gap % 60,
					e_gap / 3600, ( e_gap % 3600) / 60, e_gap % 60,
					c_gap / 3600, ( c_gap % 3600) / 60, c_gap % 60,
					Mat->map->stat.rec_cnt, 
					Mat->map->stat.exe_cnt,
					Mat->map->stat.wpos
					);
		}

		usleep( Param->interval);
	}
	LogDbg( "MainProcess end.");

	return 1;
}

/** ***************************************************************************
**  @fn         int TermProcess( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패 종료
**  @retval     양수 성공 종료
**  @exception
**  @remark
**  @brief
**  프로세스 종료 루틴 수행
***************************************************************************** */
int TermProcess( int argc, char *argv[])
{
	time_t			cur_time;

	time( &cur_time);
	LogMsg( "##### PROCESS END at %s pid=[%d]", TtoS( cur_time), getpid());
	return 1;
}

/** ***************************************************************************
**  @fn         int ParamPrint( PARAM *param)
**  @param      PARAM *param - 프로그램 전역에서 사용할 변수
**  @return     항상 1
**  @exception
**  @remark
**  @brief
**  프로그램에서 사용할 전역변수들의 구조체 값 표시
***************************************************************************** */
int ParamPrint( PARAM *param)
{
	int		i;

	LogDbg( "param                 = [%p]", param);
	LogDbg( "param->argc           = [%d]", param->argc);
	for( i = 0; i < param->argc; i++)
	LogDbg( "param->argv[%3d]      = [%s]", i, param->argv[i]);
	LogDbg( "param->argo           = [%s]", param->argo);
	LogDbg( "param->args           = [%d]", param->args);
	LogDbg( "param->cfg_name       = [%s]", param->cfg_name);
	LogDbg( "param->holiday_file   = [%s]", param->holiday_file);
	LogDbg( "param->log_name       = [%s]", param->log_name);
	LogDbg( "param->log_flag       = [%d]", param->log_flag);
	LogDbg( "param->log_level      = [%d]", param->log_level);
	LogDbg( "param->timeout        = [%d]", param->timeout);
	LogDbg( "param->interval       = [%d]", param->interval);
	LogDbg( "param->start          = [%d]", param->start);
	LogDbg( "param->end            = [%d]", param->end);
	LogDbg( "param->last_time      = [%d]", param->last_time);
	return 1;
}

/*********************************************************************************************************
* PROCESS
*********************************************************************************************************/
/** ***************************************************************************
**  @fn         int ParamPrint( PARAM *param)
**  @param      PARAM *param - 프로그램 전역에서 사용할 변수
**  @return     항상 1
**  @exception
**  @remark
**  @brief
**  프로그램에서 사용할 전역변수들의 구조체 값 표시
***************************************************************************** */
int StartBatchProcess()
{
	int			rtn, i;
	MAT_MESSAGE	*msg;


	Mat_Lock( Mat);
	Mat->map->stat.service = 0;
	LogMsg( "서비스 상태 ... Mat->map->stat.service=[OFF]");

	/* error message 로드 from mat_bat.cfg */
	LogMsg( "file %s에서 메세지 정보를 load합니다.", Param->cfg_name);
	rtn = Proc_LoadMessage( Mat);
	LogDbg( "msg cnt=[%d]", rtn);

	/* 내부엔진 통화코드 로드 from mat_bat.cfg */
	LogMsg( "file %s에서 통화 정보를 load합니다.", Param->cfg_name);
	rtn = Proc_LoadCurrent();

	Mat_Unlock( Mat);

	/* Mat_BatchStart 안에서 lock 수행 */
	LogDbg( "Call Mat_BatchStart ... mat=[%p]", Mat);
	/* 통계 초기화, 시세 초기화 */
	rtn = Mat_BatchStart( Mat);

	Mat_Lock( Mat);
	/* 거래 통화 쌍, 시세 유효시간 load from DB */
	rtn = Db_Process();
	if( rtn < 0)
	{
		LogCri( "Db_Process error.");
		goto error;
	}
	/* 월말 setting */
	GetLastDay();
	Mat_Unlock( Mat);


	Mat->map->stat.service = 1;
	LogMsg( "서비스 상태 ... Mat->map->stat.service=[ON]");

	return 1;

	error:
		Mat_Unlock( Mat);
		return -1;
}

/** ***************************************************************************
**  @fn         int ParamPrint( PARAM *param)
**  @param      PARAM *param - 프로그램 전역에서 사용할 변수
**  @return     항상 1
**  @exception
**  @remark
**  @brief
**  프로그램에서 사용할 전역변수들의 구조체 값 표시
***************************************************************************** */
int EndBatchProcess()
{
	Mat_Lock( Mat);
	Mat_BatchEnd( Mat);
	Mat_Unlock( Mat);
	HolidayProcess();
	return 1;
}

/** ***************************************************************************
**  @fn         int ParamPrint( PARAM *param)
**  @param      PARAM *param - 프로그램 전역에서 사용할 변수
**  @return     항상 1
**  @exception
**  @remark
**  @brief
**  주문 중지
***************************************************************************** */
int ForceCancelProcess()
{
	time_t	cur_time;

	/* 시세 process 중지 */
	LogMsg( "시세 수신을 중지합니다.");
	Mat->map->stat.sise = 1;

	/* 시세 처리 대기 */
	usleep( Param->interval);

	Mat_Lock( Mat);

	/* 주문 강제 취소 */
	Mat_ForceCancel( Mat);

	Mat_Unlock( Mat);		/* 강제취소 process를 위해서 Unlock */

	/* 주문 중지 끝날때 까지 대기 */
	while( Continue)
	{
		time( &cur_time);
		if( cur_time > Mat->map->stat.cancel_end) break;

		if( cur_time % 60 == 0)	LogMsg( "주문 중지 시간입니다.");
		usleep( Param->interval);
	}

	/* 시세 process 중지 */
	LogMsg( "시세 수신을 시작합니다.");
	Mat->map->stat.sise = 0;

	return 1;
}

/** ***************************************************************************
**  @fn         int HolidayProcess()
**  @param      PARAM *param - 프로그램 전역에서 사용할 변수
**  @return     항상 1
**  @exception
**  @remark
**  @brief
**  휴일테이블 생성
***************************************************************************** */
int HoliCompare( const void *e1, const void *e2)
{
	MAT_HOLIDAY_REC	*p1 = e1;
	MAT_HOLIDAY_REC	*p2 = e2;

	if( p2->day <= 0) return -1;

	return p1->day - p2->day;
}

int HolidayProcess()
{
	int		i;
	int		rtn;
	MAT_HOLIDAY		_holiday, *holiday = &_holiday;
	MAT_HOLIDAY_REC	*rec;
	char			line[ 512];
	FILE			*fp;
	int				sz = 0;;
	time_t			cur_time;

	LogMsg( "휴일정보를 LOAD합니다.");

	memset( holiday, 0x00, sizeof( MAT_HOLIDAY));

	l_dbconnect();

	rtn = Db_GetFixHoliday( holiday);
	rtn = Db_GetFltHoliday( holiday);
	LogDbg( "rtn=[%d]", rtn);

	l_dbdisconnect();

	if( holiday->cnt <= 0)
	{
		LogCri( "휴일 테이블을 생성 할 수 없습니다. holiday->cnt=[%d]", holiday->cnt);
		return -1;
	}

	qsort( &holiday->rec[ 0], holiday->cnt, sizeof( MAT_HOLIDAY_REC), HoliCompare);

	fp = fopen( Param->holiday_file, "w+");
	if( fp == NULL)
	{
		LogErr( "fopen error. name=[%s]", Param->holiday_file);
		return -1;
	}

	time( &cur_time);
	sprintf( line, "# Create holiday config file. at=[%s] by mat_bat(pid=[%d]).\n\n", TtoS( cur_time), getpid());
	fputs( line, fp);

	sprintf( line, "holiday_list\n");
	fputs( line, fp);
	sprintf( line, "{\n");
	fputs( line, fp);

	for( i = 0; i < holiday->cnt; i++)
	{
		rec = &holiday->rec[ i];
		LogRaw( "[%03d]", i);
		LogRaw( "[%d]", rec->day);
		LogRaw( "[%s]", rec->str);
		LogRaw( "[%s]", rec->comment);
		LogRaw( "\n");

		sz  = 0;
		sz += sprintf( &line[ sz], "\t\"");
		sz += sprintf( &line[ sz], "%03d:", i);
		sz += sprintf( &line[ sz], "%08d:", rec->day);
		sz += sprintf( &line[ sz], "%s:",   rec->str);
		sz += sprintf( &line[ sz], "%s:",   rec->comment);
		sz += sprintf( &line[ sz], ":\"\n");
		fputs( line, fp);
	}
	sprintf( line, "}\n\n");
	fputs( line, fp);

	fclose( fp);
	LogMsg( "휴일정보를 저장했습니다. cnt=[%d] file=[%s]", holiday->cnt, Param->holiday_file);

	return 1;
}

/** ***************************************************************************
**  @fn         int HolidayProcess()
**  @param      PARAM *param - 프로그램 전역에서 사용할 변수
**  @return     항상 1
**  @exception
**  @remark
**  @brief
**  이달의 마지막 영업일 return
***************************************************************************** */
int GetLastDay()
{
	int		i;
	int		rtn;
	MAT_HOLIDAY		_holiday, *holiday = &_holiday;
	MAT_HOLIDAY_REC	*rec;
	int				sz = 0;;

	time_t			cur_time;
	time_t			pre_time;
	int				int_time;
	int				int_today;
	int				holiday_flag = 0;
	struct tm		_tm, *tp = &_tm;

	LogMsg( "마지막 영업일을 조회합니다.");

	memset( holiday, 0x00, sizeof( MAT_HOLIDAY));

	l_dbconnect();

	rtn = Db_GetFixHoliday( holiday);
	rtn = Db_GetFltHoliday( holiday);
	LogDel( "rtn=[%d]", rtn);

	l_dbdisconnect();

	if( holiday->cnt <= 0)
	{
		LogCri( "마지막 영업일을 조회할 수 없습니다. holiday->cnt=[%d]", holiday->cnt);
		return -1;
	}

	qsort( &holiday->rec[ 0], holiday->cnt, sizeof( MAT_HOLIDAY_REC), HoliCompare);

	time( &cur_time);
	localtime_r( &cur_time, tp);

	int_today = ( tp->tm_year + 1900 ) * 10000
				+ ( tp->tm_mon +1) * 100
				+ tp->tm_mday;
	LogDel( "int today = [%d]", int_today);

	/* test --- 2023 9월 */
#if 0
	int_today = 20230927;
	tp->tm_mon = 8;
	tp->tm_year = 123;
#endif

	/* 다음달 1일 time_t 구함 */
	if( tp->tm_mon >= 11)	tp->tm_mon = 1;
	else					tp->tm_mon += 1;
	tp->tm_mday = 1;

	pre_time = mktime( tp);
	LogDel( "next month = [%s]", ctime( &pre_time));

	/* 1일의 전일이 영업일인 날  */
	while( Continue)
	{
		pre_time -= 24 * 3600;
		LogDel( "last day   = [%s]", ctime( &pre_time));
		localtime_r( &pre_time, tp);

		int_time = ( tp->tm_year + 1900 ) * 10000
					+ ( tp->tm_mon +1) * 100
					+ tp->tm_mday;
		LogDel( "int day    = [%d]", int_time);

		/* 토요일, 일요일은 영업일이 아님 */
		if( tp->tm_wday == 0) continue;
		if( tp->tm_wday == 6) continue;

		/* 휴일이면 영업일이 아님 */
		holiday_flag = 0;
		for( i = 0; i < holiday->cnt; i++)
		{
			rec = &holiday->rec[ i];
			if( int_time == rec->day) 
			{
				holiday_flag = 1;
				break;
			}
		}

		if( holiday_flag) continue;
		break;
	}

	LogMsg( "마지막 영업일 = [%d]", int_time);

	if( int_time == int_today)
	{
		LogMsg( "오늘은 월말일 입니다.");
		Mat->map->stat.last_day = 1;
		time( &cur_time);
		localtime_r( &cur_time, tp);
		tp->tm_hour = Param->last_time / 10000;
		tp->tm_min  = ( Param->last_time % 10000) / 100;
		tp->tm_sec  = Param->last_time % 100;
		Mat->map->stat.last_time = mktime( tp);;
		LogMsg( "최종거래시간=[%s]", TtoS( Mat->map->stat.last_time));

		return int_time;
	}
	else						
	{
		LogMsg( "오늘은 월말일이 아닙니다.");
		Mat->map->stat.last_day = 0;
		Mat->map->stat.last_time = 0;
		return 0;
	}
}

