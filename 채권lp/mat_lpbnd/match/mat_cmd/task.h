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

#include "mat.h"
#include "smq.h"

#include "main.h"

extern CMD			*Cmd;
extern CMD_TBL		CmdTable[];

#endif /* TASK_H */

/***** Module : task.c *****/
int         TaskInit();                                                     /*  */
int         CmdTest( int argc, char *argv[]);                               /* 1. 함수 프로토타입 정의 */
int         CmdCreate( int argc, char *argv[]);                             /* 함수 작성    */
int         CmdRemove( int argc, char *argv[]);                             /* 함수 작성    */
int         CmdOpen( int argc, char *argv[]);                               /* 함수 작성    */
int         CmdClose( int argc, char *argv[]);                              /* 함수 작성    */
int         CmdStat( int argc, char *argv[]);                               /* 함수 작성    */
int         CmdInsert( int argc, char *argv[]);                             /* 함수 작성    */
int         CmdUpdate( int argc, char *argv[]);                             /* delete and insert */
int         CmdDelete( int argc, char *argv[]);                             /* delete and insert */
int         CmdLock( int argc, char *argv[]);                               /* 함수 작성    */
int         CmdOrder( int argc, char *argv[]);                              /* 함수 작성    */
int         CmdMatch( int argc, char *argv[]);                              /* 함수 작성    */
int         CmdExecute( int argc, char *argv[]);                            /* 함수 작성    */
int         CmdPipe( int argc, char *argv[]);                               /* 함수 작성    */
char*       GetInput( char *msg);                                           /* 함수 작성    */
int         CmdMon( int argc, char *argv[]);                                /* 함수 작성    */
int         CmdHelp( int argc, char *argv[]);                               /*  */
int         CmdQuit( int argc, char *argv[]);

