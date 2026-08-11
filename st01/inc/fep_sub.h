#ifndef		__FEP_SUB_H
#define		__FEP_SUB_H
/*------------------------------------------------------------------------
#	Module	: common constants, variables and functions used in library
#	File	: fep_sub.h
------------------------------------------------------------------------*/
 
/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#if defined __linux
#define _FILE_OFFSET_BITS 64
#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif
#define _LARGEFILE64_SOURCE
#define _LARGE_FILES

#if defined(sun) || defined(_AIX) || defined(__linux__)
#define _GNU_SOURCE
#endif

/* Start of XML (removed: libxml includes - unused) */

/* NOFILE include */
#include	<sys/param.h>
/* fd_set include */
#include	<sys/select.h>
#endif

/* common headers	*/
#include	<errno.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdarg.h>
#include	<math.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<sys/ipc.h>
#include	<sys/msg.h>
#include	<sys/sem.h>
#include	<sys/shm.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<sys/ioctl.h>
#include	<unistd.h>
#include	<sys/timeb.h>
#include	<poll.h>

#include	<stdint.h>

#ifndef u_short
typedef uint16_t u_short;
#endif

#ifndef u_char
typedef uint8_t u_char;
#endif


#if defined(sun) || defined(_AIX)
#include	<termio.h>
#include	<sys/termio.h>
#elif defined __linux
#include	<termios.h>
#include	<sys/termios.h>
/* 202108
#include	<openssl/md5.h>
*/
#endif

#if defined __hpux
#include    <sys/syscall.h>
#include    <sys/pstat.h>
#elif defined(sun) || defined(__linux)
#include    <stdlib.h>
#include    <dirent.h>
#include    <limits.h>
#include    <sys/syscall.h>
#include    <sys/procfs.h>
#elif defined _AIX
#include    <procinfo.h>
#endif

#if defined __hpux
#include	<sys/time.h>
#include	<string.h>
#include	<sys/fcntl.h>
#elif defined(sun) || defined(_AIX)
#include	<time.h>
#include	<string.h>
#include	<fcntl.h>
#elif defined __linux
#include	<time.h>
#include	<string.h>
#include	<fcntl.h>
/* 2025 Add */
#include	<sys/fcntl.h>
#include	<sys/time.h>
/* 2025 Add */
#endif

/* TCP/IP headers	*/
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netinet/tcp.h>
#include	<arpa/inet.h>
#include	<netdb.h>

/* X.25 headers	*/
/*
#if defined __hpux
#include	<x25/x25.h>
#include	<x25/x25addrstr.h>
#include	<x25/x25ioctls.h>
#include	<x25/x25str.h>
#include	<x25/ccittproto.h>
#elif defined sun
#include	<sys/stropts.h>
#include	<netx25/uint.h>  
#include	<netx25/timer.h>  
#include	<netdlc/ll_proto.h> 
#include	<netdlc/ll_control.h> 
#include	<netx25/x25_proto.h>  
#include	<netx25/x25_control.h> 
#elif defined _AIX
#include	<sys/twtypes.h>
#ifndef _SQL
#include	<sys/twlib.h>
#endif
#include	<sys/stream.h>
#include	<sys/stropts.h>
#include	<sys/npi_20.h>
#include	<sys/npiapi.h>
#include	<sys/pktintf.h>
#endif
*/

/* IT headers	*/
#include	<ctype.h>
#ifndef _SQL
#include	<string.h>
#endif

#if defined __hpux
#include	<varargs.h>
#include	<curses_colr/curses.h>
#elif defined(sun) || defined(_AIX)
#ifndef _SQL
#include	<curses.h>
#endif
#elif defined __linux
#include	<curses.h>
#endif

/* C-ISAM header	*/
#if (0)
#include 	<isam.h>
#endif

/* OMS headers	*/
#include	"def_error.h"
#include	"shm_memory.h"
#include	"fep_file.h"
#include	"krx_mk.h"
#include	"pa_struct.h"
#include	"fep_tcpip.h"
#include	"cli_interface.h"
#include	"key_code.h"
#include	"strategy.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#if !defined(OK) || ((OK) != 0)
#ifdef		OK
#undef		OK
#endif
#define 	OK				0
#endif

#define 	NOTOK			-1
#define 	FAIL			1

#define		OFF				0
#define		ON				1
#define		END				2		

#define 	SIZE_MB			((int)1024*1024)					/* 1 MB	*/
#define 	SIZE_GB 		((int)1024*1024*1024)				/* 1 GB	*/
#define 	LOG_SIZE 		5120
#define		LOG_HEAD_SIZE	42

#define		SHM_LOG_HEAD_SIZE	(sizeof (SHM_LOG_HEAD))
#define		SHM_LOG_SIZE		1057		/* head (33) + msg (1024)	*/
#define		SHM_LOG_MAX			10000

typedef struct {
	char	Length[4];							/* length				*/
	char	ErrCd[4];							/* error code			*/
	char	LogName[10];						/* log file name		*/
	char	Time[12];							/* time (HHMMSSmmmmmm)	*/
}   SHM_LOG_HEAD;

#define 	FEP_LOG 		1
#define 	FEP_DAT 		2
#define 	FEP_BIN 		3
#define 	FEP_TMP 		4
#define		FEP_CFG			5
#define		FEP_SHL			6
#define		FEP_FIFO		7

#define		TY_MP			1				/* Manager Process			*/
#define		TY_DD			2				/* Division Demanded		*/
#define		TY_TRS1			3				/* TCP/IP 1 Receive Send	*/
#define		TY_TRS2			4				/* TCP/IP 2 Receive Send	*/
#define		TY_BRS			5				/* DB Receive Send			*/
#define		TY_URS			6				/* UDP/IP Receive Send		*/

#define		Max(a,b)	((a>b)?a:b)
#define		PROCI		PROC(D_K,P_K)

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
#ifdef	_GLOBAL

char			*_FEP_LOG;						/* log path				*/
char			*_FEP_DAT;						/* data path			*/
char			*_FEP_BIN;			    		/* execution path		*/
char			*_FEP_TMP;						/* temporary path		*/
char			*_FEP_CFG;						/* configuration path	*/
char			*_FEP_SHL;						/* shell path			*/
char			*_FEP_FIFO;						/* FIFO path			*/
char			*_FEP_DIV;						/* env division			*/
char			_System_Name[2];				/* system name (upper)	*/
char			_SubSystem_Name[4];				/* sub name (lower)		*/
char			_Exe_Name[20];					/* execution name		*/
char			_Process_Name[20];				/* process name			*/
struct stat		_F_Info;						/* file information		*/

#else

extern char			*_FEP_LOG;
extern char			*_FEP_DAT;
extern char			*_FEP_BIN;
extern char			*_FEP_TMP;
extern char			*_FEP_CFG;
extern char			*_FEP_SHL;
extern char			*_FEP_FIFO;
extern char			*_FEP_DIV;
extern char			_System_Name[2];
extern char			_SubSystem_Name[4];
extern char			_Exe_Name[20];
extern char			_Process_Name[20];
extern struct stat	_F_Info;

#endif

/*------------------------------------------------------------------------
	Function Prototype
------------------------------------------------------------------------*/
#if defined __hpux
extern pid_t   Check_Proc_HPUX (char *);
#elif defined sun
extern pid_t   Check_Proc_SUN (char *);
extern int     Make_List (char *, char *);
#elif defined _AIX
extern pid_t   Check_Proc_AIX (char *);
#elif defined __linux
extern pid_t   Check_Proc_LINUX (char *);
extern int     Make_List_Linux (char *, char *);
#endif
extern pid_t	Check_Proc (char *);

extern int		AtoIf (char*, int);
extern long		AtoLf (char*, int);
extern double	AtoDf (char*, int);
extern char		*ItoAf (int, char *, int);
extern char		*LtoU (char *, int);
extern char		*UtoL (char *, int);
extern void     Init_Proc(int argc, char *argv[]);
extern char     *Get_DateTime (char *);
extern char     *Get_MicroTime (char *);
extern char     *Get_Time (char *);
extern int		Chk_Korean (char *, int);
extern void		Set_TR_Time (void);

extern int		Get_Environment (void);
extern void		Log (int, const char *, ...);
extern void		Log_Proc (char *, char *);
extern void		Log_Emergency (char *, char *);
extern void		Log_Save (char *, int);
extern void		SLog (int, const char *, ...);
extern void		Write_SLog (char *p_msg);
extern int		Log_Hot_Mode (void);
extern void		Log_Hot (int, const char *, ...);
extern int		Seq_Throttle_Interval (void);
extern int		Seq_Throttle_Check (const char *, long);
extern void		Seq_Save_Flush (void);
extern int		Fd_Cache_Get (const char *, int, int);
extern FILE		*Fp_Cache_Get (const char *, const char *);
extern void		Fd_Cache_Close_All (void);

extern int		SHM_Creat (key_t, size_t);
extern int		SHM_Creat_Excl (key_t, size_t);
extern char		*SSHM_RemoveHM_Attach (int);
extern void		SHM_Detach (char *);
extern int		SHM_Remove (int);
extern char		*Shm_Attach (key_t, int *);
extern char		*SHM_Creat_Attach (key_t, size_t, int *);
extern char     *SHM_Attach (int);
extern char		*SHM_Attach_Verify (int, size_t);
extern char		*Shm_Map_SubDaemon (char *, ALL_DAEMON_INFO *, SHM_MEMORY *);
extern size_t	Shm_Calc_SubDaemon_Size (ALL_DAEMON_INFO *);
extern int		Shm_Check_Version (int);

extern int		Socket (void);
extern int		Bind (int, int);
extern int		Listen (int);
extern int		Accept (int, char *, u_short *);
extern int		Connect (int, char *, int);
extern int		Select (int, int, int);
extern int		Sendn (int, char *, int);
extern int		Recvn (int, char *, int);

extern int		F_R (int, char *, int);
extern int		F_W (int, char *, int);
extern int		F_W_Proc (int, char *, int, int);
extern void		Add_Count (int, int);
extern int		F_R2 (char *, int);
extern void		F_W2 (char *, char *);
extern int		F_W2_Proc (int, char *, int, int);
extern int		F_R3 (int, int, char *);
extern int		F_W3 (int, int, char *);
extern int		B_F_R (char *, int, char *, int);
extern int		B_F_W (char *, int, char *, int);
extern int		F_WB (int, int, char *, int);
extern int		SF_W (int, char *);
extern int		SF_W_Proc (int, char *, int);

extern int		Key_Search (int , int, char *);
extern int		Key_Search_FX (char *, char *);		/* FX 시세 slot 검색/등록 (fx_util.c) */
extern int		CmpExpcode (const void *, const void *);
extern int		CmpLongcode (const void *, const void *);
extern int		BsearchMode (char *, char *, int, int, int (*compar)(), int);
extern void		Init_Mana(int argc, char *argv[]);

extern	int     DSHM_R (int, char *, int);
extern	void    Dshm_Add_Count (int, int);
extern	int     DSHM_W (int, char *, int);
extern	int     DSHM_W2 (int, char *, int);
extern	int     F_R_Proc (int, char *, int, int);
extern	int     Dshm_Seq_Save (char *, int, int, int);
extern	void    Exit_Process (void);
extern  void	End_Routine (int);
extern volatile sig_atomic_t _in_signal_handler;

/* Write a static string to stderr from signal handler (async-signal-safe) */
#define SIG_WRITE_MSG(msg) write(STDERR_FILENO, msg, sizeof(msg) - 1)
extern  int		SEM_Creat_Excl (key_t);
extern	int     SEM_Creat (key_t);
extern	int     SSEM_Creat_ExclEM_Creat_Excl (key_t);
extern	int     SEM_Lock (int);
extern	int     SEM_UnLock (int);
extern	int		DSHM_WT (int, char *, int);
extern	char	*Get_DateMilliTime (char *);

#ifndef STAT_SAVE_INTERVAL_SEC
#define STAT_SAVE_INTERVAL_SEC  3
#endif
extern void		Stat_Save (void);
extern void		Stat_Save_Force (void);
extern  int		Seq_Save (char *, int, int);
extern void		Check_Exist (void);
extern void		Check_Environment (void);
extern void		Setsigfatal (void);
extern  int		Select_Receive (int, char *);
extern  int		Sise_Select_Receive (int, char *);
extern  int		Select_Receive_Krx(int, char *, int);
extern  int		Select_Receive_Cli(int, char *);
extern  int		Select_Receive_Imeco(int, char *);
extern  int		Select_Receive_Imeco_Sise(int, char *);
extern  int		Select_Send (int, char *, int);
extern  int		Poll_File (int);
extern	int		Create_FIFO (const char *);
extern	int		Create_Dir (const char *);

/*************************************************************************
	End of Program (fep_sub.h)
*************************************************************************/
#endif
