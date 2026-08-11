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

#endif /* TASK_H */

/***** Module : task.c *****/
int         TaskInit();
int         CmdTest( int argc, char *argv[]);
int         CmdHelp( int argc, char *argv[]);
int         CmdQuit( int argc, char *argv[]);

/***** Module : file_load.c *****/
char**      FileLoad( char *file_name);                                     /* GetOption   - 인수 분석 및 변수 초기화 */
int         FileFree( char **file);

/***** Module : process.c *****/
int         MapEditFile( char *file_name);                                  /* 1. 함수 프로토타입 정의 */
int         MapEditFileInit( MAP *map, char **file);                            /* 함수 작성    */
int         Process( MAP *map);                                             /* 함수 작성    */
int         Proc_Init( MAP *map, MY_DATA *my);                              /* 함수 작성    */
int         Proc_AlarmStat( MAP *map, MAP_FIELD *field);                    /* 함수 작성    */
int         Proc_TimeProc( MAP *map, MAP_FIELD *field);                     /* 함수 작성    */

