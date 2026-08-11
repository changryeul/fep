/** ***************************************************************************
**  @file       main.h
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  메인 모듈 관련 헤더
**  프로그램 사용 파라메터 정의
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

#ifndef DB_ACCESS_H
#define	DB_ACCESS_H	1

EXEC SQL BEGIN DECLARE SECTION;
typedef struct _opr_dhpair_cfg_
{
	char	pair_id			[7	+1];
	char	ccy1			[3	+1];
	char	ccy2			[3	+1];
	char	pnt_digt		[22	+1];
	char	pnt_digt_val	[22	+1];
	char	cli_digt		[22	+1];
	char	cli_digt_val	[22	+1];
	char	pl_pair_id		[7	+1];
	char	pl_calc_tp		[1	+1];
	char	px_unit			[22	+1];
	char	roundingtype	[1	+1];
	char	spotleg			[22	+1];
	char	disp_seq		[22	+1];
	char	active_yn		[1	+1];
	char	upd_id			[20	+1];
	char	upd_dt			[8	+1];
	char	upd_tm			[6	+1];
}	OPR_DHPAIR_CFG;
EXEC SQL END DECLARE SECTION;

#define NO_DATA_FOUND	1403


#endif	/* DB_ACCESS_H */


