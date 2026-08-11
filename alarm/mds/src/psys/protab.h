/******************************************************************************/
/*  Components  : protab.h						      */
/*  Description	: process table definition.				      */
/*  Rev. History: Ver	Date	Description				      */
/*		  ----	-------	--------------------------------------------- */
/*		  1.0	2006-07	Initial version				      */
/******************************************************************************/
#ifndef	_PROTAB_H
#define	_PROTAB_H

#define	MAX_PT		2		/* max. process tables		*/
#define	M_PROC		50		/* max. daemon processes	*/
#define	M_BOOT		20		/* max. boot processes		*/
#define	L_DESC		64		/* length of description	*/
#define	M_ARGC		8		/* max. argument #		*/

struct	bootab {			/* boot process	 table		*/
	int	nrec;			/* # of records			*/
	struct	{
		int	xpid;		/* process id			*/
		char	proc[128];	/* process name			*/
		char	desc[L_DESC];	/* description			*/
	} rec[M_BOOT];
};

struct	protab {			/* daemon/batch process table	*/
	int	cnfg;			/* configured ?			*/
	int	type;			/* process type ?		*/
	int	xpid;			/* process id			*/
	char	proc[128];		/* process name			*/
	short	tmhh;			/* running hour			*/
					/* -1 인 경우 시간 무시, 매시간	*/
	short	tmmm;			/* running minute		*/
	char	wday[8];		/* running week			*/
					/* [7]이 TRUE인 경우 매일	*/
	time_t	extm;			/* final execution timestamp	*/
	int	excc;			/* exeution count		*/
	char	chck[24*60];		/* execution time checked board	*/
	char	desc[L_DESC];		/* description			*/
};

/* protab.cnfg */
#define	_BAR_		0
#define	_RUN_		1
#define	_MOD_		2
#define	_DEL_		9

/* protab.type */
#define	T_BAS		0		/* basic(daemon) process	*/
#define	T_TIM		1		/* timely(batch) process	*/

/* section name */
#define	SECT_ENVS	"ENVS"		/* Environment section		*/
#define	SECT_BOOT	"BOOT"		/* On-boot job list		*/
#define	SECT_BASE	"BASE"		/* Daemon process list		*/
#define	SECT_TIME	"TIME"		/* Batch job list		*/

#endif
