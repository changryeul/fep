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
#include "smq.h"

#include "main.h"

extern CMD			*Cmd;
extern CMD_TBL		CmdTable[];

#endif /* TASK_H */


/***** Module : main.c *****/
int         main( int argc, char *argv[]);                                  /* GetOption   - 인수 분석 및 변수 초기화 */
int         GetOption( int argc, char *argv[]);                             /* 프로그램 시작시 받은 인수를 분석, 프로그램에서 사용할 변수를 초기화 한다. */
void        SignalProcess( int sig_id);                                     /* 프로그램에서 사용할 시그널 */
int         InitProcess( int argc, char *argv[]);                           /* 프로그램 초기화 */
int         MainProcess( int argc, char *argv[]);                           /* 프로그램 수행 */
int         TermProcess( int argc, char *argv[]);                           /* 프로세스 종료 루틴 수행 */
int         ParamPrint( PARAM *param);                                      /* 프로그램에서 사용할 전역변수들의 구조체 값 표시 */

/***** Module : task.c *****/
int         TaskInit();                                                     /*  */
int         CmdTest( int argc, char *argv[]);                               /* 1. 함수 프로토타입 정의 */
int         CmdCreate( int argc, char *argv[]);                             /* 함수 작성    */
int         CmdRemove( int argc, char *argv[]);                             /* 함수 작성    */
int         CmdOpen( int argc, char *argv[]);                               /* 함수 작성    */
int         CmdClose( int argc, char *argv[]);                              /* 함수 작성    */
int         CmdStat( int argc, char *argv[]);                               /* 함수 작성    */
int         CmdSend( int argc, char *argv[]);                               /* 함수 작성    */
int         CmdRecv( int argc, char *argv[]);                               /* delete and insert */
int         CmdLock( int argc, char *argv[]);                               /* 함수 작성    */
int         CmdOrder( int argc, char *argv[]);                              /* 함수 작성    */
int         CmdPipe( int argc, char *argv[]);                               /* 함수 작성    */
char*       GetInput( char *msg);                                           /* 함수 작성    */
int         CmdMon( int argc, char *argv[]);                                /* 함수 작성    */
int         CmdHelp( int argc, char *argv[]);                               /*  */
int         CmdQuit( int argc, char *argv[]);

/***** Module : ../smq.c *****/
SMQ*        Smq_CreateForce();                                              /* 매칭엔진에 필요한 ipc를 생성 */
SMQ*        Smq_Create();                                                   /* 매칭엔진에 필요한 ipc를 생성 */
int         Smq_Remove( SMQ *smq);                                          /* 매칭엔진에서 생성한 ipc를 삭제 */
SMQ*        Smq_Open( char *name);                                          /* 매칭엔진 Open */
int         Smq_Close( SMQ *smq);                                           /* 매칭엔진 Close */
int         Smq_Init( SMQ *smq);                                            /* index initial */
int         Smq_FindIndex( SMQ *smq, char *name);                           /* find index - if( name == NULL) 이면 빈 index return; */
int         Smq_WritePipe( SMQ *smq);                                       /* pipe로 부터 체결 record position을 수신 */
int         Smq_ReadPipe( SMQ *smq, int timeout);                           /* pipe로 부터 체결 record position을 수신 */
int         Smq_Send( SMQ *smq, char *rec, int sz);                         /* 주문 insert  */
int         Smq_Recv( SMQ *smq, char *rec, int sz, int timeout);            /* queue read */
int         Smq_Commit( SMQ *smq);                                          /* Smq_SetRecord/Smq_GetRecord 이후 Lock 해제 */
int         Smq_SetRecord( SMQ *smq, char *rec, int sz);                    /* 빈 record 하나를 allocate - Lock 유지, Smq_WriteRecord에서 Lock 해제 */
int         Smq_InsertRecord( SMQ *smq);                                    /* Smq_SetRecord 이후 Lock 해제 및 insert */
int         Smq_GetRecord( SMQ *smq, char *rec, int sz, int timeout);       /* current queue get - Lock 유지, Smq_DeleteRecord에서 Lock 해제 */
int         Smq_DeleteRecord( SMQ *smq);                                    /* current queue get - Lock 유지, Smq_DeleteRecord에서 Lock 해제 */
int         Smq_GetEmptyRecordPos( SMQ *smq);                               /* 빈 record 찾기 */
SMQ_RECORD* Smq_GetRecordByPos( SMQ *smq, int pos);                         /* position을 입력하여 record 찾기 */
int         Smq_GetCurrentInt( SMQ *smq, char *current);                    /* 통화 string을 int로 환산 */
int         Smq_MemLock( SMQ *smq);                                         /* semaphore lock 수행 */
int         Smq_MemUnlock( SMQ *smq);                                       /* semaphore lock 해제 */
int         Smq_Lock( SMQ *smq);                                            /* semaphore lock 수행 */
int         Smq_Unlock( SMQ *smq);                                          /* semaphore lock 해제 */
int         Smq_StatisticsSet( SMQ *smq, SMQ_STEL *stel, int opt);          /* 통계 setting */
int         Smq_TimeGap( SMQ *smq, struct timeval *tv_1, struct timeval *tv_2);/* 시간 차이 구하기 */
int         Smq_Stat( SMQ *smq);                                            /* stat 출력 */

/***** Module : mon.c *****/
int         Mon_Main( SMQ *smq);                                            /* 함수 작성    */
int         Mon_MainInit( MAP *map, SMQ *smq);                              /* 모니터링맵 초기화 */
int         Mon_TimeSet( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_TimeStr( MAP *map, MAP_FIELD *field);                       /* time stemp convert */

