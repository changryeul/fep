#ifndef		__DAEMON_H
#define		__DAEMON_H
/*------------------------------------------------------------------------
#	Module	: common variables and functions used by daemons
#	File	: daemon.h
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

#else

extern int		Start_HH;
extern int		Start_MM;
extern int		Start_SS;
extern int		End_HH;
extern int		End_MM;
extern int		End_SS;

#endif

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
extern void		Check_Argument (int argc, char *argv[]);
extern void		Sub_SHM (void);
extern void		Mem_SHM (int, int);
extern void		Sys_Config_Read (void);

extern void		Daemon_Config_Read (int);
extern void		File_Config_Read (int);
extern void		Dshm_Config_Read (int);
#if defined ISAM_INCL
extern void		Cisam_Config_Read (int);
#endif
extern void		Tcp1_Config_Read (int);
extern void		Tcp2_Config_Read (int);
extern void		Udpip_Config_Read (int);
extern void		SiseTr_Config_Read (int);
extern void		Accno_Config_Read (int);
extern void		Proc_Config_Read (int);

/*************************************************************************
	End of Program (daemon.h)
*************************************************************************/
#endif
