#ifndef      __PY_CM_H
#define      __PY_CM_H
/*------------------------------------------------------------------------
#	Module	: curses management tool
#	File	: py_cm.h
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		B_HOME		"¦®"
#define		B_PGUP		"¦¯"
#define		B_END		"¦±"
#define		B_PGDN		"¦°"
#define		B_2			"¦¬"
#define		B_1			"¦­"

#define		YES			1
#define		NO			0
#define		QUIT		-1
#define		KEY_BS		010					/* Backspace: = 0x08 = 8	*/
#define		KEY_TAB		011					/* Tab: = 0x09 = 9			*/
#define		KEY_DEL		0177				/* Del: = 0x7f = 127		*/
#define		KEY_ESC		033					/* Esc: = 0x1b = 27			*/

#define		PGMLIN		__FILE__,__LINE__			/* file name, line	*/

typedef struct {
	char	p_name[12];						/* process name				*/
	char	tr_e_tm[8];						/* last TR time (hhmmss)	*/
	char	exist_flag;						/* '*':run  ' ':stop		*/
	char	run_flag;						/* 0:process_no == 0		*/
	int		seq;							/* interface sequence		*/
	char	p_info[40];						/* process information		*/
}	P_FMT;	/* process	*/

typedef struct {
	char	f_name[12];						/* file name				*/
	int		f_key;							/* file key					*/
	char	pile_flag;						/* data piled up			*/
	char	type;							/* data type (0:DSHM, 1:SAM)*/
	int		sm_rcnt;						/* sync manager read count	*/
	int		rcnt[9];						/* read count				*/
	int		wcnt;							/* write count				*/
	u_char	f_cnt;							/* fifo number				*/
	short	rec_sz;							/* record size				*/
	char	f_info[40];						/* file information			*/
}	F_FMT;	/* file	*/

/*************************************************************************
	End of File (py_cm.h)
*************************************************************************/
#endif
