#ifndef		__SUB_DAEMON_H
#define		__SUB_DAEMON_H
/*------------------------------------------------------------------------
#	Module	: common in sub daemons
#	File	: sub_daemon.h
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"
#include	"fep_interface.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
#ifdef	_GLOBAL

int 	Start_HH;				/* start time of sub daemon (hour)		*/
int 	Start_MM;				/* start time of sub daemon (minute)	*/
int 	Start_SS;				/* start time of sub daemon (in sec)	*/
int 	End_HH;					/* end time of sub daemon (hour)		*/
int 	End_MM;					/* end time of sub daemon (minute)		*/
int 	End_SS;					/* end time of sub daemon (in sec)		*/
size_t	Shmsize;				/* shared memory size					*/
char	*Shmptr;				/* shared memory pointer				*/

#else

extern int		Start_HH;
extern int		Start_MM;
extern int		Start_SS;
extern int		End_HH;
extern int 		End_MM;
extern int 		End_SS;
extern size_t	Shmsize;
extern char		*Shmptr;

#endif

/*************************************************************************
	End of Program (sub_daemon.h)
*************************************************************************/
#endif
