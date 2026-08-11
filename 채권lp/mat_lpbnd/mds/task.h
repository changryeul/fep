/** ***************************************************************************
**  @file       task.h
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  명령어 수행 관련 해더
***************************************************************************** */
#ifndef TASK_H
#define	TASK_H	1

#include <stdio.h>
#include <stdlib.h>


#include "map.h"
#include "cmd.h"

#include "mds.h"
extern CMD			*Cmd;
extern CMD_TBL		CmdTable[];

#endif /* TASK_H */

/***** Module : task.c *****/
int         TaskInit();                                                     /*  */
int         CmdTest( int argc, char *argv[]);                               /* 1. 함수 프로토타입 정의 */
int         CmdSet( int argc, char *argv[]);                                /* 함수 작성    */
int         CmdList( int argc, char *argv[]);                               /* 함수 작성    */
int         CmdView( int argc, char *argv[]);                               /* 함수 작성    */
int         CmdFold( int argc, char *argv[]);                               /* 함수 작성    */
int         CmdHelp( int argc, char *argv[]);                               /*  */
int         CmdQuit( int argc, char *argv[]);
#if 0
int         SidePrint( mdside_t *side, char *title);
int         PrintFold( MDFOLD *fold, int symb);
int         PrintFoldList( MDFOLD *fold);
int         PrintSwap( MDFOLD *mdfold);
#endif

/***** Module : mon.c *****/
int         Mon_Main();                                                     /* 함수 작성    */
int         Mon_MainInit( MAP *map);                                        /* 모니터링맵 초기화 */
int         Mon_Market( MAP *main_map, int market_no);                      /* 함수 작성    */
int         Mon_MarketInit( MAP *map, int market_no);                       /* 모니터링맵 초기화 */
int         Mon_MdFold( MAP *main_map, int market_no, int fold_no);         /* 함수 작성    */
int         Mon_MdFoldInit( MAP *map, int market_no, int fold_no, int quot_no);/* 모니터링맵 초기화 */
int         Mon_MdQuotInit( MAP *map, int market_no, int fold_no, int quot_no);/* 모니터링맵 초기화 */
int         Mon_TimeMds( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_TimeSet( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_TimeQuotDbl( MAP *map, MAP_FIELD *field);                   /* time stemp convert */
int         Mon_TimeDbl( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_TimeStr( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_GapTime( MAP *map, MAP_FIELD *field);                       /* time stemp convert */

