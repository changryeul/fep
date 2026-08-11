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

extern CMD			*Cmd;
extern CMD_TBL		CmdTable[];
extern char			DirList[ 64][512];
extern int			DirListCnt;
extern int			Task;

#endif /* TASK_H */

/***** Module : task.c *****/
int         TaskInit();                                                     /*  */
int         CmdTest( int argc, char *argv[]);                               /* 1. 함수 프로토타입 정의 */
int         TaskCurrentTreeView( int argc, char *argv[]);                   /* 함수 작성    */
int         TaskFindDir( int argc, char *argv[]);                           /* 함수 작성    */
int         TaskTreeView( int argc, char *argv[]);                          /* 함수 작성    */
int         TaskScanDir( int argc, char *argv[]);                           /* 함수 작성    */
int         NotMainProcess( int argc, char *argv[]);                        /* 함수 작성    */
int         CmdHelp( int argc, char *argv[]);                               /*  */
int         CmdQuit( int argc, char *argv[]);

