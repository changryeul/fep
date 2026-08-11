/** ***************************************************************************
**  @file       tree.h
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  tree header
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

#include "log.h"
#include "dll.h"
#include "cfg.h"
#include "etc.h"
#include "map.h"

#ifndef TREE_H
#define	TREE_H	1

/** ***************************************************************************
**  TREE STRUCTURE
***************************************************************************** */

#endif	/* TREE_H */

/***** Module : tree.c *****/
int         Tree_ScanDir( char *path, FILE *fp);                            /* 인수로 받은 디렉토리의 하위 디렉토리를 스캔하여 파일에 저장 */
int         Tree_View( char *f_name);                                       /* directory 파일 list data 를 tree view로 출력 */
char**      Tree_Search( char *f_name, int argc, char *argv[], int *nfind); /* directory 파일 list data 를 tree view로 출력 */

