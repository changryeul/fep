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
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>

#include "log.h"
#include "etc.h"

#include "mat.h"
#include "order.h"
#include "sise.h"
#include "main.h"

/** ***************************************************************************
**	GLOBAL
***************************************************************************** */
extern int			Continue;

/** ***************************************************************************
**	
***************************************************************************** */
/** ***************************************************************************
**  @fn         int Proc_()
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  신규주문 매칭
**  시세와 관계없이 현 시세로 매칭
***************************************************************************** */
int Proc_Timeout( MAT *mat)
{
	int		rtn;

	rtn = Mat_MatchTimeout( mat);
	if( rtn < 0)
	{
		return -1;
	}

	return 1;
}

/** ***************************************************************************
**  @fn         int Proc_()
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  매칭
***************************************************************************** */
int Proc_Match( MAT *mat, APSISE *sise)
{
	int			rtn = 1;
	MAT_INDEX	*index;
	MATSISE		*mat_sise = ( MATSISE *)sise;
	int			base, cont;
	time_t		cur_time;
	struct tm	_tp, *tp = &_tp;

	LogDel( "Match Symb=[%.6s] bid=[%9f] ask=[%9f]", 
		sise->symb, sise->bidprc, sise->offerprc);

	/* convert APSISE -> MATSISE */
	time( &cur_time);
	localtime_r( &cur_time, tp);
	tp->tm_year = AtoI( &sise->date[ 0], 4) -1900;
	tp->tm_mon = AtoI( &sise->date[ 4], 2) -1;
	tp->tm_mday = AtoI( &sise->date[ 6], 2);
	tp->tm_hour = AtoI( &sise->time[ 0], 2);
	tp->tm_min = AtoI( &sise->time[ 2], 2);
	tp->tm_sec = AtoI( &sise->time[ 4], 2);
	mat_sise->ctime = mktime( tp);
	base = Mat_GetCurrentInt( mat, &sise->symb[ 0]);
	if( base < 0) { rtn = -1; goto error; }
	cont = Mat_GetCurrentInt( mat, &sise->symb[ 4]);
	if( cont < 0) { rtn = -2; goto error; }
	index = Mat_GetIndex( mat, base, cont);
	if( index == NULL) { rtn = -3; goto error; }
	mat_sise->price_time = index->price_time;
	LogDel( "[%d][%d][%d]", tp->tm_year, tp->tm_mon, tp->tm_mday);
	LogDel( "[%d][%d][%d]", tp->tm_hour, tp->tm_min, tp->tm_sec);
	LogDel(  "sym=[%.7s][%ld]", mat_sise->symb, mat_sise->ctime);

#if 0
	MATSISE_Print( mat_sise);
#endif
	rtn = Mat_Match( mat, mat_sise);
	if( rtn < 0)
	{
		LogCri( "Mat_Match error. rtn=[%d]", rtn);
		rtn = -4;
		goto error;
	}
	LogDel( "Mat_Match ... rtn = [%d]", rtn);

	return 1;

	error:
		switch( rtn)
		{
			case -1:
				LogCri( "base current not found. current=[%.3s]", &sise->symb[ 0]);
				break;
			case -2:
				LogCri( "cont current not found. current=[%.3s]", &sise->symb[ 4]);
				break;
			case -3:
				LogCri( "index not found. current=[%.7s]", sise->symb);
				break;
			case -4:
				break;
		}
		MATSISE_Print( mat_sise);
		return rtn;
}

/** ***************************************************************************
**  @fn         int Proc_()
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  매칭
***************************************************************************** */
int Proc_MatchPipe( MAT *mat, int pos)
{
	int		rtn;

	LogDel( "Match pos=[%5d]", pos);
	
	rtn = Mat_MatchPos( mat, pos);
	if( rtn < 0)
	{
		LogCri( "Mat_MatchPos error. rtn=[%d]", rtn);
		return -1;
	}

	return rtn;
}


