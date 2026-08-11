#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "shm_memory.h"

#include "log.h"
#include "cfg.h"
#include "etc.h"
#include "map.h"
#include "win.h"

#include "blp.h"

/***** Module : mon.c *****/
int         Mon_Main( BLP *blp, char *map_name);                            /* 함수 작성    */
int         Mon_MainInit( MAP *map, BLP *blp);                              /* 모니터링맵 초기화 */
int         Mon_Hoga( MAP *main_map, BLP *blp, int tbl_no);                 /* 함수 작성    */
int         Mon_HogaProc( MAP *map, BLP *blp, int tbl_no);                  /* 모니터링맵 초기화 */
int         Mon_HogaInit( MAP *map, BLP *blp, int tbl_no);                  /* 모니터링맵 초기화 */
int         Mon_Jang( MAP *main_map, BLP *blp, BLP_TBL *tbl, int tbl_no);   /* 함수 작성    */
int         Mon_JangInit( MAP *map, BLP *blp, BLP_TBL *tbl, int tbl_no);    /* 모니터링맵 초기화 */
int         Mon_HogaStat( MAP *map, MAP_FIELD *field);                      /* time stemp convert */
int         Mon_MkStat( MAP *map, MAP_FIELD *field);                        /* time stemp convert */
int         Mon_OrdStat( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_OrdStat2( MAP *map, MAP_FIELD *field);                      /* time stemp convert */
int         Mon_OrdResp( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_OrdSide( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_TblMode( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_TblModeNext( MAP *map, MAP_FIELD *field);                   /* time stemp convert */
int         Mon_TblStat( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_TimeSet( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_TimeHour( MAP *map, MAP_FIELD *field);                      /* time stemp convert */
int         Mon_TimeStr( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_TimeProc( MAP *map, MAP_FIELD *field);                      /* time stemp convert */
int         Mon_GapTime( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_Current( MAP *map, MAP_FIELD *field);                       /* time stemp convert */
int         Mon_IndexChk( MAP *map, MAP_FIELD *field);                      /* time stemp convert */

