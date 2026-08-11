/*******************************************************************************
 * (C) COPYRIGHT Winway Co., Ltd. 2020
 * All Rights Reserved
 * Licensed Materials - Property of Winway
 *
 * This program contains proprietary information of Winway System.
 * All embodying confidential information, ideas and expressions can't be
 * reproceduced, or transmitted in any form or by any means, electronic,
 * mechanical, or otherwise without the written permission of Winway System.
 *
 *  Components  : mrgnsise.c
 *  Rev. History: 
 *      Ver     Date    Information
 *      ------- ------- -----------------------------------------------
 *      1.00    2023-10 Winway initial version.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <errno.h>
#include <unistd.h>

#include "log.h"

#include "mrg.h"
#include "mrgnsise.h"

MRG *Mrg_Create( key_t key)
{
	MRG		*mrg;

	mrg = malloc( sizeof( MRG));
	if( mrg == NULL)
	{
		LogErr( "malloc");
		goto error;
	}
	memset( mrg, 0x00, sizeof( MRG));

	mrg->mem = Mem_Create( key, sizeof( MRG_MAP));
	if( mrg->mem == NULL)
	{
		LogCri( "Mem_Create error. key=[0x%08x]", key);
		goto error_1;
	}
	mrg->map = Mem_GetPtr( mrg->mem);

	return mrg;

	error_1:
		free( mrg);
	error:
		return NULL;
}

MRG *Mrg_Open( key_t key)
{
	MRG		*mrg;

	mrg = malloc( sizeof( MRG));
	if( mrg == NULL)
	{
		LogErr( "malloc");
		goto error;
	}
	memset( mrg, 0x00, sizeof( MRG));

	mrg->mem = Mem_Open( key);
	if( mrg->mem == NULL)
	{
		LogCri( "Mem_Open error. key=[0x%08x]", key);
		goto error_1;
	}
	mrg->map = Mem_GetPtr( mrg->mem);


	return mrg;

	error_1:
		free( mrg);
	error:
		return NULL;
}

int f_create_custmrgn(cstptr, s_errcd)
CUSTMRGN_SHM_ST  *cstptr;
char            *s_errcd;
{
	int n_id;

	n_id = shmget(CUST_MRGN_KEY, CUSTMRGN_SHM_ST_SZ + 10000000, IPC_CREAT | IPC_EXCL | 0666);
	if (n_id < 0)
	{
		LogDbg("고객 SHM 생성 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	}

	cstptr = shmat(n_id, NULL, IPC_CREAT | 0666);
	if (cstptr == (void *)-1)
	{
		LogDbg("고객 SHM attach 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	} 

	return (0);
}

int f_rget_custmrgn(cstptr, s_errcd)
CUSTMRGN_SHM_ST  *cstptr;
char            *s_errcd;
{
	int n_id;

	n_id = shmget(CUST_MRGN_KEY, 0 /* CUSTMRGN_SHM_ST_SZ */, 0666);
	if (n_id < 0)
	{
		LogDbg("고객 SHM get 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	}

	cstptr = shmat(n_id, NULL, SHM_RDONLY);
	if (cstptr == (void *)-1)
	{
		LogDbg("고객 SHM attach 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	} 

	return 0;
}

int f_get_wcustmrgn(cstptr, s_errcd)
CUSTMRGN_SHM_ST  *cstptr;
char            *s_errcd;
{
	int n_id;

	n_id = shmget(CUST_MRGN_KEY, 0 /* CUSTMRGN_SHM_ST_SZ */, 0666);
	if (n_id < 0)
	{
		LogDbg("고객 SHM get 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	}

	cstptr = shmat(n_id, NULL, 0666);
	if (cstptr == (void *)-1)
	{
		LogDbg("고객 SHM attach 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	} 

	return 0;
}

int f_remove_custmrgn(s_errcd)
char *s_errcd;
{
	int	n_id;
	int rtn;
	struct shmid_ds buf;

	n_id = shmget(CUST_MRGN_KEY, 0, 0666);
	if (n_id < 0)
	{
		LogDbg("고객 SHM get 오류 [%d][%s]\n", errno, strerror(errno));
		return -1;
	}

	rtn = shmctl(n_id, IPC_RMID, &buf);
	if (n_id < 0)
	{
		LogDbg("고객 SHM remove 오류 [%d][%s]\n", errno, strerror(errno));
		return -1;
	}	

	return 0;
}


int f_create_hdomgrp(p_hdomgrp, s_errcd)
HDOMGRP_SHM_ST  *p_hdomgrp;
char            *s_errcd;
{
	int n_id;

	void *ptr;

	LogDbg( "AAAA shm create key=[0x%08x]\n", HDOF_MRGN_KEY);
	n_id = shmget(HDOF_MRGN_KEY, HDOMGRP_SHM_ST_SZ + 10000000, IPC_CREAT | IPC_EXCL | 0666);
	if (n_id < 0)
	{
		LogDbg("본점마진 SHM 생성 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	}
	LogDbg( "AAAA shm create key=[0x%08x] ... success id=[%d]\n", HDOF_MRGN_KEY, n_id);

	ptr = shmat(n_id, NULL, IPC_CREAT | 0666);
	if (ptr == (void *)-1)
	{
		LogDbg("본점마진 SHM create attach 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	} 
	LogDbg( "attach mem ptr=[%p]\n", ptr);

	p_hdomgrp = (HDOMGRP_SHM_ST *)ptr;

	memset(p_hdomgrp, 0x00, HDOMGRP_SHM_ST_SZ);

	return 0;
}

int f_rget_hdomgrp(p_hdomgrp, s_errcd)
HDOMGRP_SHM_ST  *p_hdomgrp;
char            *s_errcd;
{
	int n_id;

	void *ptr;

	n_id = shmget(HDOF_MRGN_KEY, 0, 0666);
	if (n_id < 0)
	{
		LogDbg("본점마진 SHM read get 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	}

	ptr = shmat(n_id, NULL, SHM_RDONLY);
	if (ptr == (void *)-1)
	{
		LogDbg("본점마진 SHM read attach 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	} 

	p_hdomgrp = (HDOMGRP_SHM_ST *)ptr;

	return 0;
}

int f_wget_hdomgrp(p_hdomgrp, s_errcd)
HDOMGRP_SHM_ST  *p_hdomgrp;
char            *s_errcd;
{
	int n_id;

	void *ptr;

	n_id = shmget(HDOF_MRGN_KEY, 0, 0666);
	if (n_id < 0)
	{
		LogDbg("본점마진 SHM write get 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	}

	ptr = shmat(n_id, NULL, 0666);
	if (ptr == (void *)-1)
	{
		LogDbg("본점마진 SHM write attach 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	} 

	p_hdomgrp = (HDOMGRP_SHM_ST *)ptr;

	return 0;
}

int f_remove_hdomgrp(s_errcd)
char *s_errcd;
{
	int	n_id;
	int rtn;
	struct shmid_ds buf;

	n_id = shmget(HDOF_MRGN_KEY, 0, 0666);
	if (n_id < 0)
	{
		LogDbg("본점마진 SHM get 오류 [%d][%s]\n", errno, strerror(errno));
		return -1;
	}

	rtn = shmctl(n_id, IPC_RMID, &buf);
	if (n_id < 0)
	{
		LogDbg("본점마진 SHM remove 오류 [%d][%s]\n", errno, strerror(errno));
		return -1;
	}	

	return 0;
}



int f_create_drtrhdom(cstptr, s_errcd)
DRTRHDOM_SHM_ST  *cstptr;
char            *s_errcd;
{
	int n_id;

	n_id = shmget(DRTR_MRGN_KEY, DRTRHDOM_SHM_ST_SZ + 10000000, IPC_CREAT | IPC_EXCL | 0666);
	if (n_id < 0)
	{
		LogDbg("본점마진 SHM 생성 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	}

	cstptr = shmat(n_id, NULL, IPC_CREAT | 0666);
	if (cstptr == (void *)-1)
	{
		LogDbg("본점마진 SHM attach 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	} 

	return 0;
}

int f_rget_drtrhdom(cstptr, s_errcd)
DRTRHDOM_SHM_ST  *cstptr;
char            *s_errcd;
{
	int n_id;

	n_id = shmget(DRTR_MRGN_KEY, 0 /* DRTRHDOM_SHM_ST_SZ */, 0666);
	if (n_id < 0)
	{
		LogDbg("본점마진 SHM get 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	}

	cstptr = shmat(n_id, NULL, SHM_RDONLY);
	if (cstptr == (void *)-1)
	{
		LogDbg("본점마진 SHM attach 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	} 

	return 0;
}

int f_wget_drtrhdom(cstptr, s_errcd)
DRTRHDOM_SHM_ST  *cstptr;
char            *s_errcd;
{
	int n_id;

	n_id = shmget(DRTR_MRGN_KEY, 0 /* DRTRHDOM_SHM_ST_SZ */, 0666);
	if (n_id < 0)
	{
		LogDbg("본점마진 SHM get 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	}

	cstptr = shmat(n_id, NULL, 0666);
	if (cstptr == (void *)-1)
	{
		LogDbg("본점마진 SHM attach 오류[%d][%s]\n", errno, strerror(errno));
		return -1;
	} 

	return 0;
}

int f_remove_drtrhdom(s_errcd)
char *s_errcd;
{
	int	n_id;
	int rtn;
	struct shmid_ds buf;

	n_id = shmget(DRTR_MRGN_KEY, 0, 0666);
	if (n_id < 0)
	{
		LogDbg("본점마진 SHM get 오류 [%d][%s]\n", errno, strerror(errno));
		return -1;
	}

	rtn = shmctl(n_id, IPC_RMID, &buf);
	if (n_id < 0)
	{
		LogDbg("본점마진 SHM remove 오류 [%d][%s]\n", errno, strerror(errno));
		return -1;
	}	

	return 0;
}

