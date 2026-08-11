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
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#define _LARGE_FILES

/* Start of XML */
// #include <libxml/encoding.h>
// #include <libxml/xmlwriter.h>
// #include <libxml/parser.h>
// #include <libxml/xmlsave.h>
/* End of XML */

/* NOFILE include */
#include	<sys/param.h>
#include	<string.h>
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

#if defined sun || _AIX
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
#include	<sys/fcntl.h>
#include	<sys/time.h>
#elif defined sun || _AIX
#include	<fcntl.h>
#include	<time.h>
#elif defined __linux
#include	<fcntl.h>
#include	<time.h>
#endif

/* TCP/IP headers	*/
#include	<sys/socket.h>
#include	<netinet/in.h>
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
#elif defined sun || _AIX
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
extern char			_System_Name[2];
extern char			_SubSystem_Name[4];
extern char			_Exe_Name[20];
extern char			_Process_Name[20];
extern struct stat	_F_Info;

#endif

/*------------------------------------------------------------------------
	Function Prototype
------------------------------------------------------------------------*/
extern int		AtoIf (char*, int);
extern long		AtoLf (char*, int);
extern double	AtoDf (char*, int);
extern char		*ItoAf (int, char *, int);
extern char		*LtoU (char *, int);

extern int		Get_Environment (void);
extern void		Log (int, const char *, ...);
extern void		SLog (int, const char *, ...);

extern int		SHM_Creat (key_t, size_t);
extern int		SHM_Creat_Excl (key_t, size_t);
extern char		*SHM_Attach (int);
extern void		SHM_Detach (char *);
extern int		SHM_Remove (int);
extern char		*Shm_Attach (key_t, int *);
extern char		*SHM_Creat_Attach (key_t, size_t, int *);

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
extern void		Add_Count (int, int);
extern int		F_R2 (char *, int);
extern void		F_W2 (char *, char *);
extern int		F_R3 (int, int, char *);
extern int		F_W3 (int, int, char *);
extern int		B_F_R (char *, int, char *, int);
extern int		B_F_W (char *, int, char *, int);

extern int		Key_Search (int , int, char *);
extern int		CmpExpcode (const void *, const void *);
extern int		CmpLongcode (const void *, const void *);
extern int		BsearchMode (char *, char *, int, int, int (*compar)(), int);
/*************************************************************************
	End of Program (fep_sub.h)
*************************************************************************/
#endif
