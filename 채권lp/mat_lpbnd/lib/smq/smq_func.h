/***** Module : smq.c *****/
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

