#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <errno.h>

#include "mem.h"

#include "proc.h"
#include "mrgnsise.h"

#define CUST_MRGN_KEY   (key_t) 0x6365303031     //ct001 
#define HDOF_MRGN_KEY   (key_t) 0x6874303031     //ht001
#define DRTR_MRGN_KEY   (key_t) 0x6474303031	 //dt001

typedef struct _mrg_map_
{
	int				grp_cnt;
	FXGRP_ST		grp[ MAX_GRP_CNT];
}	MRG_MAP;

typedef struct _mrg_
{
	key_t		key;				/* shared memory key */
	MEM			*mem;				/* shm struct */
	MRG_MAP		*map;
}	MRG;



